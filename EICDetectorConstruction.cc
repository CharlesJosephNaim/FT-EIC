// EICDetectorConstruction.cc
#include "EICDetectorConstruction.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Material.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "EICSensitiveDetector.hh"
#include "G4SDManager.hh"

#include <vector>
#include <string>

struct MicromegasSet {
  G4double rInner;
  G4double rOuter;
  G4double lengthZ;
  G4String name;
  MicromegasSet(G4double rIn, G4double rOut, G4double len, const G4String& nm)
      : rInner(rIn), rOuter(rOut), lengthZ(len), name(nm) {}
};

G4ThreeVector EICDetectorConstruction::GetTargetPosition() const {
  return fTargetPosition;
}

EICDetectorConstruction::EICDetectorConstruction() {}
EICDetectorConstruction::~EICDetectorConstruction() {}

G4Material* EICDetectorConstruction::CreateMaterial(const G4String& name) {
  return G4NistManager::Instance()->FindOrBuildMaterial(name);
}
G4Material* EICDetectorConstruction::CreateCompositeMaterial(const G4String& name) {
  auto nist = G4NistManager::Instance();

  if (name == "PbSc") {
    auto* pb = nist->FindOrBuildMaterial("G4_Pb");
    auto* scint = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    auto* mix = new G4Material("PbSc", 11.35 * g / cm3, 2);
    mix->AddMaterial(pb, 0.85);
    mix->AddMaterial(scint, 0.15);
    return mix;
  }
  if (name == "FeSc") {
    auto* fe = nist->FindOrBuildMaterial("G4_Fe");
    auto* plastic = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    auto* mix = new G4Material("FeSc", 7.8 * g / cm3, 2);
    mix->AddMaterial(fe, 0.79);
    mix->AddMaterial(plastic, 0.21);
    return mix;
  }
  if (name == "SciGlass") {
    auto* glass = nist->FindOrBuildMaterial("G4_Pyrex_Glass");
    auto* scint = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    auto* mix = new G4Material("SciGlass", 3.0 * g / cm3, 2);
    mix->AddMaterial(glass, 0.70);
    mix->AddMaterial(scint, 0.30);
    return mix;
  }
  if (name == "ArCO2") {
    auto* ar = nist->FindOrBuildMaterial("G4_Ar");
    auto* co2 = nist->FindOrBuildMaterial("G4_CARBON_DIOXIDE");
    auto* mix = new G4Material("ArCO2", 1.8 * mg / cm3, 2, kStateGas);
    mix->AddMaterial(ar, 0.7);
    mix->AddMaterial(co2, 0.3);
    return mix;
  }
  return nist->FindOrBuildMaterial("G4_AIR");
}

G4VPhysicalVolume* EICDetectorConstruction::Construct() {
  auto* air = CreateMaterial("G4_AIR");

  auto* worldBox = new G4Box("World", 10 * m, 10 * m, 10 * m);
  auto* worldLV  = new G4LogicalVolume(worldBox, air, "WorldLV");
  auto* worldPV  = new G4PVPlacement(nullptr, {}, worldLV, "World", nullptr, false, 0);

  ConstructEMCal(worldLV);
  ConstructHCal(worldLV);
  ConstructSolenoid(worldLV);
  ConstructRICH(worldLV);
  ConstructBarrelEMCal(worldLV);
  ConstructTimeOfFlight(worldLV);

  ConstructTarget(worldLV);
  ConstructFVTXDetector(worldLV);

  ConstructInnerTracker(worldLV);
  ConstructMicromegas(worldLV);

  auto* worldVis = new G4VisAttributes(G4Colour(1., 1., 1., 0.05));
  worldVis->SetVisibility(true);
  worldLV->SetVisAttributes(worldVis);

  return worldPV;
}

void EICDetectorConstruction::ConstructTarget(G4LogicalVolume* worldLV) {
  auto* be = CreateMaterial("G4_Be");
  const G4double foilThick  = 100.0 * um;
  const G4double foilRadius = 10.0 * mm;
  auto* solid = new G4Tubs("TargetFoil", 0., foilRadius, 0.5 * foilThick, 0., 360. * deg);

  targetLV = new G4LogicalVolume(solid, be, "TargetLV");
  fTargetPosition = G4ThreeVector(0, 0, -300.0 * cm);
  targetPV = new G4PVPlacement(nullptr, fTargetPosition, targetLV, "Target", worldLV, false, 0);

  auto* vis = new G4VisAttributes(G4Colour(1., 1., 0., 0.9));
  vis->SetVisibility(true);
  vis->SetForceSolid(true);
  targetLV->SetVisAttributes(vis);
}

