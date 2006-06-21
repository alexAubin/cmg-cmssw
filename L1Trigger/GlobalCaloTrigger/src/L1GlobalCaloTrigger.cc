#include "L1Trigger/GlobalCaloTrigger/interface/L1GlobalCaloTrigger.h"

#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctMap.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctSourceCard.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctJetLeafCard.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctEmLeafCard.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctWheelJetFpga.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctWheelEnergyFpga.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctJetFinalStage.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctGlobalEnergyAlgos.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctElectronFinalSort.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctJetEtCalibrationLut.h"
#include "L1Trigger/GlobalCaloTrigger/interface/L1GctJetCounterLut.h"

#include "FWCore/Utilities/interface/Exception.h"

#include <sstream>

using std::cout;
using std::string;
using std::stringstream;
using std::endl;
using std::vector;

//DEFINE STATICS
const int L1GlobalCaloTrigger::N_SOURCE_CARDS = 54;
const int L1GlobalCaloTrigger::N_JET_LEAF_CARDS = 6;
const int L1GlobalCaloTrigger::N_EM_LEAF_CARDS = 2;
const int L1GlobalCaloTrigger::N_WHEEL_CARDS = 2;

const unsigned int L1GlobalCaloTrigger::N_JET_COUNTERS_PER_WHEEL = L1GctWheelJetFpga::N_JET_COUNTERS;


// constructor
L1GlobalCaloTrigger::L1GlobalCaloTrigger(bool useFile) :
  readFromFile(useFile),
  theSourceCards(N_SOURCE_CARDS),
  theJetLeafCards(N_JET_LEAF_CARDS),
  theEmLeafCards(N_EM_LEAF_CARDS),
  theWheelJetFpgas(N_WHEEL_CARDS),
  theWheelEnergyFpgas(N_WHEEL_CARDS),
  m_minusWheelJetCounterLuts(N_JET_COUNTERS_PER_WHEEL),
  m_plusWheelJetCounterLuts(N_JET_COUNTERS_PER_WHEEL)
{
  setupLuts();
  build();
  
}

L1GlobalCaloTrigger::~L1GlobalCaloTrigger()
{
  theSourceCards.clear();
}

void L1GlobalCaloTrigger::openSourceCardFiles(string fileBase){
  //Loop running over the 18 RCT-crate files, allocating 3 sourcecards per file
  for(int i = 0;i < 18; i++){
    string fileNo;
    stringstream ss;
    ss << i;
    ss >> fileNo;
    string fileName = fileBase+fileNo;
    theSourceCards.at(3*i)->openInputFile(fileName);
    theSourceCards.at(3*i+1)->openInputFile(fileName);
    theSourceCards.at(3*i+2)->openInputFile(fileName);
  }
}

void L1GlobalCaloTrigger::reset() {

  // Source cards
  for (int i=0; i<N_SOURCE_CARDS; i++) {
    theSourceCards.at(i)->reset();
  }

  // EM Leaf Card
  for (int i=0; i<N_EM_LEAF_CARDS; i++) {
    theEmLeafCards.at(i)->reset();
  }

  // Jet Leaf cards
  for (int i=0; i<N_JET_LEAF_CARDS; i++) {
    theJetLeafCards.at(i)->reset();
  }

  // Wheel Cards
  for (int i=0; i<N_WHEEL_CARDS; i++) {
    theWheelJetFpgas.at(i)->reset();
  }

  for (int i=0; i<N_WHEEL_CARDS; i++) {
    theWheelEnergyFpgas.at(i)->reset();
  }

  // Electron Final Stage
  theIsoEmFinalStage->reset();
  theNonIsoEmFinalStage->reset();

  // Jet Final Stage
  theJetFinalStage->reset();

  // Energy Final Stage
  theEnergyFinalStage->reset();

}

