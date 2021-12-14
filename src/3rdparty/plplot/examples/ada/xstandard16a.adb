--  Shade_Region demo, using color fill.

--  Maurice LeBrun
--  IFS, University of Texas at Austin
--  20 Mar 1994

-- Copyright (C) 2008-2013 Jerry Bauck

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
    Ada.Numerics,
    System,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard16a is
    -- Fundamental settings. See notes() for more info.
    ns : Integer := 20;		    -- Default number of shade levels
    nx : Integer := 35;		    -- Default number of data points in x
    ny : Integer := 46;		    -- Default number of data points in y
    exclude : Boolean := False; -- By default do not plot a page illustrating
                                -- exclusion.  API is probably going to change
                                -- anyway, and cannot be reproduced by any
                                -- front end other than the C one.

    x, y, argx, argy, distort, r, t : Long_Float;
    px, py : Real_Vector(0 .. 99);
    zmin, zmax : Long_Float;
    z, w : Real_Matrix(0 .. nx - 1, 0 .. ny - 1);
    clevel : Real_Vector(0 .. ns - 1);
    shedge : Real_Vector(0 .. ns);

    cgrid1 : aliased Transformation_Data_Type
       (x_Last => nx - 1,
        y_Last => ny - 1,
        z_Last => 0);

    cgrid2 : aliased Transformation_Data_Type_2
       (x_Last => nx - 1,
        y_Last => ny - 1);

    fill_width : Long_Float := 2.0;
    colorbar_width, colorbar_height : Long_Float;
    cont_color : Integer    := 0;
    cont_width : Long_Float := 0.0;
    num_axes : constant Integer := 1;
    n_axis_opts : constant Integer := num_axes;
    -- TUB is renamed To_Unbounded_String.
    axis_opts : Legend_String_Array_Type(0 .. n_axis_opts - 1) := (others => TUB("bcvtm"));
    num_values : Integer_Array_1D(0 .. num_axes - 1);
    values : Real_Matrix(0 .. num_axes - 1, 0 .. ns); --SWAP THESE???
    axis_ticks : Real_Vector(0 .. num_axes - 1) := (others => 0.0);
    axis_subticks : Integer_Array_1D(0 .. num_axes - 1) := (others => 0);
    num_labels : constant Integer := 1;
    label_opts : Integer_Array_1D(0 .. num_labels - 1);
    labels : Legend_String_Array_Type(0 .. num_labels - 1) := (others => TUB("Magnitude"));

    -- Transformation function
    tr : Real_Vector(0 .. 5);

    procedure mypltr -- This spec is necessary to accommodate pragma Convention().
       (x, y : Long_Float;
        tx, ty : out Long_Float;
        pltr_data : PL_Pointer);
    pragma Convention(Convention => C, Entity => mypltr);


    procedure mypltr
       (x, y : Long_Float;
        tx, ty : out Long_Float;
        pltr_data : PL_Pointer) is
    begin
        tx := tr(0) * x + tr(1) * y + tr(2);
        ty := tr(3) * x + tr(4) * y + tr(5);
    end mypltr;


    -- Masking function
    function zdefined(x, y : Long_Float) return Integer;
    pragma Convention(C, zdefined);

    function zdefined(x, y : Long_Float) return Integer is
        z : Long_Float := sqrt(x * x + y * y);
    begin
        if z < 0.4 or z > 0.6 then
            return 1; -- Don't mask.
        else
            return 0; -- Do mask.
        end if;
    end zdefined;

