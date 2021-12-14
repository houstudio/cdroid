(*
Copyright 2008, 2009, 2010  Hezekiah M. Carty

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

(** Support for the extcairo PLplot device from OCaml. *)

open Plplot
open Printf

module Option = struct
  let may f o =
    match o with
    | Some x -> f x
    | None -> ()

  let default x_default o =
    match o with
    | Some x -> x
    | None -> x_default

  let map_default f x_default o =
    match o with
    | Some x -> f x
    | None -> x_default
end

(** A record to keep track of the Cairo surface and context information *)
type 'a t = {
  width : float; (** Width in device units *)
  height : float; (** Height in device units *)
  surface : Cairo.Surface.t; (** Cairo surface to plot to *)
  context : Cairo.context; (** Cairo context to plot to *)
  file : 'a option; (** An optional file name or stream for the plot output *)
  clear : bool; (** Should each new page be cleared? *)
  plstream : int; (** PLplot stream number associated with this plot *)
}

(** Types of Cairo surfaces available for {!init}. *)
type 'a plcairo_sfc_t =
  width:int -> height:int -> string option -> 'a option * Cairo.Surface.t

(** Provide PLplot with a Cairo context to plot on. *)
external plset_cairo_context : Cairo.context -> unit = "ml_set_plplot_cairo_context"

(** Get the various components of a {!t} instance *)
let plget_dims t = t.width, t.height
let plget_surface t = t.surface
let plget_context t = t.context
let plget_stream t = t.plstream
let plget_output t = t.file

