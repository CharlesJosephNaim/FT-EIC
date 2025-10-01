#ifndef ACTIONINITIALIZATION_HH
#define ACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"
#include "EICDetectorConstruction.hh"

class ActionInitialization : public G4VUserActionInitialization {
public:
    explicit ActionInitialization(EICDetectorConstruction* det);
    virtual ~ActionInitialization();

    virtual void Build() const override;

private:
    EICDetectorConstruction* fDetector;
};

#endif
