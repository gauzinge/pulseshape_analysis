#include <string>

#include <TH1.h>
#include <TF1.h>
#include <TMath.h>

std::string gFitString = "MR+";

// sigmoid to fit the turn-on of the pulse
// par[0]: amplitude
// par[1]: width parameter
// par[2]: turn-on time t_0
// par[3]: baseline / y-offset
double fturnon (double* x, double* par)
{
    return par[0] / (1 + TMath::Exp (-par[1] * (x[0] - par[2]) ) ) + par[3];
}

// CR-RC shape to fit the peak(middle part of the curve)
// par[0]: baseline / y-offset
// par[1]: turn-on time t_0
// par[2]: scape parameter
// par[3]: time constant \tau
double fpeak (double* x, double* par)
{
    if (x[0] - par[1] < 0) return par[0];

    return par[0] +  par[2] *  (x[0] - par[1])  * TMath::Exp (- (x[0] - par[1]) / par[3]);
}

// weighted sum of 3 CR-RCs to calculate the pulse shape in deconvolution mode
// par[4]: normalization factor for pulseheight
// weights from literature: 1.2131, -1.4715. 0.4463
double fdeconv (double* x, double* par)
{
    double xm = par[4] * (x[0] - 25);
    double xp = par[4] * (x[0] + 25);
    double xz = par[4] * x[0];
    return 1.2131 * fpeak (&xp, par) - 1.4715 * fpeak (&xz, par) + 0.4463 * fpeak (&xm, par);
}

// simple negative CR-RC to parametrize the undershoot of the doconvolution mode
// pulse shape
// parameters as for fpeak() without the 2 cases
double fdeconv_undershoot (double* x, double* par)
{
    return  par[0] - par[2] *  (x[0] - par[1])  * TMath::Exp (- (x[0] - par[1]) / par[3]) ;
}

//simple struct to store pulse parameters
struct pulse_parameters
{
    pulse_parameters (bool pPeakMode) : peak_mode (pPeakMode),
        turn_on_time (0),
        peak_time (0),
        rise_time (0),
        time_constant (0),
        undershoot_time (0),
        return_to_baseline (0),
        baseline (0),
        max_pulseheight (0),
        amplitude (0),
        tail_amplitude (0),
        undershoot (0),
        chi2_turnon (0),
        chi2_peak (0),
        chi2_undershoot (0)
    {
    }

    bool  peak_mode;
    float turn_on_time;
    float peak_time;
    float rise_time; //peak time-turn_on_time
    float time_constant;
    float undershoot_time;
    float return_to_baseline;

    float baseline;
    float max_pulseheight;
    float amplitude; //max_pulseheight-turn_on_time
    float tail_amplitude; //125ns after maximum
    float undershoot;

    float chi2_turnon;
    float chi2_peak;
    float chi2_undershoot;

    void compute()
    {
        rise_time = peak_time - turn_on_time;
        amplitude = max_pulseheight - baseline;

        std::string cModeString;

        if (peak_mode)
        {
            undershoot_time = 0;
            undershoot = 0;
            cModeString = "PEAK";
        }
        else cModeString = "DECONVOLUTION";

        std::cout << "***************************************" << std::endl;
        std::cout << "PULSESHAPE PARAMETERS: " << cModeString << " Mode" << std::endl;
        std::cout << "***************************************" << std::endl;
        std::cout << "Turn-on-Time : " << turn_on_time << " ns" << std::endl;
        std::cout << "Peak-Time    : " << peak_time << " ns" << std::endl;
        std::cout << "Rise-Time    : " << rise_time << " ns" << std::endl;
        std::cout << "t_Undershoot : " << undershoot_time << " ns" << std::endl;
        std::cout << "Back to base : " << return_to_baseline << " ns" << std::endl;
        std::cout << "Time-Constant: " << time_constant << " ns" << std::endl;
        std::cout << std::endl;
        std::cout << "Baseline     : " << baseline << " ADC" << std::endl;
        std::cout << "Pulseheight  : " << max_pulseheight << " ADC" << std::endl;
        std::cout << "Amplitude    : " << amplitude << " ADC" << std::endl;
        std::cout << "Undershoot   : " << undershoot << " ADC" << std::endl;
        std::cout << "TailAmpli    : " << tail_amplitude << " ADC" << std::endl;
        std::cout << std::endl;
        std::cout << "Chi2/NDF TO  : " << chi2_turnon << std::endl;
        std::cout << "Chi2/NDF PE  : " << chi2_peak << std::endl;
        std::cout << "Chi2/NDF US  : " << chi2_undershoot << std::endl;
        std::cout << "***************************************" << std::endl;
    }
};

