// Needed for g-speak version
#include <libLoam/c/ob-vers.h>

// Installed under {{g_speak_home}}
#include <libGreenhouse/Greenhouse.h> // Calls Setup()

// growroom includes
#include <libGrowroom/JSGreenhouseExecutor.h>  // Needed to run JS code
#include <libGrowroom/greenhouse_v8_binding.h> // Global growroom bindings
#include <libGrowroom/PoolListener.h>          // Run code from pool

// We're transitioning to some new stuff in g-speak 3.28, this
// supports both 3.26 and 3.28
#if G_SPEAK_VERSION_NUMBER > 32700
#include <libGrowroom/REPLHelper.h>            // Run code from REPL
#endif

// From staging
#include <libStaging/libPlasma/c++/Slawzall.h>  // Emit/Delve sugar
#include <libStaging/libBasement/Lumberjack.h>  // logging (OB_INFORM, OB_WARN)
#include <libStaging/libTwillig/Barbiturate.h>  // Support for application sedation and revival
#include <libStaging/libTwillig/RadLoc.h>      // Room-agnostic feld positioning
#include <libStaging/libRad/Global.h>          // Magical functions
#include <libStaging/libTwillig/SynchroPod.h>  // Multi-machine
#include <libStaging/libNoodoo/FeldUtil.h>     // SetFeldsColor

// From g-speak
#include <libLoam/c++/ObTrove.h>    // array class
#include <libLoam/c++/ObColor.h>    // color class
#include <libLoam/c++/Vect.h>       // Vectors
#include <libBasement/Find.h>       // for searching in the g-speak scene graph
#include <libBasement/UrDrome.h>    // Application object
#include <libTwillig/HandiPoint.h>  // cursors
#include <libLoam/c/ob-log.h>       // Logging flags

// Local growroom bindings
#include "generated/{{project_name_symbol}}_binding.h"

using namespace oblong;




void CommonSetup (plasma::Slaw const &config)
{ // Common setup options before your custom options

  // PID for core dumps
  basement::UrDrome::WritePID ("{{project_name}}", getpid ());

  // All usage of OB_[INFORM|WARN] is sent to our log pool
  staging::SendLogsToPool (Global::LogPool ());

  // Many classes inherit from the Cascaderific mixin, which means they can
  // quickly look up settings in themselves or up their parent chain.
  // (See libStaging/libRad/Cascaderific.h).
  // This function establishes the top-level environment which
  // is searched when the chain tops out.  Lookups can always successfully
  // find attributes which live at the top level of the config protein.
  Cascaderific::InstallTopLevel (config);

  // Initialize the radloc address system
  staging::radloc::InitRadLoc (config);

  // Application's background color, specified by config
  staging::SetFeldsColor (
    staging::Emit <loam::ObColor> (config, "background-color"));

  // Initialize Touch
  Str touch_pool("touch");
  OB_INFORM ("Setting up touch on ", touch_pool);
  UrDrome *drome = UrDrome::OutermostDrome ();
  VisiFeld *vf = VisiFeld::NthOfAllVisiFelds (0);
  auto tris = drome -> Find <Tristero> ("*");
  Gestator *g_nadir = new Gestator;
  g_nadir -> SetName ("touch-gestator");
  g_nadir -> SetSenseCase (new DualCase (150));
  g_nadir -> SetWandsPoolName (touch_pool);
  WandPointingRod *wprod = new WandPointingRod ();
  wprod -> SetNumButtons (3);
  wprod -> SetSpaceContext (vf);
  wprod -> SetLingerDuration (1.25);
  wprod -> SetGenus ("wand");
  wprod -> SetWordstamp ("wand");
  RoddedGlimpser *wglim_two = new RoddedGlimpser ();
  wglim_two -> SetProtoRod (wprod);
  wglim_two -> SetName ("touch-glimpser");
  g_nadir -> AppendGlimpser (wglim_two);
  drome -> AppendChild (g_nadir);
  wglim_two -> AppendEventTarget (tris);
  OB_INFORM ("Finished Touch Init");
 

  // Initialize handipoint size, based on a setting in the config file
  auto const handi_size = staging::radloc::HeightFrac (
                          staging::Emit <float64> (config, "handi-size"));
  loam::ObTrove <twillig::HandiPoint *> hps =
    basement::find::Gather <twillig::HandiPoint> ("*");
  for (int64 i = 0  ;  i  <  hps . Count ()  ;  i++)
    { hps . Nth (i) -> SetSize (handi_size, handi_size);
      hps . Nth (i) -> SetShouldShowOffFeldMarker (false);
    }
}

