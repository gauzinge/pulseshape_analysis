#include <TF1.h>
#include <TMath.h>

double fpeak_all (double* x, double* par)
{
    if (x[0] + par[5] - 10 < 0) return par[0] / (1 + TMath::Exp (-par[1] * (x[0] - par[2]) ) ) + par[3];
    else return par[4] + par[6] *  (x[0] + par[5])  * TMath::Exp (-  (x[0] + par[5]) / par[7]) ;

    //cFit->SetParLimits (0, pAmplitude / 10., pAmplitude * 10); //amplitude of sigmoid
    //cFit->SetParLimits (1, 0.05, 0.5);//steepness/width
    //cFit->SetParLimits (2, 50 - 20, 50 + 20);//turn on
    //cFit->SetParLimits (3, -pAmplitude / 10., pAmplitude / 10.);//baseline

    //cFit->SetParLimits (4, -100, 500);//baseline
    //cFit->SetParLimits (5, -200, 0);//turn on time
    //cFit->SetParLimits (6, 0, 500);//scale
    //cFit->SetParLimits (7, 5, 100);//timeconstant
    //cFit->SetParameters ( pAmplitude / 2.0, 0.1, 40, 0,
    //150, -35, 180, 45 );
}

double fpeak (double* x, double* par)
{
    if (x[0] + par[1] < 0) return par[0];

    return par[0] +  par[2] *  (x[0] + par[1])  * TMath::Exp (- (x[0] + par[1]) / par[3]);
}

double fdeconv (double* x, double* par)
{
    double xm = par[4] * (x[0] - 25);
    double xp = par[4] * (x[0] + 25);
    double xz = par[4] * x[0];
    return 1.2131 * fpeak (&xp, par) - 1.4715 * fpeak (&xz, par) + 0.4463 * fpeak (&xm, par);
}
double fdeconv_undershoot (double* x, double* par)
{
    return  par[0] - par[2] *  (x[0] + par[1])  * TMath::Exp (- (x[0] + par[1]) / par[3]) ;
}

double fpeak_convoluted (double* x, double* par)
{
    TF1 f ("peak_convoluted", fpeak, 0, 200, 4);
    return f.IntegralError (x[0] - par[4] / 2., x[0] + par[4] / 2., par, 0, 0.1) / (par[4]);
}

double fdeconv_convoluted (double* x, double* par)
{
    double xm = (x[0] - 25);
    double xp = (x[0] + 25);
    double xz = x[0];
    return 1.2131 * fpeak_convoluted (&xp, par) - 1.4715 * fpeak_convoluted (&xz, par) + 0.4463 * fpeak_convoluted (&xm, par);
}

void correctDistribution (TH1* h)
{
    h->Scale (-1);
}