pulse_parameters analyze_hist (TH1* pHist)
{
    //boolen to tell me if peak mode
    bool cPeakMode;
    //first, format the histogram and set the scale correctly
    pHist->Scale (-1.);

    // to fit the whole pulse, also if it undershoots
    //or has negative baseline
    pHist->SetMinimum (-600); // to fit the whole pulse, also if it undershoots

    //assign Bin errors
    float noise = 4;
    float N = round (pHist->GetMaximum() / 125.);
    float error = sqrt (2 * N) * noise;

    for (int i = 1; i <= pHist->GetNbinsX(); ++i)
        pHist->SetBinError (i, error);

    std::string cHistName = pHist->GetName();

    //histogram was acquired in peak mode
    if (cHistName.find ("Peak") != std::string::npos)
        cPeakMode = true;
    else if (cHistName.find ("Deco") != std::string::npos)
        cPeakMode = false;
    else
        exit (1);

    // now lets find the maximum of the histogram
    float cMaxAmplitude = pHist->GetMaximum();
    // end at 0.3 * max amplitude
    float cTurnOnLimit = pHist->GetBinCenter (pHist->FindFirstBinAbove (0.3 * cMaxAmplitude) );
    // start at 0.8 * the max amplitude
    float cPeakLimit = pHist->GetBinCenter (pHist->FindFirstBinAbove (0.8 * cMaxAmplitude) );

    std::cout << cHistName << " maximum time for turn-on: " << cTurnOnLimit << " minimum time for peak: " << cPeakLimit << std::endl;

    // so we have survived rescaling and mode extraction, let's fit some!

    // fit function for the turn_on
    TF1* f_turn_on = new TF1 ("turn_on", fturnon, 0, cTurnOnLimit, 4);
    // set parameter limits
    f_turn_on->SetParLimits (0, cMaxAmplitude / 10., cMaxAmplitude * 10); //amplitude
    f_turn_on->SetParLimits (1, 0.05, 0.5); //width parameter
    f_turn_on->SetParLimits (2, cTurnOnLimit - 25, cTurnOnLimit + 25); //turn_on_time
    f_turn_on->SetParLimits (3, -cMaxAmplitude / 10., cMaxAmplitude / 10.); //baseline
    // set initial parameters
    f_turn_on->SetParameters (cMaxAmplitude / 2., 0.1, cTurnOnLimit, 0.);
    f_turn_on->SetLineColor (1);

    // fit function for the peak
    TF1* f_peak = (cPeakMode) ? new TF1 ("fit_peak", fpeak, cPeakLimit, 200, 4) : new TF1 ("fit_deco", fdeconv, cPeakLimit, 200, 5);
    // set the parameter limits
    f_peak->SetParLimits (0, -200, 500);//baseline
    f_peak->SetParLimits (1, 0, 200);//turn on time
    f_peak->SetParLimits (2, 0, 500);//scale
    f_peak->SetParLimits (3, 5, 100);//timeconstant
    f_peak->SetParLimits (4, 0, 10);//scaling for sum
    // set the initial parameter guess
    f_peak->SetParameters (150, 35, 180, 45, 1.8);
    f_peak->SetLineColor (2);

    // this is a struct for holding the pulse parameters
    pulse_parameters cPulse (cPeakMode);

    // now fit turn-on and peak because I need this independ of the APV mode
    pHist->Fit (f_turn_on, gFitString.c_str() );
    pHist->Fit (f_peak, gFitString.c_str() );

    // return the point where the turn-on fit = 3% signal.
    float time = 0.;
    float base = f_turn_on->GetMinimum (0, cTurnOnLimit);

    for (; time < cTurnOnLimit && (f_turn_on->Eval (time) - base) < 0.05 * (cMaxAmplitude - base); time += 0.1) {}

    // extract the parameters from the turn on
    cPulse.turn_on_time = time - 0.05;
    cPulse.baseline = f_turn_on->GetParameter (3);
    cPulse.chi2_turnon = f_turn_on->GetChisquare() / f_turn_on->GetNDF();

    // extract the pulse parameters from the peak fit
    cPulse.peak_time = f_peak->GetMaximumX();
    cPulse.max_pulseheight = f_peak->GetMaximum();

    cPulse.time_constant = f_peak->GetParameter (3);
    cPulse.chi2_peak = f_peak->GetChisquare() / f_peak->GetNDF();

    if (cPeakMode)
    {
        if (cPulse.peak_time + 125 < 200)
            cPulse.tail_amplitude = f_peak->Eval (cPulse.peak_time + 125);
        else
            cPulse.tail_amplitude = f_peak->Eval (200);
    }
    else
    {
        // start 20 ns before the minimum of the peak fit
        float cUndershootLimit = f_peak->GetMinimumX() - 20;

        // OK, here I am done in Peak mode but in deconvolution mode I still need
        // to fit the undershoot with a negative CR-RC
        // extend the range beyond the histogram in case baseline recovery takes longer than to 200 ns in the scale
        TF1* f_undershoot = new TF1 ("fit_undershoot", fdeconv_undershoot, cUndershootLimit, 300, 4);
        // set the parameter limits
        f_undershoot->SetParLimits (0, -500, 200);//baseline
        f_undershoot->SetParLimits (1, 0, 200);//turn on time
        f_undershoot->SetParLimits (2, 0, 500);//scale
        f_undershoot->SetParLimits (3, 0, 100);//timeconstant
        // initial parameter guess
        f_undershoot->SetParameters (-10, 90, 40, 110);
        f_undershoot->SetLineColor (3);
        // and fit!
        pHist->Fit (f_undershoot, gFitString.c_str() );

        // extract the parameters only relevant in Deco mode:
        // undershoot
        // evaluate the tail_amplitude from the undershoot fit!
        cPulse.undershoot = f_undershoot->GetMinimum();
        cPulse.undershoot_time = f_undershoot->GetMinimumX();
        cPulse.return_to_baseline = f_undershoot->GetX (cPulse.baseline);
        cPulse.chi2_undershoot = f_undershoot->GetChisquare() / f_undershoot->GetNDF();

        if (cPulse.peak_time + 125 < 200)
            cPulse.tail_amplitude = f_undershoot->Eval (cPulse.peak_time + 125);
        else
            cPulse.tail_amplitude = f_undershoot->Eval (200);
    }

    // pulse shape completely characterized, now just compute some internals in the Pulse Parameters!
    cPulse.compute();
    return cPulse;
}
