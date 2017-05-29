#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TClass.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TROOT.h"

#include <iostream>
#include <string>
#include <set>

#include "fitter.cc"

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

    return cHist;
}

void analyze_fileStructure ( TDirectory* pDirectory, std::set<std::string>& pDirList)
{
    // Figure out where we are
    std::string cPath = pDirectory->GetPath();
    // go there
    gDirectory->cd (cPath.c_str() );

    // print where we are just for fun!
    //if (cPath.find (":/DQMData/SiStrip/ControlView") != std::string::npos)
    //std::cout << cPath.substr (cPath.find (":/") + 29 ) << std::endl;

    TDirectory* cCurrentDir = gDirectory;
    TKey* cKey;
    TIter next (cCurrentDir->GetListOfKeys() );

    while ( (cKey = (TKey*) next() ) )
    {
        TObject* cObj = cKey->ReadObj();

        // Check if this is a 1D histogram or a directory
        if ( cObj->IsA()->InheritsFrom ( "TDirectory" ) )
        {
            analyze_fileStructure ( (TDirectory*) cObj, pDirList);
            // obj is now the starting point of another round of merging
            // obj still knows its depth within the target file via
            // GetPath(), so we can still figure out where we are in the recursion
        }
        else if (cObj->IsA()->InheritsFrom ("TH1") )
        {
            std::string cDirPath = pDirectory->GetPath();

            if (cDirPath.find ("CcuChan") != std::string::npos)
            {
                if (pDirList.find ("cDirPath") == pDirList.end() )
                    pDirList.insert (cDirPath);

                //std::cout << cDirPath << std::endl;
            }

            // ok, I am in the deepest level, no more subfolders
            //TH1F* cHist = (TH1F*) cObj;
            // I can stop since I am in the deepest level
            break;
        }
    }
}

