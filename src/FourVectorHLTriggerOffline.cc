// $Id: FourVectorHLTriggerOffline.cc,v 1.25 2009/06/04 22:47:48 rekovic Exp $
// See header file for information. 
#include "TMath.h"
#include "HLTriggerOffline/Common/interface/FourVectorHLTriggerOffline.h"



/*
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "FWCore/Framework/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HLTReco/interface/TriggerTypeDefs.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackExtra.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/TauReco/interface/CaloTauFwd.h"
#include "DataFormats/TauReco/interface/CaloTau.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/BTauReco/interface/JetTag.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/METReco/interface/GenMETCollection.h"
#include "DataFormats/METReco/interface/GenMET.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "DQMServices/Core/interface/MonitorElement.h"
*/




#include <map>
#include <utility>


using namespace edm;
using namespace trigger;

FourVectorHLTriggerOffline::FourVectorHLTriggerOffline(const edm::ParameterSet& iConfig):
  resetMe_(true),  currentRun_(-99)
{
  LogDebug("FourVectorHLTriggerOffline") << "constructor...." ;

  dbe_ = Service < DQMStore > ().operator->();
  if ( ! dbe_ ) {
    LogInfo("FourVectorHLTriggerOffline") << "unabel to get DQMStore service?";
  }
  if (iConfig.getUntrackedParameter < bool > ("DQMStore", false)) {
    dbe_->setVerbose(0);
  }
  
  dirname_ = iConfig.getUntrackedParameter("dirname",
					   std::string("HLT/FourVector/"));
  //dirname_ +=  iConfig.getParameter<std::string>("@module_label");
  
  if (dbe_ != 0 ) {
    dbe_->setCurrentFolder(dirname_);
  }
  
  processname_ = iConfig.getParameter<std::string>("processname");

  // plotting paramters
  ptMin_ = iConfig.getUntrackedParameter<double>("ptMin",0.);
  ptMax_ = iConfig.getUntrackedParameter<double>("ptMax",1000.);
  nBins_ = iConfig.getUntrackedParameter<unsigned int>("Nbins",20);
  
  plotAll_ = iConfig.getUntrackedParameter<bool>("plotAll", false);
     // this is the list of paths to look at.
     std::vector<edm::ParameterSet> paths = 
     iConfig.getParameter<std::vector<edm::ParameterSet> >("paths");
     for(std::vector<edm::ParameterSet>::iterator 
	pathconf = paths.begin() ; pathconf != paths.end(); 
      pathconf++) {
       std::pair<std::string, std::string> custompathnamepair;
       custompathnamepair.first =pathconf->getParameter<std::string>("pathname"); 
       custompathnamepair.second = pathconf->getParameter<std::string>("denompathname");   
       custompathnamepairs_.push_back(custompathnamepair);
       //    customdenompathnames_.push_back(pathconf->getParameter<std::string>("denompathname"));  
       // custompathnames_.push_back(pathconf->getParameter<std::string>("pathname"));  
    }

  if (hltPaths_.size() > 0)
    {
      // book a histogram of scalers
     scalersSelect = dbe_->book1D("selectedScalers","Selected Scalers", hltPaths_.size(), 0.0, (double)hltPaths_.size());
    }

 
  triggerSummaryLabel_ = 
    iConfig.getParameter<edm::InputTag>("triggerSummaryLabel");
  triggerResultsLabel_ = 
    iConfig.getParameter<edm::InputTag>("triggerResultsLabel");

  electronEtaMax_ = iConfig.getUntrackedParameter<double>("electronEtaMax",2.5);
  electronEtMin_ = iConfig.getUntrackedParameter<double>("electronEtMin",3.0);
  electronDRMatch_  =iConfig.getUntrackedParameter<double>("electronDRMatch",0.3); 

  muonEtaMax_ = iConfig.getUntrackedParameter<double>("muonEtaMax",2.1);
  muonEtMin_ = iConfig.getUntrackedParameter<double>("muonEtMin",3.0);
  muonDRMatch_  =iConfig.getUntrackedParameter<double>("muonDRMatch",0.3); 

  tauEtaMax_ = iConfig.getUntrackedParameter<double>("tauEtaMax",2.5);
  tauEtMin_ = iConfig.getUntrackedParameter<double>("tauEtMin",3.0);
  tauDRMatch_  =iConfig.getUntrackedParameter<double>("tauDRMatch",0.3); 

  jetEtaMax_ = iConfig.getUntrackedParameter<double>("jetEtaMax",5.0);
  jetEtMin_ = iConfig.getUntrackedParameter<double>("jetEtMin",10.0);
  jetDRMatch_  =iConfig.getUntrackedParameter<double>("jetDRMatch",0.3); 

  bjetEtaMax_ = iConfig.getUntrackedParameter<double>("bjetEtaMax",2.5);
  bjetEtMin_ = iConfig.getUntrackedParameter<double>("bjetEtMin",10.0);
  bjetDRMatch_  =iConfig.getUntrackedParameter<double>("bjetDRMatch",0.3); 

  photonEtaMax_ = iConfig.getUntrackedParameter<double>("photonEtaMax",2.5);
  photonEtMin_ = iConfig.getUntrackedParameter<double>("photonEtMin",3.0);
  photonDRMatch_  =iConfig.getUntrackedParameter<double>("photonDRMatch",0.3); 

  trackEtaMax_ = iConfig.getUntrackedParameter<double>("trackEtaMax",2.5);
  trackEtMin_ = iConfig.getUntrackedParameter<double>("trackEtMin",3.0);
  trackDRMatch_  =iConfig.getUntrackedParameter<double>("trackDRMatch",0.3); 

  metMin_ = iConfig.getUntrackedParameter<double>("metMin",10.0);
  htMin_ = iConfig.getUntrackedParameter<double>("htMin",10.0);
  sumEtMin_ = iConfig.getUntrackedParameter<double>("sumEtMin",10.0);
  
}


