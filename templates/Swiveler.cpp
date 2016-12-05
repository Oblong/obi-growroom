
/* (c)  oblong industries */

#include <libStaging/libBasement/SecondOrderChaseQuat.h>
#include "Swiveler.h"
#include "SeismoUtils.h"
#include <cmath>

using namespace oblong::greenhouse;


Swiveler::Swiveler (const Slaw &config)  :  Cascaderific (config)
{ // note: gets config from "control-band-config"
  move_handled_this_cycle = false;
  running_inc_angle       = 0.0;
  fake_swivel_mode      = false;
  pause_swiveler        = false;

  ParticipateInPool (Global::ChiefPool());
  float64 tau = ChaseHelpers::TauFromHalfLife (ChaseHelpers::HalfLifeFromFrac (0.25));
  InstallRotation (new SecondOrderChaseQuat (Rotation (), tau));
}

void Swiveler::Travail ()
{ move_handled_this_cycle = false;
}

void Swiveler::Metabolize (Protein const &p)
{ if (HasDescrip (p, "band-drag") && !pause_swiveler)
    { Quat rotation;
      if (p  .  Ingests ()  .  Find ("rot")  .  Into (rotation))
        SetRotation (rotation);
    }
  else if (HasDescrip (p, "begin-band-drag"))
    { }
  else if (HasDescrip (p, "end-band-drag"))
    { }
  else if (HasDescrip (p, "reset"))
    { SetRotation (Vect (0, 1, 0), 0);
      pause_swiveler = false;
    }
  else if (HasDescrip (p, "inc-planes-start"))
    { pause_swiveler = true; }
  else if (HasDescrip (p, "inc-planes-stop") || HasDescrip (p, "push-stop"))
    { pause_swiveler = false; }

}

void Swiveler::PointingHarden (OEPointingEvent *e)
{ if (! IsHeedless ())    // refuse the 2nd wand if heeding the 1st already
    return;

  if (e  ->  PhysAim ().y  >  0.8  ||  (ItsAMouse (e)  &&  fake_swivel_mode)
      ||  e  ->  PhysAim ().y  <  - 0.8)
    { OB_INFORM ("return out of rotating the system");
      return;
    }

  Heed (e);
  running_inc_angle = 0;
  drag_start_rotation = Rotation ();
  Deposit ({Slaw::List ("begin-band-drag"), Slaw::Map ()}, Global::ChiefPool());
}

void Swiveler::PointingMove (OEPointingEvent *e)
{ if (! IsHeeding (e)  ||  move_handled_this_cycle)
    return;

  move_handled_this_cycle = true;

  float64 inc_angle = - AngularIncrementFromWandPointingMoveEvent (e);
  if (ItsAMouse (e)) // LAPTOP DEBUGGING w 'p'
    inc_angle = IntersectionDiff (e) . Dot (Feld () -> Over ()) / 100.0;

  running_inc_angle += inc_angle;

  if (std::isnan (running_inc_angle)) // return if wand's gone missing
    OB_WARN ("NaN increment angle is bad, wand must have been lost");
  else
    { Quat rot_add;
      rot_add . LoadQRotFromAxisAngle (Vect (0, 1, 0), //InvWrangleDirec (Feld () -> Up ()),
                                       running_inc_angle);
      Quat new_rotation = (drag_start_rotation * rot_add) . Norm ();
      Deposit ({Slaw::List ("band-drag"),
                Slaw::Map ("rot", Slaw (new_rotation))
               }, Global::ChiefPool());
    }
}
void Swiveler::PointingSoften (OEPointingEvent *e)
{ if (IsHeeding (e))
    { StopHeeding ();
      Deposit ({Slaw::List ("end-band-drag"), Slaw::Map ()}, Global::ChiefPool());
    }
}

void Swiveler::Blurt (OEBlurtEvent *e)
{ const float64 rotation_amount    = M_PI / 36.0;
  auto f = Feld ("main");

  if ((e->Wordstamp() == "p"))
    fake_swivel_mode = ! fake_swivel_mode;
  else if ((e->Wordstamp() == "w"))
    IncRotation (WrangleDirec (f -> Over ()), -rotation_amount);
  else if ((e->Wordstamp() == "s"))
    IncRotation (WrangleDirec (f -> Over ()), rotation_amount);
  else if ((e->Wordstamp() == "a"))
    IncRotation (WrangleDirec (f -> Up ()), -rotation_amount);
  else if ((e->Wordstamp() == "d"))
    IncRotation (WrangleDirec (f -> Up ()), rotation_amount);
}



