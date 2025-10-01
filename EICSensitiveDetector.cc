#include "EICSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"

EICSensitiveDetector::EICSensitiveDetector(const G4String& name)
  : G4VSensitiveDetector(name), totalEnergyDeposit(0.)
{}

EICSensitiveDetector::~EICSensitiveDetector() {}

G4bool EICSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    if (!step) return false;

    auto edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) return false;

    totalEnergyDeposit += edep;

    auto preStepPoint = step->GetPreStepPoint();
    auto track = step->GetTrack();
    auto pos = preStepPoint->GetPosition();
    auto kineticEnergy = track->GetTotalEnergy();
    auto trackID = track->GetTrackID();
    auto particleName = track->GetDefinition()->GetParticleName();

    auto momentum = track->GetMomentum();
    
    auto px = momentum.x();
    auto py = momentum.y();
    auto pz = momentum.z();
    
    auto e = track->GetTotalEnergy();

    auto it = trackInfos.find(trackID);
    if (it == trackInfos.end()) {
        trackInfos[trackID] = {trackID, particleName, pos, edep, kineticEnergy, px, py, pz, e};
    } else {
        it->second.energyDep += edep;
    }

    return true;
}
void EICSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{TString setup = "pp";
    if(setup=="pp"){
        static TFile* file = nullptr;
        static TTree* tree = nullptr;
        
        static Int_t    trackID;
        static Float_t  posX, posY, posZ;
        static Float_t  energyDep;
        static Float_t  kineticEnergy;
        static char     particleName[50];
        
        static Float_t  px, py, pz, e;
        
        static Float_t xBj, x1, x2, xF, pT, mass, y;
        
        // Ajout des angles dans le lab
        static Float_t theta, phi;
        
        if (!file) {
            file = new TFile("tracks_output.root", "RECREATE");
            tree = new TTree("TrackTree", "Track information per event");
            
            tree->Branch("TrackID", &trackID, "TrackID/I");
            tree->Branch("ParticleName", particleName, "ParticleName/C");
            tree->Branch("PosX", &posX, "PosX/F");
            tree->Branch("PosY", &posY, "PosY/F");
            tree->Branch("PosZ", &posZ, "PosZ/F");
            tree->Branch("EnergyDeposit_GeV", &energyDep, "EnergyDeposit_GeV/F");
            tree->Branch("KineticEnergy_GeV", &kineticEnergy, "KineticEnergy_GeV/F");
            
            tree->Branch("Px_GeV", &px, "Px_GeV/F");
            tree->Branch("Py_GeV", &py, "Py_GeV/F");
            tree->Branch("Pz_GeV", &pz, "Pz_GeV/F");
            
            tree->Branch("x1", &x1, "x1/F");
            tree->Branch("x2", &x2, "x2/F");
            tree->Branch("xF", &xF, "xF/F");
            
            tree->Branch("y", &y, "y/F");
            
            tree->Branch("pT", &pT, "pT/F");
            tree->Branch("e", &e, "e/F");
            tree->Branch("Mass", &mass, "Mass/F");
            
            tree->Branch("Theta_rad", &theta, "Theta_rad/F");
            tree->Branch("Phi_rad", &phi, "Phi_rad/F");
        }
        
        std::vector<const TrackInfo*> muPlusTracks;
        std::vector<const TrackInfo*> muMinusTracks;
        
        for (const auto& [id, info] : trackInfos) {
            if (info.particleName == "mu+") {
                muPlusTracks.push_back(&info);
            } else if (info.particleName == "mu-") {
                muMinusTracks.push_back(&info);
            }
        }
        
        double E_beam = 100.00 ;
        
        for (auto mup : muPlusTracks) {
            for (auto mum : muMinusTracks) {
                G4LorentzVector p1(
                                   mup->px,
                                   mup->py,
                                   mup->pz,
                                   mup->e
                                   );
                G4LorentzVector p2(
                                   mum->px,
                                   mum->py,
                                   mum->pz,
                                   mum->e
                                   );
                
                G4LorentzVector pair = p1 + p2;
                
                double sqrt_s = sqrt(2.0 * E_beam * 0.938);
                double s = sqrt_s * sqrt_s;
                
                TLorentzVector p_pair(pair.px() / GeV, pair.py() / GeV, pair.pz() / GeV, pair.e() / GeV);
                
                theta = p_pair.Theta();
                phi   = p_pair.Phi();
                
                double E_p = E_beam;
                double E_t = 0.938;
                double beta_cm = (E_p - E_t) / (E_p + E_t);
                TVector3 boost_vector(0, 0, -beta_cm);
                
                TLorentzVector p_pair_cm = p_pair;
                p_pair_cm.Boost(boost_vector);
                
                double pL_cm = p_pair_cm.Pz();
                xF = 2.0 * pL_cm / sqrt_s;
                
                pT = p_pair_cm.Pt();
                mass = p_pair_cm.M();
                
                y = p_pair_cm.Rapidity();
                
                double delta = std::sqrt(xF * xF + 4.0 * mass * mass / s);
                x1 = 0.5 * (xF + delta);
                x2 = 0.5 * (-xF + delta);
                
                trackID = mup->trackID;
                strncpy(particleName, mup->particleName.c_str(), sizeof(particleName));
                particleName[sizeof(particleName)-1] = '\0';
                
                posX = mup->position.x() / mm;
                posY = mup->position.y() / mm;
                posZ = mup->position.z() / mm;
                energyDep = mup->energyDep / GeV;
                kineticEnergy = mup->kineticEnergy / GeV;
                
                px = mup->px / GeV;
                py = mup->py / GeV;
                pz = mup->pz / GeV;
                e = mup->e / GeV;
                
                tree->Fill();
                
                trackID = mum->trackID;
                strncpy(particleName, mum->particleName.c_str(), sizeof(particleName));
                particleName[sizeof(particleName)-1] = '\0';
                
                posX = mum->position.x() / mm;
                posY = mum->position.y() / mm;
                posZ = mum->position.z() / mm;
                energyDep = mum->energyDep / GeV;
                kineticEnergy = mum->kineticEnergy / GeV;
                
                px = mum->px / GeV;
                py = mum->py / GeV;
                pz = mum->pz / GeV;
                e = mum->e / GeV;
                
                tree->Fill();
            }
        }
        
        trackInfos.clear();
        totalEnergyDeposit = 0.;
        
        file->Write("", TObject::kOverwrite);
    }
}
