#include "BaconProd/Utils/interface/JetTools.hh"
#include "BaconProd/Utils/interface/QjetsPlugin.h"
#include "BaconProd/Utils/interface/Qjets.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "fastjet/GhostedAreaSpec.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include <TLorentzVector.h>
#include <TMath.h>
#include <TMatrixDSym.h>
#include <TMatrixDSymEigen.h>

using namespace baconhep;

//--------------------------------------------------------------------------------------------------
double JetTools::beta(const reco::PFJet &jet, const reco::Vertex &pv, const double dzCut)
{
  double pt_jets=0, pt_jets_tot=0;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    const reco::TrackRef       track  = pfcand->trackRef();
    if(track.isNull()) continue;
    
    pt_jets_tot += track->pt();
    
    if(fabs(track->dz(pv.position())) < dzCut) {
      pt_jets += track->pt();
    }
  }
  
  return (pt_jets_tot>0) ? pt_jets/pt_jets_tot : 0;
}

//--------------------------------------------------------------------------------------------------
double JetTools::betaStar(const reco::PFJet &jet, const reco::Vertex &pv, const reco::VertexCollection *pvCol, const double dzCut)
{
  double pileup=0, total=0;
  
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    const reco::TrackRef       track  = pfcand->trackRef();
    if(track.isNull()) continue;
    total += track->pt();
    
    double dzPV = fabs(track->dz(pv.position()));
    if(dzPV <= dzCut) continue;
    
    double dzMin = dzPV;
    for(reco::VertexCollection::const_iterator itVtx = pvCol->begin(); itVtx!=pvCol->end(); ++itVtx) {
      if(itVtx->ndof() < 4 || (pv.position() - itVtx->position()).R() < 0.02) continue;
      dzMin = TMath::Min(dzMin, fabs(track->dz(itVtx->position())));
    }
    if(dzMin < dzCut) pileup += track->pt();
  }
  if(total==0) total=1;
  
  return pileup/total;
}

//--------------------------------------------------------------------------------------------------
double JetTools::dRMean(const reco::PFJet &jet, const int pfType)
{
  double drmean=0;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    if(pfType!=-1 && pfcand->particleId() != pfType) continue;
    
    double dr = reco::deltaR(jet.eta(),jet.phi(),pfcand->eta(),pfcand->phi());    
    drmean += dr*(pfcand->pt())/(jet.pt());
  }
  
  return drmean;
}

//--------------------------------------------------------------------------------------------------
double JetTools::dR2Mean(const reco::PFJet &jet, const int pfType)
{
  double dr2mean=0;
  double sumpt2=0;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    if(pfType!=-1 && pfcand->particleId() != pfType) continue;
    
    sumpt2 += pfcand->pt() * pfcand->pt();
    
    double dr = reco::deltaR(jet.eta(),jet.phi(),pfcand->eta(),pfcand->phi());    
    dr2mean += dr*dr*(pfcand->pt() * pfcand->pt());
  }
  dr2mean/=sumpt2;
  
  return dr2mean;
}

//--------------------------------------------------------------------------------------------------
double JetTools::frac(const reco::PFJet &jet, const double dRMax, const int pfType)
{
  const double dRMin = dRMax - 0.1;
  
  double fraction = 0;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    if(pfType!=-1 && pfcand->particleId() != pfType) continue;
    
    double dr = reco::deltaR(jet.eta(),jet.phi(),pfcand->eta(),pfcand->phi());    
    if(dr > dRMax) continue;
    if(dr < dRMin) continue;
    
    fraction += pfcand->pt() / jet.pt();
  }
  
  return fraction;
}

