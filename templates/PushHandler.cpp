
/* (c)  oblong industries */

#include <libStaging/libBasement/SecondOrderChaseVect.h>
#include <libStaging/libRad/Global.h>

#include "PushHandler.h"
#include "SeismoUtils.h"


using namespace oblong::greenhouse;

PushHandler::PushHandler (const Slaw &config)  :
      Cascaderific (config),
      fake_pushup_mode (false),
      fake_pushdown_mode (false),
      fake_pushover_mode (false),
      fake_pushnorm_mode (false),
      pushup (false),
      pushdown (false),
      pushover (false),
      pushnorm (false),
      current_phase (0)
    { ParticipateInPool (Global::ChiefPool());
      float64 tau = ChaseHelpers::TauFromHalfLife (ChaseHelpers::HalfLifeFromFrac (0.25));
      InstallTranslation (new SecondOrderChaseVect (Translation (), tau));
    }
    
void PushHandler::Metabolize (Protein const &p)
{ 
  if (HasDescrip (p, "push-start") || HasDescrip (p, "push-stop"))
    { starting_transl = Ingest <Vect> (p, "starting-transl");
    }
  else if (HasDescrip (p, "push"))
    { Vect t = Ingest <Vect> (p, "transl");
      SetTranslation (starting_transl + t);
    }
  else if (HasDescrip (p, "reset"))
    { SetTranslation (Feld () -> Loc ());
    }

  // APP SPECIFIC METABOLIZATION
  // else if (HasDescrip (p, "zoom-to-node"))
  //   { Str n = Ingest <Str> (p, "name");
  //     Vect l = Ingest <Vect> (p, "loc");

  //     float64 n_dist = (InvWranglePoint (CameraLoc ()) - InvWranglePoint (Feld () -> Loc ())) . Dot (InvWrangleDirec (Feld () -> Norm ()));
  //     n_dist *= .85;
  //     Vect c = Feld () -> Loc () + InvWrangleDirec (Feld () -> Norm ()) * n_dist; 
  //     SetTranslation (Loc () + (c - l));
  //   }
}
void PushHandler::PointingHarden (OEPointingEvent *e)
{ 
  // if (QueryObliviousness ())
  //   return;

  if (! IsHeedless ())    // refuse the 2nd wand if heeding the 1st already
    return;

  click_timer . ZeroTime ();

  if ((e -> PhysAim ().y > 0.8 && ! ItsAMouse (e))//CascadingFind <float64> ("point-at-ceiling-y-component")
      ||  (ItsAMouse (e)  &&  fake_pushup_mode))
    { Heed (e);
      pushup = true;
      pushdown = false;
      pushover = false;
      pushnorm = false;
      Protein p = Protein (Slaw::List ("push-start"), Slaw::Map ("starting-transl",
                                                         Loc ()));
      Deposit (p, Global::ChiefPool());
      pushup_start_phys_through = ZFlippedPhysThru (e);
    }
  else if ((e -> PhysAim ().y < -0.8  && ! ItsAMouse (e)) //CascadingFind <float64> ("point-at-floor-y-component"))
           ||  (ItsAMouse (e)  &&  fake_pushdown_mode))
    { Heed (e);
      pushup = false;
      pushdown = true;
      pushover = false;
      pushnorm = false;

      // pushup_start_phys_through = ZFlippedPhysThru (e);
      // Deposit (ProteinWithDescrip ("inc-planes-start"), Global::ChiefPool ());
    }
  else if ((e -> PhysAim ().x < -0.8  && ! ItsAMouse (e)) //CascadingFind <float64> ("point-at-floor-y-component"))
           ||  (ItsAMouse (e)  &&  fake_pushover_mode))
    { Heed (e);
      pushup = false;
      pushdown = false;
      pushover = true;
      pushnorm = false;

      pushup_start_phys_through = ZFlippedPhysThru (e);
      Deposit (ProteinWithDescrip ("inc-planes-start"), Global::ChiefPool ());
    }
  else if ((e -> PhysAim ().z < -0.8  && ! ItsAMouse (e)) // TIME PUSH TURNED OFF HERE, EVERYTHING ELSE IN PLACE
      ||  (ItsAMouse (e)  &&  fake_pushnorm_mode))
    { Heed (e);
      pushup = false;
      pushnorm = true;
      pushover = false;
      pushdown = false;
      // Protein p = ProteinWithDescrip ("push-time-start");
      // Deposit (p, Global::ChiefPool());
      pushup_start_phys_through = ZFlippedPhysThru (e);
    }
}