FourVectorHLTriggerOffline::~FourVectorHLTriggerOffline()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to for each event  ------------
void
FourVectorHLTriggerOffline::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace trigger;
  ++nev_;
  LogDebug("FourVectorHLTriggerOffline")<< " analyze...." ;
  
  Handle<GenParticleCollection> genParticles;
  iEvent.getByLabel("genParticles", genParticles);
  if(!genParticles.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "genParticles not found, "
      "skipping event"; 
    return;
  }

  Handle<GenJetCollection> genJets;
  iEvent.getByLabel("iterativeCone5GenJets",genJets);
  if(!genJets.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "genJets not found, "
      "skipping event"; 
    return;
  }

  Handle<GenMETCollection> genMets;
  iEvent.getByLabel("genMetTrue",genMets);
  if(!genMets.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "genMets not found, "
      "skipping event"; 
    return;
  }

  edm::Handle<TriggerResults> triggerResults;
  iEvent.getByLabel(triggerResultsLabel_,triggerResults);
  if(!triggerResults.isValid()) {
    edm::InputTag triggerResultsLabelFU(triggerResultsLabel_.label(),triggerResultsLabel_.instance(), "FU");
   iEvent.getByLabel(triggerResultsLabelFU,triggerResults);
  if(!triggerResults.isValid()) {
    edm::LogInfo("FourVectorHLTriggerOffline") << "TriggerResults not found, "
      "skipping event"; 
    return;
   }
  }
  TriggerNames triggerNames(*triggerResults);  
  int npath = triggerResults->size();

  edm::Handle<TriggerEvent> triggerObj;
  iEvent.getByLabel(triggerSummaryLabel_,triggerObj); 
  if(!triggerObj.isValid()) {
    edm::InputTag triggerSummaryLabelFU(triggerSummaryLabel_.label(),triggerSummaryLabel_.instance(), "FU");
   iEvent.getByLabel(triggerSummaryLabelFU,triggerObj);
  if(!triggerObj.isValid()) {
    edm::LogInfo("FourVectorHLTriggerOffline") << "TriggerEvent not found, "
      "skipping event"; 
    return;
   }
  }

  edm::Handle<reco::MuonCollection> muonHandle;
  iEvent.getByLabel("muons",muonHandle);
  if(!muonHandle.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "muonHandle not found, ";
    //  "skipping event"; 
    //  return;
   }

  edm::Handle<reco::GsfElectronCollection> gsfElectrons;
  iEvent.getByLabel("gsfElectrons",gsfElectrons); 
  if(!gsfElectrons.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "gsfElectrons not found, ";
      //"skipping event"; 
      //return;
  }

  edm::Handle<reco::CaloTauCollection> tauHandle;
  iEvent.getByLabel("caloRecoTauProducer",tauHandle);
  if(!tauHandle.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "tauHandle not found, ";
      //"skipping event"; 
      //return;
  }

  edm::Handle<reco::CaloJetCollection> jetHandle;
  iEvent.getByLabel("iterativeCone5CaloJets",jetHandle);
  if(!jetHandle.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "jetHandle not found, ";
      //"skipping event"; 
      //return;
  }
 
   // Get b tag information
 edm::Handle<reco::JetTagCollection> bTagIPHandle;
 iEvent.getByLabel("jetProbabilityBJetTags", bTagIPHandle);
 if (!bTagIPHandle.isValid()) {
    edm::LogInfo("FourVectorHLTriggerOffline") << "mTagIPHandle trackCountingHighEffJetTags not found, ";
      //"skipping event"; 
      //return;
  }

   // Get b tag information
 edm::Handle<reco::JetTagCollection> bTagMuHandle;
 iEvent.getByLabel("softMuonBJetTags", bTagMuHandle);
 if (!bTagMuHandle.isValid()) {
    edm::LogInfo("FourVectorHLTriggerOffline") << "bTagMuHandle  not found, ";
      //"skipping event"; 
      //return;
  }

  edm::Handle<reco::CaloMETCollection> metHandle;
  iEvent.getByLabel("met",metHandle);
  if(!metHandle.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "metHandle not found, ";
      //"skipping event"; 
      //return;
  }

  edm::Handle<reco::PhotonCollection> photonHandle;
  iEvent.getByLabel("photons",photonHandle);
  if(!photonHandle.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "photonHandle not found, ";
      //"skipping event"; 
      //return;
  }

  edm::Handle<reco::TrackCollection> trackHandle;
  iEvent.getByLabel("pixelTracks",trackHandle);
  if(!trackHandle.isValid()) { 
    edm::LogInfo("FourVectorHLTriggerOffline") << "trackHandle not found, ";
      //"skipping event"; 
      //return;
  }

  const trigger::TriggerObjectCollection & toc(triggerObj->getObjects());

  // electron Monitor
	// ------------
  objMon<reco::GsfElectronCollection> eleMon;
  eleMon.setReco(gsfElectrons);
  eleMon.setMC(genParticles, 11, 1);
  eleMon.setLimits(electronEtaMax_, electronEtMin_, electronDRMatch_);
  
  eleMon.pushTriggerType(TriggerElectron);
  eleMon.pushTriggerType(TriggerL1NoIsoEG);
  eleMon.pushTriggerType(TriggerL1IsoEG);

  eleMon.pushL1TriggerType(TriggerL1NoIsoEG);
  eleMon.pushL1TriggerType(TriggerL1IsoEG);

  // muon Monitor
	// ------------
  objMon<reco::MuonCollection>  muoMon;
  muoMon.setReco(muonHandle);
  muoMon.setMC(genParticles, 13, 1);
  muoMon.setLimits(muonEtaMax_, muonEtMin_, muonDRMatch_);
  
  muoMon.pushTriggerType(TriggerMuon);
  muoMon.pushTriggerType(TriggerL1Mu);

  muoMon.pushL1TriggerType(TriggerL1Mu);
	
  // tau Monitor
	// ------------
  objMon<reco::CaloTauCollection>  tauMon;
  tauMon.setReco(tauHandle);
  tauMon.setMC(genParticles, 15, 3);
  tauMon.setLimits(tauEtaMax_, tauEtMin_, tauDRMatch_);
  
  tauMon.pushTriggerType(TriggerTau);
  tauMon.pushTriggerType(TriggerL1TauJet);

  tauMon.pushL1TriggerType(TriggerL1TauJet);
  tauMon.pushL1TriggerType(TriggerL1CenJet);
  tauMon.pushL1TriggerType(TriggerL1ForJet);
	
  // photon Monitor
	// ------------
  objMon<reco::PhotonCollection> phoMon;
  phoMon.setReco(photonHandle);
  phoMon.setMC(genParticles, 22, 1);
  phoMon.setLimits(photonEtaMax_, photonEtMin_, photonDRMatch_);
  
  phoMon.pushTriggerType(TriggerPhoton);

  phoMon.pushL1TriggerType(TriggerL1NoIsoEG);
  phoMon.pushL1TriggerType(TriggerL1IsoEG);

  // jet Monitor - NOTICE: we use genJets for MC
	// -------------------------------------------
  objMon<reco::CaloJetCollection> jetMon;
  jetMon.setReco(jetHandle);
  jetMon.setGenJets(true, genJets);
  jetMon.setLimits(jetEtaMax_, jetEtMin_, jetDRMatch_);

  jetMon.pushTriggerType(TriggerJet);
  jetMon.pushTriggerType(TriggerL1CenJet);
  jetMon.pushTriggerType(TriggerL1ForJet);
  
  jetMon.pushL1TriggerType(TriggerL1CenJet);
  jetMon.pushL1TriggerType(TriggerL1ForJet);
  jetMon.pushL1TriggerType(TriggerL1TauJet);

  // bjet Monitor - NOTICE: we use genJets for MC
	// -------------------------------------------
  //objMon<reco::JetTagCollection> bjetMon;
  objMon<reco::CaloJetCollection> bjetMon;
  //bjetMon.setReco(bTagIPHandle);
  bjetMon.setGenJets(true, genJets);
  bjetMon.setBJetsFlag(true);

  bjetMon.pushTriggerType(TriggerBJet);

  bjetMon.pushL1TriggerType(TriggerL1CenJet);
  bjetMon.pushL1TriggerType(TriggerL1ForJet);
  bjetMon.pushL1TriggerType(TriggerL1TauJet);

 
    for(PathInfoCollection::iterator v = hltPaths_.begin();
	v!= hltPaths_.end(); ++v ) 
{ 

    //LogTrace("FourVectorHLTriggerOffline") << " path " << v->getPath() << endl;

  int NMc = 0;
  int NOn = 0;
  int NOff = 0;
  int NL1 = 0;
  int NL1Mc = 0;
  int NOffMc = 0;
  int NOnMc = 0;
  int NOnOff = 0;
  int NL1On = 0;
  int NL1Off = 0;
  int NL1McUM = 0;
  int NOffMcUM = 0;
  int NOnMcUM = 0;
  int NOnOffUM = 0;
  int NL1OnUM = 0;
  int NL1OffUM = 0;


  // did we pass the denomPath?
  bool denompassed = false;  
  for(int i = 0; i < npath; ++i) {
     if (triggerNames.triggerName(i).find(v->getDenomPath()) != std::string::npos && triggerResults->accept(i))
       { 
        denompassed = true;
        break;
       }
  }

  if (denompassed)
  {  

      //LogTrace("FourVectorHLTriggerOffline") << " denominator path " << v->getPath() << endl;

      eleMon.clearSets();
      muoMon.clearSets();
      tauMon.clearSets();
      phoMon.clearSets();
      jetMon.clearSets();
      bjetMon.clearSets();

      //cout << " New hltPath  and denompassed" << endl;
      // set to keep maps of DR matches of each L1 objects with each Off object
      mmset eleL1OffDRMatchSet;
      // set to keep maps of DR matches of each L1 objects with each MC object
      mmset eleL1MCDRMatchSet;

      int triggertype = 0;     
      triggertype = v->getObjectType();

      bool l1accept = false;
      edm::InputTag l1testTag(v->getl1Path(),"",processname_);
      const int l1index = triggerObj->filterIndex(l1testTag);
      if ( l1index >= triggerObj->sizeFilters() ) {
        edm::LogInfo("FourVectorHLTriggerOffline") << "no index "<< l1index << " of that name " << v->getl1Path() << "\t" << "\t" << l1testTag;
	continue; // not in this event
      }

      const trigger::Vids & idtype = triggerObj->filterIds(l1index);
      const trigger::Keys & l1k = triggerObj->filterKeys(l1index);
      l1accept = l1k.size() > 0;
      //LogTrace("FourVectorHLTriggerOffline") << " triggertype = " << triggertype << " TriggerMuon  " <<  TriggerMuon << "   l1accept = " << l1accept << endl;
      //if (l1k.size() == 0) cout << v->getl1Path() << endl;
      //l1accept = true;

			  eleMon.monitorDenominator(v, l1accept, idtype, l1k, toc);
			  muoMon.monitorDenominator(v, l1accept, idtype, l1k, toc);
			  tauMon.monitorDenominator(v, l1accept, idtype, l1k, toc);
			  phoMon.monitorDenominator(v, l1accept, idtype, l1k, toc);
			  jetMon.monitorDenominator(v, l1accept, idtype, l1k, toc);
			  bjetMon.monitorDenominator(v, l1accept, idtype, l1k, toc);

  		eleMon.fillL1Match(this);

  		muoMon.fillL1Match(this);
  
  		tauMon.fillL1Match(this);

  		phoMon.fillL1Match(this);

  		jetMon.fillL1Match(this);

  		bjetMon.fillL1Match(this);

      v->getNMcHisto()->Fill(NMc);      
      v->getNOffHisto()->Fill(NOff);      
      v->getNL1Histo()->Fill(NL1);
      v->getNL1OffHisto()->Fill(NL1Off);
      v->getNL1McHisto()->Fill(NL1Mc);
      v->getNL1OffUMHisto()->Fill(NL1OffUM);
      v->getNL1McUMHisto()->Fill(NL1McUM);

	/*
      // for electron triggers, loop over and fill offline 4-vectors
      //if (triggertype == trigger::TriggerElectron || triggertype == trigger::TriggerL1NoIsoEG || triggertype == trigger::TriggerL1IsoEG)
			{
	if (genParticles.isValid()){
           for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.pdgId()) == 11 && p.status() == 1 && fabs(p.eta()) <= electronEtaMax_ && p.pt() >= electronEtMin_ ){
            NMc++;
	    v->getMcEtMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiMcHisto()->Fill(p.eta(),p.phi());
	  }
	 }
	}

	 if (gsfElectrons.isValid()){

         for (reco::PixelMatchGsfElectronCollection::const_iterator gsfIter=gsfElectrons->begin(); gsfIter!=gsfElectrons->end(); gsfIter++)
         {

	  if (fabs(gsfIter->eta()) <= electronEtaMax_ && gsfIter->pt() >= electronEtMin_ ){

	  NOff++;
    LogTrace("FourVectorHLTriggerOffline") << " Filling Electron Off  ....v->getOffEtOffHisto()->Fill(gsfIter->pt()) pt = " << gsfIter->pt() << endl;
	  //v->getOffEtOffHisto()->Fill(gsfIter->pt());
	  //v->getOffEtaVsOffPhiOffHisto()->Fill(gsfIter->eta(), gsfIter->phi());
	   }
         }
         }


        if (l1accept)
        {
	

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
            if ( *idtypeiter == trigger::TriggerL1IsoEG || *idtypeiter == trigger::TriggerL1NoIsoEG ) {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (fabs(toc[*ki].eta()) <= electronEtaMax_ && toc[*ki].pt() >= electronEtMin_)
            { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	    }


	    if (gsfElectrons.isValid())
            {

              fimmap electronL1OffDRMatchMap;
	      // keep count of iterartor steps
	      int j=0;
              for (reco::PixelMatchGsfElectronCollection::const_iterator gsfIter=gsfElectrons->begin(); gsfIter!=gsfElectrons->end(); gsfIter++)
               { 
	        
		float dR = reco::deltaR((*gsfIter).eta(),(*gsfIter).phi(),toc[*ki].eta(),toc[*ki].phi());
	        if (dR < electronDRMatch_ && fabs((*gsfIter).eta()) <= electronEtaMax_ && (*gsfIter).pt() >= electronEtMin_ )
                 {
                   

			//cout << "Found mathching L1-Off electron " << endl;
			//cout << "Storing it in the map." << endl;

		        electronL1OffDRMatchMap.insert(std::pair<float,int>(dR,j));
	          //NL1Off++;
	          //v->getOffEtL1OffHisto()->Fill((*gsfIter).pt());
	          //v->getOffEtaVsOffPhiL1OffHisto()->Fill((*gsfIter).eta(),(*gsfIter).phi());
	         }
	        if (NL1==1 && fabs((*gsfIter).eta()) <= electronEtaMax_ && (*gsfIter).pt() >= electronEtMin_ )
                 {
	          NL1OffUM++;
	          v->getOffEtL1OffUMHisto()->Fill((*gsfIter).pt());
	          v->getOffEtaVsOffPhiL1OffUMHisto()->Fill((*gsfIter).eta(),(*gsfIter).phi());
            LogTrace("FourVectorHLTriggerOffline") << " Filling Electron Off UM v->getOffEtOffUMHisto()->Fill(gsfIter->pt()) pt = " << gsfIter->pt() << endl;
	         }

	         j++;

	       } // end for
               if(! electronL1OffDRMatchMap.empty()) eleL1OffDRMatchSet.insert(electronL1OffDRMatchMap);
	     }


	    if (genParticles.isValid()){
	      fimmap electronL1MCDRMatchMap;
              for(size_t i = 0; i < genParticles->size(); ++ i) {
              const GenParticle & p = (*genParticles)[i];
              if (abs(p.pdgId()) == 11 && p.status() == 1 && fabs(p.eta()) <= electronEtaMax_ && p.pt() >= electronEtMin_){ 

	       float dR = reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi());
	       if (dR < electronDRMatch_){
		      electronL1MCDRMatchMap.insert(pair<float,int>(dR,i));
	        //NL1Mc++;
	        //v->getMcEtL1McHisto()->Fill(p.pt());
	        //v->getMcEtaVsMcPhiL1McHisto()->Fill(p.eta(),p.phi());
    	       }
	       if (NL1==1){
	        NL1McUM++;
	        v->getMcEtL1McUMHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McUMHisto()->Fill(p.eta(),p.phi());
    	       }
	      }
             } // end for
             if(! electronL1MCDRMatchMap.empty()) eleL1MCDRMatchSet.insert(electronL1MCDRMatchMap);

	    } 

	   } // if idtrigtype
            ++idtypeiter;
	   }
         }

      }

      // for muon triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerMuon || triggertype == trigger::TriggerL1Mu){

	if (genParticles.isValid()){
          for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.pdgId()) == 13 && p.status() == 1 && fabs(p.eta()) <= muonEtaMax_ && p.pt() >= muonEtMin_){
            NMc++;
	    v->getMcEtMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiMcHisto()->Fill(p.eta(),p.phi());
	  }
	 }
	}

	if (muonHandle.isValid()){
         const reco::MuonCollection muonCollection = *(muonHandle.product());
         for (reco::MuonCollection::const_iterator muonIter=muonCollection.begin(); muonIter!=muonCollection.end(); muonIter++)
         {
	   if (fabs((*muonIter).eta()) <= muonEtaMax_ && (*muonIter).pt() >= muonEtMin_){
	  NOff++;
	  v->getOffEtOffHisto()->Fill((*muonIter).pt());
	  v->getOffEtaVsOffPhiOffHisto()->Fill((*muonIter).eta(),(*muonIter).phi());
	   }
         }
	}

        if (l1accept)
        {
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
	    if (*idtypeiter == trigger::TriggerL1Mu)
	      {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (fabs(toc[*ki].eta()) <= muonEtaMax_ && toc[*ki].pt() >= muonEtMin_)
             { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	     }

	    if (muonHandle.isValid())
             {
              const reco::MuonCollection muonCollection = *(muonHandle.product());
              for (reco::MuonCollection::const_iterator muonIter=muonCollection.begin(); muonIter!=muonCollection.end(); muonIter++)
               { 
	        if (reco::deltaR((*muonIter).eta(),(*muonIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < muonDRMatch_ && fabs((*muonIter).eta()) <= muonEtaMax_ && (*muonIter).pt() >= muonEtMin_ )
                 {
	           NL1Off++;
	           v->getOffEtL1OffHisto()->Fill((*muonIter).pt());
	           v->getOffEtaVsOffPhiL1OffHisto()->Fill((*muonIter).eta(),(*muonIter).phi());
	         }
	        if (NL1==1 && fabs((*muonIter).eta()) <= muonEtaMax_ && (*muonIter).pt() >= muonEtMin_ )
                 {
	           NL1OffUM++;
	           v->getOffEtL1OffUMHisto()->Fill((*muonIter).pt());
	           v->getOffEtaVsOffPhiL1OffUMHisto()->Fill((*muonIter).eta(),(*muonIter).phi());
	         }
	       }
	     }


	    if (genParticles.isValid()){
               for(size_t i = 0; i < genParticles->size(); ++ i) {
              const GenParticle & p = (*genParticles)[i];
              if (abs(p.pdgId()) == 13 && p.status() == 1 && fabs(p.eta()) <= muonEtaMax_ && p.pt() >= muonEtMin_){ 
	       if (reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi()) < muonDRMatch_){
	        NL1Mc++;
	        v->getMcEtL1McHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McHisto()->Fill(p.eta(),p.phi());
    	       }
	       if (NL1==1){
	        NL1McUM++;
	        v->getMcEtL1McUMHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McUMHisto()->Fill(p.eta(),p.phi());
    	       }
	      }
             }
	    } 

	   }
            ++idtypeiter;
	   }
         }
     } 


    

      // for tau triggers, loop over and fill offline 4-vectors
      if (triggertype == trigger::TriggerTau || triggertype == trigger::TriggerL1TauJet)
	{

	if (genParticles.isValid()){
           for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.pdgId()) == 15 && p.status() == 1 && fabs(p.eta()) <= tauEtaMax_ && p.pt() >= tauEtMin_){
            NMc++; 
	    v->getMcEtMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiMcHisto()->Fill(p.eta(),p.phi());
	  }
	 }
	}

	  if (tauHandle.isValid()){
	    const reco::CaloTauCollection tauCollection = *(tauHandle.product());
         for (reco::CaloTauCollection::const_iterator tauIter=tauCollection.begin(); tauIter!=tauCollection.end(); tauIter++)
         {
	   if (fabs((*tauIter).eta()) <= tauEtaMax_ && (*tauIter).pt() >= tauEtMin_ ){
	  NOff++;
	  v->getOffEtOffHisto()->Fill((*tauIter).pt());
	  v->getOffEtaVsOffPhiOffHisto()->Fill((*tauIter).eta(),(*tauIter).phi());
	   }
         }
         }


        if (l1accept)
         {
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
	    if (*idtypeiter == trigger::TriggerL1TauJet || *idtypeiter == trigger::TriggerL1ForJet)
	      {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (fabs(toc[*ki].eta()) <= tauEtaMax_ && toc[*ki].pt() >= tauEtMin_)
             { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	     }

	    if (tauHandle.isValid())
             {
              const reco::CaloTauCollection tauCollection = *(tauHandle.product());
              for (reco::CaloTauCollection::const_iterator tauIter=tauCollection.begin(); tauIter!=tauCollection.end(); tauIter++)
               { 
	        if (reco::deltaR((*tauIter).eta(),(*tauIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < tauDRMatch_ && fabs((*tauIter).eta()) <= tauEtaMax_ && (*tauIter).pt() >= tauEtMin_ )
                 {
	          NL1Off++;
	          v->getOffEtL1OffHisto()->Fill((*tauIter).pt());
	          v->getOffEtaVsOffPhiL1OffHisto()->Fill((*tauIter).eta(),(*tauIter).phi());
	         }
	        if (NL1==1 && fabs((*tauIter).eta()) <= tauEtaMax_ && (*tauIter).pt() >= tauEtMin_ )
                 {
	          NL1OffUM++;
	          v->getOffEtL1OffUMHisto()->Fill((*tauIter).pt());
	          v->getOffEtaVsOffPhiL1OffUMHisto()->Fill((*tauIter).eta(),(*tauIter).phi());
	         }
	       }
	     }


	    if (genParticles.isValid()){
               for(size_t i = 0; i < genParticles->size(); ++ i) {
              const GenParticle & p = (*genParticles)[i];
              if (abs(p.pdgId()) == 15 && p.status() == 1 && fabs(p.eta()) <= tauEtaMax_ && p.pt() >= tauEtMin_){ 
	       if (reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi()) < tauDRMatch_){
	        NL1Mc++;
	        v->getMcEtL1McHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McHisto()->Fill(p.eta(),p.phi());
    	       }
	       if (NL1==1){
	        NL1McUM++;
	        v->getMcEtL1McUMHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McUMHisto()->Fill(p.eta(),p.phi());
    	       }
	      }
             }
	    } 

	   }
            ++idtypeiter;
	   }
         }

    }



      // for jet triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerJet || triggertype == trigger::TriggerL1CenJet || triggertype == trigger::TriggerL1ForJet)
	{
	if (genJets.isValid()){
           for(GenJetCollection::const_iterator gjet=genJets->begin(); gjet!=genJets->end(); gjet++) {
          if (fabs(gjet->eta()) <= jetEtaMax_ && gjet->pt() >= jetEtMin_ ){
            NMc++; 
	    v->getMcEtMcHisto()->Fill(gjet->pt());
	    v->getMcEtaVsMcPhiMcHisto()->Fill(gjet->eta(),gjet->phi());
	  }
	 }
	}

	  if (jetHandle.isValid()){
         const reco::CaloJetCollection jetCollection = *(jetHandle.product());
         for (reco::CaloJetCollection::const_iterator jetIter=jetCollection.begin(); jetIter!=jetCollection.end(); jetIter++)
         {
	   if (fabs((*jetIter).eta()) <= jetEtaMax_ && (*jetIter).pt() >= jetEtMin_ ){
	  NOff++;
	  v->getOffEtOffHisto()->Fill((*jetIter).pt());
	  v->getOffEtaVsOffPhiOffHisto()->Fill((*jetIter).eta(),(*jetIter).phi());
	   }
         }
         }

        if (l1accept)
         {
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
	    if (*idtypeiter == trigger::TriggerL1TauJet || *idtypeiter == trigger::TriggerL1ForJet || *idtypeiter == trigger::TriggerL1CenJet)
	      {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (fabs(toc[*ki].eta()) <= jetEtaMax_ && toc[*ki].pt() >= jetEtMin_)
             { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	     }

	    if (jetHandle.isValid())
             {
              const reco::CaloJetCollection jetCollection = *(jetHandle.product());
              for (reco::CaloJetCollection::const_iterator jetIter=jetCollection.begin(); jetIter!=jetCollection.end(); jetIter++)
               { 
	        if (reco::deltaR((*jetIter).eta(),(*jetIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < jetDRMatch_ && fabs((*jetIter).eta()) <= jetEtaMax_ && (*jetIter).pt() >= jetEtMin_ )
                 {
	          NL1Off++;
	          v->getOffEtL1OffHisto()->Fill((*jetIter).pt());
	          v->getOffEtaVsOffPhiL1OffHisto()->Fill((*jetIter).eta(),(*jetIter).phi());
	         }
	        if (NL1==1 && fabs((*jetIter).eta()) <= jetEtaMax_ && (*jetIter).pt() >= jetEtMin_ )
                 {
	          NL1OffUM++;
	          v->getOffEtL1OffUMHisto()->Fill((*jetIter).pt());
	          v->getOffEtaVsOffPhiL1OffUMHisto()->Fill((*jetIter).eta(),(*jetIter).phi());
	         }
	       }
	     }

	if (genJets.isValid()){
           for(GenJetCollection::const_iterator gjet=genJets->begin(); gjet!=genJets->end(); gjet++) {
             if (fabs(gjet->eta()) <= jetEtaMax_ && gjet->pt() >= jetEtMin_ ){
	       if (reco::deltaR(gjet->eta(),gjet->phi(),toc[*ki].eta(),toc[*ki].phi()) < jetDRMatch_){
	        NL1Mc++;
	        v->getMcEtL1McHisto()->Fill(gjet->pt());
	        v->getMcEtaVsMcPhiL1McHisto()->Fill(gjet->eta(),gjet->phi());
    	       }
	       if (NL1==1){
	        NL1McUM++;
	        v->getMcEtL1McUMHisto()->Fill(gjet->pt());
	        v->getMcEtaVsMcPhiL1McUMHisto()->Fill(gjet->eta(),gjet->phi());
    	       }
	      }
             }
	    } 

	   }
            ++idtypeiter;
	   }
         }

	}

      // for bjet triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerBJet)
	{ 
	  if (v->getPath().find("BTagIP") != std::string::npos && bTagIPHandle.isValid()){
          const reco::JetTagCollection & bTags = *(bTagIPHandle.product());
          for (size_t i = 0; i != bTags.size(); ++i) {
           edm::RefToBase<reco::Jet>  BRefJet=bTags[i].first;
 	   if (fabs(BRefJet->eta()) <= bjetEtaMax_ && BRefJet->pt() >= bjetEtMin_ ){
	      NOff++;
	      v->getOffEtOffHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiOffHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	   }
	  }
	 }

	  if (v->getPath().find("BTagMu") != std::string::npos && bTagMuHandle.isValid()){
          const reco::JetTagCollection & bTags = *(bTagMuHandle.product());
          for (size_t i = 0; i != bTags.size(); ++i) {
           edm::RefToBase<reco::Jet>  BRefJet=bTags[i].first;
 	   if (fabs(BRefJet->eta()) <= bjetEtaMax_ && BRefJet->pt() >= bjetEtMin_ ){
	      NOff++;
	      v->getOffEtOffHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiOffHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	   }
	  }
	 }

	
	 if (genJets.isValid()){
           for(GenJetCollection::const_iterator gjet=genJets->begin(); gjet!=genJets->end(); gjet++) {
          if (fabs(gjet->eta()) <= bjetEtaMax_ && gjet->pt() >= bjetEtMin_){
	    // now check for a b parton in the cone
            bool bjet = false;
	    if (genParticles.isValid()){
               for(size_t i = 0; i < genParticles->size(); ++ i) {
              const GenParticle & p = (*genParticles)[i];
              if (abs(p.pdgId()) == 5){ 
	       if (reco::deltaR(p.eta(),p.phi(),gjet->eta(),gjet->phi()) < 0.7){
		 bjet = true;
	       }
	      }
	     }
	    }
            if (bjet)
             {
              NMc++; 
	      v->getMcEtMcHisto()->Fill(gjet->pt());
	      v->getMcEtaVsMcPhiMcHisto()->Fill(gjet->eta(),gjet->phi());
	     }
	   }
	  }
	 }


        if (l1accept)
         {
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
	    if (*idtypeiter == trigger::TriggerL1TauJet || *idtypeiter == trigger::TriggerL1ForJet || *idtypeiter == trigger::TriggerL1CenJet)
	      {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (fabs(toc[*ki].eta()) <= bjetEtaMax_ && toc[*ki].pt() >= bjetEtMin_)
             { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	     }

	  if (v->getPath().find("BTagIP") != std::string::npos && bTagIPHandle.isValid()){
          const reco::JetTagCollection & bTags = *(bTagIPHandle.product());
          for (size_t i = 0; i != bTags.size(); ++i) {
           edm::RefToBase<reco::Jet>  BRefJet=bTags[i].first;
           
 	   if (fabs(BRefJet->eta()) <= bjetEtaMax_ && BRefJet->pt() >= bjetEtMin_ ){
	   if (reco::deltaR(BRefJet->eta(),BRefJet->phi(),toc[*ki].eta(),toc[*ki].phi()) < bjetDRMatch_){
	      NL1Off++;
	      v->getOffEtL1OffHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiL1OffHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	    }
	   if (NL1==1){
	      NL1OffUM++;
	      v->getOffEtL1OffUMHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiL1OffUMHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	    }
	   }
	  }
	 }


	  if (v->getPath().find("BTagMu") != std::string::npos && bTagMuHandle.isValid()){
          const reco::JetTagCollection & bTags = *(bTagMuHandle.product());
          for (size_t i = 0; i != bTags.size(); ++i) {
           edm::RefToBase<reco::Jet>  BRefJet=bTags[i].first;
           
 	   if (fabs(BRefJet->eta()) <= bjetEtaMax_ && BRefJet->pt() >= bjetEtMin_ ){
	   if (reco::deltaR(BRefJet->eta(),BRefJet->phi(),toc[*ki].eta(),toc[*ki].phi()) < bjetDRMatch_){
	      NL1Off++;
	      v->getOffEtL1OffHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiL1OffHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	    }
	   if (NL1==1){
	      NL1OffUM++;
	      v->getOffEtL1OffUMHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiL1OffUMHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	    }
	   }
	  }
	 }
	
	    if (genJets.isValid()){
               for(GenJetCollection::const_iterator gjet=genJets->begin(); gjet!=genJets->end(); gjet++) {
              if (fabs(gjet->eta()) <= bjetEtaMax_ && gjet->pt() >= bjetEtMin_ ){
            bool bjet = false;
	    if (genParticles.isValid()){
               for(size_t i = 0; i < genParticles->size(); ++ i) {
              const GenParticle & p = (*genParticles)[i];
              if (abs(p.pdgId()) == 5){ 
	       if (reco::deltaR(p.eta(),p.phi(),gjet->eta(),gjet->phi()) < 0.7){
		 bjet = true;
	       }
	      }
	     }
	    }
	    if (bjet){
	       if (reco::deltaR(gjet->eta(),gjet->phi(),toc[*ki].eta(),toc[*ki].phi()) < bjetDRMatch_){
	        NL1Mc++;
	        v->getMcEtL1McHisto()->Fill(gjet->pt());
	        v->getMcEtaVsMcPhiL1McHisto()->Fill(gjet->eta(),gjet->phi());
    	     }
	       if (NL1==1){
	        NL1McUM++;
	        v->getMcEtL1McUMHisto()->Fill(gjet->pt());
	        v->getMcEtaVsMcPhiL1McUMHisto()->Fill(gjet->eta(),gjet->phi());
    	     }
	    }
             }
	    } 
	   }

	   }
            ++idtypeiter;
	   }
         }

	}
      // for met triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerMET || triggertype == trigger::TriggerL1ETM)
	{

	  if (genMets.isValid())
	  {
           for(GenMETCollection::const_iterator gmet=genMets->begin(); gmet!=genMets->end(); gmet++) {
	     if (gmet->pt() > metMin_){
	    NMc++;
            v->getMcEtMcHisto()->Fill(gmet->pt());
	    v->getMcEtaVsMcPhiMcHisto()->Fill(0.0,gmet->phi());
	     }
	   }
	  }

	  if (metHandle.isValid()){
         const reco::CaloMETCollection metCollection = *(metHandle.product());
         for (reco::CaloMETCollection::const_iterator metIter=metCollection.begin(); metIter!=metCollection.end(); metIter++)
         {
	   if ((*metIter).pt() >= metMin_){
	  NOff++;
	  v->getOffEtOffHisto()->Fill((*metIter).pt());
	  v->getOffEtaVsOffPhiOffHisto()->Fill((*metIter).eta(),(*metIter).phi());
	   }
         }
         }


        if (l1accept)
         {
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
	    if (*idtypeiter == trigger::TriggerL1ETM)
	      {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (toc[*ki].pt() >= metMin_)
             { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	     }

	    if (metHandle.isValid())
             {
              const reco::CaloMETCollection metCollection = *(metHandle.product());
              for (reco::CaloMETCollection::const_iterator metIter=metCollection.begin(); metIter!=metCollection.end(); metIter++)
               { 
		 //	        if (reco::deltaR((*metIter).eta(),(*metIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < 0.3 && (*metIter).pt() >= metMin_)
		 //   {
	          NL1Off++;
	          v->getOffEtL1OffHisto()->Fill((*metIter).pt());
	          v->getOffEtaVsOffPhiL1OffHisto()->Fill((*metIter).eta(),(*metIter).phi());
		  //   }
	       }
	     }


	  if (genMets.isValid())
	  {
           for(GenMETCollection::const_iterator gmet=genMets->begin(); gmet!=genMets->end(); gmet++) {
	     if (gmet->pt() > metMin_){
	    NL1Mc++;
            v->getMcEtL1McHisto()->Fill(gmet->pt());
	    v->getMcEtaVsMcPhiL1McHisto()->Fill(0.0,gmet->phi());
	     }
	   }
	  }

	      }

            ++idtypeiter;
	   }
         }

	}
      else if (triggertype == trigger::TriggerTET || triggertype == trigger::TriggerL1ETT)
	{

	if (genParticles.isValid()){
          double sumet = 0.0; //double metphi = 0.0;
          for(size_t i = 0; i < genParticles->size(); ++ i) {
           const GenParticle & p = (*genParticles)[i];
          if ((abs(p.pdgId()) != 12 && abs(p.pdgId()) != 14 && abs(p.pdgId()) != 16 && abs(p.pdgId()) != 18 && abs(p.pdgId()) != 1000022 && abs(p.pdgId()) != 1000039) && p.status() == 1 && fabs(p.eta()) < 5.0){ 
	    sumet += p.pt();
	  }
	 }
            if (sumet >= sumEtMin_){
	    NMc++;
            v->getMcEtMcHisto()->Fill(sumet);
	    v->getMcEtaVsMcPhiMcHisto()->Fill(0.0,0.0);
	    }
	}

	  if (metHandle.isValid()){
         const reco::CaloMETCollection metCollection = *(metHandle.product());
         for (reco::CaloMETCollection::const_iterator metIter=metCollection.begin(); metIter!=metCollection.end(); metIter++)
         {
	   if ((*metIter).sumEt() >= sumEtMin_){
	  NOff++;
	  v->getOffEtOffHisto()->Fill((*metIter).sumEt());
	  v->getOffEtaVsOffPhiOffHisto()->Fill(0.0,0.0);
	   }
         }
         }


        if (l1accept)
         {
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
	    if (*idtypeiter == trigger::TriggerL1ETT)
	      {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (toc[*ki].pt() >= sumEtMin_)
             { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	     }

	    if (metHandle.isValid())
             {
              const reco::CaloMETCollection metCollection = *(metHandle.product());
              for (reco::CaloMETCollection::const_iterator metIter=metCollection.begin(); metIter!=metCollection.end(); metIter++)
               { 
	        if ((*metIter).sumEt() >= metMin_)
                 {
	          NL1Off++;
	          v->getOffEtL1OffHisto()->Fill((*metIter).sumEt());
	          v->getOffEtaVsOffPhiL1OffHisto()->Fill(0.0,0.0);
	         }
	       }
	     }


	  if (genMets.isValid())
	  {
           for(GenMETCollection::const_iterator gmet=genMets->begin(); gmet!=genMets->end(); gmet++) {
	     if (gmet->sumEt() > sumEtMin_){
	    NL1Mc++;
            v->getMcEtL1McHisto()->Fill(gmet->sumEt());
	    v->getMcEtaVsMcPhiL1McHisto()->Fill(0.0,0.0);
	     }
	   }
	  }

	   }
            ++idtypeiter;
	   }
         }

	}
      // for photon triggers, loop over and fill offline and L1 4-vectors
      else if (triggertype == trigger::TriggerPhoton)
	{

	if (genParticles.isValid()){
           for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.pdgId()) == 22 && p.status() == 1 && fabs(p.eta()) <= photonEtaMax_ && p.pt() >= photonEtMin_){
            NMc++; 
	    v->getMcEtMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiMcHisto()->Fill(p.eta(),p.phi());
	  }
	 }
	}

	  if (photonHandle.isValid()){
          const reco::PhotonCollection photonCollection = *(photonHandle.product());
         for (reco::PhotonCollection::const_iterator photonIter=photonCollection.begin(); photonIter!=photonCollection.end(); photonIter++)
         {
	   if (fabs((*photonIter).eta()) <= photonEtaMax_ && (*photonIter).pt() >= photonEtMin_ ){
	  NOff++;
	  v->getOffEtOffHisto()->Fill((*photonIter).pt());
	  v->getOffEtaVsOffPhiOffHisto()->Fill((*photonIter).eta(),(*photonIter).phi());
	   }
         }
	  }



        if (l1accept)
         {
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
	    if (*idtypeiter == trigger::TriggerL1IsoEG || *idtypeiter == trigger::TriggerL1NoIsoEG)
	      {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (fabs(toc[*ki].eta()) <= photonEtaMax_ && toc[*ki].pt() >= photonEtMin_)
             { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	     }

	    if (photonHandle.isValid())
             {
              const reco::PhotonCollection photonCollection = *(photonHandle.product());
              for (reco::PhotonCollection::const_iterator photonIter=photonCollection.begin(); photonIter!=photonCollection.end(); photonIter++)
               { 
	        if (reco::deltaR((*photonIter).eta(),(*photonIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < photonDRMatch_ && fabs((*photonIter).eta()) <= photonEtaMax_ && (*photonIter).pt() >= photonEtMin_ )
                 {
	          NL1Off++;
	          v->getOffEtL1OffHisto()->Fill((*photonIter).pt());
	          v->getOffEtaVsOffPhiL1OffHisto()->Fill((*photonIter).eta(),(*photonIter).phi());
	         }
	        if (NL1==1 && fabs((*photonIter).eta()) <= photonEtaMax_ && (*photonIter).pt() >= photonEtMin_ )
                 {
	          NL1OffUM++;
	          v->getOffEtL1OffUMHisto()->Fill((*photonIter).pt());
	          v->getOffEtaVsOffPhiL1OffUMHisto()->Fill((*photonIter).eta(),(*photonIter).phi());
	         }
	       }
	     }


	    if (genParticles.isValid()){
               for(size_t i = 0; i < genParticles->size(); ++ i) {
              const GenParticle & p = (*genParticles)[i];
              if (abs(p.pdgId()) == 22 && p.status() == 1 && fabs(p.eta()) <= photonEtaMax_ && p.pt() >= photonEtMin_ ){
	       if (reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi()) < photonDRMatch_){
	        NL1Mc++;
	        v->getMcEtL1McHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McHisto()->Fill(p.eta(),p.phi());
    	       }
	       if (NL1==1){
	        NL1McUM++;
	        v->getMcEtL1McUMHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McUMHisto()->Fill(p.eta(),p.phi());
    	       }
	      }
             }
	    } 

	   }
            ++idtypeiter;
	   }
         }
	}

      // for IsoTrack triggers, loop over and fill offline and L1 4-vectors
      else if (triggertype == trigger::TriggerTrack)
	{

	if (genParticles.isValid()){
           for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.charge()) > 0 && p.status() == 1 && fabs(p.eta()) <= trackEtaMax_ && p.pt() >= trackEtMin_){
            NMc++; 
	    v->getMcEtMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiMcHisto()->Fill(p.eta(),p.phi());
	  }
	 }
	}

	  if (trackHandle.isValid()){
          const reco::TrackCollection trackCollection = *(trackHandle.product());
         for (reco::TrackCollection::const_iterator trackIter=trackCollection.begin(); trackIter!=trackCollection.end(); trackIter++)
         {
	   if (fabs((*trackIter).eta()) <= trackEtaMax_ && (*trackIter).pt() >= trackEtMin_ ){
	  NOff++;
	  v->getOffEtOffHisto()->Fill((*trackIter).pt());
	  v->getOffEtaVsOffPhiOffHisto()->Fill((*trackIter).eta(),(*trackIter).phi());
	   }
         }
	  }



        if (l1accept)
         {
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator ki = l1k.begin(); ki !=l1k.end(); ++ki ) {
	    if (*idtypeiter == trigger::TriggerL1CenJet || *idtypeiter == trigger::TriggerL1ForJet || *idtypeiter == trigger::TriggerL1TauJet)
	      {
	    //	cout << v->getl1Path() << "\t" << *idtypeiter << "\t" << toc[*ki].pt() << "\t" << toc[*ki].eta() << "\t" << toc[*ki].phi() << endl;
  	    if (fabs(toc[*ki].eta()) <= trackEtaMax_ && toc[*ki].pt() >= trackEtMin_)
             { 
	      NL1++;    
              v->getL1EtL1Histo()->Fill(toc[*ki].pt());
	      v->getL1EtaVsL1PhiL1Histo()->Fill(toc[*ki].eta(), toc[*ki].phi());
	     }

	    if (trackHandle.isValid())
             {
              const reco::TrackCollection trackCollection = *(trackHandle.product());
              for (reco::TrackCollection::const_iterator trackIter=trackCollection.begin(); trackIter!=trackCollection.end(); trackIter++)
               { 
	        if (reco::deltaR((*trackIter).eta(),(*trackIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < trackDRMatch_ && fabs((*trackIter).eta()) <= trackEtaMax_ && (*trackIter).pt() >= trackEtMin_ )
                 {
	          NL1Off++;
	          v->getOffEtL1OffHisto()->Fill((*trackIter).pt());
	          v->getOffEtaVsOffPhiL1OffHisto()->Fill((*trackIter).eta(),(*trackIter).phi());
	         }
	        if (NL1==1 && fabs((*trackIter).eta()) <= trackEtaMax_ && (*trackIter).pt() >= trackEtMin_ )
                 {
	          NL1OffUM++;
	          v->getOffEtL1OffUMHisto()->Fill((*trackIter).pt());
	          v->getOffEtaVsOffPhiL1OffUMHisto()->Fill((*trackIter).eta(),(*trackIter).phi());
	         }
	       }
	     }


	    if (genParticles.isValid()){
               for(size_t i = 0; i < genParticles->size(); ++ i) {
              const GenParticle & p = (*genParticles)[i];
              if (abs(p.charge()) > 0 && p.status() == 1 && fabs(p.eta()) <= trackEtaMax_ && p.pt() >= trackEtMin_ ){
	       if (reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi()) < trackDRMatch_){
	        NL1Mc++;
	        v->getMcEtL1McHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McHisto()->Fill(p.eta(),p.phi());
    	       }
	       if (NL1==1){
	        NL1McUM++;
	        v->getMcEtL1McUMHisto()->Fill(p.pt());
	        v->getMcEtaVsMcPhiL1McUMHisto()->Fill(p.eta(),p.phi());
    	       }
	      }
             }
	    } 

	   }
            ++idtypeiter;
	   }
         }

	}
	   */

    // did we pass the numerator path?
  bool numpassed = false;
  for(int i = 0; i < npath; ++i) {
     if (triggerNames.triggerName(i) == v->getPath() && triggerResults->accept(i)) numpassed = true;
  }

  if (numpassed)
    { 

			/*
      // set to keep maps of DR matches of each On objects with each Off object
      mmset eleOnOffDRMatchSet;
      // set to keep maps of DR matches of each On objects with each L1 object
      mmset eleOnL1DRMatchSet;
      // set to keep maps of DR matches of each On objects with each MC object
      mmset eleOnMCDRMatchSet;
			*/

 
      if (!l1accept) {
            edm::LogInfo("FourVectorHLTriggerOffline") << "l1 seed path not accepted for hlt path "<< v->getPath() << "\t" << v->getl1Path();
      }
    // ok plot On, L1On, OnOff, and OnMc objects

    // fill scaler histograms
      edm::InputTag filterTag = v->getTag();

	// loop through indices and see if the filter is on the list of filters used by this path
      
    if (v->getLabel() == "dummy"){
        const std::vector<std::string> filterLabels = hltConfig_.moduleLabels(v->getPath());
	//loop over labels
        for (std::vector<std::string>::const_iterator labelIter= filterLabels.begin(); labelIter!=filterLabels.end(); labelIter++)          
	 {
	   //cout << v->getPath() << "\t" << *labelIter << endl;
           // last match wins...
	   edm::InputTag testTag(*labelIter,"",processname_);
	   //           cout << v->getPath() << "\t" << testTag.label() << "\t" << testTag.process() << endl;
           int testindex = triggerObj->filterIndex(testTag);
           if ( !(testindex >= triggerObj->sizeFilters()) ) {
	     //cout << "found one! " << v->getPath() << "\t" << testTag.label() << endl; 
            filterTag = testTag; v->setLabel(*labelIter);}
	 }
         }
	
      const int index = triggerObj->filterIndex(filterTag);
      if ( index >= triggerObj->sizeFilters() ) {
	//        cout << "WTF no index "<< index << " of that name "
	//	     << filterTag << endl;
	continue; // not in this event
      }
      //LogDebug("FourVectorHLTriggerOffline") << "filling ... " ;
      const trigger::Keys & k = triggerObj->filterKeys(index);
      //      const trigger::Vids & idtype = triggerObj->filterIds(index);
      // assume for now the first object type is the same as all objects in the collection
      //    cout << filterTag << "\t" << idtype.size() << "\t" << k.size() << endl;
      //     cout << "path " << v->getPath() << " trigger type "<<triggertype << endl;
      //if (k.size() > 0) v->getNOnHisto()->Fill(k.size());



      for (trigger::Keys::const_iterator ki = k.begin(); ki !=k.end(); ++ki ) {

        eleMon.monitorOnline(idtype, l1k, ki, toc, NOn);

        muoMon.monitorOnline(idtype, l1k, ki, toc, NOn);

        tauMon.monitorOnline(idtype, l1k, ki, toc, NOn);

        phoMon.monitorOnline(idtype, l1k, ki, toc, NOn);

        jetMon.monitorOnline(idtype, l1k, ki, toc, NOn);

        bjetMon.monitorOnline(idtype, l1k, ki, toc, NOn);
			/*
         
        double tocEtaMax = 2.5;
        double tocEtMin = 3.0;
        if (triggertype == trigger::TriggerMuon || triggertype == trigger::TriggerL1Mu) 
	  {
	    tocEtaMax = muonEtaMax_; tocEtMin = muonEtMin_;
	  }
        else if (triggertype == trigger::TriggerElectron || triggertype == trigger::TriggerL1NoIsoEG || triggertype == trigger::TriggerL1IsoEG )
	  {
	    tocEtaMax = electronEtaMax_; tocEtMin = electronEtMin_;
	  }
        else if (triggertype == trigger::TriggerTau || triggertype == trigger::TriggerL1TauJet )
	  {
	    tocEtaMax = tauEtaMax_; tocEtMin = tauEtMin_;
	  }
        else if (triggertype == trigger::TriggerJet || triggertype == trigger::TriggerL1CenJet || triggertype == trigger::TriggerL1ForJet )
	  {
	    tocEtaMax = jetEtaMax_; tocEtMin = jetEtMin_;
	  }
        else if (triggertype == trigger::TriggerBJet)
	  {
	    tocEtaMax = bjetEtaMax_; tocEtMin = bjetEtMin_;
	  }
        else if (triggertype == trigger::TriggerMET || triggertype == trigger::TriggerL1ETM )
	  {
	    tocEtaMax = 999.0; tocEtMin = metMin_;
	  }
        else if (triggertype == trigger::TriggerPhoton)
	  {
	    tocEtaMax = photonEtaMax_; tocEtMin = photonEtMin_;
	  }
        else if (triggertype == trigger::TriggerTrack)
	  {
	    tocEtaMax = trackEtaMax_; tocEtMin = trackEtMin_;
	  }

        if (fabs(toc[*ki].eta()) <= tocEtaMax && toc[*ki].pt() >= tocEtMin)
	  {
	NOn++;    
        //v->getOnEtOnHisto()->Fill(toc[*ki].pt());
	//v->getOnEtaVsOnPhiOnHisto()->Fill(toc[*ki].eta(), toc[*ki].phi());
	  }
	//	  cout << "pdgId "<<toc[*ki].id() << endl;
      // for electron triggers, loop over and fill offline 4-vectors
			
  if (triggertype == trigger::TriggerElectron || triggertype == trigger::TriggerL1IsoEG || triggertype == trigger::TriggerL1NoIsoEG )
	{
	  //	  std::cout << "Electron trigger" << std::endl;

	  if (gsfElectrons.isValid()){
          fimmap electronOnOffDRMatchMap;
	  int j=0;
         for (reco::PixelMatchGsfElectronCollection::const_iterator gsfIter=gsfElectrons->begin(); gsfIter!=gsfElectrons->end(); gsfIter++)
         {
           float dR = reco::deltaR((*gsfIter).eta(),(*gsfIter).phi(),toc[*ki].eta(),toc[*ki].phi());
	   if (dR < electronDRMatch_ && fabs((*gsfIter).eta()) <= electronEtaMax_ && (*gsfIter).pt() >= electronEtMin_ )
	   //if (reco::deltaR((*gsfIter).eta(),(*gsfIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < electronDRMatch_ && fabs((*gsfIter).eta()) <= electronEtaMax_ && (*gsfIter).pt() >= electronEtMin_ )
	   {
	  //NOnOff++;
	  //v->getOffEtOnOffHisto()->Fill(gsfIter->pt());
	  //v->getOffEtaVsOffPhiOnOffHisto()->Fill(gsfIter->eta(), gsfIter->phi());
	  electronOnOffDRMatchMap.insert(std::pair<float,int>(dR,j));
	   }
	   if (NOn==1 && fabs((*gsfIter).eta()) <= electronEtaMax_ && (*gsfIter).pt() >= electronEtMin_ ){
	  NOnOffUM++;
	  v->getOffEtOnOffUMHisto()->Fill(gsfIter->pt());
	  v->getOffEtaVsOffPhiOnOffUMHisto()->Fill(gsfIter->eta(), gsfIter->phi());
	   }

	   j++;
         }
         if(! electronOnOffDRMatchMap.empty()) eleOnOffDRMatchSet.insert(electronOnOffDRMatchMap);

         }

          fimmap electronOnL1DRMatchMap;
	  int j=0;
          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	   if (*idtypeiter == trigger::TriggerL1IsoEG || *idtypeiter == trigger::TriggerL1NoIsoEG)
	      {
           float dR = reco::deltaR(toc[*l1ki].eta(),toc[*l1ki].phi(),toc[*ki].eta(),toc[*ki].phi());
	   if (dR < electronDRMatch_ && fabs(toc[*l1ki].eta()) <= electronEtaMax_ && toc[*l1ki].pt() >= electronEtMin_ )
            {
	     //NL1On++;
	     //v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     //v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	     electronOnL1DRMatchMap.insert(std::pair<float,int>(dR,j));
	    }
	   if (NOn==1 && fabs(toc[*l1ki].eta()) <= electronEtaMax_ && toc[*l1ki].pt() >= electronEtMin_ )
            {
	     NL1OnUM++;
	     v->getL1EtL1OnUMHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnUMHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
              }
	    ++idtypeiter;
	    j++;
	  }
          if(! electronOnL1DRMatchMap.empty()) eleOnL1DRMatchSet.insert(electronOnL1DRMatchMap);


	if (genParticles.isValid()){

          fimmap electronOnMCDRMatchMap;
          for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.pdgId()) == 11 && p.status() == 1 && fabs(p.eta()) <= electronEtaMax_ && p.pt() >= electronEtMin_ ){ 
	   float dR = reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi());
	   if (dR < electronDRMatch_){

	    //NOnMc++;
	    //v->getMcEtOnMcHisto()->Fill(p.pt());
	    //v->getMcEtaVsMcPhiOnMcHisto()->Fill(p.eta(),p.phi());
	    electronOnMCDRMatchMap.insert(std::pair<float,int>(dR,i));
	   }
	   if (NOn==1){
	    NOnMcUM++;
	    v->getMcEtOnMcUMHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcUMHisto()->Fill(p.eta(),p.phi());
	   }
	  }
	 }
         if(! electronOnMCDRMatchMap.empty()) eleOnMCDRMatchSet.insert(electronOnMCDRMatchMap);
	}

 }

      // for muon triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerMuon || triggertype == trigger::TriggerL1Mu)
	{

	  if (muonHandle.isValid()){
         const reco::MuonCollection muonCollection = *(muonHandle.product());
         for (reco::MuonCollection::const_iterator muonIter=muonCollection.begin(); muonIter!=muonCollection.end(); muonIter++)
         {
	   if (reco::deltaR((*muonIter).eta(),(*muonIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < muonDRMatch_ && fabs((*muonIter).eta())<= muonEtaMax_ && (*muonIter).pt() >= muonEtMin_){
	  NOnOff++;
	  v->getOffEtOnOffHisto()->Fill((*muonIter).pt());
	  v->getOffEtaVsOffPhiOnOffHisto()->Fill((*muonIter).eta(),(*muonIter).phi());
	   }
	   if (NOn==1 && fabs((*muonIter).eta())<= muonEtaMax_ && (*muonIter).pt() >= muonEtMin_){
	  NOnOffUM++;
	  v->getOffEtOnOffUMHisto()->Fill((*muonIter).pt());
	  v->getOffEtaVsOffPhiOnOffUMHisto()->Fill((*muonIter).eta(),(*muonIter).phi());
	   }
         }

	}

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	    if (*idtypeiter == trigger::TriggerL1Mu)
	      {
	   if (reco::deltaR(toc[*l1ki].eta(),toc[*l1ki].phi(),toc[*ki].eta(),toc[*ki].phi()) < muonDRMatch_ && fabs(toc[*l1ki].eta()) <= muonEtaMax_ && toc[*l1ki].pt() >= muonEtMin_ )
            {
	     NL1On++;
	     v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
	   if (NOn==1 && fabs(toc[*l1ki].eta()) <= muonEtaMax_ && toc[*l1ki].pt() >= muonEtMin_ )
            {
	     NL1OnUM++;
	     v->getL1EtL1OnUMHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnUMHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
              }
	    ++idtypeiter;
	  }

	if (genParticles.isValid()){
           for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.pdgId()) == 13 && p.status() == 1 && fabs(p.eta()) <= muonEtaMax_ && p.pt() >= muonEtMin_){ 
	   if (reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi()) < muonDRMatch_){
	    NOnMc++;
	    v->getMcEtOnMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcHisto()->Fill(p.eta(),p.phi());
	   }
	   if (NOn==1){
	    NOnMcUM++;
	    v->getMcEtOnMcUMHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcUMHisto()->Fill(p.eta(),p.phi());
	   }
	  }
	 }
	}


      }



      // for tau triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerTau || triggertype == trigger::TriggerL1TauJet)
	{

	  if (tauHandle.isValid()){
	    const reco::CaloTauCollection tauCollection = *(tauHandle.product());
         for (reco::CaloTauCollection::const_iterator tauIter=tauCollection.begin(); tauIter!=tauCollection.end(); tauIter++)
         {
	   if (reco::deltaR((*tauIter).eta(),(*tauIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < tauDRMatch_ && fabs((*tauIter).eta()) <= tauEtaMax_ && (*tauIter).pt() >= tauEtMin_ ){
	  NOnOff++;
	  v->getOffEtOnOffHisto()->Fill((*tauIter).pt());
	  v->getOffEtaVsOffPhiOnOffHisto()->Fill((*tauIter).eta(),(*tauIter).phi());
	   }
	   if (NOn==1 && fabs((*tauIter).eta()) <= tauEtaMax_ && (*tauIter).pt() >= tauEtMin_ ){
	  NOnOffUM++;
	  v->getOffEtOnOffUMHisto()->Fill((*tauIter).pt());
	  v->getOffEtaVsOffPhiOnOffUMHisto()->Fill((*tauIter).eta(),(*tauIter).phi());
	   }
         }}

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	    if (*idtypeiter == trigger::TriggerL1TauJet || *idtypeiter == trigger::TriggerL1ForJet)
	      {
	   if (reco::deltaR(toc[*l1ki].eta(),toc[*l1ki].phi(),toc[*ki].eta(),toc[*ki].phi()) < tauDRMatch_ && fabs(toc[*l1ki].eta()) <= tauEtaMax_ && toc[*l1ki].pt() >= tauEtMin_ )
            {
	     NL1On++;
	     v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
	   if (NOn==1 && fabs(toc[*l1ki].eta()) <= tauEtaMax_ && toc[*l1ki].pt() >= tauEtMin_ )
            {
	     NL1OnUM++;
	     v->getL1EtL1OnUMHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnUMHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
              }
	    ++idtypeiter;
	  }

	if (genParticles.isValid()){
           for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.pdgId()) == 15 && p.status() == 1 && fabs(p.eta()) <= tauEtaMax_ && p.pt() >= tauEtMin_ ){ 
	   if (reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi()) < tauDRMatch_ ){
	    NOnMc++;
	    v->getMcEtOnMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcHisto()->Fill(p.eta(),p.phi());
	   }
	   if (NOn==1){
	    NOnMcUM++;
	    v->getMcEtOnMcUMHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcUMHisto()->Fill(p.eta(),p.phi());
	   }
	  }
	 }
	}

      }


      // for jet triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerJet || triggertype == trigger::TriggerL1CenJet || triggertype == trigger::TriggerL1ForJet )
	{

	  if (jetHandle.isValid()){
         const reco::CaloJetCollection jetCollection = *(jetHandle.product());
         for (reco::CaloJetCollection::const_iterator jetIter=jetCollection.begin(); jetIter!=jetCollection.end(); jetIter++)
         {
	   if (reco::deltaR((*jetIter).eta(),(*jetIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < jetDRMatch_ && fabs((*jetIter).eta()) <= jetEtaMax_ && (*jetIter).pt() >= jetEtMin_ ){
	  NOnOff++;
	  v->getOffEtOnOffHisto()->Fill((*jetIter).pt());
	  v->getOffEtaVsOffPhiOnOffHisto()->Fill((*jetIter).eta(),(*jetIter).phi());
	   }
	   if (NOn==1 && fabs((*jetIter).eta()) <= jetEtaMax_ && (*jetIter).pt() >= jetEtMin_ ){
	  NOnOffUM++;
	  v->getOffEtOnOffUMHisto()->Fill((*jetIter).pt());
	  v->getOffEtaVsOffPhiOnOffUMHisto()->Fill((*jetIter).eta(),(*jetIter).phi());
	   }
         }}

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	    if (*idtypeiter == trigger::TriggerL1TauJet || *idtypeiter == trigger::TriggerL1ForJet || *idtypeiter == trigger::TriggerL1CenJet)
	      {
	   if (reco::deltaR(toc[*l1ki].eta(),toc[*l1ki].phi(),toc[*ki].eta(),toc[*ki].phi()) < jetDRMatch_ && fabs(toc[*l1ki].eta()) <= jetEtaMax_ && toc[*l1ki].pt() >= jetEtMin_ )
            {
	     NL1On++;
	     v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
	   if (NOn==1 && fabs(toc[*l1ki].eta()) <= jetEtaMax_ && toc[*l1ki].pt() >= jetEtMin_ )
            {
	     NL1OnUM++;
	     v->getL1EtL1OnUMHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnUMHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
              }
	    ++idtypeiter;
	  }

	if (genJets.isValid()){
          for(GenJetCollection::const_iterator gjet=genJets->begin(); gjet!=genJets->end(); gjet++) {
          if (fabs(gjet->eta()) <= jetEtaMax_ && gjet->pt() >= jetEtMin_ ){ 
	   if (reco::deltaR(gjet->eta(),gjet->phi(),toc[*ki].eta(),toc[*ki].phi()) < jetDRMatch_){
	    NOnMc++;
	    v->getMcEtOnMcHisto()->Fill(gjet->pt());
	    v->getMcEtaVsMcPhiOnMcHisto()->Fill(gjet->eta(),gjet->phi());
	   }
	   if (NOn==1){
	    NOnMcUM++;
	    v->getMcEtOnMcUMHisto()->Fill(gjet->pt());
	    v->getMcEtaVsMcPhiOnMcUMHisto()->Fill(gjet->eta(),gjet->phi());
	   }
	  }
	 }
	}

      }

      // for bjet triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerBJet)
	{

	  if (v->getPath().find("BTagIP") != std::string::npos && bTagIPHandle.isValid()){
          const reco::JetTagCollection & bTags = *(bTagIPHandle.product());
          for (size_t i = 0; i != bTags.size(); ++i) {
           edm::RefToBase<reco::Jet>  BRefJet=bTags[i].first;
           
 	   if (fabs(BRefJet->eta()) <= bjetEtaMax_ && BRefJet->pt() >= bjetEtMin_ ){
	   if (reco::deltaR(BRefJet->eta(),BRefJet->phi(),toc[*ki].eta(),toc[*ki].phi()) < bjetDRMatch_){
	      NOnOff++;
	      v->getOffEtOnOffHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiOnOffHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	    }
	   if (NOn==1){
	      NOnOffUM++;
	      v->getOffEtOnOffUMHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiOnOffUMHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	    }
	   }
	  }
	}


	  if (v->getPath().find("BTagMu") != std::string::npos && bTagMuHandle.isValid()){
          const reco::JetTagCollection & bTags = *(bTagMuHandle.product());
          for (size_t i = 0; i != bTags.size(); ++i) {
           edm::RefToBase<reco::Jet>  BRefJet=bTags[i].first;
           
 	   if (fabs(BRefJet->eta()) <= bjetEtaMax_ && BRefJet->pt() >= bjetEtMin_ ){
	   if (reco::deltaR(BRefJet->eta(),BRefJet->phi(),toc[*ki].eta(),toc[*ki].phi()) < bjetDRMatch_){
	      NOnOff++;
	      v->getOffEtOnOffHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiOnOffHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	    }
	   if (NOn==1){
	      NOnOffUM++;
	      v->getOffEtOnOffUMHisto()->Fill(BRefJet->pt());
	      v->getOffEtaVsOffPhiOnOffUMHisto()->Fill(BRefJet->eta(),BRefJet->phi());
	    }
	   }
	  }
	}

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	    if (*idtypeiter == trigger::TriggerL1ForJet || *idtypeiter == trigger::TriggerL1CenJet)
	      {
	   if (reco::deltaR(toc[*l1ki].eta(),toc[*l1ki].phi(),toc[*ki].eta(),toc[*ki].phi()) < bjetDRMatch_ && fabs(toc[*l1ki].eta()) <= bjetEtaMax_ && toc[*l1ki].pt() >= bjetEtMin_ )
            {
	     NL1On++;
	     v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
	   if (NOn==1 && fabs(toc[*l1ki].eta()) <= bjetEtaMax_ && toc[*l1ki].pt() >= bjetEtMin_ )
            {
	     NL1OnUM++;
	     v->getL1EtL1OnUMHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnUMHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
              }
	    ++idtypeiter;
	  }

	if (genJets.isValid()){
          for(GenJetCollection::const_iterator gjet=genJets->begin(); gjet!=genJets->end(); gjet++) {
          if (fabs(gjet->eta()) <= bjetEtaMax_ && gjet->pt() >= bjetEtMin_ ){
            bool bjet = false;
	    if (genParticles.isValid()){
               for(size_t i = 0; i < genParticles->size(); ++ i) {
              const GenParticle & p = (*genParticles)[i];
              if (abs(p.pdgId()) == 5){ 
	       if (reco::deltaR(p.eta(),p.phi(),gjet->eta(),gjet->phi()) < 0.7){
		 bjet = true;
	       }
	      }
	     }
	    }
	    if (bjet){ 
	   if (reco::deltaR(gjet->eta(),gjet->phi(),toc[*ki].eta(),toc[*ki].phi()) < bjetDRMatch_){
	    NOnMc++;
	    v->getMcEtOnMcHisto()->Fill(gjet->pt());
	    v->getMcEtaVsMcPhiOnMcHisto()->Fill(gjet->eta(),gjet->phi());
	   }
	   if (NOn==1){
	    NOnMcUM++;
	    v->getMcEtOnMcUMHisto()->Fill(gjet->pt());
	    v->getMcEtaVsMcPhiOnMcUMHisto()->Fill(gjet->eta(),gjet->phi());
	   }
	  }
	 }
	}
       }

	}
      // for met triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerMET || triggertype == trigger::TriggerL1ETM )
	{

	  if (metHandle.isValid()){
         const reco::CaloMETCollection metCollection = *(metHandle.product());
         for (reco::CaloMETCollection::const_iterator metIter=metCollection.begin(); metIter!=metCollection.end(); metIter++)
         {
	   //   if (reco::deltaR((*metIter).eta(),(*metIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < 0.3 && (*metIter).pt() >= metMin_ ){
	  NOnOff++;
	  v->getOffEtOnOffHisto()->Fill((*metIter).pt());
	  v->getOffEtaVsOffPhiOnOffHisto()->Fill((*metIter).eta(),(*metIter).phi());
	  //   }
         }}

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	    if (*idtypeiter == trigger::TriggerL1ETM)
	      {
		//   if (reco::deltaR(toc[*l1ki].eta(),toc[*l1ki].phi(),toc[*ki].eta(),toc[*ki].phi()) < 0.3 && toc[*l1ki].pt() >= metMin_ )
		// {
	     NL1On++;
	     v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	     // }
              }
	    ++idtypeiter;
	  }

	  if (genMets.isValid())
	  {
           for(GenMETCollection::const_iterator gmet=genMets->begin(); gmet!=genMets->end(); gmet++) {
	     if (gmet->pt() > metMin_){
	    NOnMc++;
            v->getMcEtOnMcHisto()->Fill(gmet->pt());
	    v->getMcEtaVsMcPhiOnMcHisto()->Fill(0.0,gmet->phi());
	     }
	   }
	  }

      }
      // for sumet triggers, loop over and fill offline 4-vectors
      else if (triggertype == trigger::TriggerTET || triggertype == trigger::TriggerL1ETT )
	{

	  if (metHandle.isValid()){
         const reco::CaloMETCollection metCollection = *(metHandle.product());
         for (reco::CaloMETCollection::const_iterator metIter=metCollection.begin(); metIter!=metCollection.end(); metIter++)
         {
	   if ((*metIter).sumEt() >= sumEtMin_ ){
	  NOnOff++;
	  v->getOffEtOnOffHisto()->Fill((*metIter).sumEt());
	  v->getOffEtaVsOffPhiOnOffHisto()->Fill(0.0,0.0);
	   }
         }}

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	    if (*idtypeiter == trigger::TriggerL1ETT)
	      {
	   if (toc[*l1ki].pt() >= sumEtMin_ )
            {
	     NL1On++;
	     v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
              }
	    ++idtypeiter;
	  }



	  if (genMets.isValid())
	  {
           for(GenMETCollection::const_iterator gmet=genMets->begin(); gmet!=genMets->end(); gmet++) {
	     if (gmet->sumEt() > sumEtMin_){
	    NOnMc++;
            v->getMcEtOnMcHisto()->Fill(gmet->sumEt());
	    v->getMcEtaVsMcPhiOnMcHisto()->Fill(0.0,0.0);
	     }
	   }
	  }

      }


      // for photon triggers, loop over and fill offline and L1 4-vectors
      else if (triggertype == trigger::TriggerPhoton)
	{

	  if (photonHandle.isValid()){
          const reco::PhotonCollection photonCollection = *(photonHandle.product());
         for (reco::PhotonCollection::const_iterator photonIter=photonCollection.begin(); photonIter!=photonCollection.end(); photonIter++)
         {
	   if (reco::deltaR((*photonIter).eta(),(*photonIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < photonDRMatch_ && fabs((*photonIter).eta()) <= photonEtaMax_ && (*photonIter).pt() >= photonEtMin_ ){
	  NOnOff++;
	  v->getOffEtOnOffHisto()->Fill((*photonIter).pt());
	  v->getOffEtaVsOffPhiOnOffHisto()->Fill((*photonIter).eta(),(*photonIter).phi());
	   }
	   if (NOn==1 && fabs((*photonIter).eta()) <= photonEtaMax_ && (*photonIter).pt() >= photonEtMin_ ){
	  NOnOffUM++;
	  v->getOffEtOnOffUMHisto()->Fill((*photonIter).pt());
	  v->getOffEtaVsOffPhiOnOffUMHisto()->Fill((*photonIter).eta(),(*photonIter).phi());
	   }
         }}
	

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	    if (*idtypeiter == trigger::TriggerL1IsoEG || *idtypeiter == trigger::TriggerL1NoIsoEG)
	      {
	   if (reco::deltaR(toc[*l1ki].eta(),toc[*l1ki].phi(),toc[*ki].eta(),toc[*ki].phi()) < photonDRMatch_ && fabs(toc[*l1ki].eta()) <= photonEtaMax_ && toc[*l1ki].pt() >= photonEtMin_ )
            {
	     NL1On++;
	     v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
	   if (NOn==1 && fabs(toc[*l1ki].eta()) <= photonEtaMax_ && toc[*l1ki].pt() >= photonEtMin_ )
            {
	     NL1OnUM++;
	     v->getL1EtL1OnUMHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnUMHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
              }
	    ++idtypeiter;
	  }

	if (genParticles.isValid()){
           for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.pdgId()) == 22 && p.status() == 1 && fabs(p.eta()) <= photonEtaMax_ && p.pt() >= photonEtMin_ ){ 
	   if (reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi()) < photonDRMatch_){
	    NOnMc++;
	    v->getMcEtOnMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcHisto()->Fill(p.eta(),p.phi());
	   }
	   if (NOn==1){
	    NOnMcUM++;
	    v->getMcEtOnMcUMHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcUMHisto()->Fill(p.eta(),p.phi());
	   }
	  }
	 }
	}
       

	}// photon trigger type


      // for track triggers, loop over and fill offline and L1 4-vectors
      else if (triggertype == trigger::TriggerTrack)
	{

	  if (trackHandle.isValid()){
          const reco::TrackCollection trackCollection = *(trackHandle.product());
         for (reco::TrackCollection::const_iterator trackIter=trackCollection.begin(); trackIter!=trackCollection.end(); trackIter++)
         {
	   if (reco::deltaR((*trackIter).eta(),(*trackIter).phi(),toc[*ki].eta(),toc[*ki].phi()) < trackDRMatch_ && fabs((*trackIter).eta()) <= trackEtaMax_ && (*trackIter).pt() >= trackEtMin_ ){
	  NOnOff++;
	  v->getOffEtOnOffHisto()->Fill((*trackIter).pt());
	  v->getOffEtaVsOffPhiOnOffHisto()->Fill((*trackIter).eta(),(*trackIter).phi());
	   }
	   if (NOn==1 && fabs((*trackIter).eta()) <= trackEtaMax_ && (*trackIter).pt() >= trackEtMin_ ){
	  NOnOffUM++;
	  v->getOffEtOnOffUMHisto()->Fill((*trackIter).pt());
	  v->getOffEtaVsOffPhiOnOffUMHisto()->Fill((*trackIter).eta(),(*trackIter).phi());
	   }
         }}
	

          trigger::Vids::const_iterator idtypeiter = idtype.begin(); 
          for (trigger::Keys::const_iterator l1ki = l1k.begin(); l1ki !=l1k.end(); ++l1ki ) {
	    if (*idtypeiter == trigger::TriggerL1CenJet || *idtypeiter == trigger::TriggerL1ForJet || *idtypeiter == trigger::TriggerL1TauJet)
	      {
	   if (reco::deltaR(toc[*l1ki].eta(),toc[*l1ki].phi(),toc[*ki].eta(),toc[*ki].phi()) < trackDRMatch_ && fabs(toc[*l1ki].eta()) <= trackEtaMax_ && toc[*l1ki].pt() >= trackEtMin_ )
            {
	     NL1On++;
	     v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
	   if (NOn==1 && fabs(toc[*l1ki].eta()) <= trackEtaMax_ && toc[*l1ki].pt() >= trackEtMin_ )
            {
	     NL1OnUM++;
	     v->getL1EtL1OnUMHisto()->Fill(toc[*l1ki].pt());
	     v->getL1EtaVsL1PhiL1OnUMHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());
	    }
              }
	    ++idtypeiter;
	  }

	if (genParticles.isValid()){
           for(size_t i = 0; i < genParticles->size(); ++ i) {
          const GenParticle & p = (*genParticles)[i];
          if (abs(p.charge()) > 0 && p.status() == 1 && fabs(p.eta()) <= trackEtaMax_ && p.pt() >= trackEtMin_ ){ 
	   if (reco::deltaR(p.eta(),p.phi(),toc[*ki].eta(),toc[*ki].phi()) < trackDRMatch_){
	    NOnMc++;
	    v->getMcEtOnMcHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcHisto()->Fill(p.eta(),p.phi());
	   }
	   if (NOn==1){
	    NOnMcUM++;
	    v->getMcEtOnMcUMHisto()->Fill(p.pt());
	    v->getMcEtaVsMcPhiOnMcUMHisto()->Fill(p.eta(),p.phi());
	   }
	  }
	 }
	}
       

	}// track trigger type

    */
      } //online object loop

      eleMon.fillOnlineMatch(this, l1k, toc);

      muoMon.fillOnlineMatch(this, l1k, toc);

      tauMon.fillOnlineMatch(this, l1k, toc);

      phoMon.fillOnlineMatch(this, l1k, toc);

      jetMon.fillOnlineMatch(this, l1k, toc);

      bjetMon.fillOnlineMatch(this, l1k, toc);


  		eleMon.monitorOffline(this);

  		muoMon.monitorOffline(this);
  
  		tauMon.monitorOffline(this);

  		phoMon.monitorOffline(this);

  		jetMon.monitorOffline(this);

  		bjetMon.monitorOffline(this);


  		eleMon.fillOffMatch(this);

  		muoMon.fillOffMatch(this);
  
  		tauMon.fillOffMatch(this);

  		phoMon.fillOffMatch(this);

  		jetMon.fillOffMatch(this);

  		bjetMon.fillOffMatch(this);

	/*
	// clean the set On-Off
	if(eleOnOffDRMatchSet.size() > 1) cleanDRMatchSet(eleOnOffDRMatchSet);

	// now fill histos
        for ( mmset::iterator setIter = eleOnOffDRMatchSet.begin( ); setIter != eleOnOffDRMatchSet.end( ); setIter++ ) 
        {

         fimmap tempMap = *setIter;
           
         fimmap::iterator it = tempMap.begin(); 
         int i  = (*it).second ;

         reco::PixelMatchGsfElectronCollection::const_iterator gsfIter=gsfElectrons->begin();
         for (int count = 0; count < i; count++) gsfIter++;
         //LogTrace("FourVectorHLTriggerOffline") << " Filling Electron OnOff   electronOnOffDRMatchMap.insert(std::pair<float,int>(dR,j)): pt = " << gsfIter->pt() << endl;

	 NOnOff++;
	 v->getOffEtOnOffHisto()->Fill(gsfIter->pt());
	 v->getOffEtaVsOffPhiOnOffHisto()->Fill(gsfIter->eta(), gsfIter->phi());

	}

	// clean the set On-L1
	if(eleOnL1DRMatchSet.size() > 1) cleanDRMatchSet(eleOnL1DRMatchSet);
	// now fill histos
        for ( mmset::iterator setIter = eleOnL1DRMatchSet.begin( ); setIter != eleOnL1DRMatchSet.end( ); setIter++ ) 
        {

         fimmap tempMap = *setIter;
           
         fimmap::iterator it = tempMap.begin(); 
         int i  = (*it).second ;
         trigger::Keys::const_iterator l1ki = l1k.begin();
         for (int count = 0; count < i; count++) l1ki++;

	  NL1On++;
	  v->getL1EtL1OnHisto()->Fill(toc[*l1ki].pt());
	  v->getL1EtaVsL1PhiL1OnHisto()->Fill(toc[*l1ki].eta(),toc[*l1ki].phi());

	}

	// clean the set On-MC
	if(eleOnMCDRMatchSet.size() > 1) cleanDRMatchSet(eleOnMCDRMatchSet);
	// now fill histos
        for ( mmset::iterator setIter = eleOnMCDRMatchSet.begin( ); setIter != eleOnMCDRMatchSet.end( ); setIter++ ) 
        {

         fimmap tempMap = *setIter;
           
         fimmap::iterator it = tempMap.begin(); 
         int i  = (*it).second ;
              
         const GenParticle & p = (*genParticles)[i];
         NOnMc++;
         v->getMcEtOnMcHisto()->Fill(p.pt());
         v->getMcEtaVsMcPhiOnMcHisto()->Fill(p.eta(),p.phi());

	}
	*/


      v->getNOnHisto()->Fill(NOn);      
      v->getNL1OnHisto()->Fill(NL1On);      
      v->getNOnOffHisto()->Fill(NOnOff);
      v->getNOffMcHisto()->Fill(NOffMc);
      v->getNOnMcHisto()->Fill(NOnMc);
      v->getNL1OnUMHisto()->Fill(NL1OnUM);      
      v->getNOnOffUMHisto()->Fill(NOnOffUM);
      v->getNOffMcUMHisto()->Fill(NOffMcUM);
      v->getNOnMcUMHisto()->Fill(NOnMcUM);
  
			/*
			*/

    } //numpassed

		/*
		// Lets try it
	  if(eleMon.L1OffDRMatchSet.size() > 1) cleanDRMatchSet(eleMon.L1OffDRMatchSet);

	  eleL1OffDRMatchSet = eleMon.L1OffDRMatchSet;
	  // clean the set L1-Off
	  if(eleL1OffDRMatchSet.size() > 1) cleanDRMatchSet(eleL1OffDRMatchSet);
	  // now fill histos
    for ( mmset::iterator setIter = eleL1OffDRMatchSet.begin( ); setIter != eleL1OffDRMatchSet.end( ); setIter++ ) 
    {

         fimmap tempMap = *setIter;
           
         fimmap::iterator it = tempMap.begin(); 
         int i  = (*it).second ;

         reco::PixelMatchGsfElectronCollection::const_iterator gsfIter=gsfElectrons->begin();
         for (int count = 0; count < i; count++) gsfIter++;

         //LogTrace("FourVectorHLTriggerOffline") << " Filling Electron L1Off   electronL1OffDRMatchMap.insert(std::pair<float,int>(dR,j)): pt = " << gsfIter->pt() << endl;

	       NL1Off++;
	       v->getOffEtL1OffHisto()->Fill((*gsfIter).pt());
	       v->getOffEtaVsOffPhiL1OffHisto()->Fill((*gsfIter).eta(),(*gsfIter).phi());

	    }

	    eleL1MCDRMatchSet = eleMon.L1MCDRMatchSet;
	    // clean the set L1-MC
	    if(eleL1MCDRMatchSet.size() > 1) cleanDRMatchSet(eleL1MCDRMatchSet);
	    // now fill histos
      for ( mmset::iterator setIter = eleL1MCDRMatchSet.begin( ); setIter != eleL1MCDRMatchSet.end( ); setIter++ ) 
      {

         fimmap tempMap = *setIter;
           
         fimmap::iterator it = tempMap.begin(); 
         int i  = (*it).second ;
              
         const GenParticle & p = (*genParticles)[i];
	       NL1Mc++;
	       v->getMcEtL1McHisto()->Fill(p.pt());
	       v->getMcEtaVsMcPhiL1McHisto()->Fill(p.eta(),p.phi());

	    }
			*/
    

    } //denompassed

		/*
		if(v->getPath() == "HLT_Ele10_SW_L1R")
		{
	   
		 //LogTrace("FourVectorHLTriggerOffline") << "HLT_Ele10_SW_L1R Et, content bin 14:  Off = " << v->getOffEtOffHisto()->getTH1()->GetBinContent(14) << " OnOff = " << v->getOffEtOnOffHisto()->getTH1()->GetBinContent(14) << endl;
		 //LogTrace("FourVectorHLTriggerOffline") << "                     content bin 15:  Off = " << v->getOffEtOffHisto()->getTH1()->GetBinContent(15) << " OnOff = " << v->getOffEtOnOffHisto()->getTH1()->GetBinContent(15) << endl;
		 LogTrace("FourVectorHLTriggerOffline") << "HLT_Ele10_SW_L1R Et, content bin 12:  Off = " << v->getOffEtOffHisto()->getTH1()->GetBinContent(12) << " L1OffUM = " << v->getOffEtL1OffUMHisto()->getTH1()->GetBinContent(12) << endl;
		 LogTrace("FourVectorHLTriggerOffline") << "                     content bin 13:  Off = " << v->getOffEtOffHisto()->getTH1()->GetBinContent(13) << " L1OffUM = " << v->getOffEtL1OffUMHisto()->getTH1()->GetBinContent(13) << endl;

		}
		*/
  } //pathinfo loop

}



