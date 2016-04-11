// $Id$
//
// Jet+h correlation task
//
// Author: R. Haake

#include <TClonesArray.h>
#include <TF1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TProfile.h>
#include <TList.h>
#include <TLorentzVector.h>

#include "AliVTrack.h"
#include "AliEmcalJet.h"
#include "AliRhoParameter.h"
#include "AliLog.h"
#include "AliJetContainer.h"
#include "AliTrackContainer.h"
#include "AliPicoTrack.h"
#include "AliVParticle.h"
#include "TRandom3.h"
#include "AliEmcalJetFinder.h"

#include "AliAnalysisTaskChargedJetsHadronCF.h"

ClassImp(AliAnalysisTaskChargedJetsHadronCF)

//________________________________________________________________________
AliAnalysisTaskChargedJetsHadronCF::AliAnalysisTaskChargedJetsHadronCF() : 
  AliAnalysisTaskEmcalJet("AliAnalysisTaskChargedJetsHadronCF", kTRUE),
  fJetsCont(0),
  fTracksCont(0),
  fNumberOfCentralityBins(10),
  fJetsOutput(),
  fTracksOutput(),
  fJetsInput(),
  fJetParticleArrayName("JetsDPhiBasicParticles"),
  fTrackParticleArrayName(""),
  fJetMatchingArrayName(""),
  fRandom(0),
  hFakeFactorCutProfile(0),
  fJetOutputMode(0),
  fUseFakejetRejection(kFALSE),
  fMinFakeFactorPercentage(0),
  fMaxFakeFactorPercentage(0),
  fEventCriteriumMode(0),
  fEventCriteriumMinBackground(0),
  fEventCriteriumMaxBackground(0),
  fEventCriteriumMinLeadingJetPt(0),
  fEventCriteriumMinSubleadingJetPt(0),
  fLeadingJet(),
  fSubleadingJet(),
  fAcceptedJets(0),
  fAcceptedTracks(0)
{
  // Default constructor.
  SetMakeGeneralHistograms(kTRUE);
  fRandom = new TRandom3(0);
}


//________________________________________________________________________
AliAnalysisTaskChargedJetsHadronCF::AliAnalysisTaskChargedJetsHadronCF(const char *name) : 
  AliAnalysisTaskEmcalJet(name, kTRUE),
  fJetsCont(0),
  fTracksCont(0),
  fNumberOfCentralityBins(10),
  fJetsOutput(),
  fTracksOutput(),
  fJetsInput(),
  fJetParticleArrayName("JetsDPhiBasicParticles"),
  fTrackParticleArrayName(""),
  fJetMatchingArrayName(""),
  fRandom(0),
  hFakeFactorCutProfile(0),
  fJetOutputMode(0),
  fUseFakejetRejection(kFALSE),
  fMinFakeFactorPercentage(0),
  fMaxFakeFactorPercentage(0),
  fEventCriteriumMode(0),
  fEventCriteriumMinBackground(0),
  fEventCriteriumMaxBackground(0),
  fEventCriteriumMinLeadingJetPt(0),
  fEventCriteriumMinSubleadingJetPt(0),
  fLeadingJet(),
  fSubleadingJet(),
  fAcceptedJets(0),
  fAcceptedTracks(0)
{
  // Constructor
  SetMakeGeneralHistograms(kTRUE);
  fRandom = new TRandom3(0);
}

//________________________________________________________________________
AliAnalysisTaskChargedJetsHadronCF::~AliAnalysisTaskChargedJetsHadronCF()
{
  // Destructor.
}

