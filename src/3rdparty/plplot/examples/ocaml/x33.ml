(*
Demonstrate most pllegend capability including unicode symbols.

Copyright (C) 2010-2018 Alan W. Irwin
Copyright (C) 2011 Hezekiah M. Carty

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *)

(* This example designed just for devices (e.g., the cairo-related and
   qt-related devices) where the best choice of glyph is automatically
   selected by the related libraries (pango/cairo or Qt4) for each
   unicode character depending on what system fonts are installed.  Of
   course, you must have the appropriate TrueType fonts installed to
   have access to all the required glyphs. *)

open Plplot
open Printf

let position_options = [|
  [PL_POSITION_LEFT; PL_POSITION_TOP; PL_POSITION_OUTSIDE];
  [PL_POSITION_TOP; PL_POSITION_OUTSIDE];
  [PL_POSITION_RIGHT; PL_POSITION_TOP; PL_POSITION_OUTSIDE];
  [PL_POSITION_RIGHT; PL_POSITION_OUTSIDE];
  [PL_POSITION_RIGHT; PL_POSITION_BOTTOM; PL_POSITION_OUTSIDE];
  [PL_POSITION_BOTTOM; PL_POSITION_OUTSIDE];
  [PL_POSITION_LEFT; PL_POSITION_BOTTOM; PL_POSITION_OUTSIDE];
  [PL_POSITION_LEFT; PL_POSITION_OUTSIDE];
  [PL_POSITION_LEFT; PL_POSITION_TOP; PL_POSITION_INSIDE];
  [PL_POSITION_TOP; PL_POSITION_INSIDE];
  [PL_POSITION_RIGHT; PL_POSITION_TOP; PL_POSITION_INSIDE];
  [PL_POSITION_RIGHT; PL_POSITION_INSIDE];
  [PL_POSITION_RIGHT; PL_POSITION_BOTTOM; PL_POSITION_INSIDE];
  [PL_POSITION_BOTTOM; PL_POSITION_INSIDE];
  [PL_POSITION_LEFT; PL_POSITION_BOTTOM; PL_POSITION_INSIDE];
  [PL_POSITION_LEFT; PL_POSITION_INSIDE];
|]

(* Pick 5 arbitrary UTF-8 symbols useful for plotting points (✠✚✱✪✽✺✰✴✦). *)
let special_symbols = [|
  "✰";
  "✴";
  "✱";
  "✽";
  "✦";
|]

(* plcolorbar options *)

(* Colorbar type options *)
let colorbar_kinds = 4
let colorbar_option_kinds = [|
  [PL_COLORBAR_SHADE];
  [PL_COLORBAR_SHADE; PL_COLORBAR_SHADE_LABEL];
  [PL_COLORBAR_IMAGE];
  [PL_COLORBAR_GRADIENT]
|]

let colorbar_option_kind_labels = [|
  "Shade colorbars";
  "Shade colorbars with custom labels";
  "Image colorbars";
  "Gradient colorbars"
|]

(* Which side of the page are we positioned relative to? *)
let colorbar_positions = 4
let colorbar_position_options = [|
  [PL_POSITION_LEFT];
  [PL_POSITION_RIGHT];
  [PL_POSITION_TOP];
  [PL_POSITION_BOTTOM]
|]

let colorbar_position_option_labels = [|
  "Left";
  "Right";
  "Top";
  "Bottom"
|]

(* Colorbar label positioning options *)
let colorbar_labels = 4
let colorbar_label_options = [|
  [PL_COLORBAR_LABEL_LEFT];
  [PL_COLORBAR_LABEL_RIGHT];
  [PL_COLORBAR_LABEL_TOP];
  [PL_COLORBAR_LABEL_BOTTOM]
|]

let colorbar_label_option_labels = [|
  "Label left";
  "Label right";
  "Label top";
  "Label bottom"
|]

(* Colorbar cap options *)
let colorbar_caps = 4
let colorbar_cap_options = [|
  [PL_COLORBAR_CAP_NONE];
  [PL_COLORBAR_CAP_LOW];
  [PL_COLORBAR_CAP_HIGH];
  [PL_COLORBAR_CAP_LOW;PL_COLORBAR_CAP_HIGH]
|]

