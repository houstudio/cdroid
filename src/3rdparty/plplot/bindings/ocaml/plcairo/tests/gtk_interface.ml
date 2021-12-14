(*
Copyright 2008, 2009  Hezekiah M. Carty

This file is part of PLplot.

PLplot is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

PLplot is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with PLplot.  If not, see <http://www.gnu.org/licenses/>.
*)

(* An example of using PLplot + Plcairo + Cairo + lablgtk to show a plot in a
   Gtk+ application.  This is meant to be run from the OCaml toplevel, after
   running "make" in the plcairo/ directory. *)

#use "topfind";;
#thread;;
#require "cairo.lablgtk2";;
#require "plplot";;
#directory "../_build/";;
#load "plcairo.cma";;

open Plplot

(* The plot surface size *)
let plot_width = 1600
let plot_height = 1000

(* The size of the GUI view of the plot *)
let gui_width = plot_width / 2
let gui_height = plot_height / 2

(** Get a Cairo context from the Gtk drawing area. *)
let get_cairo w =
  Cairo_lablgtk.create w#misc#window

(** Redraw the plot contents. *)
let redraw w plcairo _ =
  let cr = get_cairo w in
  let { Gtk.width = width ; Gtk.height = height } =
    w#misc#allocation in
  let width = float_of_int width in
  let height = float_of_int height in
  Plcairo.blit_to_cairo ~dest:cr ~dim:(`Both (width, height))
    ~xoff:0.0 ~yoff:0.0 plcairo;
  true

let () =
  (* Make a simple plot. *)
  let plcairo =
    Plcairo.init_cairo ~width:plot_width ~height:plot_height
      ~clear:true Plcairo.imagecairo
  in
  Plcairo.make_active plcairo;
  plenv (-180.0) 180.0 (-90.0) 90.0 1 0;
  plmap "globe" (-180.0) 180.0 (-90.0) 90.0;
  pllab "Latitude" "Longitude" "It's Earth!";
  plend ();

  (* DO NOT call Plcairo.finish yet.  The Cairo surface needs to remain active
     in order to use it in the GUI. *)

  (* Create a window for the app. *)
  let w = GWindow.window ~title:"PLplot + Gtk Integration Demo" () in
  (* Quit cleanly when the close button is clicked. *)
  ignore (w#connect#destroy GMain.quit);

  (* Put a box and frame around the plot. *)
  let b = GPack.vbox ~spacing:6 ~border_width:12 ~packing:w#add () in

  let f =
    GBin.frame ~shadow_type:`IN ~packing:(b#pack ~expand:true ~fill:true) ()
  in

  (* This drawing_area is where the plot will be displayed. *)
  let area =
    GMisc.drawing_area
      ~width:gui_width ~height:gui_height ~packing:f#add ()
  in

  (* Be sure to update the plot display when required. *)
  ignore (area#event#connect#expose (redraw area plcairo));

  (* Show the window and enter the main Gtk+ application loop. *)
  w#show ();
  GMain.main ()