//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::UserCreateOutputObjects()
{
  AliAnalysisTaskEmcalJet::UserCreateOutputObjects();

  // ### Basic container settings
  fJetsCont           = GetJetContainer(0);
  if(fJetsCont) { //get particles connected to jets
    fJetsCont->PrintCuts();
    fTracksCont       = static_cast<AliTrackContainer*>(fJetsCont->GetParticleContainer());
  } else {        //no jets, just analysis tracks
    fTracksCont       = static_cast<AliTrackContainer*>(GetParticleContainer(0));
  }
  if(fTracksCont) fTracksCont->SetClassName("AliVTrack");

  // ### Create all histograms

  // Change the event rejection histogram -> Add a custom value
  fHistEventRejection->GetXaxis()->SetBinLabel(14,"JetCrit");

  // Track QA plots
  AddHistogram2D<TH2D>("hTrackPt", "Tracks p_{T} distribution", "", 300, 0., 300., fNumberOfCentralityBins, 0, 100, "p_{T} (GeV/c)", "Centrality", "dN^{Tracks}/dp_{T}");
  AddHistogram2D<TH2D>("hTrackPhi", "Track angular distribution in #phi", "LEGO2", 180, 0., 2*TMath::Pi(), fNumberOfCentralityBins, 0, 100, "#phi", "Centrality", "dN^{Tracks}/(d#phi)");
  AddHistogram2D<TH2D>("hTrackEta", "Track angular distribution in #eta", "LEGO2", 100, -2.5, 2.5, fNumberOfCentralityBins, 0, 100, "#eta", "Centrality", "dN^{Tracks}/(d#eta)");
  AddHistogram2D<TH2D>("hTrackPhiEta", "Track angular distribution #phi/#eta", "COLZ", 180, 0., 2*TMath::Pi(), 100, -2.5, 2.5, "#phi", "#eta", "dN^{Tracks}/d#phi d#eta");

  AddHistogram2D<TH2D>("hLeadingTrackPt", "Leading tracks p_{T} distribution", "", 300, 0., 300., fNumberOfCentralityBins, 0, 100, "p_{T} (GeV/c)", "Centrality", "dN^{Tracks}/dp_{T}");
  AddHistogram2D<TH2D>("hLeadingTrackPhi", "Leading tracks angular distribution in #phi", "LEGO2", 180, 0., 2*TMath::Pi(), fNumberOfCentralityBins, 0, 100, "#phi", "Centrality", "dN^{Tracks}/(d#phi)");
  AddHistogram2D<TH2D>("hLeadingTrackEta", "Leading tracks angular distribution in #eta", "LEGO2", 100, -2.5, 2.5, fNumberOfCentralityBins, 0, 100, "#eta", "Centrality", "dN^{Tracks}/(d#eta)");
  AddHistogram2D<TH2D>("hLeadingTrackPhiEta", "Track angular distribution #phi/#eta", "COLZ", 180, 0., 2*TMath::Pi(), 100, -2.5, 2.5, "#phi", "#eta", "dN^{Tracks}/d#phi d#eta");

  AddHistogram2D<TH2D>("hTrackEtaPt", "Track angular distribution in #eta vs. p_{T}", "LEGO2", 100, -2.5, 2.5, 300, 0., 300., "#eta", "p_{T} (GeV/c)", "dN^{Tracks}/(d#eta dp_{T})");
  AddHistogram2D<TH2D>("hTrackPhiPt", "Track angular distribution in #phi vs. p_{T}", "LEGO2", 180, 0, 2*TMath::Pi(), 300, 0., 300., "#phi", "p_{T} (GeV/c)", "dN^{Tracks}/(d#phi dp_{T})");


  // Jet QA plots
  AddHistogram2D<TH2D>("hJetPtRaw", "Jets p_{T} distribution (no bgrd. corr.)", "", 300, 0., 300., fNumberOfCentralityBins, 0, 100, "p_{T, jet} (GeV/c)", "Centrality", "dN^{Jets}/dp_{T}");
  AddHistogram2D<TH2D>("hJetPt", "Jets p_{T} distribution (background subtracted)", "", 400, -100., 300., fNumberOfCentralityBins, 0, 100, "p_{T, jet} (GeV/c)", "Centrality", "dN^{Jets}/dp_{T}");
  AddHistogram2D<TH2D>("hJetPhi", "Jet angular distribution #phi", "LEGO2", 180, 0., 2*TMath::Pi(), fNumberOfCentralityBins, 0, 100, "#phi", "Centrality", "dN^{Jets}/d#phi");
  AddHistogram2D<TH2D>("hJetEta", "Jet angular distribution #eta", "LEGO2", 100, -2.5, 2.5, fNumberOfCentralityBins, 0, 100, "#eta","Centrality","dN^{Jets}/d#eta");
  AddHistogram2D<TH2D>("hJetPhiPt", "Jet angular distribution #phi vs. p_{T}", "LEGO2", 180, 0., 2*TMath::Pi(), 400, -100., 300., "#phi", "p_{T, jet} (GeV/c)", "dN^{Jets}/d#phi dp_{T}");
  AddHistogram2D<TH2D>("hJetEtaPt", "Jet angular distribution #eta  vs. p_{T}", "LEGO2", 100, -2.5, 2.5, 400, -100., 300., "#eta","p_{T, jet} (GeV/c)","dN^{Jets}/d#eta dp_{T}");
  AddHistogram2D<TH2D>("hJetPhiEta", "Jet angular distribution #phi/#eta", "COLZ", 180, 0., 2*TMath::Pi(), 100, -2.5, 2.5, "#phi", "#eta", "dN^{Jets}/d#phi d#eta");
  AddHistogram2D<TH2D>("hJetArea", "Jet area", "LEGO2", 200, 0., 2., fNumberOfCentralityBins, 0, 100, "Jet A", "Centrality", "dN^{Jets}/dA");
  AddHistogram2D<TH2D>("hJetAreaPt", "Jet area vs. p_{T}", "LEGO2", 200, 0., 2., 400, -100., 300., "Jet A", "p_{T, jet} (GeV/c)", "dN^{Jets}/dA dp_{T}");
  AddHistogram2D<TH2D>("hJetPtLeadingHadron", "Jet leading hadron p_{T} distribution vs. jet p_{T}", "", 300, 0., 300., 300, 0., 300., "p_{T, jet} (GeV/c)", "p_{T,lead had} (GeV/c)", "dN^{Jets}/dp_{T}dp_{T,had}");

  AddHistogram2D<TH2D>("hLeadingJetPtRaw", "Jets p_{T} distribution (no bgrd. corr.)", "", 300, 0., 300., fNumberOfCentralityBins, 0, 100, "p_{T, jet} (GeV/c)", "Centrality", "dN^{Jets}/dp_{T}");
  AddHistogram2D<TH2D>("hLeadingJetPt", "Jets p_{T} distribution (background subtracted)", "", 400, -100., 300., fNumberOfCentralityBins, 0, 100, "p_{T, jet} (GeV/c)", "Centrality", "dN^{Jets}/dp_{T}");
  AddHistogram2D<TH2D>("hLeadingJetPhi", "Jet angular distribution #phi", "LEGO2", 180, 0., 2*TMath::Pi(), fNumberOfCentralityBins, 0, 100, "#phi", "Centrality", "dN^{Jets}/d#phi");
  AddHistogram2D<TH2D>("hLeadingJetEta", "Jet angular distribution #eta", "LEGO2", 100, -2.5, 2.5, fNumberOfCentralityBins, 0, 100, "#eta","Centrality","dN^{Jets}/d#eta");
  AddHistogram2D<TH2D>("hLeadingJetPhiPt", "Jet angular distribution #phi vs. p_{T}", "LEGO2", 180, 0., 2*TMath::Pi(), 400, -100., 300., "#phi", "p_{T, jet} (GeV/c)", "dN^{Jets}/d#phi dp_{T}");
  AddHistogram2D<TH2D>("hLeadingJetEtaPt", "Jet angular distribution #eta  vs. p_{T}", "LEGO2", 100, -2.5, 2.5, 400, -100., 300., "#eta","p_{T, jet} (GeV/c)","dN^{Jets}/d#eta dp_{T}");
  AddHistogram2D<TH2D>("hLeadingJetPhiEta", "Jet angular distribution #phi/#eta", "COLZ", 180, 0., 2*TMath::Pi(), 100, -2.5, 2.5, "#phi", "#eta", "dN^{Jets}/d#phi d#eta");
  AddHistogram2D<TH2D>("hLeadingJetArea", "Jet area", "LEGO2", 200, 0., 2., fNumberOfCentralityBins, 0, 100, "Jet A", "Centrality", "dN^{Jets}/dA");
  AddHistogram2D<TH2D>("hLeadingJetAreaPt", "Jet area vs. p_{T}", "LEGO2", 200, 0., 2., 400, -100., 300., "Jet A", "p_{T, jet} (GeV/c)", "dN^{Jets}/dA dp_{T}");
  AddHistogram2D<TH2D>("hLeadingJetPtLeadingHadron", "Jet leading hadron p_{T} distribution vs. jet p_{T}", "", 300, 0., 300., 300, 0., 300., "p_{T, jet} (GeV/c)", "p_{T,lead had} (GeV/c)", "dN^{Jets}/dp_{T}dp_{T,had}");

  AddHistogram2D<TH2D>("hSubleadingJetPtRaw", "Jets p_{T} distribution (no bgrd. corr.)", "", 300, 0., 300., fNumberOfCentralityBins, 0, 100, "p_{T, jet} (GeV/c)", "Centrality", "dN^{Jets}/dp_{T}");
  AddHistogram2D<TH2D>("hSubleadingJetPt", "Jets p_{T} distribution (background subtracted)", "", 400, -100., 300., fNumberOfCentralityBins, 0, 100, "p_{T, jet} (GeV/c)", "Centrality", "dN^{Jets}/dp_{T}");
  AddHistogram2D<TH2D>("hSubleadingJetPhi", "Jet angular distribution #phi", "LEGO2", 180, 0., 2*TMath::Pi(), fNumberOfCentralityBins, 0, 100, "#phi", "Centrality", "dN^{Jets}/d#phi");
  AddHistogram2D<TH2D>("hSubleadingJetEta", "Jet angular distribution #eta", "LEGO2", 100, -2.5, 2.5, fNumberOfCentralityBins, 0, 100, "#eta","Centrality","dN^{Jets}/d#eta");
  AddHistogram2D<TH2D>("hSubleadingJetPhiPt", "Jet angular distribution #phi vs. p_{T}", "LEGO2", 180, 0., 2*TMath::Pi(), 400, -100., 300., "#phi", "p_{T, jet} (GeV/c)", "dN^{Jets}/d#phi dp_{T}");
  AddHistogram2D<TH2D>("hSubleadingJetEtaPt", "Jet angular distribution #eta  vs. p_{T}", "LEGO2", 100, -2.5, 2.5, 400, -100., 300., "#eta","p_{T, jet} (GeV/c)","dN^{Jets}/d#eta dp_{T}");
  AddHistogram2D<TH2D>("hSubleadingJetPhiEta", "Jet angular distribution #phi/#eta", "COLZ", 180, 0., 2*TMath::Pi(), 100, -2.5, 2.5, "#phi", "#eta", "dN^{Jets}/d#phi d#eta");
  AddHistogram2D<TH2D>("hSubleadingJetArea", "Jet area", "LEGO2", 200, 0., 2., fNumberOfCentralityBins, 0, 100, "Jet A", "Centrality", "dN^{Jets}/dA");
  AddHistogram2D<TH2D>("hSubleadingJetAreaPt", "Jet area vs. p_{T}", "LEGO2", 200, 0., 2., 400, -100., 300., "Jet A", "p_{T, jet} (GeV/c)", "dN^{Jets}/dA dp_{T}");
  AddHistogram2D<TH2D>("hSubleadingJetPtLeadingHadron", "Jet leading hadron p_{T} distribution vs. jet p_{T}", "", 300, 0., 300., 300, 0., 300., "p_{T, jet} (GeV/c)", "p_{T,lead had} (GeV/c)", "dN^{Jets}/dp_{T}dp_{T,had}");

  AddHistogram2D<TH2D>("hTrackCount", "Number of tracks in acceptance vs. centrality", "LEGO2", 500, 0., 5000., fNumberOfCentralityBins, 0, 100, "N tracks","Centrality", "dN^{Events}/dN^{Tracks}");
  AddHistogram2D<TH2D>("hJetCount", "Number of jets in acceptance vs. centrality", "LEGO2", 100, 0., 100., fNumberOfCentralityBins, 0, 100, "N Jets","Centrality", "dN^{Events}/dN^{Jets}");
  AddHistogram2D<TH2D>("hFakeFactor", "Fake factor distribution", "LEGO2", 1000, 0., 100., fNumberOfCentralityBins, 0, 100, "Fake factor","Centrality", "dN^{Jets}/df");
  AddHistogram2D<TH2D>("hBackgroundPt", "Background p_{T} distribution", "", 1000, 0., 50., fNumberOfCentralityBins, 0, 100, "Background p_{T} (GeV/c)", "Centrality", "dN^{Events}/dp_{T}");

  PostData(1, fOutput); // Post data for ALL output slots > 0 here.
}


