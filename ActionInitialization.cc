#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"

ActionInitialization::ActionInitialization(EICDetectorConstruction* detector)
 : G4VUserActionInitialization(),
   fDetector(detector)
{}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::Build() const
{
   
    auto primaryGen = new PrimaryGeneratorAction(fDetector);
    SetUserAction(primaryGen);

}
