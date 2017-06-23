#include "pulse.cc"
#include "TF1.h"


double fpulse1after (double* x, double* par)
{
    double xx = 17;
    double tau = 53;
    double a_0 = 39;
    double s = 7000;
    double t_0 = 25;
    double t = x[0] - t_0;

    //double y = adjust_maximum (tau, xx);
    double y = get_compensation (xx);
    //double y = par[1];

    if (x[0] < t_0) return a_0;

    return a_0 + s * (pulse (tau, xx, y, t) );
}

double fpulsedeconv1after (double* x, double* par)
{
    double xm =  (x[0] - 25);
    double xp =  (x[0] + 25);
    double xz =  x[0];
    return 1.2131 * fpulse1after (&xp, par) - 1.4715 * fpulse1after (&xz, par) + 0.4463 * fpulse1after (&xm, par);
}

double fpulse1before (double* x, double* par)
{
    double xx = 17;
    double tau = 63;
    double a_0 = 48;
    double s = 7000;
    double t_0 = 24;
    double t = x[0] - t_0;

    //double y = adjust_maximum (tau, xx);
    double y = get_compensation (xx);
    //double y = par[1];

    if (x[0] < t_0) return a_0;

    return a_0 + s * (pulse (tau, xx, y, t) );
}

double fpulsedeconv1before (double* x, double* par)
{
    double xm =  (x[0] - 25);
    double xp =  (x[0] + 25);
    double xz =  x[0];
    return 1.2131 * fpulse1before (&xp, par) - 1.4715 * fpulse1before (&xz, par) + 0.4463 * fpulse1before (&xm, par);
}
void make_shape()
{
    TF1* f_peakafter = new TF1 ("peak_after", fpulse1after, 1, 200, 0);
    f_peakafter->SetMaximum (4000);
    f_peakafter->SetMinimum (-600);
    TF1* f_decoafter = new TF1 ("deco_after", fpulsedeconv1after, 1, 200, 0);
    f_peakafter->SetLineColor (2);
    f_decoafter->SetLineColor (46);
    TF1* f_peakbefore = new TF1 ("peak_before", fpulse1before, 1, 200, 0);
    TF1* f_decobefore = new TF1 ("deco_before", fpulsedeconv1before, 1, 200, 0);
    f_peakbefore->SetLineColor (4);
    f_decobefore->SetLineColor (9);


    TCanvas* a = new TCanvas();
    a->cd();
    f_peakafter->Draw();
    f_decoafter->Draw ("same");
    f_peakbefore->Draw ("same");
    f_decobefore->Draw ("same");
    TLegend* legend = new TLegend (0.7, 0.7, 0.9, 0.9);
    legend->SetHeader ("PulseShapes simulated", ""); // option "C" allows to center the header
    legend->AddEntry ("peak_after", "Peak mode V_{fb} = 0", "l");
    legend->AddEntry ("peak_before", "Peak mode V_{fb} = 30", "l");
    legend->AddEntry ("deco_after", "Deco mode V_{fb} = 0", "l");
    legend->AddEntry ("deco_before", "Deco mode V_{fb} = 30", "l");
    legend->Draw ("same");

}