//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::ExecOnce() {

  AliAnalysisTaskEmcalJet::ExecOnce();

  // ### Add the jets as basic correlation particles to the event
  if (!(fInputEvent->FindListObject(Form("%s", fJetParticleArrayName.Data()))))
  {
    fJetsOutput = new TClonesArray("AliPicoTrack");
    fJetsOutput->SetName(fJetParticleArrayName.Data());
    fInputEvent->AddObject(fJetsOutput);
  }
  else
    AliError(Form("%s: Object with name %s already in event!", GetName(), Form("%s", fJetParticleArrayName.Data())));

  // ### Add the tracks as basic correlation particles to the event (optional)
  if(fTrackParticleArrayName != "")
  {
    if (!(fInputEvent->FindListObject(Form("%s", fTrackParticleArrayName.Data()))))
    {
      fTracksOutput = new TClonesArray("AliPicoTrack");
      fTracksOutput->SetName(fTrackParticleArrayName.Data());
      fInputEvent->AddObject(fTracksOutput);
    }
    else
      AliError(Form("%s: Object with name %s already in event!", GetName(), Form("%s", fTrackParticleArrayName.Data())));
  }

  // ### Import generated jets from toymodel for matching (optional)
  if(fJetMatchingArrayName != "")
  {
    fJetsInput = static_cast<TClonesArray*>(InputEvent()->FindListObject(Form("%s", fJetMatchingArrayName.Data())));
    if(!fJetsInput)
      AliFatal(Form("Importing jets for matching failed! Array '%s' not found!", fJetMatchingArrayName.Data()));
  }

}

