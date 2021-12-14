(*
  Copyright (C) 2009  Hezekiah M. Carty

  Plots a simple stripchart with four pens.
  Direct adaptation of the C version.

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

let pi = atan 1.0 *. 4.0

(** [sleep time] sleeps for [time] seconds. *)
let sleep time =
  ignore (Unix.select [] [] [] time)

let () =
  let nsteps = 1000 in

  (* plplot initialization *)
  (* Parse and process command line arguments *)

  plparseopts Sys.argv [PL_PARSE_FULL];

  (* If db is used the plot is much more smooth. However, because of the
     async X behaviour, one does not have a real-time scripcharter. *)
  (*
  plsetopt "db" "";
  plsetopt "np" "";
  *)

  (* User sets up plot completely except for window and data
     Eventually settings in place when strip chart is created will be
     remembered so that multiple strip charts can be used simultaneously. *)

  (* Specify some reasonable defaults for ymin and ymax *)
  (* The plot will grow automatically if needed (but not shrink) *)

  let ymin = -0.1 in
  let ymax = 0.1 in

  (* Specify initial tmin and tmax -- this determines length of window. *)
  (* Also specify maximum jump in t *)
  (* This can accomodate adaptive timesteps *)

  let tmin = 0.0 in
  let tmax = 10.0 in
  let tjump = 0.3 in (* percentage of plot to jump *)

  (* Axes options same as plbox. *)
  (* Only automatic tick generation and label placement allowed *)
  (* Eventually I'll make this fancier *)

  let colbox = 1 in
  let collab = 3 in
  (* pens' color and line style *)
  let styline = [|2; 3; 4; 5|] in
  let colline = [|2; 3; 4; 5|] in
  (* pens legend *)
  let legline = [|"sum"; "sin"; "sin*noi"; "sin+noi"|] in

  (* legend position *)
  let xlab = 0.0 in
  let ylab = 0.25 in

  let autoy = true in (* autoscale y *)
  let acc = true in (* don't scrip, accumulate *)

  (* Initialize plplot *)
  plinit ();

  pladv 0;
  plvsta ();

  (* Register our error variables with PLplot *)
  (* From here on, we're handling all errors here *)
  (* TODO : Eventually wrap plsError
  plsError pl_errcode errmsg;
  *)

  let id1 =
    plstripc
      "bcnst" "bcnstv"
      tmin tmax tjump ymin ymax
      xlab ylab
      autoy acc
      colbox collab
      colline styline legline
      "t" "" "Strip chart demo"
  in

  (* TODO : This should either be removed or plsError should be
     properly wrapped.
  if pl_errcode then (
    eprintf "%s\n" errmsg;
    exit 1;
  );
  *)

  (* Let plplot handle errors from here on *)
  (* plsError(NULL, NULL); *)

  (* XXX : These seem unused?
  let autoy = 0 in (* autoscale y *)
  let acc = 1 in (* accumulate *)
  *)

  (* This is to represent a loop over time *)
  (* Let's try a random walk process *)

  let y1 = ref 0.0 in
  let y2 = ref 0.0 in
  let y3 = ref 0.0 in
  let y4 = ref 0.0 in
  let dt = 0.1 in

  for n = 0 to nsteps - 1 do
    (* Wait a little (10 ms) to simulate time elapsing *)
    sleep 1e-2;
    let t = float_of_int n *. dt in
    let noise = plrandd () -. 0.5 in
    y1 := !y1 +. noise;
    y2 := sin (t *. pi /. 18.0);
    y3 := !y2 *. noise;
    y4 := !y2 +. noise /. 3.0;

    (* There is no need for all pens to have the same number of
       points or beeing equally time spaced. *)
    if n mod 2 <> 0 then
      plstripa id1 0 t !y1;
    if n mod 3 <> 0 then
      plstripa id1 1 t !y2;
    if n mod 4 <> 0 then
      plstripa id1 2 t !y3;
    if n mod 5 <> 0 then
      plstripa id1 3 t !y4;
    (*pleop (); use double buffer (-db on command line) *)
  done;

  (* Destroy strip chart and it's memory *)
  plstripd id1;
  plend ();
  ()

