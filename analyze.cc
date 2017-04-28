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

struct pulse_parameters
{
    pulse_parameters (std::string pMode) : mode (pMode),
        turn_on_time (0),
        peak_time (0),
        rise_time (0),
        time_constant (0),
        undershoot_time (0),
        baseline (0),
        max_pulseheight (0),
        amplitude (0),
        tail_amplitude (0),
        undershoot (0),
        chi2 (0)
    {    }

    std::string mode;
    float turn_on_time;
    float peak_time;
    float rise_time; //peak time-turn_on_time
    float time_constant;
    float undershoot_time;

    float baseline;
    float max_pulseheight;
    float amplitude; //max_pulseheight-turn_on_time
    float tail_amplitude; //125ns after maximum
    float undershoot;

    float chi2;

    void compute()
    {
        rise_time = peak_time - turn_on_time;
        amplitude = max_pulseheight - baseline;

        if (mode == "Peak")
        {
            undershoot_time = 0;
            undershoot = 0;
        }

        std::cout << "***************************************" << std::endl;
        std::cout << "PULSESHAPE PARAMETERS: " << mode << " Mode" << std::endl;
        std::cout << "***************************************" << std::endl;
        std::cout << "Turn-on-Time : " << turn_on_time << " ns" << std::endl;
        std::cout << "Peak-Time    : " << peak_time << " ns" << std::endl;
        std::cout << "Rise-Time    : " << rise_time << " ns" << std::endl;
        std::cout << "t_Undershoot : " << undershoot_time << " ns" << std::endl;
        std::cout << "Time-Constant: " << time_constant << " ns" << std::endl;
        std::cout << std::endl;
        std::cout << "Baseline     : " << baseline << " ADC" << std::endl;
        std::cout << "Pulseheight  : " << max_pulseheight << " ADC" << std::endl;
        std::cout << "Amplitude    : " << amplitude << " ADC" << std::endl;
        std::cout << "Undershoot   : " << undershoot << " ADC" << std::endl;
        std::cout << "TailAmpli    : " << tail_amplitude << " ADC" << std::endl;
        std::cout << std::endl;
        std::cout << "Chi2/NDF     : " << chi2 << std::endl;
        std::cout << "***************************************" << std::endl;
    }
};

//DQM code fits
TF1* fitTurnOn ( TH1* pHist, float pAmplitude, float pBoundary, pulse_parameters& pPulseParam )
{
    // fit the rising edge with a sigmoid
    TF1* sigmoid = new TF1 ("sigmoid", "[0]/(1+exp(-[1]*(x-[2])))+[3]", 0, pBoundary);
    sigmoid->SetParLimits (0, pAmplitude / 10., pAmplitude * 10); //amplitude of sigmoid
    sigmoid->SetParLimits (1, 0.05, 0.5);//steepness/width
    sigmoid->SetParLimits (2, pBoundary - 20, pBoundary + 20);//turn on
    sigmoid->SetParLimits (3, -pAmplitude / 10., pAmplitude / 10.);//baseline
    sigmoid->SetParameters (pAmplitude / 2., 0.1, pBoundary, 0.);
    pHist->Fit (sigmoid, "MRQ+");

    // return the point where the fit = 3% signal.
    float time = 0.;
    float base = sigmoid->GetMinimum (0, pBoundary);

    for (; time < pBoundary && (sigmoid->Eval (time) - base) < 0.03 * (pAmplitude - base); time += 0.1) {}

    pPulseParam.turn_on_time = time - 0.05;
    pPulseParam.baseline = sigmoid->GetParameter (3);

    return sigmoid;
}
TF1* fitTail (TH1F* pHist, float pAmplitude, float pBoundary, std::string pMode, pulse_parameters& pPulseParam)
{
    TF1* cFit = nullptr;

    if (pMode == "Deco")
        cFit = new TF1 ("fit_deco", fdeconv, pBoundary, 200, 5);
    else
        cFit = new TF1 ("fit_peak", fpeak, pBoundary, 200, 4);


    cFit->SetParLimits (0, -100, 500);//baseline
    cFit->SetParLimits (1, -200, 0);//turn on time
    cFit->SetParLimits (2, 0, 500);//scale
    cFit->SetParLimits (3, 5, 100);//timeconstant
    //cFit->SetParameters (150, -35, 180, 45);

    pHist->Fit (cFit, "MRQ+");

    pPulseParam.peak_time = cFit->GetMaximumX();
    pPulseParam.max_pulseheight = cFit->GetMaximum();
    pPulseParam.undershoot = cFit->GetMinimum();
    pPulseParam.undershoot_time = cFit->GetMinimumX();

    pPulseParam.time_constant = cFit->GetParameter (3);

    if (pPulseParam.peak_time + 125 < 200)
        pPulseParam.tail_amplitude = cFit->Eval (pPulseParam.peak_time + 125);
    else
        pPulseParam.tail_amplitude = cFit->Eval (200);

    pPulseParam.chi2 = cFit->GetChisquare() / cFit->GetNDF();

    return cFit;
}

void fitHist (TH1F* pHist, std::string pMode)
{
    //assign Bin errors
    float noise = 4;
    float N = round (pHist->GetMaximum() / 125.);
    float error = sqrt (2 * N) * noise;

    for (int i = 1; i <= pHist->GetNbinsX(); ++i)
        pHist->SetBinError (i, error);

    // struct for pulse parameters
    pulse_parameters cPulseParam (pMode);

    // localize the maximum amplitude
    float cAmplitude = pHist->GetMaximum();

    int bin = pHist->FindFirstBinAbove (0.4 * cAmplitude);
    float cRiseBoundary = pHist->GetBinLowEdge (bin);
    TF1* cRise = fitTurnOn (pHist, cAmplitude, cRiseBoundary, cPulseParam);

    bin = pHist->FindFirstBinAbove (0.9 * cAmplitude);
    float cTailBoundary = pHist->GetBinLowEdge (bin);
    TF1* cTail = fitTail (pHist, cAmplitude, cTailBoundary, pMode, cPulseParam);

    cPulseParam.compute();
}

void analyze()
{
    // get the histograms
    TH1F* cPeakBefore = getHist ("Peak_before", 0);
    fitHist (cPeakBefore, "Peak");
    TH1F* cPeakAfter = getHist ("Peak_after", 0);
    fitHist (cPeakAfter, "Peak");
    cPeakAfter->SetLineColor (2);
    TH1F* cDecoAfter = getHist ("Deco_after", 0);
    cDecoAfter->SetLineColor (3);
    fitHist (cDecoAfter, "Deco");

    TCanvas* cCanvas = new TCanvas ("comparison", "comparison");
    cCanvas->cd();
    cPeakBefore->Draw ("PE X0");
    cPeakAfter->Draw ("PE X0 same");
    cDecoAfter->Draw ("PE X0 same");

    //check the uniformity of the 10 sample channels
    //TCanvas* bCanvas = new TCanvas ("uniformity", "uniformity");
    //bCanvas->cd();

    //for (int i = 0; i < 10; i++)
    //{
    //TH1F* cHist = getHist ("Peak_before", i);
    //cHist->SetLineColor (i + 1);
    //fitHist (cHist, "Peak");

    //if (i == 0) cHist->DrawClone();
    //else cHist->DrawClone ("same");
    //}

}