//________________________________________________________________________
Bool_t AliAnalysisTaskChargedJetsHadronCF::IsEventSelected()
{

    // In case of special selection criteria, trigger on certain events
    if(fEventCriteriumMode==0) // "minimum bias"
    {
      // do nothing
    }
    else if(fEventCriteriumMode==1) // background constraints
    {
      if( (fJetsCont->GetRhoVal() < fEventCriteriumMinBackground) || (fJetsCont->GetRhoVal() > fEventCriteriumMaxBackground) )
      {
        fHistEventRejection->Fill("JetCrit", 1);
        return kFALSE;
      }
    }
    else if(fEventCriteriumMode==2) // Minimum leading jet pT
    {
      if(fLeadingJet)
      {
        if(fLeadingJet->Pt() - fJetsCont->GetRhoVal()*fLeadingJet->Area() <= fEventCriteriumMinLeadingJetPt)
        {
          fHistEventRejection->Fill("JetCrit", 1);
          return kFALSE;
        }
      }
    }
    else if(fEventCriteriumMode==3) // Simple dijet trigger
    {
      if(fLeadingJet && fSubleadingJet)
      {
        if((fLeadingJet->Pt() - fJetsCont->GetRhoVal()*fLeadingJet->Area() <= fEventCriteriumMinLeadingJetPt) || (fSubleadingJet->Pt() - fJetsCont->GetRhoVal()*fSubleadingJet->Area() <= fEventCriteriumMinSubleadingJetPt))
        {
          fHistEventRejection->Fill("JetCrit", 1);
          return kFALSE;
      }
      }
    }
  return kTRUE;
}