void EICDetectorConstruction::ConstructFVTXDetector(G4LogicalVolume* worldLV) {
  auto* si  = CreateMaterial("G4_Si");
  auto* air = CreateMaterial("G4_AIR");
  auto* be  = CreateMaterial("G4_Be");

  const G4double rInDisk  = 44.0 * mm;
  const G4double rOutDisk = 120.0 * mm;   
  const G4double tSi      = 0.320 * mm;
  const G4double zStep[4] = {201.1 * mm, 261.4 * mm, 321.7 * mm, 382.0 * mm};

  const G4double envHalfZ = zStep[3] + 20.0 * mm;
  const G4double envROut  = rOutDisk + 10.0 * mm;

  const G4double rPipeIn  = 15.0 * mm;
  const G4double tPipe    = 0.5 * mm;
  const G4double rPipeOut = rPipeIn + tPipe;

  auto* envSolid = new G4Tubs("FVTXEnvelope", 0., envROut, envHalfZ, 0., 360.*deg);
  fvtxEnvelopeLV = new G4LogicalVolume(envSolid, air, "FVTXEnvelopeLV");
  new G4PVPlacement(nullptr, fTargetPosition, fvtxEnvelopeLV, "FVTXEnvelope", worldLV, false, 0);

  auto* pipeSolid = new G4Tubs("FVTXBeamPipe", rPipeIn, rPipeOut, envHalfZ, 0., 360.*deg);
  fvtxBeamPipeLV  = new G4LogicalVolume(pipeSolid, be, "FVTXBeamPipeLV");
  new G4PVPlacement(nullptr, {}, fvtxBeamPipeLV, "FVTXBeamPipe", fvtxEnvelopeLV, false, 0);

  fvtxDisksLV.clear();
  for (int i = 0; i < 4; ++i) {
    const G4String nm = "FVTX_Disk_" + std::to_string(i + 1);
    auto* diskSolid   = new G4Tubs(nm, rInDisk, rOutDisk, 0.5 * tSi, 0., 360.*deg);
    auto* diskLV      = new G4LogicalVolume(diskSolid, si, nm + "_LV");
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, zStep[i]), diskLV, nm, fvtxEnvelopeLV, false, i);
    fvtxDisksLV.push_back(diskLV);
  }

  auto* envVis = new G4VisAttributes(G4Colour(0.95, 0.5, 0.5, 0.10)); 
  envVis->SetVisibility(true);
  fvtxEnvelopeLV->SetVisAttributes(envVis);

  auto* beVis  = new G4VisAttributes(G4Colour(0.8, 0.8, 0.2, 0.5));   
  beVis->SetVisibility(true);
  beVis->SetForceSolid(true);
  fvtxBeamPipeLV->SetVisAttributes(beVis);

  for (size_t i = 0; i < fvtxDisksLV.size(); ++i) {
    auto* vis = new G4VisAttributes(G4Colour(0.2 + 0.2 * i, 0.7, 1.0, 0.6));
    vis->SetVisibility(true);
    vis->SetForceSolid(true);
    fvtxDisksLV[i]->SetVisAttributes(vis);
  }
}

