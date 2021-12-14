(*
    Unicode Pace Flag

    Copyright (C) 2005 Rafael Laboissiere
    Copyright 2008 Hezekiah M. Carty


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


   In Debian, run like this:

   ( TTFDIR=/usr/share/fonts/truetype ; \
     PLPLOT_FREETYPE_SANS_FONT=$TTFDIR/arphic/bkai00mp.ttf \
     PLPLOT_FREETYPE_SERIF_FONT=$TTFDIR/freefont/FreeSerif.ttf \
     PLPLOT_FREETYPE_MONO_FONT=$TTFDIR/ttf-devanagari-fonts/lohit_hi.ttf \
     PLPLOT_FREETYPE_SCRIPT_FONT=$TTFDIR/unfonts/UnBatang.ttf \
     PLPLOT_FREETYPE_SYMBOL_FONT=$TTFDIR/ttf-bengali-fonts/JamrulNormal.ttf \
     ./x24ocaml -dev png -o x24ocaml.png )

   Packages needed:

   ttf-arphic-bkai00mp
   ttf-freefont
   ttf-devanagari-fonts
   ttf-unfonts
   ttf-bengali-fonts

   For the latest Ubuntu systems lohit_hi.ttf has been moved to the
   ttf-indic-fonts-core package instead of ttf-devanagari-fonts so you
   will have to use this package instead and update the font path.

 *)

open Plplot

let red   = [|240; 204; 204; 204;   0;  39; 125|]
let green = [|240;   0; 125; 204; 204;  80;   0|]
let blue  = [|240;   0;   0;   0;   0; 204; 125|]

let px = [|0.0; 0.0; 1.0; 1.0|]
let py = [|0.0; 0.25; 0.25; 0.0|]

let sx = [|
  0.16374;
  0.15844;
  0.15255;
  0.17332;
  0.50436;
  0.51721;
  0.49520;
  0.48713;
  0.83976;
  0.81688;
  0.82231;
  0.82647
|]

let sy = [|
  0.125;
  0.375;
  0.625;
  0.875;
  0.125;
  0.375;
  0.625;
  0.875;
  0.125;
  0.375;
  0.625;
  0.875
|]


(* Taken from http://www.columbia.edu/~fdc/pace/ *)

let peace = [|
  (* Mandarin *)
  "#<0x00>和平";
  (* Hindi *)
  "#<0x20>शांति";
  (* English *)
  "#<0x10>Peace";
  (* Hebrew *)
  "#<0x10>שלום";
  (* Russian *)
  "#<0x10>Мир";
  (* German *)
  "#<0x10>Friede";
  (* Korean *)
  "#<0x30>평화";
  (* French *)
  "#<0x10>Paix";
  (* Spanish *)
  "#<0x10>Paz";
  (* Arabic *)
  "#<0x10>ﺳﻼم";
  (* Turkish*)
  "#<0x10>Barış";
  (* Kurdish *)
  "#<0x10>Hasîtî";
|]

let () =
  plparseopts Sys.argv [PL_PARSE_FULL];

  plinit ();

  pladv 0;
  plvpor 0.0 1.0 0.0 1.0;
  plwind 0.0 1.0 0.0 1.0;
  plcol0 0;
  plbox "" 1.0 0 "" 1.0 0;

  plscmap0n 7;
  plscmap0 red green blue;

  plschr 0.0 4.0;
  plfont 1;

  for i = 0 to 3 do
    plcol0 (i + 1);
    plfill px py;
    for j = 0 to 3 do
      py.(j) <- py.(j) +. 1.0 /. 4.0;
    done
  done;

  plcol0 0;
  for i = 0 to 11 do
    plptex sx.(i) sy.(i) 1.0 0.0 0.5 peace.(i);
  done;


  plend ();
  ()

