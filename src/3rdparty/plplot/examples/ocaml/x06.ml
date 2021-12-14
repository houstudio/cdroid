(*
        Font demo.
*)

open Printf
open Plplot

(*--------------------------------------------------------------------------*\
 * Displays the entire "plpoin" symbol (font) set.
\*--------------------------------------------------------------------------*)
let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  for kind_font = 0 to 1 do
    plfontld kind_font;
    let maxfont =
      if kind_font = 0 then
        1
      else
        4
    in
    for font = 0 to maxfont - 1 do
      plfont (font + 1);

      pladv 0;

      (* Set up viewport and window *)
      plcol0 2;
      plvpor 0.1 1.0 0.1 0.9;
      plwind 0.0 1.0 0.0 1.3;

      (* Draw the grid using plbox *)
      plbox "bcg" 0.1 0 "bcg" 0.1 0;

      (* Write the digits below the frame *)
      plcol0 15;
      for i = 0 to 9 do
        plmtex "b" 1.5 (0.1 *. float_of_int i +. 0.05) 0.5 (string_of_int i);
      done;

      let k = ref 0 in
      for i = 0 to 12 do
        (* Write the digits to the left of the frame *)
        plmtex "lv" 1.0 (1.0 -. (2.0 *. float_of_int i +. 1.0) /. 26.0) 1.0
          (string_of_int (10 * i));
        for j = 0 to 9 do
          let x = 0.1 *. float_of_int j +. 0.05 in
          let y = 1.25 -. 0.1 *. float_of_int i in

          (* Display the symbols *)
          let () =
            if (!k < 128) then
              plpoin [|x|] [|y|] !k
            else
              ()
          in
          incr k;
        done
      done;

      if kind_font = 0 then
        plmtex "t" 1.5 0.5 0.5 "PLplot Example 6 - plpoin symbols (compact)"
      else
        plmtex "t" 1.5 0.5 0.5 "PLplot Example 6 - plpoin symbols (extended)";
    done;
  done;

  plend ();
  ()
