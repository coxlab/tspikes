#ifndef TSPIKEWIN_H
#define TSPIKEWIN_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <assert.h>

#include <stdlib.h>
#include <gtkmm.h>
#define GL_GLEXT_PROTOTYPES
#include <gtkglmm.h>

#include <boost/ptr_container/ptr_map.hpp>
#include <boost/scoped_ptr.hpp>

#include <somanetwork/networkinterface.h>
#include <somanetwork/tspike.h>
#include <somanetwork/datapacket.h>

#include "clusterview.h"
#include "spikewaveview.h"
#include "ratetimeline.h"
#include "somanetcodec.h"
#include "sourcesettingswin.h" 
#include "tslogging.h" 
#include "clusterviewcontroller.h"

// widget list types
typedef boost::ptr_vector<SpikeWaveView> pSpikeWaveViewVect_t; 
typedef boost::ptr_vector<SpikeWaveView> pClusterViewVect_t;

// helper
void printEvent(Event_t event); 

const reltime_t SPVECTDURATION = 5.0; 
const reltime_t RATEUPDATE = 1.0; 


class TSpikeWin : public Gtk::Window
{
public:
  /*
    The preloaded spikes are assumed to : 
    1. occur after the expStartTime. 
    2. the next somatime_t coming across the network should be -later-. 

   */ 
  explicit TSpikeWin(pNetworkInterface_t network, 
		     datasource_t src, 
		     somatime_t expStartTime, 
		     const std::vector<TSpike_t> & preloadspikes);

  virtual ~TSpikeWin();

  void setTime(somatime_t t);
  void loadExistingSpikes(const std::vector<TSpike_t> & spikes); 

protected:
  virtual bool on_idle();

protected:
  
  
  pNetworkInterface_t pNetwork_; 
  datasource_t dsrc_; 
  
  SpikePointVectDatabase spvdb_; 

  // member widgets:

  Gtk::Table clusterTable_; 
  Gtk::Table spikeWaveTable_; 
  Gtk::VBox spikeWaveVBox_, clusterViewVBox_; 
  Gtk::HBox mainHBox_; 
  Gtk::HBox timeLineHBox_; 

  // 
  #ifndef NO_GL
  
  clusterViews_t clusterViews_; 

  ClusterViewController * pClusterViewController_; 
  

  SpikeWaveView spikeWaveViewX_;
  SpikeWaveView spikeWaveViewY_;
  SpikeWaveView spikeWaveViewA_;
  SpikeWaveView spikeWaveViewB_;
  
  RateTimeline rateTimeline_; 
#endif //NO_GL
  // action menu item 
  Glib::RefPtr<Gtk::ActionGroup> refActionGroup_; 
  Glib::RefPtr<Gtk::UIManager> refUIManager_; 
  
  Gtk::Menu * pMenuPopup_; 

  // properties

 

  // append data functions
  

  // update clusters
  void updateClusterView(bool, reltime_t, float); 

  void liveToggle(); 

  // timeline manipulation
  reltime_t spVectorStartTime_; 
  reltime_t lastRateTime_; 
  long lastRateSpikeCount_; 
  long spikeCount_;
  
  
  Gtk::ToggleButton liveButton_; 
  pSomaNetworkCodec_t pSomaNetworkCodec_; 

  // property editor

  SourceSettingsWin sourceSettingsWin_;   
  // callbacks
  void timeUpdateCallback(somatime_t); 
  void sourceStateChangeCallback(int, TSpikeChannelState); 
  void newTSpikeCallback(const TSpike_t &); 
  void setupMenus(); 

  bool on_button_press_event(GdkEventButton* event); 

  // functions
  void on_action_quit(); 
  void on_action_reset_views(); 
  void on_action_reset_data(); 
  void on_action_source_settings(void);

  somatime_t expStartTime_; 
  somatime_t lastSomaTime_; 

};

#endif // TSPIKEWIN_H
