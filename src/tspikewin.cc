#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/assign/std/vector.hpp> 
#include <boost/foreach.hpp>
#include "tspikewin.h"
#include "glspikes.h"
#include <boost/assign.hpp>

using namespace boost::posix_time;


TSpikeWin::TSpikeWin(pNetworkInterface_t pNetwork, 
		     datasource_t src,
		     somatime_t expStartTime_soma, 
		     const std::vector<TSpike_t> & preloadspikes
		     ) : 

  pNetwork_(pNetwork), 
  spvdb_(5, 1000000), 
  clusterTable_(2, 3), 
  spikeWaveTable_(2, 2), 
  spikeWaveVBox_(false, 0), 
  clusterViewVBox_(false, 0), 
  mainHBox_(false, 0), 
#ifndef NO_GL
  spikeWaveViewX_(CHANX), 
  spikeWaveViewY_(CHANY), 
  spikeWaveViewA_(CHANA), 
  spikeWaveViewB_(CHANB),
#endif // NO_GL
  spVectorStartTime_(0.0),
  liveButton_("Live"), 
  sourceSettingsWin_(), 
  pMenuPopup_(0), 
  dsrc_(src), 
  spikeCount_(0), 
  expStartTime_(expStartTime_soma), 
  lastSomaTime_(expStartTime_)
{
  //
  // Top-level window.
  //
  
  set_title("Tetrode Spike Viewer");

  set_reallocate_redraws(true);

  pNetwork_->enableDataRX(dsrc_, TSPIKE); 

  //boost::assign::ptr_map_insert<GLSPVect_t>(spvdb_)(abstimeToRelTime(spVectorStartTime_, offsetTime_)); 
  add(mainHBox_); 
  
  int clusterWidth = 165; 
#ifndef NO_GL
  using namespace boost::assign; 
  std::vector<CViewMode> modes; 
  modes += VIEW12, VIEW13, VIEW14, VIEW23, VIEW24, VIEW34; 

  for (int i = 0; i < 6; i++) {
    clusterViews_.push_back(new ClusterView(spvdb_, modes[i])); 
    clusterViews_[i].set_size_request(clusterWidth, clusterWidth);
  }

  pClusterViewController_ = new ClusterViewController(clusterViews_); 

  clusterTable_.attach(clusterViews_[0], 0, 1, 0, 1);
  clusterTable_.attach(clusterViews_[1], 1, 2, 0, 1);
  clusterTable_.attach(clusterViews_[2], 2, 3, 0, 1);
  clusterTable_.attach(clusterViews_[3], 0, 1, 1, 2);
  clusterTable_.attach(clusterViews_[4], 1, 2, 1, 2);
  clusterTable_.attach(clusterViews_[5], 2, 3, 1, 2);

  rateTimeline_.set_size_request(clusterWidth*3, 60); 
#endif // NO_GL
  
  clusterTable_.set_border_width(1.0); 
  clusterTable_.set_row_spacings(1.0); 
  clusterTable_.set_col_spacings(1.0); 

  clusterViewVBox_.pack_start(clusterTable_) ;
  clusterViewVBox_.pack_start(timeLineHBox_); 
#ifndef NO_GL
  timeLineHBox_.pack_start(rateTimeline_); 
#endif // NO_GL
  timeLineHBox_.pack_start(liveButton_); 
  
#ifndef NO_GL
  spikeWaveViewX_.set_size_request(150, clusterWidth); 
  spikeWaveViewY_.set_size_request(150, clusterWidth); 
  spikeWaveViewA_.set_size_request(150, clusterWidth); 
  spikeWaveViewB_.set_size_request(150, clusterWidth); 

  spikeWaveTable_.attach(spikeWaveViewX_, 0, 1, 0, 1); 
  spikeWaveTable_.attach(spikeWaveViewY_, 1, 2, 0, 1); 
  spikeWaveTable_.attach(spikeWaveViewA_, 0, 1, 1, 2); 
  spikeWaveTable_.attach(spikeWaveViewB_, 1, 2, 1, 2); 

#endif // NO_GL

  spikeWaveTable_.set_border_width(1.0); 
  spikeWaveTable_.set_row_spacings(1.0); 
  spikeWaveTable_.set_col_spacings(1.0); 
  spikeWaveVBox_.pack_start(spikeWaveTable_); 
  
  mainHBox_.pack_start(spikeWaveVBox_); 
  mainHBox_.pack_start(clusterViewVBox_);   
  //
  // Show window.
  //

  float decay = 0.05; 
#ifndef NO_GL
  for (int i = 0; i < 6; i++) {
    clusterViews_[i].setView(spvdb_.begin(), 
			   spvdb_.end(), 
			   decay, LOG); 
    clusterViews_[i].setViewingWindow(0, 0, float(150e-6), float(150e-6));
  }

  
  spikeWaveViewX_.setAmplitudeView(-100e-6, 300e-6); 
  spikeWaveViewY_.setAmplitudeView(-100e-6, 300e-6); 
  spikeWaveViewA_.setAmplitudeView(-100e-6, 300e-6); 
  spikeWaveViewB_.setAmplitudeView(-100e-6, 300e-6); 
  

  spikeWaveViewX_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewY_, 
								     &SpikeWaveView::setAmplitudeView)); 
  spikeWaveViewX_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewA_, 
								     &SpikeWaveView::setAmplitudeView)); 
  spikeWaveViewX_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewB_, 
								     &SpikeWaveView::setAmplitudeView)); 


  spikeWaveViewY_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewX_, 
								     &SpikeWaveView::setAmplitudeView)); 
  spikeWaveViewY_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewA_, 
								     &SpikeWaveView::setAmplitudeView)); 
  spikeWaveViewY_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewB_, 
								     &SpikeWaveView::setAmplitudeView)); 

  spikeWaveViewA_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewX_, 
								     &SpikeWaveView::setAmplitudeView)); 
  spikeWaveViewA_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewY_, 
								     &SpikeWaveView::setAmplitudeView)); 
  spikeWaveViewA_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewB_, 
								     &SpikeWaveView::setAmplitudeView)); 

  spikeWaveViewB_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewX_, 
								     &SpikeWaveView::setAmplitudeView)); 
  spikeWaveViewB_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewY_, 
								     &SpikeWaveView::setAmplitudeView)); 
  spikeWaveViewB_.amplitudeViewChangedSignal().connect(
						       sigc::mem_fun(spikeWaveViewA_, 
								     &SpikeWaveView::setAmplitudeView)); 


