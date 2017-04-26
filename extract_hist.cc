#include "TH1.h"
#include "TFile.h"
#include "TDirectory.h"

{
    //std::vector<std::string> cHists{
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_0",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_1",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_2",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_3",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_4",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_5",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_6",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_7",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_8",
    //"ExpertHisto_CalibrationPeak_DetKey0x1c0a6290_Apv32_9"
    //};

    std::vector<std::string> cHists{
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_0",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_1",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_2",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_3",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_4",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_5",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_6",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_7",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_8",
        "ExpertHisto_CalibrationDeco_DetKey0x1c0a6290_Apv32_9"
    };

    TFile* cFile = new TFile ("Summary.root", "UPDATE");
    TDirectory* cDir = cFile->mkdir ("Deco_after");

    TFile* dFile = TFile::Open ("Data/SiStripCommissioningSource_285786_Deco_CALCHAN0_after.root");

    TH1F* cHist;

    int counter = 0;

    for (auto cHistName : cHists)
    {
        dFile->cd ("DQMData/SiStrip/ControlView/FecCrate2/FecSlot11/FecRing1/CcuAddr111/CcuChan27");
        gDirectory->GetObject (cHistName.c_str(), cHist);
        cHist->SetDirectory (cDir);
        cDir->cd();
        cHist->Write (Form ("Channel_%d", counter), TObject::kOverwrite);
        counter++;
    }
}
