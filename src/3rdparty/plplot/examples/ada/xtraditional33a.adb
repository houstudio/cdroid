-- Demonstrate most pllegend capability including unicode symbols.
--
-- Copyright (C) 2011-2013 Jerry Bauck
--
-- This file is part of PLplot.
--
-- PLplot is free software; you can redistribute it and/or modify
-- it under the terms of the GNU Library General Public License as published
-- by the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- PLplot is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Library General Public License for more details.
--
-- You should have received a copy of the GNU Library General Public License
-- along with PLplot; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
--

-- This example designed just for devices (e.g., the cairo-related and
-- qt-related devices) where the best choice of glyph is automatically
-- selected by the related libraries (pango/cairo or Qt4) for each
-- unicode character depending on what system fonts are installed.  Of
-- course, you must have the appropriate TrueType fonts installed to
-- have access to all the required glyphs.

-- Ada note: The function TUB is Ada.Strings.Unbounded.To_Unbounded_String
-- renamed. See PLplot_Traditional.ads or PLplot.ads.

with
    Ada.Strings.Unbounded,
    Ada.Strings,
    Ada.Strings.Fixed,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    Ada.Strings.Unbounded,
    Ada.Strings,
    Ada.Strings.Fixed,
    PLplot_Auxiliary,
    PLplot_Traditional;

----------------------------------------------------------------------------
-- Demonstrate most pllegend capability including unicode symbols.
----------------------------------------------------------------------------

