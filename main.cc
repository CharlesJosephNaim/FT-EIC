#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "EICDetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "FTFP_BERT.hh"

#include <iostream>
#include <cstdlib>
#include <cmath>


namespace phys {
  constexpr long double NA   = 6.02214076e23L;   // Avogadro [mol^-1]
  constexpr long double WEEK = 7.0L*24.0L*3600.0L; // [s] = 604800
  constexpr long double THICK = 1.0L;            // t = 1 cm (fixed target)
  constexpr long double EFF   = 1.0L;            // Efficiency
  // PHI = dN/dt [part/s]. 6.24e18 ~ 1 A ; 6.24e12 ~ 1 µA.
  constexpr long double PHI  = 6.24e18L;         // beam (part/s)
}

inline bool GetMatProps(const std::string& nuc, long double& M, long double& rho) {
  std::string k = nuc; for (auto& c : k) c = std::toupper(c);
  if (k=="U"  || k=="URANIUM") { M=238.0L;  rho=18.95L;  return true; }
  if (k=="C"  || k=="CARBON")  { M=12.01L;  rho=2.267L;  return true; }
  if (k=="H"  || k=="HYDROGEN"){ M=1.008L;  rho=0.08988e-3L; return true; }
  if (k=="AL" || k=="ALUMINUM"){ M=26.98L;  rho=2.70L;   return true; }
  if (k=="CU" || k=="COPPER")  { M=63.546L; rho=8.96L;   return true; }
  if (k=="PB" || k=="LEAD")    { M=207.2L;  rho=11.34L;  return true; }
  return false;
}

static long long ComputeEvents(double sigma_mb, const std::string& nucleus) {
  long double M=0.0L, rho=0.0L;
  if (!GetMatProps(nucleus, M, rho)) {
    std::cerr << "[ERROR] Unknown nucleus/material: " << nucleus << "\n";
    return 0;
  }

  // Areal density [atoms/cm^2]
  const long double RHOT = (phys::NA / M) * rho * phys::THICK;

  // Luminosity [cm^-2 s^-1]
  const long double L = phys::PHI * RHOT;

  // sigma: mb -> barns -> cm^2
  const long double sigma_b = (long double)sigma_mb * 1.0e-3L; // mb -> b
  const long double sigma_c = sigma_b * 1.0e-24L;              // b  -> cm^2

  // Events over one week
  long double N = L * phys::WEEK * sigma_c * phys::EFF;

  // Clamp to signed 64-bit
  if (N < 0.0L) N = 0.0L;
  constexpr long double Nmax64 = 9.22e18L;
  if (N > Nmax64) {
    std::cerr << "[WARN] N exceeds 64-bit range; clamping to " << (long double)Nmax64 << "\n";
    N = Nmax64;
  }
  return static_cast<long long>(llround(N));
}

int main(int argc, char** argv) {
    // --- Run manager ---
    auto runManager = new G4MTRunManager();
    runManager->SetNumberOfThreads(1);

    auto detector = new EICDetectorConstruction();
    runManager->SetUserInitialization(detector);
    runManager->SetUserInitialization(new FTFP_BERT());
    runManager->SetUserInitialization(new ActionInitialization(detector));

    // --- Initialize the kernel ---
    runManager->Initialize();

    G4VisExecutive* visManager = nullptr;
    G4UIExecutive* ui = nullptr;
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    if (argc == 1) {
        // ----- Interactive mode -----
        ui = new G4UIExecutive(argc, argv);
        visManager = new G4VisExecutive();
        visManager->Initialize();
        
        // Load visualization macro (customize as needed)
        UImanager->ApplyCommand("/control/execute init_vis.mac");

        // Run 1000 events in interactive mode
        runManager->BeamOn(100);

        // Start UI session
        ui->SessionStart();
        delete ui;
    } else {
        // ----- Batch mode -----
        std::string arg1 = argv[1];
        if (arg1.find(".mac") != std::string::npos) {
            // Case: macro file
            UImanager->ApplyCommand("/control/execute " + arg1);
        } else {
            // Case: cross section in mb
            double sigma_mb = std::atof(argv[1]);
            if (sigma_mb <= 0) {
                std::cerr << "Usage: " << argv[0]
                          << " [macro.mac | sigma_mb]\n";
                delete runManager;
                return 1;
            }
            long long N = 100000;
            //ComputeEvents(sigma_mb, "H");
            
            std::cout << "[INFO] sigma = " << sigma_mb
                      << "  mb -> BeamOn(" << N << ")\n" << std::endl;
            
            runManager->BeamOn(N);
        }
    }
    delete visManager;
    delete runManager;
    return 0;
}