begin
    label_opts(0) := Colorbar_Label_Bottom;

    ----------------------------------------------------------------------------
    -- Does several shade plots using different coordinate mappings.
    ----------------------------------------------------------------------------

    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    -- Load colour palettes
    Set_Color_Map_0_From_File("cmap0_black_on_white.pal");
    Set_Color_Map_1_From_File("cmap1_gray.pal", True);

    -- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    Set_Number_Of_Colors_Map_0(3);

    -- Initialize plplot
    Initialize_PLplot;

    -- Set up transformation function
    tr(0) :=  2.0 / Long_Float(nx - 1);
    tr(1) :=  0.0;
    tr(2) := -1.0;
    tr(3) :=  0.0;
    tr(4) :=  2.0 / Long_Float(ny - 1);
    tr(5) := -1.0;

    -- Set up data array
    for i in z'range(1) loop
        x := Long_Float(i - nx / 2) / Long_Float(nx / 2);
        for j in z'range(2) loop
            y := Long_Float(j - ny / 2) / Long_Float(ny / 2) - 1.0;
            z(i, j) := -sin(7.0 * x) * cos(7.0 * y) + x * x - y * y;
            w(i, j) := -cos(7.0 * x) * sin(7.0 * y) + 2.0 * x * y;
        end loop;
    end loop;
    zmin := Matrix_Min(z);
    zmax := Matrix_Max(z);
    for i in clevel'range loop
        clevel(i) := zmin + (zmax - zmin) * (Long_Float(i) + 0.5) / Long_Float(ns);
    end loop;

    for i in shedge'range loop
        shedge(i) := zmin + (zmax - zmin) * Long_Float(i) / Long_Float(ns);
    end loop;

    -- Set up coordinate grids
    cgrid1.zg(0) := 0.0; -- Not used, but initialize anway.
    for i in cgrid1.xg'range loop
        for j in cgrid1.yg'range loop
            mypltr(Long_Float(i), Long_Float(j), x, y, System.Null_Address);

            argx := x * pi / 2.0;
            argy := y * pi / 2.0;
            distort := 0.4;

            cgrid1.xg(i) := x + distort * cos(argx); -- This gets assigned j times.
            cgrid1.yg(j) := y - distort * cos(argy);

            cgrid2.xg(i, j) := x + distort * cos(argx) * cos(argy);
            cgrid2.yg(i, j) := y - distort * cos(argx) * cos(argy);
            cgrid2.zg(i, j) := 0.0; -- Not used, but initialize anway.
        end loop;
    end loop;

    -- Plot using identity transform
    Advance_To_Subpage(Next_Subpage);
    Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
    Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);

    Select_Fill_Pattern(0);

    Shade_Regions(z, Null, -1.0, 1.0, -1.0, 1.0,
         shedge, fill_width,
         cont_color, cont_width,
         Fill_Polygon'access, True, Null, System.Null_Address);

    -- Smaller text
    Set_Character_Height(0.0, 0.75);
    -- Small ticks on the vertical axis
    Set_Major_Tick_Length(0.0, 0.5);
    Set_Minor_Tick_Length(0.0, 0.5);

    num_values(0) := ns + 1;
    for i in values'range(2) loop
        values(0, i) := shedge(i);
    end loop;
    Create_Colorbar(colorbar_width, colorbar_height,
        Colorbar_Shade + Colorbar_Shade_Label, 0,
        0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
        cont_color, cont_width,
        label_opts, labels,
        axis_opts,
        axis_ticks, axis_subticks,
        num_values, values);

    -- Reset text and tick sizes
    Set_Character_Height(0.0, 1.0);
    Set_Major_Tick_Length(0.0, 1.0);
    Set_Minor_Tick_Length(0.0, 1.0);

    Set_Pen_Color(Red);
    Box_Around_Viewport("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    Set_Pen_Color(Yellow);

    Write_Labels("distance", "altitude", "Bogon density");

    -- Plot using 1d coordinate transform

    -- Load colour palettes
    Set_Color_Map_0_From_File("cmap0_black_on_white.pal");
    Set_Color_Map_1_From_File("cmap1_blue_yellow.pal", True);

    -- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display.
    Set_Number_Of_Colors_Map_0(3);

    Advance_To_Subpage(Next_Subpage);
    Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
    Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);

    Select_Fill_Pattern(0);

    Shade_Regions(z, Null, -1.0, 1.0, -1.0, 1.0,
         shedge, fill_width,
         cont_color, cont_width,
         Fill_Polygon'access, True, Plot_Transformation_1'access, cgrid1'Address);

    -- Smaller text
    Set_Character_Height(0.0, 0.75);
    -- Small ticks on the vertical axis
    Set_Major_Tick_Length(0.0, 0.5);
    Set_Minor_Tick_Length(0.0, 0.5);

    num_values(0) := ns + 1;
    for i in values'range(2) loop
        values(0, i) := shedge(i);
    end loop;
    Create_Colorbar(colorbar_width, colorbar_height,
        Colorbar_Shade + Colorbar_Shade_Label, 0,
        0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
        cont_color, cont_width,
        label_opts, labels,
        axis_opts,
        axis_ticks, axis_subticks,
        num_values, values);

    -- Reset text and tick sizes
    Set_Character_Height(0.0, 1.0);
    Set_Major_Tick_Length(0.0, 1.0);
    Set_Minor_Tick_Length(0.0, 1.0);

    Set_Pen_Color(Red);
    Box_Around_Viewport("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    Set_Pen_Color(Yellow);

    Write_Labels("distance", "altitude", "Bogon density");

    -- Plot using 2d coordinate transform

    -- Load colour palettes
    Set_Color_Map_0_From_File("cmap0_black_on_white.pal");
    Set_Color_Map_1_From_File("cmap1_blue_red.pal", True);

    -- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display.
    Set_Number_Of_Colors_Map_0(3);

    Advance_To_Subpage(Next_Subpage);
    Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
    Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);

    Select_Fill_Pattern(0);

    Shade_Regions(z, Null, -1.0, 1.0, -1.0, 1.0,
         shedge, fill_width,
         cont_color, cont_width,
         Fill_Polygon'access, False, Plot_Transformation_2'access, cgrid2'Address);

    -- Smaller text
    Set_Character_Height(0.0, 0.75);
    -- Small ticks on the vertical axis
    Set_Major_Tick_Length(0.0, 0.5);
    Set_Minor_Tick_Length(0.0, 0.5);

    num_values(0) := ns + 1;
    for i in values'range(2) loop
        values(0, i) := shedge(i);
    end loop;
    Create_Colorbar(colorbar_width, colorbar_height,
        Colorbar_Shade + Colorbar_Shade_Label, 0,
        0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
        cont_color, cont_width,
        label_opts, labels,
        axis_opts,
        axis_ticks, axis_subticks,
        num_values, values);

    -- Reset text and tick sizes
    Set_Character_Height(0.0, 1.0);
    Set_Major_Tick_Length(0.0, 1.0);
    Set_Minor_Tick_Length(0.0, 1.0);

    Set_Pen_Color(Red);
    Box_Around_Viewport("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    Set_Pen_Color(Yellow);
    Contour_Plot(w, 1, nx, 1, ny, clevel, Plot_Transformation_2'access, cgrid2'Address);

    Write_Labels("distance", "altitude", "Bogon density, with streamlines");

    -- Plot using 2d coordinate transform

    -- Load colour palettes
    Set_Color_Map_0_From_File("");
    Set_Color_Map_1_From_File("", True);

    -- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display.
    Set_Number_Of_Colors_Map_0(3);

    Advance_To_Subpage(Next_Subpage);
    Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
    Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);

    Select_Fill_Pattern(0);

    Shade_Regions(z, Null, -1.0, 1.0, -1.0, 1.0,
         shedge, fill_width,
         2, 3.0,
         Fill_Polygon'access, False, Plot_Transformation_2'access, cgrid2'Address);

    -- Smaller text
    Set_Character_Height(0.0, 0.75);
    -- Small ticks on the vertical axis
    Set_Major_Tick_Length(0.0, 0.5);
    Set_Minor_Tick_Length(0.0, 0.5);

    num_values(0) := ns + 1;
    for i in values'range(2) loop
        values(0, i) := shedge(i);
    end loop;
    Create_Colorbar(colorbar_width, colorbar_height,
        Colorbar_Shade + Colorbar_Shade_Label, 0,
        0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
        2, 3.0,
        label_opts, labels,
        axis_opts,
        axis_ticks, axis_subticks,
        num_values, values);

    -- Reset text and tick sizes
    Set_Character_Height(0.0, 1.0);
    Set_Major_Tick_Length(0.0, 1.0);
    Set_Minor_Tick_Length(0.0, 1.0);

    Set_Pen_Color(Red);
    Box_Around_Viewport("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    Set_Pen_Color(Yellow);

    Write_Labels("distance", "altitude", "Bogon density");

    -- Note this exclusion API will probably change.

    -- Plot using 2d coordinate transform and exclusion

    -- Ada note: This "exclusion" part works if exclude is set to True.
    -- In the C original example, the setting of exclude was handled by the
    -- the input parser which handling is not implemented in this Ada example.
    --    exclude := False;
    if exclude then

        -- Load colour palettes.
        Set_Color_Map_0_From_File("cmap0_black_on_white.pal");
        Set_Color_Map_1_From_File("cmap1_gray.pal", True);

        -- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display.
        Set_Number_Of_Colors_Map_0(3);

        Advance_To_Subpage(Next_Subpage);
        Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
        Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);

        Select_Fill_Pattern(0);

        Shade_Regions(z, zdefined'Unrestricted_Access, -1.0, 1.0, -1.0, 1.0,
             shedge, fill_width,
             cont_color, cont_width,
             Fill_Polygon'access, False, Plot_Transformation_2'access, cgrid2'Address);

        Set_Pen_Color(Red);
        Box_Around_Viewport("bcnst", 0.0, 0, "bcnstv", 0.0, 0);

        Write_Labels("distance", "altitude", "Bogon density with exclusion");
    end if;

    -- Example with polar coordinates.

    -- Load colour palettes.
    Set_Color_Map_0_From_File("cmap0_black_on_white.pal");
    Set_Color_Map_1_From_File("cmap1_gray.pal", True);

    -- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display.
    Set_Number_Of_Colors_Map_0(3);

    Advance_To_Subpage(Next_Subpage);
    Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
    Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);

    Select_Fill_Pattern(0);

    -- Build new coordinate matrices.
    for i in cgrid2.xg'range(1) loop
        r := Long_Float(i) / Long_Float(nx - 1);
        for j in cgrid2.xg'range(2) loop
           t := (2.0 * pi /(Long_Float(ny) - 1.0)) * Long_Float(j);
           cgrid2.xg(i, j) := r * cos(t);
           cgrid2.yg(i, j) := r * sin(t);
           z(i, j) := exp(-r * r) * cos(5.0 * pi * r) * cos(5.0 * t);
        end loop;
    end loop;

    -- Need a new shedge to go along with the new data set.
    zmin := Matrix_Min(z);
    zmax := Matrix_Max(z);

    for i in shedge'range loop
        shedge(i) := zmin + (zmax - zmin) * Long_Float(i) / Long_Float(ns);
    end loop;

    -- Now we can shade the interior region.
    Shade_Regions(z, Null, -1.0, 1.0, -1.0, 1.0,
         shedge, fill_width,
         cont_color, cont_width,
         Fill_Polygon'access, False, Plot_Transformation_2'access, cgrid2'Address);

    -- Smaller text
    Set_Character_Height(0.0, 0.75);
    -- Small ticks on the vertical axis
    Set_Major_Tick_Length(0.0, 0.5);
    Set_Minor_Tick_Length(0.0, 0.5);

    num_values(0) := ns + 1;
    for i in values'range(2) loop
        values(0, i) := shedge(i);
    end loop;
    Create_Colorbar(colorbar_width, colorbar_height,
        Colorbar_Shade + Colorbar_Shade_Label, 0,
        0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
        cont_color, cont_width,
        label_opts, labels,
        axis_opts,
        axis_ticks, axis_subticks,
        num_values, values);

    -- Reset text and tick sizes
    Set_Character_Height(0.0, 1.0);
    Set_Major_Tick_Length(0.0, 1.0);
    Set_Minor_Tick_Length(0.0, 1.0);

    -- Now we can draw the perimeter. (If do before, shade stuff may overlap.)
      for i in px'range loop
           t := 2.0 * pi / Long_Float(px'Length - 1) * Long_Float(i);
           px(i) := cos(t);
           py(i) := sin(t);
      end loop;

      Set_Pen_Color(Red);
      Draw_Curve(px, py);

      -- And label the plot.
      Set_Pen_Color(Yellow);
      Write_Labels("", "",  "Tokamak Bogon Instability");

    -- Clean up
    End_PLplot;

end xstandard16a;