void loop_histograms (std::string pFilename, bool pAnalytical = true)
{
    TCanvas* debug = new TCanvas ("debug", "debug");
    debug->cd();
    bool cFirst = true;

    // open the file an perform a sanity check!
    std::string cPath = pFilename;
    TFile* cFile = TFile::Open (pFilename.c_str() );

    if (cFile == nullptr)
    {
        std::cout << "Something is wrong, could not open File " << pFilename << std::endl;
        exit (1);
    }

    // go in the highest level that is always the same
    cPath += ":/DQMData/SiStrip/ControlView";
    cFile->cd (cPath.c_str() );
    // get the TDirectory object
    TDirectory* cDir = gDirectory;
    int cCounter = 0;
    int cDirCounter = 0;
    int cGoodCounter = 0;
    int cBadCounter = 0;
    // recurse through the directory tree and get a std::set of lowest level Dirs
    std::set<std::string> cDirTree;
    analyze_fileStructure (cDir, cDirTree);

    // here crate a new root file with the results and the non fitted histos
    TFile* cResultFile = new TFile ("Data/Results.root", "RECREATE");
    TDirectory* cNotFittedDir = cResultFile->mkdir ("NotFitted");
    cResultFile->cd();
    TDirectory* cResultDir;
    TObject* cTmp = gROOT->FindObject ("Results");

    // 1 histogram for each parameter
    TH1F* cTurnOnTime;
    TH1F* cPeakTime;
    TH1F* cRiseTime;
    TH1F* cTimeConstant;
    TH1F* cUndershootTime;
    TH1F* cReturnTime;
    TH1F* cBaseline;
    TH1F* cMaximumAmp;
    TH1F* cAmplitude;
    TH1F* cTailAmplitude;
    TH1F* cUndershootAmplitude;
    TH1F* cChi2;
    TH1F* cStatus;

    // if the Results directory exists in the Results file
    if (cTmp)
    {
        cResultDir = (TDirectory*) cTmp;
        cResultDir->cd();

        cTurnOnTime = (TH1F*) gROOT->FindObject ("h_turn_on_time");
        cPeakTime = (TH1F*) gROOT->FindObject ("h_peak_time");
        cRiseTime = (TH1F*) gROOT->FindObject ("h_rise_time");
        cTimeConstant = (TH1F*) gROOT->FindObject ("h_time_constant");
        cUndershootTime = (TH1F*) gROOT->FindObject ("h_undershoot_time");
        cReturnTime = (TH1F*) gROOT->FindObject ("h_return_time");

        cBaseline = (TH1F*) gROOT->FindObject ("h_baseline");
        cMaximumAmp = (TH1F*) gROOT->FindObject ("h_maximum_amplitude");
        cAmplitude = (TH1F*) gROOT->FindObject ("h_amplitude");
        cTailAmplitude = (TH1F*) gROOT->FindObject ("h_tail_amplitude");
        cUndershootAmplitude = (TH1F*) gROOT->FindObject ("h_undershoot_amplitude");

        cChi2 = (TH1F*) gROOT->FindObject ("h_chi2");
        cStatus = (TH1F*) gROOT->FindObject ("h_status");
    }
    // or create them
    else
    {
        // in this case I have to result the histograms
        cResultDir = cResultFile->mkdir ("Results");

        cTurnOnTime = new TH1F ("h_turn_on_time", "h_turn_on_time", 300, 20, 50);
        cTurnOnTime->SetDirectory (cResultDir);
        cPeakTime = new TH1F ("h_peak_time", "h_peak_time", 400, 50, 90);
        cPeakTime->SetDirectory (cResultDir);
        cRiseTime = new TH1F ("h_rise_time", "h_rise_time", 450, 15, 60);
        cRiseTime->SetDirectory (cResultDir);
        cTimeConstant = new TH1F ("h_time_constant", "h_time_constant", 700, 15, 70);
        cTimeConstant->SetDirectory (cResultDir);
        cUndershootTime = new TH1F ("h_undershoot_time", "h_undershoot_time", 600, 80, 140);
        cUndershootTime->SetDirectory (cResultDir);
        cReturnTime = new TH1F ("h_return_time", "h_return_time", 1000, 100, 200);
        cReturnTime->SetDirectory (cResultDir);

        // histograms for amplitude
        cBaseline = new TH1F ("h_baseline", "h_baseline", 1000, -500, 500);
        cBaseline->SetDirectory (cResultDir);
        cMaximumAmp = new TH1F ("h_maximum_amplitude", "h_maximum_amplitude", 3000, 1500, 4500);
        cMaximumAmp->SetDirectory (cResultDir);
        cAmplitude = new TH1F ("h_amplitude", "h_amplitude", 3000, 1500, 4500);
        cAmplitude->SetDirectory (cResultDir);
        cTailAmplitude = new TH1F ("h_tail_amplitude", "h_tail_amplitude", 240, -120, 120);
        cTailAmplitude->SetDirectory (cResultDir);
        cUndershootAmplitude = new TH1F ("h_undershoot_amplitude", "h_undershoot_amplitude", 1100, -1000, 100);
        cUndershootAmplitude->SetDirectory (cResultDir);

        // chi2 and status
        cChi2 = new TH1F ("h_chi2", "h_chi2", 200, 0, 20);
        cChi2->SetDirectory (cResultDir);
        cStatus = new TH1F ("h_status", "h_status", 4, 0, 4);
        cStatus->SetDirectory (cResultDir);
    }

    // iterate the set and extract all the source histos in each subdir
    for (auto cPath : cDirTree)
    {
        if (cDirCounter == 100) break;

        //std::cout << cPath << std::endl;
        gDirectory->cd (cPath.c_str() );
        // set the current directory to the path & get a handle
        TDirectory* cCurrentDir = gDirectory;

        // get the list of keys in that directory and iterate them
        for (auto cKey : *cCurrentDir->GetListOfKeys() )
        {
            TH1F* cHist;
            cCurrentDir->GetObject (cKey->GetName(), cHist);
            cCounter++;
            //std::cout << cKey->GetName() << " " << cHist << std::endl;

            //do what we came for !
            pulse_parameters cPulse;

            if (pAnalytical)
                cPulse = analyze_hist_analytical (cHist, false);
            else
                cPulse = analyze_hist (cHist);

            //save the histogram in case it exceeds the Chi2
            //and do not fill the summary
            if (cPulse.fit_status != 0 ||  cPulse.chi2_peak > 10)
            {
                cHist->SetDirectory (cNotFittedDir);
                cBadCounter++;
                continue;
            }
            else
            {
                // fill the histograms
                cTurnOnTime->Fill (cPulse.turn_on_time);
                cPeakTime->Fill (cPulse.peak_time);
                cRiseTime->Fill (cPulse.rise_time);
                cTimeConstant->Fill (cPulse.time_constant);
                cUndershootTime->Fill (cPulse.undershoot_time);
                cReturnTime->Fill (cPulse.return_to_baseline);

                cBaseline->Fill (cPulse.baseline);
                cMaximumAmp->Fill (cPulse.max_pulseheight);
                cAmplitude->Fill (cPulse.amplitude);
                cTailAmplitude->Fill (cPulse.tail_amplitude);
                cUndershootAmplitude->Fill (cPulse.undershoot);

                cChi2->Fill (cPulse.chi2_peak);
                cStatus->Fill (cPulse.fit_status);
                cGoodCounter++;
            }

        }

        cDirCounter++;
    }

    std::cout << cCounter << " Histograms in " << cDirCounter << " Directories detected!" << std::endl;
    std::cout << cGoodCounter << " of which had Chi^2 <10 and " << cDirCounter << " were not successfully fitted!" << std::endl;

    cResultFile->Write();

    // create a canvas, divide it and show the histograms
    TCanvas* cResultCanvas = new TCanvas ("Results", "Results", 1000, 1000);
    //cResultCanvas->cd();
    cResultCanvas->Divide (3, 5);
    cResultCanvas->cd (1);
    cTurnOnTime->Draw();
    cResultCanvas->cd (2);
    cPeakTime->Draw();
    cResultCanvas->cd (3);
    cRiseTime->Draw();
    cResultCanvas->cd (4);
    cTimeConstant->Draw();
    cResultCanvas->cd (5);
    cUndershootTime->Draw();
    cResultCanvas->cd (6);
    cReturnTime->Draw();
    cResultCanvas->cd (7);
    cBaseline->Draw();
    cResultCanvas->cd (8);
    cMaximumAmp->Draw();
    cResultCanvas->cd (9);
    cAmplitude->Draw();
    cResultCanvas->cd (10);
    cTailAmplitude->Draw();
    cResultCanvas->cd (11);
    cUndershootAmplitude->Draw();
    cResultCanvas->cd (12);
    cChi2->Draw();
    cResultCanvas->cd (13);
    cStatus->Draw();

    cResultCanvas->SaveAs ("Results.root");

}

void tester()
{
    loop_histograms ("Data/SiStripCommissioningSource_285786_Deco_CALCHAN0_after.root");
    //TH1F* cPeakBefore = getHist ("Peak_after", 6);
    //analyze_hist_analytical (cPeakBefore, false);
    //TCanvas* testcanvas = new TCanvas ("test", "test");
    //testcanvas->cd();
    //cPeakBefore->Draw ("PE X0");
}
