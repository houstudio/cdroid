(*
   Simple line plot and multiple windows demo.  A fairly straightforward
   translation from the original C to OCaml.

   Copyright (C) 2008  Hezekiah M. Carty

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

type ex1_params = {
  xscale : float;
  yscale : float;
  xoff : float;
  yoff : float;
}

let pi = atan 1.0 *. 4.0

(*
/* Variables and data arrays used by plot generators */

static PLFLT x[101], y[101];
static PLFLT xscale, yscale, xoff, yoff, xs[6], ys[6];
static PLGraphicsIn gin;

static int locate_mode;
static int test_xor;
static int fontset = 1;
static char *f_name = NULL;

/* Options data structure definition. */

static PLOptionTable options[] = {
{
    "locate",                   /* Turns on test of API locate function */
    NULL,
    NULL,
    &locate_mode,
    PL_OPT_BOOL,
    "-locate",
    "Turns on test of API locate function" },
{
    "xor",                      /* Turns on test of xor function */
    NULL,
    NULL,
    &test_xor,
    PL_OPT_BOOL,
    "-xor",
    "Turns on test of XOR" },
{
    "font",                     /* For switching between font set 1 & 2 */
    NULL,
    NULL,
    &fontset,
    PL_OPT_INT,
    "-font number",
    "Selects stroke font set (0 or 1, def:1)" },
{
    "save",                     /* For saving in postscript */
    NULL,
    NULL,
    &f_name,
    PL_OPT_STRING,
    "-save filename",
      "Save plot in color postscript `file'" },
{
    NULL,                       /* option */
    NULL,                       /* handler */
    NULL,                       /* client data */
    NULL,                       /* address of variable to set */
    0,                          /* mode flag */
    NULL,                       /* short syntax */
    NULL }                      /* long syntax */
};

const char *notes[] = {"Make sure you get it right!", NULL};

*)

let plot1 (do_test, test_xor) params =
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

  let xs = Array.init 6 (fun i -> x.(i * 10 + 3)) in
  let ys = Array.init 6 (fun i -> y.(i * 10 + 3)) in

  (* Set up the viewport and window using PLENV. The range in X is
     0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
     scaled separately (just = 0), and we just draw a labelled
     box (axis = 0). *)
  plcol0 1;
  plenv xmin xmax ymin ymax 0 0;
  plcol0 2;
  pllab "(x)" "(y)" "#frPLplot Example 1 - y=x#u2";

  (* Plot the data points *)
  plcol0 4;
  plpoin xs ys 9;

  (* Draw the line through the data *)
  plcol0 3;
  plline x y;

  (* COMMENT THIS SECTION OUT FOR NOW, AS IT DOES NOT FUNCTION EXACTLY
     AS THE C EXAMPLE.
  (* xor mode enable erasing a line/point/text by replotting it again *)
  (* it does not work in double buffering mode, however *)
  let () =
    if do_test && test_xor then
      let st = plxormod 1 in (* enter xor mode *)
      if st > 0 then begin
        for i = 1 to 10 do
          let i' = i * 6 - 6 in
          plpoin (Array.sub x i' 1) (Array.sub y i' 1) 9;   (* draw a point *)
          Unix.sleep 1;                                     (* wait a little *)
          plflush ();                               (* force an update of the driver *)
          plpoin (Array.sub x i' 1) (Array.sub y i' 1) 9;   (* erase point *)
        done;
        ignore (plxormod 0)                                 (* leave xor mode *)
      end
      else
        print_endline "xor mode not supported."
  in
  END BIG BLOCK COMMENT *)
  (* All done. *)
  ()

let plot2 () =
  (* Set up the viewport and window using PLENV. The range in X is -2.0 to
     10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
     (just = 0), and we draw a box with axes (axis = 1). *)
  plcol0 1;
  plenv (-2.0) 10.0 (-0.4) 1.2 0 1;
  plcol0 2;
  pllab "(x)" "sin(x)/x" "#frPLplot Example 1 - Sinc Function";

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

  (* Draw the line *)
  plcol0 3;
  plwidth 2.0;
  plline x y;
  plwidth 1.0;

  (* All done. *)
  ()

let plot3 () =
  let space1 = 1500 in
  let mark1 = 1500 in

  (* For the final graph we wish to override the default tick intervals, and
     so do not use plenv(). *)
  pladv 0;

  (* Use standard viewport, and define X range from 0 to 360 degrees, Y range
     from -1.2 to 1.2.*)
  plvsta ();
  plwind 0.0 360.0 (-1.2) 1.2;

  (* Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y. *)
  plcol0 1;
  plbox "bcnst" 60.0 2 "bcnstv" 0.2 2;

  (* Superimpose a dashed line grid, with 1.5 mm marks and spaces.
     plstyl expects a pointer!*)
  plstyl [|mark1|] [|space1|];
  plcol0 2;
  plbox "g" 30.0 0 "g" 0.2 0;
  plstyl [||] [||];

  plcol0 3;
  pllab "Angle (degrees)" "sine" "#frPLplot Example 1 - Sine function";

  let x = Array.init 101 (fun i -> 3.6 *. float_of_int i) in
  let y = Array.init 101 (fun i -> sin (x.(i) *. pi /. 180.0)) in

  plcol0 4;
  plline x y;

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
  (* plplot initialization *)

  (* NOT IMPLEMENTED YET
  (* Parse and process command line arguments *)
  plMergeOpts(options, "x01c options", notes);
  *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Get version number, just for kicks *)
  let plplot_version = plgver () in
  fprintf stdout "PLplot library version: %s\n" plplot_version;

  (* Initialize plplot *)
  (* Divide page into 2x2 plots *)
  plstar 2 2;

  (* Select font set as per input flag *)
  plfontld (if fontset then 1 else 0);

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
  (* Do a plot *)
  plot1 (false, false) plot1_params;

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
  plsyax digmax 0;
  plot1 (true, true) plot1_params;

  plot2 ();

  plot3 ();

  (* NOT IMPLEMENTED YET
  (*
   * Show how to save a plot:
   * Open a new device, make it current, copy parameters,
   * and replay the plot buffer
   *)
    if (f_name) { /* command line option '-save filename' */

      printf("The current plot was saved in color Postscript under the name `%s'.\n", f_name);
      plgstrm(&cur_strm);    /* get current stream */
      plmkstrm(&new_strm);   /* create a new one */

      plsfnam(f_name);       /* file name */
      plsdev("psc");         /* device type */

      plcpstrm(cur_strm, 0); /* copy old stream parameters to new stream */
      plreplot();            /* do the save by replaying the plot buffer */
      plend1();              /* finish the device */

      plsstrm(cur_strm);     /* return to previous stream */
    }

  /* Let's get some user input */

    if (locate_mode) {
        for (;;) {
          if (! plGetCursor(&gin)) break;
          if (gin.keysym == PLK_Escape) break;

            pltext();
            if (gin.keysym < 0xFF && isprint(gin.keysym))
                printf("subwin = %d, wx = %f,  wy = %f, dx = %f,  dy = %f,  c = '%c'\n",
                       gin.subwindow, gin.wX, gin.wY, gin.dX, gin.dY, gin.keysym);
            else
                printf("subwin = %d, wx = %f,  wy = %f, dx = %f,  dy = %f,  c = 0x%02x\n",
                       gin.subwindow, gin.wX, gin.wY, gin.dX, gin.dY, gin.keysym);

            plgra();
        }
    }
  *)

  (* Don't forget to call plend() to finish off! *)
  plend ();
  ()

let () = main true
