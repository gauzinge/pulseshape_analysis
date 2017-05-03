#include "TF1.h"

double pulse_raw (double tau,
                  double x, double y, double a_0, double s, double t_0, double t)
{
    double result1, result2, result3;
    t -= t_0;

    // term 2
    result1 = tau * y * exp (-t / y);
    result1 /= pow (y, 2) - (x + tau) * y + tau * x;

    // term 3
    result2 = tau * x * exp (-t / x);
    result2 /= pow (x, 2) - (x - tau) * y - tau * x;

    //term 1
    result3 = pow (tau, 2) * exp (-t / tau);
    result3 /= pow (tau, 2) + (x - tau) * y - tau * x;
    return a_0 + s * (result1 + result2 + result3);
}

double pulse_x0 (double tau,
                 double y, double a_0, double s, double t_0, double t)
{
    t -= t_0;
    return a_0 + s * (tau / (y - tau) * (exp (-t / y) - exp (-t / tau) ) );
}

double pulse_ytau (double tau,
                   double x, double a_0, double s, double t_0, double t)
{
    t -= t_0;
    double result1, result2;
    result1 = exp (-t / x) - exp (-t / tau);
    result1 *= tau * x / (tau - x);
    result2 = t * exp (-t / tau);
    return a_0 + s * ( (result1 + result2) / (tau - x) );
}

double pulse_x0_ytau (double tau, double a_0, double s, double t_0, double t)
{
    t -= t_0;
    return a_0 + s * (t / tau * exp (-t / tau) );
}

double pulse (double tau,
              double x, double y, double a_0, double s, double t_0, double t)
{
    if (x > y)
    {
        double pivot = x;
        x = y;
        y = pivot;
    }

    if ( (x == 0) && (y == tau) ) return pulse_x0_ytau (tau, a_0, s, t_0, t);
    else if (x == 0) return pulse_x0 (tau, y, a_0, s, t_0, t);
    else if (y == tau) return pulse_ytau (tau, x, a_0, s, t_0, t);
    else return pulse_raw (tau, x, y, a_0, s, t_0, t);
}

double fpulse (double* x, double* par)
{
    double xx = par[0];
    double y = par[1];
    double tau = par[2];
    double a_0 = par[3];
    double s = par[4];
    double t_0 = par[5];

    if (x[0] < t_0) return a_0;

    return pulse (tau, xx, y, a_0, s, t_0, x[0]);
}

TH1F* getHist (std::string pDirectory, int pChannel)
{
    if (pChannel > 9)
    {
        std::cout << "Error, Channel must be smaller than 10 for this test case!" << std::endl;
        return nullptr;
    }

    //open the summary file
    TFile* cFile = TFile::Open ("Summary.root");

    TDirectory* cDir;
    gDirectory->GetObject (pDirectory.c_str(), cDir);

    if (cDir == nullptr)
    {
        std::cout << "Error, Directory " << pDirectory << " does not exist!" << std::endl;
        return nullptr;
    }

    std::string cHistName = Form ("Channel_%1d", pChannel);
    TH1F* cHist;
    cDir->GetObject (cHistName.c_str(), cHist);

    if (cHist == nullptr)
        std::cout << "Error, histogram does not exist -- something else went wrong here!" << std::endl;
    else
    {
        // invert histogram
        cHist->Scale (-1.);
        //cHist->GetXaxis()->SetRangeUser (-600, 3000);
        cHist->SetMinimum (-600);
    }

    return cHist;
}
void fit (TH1* pHist)
{

    TF1* a = new TF1 ("test", fpulse, 0, 200, 6);
    a->SetParNames ("PA time constant x         ", "shaper RC time constant y  ", "shaper CR time constant tau", "baseline                   ", "scale                      ", "turn on time               " );
    a->SetParLimits (0, 1, 30); //x
    a->SetParLimits (1, 1, 30); //y
    a->FixParameter (2, 50);//tau
    a->SetParLimits (3, -500, 500);//baseline
    a->SetParLimits (4, 0, 10000 );//scale
    a->SetParLimits (5, 0, 50); // turn-on time
    a->SetParameters (15, 30, 50, 180, 560, 23);
    pHist->Fit (a, "RM+");
    pHist->Draw();
    std::cout << a->GetChisquare() / a->GetNDF() << std::endl;
}

void pulse()
{
    TH1F* cPeakBefore = getHist ("Peak_before", 0);
    fit (cPeakBefore);
}
