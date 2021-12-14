(*
  Bar chart demo.
*)

open Plplot

let plfbox x0 y0 =
  let x = [|x0; x0; x0 +. 1.0; x0 +. 1.0|] in
  let y = [|0.0; y0; y0; 0.0|] in
  plfill x y;
  plcol0 1;
  pllsty 1;
  plline x y;
  ()

(*--------------------------------------------------------------------------*\
 * Does a simple bar chart, using color fill.  If color fill is
 * unavailable, pattern fill is used instead (automatic).
\*--------------------------------------------------------------------------*)

let pos = [|0.0; 0.25; 0.5; 0.75; 1.0|]
let red = [|0.0; 0.25; 0.5; 1.00; 1.0|]
let green = [|1.0; 0.50; 0.5; 0.50; 1.0|]
let blue = [|1.0; 1.0; 0.5; 0.25; 0.0|]

let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  pladv 0;
  plvsta ();
  plwind 1980.0 1990.0 0.0 35.0;
  plbox "bc" 1.0 0 "bcnv" 10.0 0;
  plcol0 2;
  pllab "Year" "Widget Sales (millions)" "#frPLplot Example 12";

  let y0 = [|5.0; 15.0; 12.0; 24.0; 28.0; 30.0; 20.0; 8.0; 12.0; 3.0|] in

  plscmap1l true pos red green blue None;

  for i = 0 to 9 do
    (* plcol0 (i + 1); *)
    plcol1(float_of_int i /. 9.0);
    plpsty 0;
    plfbox (1980.0 +. float_of_int i) y0.(i);
    let text = Printf.sprintf "%.0f" y0.(i) in
    plptex (1980.0 +. float_of_int i +. 0.5) (y0.(i) +. 1.0) 1.0 0.0 0.5 text;
    let text = string_of_int (1980 + i) in
    plmtex "b" 1.0 (float_of_int (i + 1) *. 0.1 -. 0.05) 0.5 text;
  done;

  (* Don't forget to call plend() to finish off! *)
  plend ();
  ()

