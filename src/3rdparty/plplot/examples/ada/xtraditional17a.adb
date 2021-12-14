-- Plots a simple stripchart with four pens.

-- Copyright (C) 2008 Jerry Bauck

-- This file is part of PLplot.

-- PLplot is free software; you can redistribute it and/or modify
-- it under the terms of the GNU Library General Public License as published
-- by the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.

-- PLplot is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Library General Public License for more details.

-- You should have received a copy of the GNU Library General Public License
-- along with PLplot; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

with
    Ada.Text_IO,
    Ada.Strings.Unbounded,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    Ada.Text_IO,
    Ada.Strings.Unbounded,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

procedure xtraditional17a is
    autoy, acc : Boolean;
    pl_errcode : Integer := 0;
    id1 : Integer;
    nsteps : Integer := 1000;
    colbox, collab : Integer;
    colline, styline : Integer_Array_1D(0 .. 3);
    y1, y2, y3, y4, ymin, ymax, xlab, ylab : Long_Float;
    t, tmin, tmax, tjump, dt, noise : Long_Float;
    legline : Stripchart_Label_String_Array_Type :=
       (To_Unbounded_String("sum"),
        To_Unbounded_String("sin"),
        To_Unbounded_String("sin*noi"),
        To_Unbounded_String("sin+noi"));
begin
    -- plplot initialization
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- If db is used the plot is much more smooth. However, because of the
    -- async X behaviour, one does not have a real-time scripcharter.
    -- plsetopt("db", "");
    -- plsetopt("np", "");

    -- User sets up plot completely except for window and data
    -- Eventually settings in place when strip chart is created will be
    -- remembered so that multiple strip charts can be used simultaneously.

    -- Specify some reasonable defaults for ymin and ymax
    -- The plot will grow automatically if needed (but not shrink)
    ymin := -0.1;
    ymax :=  0.1;

    -- Specify initial tmin and tmax -- this determines length of window.
    -- Also specify maximum jump in t
    -- This can accomodate adaptive timesteps
    tmin := 0.0;
    tmax := 10.0;
    tjump := 0.3; -- percentage of plot to jump

    -- Axes options same as plbox.
    -- Only automatic tick generation and label placement allowed.
    -- Eventually I'll make this fancier.
    colbox := 1;
    collab := 3;

    styline(0) := 2; -- pens line style
    styline(1) := 3;
    styline(2) := 4;
    styline(3) := 5;

    colline(0) := 2; -- pens color
    colline(1) := 3;
    colline(2) := 4;
    colline(3) := 5;

    xlab := 0.0; -- legend position
    ylab := 0.25;

    autoy := True; -- autoscale y
    acc := True;   -- don't scrip, accumulate

    -- Initialize plplot
    plinit;

    pladv(0);
    plvsta;

    -- Register our error variables with PLplot
    -- From here on, we're handling all errors here
    --    plsError(pl_errcode, errmsg);

    -- Ada note: plsError is not yet implemented in Ada. Thus, pl_errcode is
    -- hardwired to 0 for now and no effective error handling is done.

    plstripc(id1, "bcnst", "bcnstv",
	     tmin, tmax, tjump, ymin, ymax,
	     xlab, ylab,
	     autoy, acc,
	     colbox, collab,
	     colline, styline, legline,
	     "t", "", "Strip chart demo");

    -- Ada note: this would handle errors if error handling were implemented.
    if pl_errcode /= 0 then
        Put_Line("There was a problem which caused the program to quit.");
        plend;
        return;
    end if;

    -- Let plplot handle errors from here on
    --    plsError(NULL, NULL);

    autoy := False;	-- autoscale y
    acc := True;	-- accumulate

    -- This is to represent a loop over time
    -- Let's try a random walk process
    y1 := 0.0;
    y2 := 0.0;
    y3 := 0.0;
    y4 := 0.0;

    dt := 0.1;

    for n in 0 .. nsteps - 1 loop
        -- The Ada standard requires that the delay statement have a granularity
        -- of at most 20 ms but strongly recommends at most 100 microseconds.
        -- We'll try 10 ms to match the C example, x17c, but in theory this
        -- might fail to compile on some Ada systems. If so, the cure is to set
        -- the delay to 0.02.
        delay 0.01; -- wait a little (10 ms) to simulate time elapsing
        t := Long_Float(n) * dt;
        noise := plrandd - 0.5;
        y1 := y1 + noise;
        y2 := sin(t * pi / 18.0);
        y3 := y2 * noise;
        y4 := y2 + noise / 3.0;

        -- There is no need for all pens to have the same number of
        -- points or being equally time spaced.
        if n mod 2 /= 0 then
            plstripa(id1, 0, t, y1);
        end if;

        if n mod 3 /= 0 then
            plstripa(id1, 1, t, y2);
        end if;

        if n mod 4 /= 0 then
            plstripa(id1, 2, t, y3);
        end if;

        if n mod 5 /= 0 then
            plstripa(id1, 3, t, y4);
        end if;
        -- pleop;  -- use double buffer (-db on command line)
    end loop;

    -- Destroy strip chart and its memory
    plstripd(id1);
    plend;
end xtraditional17a;
