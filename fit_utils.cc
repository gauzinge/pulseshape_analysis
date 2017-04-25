#include <TF1.h>
#include <TMath.h>

double fpeak (double* x, double* par)
{
    if (x[0] + par[1] < 0) return par[0];

    return par[0] + par[2] * (x[0] + par[1]) * TMath::Exp (- (x[0] + par[1]) / par[3]);
}

double fdeconv (double* x, double* par)
{
    double xm = par[4] * (x[0] - 25);
    double xp = par[4] * (x[0] + 25);
    double xz = par[4] * x[0];
    return 1.2131 * fpeak (&xp, par) - 1.4715 * fpeak (&xz, par) + 0.4463 * fpeak (&xm, par);
}

double fpeak_convoluted (double* x, double* par)
{
    TF1 f ("peak_convoluted", fpeak, 0, 200, 4);
    return f.IntegralError (x[0] - par[4] / 2., x[0] + par[4] / 2., par, 0, 1.) / (par[4]);
}

double fdeconv_convoluted (double* x, double* par)
{
    double xm = (x[0] - 25);
    double xp = (x[0] + 25);
    double xz = x[0];
    return 1.2131 * fpeak_convoluted (&xp, par) - 1.4715 * fpeak_convoluted (&xz, par) + 0.4463 * fpeak_convoluted (&xm, par);
}

//float maximum ( TH1* h )
//{
//int bin = h->GetMaximumBin();
//// fit around the maximum with the detector response and take the max from the fit
//TF1* fit = fitPulse (h, h->GetBinCenter (bin) - 25, h->GetBinCenter (bin) + 25);
//return fit->GetMaximumX();
//}

float turnOn ( TH1* h )
{
    // localize the rising edge
    int bin = 1;
    float amplitude = h->GetMaximum();

    for (; bin <= h->GetNbinsX() && h->GetBinContent (bin) < 0.4 * amplitude; ++bin) {}

    float end = h->GetBinLowEdge (bin);
    // fit the rising edge with a sigmoid
    TF1* sigmoid = new TF1 ("sigmoid", "[0]/(1+exp(-[1]*(x-[2])))+[3]", 0, end);
    sigmoid->SetParLimits (0, amplitude / 10., amplitude);
    sigmoid->SetParLimits (1, 0.05, 0.5);
    sigmoid->SetParLimits (2, end - 10, end + 10);
    sigmoid->SetParLimits (3, -amplitude / 10., amplitude / 10.);
    sigmoid->SetParameters (amplitude / 2., 0.1, end, 0.);
    h->Fit (sigmoid, "0QR");
    // return the point where the fit = 3% signal.
    float time = 0.;
    float base = sigmoid->GetMinimum (0, end);

    for (; time < end && (sigmoid->Eval (time) - base) < 0.03 * (amplitude - base); time += 0.1) {}

    delete sigmoid;
    return time - 0.05;
}
