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

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "glconfig.h"
#include "glspikes.h"

enum DecayMode {LINEAR, LOG}; 
enum CViewMode  {VIEW12, VIEW13, VIEW14, VIEW23, VIEW24, VIEW34}; 


class ClusterView : public Gtk::GL::DrawingArea
{
public:
  explicit ClusterView(GLSPVectpList_t * pspvl, CViewMode cvm);  
  virtual ~ClusterView();
  void setView(GLSPVectpList_t::iterator sstart, 
		   GLSPVectpList_t::iterator send, 
		   float decayRate, DecayMode dm); 
  // Invalidate whole window.
  void inv(); 
  bool setViewingWindow(float x1, float y1,  float x2, float y2); 
  int getFrames(); 

  // Update window synchronously (fast).
  void update()
  { get_window()->process_updates(false); }

protected:

  // signal handlers:
  virtual void on_realize();
  virtual bool on_configure_event(GdkEventConfigure* event);
  virtual bool on_expose_event(GdkEventExpose* event);
  virtual bool on_map_event(GdkEventAny* event);
  virtual bool on_unmap_event(GdkEventAny* event);
  virtual bool on_visibility_notify_event(GdkEventVisibility* event);
  virtual bool on_idle();

  void resetAccumBuffer(GLSPVectpList_t::iterator sstart, 
				GLSPVectpList_t::iterator send); 
  void renderSpikeVector(const GLSPVect_t & spvect, 
			 bool live = false); 
  void updateView(); 
  void updateViewingWindow(); 
  
  
  GLSPVectpList_t*  pspvl_; 
  float decayRate_; 
  DecayMode decayMode_; 
  bool viewChanged_;   
  GLSPVectpList_t::iterator viewStartIter_;  
  GLSPVectpList_t::iterator viewEndIter_;  

  int m_Frames;
  CViewMode viewMode_; 
  GLSPVectpList_t::iterator pCurSPVect_; 
  GLfloat viewX1_, viewX2_, viewY1_, viewY2_; 
  
  GLuint gpuProg_; 


};
#endif
