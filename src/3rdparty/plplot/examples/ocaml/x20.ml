(*
  plimage demo

*)

open Plplot

let pi = atan 1.0 *. 4.0

let xdim = 260
let ydim = 220

(* Change this to 'Some "outfile.psc"' if you want to test the plot saving
   option. *)
let filename = None

type stretch_data = {
  xmin : float;
  xmax : float;
  ymin : float;
  ymax : float;
  stretch : float;
}

(* Transformation function *)
let mypltr pltr_data x y =
  let x0 = (pltr_data.xmin +. pltr_data.xmax) *. 0.5 in
  let y0 = (pltr_data.ymin +. pltr_data.ymax) *. 0.5 in
  let dy = (pltr_data.ymax -. pltr_data.ymin) *. 0.5 in
  x0 +. (x0 -. x) *.
    (1.0 -. pltr_data.stretch *. cos ((y -. y0) /. dy *. pi *. 0.5)),
  y

(* read image from file in binary ppm format *)
let read_img fname =
  (* naive grayscale binary ppm reading. If you know how to, improve it *)
  let fp = open_in fname in

  let ver = input_line fp in (* version *)

  if ver <> "P5" then
    raise (Invalid_argument ("Unable to read image: " ^ fname))
  else
    ();

  (* This will skip comments. *)
  let rec ignore_comments_then_get_w_h () =
    let maybe_comment = input_line fp in
    if maybe_comment.[0] = '#' then
      ignore_comments_then_get_w_h ()
    else
      (* This line has the width and height in it *)
      maybe_comment
  in

  (* width, height num colors *)
  let w_h_line = ignore_comments_then_get_w_h () in
  let num_col_line = input_line fp in
  let w, h = Scanf.sscanf w_h_line "%d %d" (fun w h -> w, h) in
  let num_col = Scanf.sscanf num_col_line "%d" (fun n -> n) in

  let img = String.make (w * h) ' ' in
  let imf = Array.make_matrix w h 0.0 in

  (* Note that under 32bit OCaml, this will only work when reading strings up
     to ~16 megabytes. *)
  really_input fp img 0 (w * h);

  close_in fp;

  for i = 0 to w - 1 do
    for j = 0 to h - 1 do
      imf.(i).(j) <-
        (* flip image up-down *)
        float_of_int (int_of_char (img.[(h - 1 - j ) * w + i]));
    done
  done;
  imf, w, h, num_col

(* save plot *)
let save_plot fname =
  let cur_strm = plgstrm () in (* get current stream *)
  let _ = plmkstrm () in  (* create a new one *)

  plsdev "psc"; (* new device type. Use a known existing driver *)
  plsfnam fname; (* file name *)

  plcpstrm cur_strm false; (* copy old stream parameters to new stream *)
  plreplot (); (* do the save *)
  plend1 (); (* close new device *)

  plsstrm cur_strm; (* and return to previous one *)
  ()

(* set gray colormap *)
let gray_cmap num_col =
  let r = [|0.0; 1.0|] in
  let g = [|0.0; 1.0|] in
  let b = [|0.0; 1.0|] in
  let pos = [|0.0; 1.0|] in
  plscmap1n num_col;
  plscmap1l true pos r g b None;
  ()

