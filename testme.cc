#include "TH1.h"
#include "TF1Convolution.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TMath.h"

#include "fit_utils.cc"

double test (double* x, double* par)
{
    if (x[0] + par[2] < 0) return par[0];

    return (par[0] / (1 + TMath::Exp (-par[1] * (x[0] - par[2]) ) ) + par[3]) (par[4] + par[5] * (x[0] - par[6]) * TMath::Exp (- (x[0] - par[6]) / par[7]) );
}

void testme()
{
    double offset = 0.;
    double sigmoid_scale = .4;
    double sigmoid_max = 100;

    double time_const = 50;
    double crrc_scale = 20;
    double turn_on_time = 20;

    TF1* sigmoid = new TF1 ("sigmoid", "[0]/(1+exp(-[1]*(x-[2])))+[3]", 0, 200);
    sigmoid->SetParameters (sigmoid_max, sigmoid_scale, turn_on_time, offset);

    TF1* crrc = new TF1 ("crrc", "[0]+[1]*(x-[2])*exp(-(x-[2])/[3])", 0, 200);
    crrc->SetParameters (offset, crrc_scale, turn_on_time, time_const   );


    TF1* sum = new TF1 ("sum", "([0]/(1+exp(-[1]*(x-[2])))+[3])+([4]+[5]*(x-[6])*exp(-(x-[6])/[7]))", 0, 200);
    sum->SetLineColor (3);
    sum->SetParameters (sigmoid_max, sigmoid_scale, turn_on_time, offset, offset, crrc_scale, turn_on_time - 10, time_const);

    //c->Draw();
    sum->Draw ();
    crrc->Draw ("same");
    sigmoid->Draw ("same");
}
