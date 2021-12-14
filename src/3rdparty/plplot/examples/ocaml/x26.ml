(*
   Multi-lingual version of the first page of example 4.

   Copyright (C) 2006 Alan Irwin
   Copyright (C) 2006 Andrew Ross
   Copyright (C) 2008 Hezekiah M. Carty

   Thanks to the following for providing translated strings for this example:
   Valery Pipin (Russian)

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   PLplot is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with PLplot; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

*)

(*
  This example designed just for devices (e.g., psttfc and the
  cairo-related devices) that use the pango and fontconfig libraries. The
  best choice of glyph is selected by fontconfig and automatically rendered
  by pango in way that is sensitive to complex text layout (CTL) language
  issues for each unicode character in this example. Of course, you must
  have the appropriate TrueType fonts installed to have access to all the
  required glyphs.

  Translation instructions: The strings to be translated are given by
  x_label, y_label, alty_label, title_label, and line_label below.  The
  encoding used must be UTF-8.

  The following strings to be translated involve some scientific/mathematical
  jargon which is now discussed further to help translators.

  (1) dB is a decibel unit, see http://en.wikipedia.org/wiki/Decibel .
  (2) degrees is an angular measure, see
      http://en.wikipedia.org/wiki/Degree_(angle) .
  (3) low-pass filter is one that transmits (passes) low frequencies.
  (4) pole is in the mathematical sense, see
      http://en.wikipedia.org/wiki/Pole_(complex_analysis) .  "Single Pole"
      means a particular mathematical transformation of the filter function has
      a single pole, see
      http://ccrma.stanford.edu/~jos/filters/Pole_Zero_Analysis_I.html .
      Furthermore, a single-pole filter must have an inverse square decline
      (or -20 db/decade). Since the filter plotted here does have that
      characteristic, it must by definition be a single-pole filter, see also
      http://www-k.ext.ti.com/SRVS/Data/ti/KnowledgeBases/analog/document/faqs/1p.htm
  (5) decade represents a factor of 10, see
      http://en.wikipedia.org/wiki/Decade_(log_scale) .
*)

open Plplot

let x_label = [|
  "Frequency";
  "Частота";
|]

let y_label = [|
  "Amplitude (dB)";
  "Амплитуда (dB)";
|]

let alty_label = [|
  "Phase shift (degrees)";
  "Фазовый сдвиг (градусы)";
|]

let title_label = [|
  "Single Pole Low-Pass Filter";
  "Однополюсный Низко-Частотный Фильтр";
|]

let line_label = [|
  "-20 dB/decade";
  "-20 dB/десяток";
|]

(* Short rearranged versions of y_label and alty_label. *)
let legend_text = [|
  [|"Amplitude"; "Phase shift"|];
  [|"Амплитуда"; "Фазовый сдвиг"|];
|]

let pi = atan 1.0 *. 4.0

(*--------------------------------------------------------------------------*\
 * plot1
 *
 * Log-linear plot.
\*--------------------------------------------------------------------------*)
let plot1 label_type x_label y_label alty_label legend_text title_label line_label =
  pladv 0;

  (* Set up data for log plot *)
  let freql = Array.init 101 (fun i -> ~-.2.0 +. float_of_int i /. 20.0) in
  let freq = Array.map (fun f -> 10.0**f) freql in
  let f0 = 1.0 in
  let ampl =
    Array.map (
      fun f ->
        20.0 *. log10 (1.0 /. sqrt (1.0 +. (f /. f0)**2.0))
    ) freq
  in
  let phase =
    Array.map (
      fun f ->
        ~-.(180.0 /. pi) *. atan (f /. f0)
    ) freq
  in

  plvpor 0.15 0.85 0.1 0.9;
  plwind (-2.0) 3.0 (-80.0) 0.0;

  (* Try different axis and labelling styles. *)

  plcol0 1;
  let () =
    match label_type with
        0 ->
        plbox "bclnst" 0.0 0 "bnstv" 0.0 0;
      | 1 ->
        plbox "bcfghlnst" 0.0 0 "bcghnstv" 0.0 0;
      | _ -> raise (Failure "invalid label_type")
  in

  (* Plot ampl vs freq *)

  plcol0 2;
  plline freql ampl;
  plcol0 2;
  plptex 1.6 (-30.0) 1.0 (-20.0) 0.5 line_label;

  (* Put labels on *)
  plcol0 1;
  plmtex "b" 3.2 0.5 0.5 x_label;
  plmtex "t" 2.0 0.5 0.5 title_label;
  plcol0 2;
  plmtex "l" 5.0 0.5 0.5 y_label;

  (* For the gridless case, put phase vs freq on same plot *)
  if label_type = 0 then (
    plcol0 1;
    plwind (-2.0) 3.0 (-100.0) 0.0;
    plbox "" 0.0 0 "cmstv" 30.0 3;
    plcol0 3;
    plline freql phase;
    plstring freql phase "#(728)";
    plcol0 3;
    plmtex "r" 5.0 0.5 0.5 alty_label;
  );

  (* Draw a legend *)
  (* First legend entry. *)
  let opt_array = [| [PL_LEGEND_LINE]; [PL_LEGEND_LINE; PL_LEGEND_SYMBOL] |] in
  let text_colors = [| 2; 3 |] in
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
  let _, _ =
    pllegend [PL_LEGEND_BACKGROUND; PL_LEGEND_BOUNDING_BOX] []
      0.0 0.0 0.10 15
      1 1 0 0
      opt_array
      1.0 1.0 2.0
      1.0 text_colors legend_text
      [||] [||] [||] [||]
      line_colors line_styles line_widths
      symbol_colors symbol_scales symbol_numbers symbols
  in
  ()

(*--------------------------------------------------------------------------*\
 * Illustration of logarithmic axes, and redefinition of window.
\*--------------------------------------------------------------------------*)
let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();
  plfont 2;

  (* Make log plots using two different styles. *)
  Array.iteri (
    fun i xl ->
      plot1 0 xl y_label.(i) alty_label.(i) legend_text.(i) title_label.(i) line_label.(i)
  ) x_label;

  plend ();
  ()

