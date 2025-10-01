CXX = g++
PYTHIA8_DIR = /Users/charles-josephnaim/Desktop/EIC/ePIC/releases

# Flags Geant4 et ROOT + options warnings/erreurs
CXXFLAGS = $(shell $(G4INSTALL)/bin/geant4-config --cflags) \
           $(shell root-config --cflags) \
           -std=c++17 \
           -Wall -Wextra -Wpedantic -g

# Ajout flags Pythia8 includes
CXXFLAGS += -I$(PYTHIA8_DIR)/include

# Librairies Geant4, ROOT, Pythia8
LDFLAGS = $(shell $(G4INSTALL)/bin/geant4-config --libs) \
          $(shell root-config --libs)
LDFLAGS += -L$(PYTHIA8_DIR)/lib -lpythia8 -ldl -lz -Wl,-rpath,$(PYTHIA8_DIR)/lib

SRC = main.cc EICSensitiveDetector.cc ActionInitialization.cc \
      PrimaryGeneratorAction.cc EICDetectorConstruction.cc \
      PrimaryGeneratorAction.cc
OBJ = $(SRC:.cc=.o)
EXEC = mySimulation

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
