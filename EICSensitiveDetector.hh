#ifndef EICSensitiveDetector_h
#define EICSensitiveDetector_h

#include "G4VSensitiveDetector.hh"
#include "globals.hh"
#include <map>
#include <vector>  
#include <string>  

class EICSensitiveDetector : public G4VSensitiveDetector {
public:
    EICSensitiveDetector(const G4String& name);
    virtual ~EICSensitiveDetector();

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    virtual void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    struct TrackInfo {
        G4int trackID;
        G4String particleName;
        G4ThreeVector position;
        G4double energyDep;
        G4double kineticEnergy;

       
        G4double px;
        G4double py;
        G4double pz;
        G4double e;

    };

    std::map<G4int, TrackInfo> trackInfos;  

    G4double totalEnergyDeposit = 0.;
};

#endif // EICSensitiveDetector_h
