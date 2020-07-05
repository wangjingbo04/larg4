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
// 20200704  D. Rivera -- Copy based on :
//    geant4.10.03.p03/source/processes/hadronic/stopping/include/G4SigmaMinusAbsorptionBertini.hh
//
//----------------------------------------------------------------------------

#ifndef MyG4SigmaMinusAbsorptionBertini_h
#define MyG4SigmaMinusAbsorptionBertini_h 1

// Class Description:
//
// Process for Sigma- absorption at rest. 
// To be used in your physics list in case you need this physics.

#include "MyG4HadronicAbsorptionBertini.hh"
#include "Geant4/G4SigmaMinus.hh"


class MyG4SigmaMinusAbsorptionBertini : public MyG4HadronicAbsorptionBertini {
private:
  // hide assignment operator as private 
  MyG4SigmaMinusAbsorptionBertini& operator=(const MyG4SigmaMinusAbsorptionBertini&);
  MyG4SigmaMinusAbsorptionBertini(const MyG4SigmaMinusAbsorptionBertini&);
  
public:
  MyG4SigmaMinusAbsorptionBertini()
    : MyG4HadronicAbsorptionBertini(G4SigmaMinus::Definition()) {}

  virtual ~MyG4SigmaMinusAbsorptionBertini() {}
};

#endif