// PostSyncHook is a helper that calls the provided callback once the
// application is running and synchronized across all machines
struct PostSyncHook  :  public greenhouse::Thing
{ std::function <void (PostSyncHook *)> callback_;
  PostSyncHook (std::function <void (PostSyncHook *)> c) : callback_ (c) {}

  // Called after the applications are synced
  void TemporalReset () OB_OVERRIDE
    { static bool do_once = true;
      if (do_once)
        { do_once = false;
          callback_ (this);
        }
    }
};
  

void Setup ()
{ //  This output is driven by the contents of config.protein --
  //  read libRad/Global.h, it won't bite
  OB_INFORM ("Command Line Args: " + ToStr (Global::CmdLineArgs ()));
  OB_INFORM ("Config Protein: "    + ToStr (Global::ConfigProtein ()));
  OB_INFORM ("Site Protein: "      + ToStr (Global::SiteProtein ()));
  OB_INFORM ("Chief Pool: "        + Global::ChiefPool ());
  OB_INFORM ("Log Pool: "          + Global::LogPool ());
  OB_INFORM ("Search Path List: "  + ToStr (Global::SearchPath ()));

  if (! Feld ())
    { OB_WARN ("Couldn't make any windows -- quitting.");
      UrDrome::OutermostDrome () -> SetContinueRespiring (false);
      return;
    }

  plasma::Protein const config = staging::Global::ConfigProtein ();
  plasma::Protein const site = staging::Global::SiteProtein ();

  // Merge the config and site config, prefering the configuration from the
  // config protein when conflicts arise.  The policy is, per-app configuration
  // wins over shared configuration.
  plasma::Slaw const app_config = site . Ingests () . MapMerge (config . Ingests ());

  CommonSetup (app_config);

  auto *js_executor = new growroom::JSExecutor (growroom::JSExecutorSettingsFromCmdLine (args));

  // Bind global growroom symbols
#if G_SPEAK_VERSION_NUMBER > 32700
  js_executor -> AddJSBinding (Bind_loam_plasma_basement_noodoo_afferent_impetus_twillig_staging_greenhouse_growroom_easing_In_Global);
#else
  js_executor -> AddJSBinding (Bind_loam_plasma_basement_noodoo_afferent_impetus_twillig_staging_greenhouse_growroom_In_Global);
#endif

  // Bind local growroom symbols
  js_executor -> AddJSBinding (Bind_{{project_name_symbol}}_In_Global);
 
  // JSExecutor needs to respire to process JS code, so append it to something that respires
  Feld () -> AppendChild (js_executor);


  // Enable live coding by watching for JS in `growroom-pool` and
  // in a REPL on port 4242
  Str input_pool = Global::GetPoolName ("growroom-pool");
  OB_LOG_INFO (":: watching for JS input on pool %s", input_pool . utf8 ());
  js_executor -> AppendChild (new growroom::PoolListener (js_executor, input_pool));

// We're transitioning to some new stuff in g-speak 3.28, this
// supports both 3.26 and 3.28
#if G_SPEAK_VERSION_NUMBER > 32700
  js_executor -> AppendChild (growroom::repl::MakeJS (js_executor));
#endif

  // Let's get to running
  js_executor -> Go ();

  // ----------------------------------------------------------------------
  // A d d    C u s t o m    C + +    C o d e    H e r e
  // ----------------------------------------------------------------------

}