//________________________________________________________________________
Bool_t AliAnalysisTaskChargedJetsHadronCF::IsJetSelected(AliEmcalJet* jet)
{
  if( (fJetOutputMode==1) || (fJetOutputMode==3) ) // output the leading jet
    if(jet!=fLeadingJet)
      return kFALSE;

  if( (fJetOutputMode==2) || (fJetOutputMode==3) ) // output the subleading jet
    if(jet!=fSubleadingJet)
      return kFALSE;

  // Fake jet rejection (0810.1219)
  if(fUseFakejetRejection)
  {
    Double_t fakeFactor = CalculateFakeFactor(jet);
    FillHistogram("hFakeFactor", fakeFactor, fCent);
    if( (fakeFactor >= fMinFakeFactorPercentage*hFakeFactorCutProfile->GetBinContent(hFakeFactorCutProfile->GetXaxis()->FindBin(fCent))) && (fakeFactor < fMaxFakeFactorPercentage*hFakeFactorCutProfile->GetBinContent(hFakeFactorCutProfile->GetXaxis()->FindBin(fCent))) )
      return kFALSE;
  }

  // Jet matching. Only done if SetJetMatchingArrayName() called
  Bool_t matchedFound = kFALSE;
  if(fJetsInput)
  {
    // Go through all jets and check if the matching condition is fulfiled by at least one jet
    Double_t bestMatchDeltaR = 999.;
    for(Int_t i=0; i<fJetsInput->GetEntries(); i++)
    {
      AliEmcalJet* matchJet = static_cast<AliEmcalJet*>(fJetsInput->At(i));
      Double_t deltaPhi = TMath::Min(TMath::Abs(jet->Phi()-matchJet->Phi()),TMath::TwoPi() - TMath::Abs(jet->Phi()-matchJet->Phi()));
      Double_t deltaEta = TMath::Abs(jet->Eta() - matchJet->Eta());
      Double_t deltaR   = TMath::Sqrt((deltaPhi*deltaPhi) + (deltaEta*deltaEta));

      if(deltaR < bestMatchDeltaR)
      {
        bestMatchDeltaR = deltaR;
      }
    }
    // Check if a matching jet is found.
    if(bestMatchDeltaR < 0.9*fJetsCont->GetJetRadius())
      matchedFound = kTRUE;
  }

  if(fJetOutputMode==4) // matching jets only
    return matchedFound;
  else if(fJetOutputMode==5) // non-matching jets only
    return !matchedFound;

  return kTRUE;
}