procedure xtraditional33a is
    position_options : Integer_Array_1D(0 .. 15) :=
       (Pl_Position_Left + Pl_Position_Top + Pl_Position_Outside,
        Pl_Position_Top + Pl_Position_Outside,
        Pl_Position_Right + Pl_Position_Top + Pl_Position_Outside,
        Pl_Position_Right + Pl_Position_Outside,
        Pl_Position_Right + Pl_Position_Bottom + Pl_Position_Outside,
        Pl_Position_Bottom + Pl_Position_Outside,
        Pl_Position_Left + Pl_Position_Bottom + Pl_Position_Outside,
        Pl_Position_Left + Pl_Position_Outside,
        Pl_Position_Left + Pl_Position_Top + Pl_Position_Inside,
        Pl_Position_Top + Pl_Position_Inside,
        Pl_Position_Right + Pl_Position_Top + Pl_Position_Inside,
        Pl_Position_Right + Pl_Position_Inside,
        Pl_Position_Right + Pl_Position_Bottom + Pl_Position_Inside,
        Pl_Position_Bottom + Pl_Position_Inside,
        Pl_Position_Left + Pl_Position_Bottom + Pl_Position_Inside,
        Pl_Position_Left + Pl_Position_Inside);

    -- Pick 5 arbitrary UTF-8 symbols useful for plotting points (✠✚✱✪✽✺✰✴✦).
    special_symbols : Legend_String_Array_Type(0 .. 4) :=
       (TUB("✰"), TUB("✴"), TUB("✱"), TUB("✽"), TUB("✦")); -- TUB is renamed To_Unbounded_String.

    -- plcolorbar options

    -- Colorbar type options
    Colorbar_Kinds : constant Integer := 4;
    colorbar_option_kinds : Integer_Array_1D(0 .. Colorbar_Kinds - 1) := (
        Pl_Colorbar_Shade,
        Pl_Colorbar_Shade + Pl_Colorbar_Shade_Label,
        Pl_Colorbar_Image,
        Pl_Colorbar_Gradient);
    colorbar_option_kind_labels : Legend_String_Array_Type(0 .. Colorbar_Kinds - 1) :=
       (TUB("Shade colorbars"),
        TUB("Shade colorbars with custom labels"),
        TUB("Image colorbars"),
        TUB("Gradient colorbars"));

    -- Which side of the page are we positioned relative to?
    Colorbar_Positions :constant Integer := 4;
    colorbar_position_options : Integer_Array_1D(0 .. Colorbar_Positions - 1) :=
       (Pl_Position_Left,
        Pl_Position_Right,
        Pl_Position_Top,
        Pl_Position_Bottom);

    colorbar_position_option_labels : Legend_String_Array_Type(0 .. Colorbar_Positions - 1) :=
       (TUB("Left"),
        TUB("Right"),
        TUB("Top"),
        TUB("Bottom"));

    -- Colorbar label positioning options
    Colorbar_Labels : constant Integer := 4;
    colorbar_label_options : Integer_Array_1D(0 .. Colorbar_Labels - 1) :=
       (Pl_Colorbar_Label_Left,
        Pl_Colorbar_Label_Right,
        Pl_Colorbar_Label_Top,
        Pl_Colorbar_Label_Bottom);

    colorbar_label_option_labels : Legend_String_Array_Type(0 .. Colorbar_Labels - 1) :=
       (TUB("Label left"),
        TUB("Label right"),
        TUB("Label top"),
        TUB("Label bottom"));

    -- Colorbar cap options
    Colorbar_Caps : constant Integer := 4;
    colorbar_cap_options : Integer_Array_1D(0 .. Colorbar_Caps - 1) :=
       (Pl_Colorbar_Cap_None,
        Pl_Colorbar_Cap_Low,
        Pl_Colorbar_Cap_High,
        Pl_Colorbar_Cap_Low + Pl_Colorbar_Cap_High);
    colorbar_cap_option_labels : Legend_String_Array_Type(0 .. Colorbar_Caps - 1) :=
       (TUB("No caps"),
        TUB("Low cap"),
        TUB("High cap"),
        TUB("Low and high caps"));


    procedure plcolorbar_example_page(kind_i : Integer; label_i : Integer; cap_i : Integer;
        cont_color : Integer; cont_width : Long_Float; n_values : Integer; values : Real_Vector)
    is
        -- Parameters for the colorbars on this page
        position, opt : Integer;
        x, y, x_length, y_length : Long_Float;
        ticks : Real_Vector(0 .. 0) := (others => 0.0);
        sub_ticks : Integer_Array_1D(0 .. 0) := (others => 0);
        low_cap_color, high_cap_color : Long_Float;
        vertical, ifn : Boolean;
        n_axes : Integer := 1;
        axis_opts : Legend_String_Array_Type(0 .. 0);
        n_labels : Integer := 1;
        label_opts : Integer_Array_1D(0 .. 0) := (others => 0);
        label : Legend_String_Array_Type(0 .. 0);
        title : Unbounded_String;
        colorbar_width, colorbar_height : Long_Float;
        n_values_array : Integer_Array_1D(0 .. 0);
        values_array : Real_Matrix(0 .. 0, values'first .. values'last);
        type Stupid is mod 2**7; -- To cover Legend_Colorbar_Position_Type in plplot_thin.ads.
    begin
        n_values_array(0) := n_values;
        for i in values'range loop
           values_array(0, i) := values(i);
        end loop;

        low_cap_color := 0.0;
        high_cap_color := 1.0;

        -- Start a new page.
        pladv(0);

        -- Draw one colorbar relative to each side of the page.
        for position_i in 0 .. Colorbar_Positions - 1 loop
            position := colorbar_position_options(position_i);
            opt := colorbar_option_kinds(kind_i) +
                colorbar_label_options(label_i) + colorbar_cap_options(cap_i);

            -- We have to convert these integers to modular type Stupid before and-ing the bits.
            -- That's because manipulating bits of non-modular types is nonsense but other
            -- languages seem to not care about stuff like this.
            vertical := ((Stupid(position) and Stupid(Pl_Position_Left)) > 0) or
                        ((Stupid(position) and Stupid(Pl_Position_Right)) > 0);

            ifn := ((Stupid(position) and Stupid(Pl_Position_Left)) > 0) or
                ((Stupid(position) and Stupid(Pl_Position_Bottom)) > 0);

            -- Set the offset position on the page.
            if vertical then
                x        := 0.0;
                y        := 0.0;
                x_length := 0.05;
                y_length := 0.5;
            else
                x        := 0.0;
                y        := 0.0;
                x_length := 0.5;
                y_length := 0.05;
            end if;

            -- Set appropriate labelling options.
            if ifn then
                if cont_color = 0 or cont_width = 0.0 then
                    axis_opts(0) := TUB("uwtivn"); -- TUB is renamed To_Unbounded_String.
                else
                    axis_opts(0) := TUB("uwxvn");
                end if;
            else
                if cont_color = 0 or cont_width = 0.0 then
                    axis_opts(0) := TUB("uwtivm");
                else
                    axis_opts(0) := TUB("uwxvm");
                end if;
            end if;

            label(0) := colorbar_position_option_labels(position_i) & ", "
                & colorbar_label_option_labels(label_i);

            -- Smaller text
            plschr(0.0, 0.75);
            -- Small ticks on the vertical axis
            plsmaj(0.0, 0.5);
            plsmin(0.0, 0.5);

            plvpor(0.20, 0.80, 0.20, 0.80);
            plwind(0.0, 1.0, 0.0, 1.0);
            -- Set interesting background colour.
            plscol0a(15, 0, 0, 0, 0.20);
            plcolorbar(colorbar_width, colorbar_height,
                opt + Pl_Colorbar_Bounding_Box + Pl_Colorbar_Background, position,
                x, y, x_length, y_length,
                15, 1, 1,
                low_cap_color, high_cap_color,
                cont_color, cont_width,
                label_opts, label,
                axis_opts,
                ticks, sub_ticks,
                n_values_array, values_array);

            -- Reset text and tick sizes
            plschr(0.0, 1.0);
            plsmaj(0.0, 1.0);
            plsmin(0.0, 1.0);
        end loop;

        -- Draw a page title
        title := colorbar_option_kind_labels(kind_i) & " - "
            & colorbar_cap_option_labels(cap_i);
        plvpor(0.0, 1.0, 0.0, 1.0);
        plwind(0.0, 1.0, 0.0, 1.0);
        plptex(0.5, 0.5, 0.0, 0.0, 0.5, To_String(title));
    end plcolorbar_example_page;


    procedure plcolorbar_example(palette : String; kind_i : Integer; cont_color : Integer;
        cont_width : Long_Float; n_values : Integer; values : Real_Vector) is
    begin
        -- Load the color palette
        plspal1(palette, True);

        for label_i in 0 .. Colorbar_Labels - 1 loop
            for cap_i in 0 .. Colorbar_Caps - 1 loop
                plcolorbar_example_page(kind_i, label_i, cap_i, cont_color, cont_width,
                    n_values, values );
            end loop;
        end loop;
    end plcolorbar_example;


    colorbar : Boolean := True; -- By default do not plot plcolorbar pages
                                -- for now while we are working out the API.
    opt : Integer;
    nlegend : Integer;
    nturn : Integer;
    legend_width, legend_height, x, y, xstart, ystart : Long_Float;
    max_height, text_scale : Long_Float;
    position, opt_base, nrow, ncolumn : Integer;