#endif // NO_GL

  liveButton_.set_active(true); 

  show_all();

#ifndef NO_GL
  rateTimeline_.viewSignal().connect(sigc::mem_fun(*this, 
						   &TSpikeWin::updateClusterView)); 
#endif
  liveButton_.signal_toggled().connect(sigc::mem_fun(*this,
						     &TSpikeWin::liveToggle)); 

  
  pSomaNetworkCodec_ = pSomaNetworkCodec_t(new SomaNetworkCodec(pNetwork_, dsrc_)); 

  /// data recovery callback hookup
  pSomaNetworkCodec_->signalSourceStateChange().connect(sigc::mem_fun(*this, 
								    &TSpikeWin::sourceStateChangeCallback)); 
  
  pSomaNetworkCodec_->signalTimeUpdate().connect(sigc::mem_fun(*this, 
							     &TSpikeWin::timeUpdateCallback)); 

  pSomaNetworkCodec_->signalNewTSpike().connect(sigc::mem_fun(*this, 
							    &TSpikeWin::newTSpikeCallback)); 


  set_decorated(false);

  
  setupMenus(); 


  // preload the indicated data
  loadExistingSpikes(preloadspikes); 

  sourceSettingsWin_.setCodec(pSomaNetworkCodec_); 

  // now query state
  pSomaNetworkCodec_->refreshStateCache(); 

  sourceSettingsWin_.show(); // FIXME

}

void TSpikeWin::setupMenus()
{

  // First we create "Actions", these are things that we want to "do" from our
  // menu 
  refActionGroup_ = Gtk::ActionGroup::create();

  refActionGroup_->add(Gtk::Action::create("Quit", "Quit"), 
		       sigc::mem_fun(*this, &TSpikeWin::on_action_quit)); 
  refActionGroup_->add(Gtk::Action::create("ResetViews", "Reset All Views"), 
		       sigc::mem_fun(*this, &TSpikeWin::on_action_reset_views)); 

  refActionGroup_->add(Gtk::Action::create("ResetData", "Reset All Data History"), 
		       sigc::mem_fun(*this, &TSpikeWin::on_action_reset_data)); 

  refActionGroup_->add(Gtk::Action::create("SourceSettings", "Change Source Settings"), 
		       sigc::mem_fun(*this, &TSpikeWin::on_action_source_settings)); 
  
  refActionGroup_->add(Gtk::Action::create("About", "About TSpikes")); 
  
  




  refUIManager_ = Gtk::UIManager::create();

  Glib::ustring ui_info =
    "<ui>"
    "  <popup name='PopupMenu'>"
    "    <menuitem action='SourceSettings'/>"
    "    <menuitem action='ResetViews'/>"
    "    <menuitem action='ResetData'/>"
    "    <menuitem action='About'/>"
    "    <menuitem action='Quit'/>"
    "  </popup>"
    "</ui>";

  refUIManager_->insert_action_group(refActionGroup_);
  add_accel_group(refUIManager_->get_accel_group());
  refUIManager_->add_ui_from_string(ui_info); 
  


  pMenuPopup_ = dynamic_cast<Gtk::Menu*>(refUIManager_->get_widget("/PopupMenu")); 

  add_events(Gdk::VISIBILITY_NOTIFY_MASK | 
	     Gdk::BUTTON_PRESS_MASK );
  signal_button_press_event().connect(sigc::mem_fun(*this, 
						    &TSpikeWin::on_button_press_event)); 


}

