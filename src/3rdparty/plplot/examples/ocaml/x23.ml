(*

    Displays Greek letters and mathematically interesting Unicode ranges
    Copyright (C) 2005,2008 Alan Irwin
    Copyright (C) 2005,2008 Andrew Ross
    Copyright (C) 2008 Hezekiah M. Carty


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

(*
 * Displays Greek letters and mathematically interesting Unicode ranges
 *)

let greek = [|
  "#gA";"#gB";"#gG";"#gD";"#gE";"#gZ";"#gY";"#gH";"#gI";"#gK";"#gL";"#gM";
  "#gN";"#gC";"#gO";"#gP";"#gR";"#gS";"#gT";"#gU";"#gF";"#gX";"#gQ";"#gW";
  "#ga";"#gb";"#gg";"#gd";"#ge";"#gz";"#gy";"#gh";"#gi";"#gk";"#gl";"#gm";
  "#gn";"#gc";"#go";"#gp";"#gr";"#gs";"#gt";"#gu";"#gf";"#gx";"#gq";"#gw";
|]

let type1 = [|
  0x0020;0x0021;0x0023;0x0025;0x0026;
  0x0028;0x0029;0x002b;0x002c;0x002e;
  0x002f;0x0030;0x0031;0x0032;0x0033;
  0x0034;0x0035;0x0036;0x0037;0x0038;
  0x0039;0x003a;0x003b;0x003c;0x003d;
  0x003e;0x003f;0x005b;0x005d;0x005f;
  0x007b;0x007c;0x007d;0x00a9;0x00ac;
  0x00ae;0x00b0;0x00b1;0x00d7;0x00f7;
  0x0192;0x0391;0x0392;0x0393;0x0394;
  0x0395;0x0396;0x0397;0x0398;0x0399;
  0x039a;0x039b;0x039c;0x039d;0x039e;
  0x039f;0x03a0;0x03a1;0x03a3;0x03a4;
  0x03a5;0x03a6;0x03a7;0x03a8;0x03a9;
  0x03b1;0x03b2;0x03b3;0x03b4;0x03b5;
  0x03b6;0x03b7;0x03b8;0x03b9;0x03ba;
  0x03bb;0x03bc;0x03bd;0x03be;0x03bf;
  0x03c0;0x03c1;0x03c2;0x03c3;0x03c4;
  0x03c5;0x03c6;0x03c7;0x03c8;0x03c9;
  0x03d1;0x03d2;0x03d5;0x03d6;0x2022;
  0x2026;0x2032;0x2033;0x203e;0x2044;
  0x2111;0x2118;0x211c;0x2122;0x2126;
  0x2135;0x2190;0x2191;0x2192;0x2193;
  0x2194;0x21b5;0x21d0;0x21d1;0x21d2;
  0x21d3;0x21d4;0x2200;0x2202;0x2203;
  0x2205;0x2206;0x2207;0x2208;0x2209;
  0x220b;0x220f;0x2211;0x2212;0x2215;
  0x2217;0x221a;0x221d;0x221e;0x2220;
  0x2227;0x2228;0x2229;0x222a;0x222b;
  0x2234;0x223c;0x2245;0x2248;0x2260;
  0x2261;0x2264;0x2265;0x2282;0x2283;
  0x2284;0x2286;0x2287;0x2295;0x2297;
  0x22a5;0x22c5;0x2320;0x2321;0x2329;
  0x232a;0x25ca;0x2660;0x2663;0x2665;
  0x2666;
|]

let title = [|
  "#<0x10>PLplot Example 23 - Greek Letters";
  "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (a)";
  "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (b)";
  "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (c)";
  "#<0x10>PLplot Example 23 - Number Forms Unicode Block";
  "#<0x10>PLplot Example 23 - Arrows Unicode Block (a)";
  "#<0x10>PLplot Example 23 - Arrows Unicode Block (b)";
  "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (a)";
  "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (b)";
  "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (c)";
  "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (d)";
|]

let lo = [|
  0x0;
  0x0;
  0x40;
  0x80;
  0x2153;
  0x2190;
  0x21d0;
  0x2200;
  0x2240;
  0x2280;
  0x22c0;
|]

let hi = [|
  0x30;
  0x40;
  0x80;
  0xA6;
  0x2184;
  0x21d0;
  0x2200;
  0x2240;
  0x2280;
  0x22c0;
  0x2300;
|]

let nxcells = [|
  12;
  8;
  8;
  8;
  8;
  8;
  8;
  8;
  8;
  8;
  8;
|]

let nycells = [|
  8;
  8;
  8;
  8;
  8;
  8;
  8;
  8;
  8;
  8;
  8;
|]

(* non-zero values Must be consistent with nxcells and nycells. *)
let offset = [|
  0;
  0;
  64;
  128;
  0;
  0;
  0;
  0;
  0;
  0;
  0;
|]

(* 30 possible FCI values. *)
let fci_combinations = 30

let fci = [|
  0x80000000L;
  0x80000001L;
  0x80000002L;
  0x80000003L;
  0x80000004L;
  0x80000010L;
  0x80000011L;
  0x80000012L;
  0x80000013L;
  0x80000014L;
  0x80000020L;
  0x80000021L;
  0x80000022L;
  0x80000023L;
  0x80000024L;
  0x80000100L;
  0x80000101L;
  0x80000102L;
  0x80000103L;
  0x80000104L;
  0x80000110L;
  0x80000111L;
  0x80000112L;
  0x80000113L;
  0x80000114L;
  0x80000120L;
  0x80000121L;
  0x80000122L;
  0x80000123L;
  0x80000124L;
|]

let family = [|
  "sans-serif";
  "serif";
  "monospace";
  "script";
  "symbol";
|]

let style = [|
  "upright";
  "italic";
  "oblique";
|]

let weight = [|
  "medium";
  "bold";
|]

let family_of_int = [|
  PL_FCI_SANS;
  PL_FCI_SERIF;
  PL_FCI_MONO;
  PL_FCI_SCRIPT;
  PL_FCI_SYMBOL;
|]

let style_of_int = [|
  PL_FCI_UPRIGHT;
  PL_FCI_ITALIC;
  PL_FCI_OBLIQUE;
|]

let weight_of_int = [|
  PL_FCI_MEDIUM;
  PL_FCI_BOLD;
|]

let () =
  plparseopts Sys.argv [PL_PARSE_FULL];

  plinit ();

  for page = 0 to 10 do
    pladv 0;

    (* Set up viewport and window *)
    plvpor 0.02 0.98 0.02 0.90;
    plwind 0.0 1.0 0.0 1.0;
    let xmin, xmax, ymin, ymax = plgspa () in
    plschr 0.0 0.8;
    let ycharacter_scale = 1.0 /. (ymax -. ymin) in

    (* Factor should be 0.5, but heuristically it turns out to be larger. *)
    let chardef, charht = plgchr () in
    let yoffset = 1.0 *. charht *. ycharacter_scale in

    (* Draw the grid using plbox *)
    plcol0 2;
    let deltax = 1.0 /. float_of_int nxcells.(page) in
    let deltay = 1.0 /. float_of_int nycells.(page) in
    plbox "bcg" deltax 0 "bcg" deltay 0;
    plcol0 15;
    let length = hi.(page) - lo.(page) in
    let slice = ref 0 in
    for j = nycells.(page) - 1 downto -1 do
      let y = (0.5 +. float_of_int j) *. deltay in
      for i = 0 to nxcells.(page) - 1 do
        let x = (0.5 +. float_of_int i) *. deltax in
        if !slice < length then (
          let cmdString =
            match page with
                0 -> sprintf "#%s" greek.(!slice)
              | 1
              | 2
              | 3 -> sprintf "##[0x%.4x]" type1.(offset.(page) + !slice)
              | _ -> sprintf "##[0x%.4x]" (lo.(page) + !slice);
          in
          let cmdString' =
            String.sub cmdString 1 (String.length cmdString - 1)
          in
          plptex x (y +. yoffset) 1.0 0.0 0.5 cmdString';
          plptex x (y -. yoffset) 1.0 0.0 0.5 cmdString;
        );
        incr slice;
      done
    done;

    plschr 0.0 1.0;

    (* Page title *)
    plmtex "t" 1.5 0.5 0.5 title.(page);
  done;

  (* Demonstrate methods of getting the current fonts *)
  let fci_old = plgfci () in
  let ifamily, istyle, iweight = plgfont () in
  printf "For example 23 prior to page 12 the FCI is 0x%lx\n"
    (Int64.to_int32 fci_old);
  printf "For example 23 prior to page 12 the font family, style and weight are %s %s %s\n"
    family.(ifamily) style.(istyle) weight.(iweight);

  for page = 11 to 15 do
    let dy = 0.030 in

    pladv 0;
    plvpor 0.02 0.98 0.02 0.90;
    plwind 0.0 1.0 0.0 1.0;
    plsfci 0L;
    let () =
      match page with
          11 ->
            plmtex "t" 1.5 0.5 0.5
                   "#<0x10>PLplot Example 23 - Set Font with plsfci";
        | 12 ->
            plmtex "t" 1.5 0.5 0.5
                   "#<0x10>PLplot Example 23 - Set Font with plsfont";
        | 13 ->
            plmtex "t" 1.5 0.5 0.5
                   "#<0x10>PLplot Example 23 - Set Font with ##<0x8nnnnnnn> construct";
        | 14 ->
            plmtex "t" 1.5 0.5 0.5
                   "#<0x10>PLplot Example 23 - Set Font with ##<0xmn> constructs";
        | 15 ->
            plmtex "t" 1.5 0.5 0.5
                   "#<0x10>PLplot Example 23 - Set Font with ##<FCI COMMAND STRING/> constructs";
        | _ -> failwith "Invalid page"
    in
    plschr 0.0 0.75;
    for i = 0 to fci_combinations - 1 do
      let family_index = i mod 5 in
      let style_index = (i / 5) mod 3 in
      let weight_index = ((i / 5) / 3) mod 2 in
      let text_string =
        match page with
            11 ->
              plsfci fci.(i);
              sprintf
                "Page 12, %s, %s, %s:  The quick brown fox jumps over the lazy dog"
                family.(family_index)
                style.(style_index)
                weight.(weight_index);
          | 12 ->
              plsfont family_of_int.(family_index) style_of_int.(style_index)
                weight_of_int.(weight_index);
              sprintf
                "Page 13, %s, %s, %s:  The quick brown fox jumps over the lazy dog"
                family.(family_index)
                style.(style_index)
                weight.(weight_index);
          | 13 ->
              sprintf
                "Page 14, %s, %s, %s:  #<0x%Lx>The quick brown fox jumps over the lazy dog"
                family.(family_index)
                style.(style_index)
                weight.(weight_index)
                fci.(i);
          | 14 ->
              sprintf
                "Page 15, %s, %s, %s:  #<0x%1x0>#<0x%1x1>#<0x%1x2>The quick brown fox jumps over the lazy dog"
                family.(family_index)
                style.(style_index)
                weight.(weight_index)
                family_index
                style_index
                weight_index;
          | 15 ->
              sprintf
                "Page 16, %s, %s, %s:  #<%s/>#<%s/>#<%s/>The quick brown fox jumps over the lazy dog"
                family.(family_index)
                style.(style_index)
                weight.(weight_index)
                family.(family_index)
                style.(style_index)
                weight.(weight_index);
        | _ -> failwith "Invalid page"
      in
      plptex 0.0 (1.0 -. (float_of_int i +. 0.5) *. dy) 1.0 0.0 0.0 text_string;
    done;
    plschr 0.0 1.0;
  done;

  (* Restore defaults *)
  plcol0 1;

  plend ();
  ()

