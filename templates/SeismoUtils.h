/* (c)  oblong industries */

#pragma once

#include "libStaging/libRad/Cascaderific.h"
#include "libGreenhouse/GreenhouseHeaders.h"
#include "libStaging/libRad/Global.h"
#include <libLoam/c++/Vect.h>
#include <libBasement/SoftFloat.h>

using namespace oblong::greenhouse;
using namespace oblong::loam;
using namespace oblong::basement;


///  the "equality" test is whether the difference between the two floats is very small
static inline bool equal (const float64 a, const float64 b)
{ return fabs (a - b) <  1.0e-6; }

static inline bool ItsAMouse (OEPointingEvent *e)
{ return e -> Provenance () . Index ("mouse") >= 0; }

// for mouse event, flips y and z, to help simulate pushback with mouse
static inline Vect ZFlippedPhysThru (OEPointingEvent *e)
{ const Vect ept = e -> PhysThrough ();
  if (ItsAMouse (e))
    return Vect (ept.x, ept.z, ept.y);
  else
    return ept;
}

static inline float64 AngularIncrementFromWandPointingMoveEvent (
  OEPointingEvent *e)
{ Vect t1 = e -> PhysThrough ();
  Vect t0 = e -> PrevThrough ();
  Vect o1 = e -> PhysOrigin ();
  Vect o0 = e -> PrevOrigin ();
  t1.y = 0;
  o1.y = 0;
  t0.y = 0;
  o0.y = 0;
  Vect v1 = (t1 - o1) . Norm ();
  Vect v0 = (t0 - o0) . Norm ();
  Vect cross = v1 . Cross (v0);
  float64 sign = (cross.y >= 0.0 ? 1.0 : -1.0);
  return -4.0 * sign * v1 . AngleWith (v0);
}

//  Function to calculate the earthquake data GL point size
static inline float ComputePointSize (float m)
{ float output = m * m * m * m * Feld () -> Width () / 30.0; //300.0
 if (output < 1.5)
   output = 1.5;
 return output;
}

static inline Vect LatLongToSphere (float64 radius, float64 lat, float64 lng)
{ lat *= (M_PI / 180.0);
  lng *= (M_PI / 180.0);
  return Vect (cos (lat) * sin (lng) * radius,
               sin (lat) * radius,
               cos (lat) * cos (lng) * radius);
}


//  Functions to determine the Min, Max, and Avg of the data
static inline float MinValue (const ObTrove <float64> &t)
{ float min = FLT_MAX;
  for (int i = 0; i < t . Count (); i++)
    if (t . Nth (i) < min)
      min = t . Nth (i);
  return min;
}

static inline float MaxValue (const ObTrove <float64> &t)
{ float max = FLT_MIN;
  for (int i = 0; i < t . Count (); i++)
    if (t . Nth (i) > max)
      max = t . Nth (i);
  return max;
}

static inline float AvgValue (const ObTrove <float64> &t)
{ float avg = 0;
  if (t . Count () == 0)
    return 0;
  for (int i = 0; i < t . Count (); i++)
    avg = avg + t . Nth (i);
  return avg / (float)t . Count ();
}
