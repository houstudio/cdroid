(*
  Copyright (C) 2008-2016 Alan W. Irwin
  Copyright (C) 2008 Andrew Ross
  Copyright (C) 2008 Hezekiah M. Carty

  set/get tester

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

open Printf
open Plplot

let () =
  let r1 = [|0; 255|] in
  let g1 = [|255; 0|] in
  let b1 = [|0; 0|] in
  let a1 = [|1.0; 1.0|] in
  let status = ref 0 in

  let failed_if t s =
    if t then (
      eprintf s;
      status := 1;
    );
  in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Test setting / getting familying parameters before plinit *)
  (* Save values set by plparseopts to be restored later. *)
  let fam0, num0, bmax0 = plgfam () in
  let fam1 = 0 in
  let num1 = 10 in
  let bmax1 = 1000 in
  plsfam fam1 num1 bmax1;

  (* Retrieve the same values? *)
  let fam2, num2, bmax2 = plgfam () in
  printf "family parameters: fam, num, bmax = %d %d %d\n" fam2 num2 bmax2;
  failed_if
    (fam2 <> fam1 || num2 <> num1 || bmax2 <> bmax1)
    "plgfam test failed\n";
  (* Restore values set initially by plparseopts. *)
  plsfam fam0 num0 bmax0;

  (* Test setting / getting page parameters before plinit *)
  (* Save values set by plparseopts to be restored later. *)
  let xp0, yp0, xleng0, yleng0, xoff0, yoff0 = plgpage () in

  let xp1 = 200. in
  let yp1 = 200. in
  let xleng1 = 400 in
  let yleng1 = 200 in
  let xoff1 = 10 in
  let yoff1 = 20 in
  plspage xp1 yp1 xleng1 yleng1 xoff1 yoff1;

  (* Retrieve the same values? *)
  let xp2, yp2, xleng2, yleng2, xoff2, yoff2 = plgpage () in
  printf
    "page parameters: xp, yp, xleng, yleng, xoff, yoff = %f %f %d %d %d %d\n"
    xp2 yp2 xleng2 yleng2 xoff2 yoff2;
  failed_if
    (xp2 <> xp1 || yp2 <> yp1 || xleng2 <> xleng1 || yleng2 <> yleng1 ||
     xoff2 <> xoff1 || yoff2 <> yoff1)
    "plgpage test failed\n";
  (* Restore values set initially by plparseopts. *)
  plspage xp0 yp0 xleng0 yleng0 xoff0 yoff0;

  (* Test setting / getting compression parameter across plinit. *)
  let compression1 = 95 in
  plscompression compression1;

  (* Initialize plplot *)
  plinit ();

  (* Test if device initialization screwed around with the preset
     compression parameter. *)
  let compression2 = plgcompression () in
  printf "Output various PLplot parameters\n";
  printf "compression parameter = %d\n" compression2;
  failed_if
    (compression2 <> compression1)
    "plgcompression test failed\n";

  (* Exercise plscolor, plscol0, plscmap1, and plscmap1a to make sure
     they work without any obvious error messages. *)
  plscolor 1;
  plscol0 1 255 0 0;
  plscmap1 r1 g1 b1;
  plscmap1a r1 g1 b1 a1;

  let level2 = plglevel () in
  (* WARNING: The use of Obj.magic here is a cheat.  This really should be
     replaced by a proper int_of_plplot_run_level function.  This example is
     the only place it would be needed though, so it is not worth it at
     this time. *)
  printf "level parameter = %d\n" (Obj.magic level2);
  failed_if (level2 <> PL_INITIALIZED) "plglevel test failed.\n";

  pladv 0;

  let xmin0 = 0.01 in
  let xmax0 = 0.99 in
  let ymin0 = 0.02 in
  let ymax0 = 0.49 in
  plvpor xmin0 xmax0 ymin0 ymax0;
  let xmin, xmax, ymin, ymax = plgvpd () in
  printf
    "plvpor: xmin, xmax, ymin, ymax = %f %f %f %f\n"
    xmin xmax ymin ymax;
  failed_if
    (xmin <> xmin0 || xmax <> xmax0 || ymin <> ymin0 || ymax <> ymax0)
    "plgvpd test failed\n";

  let xmid = 0.5 *. (xmin +. xmax) in
  let ymid = 0.5 *. (ymin +. ymax) in

  let xmin0 = 0.2 in
  let xmax0 = 0.3 in
  let ymin0 = 0.4 in
  let ymax0 = 0.5 in
  plwind xmin0 xmax0 ymin0 ymax0;
  let xmin, xmax, ymin, ymax = plgvpw () in
  printf
    "plwind: xmin, xmax, ymin, ymax = %f %f %f %f\n"
    xmin xmax ymin ymax;
  failed_if
    (xmin <> xmin0 || xmax <> xmax0 || ymin <> ymin0 || ymax <> ymax0)
    "plgvpw test failed\n";

  (* Get world coordinates for middle of viewport *)
  let wx, wy, win = plcalc_world xmid ymid in
  printf "world parameters: wx, wy, win = %f %f %d\n" wx wy win;
  failed_if
    (
      abs_float (wx -. 0.5 *. (xmin +. xmax)) > 1.0E-5 ||
      abs_float (wy -. 0.5 *. (ymin +. ymax)) > 1.0E-5
    )
    "plcalc_world test failed\n";

  (* Retrieve and print the name of the output file (if any).
     This goes to stderr not stdout since it will vary between tests and
     we want stdout to be identical for compare test. *)
  let fnam = plgfnam () in
  if String.length fnam = 0 then
    printf "No output file name is set\n"
  else
    printf"Output file name read\n";
  eprintf "Output file name is %s\n" fnam;

  (* Set and get the number of digits used to display axis labels *)
  (* Note digits is currently ignored in pls[xyz]ax and
     therefore it does not make sense to test the returned
     value *)
  plsxax 3 0;
  let digmax, digits = plgxax () in
  printf "x axis parameters: digmax, digits = %d %d\n" digmax digits;
  failed_if (digmax <> 3) "plgxax test failed\n";

  plsyax 4 0;
  let digmax, digits = plgyax () in
  printf "y axis parameters: digmax, digits = %d %d\n" digmax digits;
  failed_if (digmax <> 4) "plgyax test failed\n";

  plszax 5 0;
  let digmax, digits = plgzax () in
  printf "z axis parameters: digmax, digits = %d %d\n" digmax digits;
  failed_if (digmax <> 5) "plgzax test failed\n";

  let mar0 = 0.05 in
  let aspect0 = (-42.0) in
  let jx0 = 0.1 in
  let jy0 = 0.2 in
  plsdidev mar0 aspect0 jx0 jy0;
  let mar, aspect, jx, jy = plgdidev () in
  printf
    "device-space window parameters: mar, aspect, jx, jy = %f %f %f %f\n"
    mar aspect jx jy;
  failed_if
    (mar <> mar0 || jx <> jx0 || jy <> jy0)
    "plgdidev test failed\n";

  let ori0 = 1.0 in
  plsdiori ori0;
  let ori = plgdiori () in
  printf "ori parameter = %f\n" ori;
  failed_if (ori <> ori0) "plgdiori test failed\n";

  let xmin0 = 0.1 in
  let ymin0 = 0.2 in
  let xmax0 = 0.9 in
  let ymax0 = 0.8 in
  plsdiplt xmin0 ymin0 xmax0 ymax0;
  let xmin, ymin, xmax, ymax = plgdiplt () in
  printf
    "plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f\n"
    xmin ymin xmax ymax;
  failed_if
    (xmin <> xmin0 || ymin <> ymin0 || xmax <> xmax0 || ymax <> ymax0)
    "plgdiplt test failed\n";

  let zxmin0 = 0.1 in
  let zymin0 = 0.1 in
  let zxmax0 = 0.9 in
  let zymax0 = 0.9 in
  plsdiplz zxmin0 zymin0 zxmax0 zymax0;
  let zxmin, zymin, zxmax, zymax = plgdiplt () in
  printf
    "zoomed plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f\n"
    zxmin zymin zxmax zymax;
  failed_if (
    abs_float (zxmin -. (xmin +. (xmax -. xmin) *. zxmin0)) > 1.0E-5 ||
    abs_float (zymin -. (ymin +. (ymax -. ymin) *. zymin0)) > 1.0E-5 ||
    abs_float (zxmax -. (xmin +. (xmax -. xmin) *. zxmax0)) > 1.0E-5 ||
    abs_float (zymax -. (ymin +. (ymax -. ymin) *. zymax0)) > 1.0E-5
  ) "plsdiplz test failed\n";

  let r0 = 10 in
  let g0 = 20 in
  let b0 = 30 in
  plscolbg r0 g0 b0;
  let r, g, b = plgcolbg () in
  printf "background colour parameters: r, g, b = %d %d %d\n" r g b;
  failed_if (r <> r0 || g <> g0 || b <> b0) "plgcolbg test failed\n";

  let r0 = 20 in
  let g0 = 30 in
  let b0 = 40 in
  let a0 = 0.5 in
  plscolbga r0 g0 b0 a0;
  let r, g, b, a = plgcolbga () in
  printf
    "background/transparency colour parameters: r, g, b, a = %d %d %d %f\n"
    r g b a;
  failed_if
    (r <> r0 || g <> g0 || b <> b0 || a <> a0)
    "plgcolbga test failed\n";

  plend ();

  exit !status
