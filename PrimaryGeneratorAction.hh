#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "Pythia8/Pythia.h"
#include "Rtypes.h"

class TFile;
class TTree;

class EICDetectorConstruction;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    explicit PrimaryGeneratorAction(EICDetectorConstruction* detector);
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event* anEvent) override;

    void SetVertexPosition(const G4ThreeVector& pos) { fVertexPosition = pos; }

private:
    Pythia8::Pythia* fPythia;
    EICDetectorConstruction* fDetector;
    G4ThreeVector fVertexPosition;
};

#endif