//--------------------------------------------------------------------------------------------------
double JetTools::jetDz(const reco::PFJet &jet, const reco::Vertex &pv)
{
  // Assumes constituents are stored by descending pT
  double dz=-1000;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    const reco::TrackRef       track  = pfcand->trackRef();
    if(track.isNull()) continue;
    dz = track->dz(pv.position());
    break;
  }
  
  return dz;
}
//--------------------------------------------------------------------------------------------------
double JetTools::jetD0(const reco::PFJet &jet, const reco::Vertex &pv)
{
  // Assumes constituents are stored by descending pT
  double d0=-1000;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    const reco::TrackRef       track  = pfcand->trackRef();
    if(track.isNull()) continue;
    d0 = -track->dxy(pv.position());
    break;
  }
  
  return d0;
}

//--------------------------------------------------------------------------------------------------
double JetTools::jetWidth(const reco::PFJet &jet, const int varType, const int pfType)
{
  double ptD=0, sumPt=0, sumPt2=0;
  TMatrixDSym covMatrix(2); covMatrix=0.;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    if(pfType!=-1 && pfcand->particleId() != pfType) continue;
    
    double dEta = jet.eta() - pfcand->eta();
    double dPhi = reco::deltaPhi(jet.phi(), pfcand->phi());
    
    covMatrix(0,0) += pfcand->pt() * pfcand->pt() * dEta * dEta;
    covMatrix(0,1) += pfcand->pt() * pfcand->pt() * dEta * dPhi;
    covMatrix(1,1) += pfcand->pt() * pfcand->pt() * dPhi * dPhi;
    ptD            += pfcand->pt() * pfcand->pt();
    sumPt          += pfcand->pt();
    sumPt2         += pfcand->pt() * pfcand->pt();
  }
  covMatrix(0,0) /= sumPt2;
  covMatrix(0,1) /= sumPt2;
  covMatrix(1,1) /= sumPt2;
  covMatrix(1,0) = covMatrix(0,1);
  
  ptD /= sqrt(ptD);
  ptD /= sumPt;
  
  double etaW = sqrt(covMatrix(0,0));
  double phiW = sqrt(covMatrix(1,1));
  double jetW = 0.5*(etaW+phiW);
  
  TVectorD eigVals(2);
  eigVals = TMatrixDSymEigen(covMatrix).GetEigenValues();
  double majW = sqrt(fabs(eigVals(0)));
  double minW = sqrt(fabs(eigVals(1)));
  
  if     (varType==1) { return majW; }
  else if(varType==2) { return minW; }
  else if(varType==3) { return etaW; }
  else if(varType==4) { return phiW; }
  else if(varType==5) { return jetW; }  

  return ptD;
}
//--------------------------------------------------------------------------------------------------
double JetTools::jetWidth(fastjet::PseudoJet &jet, const int varType) { 
  double ptD=0, sumPt=0, sumPt2=0;
  TMatrixDSym covMatrix(2); covMatrix=0.;
  const unsigned int nPFCands = jet.constituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    fastjet::PseudoJet pfcand = jet.constituents()[ipf];
    
    double dEta = jet.eta() - pfcand.eta();
    double dPhi = reco::deltaPhi(jet.phi(), pfcand.phi());
    
    covMatrix(0,0) += pfcand.pt() * pfcand.pt() * dEta * dEta;
    covMatrix(0,1) += pfcand.pt() * pfcand.pt() * dEta * dPhi;
    covMatrix(1,1) += pfcand.pt() * pfcand.pt() * dPhi * dPhi;
    ptD            += pfcand.pt() * pfcand.pt();
    sumPt          += pfcand.pt();
    sumPt2         += pfcand.pt() * pfcand.pt();
  }
  covMatrix(0,0) /= sumPt2;
  covMatrix(0,1) /= sumPt2;
  covMatrix(1,1) /= sumPt2;
  covMatrix(1,0) = covMatrix(0,1);
  
  ptD /= sqrt(ptD);
  ptD /= sumPt;
  
  double etaW = sqrt(covMatrix(0,0));
  double phiW = sqrt(covMatrix(1,1));
  double jetW = 0.5*(etaW+phiW);
  
  TVectorD eigVals(2);
  eigVals = TMatrixDSymEigen(covMatrix).GetEigenValues();
  double majW = sqrt(fabs(eigVals(0)));
  double minW = sqrt(fabs(eigVals(1)));

  if     (varType==1) { return majW; }
  else if(varType==2) { return minW; }
  else if(varType==3) { return etaW; }
  else if(varType==4) { return phiW; }
  else if(varType==5) { return jetW; }
  return ptD;
}
//--------------------------------------------------------------------------------------------------
bool JetTools::passPFLooseID(const reco::PFJet &jet)
{
  if(jet.energy() == 0) return false;
  if(jet.neutralHadronEnergy() / jet.energy() > 0.99) return false;
  if(jet.neutralEmEnergy() / jet.energy()     > 0.99) return false;
  if(jet.getPFConstituents().size()           < 2)    return false;
  if(fabs(jet.eta())<2.4) {
    if(jet.chargedHadronEnergy() / jet.energy() <= 0)   return false;
    if(jet.chargedEmEnergy() / jet.energy()     > 0.99) return false;
    if(jet.chargedMultiplicity()                < 1)    return false;
  }
  
  return true;
}
//--------------------------------------------------------------------------------------------------
double JetTools::jetCharge(const reco::PFJet &jet)
{
  // Assumes constituents are stored by descending pT
  double charge=0; double lSumPt = 0;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    const reco::TrackRef       track  = pfcand->trackRef();
    if(track.isNull()) continue;
    charge += pfcand->pt()*track->charge();
    lSumPt += pfcand->pt();
  }
  if(lSumPt == 0) lSumPt = 1;
  return charge/lSumPt;
}
//--------------------------------------------------------------------------------------------------
double* JetTools::subJetBTag(const reco::PFJet &jet,reco::JetTagCollection &subJetVal,double iConeSize )
{
  // Following CMS convention first two daughters are leading subjets
  double* vals = new double[2]; 
  int lCount = 0;
  for (unsigned int i = 0; i != subJetVal.size(); ++i) {
    if(lCount > 1) break;
    double pDR = reco::deltaR(subJetVal[i].first->eta(),subJetVal[i].first->phi(),jet.eta(),jet.phi());
    if(pDR  > iConeSize) continue;
    vals[lCount] = subJetVal[i].second;
    lCount++;
  }
  return vals;
}
//--------------------------------------------------------------------------------------------------
double* JetTools::subJetQG(const reco::PFJet &jet,edm::Handle<reco::PFJetCollection> &subJets,const edm::ValueMap<float> iQGLikelihood,double iConeSize)
{
  double* vals = new double[2];
  int lCount = 0;
  for (unsigned int i = 0; i != subJets->size(); ++i) {
    if(lCount > 1) break;
    double pDR = reco::deltaR((*subJets)[i].eta(),(*subJets)[i].phi(),jet.eta(),jet.phi());
    if(pDR  > iConeSize) continue;
    edm::RefToBase<reco::Jet> jetRef(edm::Ref<reco::PFJetCollection>(subJets,i));
    vals[lCount] = (iQGLikelihood)[jetRef];
    if(vals[lCount] == -1) std::cout << "===> " << (*subJets)[i].pt() << " -- " << (*subJets)[i].eta() << std::endl;
    lCount++;
  }
  return vals;
}
//--------------------------------------------------------------------------------------------------
TLorentzVector JetTools::jetPull(const reco::PFJet &jet )
{
  TLorentzVector lPull;
  const unsigned int nPFCands = jet.getPFConstituents().size();
  for(unsigned int ipf=0; ipf<nPFCands; ipf++) {
    const reco::PFCandidatePtr pfcand = jet.getPFConstituents().at(ipf);
    double dEta = pfcand->eta()-jet.eta();
    double dPhi = reco::deltaPhi(pfcand->phi(),jet.phi());
    double dR2  = dEta*dEta + dPhi*dPhi;
    TLorentzVector pVec; pVec.SetPtEtaPhiM((pfcand->pt()/jet.pt()) *dR2 ,dEta,dPhi,0);
    lPull += pVec;
  }
  return lPull;
}
//--------------------------------------------------------------------------------------------------
double JetTools::jetPullAngle(const reco::PFJet &jet ,edm::Handle<reco::PFJetCollection> &subJets,double iConeSize)
{
  // Following CMS convention first two daughters are leading subjets
  int lCount = 0;
  const reco::PFJet *subjet0 = 0;
  const reco::PFJet *subjet1 = 0;
  for(unsigned int i0 = 0; i0 < subJets->size(); i0++) { 
    if(lCount > 1) break;
    double pDR = reco::deltaR((*subJets)[i0].eta(),(*subJets)[i0].phi(),jet.eta(),jet.phi());
    if(pDR  > iConeSize) continue;
    lCount == 0 ? subjet0 = &(*subJets)[i0] : subjet1 = &(*subJets)[i0];
    lCount++;
  }
  if(subjet0 == 0 || subjet1 == 0) return -20;
  TLorentzVector lPull = jetPull(*subjet0);
  TLorentzVector lJet0; lJet0.SetPtEtaPhiM(subjet0->pt(),subjet0->eta(),subjet0->phi(),subjet0->mass());
  TLorentzVector lJet1; lJet1.SetPtEtaPhiM(subjet1->pt(),subjet1->eta(),subjet1->phi(),subjet1->mass());
  //Rotate Jet into subjet0 coordinates                                                                                                                                                                     
  lJet1.RotateZ(-lJet0.Phi());
  lJet1.RotateY(-lJet0.Theta()+TMath::Pi()/2.);
  double lPhi = lJet1.DeltaPhi(lPull);
  return lPhi;
}
//--------------------------------------------------------------------------------------------------
float JetTools::findRMS( std::vector<float> &iQJetMass) {
  float total = 0.;
  float ctr = 0.;
  for (unsigned int i = 0; i < iQJetMass.size(); i++){
    total = total + iQJetMass[i];
    ctr++;
  }
  float mean =  total/ctr;
    
  float totalsquared = 0.;
  for (unsigned int i = 0; i < iQJetMass.size(); i++){
    totalsquared += (iQJetMass[i] - mean)*(iQJetMass[i] - mean) ;
  }
  float RMS = sqrt( totalsquared/ctr );
  return RMS;
}
//--------------------------------------------------------------------------------------------------
float JetTools::findMean( std::vector<float> &iQJetMass ){
  float total = 0.;
  float ctr = 0.;
  for (unsigned int i = 0; i < iQJetMass.size(); i++){
    total = total + iQJetMass[i];
    ctr++;
  }
  return total/ctr;
}
//-------------------------------------------------------------------------------------P-------------
double JetTools::qJetVolatility(std::vector <fastjet::PseudoJet> &iConstits, int iQJetsN, int iSeed){
  std::vector< float > qjetmasses;
  double zcut(0.1), dcut_fctr(0.5), exp_min(0.), exp_max(0.), rigidity(0.1), truncationFactor(0.01);          
  for(unsigned int ii = 0 ; ii < (unsigned int) iQJetsN ; ii++){
    QjetsPlugin qjet_plugin(zcut, dcut_fctr, exp_min, exp_max, rigidity, truncationFactor);
    qjet_plugin.SetRandSeed(iSeed+ii); // new feature in Qjets to set the random seed
    fastjet::JetDefinition qjet_def(&qjet_plugin);
    fastjet::ClusterSequence qjet_seq(iConstits, qjet_def);
    std::vector<fastjet::PseudoJet> inclusive_jets2 = sorted_by_pt(qjet_seq.inclusive_jets(5.0));
    if (inclusive_jets2.size()>0) { qjetmasses.push_back( inclusive_jets2[0].m() ); }
  }
  // find RMS of a vector
  float qjetsRMS = findRMS( qjetmasses );
  // find mean of a vector
  float qjetsMean = findMean( qjetmasses );
  float qjetsVolatility = qjetsRMS/qjetsMean;
  return qjetsVolatility;
}