void EICDetectorConstruction::ConstructEMCal(G4LogicalVolume* worldLV) {
  auto* pbsc = CreateCompositeMaterial("PbSc");
  auto* emcalBox = new G4Box("EMCal", 2.5 * cm / 2, 2.5 * cm / 2, 30 * cm / 2);
  emcalLV = new G4LogicalVolume(emcalBox, pbsc, "EMCalLV");
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 345 * cm), emcalLV, "EMCal", worldLV, false, 0);
}
void EICDetectorConstruction::ConstructHCal(G4LogicalVolume* worldLV) {
  auto* fesc = CreateCompositeMaterial("FeSc");
  auto* hcalBox = new G4Box("HCal", 5 * cm / 2, 5 * cm / 2, 140 * cm / 2);
  hcalLV = new G4LogicalVolume(hcalBox, fesc, "HCalLV");
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 429 * cm), hcalLV, "HCal", worldLV, false, 0);
}
void EICDetectorConstruction::ConstructSolenoid(G4LogicalVolume* worldLV) {
  auto* copper = CreateMaterial("G4_Cu");
  auto* solenoid = new G4Tubs("Solenoid", 142 * cm, 177 * cm, 384 * cm / 2, 0, 360 * deg);
  solenoidLV = new G4LogicalVolume(solenoid, copper, "SolenoidLV");
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, -10 * cm), solenoidLV, "Solenoid", worldLV, false, 0);
}
void EICDetectorConstruction::ConstructRICH(G4LogicalVolume* worldLV) {
  auto* aerogel = CreateMaterial("G4_AIR");
  auto* rich = new G4Tubs("RICH", 15 * cm, 185 * cm, 60 * cm, 0, 360 * deg);
  richLV = new G4LogicalVolume(rich, aerogel, "RICHLV");
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 255 * cm), richLV, "RICH", worldLV, false, 0);
}
void EICDetectorConstruction::ConstructTimeOfFlight(G4LogicalVolume* worldLV) {
  auto* plastic = CreateMaterial("G4_POLYSTYRENE");
  auto* tofBox = new G4Box("ToF", 15 * cm / 2, 8 * cm / 2, 15 * cm / 2);
  tofLV = new G4LogicalVolume(tofBox, plastic, "ToFLV");
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 187.5 * cm), tofLV, "ToF", worldLV, false, 0);
}
void EICDetectorConstruction::ConstructBarrelEMCal(G4LogicalVolume* worldLV) {
  G4ThreeVector emcalPos(0, 0, -49.685 * cm);
  auto* sciglass = CreateCompositeMaterial("SciGlass");
  auto* crystalsSolid =
      new G4Tubs("EMCalCrystals", 80.5 * cm, 120.5 * cm, 497.91 * cm / 2, 0, 360 * deg);
  emcalCrystalsLV = new G4LogicalVolume(crystalsSolid, sciglass, "EMCalCrystalsLV");
  new G4PVPlacement(nullptr, emcalPos, emcalCrystalsLV, "EMCalCrystals", worldLV, false, 0);

  auto* silicon = CreateMaterial("G4_Si");
  auto* air     = CreateMaterial("G4_AIR");
  auto* electronicsMat =
      new G4Material("ElectronicsMat",
                     0.25 * silicon->GetDensity() + 0.75 * air->GetDensity(), 2);
  electronicsMat->AddMaterial(silicon, 0.25);
  electronicsMat->AddMaterial(air, 0.75);
  auto* electronicsSolid =
      new G4Tubs("EMCalElectronics", 120.5 * cm, 130.5 * cm, 497.91 * cm / 2, 0, 360 * deg);
  emcalElectronicsLV = new G4LogicalVolume(electronicsSolid, electronicsMat, "EMCalElectronicsLV");
  new G4PVPlacement(nullptr, emcalPos, emcalElectronicsLV, "EMCalElectronics", worldLV, false, 0);

  auto* aluminum = CreateMaterial("G4_Al");
  auto* outerSurfaceSolid =
      new G4Tubs("EMCalOuterSurface", 130.5 * cm, 132.85 * cm, 497.91 * cm / 2, 0, 360 * deg);
  emcalOuterSurfaceLV = new G4LogicalVolume(outerSurfaceSolid, aluminum, "EMCalOuterSurfaceLV");
  new G4PVPlacement(nullptr, emcalPos, emcalOuterSurfaceLV, "EMCalOuterSurface", worldLV, false, 0);

  auto* innerSurfaceSolid =
      new G4Tubs("EMCalInnerSurface", 80.2 * cm, 80.5 * cm, 497.91 * cm / 2, 0, 360 * deg);
  emcalInnerSurfaceLV = new G4LogicalVolume(innerSurfaceSolid, aluminum, "EMCalInnerSurfaceLV");
  new G4PVPlacement(nullptr, emcalPos, emcalInnerSurfaceLV, "EMCalInnerSurface", worldLV, false, 0);

  auto* offsetAirSolid =
      new G4Tubs("EMCalOffsetAir", 79.02 * cm, 80.2 * cm, 497.91 * cm / 2, 0, 360 * deg);
  emcalOffsetAirLV = new G4LogicalVolume(offsetAirSolid, air, "EMCalOffsetAirLV");
  new G4PVPlacement(nullptr, emcalPos, emcalOffsetAirLV, "EMCalOffsetAir", worldLV, false, 0);

  auto* aluminumPlateSolid =
      new G4Tubs("EMCalAluminumPlate", 78.72 * cm, 79.02 * cm, 497.91 * cm / 2, 0, 360 * deg);
  emcalAluminumPlateLV =
      new G4LogicalVolume(aluminumPlateSolid, aluminum, "EMCalAluminumPlateLV");
  new G4PVPlacement(nullptr, emcalPos, emcalAluminumPlateLV, "EMCalAluminumPlate", worldLV, false, 0);
}