(** [plblit_to_cairo ?xoff ?yoff ?scale_by plcairo dest] *)
let plblit_to_cairo ?(xoff = 0.0) ?(yoff = 0.0) ?scale_by t dest =
  let x, y =
    match scale_by with
    | None -> 1.0, 1.0
    | Some scale -> (
      match scale with
      | `width width -> let scale = width /. t.width in scale, scale
      | `height height -> let scale = height /. t.height in scale, scale
      | `both (width, height) -> width /. t.width, height /. t.height
    )
  in
  Cairo.save dest;
  begin
    Cairo.scale ~x ~y dest;
    Cairo.set_source_surface dest t.surface xoff yoff;
    Cairo.paint dest;
  end;
  Cairo.restore dest;
  ()

(** [plrasterize ?alpha t f] applies the plotting function [f ()] to [t] *)
let plrasterize ?alpha ?(antialias = Cairo.ANTIALIAS_NONE) t f =
  (* TODO FIXME: This function should only pain the actual (semi-)opaque pixels
     in the source image.  As it is, each call to this function will generate
     an image the size of the source canvas which will often not be the
     desired effect. *)
  (* Create a Cairo image surface and context to plot the rasterized image
     on.  This will be a duplicate in size and shape of the current plot's
     surface.  Leave the background transparent, so only the plotted image
     is transfered over to the main plot surface. *)
  let img_sfc =
    Cairo.Image.create
      Cairo.Image.ARGB32
      ~width:(int_of_float t.width)
      ~height:(int_of_float t.height)
  in
  let img_context = Cairo.create img_sfc in
  (* Assign the transformation matrix from the main plot context to maintain
     consistency.  It will also have to be applied to the main plot context
     again once back to it. *)
  let t_matrix = Cairo.get_matrix t.context in
  Cairo.set_matrix img_context t_matrix;
  plset_cairo_context img_context;
  (* Make sure antialiasing is turned OFF for the.  The output looks bad
     otherwise. *)
  Cairo.set_antialias img_context antialias;
  (* This will now be plotted on to the Cairo image surface. *)
  f ();
  (* Blit the raster image on to the main plot surface *)
  Cairo.set_source_surface t.context img_sfc 0.0 0.0;
  Cairo.paint ?alpha t.context;
  (* Now set PLplot back to using the proper plot context. *)
  plset_cairo_context t.context;
  Cairo.set_matrix t.context t_matrix;
  (* Don't forget to do a [Cairo.surface_finish] when everything is done!
     That isn't done here because the plot may not be finished yet. *)
  ()

(*
(** FIXME TODO XXX : I don't think this works properly yet.
    [plxy_to_cairoxy ~x ~y context] will convert the plot world
    coordinates [x, y] to Cairo device coordinates. *)
let plxy_to_cairoxy context ~x ~y =
  (* Normalized device coordinates *)
  let nxmin, nxmax, nymin, nymax = plgvpd () in
  (* World (plot-space) coordinates *)
  let wxmin, wxmax, wymin, wymax = plgvpw () in
  (* Cairo device coordinates *)
  let xmin = context.width *. nxmin in
  let xmax = context.width *. nxmax in
  let ymin = context.height *. nymin in
  let ymax = context.height *. nymax in
  (* World coordinates -> Cairo coordinates *)
  xmin +. ((xmax -. xmin) *. (x /. (wxmax -. wxmin))),
  ymin +. ((ymax -. ymin) *. (y /. (wymax -. wymin)))
*)

(** [with_stream ?stream f] calls [f ()] with [stream] as the active
    plotting stream if [stream] is present.  Otherwise it just calls
    [f ()]. *)
let with_stream ?stream f =
  match stream with
  | None -> f ()
  | Some s ->
      let old_stream = plgstrm () in
      plsstrm s.plstream;
      let result = f () in
      plsstrm old_stream;
      result

(** [plcairo_new_page t] will advance the Cairo context associated with [t] to
    a new page, for devices which support this. *)
let plcairo_new_page t =
  Cairo.show_page t.context;
  (* TODO: Is the call to plclear () the right thing to do here? *)
  if t.clear then with_stream ~stream:t (fun () -> pladv 0; plclear ()) else ()

(** The following 4 functions provide a relatively simple way to setup an
    appropriate Cairo surface for use with this library and the extcairo
    driver.  They should be passed as the [init] argument to the
    [plinit_cairo] function.*)
let plpscairo ~width ~height filename =
  let outfile =
    match filename with
    | Some f -> open_out f
    | None -> raise (Invalid_argument "plpscairo needs a filename")
  in
  Some outfile,
  Cairo.PS.create_for_stream
    ~output:(fun bytes -> output_string outfile bytes)
    ~width:(float_of_int width)
    ~height:(float_of_int height)

let plpdfcairo ~width ~height filename =
  let outfile =
    match filename with
    | Some f -> open_out f
    | None -> raise (Invalid_argument "plpdfcairo needs a filename")
  in
  Some outfile,
  Cairo.PDF.create_for_stream
    ~output:(fun bytes -> output_string outfile bytes)
    ~width:(float_of_int width)
    ~height:(float_of_int height)

let plimagecairo ~width ~height (filename : string option) =
  filename,
  Cairo.Image.create Cairo.Image.RGB24 ~width ~height

let plimagecairo_rgba ~width ~height (filename : string option) =
  filename,
  Cairo.Image.create Cairo.Image.ARGB32 ~width ~height

(** [plinit_cairo ?filename ?clear ?pre (width, height) init] creates a Cairo
    context and associates it with a new PLplot stream. *)
let plinit_cairo ?filename ?(clear = false) ?pre
                 (width, height) (init : 'a plcairo_sfc_t) =
  let file, sfc = init ~width ~height filename in
  let context = Cairo.create sfc in
  (* Associate the Cairo context with PLplot, then initialize PLplot *)
  let old_stream = plgstrm () in
  let new_stream = plmkstrm () in
  plsdev "extcairo";
  plsetopt "geometry" (sprintf "%dx%d" width height);
  Option.may (fun f -> f ()) pre;
  plinit ();
  (* Associate our new Cairo context with the PLplot stream. *)
  plset_cairo_context context;
  (* Clear the plot if requested and restore the previously active plot
     stream. *)
  if clear then (pladv 0; plclear ()) else ();
  plsstrm old_stream;
  (* Return the Cairo information so that the plot surface can be manipulated
     and/or saved late. *)
  {
    width = float_of_int width;
    height = float_of_int height;
    surface = sfc;
    context = context;
    file = file;
    clear = clear;
    plstream = new_stream;
  }

(** [plcairo_make_active t] sets PLplot to using the plot stream associated
    with [t] the active plot stream. *)
let plcairo_make_active t = plsstrm t.plstream

(** [plcairo_finish t] calls [Cairo.surface_finish] on the Cairo surface
    associated with [t]. *)
let plcairo_finish t =
  Cairo.Surface.finish t.surface;
  ()

(** [plsave_cairo_image ?filename t] saves the plot surface in [t]
    as a png to its associated file.  If [filename] is given then the file is
    saved with that name instead.  {!Plplot.plend} should be called first or
    the plotting may not be complete!
    Raises [Invalid_argument "No filename associated with this plot" if no
    filename is provided and [t] does not have a filename associated with it. *)
let plsave_cairo_image ?filename t =
  let output_filename =
    match filename with
    | Some f -> f
    | None -> (
      match t.file with
      | Some f -> f
      | None -> invalid_arg "No filename associated with this plot"
    )
  in
  Cairo.PNG.write t.surface output_filename;
  ()

(** [plsave_cairo ?filename t] is like {!plsave_cairo_image} but for
    non-image surfaces. *)
let plsave_cairo t =
  match t.file with
  | Some fout ->
      plcairo_finish t;
      close_out fout;
      ()
  | None -> invalid_arg "No filename associated with this plot"

(** NOTE TODO FIXME : This function will almost definitely change in a future
    revision.
    [plcairo_copy_plot t driver filename] copies the plot stream from
    [t] to a new output stream, using the plot driver [driver], saving the
    output to [filename]. *)
let plcairo_copy_plot t driver filename =
  (* Get the current stream so we can restore it when done. *)
  let old_stream = plgstrm () in
  (* Make a new plot stream *)
  ignore (plmkstrm ());
  plsdev driver;
  plsfnam filename;
  let geometry = sprintf "%.0fx%.0f" t.width t.height in
  plsetopt "geometry" geometry;
  plinit ();
  plcpstrm t.plstream true;
  plreplot ();
  plend1 ();
  plsstrm old_stream;
  ()

