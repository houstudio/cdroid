-- Ada binding to PLplot using the traditional PLplot subprogram names

-- Copyright (C) 2006-2013 Jerry Bauck

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
    PLplot_Thin,
    PLplot_Auxiliary,
    Ada.Text_IO,
    Ada.Numerics.Long_Elementary_Functions,
    Ada.Strings.Unbounded,
    Ada.Unchecked_Conversion,
    Ada.Strings.Maps,
    Ada.Command_Line,
    System,
    System.Address_To_Access_Conversions,
    Interfaces.C.Pointers,
    Interfaces.C;
use
    Ada.Text_IO,
    PLplot_Auxiliary,
    Ada.Numerics.Long_Elementary_Functions,
    Ada.Strings.Unbounded,
    Interfaces.C;

package body PLplot_Traditional is

--------------------------------------------------------------------------------
--        High-Level subroutines for thick binding                            --
--------------------------------------------------------------------------------

    -- When asked to draw white lines on black background, do it.
    -- This is the default.
    procedure Draw_On_Black is
    begin
        plscol0(Black, 0,   0  ,   0);
        plscol0(White, 255, 255, 255);
    end Draw_On_Black;


    -- When asked to draw black lines on white background, reverse black and white.
    -- This might look better on anti-aliased displays.
    -- fix this Darken some colors which have low contrast on white background, e.g. Yellow.
    -- fix this Make a version that draws on white and converts _all_ colors to black for publications.
    -- fix this Make this so that it works on Color Map 1 because as of now, it does
    -- not change the background color for e.g. surface plots. See also Draw_On_Black.
    procedure Draw_On_White is
    begin
        plscol0(Black, 255, 255, 255);
        plscol0(White, 0,   0,   0);
    end Draw_On_White;


    -- Set default pen width. Docs don't say, so I'll make it 1.
    -- I could make this depend on the type of outut device used.
    Default_Pen_Width : constant Long_Float := 1.0;
    procedure Set_Default_Pen_Width is
    begin
        plwidth(Default_Pen_Width);
    end Set_Default_Pen_Width;


    -- Plotter for up to five x-y pairs and settable axis style, plot
    -- line colors, widths, and styles, justification, zoom, and labels.
    -- Can be used directly or as part of a "simple" plotter
    -- such as those that follow or which are made by the user.
    -- fixme Add capability for labels, legends.
    procedure Multiplot_Pairs
       (x1            : Real_Vector     := Dont_Plot_This;
        y1            : Real_Vector     := Dont_Plot_This;
        x2            : Real_Vector     := Dont_Plot_This;
        y2            : Real_Vector     := Dont_Plot_This;
        x3            : Real_Vector     := Dont_Plot_This;
        y3            : Real_Vector     := Dont_Plot_This;
        x4            : Real_Vector     := Dont_Plot_This;
        y4            : Real_Vector     := Dont_Plot_This;
        x5            : Real_Vector     := Dont_Plot_This;
        y5            : Real_Vector     := Dont_Plot_This;
        X_Labels      : Label_String_Array_Type := Default_Label_String_Array;
        Y_Labels      : Label_String_Array_Type := Default_Label_String_Array;
        Title_Labels  : Label_String_Array_Type := Default_Label_String_Array;
        Axis_Style    : Axis_Style_Type         := Linear_Box_Plus;
        Colors        : Color_Array_Type        := Default_Color_Array;
        Line_Widths   : Line_Width_Array_Type   := Default_Line_Width_Array;
        Line_Styles   : Line_Style_Array_Type   := Default_Line_Style_Array;
        Justification : Justification_Type      := Not_Justified;
        x_Min_Zoom    : Long_Float              := Long_Float'small;
        x_Max_Zoom    : Long_Float              := Long_Float'large;
        y_Min_Zoom    : Long_Float              := Long_Float'small;
        y_Max_Zoom    : Long_Float              := Long_Float'large) is

        x_Min, y_Min : Long_Float := Long_Float'large;
        x_Max, y_Max : Long_Float := Long_Float'small;

    begin
        -- Set or find x_Min.
        if x_Min_Zoom /= Long_Float'small then -- zoom
            x_Min := x_Min_Zoom;
        else -- Auto-scale x_Min.
            if x1'length /= 1 then
                x_Min := Long_Float'min(x_Min, Vector_Min(x1));
            end if;
            if x2'length /= 1 then
                x_Min := Long_Float'min(x_Min, Vector_Min(x2));
            end if;
            if x3'length /= 1 then
                x_Min := Long_Float'min(x_Min, Vector_Min(x3));
            end if;
            if x4'length /= 1 then
                x_Min := Long_Float'min(x_Min, Vector_Min(x4));
            end if;
            if x5'length /= 1 then
                x_Min := Long_Float'min(x_Min, Vector_Min(x5));
            end if;
        end if; -- Set or find x_Min.

        -- Set or find x_Max.
        if x_Max_Zoom /= Long_Float'large then -- zoom
            x_Max := x_Max_Zoom;
        else  -- Auto-scale x_Max.
            if x1'length /= 1 then
                x_Max := Long_Float'max(x_Max, Vector_Max(x1));
            end if;
            if x2'length /= 1 then
                x_Max := Long_Float'max(x_Max, Vector_Max(x2));
            end if;
            if x3'length /= 1 then
                x_Max := Long_Float'max(x_Max, Vector_Max(x3));
            end if;
            if x4'length /= 1 then
                x_Max := Long_Float'max(x_Max, Vector_Max(x4));
            end if;
            if x5'length /= 1 then
                x_Max := Long_Float'max(x_Max, Vector_Max(x5));
            end if;
        end if; -- Set or find x_Max.

        -- Set or find y_Min.
        if y_Min_Zoom /= Long_Float'small then -- zoom
            y_Min := y_Min_Zoom;
        else -- Auto-scale y_Min.
            if y1'length /= 1 then
                y_Min := Long_Float'min(y_Min, Vector_Min(y1));
            end if;
            if y2'length /= 1 then
                y_Min := Long_Float'min(y_Min, Vector_Min(y2));
            end if;
            if y3'length /= 1 then
                y_Min := Long_Float'min(y_Min, Vector_Min(y3));
            end if;
            if y4'length /= 1 then
                y_Min := Long_Float'min(y_Min, Vector_Min(y4));
            end if;
            if y5'length /= 1 then
                y_Min := Long_Float'min(y_Min, Vector_Min(y5));
            end if;
        end if; -- Set or find y_Min.

        -- Set or find y_Max.
        if y_Max_Zoom /= Long_Float'large then -- zoom
            y_Max := y_Max_Zoom;
        else  -- Auto-scale y_Max.
            if y1'length /= 1 then
                y_Max := Long_Float'max(y_Max, Vector_Max(y1));
            end if;
            if y2'length /= 1 then
                y_Max := Long_Float'max(y_Max, Vector_Max(y2));
            end if;
            if y3'length /= 1 then
                y_Max := Long_Float'max(y_Max, Vector_Max(y3));
            end if;
            if y4'length /= 1 then
                y_Max := Long_Float'max(y_Max, Vector_Max(y4));
            end if;
            if y5'length /= 1 then
                y_Max := Long_Float'max(y_Max, Vector_Max(y5));
            end if;
        end if; -- Set or find x_Max.


        -- Set environment and its color.
        plcol0(White);
--        Set_Environment_Clear_Subpage(x_Min, x_Max, y_Min, y_Max, Justification, Axis_Style);
        plenv(x_Min, x_Max, y_Min, y_Max, Justification, Axis_Style);

        if x1'length /= 1 and y1'length /= 1 then
            pllab(To_String(X_Labels(1)), To_String(Y_Labels(1)), To_String(Title_Labels(1)));
            plcol0(Colors(1));
            --Set_Pen_Width(Line_Widths(1));
            pllsty(Line_Styles(1));
            plline(x1, y1);
        end if;

        if x2'length /= 1 and y2'length /= 1 then
            pllab(To_String(X_Labels(2)), To_String(Y_Labels(2)), To_String(Title_Labels(2)));
            plcol0(Colors(2));
            --Set_Pen_Width(Line_Widths(2));
            pllsty(Line_Styles(2));
            plline(x2, y2);
        end if;

        if x3'length /= 1 and y3'length /= 1 then
            pllab(To_String(X_Labels(3)), To_String(Y_Labels(3)), To_String(Title_Labels(3)));
            plcol0(Colors(3));
            --Set_Pen_Width(Line_Widths(3));
            pllsty(Line_Styles(3));
            plline(x3, y3);
        end if;

        if x4'length /= 1 and y4'length /= 1 then
            pllab(To_String(X_Labels(4)), To_String(Y_Labels(4)), To_String(Title_Labels(4)));
            plcol0(Colors(4));
            --Set_Pen_Width(Line_Widths(4));
            pllsty(Line_Styles(4));
            plline(x4, y4);
        end if;

        if x5'length /= 1 and y5'length /= 1 then
            pllab(To_String(X_Labels(5)), To_String(Y_Labels(5)), To_String(Title_Labels(5)));
            plcol0(Colors(5));
            --Set_Pen_Width(Line_Widths(5));
            pllsty(Line_Styles(5));
            plline(x5, y5);
        end if;

        plcol0(White);
        Set_Default_Pen_Width;
        pllsty(1); --solid
    end Multiplot_Pairs;


--------- Simple plotters requiring minimal arguments -----


    -- Quick plotter requires no x-axis as input; makes x up from indices of first of multiple y's.
    procedure Quick_Plot
       (y1 : Real_Vector := Dont_Plot_This;
        y2 : Real_Vector := Dont_Plot_This;
        y3 : Real_Vector := Dont_Plot_This;
        y4 : Real_Vector := Dont_Plot_This;
        y5 : Real_Vector := Dont_Plot_This;
        X_Label     : String := To_String(Default_Label_String);
        Y_Label     : String := To_String(Default_Label_String);
        Title_Label : String := To_String(Default_Label_String))
    is
        x : Real_Vector(y1'range);
        X_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Y_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Title_Label_String_Array : Label_String_Array_Type := Default_Label_String_Array;
    begin
        for i in x'range loop
            x(i) := Long_Float(i);
        end loop;

        X_Label_String_Array(1)     := TUB(X_Label);     -- First slot only; others not used.
        Y_Label_String_Array(1)     := TUB(Y_Label);     -- First slot only; others not used.
        Title_Label_String_Array(1) := TUB(Title_Label); -- First slot only; others not used.

        Multiplot_Pairs(x, y1, x, y2, x, y3, x, y4, x, y5,
            X_Labels     => X_Label_String_Array,
            Y_Labels     => Y_Label_String_Array,
            Title_Labels => Title_Label_String_Array,
            Axis_Style   => Linear_Major_Grid);
    end Quick_Plot;


    -- Simple plotter for single x array and multiple y arrays
    procedure Simple_Plot
       (x  : Real_Vector;
        y1 : Real_Vector := Dont_Plot_This;
        y2 : Real_Vector := Dont_Plot_This;
        y3 : Real_Vector := Dont_Plot_This;
        y4 : Real_Vector := Dont_Plot_This;
        y5 : Real_Vector := Dont_Plot_This;
        X_Label     : String := To_String(Default_Label_String);
        Y_Label     : String := To_String(Default_Label_String);
        Title_Label : String := To_String(Default_Label_String)) is

        X_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Y_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Title_Label_String_Array : Label_String_Array_Type := Default_Label_String_Array;

    begin
        X_Label_String_Array(1)     := TUB(X_Label);     -- First slot only; others not used.
        Y_Label_String_Array(1)     := TUB(Y_Label);     -- First slot only; others not used.
        Title_Label_String_Array(1) := TUB(Title_Label); -- First slot only; others not used.

        Multiplot_Pairs(x, y1, x, y2, x, y3, x, y4, x, y5,
            X_Labels     => X_Label_String_Array,
            Y_Labels     => Y_Label_String_Array,
            Title_Labels => Title_Label_String_Array,
            Axis_Style   => Linear_Major_Grid);
    end Simple_Plot;


    -- Simple log x plotter for single x array and multiple y arrays
    -- fix this: Automatically skip zero-valued abscissa; place marker at the
    -- left-hand side of the plot at the ordinate of the deleted point.
    procedure Simple_Plot_Log_X
       (x  : Real_Vector;
        y1 : Real_Vector := Dont_Plot_This;
        y2 : Real_Vector := Dont_Plot_This;
        y3 : Real_Vector := Dont_Plot_This;
        y4 : Real_Vector := Dont_Plot_This;
        y5 : Real_Vector := Dont_Plot_This;
        X_Label     : String := To_String(Default_Label_String);
        Y_Label     : String := To_String(Default_Label_String);
        Title_Label : String := To_String(Default_Label_String);
        Log_Base : Long_Float := 10.0) is -- Should this default to e?

        X_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Y_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Title_Label_String_Array : Label_String_Array_Type := Default_Label_String_Array;

        x_Log : Real_Vector(x'range);

    begin
        X_Label_String_Array(1)     := TUB(X_Label);     -- First slot only; others not used.
        Y_Label_String_Array(1)     := TUB(Y_Label);     -- First slot only; others not used.
        Title_Label_String_Array(1) := TUB(Title_Label); -- First slot only; others not used.

        for i in x_Log'range loop
            x_Log(i) := Log(x(i), Log_Base);
        end loop;
        Multiplot_Pairs(x_Log, y1, x_Log, y2, x_Log, y3, x_Log, y4, x_Log, y5,
            X_Labels     => X_Label_String_Array,
            Y_Labels     => Y_Label_String_Array,
            Title_Labels => Title_Label_String_Array,
            Axis_Style   => Log_X_Minor_Grid);
    end Simple_Plot_Log_X;


    -- Simple log y plotter for multiple x arrays and single y array
    -- fix this: Automatically skip zero-valued ordinate; place marker at the
    -- bottom of the plot at the abscissa of the deleted point.
    procedure Simple_Plot_Log_Y
       (x1 : Real_Vector := Dont_Plot_This;
        y  : Real_Vector := Dont_Plot_This; -- Beware of argument order.
        x2 : Real_Vector := Dont_Plot_This;
        x3 : Real_Vector := Dont_Plot_This;
        x4 : Real_Vector := Dont_Plot_This;
        x5 : Real_Vector := Dont_Plot_This;
        X_Label     : String := To_String(Default_Label_String);
        Y_Label     : String := To_String(Default_Label_String);
        Title_Label : String := To_String(Default_Label_String);
        Log_Base : Long_Float := 10.0) is -- Should this default to e?

        X_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Y_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Title_Label_String_Array : Label_String_Array_Type := Default_Label_String_Array;

        y_Log : Real_Vector(y'range);

    begin
        X_Label_String_Array(1)     := TUB(X_Label);     -- First slot only; others not used.
        Y_Label_String_Array(1)     := TUB(Y_Label);     -- First slot only; others not used.
        Title_Label_String_Array(1) := TUB(Title_Label); -- First slot only; others not used.

        for i in y_Log'range loop
            y_Log(i) := Log(y(i), Log_Base);
        end loop;
        Multiplot_Pairs(x1, y_Log, x2, y_Log, x3, y_Log, x4, y_Log, x5, y_Log,
            X_Labels     => X_Label_String_Array,
            Y_Labels     => Y_Label_String_Array,
            Title_Labels => Title_Label_String_Array,
            Axis_Style   => Log_Y_Minor_Grid);
    end Simple_Plot_Log_Y;


    -- Simple log x - log y plotter
    procedure Simple_Plot_Log_XY
       (x, y        : Real_Vector;
        X_Label     : String := To_String(Default_Label_String);
        Y_Label     : String := To_String(Default_Label_String);
        Title_Label : String := To_String(Default_Label_String);
        x_Log_Base, y_Log_Base : Long_Float := 10.0) is -- Should this default to e?

        X_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Y_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Title_Label_String_Array : Label_String_Array_Type := Default_Label_String_Array;

        x_Log : Real_Vector(x'range);
        y_Log : Real_Vector(y'range);
    begin
        X_Label_String_Array(1)     := TUB(X_Label);     -- First slot only; others not used.
        Y_Label_String_Array(1)     := TUB(Y_Label);     -- First slot only; others not used.
        Title_Label_String_Array(1) := TUB(Title_Label); -- First slot only; others not used.

        for i in x_Log'range loop
            x_Log(i) := Log(x(i), x_Log_Base);
            y_Log(i) := Log(y(i), y_Log_Base);
        end loop;
        Multiplot_Pairs(x_Log, y_Log,
            X_Labels     => X_Label_String_Array,
            Y_Labels     => Y_Label_String_Array,
            Title_Labels => Title_Label_String_Array,
            Axis_Style   => Log_XY_Minor_Grid);
    end Simple_Plot_Log_XY;


    -- Simple plotter for multiple x-y arrays specified pairwise.
    procedure Simple_Plot_Pairs
       (x1 : Real_Vector := Dont_Plot_This;
        y1 : Real_Vector := Dont_Plot_This;
        x2 : Real_Vector := Dont_Plot_This;
        y2 : Real_Vector := Dont_Plot_This;
        x3 : Real_Vector := Dont_Plot_This;
        y3 : Real_Vector := Dont_Plot_This;
        x4 : Real_Vector := Dont_Plot_This;
        y4 : Real_Vector := Dont_Plot_This;
        x5 : Real_Vector := Dont_Plot_This;
        y5 : Real_Vector := Dont_Plot_This;
        X_Label     : String := To_String(Default_Label_String);
        Y_Label     : String := To_String(Default_Label_String);
        Title_Label : String := To_String(Default_Label_String)) is

        X_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Y_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Title_Label_String_Array : Label_String_Array_Type := Default_Label_String_Array;
    begin
        X_Label_String_Array(1)     := TUB(X_Label);     -- First slot only; others not used.
        Y_Label_String_Array(1)     := TUB(Y_Label);     -- First slot only; others not used.
        Title_Label_String_Array(1) := TUB(Title_Label); -- First slot only; others not used.

        Multiplot_Pairs(x1, y1, x2, y2, x3, y3, x4, y4, x5, y5,
            X_Labels     => X_Label_String_Array,
            Y_Labels     => Y_Label_String_Array,
            Title_Labels => Title_Label_String_Array,
            Axis_Style   => Linear_Major_Grid);
    end Simple_Plot_Pairs;


--------- Plotter requiring somewhat more arguments ------

    -- Single plotter with flexible attributes
    -- Similar to Multiplot_Pairs except single trace and no attribute arrays.
    procedure Single_Plot
       (x, y          : Real_Vector;
        X_Label       : String             := To_String(Default_Label_String);
        Y_Label       : String             := To_String(Default_Label_String);
        Title_Label   : String             := To_String(Default_Label_String);
        Axis_Style    : Axis_Style_Type    := Linear_Major_Grid;
        Color         : Plot_Color_Type    := Red;
        Line_Width    : Long_Float         := 1.0;
        Line_Style    : Line_Style_Type    := 1;
        Justification : Justification_Type := Not_Justified;
        x_Min_Zoom    : Long_Float         := Long_Float'small;
        x_Max_Zoom    : Long_Float         := Long_Float'large;
        y_Min_Zoom    : Long_Float         := Long_Float'small;
        y_Max_Zoom    : Long_Float         := Long_Float'large) is

        X_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Y_Label_String_Array     : Label_String_Array_Type := Default_Label_String_Array;
        Title_Label_String_Array : Label_String_Array_Type := Default_Label_String_Array;
        Color_Array              : Color_Array_Type        := Default_Color_Array;
        Line_Width_Array         : Line_Width_Array_Type   := Default_Line_Width_Array;
        Line_Style_Array         : Line_Style_Array_Type   := Default_Line_Style_Array;

    begin
        X_Label_String_Array(1)     := TUB(X_Label);     -- First slot only; others not used.
        Y_Label_String_Array(1)     := TUB(Y_Label);     -- First slot only; others not used.
        Title_Label_String_Array(1) := TUB(Title_Label); -- First slot only; others not used.
        Color_Array(1)              := Color;
        Line_Width_Array(1)         := Line_Width;  -- First slot only; others not used.
        Line_Style_Array(1)         := Line_Style;  -- First slot only; others not used.

        -- Process arrays if log plot is indicated.
        -- The declare blocks save memory and time for non-log plots.

        -- Log x.
        if Axis_Style in Log_X_Box_Plus..Log_X_Minor_Grid then
            declare
                x_Log : Real_Vector(x'range);
            begin
                for i in x_Log'range loop
                    x_Log(i) := Log(x(i), 10.0);
                end loop;

                Multiplot_Pairs(
                    x_Log, y,
                    X_Labels      => X_Label_String_Array,
                    Y_Labels      => Y_Label_String_Array,
                    Title_Labels  => Title_Label_String_Array,
                    Axis_Style    => Axis_Style,
                    Colors        => Color_Array,
                    Line_Widths   => Line_Width_Array,
                    Line_Styles   => Line_Style_Array,
                    Justification => Justification,
                    x_Min_Zoom    => x_Min_Zoom,
                    x_Max_Zoom    => x_Max_Zoom,
                    y_Min_Zoom    => y_Min_Zoom,
                    y_Max_Zoom    => y_Max_Zoom);
            end; -- declare
        end if; -- log x

        -- Log y
        if Axis_Style in Log_Y_Box_Plus..Log_Y_Minor_Grid then
            declare
                y_Log : Real_Vector(y'range);
            begin
                for i in y_Log'range loop
                    y_Log(i) := Log(y(i), 10.0);
                end loop;
                Multiplot_Pairs(
                    x, y_Log,
                    X_Labels      => X_Label_String_Array,
                    Y_Labels      => Y_Label_String_Array,
                    Title_Labels  => Title_Label_String_Array,
                    Axis_Style    => Axis_Style,
                    Colors        => Color_Array,
                    Line_Widths   => Line_Width_Array,
                    Line_Styles   => Line_Style_Array,
                    Justification => Justification,
                    x_Min_Zoom    => x_Min_Zoom,
                    x_Max_Zoom    => x_Max_Zoom,
                    y_Min_Zoom    => y_Min_Zoom,
                    y_Max_Zoom    => y_Max_Zoom);
            end; -- declare
        end if; -- log y

        -- Log x and log y
        if Axis_Style in Log_XY_Box_Plus..Log_XY_Minor_Grid then
            declare
                x_Log : Real_Vector(x'range);
                y_Log : Real_Vector(y'range);
            begin
                for i in x_Log'range loop
                    x_Log(i) := Log(x(i), 10.0);
                    y_Log(i) := Log(y(i), 10.0);
                end loop;
                Multiplot_Pairs(
                    x_Log, y_Log,
                    X_Labels      => X_Label_String_Array,
                    Y_Labels      => Y_Label_String_Array,
                    Title_Labels  => Title_Label_String_Array,
                    Axis_Style    => Axis_Style,
                    Colors        => Color_Array,
                    Line_Widths   => Line_Width_Array,
                    Line_Styles   => Line_Style_Array,
                    Justification => Justification,
                    x_Min_Zoom    => x_Min_Zoom,
                    x_Max_Zoom    => x_Max_Zoom,
                    y_Min_Zoom    => y_Min_Zoom,
                    y_Max_Zoom    => y_Max_Zoom);
            end; -- declare
        end if; -- log x and log y

        -- Linear plot is indicated.
        if Axis_Style in No_Box..Linear_Minor_Grid then
            Multiplot_Pairs(
                x, y,
                X_Labels      => X_Label_String_Array,
                Y_Labels      => Y_Label_String_Array,
                Title_Labels  => Title_Label_String_Array,
                Axis_Style    => Axis_Style,
                Colors        => Color_Array,
                Line_Widths   => Line_Width_Array,
                Line_Styles   => Line_Style_Array,
                Justification => Justification,
                x_Min_Zoom    => x_Min_Zoom,
                x_Max_Zoom    => x_Max_Zoom,
                y_Min_Zoom    => y_Min_Zoom,
                y_Max_Zoom    => y_Max_Zoom);
        end if; -- Linear plot
    end Single_Plot;


--------- Simple Contour Plotter ------

    procedure Simple_Contour
       (z             : Real_Matrix;
        Number_Levels : Integer := 10;
        X_Label       : String  := To_String(Default_Label_String);
        Y_Label       : String  := To_String(Default_Label_String);
        Title_Label   : String  := To_String(Default_Label_String)) is

        Contour_Levels : Real_Vector (0 .. Number_Levels);

    begin
        -- Fill the contour vector with some levels.
        Calculate_Contour_Levels(Contour_Levels, Matrix_Min(z), Matrix_Max(z));

        pladv(Next_Subpage);
        plvpor(0.1, 0.9, 0.1, 0.9);
        plwind(1.0, 35.0, 1.0, 46.0); -- fix
        plcol0(White);
        PLplot_Traditional.plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0); -- I have no idea why the compiler requires PLplot_Traditional. here.
        plcol0(White);
        pllab(X_Label, Y_Label, Title_Label);
        plcol0(White);
        pl_setcontlabelparam(0.008, 0.6, 0.1, True);
        plcont(z, z'First(1), z'Last(1), z'First(2), z'Last(2),
            Contour_Levels, PLplot_Thin.pltr0'access, System.Null_Address);
    end Simple_Contour;


--------- Simple 3D Mesh Plotter ------

    procedure Simple_Mesh_3D
       (x, y     : Real_Vector; -- data definition points
        z        : Real_Matrix; -- z(x, y) = z(x(i), y(j))
        x_Min    : Long_Float := 0.0;  -- user coordinate limits
        x_Max    : Long_Float := 0.0;  -- If x_Min = x_Max = 0.0 then plot
        y_Min    : Long_Float := 0.0;  -- x-limits are found automatically.
        y_Max    : Long_Float := 0.0;  -- Ditto y_Min and y_Max.
        Altitude : Long_Float := 30.0; -- viewing elevation angle in degrees
        Azimuth  : Long_Float := 30.0; -- viewing azimuth in degrees
        X_Label  : String := "x";
        Y_Label  : String := "y";
        Z_Label  : String := "z") is

        x_Min_Local, x_Max_Local, y_Min_Local, y_Max_Local : Long_Float;

    begin
        -- Check for compatible lengths in x, y, and z.
        if x'length /= z'length(1) then
            Put_Line("*** WARNING: Mismatached x-lengths in Simple_Mesh_3D");
        end if;
        if y'length /= z'length(2) then
            Put_Line("*** WARNING: Mismatached y-lengths in Simple_Mesh_3D");
        end if;

        -- Set min and max for x and y if they are not the defaults.
        if x_Min = 0.0 and x_Max = 0.0 then -- override
            x_Min_Local := Vector_Min(x);
            x_Max_Local := Vector_Max(x);
        else
            x_Min_Local := x_Min;
            x_Max_Local := x_Max;
        end if;

        if y_Min = 0.0 and y_Max = 0.0 then -- override
            y_Min_Local := Vector_Min(y);
            y_Max_Local := Vector_Max(y);
        else
            y_Min_Local := y_Min;
            y_Max_Local := y_Max;
        end if;

        Quick_Set_Color_Map_1(Blue_Green_Red); -- no way to restore after doing this
        pladv(Next_Subpage);
        plcol0(White);
        plvpor(0.0, 1.0, 0.0, 1.0);
        plwind(-0.9, 0.9, -0.8, 1.5);
        plw3d(1.0, 1.0, 1.0, x_Min_Local, x_Max_Local, y_Min_Local, y_Max_Local,
            Matrix_Min(z), Matrix_Max(z), Altitude, Azimuth); -- plw3d
        plbox3("bnstu", X_Label, 0.0, 0,
            "bnstu", Y_Label, 0.0, 0,
            "bcdmnstuv", Z_Label, 0.0, 0); -- plbox3
        plmesh(x, y, z, Lines_Parallel_To_X_And_Y + Magnitude_Color); -- plmesh
    end Simple_Mesh_3D;


--------- Simple 3D Surface Plotter ------

    procedure Simple_Surface_3D
       (x, y     : Real_Vector; -- data definition points
        z        : Real_Matrix; -- z(x, y) = z(x(i), y(j))
        x_Min    : Long_Float := 0.0;  -- user coordinate limits;
        x_Max    : Long_Float := 0.0;  -- If x_Min = x_Max = 0.0 then plot
        y_Min    : Long_Float := 0.0;  -- x-limits are found automatically.
        y_Max    : Long_Float := 0.0;  -- Ditto y_Min and y_Max.
        Altitude : Long_Float := 30.0; -- viewing elevation angle in degrees
        Azimuth  : Long_Float := 30.0; -- viewing azimuth in degrees
        X_Label  : String := "x";
        Y_Label  : String := "y";
        Z_Label  : String := "z") is

        x_Min_Local, x_Max_Local, y_Min_Local, y_Max_Local : Long_Float;
        Contour_Levels_Dummy : Real_Vector(0..1) := (others => 0.0);

    begin
        -- Check for compatible lengths in x, y, and z.
        if x'length /= z'length(1) then
            Put_Line("*** WARNING: Mismatached x-lengths in Simple_Mesh_3D");
        end if;
        if y'length /= z'length(2) then
            Put_Line("*** WARNING: Mismatached y-lengths in Simple_Mesh_3D");
        end if;

        -- Set min and max for x and y if they are not the defaults.
        if x_Min = 0.0 and x_Max = 0.0 then -- override
            x_Min_Local := Vector_Min(x);
            x_Max_Local := Vector_Max(x);
        else
            x_Min_Local := x_Min;
            x_Max_Local := x_Max;
        end if;

        if y_Min = 0.0 and y_Max = 0.0 then -- override
            y_Min_Local := Vector_Min(y);
            y_Max_Local := Vector_Max(y);
        else
            y_Min_Local := y_Min;
            y_Max_Local := y_Max;
        end if;

        Quick_Set_Color_Map_1(Blue_Green_Red); -- no way to restore after doing this
        pladv(Next_Subpage);
        plcol0(White);
        plvpor(0.0, 1.0, 0.0, 1.0);
        plwind(-0.9, 0.9, -0.8, 1.5);
        plw3d(1.0, 1.0, 1.0, x_Min_Local, x_Max_Local, y_Min_Local, y_Max_Local,
            Matrix_Min(z), Matrix_Max(z), Altitude, Azimuth); -- plw3d
        plbox3("bnstu", X_Label, 0.0, 0,
            "bnstu", Y_Label, 0.0, 0,
            "bcdmnstuv", Z_Label, 0.0, 0); -- plbox3
        plmesh(x, y, z, Lines_Parallel_To_X_And_Y + Magnitude_Color); -- plmesh
        plsurf3d(x, y, z, Magnitude_Color, Contour_Levels_Dummy);
    end Simple_Surface_3D;



--------- Simple color table manipulatons -----

    -- Things for manipulating color map 0 --

    -- Make a snapshot of color map 0 for possible later full or partial restoration.
    -- This is automatically called at package initialization with results stored
    -- in Default_Red_Components, Default_Green_Components, Default_Blue_Components.
    procedure Make_Snapshot_Of_Color_Map_0
       (Reds, Greens, Blues : out Integer_Array_1D) is
    begin
        for i in Reds'range loop
            plgcol0(i, Reds(i), Greens(i), Blues(i));
        end loop;
    end Make_Snapshot_Of_Color_Map_0;


    -- Restore an arbitray snapshot of color map 0.
    procedure Restore_Snapshot_Of_Color_Map_0
       (Reds, Greens, Blues : Integer_Array_1D) is
    begin
        plscmap0(Reds, Greens, Blues);
    end Restore_Snapshot_Of_Color_Map_0;


    -- Restore the default colors of color map 0 taken as a snapshot at initialization.
    procedure Restore_Default_Snapshot_Of_Color_Map_0 is
    begin
        plscmap0n(Number_Of_Default_Colors);
        plscmap0(Default_Red_Components, Default_Green_Components, Default_Blue_Components);
    end Restore_Default_Snapshot_Of_Color_Map_0;


    -- Functions which correspond to the default colors of color map 0. Calling
    -- one of these (1) resets the corresponding slot in color map 0 to its
    -- default value, and (2) returns the correct integer value for the default
    -- color specified. Thus, using plcol0(Reset_Red) instead of
    -- plcol0(Red) guarantees that the color will be set to Red even if
    -- there have been prior manipulations of color 1.

    function Reset_Black return Integer is
    begin
        plscol0(0, Default_Red_Components(0), Default_Green_Components(0), Default_Blue_Components(0));
        return 0;
    end Reset_Black;

    function Reset_Red return Integer is
    begin
        plscol0(1, Default_Red_Components(1), Default_Green_Components(1), Default_Blue_Components(1));
        return 1;
    end Reset_Red;

    function Reset_Yellow return Integer is
    begin
        plscol0(2, Default_Red_Components(2), Default_Green_Components(2), Default_Blue_Components(2));
        return 2;
    end Reset_Yellow;

    function Reset_Green return Integer is
    begin
        plscol0(3, Default_Red_Components(3), Default_Green_Components(3), Default_Blue_Components(3));
        return 3;
    end Reset_Green;

    function Reset_Aquamarine return Integer is
    begin
        plscol0(4, Default_Red_Components(4), Default_Green_Components(4), Default_Blue_Components(4));
        return 4;
    end Reset_Aquamarine;

    function Reset_Pink return Integer is
    begin
        plscol0(5, Default_Red_Components(5), Default_Green_Components(5), Default_Blue_Components(5));
        return 5;
    end Reset_Pink;

    function Reset_Wheat return Integer is
    begin
        plscol0(6, Default_Red_Components(6), Default_Green_Components(6), Default_Blue_Components(6));
        return 6;
    end Reset_Wheat;

    function Reset_Grey return Integer is
    begin
        plscol0(7, Default_Red_Components(7), Default_Green_Components(7), Default_Blue_Components(7));
        return 7;
    end Reset_Grey;

    function Reset_Brown return Integer is
    begin
        plscol0(8, Default_Red_Components(8), Default_Green_Components(8), Default_Blue_Components(8));
        return 8;
    end Reset_Brown;

    function Reset_Blue return Integer is
    begin
        plscol0(9, Default_Red_Components(9), Default_Green_Components(9), Default_Blue_Components(9));
        return 9;
    end Reset_Blue;

    function Reset_BlueViolet return Integer is
    begin
        plscol0(10, Default_Red_Components(10), Default_Green_Components(10), Default_Blue_Components(10));
        return 10;
    end Reset_BlueViolet;

    function Reset_Cyan return Integer is
    begin
        plscol0(11, Default_Red_Components(11), Default_Green_Components(11), Default_Blue_Components(11));
        return 11;
    end Reset_Cyan;

    function Reset_Turquoise return Integer is
    begin
        plscol0(12, Default_Red_Components(12), Default_Green_Components(12), Default_Blue_Components(12));
        return 12;
    end Reset_Turquoise;

    function Reset_Magenta return Integer is
    begin
        plscol0(13, Default_Red_Components(13), Default_Green_Components(13), Default_Blue_Components(13));
        return 13;
    end Reset_Magenta;

    function Reset_Salmon return Integer is
    begin
        plscol0(14, Default_Red_Components(14), Default_Green_Components(14), Default_Blue_Components(14));
        return 14;
    end Reset_Salmon;

    function Reset_White return Integer is
    begin
        plscol0(15, Default_Red_Components(15), Default_Green_Components(15), Default_Blue_Components(15));
        return 15;
    end Reset_White;


    -- Things for manipulating color map 1 --

    -- Quick application of pre-fabricated color schemes to color map 1.
    procedure Quick_Set_Color_Map_1(Color_Theme : Color_Themes_For_Map_1_Type) is

        Controls_3 : Real_Vector (0..2); -- 3 control points
        Controls_4 : Real_Vector (0..3); -- 4 control points
        Red_3, Green_3, Blue_3 : Real_Vector (0..2); -- define 3
        Red_4, Green_4, Blue_4 : Real_Vector (0..3); -- define 4
        Hue_3, Lightness_3, Saturation_3 : Real_Vector (0..2); -- define 3
        Hue_4, Lightness_4, Saturation_4 : Real_Vector (0..3); -- define 4
        Alt_Hue_Path_3 : Boolean_Array_1D (0..2);
        Alt_Hue_Path_4 : Boolean_Array_1D (0..3);
    begin
        plscmap1n(256);

        Controls_3(0) := 0.0;
        Controls_3(1) := 0.5;
        Controls_3(2) := 1.0;

        Controls_4(0) := 0.0;
        Controls_4(1) := 0.5; -- allow a "bend" at the mid-point
        Controls_4(2) := 0.5; -- allow a "bend" at the mid-point
        Controls_4(3) := 1.0;

        -- Initialize everything, even unused stuff.
        Red_3(0)   := 0.0;
        Red_3(1)   := 0.0;
        Red_3(2)   := 0.0;
        Green_3(0) := 0.0;
        Green_3(1) := 0.0;
        Green_3(2) := 0.0;
        Blue_3(0)  := 0.0;
        Blue_3(1)  := 0.0;
        Blue_3(2)  := 0.0;

        Red_4(0)   := 0.0;
        Red_4(1)   := 0.0;
        Red_4(2)   := 0.0;
        Red_4(3)   := 0.0;
        Green_4(0) := 0.0;
        Green_4(1) := 0.0;
        Green_4(2) := 0.0;
        Green_4(3) := 0.0;
        Blue_4(0)  := 0.0;
        Blue_4(1)  := 0.0;
        Blue_4(2)  := 0.0;
        Blue_4(3)  := 0.0;

        Hue_3(0)        := 0.0;
        Hue_3(1)        := 0.0;
        Hue_3(2)        := 0.0;
        Lightness_3(0)  := 0.0;
        Lightness_3(1)  := 0.0;
        Lightness_3(2)  := 0.0;
        Saturation_3(0) := 0.0;
        Saturation_3(1) := 0.0;
        Saturation_3(2) := 0.0;

        Hue_4(0)        := 0.0;
        Hue_4(1)        := 0.0;
        Hue_4(2)        := 0.0;
        Hue_4(3)        := 0.0;
        Lightness_4(0)  := 0.0;
        Lightness_4(1)  := 0.0;
        Lightness_4(2)  := 0.0;
        Lightness_4(3)  := 0.0;
        Saturation_4(0) := 0.0;
        Saturation_4(1) := 0.0;
        Saturation_4(2) := 0.0;
        Saturation_4(3) := 0.0;

        case Color_Theme is
            when Gray =>
                begin
                    Hue_3(0) := 0.0;
                    Hue_3(1) := 0.0;
                    Hue_3(2) := 0.0;
                    Lightness_3(0) := 0.0;
                    Lightness_3(1) := 0.5;
                    Lightness_3(2) := 1.0;
                    Saturation_3(0) := 0.0;
                    Saturation_3(1) := 0.0;
                    Saturation_3(2) := 0.0;
                    Alt_Hue_Path_3(0) := False;
                    Alt_Hue_Path_3(1) := False;
                    Alt_Hue_Path_3(2) := False;
                    plscmap1l(HLS, Controls_3, Hue_3, Lightness_3, Saturation_3, Alt_Hue_Path_3);
                end;
            when Blue_Green_Red => -- This appears to be the PLplot default color theme.
                begin
                    Blue_3(0) := 1.0;
                    Blue_3(1) := 0.0;
                    Blue_3(2) := 0.0;
                    Green_3(0) := 0.0;
                    Green_3(1) := 1.0;
                    Green_3(2) := 0.0;
                    Red_3(0) := 0.0;
                    Red_3(1) := 0.0;
                    Red_3(2) := 1.0;
                    Alt_Hue_Path_3(0) := False;
                    Alt_Hue_Path_3(1) := False;
                    Alt_Hue_Path_3(2) := False;
                    plscmap1l(RGB, Controls_3, Red_3, Green_3, Blue_3, Alt_Hue_Path_3);
                end;
            when Red_Green_Blue =>
                begin
                    Blue_3(0) := 0.0;
                    Blue_3(1) := 0.0;
                    Blue_3(2) := 1.0;
                    Green_3(0) := 0.0;
                    Green_3(1) := 1.0;
                    Green_3(2) := 0.0;
                    Red_3(0) := 1.0;
                    Red_3(1) := 0.0;
                    Red_3(2) := 0.0;
                    Alt_Hue_Path_3(0) := False;
                    Alt_Hue_Path_3(1) := False;
                    Alt_Hue_Path_3(2) := False;
                    plscmap1l(RGB, Controls_3, Red_3, Green_3, Blue_3, Alt_Hue_Path_3);
                end;
            when Red_Cyan_Blue =>
                begin
                    Hue_3(0) := 360.0;
                    Hue_3(1) := 300.0;
                    Hue_3(2) := 240.0;
                    Saturation_3(0) := 1.0;
                    Saturation_3(1) := 1.0;
                    Saturation_3(2) := 1.0;
                    Lightness_3(0) := 0.5;
                    Lightness_3(1) := 0.5;
                    Lightness_3(2) := 0.5;
                    Alt_Hue_Path_3(0) := False;
                    Alt_Hue_Path_3(1) := False;
                    Alt_Hue_Path_3(2) := False;
                    plscmap1l(HLS, Controls_3, Hue_3, Lightness_3, Saturation_3, Alt_Hue_Path_3);
                end;
            when Blue_Black_Red =>
                begin
                    Hue_4(0) := 240.0; -- Blue
                    Hue_4(1) := 240.0; -- Blue
                    Hue_4(2) := 0.0;   -- Red
                    Hue_4(3) := 0.0;   -- Red
                    Saturation_4(0) := 1.0;
                    Saturation_4(1) := 1.0;
                    Saturation_4(2) := 1.0;
                    Saturation_4(3) := 1.0;
                    Lightness_4(0) := 0.5; -- Apparently different from Brightness
                    Lightness_4(1) := 0.0;
                    Lightness_4(2) := 0.0;
                    Lightness_4(3) := 0.5;
                    Alt_Hue_Path_4(0) := False;
                    Alt_Hue_Path_4(1) := False;
                    Alt_Hue_Path_4(2) := False;
                    Alt_Hue_Path_4(3) := False;
                    plscmap1l(HLS, Controls_4, Hue_4, Lightness_4, Saturation_4, Alt_Hue_Path_4);
                end;
            when Red_Blue_Green =>
                begin
                    Hue_3(0) := 360.0; -- red
                    Hue_3(1) := 210.0; -- blue
                    Hue_3(2) := 120.0;  -- green
                    Lightness_3(0) := 0.5;
                    Lightness_3(1) := 0.5;
                    Lightness_3(2) := 0.5;
                    Saturation_3(0) := 1.0;
                    Saturation_3(1) := 1.0;
                    Saturation_3(2) := 1.0;
                    Alt_Hue_Path_3(0) := False;
                    Alt_Hue_Path_3(1) := False;
                    Alt_Hue_Path_3(2) := False;
                    plscmap1l(HLS, Controls_3, Hue_3, Lightness_3, Saturation_3, Alt_Hue_Path_3);
                end;
            when Red_Yellow =>
                begin
                    Hue_3(0) := 0.0;  -- red
                    Hue_3(1) := 30.0; -- orange
                    Hue_3(2) := 60.0; -- yellow
                    Lightness_3(0) := 0.5;
                    Lightness_3(1) := 0.5;
                    Lightness_3(2) := 0.5;
                    Saturation_3(0) := 1.0;
                    Saturation_3(1) := 1.0;
                    Saturation_3(2) := 1.0;
                    Alt_Hue_Path_3(0) := False;
                    Alt_Hue_Path_3(1) := False;
                    Alt_Hue_Path_3(2) := False;
                    plscmap1l(HLS, Controls_3, Hue_3, Lightness_3, Saturation_3, Alt_Hue_Path_3);
                end;
        end case;
    end Quick_Set_Color_Map_1;


--------------------------------------------------------------------------------
--        Auxiliary things                                                    --
--------------------------------------------------------------------------------

    -- This is a mask function for Shade_Regions (aka plshades) et al that always
    -- returns 1 so that all points are plotted. Can be used as a template
    -- for other user-written mask functions. This behaves the same as
    -- when passing null for the second argument in Shade_Regions.
    function Mask_Function_No_Mask(x, y : Long_Float) return Integer is
    begin
        return 1;
    end Mask_Function_No_Mask;


    -- Given an array to hold contour levels and function minimum and maximum,
    -- fill it and return. Useful for contour and shade plots.
    procedure Calculate_Contour_Levels
       (Contour_Levels : in out Real_Vector;
        z_Min, z_Max : Long_Float) is

        step : Long_Float;
        ii : Integer;
    begin
        step := (z_Max - z_Min) / Long_Float(Contour_Levels'Last - Contour_Levels'First);
        for i in Contour_Levels'range loop
            ii := i - Contour_Levels'First; -- reference back to 0.
            Contour_Levels(i) := z_Min + step * Long_Float(ii);
        end loop;
    end Calculate_Contour_Levels;


--------------------------------------------------------------------------------
--        Re-define PLplot procedures using Ada style.                        --
--------------------------------------------------------------------------------

-- These correspond to the section in plot.h called "Function Prototypes".

    -- set the format of the contour labels
    procedure pl_setcontlabelformat
       (Limit_Exponent     : Integer := 4;
        Significant_Digits : Integer := 2) is
    begin
        PLplot_Thin.pl_setcontlabelformat(Limit_Exponent, Significant_Digits);
    end pl_setcontlabelformat;


    -- set offset and spacing of contour labels
    procedure pl_setcontlabelparam
       (Label_Offset       : Long_Float := 0.006; -- Units are ???
        Label_Font_Height  : Long_Float := 0.3;   -- Units are ???
        Label_Spacing      : Long_Float := 0.1;   -- Units are???
        Labels_Active      : Boolean := False) is

        active : Integer;

    begin
        if Labels_Active then
            active := 1;
        else
            active := 0;
        end if;
        PLplot_Thin.pl_setcontlabelparam(Label_Offset, Label_Font_Height, Label_Spacing, active);
    end pl_setcontlabelparam;


    -- Advance to subpage "page", or to the next one if "page" = 0.
    procedure pladv(Page : Natural) is
    begin
        PLplot_Thin.pladv(Page);
    end pladv;


    -- Plot an arc.
    procedure plarc
       (x, y, a, b, angle1, angle2, rotate : Long_Float;
        fill : Boolean) is

        fill_arc : PLBOOL;

    begin
        if fill then
            fill_arc := PLtrue;
        else
            fill_arc := PLfalse;
        end if;
        PLplot_Thin.plarc(x, y, a, b, angle1, angle2, rotate, fill_arc);
    end plarc;


    -- Draw a 2D vector plot.
    procedure plvect
       (u, v                             : Real_Matrix;
        Scale                            : Long_Float;
        Transformation_Procedure_Pointer : Transformation_Procedure_Pointer_Type;
        Transformation_Data_Pointer      : PL_Pointer) is
    begin
        PLplot_Thin.plvect(Matrix_To_Pointers(u), Matrix_To_Pointers(v), u'Length(1), u'Length(2),
            Scale, Transformation_Procedure_Pointer, Transformation_Data_Pointer);
    end plvect;


    -- Set the style for the arrow used by plvect to plot vectors.
    procedure plsvect
       (X_Vertices, Y_Vertices : Real_Vector; -- Should be range -0.5..0.5
        Fill_Arrow             : Boolean) is

        fill : PLBOOL;

    begin
        if Fill_Arrow then
            fill := PLtrue;
        else
            fill := PLfalse;
        end if;
        PLplot_Thin.plsvect(X_Vertices, Y_Vertices, X_Vertices'length, fill);
    end plsvect;


    -- Set the default style for the arrow used by plvect to plot vectors.
    procedure plsvect
       (X_Vertices, Y_Vertices : PL_Pointer;
        Fill_Arrow : Boolean) is

        fill : PLBOOL;

    begin
        if Fill_Arrow then
            fill := PLtrue;
        else
            fill := PLfalse;
        end if;
        PLplot_Thin.plsvectdefault(X_Vertices, Y_Vertices, 0, fill);
    end plsvect;


    -- Simple method to set the default style for the arrow used by plvect to plot vectors.
    -- This is not part of the C API and is Ada-specific.
    procedure plsvect is
    begin
        PLplot_Thin.plsvectdefault(System.Null_Address, System.Null_Address, 0, PLfalse);
    end plsvect;


    -- Another simple method to set the default style for the arrow used by plvect to plot vectors.
    -- This is not part of the C API and is Ada-specific.
    procedure Reset_Vector_Arrow_Style is
    begin
        PLplot_Thin.plsvectdefault(System.Null_Address, System.Null_Address, 0, PLfalse);
    end Reset_Vector_Arrow_Style;


    -- This functions similarly to plbox() except that the origin of the axes
    -- is placed at the user-specified point (x0, y0).
    procedure plaxes
       (X_Origin, Y_Origin       : Long_Float;
        X_Option_String          : String;
        X_Major_Tick_Interval    : Long_Float;
        X_Number_Of_Subintervals : Natural;
        Y_Option_String          : String;
        Y_Major_Tick_Interval    : Long_Float;
        Y_Number_Of_Subintervals : Natural) is
    begin
        PLplot_Thin.plaxes
           (X_Origin, Y_Origin,
            To_C(X_Option_String, True), X_Major_Tick_Interval, X_Number_Of_Subintervals,
            To_C(Y_Option_String, True), Y_Major_Tick_Interval, Y_Number_Of_Subintervals);
    end plaxes;


    -- Plot a histogram using x to store data values and y to store frequencies
    procedure plbin
       (Bin_Values     : Real_Vector; -- "x"
        Bin_Counts     : Real_Vector; -- "y"
        Options        : Integer) is -- Options are not defined in plplot.h.
    begin
        PLplot_Thin.plbin(Bin_Values'length, Bin_Values, Bin_Counts, Options);
    end plbin;


    -- Calculate broken-down time from continuous time for current stream.
    procedure plbtime
       (year, month, day, hour, min : out Integer;
        sec                         : out Long_Float;
        ctime                       : Long_Float) is
    begin
        PLplot_Thin.plbtime(year, month, day, hour, min, sec, ctime);
    end plbtime;


    -- Start new page. Should only be used with pleop().
    procedure plbop is
    begin
        PLplot_Thin.plbop;
    end plbop;


    -- This draws a box around the current viewport.
    procedure plbox
       (X_Option_String          : String;
        X_Major_Tick_Interval    : Long_Float;
        X_Number_Of_Subintervals : Natural := 0;
        Y_Option_String          : String;
        Y_Major_Tick_Interval    : Long_Float;
        Y_Number_Of_Subintervals : Natural := 0) is
    begin
        PLplot_Thin.plbox
           (To_C(X_Option_String, True), X_Major_Tick_Interval, X_Number_Of_Subintervals,
            To_C(Y_Option_String, True), Y_Major_Tick_Interval, Y_Number_Of_Subintervals);
    end plbox;


    -- This is the 3-d analogue of plbox().
    procedure plbox3
       (X_Option_String          : String;
        X_Label                  : String := To_String(Default_Label_String);
        X_Major_Tick_Interval    : Long_Float := 0.0;
        X_Number_Of_Subintervals : Natural := 0;

        Y_Option_String          : String;
        Y_Label                  : String := To_String(Default_Label_String);
        Y_Major_Tick_Interval    : Long_Float := 0.0;
        Y_Number_Of_Subintervals : Natural := 0;

        Z_Option_String          : String;
        Z_Label                  : String := To_String(Default_Label_String);
        Z_Major_Tick_Interval    : Long_Float := 0.0;
        Z_Number_Of_Subintervals : Natural := 0) is
    begin
        PLplot_Thin.plbox3
           (To_C(X_Option_String, True), To_C(X_Label, True), X_Major_Tick_Interval, X_Number_Of_Subintervals,
            To_C(Y_Option_String, True), To_C(Y_Label, True), Y_Major_Tick_Interval, Y_Number_Of_Subintervals,
            To_C(Z_Option_String, True), To_C(Z_Label, True), Z_Major_Tick_Interval, Z_Number_Of_Subintervals);
    end plbox3;


    -- Calculate world coordinates and subpage from relative device coordinates.
    procedure plcalc_world
       (x_Relative, y_Relative : Long_Float_0_1_Type;
        x_World,    y_World    : out Long_Float;
        Last_Window_Index      : out Integer) is
    begin
        PLplot_Thin.plcalc_world(x_Relative, y_Relative, x_World, y_World, Last_Window_Index);
    end plcalc_world;


    -- Clear current subpage.
    procedure plclear is
    begin
        PLplot_Thin.plclear;
    end plclear;


    -- Set color, map 0. Argument is integer between 0 and 15.
    procedure plcol0(A_Color : Plot_Color_Type) is
    begin
        PLplot_Thin.plcol0(A_Color);
    end plcol0;


    -- Set color, map 1. Argument is a float between 0. and 1.
    procedure plcol1(Color : Long_Float_0_1_Type) is
    begin
        PLplot_Thin.plcol1(Color);
    end plcol1;


    -- Configure transformation between continuous and broken-down time (and
    -- vice versa) for current stream.
    procedure plconfigtime
       (skale, offset1, offset2      : Long_Float;
        ccontrol                     : Integer;
        ifbtime_offset               : Boolean;
        year, month, day, hour, min : Integer;
        sec                          : Long_Float) is

        ifbtime_offset_As_Integer : Integer;
    begin
        if ifbtime_offset then
            ifbtime_offset_As_Integer := 1;
        else
            ifbtime_offset_As_Integer := 0;
        end if;
        PLplot_Thin.plconfigtime(skale, offset1, offset2, ccontrol,
            ifbtime_offset_As_Integer, year, month, day, hour, min, sec);
    end plconfigtime;


    -- Draws a contour plot from data in f(nx,ny). Is just a front-end to
    -- plfcont, with a particular choice for f2eval and f2eval_data.

    -- (Universal version using System.Address to the transformation data)
    procedure plcont
       (z                                : Real_Matrix;
        x_Min_Index, x_Max_Index         : Integer;
        y_Min_Index, y_Max_Index         : Integer;
        Contour_Levels                   : Real_Vector;
        Transformation_Procedure_Pointer : Transformation_Procedure_Pointer_Type;
        Transformation_Data_Pointer      : PL_Pointer) is
    begin
        PLplot_Thin.plcont(Matrix_To_Pointers(z), z'Length(1), z'Length(2),
            x_Min_Index, x_Max_Index, y_Min_Index, y_Max_Index, Contour_Levels,
            Contour_Levels'Length, Transformation_Procedure_Pointer,
            Transformation_Data_Pointer);
    end plcont;


    -- The procedure plfcont is not documented and is not part of the API.
    -- However, it is a very useful capability to have available.
    -- I have tried to implement it as I think was intended but this may be incorrect.
    -- It appears as though the intent is to pass the arbitrarily organized
    -- data (pointed to by Irregular_Data_Pointer) as a (single) pointer to a
    -- 2D C-style array. Thus, for examaple, it is not possible to pass the data
    -- as triples.
    -- For further conversion insight, see plcont, above.

    -- Draws a contour plot using the function evaluator f2eval and data stored
    -- by way of the f2eval_data pointer. This allows arbitrary organizations
    -- of 2d array data to be used.

    procedure plfcont
       (Function_Evaluator_Pointer       : Function_Evaluator_Pointer_Type;
        Irregular_Data                   : Real_Matrix;
        x_Min_Index, x_Max_Index         : Integer;
        y_Min_Index, y_Max_Index         : Integer;
        Contour_Levels                   : Real_Vector;
        Transformation_Procedure_Pointer : Transformation_Procedure_Pointer_Type;
        Transformation_Data              : Transformation_Data_Type) is

        -- This is a good place to change from the convenient form of passing
        -- the transformation data (a record) to the form required by the PLplot
        -- API (a pointer); same for the irregularly spaced data matrix.
        Transformation_Data_Address : PL_Pointer;
        Irregular_Data_Address      : PL_Pointer;
    begin
        Transformation_Data_Address := Transformation_Data'Address;
        Irregular_Data_Address      := Irregular_Data'Address;

        PLplot_Thin.plfcont(Function_Evaluator_Pointer, Irregular_Data_Address,
            Irregular_Data'Length(1), Irregular_Data'Length(2),
            x_Min_Index, x_Max_Index, y_Min_Index, y_Max_Index,
            Contour_Levels, Contour_Levels'Length,
            Transformation_Procedure_Pointer, Transformation_Data_Address);
    end plfcont;


    -- Copies state parameters from the reference stream to the current stream.
    procedure plcpstrm
       (Stream_ID                      : Integer;
        Do_Not_Copy_Device_Coordinates : Boolean) is

        PL_Do_Not_Copy_Device_Coordinates : PLBOOL;

    begin
        if Do_Not_Copy_Device_Coordinates then
            PL_Do_Not_Copy_Device_Coordinates := PLtrue;
        else
            PL_Do_Not_Copy_Device_Coordinates := PLfalse;
        end if;
        PLplot_Thin.plcpstrm(Stream_ID, PL_Do_Not_Copy_Device_Coordinates);
    end plcpstrm;


    -- Calculate continuous time from broken-down time for current stream.
    procedure plctime
       (year, month, day, hour, min : Integer;
        sec                         : Long_Float;
        ctime                       : out Long_Float) is
    begin
        PLplot_Thin.plctime(year, month, day, hour, min, sec, ctime);
    end plctime;


    -- fix this
    -- Converts input values from relative device coordinates to relative plot
    -- coordinates.
    procedure pldid2pc is
    begin
        Put_Line("Not implemented due to lack of documentation.");
    end pldid2pc;


    -- fix this
    -- Converts input values from relative plot coordinates to relative
    -- device coordinates.
    procedure pldip2dc is
    begin
        Put_Line("Not implemented due to lack of documentation.");
    end pldip2dc;


    -- End a plotting session for all open streams.
    procedure plend is
    begin
        PLplot_Thin.plend;
    end plend;


    -- End a plotting session for the current stream only.
    procedure plend1 is
    begin
        PLplot_Thin.plend1;
    end plend1;


    -- Simple interface for defining viewport and window.
    procedure plenv
       (x_Min, x_Max, y_Min, y_Max : Long_Float;
        Justification              : Justification_Type;
        Axis_Style                 : Axis_Style_Type) is
    begin
        PLplot_Thin.plenv(x_Min, x_Max, y_Min, y_Max, Justification, Axis_Style);
    end plenv;


    -- similar to plenv() above, but in multiplot mode does not advance the subpage,
    -- instead the current subpage is cleared
    procedure plenv0
       (x_Min, x_Max, y_Min, y_Max : Long_Float;
        Justification              : Justification_Type;
        Axis_Style                 : Axis_Style_Type) is
    begin
        PLplot_Thin.plenv0(x_Min, x_Max, y_Min, y_Max, Justification, Axis_Style);
    end plenv0;


    -- End current page. Should only be used with plbop().
    procedure pleop is
    begin
        PLplot_Thin.pleop;
    end pleop;


    -- Plot horizontal error bars (xmin(i),y(i)) to (xmax(i),y(i))
    procedure plerrx(x_Min, x_Max, y : Real_Vector) is
    begin
        PLplot_Thin.plerrx(x_Min'Length, x_Min, x_Max, y);
    end plerrx;


    -- Plot vertical error bars (x,ymin(i)) to (x(i),ymax(i))
    procedure plerry(x, y_Min, y_Max : Real_Vector) is
    begin
        PLplot_Thin.plerry(y_Min'Length, x, y_Min, y_Max);
    end plerry;


    -- Advance to the next family file on the next new page
    procedure plfamadv is
    begin
        PLplot_Thin.plfamadv;
    end plfamadv;

    -- Other "fill" routines similar to plfill could be written in Ada if
    -- desired, but if they are intended to be used as callbacks in subprograms
    -- such as plshade and plshades, they should be called with C
    -- calling conventions.

    -- Pattern fills the polygon bounded by the input points.
    procedure plfill(x, y : Real_Vector) is -- might have to put n:Integer first
    begin
        PLplot_Thin.plfill(x'Length, x, y);
    end plfill;


    -- Pattern fills the 3d polygon bounded by the input points.
    procedure plfill3 (x, y, z : Real_Vector) is
    begin
        PLplot_Thin.plfill3(x'Length, x, y, z);
    end plfill3;


    -- Flushes the output stream. Use sparingly, if at all.
    procedure plflush is
    begin
        PLplot_Thin.plflush;
    end plflush;


    -- Sets the global font flag to 'ifont'.
    procedure plfont(Font_Style : Font_Style_Type) is
    begin
        PLplot_Thin.plfont(Font_Style);
    end plfont;


    -- Load specified font set.
    procedure plfontld(Character_Set : Character_Set_Type) is
    begin
        PLplot_Thin.plfontld(Character_Set);
    end plfontld;


    -- Get character default height and current (scaled) height
    procedure plgchr(Default_Height, Current_Height : out Long_Float) is
    begin
        PLplot_Thin.plgchr(Default_Height, Current_Height);
    end plgchr;


    -- Returns 8 bit RGB values for given color from color map 0
    procedure plgcol0
       (Color_Index      : Integer;
        Red_Component, Green_Component, Blue_Component : out Integer) is
    begin
        PLplot_Thin.plgcol0(Color_Index, Red_Component, Green_Component, Blue_Component);
    end plgcol0;


    -- Returns 8 bit RGB values for given color from color map 0 and alpha value
    procedure plgcol0a
       (Color_Index                                    : Integer;
        Red_Component, Green_Component, Blue_Component : out Integer;
        Alpha                                          : out Long_Float_0_1_Type) is
    begin
        PLplot_Thin.plgcol0a(Color_Index, Red_Component, Green_Component, Blue_Component, Alpha);
    end plgcol0a;

    -- Returns the background color by 8 bit RGB value
    procedure plgcolbg
       (Red_Component, Green_Component, Blue_Component : out Integer) is
    begin
        PLplot_Thin.plgcolbg(Red_Component, Green_Component, Blue_Component);
    end plgcolbg;


    -- Returns the background color by 8 bit RGB value and alpha value
    procedure plgcolbga
       (Red_Component, Green_Component, Blue_Component : out Integer;
        Alpha                                          : out Long_Float_0_1_Type) is
    begin
        PLplot_Thin.plgcolbga(Red_Component, Green_Component, Blue_Component,
            Alpha);
    end;


    -- Returns the current compression setting
    procedure plgcompression(Compression_Level : out Integer) is
    begin
        PLplot_Thin.plgcompression(Compression_Level);
    end plgcompression;


    -- Make a function version of plgdev so that the caller can use it whereever
    -- a String type is expected without fooling around with conversions between
    -- Ada string types. See Example 14 for useage.
    -- This _replaces_ the procedure version.
    -- THIS IS NOT IN THE C API.

    -- Get the current device (keyword) name
    function plgdev return String is
        PL_Device_Name : char_array(0..79);
    begin
        PLplot_Thin.plgdev(PL_Device_Name);
        return To_Ada(PL_Device_Name, True);
    end plgdev;


    -- Retrieve current window into device space
    procedure plgdidev
       (Relative_Margin_Width : out Long_Float;
        Aspect_Ratio          : out Long_Float;
        x_Justification       : out Long_Float;
        y_Justification       : out Long_Float) is
    begin
        PLplot_Thin.plgdidev(Relative_Margin_Width, Aspect_Ratio, x_Justification, y_Justification);
    end plgdidev;


    -- Get plot orientation
    procedure plgdiori(Rotation : out Long_Float) is
    begin
        PLplot_Thin.plgdiori(Rotation);
    end plgdiori;


    -- Retrieve current window into plot space
    procedure plgdiplt
       (x_Min : out Long_Float;
        y_Min : out Long_Float;
        x_Max : out Long_Float;
        y_Max : out Long_Float) is
    begin
        PLplot_Thin.plgdiplt(x_Min, y_Min, x_Max, y_Max);
    end plgdiplt;


    -- Get FCI (font characterization integer)
    procedure plgfci(Font_Characterization_Integer : out Unicode) is
    begin
        PLplot_Thin.plgfci(Font_Characterization_Integer);
    end plgfci;


    -- Get family, style and weight of the current font
    procedure plgfont(Family, Style, Weight : out Integer) is
    begin
        PLplot_Thin.plgfont(Family, Style, Weight);
    end;


    -- Get family file parameters
    procedure plgfam
       (Family_Enabled : out Boolean;
        Family_File_Number : out Integer;
        Maximum_File_Size : out Integer) is

        PL_Family_Enabled : PLBOOL;

    begin
        PLplot_Thin.plgfam(PL_Family_Enabled, Family_File_Number, Maximum_File_Size);
        if PL_Family_Enabled = 0 then
            Family_Enabled := False;
        else
            Family_Enabled := True;
        end if;
    end plgfam;


    -- Make a function version of plgfnam so that the caller can use it whereever
    -- a String type is expected without fooling around with conversions between
    -- Ada string types. See Example 14 for useage.
    -- This _replaces_ the procedure version.
    -- THIS IS NOT IN THE C API.

    -- Get the (current) output file name.
    function plgfnam return String is
        PL_Output_File_Name : char_array(0..79);
    begin
        PLplot_Thin.plgfnam(PL_Output_File_Name);
        return To_Ada(PL_Output_File_Name, True);
    end plgfnam;


    -- Get the (current) run level.
    procedure plglevel(Run_Level : out Integer) is
    begin
        PLplot_Thin.plglevel(Run_Level);
    end plglevel;


    -- Get output device parameters.
    procedure plgpage
       (x_Pixels, y_Pixels           : out Long_Float;
        x_Page_Length, y_Page_Length : out Integer;
        x_Page_Offset, y_Page_Offset : out Integer) is
    begin
        PLplot_Thin.plgpage(x_Pixels, y_Pixels, x_Page_Length, y_Page_Length, x_Page_Offset, y_Page_Offset);
    end plgpage;


    -- Switches to graphics screen.
    procedure plgra is
    begin
        PLplot_Thin.plgra;
    end plgra;


    -- Draw gradient in polygon.
    procedure plgradient(x, y : Real_Vector; Angle : Long_Float) is
    begin
        plgradient(x'Length, x, y, Angle);
    end plgradient;


    -- Grid irregularly sampled data.
    procedure plgriddata
       (x, y, z                : Real_Vector; -- ungridded x- and y-points; z is height
        x_Grid, y_Grid         : Real_Vector;
        z_Gridded              : in out Real_Matrix;
        Gridding_Algorithm     : Gridding_Algorithm_Type;
        Griding_Algorithm_Data : Long_Float) is
    begin
        -- Note that since z_Gridded is converted to and passed as an array of
        -- pointers, this is effectively the same as pass-by-reference so
        -- "in out" is not required for the formal argument zg in plgriddata.
        PLplot_Thin.plgriddata(x, y, z, x'Length, x_Grid, x_Grid'Length, y_Grid, y_Grid'Length,
            Matrix_To_Pointers(z_Gridded), Gridding_Algorithm, Griding_Algorithm_Data);
    end plgriddata;


    -- Get subpage boundaries in absolute coordinates
    -- Results are millimeters from the lower left corner.
    procedure plgspa
       (Left_Edge, Right_Edge, Bottom_Edge, Top_Edge : out Long_Float) is
    begin
        PLplot_Thin.plgspa(Left_Edge, Right_Edge, Bottom_Edge, Top_Edge);
    end plgspa;


    -- Get current stream number.
    procedure plgstrm(Stream_Number : out Integer) is
    begin
        PLplot_Thin.plgstrm(Stream_Number);
    end plgstrm;


    -- Make a function version of plgver so that the caller can use it whereever
    -- a String type is expected without fooling around with conversions between
    -- Ada string types. See Example 14 for useage.
    -- This _replaces_ the procedure version.
    -- THIS IS NOT IN THE C API.

    -- Get the current library version number
    function plgver return String is
        PL_Version_Number : char_array(0..79);
    begin
        PLplot_Thin.plgver(PL_Version_Number);
        return To_Ada(PL_Version_Number, True);
    end plgver;


    -- Get viewport boundaries in normalized device coordinates
    procedure plgvpd(x_Min, x_Max, y_Min, y_Max : out Long_Float) is
    begin
        PLplot_Thin.plgvpd(x_Min, x_Max, y_Min, y_Max);
    end plgvpd;


    -- Get viewport boundaries in world coordinates
    procedure plgvpw(x_Min, x_Max, y_Min, y_Max : out Long_Float) is
    begin
        PLplot_Thin.plgvpw(x_Min, x_Max, y_Min, y_Max);
    end plgvpw;


    -- Get x axis labeling parameters
    procedure plgxax(Max_Digits, Actual_Digits : out Integer) is
    begin
        PLplot_Thin.plgxax(Max_Digits, Actual_Digits);
    end plgxax;


    -- Get y axis labeling parameters
    procedure plgyax(Max_Digits, Actual_Digits : out Integer) is
    begin
        PLplot_Thin.plgyax(Max_Digits, Actual_Digits);
    end plgyax;


    -- Get z axis labeling parameters
    procedure plgzax(Max_Digits, Actual_Digits : out Integer) is
    begin
        PLplot_Thin.plgzax(Max_Digits, Actual_Digits);
    end plgzax;


    -- Draws a histogram of n values of a variable in array data[0..n-1]
    procedure plhist
       (Data           : Real_Vector;
        Data_Min       : Long_Float; -- left  edge of left-most bin
        Data_Max       : Long_Float; -- right edge of right-most bin
        Number_Of_Bins : Positive; -- equal-sized, between Data_Min and Data_Max
        Options : Integer) is -- Options are not defined in plplot.h.
    begin
        PLplot_Thin.plhist
           (Data'length, Data, Data_Min, Data_Max, Number_Of_Bins, Options);
    end plhist;


    -- Functions for converting between HLS and RGB color space
    procedure plhlsrgb
       (Hue_Component        : Long_Float;
        Lightness_Component  : Long_Float_0_1_Type;
        Saturation_Component : Long_Float_0_1_Type;
        Red_Component        : out Long_Float_0_1_Type;
        Green_Component      : out Long_Float_0_1_Type;
        Blue_Component       : out Long_Float_0_1_Type) is
    begin
        PLplot_Thin.plhlsrgb(Hue_Component, Lightness_Component, Saturation_Component, Red_Component, Green_Component, Blue_Component);
    end plhlsrgb;


    -- Initialization. Must be called before starting plot.
    -- Certain other procedures, if used, must be called first.
    -- Initializes PLplot, using preset or default options
    procedure plinit is
    begin
        PLplot_Thin.plinit;
    end plinit;


    -- Draws a line segment from (x1, y1) to (x2, y2).
    procedure pljoin(x1, y1, x2, y2 : Long_Float) is
    begin
        PLplot_Thin.pljoin(x1, y1, x2, y2);
    end pljoin;


    -- Simple routine for labelling graphs.
    procedure pllab(X_Label, Y_Label, Title_Label : String := To_String(Default_Label_String)) is
    begin
        PLplot_Thin.pllab(To_C(X_Label, True), To_C(Y_Label, True), To_C(Title_Label, True));
    end pllab;


    -- Arrays that could have elements of Plot_Color_Type are merely arrays of
    -- integers; we have not defined special arrays (e.g., array(somerange) of
    -- Plot_Color_Type) for the arguments Text_Colors, Box_Colors, Line_Colors,
    -- or Symbol_Colors. Similarly for Entry_Options which could be an array
    -- of Legend_Flag_Type and some other arguments. fixme
    -- Routine for drawing discrete line, symbol, or cmap0 legends
    procedure pllegend
       (Legend_Width, Legend_Height           : out Long_Float;
        Options, Position                     : Integer;
        X_Offset, Y_Offset                    : Long_Float;
        Plot_Area_Width                       : Long_Float;
        Background_Color, Bounding_Box_Color  : Plot_Color_Type;
        Bounding_Box_Style                    : Line_Style_Type;
        Number_Rows, Number_Columns           : Integer;
        Entry_Options                         : Integer_Array_1D;
        Text_Offset, Text_Scale, Text_Spacing : Long_Float;
        Text_Justification                    : Long_Float;
        Text_Colors                           : Integer_Array_1D;
        Label_Text                            : Legend_String_Array_Type;
        Box_Colors, Box_Patterns              : Integer_Array_1D;
        Box_Scales                            : Real_Vector;
        Box_Line_Widths                       : Real_Vector;
        Line_Colors, Line_Styles              : Integer_Array_1D;
        Line_Widths                           : Real_Vector;
        Symbol_Colors                         : Integer_Array_1D;
        Symbol_Scales                         : Real_Vector;
        Symbol_Numbers                        : Integer_Array_1D;
        Symbols                               : Legend_String_Array_Type)
    is
        Number_Entries : Integer := Label_Text'length;
        L : Integer; -- Used to check lengths of arrays.
        PL_Label_Text, PL_Symbols : PL_Legend_String_Array(Label_Text'range);
        C_Legend_String_Array  : array(Label_Text'range) of PL_Legend_String;
        C_Symbols_String_Array : array(Symbols'range)    of PL_Legend_String;
        Dum_Text : Legend_String_Array_Type(Label_Text'range);
    begin
        -- Check that all array lengths in the argument list are the same.
        L := Entry_Options'length;
        if L /= Text_Colors'length or L /= Label_Text'length or L /= Box_Colors'length or
            L /= Box_Patterns'length or L /= Box_Scales'length or
            L /= Box_Line_Widths'length or L /= Line_Colors'length or L /= Line_Styles'length or
            L /= Line_Widths'length or L /= Symbol_Colors'length or L /= Symbol_Scales'length or
            L /= Symbol_Numbers'length or L /= Symbols'length
        then
            Put_Line("*** WARNING: Mismatched array lengths at pllegend");
        end if;

        -- Adapt Label_Text and Symbols to C. See the comment at plstripc.
        -- Adapt Label_Text first.
        for I in Label_Text'range loop

            -- Check length and adjust if necessary.
            if Length(Label_Text(I)) >= Max_Legend_Label_Length then
                Put_Line("*** Warning: Legend label was truncated to"
                    & Integer'Image(Max_Legend_Label_Length) & " characters. ***");
                Dum_Text(I) := Head(Label_Text(I), Max_Legend_Label_Length);
            else
                Dum_Text(I) := Label_Text(I);
            end if;

            -- Make the C-style string with null character and spaces immediately after the text.
            C_Legend_String_Array(I) := To_C(To_String(Dum_Text(I)
                & Character'val(0)
                & (Max_Legend_Label_Length - Length(Dum_Text(I))) * " "), False);

            -- Set the I-th pointer in the array of pointers.
            PL_Label_Text(I) := C_Legend_String_Array(I)'Address;
        end loop;

        -- Adapt Symbols next.
        for I in Symbols'range loop

            -- Check length and adjust if necessary.
            if Length(Symbols(I)) >= Max_Legend_Label_Length then
                Put_Line("*** Warning: Legend symbols label was truncated to"
                    & Integer'Image(Max_Legend_Label_Length) & " characters. ***");
                Dum_Text(I) := Head(Symbols(I), Max_Legend_Label_Length);
            else
                Dum_Text(I) := Symbols(I);
            end if;

            -- Make the C-style string with null character and spaces immediately after the text.
            C_Symbols_String_Array(I) := To_C(To_String(Dum_Text(I)
                & Character'val(0)
                & (Max_Legend_Label_Length - Length(Dum_Text(I))) * " "), False);

            -- Set the I-th pointer in the array of pointers.
            PL_Symbols(I) := C_Symbols_String_Array(I)'Address;
        end loop;

        PLplot_Thin.pllegend(Legend_Width, Legend_Height, Options, Position,
            X_Offset, Y_Offset, Plot_Area_Width, Background_Color, Bounding_Box_Color,
            Bounding_Box_Style, Number_Rows, Number_Columns, Number_Entries,
            Entry_Options, Text_Offset, Text_Scale, Text_Spacing, Text_Justification,
            Text_Colors, PL_Label_Text, Box_Colors, Box_Patterns, Box_Scales,
            Box_Line_Widths, Line_Colors, Line_Styles, Line_Widths, Symbol_Colors,
            Symbol_Scales, Symbol_Numbers, PL_Symbols);
    end pllegend;


    -- Routine for drawing continuous colour legends
    procedure plcolorbar
       (Colorbar_Width, Colorbar_Height      : out Long_Float;
        Options, Position                    : Integer;
        X_Offset, Y_Offset                   : Long_Float;
        X_Length, Y_Length                   : Long_Float;
        Background_Color, Bounding_Box_Color : Plot_Color_Type;
        Bounding_Box_Style                   : Line_Style_Type;
        Low_Cap_Color, High_Cap_Color        : Long_Float;
        Contour_Color_For_Shade              : Plot_Color_Type;
        Contour_Width_For_Shade              : Long_Float;
        Label_Options                        : Integer_Array_1D;
        Label_Text                           : Legend_String_Array_Type;
        Axis_Options                         : Legend_String_Array_Type;
        Tick_Spacing                         : Real_Vector;
        Number_Subticks                      : Integer_Array_1D;
        Number_Values                        : Integer_Array_1D;
        Values                               : Real_Matrix)
    is
        Number_Labels : Integer := Label_Options'length;
        Number_Axes : Integer := Axis_Options'length;
        PL_Label_Text : PL_Legend_String_Array(Label_Text'range);
        C_Legend_String_Array  : array(Label_Text'range) of PL_Legend_String;
        PL_Axis_Options : PL_Legend_String_Array(Axis_Options'range);
        C_Axis_Options_String_Array  : array(Axis_Options'range) of PL_Legend_String;
        Dum_Text : Legend_String_Array_Type(Label_Text'range);
        PL_Values : Long_Float_Pointer_Array(values'range(2));
    begin
        -- Check some array lengths.
        if Number_Labels /= Label_Text'length then
            Put_Line("*** WARNING: Mismatched array lengths at plcolorbar labels");
        end if;
        if Number_Axes /= Tick_Spacing'length or Number_Axes /= Number_Subticks'length
            or Number_Axes /= Values'length(1) then
            Put_Line("*** WARNING: Mismatched array lengths at plcolorbar values");
        end if;

        -- Adapt Label_Text and Axis_Options to C. See the comment at Create_Stripchart.
        -- Adapt Label_Text first.
        for I in Label_Text'range loop

            -- Check length and adjust if necessary.
            if Length(Label_Text(I)) >= Max_Legend_Label_Length then
                Put_Line("*** Warning: Colorbar label was truncated to"
                    & Integer'Image(Max_Legend_Label_Length) & " characters. ***");
                Dum_Text(I) := Head(Label_Text(I), Max_Legend_Label_Length);
            else
                Dum_Text(I) := Label_Text(I);
            end if;

            -- Make the C-style string with null character and spaces immediately after the text.
            C_Legend_String_Array(I) := To_C(To_String(Dum_Text(I)
                & Character'val(0)
                & (Max_Legend_Label_Length - Length(Dum_Text(I))) * " "), False);

            -- Set the I-th pointer in the array of pointers.
            PL_Label_Text(I) := C_Legend_String_Array(I)'Address;
        end loop;

        -- Adapt Axis_Options next.
        for I in Axis_Options'range loop

            -- Check length and adjust if necessary.
            if Length(Axis_Options(I)) >= Max_Legend_Label_Length then
                Put_Line("*** Warning: Colorbar asix options label was truncated to"
                    & Integer'Image(Max_Legend_Label_Length) & " characters. ***");
                Dum_Text(I) := Head(Axis_Options(I), Max_Legend_Label_Length);
            else
                Dum_Text(I) := Axis_Options(I);
            end if;

            -- Make the C-style string with null character and spaces immediately after the text.
            C_Axis_Options_String_Array(I) := To_C(To_String(Dum_Text(I)
                & Character'val(0)
                & (Max_Legend_Label_Length - Length(Dum_Text(I))) * " "), False);

            -- Set the I-th pointer in the array of pointers.
            PL_Axis_Options(I) := C_Axis_Options_String_Array(I)'Address;
        end loop;

        -- Adapt values in a similar way as Label_Text: make an array of pointers to Real_Vector.
        -- We need to transpose because we present the Values matrix to the user as described in the
        -- PLplot documentation for plcolorbar as rows indexing axes and columns indexing the axis
        -- values, and Matrix_To_Pointers wants things the other way.
        PL_Values := Matrix_To_Pointers(PLplot_Thin.PL_Transpose(Values));
        PLplot_Thin.plcolorbar(Colorbar_Width, Colorbar_Height, Options, Position, X_Offset,
            Y_Offset, X_Length, Y_Length, Background_Color, Bounding_Box_Color, Bounding_Box_Style,
            Low_Cap_Color, High_Cap_Color, Contour_Color_For_Shade, Contour_Width_For_Shade,
            Number_Labels, Label_Options, PL_Label_Text, Number_Axes,
            PL_Axis_Options, Tick_Spacing, Number_Subticks, Number_Values, PL_Values);
    end plcolorbar;


    -- Sets position of the light source
    procedure pllightsource
       (x : Long_Float := 1.0;
        y: Long_Float := 1.0;
        z: Long_Float := 1.0) is
    begin
        PLplot_Thin.pllightsource(x, y, z);
    end pllightsource;


    -- Draws line segments connecting a series of points.
    procedure plline(x, y : Real_Vector) is
    begin
        PLplot_Thin.plline(x'Length, x, y);
    end plline;


    -- Draws a line in 3 space.
    procedure plline3(x, y, z : Real_Vector) is
    begin
        PLplot_Thin.plline3(x'Length, x, y, z);
    end plline3;


    -- Set line style.
    procedure pllsty(Line_Style : Line_Style_Type) is
    begin
        PLplot_Thin.pllsty(Line_Style);
    end pllsty;


    -- fix this See comment in Example 19, x19a.adb or xthick19a.adb for how to
    -- possibly eliminate the need to pass array size as the first argument in
    -- the function pointed to by Map_Form_Function_Pointer. Ditto for plmeridians.

    -- Plot continental outline in world coordinates.
    procedure plmap
       (Map_Form_Function_Pointer            : Map_Form_Function_Pointer_Type;
        Map_Kind                             : Map_Type;
        Minimum_Longitude, Maximum_Longitude : Long_Float;
        Minimum_Latitude,  Maximum_Latitude  : Long_Float) is

        Map_Kind_String : Unbounded_String;
    begin
        -- Convert Map_Kind into a string; later, a C string.
        case Map_Kind is
            when Continents =>
                begin
                    Map_Kind_String := TUB("globe");
                end;
            when USA_and_States =>
                begin
                    Map_Kind_String := TUB("usa");
                end;
            when Continents_and_Countries =>
                begin
                    Map_Kind_String := TUB("cglobe");
                end;
            when USA_States_and_Continents =>
                begin
                    Map_Kind_String := TUB("usaglobe");
                end;
        end case;

        PLplot_Thin.plmap(Map_Form_Function_Pointer, To_C(To_String(Map_Kind_String), True),
            Minimum_Longitude, Maximum_Longitude, Minimum_Latitude, Maximum_Latitude);
    end plmap;


    -- Plot map fills.
    procedure plmapfill
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries               : Integer_Array_1D) is
    begin
        PLplot_Thin.plmapfill(Map_Form_Function_Pointer, To_C(Shapefile_File_Name),
            Min_X, Max_X, Min_Y, Max_Y, Plot_Entries, Plot_Entries'Length);
    end plmapfill;


    -- Plot map fills: overload passes null pointer plotentries to emulate C, match documentaton.
    procedure plmapfill
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries               : PL_Pointer) is
    begin
        PLplot_Thin.plmapfill_Overload(Map_Form_Function_Pointer, To_C(Shapefile_File_Name),
            Min_X, Max_X, Min_Y, Max_Y, Plot_Entries, 0);
    end plmapfill;


    -- Plot map fills: overload that doesn't use plotentries to emulate C, match documentaton.
    procedure Plot_Shapefile_All
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float) is
    begin
        PLplot_Thin.plmapfill_Overload(Map_Form_Function_Pointer, To_C(Shapefile_File_Name),
            Min_X, Max_X, Min_Y, Max_Y, System.Null_Address, 0);
    end Plot_Shapefile_All;


    -- Plot map outlines.
    procedure plmapline
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries               : Integer_Array_1D) is
    begin
        PLplot_Thin.plmapline(Map_Form_Function_Pointer, To_C(Shapefile_File_Name),
            Min_X, Max_X, Min_Y, Max_Y, Plot_Entries, Plot_Entries'Length);
    end plmapline;


    -- Plot map outlines: overload passes null pointer plotentries to emulate C, match documentaton.
    procedure plmapline
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries               : PL_Pointer) is
    begin
        PLplot_Thin.plmapline_Overload(Map_Form_Function_Pointer, To_C(Shapefile_File_Name),
            Min_X, Max_X, Min_Y, Max_Y, Plot_Entries, 0);
    end plmapline;


    -- Plot map outlines: overload that doesn't use plotentries to emulate C, match documentaton.
    procedure Plot_Shapefile_World_All
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float) is
    begin
        PLplot_Thin.plmapline_Overload(Map_Form_Function_Pointer, To_C(Shapefile_File_Name),
            Min_X, Max_X, Min_Y, Max_Y, System.Null_Address, 0);
    end Plot_Shapefile_World_All;


    -- Plot map points.
    procedure plmapstring
       (Map_Form_Function_Pointer : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries : Integer_Array_1D) is
    begin
        PLplot_Thin.plmapstring(Map_Form_Function_Pointer, To_C(Shapefile_File_Name),
            Min_X, Max_X, Min_Y, Max_Y, Plot_Entries, Plot_Entries'Length);
    end plmapstring;


    -- Plot map text.
    procedure plmaptex
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        dx, dy                     : Long_Float;
        Justification              : Long_Float;
        Text                       : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Which_Shapefile_String     : Integer) is
    begin
        Plplot_Thin.plmaptex(Map_Form_Function_Pointer, To_C(Shapefile_File_Name), dx, dy,
            Justification, To_C(Text), Min_X, Max_X, Min_Y, Max_Y, Which_Shapefile_String);
    end plmaptex;


    -- fix this See comment for plmap.

    -- Plot the latitudes and longitudes on the background.
    procedure plmeridians
       (Map_Form_Function_Pointer            : Map_Form_Function_Pointer_Type;
        Delta_Longitude, Delta_Latitude      : Long_Float;
        Minimum_Longitude, Maximum_Longitude : Long_Float;
        Minimum_Latitude,  Maximum_Latitude  : Long_Float) is
    begin
        PLplot_Thin.plmeridians(Map_Form_Function_Pointer, Delta_Longitude, Delta_Latitude,
            Minimum_Longitude, Maximum_Longitude, Minimum_Latitude, Maximum_Latitude);
    end plmeridians;


    -- Plots a mesh representation of the function z(x, y).
    procedure plmesh
       (x, y    : Real_Vector; -- surface definition points
        z       : Real_Matrix; -- height of surface at definition points
        Options : Integer) is
    begin
        PLplot_Thin.plmesh(x, y, Matrix_To_Pointers(z), x'Length, y'Length, Options);
    end plmesh;


    -- Plots a mesh representation of the function z(x, y) with contour
    procedure plmeshc -- prototype
       (x, y           : Real_Vector; -- surface definition points
        z              : Real_Matrix; -- height of surface at definition points
        Options        : Integer;
        Contour_Levels : Real_Vector) is -- levels at which to draw contours
    begin
        PLplot_Thin.plmeshc(x, y, Matrix_To_Pointers(z), x'Length, y'Length, Options, Contour_Levels, Contour_Levels'Length);
    end plmeshc;


    -- Creates a new stream and makes it the default.
    procedure plmkstrm(New_Stream_Number : out Integer) is
    begin
        PLplot_Thin.plmkstrm(New_Stream_Number);
    end plmkstrm;


    -- Prints out "text" at specified position relative to viewport
    procedure plmtex
       (Side : String;
        Position_From_Edge : Long_Float;
        Position_Along_Edge : Long_Float;
        Justification : Long_Float;
        The_Text : String) is
    begin
        PLplot_Thin.plmtex(To_C(Side, True), Position_From_Edge, Position_Along_Edge, Justification, To_C(The_Text, True));
    end plmtex;


    -- Prints out "The_Text" at specified position relative to viewport (3D)
    procedure plmtex3
       (Side : String;
        Position_From_Edge : Long_Float;
        Position_Along_Edge : Long_Float;
        Justification : Long_Float;
        The_Text : String) is
    begin
        PLplot_Thin.plmtex3(To_C(Side, True), Position_From_Edge, Position_Along_Edge, Justification, To_C(The_Text, True));
    end plmtex3;



    -- Plots a 3-d representation of the function z(x, y).
    procedure plot3d
       (x, y    : Real_Vector; -- surface definition points
        z       : Real_Matrix; -- height of surface at definition points
        Options : Integer;
        Sides   : Boolean) is -- draw sides?

        PLsides : PLBOOL;

    begin
        if Sides then
            PLsides := PLtrue;
        else
            PLsides := PLfalse;
        end if;
        PLplot_Thin.plot3d(x, y, Matrix_To_Pointers(z), x'Length, y'Length, Options, PLsides);
    end plot3d;


    -- Plots a 3-d representation of the function z(x, y) with contour.
    procedure plot3dc
       (x, y           : Real_Vector; -- surface definition points
        z              : Real_Matrix; -- height of surface at definition points
        Options        : Integer;
        Contour_Levels : Real_Vector) is -- levels at which to draw contours
    begin
        PLplot_Thin.plot3dc(x, y, Matrix_To_Pointers(z), x'Length, y'Length, Options, Contour_Levels, Contour_Levels'Length);
    end plot3dc;


    -- fix this; update variable names when documentation becomes available.
    -- Plots a 3-d representation of the function z(x, y) with contour and
    -- y index limits.
    procedure plot3dcl -- Lacks documentation in Chapter 17 of Ref. Man.
       (x, y                 : Real_Vector; -- surface definition points
        z                    : Real_Matrix; -- height of surface at definition points
        Options              : Integer;
        Contour_Levels       : Real_Vector;
        ixstart, ixn         : Integer;
        indexymin, indexymax : Integer_Array_1D) is -- levels at which to draw contours
    begin
        PLplot_Thin.plot3dcl(x, y, Matrix_To_Pointers(z), x'Length, y'Length, Options, Contour_Levels, Contour_Levels'Length, ixstart, ixn, indexymin, indexymax);
    end plot3dcl;


    -- Set fill pattern directly.
    procedure plpat
       (Inclinations : Integer_Array_1D;
        Spacings     : Integer_Array_1D) is
    begin
        PLplot_Thin.plpat(Inclinations'Length, Inclinations, Spacings);
    end plpat;


    -- Draw a line connecting two points, accounting for coordinate transform
    procedure plpath
       (Num_Segments   : Integer;
        x1, y1, x2, y2 : Long_Float) is
    begin
        PLplot_Thin.plpath(Num_Segments, x1, y1, x2, y2);
    end plpath;


    -- Plots array y against x for n points using ASCII code "code".
    procedure plpoin
       (x, y             : Real_Vector;
        Symbol_As_Number : Integer) is
    begin
        PLplot_Thin.plpoin(x'Length, x, y, Symbol_As_Number);
    end plpoin;



    -- Draws a series of points in 3 space.
    procedure plpoin3
       (x, y, z          : Real_Vector;
        Symbol_As_Number : Integer) is
    begin
        PLplot_Thin.plpoin3(x'Length, x, y, z, Symbol_As_Number);
    end plpoin3;


    -- Draws a polygon in 3 space.
    procedure plpoly3
       (x, y, z : Real_Vector;
        Draw_Segments : Boolean_Array_1D;
        Draw_Counterclockwise : Boolean) is

        PL_Draw_Segments : PL_Bool_Array(Draw_Segments'range);
        PL_Draw_Counterclockwise : PLBOOL;

    begin
        if Draw_Counterclockwise then
            PL_Draw_Counterclockwise := PLTrue;
        else
            PL_Draw_Counterclockwise := PLfalse;
        end if;

        for Index in Draw_Segments'range loop
            if Draw_Segments(Index) then
                PL_Draw_Segments(Index) := PLtrue;
            else
                PL_Draw_Segments(Index) := PLfalse;
            end if;
        end loop;
        PLplot_Thin.plpoly3(x'length, x, y, z, PL_Draw_Segments, PL_Draw_Counterclockwise);
    end plpoly3;


    -- Set the floating point precision (in number of places) in numeric labels.
    procedure plprec
       (Set_Manually : Boolean := True;
        Number_Digits_After_Decimal_Point : Integer := 2) is
    begin
        if Set_Manually then
            PLplot_Thin.plprec(1, Number_Digits_After_Decimal_Point);
        else
            PLplot_Thin.plprec(0, Number_Digits_After_Decimal_Point);
        end if;
    end plprec;


    -- Set fill pattern, using one of the predefined patterns.
    procedure plpsty(Fill_Pattern : Fill_Pattern_Type) is
    begin
        PLplot_Thin.plpsty(Fill_Pattern);
    end plpsty;


    -- Prints out "The_Text" at world cooordinate (x, y).
    procedure plptex
       (x, y             : Long_Float;
        Delta_X, Delta_Y : Long_Float;
        Justification    : Long_Float;
        The_Text         : String) is
    begin
        PLplot_Thin.plptex(x, y, Delta_X, Delta_Y, Justification, To_C(The_Text, True));
    end plptex;


    -- Prints out "The_Text" at world cooordinate (x, y, z).
    procedure plptex3
       (x, y, z                   : Long_Float;
        Delta_X, Delta_Y, Delta_Z : Long_Float;
        Shear_X, Shear_Y, Shear_Z : Long_Float;
        Justification             : Long_Float;
        The_Text                  : String) is
    begin
        PLplot_Thin.plptex3(x, y, z, Delta_X, Delta_Y, Delta_Z, Shear_X, Shear_Y, Shear_Z,
            Justification, To_C(The_Text, True));
    end plptex3;


    -- Random number generator based on Mersenne Twister.
    -- Obtain real random number in range [0,1].
    function plrandd return Long_Float is
    begin
        return PLplot_Thin.plrandd;
    end plrandd;


    -- Replays contents of plot buffer to current device/file.
    procedure plreplot is
    begin
        PLplot_Thin.plreplot;
    end plreplot;


    -- Functions for converting between HLS and RGB color space
    procedure plrgbhls
       (Red_Component        : Long_Float_0_1_Type;
        Green_Component      : Long_Float_0_1_Type;
        Blue_Component       : Long_Float_0_1_Type;
        Hue_Component        : out Long_Float;
        Lightness_Component  : out Long_Float_0_1_Type;
        Saturation_Component : out Long_Float_0_1_Type) is
    begin
        PLplot_Thin.plrgbhls(Red_Component, Green_Component, Blue_Component, Hue_Component, Lightness_Component, Saturation_Component);
    end plrgbhls;


    -- Set character height.
    procedure plschr(Default_Height, Scale_Factor : Long_Float) is
    begin
        PLplot_Thin.plschr(Default_Height, Scale_Factor);
    end plschr;


    -- Set color map 0 colors by 8 bit RGB values
    procedure plscmap0(Red_Components, Green_Components, Blue_Components : Integer_Array_1D) is
    begin
        PLplot_Thin.plscmap0(Red_Components, Green_Components, Blue_Components, Red_Components'Length);
    end plscmap0;


    -- Set color map 0 colors by 8 bit RGB values and alpha values
    procedure plscmap0a
       (Red_Components, Green_Components, Blue_Components : Integer_Array_1D;
        Alpha                                             : Real_Vector) is
    begin
        PLplot_Thin.plscmap0a(Red_Components, Green_Components, Blue_Components, Alpha, Red_Components'Length);
    end plscmap0a;


    -- Set number of colors in cmap 0
    procedure plscmap0n(Number_Of_Colors : Integer) is
    begin
        PLplot_Thin.plscmap0n(Number_Of_Colors);
    end plscmap0n;


    -- Set color map 1 colors by 8 bit RGB values
    procedure plscmap1
       (Red_Component, Green_Component, Blue_Component : Integer_0_255_Array) is

       PL_Red_Component, PL_Green_Component, PL_Blue_Component : Integer_Array_1D(Red_Component'range);
    begin
        -- Copy constrained color integers to unconstrained integers.
        for i in Red_Component'range loop -- Should use slices
            PL_Red_Component(i)   := Red_Component(i);
            PL_Green_Component(i) := Green_Component(i);
            PL_Blue_Component(i)  := Blue_Component(i);
        end loop;
        PLplot_Thin.plscmap1(PL_Red_Component, PL_Green_Component, PL_Blue_Component, PL_Red_Component'Length);
    end plscmap1;


    -- Set color map 1 colors by 8 bit RGB values
    procedure plscmap1a
       (Red_Component, Green_Component, Blue_Component : Integer_0_255_Array;
        Alpha                                          : Real_Vector) is

       PL_Red_Component, PL_Green_Component, PL_Blue_Component : Integer_Array_1D(Red_Component'range);
    begin
        -- Copy constrained color integers to unconstrained integers.
        for i in Red_Component'range loop -- Should use slices
            PL_Red_Component(i)   := Red_Component(i);
            PL_Green_Component(i) := Green_Component(i);
            PL_Blue_Component(i)  := Blue_Component(i);
        end loop;
        PLplot_Thin.plscmap1a(PL_Red_Component, PL_Green_Component, PL_Blue_Component, Alpha, PL_Red_Component'Length);
    end plscmap1a;


    -- Set color map 1 colors using a piece-wise linear relationship between
    -- intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.
    procedure plscmap1l
       (Color_Model    : Color_Model_Type; -- HLS or RGB
        Control_Points : Real_Vector; -- range 0.0 .. 1.0; not checked here
        H_Or_R         : Real_Vector; -- range 0.0 .. 1.0; not checked here
                                              -- Note: Hue is 0.0 .. 360.0.
        L_Or_G         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        S_Or_B         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alt_Hue_Path   : Boolean_Array_1D) is -- True means use alternative hue interpolation path which always includes the point hue = 0.  False reverses.

        PL_Color_Model : PLBOOL;
        PL_Alt_Hue_Path : PL_Bool_Array (Alt_Hue_Path'range);

    begin
        -- Check for consistent array lengths.
        if  Control_Points'length /= H_Or_R'length or Control_Points'length /= L_or_G'length or
            Control_Points'length /= S_Or_B'length or Control_Points'length /= Alt_Hue_Path'length
        then
            Put_Line("*** WARNING: Inconsistent array lengths when setting color map 1 ***");
        end if;

        if Color_Model = RGB then
            PL_Color_Model := PLtrue;
        else
            PL_Color_Model := PLfalse;
        end if;

        for i in Alt_Hue_Path'range loop
            if Alt_Hue_Path(i) then
                PL_Alt_Hue_Path(i) := PLtrue;
            else
                PL_Alt_Hue_Path(i) := PLfalse;
            end if;
        end loop;

        PLplot_Thin.plscmap1l(PL_Color_Model, Control_Points'Length, Control_Points, H_Or_R, L_Or_G, S_Or_B, PL_Alt_Hue_Path);
    end plscmap1l;


    -- Overloaded version of plscmap1l which allows simplified (alt_hue_path false) interpolation.
    -- This is an Ada-like way of doing what is described
    -- in the PLplot documentation when a C user passes a null pointer as the
    -- final argument instead of an array of booleans to indicate which
    -- of the two hue interpolation paths to take.
    procedure plscmap1l
       (Color_Model    : Color_Model_Type; -- HLS or RGB
        Control_Points : Real_Vector; -- range 0.0 .. 1.0; not checked here
        H_Or_R         : Real_Vector; -- range 0.0 .. 1.0; not checked here
                                              -- Note: Hue is 0.0 .. 360.0.
        L_Or_G         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        S_Or_B         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alt_Hue_Path    : Alt_Hue_Path_Type) is

        Alt_Hue_Path_Array : Boolean_Array_1D(Control_Points'range);
    begin
        if Alt_Hue_Path = Alt_Hue_Path_All then
            for i in Alt_Hue_Path_Array'range loop
                Alt_Hue_Path_Array(i) := True;
            end loop;
        elsif Alt_Hue_Path = Alt_Hue_Path_None then
            for i in Alt_Hue_Path_Array'range loop
                Alt_Hue_Path_Array(i) := False;
            end loop;
        end if;
        plscmap1l(Color_Model, Control_Points, H_Or_R, L_Or_G, S_Or_B, Alt_Hue_Path_Array);
    end plscmap1l;


    -- Set color map 1 colors using a piece-wise linear relationship between
    -- intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.
    -- Will also linear interpolate alpha values.
    procedure plscmap1la
       (Color_Model    : Color_Model_Type; -- HLS or RGB
        Control_Points : Real_Vector; -- range 0.0 .. 1.0; not checked here
        H_Or_R         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        L_Or_G         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        S_Or_B         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alpha          : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alt_Hue_Path    : Boolean_Array_1D) is -- True means use alternative hue interpolation path which always includes the point hue = 0.  False reverses.

        PL_Color_Model : PLBOOL;
        PL_Alt_Hue_Path : PL_Bool_Array (Alt_Hue_Path'range);

    begin
        -- Check for consistent array lengths.
        if  Control_Points'length /= H_Or_R'length or Control_Points'length /= L_or_G'length or
            Control_Points'length /= S_Or_B'length or Control_Points'length /= Alt_Hue_Path'length
        then
            Put_Line("*** WARNING: Inconsistent array lengths when setting color map 1 ***");
        end if;

        if Color_Model = RGB then
            PL_Color_Model := PLtrue;
        else
            PL_Color_Model := PLfalse;
        end if;

        for i in Alt_Hue_Path'range loop
            if Alt_Hue_Path(i) then
                PL_Alt_Hue_Path(i) := PLtrue;
            else
                PL_Alt_Hue_Path(i) := PLfalse;
            end if;
        end loop;

        PLplot_Thin.plscmap1la(PL_Color_Model, Control_Points'Length, Control_Points, H_Or_R, L_Or_G, S_Or_B, Alpha, PL_Alt_Hue_Path);
    end plscmap1la;


    -- Overloaded version of plscmap1la which allows simplified (alt_hue_path false) interpolation.
    -- This is an Ada-like way of doing what is described
    -- in the PLplot documentation when a C user passes a null pointer as the
    -- final argument instead of an array of booleans to indicate which
    -- of the two hue interpolation paths to take.
    procedure plscmap1la
       (Color_Model    : Color_Model_Type; -- HLS or RGB
        Control_Points : Real_Vector; -- range 0.0 .. 1.0; not checked here
        H_Or_R         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        L_Or_G         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        S_Or_B         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alpha          : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alt_Hue_Path    : Alt_Hue_Path_Type) is

        Alt_Hue_Path_Array : Boolean_Array_1D(Control_Points'range);
    begin
        if Alt_Hue_Path = Alt_Hue_Path_All then
            for i in Alt_Hue_Path_Array'range loop
                Alt_Hue_Path_Array(i) := True;
            end loop;
        elsif Alt_Hue_Path = Alt_Hue_Path_None then
            for i in Alt_Hue_Path_Array'range loop
                Alt_Hue_Path_Array(i) := False;
            end loop;
        end if;
        plscmap1la(Color_Model, Control_Points, H_Or_R, L_Or_G, S_Or_B, Alpha, Alt_Hue_Path_Array);
    end plscmap1la;


    -- Set number of colors in cmap 1
    procedure plscmap1n(Number_Of_Colors : Integer) is
    begin
        PLplot_Thin.plscmap1n(Number_Of_Colors);
    end plscmap1n;


    -- Set the color map 1 range used in continuous plots.
    procedure plscmap1_range(Min_Color, Max_Color : Long_Float) is
    begin
        PLplot_Thin.plscmap1_range(Min_Color, Max_Color);
    end plscmap1_range;


    -- Get the color map 1 range used in continuous plots
    procedure plgcmap1_range(Min_Color : out Long_Float; Max_Color : out Long_Float) is
    begin
        PLplot_Thin.plgcmap1_range(Min_Color, Max_Color);
    end plgcmap1_range;


    -- Set a given color from color map 0 by 8 bit RGB value
    procedure plscol0
       (Plot_Color : Plot_Color_Type;
        Red_Component, Green_Component, Blue_Component : Integer) is
    begin
        PLplot_Thin.plscol0(Plot_Color, Red_Component, Green_Component, Blue_Component);
    end plscol0;


    -- Set a given color from color map 0 by 8 bit RGB value and alpha value
    procedure plscol0a
       (Plot_Color                                     : Plot_Color_Type;
        Red_Component, Green_Component, Blue_Component : Integer;
        Alpha                                          : Long_Float_0_1_Type) is
    begin
        PLplot_Thin.plscol0a(Plot_Color, Red_Component, Green_Component, Blue_Component, Alpha);
    end plscol0a;


    -- Set the background color by 8 bit RGB value
    procedure plscolbg
       (Red_Component, Green_Component, Blue_Component : Integer) is
    begin
        PLplot_Thin.plscolbg(Red_Component, Green_Component, Blue_Component);
    end plscolbg;


    -- Set the background color by 8 bit RGB value and alpha value
    procedure plscolbga
       (Red_Component, Green_Component, Blue_Component : Integer;
        Alpha                                          : Long_Float_0_1_Type) is
    begin
        PLplot_Thin.plscolbga(Red_Component, Green_Component, Blue_Component, Alpha);
    end plscolbga;


    -- Used to globally turn color output on/off
    procedure plscolor(Enable_Color : Boolean) is

        PL_Enable_Color : PLBOOL;

    begin
        if Enable_Color then
            PL_Enable_Color := 1;
        else
            PL_Enable_Color := 0;
        end if;
        PLplot_Thin.plscolor(PL_Enable_Color);
    end plscolor;


    -- Set the compression level
    procedure plscompression(Compression_Level : Integer) is
    begin
        PLplot_Thin.plscompression(Compression_Level);
    end plscompression;


    -- Set the device (keyword) name
    procedure plsdev(Device_Name : String) is
    begin
        PLplot_Thin.plsdev(To_C(Device_Name, True));
    end plsdev;


    -- Set window into device space using margin, aspect ratio, and
    -- justification
    procedure plsdidev
       (Margin          : Long_Float;
        Aspect_Ratio    : Long_Float;
        x_Justification : Long_Float;
        y_Justification : Long_Float) is
    begin
        PLplot_Thin.plsdidev(Margin, Aspect_Ratio, x_Justification, y_Justification);
    end plsdidev;


    -- Set up transformation from metafile coordinates.
    procedure plsdimap
       (dimxmin : Integer;
        dimxmax : Integer;
        dimymin : Integer;
        dimymax : Integer;
        dimxpmm : Long_Float;
        dimypmm : Long_Float) is
    begin
        PLplot_Thin.plsdimap(dimxmin, dimxmax, dimymin, dimymax, dimxpmm, dimypmm);
    end plsdimap;


    -- Set plot orientation, specifying rotation in units of pi/2.
    procedure plsdiori(Rotation : Long_Float) is
    begin
        PLplot_Thin.plsdiori(Rotation);
    end plsdiori;


    -- Set window into plot space
    procedure plsdiplt
       (x_Min : Long_Float := 0.0;
        y_Min : Long_Float := 0.0;
        x_Max : Long_Float := 1.0;
        y_Max : Long_Float := 1.0) is
    begin
        PLplot_Thin.plsdiplt(x_Min, y_Min, x_Max, y_Max);
    end plsdiplt;


    -- Set window into plot space incrementally (zoom)
    procedure plsdiplz
       (x_Min_Relative : Long_Float;
        y_Min_Relative : Long_Float;
        x_Max_Relative : Long_Float;
        y_Max_Relative : Long_Float) is
    begin
        PLplot_Thin.plsdiplz(x_Min_Relative, y_Min_Relative, x_Max_Relative, y_Max_Relative);
    end plsdiplz;


    -- Set seed for internal random number generator
    procedure plseed(Seed : Unsigned_Int) is
    begin
        PLplot_Thin.plseed(Seed);
    end plseed;


    -- Set the escape character for text strings.
    procedure plsesc(Escape_Character : Character) is

        Escape_Characters : Ada.Strings.Maps.Character_Set;

    begin

        Escape_Characters := Ada.Strings.Maps.To_Set("!#$%&*@^~");

        if Ada.Strings.Maps.Is_In(Escape_Character, Escape_Characters) then
            PLplot_Thin.plsesc(To_C(Escape_Character));
        else
            Put_Line("Illegal escape character passed to Set_Escape_Character.");
        end if;
    end plsesc;


    -- Set family file parameters
    procedure plsfam
       (Enable_Family : Boolean;
        Family_File_Number : Integer := 1;
        Maximum_File_Size : Integer := 1_000_000) is

        PL_Enable_Family : PLBOOL;

    begin
        if Enable_Family then
            PL_Enable_Family := 1;
        else
            PL_Enable_Family := 0;
        end if;
        PLplot_Thin.plsfam(PL_Enable_Family, Family_File_Number, Maximum_File_Size);
    end plsfam;


    -- Set FCI (font characterization integer)
    procedure plsfci(Font_Characterization_Integer : Unicode) is
    begin
        PLplot_Thin.plsfci(Font_Characterization_Integer);
    end plsfci;


    -- Set the font family, style and weight
    procedure plsfont(Family, Style, Weight : Integer) is
    begin
        PLplot_Thin.plsfont(Family, Style, Weight);
    end;


    -- Set the output file name.
    procedure plsfnam(Output_File_Name : String) is
    begin
        PLplot_Thin.plsfnam(To_C(Output_File_Name, True));
    end plsfnam;


    -- Shade region.
    procedure plshade
       (z                                        : Real_Matrix;
        Mask_Function_Pointer                    : Mask_Function_Pointer_Type;
        x_Min, x_Max, y_Min, y_Max               : Long_Float; -- world mins and maxes
        Shade_Min, Shade_Max                     : Long_Float;
        Select_Color_Map                         : Natural; -- should be 0 or 1
        Color                                    : Long_Float;
        Fill_Pattern_Pen_Width                   : Long_Float;
        Shade_Min_Pen_Color                      : Natural;
        Shade_Min_Pen_Width                      : Long_Float;
        Shade_Max_Pen_Color                      : Natural;
        Shade_Max_Pen_Width                      : Long_Float;
        Fill_Procedure_Pointer                   : Fill_Procedure_Pointer_Type;
        Preserve_Rectangles                      : Boolean;
        Transformation_Procedure_Pointer         : Transformation_Procedure_Pointer_Type;
        Transformation_Data_Pointer              : PL_Pointer) is

        Preserve_Rectangles_As_Integer : Integer;
    begin
        if Preserve_Rectangles then
            Preserve_Rectangles_As_Integer := 1;
        else
            Preserve_Rectangles_As_Integer := 0;
        end if;

        PLplot_Thin.plshade(Matrix_To_Pointers(z), z'Length(1), z'Length(2), Mask_Function_Pointer,
            x_Min, x_Max, y_Min, y_Max, Shade_Min, Shade_Max, Select_Color_Map,
            Color, Fill_Pattern_Pen_Width,
            Shade_Min_Pen_Color, Shade_Min_Pen_Width,
            Shade_Max_Pen_Color, Shade_Max_Pen_Width,
            Fill_Procedure_Pointer, Preserve_Rectangles_As_Integer,
            Transformation_Procedure_Pointer, Transformation_Data_Pointer);
    end plshade;


    procedure plshades
       (z                                : Real_Matrix;
        Mask_Function_Pointer            : Mask_Function_Pointer_Type;
        x_Min, x_Max, y_Min, y_Max       : Long_Float; -- world mins and maxes
        Contour_Levels                   : Real_Vector;
        Fill_Pattern_Pen_Width           : Long_Float; -- 0.0 is allowed
        Contour_Pen_Color                : Natural; -- 0 for no contours
        Contour_Pen_Width                : Long_Float; -- 0.0 for no contours
        Fill_Procedure_Pointer           : Fill_Procedure_Pointer_Type;
        Preserve_Rectangles              : Boolean;
        Transformation_Procedure_Pointer : Transformation_Procedure_Pointer_Type;
        Transformation_Data_Pointer      : PL_Pointer) is

        Preserve_Rectangles_As_Integer : Integer;
    begin
        if Preserve_Rectangles then
            Preserve_Rectangles_As_Integer := 1;
        else
            Preserve_Rectangles_As_Integer := 0;
        end if;

        PLplot_Thin.plshades(Matrix_To_Pointers(z), z'Length(1), z'Length(2), Mask_Function_Pointer,
            x_Min, x_Max, y_Min, y_Max, Contour_Levels, Contour_Levels'Length,
            Fill_Pattern_Pen_Width, Contour_Pen_Color, Contour_Pen_Width,
            Fill_Procedure_Pointer, Preserve_Rectangles_As_Integer,
            Transformation_Procedure_Pointer, Transformation_Data_Pointer);
    end plshades;


    -- The procedure plfshade is not part of the API. If it should be necessary
    -- to make it available to Ada programs, use the binding to plfcont in this
    -- file as a guidline.

    -- fix this
    --    procedure
    --    plfshade(PLFLT (*f2eval) ( : PLINT;  : PLINT; PL_Pointer),
    --         PL_Pointer f2eval_data,
    --         PLFLT (*c2eval) ( : PLINT;  : PLINT; PL_Pointer),
    --         PL_Pointer c2eval_data,
    --         nx : PLINT; ny : PLINT;
    --         left : PLFLT; right : PLFLT; bottom : PLFLT; top : PLFLT;
    --         shade_min : PLFLT; shade_max : PLFLT;
    --         sh_cmap : PLINT; sh_color : PLFLT; sh_width : PLINT;
    --         min_color : PLINT; min_width : PLINT;
    --         max_color : PLINT; max_width : PLINT;
    --         void (*fill) ( : PLINT;  : PL_Float_Array;  : PL_Float_Array), rectangular : PLINT;
    --         void (*pltr) ( : PLFLT;  : PLFLT;  : PL_Float_Array;  : PL_Float_Array; PL_Pointer),
    --         PL_Pointer pltr_data);


    -- Setup a user-provided custom labeling function.
    procedure plslabelfunc
       (Custom_Label_Procedure_Pointer : Custom_Label_Procedure_Pointer_Type;
        label_data                     : PL_Pointer) is
    begin
        PLplot_Thin.plslabelfunc(Custom_Label_Procedure_Pointer, label_data);
    end plslabelfunc;


    -- Reset to default labeling. Not part of the C API.
    procedure Use_Default_Labels is
    begin
        plslabelfunc(null, System.Null_Address);
    end Use_Default_Labels;


    -- Set up lengths of major tick marks.
    procedure plsmaj(Default_Length, Scale_Factor : Long_Float) is
    begin
        PLplot_Thin.plsmaj(Default_Length, Scale_Factor);
    end plsmaj;


    -- Set the memory area to be plotted (with the 'mem' driver)
    procedure plsmem
       (x_Size, y_Size : Integer;
        Plot_This      : System.Address) is
    begin
        PLplot_Thin.plsmem(x_Size, y_Size, Plot_This);
    end plsmem;


    -- Set up lengths of minor tick marks.
    procedure plsmin(Default_Length, Scale_Factor : Long_Float) is
    begin
        PLplot_Thin.plsmin(Default_Length, Scale_Factor);
    end plsmin;


    -- Set orientation. Must be done before calling plinit.
    procedure plsori(Orientation : Orientation_Type) is
    begin
        if Orientation = Landscape then
            PLplot_Thin.plsori(0);
        else -- Orientation = Portrait
            PLplot_Thin.plsori(1);
        end if;
    end plsori;


    -- Set output device parameters. Usually ignored by the driver.
    procedure plspage
       (x_Pixels, y_Pixels : Long_Float;
        x_Length, y_Length : Integer;
        x_Offset, y_Offset : Integer) is
    begin
        PLplot_Thin.plspage(x_Pixels, y_Pixels, x_Length, y_Length, x_Offset, y_Offset);
    end plspage;


    -- Set the colors for color table 0 from a cmap0 file.
    procedure plspal0(Color_File_Name : String) is
    begin
        PLplot_Thin.plspal0(To_C(Color_File_Name, True));
    end plspal0;


    -- Set the colors for color table 1 from a cmap1 file.
    procedure plspal1(Color_File_Name : String; Interpolate : Boolean) is

        PL_Interpolate : PLBOOL;

    begin
        if Interpolate then
            PL_Interpolate := PLtrue;
        else
            PL_Interpolate := PLfalse;
        end if;
        PLplot_Thin.plspal1(To_C(Color_File_Name, True), PL_Interpolate);
    end plspal1;


    -- Set the pause (on end-of-page) status
    procedure plspause(Pause : Boolean) is

        PL_Pause : PLBOOL;

    begin
        if Pause then
            PL_Pause := PLtrue;
        else
            PL_Pause := PLfalse;
        end if;
        PLplot_Thin.plspause(PL_Pause);
    end plspause;


    -- Set stream number.
    procedure plsstrm(Stream_Number : Integer) is
    begin
        PLplot_Thin.plsstrm(Stream_Number);
    end plsstrm;


    -- Set the number of subwindows in x and y
    procedure plssub(x_Number, y_Number : Integer) is
    begin
        PLplot_Thin.plssub(x_Number, y_Number);
    end plssub;


    -- Set symbol height.
    procedure plssym(Default_Height, Scale_Factor : Long_Float) is
    begin
        PLplot_Thin.plssym(Default_Height, Scale_Factor);
    end plssym;


    -- Initialize PLplot, passing in the windows/page settings.
    procedure plstar
       (Number_Horizontal_Subpages, Number_Vertical_Subpages : Integer := 1) is
    begin
        PLplot_Thin.plstar(Number_Horizontal_Subpages, Number_Vertical_Subpages);
    end plstar;


    -- Initialize PLplot, passing the device name and windows/page settings.
    procedure plstart
       (Device_Name                                          : String;
        Number_Horizontal_Subpages, Number_Vertical_Subpages : Integer := 1) is
    begin
        PLplot_Thin.plstart(To_C(Device_Name), Number_Horizontal_Subpages, Number_Vertical_Subpages);
    end plstart;


    -- Set the coordinate transform.
    procedure plstransform
       (Coordinate_Transform_Procedure_Pointer : Coordinate_Transform_Procedure_Pointer_Type;
        Coordinate_Transform_Data_Pointer : PL_Pointer) is
    begin
        PLplot_Thin.plstransform(Coordinate_Transform_Procedure_Pointer,
            Coordinate_Transform_Data_Pointer);
    end plstransform;


    -- Clear the coordinate transform. Ada only; not part of the C API.
    procedure Clear_Custom_Coordinate_Transform is
    begin
        PLplot_Thin.plstransform(null, System.Null_Address);
    end Clear_Custom_Coordinate_Transform;


    -- Prints out the same string repeatedly at the n points in world
    -- coordinates given by the x and y arrays.  Supersedes plpoin and
    -- plsymbol for the case where text refers to a unicode glyph either
    -- directly as UTF-8 or indirectly via the standard text escape
    -- sequences allowed for PLplot input strings.
    procedure plstring
       (x, y : Real_Vector;
        Plot_This_String : String) is
    begin
        PLplot_Thin.plstring(x'Length, x, y, To_C(Plot_This_String));
    end plstring;


    -- Prints out the same string repeatedly at the n points in world
    -- coordinates given by the x, y, and z arrays.  Supersedes plpoin3
    -- for the case where text refers to a unicode glyph either directly
    -- as UTF-8 or indirectly via the standard text escape sequences
    -- allowed for PLplot input strings.
    procedure plstring3
       (x, y, z : Real_Vector;
        Plot_This_String : String) is
    begin
        PLplot_Thin.plstring3(x'Length, x, y, z, To_C(Plot_This_String));
    end plstring3;


    -- Add a point to a stripchart.
    procedure plstripa
       (ID         : Integer;
        Pen_Number : Integer;
        x, y       : Long_Float) is
    begin
        PLplot_Thin.plstripa(ID, Pen_Number, x, y);
    end plstripa;


    -- Create 1d stripchart
    procedure plstripc
       (ID                                   : out Integer;
        X_Options, Y_Options                 : String;
        x_Min, x_Max                         : Long_Float;
        x_Jump                               : Long_Float;
        y_Min, y_Max                         : Long_Float;
        x_Legend_Position, y_Legend_Position : Long_Float;
        Autoscale_Y                          : Boolean;
        Accumulate                           : Boolean; --Accumulate or slide
        Box_Color, Legend_Color              : Plot_Color_Type;
        Pen_Colors                           : Integer_Array_1D;
        Line_Styles                          : Integer_Array_1D;
        Pen_Labels                           : in out Stripchart_Label_String_Array_Type;
        X_Label, Y_Label, Title_Label        : String := To_String(Default_Label_String)) is

        PL_Autoscale_Y, PL_Accumulate : PLBOOL;
        PL_Pen_Labels : PL_Stripchart_String_Array;

        x_LP : Long_Float renames x_Legend_Position;
        y_LP : Long_Float renames y_Legend_Position;

    begin
        if Autoscale_Y then
            PL_Autoscale_Y := PLtrue;
        else
            PL_Autoscale_Y := PLfalse;
        end if;

        if Accumulate then
            PL_Accumulate := PLtrue;
        else
            PL_Accumulate := PLfalse;
        end if;

        -- Adapt strings for Pen_Labels to C. We have to pass an array of
        -- pointers (here, System.Address) to C-style strings, except that the
        -- C-style strings usually have the null terminator somewhere in the
        -- middle of the allocated string memory, after the "significant text."
        -- I'm not sure if C allocates the memory or not--probably not, since
        -- this is not a callback as in plslabelfunc where string memory was
        -- allocated by the intervening C code. Here, the C string length is set
        -- by Max_Stripchart_Label_Length, ranging 0 .. Max_Stripchart_Label_Length.
        for I in Pen_Labels'range loop

            -- Check length and adjust if necessary.
            if Length(Pen_Labels(I)) >= Max_Stripchart_Label_Length then
                Put_Line("*** Warning: Stripchart label was truncated to"
                    & Integer'Image(Max_Stripchart_Label_Length) & " characters. ***");
                Pen_Labels(I) := Head(Pen_Labels(I), Max_Stripchart_Label_Length);
            end if;

            -- Make the C-style string with null character immediately after the text.
            C_Stripchart_String_Array(I) := To_C(To_String(Pen_Labels(I)
                & Character'val(0)
                & (Max_Stripchart_Label_Length - Length(Pen_Labels(I))) * " "), False);

            -- Set the I-th pointer in the array of pointers.
            PL_Pen_Labels(I) := C_Stripchart_String_Array(I)'Address;
        end loop;

        PLplot_Thin.plstripc(ID, To_C(X_Options), To_C(Y_Options),
            x_Min, x_Max, x_Jump, y_Min, y_Max, x_LP, y_LP, PL_Autoscale_Y,
            PL_Accumulate, Box_Color, Legend_Color, Pen_Colors, Line_Styles,
            PL_Pen_Labels, To_C(x_Label), To_C(y_Label), To_C(Title_Label));
    end plstripc;


    -- Deletes and releases memory used by a stripchart.
    procedure plstripd(ID : Integer) is
    begin
        PLplot_Thin.plstripd(ID);
    end plstripd;


    -- plots a 2d image (or a matrix too large for plshade() )
    procedure plimagefr
       (Data                             : Real_Matrix;
        x_Min,     x_Max                 : Long_Float;
        y_Min,     y_Max                 : Long_Float;
        z_Min,     z_Max                 : Long_Float;
        Value_Min, Value_Max             : Long_Float;
        Transformation_Procedure_Pointer : Transformation_Procedure_Pointer_Type;
        Transformation_Data_Pointer      : PL_Pointer) is
    begin
        PLplot_Thin.plimagefr(Matrix_To_Pointers(Data), Data'Length(1), Data'Length(2),
            x_Min, x_Max, y_Min, y_Max, z_Min, z_Max, Value_Min, Value_Max,
            Transformation_Procedure_Pointer, Transformation_Data_Pointer);
    end plimagefr;


    -- plots a 2d image (or a matrix too large for plshade() )
    procedure plimage
       (Data : Real_Matrix;
        x_Min, x_Max : Long_Float;
        y_Min, y_Max : Long_Float;
        z_Min, z_Max : Long_Float;
        Dxmin, Dxmax : Long_Float;
        Dymin, Dymax : Long_Float) is
    begin
        PLplot_Thin.plimage(Matrix_To_Pointers(Data), Data'Length(1), Data'Length(2), x_Min, x_Max, y_Min, y_Max, z_Min, z_Max, Dxmin, Dxmax, Dymin, Dymax);
    end plimage;


    -- Set up a new line style
    procedure plstyl(Marks, Spaces : Integer_Array_1D) is
    begin
        PLplot_Thin.plstyl(Marks'Length, Marks, Spaces);
    end plstyl;

    -- This is an overloaded procedure equivalent to calling plstyl with its
    -- first argument zero.
    procedure plstyl(Default_Continuous_Line : Integer) is
        Dummy_Marks, Dummy_Spaces : Integer_Array_1D(1 .. 1) := (others => 0);
    begin
        PLplot_Thin.plstyl(0, Dummy_Marks, Dummy_Spaces);
    end plstyl;


    -- Plots the 3d surface representation of the function z(x, y).
    procedure plsurf3d
       (x, y           : Real_Vector; -- surface definition points
        z              : Real_Matrix; -- height of surface at definition points
        Options        : Integer;
        Contour_Levels : Real_Vector) is -- levels at which to draw contours
    begin
        PLplot_Thin.plsurf3d(x, y, Matrix_To_Pointers(z), x'Length, y'Length, Options, Contour_Levels, Contour_Levels'Length);
    end plsurf3d;


    -- -- Like plsurf3d, but uses an evaluator function to access z data from zp
    -- procedure plfsurf3d
    --    (x , y          : Real_Vector; -- surface definition points
    --     z_Ops          : Data_Ops_2D_Type; -- operations on z
    --     z              : Real_Matrix; -- height of surface at definition points
    --     Options        : Integer;
    --     Contour_Levels : Real_Vector) is -- levels at which to draw contours
    -- begin
    --     PLplot_Thin.plfsurf3d(x, y, z_Ops, Matrix_To_Pointers(z), x'Length, y'Length, Options,
    --     Contour_Levels, Contour_Levels'Length);
    -- end plfsurf3d;


    -- Plots the 3-d surface representation of the function z(x, y) with limits on x and y.
    procedure plsurf3dl
       (x, y                     : Real_Vector; -- surface definition points
        z                        : Real_Matrix; -- height of surface at definition points
        Options                  : Integer;
        Contour_Levels           : Real_Vector; -- levels at which to draw contours
        Index_x_Min, Index_x_Max : Integer;             -- limits on x
        Index_y_Min, Index_y_Max : Integer_Array_1D) is -- limits on y
    begin
        PLplot_Thin.plsurf3dl(x, y, Matrix_To_Pointers(z), x'Length, y'Length, Options,
            Contour_Levels, Contour_Levels'Length, Index_x_Min, Index_x_Max,
            Index_y_Min, Index_y_Max);
    end plsurf3dl;


    -- Sets the edges of the viewport to the specified absolute coordinates
    procedure plsvpa
       (Left_Edge   : Long_Float;    -- millimeters from edge of subpage
        Right_Edge  : Long_Float;    -- millimeters from edge of subpage
        Bottom_Edge : Long_Float;    -- millimeters from edge of subpage
        Top_Edge    : Long_Float) is -- millimeters from edge of subpage
    begin
        PLplot_Thin.plsvpa(Left_Edge, Right_Edge, Bottom_Edge, Top_Edge);
    end plsvpa;


    -- Set x axis labeling parameters
    -- "digits" changed to "field_digits".
    procedure plsxax(Max_Digits, Field_Digits : Integer) is
    begin
        PLplot_Thin.plsxax(Max_Digits, Field_Digits);
    end plsxax;


    -- fix this; undocumented in Chapter 17.
    -- Set inferior X window
    procedure plsxwin(Window_ID : Integer) is
    begin
        PLplot_Thin.plsxwin(Window_ID);
    end plsxwin;


    -- Set y axis labeling parameters
    -- "digits" changed to "field_digits".
    procedure plsyax(Max_Digits, Field_Digits : Integer) is
    begin
        PLplot_Thin.plsyax(Max_Digits, Field_Digits);
    end plsyax;


    -- Plots array y against x for n points using Hershey symbol "code"
    procedure plsym
       (x, y : Real_Vector;
        Hershey_Code : Integer) is
    begin
        PLplot_Thin.plsym(x'Length, x, y, Hershey_Code);
    end plsym;


    -- Set z axis labeling parameters
    -- "digits" changed to "field_digits".
    procedure plszax(Max_Digits, Field_Digits : Integer) is
    begin
        PLplot_Thin.plszax(Max_Digits, Field_Digits);
    end plszax;


    -- Switches to text screen.
    procedure pltext is
    begin
        PLplot_Thin.pltext;
    end pltext;


    -- Set the format for date / time labels
    procedure pltimefmt(Format : String) is
    begin
        PLplot_Thin.pltimefmt(To_C(Format));
    end pltimefmt;


    -- Sets the edges of the viewport with the given aspect ratio, leaving
    -- room for labels.
    procedure plvasp(Aspect_Ratio : Long_Float) is
    begin
        PLplot_Thin.plvasp(Aspect_Ratio);
    end plvasp;


    -- Creates the largest viewport of the specified aspect ratio that fits
    -- within the specified normalized subpage coordinates.
    procedure plvpas
       (x_Min, x_Max : Long_Float;
        y_Min, y_Max : Long_Float;
        Aspect_Ratio : Long_Float) is
    begin
        PLplot_Thin.plvpas(x_Min, x_Max, y_Min, y_Max, Aspect_Ratio);
    end plvpas;


    -- Creates a viewport with the specified normalized subpage coordinates.
    procedure plvpor
       (Left_Edge   : Long_Float := 0.0;
        Right_Edge  : Long_Float := 1.0;
        Bottom_Edge : Long_Float := 0.0;
        Top_Edge    : Long_Float := 1.0) is
    begin
        PLplot_Thin.plvpor(Left_Edge, Right_Edge, Bottom_Edge, Top_Edge);
    end plvpor;


    -- Defines a "standard" viewport with seven character heights for
    -- the left margin and four character heights everywhere else.
    procedure plvsta is
    begin
        PLplot_Thin.plvsta;
    end plvsta;


    -- Set up a window for three-dimensional plotting.
    procedure plw3d
       (X_Box, Y_Box, Z_Box                      : Long_Float; -- Extents of enclosing box; world coordinates
        X_Min, X_Max, Y_Min, Y_Max, Z_Min, Z_Max : Long_Float; -- Data limits; user coordinates
        Altitude, Azimuth                        : Long_Float) is -- Viewing angles in world coordinates
    begin
        PLplot_Thin.plw3d(X_Box, Y_Box, Z_Box, X_Min, X_Max, Y_Min, Y_Max, Z_Min, Z_Max, Altitude, Azimuth);
    end plw3d;


    -- Set pen width.
    procedure plwidth(Pen_Width : Long_Float) is
    begin
        PLplot_Thin.plwidth(Pen_Width);
    end plwidth;


    -- Set up world coordinates of the viewport boundaries (2d plots).
    procedure plwind
       (Left_Edge   : Long_Float;
        Right_Edge  : Long_Float;
        Bottom_Edge : Long_Float;
        Top_Edge    : Long_Float) is
    begin
        PLplot_Thin.plwind(Left_Edge, Right_Edge, Bottom_Edge, Top_Edge);
    end plwind;


    -- set xor mode; mode = 1-enter, 0-leave, status = 0 if not interactive device
    procedure plxormod
       (Use_XOR : Boolean;
        Supports_XOR : out Boolean) is

        PL_Use_XOR, PL_Supports_XOR : PLBOOL;

    begin
        if Use_XOR then
            PL_Use_XOR := PLtrue;
        else
            PL_Use_XOR := PLfalse;
        end if;

        PLplot_Thin.plxormod(PL_Use_XOR, PL_Supports_XOR);

        if PL_Supports_XOR = PLtrue then
            Supports_XOR := True;
        elsif PL_Supports_XOR = PLfalse then
            Supports_XOR := False;
        else
            Put_Line("Invalid driver status returned from PLplot in Set_XOR_Mode.");
        end if;
    end plxormod;


--------------------------------------------------------------------------------
--         Functions for use from C or C++ only                               --
--         (Not really ;).                                                    --
--------------------------------------------------------------------------------
--  THESE FUNCTIONS ^^^ ARE NOT IMPLEMENTED FOR THE ADA BINDING
--  EXCEPT FOR THE FOLLOWING.

    -- This procedure is currently GNAT-specific, importing Gnat_Argc and Gnat_Argv.
    -- Process options list using current options info.
    procedure plparseopts(Mode : Parse_Mode_Type) is

        Gnat_Argc : aliased Integer;
        pragma Import (C, Gnat_Argc);

        Gnat_Argv : System.Address;
        pragma Import (C, Gnat_Argv);

        type Gnat_Argc_Access_Type is access all Integer;
        Gnat_Argc_Access : Gnat_Argc_Access_Type;

        procedure
        plparseopts_local(argc : Gnat_Argc_Access_Type;
                          argv : System.Address;
                          mode : Parse_Mode_Type);
        pragma Import(C, plparseopts_local, "c_plparseopts");

    begin
        Gnat_Argc_Access := Gnat_Argc'access;
        plparseopts_local(Gnat_Argc_Access, Gnat_Argv, Mode);
    end plparseopts;


    -- Process input strings, treating them as an option and argument pair.
    procedure plsetopt(Option, Argument : String) is
    begin
        PLplot_Thin.plsetopt(To_C(Option, True), To_C(Argument, True));
    end plsetopt;


	-- Transformation routines

    -- fix this These functions are redundant with those in plplot_thin.
    -- This might be OK since they are now available at all levels of binding.
    -- I wonder if this is approaching "wrapper bloat" since these procedures
    -- get called a lot of times during the making of a contour plot.
    -- The way to eliminate one level of calling would be to move the bodies
    -- of pltr? from plplot_thin.adb into plplot_traditional.adb and
    -- plplot.adb, then optionally eliminating the bodies from plplot_thin.adb
    -- on the idea that nobody is going to use them anyway. But even if the
    -- bodies were left in plplot_thin.adb, having them here would still
    -- remove the extra call level. The argument for the currend arrangement is
    -- easier code maintainence.

    -- Identity transformation.
    procedure pltr0
       (x_Grid, y_Grid   : Long_Float;
        x_World, y_World : out Long_Float;
        Data             : PL_Pointer) is
    begin
        PLplot_Thin.pltr0(x_Grid, y_Grid, x_World, y_World, Data);
    end pltr0;


    -- Does linear interpolation from singly dimensioned coord arrays.
    procedure pltr1
       (x_Grid, y_Grid   : Long_Float;
        x_World, y_World : out Long_Float;
        Data_Pointer     : PL_Pointer) is
    begin
        PLplot_Thin.pltr1(x_Grid, y_Grid, x_World, y_World, Data_Pointer);
    end pltr1;


    -- Does linear interpolation from doubly dimensioned coord arrays
    -- (column dominant, as per normal C 2d arrays).
    procedure pltr2
       (x_Grid, y_Grid   : Long_Float;
        x_World, y_World : out Long_Float;
        Data_Pointer     : PL_Pointer) is
    begin
        PLplot_Thin.pltr2(x_Grid, y_Grid, x_World, y_World, Data_Pointer);
    end pltr2;


    -- Wait for graphics input event and translate to world coordinates.
    procedure plGetCursor(Graphics_Input : out PLGraphicsIn) is
    begin
        PLplot_Thin.plGetCursor(Graphics_Input);
    end plGetCursor;

begin -- package body for PLplot

    -- NOTE: One would ordinarily call the initialization procedure at this point.
    -- However, PLplot has several optional procedures that, if used, must be
    -- called _before_ the initializaiton, here called Initialize_PLplot (plinit).
    -- Therefore, Initialize_PLplot (plinit) MUST BE CALLED FROM THE USER'S PROGRAM,
    -- before beginning normal plotting but AFTER any optional pre-initialization.

    -- The following lines marked by ========= are some examples of how the
    -- entire initialzation can be done here if the user wants to do so, and is
    -- willing to uncomment these lines and/or add new lines.

    -- =========================================================================
    -- Set_Compression_Level (plscompression) should be called before
    -- Initialize_PLplot (plinit) when it is needed.
    -- There should be a list of devices which use compression, and
    -- Set_Compression_Level (plscompression) called conditioned on the current
    -- device being in that list.

    -- Optionally call Set_Command_Line_Option (plsetopt) before Initialize_PLplot
    -- (plinit).

    -- Parse_Command_Line_Arguments(1);
    -- Set_File_Family_Parameters(False); -- (plsfam(False)). Default off;
    -- other args are don't-cares here.
    -- Set_Orientation(Landscape); -- (plsori(Landscape)). Optional; before
    -- Initialize_PLplot if used.

    -- Initialize_PLplot;
    -- =========================================================================


    -- Capture the initial, default, settings of color map 0 since these will be
    -- lost if the settings for color map 0 are set by the user. They can be
    -- restored collectively by calling Restore_Default_Snapshot_Of_Color_Map_0
    -- or individually by calling functions such as Reset_Red etc. for each of
    -- the 16 default colors of color map 0.
    Make_Snapshot_Of_Color_Map_0(Default_Red_Components, Default_Green_Components, Default_Blue_Components);

end PLplot_Traditional;
