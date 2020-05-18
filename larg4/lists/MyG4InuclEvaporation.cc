//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4InuclEvaporation.cc 66241 2012-12-13 18:34:42Z gunter $
//
// 20100114  M. Kelsey -- Remove G4CascadeMomentum, use G4LorentzVector directly
// 20100405  M. Kelsey -- Pass const-ref std::vector<>
// 20100413  M. Kelsey -- Pass G4CollisionOutput by ref to ::collide(), use
//		const_iterator.
// 20100428  M. Kelsey -- Use G4InuclParticleNames enum
// 20100429  M. Kelsey -- Change "case gamma:" to "case photon:"
// 20100517  M. Kelsey -- Follow new ctors for G4*Collider family.  Make
//		G4EvaporationInuclCollider a data member.
// 20100520  M. Kelsey -- Simplify collision loop, move momentum rotations to
//		G4CollisionOutput, copy G4DynamicParticle directly from
//		G4InuclParticle, no switch-block required.  Fix scaling factors.
// 20100914  M. Kelsey -- Migrate to integer A and Z
// 20100924  M. Kelsey -- Migrate to "OutgoingNuclei" names in CollisionOutput
// 20110728  M. Kelsey -- Fix Coverity #28776, remove return after throw.
// 20120608  M. Kelsey -- Fix variable-name "shadowing" compiler warnings.
// 20140310  M. Kelsey -- *TEMPORARY* const-cast G4PD* for G4Fragment ctor.
// 20200329  D. Rivera -- Copy based on:
//    geant4.10.03.p03/source/processes/hadronic/models/cascade/cascade/src/G4InuclEvaporation.cc

#include "MyG4InuclEvaporation.hh"

#include <numeric>

//#include "Geant4/G4InuclEvaporation.hh"
#include "Geant4/globals.hh"
#include "Geant4/G4SystemOfUnits.hh"
#include "Geant4/G4IonTable.hh"
#include "Geant4/G4V3DNucleus.hh"
#include "Geant4/G4DynamicParticleVector.hh"
#include "Geant4/G4EvaporationInuclCollider.hh"
#include "Geant4/G4InuclNuclei.hh"
#include "Geant4/G4Track.hh"
#include "Geant4/G4Nucleus.hh"
#include "Geant4/G4Nucleon.hh"
#include "Geant4/G4NucleiModel.hh"
#include "Geant4/G4HadronicException.hh"
#include "Geant4/G4LorentzVector.hh"
#include "Geant4/G4EquilibriumEvaporator.hh"
#include "Geant4/G4InuclElementaryParticle.hh"
#include "Geant4/G4InuclParticle.hh"
#include "Geant4/G4CollisionOutput.hh"
#include "Geant4/G4InuclParticleNames.hh"

using namespace G4InuclParticleNames;

typedef std::vector<G4InuclElementaryParticle>::const_iterator particleIterator;
typedef std::vector<G4InuclNuclei>::const_iterator nucleiIterator;


MyG4InuclEvaporation::MyG4InuclEvaporation()
  : verboseLevel(0),   evaporator(new G4EvaporationInuclCollider) {}

MyG4InuclEvaporation::MyG4InuclEvaporation(const MyG4InuclEvaporation &) : G4VEvaporation() {
  throw G4HadronicException(__FILE__, __LINE__, "MyG4InuclEvaporation::copy_constructor meant to not be accessable.");
}

MyG4InuclEvaporation::~MyG4InuclEvaporation() {
  delete evaporator;
}

const MyG4InuclEvaporation & MyG4InuclEvaporation::operator=(const MyG4InuclEvaporation &) {
  throw G4HadronicException(__FILE__, __LINE__, "MyG4InuclEvaporation::operator= meant to not be accessable.");
}


G4bool MyG4InuclEvaporation::operator==(const MyG4InuclEvaporation &) const {
  return false;
}

G4bool MyG4InuclEvaporation::operator!=(const MyG4InuclEvaporation &) const {
  return true;
}

void MyG4InuclEvaporation::setVerboseLevel( const G4int verbose ) {
  verboseLevel = verbose;
}

G4FragmentVector* MyG4InuclEvaporation::BreakItUp(const G4Fragment &theNucleus) {
  G4FragmentVector* theResult = new G4FragmentVector;

  if (theNucleus.GetExcitationEnergy() <= 0.0) { // Check that Excitation Energy > 0
    theResult->push_back(new G4Fragment(theNucleus));
    return theResult;
  }

  G4int A = theNucleus.GetA_asInt();
  G4int Z = theNucleus.GetZ_asInt();
  G4double mTar  = G4NucleiProperties::GetNuclearMass(A, Z); // Mass of the target nucleus

  G4ThreeVector momentum =  theNucleus.GetMomentum().vect();
  G4double exitationE = theNucleus.GetExcitationEnergy();

  G4double mass = mTar;
  G4ThreeVector boostToLab( momentum/mass );

  if ( verboseLevel > 2 )
    G4cout << " MyG4InuclEvaporation : initial kinematics : boostToLab vector = " << boostToLab << G4endl
	   << "                     excitation energy  : " << exitationE << G4endl;

  if (verboseLevel > 2) {
    G4cout << "MyG4InuclEvaporation::BreakItUp >>> A: " << A << " Z: " << Z
	   << " exitation E: " << exitationE << " mass: " << mTar/GeV << " GeV"
	   << G4endl;
  };

  G4InuclNuclei* nucleus = new G4InuclNuclei(A, Z);
  nucleus->setExitationEnergy(exitationE);

  G4CollisionOutput output;
  evaporator->collide(0, nucleus, output);

  const std::vector<G4InuclNuclei>& outgoingNuclei = output.getOutgoingNuclei();
  const std::vector<G4InuclElementaryParticle>& particles = output.getOutgoingParticles();

  G4double eTot=0.0;
  G4int  i=1;

  if (!particles.empty()) {
    G4int outgoingType;
    particleIterator ipart = particles.begin();
    for (; ipart != particles.end(); ipart++) {
      outgoingType = ipart->type();

      if (verboseLevel > 2) {
	G4cout << "Evaporated particle:  " << i << " of type: "
	       << outgoingType << G4endl;
        i++;
      }

      eTot += ipart->getEnergy();

      G4LorentzVector vlab = ipart->getMomentum().boost(boostToLab);

      // TEMPORARY:  Remove constness on PD until G4Fragment is fixed
      theResult->push_back( new G4Fragment(vlab, ipart->getDefinition()) );
    }
  }

  if (!outgoingNuclei.empty()) {
    nucleiIterator ifrag = outgoingNuclei.begin();
    for (i=1; ifrag != outgoingNuclei.end(); ifrag++) {
      if (verboseLevel > 2) {
	G4cout << " Nuclei fragment: " << i << G4endl; i++;
      }

      eTot += ifrag->getEnergy();

      G4LorentzVector vlab = ifrag->getMomentum().boost(boostToLab);

      G4int fragA = ifrag->getA();
      G4int fragZ = ifrag->getZ();
      if (verboseLevel > 2) {
	G4cout << "boosted v" << vlab << G4endl;
      }
      theResult->push_back( new G4Fragment(fragA, fragZ, vlab) );
    }
  }

  return theResult;
}
