#ifndef EICDetectorConstruction_h
#define EICDetectorConstruction_h

#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include <vector>

class G4Material;

class EICDetectorConstruction : public G4VUserDetectorConstruction {
public:
  EICDetectorConstruction();
  ~EICDetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;
  void ConstructSDandField() override;

  G4ThreeVector GetTargetPosition() const;

private:
  G4Material* CreateMaterial(const G4String& name);
  G4Material* CreateCompositeMaterial(const G4String& name);

  void ConstructTarget(G4LogicalVolume* worldLV);
  void ConstructFVTXDetector(G4LogicalVolume* worldLV);

  void ConstructEMCal(G4LogicalVolume* worldLV);
  void ConstructHCal(G4LogicalVolume* worldLV);
  void ConstructSolenoid(G4LogicalVolume* worldLV);
  void ConstructRICH(G4LogicalVolume* worldLV);
  void ConstructBarrelEMCal(G4LogicalVolume* worldLV);
  void ConstructTimeOfFlight(G4LogicalVolume* worldLV);
  void ConstructInnerTracker(G4LogicalVolume* worldLV);
  void ConstructMicromegas(G4LogicalVolume* worldLV);

private:
  G4LogicalVolume* emcalLV                  = nullptr;
  G4LogicalVolume* hcalLV                   = nullptr;
  G4LogicalVolume* solenoidLV               = nullptr;
  G4LogicalVolume* richLV                   = nullptr;
  G4LogicalVolume* barrelLV                 = nullptr;
  G4LogicalVolume* tofLV                    = nullptr;

  // EMCal
  G4LogicalVolume* emcalCrystalsLV          = nullptr;
  G4LogicalVolume* emcalElectronicsLV       = nullptr;
  G4LogicalVolume* emcalOuterSurfaceLV      = nullptr;
  G4LogicalVolume* emcalInnerSurfaceLV      = nullptr;
  G4LogicalVolume* emcalOffsetAirLV         = nullptr;
  G4LogicalVolume* emcalAluminumPlateLV     = nullptr;

  // Target
  G4LogicalVolume*  targetLV                = nullptr;
  G4VPhysicalVolume* targetPV               = nullptr;
  G4ThreeVector      fTargetPosition;

  // ===== FVTX (4 discs) =====
  G4LogicalVolume* fvtxEnvelopeLV           = nullptr; // air
  G4LogicalVolume* fvtxBeamPipeLV           = nullptr; // Be
  std::vector<G4LogicalVolume*> fvtxDisksLV;           // Si

  // Trackers additionnels
  std::vector<G4LogicalVolume*> innerTrackerDisksLV;
  std::vector<G4LogicalVolume*> micromegasLV;
};

#endif
