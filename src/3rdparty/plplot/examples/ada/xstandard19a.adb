--	Illustrates backdrop plotting of world, US maps.
--	Contributed by Wesley Ebisuzaki.

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
    Text_IO,
    Interfaces.C,
    System.Storage_Elements,
    Ada.Strings,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    Ada.Strings.Unbounded,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Text_IO,
    Interfaces.C,
    System.Storage_Elements,
    Ada.Strings,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    Ada.Strings.Unbounded,
    PLplot_Auxiliary,
    PLplot_Standard;

-- Shows two views of the world map.
procedure xstandard19a is
    NL : Character := Character'Val(10); -- "New Line," replaces C's \n.
    minx, maxx, miny, maxy : Long_Float;
    x, y : Real_Vector(1 .. 1);

    -- Variables for the shapelib example
    nbeachareas : Integer := 2;
    beachareas : Integer_Array_1D(0 .. 1) := (23, 24);
    woodlandareas : Integer_Array_1D(0 .. 93);
    nshingleareas : Integer := 22;
    shingleareas : Integer_Array_1D := (0, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 217,
        2424, 2425, 2426, 2427, 2428, 2491, 2577);
    ncragareas : Integer := 2024;
    cragareas : Integer_Array_1D(0 .. 2023);
    majorroads : Integer_Array_1D  := (33, 48, 71, 83, 89, 90, 101, 102, 111);

    -- This spec is necessary in order to enforce C calling conventions, used
    -- in the callback by intervening C code.
    procedure map_transform
       (x, y   : Long_Float;
        xt, yt : out Long_Float;
        data   : PL_Pointer);
    pragma Convention(C, map_transform);

    procedure map_transform
       (x, y   : Long_Float;
        xt, yt : out Long_Float;
        data   : PL_Pointer)
    is
        radius : Long_Float;
    begin
        radius := 90.0 - y;
        xt := radius * cos(x * pi / 180.0);
        yt := radius * sin(x * pi / 180.0);
    end map_transform;


    -- This spec is necessary in order to enforce C calling conventions, used
    -- in the callback by intervening C code.
    procedure mapform19(n : Integer; x, y : in out Map_Form_Constrained_Array);
    pragma Convention(C, mapform19);

    -- Defines specific coordinate transformation for example 19.
    -- Not to be confused with mapform in src/plmap.c.
    -- x(), y() are the coordinates to be plotted.

    -- Ada note: Passing the array length as the first argument is the easiest
    -- way (for the developer of the bindings) and maintain a C-compatible
    -- argument list. It might be possible to instead pass a pointer to something
    -- with an argument list (x, y : in out Real_Vector) instead, and write a
    -- wrapper function inside Draw_Map and Draw_Latitude_Longitude that has the "correct" C
    -- argument list, and then pass a pointer to _that_ when calling Draw_Map and
    -- plmeridian.
    procedure mapform19(n : Integer; x, y : in out Map_Form_Constrained_Array) is
        xp, yp : Long_Float;
    begin
         -- DO NOT use x'range for this loop because the C function which calls
         -- this function WILL NOT SEE IT AND YOU WILL GET A SEGFAULT. Simply
         -- use 0 .. n - 1 explicitly.
        for i in 0 .. n - 1 loop
            map_transform(x(i), y(i), xp, yp, System.Null_Address);
            x(i) := xp;
            y(i) := yp;
        end loop;
    end mapform19;

    -- This spec is necessary in order to enforce C calling conventions, used
    -- in the callback by intervening C code.
    -- See Ada note above, for mapform19.
    -- a_length is used only for bounds checking against the C-allocated memory
    -- for label.
    procedure geolocation_labeler
       (axis     : Integer;
        a_value  : Long_Float;
        label    : out Label_String_Type;
        a_length : size_t;
        data     : PL_Pointer);
    pragma Convention(C, geolocation_labeler);

    -- A custom axis labeling function for longitudes and latitudes.
    procedure geolocation_labeler
       (axis     : Integer;
        a_value  : Long_Float;
        label    : out Label_String_Type;
        a_length : size_t;
        data     : PL_Pointer)
    is
        direction_label : Unbounded_String;
        label_val : Long_Float;

        -- "Normalize" longitude values so that they always fall between -180.0 and 180.0.
        function normalize_longitude(lon : Long_Float) return Long_Float is
            times : Long_Float;
        begin -- normalize_longitude
            if lon >= -180.0 and lon <= 180.0 then
                return lon;
            else
                times := Long_Float'Floor((abs(lon) + 180.0) / 360.0);
                if lon < 0.0 then
                    return lon + 360.0 * times;
                else
                    return lon - 360.0 * times;
                end if;
            end if;
        end normalize_longitude;

        -- Function to convert an unbounded string to a fixed-length C string with the
        -- null terminator somewhere in the middle and spaces after. The result, of type
        -- Label_String_Type, is fixed to a length by C, currently at 41, and is
        -- indexed in Ada as 0 .. PLplot_Traditional.Max_Label_String_Length.
        function Unbounded_To_Weird_C
           (Item     : Unbounded_String;
            C_Length : size_t) return Label_String_Type
        is
            Temp : Unbounded_String;
        begin
            -- Check length and adjust if necessary.
            if Length(Item) >= Integer(C_Length) then
                Put_Line("*** Warning: Custom label was truncated to"
                    & Integer'Image(Integer(C_Length)) & " characters. ***");
                Temp := Head(Item, Integer(C_Length));
                return To_C(To_String(Temp), True);
            else
               return To_C(To_String(Item & ASCII.nul &
                    (Max_Label_String_Length - Length(Item)) * " "), False);
            end if;
        end Unbounded_To_Weird_C;

     begin -- geolocation_labeler
        if axis = Label_Y_Axis then
            label_val := a_value;
            if label_val > 0.0 then
                direction_label := To_Unbounded_String(" N");
            elsif label_val < 0.0 then
                direction_label := To_Unbounded_String(" S");
            else
                direction_label := To_Unbounded_String("Eq");
            end if;
        elsif axis = Label_X_Axis then
            label_val := normalize_longitude(a_value);
            if label_val > 0.0 then
                direction_label := To_Unbounded_String(" E");
            elsif label_val < 0.0 then
                direction_label := To_Unbounded_String(" W");
            else
                direction_label := To_Unbounded_String("");
            end if;
        end if;

        if axis = Label_Y_Axis and a_value = 0.0 then
            -- A special case for the equator
            null;
        else
            direction_label := Trim(Integer'Image(Integer(abs(label_val))) & direction_label, Left);
        end if;
        label := Unbounded_To_Weird_C(direction_label, a_length);
    end geolocation_labeler;

begin

    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    Initialize_PLplot;

    -- Longitude (x) and latitude (y)
    miny := -70.0;
    maxy := 80.0;

    -- Cartesian plots
    -- Most of world
    minx := -170.0;
    maxx := minx + 360.0;

    -- Setup a custom latitude and longitude-based scaling function.
    Set_Custom_Label(geolocation_labeler'Unrestricted_Access, System.Null_Address);

    Set_Pen_Color(Red);
    Set_Environment(minx, maxx, miny, maxy, Justified, Custom_Labels_Linear_Box_Plus);
    Draw_Map(null, USA_States_and_Continents, minx, maxx, miny, maxy);

    -- The Americas
    minx := 190.0;
    maxx := 340.0;

    Set_Pen_Color(Red);
    Set_Environment(minx, maxx, miny, maxy, Justified, Custom_Labels_Linear_Box_Plus);
    Draw_Map(null, USA_States_and_Continents, minx, maxx, miny, maxy);

    -- Clear the labeling function.
    Use_Default_Labels;
    -- or...
    -- plslabelfunc(Null, System.Null_Address);

    -- Polar, Northern hemisphere
    minx := 0.0;
    maxx := 360.0;

    Set_Environment(-75.0, 75.0, -75.0, 75.0, Justified, Box);
    Draw_Map(mapform19'Unrestricted_Access, Continents, minx, maxx, miny, maxy);

    Select_Line_Style(Line_Style => 2);
    Draw_Latitude_Longitude(mapform19'Unrestricted_Access, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0);

    -- Polar, Northern hemisphere, this time with a PLplot-wide transform
    minx := 0.0;
    maxx := 360.0;

    Set_Custom_Coordinate_Transform(map_transform'Unrestricted_Access, System.Null_Address);

    Select_Line_Style(1);
    Set_Environment(-75.0, 75.0, -75.0, 75.0, Justified, Box);

    -- No need to set the map transform here as the global transform will be used.
    Draw_Map(null, Continents, minx, maxx, miny, maxy);

    Select_Line_Style(2);
    Draw_Latitude_Longitude(null, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0);

    -- Show Baltimore, MD on the map.
    Set_Pen_Color(Yellow);
    Set_Symbol_Size(0.0, 2.0);
    x(1) := -76.6125;
    y(1) := 39.2902778;
    Draw_Points(x, y, 18);
    Set_Symbol_Size(0.0, 1.0);
    Write_Text_World(-76.6125, 43.0, 0.0, 0.0, 0.0, "Baltimore, MD");

    -- Clear the global transform.
    Clear_Custom_Coordinate_Transform;
    -- or...
    -- Set_Custom_Coordinate_Transform(null, System.Null_Address);

    -- An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
    -- These were chosen because they provide shapefiles for small grid boxes which
    -- are easilly manageable for this demo.
    Select_Line_Style(1);
    minx := 240570.0;
    maxx := 621109.0;
    miny := 87822.0;
    maxy := 722770.0;
    Set_One_Color_Map_0(0, 255, 255, 255);
    Set_One_Color_Map_0(1, 0, 0, 0);
    Set_One_Color_Map_0(2, 150, 150, 150);
    Set_One_Color_Map_0(3, 0, 50, 200);
    Set_One_Color_Map_0(4, 50, 50, 50);
    Set_One_Color_Map_0(5, 150, 0, 0);
    Set_One_Color_Map_0(6, 100, 100, 255);

    minx := 265000.0;
    maxx := 270000.0;
    miny := 145000.0;
    maxy := 150000.0;
    Set_One_Color_Map_0(0, 255, 255, 255); --white
    Set_One_Color_Map_0(1, 0, 0, 0);       --black
    Set_One_Color_Map_0(2, 255, 200, 0);   --yelow for sand
    Set_One_Color_Map_0(3, 60, 230, 60);   -- green for woodland
    Set_One_Color_Map_0(4, 210, 120, 60);  --brown for contours
    Set_One_Color_Map_0(5, 150, 0, 0);     --red for major roads
    Set_One_Color_Map_0(6, 180, 180, 255); --pale blue for water
    Set_One_Color_Map_0(7, 100, 100, 100); --pale grey for shingle or boulders
    Set_One_Color_Map_0(8, 100, 100, 100); --dark grey for custom polygons - generally crags

    Set_Pen_Color(Red);
    Set_Environment(minx, maxx, miny, maxy, Justified, Box);
    Write_Labels("", "", "Martinhoe CP, Exmoor National Park, UK (shapelib only)");

    --Beach
    Set_Pen_Color(Yellow);
    Plot_Shapefile(null, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, beachareas);

    -- Woodland
    Set_Pen_Color(Green);
    for i in woodlandareas'range loop
        woodlandareas(i) := i + 218;
    end loop;
    Plot_Shapefile(null, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, woodlandareas);

    -- Shingle or boulders
    Set_Pen_Color(Grey);
    Plot_Shapefile(null, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, shingleareas);

    -- Crags
    Set_Pen_Color(Brown);
    for i in 0 .. ncragareas - 1 loop
        cragareas(i) := i + 325;
    end loop;
    Plot_Shapefile(null, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, cragareas);

    -- Draw contour; we need to separate contours from high/low coastline.
    -- draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy);
    Set_Pen_Color(Aquamarine);
    -- Ada note: Use overload permitting null address to be passed in plotentries slot.
    Plot_Shapefile_World(null, "ss/ss64ne_Height_Contours", minx, maxx, miny, maxy, System.Null_Address);

    -- Draw the sea and surface water.
    Set_Pen_Width(0.0);
    Set_Pen_Color(Wheat);
    -- Ada note: Use overload permitting null address to be passed in plotentries slot.
    Plot_Shapefile(null, "ss/ss64ne_Water_Area", minx, maxx, miny, maxy, System.Null_Address);
    Set_Pen_Width(2.0);
    Plot_Shapefile_World(null, "ss/ss64ne_Water_Line", minx, maxx, miny, maxy, System.Null_Address);

    -- Draw the roads, first with black and then thinner with colour to give an
    -- an outlined appearance.
    Set_Pen_Width(5.0);
    Set_Pen_Color(Red);
    -- Ada note: Use another overload; this one doesn't have the plotentries slot.
    Plot_Shapefile_World_All(null, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy);
    Set_Pen_Width(3.0);
    Set_Pen_Color(Black);
    Plot_Shapefile_World(null, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, System.Null_Address);
    Set_Pen_Color(Pink);
    Plot_Shapefile_World(null, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, majorroads);

    -- Draw buildings.
    Set_Pen_Width(1.0);
    Set_Pen_Color(Red);
    Plot_Shapefile_All(null, "ss/ss64ne_Building_Area", minx, maxx, miny, maxy);

    -- Labels; Character'Val(10) is ASCII for new line, replacing C's \n.
    Set_Font_Characterization_Integer(16#80000100#);
    Set_Character_Height(0.0, 0.8);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "MARTINHOE CP", minx, maxx, miny, maxy, 202);
    Set_Character_Height(0.0, 0.7);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale" & NL & "Down", minx, maxx, miny, maxy, 13);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "South" & NL & "Down", minx, maxx, miny, maxy, 34);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Martinhoe" & NL & "Common", minx, maxx, miny, maxy, 42);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woody Bay", minx, maxx, miny, maxy, 211);
    Set_Character_Height(0.0, 0.6);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Mill Wood", minx, maxx, miny, maxy, 16);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale Wood", minx, maxx, miny, maxy, 17);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 1.0, "Bodley", minx, maxx, miny, maxy, 31);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.0, "Martinhoe", minx, maxx, miny, maxy, 37);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woolhanger" & NL & "Common", minx, maxx, miny, maxy, 60);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "West Ilkerton" & NL & "Common", minx, maxx, miny, maxy, 61);
    Draw_Shapefile_Text_World(null, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Caffyns" & NL & "Heanton" & NL & "Down", minx, maxx, miny, maxy, 62);

    End_PLplot;
end xstandard19a;