let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  let z = Array.make_matrix xdim ydim 0.0 in
  let r = Array.make_matrix xdim ydim 0.0 in

  (* sombrero-like demo *)
  plcol0 2; (* draw a yellow plot box, useful for diagnostics *)
  plenv 0.0 (2.0 *. pi) 0.0 (3.0 *. pi) 1 (-1);

  let x =
    Array.init xdim (
      fun i -> float_of_int i *. 2.0 *. pi /. float_of_int (xdim - 1)
    )
  in
  let y =
    Array.init ydim (
      fun i -> float_of_int i *. 3.0 *. pi /. float_of_int (ydim - 1)
    )
  in

  for i = 0 to xdim - 1 do
    for j = 0 to ydim - 1 do
      r.(i).(j) <- sqrt (x.(i) *. x.(i) +. y.(j) *. y.(j)) +. 1e-3;
      z.(i).(j) <- sin r.(i).(j) /. r.(i).(j);
    done
  done;

  pllab "No, an amplitude clipped \"sombrero\"" "" "Saturn?";
  plptex 2.0 2.0 3.0 4.0 0.0 "Transparent image";
  plimage
    z 0.0 (2.0 *. pi) 0.0 (3.0 *. pi) 0.05 1.0 0.0 (2.0 *. pi) 0.0 (3.0 *. pi);

  (* save the plot *)
  let () =
    match filename with
        Some f -> save_plot f
      | None -> ()
  in

  (* read Chloe image *)
  (* Note we try two different locations to cover the case where this
   * examples is being run from the test_ocaml.sh script *)
  let img_f, width, height, num_col =
    try
      read_img "Chloe.pgm"
    with
      _ -> (
        try
          read_img "../Chloe.pgm"
        with
          Invalid_argument _ ->
            plend ();
            raise (Failure "No such file")
      )
  in

  let width = float_of_int width in
  let height = float_of_int height in

  (* set gray colormap *)
  gray_cmap num_col;

  (* display Chloe *)
  plenv 1.0 width 1.0 height 1 (-1);

  pllab "Set and drag Button 1 to (re)set selection, Button 2 to finish." " " "Chloe...";

  plimage img_f 1.0 width 1.0 height 0.0 0.0 1.0 width 1.0 height;

  (* selection/expansion demo *)
  let xi = 25.0 in
  let xe = 130.0 in
  let yi = 235.0 in
  let ye = 125.0 in

  plspause false;
  pladv 0;

  (* display selection only *)
  plimage img_f 1.0 width 1.0 height 0.0 0.0 xi xe ye yi;

  plspause true;

  (* zoom in selection *)
  plenv xi xe ye yi 1 (-1);
  plimage img_f 1.0 width 1.0 height 0.0 0.0 xi xe ye yi;

  (* Base the dynamic range on the image contents. *)
  let img_max, img_min = plMinMax2dGrid img_f in

  (* Draw a saturated version of the original image.  Only use the middle 50%
     of the image's full dynamic range. *)
  plcol0 2;
  plenv 0.0 width 0.0 height 1 (-1);
  pllab "" "" "Reduced dynamic range image example";
  plimagefr
    img_f 0.0 width 0.0 height 0.0 0.0
    (img_min +. img_max *. 0.25) (img_max -. img_max *. 0.25);

  (* Draw a distorted version of the original image, showing its full dynamic
     range. *)
  plenv 0.0 width 0.0 height 1 (-1);
  pllab "" "" "Distorted image example";

  let stretch =
    {
      xmin = 0.0;
      xmax = width;
      ymin = 0.0;
      ymax = height;
      stretch = 0.5;
    }
  in

  (* OCaml supports arbitrary coordinate transform callbacks, so take advantage
     of that to use mypltr directly and skip the use of pltr2. *)
  plset_pltr (mypltr stretch);
  (* Comment the above line, and uncomment the following section to use pltr2 *)
  (*
  let xg = Array.make_matrix (int_of_float width + 1) (int_of_float height + 1) 0.0 in
  let yg = Array.make_matrix (int_of_float width + 1) (int_of_float height + 1) 0.0 in
  for i = 0 to int_of_float width do
    for j = 0 to int_of_float height do
      let xx, yy = mypltr (float_of_int i) (float_of_int j) stretch in
      xg.(i).(j) <- xx;
      yg.(i).(j) <- yy;
    done
  done;
  plset_pltr (pltr2 xg yg);
  *)
  plimagefr img_f 0.0 width 0.0 height 0.0 0.0 img_min img_max;

  plend ();
  ()

