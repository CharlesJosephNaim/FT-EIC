#ifndef ANALYSISMANAGER_HH
#define ANALYSISMANAGER_HH

#include <mutex>
#include "TH1F.h"
#include "TFile.h"
#include "G4Types.hh"

class AnalysisManager {
public:
    static AnalysisManager* GetInstance();
    ~AnalysisManager();

    void SetEnergy(G4double energy);
    void AddKineticEnergy(G4double kineticEnergy);  // méthode ajoutée
    void EndOfEvent();
    void Write();

private:
    AnalysisManager();
    AnalysisManager(const AnalysisManager&) = delete;
    AnalysisManager& operator=(const AnalysisManager&) = delete;

    static AnalysisManager* instance;
    static std::mutex mutex;

    TFile* outputFile;
    TH1F* EnergyTrackHist;
    TH1F* KineticEnergyHist;
    G4double EnergyTrack;
    G4double TotalKineticEnergy;  
};

#endif
