(*
   Illustrates backdrop plotting of world, US maps.
   Contributed by Wesley Ebisuzaki.

   Updated for OCaml by Hezekiah Carty

Copyright 2007, 2008 Hezekiah M. Carty
Copyright 2018 Alan W. Irwin

This file is part of PLplot.

PLplot is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

PLplot is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with PLplot.  If not, see <http://www.gnu.org/licenses/>.
*)

open Plplot
open Printf

(*--------------------------------------------------------------------------*\
 * mapform19
 *
 * Defines specific coordinate transformation for example 19.
 * Not to be confused with mapform in src/plmap.c.
 * x[], y[] are the coordinates to be plotted.
\*--------------------------------------------------------------------------*)

let pi = atan 1.0 *. 4.0

(* The mapform callback gets a single pair of x and y coordinates and their
   index.
   It should return the properly transformed coordinates a an (x,y) tuple. *)
let mapform19 x y =
  let radius = 90.0 -. y in
  let xp = radius *. cos (x *. pi /. 180.0) in
  let yp = radius *. sin (x *. pi /. 180.0) in
  (xp, yp)

(* "Normalize" longitude values so that they always fall between -180.0 and
   180.0 *)
let normalize_longitude lon =
  if lon >= -180.0 && lon <= 180.0 then
    lon
  else
    let times = floor ((abs_float lon +. 180.0) /. 360.0) in
    if lon < 0.0 then
      lon +. 360.0 *. times
    else
      lon -. 360.0 *. times

(* A custom axis labeling function for longitudes and latitudes. *)
let geolocation_labeler axis value =
  let label_val, direction_label =
    match axis with
    | PL_Y_AXIS ->
        let label_val = value in
        label_val,
          if label_val > 0.0 then
            " N"
          else if label_val < 0.0 then
            " S"
          else
            "Eq"
    | PL_X_AXIS ->
        let label_val = normalize_longitude value in
        label_val,
          if label_val > 0.0 then
            " E"
          else if label_val < 0.0 then
            " W"
          else
            ""
    | PL_Z_AXIS -> invalid_arg "Invalid axis - only X or Y are supported"
  in
  if axis = PL_Y_AXIS && label_val = 0.0 then
    (* A special case for the equator *)
    sprintf "%s" direction_label
  else
    sprintf "%.0f%s" (abs_float label_val) direction_label

(*--------------------------------------------------------------------------*\
 * main
 *
 * Shows two views of the world map.
\*--------------------------------------------------------------------------*)

