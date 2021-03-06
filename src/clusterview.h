#ifndef CLUSTERVIEW_H
#define CLUSTERVIEW_H 

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <assert.h>

#include <stdlib.h>
#include <gtkmm.h>

#define GL_GLEXT_PROTOTYPES

#include <gtkglmm.h>

#include <boost/ptr_container/ptr_vector.hpp>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "glconfig.h"
#include "glspikes.h"
#include "clusterrenderer.h"
#include "somanetcodec.h"
#include "spvectdb.h" 

class ClusterView : public Gtk::GL::DrawingArea
{
public:
  explicit ClusterView(SpikePointVectDatabase & pspvl, CViewMode cvm);  
  virtual ~ClusterView();
  void setView(GLSPVectMap_t::const_iterator sstart, 
		   GLSPVectMap_t::const_iterator send, 
		   float decayRate, DecayMode dm); 
  // Invalidate whole window.

  void invalidate(); 
  void resetData(); 
  void resetView(); 

  bool setViewingWindow(float x1, float y1,  float x2, float y2); 

  int getFrames(); 

  // Update window synchronously (fast).
  void update()
  { get_window()->process_updates(false); }
  
  void setGrid(float val); 
  void zoomX(float factor); 
  void zoomY(float factor); 

  void updateState(bool X, const TSpikeChannelState & state); 
  
  float rangeX_;
  float rangeY_; 

  sigc::signal <void, float, float> & xViewChangeRequestSignal() 
    { return xViewChangeRequestSignal_; }

  sigc::signal <void, float, float> & yViewChangeRequestSignal()
  {return yViewChangeRequestSignal_; }
  
  void setXView(float); 
  void setYView(float); 

  void setXViewFraction(float); 
  void setYViewFraction(float); 

protected:

  // signal handlers:
  virtual void on_realize();
  virtual bool on_configure_event(GdkEventConfigure* event);
  virtual bool on_expose_event(GdkEventExpose* event);
  virtual bool on_map_event(GdkEventAny* event);
  virtual bool on_unmap_event(GdkEventAny* event);
  virtual bool on_visibility_notify_event(GdkEventVisibility* event);

  bool on_motion_notify_event(GdkEventMotion* event); 
  bool on_button_press_event(GdkEventButton* event); 

  void updateView(); 
  void updateViewingWindow(); 
  int frameCount_; 

  float x1_;
  float y1_;
  float x2_;
  float y2_; 
  CViewMode viewMode_;   
  ClusterRenderer clusterRenderer_; 
  float lastX_; 
  float lastY_; 

  sigc::signal<void, float, float> xViewChangeRequestSignal_; 
  sigc::signal<void, float, float> yViewChangeRequestSignal_; 
  
};

typedef boost::ptr_vector<ClusterView> clusterViews_t; 

#endif
