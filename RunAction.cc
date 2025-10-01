#include "RunAction.hh"
#include "G4Run.hh"
#include "AnalysisManager.hh"
#include "G4ios.hh"

RunAction::RunAction() : G4UserRunAction() {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
    G4cout << "### Run started ###" << G4endl;
}

void RunAction::EndOfRunAction(const G4Run*)
{
    G4cout << "### Run ended: writing data ###" << G4endl;
    AnalysisManager::GetInstance()->Write();
}

