#include "TH1.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TF1.h"

#include "fit_utils.cc"

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
        correctDistribution (cHist);
        //cHist->GetXaxis()->SetRangeUser (-600, 3000);
        cHist->SetMinimum (-600);
    }

    return cHist;
}

double pulseshape ( double* x, double* par )
{
    double xx = x[0];
    double val = par[3];

    if ( xx > par[1] )
        val += par[0] * ( xx - par[1] ) / par[2] * exp ( - ( ( xx - par[1] ) / par[2] ) );

    if ( xx > par[1] + par[5] )
        val -= par[0] * par[4] * ( xx - par[1] - par[5] ) / par[2]  * exp ( - ( ( xx - par[1] - par[5] ) / par[2] ) );

    return val;
}

TF1* fitACF (TH1F* pHist)
{
    float noise = 4;
    float N = round (pHist->GetMaximum() / 125.);
    float error = sqrt (2 * N) * noise;

    for (int i = 1; i <= pHist->GetNbinsX(); ++i)
        pHist->SetBinError (i, error);

    TF1* cPulseFit = new TF1 ( "ACF_fit", pulseshape, 0, 200, 6 );
    cPulseFit->SetParNames ( "Amplitude", "t_0", "tau", "Amplitude offset", "Rel. negative pulse amplitude", "Delta t" );

    //"scale_par"
    cPulseFit->SetParLimits ( 0, 1500, 10000 );

    //"t_0"
    cPulseFit->SetParLimits ( 1, 0, 200 );
    cPulseFit->SetParameter ( 1, 30 );

    //"tau = time_constant"
    cPulseFit->SetParLimits ( 2, 15, 125 );
    //cPulseFit->SetParameter ( 2, 50 );
    //cPulseFit->FixParameter ( 2, 44 );

    //"Amplitude_offset"
    cPulseFit->SetParLimits ( 3, 0, 600 );
    //cPulseFit->SetParameter ( 3, 50 );
    //cPulseFit->FixParameter ( 3, 200 );

    //"Relative amplitude of negative pulse"
    cPulseFit->SetParLimits ( 4, 0, 1 );
    cPulseFit->FixParameter ( 4, 0);
    //cPulseFit->SetParameter ( 4, 0.5);

    //delta t
    //cPulseFit->FixParameter ( 5, 0 );

    pHist->Fit (cPulseFit, "RM+");
    return cPulseFit;
}

//DQM code fits
TF1* fitTurnOn ( TH1* pHist, float pAmplitude, float pBoundary )
{
    // fit the rising edge with a sigmoid
    TF1* sigmoid = new TF1 ("sigmoid", "[0]/(1+exp(-[1]*(x-[2])))+[3]", 0, pBoundary);
    sigmoid->SetParLimits (0, pAmplitude / 10., pAmplitude);
    sigmoid->SetParLimits (1, 0.05, 0.5);
    sigmoid->SetParLimits (2, pBoundary - 10, pBoundary + 10);
    sigmoid->SetParLimits (3, -pAmplitude / 10., pAmplitude / 10.);
    sigmoid->SetParameters (pAmplitude / 2., 0.1, pBoundary, 0.);
    pHist->Fit (sigmoid, "MR+");

    // return the point where the fit = 3% signal.
    //float time = 0.;
    //float base = sigmoid->GetMinimum (0, end);

    //for (; time < end && (sigmoid->Eval (time) - base) < 0.03 * (amplitude - base); time += 0.1) {}

    //delete sigmoid;
    //return time - 0.05;
    return sigmoid;
}
TF1* fitTail (TH1F* pHist, float pAmplitude, float pBoundary, std::string pMode)
{
    TF1* cFit = nullptr;

    if (pMode == "Deco")
        //cFit = new TF1 ("fit_deco", fdeconv_convoluted, -50, 50, 5);
        cFit = new TF1 ("fit_deco", fdeconv, pBoundary, 200, 5);
    else
        //cFit = new TF1 ("fit_peak", fpeak_convoluted, -50, 50, 5);
        //cFit = new TF1 ("fit_peak", fpeak_convoluted, 0, 200, 5);
        cFit = new TF1 ("fit_peak", fpeak, pBoundary, 200, 4);

    //cFit->FixParameter (0, 0);
    cFit->SetParLimits (0, -100, 200);
    cFit->SetParLimits (1, -100, 0);
    cFit->SetParLimits (2, 0, 200);
    cFit->SetParLimits (3, 5, 100);
    //cFit->FixParameter (3, 50);
    cFit->SetParLimits (4, 0, 50);
    //cFit->SetParameters (0., -10, 0.96, 50, 20);


    pHist->Fit (cFit, "RM+");
    return cFit;
}

void fitHist (TH1F* pHist, std::string pMode)
{
    float noise = 4;
    float N = round (pHist->GetMaximum() / 125.);
    float error = sqrt (2 * N) * noise;

    for (int i = 1; i <= pHist->GetNbinsX(); ++i)
        pHist->SetBinError (i, error);

    // localize the rising edge
    float cAmplitude = pHist->GetMaximum();

    int bin = 1;

    for (; bin <= pHist->GetNbinsX() && pHist->GetBinContent (bin) < 0.4 * cAmplitude; ++bin) {}

    float cRiseBoundary = pHist->GetBinLowEdge (bin);
    TF1* cRise = fitTurnOn (pHist, cAmplitude, cRiseBoundary);

    for (bin = 1; bin <= pHist->GetNbinsX() && pHist->GetBinContent (bin) < 0.9 * cAmplitude; ++bin) {}

    float cTailBoundary = pHist->GetBinLowEdge (bin);

    TF1* cTail = fitTail (pHist, cAmplitude, cTailBoundary, pMode);
}

void analyze()
{

    // get the histograms
    TH1F* cPeakBefore = getHist ("Peak_before", 0);
    fitHist (cPeakBefore, "Peak");
    //fitACF (cPeakBefore);
    TH1F* cPeakAfter = getHist ("Peak_after", 0);
    fitHist (cPeakAfter, "Peak");
    //fitACF (cPeakAfter);
    cPeakAfter->SetLineColor (2);
    TH1F* cDecoAfter = getHist ("Deco_after", 0);
    cDecoAfter->SetLineColor (3);
    fitHist (cDecoAfter, "Deco");
    //fitACF (cDecoAfter);

    TCanvas* cCanvas = new TCanvas ("comparison", "comparison");
    cCanvas->cd();
    cPeakBefore->Draw ("PE X0");
    cPeakAfter->Draw ("PE X0 same");
    cDecoAfter->Draw ("PE X0 same");

    std::cout << "Bins: " << cPeakBefore->GetNbinsX() << std::endl;

    //check the uniformity of the 10 sample channels
    //TCanvas* bCanvas = new TCanvas ("uniformity", "uniformity");
    //bCanvas->cd();

    //for (int i = 0; i < 10; i++)
    //{
    //TH1F* cHist = getHist ("Peak_before", i);
    //cHist->SetLineColor (i + 1);

    //if (i == 0) cHist->DrawCopy();
    //else cHist->DrawCopy ("same");
    //}
}
