(*
  Pie chart demo.
*)

open Plplot

let pi = atan 1.0 *. 4.0

let text = [|
  "Maurice";
  "Geoffrey";
  "Alan";
  "Rafael";
  "Vince";
|]

(*--------------------------------------------------------------------------*\
 * Does a simple pie chart.
\*--------------------------------------------------------------------------*)

let () =
  let x = Array.make 500 0.0 in
  let y = Array.make 500 0.0 in
  let per = [|10.0; 32.0; 12.0; 30.0; 16.0|] in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  pladv 0;
  (* Ensure window has aspect ratio of one so circle is
   * plotted as a circle. *)
  plvasp 1.0;
  plwind 0. 10. 0. 10.;
  (* plenv 0.0 10.0 0.0 10.0 1 (-2); *)
  plcol0 2;
  (* All theta quantities scaled by 2*M_PI/500 to be integers to avoid
     floating point logic problems. *)
  let theta0 = ref 0 in
  for i = 0 to 4 do
    let j = ref 0 in
    x.(!j) <- 5.0;
    y.(!j) <- 5.0;
    incr j;
    (* Theta quantities multiplied by 2*M_PI/500 afterward so
       in fact per is interpreted as a percentage. *)
    let theta1 =
      if i = 4 then
        500
      else
        int_of_float (float_of_int !theta0 +. 5.0 *. per.(i))
    in
    for theta = !theta0 to theta1 do
      x.(!j) <- 5.0 +. 3.0 *. cos ((2.0 *. pi /. 500.0) *. float_of_int theta);
      y.(!j) <- 5.0 +. 3.0 *. sin ((2.0 *. pi /. 500.0) *. float_of_int theta);
      incr j;
    done;
    let x' = Array.sub x 0 !j in
    let y' = Array.sub y 0 !j in
    plcol0 (i + 1);
    plpsty ((i + 3) mod 8 + 1);
    plfill x' y';
    plcol0 1;
    plline x' y';
    let just = (2.0 *. pi /. 500.0) *. float_of_int (!theta0 + theta1) /. 2.0 in
    let dx = 0.25 *. cos just in
    let dy = 0.25 *. sin just in
    let just =
      if (!theta0 + theta1 < 250 || !theta0 + theta1 > 750) then
        0.0
      else
        1.0
    in
    plptex (x.(!j / 2) +. dx) (y.(!j / 2) +. dy) 1.0 0.0 just text.(i);
    theta0 := theta1;
  done;
  plfont 2;
  plschr 0.0 1.3;
  plptex 5.0 9.0 1.0 0.0 0.5 "Percentage of Sales";

  (* Don't forget to call PLEND to finish off! *)
  plend ();
  ()
