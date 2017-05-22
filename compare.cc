#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TClass.h"

#include <iostream>
#include <string>
#include <set>

void plot (std::string pFilename, TCanvas* pCanvas, int pIteration = 0)
{
    //open the summary file
    TFile* cFile = TFile::Open (pFilename.c_str() );

    TDirectory* cDir;
    gDirectory->GetObject ("Results", cDir);

    if (cDir == nullptr)
        std::cout << "Error, Directory does not exist!" << std::endl;

    cDir->cd();

    int cPadCounter = 0;
    std::string cArgument = (pIteration == 0) ? "" : "same";

    //if (pCanvas == nullptr)
    //{
    //std::cout << "Canvas is nullptr!" << std::endl;
    //pCanvas = new TCanvas ("comparison plots", "comparison plots", 1000, 1000 );
    //pCanvas->Divide (3, 5);
    //cArgument = "";
    //pCanvas->cd();
    //}


    for (auto cKey : *cDir->GetListOfKeys() )
    {
        TH1F* cHist;
        cDir->GetObject (cKey->GetName(), cHist);
        cHist->SetLineColor (pIteration + 1);
        pCanvas->cd (++cPadCounter);
        cHist->Draw (cArgument.c_str() );
    }

    //cFile->Close();
}

void compare()
{
    TCanvas* cCanvas = nullptr;
    cCanvas = new TCanvas ("comparison plots", "comparison plots", 1000, 1000 );
    cCanvas->Divide (3, 5);

    TLegend* cLegend = new TLegend (0.2, 0.2, .8, .8);
    TLegendEntry* e1 = cLegend->AddEntry ( (TObject*) 0, "before", "l");
    e1->SetTextColor (1);
    TLegendEntry* e2 = cLegend->AddEntry ( (TObject*) 0, "after", "l");
    e2->SetTextColor (2);

    std::vector<std::string> cFiles = {"Data/Results_peak_before.root", "Data/Results_peak_after.root"};

    int cIterationCounter = 0;

    for (auto cFile : cFiles)
        plot (cFile, cCanvas, cIterationCounter++);

    cCanvas->cd (15);
    cLegend->Draw();
}