begin -- main
    plparseopts(Pl_Parse_Full);

    -- Initialize plplot
    plinit;

    -- First page illustrating the 16 standard positions.
    declare
        nlegend : constant Integer := 1;
        opt_array       :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        text_colors     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_colors      :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_patterns    :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_scales      :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        box_line_widths :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        line_colors     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        line_styles     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        line_widths     :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        symbol_numbers  :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        symbol_colors   :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        symbol_scales   :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        text            : Legend_String_Array_Type(0 .. nlegend - 1) := (others => TUB(" "));
        symbols         : Legend_String_Array_Type(0 .. nlegend - 1) := (others => TUB(" "));
    begin
        pladv(0);
        plvpor(0.25, 0.75, 0.25, 0.75);
        plwind(0.0, 1.0, 0.0, 1.0);
        plbox("bc", 0.0, 0, "bc", 0.0, 0);
        plsfont(PL_FCI_SANS, -1, -1);
        plmtex("t", 8.0, 0.5, 0.5, "The 16 standard legend positions with");
        plmtex("t", 6.0, 0.5, 0.5, "the same (0.05) offset in x and y");

        -- Only specify legend data that are required according to the
        -- value of opt_array for that entry.
        opt_base          := Pl_Legend_Background + Pl_Legend_Bounding_Box;
        opt_array(0)      := Pl_Legend_Line + Pl_Legend_Symbol;
        line_styles(0)    := 1;
        line_widths(0)    := 1.0;
        symbol_scales(0)  := 1.0;
        symbol_numbers(0) := 4;
        symbols(0)        := TUB("#(728)");

        -- Use monotype fonts so that all legends are the same size.
        plsfont(PL_FCI_MONO, -1, -1);
        plscol0a(15, 32, 32, 32, 0.70);

        for k in 0 .. 15 loop
            position := position_options(k);
            opt      := opt_base;
            -- Make a 2-digit string with leading 0 for 0 .. 9.
            if k >= 10 then
                text(0) := To_Unbounded_String(Trim(Integer'image(k), Left));
            else
                text(0) := To_Unbounded_String("0" & Trim(Integer'image(k), Left));
            end if;
            text_colors(0)   := 1 + (k mod 8);
            line_colors(0)   := 1 + (k mod 8);
            symbol_colors(0) := 1 + (k mod 8);

            pllegend(legend_width, legend_height, opt, position, 0.05, 0.05,
                0.1, 15, 1, 1, 0, 0,
                opt_array, 1.0, 1.0, 2.0,
                1.0, text_colors, text,
                Box_Colors, Box_Patterns,
                Box_Scales, Box_Line_Widths,
                line_colors, line_styles, line_widths,
                symbol_colors, symbol_scales, symbol_numbers, symbols);
        end loop;
    end; -- declare

    -- Second page illustrating effect of nrow, ncolumn for the same legend data.
    declare
        nlegend : constant Integer := 7;
        opt_array       :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        text_colors     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_colors      :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_patterns    :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_scales      :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        box_line_widths :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        line_colors     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        line_styles     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        line_widths     :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        symbol_numbers  :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        symbol_colors   :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        symbol_scales   :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        text            : Legend_String_Array_Type(0 .. nlegend - 1) := (others => TUB(" "));
        symbols         : Legend_String_Array_Type(0 .. nlegend - 1) := (others => TUB(" "));
    begin
        pladv(0);
        plvpor(0.25, 0.75, 0.25, 0.75);
        plwind(0.0, 1.0, 0.0, 1.0);
        plbox("bc", 0.0, 0, "bc", 0.0, 0);
        plsfont(PL_FCI_SANS, -1, -1);
        plmtex("t", 8.0, 0.5, 0.5, "The effect of nrow, ncolumn, PL_LEGEND_ROW_MAJOR,");
        plmtex("t", 6.0, 0.5, 0.5, "and position for the same legend data");

        -- Only specify legend data that are required according to the
        -- value of opt_array for that entry.
        opt_base := Pl_Legend_Background + Pl_Legend_Bounding_Box;
        for k in 0 .. nlegend - 1 loop
            opt_array(k)      := Pl_Legend_Line + Pl_Legend_Symbol;
            line_styles(k)    := 1;
            line_widths(k)    := 1.0;
            symbol_scales(k)  := 1.0;
            symbol_numbers(k) := 2;
            symbols(k)        := TUB("#(728)");
            -- Make 2-digit strings with leading 0 for 0 .. 9.
            if k >= 10 then
                text(k) := To_Unbounded_String(Trim(Integer'image(k), Left));
            else
                text(k) := To_Unbounded_String("0" & Trim(Integer'image(k), Left));
            end if;
            text_colors(k)   := 1 + (k mod 8);
            line_colors(k)   := 1 + (k mod 8);
            symbol_colors(k) := 1 + (k mod 8);
        end loop;

        -- Use monotype fonts so that all legends are the same size.
        plsfont(PL_FCI_MONO, -1, -1);
        plscol0a(15, 32, 32, 32, 0.70);

        position := Pl_Position_Top + Pl_Position_Outside;
        opt      := opt_base;
        x        := 0.0;
        y        := 0.1;
        nrow     := 1;
        ncolumn  := nlegend;
        pllegend(legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);

        position := Pl_Position_Bottom + Pl_Position_Outside;
        opt      := opt_base;
        x        := 0.0;
        y        := 0.1;
        nrow     := 1;
        ncolumn  := nlegend;
        pllegend(legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);

        position := Pl_Position_Left + Pl_Position_Outside;
        opt      := opt_base;
        x        := 0.1;
        y        := 0.0;
        nrow     := nlegend;
        ncolumn  := 1;
        pllegend(legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);

        position := Pl_Position_Right + Pl_Position_Outside;
        opt      := opt_base;
        x        := 0.1;
        y        := 0.0;
        nrow     := nlegend;
        ncolumn  := 1;
        pllegend(legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);

        position := Pl_Position_Left + Pl_Position_Top + Pl_Position_Inside;
        opt      := opt_base;
        x        := 0.0;
        y        := 0.0;
        nrow     := 6;
        ncolumn  := 2;
        pllegend(legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);

        position := Pl_Position_Right + Pl_Position_Top + Pl_Position_Inside;
        opt      := opt_base + Pl_Legend_Row_Major;
        x        := 0.0;
        y        := 0.0;
        nrow     := 6;
        ncolumn  := 2;
        pllegend(legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);

        position := Pl_Position_Bottom + Pl_Position_Inside;
        opt      := opt_base + Pl_Legend_Row_Major;
        x        := 0.0;
        y        := 0.0;
        nrow     := 3;
        ncolumn  := 3;
        pllegend(legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
    end; -- declare

    -- Third page demonstrating legend alignment
    pladv(0);
    plvpor(0.0, 1.0, 0.0, 0.9);
    plwind(0.0, 1.0, 0.0, 1.0);
    plsfont(Pl_Fci_Sans, -1, -1);
    plmtex("t", 2.0, 0.5, 0.5, "Demonstrate legend alignment");

    x        := 0.1;
    y        := 0.1;
    nturn    := 4;
    nlegend  := 0;
    position := Pl_Position_Top + Pl_Position_Left + Pl_Position_Subpage;
    opt_base := Pl_Legend_Background + Pl_Legend_Bounding_Box;
    opt      := opt_base;
    for i in 0 .. 8 loop

        -- Set up legend arrays with the correct size, type.
        if i <= nturn then
            nlegend := nlegend + 1;
        else
            nlegend := nlegend - 1;
        end if;
        nlegend := Integer'max(1, nlegend);
        declare
            opt_array       :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
            text_colors     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
            box_colors      :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
            box_patterns    :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
            box_scales      :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
            box_line_widths :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
            line_colors     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
            line_styles     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
            line_widths     :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
            symbol_numbers  :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
            symbol_colors   :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
            symbol_scales   :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
            text            : Legend_String_Array_Type(0 .. nlegend - 1) := (others => TUB(" "));
            symbols         : Legend_String_Array_Type(0 .. nlegend - 1) := (others => TUB(" "));
        begin
            -- Only specify legend data that are required according to the
            -- value of opt_array for that entry.
            for k in 0 .. nlegend - 1 loop
                opt_array(k)      := Pl_Legend_Line + Pl_Legend_Symbol;
                line_styles(k)    := 1;
                line_widths(k)    := 1.0;
                symbol_scales(k)  := 1.0;
                symbol_numbers(k) := 2;
                symbols(k)        := TUB("#(728)");
                -- Make 2-digit strings with leading 0 for 0 .. 9.
                if k >= 10 then
                    text(k) := To_Unbounded_String(Trim(Integer'image(k), Left));
                else
                    text(k) := To_Unbounded_String("0" & Trim(Integer'image(k), Left));
                end if;
                text_colors(k)   := 1 + (k mod 8);
                line_colors(k)   := 1 + (k mod 8);
                symbol_colors(k) := 1 + (k mod 8);
            end loop; -- k

            -- Use monotype fonts so that all legends are the same size.
            plsfont(Pl_Fci_Mono, -1, -1);
            plscol0a(15, 32, 32, 32, 0.70);

            nrow    := Integer'min(3, nlegend);
            ncolumn := 0;

            pllegend(legend_width, legend_height, opt, position, x, y,
                0.025, 15, 1, 1, nrow, ncolumn,
                opt_array, 1.0, 1.0, 1.5,
                1.0, text_colors, text,
                Box_Colors, Box_Patterns,
                Box_Scales, Box_Line_Widths,
                line_colors, line_styles, line_widths,
                symbol_colors, symbol_scales, symbol_numbers, symbols);

            if i = nturn then
                position := Pl_Position_Top + Pl_Position_Right + Pl_Position_Subpage;
                opt      := opt_base;
                x        := 1.0 - x;
                y        := y + legend_height;
            else
                x := x + legend_width;
                y := y + legend_height;
            end if;
        end; -- declare
    end loop; -- i

    -- Fourth page illustrating various kinds of legends
    declare
        nlegend : constant Integer := 5;
        opt_array       :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        text_colors     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_colors      :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_patterns    :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        box_scales      :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        box_line_widths :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        line_colors     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        line_styles     :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        line_widths     :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        symbol_numbers  :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        symbol_colors   :         Integer_Array_1D(0 .. nlegend - 1) := (others => 0);
        symbol_scales   :         Real_Vector     (0 .. nlegend - 1) := (others => 0.0);
        text            : Legend_String_Array_Type(0 .. nlegend - 1) := (others => TUB(" "));
        symbols         : Legend_String_Array_Type(0 .. nlegend - 1) := (others => TUB(" "));
    begin
        max_height := 0.0;
        xstart     := 0.0;
        ystart     := 0.1;
        x          := xstart;
        y          := ystart;
        text_scale := 0.90;
        pladv(0);
        plvpor(0.0, 1.0, 0.0, 0.90);
        plwind(0.0, 1.0, 0.0, 1.0);
        plsfont(Pl_Fci_Sans, -1, -1);
        plmtex("t", 2.0, 0.5, 0.5, "Demonstrate Various Kinds of Legends");

        -- Only specify legend data that are required according to the
        -- value of opt_array for that entry.
        position := Pl_Position_Left + Pl_Position_Top;
        opt_base := Pl_Legend_Background + Pl_Legend_Bounding_Box + Pl_Legend_Text_Left;

        -- Set up None, Box, Line, Symbol, and Line & Symbol legend entries.
        opt_array(0) := Pl_Legend_None;
        text(0) := TUB("None");
        text_colors(0) := 1;

        opt_array(1) := Pl_Legend_Color_Box;
        text(1) := TUB("Box");
        text_colors(1)     := 2;
        box_colors(1)      := 2;
        box_patterns(1)    := 0;
        box_scales(1)      := 0.8;
        box_line_widths(1) := 1.0;

        opt_array(2) := Pl_Legend_Line;
        text(2) := TUB("Line");
        text_colors(2) := 3;
        line_colors(2) := 3;
        line_styles(2) := 1;
        line_widths(2) := 1.0;

        opt_array(3) := Pl_Legend_Symbol;
        text(3) := TUB("Symbol");
        text_colors(3)    := 4;
        symbol_colors(3)  := 4;
        symbol_scales(3)  := text_scale;
        symbol_numbers(3) := 4;
        symbols(3)        := special_symbols(2);

        opt_array(4) := Pl_Legend_Symbol + Pl_Legend_Line;
        text(4) := TUB("L & S");
        text_colors(4)    := 5;
        line_colors(4)    := 5;
        line_styles(4)    := 1;
        line_widths(4)    := 1.0;
        symbol_colors(4)  := 5;
        symbol_scales(4)  := text_scale;
        symbol_numbers(4) := 4;
        symbols(4)        := special_symbols(2);

        opt := opt_base;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);

        -- Set up symbol legend entries with various symbols.
        for i in 0 .. nlegend - 1 loop
            opt_array(i) := Pl_Legend_Symbol;
            text(i) := TUB("Symbol ") & special_symbols(i);
            text_colors(i)    := i + 1;
            symbol_colors(i)  := i + 1;
            symbol_scales(i)  := text_scale;
            symbol_numbers(i) := 4;
            symbols(i)        := special_symbols(i);
        end loop;

        opt := opt_base;
        x  := x + legend_width;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);

        -- Set up symbol legend entries with various numbers of symbols.
        for i in 0 .. nlegend - 1 loop
            opt_array(i) := Pl_Legend_Symbol;
            text(i) := TUB("Symbol Number" & Integer'image(i + 2));
            text_colors(i)    := i + 1;
            symbol_colors(i)  := i + 1;
            symbol_scales(i)  := text_scale;
            symbol_numbers(i) := i + 2;
            symbols(i)        := special_symbols(2);
        end loop;

        opt := opt_base;
        x  := x + legend_width;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            Box_Colors, Box_Patterns,
            Box_Scales, Box_Line_Widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);

        -- Set up box legend entries with various colours.
        for i in 0 .. nlegend - 1 loop
            opt_array(i) := Pl_Legend_Color_Box;
            text(i) := TUB("Box Color" & Integer'image(i + 1));
            text_colors(i)     := i + 1;
            box_colors(i)      := i + 1;
            box_patterns(i)    := 0;
            box_scales(i)      := 0.8;
            box_line_widths(i) := 1.0;
        end loop;

        opt := opt_base;
        -- Use new origin
        x          := xstart;
        y         := y + max_height;
        max_height := 0.0;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);

        -- Set up box legend entries with various patterns.
        for i in 0 .. nlegend - 1 loop
            opt_array(i) := Pl_Legend_Color_Box;
            text(i) := TUB("Box Pattern" & Integer'image(i));
            text_colors(i)     := 2;
            box_colors(i)      := 2;
            box_patterns(i)    := i;
            box_scales(i)      := 0.8;
            box_line_widths(i) := 1.0;
        end loop;

        opt := opt_base;
        x  := x + legend_width;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);

        -- Set up box legend entries with various box pattern line widths.
        for i in 0 .. nlegend - 1 loop
            opt_array(i) := Pl_Legend_Color_Box;
            text(i) := TUB("Box Line Width" & Integer'image(i + 1));
            text_colors(i)     := 2;
            box_colors(i)      := 2;
            box_patterns(i)    := 3;
            box_scales(i)      := 0.8;
            box_line_widths(i) := Long_Float(i) + 1.0;
        end loop;

        opt := opt_base;
        x  := x + legend_width;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);

        -- Set up line legend entries with various colours.
        for i in 0 .. nlegend - 1 loop
            opt_array(i) := Pl_Legend_Line;
            text(i) := TUB("Line Color" & Integer'image(i + 1));
            text_colors(i) := i + 1;
            line_colors(i) := i + 1;
            line_styles(i) := 1;
            line_widths(i) := 1.0;
        end loop;

        opt := opt_base;
        -- Use new origin
        x := xstart;
        y := y + max_height;
        max_height := 0.0;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);

        -- Set up line legend entries with various styles.
        for i in 0 .. nlegend - 1 loop
            opt_array(i) := Pl_Legend_Line;
            text(i) := TUB("Line Style" & Integer'image(i + 1));
            text_colors(i) := 2;
            line_colors(i) := 2;
            line_styles(i) := i + 1;
            line_widths(i) := 1.0;
        end loop;

        opt := opt_base;
        x  := x + legend_width;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);

        -- Set up line legend entries with various widths.
        for i in 0 .. nlegend - 1 loop
            opt_array(i) := Pl_Legend_Line;
            text(i) := TUB("Line Width" & Integer'image(i + 1));
            text_colors(i) := 2;
            line_colors(i) := 2;
            line_styles(i) := 1;
            line_widths(i) := Long_Float(i) + 1.0;
        end loop;

        opt := opt_base;
        x  := x + legend_width;
        plscol0a(15, 32, 32, 32, 0.70);

        pllegend(legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0.0, text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols);
        max_height := Long_Float'max(max_height, legend_height);
    end; -- declare

    if colorbar then
        -- Color bar examples
        declare
            values_small : Real_Vector(0 .. 1)  := (-1.0e-20, 1.0e-20);
            values_uneven : Real_Vector(0 .. 8) := (-1.0e-20, 2.0e-20, 2.6e-20, 3.4e-20,
                6.0e-20, 7.0e-20, 8.0e-20, 9.0e-20, 10.0e-20);
            values_even : Real_Vector(0 .. 8)   := (-2.0e-20, -1.0e-20, 0.0e-20, 1.0e-20,
                2.0e-20, 3.0e-20, 4.0e-20, 5.0e-20, 6.0e-20);
        begin
            -- Use unsaturated green background colour to contrast with black caps.
            plscolbg(70, 185, 70);
            -- Cut out the greatest and smallest bits of the color spectrum to
            -- leave colors for the end caps.
            plscmap1_range(0.01, 0.99);

            -- We can only test image and gradient colorbars with two element arrays
            for i in 2 .. COLORBAR_KINDS - 1 loop
                plcolorbar_example("cmap1_blue_yellow.pal", i, 0, 0.0, 2, values_small);
            end loop;
            -- Test shade colorbars with larger arrays
            for i in 0 .. 1 loop
                plcolorbar_example("cmap1_blue_yellow.pal", i, 4, 2.0, 9, values_even);
            end loop;
            for i in 0 .. 1 loop
                plcolorbar_example("cmap1_blue_yellow.pal", i, 0, 0.0, 9, values_uneven);
            end loop;
        end;
    end if;
    plend;
end xtraditional33a;