//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::FillHistogramsJets(AliEmcalJet* jet)
{
    // All jets
    FillHistogram("hJetPtRaw", jet->Pt(), fCent); 
    FillHistogram("hJetPt", jet->Pt() - fJetsCont->GetRhoVal()*jet->Area(), fCent); 
    FillHistogram("hJetPhi", jet->Phi(), fCent); 
    FillHistogram("hJetEta", jet->Eta(), fCent); 
    FillHistogram("hJetEtaPt", jet->Eta(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
    FillHistogram("hJetPhiPt", jet->Phi(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
    FillHistogram("hJetPhiEta", jet->Phi(), jet->Eta()); 
    FillHistogram("hJetArea", jet->Area(), fCent); 
    FillHistogram("hJetAreaPt", jet->Area(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
    FillHistogram("hJetPtLeadingHadron", jet->Pt() - fJetsCont->GetRhoVal()*jet->Area(), fJetsCont->GetLeadingHadronPt(jet));

    // Leading jet plots
    if(jet==fLeadingJet)
    {
      FillHistogram("hLeadingJetPtRaw", jet->Pt(), fCent); 
      FillHistogram("hLeadingJetPt", jet->Pt() - fJetsCont->GetRhoVal()*jet->Area(), fCent); 
      FillHistogram("hLeadingJetPhi", jet->Phi(), fCent); 
      FillHistogram("hLeadingJetEta", jet->Eta(), fCent); 
      FillHistogram("hLeadingJetEtaPt", jet->Eta(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
      FillHistogram("hLeadingJetPhiPt", jet->Phi(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
      FillHistogram("hLeadingJetPhiEta", jet->Phi(), jet->Eta()); 
      FillHistogram("hLeadingJetArea", jet->Area(), fCent); 
      FillHistogram("hLeadingJetAreaPt", jet->Area(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
      FillHistogram("hLeadingJetPtLeadingHadron", jet->Pt() - fJetsCont->GetRhoVal()*jet->Area(), fJetsCont->GetLeadingHadronPt(jet));
    }

    // Subleading jet plot
    else if(jet==fSubleadingJet)
    {
      FillHistogram("hSubleadingJetPtRaw", jet->Pt(), fCent); 
      FillHistogram("hSubleadingJetPt", jet->Pt() - fJetsCont->GetRhoVal()*jet->Area(), fCent); 
      FillHistogram("hSubleadingJetPhi", jet->Phi(), fCent); 
      FillHistogram("hSubleadingJetEta", jet->Eta(), fCent); 
      FillHistogram("hSubleadingJetEtaPt", jet->Eta(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
      FillHistogram("hSubleadingJetPhiPt", jet->Phi(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
      FillHistogram("hSubleadingJetPhiEta", jet->Phi(), jet->Eta());
      FillHistogram("hSubleadingJetArea", jet->Area(), fCent); 
      FillHistogram("hSubleadingJetAreaPt", jet->Area(), jet->Pt() - fJetsCont->GetRhoVal()*jet->Area()); 
      FillHistogram("hSubleadingJetPtLeadingHadron", jet->Pt() - fJetsCont->GetRhoVal()*jet->Area(), fJetsCont->GetLeadingHadronPt(jet));
    }
}

//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::FillHistogramsTracks(AliVTrack* track)
{
  FillHistogram("hTrackPt", track->Pt(), fCent); 
  FillHistogram("hTrackPhi", track->Phi(), fCent); 
  FillHistogram("hTrackEta", track->Eta(), fCent); 
  FillHistogram("hTrackEtaPt", track->Eta(), track->Pt()); 
  FillHistogram("hTrackPhiPt", track->Phi(), track->Pt()); 
  FillHistogram("hTrackPhiEta", track->Phi(), track->Eta()); 
}

//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::AddJetToOutputArray(AliEmcalJet* jet)
{
  new ((*fJetsOutput)[fAcceptedJets]) AliPicoTrack(jet->Pt() - fJetsCont->GetRhoVal()*jet->Area(), jet->Eta(), jet->Phi(), jet->Charge(), 0, 0);
  fAcceptedJets++;
}

//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::AddTrackToOutputArray(AliVTrack* track)
{
  if(fTrackParticleArrayName != "")
  {
    new ((*fTracksOutput)[fAcceptedTracks]) AliPicoTrack(track->Pt(), track->Eta(), track->Phi(), track->Charge(), 0, 0); // only Pt,Eta,Phi are interesting for correlations;
    fAcceptedTracks++;
  }
}

//________________________________________________________________________
Bool_t AliAnalysisTaskChargedJetsHadronCF::Run()
{
  CalculateEventProperties();

  if(!IsEventSelected())
    return kFALSE;

  // ####### Jet loop
  fAcceptedJets = 0;
  fJetsCont->ResetCurrentID();
  while(AliEmcalJet *jet = fJetsCont->GetNextAcceptJet())
  {
    if(!IsJetSelected(jet))
      continue;

    // Jet plots
    FillHistogramsJets(jet);

    // Add jet to output array
    AddJetToOutputArray(jet);
  }


  // ####### Particle loop
  fAcceptedTracks = 0;
  fTracksCont->ResetCurrentID();
  while(AliVTrack *track = static_cast<AliVTrack*>(fTracksCont->GetNextAcceptParticle()))
  {
    // Track plots
    FillHistogramsTracks(track);

    // Add track to output array
    AddTrackToOutputArray(track);
  }

  // ####### Event properties
  FillHistogram("hJetCount", fAcceptedJets, fCent);
  FillHistogram("hTrackCount", fAcceptedTracks, fCent);
  // NOTE: It is possible to use fTracksCont->GetLeadingParticle() since we do not apply additional track cuts
  AliVTrack* leadTrack = static_cast<AliVTrack*>(fTracksCont->GetLeadingParticle());
  if(leadTrack)
  {
    FillHistogram("hLeadingTrackPt", leadTrack->Pt(), fCent); 
    FillHistogram("hLeadingTrackPhi", leadTrack->Phi(), fCent); 
    FillHistogram("hLeadingTrackEta", leadTrack->Eta(), fCent); 
    FillHistogram("hLeadingTrackPhiEta", leadTrack->Phi(), leadTrack->Eta()); 
  }

  return kTRUE;
}

//########################################################################
// HELPERS
//########################################################################

//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::CalculateEventProperties()
{
  // Calculate leading + subleading jet
  fLeadingJet    = fJetsCont->GetLeadingJet("rho");
  fSubleadingJet = GetSubleadingJet("rho");
}

//________________________________________________________________________
Double_t AliAnalysisTaskChargedJetsHadronCF::CalculateFakeFactor(AliEmcalJet* jet)
{
  Double_t fakeFactor = 0;

  // Loop over all jet constituents
  for(Int_t i = 0; i < jet->GetNumberOfTracks(); i++)
  {
    AliVParticle* constituent = static_cast<AliVParticle*>(jet->TrackAt(i, fTracksCont->GetArray()));

    Double_t deltaPhi = TMath::Min(TMath::Abs(jet->Phi()-constituent->Phi()),TMath::TwoPi() - TMath::Abs(jet->Phi()-constituent->Phi()));
    Double_t deltaR = TMath::Sqrt( (jet->Eta() - constituent->Eta())*(jet->Eta() - constituent->Eta()) + deltaPhi*deltaPhi );
    fakeFactor += constituent->Pt() * TMath::Sin(deltaR);
  }

  return fakeFactor;
}

//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::SetEventCriteriumSelection(Int_t type)
{
  fEventCriteriumMode = type;

  if(fEventCriteriumMode==0)
    AliWarning("Set event criterium to 'default'              -- no further selection criterium.");
  else if(fEventCriteriumMode==1)
    AliWarning("Set event criterium to 'background'           -- select events with certain backgrounds");
  else if(fEventCriteriumMode==2)
    AliWarning("Set event criterium to 'simple jet trigger'   -- select events with certain minimum leading jet pT (bgrd corr.)");
  else if(fEventCriteriumMode==3)
    AliWarning("Set event criterium to 'simple dijet trigger' -- select events with certain minimum leading + subleading jet pT (bgrd corr.)");
  else
  {
    AliFatal("Event criterium not valid.");
  }
}


//________________________________________________________________________
AliEmcalJet* AliAnalysisTaskChargedJetsHadronCF::GetSubleadingJet(const char* opt)
{
  // Customized from AliJetContainer::GetLeadingJet()
  // Get the subleading jet; if opt contains "rho" the sorting is according to pt-A*rho

  TString option(opt);
  option.ToLower();

  AliEmcalJet *jetLeading = fJetsCont->GetLeadingJet(opt);
  AliEmcalJet *jetSubLeading = 0;

  fJetsCont->ResetCurrentID();
  AliEmcalJet *jet = fJetsCont->GetNextAcceptJet();
  Double_t     tmpPt = 0;

  if (option.Contains("rho")) {
    while ((jet = fJetsCont->GetNextAcceptJet())) {
      if(jet == jetLeading)
        continue;
      else if ( (jet->Pt()-jet->Area()*fJetsCont->GetRhoVal()) > tmpPt )
      {
        jetSubLeading = jet;
        tmpPt = jet->Pt()-jet->Area()*fJetsCont->GetRhoVal();
      }

    }
  }
  else {
    while ((jet = fJetsCont->GetNextAcceptJet())) {
      if(jet == jetLeading)
        continue;
      else if ( jet->Pt() > tmpPt )
      {
        jetSubLeading = jet;
        tmpPt = jet->Pt();
      }
    }
  }

  return jetSubLeading;
}

//________________________________________________________________________
inline void AliAnalysisTaskChargedJetsHadronCF::FillHistogram(const char * key, Double_t x)
{
  TH1* tmpHist = static_cast<TH1*>(fOutput->FindObject(key));
  if(!tmpHist)
  {
    AliError(Form("Cannot find histogram <%s> ",key)) ;
    return;
  }

  tmpHist->Fill(x);
}

//________________________________________________________________________
inline void AliAnalysisTaskChargedJetsHadronCF::FillHistogram(const char * key, Double_t x, Double_t y)
{
  TH1* tmpHist = static_cast<TH1*>(fOutput->FindObject(key));
  if(!tmpHist)
  {
    AliError(Form("Cannot find histogram <%s> ",key));
    return;
  }

  if (tmpHist->IsA()->GetBaseClass("TH1"))
    static_cast<TH1*>(tmpHist)->Fill(x,y); // Fill x with y
  else if (tmpHist->IsA()->GetBaseClass("TH2"))
    static_cast<TH2*>(tmpHist)->Fill(x,y); // Fill x,y with 1
}

//________________________________________________________________________
inline void AliAnalysisTaskChargedJetsHadronCF::FillHistogram(const char * key, Double_t x, Double_t y, Double_t add)
{
  TH2* tmpHist = static_cast<TH2*>(fOutput->FindObject(key));
  if(!tmpHist)
  {
    AliError(Form("Cannot find histogram <%s> ",key));
    return;
  }
  
  tmpHist->Fill(x,y,add);
}

//________________________________________________________________________
template <class T> T* AliAnalysisTaskChargedJetsHadronCF::AddHistogram1D(const char* name, const char* title, const char* options, Int_t xBins, Double_t xMin, Double_t xMax, const char* xTitle, const char* yTitle)
{
  T* tmpHist = new T(name, title, xBins, xMin, xMax);

  tmpHist->GetXaxis()->SetTitle(xTitle);
  tmpHist->GetYaxis()->SetTitle(yTitle);
  tmpHist->SetOption(options);
  tmpHist->SetMarkerStyle(kFullCircle);
  tmpHist->Sumw2();

  fOutput->Add(tmpHist);

  return tmpHist;
}

//________________________________________________________________________
template <class T> T* AliAnalysisTaskChargedJetsHadronCF::AddHistogram2D(const char* name, const char* title, const char* options, Int_t xBins, Double_t xMin, Double_t xMax, Int_t yBins, Double_t yMin, Double_t yMax, const char* xTitle, const char* yTitle, const char* zTitle)
{
  T* tmpHist = new T(name, title, xBins, xMin, xMax, yBins, yMin, yMax);
  tmpHist->GetXaxis()->SetTitle(xTitle);
  tmpHist->GetYaxis()->SetTitle(yTitle);
  tmpHist->GetZaxis()->SetTitle(zTitle);
  tmpHist->SetOption(options);
  tmpHist->SetMarkerStyle(kFullCircle);
  tmpHist->Sumw2();

  fOutput->Add(tmpHist);

  return tmpHist;
}

//________________________________________________________________________
void AliAnalysisTaskChargedJetsHadronCF::Terminate(Option_t *) 
{
  // Called once at the end of the analysis.
}

