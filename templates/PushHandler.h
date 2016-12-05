
/* (c)  oblong industries */

#pragma once

#include "libStaging/libRad/Cascaderific.h"
#include "libGreenhouse/GreenhouseHeaders.h"
// #include "PushGraphics.h"

using namespace oblong::greenhouse;


/**
 */

struct PushHandler  :  public Thing,
                       public Cascaderific
{ bool fake_pushup_mode;
  bool fake_pushdown_mode;
  bool fake_pushover_mode; //ha ha
  bool fake_pushnorm_mode;
  bool pushup;
  bool pushdown;
  bool pushover;
  bool pushnorm;
  
  int current_phase;
  Vect pushup_start_phys_through;
  // Vect pushup_start_translation;
  Vect starting_transl;
  // PushGraphics *push_glyphs;
  FatherTime click_timer;
  FatherTime gesture_timer;

  // note: gets config from "control-band-config"
  PushHandler (const Slaw &config);
  virtual ~PushHandler () {}

  void Metabolize (Protein const &p);
  void PointingHarden (OEPointingEvent *e);
  void PointingMove (OEPointingEvent *e);
  void PointingSoften (OEPointingEvent *e);
  void HandleTimePush (const Vect &diff_from_start, const bool &mouser);
  void HandlePushBack (const Vect &diff_from_start, const bool &mouser);
  void HandlePushOver (const Vect &diff_from_start, const bool &mouser);
  void Blurt (OEBlurtEvent *e);

private:
  PushHandler () { }
};