TSpikeWin::~TSpikeWin()
{
  delete pClusterViewController_; 

}

bool TSpikeWin::on_idle()

{
  TSL_(debug) << "TSpikeWin: on idle"; 

#ifndef NO_GL
  BOOST_FOREACH(ClusterView & cv, clusterViews_) {
    cv.invalidate(); 
  }

  spikeWaveViewX_.invalidate(); 
  spikeWaveViewY_.invalidate(); 
  spikeWaveViewA_.invalidate(); 
  spikeWaveViewB_.invalidate(); 
#endif // NO_GL
  return true;
}


void TSpikeWin::setTime(somatime_t  stime)
{

  if ( !(stime  > lastSomaTime_)) {
    TSL_(error) << "TSpikeWin: Error, set soma time " << stime
		<< " not strictly greater than previous time " 
		<< lastSomaTime_; 

  }
  abstime_t t= somatimeToAbsTime(stime); 

  // compute reltime
  reltime_t reltime = t - somatimeToAbsTime(expStartTime_); 

  spvdb_.setTime(reltime); 
  
  if (reltime - lastRateTime_ > RATEUPDATE ) 
    {
      rateval_t rv = double(spikeCount_ - lastRateSpikeCount_)/(reltime - lastRateTime_);  
      RatePoint_t rp; 
      rp.rate = rv; 
      rp.time = reltime; 
#ifndef NO_GL
      rateTimeline_.appendRate(rp); 
#endif // NO_GL

      lastRateTime_ = reltime; 
      lastRateSpikeCount_ = spikeCount_; 
    }

  time_duration td = microseconds(reltime * 1e6); 
  boost::format timeformat("Live \n(%d:%02d:%02d)\n%d"); 
  timeformat % td.hours() % td.minutes() % td.seconds() % stime; 
  liveButton_.set_label(boost::str(timeformat)); 
  lastSomaTime_; 
}


void TSpikeWin::updateClusterView(bool isLive, reltime_t activePos, float decayRate)
{
  // decayRate is a rate, luminance/alpha drops per unit time
  TSL_(debug) << "TSpikeWin: update cluster view"; 
  // compute time range
  reltime_t t2 = activePos; 
  reltime_t t1; 
  if (decayRate > 0 ) {
    reltime_t winsize = 1 / decayRate; 
    t1 = t2 - winsize; 
  } else {
    t1 = spvdb_.begin()->first; 
  }
  
  // now find iterators
  GLSPVectMap_t::const_iterator t1i, t2i; 

  // -----------------------------------------------------------
  // get lower bound
  // -----------------------------------------------------------
  t1i =  spvdb_.lower_bound(t1); 

  if (t1i == spvdb_.end())
    {
      // there is now element lower than t1

      // DO SOMETHING
      
    } 
  else if (t1i == spvdb_.begin())
    {
      // the first element, don't dec
    } 
  else 
    {
      // normal, we want to go one further to get a superset of the times
      t1i--; 
    }

  // -----------------------------------------------------------
  // get upper bound
  // -----------------------------------------------------------
  t2i =  spvdb_.lower_bound(t2); 

  if (t2i == spvdb_.end())
    {
      // there is now element lower than t1 - that's okay, we want the full
      // range

    } 
  else 
    {
      // normal, we want to go one further to get a superset of the times

      t2i++; 
    }

  
  // now update internal iterators
#ifndef NO_GL
  
  BOOST_FOREACH(ClusterView & cv, clusterViews_) {
    cv.setView(t1i, t2i, decayRate, LOG);
  }


  BOOST_FOREACH(ClusterView & cv, clusterViews_) {
    cv.invalidate(); 
  }

#endif // NO_GL

}


void TSpikeWin::loadExistingSpikes(const std::vector<TSpike_t> & spikes)
{
  std::vector<TSpike_t>::const_iterator pts; 
  for (pts = spikes.begin(); pts != spikes.end(); pts++)
    {
      
      somatime_t t = pts->time; 
      setTime(t);
      newTSpikeCallback(*pts); 
      
    }
}



