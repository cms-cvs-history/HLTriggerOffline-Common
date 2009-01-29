#the sequence defined herein do not get included
# in the actual sequences run centrally
#these module instances serve as examples of postprocessing
# new histograms are generated as the ratio of the specified input
# defined thru 'efficiency = cms.vstring(arg)' where arg has the form
# "ratio 'histo title; x-label; y-label; numerator denominator'"
#the base code is: Validation/RecoMuon/src/PostProcessor.cc

import FWCore.ParameterSet.Config as cms

myMuonPostVal = cms.EDFilter("PostProcessor",
    outputFileName = cms.untracked.string('MuonPostProcessor.root'),
    commands       = cms.vstring(),
    resolution     = cms.vstring(),                                    
    subDir         = cms.untracked.string('HLT/Muon/Distributions/*'),
    efficiency     = cms.vstring(
        "EFF 'my title; my x-label; my y-label' genPassEta_L1Filtered genPassEta_All"
    )
)


myEgammaPostVal = cms.EDFilter("PostProcessor",
    outputFileName = cms.untracked.string('EgammaPostProcessor.root'),
    commands       = cms.vstring(),
    resolution     = cms.vstring(),                                    
    subDir         = cms.untracked.string('HLT/HLTEgammaValidation/*'),
    efficiency     = cms.vstring(
        "EFF 'my title; my x-label; my y-label' hltL1sDoubleEgammaeta hltL1sDoubleEgammaeta"
    )
)

myTauPostVal = cms.EDFilter("PostProcessor",
    outputFileName = cms.untracked.string('TauPostProcessor.root'),
    commands       = cms.vstring(),
    resolution     = cms.vstring(),                                    
    subDir         = cms.untracked.string('HLT/HLTTAU/*'),
    efficiency     = cms.vstring(
        "EFF 'my title; my x-label; my y-label' L1Tau1Eta GenTauElecEta"
    )
)


myTopPostVal = cms.EDFilter("PostProcessor",
    outputFileName = cms.untracked.string('TopPostProcessor.root'),
    commands       = cms.vstring(),
    resolution     = cms.vstring(),                                    
    subDir         = cms.untracked.string('HLT/Top/'),
    efficiency     = cms.vstring(
    "TrigEFF 'my title; my x-label; my y-label' pt_trig_off_mu pt_off_mu"
    )
)

ExamplePostVal = cms.Sequence(
    myMuonPostVal
    +myEgammaPostVal
    +myTauPostVal
    +myTopPostVal
    )