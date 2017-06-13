#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TClass.h"
#include "TH1.h"
#include "TH2.h"
#include "TObject.h"

#include <iostream>
#include <string>
#include <set>
#include <vector>

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
            std::string cName = cObj->GetName();
            cDirPath += "/" + cName;
            cDirPath = cDirPath.substr (cDirPath.find (":") );

            if (cDirPath.find ("CcuChan") != std::string::npos)
            {
                if (pDirList.find ("cDirPath") == pDirList.end() )
                    pDirList.insert (cDirPath );

                //std::cout << cDirPath << std::endl;
            }

            // ok, I am in the deepest level, no more subfolders
            //TH1F* cHist = (TH1F*) cObj;
            // I can stop since I am in the deepest level
            //break;
        }
    }
}

void loop_histograms (std::string pFilename1, std::string pFilename2, std::string pResultfileName, bool pAnalytical = true)
{
    // open the file an perform a sanity check!
    std::string cPath = pFilename1;
    TFile* cFile1 = TFile::Open (pFilename1.c_str() );
    TFile* cFile2 = TFile::Open (pFilename2.c_str() );

    if (cFile1 == nullptr)
    {
        std::cout << "Something is wrong, could not open File 1 " << pFilename1 << std::endl;
        exit (1);
    }

    if (cFile2 == nullptr)
    {
        std::cout << "Something is wrong, could not open File 2 " << pFilename2 << std::endl;
        exit (1);
    }

    // go in the highest level that is always the same
    cPath += ":/DQMData/SiStrip/ControlView";
    cFile1->cd (cPath.c_str() );
    // get the TDirectory object
    TDirectory* cDir = gDirectory;
    int cCounter = 0;
    int cGoodCounter = 0;
    int cBadCounter = 0;
    // recurse through the directory tree and get a std::set of lowest level Dirs
    std::cout << "Analyzing filestructure ..." << std::endl;
    std::set<std::string> cDirTree;
    analyze_fileStructure (cDir, cDirTree);
    //exit (0);

    std::cout << "Retrieving Histograms!" << std::endl;

    // here crate a new root file with the results and the non fitted histos
    TFile* cResultFile = new TFile (pResultfileName.c_str(), "RECREATE");
    TDirectory* cNotFittedDir1 = cResultFile->mkdir ("NotFitted1");
    cResultFile->cd();
    TDirectory* cNotFittedDir2 = cResultFile->mkdir ("NotFitted2");
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
    TH2F* cChi2;
    TH2F* cStatus;

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

        cChi2 = (TH2F*) gROOT->FindObject ("h_chi2");
        cStatus = (TH2F*) gROOT->FindObject ("h_status");
    }
    // or create them
    else
    {
        // in this case I have to result the histograms
        cResultDir = cResultFile->mkdir ("Results");

        cTurnOnTime = new TH1F ("h_turn_on_time", "h_turn_on_time; t[ns]; counts", 200, -10, 10);
        cTurnOnTime->SetDirectory (cResultDir);
        cPeakTime = new TH1F ("h_peak_time", "h_peak_time; t[ns]; counts", 400, -20, 20);
        cPeakTime->SetDirectory (cResultDir);
        cRiseTime = new TH1F ("h_rise_time", "h_rise_time; t[ns]; counts", 200, -10, 10);
        cRiseTime->SetDirectory (cResultDir);
        cTimeConstant = new TH1F ("h_time_constant", "h_time_constant; t[ns]; counts", 600, -30, 30);
        cTimeConstant->SetDirectory (cResultDir);
        cUndershootTime = new TH1F ("h_undershoot_time", "h_undershoot_time; t[ns]; counts", 600, 30, 30);
        cUndershootTime->SetDirectory (cResultDir);
        cReturnTime = new TH1F ("h_return_time", "h_return_time; t[ns]; counts", 400, 200, 200);
        cReturnTime->SetDirectory (cResultDir);

        // histograms for amplitude
        cBaseline = new TH1F ("h_baseline", "h_baseline; ADC; counts", 1000, -500, 500);
        cBaseline->SetDirectory (cResultDir);
        cMaximumAmp = new TH1F ("h_maximum_amplitude", "h_maximum_amplitude; ADC; counts", 1000, -500, 500);
        cMaximumAmp->SetDirectory (cResultDir);
        cAmplitude = new TH1F ("h_amplitude", "h_amplitude; ADC; counts", 1000, -500, 500);
        cAmplitude->SetDirectory (cResultDir);
        cTailAmplitude = new TH1F ("h_tail_amplitude", "h_tail_amplitude; ADC; counts", 1600, -800, 800);
        cTailAmplitude->SetDirectory (cResultDir);
        cUndershootAmplitude = new TH1F ("h_undershoot_amplitude", "h_undershoot_amplitude; ADC; counts", 400, -200, 200);
        cUndershootAmplitude->SetDirectory (cResultDir);

        // chi2 and status
        cChi2 = new TH2F ("h_chi2", "h_chi2, chi^2 before; chi^2 after; counts", 200, 0, 20, 200, 0, 20);
        cChi2->SetDirectory (cResultDir);
        cStatus = new TH2F ("h_status", "h_status", 4, 0, 4, 4, 0, 4);
        cStatus->SetDirectory (cResultDir);
    }

    // iterate the set and extract all the source histos in each subdir
    for (auto cPath : cDirTree)
    {
        if (cCounter == 80) break;

        TH1F* cHist1 = nullptr;
        TH1F* cHist2 = nullptr;

        std::string cFullPath1 = pFilename1 + cPath;
        std::string cFullPath2 = pFilename2 + cPath;

        cFile1->GetObject (cFullPath1.c_str(), cHist1);
        cFile2->GetObject (cFullPath2.c_str(), cHist2);

        cCounter++;

        //do what we came for !
        pulse_parameters cPulse1;
        pulse_parameters cPulse2;

        if (pAnalytical)
        {
            cPulse1 = analyze_hist_analytical (cHist1, false);
            cPulse2 = analyze_hist_analytical (cHist2, false);
        }
        else
        {
            cPulse1 = analyze_hist (cHist1);
            cPulse2 = analyze_hist (cHist2);
        }

        //save the histogram in case it exceeds the Chi2
        //and do not fill the summary
        bool pulse1good = true;
        bool pulse2good = true;

        if (cPulse1.fit_status != 0 ||  cPulse1.chi2_peak > 10)
        {
            cHist1->SetDirectory (cNotFittedDir1);
            cBadCounter++;
            pulse1good = false;
        }

        if (cPulse2.fit_status != 0 ||  cPulse2.chi2_peak > 10)
        {
            cHist2->SetDirectory (cNotFittedDir2);
            cBadCounter++;
            pulse2good = false;
        }

        if (pulse1good && pulse2good)
        {
            pulse_parameters cPulse = cPulse2 - cPulse1;
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

            cChi2->Fill (cPulse1.chi2_peak, cPulse2.chi2_peak);
            cStatus->Fill (cPulse1.fit_status, cPulse2.fit_status);
            cGoodCounter += 2;
        }

        if (cCounter % 1000 == 0) std::cout << "Processed " << cCounter << " Histograms of total " << cDirTree.size() * 2 << std::endl;
    }


    std::cout << cCounter << " Histograms detected!" << std::endl;
    std::cout << cGoodCounter << " of which had Chi^2 <10 and " << cBadCounter << " were not successfully fitted!" << std::endl;

    cResultFile->Write();

    // create a canvas, divide it and show the histograms
    TCanvas* cResultCanvas = new TCanvas ("Results", "Results", 1000, 1000);
    //cResultCanvas->SetDirectory (cResultDir);

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
    cChi2->Draw ("colz");
    cResultCanvas->cd (13);
    cStatus->Draw ("colz");

    std::string cPdfName = pResultfileName.substr (0, pResultfileName.find (".root") );
    cPdfName += ".pdf";
    cResultCanvas->SaveAs (cPdfName.c_str() );
    std::cout << cPdfName << std::endl;
    cResultFile->cd();
    cResultCanvas->Write ("ResultSummary", TObject::kOverwrite);
    //cResultFile->Write();

}

void tester()
{
    loop_histograms ("Data/SiStripCommissioningSource_267212_Peak_CALCHAN0_before.root", "Data/SiStripCommissioningSource_285651_Peak_CALCHAN0_after.root", "testme.root");
    //TH1F* cPeakBefore = getHist ("Peak_after", 6);
    //analyze_hist_analytical (cPeakBefore, false);
    //TCanvas* testcanvas = new TCanvas ("test", "test");
    //testcanvas->cd();
    //cPeakBefore->Draw ("PE X0");
}