void EICDetectorConstruction::ConstructMicromegas(G4LogicalVolume* worldLV) {
  std::vector<MicromegasSet> micromegasSet = {
      {48.75 * cm, 49.75 * cm, 120.0 * cm, "Micromegas1"},
      {50.75 * cm, 51.75 * cm, 130.0 * cm, "Micromegas2"},
      {52.75 * cm, 53.75 * cm, 140.0 * cm, "Micromegas3"},
      {58.75 * cm, 59.75 * cm, 200.0 * cm, "Micromegas4"},
      {60.75 * cm, 61.75 * cm, 210.0 * cm, "Micromegas5"}};
  auto* gas = CreateCompositeMaterial("ArCO2");

  micromegasLV.clear();
  for (size_t i = 0; i < micromegasSet.size(); ++i) {
    auto* solid = new G4Tubs(micromegasSet[i].name,
                             micromegasSet[i].rInner,
                             micromegasSet[i].rOuter,
                             micromegasSet[i].lengthZ / 2., 0., 360.*deg);
    auto* lv = new G4LogicalVolume(solid, gas, micromegasSet[i].name);
    micromegasLV.push_back(lv);
    new G4PVPlacement(nullptr, {}, lv, micromegasSet[i].name, worldLV, false, i);

    auto* vis = new G4VisAttributes(G4Colour(0., 1., 0., 0.4));
    vis->SetVisibility(true);
    lv->SetVisAttributes(vis);
  }
}
void EICDetectorConstruction::ConstructInnerTracker(G4LogicalVolume* worldLV) {
  auto* silicon = CreateMaterial("G4_Si");
  struct DiskSpec { G4double t, rMin, rMax, z; G4String name; };
  std::vector<DiskSpec> disks = {
      {2.5 * cm, 3.676 * cm, 23.0 * cm, 25.0 * cm,   "HD_Disk_1"},
      {2.5 * cm, 3.676 * cm, 43.0 * cm, 46.25 * cm,  "HD_Disk_2"},
      {2.5 * cm, 3.842 * cm, 43.0 * cm, 68.75 * cm,  "HD_Disk_3"},
      {2.5 * cm, 5.443 * cm, 43.0 * cm, 98.75 * cm,  "HD_Disk_4"},
      {2.5 * cm, 7.014 * cm, 43.0 * cm, 133.75 * cm, "HD_Disk_5"},
      {2.5 * cm, 3.676 * cm, 43.0 * cm, -25.0 * cm,  "LD_Disk_1"},
      {2.5 * cm, 3.676 * cm, 43.0 * cm, -43.75 * cm, "LD_Disk_2"},
      {2.5 * cm, 3.676 * cm, 43.0 * cm, -66.25 * cm, "LD_Disk_3"},
      {2.5 * cm, 4.00614 * cm, 43.0 * cm, -91.25 * cm, "LD_Disk_4"},
      {2.5 * cm, 4.63529 * cm, 43.0 * cm, -116.25 * cm, "LD_Disk_5"},
  };

  innerTrackerDisksLV.clear();
  for (const auto& d : disks) {
    auto* solid = new G4Tubs(d.name, d.rMin, d.rMax, 0.5 * d.t, 0, 360 * deg);
    auto* lv    = new G4LogicalVolume(solid, silicon, d.name + "_LV");
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, d.z), lv, d.name, worldLV, false, 0);
    innerTrackerDisksLV.push_back(lv);
  }
}