void L1GlobalCaloTrigger::process() {
		
  // Source cards
  for (int i=0; i<N_SOURCE_CARDS; i++) {
    if (readFromFile) {
      theSourceCards.at(i)->readBX();
    }
  }

  // EM Leaf Card
  for (int i=0; i<N_EM_LEAF_CARDS; i++) {
    theEmLeafCards.at(i)->fetchInput();
    theEmLeafCards.at(i)->process();
  }

  // Jet Leaf cards
  for (int i=0; i<N_JET_LEAF_CARDS; i++) {
    theJetLeafCards.at(i)->fetchInput();
    theJetLeafCards.at(i)->process();
  }

  // Wheel Cards
  for (int i=0; i<N_WHEEL_CARDS; i++) {
    theWheelJetFpgas.at(i)->fetchInput();
    theWheelJetFpgas.at(i)->process();
  }

  for (int i=0; i<N_WHEEL_CARDS; i++) {
    theWheelEnergyFpgas.at(i)->fetchInput();
    theWheelEnergyFpgas.at(i)->process();
  }

  // Electron Final Stage
  theIsoEmFinalStage->fetchInput();
  theIsoEmFinalStage->process();

  theNonIsoEmFinalStage->fetchInput();
  theNonIsoEmFinalStage->process();


  // Jet Final Stage
  theJetFinalStage->fetchInput();
  theJetFinalStage->process();

  // Energy Final Stage
  theEnergyFinalStage->fetchInput();
  theEnergyFinalStage->process();

}

void L1GlobalCaloTrigger::setRegion(L1GctRegion region) {

  if (readFromFile) {
    throw cms::Exception("L1GctInputError")
      << " L1 Global Calo Trigger is set to read input data from file, "
      << " setRegion method should not be used\n"; 
  }
  unsigned scnum = L1GctMap::getMap()->sourceCard(region);
  unsigned input = L1GctMap::getMap()->sourceCardOutput(region);
  L1GctSourceCard* sc = theSourceCards.at(scnum);
  std::vector<L1GctRegion> tempRegions = sc->getRegions();
  tempRegions.at(input) = region;
  sc->setRegions(tempRegions);
}

void L1GlobalCaloTrigger::print() {

  cout << "=== Global Calo Trigger ===" << endl;
  cout << "=== START DEBUG OUTPUT  ===" << endl;

  cout << endl;
  cout << "N Source Cards " << theSourceCards.size() << endl;
  cout << "N Jet Leaf Cards " << theJetLeafCards.size() << endl;
  cout << "N Wheel Jet Fpgas " << theWheelJetFpgas.size() << endl;
  cout << "N Wheel Energy Fpgas " << theWheelEnergyFpgas.size() << endl;
  cout << "N Em Leaf Cards " << theEmLeafCards.size() << endl;
  cout << endl;

  for (unsigned i=0; i<theSourceCards.size(); i++) {
    cout << "Source Card " << i << " : " << theSourceCards.at(i) << endl;
    //cout << (*theSourceCards.at(i)); 
  }
  cout << endl;

  for (unsigned i=0; i<theJetLeafCards.size(); i++) {
    cout << "Jet Leaf Card " << i << " : " << theJetLeafCards.at(i) << endl;
    cout << (*theJetLeafCards.at(i));
  }
  cout << endl;

  for (unsigned i=0; i<theWheelJetFpgas.size(); i++) {
    cout << "Wheel Jet FPGA " << i << " : " << theWheelJetFpgas.at(i) << endl; 
    cout << (*theWheelJetFpgas.at(i));
  }
  cout << endl;

  for (unsigned i=0; i<theWheelEnergyFpgas.size(); i++) {
    cout << "Wheel Energy FPGA " << i <<" : " << theWheelEnergyFpgas.at(i) << endl; 
    cout << (*theWheelEnergyFpgas.at(i));
  }
  cout << endl;

  cout << (*theJetFinalStage);
  cout << endl;

  cout << (*theEnergyFinalStage);
  cout << endl;

  for (unsigned i=0; i<theEmLeafCards.size(); i++) {
    cout << "EM Leaf Card " << i << " : " << theEmLeafCards.at(i) << endl;
    cout << (*theEmLeafCards.at(i));
  }
  cout << endl;

  cout << (*theIsoEmFinalStage);
  cout << endl;

  cout << (*theNonIsoEmFinalStage);

  cout << "=== Global Calo Trigger ===" << endl;
  cout << "===  END DEBUG OUTPUT   ===" << endl;
 
}

