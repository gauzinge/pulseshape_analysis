#include "TFile.h"
#include "TDirectory.h"

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

void tester()
{
    TH1F* cPeakBefore = getHist ("Deco_after", 1);
    //histogram and boolean to determine if gaus or crrc in deco mode
    analyze_hist (cPeakBefore, false);

    TCanvas* testcanvas = new TCanvas ("test", "test");
    testcanvas->cd();
    cPeakBefore->Draw ("PE X0");
}