void EICDetectorConstruction::ConstructSDandField() {
  auto* sdManager = G4SDManager::GetSDMpointer();
  auto* eicSD     = new EICSensitiveDetector("EICSD");
  sdManager->AddNewDetector(eicSD);

  for (auto* lv : fvtxDisksLV) if (lv) lv->SetSensitiveDetector(eicSD);

  for (auto* lv : innerTrackerDisksLV) if (lv) lv->SetSensitiveDetector(eicSD);
  for (auto* lv : micromegasLV)        if (lv) lv->SetSensitiveDetector(eicSD);

  // (Option) EMCal sous-composants sensibles
  if (emcalCrystalsLV)      emcalCrystalsLV->SetSensitiveDetector(eicSD);
  if (emcalElectronicsLV)   emcalElectronicsLV->SetSensitiveDetector(eicSD);
  if (emcalOuterSurfaceLV)  emcalOuterSurfaceLV->SetSensitiveDetector(eicSD);
  if (emcalInnerSurfaceLV)  emcalInnerSurfaceLV->SetSensitiveDetector(eicSD);
  if (emcalOffsetAirLV)     emcalOffsetAirLV->SetSensitiveDetector(eicSD);
  if (emcalAluminumPlateLV) emcalAluminumPlateLV->SetSensitiveDetector(eicSD);

  
  //auto* invisible = new G4VisAttributes();
  //invisible->SetVisibility(false);

    /*
  if (emcalLV)              emcalLV->SetVisAttributes(invisible);
  if (hcalLV)               hcalLV->SetVisAttributes(invisible);
  if (solenoidLV)           solenoidLV->SetVisAttributes(invisible);
  if (richLV)               richLV->SetVisAttributes(invisible);
  if (barrelLV)             barrelLV->SetVisAttributes(invisible);
  if (tofLV)                tofLV->SetVisAttributes(invisible);
  if (emcalCrystalsLV)      emcalCrystalsLV->SetVisAttributes(invisible);
  if (emcalElectronicsLV)   emcalElectronicsLV->SetVisAttributes(invisible);
  if (emcalOuterSurfaceLV)  emcalOuterSurfaceLV->SetVisAttributes(invisible);
  if (emcalInnerSurfaceLV)  emcalInnerSurfaceLV->SetVisAttributes(invisible);
  if (emcalOffsetAirLV)     emcalOffsetAirLV->SetVisAttributes(invisible);
  if (emcalAluminumPlateLV) emcalAluminumPlateLV->SetVisAttributes(invisible);
  for (auto* lv : innerTrackerDisksLV) if (lv) lv->SetVisAttributes(invisible);
  for (auto* lv : micromegasLV)        if (lv) lv->SetVisAttributes(invisible);

  // Cible visible
  if (targetLV) {
    auto* visTarget = new G4VisAttributes(G4Colour(1., 1., 0., 0.9));
    visTarget->SetVisibility(true);
    visTarget->SetForceSolid(true);
    targetLV->SetVisAttributes(visTarget);
  }

  // FVTX visible (enveloppe + beampipe + disques)
  if (fvtxEnvelopeLV) {
    auto* envVis = new G4VisAttributes(G4Colour(0.95, 0.5, 0.5, 0.10));
    envVis->SetVisibility(true);
    fvtxEnvelopeLV->SetVisAttributes(envVis);
  }
  if (fvtxBeamPipeLV) {
    auto* beVis  = new G4VisAttributes(G4Colour(0.8, 0.8, 0.2, 0.5));
    beVis->SetVisibility(true);
    beVis->SetForceSolid(true);
    fvtxBeamPipeLV->SetVisAttributes(invisible);
  }
  for (auto* lv : fvtxDisksLV) {
    if (!lv) continue;
    auto* vis = new G4VisAttributes(G4Colour(0.3, 0.9, 1.0, 0.6));
    vis->SetVisibility(true);
    vis->SetForceSolid(true);
    lv->SetVisAttributes(vis);
  }
*/
  G4cout << "[FVTX] Disks set sensitive. Only Target + FVTX visible." << G4endl;
}
