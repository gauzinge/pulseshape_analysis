#include "TFile.h"
#include "TDirectory.h"
//#include "TIter.h"
#include "TKey.h"
#include "TClass.h"

#include <iostream>
#include <string>
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
void recurseOverKeys ( TDirectory* pDirectory)
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
		if (cObj->IsA()->InheritsFrom ("TH1") )
		{
			//ok, I am in the deepest level
			TH1F* cHist = (TH1F*) cObj;
			std::cout << cHist->GetName() << std::endl;
			//continue;
		}
		else if ( cObj->IsA()->InheritsFrom ( "TDirectory" ) )
		{
			// it's a subdirectory
			// obj is now the starting point of another round of merging
			// obj still knows its depth within the target file via
			// GetPath(), so we can still figure out where we are in the recursion
			recurseOverKeys ( (TDirectory*) cObj);
		}
	}
}


void loop_histograms (std::string pFilename)
{
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
	// recurse through the directory tree and treat the histograms
	recurseOverKeys (cDir);
}

void tester()
{
	loop_histograms ("Data/SiStripCommissioningSource_267212_Peak_CALCHAN0_before.root");
	//TH1F* cPeakBefore = getHist ("Deco_after", 1);
	//histogram and boolean to determine if gaus or crrc in deco mode
	//analyze_hist (cPeakBefore, false);
	//TCanvas* testcanvas = new TCanvas ("test", "test");
	//testcanvas->cd();
	//cPeakBefore->Draw ("PE X0");
}
