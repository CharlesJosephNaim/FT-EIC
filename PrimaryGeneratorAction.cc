#include "PrimaryGeneratorAction.hh"
#include "EICDetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "EICDetectorConstruction.hh"

#include "G4PrimaryVertex.hh"
#include "G4ParticleTable.hh"
#include "G4PrimaryParticle.hh"
#include "G4ParticleDefinition.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TFile.h"


PrimaryGeneratorAction::PrimaryGeneratorAction(EICDetectorConstruction* detector)
 : fDetector(detector),
   fVertexPosition(0., 0., 0.)
{
    fPythia = new Pythia8::Pythia();

    // Config Pythia fixed target
    fPythia->readString("Beams:idA = 2212");
    fPythia->readString("Beams:idB = 2212");
    fPythia->readString("Beams:eA = 100.");
    fPythia->readString("Beams:eB = 0.");
    fPythia->readString("Beams:frameType = 2");
    //fPythia->readString("HardQCD:all = on");
    
    fPythia->readString("Charmonium:all = on");
    fPythia->readString("443:onMode = off");
    fPythia->readString("443:onIfMatch = 13 -13");
    
    fPythia->readString("100443:onMode = off");
    fPythia->readString("100443:onIfMatch = 13 -13");

    
    // ccbar
    //fPythia->readString("HardQCD:hardccbar = on");

    /*
    // To muons
    // D0 (421), D+ (411), D_s+ (431) and opp.c

    // D0 → muons
    fPythia->readString("421:onMode = off");
    fPythia->readString("421:onIfAny = 13");

    // anti-D0
    fPythia->readString("-421:onMode = off");
    fPythia->readString("-421:onIfAny = 13");

    // D+
    fPythia->readString("411:onMode = off");
    fPythia->readString("411:onIfAny = 13");

    // D-
    fPythia->readString("-411:onMode = off");
    fPythia->readString("-411:onIfAny = 13");

    // Ds+
    fPythia->readString("431:onMode = off");
    fPythia->readString("431:onIfAny = 13");

    // Ds-
    fPythia->readString("-431:onMode = off");
    fPythia->readString("-431:onIfAny = 13");
     */
    fPythia->init();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete fPythia;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    if (fDetector) {
        fVertexPosition = fDetector->GetTargetPosition();
    }

    if (!fPythia->next()) return;

    G4PrimaryVertex* vertex = new G4PrimaryVertex(fVertexPosition, 0.);

    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();

    G4PrimaryParticle* firstPrimary = nullptr;
    G4PrimaryParticle* lastPrimary = nullptr;

    for (int i = 0; i < fPythia->event.size(); ++i) {
        const auto& p = fPythia->event[i];
        if (!p.isFinal()) continue;

        G4ParticleDefinition* particleDef = particleTable->FindParticle(p.id());
        
        if( p.id() != 13 && p.id() != -13) continue; //muons
        
        if (!particleDef) {
            G4cerr << "Unknown particle ID " << p.id() << " from Pythia" << G4endl;
            continue;
        }
        
        G4PrimaryParticle* primary = new G4PrimaryParticle(particleDef,
                                                         p.px() * GeV,
                                                         p.py() * GeV,
                                                         p.pz() * GeV);
        if (!firstPrimary) {
            firstPrimary = primary;
            lastPrimary = primary;
        } else {
            lastPrimary->SetNext(primary);
            lastPrimary = primary;
        }
    }
    

    if (firstPrimary) {
        vertex->SetPrimary(firstPrimary);
        anEvent->AddPrimaryVertex(vertex);
    } else {
        delete vertex;
    }
}
