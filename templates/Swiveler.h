
/* (c)  oblong industries */

#pragma once

#include "libStaging/libRad/Cascaderific.h"
#include "libGreenhouse/GreenhouseHeaders.h"

using namespace oblong::greenhouse;

/**
 */
struct Swiveler  :  public Thing,
                      public Cascaderific
{ bool move_handled_this_cycle;
  float64 running_inc_angle;
  Quat drag_start_rotation;
  bool fake_swivel_mode;
  bool pause_swiveler;

  /**
      */
  Swiveler (const Slaw &config);
  virtual ~Swiveler () {}

  void Travail ();
  void Metabolize (Protein const &p);

  void PointingHarden (OEPointingEvent *e);
  void PointingMove (OEPointingEvent *e);
  void PointingSoften (OEPointingEvent *e);
  void Blurt (OEBlurtEvent *e);

private:
  Swiveler () { }
};