// isolated EM outputs
vector<L1GctEmCand> L1GlobalCaloTrigger::getIsoElectrons() const { 
  return theIsoEmFinalStage->getOutputCands();
}	

// non isolated EM outputs
vector<L1GctEmCand> L1GlobalCaloTrigger::getNonIsoElectrons() const {
  return theNonIsoEmFinalStage->getOutputCands(); 
}

// central jet outputs to GT
vector<L1GctJet> L1GlobalCaloTrigger::getCentralJets() const {
  return theJetFinalStage->getCentralJets();
}

// forward jet outputs to GT
vector<L1GctJet> L1GlobalCaloTrigger::getForwardJets() const { 
  return theJetFinalStage->getForwardJets(); 
}

// tau jet outputs to GT
vector<L1GctJet> L1GlobalCaloTrigger::getTauJets() const { 
  return theJetFinalStage->getTauJets(); 
}

// total Et output
L1GctScalarEtVal L1GlobalCaloTrigger::getEtSum() const {
  return theEnergyFinalStage->getEtSum();
}

L1GctScalarEtVal L1GlobalCaloTrigger::getEtHad() const {
  return theEnergyFinalStage->getEtHad();
}

L1GctScalarEtVal L1GlobalCaloTrigger::getEtMiss() const {
  return theEnergyFinalStage->getEtMiss();
}

L1GctEtAngleBin L1GlobalCaloTrigger::getEtMissPhi() const {
  return theEnergyFinalStage->getEtMissPhi();
}

L1GctJcFinalType L1GlobalCaloTrigger::getJetCount(unsigned jcnum) const {
  return theEnergyFinalStage->getJetCount(jcnum);
}



/* PRIVATE METHODS */