let colorbar_cap_option_labels = [|
  "No caps";
  "Low cap";
  "High cap";
  "Low and high caps"
|]

let plcolorbar_example_page kind_i label_i cap_i cont_color cont_width values_vector =

  pladv 0;
  (* Draw one colorbar relative to each side of the page *)
  for position_i = 0 to colorbar_positions - 1 do

    (* Smaller text *)
    plschr 0.0 0.75;
    (* Small ticks on the vertical axis *)
    plsmaj 0.0 0.5;
    plsmin 0.0 0.5;

    plvpor 0.20 0.80 0.20 0.80;
    plwind 0.0 1.0 0.0 1.0;
    (* Set interesting background colour. *)
    plscol0a 15 0 0 0 0.20;

    (* Colorbar parameters *)
    let opt = colorbar_option_kinds.(kind_i) @ colorbar_label_options.(label_i) @ colorbar_cap_options.(cap_i) @ [PL_COLORBAR_BOUNDING_BOX ; PL_COLORBAR_BACKGROUND] in

    let position = colorbar_position_options.(position_i) in

    let vertical = List.mem PL_POSITION_LEFT position || List.mem PL_POSITION_RIGHT position in
    let ifn = List.mem PL_POSITION_LEFT position || List.mem PL_POSITION_BOTTOM position in

    (* Set the offset position on the page *)
    let x = 0. in
    let y = 0. in
    let x_length = if vertical then 0.05 else 0.5 in
    let y_length = if vertical then 0.5 else 0.05 in

    let bg_color = 15 in
    let bb_color = 1 in
    let bb_style = 1 in
    let low_cap_color = 0.0 in
    let high_cap_color = 1.0 in
    let label_opts = [| [PL_COLORBAR_NULL] |] in
    let labels = [| colorbar_position_option_labels.(position_i) ^ ", " ^ colorbar_label_option_labels.(label_i) |] in

    (* equivalent C code
       if ( ifn )
       {
       if ( cont_color == 0 || cont_width == 0. )
       {
       axis_opts[0] = "uwtivn";
       //axis_opts[0] = "uwtin";
       }
       else
       {
       axis_opts[0] = "uwxvn";
       //axis_opts[0] = "uwxn";
       }
       }
       else
       {
       if ( cont_color == 0 || cont_width == 0. )
       {
       axis_opts[0] = "uwtivm";
       //axis_opts[0] = "uwtim";
       }
       else
       {
       axis_opts[0] = "uwxvm";
       //axis_opts[0] = "uwxm";
       }
       }
     *)

    let axis_opts = [|
      if
	(ifn && ( cont_color = 0 || cont_width = 0.)) then "uwtivn"
      else if
	(ifn && not ( cont_color = 0 || cont_width = 0.)) then "uwxvn"
      else if
	(not ifn && ( cont_color = 0 || cont_width = 0.)) then "uwtivm"
      else if
	(not ifn && not ( cont_color = 0 || cont_width = 0.)) then "uwxvm"
      else("")
    |] in

    let tick_spacing = [|0.0|] in
    let sub_ticks = [|0|] in
    let values_matrix = Array.make 1 values_vector in

    ignore (
    plcolorbar opt position
      x y x_length y_length bg_color bb_color bb_style low_cap_color high_cap_color
      cont_color cont_width
      label_opts labels
      axis_opts tick_spacing sub_ticks values_matrix
   );

    (* Reset text and tick sizes *)
    plschr 0.0 1.0;
    plsmaj 0.0 1.0;
    plsmin 0.0 1.0
  done;

  (* Draw a page title *)
  let title = colorbar_option_kind_labels.(kind_i) ^ " - " ^ colorbar_cap_option_labels.(cap_i) in
  plvpor 0.0 1.0 0.0 1.0;
  plwind 0.0 1.0 0.0 1.0;
  plptex 0.5 0.5 0.0 0.0 0.5 title