// -- method called once each job just before starting event loop  --------
void 
FourVectorHLTriggerOffline::beginJob(const edm::EventSetup&)
{
  nev_ = 0;
  DQMStore *dbe = 0;
  dbe = Service<DQMStore>().operator->();
  
  if (dbe) {
    dbe->setCurrentFolder(dirname_);
    dbe->rmdir(dirname_);
  }
  
  
  if (dbe) {
    dbe->setCurrentFolder(dirname_);
    }  
}

// - method called once each job just after ending the event loop  ------------
void 
FourVectorHLTriggerOffline::endJob() 
{
   LogInfo("FourVectorHLTriggerOffline") << "analyzed " << nev_ << " events";
   return;
}


// BeginRun
void FourVectorHLTriggerOffline::beginRun(const edm::Run& run, const edm::EventSetup& c)
{
  LogDebug("FourVectorHLTriggerOffline") << "beginRun, run " << run.id();
// HLT config does not change within runs!
 
  if (!hltConfig_.init(processname_)) {
    processname_ = "FU";
    if (!hltConfig_.init(processname_)){
  LogDebug("FourVectorHLTriggerOffline") << "HLTConfigProvider failed to initialize.";
    }
    // check if trigger name in (new) config
    //	cout << "Available TriggerNames are: " << endl;
    //	hltConfig_.dump("Triggers");
      }


  if (1)
 {
  DQMStore *dbe = 0;
  dbe = Service<DQMStore>().operator->();
  
  if (dbe) {
    dbe->setCurrentFolder(dirname_);
  }


    const unsigned int n(hltConfig_.size());
    if (plotAll_){
    for (unsigned int j=0; j!=n; ++j) {
    std::string pathname = hltConfig_.triggerName(j);  
    std::string l1pathname = "dummy";
    for (unsigned int i=0; i!=n; ++i) {
      // cout << hltConfig_.triggerName(i) << endl;
    
    std::string denompathname = hltConfig_.triggerName(i);  
    int objectType = 0;
    int denomobjectType = 0;
    //parse pathname to guess object type
    if (pathname.find("MET") != std::string::npos) 
      objectType = trigger::TriggerMET;    
    if (pathname.find("SumET") != std::string::npos) 
      objectType = trigger::TriggerTET;    
    if (pathname.find("HT") != std::string::npos) 
      objectType = trigger::TriggerTET;    
    if (pathname.find("Jet") != std::string::npos) 
      objectType = trigger::TriggerJet;    
    if (pathname.find("BTag") != std::string::npos) 
      objectType = trigger::TriggerBJet;    
    if (pathname.find("Mu") != std::string::npos &&
	 	    pathname.find("BTag") == std::string::npos) 
      objectType = trigger::TriggerMuon;    
    if (pathname.find("Ele") != std::string::npos) 
      objectType = trigger::TriggerElectron;    
    if (pathname.find("Photon") != std::string::npos) 
      objectType = trigger::TriggerPhoton;    
    if (pathname.find("Tau") != std::string::npos) 
      objectType = trigger::TriggerTau;    
    if (pathname.find("IsoTrack") != std::string::npos) 
      objectType = trigger::TriggerTrack;    

    //parse denompathname to guess denomobject type
    if (denompathname.find("MET") != std::string::npos) 
      denomobjectType = trigger::TriggerMET;    
    if (denompathname.find("SumET") != std::string::npos) 
      denomobjectType = trigger::TriggerTET;    
    if (denompathname.find("HT") != std::string::npos) 
      denomobjectType = trigger::TriggerTET;    
    if (denompathname.find("Jet") != std::string::npos) 
      denomobjectType = trigger::TriggerJet;    
    if (denompathname.find("BTag") != std::string::npos) 
      denomobjectType = trigger::TriggerBJet;    
    if (pathname.find("Mu") != std::string::npos &&
	 	    pathname.find("BTag") == std::string::npos) 
      denomobjectType = trigger::TriggerMuon;    
    if (denompathname.find("Ele") != std::string::npos) 
      denomobjectType = trigger::TriggerElectron;    
    if (denompathname.find("Photon") != std::string::npos) 
      denomobjectType = trigger::TriggerPhoton;    
    if (denompathname.find("Tau") != std::string::npos) 
      denomobjectType = trigger::TriggerTau;    
    if (denompathname.find("IsoTrack") != std::string::npos) 
      denomobjectType = trigger::TriggerTrack;    

    // find L1 condition for numpath with numpath objecttype 

    // find PSet for L1 global seed for numpath, 
    // list module labels for numpath
    std::vector<std::string> numpathmodules = hltConfig_.moduleLabels(pathname);

            for(std::vector<std::string>::iterator numpathmodule = numpathmodules.begin();
    	  numpathmodule!= numpathmodules.end(); ++numpathmodule ) {
	      //  cout << pathname << "\t" << *numpathmodule << "\t" << hltConfig_.moduleType(*numpathmodule) << endl;
	      if (hltConfig_.moduleType(*numpathmodule) == "HLTLevel1GTSeed")
		{
		  edm::ParameterSet l1GTPSet = hltConfig_.modulePSet(*numpathmodule);
		  //                  cout << l1GTPSet.getParameter<std::string>("L1SeedsLogicalExpression") << endl;
		  //  l1pathname = l1GTPSet.getParameter<std::string>("L1SeedsLogicalExpression");
                  l1pathname = *numpathmodule; 
                  break; 
		}
    	} 
   
    



    std::string filtername("dummy");
    float ptMin = 0.0;
    float ptMax = 100.0;
    if (plotAll_ && denomobjectType == objectType && objectType != 0)
    hltPaths_.push_back(PathInfo(denompathname, pathname, l1pathname, filtername, processname_, objectType, ptMin, ptMax));

    }
    }

    }
    else
    {
     // plot all diagonal combinations plus any other specified pairs
    for (unsigned int i=0; i!=n; ++i) {
      std::string denompathname = "";  
      std::string pathname = hltConfig_.triggerName(i);  
      std::string l1pathname = "dummy";
      int objectType = 0;
      int denomobjectType = 0;
    //parse pathname to guess object type
    if (pathname.find("MET") != std::string::npos) 
      objectType = trigger::TriggerMET;    
    if (pathname.find("SumET") != std::string::npos) 
      objectType = trigger::TriggerTET;    
    if (pathname.find("HT") != std::string::npos) 
      objectType = trigger::TriggerTET;    
    if (pathname.find("Jet") != std::string::npos) 
      objectType = trigger::TriggerJet;    
    if (pathname.find("BTag") != std::string::npos) 
      objectType = trigger::TriggerBJet;    
    if (pathname.find("Mu") != std::string::npos &&
	 	    pathname.find("BTag") == std::string::npos) 
      objectType = trigger::TriggerMuon;    
    if (pathname.find("Ele") != std::string::npos) 
      objectType = trigger::TriggerElectron;    
    if (pathname.find("Photon") != std::string::npos) 
      objectType = trigger::TriggerPhoton;    
    if (pathname.find("Tau") != std::string::npos) 
      objectType = trigger::TriggerTau;    
    if (pathname.find("IsoTrack") != std::string::npos) 
      objectType = trigger::TriggerTrack;    

    //parse denompathname to guess denomobject type
    if (denompathname.find("MET") != std::string::npos) 
      denomobjectType = trigger::TriggerMET;    
    if (denompathname.find("SumET") != std::string::npos) 
      denomobjectType = trigger::TriggerTET;    
    if (denompathname.find("HT") != std::string::npos) 
      denomobjectType = trigger::TriggerTET;    
    if (denompathname.find("Jet") != std::string::npos) 
      denomobjectType = trigger::TriggerJet;    
    if (denompathname.find("BTag") != std::string::npos) 
      denomobjectType = trigger::TriggerBJet;    
    if (pathname.find("Mu") != std::string::npos &&
	 	    pathname.find("BTag") == std::string::npos) 
      denomobjectType = trigger::TriggerMuon;    
    if (denompathname.find("Ele") != std::string::npos) 
      denomobjectType = trigger::TriggerElectron;    
    if (denompathname.find("Photon") != std::string::npos) 
      denomobjectType = trigger::TriggerPhoton;    
    if (denompathname.find("Tau") != std::string::npos) 
      denomobjectType = trigger::TriggerTau;    
    if (denompathname.find("IsoTrack") != std::string::npos) 
      denomobjectType = trigger::TriggerTrack;    
    // find L1 condition for numpath with numpath objecttype 

    // find PSet for L1 global seed for numpath, 
    // list module labels for numpath
    std::vector<std::string> numpathmodules = hltConfig_.moduleLabels(pathname);

    for(std::vector<std::string>::iterator numpathmodule = numpathmodules.begin();
    	  numpathmodule!= numpathmodules.end(); ++numpathmodule ) {
	      //  cout << pathname << "\t" << *numpathmodule << "\t" << hltConfig_.moduleType(*numpathmodule) << endl;
	      if (hltConfig_.moduleType(*numpathmodule) == "HLTLevel1GTSeed")
		{
		  edm::ParameterSet l1GTPSet = hltConfig_.modulePSet(*numpathmodule);
		  //                  cout << l1GTPSet.getParameter<std::string>("L1SeedsLogicalExpression") << endl;
                  //l1pathname = l1GTPSet.getParameter<std::string>("L1SeedsLogicalExpression"); 
                  l1pathname = *numpathmodule;
                  break; 
		}
    } 
   
    



    std::string filtername("dummy");
    float ptMin = 0.0;
    float ptMax = 100.0;
    if (objectType == trigger::TriggerPhoton) ptMax = 100.0;
    if (objectType == trigger::TriggerElectron) ptMax = 100.0;
    if (objectType == trigger::TriggerMuon) ptMax = 100.0;
    if (objectType == trigger::TriggerTau) ptMax = 100.0;
    if (objectType == trigger::TriggerJet) ptMax = 300.0;
    if (objectType == trigger::TriggerBJet) ptMax = 300.0;
    if (objectType == trigger::TriggerMET) ptMax = 300.0;
    if (objectType == trigger::TriggerTET) ptMax = 300.0;
    if (objectType == trigger::TriggerTrack) ptMax = 100.0;

    if (objectType != 0){
    hltPaths_.push_back(PathInfo(denompathname, pathname, l1pathname, filtername, processname_, objectType, ptMin, ptMax));
      //create folder for pathname
     }
    }
    // now loop over denom/num path pairs specified in cfg, 
    // recording the off-diagonal ones
    for (std::vector<std::pair<std::string, std::string> >::iterator custompathnamepair = custompathnamepairs_.begin(); custompathnamepair != custompathnamepairs_.end(); ++custompathnamepair)
    {
      if (custompathnamepair->first != custompathnamepair->second)
	{

      std::string denompathname = custompathnamepair->second;  
      std::string pathname = custompathnamepair->first;  
     
      // check that these exist
      bool foundfirst = false;
      bool foundsecond = false;
      for (unsigned int i=0; i!=n; ++i) {
	if (hltConfig_.triggerName(i) == denompathname) foundsecond = true;
	if (hltConfig_.triggerName(i) == pathname) foundfirst = true;
      } 
      if (!foundfirst)
	{
	  edm::LogInfo("FourVectorHLTriggerOffline") << "pathname not found, ignoring " << pathname;
          continue;
	}
      if (!foundsecond)
	{
	  edm::LogInfo("FourVectorHLTriggerOffline") << "denompathname not found, ignoring " << pathname;
          continue;
	}

     //cout << pathname << "\t" << denompathname << endl;
      std::string l1pathname = "dummy";
      int objectType = 0;
      //int denomobjectType = 0;
    //parse pathname to guess object type
    if (pathname.find("MET") != std::string::npos) 
      objectType = trigger::TriggerMET;    
    if (pathname.find("SumET") != std::string::npos) 
      objectType = trigger::TriggerTET;    
    if (pathname.find("HT") != std::string::npos) 
      objectType = trigger::TriggerTET;    
    if (pathname.find("Jet") != std::string::npos) 
      objectType = trigger::TriggerJet;    
    if (pathname.find("BTag") != std::string::npos) 
      objectType = trigger::TriggerBJet;    
    if (pathname.find("Mu") != std::string::npos &&
	 	    pathname.find("BTag") == std::string::npos) 
      objectType = trigger::TriggerMuon;    
    if (pathname.find("Ele") != std::string::npos) 
      objectType = trigger::TriggerElectron;    
    if (pathname.find("Photon") != std::string::npos) 
      objectType = trigger::TriggerPhoton;    
    if (pathname.find("Tau") != std::string::npos) 
      objectType = trigger::TriggerTau;    
    if (pathname.find("IsoTrack") != std::string::npos) 
      objectType = trigger::TriggerTrack;    
    // find L1 condition for numpath with numpath objecttype 

    // find PSet for L1 global seed for numpath, 
    // list module labels for numpath
  
    std::vector<std::string> numpathmodules = hltConfig_.moduleLabels(pathname);
    
    for(std::vector<std::string>::iterator numpathmodule = numpathmodules.begin();
    	  numpathmodule!= numpathmodules.end(); ++numpathmodule ) {
	      //  cout << pathname << "\t" << *numpathmodule << "\t" << hltConfig_.moduleType(*numpathmodule) << endl;
	      if (hltConfig_.moduleType(*numpathmodule) == "HLTLevel1GTSeed")
		{
		  edm::ParameterSet l1GTPSet = hltConfig_.modulePSet(*numpathmodule);
		  //                  cout << l1GTPSet.getParameter<std::string>("L1SeedsLogicalExpression") << endl;
		  // l1pathname = l1GTPSet.getParameter<std::string>("L1SeedsLogicalExpression");
                  l1pathname = *numpathmodule;
                  //cout << *numpathmodule << endl; 
                  break; 
		}
    }
    
    



    std::string filtername("dummy");
    float ptMin = 0.0;
    float ptMax = 100.0;
    if (objectType == trigger::TriggerPhoton) ptMax = 100.0;
    if (objectType == trigger::TriggerElectron) ptMax = 100.0;
    if (objectType == trigger::TriggerMuon) ptMax = 100.0;
    if (objectType == trigger::TriggerTau) ptMax = 100.0;
    if (objectType == trigger::TriggerJet) ptMax = 300.0;
    if (objectType == trigger::TriggerBJet) ptMax = 300.0;
    if (objectType == trigger::TriggerMET) ptMax = 300.0;
    if (objectType == trigger::TriggerTET) ptMax = 300.0;
    if (objectType == trigger::TriggerTrack) ptMax = 100.0;

    if (objectType != 0)
    hltPaths_.push_back(PathInfo(denompathname, pathname, l1pathname, filtername, processname_, objectType, ptMin, ptMax));
    
	}
    }

    }



    // now set up all of the histos for each path
    for(PathInfoCollection::iterator v = hltPaths_.begin();
	  v!= hltPaths_.end(); ++v ) {
    	MonitorElement *NOn, *onEtOn, *onEtavsonPhiOn=0;
    	MonitorElement *NMc, *mcEtMc, *mcEtavsmcPhiMc=0;
	MonitorElement *NOff, *offEtOff, *offEtavsoffPhiOff=0;
	MonitorElement *NL1, *l1EtL1, *l1Etavsl1PhiL1=0;
    	MonitorElement *NL1On, *l1EtL1On, *l1Etavsl1PhiL1On=0;
	MonitorElement *NL1Off, *offEtL1Off, *offEtavsoffPhiL1Off=0;
	MonitorElement *NOnOff, *offEtOnOff, *offEtavsoffPhiOnOff=0;
	MonitorElement *NL1Mc, *mcEtL1Mc, *mcEtavsmcPhiL1Mc=0;
	MonitorElement *NOffMc, *mcEtOffMc, *mcEtavsmcPhiOffMc=0;
	MonitorElement *NOnMc, *mcEtOnMc, *mcEtavsmcPhiOnMc=0;
    	MonitorElement *NL1OnUM, *l1EtL1OnUM, *l1Etavsl1PhiL1OnUM=0;
	MonitorElement *NL1OffUM, *offEtL1OffUM, *offEtavsoffPhiL1OffUM=0;
	MonitorElement *NOnOffUM, *offEtOnOffUM, *offEtavsoffPhiOnOffUM=0;
	MonitorElement *NL1McUM, *mcEtL1McUM, *mcEtavsmcPhiL1McUM=0;
	MonitorElement *NOffMcUM, *mcEtOffMcUM, *mcEtavsmcPhiOffMcUM=0;
	MonitorElement *NOnMcUM, *mcEtOnMcUM, *mcEtavsmcPhiOnMcUM=0;
  MonitorElement *mcDRL1Mc, *mcDROnMc, *mcDROffMc,
                   *offDRL1Off, *offDROnOff, *l1DRL1On=0;
	std::string labelname("dummy");
        labelname = v->getPath() + "_wrt_" + v->getDenomPath();
	std::string histoname(labelname+"_NOn");
	std::string title(labelname+" N online");



        double histEtaMax = 2.5;
        if (v->getObjectType() == trigger::TriggerMuon || v->getObjectType() == trigger::TriggerL1Mu) 
	  {
	    histEtaMax = muonEtaMax_;
	  }
        else if (v->getObjectType() == trigger::TriggerElectron || v->getObjectType() == trigger::TriggerL1NoIsoEG || v->getObjectType() == trigger::TriggerL1IsoEG )
	  {
	    histEtaMax = electronEtaMax_;
	  }
        else if (v->getObjectType() == trigger::TriggerTau || v->getObjectType() == trigger::TriggerL1TauJet )
	  {
	    histEtaMax = tauEtaMax_;
	  }
        else if (v->getObjectType() == trigger::TriggerJet || v->getObjectType() == trigger::TriggerL1CenJet || v->getObjectType() == trigger::TriggerL1ForJet )
	  {
	    histEtaMax = jetEtaMax_; 
	  }
        else if (v->getObjectType() == trigger::TriggerBJet)
	  {
	    histEtaMax = bjetEtaMax_;
	  }
        else if (v->getObjectType() == trigger::TriggerMET || v->getObjectType() == trigger::TriggerL1ETM )
	  {
	    histEtaMax = 5.0; 
	  }
        else if (v->getObjectType() == trigger::TriggerPhoton)
	  {
	    histEtaMax = photonEtaMax_; 
	  }
        else if (v->getObjectType() == trigger::TriggerTrack)
	  {
	    histEtaMax = trackEtaMax_; 
	  }

        TString pathfolder = dirname_ + TString("/") + v->getPath();
        dbe_->setCurrentFolder(pathfolder.Data());

	NOn =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NMc";
	title = labelname+" N Mc";
	NMc =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NOff";
	title = labelname+" N Off";
	NOff =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);
      
	histoname = labelname+"_NL1";
	title = labelname+" N L1";
	NL1 =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NL1On";
	title = labelname+" N L1On";
	NL1On =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NL1Off";
	title = labelname+" N L1Off";
	NL1Off =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NOnOff";
	title = labelname+" N OnOff";
	NOnOff =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NL1Mc";
	title = labelname+" N L1Mc";
	NL1Mc =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NOffMc";
	title = labelname+" N OffMc";
	NOffMc =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NOnMc";
	title = labelname+" N OnMc";
	NOnMc =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NL1OnUM";
	title = labelname+" N L1OnUM";
	NL1OnUM =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NL1OffUM";
	title = labelname+" N L1OffUM";
	NL1OffUM =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NOnOffUM";
	title = labelname+" N OnOffUM";
	NOnOffUM =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NL1McUM";
	title = labelname+" N L1McUM";
	NL1McUM =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NOffMcUM";
	title = labelname+" N OffMcUM";
	NOffMcUM =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_NOnMcUM";
	title = labelname+" N OnMcUM";
	NOnMcUM =  dbe->book1D(histoname.c_str(),
			  title.c_str(),10,
			  0.5,
			  10.5);

	histoname = labelname+"_mcEtMc";
	title = labelname+" mcE_t Mc";
	mcEtMc =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

        histoname = labelname+"_onEtOn";
	title = labelname+" onE_t online";
	onEtOn =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_offEtOff";
	title = labelname+" offE_t offline";
	offEtOff =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_l1EtL1";
	title = labelname+" l1E_t L1";
	l1EtL1 =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

        int nBins2D = 10;

	histoname = labelname+"_mcEtamcPhiMc";
	title = labelname+" mc#eta vs mc#phi Mc";
	mcEtavsmcPhiMc =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_onEtaonPhiOn";
	title = labelname+" on#eta vs on#phi online";
	onEtavsonPhiOn =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_offEtaoffPhiOff";
	title = labelname+" off#eta vs off#phi offline";
	offEtavsoffPhiOff =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_l1Etal1PhiL1";
	title = labelname+" l1#eta vs l1#phi L1";
	l1Etavsl1PhiL1 =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_l1EtL1On";
	title = labelname+" l1E_t L1+online";
	l1EtL1On =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_offEtL1Off";
	title = labelname+" offE_t L1+offline";
	offEtL1Off =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_offEtOnOff";
	title = labelname+" offE_t online+offline";
	offEtOnOff =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_mcEtL1Mc";
	title = labelname+" mcE_t L1+MC truth";
	mcEtL1Mc =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_mcEtOffMc";
	title = labelname+" mcE_t Off+MC truth";
	mcEtOffMc =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_mcEtOnMc";
	title = labelname+" mcE_t online+MC truth";
	mcEtOnMc =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_l1Etal1PhiL1On";
	title = labelname+" l1#eta vs l1#phi L1+online";
	l1Etavsl1PhiL1On =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_offEtaoffPhiL1Off";
	title = labelname+" off#eta vs off#phi L1+offline";
	offEtavsoffPhiL1Off =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_offEtaoffPhiOnOff";
	title = labelname+" off#eta vs off#phi online+offline";
	offEtavsoffPhiOnOff =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_mcEtamcPhiL1Mc";
	title = labelname+" mc#eta vs mc#phi L1+MC truth";
	mcEtavsmcPhiL1Mc =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_mcEtamcPhiOffMc";
	title = labelname+" mc#eta vs mc#phi Off+MC truth";
	mcEtavsmcPhiOffMc =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());


	histoname = labelname+"_mcEtamcPhiOnMc";
	title = labelname+" mc#eta vs mc#phi online+MC truth";
	mcEtavsmcPhiOnMc =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_l1EtL1OnUM";
	title = labelname+" l1E_t L1+onlineUM";
	l1EtL1OnUM =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_offEtL1OffUM";
	title = labelname+" offE_t L1+offlineUM";
	offEtL1OffUM =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_offEtOnOffUM";
	title = labelname+" offE_t online+offlineUM";
	offEtOnOffUM =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_mcEtL1McUM";
	title = labelname+" mcE_t L1+MC truthUM";
	mcEtL1McUM =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_mcEtOffMcUM";
	title = labelname+" mcE_t Off+MC truthUM";
	mcEtOffMcUM =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_mcEtOnMcUM";
	title = labelname+" mcE_t online+MC truthUM";
	mcEtOnMcUM =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 
                           v->getPtMin(),
			   v->getPtMax());

	histoname = labelname+"_l1Etal1PhiL1OnUM";
	title = labelname+" l1#eta vs l1#phi L1+onlineUM";
	l1Etavsl1PhiL1OnUM =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_offEtaoffPhiL1OffUM";
	title = labelname+" off#eta vs off#phi L1+offlineUM";
	offEtavsoffPhiL1OffUM =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_offEtaoffPhiOnOffUM";
	title = labelname+" off#eta vs off#phi online+offlineUM";
	offEtavsoffPhiOnOffUM =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_mcEtamcPhiL1McUM";
	title = labelname+" mc#eta vs mc#phi L1+MC truthUM";
	mcEtavsmcPhiL1McUM =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_mcEtamcPhiOffMcUM";
	title = labelname+" mc#eta vs mc#phi Off+MC truthUM";
	mcEtavsmcPhiOffMcUM =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_mcEtamcPhiOnMcUM";
	title = labelname+" mc#eta vs mc#phi online+MC truthUM";
	mcEtavsmcPhiOnMcUM =  dbe->book2D(histoname.c_str(),
				title.c_str(),
				nBins2D,-histEtaMax,histEtaMax,
				nBins2D,-TMath::Pi(), TMath::Pi());

	histoname = labelname+"_l1DRL1On";
	title = labelname+" l1DR L1+online";
	l1DRL1On =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 0, 1.);

	histoname = labelname+"_offDRL1Off";
	title = labelname+" offDR L1+offline";
	offDRL1Off =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 0, 1.);

	histoname = labelname+"_offDROnOff";
	title = labelname+" offDR online+offline";
	offDROnOff =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 0, 1.);

	histoname = labelname+"_mcDRL1Mc";
	title = labelname+" mcDR L1+MC truth";
	mcDRL1Mc =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 0, 1.);

	histoname = labelname+"_mcDROffMc";
	title = labelname+" mcDR Off+MC truth";
	mcDROffMc =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 0, 1.);

	histoname = labelname+"_mcDROnMc";
	title = labelname+" mcDR online+MC truth";
	mcDROnMc =  dbe->book1D(histoname.c_str(),
			   title.c_str(),nBins_, 0, 1.);


	v->setHistos( NMc, mcEtMc, mcEtavsmcPhiMc, NOn, onEtOn, onEtavsonPhiOn, NOff, offEtOff, offEtavsoffPhiOff, NL1, l1EtL1, l1Etavsl1PhiL1, NL1On, l1EtL1On, l1Etavsl1PhiL1On, NL1Off, offEtL1Off, offEtavsoffPhiL1Off, NOnOff, offEtOnOff, offEtavsoffPhiOnOff, NL1Mc, mcEtL1Mc, mcEtavsmcPhiL1Mc, NOffMc, mcEtOffMc, mcEtavsmcPhiOffMc, NOnMc, mcEtOnMc, mcEtavsmcPhiOnMc, NL1OnUM, l1EtL1OnUM, l1Etavsl1PhiL1OnUM, NL1OffUM, offEtL1OffUM, offEtavsoffPhiL1OffUM, NOnOffUM, offEtOnOffUM, offEtavsoffPhiOnOffUM, NL1McUM, mcEtL1McUM, mcEtavsmcPhiL1McUM, NOffMcUM, mcEtOffMcUM, mcEtavsmcPhiOffMcUM, NOnMcUM, mcEtOnMcUM, mcEtavsmcPhiOnMcUM, mcDRL1Mc, mcDROnMc, mcDROffMc, offDRL1Off, offDROnOff, l1DRL1On
);


    }
 }
 return;



}