// instantiate hardware/algorithms
void L1GlobalCaloTrigger::build() {

  // Jet Et LUT
  m_jetEtCalLut = new L1GctJetEtCalibrationLut();

  // Source cards
  for (int i=0; i<(N_SOURCE_CARDS/3); i++) {
    theSourceCards.at(3*i)   = new L1GctSourceCard(3*i,   L1GctSourceCard::cardType1);
    theSourceCards.at(3*i+1) = new L1GctSourceCard(3*i+1, L1GctSourceCard::cardType2);
    theSourceCards.at(3*i+2) = new L1GctSourceCard(3*i+2, L1GctSourceCard::cardType3);
  }

  // Now we have the source cards prepare vectors of the relevent cards for the connections

  // Jet leaf cards
  vector<L1GctSourceCard*> jetSourceCards(15);

   // Jet Leaf cards
  for (int i=0; i<N_JET_LEAF_CARDS; i++) {
    for (int j=0; j<6; j++) {
      // Source card numbering:
      // 3*i+1 cover the endcap and HF regions
      // 3*i+2 cover the barrel regions
      //
      // In the Leaf card and JetFinder, the even-numbered
      // inputs are the barrel regions and the odd-numbered
      // regions the endcap and HF
      int k = 3*(j/2) - (j%2) + 2;
      jetSourceCards.at(j)=theSourceCards.at((i*9+k));
      // Neighbour connections
      int iup = (i*3+3) % 9;
      int idn = (i*3+8) % 9;
      int ii, i0, i1, i2, i3, i4, i5;
      if (i<3) {
	ii = iup;
	i0 = idn;
	// Remaining connections are the ones across the eta-0 boundary
	i1 = idn+9;
	i2 = i*3+9;
	i3 = i*3+10;
	i4 = i*3+11;
	i5 = iup+9;
      } else {
	ii = iup+9;
	i0 = idn + 9;
	// Remaining connections are the ones across the eta-0 boundary
	i1 = idn;
	i2 = i*3-9;
	i3 = i*3-8;
	i4 = i*3-7;
	i5 = iup;
      }
      // Leaf card inputs 6-9 are the neighbours in phi,
      // taking account of wraparound at phi=0
      jetSourceCards.at(6) = theSourceCards.at(ii*3+2);
      jetSourceCards.at(7) = theSourceCards.at(ii*3+1);
      jetSourceCards.at(8) = theSourceCards.at(i0*3+2);
      jetSourceCards.at(9) = theSourceCards.at(i0*3+1);
      // The remaining connections are those for the eta-0
      // regions from the other wheel and may change
      jetSourceCards.at(10)= theSourceCards.at(i1*3+2);
      jetSourceCards.at(11)= theSourceCards.at(i2*3+2);
      jetSourceCards.at(12)= theSourceCards.at(i3*3+2);
      jetSourceCards.at(13)= theSourceCards.at(i4*3+2);
      jetSourceCards.at(14)= theSourceCards.at(i5*3+2);
      //
      
    }
    theJetLeafCards.at(i) = new L1GctJetLeafCard(i,i % 3,jetSourceCards, m_jetEtCalLut);
  }

  // EM leaf cards  
  vector<L1GctSourceCard*> emSourceCards(9);

  for (int i=0; i<N_EM_LEAF_CARDS; i++) {
    for (int j=0; j<9; j++) {
      emSourceCards.at(j)=theSourceCards.at((i*9+j)*3);
    }
    theEmLeafCards.at(i) = new L1GctEmLeafCard(i,emSourceCards);
  }

   // Wheel Fpgas
   vector<L1GctJetLeafCard*> wheelJetLeafCards(3);
   vector<L1GctJetLeafCard*> wheelEnergyLeafCards(3);

   for (int i=0; i<N_WHEEL_CARDS; i++) {
     for (int j=0; j<3; j++) {
       wheelJetLeafCards.at(j)=theJetLeafCards.at(i*3+j);
       wheelEnergyLeafCards.at(j)=theJetLeafCards.at(i*3+j);
     }
     if (i==0) { 
       theWheelJetFpgas.at(i) = new L1GctWheelJetFpga(i,wheelJetLeafCards,m_minusWheelJetCounterLuts);
     } else { 
       theWheelJetFpgas.at(i) = new L1GctWheelJetFpga(i,wheelJetLeafCards,m_plusWheelJetCounterLuts);
     } 
     theWheelEnergyFpgas.at(i) = new L1GctWheelEnergyFpga(i,wheelEnergyLeafCards);
   }
  
   // Jet Final Stage  
   theJetFinalStage = new L1GctJetFinalStage(theWheelJetFpgas);

  // Electron Final Sort
   theIsoEmFinalStage = new L1GctElectronFinalSort(true,theEmLeafCards.at(0), theEmLeafCards.at(1));
   theNonIsoEmFinalStage = new L1GctElectronFinalSort(false,theEmLeafCards.at(0), theEmLeafCards.at(1));  

  // Global Energy Algos
  theEnergyFinalStage = new L1GctGlobalEnergyAlgos(theWheelEnergyFpgas, theWheelJetFpgas);

}

void L1GlobalCaloTrigger::setupLuts() {

  setupJetEtCalibrationLut();
  setupJetCounterLuts();

}

void L1GlobalCaloTrigger::setupJetEtCalibrationLut() {

  // Jet Et LUT
  // We may have some parameters here at some point
  m_jetEtCalLut = new L1GctJetEtCalibrationLut();

}

void L1GlobalCaloTrigger::setupJetCounterLuts() {

  // Initialise look-up tables for Minus and Plus wheels
  unsigned j=0;
  // Setup the first counters in the list for some arbitrary conditions
  // Energy cut
  m_minusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut(L1GctJetCounterLut::minRank, 5);
  m_plusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut(L1GctJetCounterLut::minRank, 5);
  j++;

  // Eta cuts
  m_minusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut(L1GctJetCounterLut::centralEta, 5);
  m_plusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut(L1GctJetCounterLut::centralEta, 5);
  j++;

  // Some one-sided eta cuts
  m_minusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut(L1GctJetCounterLut::forwardEta, 6);
  m_plusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut();
  j++;

  m_minusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut();
  m_plusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut(L1GctJetCounterLut::forwardEta, 6);
  j++;


  // Set the remainder to null counters
  for (; j<N_JET_COUNTERS_PER_WHEEL; j++) {
    m_minusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut();
    m_plusWheelJetCounterLuts.at(j) = new L1GctJetCounterLut();
  }
}
