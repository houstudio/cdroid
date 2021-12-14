(*
   Simple line plot and multiple windows demo, using the Plot module.

   Copyright (C) 2008, 2009  Hezekiah M. Carty

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

open Plplot
open Printf

module P = Plot

type ex1_params = {
  xscale : float;
  yscale : float;
  xoff : float;
  yoff : float;
}

let pi = atan 1.0 *. 4.0

(* This function will be called just before PLplot is initialized. *)
let pre () =
  P.load_palette (P.indexed_palette "cmap0_alternate.pal");
  plparseopts Sys.argv [PL_PARSE_FULL];
  ()

let plot1 ?stream ?fontset (do_test, test_xor) params =
  let x =
    Array.init 60 (
      fun i ->
        params.xoff +. params.xscale *. (float_of_int i +. 1.0) /. 60.0
    )
  in
  let y =
    Array.init 60 (
      fun i ->
        params.yoff +. params.yscale *. x.(i) ** 2.0
    )
  in

  let xmin = x.(0) in
  let xmax = x.(59) in
  let ymin = y.(0) in
  let ymax = y.(59) in

  (* If no stream was provided, initialize a new one. *)
  let stream =
    match stream, fontset with
    | None, Some fs -> (
          (* Initialize plplot *)
          (* Divide page into 2x2 plots *)
          (* plplot initialization *)
          let stream =
            P.init ~pre ~pages:(2, 2) (xmin, ymin) (xmax, ymax)
              `greedy `prompt
          in

          (* Select font set as per input flag *)
          P.with_stream ~stream (fun () -> plfontld (if fs then 1 else 0));
          stream
        )
    | Some s, None -> (
          (* Set up the viewport and window. The range in X is
             0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
             scaled separately, and we just draw a labelled box. *)
          P.set_color ~stream:s `black;
          P.start_page ~stream:s (xmin, ymin) (xmax, ymax) `greedy;
          s
        )
    | _ -> invalid_arg "Provide a stream or font option, not both"
  in

  let xs = Array.init 6 (fun i -> x.(i * 10 + 3)) in
  let ys = Array.init 6 (fun i -> y.(i * 10 + 3)) in

  P.set_color ~stream `blue;

  P.plot ~stream [
    (* Plot the data points *)
    P.points ~symbol:"⊙" `green xs ys;
    (* Draw the line through the data *)
    P.lines `red x y;
    (* Show the axes *)
    P.default_axes;
    (* Title and axis labels *)
    P.label "(x)" "(y)" "#frPLplot Example 1 - y=x#u2";
  ];

  (* All done. *)
  stream

let plot2 stream =
  (* Set up the viewport and window. The range in X is -2.0 to
     10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately,
     and we draw a box with axes. *)
  P.set_color ~stream `black;
  P.start_page ~stream (-2.0, -0.4) (10.0, 1.2) `greedy;

  P.set_color ~stream `blue;

  (* Fill up the arrays *)
  let x = Array.init 100 (fun i -> (float_of_int i -. 19.0) /. 6.0) in
  let y =
    Array.init 100 (
      fun i ->
        if x.(i) <> 0.0 then
          sin x.(i) /. x.(i)
        else
          1.0
    )
  in

  (* Show the axes *)
  let x_axis, y_axis =
    `axis :: P.default_axis_options,
    `axis :: P.default_axis_options
  in

  (* Draw the line *)
  P.plot ~stream [
    P.lines ~width:2.0 `red x y;
    P.axes x_axis y_axis;
    P.label "(x)" "sin(x)/x" "#frPLplot Example 1 - Sinc Function";
  ];

  (* All done. *)
  ()

let plot3 stream =
  let space1 = 1500 in
  let mark1 = 1500 in

  (* Use standard viewport, and define X range from 0 to 360 degrees, Y range
     from -1.2 to 1.2.*)
  P.start_page ~stream  (0.0, -1.2) (360.0, 1.2) `greedy;

  P.set_color ~stream `red;

  let x = Array.init 101 (fun i -> 3.6 *. float_of_int i) in
  let y = Array.init 101 (fun i -> sin (x.(i) *. pi /. 180.0)) in

  (* For the final graph we wish to override the default axis attributes,
     including tick intervals.
     Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y. *)
  let x_axis, y_axis =
    (* x-axis *)
    [`frame0; `frame1; `label; `minor_ticks; `major_tick_spacing 60.0],
    (* y-axis *)
    [
      `frame0; `frame1; `label; `minor_ticks; `major_tick_spacing 0.2;
      `vertical_label
    ]
  in

  P.plot ~stream [
    (* Superimpose a dashed line grid, with 1.5 mm marks and spaces. *)
    P.axes ~color:`yellow ~style:(`custom [mark1, space1])
      [`major_grid; `major_tick_spacing 30.0]
      [`major_grid; `major_tick_spacing 0.2];
    P.lines `brown x y;
    (* The normal plot axes *)
    P.axes x_axis y_axis;
    (* Plot title and axis labels *)
    P.label "Angle (degrees)" "sine"
      "#frPLplot Example 1 - Sine function";
  ];

  (* All done. *)
  ()

(*--------------------------------------------------------------------------*\
 * main
 *
 * Generates several simple line plots.  Demonstrates:
 *   - subwindow capability
 *   - setting up the window, drawing plot, and labelling
 *   - changing the color
 *   - automatic axis rescaling to exponential notation
 *   - placing the axes in the middle of the box
 *   - gridded coordinate axes
\*--------------------------------------------------------------------------*)
let main fontset =
  (* Get version number, just for kicks *)
  let plplot_version = plgver () in
  fprintf stdout "PLplot library version: %s\n" plplot_version;

  (* Set up the data *)
  (* Original case *)
  let plot1_params =
    {
      xscale = 6.0;
      yscale = 1.0;
      xoff = 0.0;
      yoff = 0.0;
    }
  in
  (* Do a plot, and receive a new stream *)
  let stream = plot1 ~fontset (false, false) plot1_params in

  (* Set up the data *)
  let plot1_params =
    {
      plot1_params with
        xscale = 1.0;
        yscale = 0.0014;
        yoff = 0.0185;
    }
  in
  (* Do a plot *)
  let digmax = 5 in
  P.with_stream ~stream (fun () -> plsyax digmax 0);
  let stream = plot1 ~stream (true, true) plot1_params in

  plot2 stream;

  plot3 stream;

  (* Don't forget to finish off!  All we need to do here is close up the
     plot stream. *)
  P.end_stream ~stream ();
  ()

let () = main true
