(*
        Shade plot demo.

        Maurice LeBrun
        IFS, University of Texas at Austin
        31 Aug 1993
*)

open Plplot

let xpts = 35 (* Data points in x *)
let ypts = 46 (* Data points in y *)

(*--------------------------------------------------------------------------*\
 * cmap1_init1
 *
 * Initializes color map 1 in HLS space.
\*--------------------------------------------------------------------------*)

let cmap1_init1 () =
  let i = [|0.0; 0.45; 0.55; 1.0|] in
  let h = [|260.0; 260.0; 20.0; 20.0|] in
  let l = [|0.5; 0.0; 0.0; 0.5|] in
  let s = [|1.0; 1.0; 1.0; 1.0|] in
  plscmap1l false i h l s None;
  ()

(*--------------------------------------------------------------------------*\
 * cmap1_init2
 *
 * Initializes color map 1 in HLS space.
\*--------------------------------------------------------------------------*)

let cmap1_init2 () =
  let i = [|0.0; 0.45; 0.55; 1.0|] in
  let h = [|260.0; 260.0; 20.0; 20.0|] in
  let l = [|0.6; 0.0; 0.0; 0.6|] in
  let s = [|1.0; 0.5; 0.5; 1.0|] in
  plscmap1l false i h l s None;
  ()

(*--------------------------------------------------------------------------*\
 * plot1
 *
 * Illustrates a single shaded region.
\*--------------------------------------------------------------------------*)

let plot1 z zmin zmax =
  let sh_cmap = 0 in

  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind (-1.0) 1.0 (-1.0) 1.0;

  (* Plot using identity transform *)
  let shade_min = zmin +. (zmax -. zmin) *. 0.4 in
  let shade_max = zmin +. (zmax -. zmin) *. 0.6 in
  let sh_color = 7.0 in
  let sh_width = 2.0 in
  let min_color = 9 in
  let max_color = 2 in
  let min_width = 2.0 in
  let max_width = 2.0 in

  plpsty 8;
  plshade
    z (-1.0) 1.0 (-1.0) 1.0
    shade_min shade_max
    sh_cmap sh_color sh_width
    min_color min_width max_color max_width true;

  plcol0 1;
  plbox "bcnst" 0.0 0 "bcnstv" 0.0 0;
  plcol0 2;
  pllab "distance" "altitude" "Bogon flux";
  ()

(*--------------------------------------------------------------------------*\
 * plot2
 *
 * Illustrates multiple adjacent shaded regions, using different fill
 * patterns for each region.
\*--------------------------------------------------------------------------*)

let plot2 z zmin zmax =
  let sh_cmap = 0 in
  let min_color = 0 in
  let min_width = 0.0 in
  let max_color = 0 in
  let max_width = 0.0 in
  let sh_width = 2.0 in

  let inc =
    [|
      [|450|]; [|-450|]; [|0|]; [|900|];
      [|300|]; [|450;-450|]; [|0; 900|]; [|0; 450|];
      [|450; -450|]; [|0; 900|];
    |]
  in
  let del =
    [|
      [|2000|]; [|2000|]; [|2000|];
      [|2000|]; [|2000|]; [|2000; 2000|];
      [|2000; 2000|]; [|2000; 2000|]; [|4000; 4000|];
      [|4000; 2000|];
    |]
  in

  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind (-1.0) 1.0 (-1.0) 1.0;

  (* Plot using identity transform *)
  for i = 0 to 9 do
    let shade_min = zmin +. (zmax -. zmin) *. float_of_int i /. 10.0 in
    let shade_max = zmin +. (zmax -. zmin) *. float_of_int (i + 1) /. 10.0 in
    let sh_color = float_of_int (i + 6) in
    plpat inc.(i) del.(i);

    plshade
      z (-1.0) 1.0 (-1.0) 1.0
      shade_min shade_max
      sh_cmap sh_color sh_width
      min_color min_width max_color max_width true;
  done;

  plcol0 1;
  plbox "bcnst" 0.0 0 "bcnstv" 0.0 0;
  plcol0 2;
  pllab "distance" "altitude" "Bogon flux";
  ()

(*--------------------------------------------------------------------------*\
 * plot3
 *
 * Illustrates shaded regions in 3d, using a different fill pattern for
 * each region.
\*--------------------------------------------------------------------------*)

let plot3 () =
  let xx =
    [|
      [|-1.0; 1.0; 1.0; -1.0; -1.0|];
      [|-1.0; 1.0; 1.0; -1.0; -1.0|];
    |]
  in
  let yy =
    [|
      [|1.0; 1.0; 0.0; 0.0; 1.0|];
      [|-1.0; -1.0; 0.0; 0.0; -1.0|];
    |]
  in
  let zz =
    [|
      [|0.0; 0.0; 1.0; 1.0; 0.0|];
      [|0.0; 0.0; 1.0; 1.0; 0.0|];
    |]
  in

  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind (-1.0) 1.0 (-1.0) 1.0;
  plw3d 1.0 1.0 1.0 (-1.0) 1.0 (-1.0) 1.0 0.0 1.5 30.0 (-40.0);

  (* Plot using identity transform *)
  plcol0 1;
  plbox3 "bntu" "X" 0.0 0 "bntu" "Y" 0.0 0 "bcdfntu" "Z" 0.5 0;
  plcol0 2;
  pllab "" "" "3-d polygon filling";

  plcol0 3;
  plpsty 1;
  plline3 xx.(0) yy.(0) zz.(0);
  let sub a = Array.sub a 0 4 in
  plfill3 (sub xx.(0)) (sub yy.(0)) (sub zz.(0));
  plpsty 2;
  plline3 xx.(1) yy.(1) zz.(1);
  plfill3 (sub xx.(1)) (sub yy.(1)) (sub zz.(1));
  ()

(*--------------------------------------------------------------------------*\
 * Does a variety of shade plots.
\*--------------------------------------------------------------------------*)
let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Set up color map 1 *)
  cmap1_init2 ();

  (* Initialize plplot *)
  plinit ();

  (* Set up data array *)
  let z = Array.make_matrix xpts ypts 0.0 in
  for i = 0 to xpts - 1 do
    let xx = float_of_int (i - (xpts / 2)) /. float_of_int (xpts / 2) in
    for j = 0 to ypts - 1 do
      let yy =
        float_of_int (j - (ypts / 2)) /. float_of_int (ypts / 2) -. 1.0
      in
      z.(i).(j) <-
        xx *. xx -. yy *. yy +. (xx -. yy) /. (xx *. xx +. yy *. yy +. 0.1);
    done
  done;
  let zmax, zmin = plMinMax2dGrid z in

  plot1 z zmin zmax;
  plot2 z zmin zmax;
  plot3 ();

  plend ();
  ()