/// EndRun
void FourVectorHLTriggerOffline::endRun(const edm::Run& run, const edm::EventSetup& c)
{
  LogDebug("FourVectorHLTriggerOffline") << "endRun, run " << run.id();
}

/// Clean DR Match Set
void FourVectorHLTriggerOffline::cleanDRMatchSet(mmset& tempSet)
{

 LogDebug("FourVectorHLTriggerOffline") << "cleanDRMatchSet(mmset& tempSet) " << endl;
 LogDebug("FourVectorHLTriggerOffline") << "size of the set (before CLEANED)= " << tempSet.size() << " maps." << endl;

 if(tempSet.size() < 2) return;
 
 bool cleanedOneMap = false;
 
 // cleaning needed if the set has at least two maps

 while(! cleanedOneMap && tempSet.size() > 1) {

 cleanedOneMap=false;

 //LogTrace("FourVectorHLTriggerOffline") << "cleaning: size of the set  = " << tempSet.size() << " maps." << endl;

 int imap = 0;
 for ( mmset::iterator setIter_i = tempSet.begin( ); setIter_i != tempSet.end( ); setIter_i++ ) 
 {

      fimmap tempMap_j = *setIter_i;

      //LogTrace("FourVectorHLTriggerOffline") << " map " << imap << endl;
      //LogTrace("FourVectorHLTriggerOffline") << " --------" << endl;
      for (fimmap::iterator it = tempMap_j.begin(); it != tempMap_j.end(); ++it)
      {

        //LogTrace("FourVectorHLTriggerOffline") << " " <<   (*it).first << " :  " << (*it).second << endl;

      }

      imap++;

 }

 // loop i
 for ( mmset::iterator setIter_i = tempSet.begin( ); setIter_i != tempSet.end( ); setIter_i++ ) 
 {
     
    fimmap tempMap_i = *setIter_i;
    fimmap::iterator it = tempMap_i.begin();
    int topValue = (*it).second;
    //LogTrace("FourVectorHLTriggerOffline") << " topValue = " << topValue << endl;

    
    mmset::iterator tempIter_i = setIter_i;

    // from all the other maps, clean entries that have mapped value "topValue"
    // loop j
    for ( mmset::iterator setIter_j = ++tempIter_i; setIter_j != tempSet.end( ); setIter_j++ ) 
    {

      fimmap tempMap_j = *setIter_j;
      //LogTrace("FourVectorHLTriggerOffline") << "  size of the map  = " << tempMap_j.size() << endl;

      for (fimmap::iterator it = tempMap_j.begin(); it != tempMap_j.end(); ++it)
      {

        if(topValue == (*it).second) 
	{
				  
          //LogTrace("FourVectorHLTriggerOffline") << "   Ridding map of a doubly-matched object." << endl;
	  tempMap_j.erase(it);

	  cleanedOneMap = true;

	}

     } //end for

     if(cleanedOneMap) 
     {
        
	//remove the old map from the set
	tempSet.erase(setIter_j);

	// insert in the set the new map if it is not an empty map
	if(! tempMap_j.empty()) tempSet.insert(tempMap_j);

	break; // break from loop j

     } // end if

    }// end loop j 

    if(cleanedOneMap) break; // break from loop i

 } // end loop i

 if(cleanedOneMap) 
 { 

  // continue cleaning (in while loop)
  // but reset flag first
  cleanedOneMap=false;
  continue; 

 }
 else {

  // finished cleaing (break from while loop)
  break; 

 }

} // end while

 //LogTrace("FourVectorHLTriggerOffline") << "cleaned: size of the set  = " << tempSet.size() << " maps." << endl;
 int jmap = 0;

 for ( mmset::iterator setIter_i = tempSet.begin( ); setIter_i != tempSet.end( ); setIter_i++ ) 
 {

      fimmap tempMap_j = *setIter_i;

      //LogTrace("FourVectorHLTriggerOffline") << " map " << jmap << endl;
      //LogTrace("FourVectorHLTriggerOffline") << " --------" << endl;

      for (fimmap::iterator it = tempMap_j.begin(); it != tempMap_j.end(); ++it)
      {

        //LogTrace("FourVectorHLTriggerOffline") << " " <<   (*it).first << " :  " << (*it).second << endl;
      
      }

      jmap++;

 } // end for


 return;
}

