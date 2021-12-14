(*
        plshade demo, using color fill.

        Maurice LeBrun
        IFS, University of Texas at Austin
        20 Mar 1994
*)

open Plplot


(* Comment this out since Plot.colorbar which it wraps sets a color and removes it afterwards
which causes problems when comparing results with other languages.

let colorbar ?color ?contour values =
  (* Smaller text *)
  plschr 0.0 0.75;
  (* Small ticks on the vertical axis *)
  plsmaj 0.0 0.5;
  plsmin 0.0 0.5;

  let axis =
    [
      `frame0;
      `frame1;
      `vertical_label;
      `unconventional_label;
      `major_ticks;
    ]
  in
  let shade = Plot.shade_colorbar ~custom:true ~axis values in
  let pos = Plot.viewport_pos ~inside:false 0.005 0.0 in
  Plot.plot [
    Plot.colorbar ?color ?contour ~orient:(`top (0.0375, 0.875)) ~label:[`bottom "Magnitude"] ~pos shade;
  ];

  (* Reset text and tick sizes *)
  plschr 0.0 1.0;
  plsmaj 0.0 1.0;
  plsmin 0.0 1.0

end of commented out colorbar.  *)

(* Define my_colorbar as a convenience for this example with all fixed arguments used
for all plcolorbar calls defined here in one place.  In addition this function
sets and restores text and tick sizes before and after the plcolorball call. *)

let my_colorbar cont_color cont_width shedge =
  (* Smaller text *)
  plschr 0.0 0.75;
  (* Small ticks on the vertical axis *)
  plsmaj 0.0 0.5;
  plsmin 0.0 0.5;

  (* Fixed arguments for this entire example. *)
  let x = 0.005 in
  let y = 0.0 in
  let x_length = 0.0375 in
  let y_length = 0.875 in
  let bg_color = 0 in
  let bb_color = 1 in
  let bb_style = 1 in
  let low_cap_color = 0.0 in
  let high_cap_color = 0.0 in
  let label_opts = [| [PL_COLORBAR_LABEL_BOTTOM] |] in
  let labels = [|"Magnitude"|] in
  let axis_strings = [|"bcvtm"|] in
  let tick_spacing = [|0.0|] in
  let sub_ticks = [|0|] in
  let values = Array.make 1 shedge in

  ignore (
  plcolorbar [PL_COLORBAR_SHADE ; PL_COLORBAR_SHADE_LABEL] []
    x y x_length y_length bg_color bb_color bb_style low_cap_color high_cap_color
    cont_color cont_width
    label_opts labels
    axis_strings tick_spacing sub_ticks values
  );

  (* Reset text and tick sizes *)
  plschr 0.0 1.0;
  plsmaj 0.0 1.0;
  plsmin 0.0 1.0

let pi = atan 1.0 *. 4.0

(* Fundamental settings.  See notes[] for more info. *)

let ns = 20             (* Default number of shade levels *)
let nx = 35             (* Default number of data points in x *)
let ny = 46             (* Default number of data points in y *)
let exclude = false     (* By default do not plot a page illustrating
                           exclusion. *)

(* polar plot data *)
let perimeterpts = 100

(* Transformation function *)
let mypltr x y tr =
  tr.(0) *. x +. tr.(1) *. y +. tr.(2),
  tr.(3) *. x +. tr.(4) *. y +. tr.(5)

let zdefined x y =
  let z = sqrt (x *. x +. y *. y) in
  if z < 0.4 || z > 0.6 then 1 else 0

(*--------------------------------------------------------------------------*\
 * f2mnmx
 *
 * Returns min & max of input 2d array.
\*--------------------------------------------------------------------------*)
let f2mnmx f =
  let fmax = ref f.(0).(0) in
  let fmin = ref f.(0).(0) in
  for i = 0 to Array.length f - 1 do
    for j = 0 to Array.length f.(i) - 1 do
      fmax := max !fmax f.(i).(j);
      fmin := min !fmin f.(i).(j);
    done
  done;
  !fmin, !fmax

(*--------------------------------------------------------------------------*\
 * Does several shade plots using different coordinate mappings.
\*--------------------------------------------------------------------------*)
let () =
  let fill_width = 2.0 in
  let cont_color = 0 in
  let cont_width = 0.0 in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Load color palettes *)
  plspal0 "cmap0_black_on_white.pal";
  plspal1 "cmap1_gray.pal" true;

  (* Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display *)
  plscmap0n 3;

  (* Initialize plplot *)
  plinit ();

  (* Set up transformation function *)
  let tr =
    [|
      2.0 /. float_of_int (nx - 1); 0.0; -1.0;
      0.0; 2.0 /. float_of_int (ny-1); -1.0;
    |]
  in

  (* Set up data arrays *)
  let z = Array.make_matrix nx ny 0.0 in
  let w = Array.make_matrix nx ny 0.0 in
  for i = 0 to nx - 1 do
    let x = float_of_int (i - (nx / 2)) /. float_of_int (nx / 2) in
    for j = 0 to ny - 1 do
      let y = float_of_int (j - (ny / 2)) /. float_of_int (ny / 2) -. 1.0 in
      z.(i).(j) <- -. sin (7.0 *. x) *. cos (7.0 *. y) +. x *. x -. y *. y;
      w.(i).(j) <- -. cos (7.0 *. x) *. sin (7.0 *. y) +. 2.0 *. x *. y;
    done
  done;

  let zmin, zmax = f2mnmx z in
  let clevel =
    Array.init ns (
      fun i ->
        zmin +. (zmax -. zmin) *. (float_of_int i +. 0.5) /. float_of_int ns
    )
  in
  let shedge =
    Array.init (ns + 1) (
      fun i ->
        zmin +. (zmax -. zmin) *. float_of_int i /. float_of_int ns
    )
  in

  (* Set up coordinate grids *)
  let xg1 = Array.make nx 0.0 in
  let yg1 = Array.make ny 0.0 in
  let xg2 = Array.make_matrix nx ny 0.0 in
  let yg2 = Array.make_matrix nx ny 0.0 in

  for i = 0 to nx - 1 do
    for j = 0 to ny - 1 do
      let x, y = mypltr (float_of_int i) (float_of_int j) tr in

      let argx = x *. pi /. 2.0 in
      let argy = y *. pi /. 2.0 in
      let distort = 0.4 in

      xg1.(i) <- x +. distort *. cos argx;
      yg1.(j) <- y -. distort *. cos argy;

      xg2.(i).(j) <- x +. distort *. cos argx *. cos argy;
      yg2.(i).(j) <- y -. distort *. cos argx *. cos argy;
    done
  done;

  (* Plot using identity transform *)
  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind (-1.0) 1.0 (-1.0) 1.0;

  plpsty 0;

  plshades z (-1.0) 1.0 (-1.0) 1.0 shedge fill_width cont_color cont_width true;

  my_colorbar 0 0.0 shedge;

  plcol0 1;
  plbox "bcnst" 0.0 0 "bcnstv" 0.0 0;
  plcol0 2;
  pllab "distance" "altitude" "Bogon density";

  (* Plot using 1d coordinate transform *)

  (* Load color palettes *)
  plspal0 "cmap0_black_on_white.pal";
  plspal1 "cmap1_blue_yellow.pal" true;

  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind (-1.0) 1.0 (-1.0) 1.0;

  plpsty 0;

  plset_pltr (pltr1 xg1 yg1);
  plshades z (-1.0) 1.0 (-1.0) 1.0 shedge fill_width cont_color cont_width true;

  my_colorbar 0 0.0 shedge;

  plcol0 1;
  plbox "bcnst" 0.0 0 "bcnstv" 0.0 0;
  plcol0 2;
  pllab "distance" "altitude" "Bogon density";

  (* Plot using 2d coordinate transform *)

  (* Load color palettes *)
  plspal0 "cmap0_black_on_white.pal";
  plspal1 "cmap1_blue_red.pal" true;

  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind (-1.0) 1.0 (-1.0) 1.0;

  plpsty 0;

  plset_pltr (pltr2 xg2 yg2);
  plshades
    z (-1.0) 1.0 (-1.0) 1.0 shedge fill_width cont_color cont_width false;

  my_colorbar 0 0.0 shedge;

  plcol0 1;
  plbox "bcnst" 0.0 0 "bcnstv" 0.0 0;
  plcol0 2;
  plcont w 1 nx 1 ny clevel;

  pllab "distance" "altitude" "Bogon density, with streamlines";

  (* Plot using 2d coordinate transform *)

  (* Load color palettes *)
  plspal0 "";
  plspal1 "" true;

  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind (-1.0) 1.0 (-1.0) 1.0;

  plpsty 0;

  plshades z (-1.0) 1.0 (-1.) 1.0 shedge fill_width 2 3.0 false;

  my_colorbar 2 3.0 shedge;

  plcol0 1;
  plbox "bcnst" 0.0 0 "bcnstv" 0.0 0;
  plcol0 2;

  pllab "distance" "altitude" "Bogon density";

  (* Note this exclusion API will probably change. *)

  (* Plot using 2d coordinate transform and exclusion*)
  if exclude then (
    pladv 0;
    plvpor 0.1 0.9 0.1 0.9;
    plwind (-1.0) 1.0 (-1.0) 1.0;

    plpsty 0;

    plset_defined zdefined;
    plshades
      z (-1.0) 1.0 (-1.0) 1.0 shedge fill_width cont_color cont_width false;

    my_colorbar 0 0.0 shedge;
    plunset_defined ();

    plcol0 1;
    plbox "bcnst" 0.0 0 "bcnstv" 0.0 0;

    pllab "distance" "altitude" "Bogon density with exclusion";
  );
  (* Example with polar coordinates. *)

  (* Load colour palettes*)
  plspal0 "cmap0_black_on_white.pal";
  plspal1 "cmap1_gray.pal" true;

  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind (-1.0) 1.0 (-1.0) 1.0;
  plpsty 0;

  (* Build new coordinate matrices. *)
  for i = 0 to nx - 1 do
    let r = float_of_int i /. float_of_int (nx - 1) in
    for j = 0 to ny - 1 do
      let t = (2.0 *. pi /. (float_of_int ny -. 1.0)) *. float_of_int j in
      xg2.(i).(j) <- r *. cos t;
      yg2.(i).(j) <- r *. sin t;
      z.(i).(j) <- exp (~-.r *. r) *. cos (5.0 *. pi *. r) *. cos (5.0 *. t);
    done
  done;

  (* Need a new shedge to go along with the new data set. *)
  let zmin, zmax = f2mnmx z in

  let shedge =
    Array.init (ns + 1) (
      fun i ->
        zmin +. (zmax -. zmin) *. float_of_int i /. float_of_int ns
    )
  in

  (*  Now we can shade the interior region. *)
  plshades
    z (-1.0) 1.0 (-1.0) 1.0 shedge fill_width cont_color cont_width false;

  my_colorbar 0 0.0 shedge;

  (* Now we can draw the perimeter.  (If do before, shade stuff may overlap.) *)
  let px = Array.make perimeterpts 0.0 in
  let py = Array.make perimeterpts 0.0 in
  for i = 0 to perimeterpts - 1 do
    let t = (2.0 *. pi /. float_of_int (perimeterpts - 1)) *. float_of_int i in
    px.(i) <- cos t;
    py.(i) <- sin t;
  done;
  plcol0 1;
  plline px py;

  (* And label the plot.*)
  plcol0 2;
  pllab "" "" "Tokamak Bogon Instability";

  (* Clean up *)
  plend ();
  ()

