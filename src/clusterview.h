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

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "glconfig.h"
#include "glspikes.h"
#include "clusterrenderer.h"


class ClusterView : public Gtk::GL::DrawingArea
{
public:
  explicit ClusterView(GLSPVectpList_t * pspvl, CViewMode cvm);  
  virtual ~ClusterView();
  void setView(GLSPVectpList_t::iterator sstart, 
		   GLSPVectpList_t::iterator send, 
		   float decayRate, DecayMode dm); 
  // Invalidate whole window.

  void invalidate() {
    Glib::RefPtr<Gdk::Window> win = get_window();
    Gdk::Rectangle r(0, 0, get_allocation().get_width(),
		     get_allocation().get_height());
    win->invalidate_rect(r, false);

  }

  bool setViewingWindow(float x1, float y1,  float x2, float y2); 

  int getFrames(); 

  // Update window synchronously (fast).
  void update()
  { get_window()->process_updates(false); }
  
  void setGrid(float val); 
  void zoomX(float factor); 
  void zoomY(float factor); 

protected:

  // signal handlers:
  virtual void on_realize();
  virtual bool on_configure_event(GdkEventConfigure* event);
  virtual bool on_expose_event(GdkEventExpose* event);
  virtual bool on_map_event(GdkEventAny* event);
  virtual bool on_unmap_event(GdkEventAny* event);
  virtual bool on_visibility_notify_event(GdkEventVisibility* event);

  void updateView(); 
  void updateViewingWindow(); 
  int frameCount_; 

  float x1_;
  float y1_;
  float x2_;
  float y2_; 
  
  GLSPVectpList_t*  pspvl_; 
  ClusterRenderer clusterRenderer_; 
  
};
#endif
