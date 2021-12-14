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

(* An example of using PLplot + Plcairo + Cairo + lablgtk2 to show a plot in a
   Gtk+ application. *)

open Plplot
open Plcairo

(* The plot surface size, much larger than the actual window.  The plot will be
   scaled to fit in the window. *)
let plot_width = 1600
let plot_height = 1000

(* The size of the GUI view of the plot *)
let gui_width = plot_width / 2
let gui_height = plot_height / 2

(** Get a Cairo context from the Gtk drawing area. *)
let get_cairo w =
  Cairo_gtk.create w#misc#window

(** Callback to redraw the plot contents.  If the window is resized then the
    plot will be stretched to fit the new window dimensions. *)
let redraw widget plcairo _ =
  let cr = get_cairo widget in
  let { Gtk.width = width ; Gtk.height = height } = widget#misc#allocation in
  let width = float_of_int width in
  let height = float_of_int height in
  plblit_to_cairo ~scale_by:(`both (width, height)) plcairo cr;
  true

let () =
  ignore (GtkMain.Main.init ());
  (* Make a simple plot. *)
  let plcairo =
    plinit_cairo ~clear:true (plot_width, plot_height) plimagecairo
  in
  plcairo_make_active plcairo;
  plenv (-180.0) 180.0 (-90.0) 90.0 1 0;
  plmap "globe" (-180.0) 180.0 (-90.0) 90.0;
  pllab "Latitude" "Longitude" "It's Earth!";

  (* It's ok to call plend here because we have the plot already. *)
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

  (* Be sure to update the plot display when an expose event occurs. *)
  ignore (area#event#connect#expose (redraw area plcairo));

  (* Show the window and enter the main Gtk+ application loop. *)
  w#show ();
  GMain.main ()