let plcolorbar_example palette kind_i cont_color cont_width values =

    (* Load the color palette *)
    plspal1 palette true;

  for label_i = 0 to colorbar_labels - 1 do
    for cap_i = 0 to colorbar_caps - 1 do
      plcolorbar_example_page kind_i label_i cap_i cont_color cont_width values;
    done;
  done

(* main
   Demonstrate most pllegend capability including unicode symbols. *)

let max_nlegend = 7

let () =
  let opt_array = Array.make max_nlegend [] in
  let box_colors = Array.make max_nlegend 0 in
  let box_patterns = Array.make max_nlegend 0 in
  let box_scales = Array.make max_nlegend 0.0 in
  let box_line_widths = Array.make max_nlegend 0.0 in
  let line_colors = Array.make max_nlegend 0 in
  let line_styles = Array.make max_nlegend 0 in
  let line_widths = Array.make max_nlegend 0.0 in
  let symbol_numbers = Array.make max_nlegend 0 in
  let symbol_colors = Array.make max_nlegend 0 in
  let symbol_scales = Array.make max_nlegend 0.0 in
  let symbols = Array.make max_nlegend "" in
  let text = Array.make max_nlegend "" in
  let text_colors = Array.make max_nlegend 0 in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  (* First page illustrating the 16 standard positions. *)
  pladv 0;
  plvpor 0.25 0.75 0.25 0.75;
  plwind 0.0 1.0 0.0 1.0;
  plbox "bc" 0.0 0 "bc" 0.0 0;
  plsfont PL_FCI_SANS PL_FCI_STYLE_UNCHANGED PL_FCI_WEIGHT_UNCHANGED;
  plmtex "t" 8.0 0.5 0.5 "The 16 standard legend positions with";
  plmtex "t" 6.0 0.5 0.5 "the same (0.05) offset in x and y";

  let nlegend = 1 in
  (* Only specify legend data that are required according to the
     value of opt_array for that entry. *)
  let opt_base = [PL_LEGEND_BACKGROUND; PL_LEGEND_BOUNDING_BOX] in
  opt_array.(0) <- [PL_LEGEND_LINE; PL_LEGEND_SYMBOL];
  line_styles.(0) <- 1;
  line_widths.(0) <- 1.0;
  symbol_scales.(0) <- 1.;
  symbol_numbers.(0) <- 4;
  symbols.(0) <- "#(728)";

  (* Use monotype fonts so that all legends are the same size. *)
  plsfont PL_FCI_MONO PL_FCI_STYLE_UNCHANGED PL_FCI_WEIGHT_UNCHANGED;
  plscol0a 15 32 32 32 0.70;

  for k = 0 to 15 do
    let opt_array = Array.sub opt_array 0 nlegend in
    let position = position_options.(k) in
    let opt = opt_base in
    text.(0) <- sprintf "%2.2d" k;
    text_colors.(0) <- 1 + ( k mod 8 );
    line_colors.(0) <- 1 + ( k mod 8 );
    symbol_colors.(0) <- 1 + ( k mod 8 );

    ignore (
      pllegend opt position 0.05 0.05
          0.1 15 1 1 0 0
          opt_array 1.0 1.0 2.0
          1.0 text_colors text
          [||] [||] [||] [||]
          line_colors line_styles line_widths
          symbol_colors symbol_scales symbol_numbers symbols
    );
  done;

  (* Second page illustrating effect of nrow, ncolumn for the same legend
     data. *)
  pladv 0;
  plvpor 0.25 0.75 0.25 0.75;
  plwind 0.0 1.0 0.0 1.0;
  plbox "bc" 0.0 0 "bc" 0.0 0;
  plsfont PL_FCI_SANS PL_FCI_STYLE_UNCHANGED PL_FCI_WEIGHT_UNCHANGED;
  plmtex "t" 8.0 0.5 0.5 "The effect of nrow, ncolumn, PL_LEGEND_ROW_MAJOR,";
  plmtex "t" 6.0 0.5 0.5 "and position for the same legend data";

  let nlegend = 7 in

  (* Only specify legend data that are required according to the
     value of opt_array for that entry. *)
  let opt_base = [PL_LEGEND_BACKGROUND; PL_LEGEND_BOUNDING_BOX] in
  for k = 0 to nlegend - 1 do
    opt_array.(k) <- [PL_LEGEND_LINE; PL_LEGEND_SYMBOL];
    line_styles.(k) <- 1;
    line_widths.(k) <- 1.0;
    symbol_scales.(k) <- 1.0;
    symbol_numbers.(k) <- 2;
    symbols.(k) <- "#(728)";
    text.(k) <- sprintf "%2.2d" k;
    text_colors.(k) <- 1 + (k mod 8);
    line_colors.(k) <- 1 + (k mod 8);
    symbol_colors.(k) <- 1 + (k mod 8);
  done;

  (* Use monotype fonts so that all legends are the same size. *)
  plsfont PL_FCI_MONO PL_FCI_STYLE_UNCHANGED PL_FCI_WEIGHT_UNCHANGED;
  plscol0a 15 32 32 32 0.70;

  let position = [PL_POSITION_TOP; PL_POSITION_OUTSIDE] in
  let opt = opt_base in
  let x = 0.0 in
  let y = 0.1 in
  let nrow = 1 in
  let ncolumn = nlegend in
  ignore (
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.05 15 1 1 nrow ncolumn
        opt_array 1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols;
  );

  let position = [PL_POSITION_BOTTOM; PL_POSITION_OUTSIDE] in
  let opt = opt_base in
  let x = 0.0 in
  let y = 0.1 in
  let nrow = 1 in
  let ncolumn = nlegend in
  ignore (
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.05 15 1 1 nrow ncolumn
        opt_array 1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols
  );

  let position = [PL_POSITION_LEFT; PL_POSITION_OUTSIDE] in
  let opt = opt_base in
  let x = 0.1 in
  let y = 0.0 in
  let nrow = nlegend in
  let ncolumn = 1 in
  ignore (
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.05 15 1 1 nrow ncolumn
        opt_array 1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols;
  );

  let position = [PL_POSITION_RIGHT; PL_POSITION_OUTSIDE] in
  let opt = opt_base in
  let x = 0.1 in
  let y = 0.0 in
  let nrow = nlegend in
  let ncolumn = 1 in
  ignore (
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.05 15 1 1 nrow ncolumn
        opt_array 1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols
  );

  let position = [PL_POSITION_LEFT; PL_POSITION_TOP; PL_POSITION_INSIDE] in
  let opt = opt_base in
  let x = 0.0 in
  let y = 0.0 in
  let nrow = 6 in
  let ncolumn = 2 in
  ignore (
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.05 15 1 1 nrow ncolumn
        opt_array 1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols
  );

  let position = [PL_POSITION_RIGHT; PL_POSITION_TOP; PL_POSITION_INSIDE] in
  let opt = PL_LEGEND_ROW_MAJOR :: opt_base in
  let x = 0.0 in
  let y = 0.0 in
  let nrow = 6 in
  let ncolumn = 2 in
  ignore (
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.05 15 1 1 nrow ncolumn
        opt_array 1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols
  );

  let position = [PL_POSITION_BOTTOM; PL_POSITION_INSIDE] in
  let opt = PL_LEGEND_ROW_MAJOR :: opt_base in
  let x = 0.0 in
  let y = 0.0 in
  let nrow = 3 in
  let ncolumn = 3 in
  ignore (
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.05 15 1 1 nrow ncolumn
        opt_array 1.0 1.0 2.0
        1.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols
  );

  (* Third page demonstrating legend alignment *)
  pladv 0;
  plvpor 0.0 1.0 0.0 0.9;
  plwind 0.0 1.0 0.0 1.0;
  plsfont PL_FCI_SANS PL_FCI_STYLE_UNCHANGED PL_FCI_WEIGHT_UNCHANGED;
  plmtex "t" 2.0 0.5 0.5 "Demonstrate legend alignment";

  let x = ref 0.1 in
  let y = ref 0.1 in
  let nturn = 4 in
  let nlegend = ref 0 in
  let position = ref [PL_POSITION_TOP; PL_POSITION_LEFT; PL_POSITION_SUBPAGE] in
  let opt_base = [PL_LEGEND_BACKGROUND; PL_LEGEND_BOUNDING_BOX] in
  let opt = ref opt_base in
  for i = 0 to 8 do
    (* Set up legend arrays with the correct size, type. *)
    if i <= nturn then
      incr nlegend
    else
      decr nlegend;
    nlegend := max 1 !nlegend;
    (* Only specify legend data that are required according to the
       value of opt_array for that entry. *)
    for k = 0 to !nlegend - 1 do
      opt_array.(k) <- [PL_LEGEND_LINE; PL_LEGEND_SYMBOL];
      line_styles.(k) <- 1;
      line_widths.(k) <- 1.0;
      symbol_scales.(k) <- 1.;
      symbol_numbers.(k) <- 2;
      symbols.(k) <- "#(728)";
      text.(k) <- sprintf "%2.2d" k;
      text_colors.(k) <- 1 + (k mod 8);
      line_colors.(k) <- 1 + (k mod 8);
      symbol_colors.(k) <- 1 + (k mod 8);
    done;
    (* Use monotype fonts so that all legends are the same size. *)
    plsfont PL_FCI_MONO PL_FCI_STYLE_UNCHANGED PL_FCI_WEIGHT_UNCHANGED;
    plscol0a 15 32 32 32 0.70;

    let nrow = min 3 !nlegend in
    let ncolumn = 0 in

    let legend_width, legend_height =
      let opt_array = Array.sub opt_array 0 !nlegend in
      pllegend !opt !position !x !y
          0.025 15 1 1 nrow ncolumn
          opt_array 1.0 1.0 1.5
          1.0 text_colors text
          [||] [||] [||] [||]
          line_colors line_styles line_widths
          symbol_colors symbol_scales symbol_numbers symbols
    in

    if i = nturn then (
      position := [PL_POSITION_TOP; PL_POSITION_RIGHT; PL_POSITION_SUBPAGE];
      opt := opt_base;
      x := 1.0 -. !x;
      y := !y +. legend_height;
    )
    else (
      x := !x +. legend_width;
      y := !y +. legend_height;
    );
  done;

  (* Fourth page illustrating various kinds of legends *)
  let max_height = 0.0 in
  let xstart = 0.0 in
  let ystart = 0.1 in
  let x = xstart in
  let y = ystart in
  let text_scale = 0.90 in
  pladv 0;
  plvpor 0.0 1.0 0.0 0.90;
  plwind 0.0 1.0 0.0 1.0;
  plsfont PL_FCI_SANS PL_FCI_STYLE_UNCHANGED PL_FCI_WEIGHT_UNCHANGED;
  plmtex "t" 2.0 0.5 0.5 "Demonstrate Various Kinds of Legends";

  let nlegend = 5 in
  (* Only specify legend data that are required according to the
     value of opt_array for that entry. *)
  let position = [PL_POSITION_LEFT; PL_POSITION_TOP] in
  let opt_base = [PL_LEGEND_BACKGROUND; PL_LEGEND_BOUNDING_BOX; PL_LEGEND_TEXT_LEFT] in

  (* Set up None, Box, Line, Symbol, and Line & Symbol legend entries. *)
  opt_array.(0) <- [PL_LEGEND_NONE];
  text.(0) <- sprintf "%s" "None";
  text_colors.(0) <- 1;

  opt_array.(1) <- [PL_LEGEND_COLOR_BOX];
  text.(1) <- sprintf "%s" "Box";
  text_colors.(1) <- 2;
  box_colors.(1) <- 2;
  box_patterns.(1) <- 0;
  box_scales.(1) <- 0.8;
  box_line_widths.(1) <- 1.0;

  opt_array.(2) <- [PL_LEGEND_LINE];
  text.(2) <- sprintf "%s" "Line";
  text_colors.(2) <- 3;
  line_colors.(2) <- 3;
  line_styles.(2) <- 1;
  line_widths.(2) <- 1.0;

  opt_array.(3) <- [PL_LEGEND_SYMBOL];
  text.(3) <- sprintf "%s" "Symbol";
  text_colors.(3) <- 4;
  symbol_colors.(3) <- 4;
  symbol_scales.(3) <- text_scale;
  symbol_numbers.(3) <- 4;
  symbols.(3) <- special_symbols.(2);

  opt_array.(4) <- [PL_LEGEND_SYMBOL; PL_LEGEND_LINE];
  text.(4) <- sprintf "%s" "L & S";
  text_colors.(4) <- 5;
  line_colors.(4) <- 5;
  line_styles.(4) <- 1;
  line_widths.(4) <- 1.0;
  symbol_colors.(4) <- 5;
  symbol_scales.(4) <- text_scale;
  symbol_numbers.(4) <- 4;
  symbols.(4) <- special_symbols.(2);

  let opt = opt_base in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        box_colors box_patterns box_scales box_line_widths
        line_colors line_styles line_widths
        symbol_colors symbol_scales symbol_numbers symbols
  in
  let max_height = max max_height legend_height in

  (* Set up symbol legend entries with various symbols. *)
  for i = 0 to nlegend - 1 do
    opt_array.(i) <- [PL_LEGEND_SYMBOL];
    text.(i) <- sprintf "%s%s" "Symbol " special_symbols.(i);
    text_colors.(i) <- i + 1;
    symbol_colors.(i) <- i + 1;
    symbol_scales.(i) <- text_scale;
    symbol_numbers.(i) <- 4;
    symbols.(i) <- special_symbols.(i);
  done;

  let opt = opt_base in
  let x = x +. legend_width in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        [||] [||] [||] [||]
        [||] [||] [||]
        symbol_colors symbol_scales symbol_numbers symbols
  in
  let max_height = max max_height legend_height in

  (* Set up symbol legend entries with various numbers of symbols. *)
  for i = 0 to nlegend - 1 do
    opt_array.(i) <- [PL_LEGEND_SYMBOL];
    text.(i) <- sprintf "%s %d" "Symbol Number" (i + 2);
    text_colors.(i) <- i + 1;
    symbol_colors.(i) <- i + 1;
    symbol_scales.(i) <- text_scale;
    symbol_numbers.(i) <- i + 2;
    symbols.(i) <- special_symbols.(2);
  done;

  let opt = opt_base in
  let x = x +. legend_width in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        [||] [||] [||] [||]
        [||] [||] [||]
        symbol_colors symbol_scales symbol_numbers symbols
  in
  let max_height = max max_height legend_height in

  (* Set up box legend entries with various colours. *)
  for i = 0 to nlegend - 1 do
    opt_array.(i) <- [PL_LEGEND_COLOR_BOX];
    text.(i) <- sprintf "%s %d" "Box Color" (i + 1);
    text_colors.(i) <- i + 1;
    box_colors.(i) <- i + 1;
    box_patterns.(i) <- 0;
    box_scales.(i) <- 0.8;
    box_line_widths.(i) <- 1.0;
  done;

  let opt = opt_base in
  (* Use new origin *)
  let x = xstart in
  let y = y +. max_height in
  let max_height = 0.0 in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        box_colors box_patterns box_scales box_line_widths
        [||] [||] [||]
        [||] [||] [||] [||]
  in
  let max_height = max max_height legend_height in

  (* Set up box legend entries with various patterns. *)
  for i = 0 to nlegend - 1 do
    opt_array.(i) <- [PL_LEGEND_COLOR_BOX];
    text.(i) <- sprintf "%s %d" "Box Pattern" i;
    text_colors.(i) <- 2;
    box_colors.(i) <- 2;
    box_patterns.(i) <- i;
    box_scales.(i) <- 0.8;
    box_line_widths.(i) <- 1.0;
  done;

  let opt = opt_base in
  let x = x +. legend_width in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        box_colors box_patterns box_scales box_line_widths
        [||] [||] [||]
        [||] [||] [||] [||]
  in
  let max_height = max max_height legend_height in

  (* Set up box legend entries with various box pattern line widths. *)
  for i = 0 to nlegend - 1 do
    opt_array.(i) <- [PL_LEGEND_COLOR_BOX];
    text.(i) <- sprintf "%s %d" "Box Line Width" (i + 1);
    text_colors.(i) <- 2;
    box_colors.(i) <- 2;
    box_patterns.(i) <- 3;
    box_scales.(i) <- 0.8;
    box_line_widths.(i) <- float i +. 1.0;
  done;

  let opt = opt_base in
  let x = x +. legend_width in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        box_colors box_patterns box_scales box_line_widths
        [||] [||] [||]
        [||] [||] [||] [||]
  in
  let max_height = max max_height legend_height in

  (* Set up line legend entries with various colours. *)
  for i = 0 to nlegend - 1 do
    opt_array.(i) <- [PL_LEGEND_LINE];
    text.(i) <- sprintf "%s %d" "Line Color" (i + 1);
    text_colors.(i) <- i + 1;
    line_colors.(i) <- i + 1;
    line_styles.(i) <- 1;
    line_widths.(i) <- 1.0;
  done;

  let opt = opt_base in
  (* Use new origin *)
  let x = xstart in
  let y = y +. max_height in
  let max_height = 0.0 in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        [||] [||] [||] [||]
  in
  let max_height = max max_height legend_height in

  (* Set up line legend entries with various styles. *)
  for i = 0 to nlegend - 1 do
    opt_array.(i) <- [PL_LEGEND_LINE];
    text.(i) <- sprintf "%s %d" "Line Style" (i + 1);
    text_colors.(i) <- 2;
    line_colors.(i) <- 2;
    line_styles.(i) <- i + 1;
    line_widths.(i) <- 1.0;
  done;

  let opt = opt_base in
  let x = x +. legend_width in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        [||] [||] [||] [||]
  in
  let max_height = max max_height legend_height in

  (* Set up line legend entries with various widths. *)
  for i = 0 to nlegend - 1 do
    opt_array.(i) <- [PL_LEGEND_LINE];
    text.(i) <- sprintf "%s %d" "Line Width" (i + 1);
    text_colors.(i) <- 2;
    line_colors.(i) <- 2;
    line_styles.(i) <- 1;
    line_widths.(i) <- float i +. 1.0;
  done;

  let opt = opt_base in
  let x = x +. legend_width in
  plscol0a 15 32 32 32 0.70;

  let legend_width, legend_height =
    let opt_array = Array.sub opt_array 0 nlegend in
    pllegend opt position x y
        0.1 15 1 1 0 0
        opt_array 1.0 text_scale 2.0
        0.0 text_colors text
        [||] [||] [||] [||]
        line_colors line_styles line_widths
        [||] [||] [||] [||]
  in
  let max_height = max max_height legend_height in
  (* Silence a warning, so the reset is here once the plcolorbar pages are
     added. *)
  ignore (max_height);

  (* Color bar examples *)

  let values_small = [| -1.0e-20; 1.0e-20 |] in
  let values_uneven = [| -1.0e-20; 2.0e-20; 2.6e-20; 3.4e-20; 6.0e-20; 7.0e-20; 8.0e-20; 9.0e-20; 10.0e-20 |] in
  let values_even = [|-2.0e-20; -1.0e-20; 0.0e-20; 1.0e-20; 2.0e-20; 3.0e-20; 4.0e-20; 5.0e-20; 6.0e-20 |] in

  (* Use unsaturated green background colour to contrast with black caps. *)
  plscolbg 70 185 70;

  (* Cut out the greatest and smallest bits of the color spectrum to
     leave colors for the end caps. *)
  plscmap1_range 0.01 0.99;

  (* We can only test image and gradient colorbars with two element arrays *)
  for i = 2 to colorbar_kinds - 1 do
    plcolorbar_example "cmap1_blue_yellow.pal" i 0 0. values_small;
  done;

  (* Test shade colorbars with larger arrays *)
  for i = 0 to 1 do
    plcolorbar_example "cmap1_blue_yellow.pal" i 4 2. values_even;
  done;

  for i = 0 to 1 do
    plcolorbar_example "cmap1_blue_yellow.pal" i 0 0. values_uneven;
  done;

  plend();
  ()

