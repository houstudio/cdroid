(*
  3-d line and point plot demo.  Adapted from x08c.c.
*)

open Plplot
open Printf

let opt = [|1; 0; 1; 0|]
let alt = [|20.0; 35.0; 50.0; 65.0|]
let az = [|30.0; 40.0; 50.0; 60.0|]

let npts = 1000

let pi = atan 1.0 *. 4.0

let test_poly k =
  let draw = [| [| true;  true;  true;  true|];
                [| true; false;  true; false|];
                [|false;  true; false;  true|];
                [| true;  true; false; false|] |] in

  let two_pi = 2.0 *. pi in

  pladv 0;
  plvpor 0.0 1.0 0.0 0.9;
  plwind (-1.0) 1.0 (-0.9) 1.1;
  plcol0 1;
  plw3d 1.0 1.0 1.0 (-1.0) 1.0 (-1.0) 1.0 (-1.0) 1.0 alt.(k) az.(k);
  plbox3 "bnstu" "x axis" 0.0 0
         "bnstu" "y axis" 0.0 0
         "bcdmnstuv" "z axis" 0.0 0;

  plcol0 2;

  let theta a = two_pi *. float_of_int a /. 20.0 in
  let phi a = pi *. float_of_int a /. 20.1 in

  let x = Array.make 5 0.0 in
  let y = Array.make 5 0.0 in
  let z = Array.make 5 0.0 in

  for i = 0 to 19 do
    for j = 0 to 19 do
      x.(0) <- sin (phi j) *. cos (theta i);
      y.(0) <- sin (phi j) *. sin (theta i);
      z.(0) <- cos (phi j);

      x.(1) <- sin (phi (j + 1)) *. cos (theta i);
      y.(1) <- sin (phi (j + 1)) *. sin (theta i);
      z.(1) <- cos (phi (j + 1));

      x.(2) <- sin (phi (j + 1)) *. cos (theta (i + 1));
      y.(2) <- sin (phi (j + 1)) *. sin (theta (i + 1));
      z.(2) <- cos (phi (j + 1));

      x.(3) <- sin (phi j) *. cos (theta (i+1));
      y.(3) <- sin (phi j) *. sin (theta (i+1));
      z.(3) <- cos (phi j);

      x.(4) <- sin (phi j) *. cos (theta i);
      y.(4) <- sin (phi j) *. sin (theta i);
      z.(4) <- cos (phi j);

      plpoly3 x y z draw.(k) true;
    done
  done;

  plcol0 3;
  plmtex "t" 1.0 0.5 0.5 "unit radius sphere";
  ()

(*--------------------------------------------------------------------------*\
 * Does a series of 3-d plots for a given data set, with different
 * viewing options in each plot.
\*--------------------------------------------------------------------------*)
let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  for k = 0 to 3 do
    test_poly k
  done;

  (* From the mind of a sick and twisted physicist... *)
  let z =
    Array.init npts (
      fun i ->
        ~-.1.0 +. 2.0 *. float_of_int i /. float_of_int npts
    )
  in
  let x =
    Array.mapi (
      fun i r ->
        r *. cos (2.0 *. pi *. 6.0 *. float_of_int i /. float_of_int npts)
    ) z
  in
  let y =
    Array.mapi (
      fun i r ->
        r *. sin (2.0 *. pi *. 6.0 *. float_of_int i /. float_of_int npts)
    ) z
  in

  for k = 0 to 3 do
    pladv 0;
    plvpor 0.0 1.0 0.0 0.9;
    plwind (-1.0) 1.0 (-0.9) 1.1;
    plcol0 1;
    plw3d 1.0 1.0 1.0 (-1.0) 1.0 (-1.0) 1.0 (-1.0) 1.0 alt.(k) az.(k);
    plbox3 "bnstu" "x axis" 0.0 0
           "bnstu" "y axis" 0.0 0
           "bcdmnstuv" "z axis" 0.0 0;

    plcol0 2;

    if opt.(k) <> 0 then
      plline3 x y z
    else
      (* U+22C5 DOT OPERATOR. *)
      plstring3 x y z "⋅";

    plcol0 3;
    let title =
      sprintf "#frPLplot Example 18 - Alt=%.0f, Az=%.0f" alt.(k) az.(k)
    in
    plmtex "t" 1.0 0.5 0.5 title;
  done;

  (* Clean up *)
  plend ();
  ()

