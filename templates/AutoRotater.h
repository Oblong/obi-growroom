
/* (c)  oblong industries */

#pragma once

#include "libStaging/libRad/Cascaderific.h"
#include "libGreenhouse/GreenhouseHeaders.h"
#include "libStaging/libBasement/OneShotTimer.h"
#include <libBasement/QuadraticQuat.h>

using namespace oblong::staging;

/**
 */
struct AutoRotater  :  public oblong::greenhouse::Thing,
                       public Cascaderific
{ float64 rotation_speed;
  bool    rotate_paused;
  Quat    base_rotation;

  FLAG_MACHINERY (AutoRotateEnabled);

  /**
    */
  AutoRotater (const Slaw &config)  :  Cascaderific (config)
    { ParticipateInPool (Global::ChiefPool());
      // QuickParticipateInPool (this);
      SetAutoRotateEnabled (true);

      rotation_speed = M_PI / 200.0;
      rotate_paused  = false;
      base_rotation . LoadQRotFromAxisAngle (Vect (0, 1, 0), 0.0);

      InstallRotation (new QuadraticQuat (Rotation (), 0.3));
    }

  void Travail ()
    { if (QueryAutoRotateEnabled ()  &&  ! rotate_paused)
        { Quat q;
          q . LoadQRotFromAxisAngle (Vect (0, 1, 0),
                                     CurTime () * rotation_speed);
          SetRotationHard ((q * base_rotation) . Norm ());
        }
    }

  void StartAutoRotation ()
    { SetAutoRotateEnabled (true);
      rotate_paused = false;
      base_rotation = Rotation ();
      ZeroTime ();
    }

  void StopAutoRotation ()
    { SetAutoRotateEnabled (false);
      SetRotation (InvWrangleDirec (Vect (0, 1, 0)), 0); //Feld () -> Up ()
    }

  void Metabolize (Protein const &p)
    { if (HasDescrip (p, "inc-planes-start"))
        { rotate_paused = true;
        }
      else if (HasDescrip (p, "inc-planes-stop"))
        { rotate_paused = false;
          base_rotation = Rotation ();
          ZeroTime ();
          // SetAutoRotateEnabled (true);
        }
      else if (HasDescrip (p, "start-auto-rotation"))
        { StartAutoRotation ();
        }
      else if (HasDescrip (p, "stop-auto-rotation"))
        { StopAutoRotation ();
        }

      // APP SPECIFIC METABOLIZATION
      else if (HasDescrip (p, "reset"))
        { rotate_paused = false;
          SetAutoRotateEnabled (true);
          SetRotation (InvWrangleDirec (Feld () -> Up ()), 0);

          // AppendTrigger (new oblong::staging::OneShotTimer (0.4, [this] ()
          //                { StartAutoRotation (); }));
        }
    }
};
