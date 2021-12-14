(*
        Histogram demo.
*)

open Plplot

let pi = atan 1.0 *. 4.0

let npts = 2047

(*--------------------------------------------------------------------------*\
 * Draws a histogram from sample data.
\*--------------------------------------------------------------------------*)

let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  (* Fill up data points *)

  let delta = 2.0 *. pi /. float_of_int npts in
  let data = Array.init npts (fun i -> sin (float_of_int i *. delta)) in

  plcol0 1;
  plhist data (-1.1) 1.1 44 [PL_HIST_DEFAULT];
  plcol0 2;
  pllab "#frValue" "#frFrequency"
    "#frPLplot Example 5 - Probability function of Oscillator";

  plend ();
  ()

