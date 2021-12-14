-- plimage demo

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
    Interfaces.C,
    System,
    Ada.Sequential_IO,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    Ada.Text_IO,
    Interfaces.C,
    System,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

procedure xtraditional20a is

    XDIM : constant Integer := 260;
    YDIM : constant Integer := 220;
    PLK_Return : constant unsigned := 16#0D#;
    dbg : Integer := 0;
    nosombrero : Integer := 0;
    nointeractive : Integer := 0;
    x : Real_Vector(0 .. XDIM - 1);
    y : Real_Vector(0 .. YDIM - 1);
    z, r : Real_Matrix(0 .. XDIM - 1, 0 .. YDIM - 1);
    xi, yi, xe, ye : Long_Float;
    width, height, num_col : Integer;
    img_f : Real_Matrix(0 .. 310, 0 .. 239); -- Chloe is width 311, height 240.
    img_min, img_max : Long_Float;
    Get_Clip_Return : Integer;
    type stretch_data is
        record
            xmin, xmax, ymin, ymax : Long_Float;
            stretch                : Long_Float;
        end record;
    stretch : stretch_data;
    xx, yy : Long_Float;
    f_name : String(1 .. 200);
    f_name_length : Integer;
    Save_Sombrero : Boolean := False;

    -- Read image from file in binary ppm format.
    procedure read_img
       (fname : String; img_f : out Real_Matrix;
        width, height, num_col : out Integer)
    is
        type Byte is mod 2 ** 8;
        A_Byte : Byte;
        package Chloe_IO is new Ada.Sequential_IO(Byte);
        use Chloe_IO;
        Input_File : Chloe_IO.File_Type;
    begin
        -- Naive grayscale binary ppm reading. If you know how to, improve it.
        -- Mine is naiver than yours.
        -- Chloe.pgm has 15 bytes of header followed by 311 * 240 bytes of 8-bit pixels.
        Chloe_IO.Open(Input_File, In_File, fname);

        for i in 1 .. 15 loop
            Chloe_IO.Read(Input_File, A_Byte);
        end loop;

        width  := 311; -- columns
        height := 240; -- rows
        num_col := 255; -- number of colors

        for j in img_f'range(2) loop
            for i in img_f'range(1) loop
                Chloe_IO.Read(Input_File, A_Byte);
                img_f(i, height - j - 1) := Long_Float(A_Byte); -- Flip image up-down.
            end loop;
        end loop;
    end read_img;


    -- Save plot.
    procedure save_plot(fname : String) is
        cur_strm, new_strm : Integer;
    begin
        plgstrm(cur_strm); -- Get current stream.
        plmkstrm(new_strm); -- Create a new one.
        plsdev("psc"); -- New device type. Use a known existing driver.
        plsfnam(fname); -- Set the file name.
        plcpstrm(cur_strm, False); -- Copy old stream parameters to new stream.
        plreplot;	-- Do the save.
        plend1; -- Close new device...
        plsstrm(cur_strm);	-- ...and return to previous one.
    end save_plot;


    -- Get selection square interactively.
    procedure get_clip(xi, xe, yi, ye : in out Long_Float; Return_This : out Integer) is
        gin : Graphics_Input_Record_Type;
        xxi : Long_Float := xi;
        yyi : Long_Float := yi;
        xxe : Long_Float := xe;
        yye : Long_Float := ye;
        t : Long_Float;
        start : Integer := 0;
        st : Boolean := False;
        sx, sy : Real_Vector(0 .. 4);
    begin
        plxormod(True, st); -- Enter xor mode to draw a selection rectangle.

        if st then -- Driver has xormod capability. Continue.
            loop
                plxormod(False, st);
                plGetCursor(gin);
                plxormod(True, st);

                if gin.button = 1 then
                    xxi := gin.wX;
                    yyi := gin.wY;
                    if start /= 0 then
                        -- Suppress warning """sy"" may be referenced before it has a value".
                        pragma Warnings(Off);
                        plline(sx, sy); -- Clear previous rectangle.
                        pragma Warnings(On);
                    end if;

                    start := 0;

                    sx(0) := xxi;
                    sy(0) := yyi;
                    sx(4) := xxi;
                    sy(4) := yyi;
                end if;

                if (gin.state and Unsigned(16#100#)) /= 0 then
                    xxe := gin.wX;
                    yye := gin.wY;

                    if start /= 0 then
                        plline(sx, sy); -- Clear previous rectangle.
                    end if;

                    start := 1;

                    sx(2) := xxe;
                    sy(2) := yye;
                    sx(1) := xxe;
                    sy(1) := yyi;
                    sx(3) := xxi;
                    sy(3) := yye;
                    plline(sx, sy); -- Draw new rectangle.
                end if;

                if gin.button = 3 or gin.keysym = PLK_Return or
                    gin.keysym = unsigned(Character'pos('Q')) then
                    if start /= 0 then
                        plline(sx, sy); -- Clear previous rectangle.
                    end if;
                    exit;
                end if;
            end loop;

            plxormod(False, st); -- Leave xor mod.

            if xxe < xxi then
                t   := xxi;
                xxi := xxe;
                xxe := t;
            end if;

            if yyi < yye then
                t   :=yyi;
                yyi := yye;
                yye := t;
            end if;

            xe := xxe;
            xi := xxi;
            ye := yye;
            yi := yyi;

            if gin.keysym = unsigned(Character'pos('Q')) then
                Return_This := 1;
            else
                Return_This := 0;
            end if;
        else -- Driver has no xormod capability; just do nothing.
            Return_This := 0;
        end if;
    end get_clip;


    -- Set gray colormap.
    procedure gray_cmap(num_col : Integer) is
        r, g, b, pos : Real_Vector(0 .. 1);
    begin
        r(0) := 0.0;
        g(0) := 0.0;
        b(0) := 0.0;

        r(1) := 1.0;
        g(1) := 1.0;
        b(1) := 1.0;

        pos(0) := 0.0;
        pos(1) := 1.0;

        plscmap1n(num_col);
        plscmap1l(RGB, pos, r, g, b, Alt_Hue_Path_None);
    end gray_cmap;


    procedure mypltr
       (x, y   : Long_Float;
        tx, ty : out Long_Float;
        s      : stretch_data)
    is
        x0, y0, dy : Long_Float;
    begin
        x0 := (s.xmin + s.xmax) * 0.5;
        y0 := (s.ymin + s.ymax) * 0.5;
        dy := (s.ymax - s.ymin) * 0.5;
        tx := x0 + (x0 - x) * (1.0 - s.stretch * cos((y - y0) / dy * pi * 0.5));
        ty := y;
    end mypltr;

begin

    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot
    plinit;

    -- View image border pixels.
    if dbg /= 0 then
        plenv(1.0, Long_Float(XDIM), 1.0, Long_Float(YDIM), 1, 1); -- no plot box

        -- Build a one pixel square border, for diagnostics.
        for i in z'range(1) loop
            z(i, YDIM - 1) := 1.0; -- right
        end loop;

        for i in z'range(1) loop
            z(i, 0) := 1.0; -- left
        end loop;

        for i in z'range(2) loop
            z(0, i) := 1.0; -- top
        end loop;

        for i in z'range(2) loop
            z(XDIM - 1, i) := 1.0; -- botton
        end loop;

        pllab("...around a blue square."," ","A red border should appear...");

        plimage(z,
            1.0, Long_Float(XDIM), 1.0, Long_Float(YDIM), 0.0, 0.0,
            1.0, Long_Float(XDIM), 1.0, Long_Float(YDIM));
    end if;

    -- sombrero-like demo
    if nosombrero = 0 then
        plcol0(2); -- Draw a yellow plot box, useful for diagnostics! :(
        plenv(0.0, 2.0 * pi, 0.0, 3.0 * pi, 1, -1);

        for i in x'range loop
            x(i) := Long_Float(i) * 2.0 * pi / Long_Float(XDIM - 1);
        end loop;

        for i in y'range loop
            y(i) := Long_Float(i) * 3.0 * pi / Long_Float(YDIM - 1);
        end loop;

        for i in z'range(1) loop
            for j in z'range(2) loop
                r(i, j) := sqrt(x(i) * x(i) + y(j) * y(j)) + 1.0e-3;
                z(i, j) := sin(r(i, j)) / (r(i, j));
            end loop;
        end loop;

        pllab("No, an amplitude clipped ""sombrero""", "", "Saturn?");
        plptex(2.0, 2.0, 3.0, 4.0, 0.0, "Transparent image");
        plimage(z, 0.0, 2.0 * pi, 0.0, 3.0 * pi, 0.05, 1.0,
            0.0, 2.0 * pi, 0.0, 3.0 * pi);

        -- Save the plot.
        if Save_Sombrero then
            Put("Enter a path and name to save the Postscript file or RETURN to not save: ");
            Get_Line(f_name, f_name_length);
            if f_name'Length /= 0 then
                save_plot(f_name(1 .. f_name_length));
            end if;
        end if;
    end if;

    -- Read the Chloe image.
    -- Note we try two different locations to cover the case where this
    -- examples is being run from the test_c.sh script.
    begin
        read_img("./Chloe.pgm", img_f, width, height, num_col);
    exception
        when NAME_ERROR =>
            null;
        begin
            read_img("../Chloe.pgm", img_f, width, height, num_col);
        exception
            when NAME_ERROR =>
                Put_Line("Failed to open Chloe.pgm. Aborting.");
                plend;
                return;
        end; -- second exception block
    end; -- first exception block

    -- Set gray colormap.
    gray_cmap(num_col);

    -- Display Chloe.
    plenv(1.0, Long_Float(width), 1.0, Long_Float(height), 1, -1);

    if nointeractive = 0 then
        pllab("Set and drag Button 1 to (re)set selection, Button 2 to finish."," ","Chloe...");
    else
        pllab(""," ","Chloe...");
    end if;

    plimage(img_f, 1.0, Long_Float(width), 1.0, Long_Float(height), 0.0, 0.0, 1.0,
        Long_Float(width), 1.0, Long_Float(height));

    -- Selection/expansion demo
    if nointeractive = 0 then
        xi := 25.0;
        xe := 130.0;
        yi := 235.0;
        ye := 125.0;

        get_clip(xi, xe, yi, ye, Get_Clip_Return); -- get selection rectangle
        if Get_Clip_Return /= 0 then
            plend;
        end if;

        -- I'm unable to continue, clearing the plot and advancing to the next
        -- one, without hiting the enter key, or pressing the button... help!

        -- Forcing the xwin driver to leave locate mode and destroying the
        -- xhairs (in GetCursorCmd()) solves some problems, but I still have
        -- to press the enter key or press Button-2 to go to next plot, even
        -- if a pladv() is not present!  Using plbop() solves the problem, but
        -- it shouldn't be needed!

        -- plbop();

        -- plspause(0), pladv(0), plspause(1), also works,
        -- but the above question remains.
        -- With this approach, the previous pause state is lost,
        -- as there is no API call to get its current state.

        plspause(False);
        pladv(0);

        -- Display selection only.
        plimage(img_f, 1.0, Long_Float(width), 1.0, Long_Float(height), 0.0, 0.0, xi, xe, ye, yi);

        plspause(True);

        -- Zoom in selection.
        plenv(xi, xe, ye, yi, 1, -1);
        plimage(img_f, 1.0, Long_Float(width), 1.0, Long_Float(height), 0.0, 0.0, xi, xe, ye, yi);
    end if;

    -- Base the dynamic range on the image contents.
    img_min := Matrix_Min(img_f);
    img_max := Matrix_Max(img_f);

    -- Draw a saturated version of the original image. Use only the middle 50%
    -- of the image's full dynamic range.
    plcol0(2);
    plenv(0.0, Long_Float(width), 0.0, Long_Float(height), 1, -1);
    pllab("", "", "Reduced dynamic range image example");
    plimagefr(img_f, 0.0, Long_Float(width), 0.0, Long_Float(height), 0.0, 0.0,
        img_min + img_max * 0.25, img_max - img_max * 0.25, Null, System.Null_Address);

    -- Draw a distorted version of the original image, showing its full dynamic range.
    plenv(0.0, Long_Float(width), 0.0, Long_Float(height), 1, -1);
    pllab("", "", "Distorted image example");

    stretch.xmin := 0.0;
    stretch.xmax := Long_Float(width);
    stretch.ymin := 0.0;
    stretch.ymax := Long_Float(height);
    stretch.stretch := 0.5;

    -- In C / C++ the following would work, with plimagefr directly calling
    -- mypltr. For compatibilty with other language bindings the same effect
    -- can be achieved by generating the transformed grid first and then
    -- using pltr2.
    -- plimagefr(img_f, width, height, 0., width, 0., height, 0., 0., img_min, img_max, mypltr, &stretch);

    declare -- Declare block is based on runtime-determined values of width, height.
        cgrid2 : aliased Transformation_Data_Type_2
           (x_Last => width,
            y_Last => height);
    begin
        for i in 0 .. width loop
            for j in 0 .. height loop
                mypltr(Long_Float(i), Long_Float(j), xx, yy, stretch);
                cgrid2.xg(i, j) := xx;
                cgrid2.yg(i, j) := yy;
            end loop;
        end loop;

        plimagefr(img_f, 0.0, Long_Float(width), 0.0, Long_Float(height), 0.0, 0.0, img_min, img_max,
            pltr2'access, cgrid2'Address);
    end;
    plend;
end xtraditional20a;