void PushHandler::PointingMove (OEPointingEvent *e)
{ if (IsHeeding (e))
    { if (pushnorm && e -> PhysAim ().z > -0.975)
        { Deposit (ProteinWithDescrip ("time-push-stop"), Global::ChiefPool ());
          pushnorm = false;
        }

      if (pushnorm)
        { if (click_timer.CurTime () > 1.2)
            { Vect diff = -(ZFlippedPhysThru (e) - pushup_start_phys_through);
              HandleTimePush (diff, ItsAMouse (e));
            }
        }
      else if (pushup)
        { Vect diff = (ZFlippedPhysThru (e) - pushup_start_phys_through);
          HandlePushBack (diff, ItsAMouse (e));
        }
      else if (pushover)
        { Vect diff = (ZFlippedPhysThru (e) - pushup_start_phys_through);
          HandlePushOver (diff, ItsAMouse (e));
        }
    }
}

void PushHandler::PointingSoften (OEPointingEvent *e)
{ 
  // if (QueryObliviousness ())
  //   return;

  if (IsHeeding (e))
    { StopHeeding ();

      if (pushup)
        { pushup = false;
          Protein p = Protein (Slaw::List ("push-stop"), Slaw::Map ("starting-transl",
                                                                    TranslationGoal ()));
          Deposit (p, Global::ChiefPool ());
        }

      if (pushnorm)
        { Deposit (ProteinWithDescrip ("time-push-stop"), Global::ChiefPool ());
          pushnorm = false;
        }

      if (pushover)
        { pushover = false;
          Deposit (ProteinWithDescrip ("inc-planes-stop"), Global::ChiefPool ());
        }

      if (pushdown)
        { pushdown = false;
          Deposit (ProteinWithDescrip ("reset"), Global::ChiefPool()); 
        }
    }
}

void PushHandler::HandleTimePush (const Vect &diff_from_start, const bool &mouser)
{ const static float64 ARM_LENGTH = 200.0;
  float64 diff_norm = diff_from_start . Dot (Feld () -> Norm ());
  float64 pushbck_norm_prcnt = - diff_norm / ARM_LENGTH;
  if (pushbck_norm_prcnt > 1.0) pushbck_norm_prcnt = 1.0;
  if (pushbck_norm_prcnt < 0) pushbck_norm_prcnt = 0;

  Protein p = Protein (Slaw::List ("time-push"), Slaw::Map (
                                                       "norm-prcnt",
                                                       pushbck_norm_prcnt));
  Deposit (p, Global::ChiefPool());
}

void PushHandler::HandlePushBack (const Vect &diff_from_start, const bool &mouser)
{ float64 pushup_translation_speed = 4.5;
  Vect new_translation = diff_from_start * pushup_translation_speed;
  
  const static float64 ARM_LENGTH = 200.0;
  // NORM ONLY
  // Vect new_translation = ZeroVect;
  float64 diff_norm = diff_from_start . Dot (Feld () -> Norm ());
  diff_norm *= pushup_translation_speed;
  // new_translation = Feld () -> Norm () * diff_norm;
  float64 pushbck_norm_prcnt = diff_norm / ARM_LENGTH;

  Protein p = Protein (Slaw::List ("push"), Slaw::Map ("transl",
                                                       new_translation,
                                                       "norm-prcnt",
                                                       pushbck_norm_prcnt));
  Deposit (p, Global::ChiefPool());
}

void PushHandler::HandlePushOver (const Vect &diff_from_start, const bool &mouser)
{ Protein p = Protein (Slaw::List ("inc-planes-cursor"), 
                        Slaw::Map ("inc-transl", diff_from_start));
  Deposit (p, Global::ChiefPool());
}

void PushHandler::Blurt (OEBlurtEvent *e)
{ 
  // if (QueryObliviousness ())
  //   return;
  const float64 translation_amount = 40.0;
  auto f = Feld ("main");

  if ((e->Wordstamp() == "p"))
    fake_pushup_mode = ! fake_pushup_mode;
  else if ((e->Wordstamp() == "o"))
    fake_pushover_mode = ! fake_pushover_mode;
  else if ((e->Wordstamp() == "t"))
    fake_pushnorm_mode = ! fake_pushnorm_mode;
  else if ((e->Wordstamp() == "i"))
    IncTranslation (translation_amount * 2 * f -> Norm ());
  else if ((e->Wordstamp() == "k"))
    IncTranslation (-translation_amount * 2 * f -> Norm ());
  else if ((e->Wordstamp() == "j"))
    IncTranslation (-translation_amount * f -> Over ());
  else if ((e->Wordstamp() == "l"))
    IncTranslation (translation_amount * f -> Over ());
  else if ((e->Wordstamp() == "y"))
    IncTranslation (-translation_amount * f -> Up ());
  else if ((e->Wordstamp() == "h"))
    IncTranslation (translation_amount * f -> Up ());
}