void TSpikeWin::liveToggle()
{
#ifndef NO_GL

  rateTimeline_.setLive(liveButton_.get_active()); 
  spikeWaveViewX_.setLive(liveButton_.get_active()); 
  spikeWaveViewY_.setLive(liveButton_.get_active()); 
  spikeWaveViewA_.setLive(liveButton_.get_active()); 
  spikeWaveViewB_.setLive(liveButton_.get_active()); 
#endif // NO_GL  

}

void TSpikeWin::timeUpdateCallback(somatime_t stime)
{

  setTime(stime); 
}

void TSpikeWin::sourceStateChangeCallback(int chan, TSpikeChannelState state)
{
  TSL_(info) << "TSPikeWin: source state change chan=" << chan; 

#ifndef NO_GL

  switch(chan) {
  case 0:
    spikeWaveViewX_.updateState(state); 

    clusterViews_[0].updateState(true, state); 
    clusterViews_[1].updateState(true, state); 
    clusterViews_[2].updateState(true, state); 
    break; 

  case 1:
    spikeWaveViewY_.updateState(state); 

    clusterViews_[0].updateState(false, state);
    clusterViews_[3].updateState(true, state); 
    clusterViews_[4].updateState(true, state); 
    break;

  case 2:
    spikeWaveViewA_.updateState(state); 

    clusterViews_[1].updateState(false, state);
    clusterViews_[3].updateState(false, state); 
    clusterViews_[5].updateState(true, state); 
    break;

  case 3:
    spikeWaveViewB_.updateState(state); 

    clusterViews_[2].updateState(false, state);
    clusterViews_[4].updateState(false, state); 
    clusterViews_[5].updateState(false, state); 
    break;

  }
#endif  // END_IF
}

void TSpikeWin::newTSpikeCallback(const TSpike_t & ts)
{
  //  std::cout << "New tspike callback" << std::endl; 
  reltime_t offsetTime = somatimeToAbsTime(expStartTime_); 
  std::vector<GLSpikeWave_t> gls = splitSpikeToGLSpikeWaves(ts, offsetTime); 
  
#ifndef NO_GL
  
  spikeWaveViewX_.newSpikeWave(gls[0]); 
  spikeWaveViewY_.newSpikeWave(gls[1]); 
  spikeWaveViewA_.newSpikeWave(gls[2]); 
  spikeWaveViewB_.newSpikeWave(gls[3]); 
#endif // NO_GL  
  GLSpikePoint_t sp = convertTSpikeToGLSpike(ts, offsetTime); 
  spvdb_.append(sp); 


  spikeCount_++; 
#ifndef NO_GL
  BOOST_FOREACH(ClusterView & cv, clusterViews_) {
    cv.invalidate(); 
  }

#endif


}

bool TSpikeWin::on_button_press_event(GdkEventButton* event)
{
  if( (event->type == GDK_BUTTON_PRESS) &&
      (event->button == 3) )
  {
    pMenuPopup_->popup(event->button, event->time);

    return true; 
  } else if (event->type == GDK_2BUTTON_PRESS) { 
    set_decorated(!get_decorated()); 

    return true; 
  }
  else
    return false;
}

void TSpikeWin::on_action_quit(void)
{
  TSL_(debug) << "TSpikeWin: Quitting..."; 
  hide(); 

}

void TSpikeWin::on_action_source_settings(void)
{
  sourceSettingsWin_.show(); 
  
}

void TSpikeWin::on_action_reset_data()
{
  //reltime_t rt = abstimeToRelTime(spVectorStartTime_, offsetTime_); 
  spvdb_.reset(); 

  //  spvdb_.insert((reltime_t)0.0, new GLSPVect_t);
  #ifndef NO_GL
  rateTimeline_.resetData(); 
  // reset offset time

  spikeWaveViewX_.resetData(); 
  spikeWaveViewY_.resetData(); 
  spikeWaveViewA_.resetData(); 
  spikeWaveViewB_.resetData();

  
  BOOST_FOREACH(ClusterView & cv, clusterViews_) {
    cv.resetData(); 
  }

  #endif
}

void TSpikeWin::on_action_reset_views()
{
  #ifndef NO_GL
  spikeWaveViewX_.resetView(); 
  spikeWaveViewY_.resetView(); 
  spikeWaveViewA_.resetView(); 
  spikeWaveViewB_.resetView();

  BOOST_FOREACH(ClusterView & cv, clusterViews_) {
    cv.resetView(); 
  }

  #endif

}
