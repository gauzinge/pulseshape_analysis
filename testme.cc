#include "TH1.h"
#include "TF1Convolution.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TMath.h"

//#include "fit_utils.cc"

//double fpeak (double* x, double* par)
//{
//if (x[0] + par[1] < 0) return par[0];

//return par[0] + par[2] * (x[0] + par[1]) * TMath::Exp (- (x[0] + par[1]) / par[3]);
//}

//double fdeconv (double* x, double* par)
//{
//double xm = par[4] * (x[0] - 25);
//double xp = par[4] * (x[0] + 25);
//double xz = par[4] * x[0];
//return 1.2131 * fpeak (&xp, par) - 1.5715 * fpeak (&xz, par) + 0.4063 * fpeak (&xm, par);
//}

//double fdeconvtest (double* x, double* par)
//{
//double xm = par[4] * (x[0] - 25);
//double xp = par[4] * (x[0] + 25);
//double xz = par[4] * x[0];
//return 0.44 * testfunc (&xm, par) - 1.47 * testfunc (&xz, par) + 1.21 * testfunc (&xp, par);
//}



void testme()
{
    double offset = 0.;
    //double sigmoid_scale = .4;
    //double sigmoid_max = 100;

    double time_const = 50;
    double crrc_scale = 20;
    double turn_on_time = 20;

    //TF1* sigmoid = new TF1 ("sigmoid", "[0]/(1+exp(-[1]*(x-[2])))+[3]", 0, 200);
    //sigmoid->SetParameters (sigmoid_max, sigmoid_scale, turn_on_time, offset);

    TF1* crrc = new TF1 ("crrc", "[0]+[1]*(x-[2])*exp(-(x-[2])/[3])", 0, 200);
    crrc->SetParameters (offset, crrc_scale, turn_on_time, time_const   );

    //TF1* a = new TF1 ("a", fdeconv, 0, 200, 5);
    //a->SetParameters (1, -65, 200, 40, 1.01);
    //a->SetParameters (offset, crrc_scale, turn_on_time, time_const, 20);

    TF1* b = new TF1 ("b", "-(x-50)*exp(-(x-50)/50)", 0, 200 );
    //b->SetParameters (20, 50, -25, 100);
    //b->SetParameter (0, 20);

    TF1* c = new TF1 ("b", " 5*log(x-5)", 10, 200);
    c->SetParameters (200, 50, -25, -10);


    //TF1* sum = new TF1 ("sum", "([0]/(1+exp(-[1]*(x-[2])))+[3])+([4]+[5]*(x-[6])*exp(-(x-[6])/[7]))", 0, 200);
    //sum->SetLineColor (3);
    //sum->SetParameters (sigmoid_max, sigmoid_scale, turn_on_time, offset, offset, crrc_scale, turn_on_time - 10, time_const);

    b->Draw();
    //c->Draw ("same");
    //c->Draw();
    //sum->Draw ();
    //crrc->Draw ("same");
    //sigmoid->Draw ("same");
}
