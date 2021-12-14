(*
        Font demo.
*)

open Plplot

let base =
  [|0; 100; 0; 100; 200; 500; 600; 700; 800; 900;
    2000; 2100; 2200; 2300; 2400; 2500; 2600; 2700; 2800; 2900|]

(*--------------------------------------------------------------------------*\
 * Displays the entire "plsym" symbol (font) set.
\*--------------------------------------------------------------------------*)

let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  plfontld 0;
  for l = 0 to 19 do
    if l = 2 then plfontld 1;
    pladv 0;

    (* Set up viewport and window *)
    plcol0 2;
    plvpor 0.15 0.95 0.1 0.9;
    plwind 0.0 1.0 0.0 1.0;

    (* Draw the grid using plbox *)
    plbox "bcg" 0.1 0 "bcg" 0.1 0;

    (* Write the digits below the frame *)
    plcol0 15;
    for i = 0 to 9 do
      plmtex "b" 1.5 (0.1 *. float_of_int i +. 0.05) 0.5 (string_of_int i);
    done;

    let k = ref 0 in
    for i = 0 to 9 do
      (* Write the digits to the left of the frame *)
      plmtex "lv" 1.0 (0.95 -. 0.1 *. float_of_int i) 1.0
        (string_of_int (base.(l) + 10 * i));
      for j = 0 to 9 do
        let x = 0.1 *. float_of_int j +. 0.05 in
        let y = 0.95 -. 0.1 *. float_of_int i in

        (* Display the symbols *)
        plsym [|x|] [|y|] (base.(l) + !k);
        incr k;
      done;
    done;

    if l < 2 then
      plmtex "t" 1.5 0.5 0.5 "PLplot Example 7 - PLSYM symbols (compact)"
    else
      plmtex "t" 1.5 0.5 0.5 "PLplot Example 7 - PLSYM symbols (extended)"
  done;
  plend ();
  ()

