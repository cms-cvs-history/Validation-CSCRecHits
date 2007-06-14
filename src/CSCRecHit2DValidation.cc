#include "Validation/CSCRecHits/src/CSCRecHit2DValidation.h"
#include "DataFormats/CSCRecHit/interface/CSCRecHit2DCollection.h"



CSCRecHit2DValidation::CSCRecHit2DValidation(DaqMonitorBEInterface* dbe, const edm::InputTag & inputTag)
: CSCBaseValidation(dbe, inputTag),
  theNPerEventPlot( dbe_->book1D("CSCRecHitsPerEvent", "Number of CSC Rec Hits per event", 100, 0, 500) )
{
   dbe_->setCurrentFolder("CSCRecHitTask");

   for(int i = 0; i < 10; ++i)
  {
    char title1[200], title2[200], title3[200], title4[200], title5[200], title6[200];
    sprintf(title1, "CSCRecHitResolution%d", i+1);
    sprintf(title2, "CSCRecHitPull%d", i+1);
    sprintf(title3, "CSCRecHitYResolution%d", i+1);
    sprintf(title4, "CSCRecHitYPull%d", i+1);
    sprintf(title5, "CSCRecHitLocalPhi%d", i+1);
    sprintf(title6, "CSCRecHitLocalY%d", i+1);


    theResolutionPlots[i] = dbe_->book1D(title1, title1, 100, -0.2, 0.2);
    thePullPlots[i] = dbe_->book1D(title2, title2, 100, -3, 3);
    theYResolutionPlots[i] = dbe_->book1D(title3, title3, 100, -5, 5);
    theYPullPlots[i] = dbe_->book1D(title4, title4, 100, -3, 3);

    thePhiPlots[i] =  dbe_->book1D(title5, title5, 100, -30, 30);
    theYPlots[i] = dbe_->book1D(title6, title6, 100, -35, 35);
  }

}

void CSCRecHit2DValidation::analyze(const edm::Event&e, const edm::EventSetup& eventSetup)
{
  // get the collection of CSCRecHrecHitItrD
  edm::Handle<CSCRecHit2DCollection> hRecHits;
  e.getByLabel(theInputTag, hRecHits);
  const CSCRecHit2DCollection * cscRecHits = hRecHits.product();

  unsigned nPerEvent = 0;

  for(CSCRecHit2DCollection::const_iterator recHitItr = cscRecHits->begin(); 
      recHitItr != cscRecHits->end(); recHitItr++) 
  {
    ++nPerEvent;
    int detId = (*recHitItr).cscDetId().rawId();
    edm::PSimHitContainer simHits = theSimHitMap->hits(detId);
    const CSCLayer * layer = findLayer(detId);
    int chamberType = layer->chamber()->specs()->chamberType();


    if(simHits.size() == 1)
    {
      plotResolution(simHits[0], *recHitItr, layer, chamberType);
    }

    float localY = recHitItr->localPosition().y();
    theYPlots[chamberType]->Fill(localY);
    // find a local phi
    float globalR = layer->toGlobal(recHitItr->localPosition()).perp();
    GlobalPoint axisThruChamber(recHitItr->localPosition().x(), globalR+localY, 0.);
    thePhiPlots[chamberType]->Fill(axisThruChamber.phi().degrees());
  }    
  theNPerEventPlot->Fill(nPerEvent);

}


void CSCRecHit2DValidation::plotResolution(const PSimHit & simHit, const CSCRecHit2D & recHit,
                                         const CSCLayer * layer, int chamberType)
{
  GlobalPoint simHitPos = layer->toGlobal(simHit.localPosition());
  GlobalPoint recHitPos = layer->toGlobal(recHit.localPosition());

  double dphi = recHitPos.phi() - simHitPos.phi();
  double rdphi = recHitPos.perp() * dphi;
  theResolutionPlots[chamberType-1]->Fill( rdphi );
  thePullPlots[chamberType-1]->Fill( rdphi/ sqrt(recHit.localPositionError().xx()) );
std::cout << "XX " << sqrt(recHit.localPositionError().xx()) << std::endl;
  double dy = recHit.localPosition().y() - simHit.localPosition().y();
  theYResolutionPlots[chamberType-1]->Fill( dy );
  theYPullPlots[chamberType-1]->Fill( dy/ sqrt(recHit.localPositionError().yy()) );

}

