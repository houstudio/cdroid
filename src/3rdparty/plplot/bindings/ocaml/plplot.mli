(*
Copyright 2009, 2010, 2011 Hezekiah M. Carty
Copyright 2017-2018 Alan W. Irwin

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

(** {1 PLplot - A library for creating scientific plots} *)

(** {e Note:} The API for the {!Plot} and {!Quick_plot} modules is not yet
    fixed and may change between now and the 5.10.0 PLplot release.

    {!core} is {b not} expected to change. *)

(** {3 A higher-level OCaml interface to PLplot} *)
module Plot :
  sig

    (** {b THIS API IS NOT FIXED AND MAY CHANGE BEFORE THE 5.10.0 RELEASE} *)

    type side_t = [`top | `bottom | `left | `right]
    type 'a tagged_side_t =
      [`top of 'a | `bottom of 'a | `left of 'a | `right of 'a]
    (** The different sides of a (2D) plot.  These values are used to
        represent, e.g., offsets along axes and text placement. *)

    (** These are the available options for drawing plot axes.  These values
        map to individual letters provided to {!Plplot.plbox} and similar
        functions. *)
    type axis_options_t =
      [ `axis
      | `frame0
      | `frame1
      | `time
      | `fixed_point
      | `major_grid
      | `minor_grid
      | `invert_ticks
      | `log
      | `unconventional_label
      | `label
      | `custom_label
      | `minor_ticks
      | `minor_tick_count of int
      | `major_ticks
      | `major_tick_spacing of float
      | `vertical_label ]

    (** A plot stream. *)
    type stream_t

    (** Colors from the indexed color map (color map 0), from the
        [cmapo_alternate.pal] file!  Note that these will NOT match up with
        colors from the default color palette! *)
    type color_t =
      [ `white
      | `red
      | `yellow
      | `green
      | `gray
      | `blue
      | `light_blue
      | `purple
      | `pink
      | `black
      | `brown
      | `index of int ]

    (** Different map outlines available for {!map} *)
    type map_t =
      [ `globe
      | `usa
      | `countries
      | `all ]

    (** Type signature for coordinate transform functions *)
    type pltr_t = float -> float -> float * float

    (** A plottable element, usable by the function {!plot}. *)
    type plot_t

    (** The different available plot device families.  This is not a complete
        representation of all of the available devices provided by PLplot. *)
    type plot_device_family_t =
      [ `cairo
      | `qt
      | `core
      | `wx ]

    (** Plot devices.  The constructor argument allows one to define which
        device family is used. *)
    type plot_device_t =
      [ `pdf of plot_device_family_t
      | `png of plot_device_family_t
      | `ps of plot_device_family_t
      | `svg of plot_device_family_t
      | `window of plot_device_family_t
      | `prompt (** Ask for the correct device to use at run-time *)
      | `stream of int (** Pre-defined plot streams *)
      | `other_device of string ] (** PLplot plotting devices otherwise not
                                     available from this list *)

    (** Type of scaling to use for plot axes, as in {!Plplot.plenv}. *)
    type plot_scaling_t = [ `preset | `greedy | `equal | `equal_square ]

    (** A color palette, usually loaded from disk *)
    type color_palette_t

    (** Line plotting styles/patterns. *)
    type line_style_t =
      [ `solid
      | `line1 | `line2 | `line3 | `line4
      | `line5 | `line6 | `line7 | `line8 (** These correspond to PLplot's
                                              pre-defined line styles as set by
                                              {!pllsty}. *)
      | `custom of ((int * int) list) ] (** A custom line style, with each
                                            (mark, space) pair defining the
                                            length of one segment and gap
                                            in the line drawing pattern. *)

    (** The default list of axis rendering options, used for all plots generated
        with {!init} if no custom options are provided. *)
    val default_axis_options : axis_options_t list

    (** Convert {!color_t} values to [r, g, b] integer values, each ranging in
        value from [0 - 255].*)
    val rgb_of_color : color_t -> int * int * int

    (** A recommended/standard file name extension, given a particular plot
        device. *)
    val recommended_extension : plot_device_t -> string

    (** {4 Plot streams}*)

    (** Make a new {!stream_t}, possibly using an existing plot stream index.
        If no [stream] argument is provided then a new plot stream is
        created. *)
    val make_stream : ?stream:int -> unit -> stream_t

    (** [start_page ?stream (x0, y0) (x1, y1) scaling] starts a new plot page
        with the given dimensions and scaling. *)
    val start_page :
      ?stream:stream_t ->
      float * float -> float * float -> plot_scaling_t -> unit

    (** An alias for {!start_page}. *)
    val next_page :
      ?stream:stream_t ->
      float * float -> float * float -> plot_scaling_t -> unit

    (** Create a new plot instance.  See {!init} for a description of the
        parameters. *)
    val make :
      ?stream:stream_t ->
      ?filename:string ->
      ?size:int * int ->
      ?pre:(unit -> unit) ->
      plot_device_t -> stream_t

    (** [init ?filename ?size ?pages (x0, y0) (x1, y1) scale device] - Start a
        new plot stream for a 2D plot with plot axis extents given by [(x0, y0)]
        to [(x1, y1)].  A call to [init] is roughly equivalent to calling
        {!make} and then {!start_page}.
        @param filename Plot output filename.  A suitable file extension
                        will be added if it does not already exist.
        @param size Dimension of the plot in physical units (e.g., pixels
                    for bitmap outputs (PNG, X) and 1/72 inch for vector
                    outputs (PS, PDF, SVG)
        @param pages Layout of multiple plots on a single page
        @param pre This function, if provided, is called just before the plot
                   instance is initialized.  It can be used to, for example,
                   load custom color palettes. *)
    val init :
      ?filename:string ->
      ?size:int * int ->
      ?pages:int * int ->
      ?pre:(unit -> unit) ->
      float * float -> float * float ->
      plot_scaling_t -> plot_device_t -> stream_t

    (** [with_stream ?stream f] performs [f] with [stream] as the active plot
        stream.  If [stream] is not provided then the currently active plot
        stream is left active.  Once [f] is complete, the previously active plot
        stream is restored. *)
    val with_stream : ?stream:stream_t -> (unit -> 'a) -> 'a

    (** [make_stream_active stream] makes [stream] in to the currently active
        plot stream for {!core} calls or any function which is not
        provided with an explicit [stream] argument. *)
    val make_stream_active : stream:stream_t -> unit

    (** {4 Colors} *)

    (** [set_color ?stream color] sets the current active plot color to
        [color]. *)
    val set_color : ?stream:stream_t -> color_t -> unit

    (** [set_color_scale ?stream ?pos ?alt_hue_path colors] sets the continuous color map
        (color map 1) to a scale determined by interpolating between [colors].
        [pos] can be used to specify where in the color scale ([0.0 - 1.0]) the
        a color interpolation point should fall.  [pos] defaults to even
        spacing.
        [alt_hue_path] can be used to specify that a given segment should be
        interpolated using the alternative hue path which always includes the
        hue=0 point.
        {!Plplot.plscmap1l} is used internally to set the color scale. *)
    val set_color_scale :
      ?stream:stream_t ->
      ?pos:float array ->
      ?alt_hue_path:bool array ->
      color_t array -> unit

    (** PLplot has two color palettes - indexed (color map 0) and
        continuous (color map 1).  These functions can be used with
        {!load_palette} to set custom indexed and continuous color palettes. *)
    val indexed_palette : string -> color_palette_t

    (** [continuous_palette ?interpolate filename] - If [interpolate] is true
        (the default) then a smooth palette is generated using the color
        definitions in the palette file [filename].  Otherwise, the colors
        are considered to be discrete, giving a segmented palette. *)
    val continuous_palette : ?interpolate:bool -> string -> color_palette_t

    (** [load_palette ?stream palette] loads either indexed or continuous
        color palette information from a file on disk. *)
    val load_palette : ?stream:stream_t -> color_palette_t -> unit

    (** {4 Plot elements} *)

    (** [arc ?fill color x y a b angle0 angle1 rotate] *)
    val arc :
      ?fill:bool ->
      color_t -> float -> float -> float -> float -> float -> float -> float ->
      plot_t

    (** [axes ?color ?style ?width xopt yopt] *)
    val axes :
      ?color:color_t ->
      ?style:line_style_t ->
      ?width:float ->
      ?labelfunc:([`x | `y | `z] -> float -> string) ->
      axis_options_t list -> axis_options_t list -> plot_t

    (** [default_axes] is equivalent to
        [axes default_axis_options default_axis_options] *)
    val default_axes : plot_t

    (** [circle ?fill color x y r] *)
    val circle : ?fill:bool -> color_t -> float -> float -> float -> plot_t

    (** [contours color tranform_func contours data] *)
    val contours :
      color_t -> pltr_t -> float array -> float array array -> plot_t

    (** [image ?range (x0, y0) (x1, y1) data] *)
    val image :
      ?range:float * float ->
      float * float -> float * float -> float array array -> plot_t

    (** [imagefr ?range ~scale (x0, y0) (x1, y1) data] *)
    val imagefr :
      ?range:float * float ->
      scale:float * float ->
      float * float -> float * float -> float array array -> plot_t

    (** [join ?style color (x0, y0) (x1, y1)] *)
    val join :
      ?style:line_style_t ->
      ?width:float ->
      color_t -> float * float -> float * float -> plot_t

    (** [label ?color x_label y_label title] adds axis labels and a title. *)
    val label : ?color:color_t -> string -> string -> string -> plot_t

    (** [lines ?label ?style color xs ys] *)
    val lines :
      ?label:string ->
      ?style:line_style_t ->
      ?width:float ->
      color_t -> float array -> float array -> plot_t

    (** [map ?sw ?ne color outline_type] *)
    val map :
      ?sw:float * float -> ?ne:float * float -> color_t -> map_t -> plot_t

    (** [points ?label ?symbol ?scale color xs ys] *)
    val points :
      ?label:string ->
      ?symbol:string ->
      ?scale:float ->
      color_t -> float array -> float array -> plot_t

    (** [polygon ?fill color xs ys] *)
    val polygon :
      ?fill:bool -> color_t -> float array -> float array -> plot_t

    (** [rectangle ?fill color (x0, y0) (x1, y1)] *)
    val rectangle :
      ?fill:bool -> color_t -> float * float -> float * float -> plot_t

    (** [shades ?fill_width ?contour ?rect (x0, y0) (x1, y1) contours data] *)
    val shades :
      ?fill_width:float ->
      ?contour:color_t * float ->
      ?rect:bool ->
      float * float -> float * float ->
      float array -> float array array -> plot_t

    (** [text ?dx ?dy ?just color x y string ] writes the text [string] inside
        the plot window, at an optional angle defined by the offsets [dx] and
        [dy]. *)
    val text :
      ?dx:float ->
      ?dy:float ->
      ?just:float ->
      color_t -> float -> float -> string -> plot_t

    (** [text_outside ?just ?perp color side offset string] writes text
        outside of the plot window, along [side], displaced from the axis by
        [offset] * character height. *)
    val text_outside :
      ?just:float ->
      ?perp:bool ->
      color_t -> float tagged_side_t -> float -> string -> plot_t

    (** [func ?symbol ?step color f (min, max)] plots the function [f] from
        [x = min] to [x = max].  [step] can be used to tighten or coarsen the
        sampling of plot points. *)
    val func :
      ?symbol:string ->
      ?step:float ->
      color_t -> (float -> float) -> float * float -> plot_t

    (** [transform f] sets the universal coordinate transformation function. *)
    val transform : pltr_t -> plot_t

    (** [clear_transform] clears any currently defined universal coordinate
        transform function. *)
    val clear_transform : plot_t

    (** [pltr f] sets the coordinate transformation function used by
        {!imagefr} and other functions affected by {!Plplot.plset_pltr}. *)
    val pltr : pltr_t -> plot_t

    (** [clear_pltr] clears any currently defined function-specific pltr
        transform function. *)
    val clear_pltr : plot_t

    (** [custom f] will call [f ()] when the plot element is used.  This
        function can be used to create customized plot elements for use with
        {!plot}. *)
    val custom : (unit -> unit) -> plot_t

    (** [list l] makes a plot element from a list of plot elements, allowing,
        for example, a plot to be created piece-by-piece before being
        rendered. *)
    val list : plot_t list -> plot_t

    (** [maybe m_p] will plot the contents of [m_p].  If [m_p] is [None] then
        this is effectively a no-op. *)
    val maybe : plot_t option -> plot_t

    (** Plot a list of plottable elements *)
    val plot : ?stream:stream_t -> plot_t list -> unit

    (** {4 Support functions} *)

    (** Character height in world coordinate units *)
    val character_height : ?stream:stream_t -> unit -> float

    (** Positioning within viewport or subpage *)
    type position_t

    (** Position relative to the plot viewport *)
    val viewport_pos :
      ?side1:side_t ->
      ?side2:side_t -> ?inside:bool -> float -> float -> position_t

    (** Position relative to the plot subpage *)
    val subpage_pos :
      ?side1:side_t ->
      ?side2:side_t -> ?inside:bool -> float -> float -> position_t

    (** Legend entry *)
    type legend_entry_t

    (** Layout of elements *)
    type 'a layout_t

    (** Empty legend entry *)
    val no_legend : legend_entry_t

    (** Color-filled box legend entry *)
    val box_legend :
      ?pattern:int ->
      ?scale:float ->
      ?line_width:float ->
      ?label_color:color_t -> label:string -> color_t -> legend_entry_t

    (** Line legend entry *)
    val line_legend :
      ?style:int ->
      ?width:float ->
      ?label_color:color_t -> label:string -> color_t -> legend_entry_t

    (** Symbol/point legend entry *)
    val symbol_legend :
      ?scale:float ->
      ?number:int ->
      ?label_color:color_t ->
      label:string -> color_t -> string -> legend_entry_t

    (** Row-major layout *)
    val row_major : 'a -> 'b -> ('a * 'b) layout_t

    (** Column-major layout *)
    val column_major : 'a -> 'b -> ('a * 'b) layout_t

    (** [legend entries] *)
    val legend :
      ?pos:position_t ->
      ?plot_width:float ->
      ?bg:color_t ->
      ?bb:color_t * line_style_t ->
      ?layout:(int * int) layout_t ->
      ?color:color_t ->
      ?text_offset:float ->
      ?text_scale:float ->
      ?text_spacing:float ->
      ?text_justification:float ->
      ?text_left:bool -> legend_entry_t list list -> plot_t

    (** Colorbar axis definitions *)
    type colorbar_axis_t

    (** Available colorbar kinds *)
    type colorbar_kind_t

    (** [colorbar_axis ?axis values] creates a {!colorbar_axis_t} with the
        given axis options and range values. *)
    val colorbar_axis :
      ?axis:axis_options_t list -> float array -> colorbar_axis_t

    (** [gradient_colorbar ?axis [|min; max|]] from [min] to [max] *)
    val gradient_colorbar :
      ?axis:axis_options_t list -> float array -> colorbar_kind_t

    (** [image_colorbar ?axis [|min; max|]] from [min] to [max] *)
    val image_colorbar :
      ?axis:axis_options_t list -> float array -> colorbar_kind_t

    (** [shade_colorbar ?custom ?axis contours] defines a shaded contour
        colorbar with axis labels at even spacings ([custom = false]) or at the
        locations of the values in [contours] ([custom = true] - the
        default). *)
    val shade_colorbar :
      ?custom:bool ->
      ?axis:axis_options_t list ->
      float array -> colorbar_kind_t

    (** Default options used for a colorbar axis *)
    val default_colorbar_axis : axis_options_t list

    (** [colorbar colorbar_kind] *)
    val colorbar :
      ?pos:position_t ->
      ?bg:color_t ->
      ?bb:color_t * line_style_t ->
      ?cap:float option * float option ->
      ?contour:color_t * float ->
      ?orient:(float * float) tagged_side_t ->
      ?axis:colorbar_axis_t list ->
      ?label:string tagged_side_t list ->
      ?color:color_t -> ?scale:float ->
      colorbar_kind_t -> plot_t

    (** Draw the plot axes on the current plot page *)
    val plot_axes :
      ?stream:stream_t ->
      axis_options_t list -> axis_options_t list -> unit

    (** {4 Finishing up a plot page} *)

    (** [end_stream ?stream ()] ends [stream].  This or {!finish} should be
        called once all plotting is complete. *)
    val end_stream : ?stream:stream_t -> unit -> unit

    (** [finish ?stream ()] draws default x and y axes, then closes [stream]. *)
    val finish : ?stream:stream_t -> unit -> unit
  end

(** {3 A module for quick, "throw-away" plots} *)
module Quick_plot :
  sig

    (** {b THIS API IS NOT FIXED AND MAY CHANGE BEFORE THE 5.10.0 RELEASE} *)

    (** [points xs ys] plots the points described by the coordinates [xs]
        and [ys]. *)
    val points :
      ?filename:string ->
      ?size:int * int ->
      ?device:Plot.plot_device_t ->
      ?labels:string * string * string ->
      ?names:string list ->
      ?log:bool * bool -> (float array * float array) list -> unit

    (** [lines xs ys] plots the line segments described by the coordinates
        [xs] and [ys]. *)
    val lines :
      ?filename:string ->
      ?size:int * int ->
      ?device:Plot.plot_device_t ->
      ?labels:string * string * string ->
      ?names:string list ->
      ?log:bool * bool -> (float array * float array) list -> unit

    (** [image ?log m] plots the image [m] with a matching colorbar.  If [log]
        is true then the data in [m] are assumed to be [log10(x)] values. *)
    val image :
      ?filename:string ->
      ?size:int * int ->
      ?device:Plot.plot_device_t ->
      ?labels:string * string * string ->
      ?log:bool -> ?palette:Plot.color_palette_t -> float array array -> unit

    (** [func ?point ?step fs (min, max)] plots the functions [fs] from
        [x = min] to [x = max].  [step] can be used to tighten or coarsen the
        sampling of plot points. *)
    val func :
      ?filename:string ->
      ?size:int * int ->
      ?device:Plot.plot_device_t ->
      ?labels:string * string * string ->
      ?names:string list ->
      ?symbol:string ->
      ?step:float -> (float -> float) list -> float * float -> unit

    (** [shades ?log ?contours m] plots a filled contour/shaded [m] with a
        matching colorbar.  If [log] is true then the data in [m] are assumed
        to be [log10(x)] values. *)
    val shades :
      ?filename:string ->
      ?size:int * int ->
      ?device:Plot.plot_device_t ->
      ?labels:string * string * string ->
      ?log:bool ->
      ?palette:Plot.color_palette_t ->
      ?contours:float array -> float array array -> unit
  end

(** {3:core The standard PLplot API} *)

(** See the main PLplot documentation for the description and documentation of
    these functions. *)

type plplot_axis_type =
    PL_X_AXIS
  | PL_Y_AXIS
  | PL_Z_AXIS
and plplot3d_style_enum =
    PL_DIFFUSE
  | PL_DRAW_LINEX
  | PL_DRAW_LINEY
  | PL_DRAW_LINEXY
  | PL_MAG_COLOR
  | PL_BASE_CONT
  | PL_TOP_CONT
  | PL_SURF_CONT
  | PL_DRAW_SIDES
  | PL_FACETED
  | PL_MESH
and plplot3d_style = plplot3d_style_enum list
and plplot_bin_enum =
    PL_BIN_DEFAULT
  | PL_BIN_CENTRED
  | PL_BIN_NOEXPAND
  | PL_BIN_NOEMPTY
and plplot_bin_style = plplot_bin_enum list
and plplot_hist_enum =
    PL_HIST_DEFAULT
  | PL_HIST_NOSCALING
  | PL_HIST_IGNORE_OUTLIERS
  | PL_HIST_NOEXPAND
  | PL_HIST_NOEMPTY
and plplot_hist_style = plplot_hist_enum list
and plplot_run_level_enum =
  | PL_UNINITIALIZED
  | PL_INITIALIZED
  | PL_VIEWPORT_DEFINED
  | PL_WORLD_COORDINATES_DEFINED
and plplot_run_level = plplot_run_level_enum
and plplot_position_enum =
    PL_POSITION_NULL
  | PL_POSITION_LEFT
  | PL_POSITION_RIGHT
  | PL_POSITION_TOP
  | PL_POSITION_BOTTOM
  | PL_POSITION_INSIDE
  | PL_POSITION_OUTSIDE
  | PL_POSITION_VIEWPORT
  | PL_POSITION_SUBPAGE
and plplot_position_opt = plplot_position_enum list
and plplot_legend_enum =
    PL_LEGEND_NULL
  | PL_LEGEND_NONE
  | PL_LEGEND_COLOR_BOX
  | PL_LEGEND_LINE
  | PL_LEGEND_SYMBOL
  | PL_LEGEND_TEXT_LEFT
  | PL_LEGEND_BACKGROUND
  | PL_LEGEND_BOUNDING_BOX
  | PL_LEGEND_ROW_MAJOR
and plplot_legend_opt = plplot_legend_enum list
and plplot_colorbar_enum =
    PL_COLORBAR_NULL
  | PL_COLORBAR_LABEL_LEFT
  | PL_COLORBAR_LABEL_RIGHT
  | PL_COLORBAR_LABEL_TOP
  | PL_COLORBAR_LABEL_BOTTOM
  | PL_COLORBAR_IMAGE
  | PL_COLORBAR_SHADE
  | PL_COLORBAR_GRADIENT
  | PL_COLORBAR_CAP_NONE
  | PL_COLORBAR_CAP_LOW
  | PL_COLORBAR_CAP_HIGH
  | PL_COLORBAR_SHADE_LABEL
  | PL_COLORBAR_ORIENT_RIGHT
  | PL_COLORBAR_ORIENT_TOP
  | PL_COLORBAR_ORIENT_LEFT
  | PL_COLORBAR_ORIENT_BOTTOM
  | PL_COLORBAR_BACKGROUND
  | PL_COLORBAR_BOUNDING_BOX
and plplot_colorbar_opt = plplot_colorbar_enum list
and plplot_fci_family_enum =
  | PL_FCI_FAMILY_UNCHANGED
  | PL_FCI_SANS
  | PL_FCI_SERIF
  | PL_FCI_MONO
  | PL_FCI_SCRIPT
  | PL_FCI_SYMBOL
and plplot_fci_style_enum =
  | PL_FCI_STYLE_UNCHANGED
  | PL_FCI_UPRIGHT
  | PL_FCI_ITALIC
  | PL_FCI_OBLIQUE
and plplot_fci_weight_enum =
  | PL_FCI_WEIGHT_UNCHANGED
  | PL_FCI_MEDIUM
  | PL_FCI_BOLD
and plplot_draw_mode_enum =
  | PL_DRAWMODE_UNKNOWN
  | PL_DRAWMODE_DEFAULT
  | PL_DRAWMODE_REPLACE
  | PL_DRAWMODE_XOR
external pl_setcontlabelformat : int -> int -> unit
  = "camlidl_plplot_core_c_pl_setcontlabelformat"
external pl_setcontlabelparam : float -> float -> float -> int -> unit
  = "camlidl_plplot_core_c_pl_setcontlabelparam"
external pladv : int -> unit = "camlidl_plplot_core_c_pladv"
external plarc :
  float -> float -> float -> float -> float -> float -> float -> bool -> unit
  = "camlidl_plplot_core_c_plarc_bytecode" "camlidl_plplot_core_c_plarc"
external plaxes :
  float -> float -> string -> float -> int -> string -> float -> int -> unit
  = "camlidl_plplot_core_c_plaxes_bytecode" "camlidl_plplot_core_c_plaxes"
external plbin : float array -> float array -> plplot_bin_style -> unit
  = "camlidl_plplot_core_c_plbin"
external plbtime : float -> int * int * int * int * int * float
  = "camlidl_plplot_core_c_plbtime"
external plbop : unit -> unit = "camlidl_plplot_core_c_plbop"
external plbox : string -> float -> int -> string -> float -> int -> unit
  = "camlidl_plplot_core_c_plbox_bytecode" "camlidl_plplot_core_c_plbox"
external plbox3 :
  string ->
  string ->
  float ->
  int ->
  string ->
  string -> float -> int -> string -> string -> float -> int -> unit
  = "camlidl_plplot_core_c_plbox3_bytecode" "camlidl_plplot_core_c_plbox3"
external plcalc_world : float -> float -> float * float * int
  = "camlidl_plplot_core_c_plcalc_world"
external plclear : unit -> unit = "camlidl_plplot_core_c_plclear"
external plcol0 : int -> unit = "camlidl_plplot_core_c_plcol0"
external plcol1 : float -> unit = "camlidl_plplot_core_c_plcol1"
external plconfigtime :
  float ->
  float ->
  float -> int -> bool -> int -> int -> int -> int -> int -> float -> unit
  = "camlidl_plplot_core_c_plconfigtime_bytecode"
  "camlidl_plplot_core_c_plconfigtime"
external plcpstrm : int -> bool -> unit = "camlidl_plplot_core_c_plcpstrm"
external plctime : int -> int -> int -> int -> int -> float -> float
  = "camlidl_plplot_core_c_plctime_bytecode" "camlidl_plplot_core_c_plctime"
external plend : unit -> unit = "camlidl_plplot_core_c_plend"
external plend1 : unit -> unit = "camlidl_plplot_core_c_plend1"
external plenv : float -> float -> float -> float -> int -> int -> unit
  = "camlidl_plplot_core_c_plenv_bytecode" "camlidl_plplot_core_c_plenv"
external plenv0 : float -> float -> float -> float -> int -> int -> unit
  = "camlidl_plplot_core_c_plenv0_bytecode" "camlidl_plplot_core_c_plenv0"
external pleop : unit -> unit = "camlidl_plplot_core_c_pleop"
external plerrx : float array -> float array -> float array -> unit
  = "camlidl_plplot_core_c_plerrx"
external plerry : float array -> float array -> float array -> unit
  = "camlidl_plplot_core_c_plerry"
external plfamadv : unit -> unit = "camlidl_plplot_core_c_plfamadv"
external plfill : float array -> float array -> unit
  = "camlidl_plplot_core_c_plfill"
external plfill3 : float array -> float array -> float array -> unit
  = "camlidl_plplot_core_c_plfill3"
external plflush : unit -> unit = "camlidl_plplot_core_c_plflush"
external plfont : int -> unit = "camlidl_plplot_core_c_plfont"
external plfontld : int -> unit = "camlidl_plplot_core_c_plfontld"
external plgchr : unit -> float * float = "camlidl_plplot_core_c_plgchr"
external plgcmap1_range : unit -> float * float =
  "camlidl_plplot_core_c_plgcmap1_range"
external plgcol0 : int -> int * int * int = "camlidl_plplot_core_c_plgcol0"
external plgcol0a : int -> int * int * int * float
  = "camlidl_plplot_core_c_plgcol0a"
external plgcolbg : unit -> int * int * int
  = "camlidl_plplot_core_c_plgcolbg"
external plgcolbga : unit -> int * int * int * float
  = "camlidl_plplot_core_c_plgcolbga"
external plgcompression : unit -> int
  = "camlidl_plplot_core_c_plgcompression"
external plgdev : unit -> string = "camlidl_plplot_core_c_plgdev"
external plgdidev : unit -> float * float * float * float
  = "camlidl_plplot_core_c_plgdidev"
external plgdiori : unit -> float = "camlidl_plplot_core_c_plgdiori"
external plgdiplt : unit -> float * float * float * float
  = "camlidl_plplot_core_c_plgdiplt"
external plgdrawmode : unit -> plplot_draw_mode_enum
  = "camlidl_plplot_core_c_plgdrawmode"
external plgfci : unit -> int64 = "camlidl_plplot_core_c_plgfci"
external plgfam : unit -> int * int * int = "camlidl_plplot_core_c_plgfam"
external plgfnam : unit -> string = "camlidl_plplot_core_c_plgfnam"
external plgfont : unit -> int * int * int = "camlidl_plplot_core_c_plgfont"
external plglevel : unit -> plplot_run_level = "camlidl_plplot_core_c_plglevel"
external plgpage : unit -> float * float * int * int * int * int
  = "camlidl_plplot_core_c_plgpage"
external plgra : unit -> unit = "camlidl_plplot_core_c_plgra"
external plgradient : float array -> float array -> float -> unit
  = "camlidl_plplot_core_c_plgradient"
external plgspa : unit -> float * float * float * float
  = "camlidl_plplot_core_c_plgspa"
external plgstrm : unit -> int = "camlidl_plplot_core_c_plgstrm"
external plgver : unit -> string = "camlidl_plplot_core_c_plgver"
external plgvpd : unit -> float * float * float * float
  = "camlidl_plplot_core_c_plgvpd"
external plgvpw : unit -> float * float * float * float
  = "camlidl_plplot_core_c_plgvpw"
external plgxax : unit -> int * int = "camlidl_plplot_core_c_plgxax"
external plgyax : unit -> int * int = "camlidl_plplot_core_c_plgyax"
external plgzax : unit -> int * int = "camlidl_plplot_core_c_plgzax"
external plhist :
  float array -> float -> float -> int -> plplot_hist_style -> unit
  = "camlidl_plplot_core_c_plhist"
external plhlsrgb : float -> float -> float -> float * float * float
  = "camlidl_plplot_core_c_plhlsrgb"
external plinit : unit -> unit = "camlidl_plplot_core_c_plinit"
external pljoin : float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_pljoin"
external pllab : string -> string -> string -> unit
  = "camlidl_plplot_core_c_pllab"
external plcolorbar : plplot_colorbar_opt -> plplot_position_opt -> float ->
  float -> float -> float -> int -> int -> int -> float -> float -> int ->
  float -> plplot_colorbar_opt array -> string array ->
  string array -> float array -> int array -> float array array ->
  float * float
  = "ml_plcolorbar_byte" "ml_plcolorbar"
external pllegend : plplot_legend_opt -> plplot_position_opt ->
  float -> float -> float -> int -> int -> int -> int -> int ->
  plplot_legend_opt array -> float -> float ->
  float -> float -> int array -> string array -> int array -> int array ->
  float array -> float array -> int array -> int array -> float array ->
  int array -> float array -> int array -> string array -> float * float
  = "ml_pllegend_byte" "ml_pllegend"
external pllightsource : float -> float -> float -> unit
  = "camlidl_plplot_core_c_pllightsource"
external plline : float array -> float array -> unit
  = "camlidl_plplot_core_c_plline"
external plline3 : float array -> float array -> float array -> unit
  = "camlidl_plplot_core_c_plline3"
external pllsty : int -> unit = "camlidl_plplot_core_c_pllsty"
external plmesh :
  float array -> float array -> float array array -> plplot3d_style -> unit
  = "camlidl_plplot_core_c_plmesh"
external plmeshc :
  float array ->
  float array -> float array array -> plplot3d_style -> float array -> unit
  = "camlidl_plplot_core_c_plmeshc"
external plmkstrm : unit -> int = "camlidl_plplot_core_c_plmkstrm"
external plmtex : string -> float -> float -> float -> string -> unit
  = "camlidl_plplot_core_c_plmtex"
external plmtex3 : string -> float -> float -> float -> string -> unit
  = "camlidl_plplot_core_c_plmtex3"
external plot3d :
  float array ->
  float array -> float array array -> plplot3d_style -> bool -> unit
  = "camlidl_plplot_core_c_plot3d"
external plot3dc :
  float array ->
  float array -> float array array -> plplot3d_style -> float array -> unit
  = "camlidl_plplot_core_c_plot3dc"
external plot3dcl :
  float array ->
  float array -> float array array -> plplot3d_style -> float array -> int -> int array -> int array -> unit
  = "camlidl_plplot_core_c_plot3dcl_bytecode" "camlidl_plplot_core_c_plot3dcl"
external plpat : int array -> int array -> unit
  = "camlidl_plplot_core_c_plpat"
external plpath : int -> float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plpath"
external plpoin : float array -> float array -> int -> unit
  = "camlidl_plplot_core_c_plpoin"
external plpoin3 : float array -> float array -> float array -> int -> unit
  = "camlidl_plplot_core_c_plpoin3"
external plprec : int -> int -> unit = "camlidl_plplot_core_c_plprec"
external plpsty : int -> unit = "camlidl_plplot_core_c_plpsty"
external plptex : float -> float -> float -> float -> float -> string -> unit
  = "camlidl_plplot_core_c_plptex_bytecode" "camlidl_plplot_core_c_plptex"
external plptex3 :
  float ->
  float ->
  float ->
  float ->
  float -> float -> float -> float -> float -> float -> string -> unit
  = "camlidl_plplot_core_c_plptex3_bytecode" "camlidl_plplot_core_c_plptex3"
external plrandd : unit -> float = "camlidl_plplot_core_c_plrandd"
external plreplot : unit -> unit = "camlidl_plplot_core_c_plreplot"
external plrgbhls : float -> float -> float -> float * float * float
  = "camlidl_plplot_core_c_plrgbhls"
external plschr : float -> float -> unit = "camlidl_plplot_core_c_plschr"
external plscmap0 : int array -> int array -> int array -> unit
  = "camlidl_plplot_core_c_plscmap0"
external plscmap0a :
  int array -> int array -> int array -> float array -> unit
  = "camlidl_plplot_core_c_plscmap0a"
external plscmap0n : int -> unit = "camlidl_plplot_core_c_plscmap0n"
external plscmap1 : int array -> int array -> int array -> unit
  = "camlidl_plplot_core_c_plscmap1"
external plscmap1a :
  int array -> int array -> int array -> float array -> unit
  = "camlidl_plplot_core_c_plscmap1a"
external plscmap1l :
  bool ->
  float array ->
  float array -> float array -> float array -> bool array option -> unit
  = "camlidl_plplot_core_c_plscmap1l_bytecode"
  "camlidl_plplot_core_c_plscmap1l"
external plscmap1la :
  bool ->
  float array ->
  float array ->
  float array -> float array -> float array -> bool array option -> unit
  = "camlidl_plplot_core_c_plscmap1la_bytecode"
  "camlidl_plplot_core_c_plscmap1la"
external plscmap1n : int -> unit = "camlidl_plplot_core_c_plscmap1n"
external plscmap1_range : float -> float -> unit =
  "camlidl_plplot_core_c_plscmap1_range"
external plscol0 : int -> int -> int -> int -> unit
  = "camlidl_plplot_core_c_plscol0"
external plscol0a : int -> int -> int -> int -> float -> unit
  = "camlidl_plplot_core_c_plscol0a"
external plscolbg : int -> int -> int -> unit
  = "camlidl_plplot_core_c_plscolbg"
external plscolbga : int -> int -> int -> float -> unit
  = "camlidl_plplot_core_c_plscolbga"
external plscolor : int -> unit = "camlidl_plplot_core_c_plscolor"
external plscompression : int -> unit
  = "camlidl_plplot_core_c_plscompression"
external plsdev : string -> unit = "camlidl_plplot_core_c_plsdev"
external plsdidev : float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plsdidev"
external plsdimap : int -> int -> int -> int -> float -> float -> unit
  = "camlidl_plplot_core_c_plsdimap_bytecode"
  "camlidl_plplot_core_c_plsdimap"
external plsdiori : float -> unit = "camlidl_plplot_core_c_plsdiori"
external plsdiplt : float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plsdiplt"
external plsdiplz : float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plsdiplz"
external plseed : int64 -> unit = "camlidl_plplot_core_c_plseed"
external plsesc : char -> unit = "camlidl_plplot_core_c_plsesc"
external plsfam : int -> int -> int -> unit = "camlidl_plplot_core_c_plsfam"
external plsfci : int64 -> unit = "camlidl_plplot_core_c_plsfci"
external plsfnam : string -> unit = "camlidl_plplot_core_c_plsfnam"
external plsfont : plplot_fci_family_enum -> plplot_fci_style_enum ->
  plplot_fci_weight_enum -> unit
  = "camlidl_plplot_core_c_plsfont"
external plsmaj : float -> float -> unit = "camlidl_plplot_core_c_plsmaj"
external plsmin : float -> float -> unit = "camlidl_plplot_core_c_plsmin"
external plsdrawmode : plplot_draw_mode_enum -> unit
  = "camlidl_plplot_core_c_plsdrawmode"
external plsori : int -> unit = "camlidl_plplot_core_c_plsori"
external plspage : float -> float -> int -> int -> int -> int -> unit
  = "camlidl_plplot_core_c_plspage_bytecode" "camlidl_plplot_core_c_plspage"
external plspal0 : string -> unit = "camlidl_plplot_core_c_plspal0"
external plspal1 : string -> bool -> unit = "camlidl_plplot_core_c_plspal1"
external plspause : bool -> unit = "camlidl_plplot_core_c_plspause"
external plsstrm : int -> unit = "camlidl_plplot_core_c_plsstrm"
external plssub : int -> int -> unit = "camlidl_plplot_core_c_plssub"
external plssym : float -> float -> unit = "camlidl_plplot_core_c_plssym"
external plstar : int -> int -> unit = "camlidl_plplot_core_c_plstar"
external plstart : string -> int -> int -> unit
  = "camlidl_plplot_core_c_plstart"
external plstring : float array -> float array -> string -> unit
  = "camlidl_plplot_core_c_plstring"
external plstring3 : float array -> float array -> float array -> string -> unit
  = "camlidl_plplot_core_c_plstring3"
external plstripa : int -> int -> float -> float -> unit
  = "camlidl_plplot_core_c_plstripa"
external plstripd : int -> unit = "camlidl_plplot_core_c_plstripd"
external plimage :
  float array array ->
  float ->
  float ->
  float ->
  float -> float -> float -> float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plimage_bytecode" "camlidl_plplot_core_c_plimage"
external plstyl : int array -> int array -> unit
  = "camlidl_plplot_core_c_plstyl"
external plsurf3d :
  float array ->
  float array -> float array array -> plplot3d_style -> float array -> unit
  = "camlidl_plplot_core_c_plsurf3d"
external plsurf3dl :
  float array -> float array -> float array array -> plplot3d_style -> float array -> int -> int array -> int array -> unit
  = "camlidl_plplot_core_c_plsurf3dl_bytecode" "camlidl_plplot_core_c_plsurf3dl"
external plsvect : float array -> float array -> bool -> unit
  = "camlidl_plplot_core_c_plsvect"
external plsvpa : float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plsvpa"
external plsxax : int -> int -> unit = "camlidl_plplot_core_c_plsxax"
external plsxwin : int -> unit = "camlidl_plplot_core_plsxwin"
external plsyax : int -> int -> unit = "camlidl_plplot_core_c_plsyax"
external plsym : float array -> float array -> int -> unit
  = "camlidl_plplot_core_c_plsym"
external plszax : int -> int -> unit = "camlidl_plplot_core_c_plszax"
external pltext : unit -> unit = "camlidl_plplot_core_c_pltext"
external pltimefmt : string -> unit = "camlidl_plplot_core_c_pltimefmt"
external plvasp : float -> unit = "camlidl_plplot_core_c_plvasp"
external plvpas : float -> float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plvpas"
external plvpor : float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plvpor"
external plvsta : unit -> unit = "camlidl_plplot_core_c_plvsta"
external plw3d :
  float ->
  float ->
  float ->
  float ->
  float -> float -> float -> float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plw3d_bytecode" "camlidl_plplot_core_c_plw3d"
external plwidth : float -> unit = "camlidl_plplot_core_c_plwidth"
external plwind : float -> float -> float -> float -> unit
  = "camlidl_plplot_core_c_plwind"
external plxormod : bool -> bool = "camlidl_plplot_core_c_plxormod"
external plsetopt : string -> string -> unit
  = "camlidl_plplot_core_c_plsetopt"
external plMinMax2dGrid : float array array -> float * float
  = "camlidl_plplot_core_plMinMax2dGrid"
external plcont :
  float array array -> int -> int -> int -> int -> float array -> unit
  = "camlidl_plplot_core_ml_plcont_bytecode" "camlidl_plplot_core_ml_plcont"
external plshade :
  float array array ->
  float ->
  float ->
  float ->
  float ->
  float ->
  float -> int -> float -> float -> int -> float -> int -> float -> bool -> unit
  = "camlidl_plplot_core_ml_plshade_bytecode"
  "camlidl_plplot_core_ml_plshade"
external plshades :
  float array array ->
  float ->
  float -> float -> float -> float array -> float -> int -> float -> bool -> unit
  = "camlidl_plplot_core_ml_plshades_bytecode"
  "camlidl_plplot_core_ml_plshades"
external plimagefr :
  float array array ->
  float ->
  float -> float -> float -> float -> float -> float -> float -> unit
  = "camlidl_plplot_core_ml_plimagefr_bytecode"
  "camlidl_plplot_core_ml_plimagefr"
external plvect : float array array -> float array array -> float -> unit
  = "camlidl_plplot_core_ml_plvect"
external plmap : string -> float -> float -> float -> float -> unit
  = "camlidl_plplot_core_ml_plmap"
external plmapline : string -> float -> float -> float -> float -> int array -> unit
  = "camlidl_plplot_core_ml_plmapline_bytecode" "camlidl_plplot_core_ml_plmapline"
external plmapstring : string -> string -> float -> float -> float -> float -> int array -> unit
  = "camlidl_plplot_core_ml_plmapstring_bytecode" "camlidl_plplot_core_ml_plmapstring"
external plmaptex : string -> float -> float -> float -> string -> float -> float -> float -> float -> int -> unit
  = "camlidl_plplot_core_ml_plmaptex_bytecode" "camlidl_plplot_core_ml_plmaptex"
external plmapfill : string -> float -> float -> float -> float -> int array -> unit
  = "camlidl_plplot_core_ml_plmapfill_bytecode" "camlidl_plplot_core_ml_plmapfill"
external plmeridians :
  float -> float -> float -> float -> float -> float -> unit
  = "camlidl_plplot_core_ml_plmeridians_bytecode"
  "camlidl_plplot_core_ml_plmeridians"
external plpoly3 :
  float array -> float array -> float array -> bool array -> bool -> unit
  = "camlidl_plplot_core_ml_plpoly3"
external pltr0 : float -> float -> float * float
  = "camlidl_plplot_core_ml_pltr0"
external plsvect_reset : unit -> unit = "camlidl_plplot_core_ml_plsvect_reset"
external plg_current_col0 : unit -> int
  = "camlidl_plplot_core_plg_current_col0"
external plg_current_col1 : unit -> float
  = "camlidl_plplot_core_plg_current_col1"
external plgwidth : unit -> float = "camlidl_plplot_core_plgwidth"
external plgchrht : unit -> float = "camlidl_plplot_core_plgchrht"
external plstripc :
  string ->
  string ->
  float ->
  float ->
  float ->
  float ->
  float ->
  float ->
  float ->
  bool ->
  bool ->
  int ->
  int ->
  int array -> int array -> string array -> string -> string -> string -> int
  = "ml_plstripc_byte" "ml_plstripc"
external pltr1 :
  float array -> float array -> float -> float -> float * float = "ml_pltr1"
external pltr2 :
  float array array -> float array array -> float -> float -> float * float
  = "ml_pltr2"
val plset_pltr : (float -> float -> float * float) -> unit
val plunset_pltr : unit -> unit
val plset_mapform : (float -> float -> float * float) -> unit
val plunset_mapform : unit -> unit
val plset_defined : (float -> float -> int) -> unit
val plunset_defined : unit -> unit
val plstransform : (float -> float -> (float * float)) -> unit
val plunset_transform : unit -> unit
type plplot_grid_method_type =
    PL_GRID_CSA
  | PL_GRID_DTLI
  | PL_GRID_NNI
  | PL_GRID_NNIDW
  | PL_GRID_NNLI
  | PL_GRID_NNAIDW
type plplot_parse_method_type =
    PL_PARSE_PARTIAL
  | PL_PARSE_FULL
  | PL_PARSE_QUIET
  | PL_PARSE_NODELETE
  | PL_PARSE_SHOWALL
  | PL_PARSE_OVERRIDE
  | PL_PARSE_NOPROGRAM
  | PL_PARSE_NODASH
  | PL_PARSE_SKIP
val plslabelfunc : (plplot_axis_type -> float -> string) -> unit
val plunset_labelfunc : unit -> unit
val plsabort : (string -> unit) -> unit
val plunset_abort : unit -> unit
val plsexit : (string -> int) -> unit
val plunset_exit : unit -> unit
external plgriddata :
  float array ->
  float array ->
  float array ->
  float array ->
  float array -> plplot_grid_method_type -> float -> float array array
  = "ml_plgriddata_bytecode" "ml_plgriddata"
external plparseopts : string array -> plplot_parse_method_type list -> unit
  = "ml_plparseopts"

(** {3 Non-standard convenience functions (not part of the standard PLplot
    API.} *)

(** [plcalc_device x y] will give the device position, in normalized device
    coordinates, of the world coordinates (x, y). *)
val plcalc_device : float -> float -> float * float

(** [plfullcanvas ()] maximizes the plot viewport and window.  Dimensions are
    set to (0.0, 0.0) to (1.0, 1.0). *)
val plfullcanvas : unit -> unit

(** Draw an unfilled polygon.  The only functional difference between this and
    {!plline} is that this function will close the given polygon, so there is no
    need to duplicate points to have a closed figure. *)
val plpolyline : float array -> float array -> unit

