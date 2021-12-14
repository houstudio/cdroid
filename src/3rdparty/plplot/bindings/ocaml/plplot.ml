(*
Copyright 2009, 2010, 2011  Hezekiah M. Carty

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

include Plplot_core
open Printf

(** [plcalc_device x y] will give the device position, in normalized device
    coordinates, of the world coordinates (x, y). *)
let plcalc_device x y =
  let xmin, xmax, ymin, ymax = plgvpw () in
  let dev_xmin, dev_xmax, dev_ymin, dev_ymax = plgvpd () in
  let width = xmax -. xmin in
  let height = ymax -. ymin in
  let dev_width = dev_xmax -. dev_xmin in
  let dev_height = dev_ymax -. dev_ymin in
  ((x -. xmin) /. width) *. dev_width +. dev_xmin,
  ((y -. ymin) /. height) *. dev_height +. dev_ymin

(** [plfullcanvas ()] maximizes the plot viewport and window.  Dimensions are
    set to (0.0, 0.0) to (1.0, 1.0). *)
let plfullcanvas () =
  plvpor 0.0 1.0 0.0 1.0;
  plwind 0.0 1.0 0.0 1.0;
  ()

(** Draw an unfilled polygon.  The only functional difference between this and
    {plline} is that this function will close the given polygon, so there is no
    need to duplicate points to have a closed figure. *)
let plpolyline xs ys =
  plline xs ys;
  (* Close off the polygon. *)
  pljoin xs.(0) ys.(0) xs.(Array.length xs - 1) ys.(Array.length ys - 1)

(** Support modules for Plot and Quick_plot *)

module Option = struct
  let may f o =
    match o with
    | Some x -> f x
    | None -> ()

  let default x_default o =
    match o with
    | Some x -> x
    | None -> x_default

  let map f o =
    match o with
    | Some x -> Some (f x)
    | None -> None

  let map_default f x_default o =
    match o with
    | Some x -> f x
    | None -> x_default
end

module Array_ext = struct
  let matrix_dims m =
    let ni = Array.length m in
    if ni = 0 then 0, 0 else (
      let nj = Array.length (Array.unsafe_get m 0) in
      for i = 1 to ni - 1 do
        if Array.length (Array.unsafe_get m i) <> nj
        then invalid_arg "Non-rectangular array"
      done;
      ni, nj
    )

  (** [range ~n a b] returns an array with elements ranging from [a] to [b]
      in [n] total elements. *)
  let range ~n a b =
    let step = (b -. a) /. (float n -. 1.0) in
    Array.init n (
      fun i ->
        a +. float i *. step
    )

  (** [reduce f a] performs a fold of [f] over [a], using only the elements of
      [a]. *)
  let reduce f a =
    if Array.length a = 0 then
      invalid_arg "reduce: empty array"
    else (
      let acc = ref a.(0) in
      for i = 1 to Array.length a - 1 do acc := f !acc a.(i) done;
      !acc
    )
end

(** Work-in-progress easy plotting library for PLplot.  The intention is to
    provide a PLplot interface which is closer in style to other OCaml
    libraries. *)
module Plot = struct
  let ( |? ) o x_default = Option.default x_default o

  (** [verify_arg x s] will raise [Invalid_arg s] if [x] is false.  Otherwise,
      it does nothing. *)
  let verify_arg x s = if x then () else (invalid_arg s)

  type axis_t = [`x | `y | `z]

  type side_t = [`top | `bottom | `left | `right]
  type 'a tagged_side_t =
    [`top of 'a | `bottom of 'a | `left of 'a | `right of 'a]

  type axis_options_t =
    [ `axis
    | `frame0 | `frame1
    | `time
    | `fixed_point
    | `major_grid | `minor_grid
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

  type stream_t = {
    stream : int;
  }

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

  type map_t =
    [ `globe
    | `usa
    | `countries
    | `all ]

  type pltr_t = float -> float -> float * float

  (** Line plotting styles/patterns. *)
  type line_style_t =
    [ `solid
    | `line1 | `line2 | `line3 | `line4
    | `line5 | `line6 | `line7 | `line8
    | `custom of ((int * int) list) ]

  type plot_t =
    (* Standard plot elements *)
    | Arc of
      (color_t * float * float * float * float * float * float * float * bool)
    | Axes of
      (color_t * axis_options_t list * axis_options_t list * float *
       line_style_t * (axis_t -> float -> string) option)
    | Contours of (color_t * pltr_t * float array * float array array)
    | Image of image_t
    | Image_fr of (image_t * (float * float))
    | Join of (color_t * float * float * float * float * float * line_style_t)
    | Labels of (color_t * string * string * string)
    | Lines of
      (string option * color_t * float array * float array * float * line_style_t)
    | Map of (color_t * map_t * float * float * float * float)
    | Points of
      (string option * color_t * float array * float array * string * float)
    | Polygon of (color_t * float array * float array * bool)
    | Shades of (
      float * color_t * float * bool * float * float * float * float * float array
      array * float array
    )
    | Text of (color_t * string * float * float * float * float * float)
    | Text_outside of
      (color_t * string * float tagged_side_t * float * float * bool)
    (* Set/clear UNIVERSAL coordinate transforms *)
    | Set_transform of pltr_t
    | Clear_transform
    (* Set/clear item-specific coordinate transforms (ex. plimagefr) *)
    | Set_pltr of pltr_t
    | Clear_pltr
    (* Custom functions *)
    | Custom of (unit -> unit)
    (* Embedded list of plottable elements *)
    | List of plot_t list
    (* Could be a plottable element... *)
    | Maybe of plot_t option
  and image_t = (float * float) option * float * float * float * float * float array array

  type plot_device_family_t =
    [ `cairo
    | `qt
    | `core
    | `wx ]
  type plot_device_t =
    [ `pdf of plot_device_family_t
    | `png of plot_device_family_t
    | `ps of plot_device_family_t
    | `svg of plot_device_family_t
    | `window of plot_device_family_t
    | `prompt
    | `stream of int (* Existing stream *)
    | `other_device of string ]
  type plot_scaling_t = [ `preset | `greedy | `equal | `equal_square ]

  type color_palette_t =
    | Indexed_palette of string
    | Continuous_palette of (string * bool)

  let axis_of_axis = function
    | PL_X_AXIS -> `x
    | PL_Y_AXIS -> `y
    | PL_Z_AXIS -> `z
  let convert_labelfunc f axis v =
    f (axis_of_axis axis) v

  let indexed_palette filename = Indexed_palette filename
  let continuous_palette ?(interpolate = true) filename =
    Continuous_palette (filename, interpolate)

  let default_axis_options =
    [`frame0; `frame1; `major_ticks; `minor_ticks; `invert_ticks; `label]

  let string_ticks_of_axis_options ol =
    let tick = ref 0.0 in
    let sub = ref 0 in
    let translated_list =
      List.map (
        function
        | `axis -> "a"
        | `frame0 -> "b"
        | `frame1 -> "c"
        | `time -> "d"
        | `fixed_point -> "f"
        | `major_grid -> "g"
        | `minor_grid -> "h"
        | `invert_ticks -> "i"
        | `log -> "l"
        | `unconventional_label -> "m"
        | `label -> "n"
        | `custom_label -> "o"
        | `minor_ticks -> "s"
        | `minor_tick_count t_sub -> sub := t_sub; "s"
        | `major_ticks -> "t"
        | `major_tick_spacing t_space -> tick := t_space; "t"
        | `vertical_label -> "v"
      ) ol
    in
    String.concat "" translated_list, !tick, !sub

  (** Convert a color to a (red, green, blue) triple *)
  let rgb_of_color = function
    | `white -> 255, 255, 255
    | `red -> 255, 0, 0
    | `yellow -> 255, 255, 0
    | `green -> 0, 255, 0
    | `gray -> 200, 200, 200
    | `blue -> 0, 0, 255
    | `light_blue -> 0, 255, 255
    | `purple -> 160, 0, 213
    | `pink -> 255, 0, 255
    | `black -> 0, 0, 0
    | `brown -> 165, 42, 42
    | `index i -> plgcol0 i

  let string_of_map_t = function
    | `globe -> "globe"
    | `usa -> "usa"
    | `countries -> "cglobe"
    | `all -> "usaglobe"

  (** An internal function for converting a scaling variant value to the
      associated PLplot integer value. *)
  let int_of_scaling = function
    | `preset -> -1     (* Scaling must be set beforehand *)
    | `greedy -> 0      (* Use as much of the plot space as possible *)
    | `equal -> 1       (* Square aspect ratio *)
    | `equal_square -> 2 (* Square aspect ratio and square plot area *)


  (** Get the suffix string which matches the given device family *)
  let string_of_device_family = function
    | `cairo -> "cairo"
    | `qt -> "qt"
    | `core -> ""
    | `wx -> ""

  (** Returns the string to pass to plsdev and a boolean value indicating if
      the device is interactive or not. *)
  let devstring_of_plot_device = function
    | `prompt -> "", false
    | `stream _ -> invalid_arg "External stream"
    | `other_device s -> s, false
    | `pdf family -> "pdf" ^ string_of_device_family family, false
    | `png family -> "png" ^ string_of_device_family family , false
    | `ps family -> "ps" ^ string_of_device_family family, false
    | `svg family -> "svg" ^ string_of_device_family family, false
    | `window family -> (
      match family with
      | `cairo -> "xcairo"
      | `qt -> "qtwidget"
      | `core -> "xwin"
      | `wx -> "wxwidgets"
      ), true

  let recommended_extension = function
    | `png _ -> ".png"
    | `ps _ -> ".ps"
    | `pdf _ -> ".pdf"
    | `svg _ -> ".svg"
    | `window _ -> invalid_arg "interactive plot device, no extension"
    | `stream _ -> invalid_arg "external stream, unknown extension"
    | `other_device _ -> invalid_arg "other device, unknown extension"
    | `prompt -> invalid_arg "user prompted for device, unknown extension"

  (** Make a new stream, without disturbing the current plot state. *)
  let make_stream ?stream () =
    let this_stream =
      match stream with
      | None ->
          let old_stream = plgstrm () in
          let new_stream = plmkstrm () in
          plsstrm old_stream;
          new_stream
      | Some s -> s
    in
    { stream = this_stream }

  (** [with_stream ?stream f] calls [f ()] with [stream] as the active
      plotting stream if [stream] is present.  Otherwise it just calls
      [f ()]. *)
  let with_stream ?stream f =
    match stream with
    | None -> f ()
    | Some s ->
        let old_stream = plgstrm () in
        plsstrm s.stream;
        let result = f () in
        plsstrm old_stream;
        result

  (** Get the integer representation of the line style for use with pllsty *)
  let int_of_line_style = function
    | `solid -> 1
    | `line1 -> 1
    | `line2 -> 2
    | `line3 -> 3
    | `line4 -> 4
    | `line5 -> 5
    | `line6 -> 6
    | `line7 -> 7
    | `line8 -> 8
    | `custom _ -> assert false

  (** Set the line drawing style *)
  let set_line_style ?stream style =
    with_stream ?stream (
      fun () ->
        match style with
        | `custom l ->
            let marks, spaces = List.split l in
            let marks = Array.of_list marks in
            let spaces = Array.of_list spaces in
            plstyl marks spaces
        | s -> pllsty (int_of_line_style s)
    )

  (** NOTE that these are for the ALTERNATE color palette, not the DEFAULT
      color palette. *)
  let int_of_color = function
    | `white -> 0
    | `red -> 3
    | `yellow -> 13
    | `green -> 12
    | `gray -> 10
    | `blue -> 2
    | `light_blue -> 11
    | `purple -> 15
    | `pink -> 14
    | `black -> 1
    | `brown -> 4
    | `index i -> i

  (** Set the plotting color (color scale 0). NOTE that these are for the
      ALTERNATE color palette, not the DEFAULT color palette. *)
  let set_color ?stream c =
    with_stream ?stream (fun () -> plcol0 (int_of_color c))

  (** [set_color_scale ?stream ?pos ?alt_hue_path colors] sets the color scale 1 (images
      and shade plots) using a linear interpolation between the given list of
      colors.  If [alt_hue_path] is true then the interpolation of any segment
      uses the alternative hue path which always includes the hue = 0 point. *)
  let set_color_scale ?stream ?pos ?alt_hue_path colors =
    let cs = Array.map rgb_of_color colors in
    let r, g, b =
      Array.map (fun (r, _, _) -> float_of_int r /. 255.0) cs,
      Array.map (fun (_, g, _) -> float_of_int g /. 255.0) cs,
      Array.map (fun (_, _, b) -> float_of_int b /. 255.0) cs
    in
    let positions = pos |? Array_ext.range ~n:(Array.length cs) 0.0 1.0 in
    with_stream ?stream (fun () -> plscmap1l true positions r g b alt_hue_path);
    ()

  (** Start a new page *)
  let start_page ?stream (x0, y0) (x1, y1) axis_scaling =
    with_stream ?stream (
      fun () ->
        (* Start with a black plotting color. *)
        set_color `black;
        plenv x0 x1 y0 y1 (int_of_scaling axis_scaling) (-2);
    )

  (** Load a color palette from a file on disk *)
  let load_palette ?stream which =
    with_stream ?stream (
      fun () ->
        match which with
        | Indexed_palette file -> plspal0 file
        | Continuous_palette (file, segmented) -> plspal1 file segmented
    )

  (** Create a new plot instance *)
  let make ?stream ?filename ?size ?pre device =
    (* Make a new plot stream. *)
    let stream, init =
      match device with
      | `stream stream -> make_stream ~stream (), false
      | _ -> make_stream (), true
    in
    (* If an external stream is provided, assume all initialization has been
       performed before we get here. *)
    if init then with_stream ~stream (
      fun () ->
        (* Set the output file name *)
        let dev, is_interactive = devstring_of_plot_device device in
        Option.may (
          fun name_base ->
            plsfnam (
              name_base ^ (
                if is_interactive then ""
                else (
                  (* Only append an extension if the filename does not have
                     one already. *)
                  let extension = recommended_extension device in
                  if Filename.check_suffix name_base extension then ""
                  else extension
                )
              )
            );
        ) filename;

        (* Set the physical page dimensions for the plot *)
        Option.may (
          fun (x, y) ->
            let dim_string = sprintf "%dx%d" x y in
            ignore (plsetopt "geometry" dim_string)
        ) size;
        plsdev dev;

        (* Run the requested pre-plot-initialization function and then
           initialize the new plot. *)
        Option.may (fun f -> f ()) pre;
        plinit ();
    );
    stream

  (** Create a new plot instance and initialize it. *)
  let init ?filename ?size ?pages ?pre (x0, y0) (x1, y1) axis_scaling device =
    (* Creat a new plot stream and set it up. *)
    let stream = make ?filename ?size ?pre device in
    (* If requested, set up multiple sub-pages. *)
    Option.may (fun (x, y) -> with_stream ~stream (fun () -> plssub x y)) pages;
    start_page ~stream (x0, y0) (x1, y1) axis_scaling;
    stream

  (** [make_stream_active stream] makes [stream] the active plot stream.  If
      there is another active plot stream its identity is not saved. *)
  let make_stream_active ~stream = plsstrm stream.stream

  (** {3 Simplified plotting interface} *)

  (** [arc ?fill color x y a b angle1 angle2 rotation] *)
  let arc ?(fill = false) color x y a b angle1 angle2 rotation =
    Arc (color, x, y, a, b, angle1, angle2, rotation, fill)

  (** [axes ?color ?style ?width xopt yopt] *)
  let axes ?(color = `black) ?(style = `solid) ?(width = 1.0) ?labelfunc xopt yopt =
    Axes (color, xopt, yopt, width, style, labelfunc)

  (** Default axes *)
  let default_axes = axes default_axis_options default_axis_options

  (** [circle ?fill color x y r] - A special case of [arc]. *)
  let circle ?fill color x y r = arc ?fill color x y r r 0.0 360.0 0.0

  (** [contours color pltr contours data] *)
  let contours color pltr contours data =
    Contours (color, pltr, contours, data)

  (** [image ?range sw ne image] *)
  let image ?range (x0, y0) (x1, y1) data = Image (range, x0, y0, x1, y1, data)

  (** [imagefr ?range ~scale sw ne image] *)
  let imagefr ?range ~scale (x0, y0) (x1, y1) data =
    Image_fr ((range, x0, y0, x1, y1, data), scale)

  (** [join color (x0, y0) (x1, y1)] *)
  let join ?style ?width color (x0, y0) (x1, y1) =
    Join (color, x0, y0, x1, y1, width |? 1.0, style |? `solid)

  (** [label x y title] labels the axes and adds plot title *)
  let label ?(color = `black) x y title =
    Labels (color, x, y, title)

  (** [lines ?label color xs ys] *)
  let lines ?label ?style ?width color xs ys =
    Lines (label, color, xs, ys, width |? 1.0, style |? `solid)

  (** [map ?sw ?ne color outline] *)
  let map ?sw ?ne color outline =
    let x0, y0 = sw |? (-180.0, -90.0) in
    let x1, y1 = ne |? (180.0, 90.0) in
    Map (color, outline, x0, y0, x1, y1)

  (** [points ?label ?scale color xs ys] *)
  let points ?label ?symbol ?scale color xs ys =
    Points (label, color, xs, ys, symbol |? "#(135)", scale |? 1.0)

  (** [polygon ?fill color xs ys fill] *)
  let polygon ?(fill = false) color xs ys = Polygon (color, xs, ys, fill)

  (** [rectangle ?fill color (x0, y0) (x1, y1)] *)
  let rectangle ?(fill = false) color (x0, y0) (x1, y1) =
    polygon ~fill color [|x0; x1; x1; x0; x0|] [|y0; y0; y1; y1; y0|]

  (** [shades ?fill_width ?contour ?rect (x0, y0) (x1, y1) data] *)
  let shades ?fill_width ?contour ?(rect = true)
             (x0, y0) (x1, y1) contours data =
    let cont_color, cont_width =
      contour |? (`index 0, 0.0)
    in
    Shades (
      fill_width |? 1.0, cont_color, cont_width, rect, x0, y0, x1, y1,
      data, contours
    )

  (** [text ?dx ?dy ?just ?color s x y] *)
  let text ?(dx = 0.0) ?(dy = 0.0) ?(just = 0.5) color x y s =
    Text (color, s, x, y, dx, dy, just)

  (** [text_outside ?just side displacement s] *)
  let text_outside ?(just = 0.5) ?(perp = false) color side displacement s =
    Text_outside (color, s, side, displacement, just, perp)

  (** [func ?point ?step color f (min, max)] plots the function [f] from
      [x = min] to [x = max].  [step] can be used to tighten or coarsen the
      sampling of plot points. *)
  let func ?symbol ?step color f (min, max) =
    let step =
      match step with
      | None -> (max -. min) /. 100.0
      | Some s -> s
    in
    let xs =
      Array_ext.range ~n:(int_of_float ((max -. min) /. step) + 1) min max
    in
    let ys = Array.map f xs in
    match symbol with
    | Some s -> points ~symbol:s color xs ys
    | None -> lines color xs ys

  (** [transform f] *)
  let transform f = Set_transform f

  (** [clear_transform] *)
  let clear_transform = Clear_transform

  (** [pltr f] *)
  let pltr f = Set_pltr f

  (** [clear_pltr] *)
  let clear_pltr = Clear_pltr

  (** [custom f] *)
  let custom f = Custom f

  (** [list l] *)
  let list l = List l

  (** [maybe x] *)
  let maybe x = Maybe x

  (** Get the font character height in plot world coordinates *)
  let character_height ?stream () =
    with_stream ?stream (
      fun () ->
        let (_, char_height_mm) = plgchr () in
        (* Normalized viewport dims * dimensions in mm = world size in mm *)
        let (_, _, vymin, vymax) = plgvpd () in
        let vy = vymax -. vymin in
        let (_, _, mymin, mymax) = plgspa () in
        let mm_y = mymax -. mymin in
        let world_height_mm = (mm_y *. vy) in
        (* Character height (mm) / World height (mm) = Normalized char height *)
        let char_height_norm = char_height_mm /. world_height_mm in
        (* Normalized character height * World height (world) =
           Character height (world) *)
        let (_, _, wymin, wymax) = plgvpw () in
        let world_y = wymax -. wymin in
        char_height_norm *. world_y
    )

  type legend_entry_t =
    | No_legend (** An empty entry *)
    | Box_legend of (int * int * float * float * int * string)
    | Line_legend of (int * int * float * int * string)
    | Symbol_legend of (int * float * int * string * int * string)

  let opt_of_entry = function
    | No_legend -> PL_LEGEND_NONE
    | Box_legend _ -> PL_LEGEND_COLOR_BOX
    | Line_legend _ -> PL_LEGEND_LINE
    | Symbol_legend _ -> PL_LEGEND_SYMBOL

  type 'a layout_t =
    | Row_major of 'a
    | Column_major of 'a

  let no_legend = No_legend

  let box_legend ?(pattern = 0) ?(scale = 1.0) ?(line_width = 1.0)
                 ?(label_color = `black) ~label color =
    let label_color = int_of_color label_color in
    let color = int_of_color color in
    Box_legend (color, pattern, scale, line_width, label_color, label)

  let line_legend ?(style = 1) ?(width = 1.0) ?(label_color = `black) ~label color =
    let label_color = int_of_color label_color in
    let color = int_of_color color in
    Line_legend (color, style, width, label_color, label)

  let symbol_legend ?(scale = 1.0) ?(number = 3) ?(label_color = `black) ~label
                    color symbol =
    let label_color = int_of_color label_color in
    let color = int_of_color color in
    Symbol_legend (color, scale, number, symbol, label_color, label)

  let row_major x y = Row_major (x, y)

  let column_major x y = Column_major (x, y)

  type position_t =
    | Viewport of side_t option * side_t option * float * float * bool option
    | Subpage of side_t option * side_t option * float * float * bool option

  let viewport_pos ?side1 ?side2 ?inside x y =
    Viewport (side1, side2, x, y, inside)

  let subpage_pos ?side1 ?side2 ?inside x y =
    Subpage (side1, side2, x, y, inside)

  let pos_opt_of_side = function
    | `right -> PL_POSITION_RIGHT
    | `left -> PL_POSITION_LEFT
    | `top -> PL_POSITION_TOP
    | `bottom -> PL_POSITION_BOTTOM

  (** Convert a position_t to a low-level position definition. *)
  let convert_position pos_maybe =
    match pos_maybe with
    | None -> 0.0, 0.0, []
    | Some pos -> begin
      let opt_list side1 side2 inside =
        let side1_opt =
          Option.map_default (fun s -> [pos_opt_of_side s]) [] side1
        in
        let side2_opt =
          Option.map_default (fun s -> [pos_opt_of_side s]) [] side2
        in
        let inside_opt =
          Option.map_default (
            fun b ->
              if b then [PL_POSITION_INSIDE] else [PL_POSITION_OUTSIDE]
          ) [] inside
        in
        List.concat [side1_opt; side2_opt; inside_opt]
      in
      match pos with
      | Viewport (side1, side2, x, y, inside) ->
          (* x-offset, y-offset and position options *)
          x, y, PL_POSITION_VIEWPORT :: opt_list side1 side2 inside
      | Subpage (side1, side2, x, y, inside) ->
          (* x-offset, y-offset and position options *)
          x, y, PL_POSITION_SUBPAGE :: opt_list side1 side2 inside
    end

  (** Convert a backgroun color into a legend/colorbar-ready definition *)
  let convert_bg bg opt =
    match bg with
    | Some color -> int_of_color color, [opt]
    | None -> 0, []

  (** Convert a bounding box definition into a legend/colorbar-ready
      definition *)
  let convert_bb bb opt =
    match bb with
    | Some (color, style) -> int_of_color color, int_of_line_style style, [opt]
    | None -> 0, 0, []

  (** Set the drawing color in [f], and restore the previous drawing color
      when [f] is complete. *)
  let set_color_in ?stream c f =
    with_stream ?stream (
      fun () ->
        let old_color = plg_current_col0 () in
        set_color c;
        f ();
        plcol0 old_color;
        ()
    )

  let legend ?pos ?(plot_width = 0.1) ?bg ?bb ?layout ?color
             ?(text_offset = 1.0) ?(text_scale = 1.0) ?(text_spacing = 2.0)
             ?(text_justification = 1.0)
             ?(text_left = false)
             entries =
    (* Legend position *)
    let legend_x, legend_y, pos_opt = convert_position pos in
    (* Legend background color *)
    let bg, bg_opt = convert_bg bg PL_LEGEND_BACKGROUND in
    (* Legend bounding box *)
    let bb, bb_style, bb_opt = convert_bb bb PL_LEGEND_BOUNDING_BOX in
    (* Legend layout *)
    let rows, columns, layout_opt =
      match layout with
      | Some l ->
          (* Custom layout *)
          begin
            match l with
            | Row_major (x, y) -> x, y, [PL_LEGEND_ROW_MAJOR]
            | Column_major (x, y) -> x, y, []
          end
      | None ->
          (* PLplot's default layout *)
          0, 0, []
    in
    (* Default drawing color *)
    let color = color |? `black in
    (* Text on the left? *)
    let text_left_opt =
      if text_left then [PL_LEGEND_TEXT_LEFT]
      else []
    in
    (* General legend options *)
    let opt =
      List.flatten [
        bg_opt;
        bb_opt;
        layout_opt;
        text_left_opt;
      ]
    in

    (* Get the types of entries in a form PLplot understands *)
    let entries = Array.of_list entries in
    let entry_opts = Array.map (List.map opt_of_entry) entries in

    (* Build the actual entries *)
    let n_entries = Array.length entries in
    (* Text labels *)
    let text_colors = Array.make n_entries 0 in
    let text = Array.make n_entries "" in
    (* Color filled boxes *)
    let box_colors = Array.make n_entries 0 in
    let box_patterns = Array.make n_entries 0 in
    let box_scales = Array.make n_entries 0.0 in
    let box_line_widths = Array.make n_entries 0.0 in
    let line_colors = Array.make n_entries 0 in
    let line_styles = Array.make n_entries 0 in
    let line_widths = Array.make n_entries 0.0 in
    let symbol_colors = Array.make n_entries 0 in
    let symbol_scales = Array.make n_entries 0.0 in
    let symbol_numbers = Array.make n_entries 0 in
    let symbols = Array.make n_entries "" in
    for i = 0 to n_entries - 1 do
      List.iter (
        fun entry ->
          match entry with
          | No_legend ->
              (* Nothing to do here - just leave an empty space *)
              ()
          | Box_legend (color, pattern, scale, line_width, label_color, label) ->
              box_colors.(i) <- color;
              box_patterns.(i) <- pattern;
              box_scales.(i) <- scale;
              box_line_widths.(i) <- line_width;
              text_colors.(i) <- label_color;
              text.(i) <- label;
          | Line_legend (color, style, width, label_color, label) ->
              line_colors.(i) <- color;
              line_styles.(i) <- style;
              line_widths.(i) <- width;
              text_colors.(i) <- label_color;
              text.(i) <- label;
          | Symbol_legend (color, scale, number, symbol, label_color, label) ->
              symbol_colors.(i) <- color;
              symbol_scales.(i) <- scale;
              symbol_numbers.(i) <- number;
              symbols.(i) <- symbol;
              text_colors.(i) <- label_color;
              text.(i) <- label;
      ) entries.(i)
    done;
    custom (
      fun () ->
        set_color_in color (
          fun () ->
            ignore (
              pllegend opt pos_opt legend_x legend_y plot_width bg bb bb_style
                rows columns
                entry_opts
                text_offset text_scale text_spacing text_justification text_colors text
                box_colors box_patterns box_scales box_line_widths
                line_colors line_styles line_widths
                symbol_colors symbol_scales symbol_numbers symbols
            )
        )
    )

  type colorbar_axis_t = {
    axis_def : axis_options_t list;
    axis_values : float array;
  }

  type colorbar_kind_t =
    | Gradient_colorbar of colorbar_axis_t
    | Image_colorbar of colorbar_axis_t
    | Shade_colorbar of (bool * colorbar_axis_t)

  let default_colorbar_axis =
    [`frame0; `frame1; `invert_ticks; `unconventional_label; `major_ticks; `vertical_label]

  let colorbar_axis ?(axis = default_colorbar_axis) vs =
    { axis_def = axis; axis_values = vs }

  let gradient_colorbar ?axis vs =
    Gradient_colorbar (colorbar_axis ?axis vs)
  let image_colorbar ?axis vs =
    Image_colorbar (colorbar_axis ?axis vs)
  let shade_colorbar ?(custom = true) ?axis vs =
    Shade_colorbar (custom, colorbar_axis ?axis vs)

  let colorbar ?pos ?bg ?bb ?cap ?contour ?orient ?axis ?label ?color ?scale kind =
    (* Colorbar position *)
    let colorbar_x, colorbar_y, pos_opt = convert_position pos in
    (* Colorbar background color *)
    let bg, bg_opt = convert_bg bg PL_COLORBAR_BACKGROUND in
    (* Colorbar bounding box *)
    let bb, bb_style, bb_opt = convert_bb bb PL_COLORBAR_BOUNDING_BOX in
    (* End-caps *)
    let cap_low_color, cap_high_color, cap_opt =
      match cap with
      | Some (Some l, Some h) -> l, h, [PL_COLORBAR_CAP_LOW; PL_COLORBAR_CAP_HIGH]
      | Some (Some l, None) -> l, 0.0, [PL_COLORBAR_CAP_LOW]
      | Some (None, Some h) -> 0.0, h, [PL_COLORBAR_CAP_HIGH]
      | Some (None, None)
      | None -> 0.0, 0.0, []
    in
    (* Contours *)
    let cont_color, cont_width =
      match contour with
      | None -> 0, 0.0
      | Some (col, wid) -> int_of_color col, wid
    in
    (* Orientation *)
    let x_length, y_length, orient_opt =
      match orient with
      | Some o -> begin
        (* TODO: Pick something better to use as a type here... *)
        match o with
        | `right (l, w) -> w, l, [PL_COLORBAR_ORIENT_RIGHT]
        | `left (l, w) -> w, l, [PL_COLORBAR_ORIENT_LEFT]
        | `top (l, w) -> l, w, [PL_COLORBAR_ORIENT_TOP]
        | `bottom (l, w) -> l, w, [PL_COLORBAR_ORIENT_BOTTOM]
      end
      | None ->
          let default_length = 0.75 in
          let default_width = 0.05 in
          if
            List.mem PL_POSITION_LEFT pos_opt ||
            List.mem PL_POSITION_RIGHT pos_opt
          then
            (* Tall and narrow (vertical colorbar) *)
            default_width, default_length, []
          else if
            List.mem PL_POSITION_TOP pos_opt ||
            List.mem PL_POSITION_BOTTOM pos_opt
          then
            (* Wide and short (horizontal colorbar) *)
            default_length, default_width, []
          else
            (* Default to vertical *)
            default_width, default_length, []
    in
    (* Extra axis definitions *)
    let extra_axes = axis |? [] in
    (* Labels *)
    let labels, label_opts =
      let result =
        match label with
        | Some l -> begin
          List.map (
            fun p ->
              match p with
              | `right l -> l, [PL_COLORBAR_LABEL_RIGHT]
              | `left l -> l, [PL_COLORBAR_LABEL_LEFT]
              | `top l -> l, [PL_COLORBAR_LABEL_TOP]
              | `bottom l -> l, [PL_COLORBAR_LABEL_BOTTOM]
          ) l
        end
        | None -> ["", []]
      in
      List.split result
    in
    let labels = Array.of_list labels in
    let label_opts = Array.of_list label_opts in
    (* Color for labels, axes, etc. *)
    let color = color |? `black in
    (* Values and colorbar type *)
    let { axis_def = main_axis; axis_values = values}, kind_opt =
      match kind with
      | Gradient_colorbar a -> a, [PL_COLORBAR_GRADIENT]
      | Image_colorbar a -> a, [PL_COLORBAR_IMAGE]
      | Shade_colorbar (custom, a) ->
          a, (
            PL_COLORBAR_SHADE ::
                if custom then [PL_COLORBAR_SHADE_LABEL] else []
          )
    in
    let main_axis_string, tick_spacing, sub_ticks =
      string_ticks_of_axis_options main_axis
    in
    let extra_axis_strings, extra_axis_tick_spacing, extra_axis_sub_ticks, extra_values =
      let axes =
        Array.of_list (
          List.map (
            fun { axis_def; axis_values } ->
              let str, tick_spacing, sub_ticks =
                string_ticks_of_axis_options axis_def
              in
              str, tick_spacing, sub_ticks, axis_values
          ) extra_axes
        )
      in
      Array.to_list (Array.map (fun (x, _, _, _) -> x) axes),
      Array.to_list (Array.map (fun (_, x, _, _) -> x) axes),
      Array.to_list (Array.map (fun (_, _, x, _) -> x) axes),
      Array.to_list (Array.map (fun (_, _, _, x) -> x) axes)
    in
    let axis_strings =
      Array.of_list (main_axis_string :: extra_axis_strings)
    in
    let tick_spacing =
      Array.of_list (tick_spacing :: extra_axis_tick_spacing)
    in
    let sub_ticks =
      Array.of_list (sub_ticks :: extra_axis_sub_ticks)
    in
    let values =
      Array.of_list (values :: extra_values)
    in
    (* Combine all of the options *)
    let opt =
      List.concat [bg_opt; bb_opt; cap_opt; orient_opt; kind_opt]
    in
    (* Call the (wrapped) core plcolorbar function *)
    custom (
      fun () ->
        Option.may (fun s -> plsmaj 0.0 s; plsmin 0.0 s; plschr 0.0 s) scale;
        set_color_in color (
          fun () ->
            ignore (
              plcolorbar opt pos_opt colorbar_x colorbar_y x_length y_length
                bg bb bb_style
                cap_low_color cap_high_color
                cont_color cont_width
                label_opts labels
                axis_strings tick_spacing sub_ticks values
            )
        );
        Option.may (fun _ -> plsmaj 0.0 1.0; plsmin 0.0 1.0; plschr 0.0 1.0) scale;
    )

  (** An easier to deduce alternative to {Plplot.plbox} *)
  let plot_axes ?stream xoptions yoptions =
    let xopt, xtick, xsub = string_ticks_of_axis_options xoptions in
    let yopt, ytick, ysub = string_ticks_of_axis_options yoptions in
    let yopt = yopt ^ "v" in
    with_stream ?stream (fun () -> plbox xopt xtick xsub yopt ytick ysub)

  (** [plot stream l] plots the data in [l] to the plot [stream]. *)
  let rec plot ?stream plottable_list =
    let plot_arc (color, x, y, a, b, angle1, angle2, rotate, fill) =
      set_color_in color (
        fun () -> plarc x y a b angle1 angle2 rotate fill;
      )
    in
    let plot_axes (color, xopt, yopt, width, style, labelfunc) =
      set_color_in color (
        fun () ->
          let old_width = plgwidth () in
          plwidth width;
          set_line_style style;
          Option.may (
            fun f -> plslabelfunc (convert_labelfunc f)
          ) labelfunc;
          plot_axes xopt yopt;
          Option.may (fun _ -> plunset_labelfunc ()) labelfunc;
          set_line_style `solid;
          plwidth old_width;
      )
    in
    let plot_contours (color, pltr, contours, data) =
      set_color_in color (
        fun () ->
          plset_pltr pltr;
          let ixmax, iymax = Array_ext.matrix_dims data in
          plcont data 1 ixmax 1 iymax contours;
          plunset_pltr ();
      )
    in
    let plot_image (range, x0, y0, x1, y1, image) =
      let range_min, range_max = range |? (0.0, 0.0) in
      plimage image
        x0 x1 y0 y1 range_min range_max x0 x1 y0 y1;
      ()
    in
    let plot_imagefr (range, x0, y0, x1, y1, image) scale =
      let range_min, range_max = range |? (0.0, 0.0) in
      let scale_min, scale_max = scale in
      plimagefr image
        x0 x1 y0 y1 range_min range_max scale_min scale_max;
      ()
    in
    let plot_join (color, x0, y0, x1, y1, width, style) =
      set_color_in color (
        fun () ->
          let old_width = plgwidth () in
          plwidth width;
          set_line_style style;
          pljoin x0 y0 x1 y1;
          set_line_style `solid;
          plwidth old_width;
      )
    in
    let plot_labels (color, x, y, title) =
      set_color_in color (
        fun () -> pllab x y title
      )
    in
    let plot_lines (label, color, xs, ys, width, style) =
      set_color_in color (
        fun () ->
          let old_width = plgwidth () in
          plwidth width;
          set_line_style style;
          plline xs ys;
          set_line_style `solid;
          plwidth old_width;
      )
    in
    let plot_map (color, outline, x0, y0, x1, y1) =
      set_color_in color (
        fun () ->
          plmap (string_of_map_t outline) x0 x1 y0 y1;
      )
    in
    let plot_points (label, color, xs, ys, symbol, scale) =
      set_color_in color (
        fun () ->
          plschr 0.0 scale;
          plstring xs ys symbol;
          plschr 0.0 1.0;
      )
    in
    let plot_polygon (color, xs, ys, fill) =
      let x_len = Array.length xs in
      let y_len = Array.length ys in
      verify_arg (x_len = y_len)
        "plot_polygon: must provide same number of X and Y coordinates";
      set_color_in color (
        fun () ->
          if fill then (
            plfill xs ys;
          )
          else (
            (* Make sure the polygon is closed *)
            let xs' =
              Array.init (x_len + 1)
                (fun i -> if i < x_len - 1 then xs.(i) else xs.(0))
            in
            let ys' =
              Array.init (y_len + 1)
                (fun i -> if i < y_len - 1 then ys.(i) else ys.(0))
            in
            plline xs' ys';
          )
      )
    in
    (*
    let plot_shade
      (x0, y0, x1, y1,
       shade_min, shade_max, shade_color, shade_width,
       min_color, min_width,
       max_color, max_width,
       rect,
       data)
    =
      plshade data x0 x1 y0 y1 shade_min shade_max cmap color width
        min_color min_width max_color max_width rect
    in
    *)
    let plot_shades
      (fill_width, cont_color, cont_width, rect, x0, y0, x1, y1, data, contours)
    =
      let cont_color = int_of_color cont_color in
      plshades data x0 x1 y0 y1 contours fill_width cont_color cont_width rect
    in
    let plot_text (color, s, x, y, dx, dy, just) =
      set_color_in color (fun () -> plptex x y dx dy just s)
    in
    let plot_text_outside (color, s, side, displacement, just, perp) =
      let side_string, position =
        match side with
        | `right p -> "r", p
        | `left p -> "l", p
        | `top p -> "t", p
        | `bottom p -> "b", p
      in
      let side_string = side_string ^ if perp then "v" else "" in
      set_color_in color
        (fun () -> plmtex side_string displacement position just s)
    in

    let rec one_plot p =
      match p with
      | Arc a -> plot_arc a
      | Axes ax -> plot_axes ax
      | Contours c -> plot_contours c
      | Image i -> plot_image i
      | Image_fr (i, scale) -> plot_imagefr i scale
      | Join j -> plot_join j
      | Labels lab -> plot_labels lab
      | Lines l -> plot_lines l
      | Map m -> plot_map m
      | Points p -> plot_points p
      | Polygon poly -> plot_polygon poly
      | Shades s -> plot_shades s
      | Text t -> plot_text t
      | Text_outside t_o -> plot_text_outside t_o
      | Set_transform transform -> plstransform transform
      | Clear_transform -> plunset_transform ()
      | Set_pltr pltr -> plset_pltr pltr
      | Clear_pltr -> plunset_pltr ()
      | Custom f -> f ()
      | List l -> plot l
      | Maybe None -> () (* no-op *)
      | Maybe (Some m_p) -> one_plot m_p
    in
    List.iter (
      fun plottable -> with_stream ?stream (fun () -> one_plot plottable)
    ) plottable_list

  (** Finish the current page, start a new one (alias for {!start_page}). *)
  let next_page = start_page

  (** End the current plot stream *)
  let end_stream ?stream () =
    with_stream ?stream plend1

  (** Finish up the plot by plotting axes and ending the stream. *)
  let finish ?stream () =
    plot ?stream [default_axes];
    end_stream ?stream ()
end

(** The [Quick_plot] module is intended to be used for quick, "throw-away"
    plots.  It's is likely to be most useful from the toplevel. *)
module Quick_plot = struct
  open Plot

  let special_float x =
    match classify_float x with
    | FP_normal
    | FP_subnormal
    | FP_zero -> false
    | FP_infinite
    | FP_nan -> true

  let safe_array_reduce f a =
    let n =
      Array_ext.reduce (
        fun accu x ->
          if special_float x then
            accu
          else if special_float accu then
            x
          else
            f accu x
      ) a
    in
    verify_arg (not (special_float n)) "No non-special values";
    n

  let extents xs_list ys_list =
    List.fold_left min infinity (List.map (safe_array_reduce min) xs_list),
    List.fold_left max neg_infinity (List.map (safe_array_reduce max) xs_list),
    List.fold_left min infinity (List.map (safe_array_reduce min) ys_list),
    List.fold_left max neg_infinity (List.map (safe_array_reduce max) ys_list)

  let maybe_log log =
    let x_axis = default_axis_options in
    let y_axis = default_axis_options in
    Option.map_default (
      fun (x_log, y_log) ->
        (if x_log then `log :: x_axis else x_axis),
        (if y_log then `log :: y_axis else y_axis)
    ) (x_axis, y_axis) log

  let maybe_line_legend names colors =
    maybe (
      Option.map (
        fun names' ->
          let entries =
            List.map2 (
              fun color label -> [line_legend ~label color]
            ) (Array.to_list colors) names'
          in
          legend entries
      ) names
    )

  let maybe_symbol_legend names colors symbols =
    maybe (
      Option.map (
        fun names' ->
          let entries =
            Array.mapi (
              fun i label ->
                [symbol_legend ~label colors.(i) symbols.(i)]
            ) (Array.of_list names')
          in
          legend (Array.to_list entries)
      ) names
    )

  (** [points [xs, ys; ...] plots the points described by the coordinates [xs]
      and [ys]. *)
  let points ?filename ?size ?(device = `window `cairo) ?labels ?names ?log xs_ys_list =
    let xs_list, ys_list = List.split xs_ys_list in
    let xmin, xmax, ymin, ymax = extents xs_list ys_list in
    let ys_array = Array.of_list ys_list in
    let stream =
      init ?filename ?size (xmin, ymin) (xmax, ymax) `greedy device
    in
    let colors = Array.mapi (fun i _ -> `index (i + 1)) ys_array in
    let symbols =
      Array.init (Array.length ys_array) (fun i -> sprintf "#(%03d)" (i + 135))
    in
    let plottable_points =
      Array.to_list (
        Array.mapi (
          fun i xs ->
            points ~symbol:symbols.(i) colors.(i)
              xs ys_array.(i)
        ) (Array.of_list xs_list)
      )
    in
    let x_axis, y_axis = maybe_log log in
    plot ~stream [
      list plottable_points;
      (* TODO: Make the legend work *)
      maybe_symbol_legend names colors symbols;
      axes x_axis y_axis;
      maybe (Option.map (fun (x, y, t) -> label x y t) labels);
    ];
    end_stream ~stream ();
    ()

  (** [lines [xs, ys; ...] plots the line segments described by the coordinates
      [xs] and [ys]. *)
  let lines
        ?filename ?size ?(device = `window `cairo) ?labels ?names ?log
        xs_ys_list =
    let xs_list, ys_list = List.split xs_ys_list in
    let xmin, xmax, ymin, ymax = extents xs_list ys_list in
    let ys_array = Array.of_list ys_list in
    let stream = init ?filename ?size (xmin, ymin) (xmax, ymax) `greedy device in
    let colors = Array.mapi (fun i _ -> `index (i + 1)) ys_array in
    let plottable_lines =
      Array.to_list (
        Array.mapi (
          fun i xs -> lines colors.(i) xs ys_array.(i)
        ) (Array.of_list xs_list)
      )
    in
    let x_axis, y_axis = maybe_log log in
    plot ~stream [
      list plottable_lines;
      maybe_line_legend names colors;
      axes x_axis y_axis;
      maybe (Option.map (fun (x, y, t) -> label x y t) labels);
    ];
    end_stream ~stream ();
    ()

  (** [image ?log m] plots the image [m] with a matching colorbar.  If [log] is
      true then the data in [m] are assumed to be log10(x) values. *)
  let image ?filename ?size ?(device = `window `cairo) ?labels ?log ?palette m =
    let m_max, m_min = plMinMax2dGrid m in
    let xmin, ymin = 0.0, 0.0 in
    let xmax, ymax = Array_ext.matrix_dims m in
    let xmax, ymax = float_of_int xmax, float_of_int ymax in
    let axis =
      Option.map_default (
        fun l ->
          if l then
            `log :: default_colorbar_axis
          else
            default_colorbar_axis
      ) default_colorbar_axis log
    in
    let stream =
      init ?filename ?size (xmin, ymin) (xmax, ymax) `equal_square device
    in
    Option.may (load_palette ~stream) palette;
    plot ~stream [
      image (xmin, ymin) (xmax, ymax) m;
      default_axes;
      maybe (Option.map (fun (x, y, t) -> label x y t) labels);
      colorbar ~scale:0.75 ~pos:(viewport_pos 0.05 0.0)
        (image_colorbar ~axis [|m_min; m_max|]);
    ];
    end_stream ~stream ();
    ()

  (** [func ?point ?step fs (min, max)] plots the functions [fs] from [x = min]
      to [x = max].  [step] can be used to tighten or coarsen the sampling of
      plot points. *)
  let func
        ?filename ?size ?(device = `window `cairo) ?labels ?names ?symbol ?step
        fs (xmin, xmax) =
    let fs_array = Array.of_list fs in
    let colors = Array.mapi (fun i _ -> `index (i + 1)) fs_array in
    let plot_content =
      Array.to_list (
        Array.mapi (
          fun i f ->
            func ?symbol ?step colors.(i) f (xmin, xmax)
        ) fs_array
      )
    in
    let ys =
      Array.of_list (
        List.map (
          function
            | Lines (_, _, _, y, _, _)
            | Points (_, _, _, y, _, _) -> y
            | _ -> invalid_arg "Invalid function output"
        ) plot_content
      )
    in
    let ymax, ymin = plMinMax2dGrid ys in
    let stream =
      init ?filename ?size (xmin, ymin) (xmax, ymax) `greedy device
    in
    plot ~stream [
      list plot_content;
      maybe_line_legend names colors;
      default_axes;
      maybe (Option.map (fun (x, y, t) -> label x y t) labels);
    ];
    end_stream ~stream ();
    ()

  let shades ?filename ?size ?(device = `window `cairo) ?labels ?log ?palette
        ?contours m =
    let xmin, ymin = 0.0, 0.0 in
    let xmax, ymax = Array_ext.matrix_dims m in
    let xmax, ymax = float_of_int xmax, float_of_int ymax in
    let axis =
      Option.map_default (
        fun l ->
          if l then
            `log :: default_colorbar_axis
          else
            default_colorbar_axis
      ) default_colorbar_axis log
    in
    let stream =
      init ?filename ?size (xmin, ymin) (xmax, ymax) `equal_square device
    in
    Option.may (load_palette ~stream) palette;
    let contours =
      contours |? (
        let m_max, m_min = plMinMax2dGrid m in
        Array_ext.range ~n:11 m_min m_max
      )
    in
    plot ~stream [
      shades (xmin, ymin) (xmax, ymax) contours m;
      default_axes;
      maybe (Option.map (fun (x, y, t) -> label x y t) labels);
      colorbar ~scale:0.75 ~pos:(viewport_pos 0.05 0.0)
        (shade_colorbar ~axis contours);
    ];
    end_stream ~stream ();
    ()
end

