(*
        Log plot demo.
*)

open Plplot

let pi = atan 1.0 *. 4.0

(*--------------------------------------------------------------------------*\
 * plot1
 *
 * Log-linear plot.
\*--------------------------------------------------------------------------*)

let plot1 plot_type =
  pladv 0;

  (* Set up data for log plot *)
  let f0 = 1.0 in
  let freql = Array.init 101 (fun i -> -2.0 +. float_of_int i /. 20.0) in
  let freq = Array.map (fun x -> 10.0**x) freql in
  let ampl =
    Array.map (
      fun x -> 20.0 *. log10 (1.0 /. sqrt (1.0 +. (x /. f0)**2.0))
    ) freq
  in
  let phase =
    Array.map (fun x -> -.(180.0 /. pi) *. atan (x /. f0)) freq
  in

  plvpor 0.15 0.85 0.1 0.9;
  plwind (-2.0) 3.0 (-80.0) 0.0;

  (* Try different axis and labelling styles. *)
  plcol0 1;
  let () =
    match plot_type with
        0 ->
          plbox "bclnst" 0.0 0 "bnstv" 0.0 0;
      | 1 ->
          plbox "bcfghlnst" 0.0 0 "bcghnstv" 0.0 0;
      | _ -> failwith "Bad plot type specified"
  in

  (* Plot ampl vs freq *)
  plcol0 2;
  plline freql ampl;
  plcol0 2;
  plptex 1.6 (-30.0) 1.0 (-20.0) 0.5 "-20 dB/decade";

  (* Put labels on *)
  plcol0 1;
  plmtex "b" 3.2 0.5 0.5 "Frequency";
  plmtex "t" 2.0 0.5 0.5 "Single Pole Low-Pass Filter";
  plcol0 2;
  plmtex "l" 5.0 0.5 0.5 "Amplitude (dB)";

  (* For the gridless case, put phase vs freq on same plot *)
  if plot_type = 0 then (
    plcol0 1;
    plwind (-2.0) 3.0 (-100.0) 0.0;
    plbox "" 0.0 0 "cmstv" 30.0 3;
    plcol0 3;
    plline freql phase;
    plstring freql phase "#(728)";
    plcol0 3;
    plmtex "r" 5.0 0.5 0.5 "Phase shift (degrees)";

    (* Draw a legend *)
    (* First legend entry. *)
    let opt_array = [| [PL_LEGEND_LINE]; [PL_LEGEND_LINE; PL_LEGEND_SYMBOL] |] in
    let text_colors = [| 2; 3 |] in
    let text = [| "Amplitude"; "Phase shift" |] in
    let line_colors = [| 2; 3 |] in
    let line_styles = [| 1; 1 |] in
    let line_widths = [| 1.0; 1.0 |] in
    (* note from the above opt_array the first symbol (and box) indices
       do not matter *)

    (* Second legend entry. *)
    let symbol_colors = [| 0; 3 |] in
    let symbol_scales = [| 0.0; 1.0 |] in
    let symbol_numbers = [| 0; 4 |] in
    let symbols = [| ""; "#(728)" |] in
    (* from the above opt_arrays we can completely ignore everything
       to do with boxes *)

    plscol0a 15 32 32 32 0.70;
    ignore (
      pllegend [PL_LEGEND_BACKGROUND; PL_LEGEND_BOUNDING_BOX] []
        0.0 0.0 0.1 15
        1 1 0 0
        opt_array
        1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols
    );
    ()
  )
  else if plot_type = 1 then (
    (* Draw a legend *)
    (* First legend entry. *)
    let opt_array = [| [PL_LEGEND_LINE] |] in
    let text_colors = [| 2 |] in
    let text = [| "Amplitude" |] in
    let line_colors = [| 2 |] in
    let line_styles = [| 1 |] in
    let line_widths = [| 1.0 |] in
    (* note from the above opt_array the first symbol (and box) indices
       do not matter *)

    (* from the above opt_arrays we can completely ignore everything
       to do with boxes and symbols *)

    plscol0a 15 32 32 32 0.70;
    ignore (
      pllegend [PL_LEGEND_BACKGROUND; PL_LEGEND_BOUNDING_BOX] []
        0.0 0.0 0.1 15
        1 1 0 0
        opt_array
        1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        [||] [||] [||] [||]
    );
    ()
  )

(*--------------------------------------------------------------------------*\
 * Illustration of logarithmic axes, and redefinition of window.
\*--------------------------------------------------------------------------*)

let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit();
  plfont 2;

  (* Make log plots using two different styles. *)
  plot1 0;
  plot1 1;

  plend ();
  ()

