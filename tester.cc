#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TClass.h"

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

void loop_histograms (std::string pFilename)
{
    // open the file an perform a sanity check!
    std::string cPath = pFilename;
    TFile* cFile = TFile::Open (pFilename.c_str() );

    if (cFile == nullptr)
    {
        std::cout << "Something is wrong, could not open File " << pFilename << std::endl;
        exit (1);
    }

    // here crate a new root file with the results and the non fitted histos
    // TFile* cResultFile = new TFile("Data/Results.root", "UPDATE");
    // cResultFile->mkdir("NotFitted");
    // cResultFile->cd();
    // cResultFile->mkdir("Results");
    // then create the histograms for the parameters and call pHist->SetDirectory() so it's associated to the result file

    // go in the highest level that is always the same
    cPath += ":/DQMData/SiStrip/ControlView";
    cFile->cd (cPath.c_str() );
    // get the TDirectory object
    TDirectory* cDir = gDirectory;
    // recurse through the directory tree and get a std::set of lowest level Dirs
    std::set<std::string> cDirTree;
    analyze_fileStructure (cDir, cDirTree);

    // iterate the set and extract all histos in each subdir
    for (auto cPath : cDirTree)
    {
        std::cout << cPath << std::endl;
        gDirectory->cd (cPath.c_str() );
        // set the current directory to the path & get a handle
        TDirectory* cCurrentDir = gDirectory;

        // get the list of keys in that directory and iterate them
        for (auto cKey : *cCurrentDir->GetListOfKeys() )
        {
            TH1F* cHist;
            cCurrentDir->GetObject (cKey->GetName(), cHist);
            //std::cout << cKey->GetName() << " " << cHist << std::endl;

            // do what we came for!
            pulse_parameters cPulse = analyze_hist_analytical (cHist, false, false);
            // Fill the histograms
            // save the histogram in case it exceeds the Chi2
            // if(cPulse.fit_status ==4)
            // cHist->SetDirectory()
        }
    }
}

void tester()
{
    //loop_histograms ("Data/SiStripCommissioningSource_267212_Peak_CALCHAN0_before.root");
    TH1F* cPeakBefore = getHist ("Peak_after", 1);
    //histogram and boolean to determine if gaus or crrc in deco mode
    //analyze_hist (cPeakBefore, false);
    // histogram, fix \tau, fix baseline
    // Peak mode: leave tau floating, deco mode: fix tau
    analyze_hist_analytical (cPeakBefore, false, false);
    TCanvas* testcanvas = new TCanvas ("test", "test");
    testcanvas->cd();
    cPeakBefore->Draw ("PE X0");
}
