
/* (c)  oblong industries */

#ifndef THIS_RIGHT_HERE_IS_THE_BORDER
#define THIS_RIGHT_HERE_IS_THE_BORDER

#define GLOBE_RADIUS 120.0

#include <libLoam/c/ob-vers.h>


//  Stores and displays global country border data
class CountryBorders  :  public LocusNode <oblong::staging::LineStrip>
#if G_SPEAK_VERSION_NUMBER > 32700
, public Shaderific
#endif 
{
 public:

  Table table;
  ObTrove <float64> latitude;
  ObTrove <float64> longitude;
  bool viewDataAsGlobe;
  //  Borders are stored as a continuous line strip;
  //  third column in the data set tells whether the point
  //  should be drawn or not
  //  0 = invisible, 1 = visible
  ObTrove <int64> drawitude;

  Str vertex_shader, fragment_shader;

  CountryBorders (bool isGlobe)
    { SetName ("country_borders_dataset");
      
      viewDataAsGlobe = isGlobe;

      table . Load ("data/Tissot_indicatrix_world_map_equirectangular_proj_"
                    "360x180_coords_cleaner2.txt", true);

      //  Interpret the 0th and 1th column in the data as floats,
      //  and the 2th column as ints
      longitude = table . FloatColumn (0);
      latitude  = table . FloatColumn (1);
      drawitude = table . IntColumn (2);
      SetVertexCount (table . RowCount ());

      if (viewDataAsGlobe)
        vertex_shader = StrFromFile ("shaders/seismo-foggy.vert");
      else
        vertex_shader = StrFromFile ("shaders/null.vert");

      fragment_shader = StrFromFile ("shaders/null.frag");
      UpdatePointLocations();
      UpdatePointColors ();

      SetVerticesReady ();
    }

  void UpdatePointLocations ()
    { float64 max_lat, max_lon, min_lat, min_lon;
      max_lat = max_lon = min_lat = min_lon = 0.0;
      for (int64 i = 0; i < table . RowCount (); i++)
        { //  hack to get vertice data from a vector 2D map to a globe :
          //  1) convert the vector map these points were based on to lat/lon
          //  2) - 0.2 lon and 0.25 lat to fix the calibration
          float64 mapped_longitude
            = Range (longitude . Nth (i), 0, 360, -180, 180) - 0.2;

          float64 mapped_latitude
            = Range (latitude . Nth (i), 0, 180, 90, -90) + 0.25;

          if (mapped_latitude > max_lat) max_lat = mapped_latitude;
          if (mapped_latitude < min_lat) min_lat = mapped_latitude;
          if (mapped_longitude > max_lon) max_lon = mapped_longitude;
          if (mapped_longitude < min_lon) min_lon = mapped_longitude;

          // set the vertice locations
          if (viewDataAsGlobe)
            { SetVertexLoc (i, LatLongToSphere (GLOBE_RADIUS,
                                                mapped_latitude,
                                                mapped_longitude));
            }
          else
            { Vect flat_loc = Vect (mapped_longitude * GLOBE_RADIUS / 75,
                                    mapped_latitude * GLOBE_RADIUS / 75,
                                    0);
              SetVertexLoc (i, flat_loc);
            }
        }
        OB_INFORM ("Min (lat, lon):", min_lat, ", ", min_lon);
        OB_INFORM ("Max (lat, lon):", max_lat, ", ", max_lon);
    }

  void UpdatePointColors ()
    { for (int64 i = 0; i < table . RowCount (); i++)
        { if (drawitude . Nth (i) == 2)
            SetVertexColor (i, MakeHSB (.5, 0, 1, 0));
          else
            SetVertexColor (i, MakeHSB (.5, 0, 1, drawitude . Nth (i)));
        }
    }

  //  Called once per render loop; where we provide input to shaders
  void Shade ()
    { SpaceFeld *sf = radloc::FeldOf (radloc::ToRadial (Loc ()) . x);
      if (viewDataAsGlobe)
        { uniform (fog_radius, GLOBE_RADIUS);
          uniform (system_distance, Loc () . DistFrom (CameraLoc ()));
          uniform (camera_position, sf -> Camera () -> ViewLoc ());
        }
      else
        { uniform (fog_radius, GLOBE_RADIUS * 4);
          uniform (system_distance, Loc () . DistFrom (CameraLoc ()));
          uniform (camera_position, sf -> Camera () -> ViewLoc ());
        }
      AppendVertexShaderCode (vertex_shader);
      AppendFragmentShaderCode (fragment_shader);
    }

  void PreDraw ()
    { //  This keeps GL from interpolating color between endpoints of the line strip.
      //  So if one end of the line has alpha==0, the whole line doesn't draw.
      //  That's what we want; the data has single rows of "0" alpha points.
      glShadeModel (GL_FLAT);
    }

  void PostDraw ()
    { //  Set back to default
      glShadeModel (GL_SMOOTH);
    }
};

#endif /* THIS_RIGHT_HERE_IS_THE_BORDER */
