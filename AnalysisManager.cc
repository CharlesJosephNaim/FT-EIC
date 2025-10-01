#include "G4ios.hh"
#include <cstdio>

std::mutex AnalysisManager::mutex;
AnalysisManager* AnalysisManager::instance = nullptr;

AnalysisManager* AnalysisManager::GetInstance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!instance) {
        instance = new AnalysisManager();
    }
    return instance;
}

AnalysisManager::AnalysisManager()
    : outputFile(nullptr), EnergyTrackHist(nullptr), EnergyTrack(0.)
{
    const char* filename = "output.root";

    if (std::remove(filename) == 0) {
        G4cout << "Old output file removed: " << filename << G4endl;
    }

    outputFile = new TFile(filename, "RECREATE");
    if (!outputFile || outputFile->IsZombie()) {
        G4cerr << "Error: Could not create output file!" << G4endl;
        delete outputFile;
        outputFile = nullptr;
        return;
    }

    EnergyTrackHist = new TH1F("EnergyTrackHist", "Total Energy per Event", 1500, 0., 1500.);
}

AnalysisManager::~AnalysisManager() {
    if (outputFile) {
        delete outputFile;
        outputFile = nullptr;
    }
    delete EnergyTrackHist;
}

void AnalysisManager::SetEnergy(G4double energy) {
    EnergyTrack = energy;
}

void AnalysisManager::EndOfEvent() {
    if (EnergyTrackHist) {
        EnergyTrackHist->Fill(EnergyTrack);
        G4cout << "Filling EnergyTrackHist with " << EnergyTrack << G4endl;
    }
    EnergyTrack = 0.;
}

void AnalysisManager::Write() {
    if (outputFile && outputFile->IsOpen()) {
        outputFile->cd();
        EnergyTrackHist->Write();
        outputFile->Write();
        outputFile->Close();  
    }
}
