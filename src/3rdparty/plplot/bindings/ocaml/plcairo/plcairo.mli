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

(** A record to keep track of the Cairo surface and context information *)
type 'a t

(** Types of Cairo surfaces available for {!plinit_cairo}. *)
type 'a plcairo_sfc_t

(** Provide PLplot with a Cairo context to plot on. *)
external plset_cairo_context : Cairo.context -> unit
  = "ml_set_plplot_cairo_context"

(** Get the [(width, height)] of the surface in device units. *)
val plget_dims : 'a t -> float * float

(** Get the Cairo surface associated with [t]. *)
val plget_surface : 'a t -> Cairo.Surface.t

(** Get the Cairo context associated with [t]. *)
val plget_context : 'a t -> Cairo.context

(** Get the PLplot stream number associated with [t]. *)
val plget_stream : 'a t -> int

(** Get the output (filename, output stream or nothing) associated with [t]. *)
val plget_output : 'a t -> 'a option

(** [plblit_to_cairo ?xoff ?yoff ?scale_by plcairo dest]
    blits the contents of [plcairo] to [dest].
    Optionally, offset the blit by [xoff] and [yoff] and
    scale by [`width w], [`height h] or [`both (w, h)]. *)
val plblit_to_cairo :
  ?xoff:float ->
  ?yoff:float ->
  ?scale_by:[< `both of float * float | `height of float | `width of float ] ->
  'a t -> Cairo.context -> unit

(** [plrasterize ?alpha t f] applies the plotting function [f ()] to [t],
    with the difference that the output will be rasterized for all plot
    output drivers, including vector-based output drivers such as PS, PDF and
    SVG.
    The [alpha] parameter may be provided to make the rasterized overlay
    transparent, even if the current color palette is not.
    Note that the plotting done by [f ()] will not be antialiased by default. *)
val plrasterize :
  ?alpha:float ->
  ?antialias:Cairo.antialias -> 'a t -> (unit -> 'b) -> unit

(** [plcairo_new_page t] will advance the Cairo context associated with [t] to
    a new page, for devices which support this. *)
val plcairo_new_page : 'a t -> unit

(** The following 4 functions provide a relatively simple way to setup an
    appropriate Cairo surface for use with this library and the extcairo
    driver.  They should be passed as the [init] argument to the
    [plinit_cairo] function.*)
val plpscairo : out_channel plcairo_sfc_t
val plpdfcairo : out_channel plcairo_sfc_t
val plimagecairo : string plcairo_sfc_t
val plimagecairo_rgba : string plcairo_sfc_t

(** [plinit_cairo ?filename ?clear ?pre (width, height) init] creates a Cairo
    context and associates it with a new PLplot stream. *)
val plinit_cairo :
  ?filename:string ->
  ?clear:bool ->
  ?pre:(unit -> unit) ->
  int * int -> 'a plcairo_sfc_t -> 'a t

(** [plcairo_make_active t] sets PLplot to using the plot stream associated
    with [t] the active plot stream. *)
val plcairo_make_active : 'a t -> unit

(** [plcairo_finish t] calls [Cairo.surface_finish] on the Cairo surface
    associated with [t]. *)
val plcairo_finish : 'a t -> unit

(** [plsave_cairo_image ?filename t] saves the plot surface in [t]
    as a png to its associated file.  If [filename] is given then the file is
    saved with that name instead.  One of [Plplot.plflush], [Plplot.plend] or
    [Plplot.plend1] should be called first or the plotting may not be complete!
    Raises [Invalid_argument "No filename associated with this plot"] if no
    filename is provided and [t] does not have a filename associated with it. *)
val plsave_cairo_image : ?filename:string -> string t -> unit

(** [plsave_cairo ?filename t] is like {!plsave_cairo_image} but for
    non-image surfaces. *)
val plsave_cairo : out_channel t -> unit

(** NOTE: This function will almost definitely change in a future revision.
    [plcairo_copy_plot t driver filename] copies the plot stream from
    [t] to a new output stream, using the plot driver [driver], saving the
    output to [filename]. *)
val plcairo_copy_plot : 'a t -> string -> string -> unit