let () =
  (* Longitude (x) and latitude (y) *)
  let miny = -70.0 in
  let maxy = 80.0 in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  plinit();

  (* Cartesian plots *)
  (* Most of world *)

  let minx = -170.0 in
  let maxx = minx +. 360.0 in

  (* Setup a custom latitude and longitude-based scaling function. *)
  plslabelfunc geolocation_labeler;

  plcol0 1;
  plenv minx maxx miny maxy 1 70;
  (* No transform function is passed to plmap.  Since we have not set one yet,
     it defaults to using an identity function (xp = x, yp = y) *)
  plmap "usaglobe" minx maxx miny maxy;

  (* The Americas *)
  let minx = 190.0 in
  let maxx = 340.0 in

  plcol0 1;
  plenv minx maxx miny maxy 1 70;
  (* Again, we have not set a transform.  Everything remains in a Cartesian
     projection. *)
  plmap "usaglobe" minx maxx miny maxy;

  (* Clear the labeling function *)
  plunset_labelfunc ();

  (* Polar, Northern hemisphere *)
  let minx = 0.0 in
  let maxx = 360.0 in

  plenv (-75.) 75. (-75.) 75. 1 (-1);
  (* Now we set mapform19 as our callback.  It will be used for every following
     plmap and plmeridians call until set to something else. *)
  plset_mapform mapform19;
  plmap "globe" minx maxx miny maxy;

  pllsty 2;
  (* This call to plmeridians is also after the set_mapform call, so it uses
     the same projection as the plmap call above. *)
  plmeridians 10.0 10.0 0.0 360.0 (-10.0) 80.0;

  plunset_mapform ();

  (* Polar, Northern hemisphere, this time with a PLplot-wide transform *)
  let minx = 0.0 in
  let maxx = 360.0 in

  plstransform mapform19;

  pllsty 1;
  plenv (-75.) 75. (-75.) 75. 1 (-1);
  (* No need to set the map transform here as the global transform will be
     used. *)
  plmap "globe" minx maxx miny maxy;

  pllsty 2;
  plmeridians 10.0 10.0 0.0 360.0 (-10.0) 80.0;

  (* Show Baltimore, MD on the map *)
  plcol0 2;
  plssym 0.0 2.0;
  plpoin [|-76.6125|] [|39.2902778|] 18;
  plssym 0.0 1.0;
  plptex ~-.76.6125 43.0 0.0 0.0 0.0 "Baltimore, MD";

  (* For OCaml, this is how the global transform is cleared *)
  plunset_transform ();

  (* An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
     These were chosen because they provide shapefiles for small grid boxes which
     are easilly manageable for this demo.*)

  pllsty 1;

  (* Drop unneeded calculations....
  let minx = 240570.0 in
  let maxx = 621109.0 in
  let miny = 87822.0 in
  let maxy = 722770.0 in

  plscol0 0 255 255 255;
  plscol0 1 0 0 0;
  plscol0 2 150 150 150;
  plscol0 3 0 50 200;
  plscol0 4 50 50 50;
  plscol0 5 150 0 0;
  plscol0 6 100 100 255;
  end drop of unneeded calculations *)

  let minx = 265000.0 in
  let maxx = 270000.0 in
  let miny = 145000.0 in
  let maxy = 150000.0 in

  plscol0 0 255 255 255; (*white*)
  plscol0 1 0 0 0;       (*black*)
  plscol0 2 255 200 0;   (*yelow for sand*)
  plscol0 3 60 230 60;   (* green for woodland*)
  plscol0 4 210 120 60;  (*brown for contours*)
  plscol0 5 150 0 0;     (*red for major roads*)
  plscol0 6 180 180 255; (*pale blue for water*)
  plscol0 7 100 100 100; (*pale grey for shingle or boulders*)
  plscol0 8 100 100 100; (*dark grey for custom polygons - generally crags*)

  plcol0 1;
  plenv minx maxx miny maxy 1 (-1) ;
  pllab "" "" "Martinhoe CP, Exmoor National Park, UK (shapelib only)";

  (*Beach*)
  plcol0 2 ;
  let beachareas = [|23; 24|] in
  plmapfill "ss/ss64ne_Landform_Area" minx maxx miny maxy beachareas;

  (*woodland*)
  plcol0 3;
  let nwoodlandareas = 94 in
  let woodlandareas = Array.make nwoodlandareas 0 in
  for iwoodlandareas = 0 to nwoodlandareas - 1 do
    woodlandareas.(iwoodlandareas) <- iwoodlandareas + 218;
  done;
  plmapfill "ss/ss64ne_Landform_Area" minx maxx miny maxy woodlandareas;

  (*shingle or boulders*)
  plcol0 7;
  let shingleareas = [|0; 1; 24; 25; 26; 27; 28; 29; 30; 31; 32; 33; 34; 35; 217; 2424; 2425; 2426; 2427; 2428; 2491; 2577 |] in
  plmapfill "ss/ss64ne_Landform_Area" minx maxx miny maxy shingleareas;

  (*crags*)
  plcol0 8;
  let ncragareas = 2024 in
  let cragareas = Array.make ncragareas 0 in
  for icragareas = 0 to ncragareas - 1 do
    cragareas.(icragareas) <- icragareas + 325;
  done;
  plmapfill "ss/ss64ne_Landform_Area" minx maxx miny maxy cragareas;

  (*draw contours, we need to separate contours from high/low coastline*)
  (*draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy );*)
  plcol0 4;
  plmapline "ss/ss64ne_Height_Contours" minx maxx miny maxy [||];

  (*draw the sea and surface water*)
  plwidth 0.0;
  plcol0 6;
  plmapfill "ss/ss64ne_Water_Area" minx maxx miny maxy [||];
  plwidth 2.0;
  plmapline "ss/ss64ne_Water_Line" minx maxx miny maxy [||];

  (*draw the roads, first with black and then thinner with colour to give*)
  (*an outlined appearance*)
  plwidth 5.0;
  plcol0 1;
  plmapline "ss/ss64ne_Road_Centreline" minx maxx miny maxy [||];
  plwidth 3.0;
  plcol0 0;
  plmapline "ss/ss64ne_Road_Centreline" minx maxx miny maxy [||];
  plcol0 5;
  let majorroads = [| 33; 48; 71; 83; 89; 90; 101; 102; 111 |] in
  plmapline "ss/ss64ne_Road_Centreline" minx maxx miny maxy majorroads;

  (*draw buildings*)
  plwidth 1.0;
  plcol0 1;
  plmapfill "ss/ss64ne_Building_Area" minx maxx miny maxy [||];

  (*labels*)
  plsfci 0x80000100L;
  plschr 0. 0.8;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "MARTINHOE CP" minx maxx miny maxy 202;
  plschr 0. 0.7;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Heale\nDown" minx maxx miny maxy 13;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "South\nDown" minx maxx miny maxy 34;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Martinhoe\nCommon" minx maxx miny maxy 42;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Woody Bay" minx maxx miny maxy 211;
  plschr 0. 0.6;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Mill Wood" minx maxx miny maxy 16;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Heale Wood" minx maxx miny maxy 17;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 1.0 "Bodley" minx maxx miny maxy 31;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.0 "Martinhoe" minx maxx miny maxy 37;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Woolhanger\nCommon" minx maxx miny maxy 60;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "West Ilkerton\nCommon" minx maxx miny maxy 61;
  plmaptex "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Caffyns\nHeanton\nDown" minx maxx miny maxy 62;

  plend ();
  ()
