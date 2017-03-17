
/* (c)  oblong industries */

#include <libLoam/c/ob-vers.h>
#include <libTwillig/JSONWrite.h>
#include <libTwillig/WebThing.h>

using namespace oblong;

class GreenWebThing  :  public greenhouse::FlatNode <twillig::WebThing>
{ PATELLA_SUBCLASS (GreenWebThing, twillig::WebThing);

public:
  GreenWebThing (const Str &url = Str ()) :
      FlatNode <twillig::WebThing> (1920, 1080)
    { SetShouldInhale  (true);
      SetShouldExhale  (true);
      SetShouldTravail (true);

// if you're using g-speak 3.28 and up this populates the `ob` global in
// any loaded webpage
#if G_SPEAK_VERSION_NUMBER > 32700
      AppendLoadFinishedHook ([] (WebThing * wt, const Str & loaded_url, int code)
      { wt -> ExecuteJavascript (ObJSONFullyDefine (*wt));
        return OB_OK;
        });
#endif

      if (! url . IsEmpty ())
        NavigateTo (url);

    }

  virtual ~GreenWebThing () {}


  virtual void PointingInsideHarden  (OEPointingEvent *e)
    { OB_WARN (Str ("pointing Inside harden in widget.  Oblivious? ") +
               (QueryObliviousness () ?  "true" : "false"));

      if (QueryObliviousness ())
        return;

      WebThing::OEPointingHarden ((OEPointingHardenEvent *)e, NULL);
    }

  void PointingInsideMove (OEPointingEvent *e)
    { if (QueryObliviousness ())
        return;

      WebThing::OEPointingMove ((OEPointingMoveEvent *)e, NULL);
    }

  void PointingSoften (OEPointingEvent *e)
    { WebThing::OEPointingSoften ((OEPointingSoftenEvent *)e, NULL); }

  void PointingInsideSoften (OEPointingEvent *e)
    { if (QueryObliviousness ())
        return;
      WebThing::OEPointingSoften ((OEPointingSoftenEvent *)e, NULL);
    }

  void Blurt (OEBlurtEvent *e)
    { if (QueryObliviousness ())
        return;

      WebThing::OEBlurtAppear ((OEBlurtAppearEvent *)e, NULL);
    }

  bool HitCheck (OEPointingEvent *e)
    { if (QueryObliviousness ())
        return false;

      Vect from, to, hit;
      if (! PerpendicularizeToFeld (e, from, to))
        return false;

      from = InvWranglePoint (from);
      to = InvWranglePoint (to);

      bool did_hit = GeomSlab::RayRectIntersection (from, to,
#if G_SPEAK_VERSION_NUMBER >= 32900
                                                    LocOf (H_Align::Left, V_Align::Bottom),
#else
                                                    LocOf (V::Bottom, H::Left),
#endif
                                                    Width  () * WebThing::Over (),
                                                    Height () * WebThing::Up (),
                                                    nullptr);
      if (did_hit  &&  Alpha ()  >  1.0e-5)
        e -> SetAdvisedlyHandled (true);
      return did_hit;
    }

};
