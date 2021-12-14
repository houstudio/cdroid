-- Displays Greek letters and mathematically interesting Unicode ranges.

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
    Ada.Integer_Text_IO,
    Ada.Strings.Unbounded,
    Ada.Characters.Handling,
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Text_IO,
    Ada.Integer_Text_IO,
    Ada.Strings.Unbounded,
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard23a is

    Greek : array(0 .. 47) of String(1 .. 3) := (
    "#gA","#gB","#gG","#gD","#gE","#gZ","#gY","#gH","#gI","#gK","#gL","#gM",
    "#gN","#gC","#gO","#gP","#gR","#gS","#gT","#gU","#gF","#gX","#gQ","#gW",
    "#ga","#gb","#gg","#gd","#ge","#gz","#gy","#gh","#gi","#gk","#gl","#gm",
    "#gn","#gc","#go","#gp","#gr","#gs","#gt","#gu","#gf","#gx","#gq","#gw");

    Type1 : array (0 .. 165) of Integer := (
    16#0020#,16#0021#,16#0023#,16#0025#,16#0026#,
    16#0028#,16#0029#,16#002b#,16#002c#,16#002e#,
    16#002f#,16#0030#,16#0031#,16#0032#,16#0033#,
    16#0034#,16#0035#,16#0036#,16#0037#,16#0038#,
    16#0039#,16#003a#,16#003b#,16#003c#,16#003d#,
    16#003e#,16#003f#,16#005b#,16#005d#,16#005f#,
    16#007b#,16#007c#,16#007d#,16#00a9#,16#00ac#,
    16#00ae#,16#00b0#,16#00b1#,16#00d7#,16#00f7#,
    16#0192#,16#0391#,16#0392#,16#0393#,16#0394#,
    16#0395#,16#0396#,16#0397#,16#0398#,16#0399#,
    16#039a#,16#039b#,16#039c#,16#039d#,16#039e#,
    16#039f#,16#03a0#,16#03a1#,16#03a3#,16#03a4#,
    16#03a5#,16#03a6#,16#03a7#,16#03a8#,16#03a9#,
    16#03b1#,16#03b2#,16#03b3#,16#03b4#,16#03b5#,
    16#03b6#,16#03b7#,16#03b8#,16#03b9#,16#03ba#,
    16#03bb#,16#03bc#,16#03bd#,16#03be#,16#03bf#,
    16#03c0#,16#03c1#,16#03c2#,16#03c3#,16#03c4#,
    16#03c5#,16#03c6#,16#03c7#,16#03c8#,16#03c9#,
    16#03d1#,16#03d2#,16#03d5#,16#03d6#,16#2022#,
    16#2026#,16#2032#,16#2033#,16#203e#,16#2044#,
    16#2111#,16#2118#,16#211c#,16#2122#,16#2126#,
    16#2135#,16#2190#,16#2191#,16#2192#,16#2193#,
    16#2194#,16#21b5#,16#21d0#,16#21d1#,16#21d2#,
    16#21d3#,16#21d4#,16#2200#,16#2202#,16#2203#,
    16#2205#,16#2206#,16#2207#,16#2208#,16#2209#,
    16#220b#,16#220f#,16#2211#,16#2212#,16#2215#,
    16#2217#,16#221a#,16#221d#,16#221e#,16#2220#,
    16#2227#,16#2228#,16#2229#,16#222a#,16#222b#,
    16#2234#,16#223c#,16#2245#,16#2248#,16#2260#,
    16#2261#,16#2264#,16#2265#,16#2282#,16#2283#,
    16#2284#,16#2286#,16#2287#,16#2295#,16#2297#,
    16#22a5#,16#22c5#,16#2320#,16#2321#,16#2329#,
    16#232a#,16#25ca#,16#2660#,16#2663#,16#2665#,
    16#2666#);


    function title(which : Integer) return String is
    begin
        if which = 0  then return "#<0x10>PLplot Example 23 - Greek Letters";                            end if;
        if which = 1  then return "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (a)"; end if;
        if which = 2  then return "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (b)"; end if;
        if which = 3  then return "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (c)"; end if;
        if which = 4  then return "#<0x10>PLplot Example 23 - Number Forms Unicode Block";               end if;
        if which = 5  then return "#<0x10>PLplot Example 23 - Arrows Unicode Block (a)";                 end if;
        if which = 6  then return "#<0x10>PLplot Example 23 - Arrows Unicode Block (b)";                 end if;
        if which = 7  then return "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (a)"; end if;
        if which = 8  then return "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (b)"; end if;
        if which = 9  then return "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (c)"; end if;
        if which = 10 then return "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (d)"; end if;
        return "oops";
    end title;


    lo : array (0 .. 10) of Integer := (
    16#0#,
    16#0#,
    16#40#,
    16#80#,
    16#2153#,
    16#2190#,
    16#21d0#,
    16#2200#,
    16#2240#,
    16#2280#,
    16#22c0#
    );

    hi : array (0 .. 10) of Integer := (
    16#30#,
    16#40#,
    16#80#,
    16#A6#,
    16#2184#,
    16#21d0#,
    16#2200#,
    16#2240#,
    16#2280#,
    16#22c0#,
    16#2300#
    );

    nxcells : array (0 .. 10) of Integer := (
    12,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8
    );

    nycells : array (0 .. 10) of Integer := (
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8
    );

    -- non-zero values must be consistent with nxcells and nycells.
    offset : array (0 .. 10) of Integer := (
    0,
    0,
    64,
    128,
    0,
    0,
    0,
    0,
    0,
    0,
    0
    );

    -- 30 possible FCI values.
    FCI_COMBINATIONS : constant integer := 30;
    fci : array (0 .. 29) of Unicode := (
    16#80000000#,
    16#80000001#,
    16#80000002#,
    16#80000003#,
    16#80000004#,
    16#80000010#,
    16#80000011#,
    16#80000012#,
    16#80000013#,
    16#80000014#,
    16#80000020#,
    16#80000021#,
    16#80000022#,
    16#80000023#,
    16#80000024#,
    16#80000100#,
    16#80000101#,
    16#80000102#,
    16#80000103#,
    16#80000104#,
    16#80000110#,
    16#80000111#,
    16#80000112#,
    16#80000113#,
    16#80000114#,
    16#80000120#,
    16#80000121#,
    16#80000122#,
    16#80000123#,
    16#80000124#
    );


    function family(which : Integer) return String is
    begin
        if which = 0 then return "sans-serif"; end if;
        if which = 1 then return "serif";      end if;
        if which = 2 then return "monospace";  end if;
        if which = 3 then return "script";     end if;
        if which = 4 then return "symbol";     end if;
        return "oops";
    end family;


    function style(which : Integer) return String is
    begin
        if which = 0 then return "upright"; end if;
        if which = 1 then return "italic";  end if;
        if which = 2 then return "oblique"; end if;
        return "oops";
    end style;


    function weight(which : Integer) return String is
    begin
        if which = 0 then return "medium"; end if;
        if which = 1 then return "bold";   end if;
        return "oops";
    end weight;

    xmin, xmax, ymin, ymax, ycharacter_scale, yoffset : Long_Float;
    chardef, charht, deltax, deltay, x, y             : Long_Float;
    length, slice : Integer;
    cmdString : Unbounded_String;
    fci_old : Unicode;
    ifamily, istyle, iweight : Integer;
    dy : Long_Float := 0.030;
    family_index, style_index, weight_index : Integer;

    -- Must be big enough to contain the prefix strings, the font-changing
    -- commands, and the "The quick brown..." string.
    a_string : Unbounded_String;

    package Unicode_IO is new Ada.Text_IO.Modular_IO(Unicode); -- fix this probably not used
    use Unicode_IO;

    -- Make a four-character hexadecimal string image of integers.
    -- Useful for font manipulations.
    -- To_Lower is used to make C-like output for A..F
    -- so that the PostScript files generated by C and Ada are identical.
    -- Integer values greater than 65535 (2^16-1) are treated modulo 2^16.
    -- Based on Stephen Leake's SAL.Generic_Hex_Image.
    -- Original: Copyright (C) 2005 Stephen Leake.  All Rights Reserved, and
    -- offered under GNU General Public License version 2 or later.
    function Hex_Image_4 (Item : in Integer) return String is
        Temp : Integer := Item;
        Nibble : Integer;
        Image : String (1 .. 4);
    begin
        for I in reverse Image'Range loop
            Nibble := Temp mod 16;
            Temp := Temp / 16;
            if Nibble > 9 then
                Image (I) := Character'Val (Character'Pos ('A') + Nibble - 10);
            else
                Image (I) := Character'Val (Character'Pos ('0') + Nibble);
            end if;
        end loop;
        Image := Ada.Characters.Handling.To_Lower(Image);
        return Image;
    end Hex_Image_4;


    -- Make a eight-character hexadecimal string image of Unicode.
    function Hex_Image_8 (Item : in Unicode) return String is
        -- Long_Long_Integer so don't overflow Integer
        Temp : Long_Long_Integer := Long_Long_Integer(Item);
        Nibble : Long_Long_Integer;
        Image : String (1 .. 8);
    begin
        for I in reverse Image'Range loop
            Nibble := Temp mod 16;
            Temp := Temp / 16;
            if Nibble > 9 then
                Image (I) := Character'Val(Character'Pos ('A') + Nibble - 10);
            else
                Image (I) := Character'Val(Character'Pos ('0') + Nibble);
            end if;
        end loop;
        Image := Ada.Characters.Handling.To_Lower(Image);
        return Image;
    end Hex_Image_8;

begin
    Parse_Command_Line_Arguments(Parse_Full);

    Initialize_PLplot;

    for page in 0 .. 10 loop
        Advance_To_Subpage(Next_Subpage);

        -- Set up viewport and window
        Set_Viewport_Normalized(0.02, 0.98, 0.02, 0.90);
        Set_Viewport_World(0.0, 1.0, 0.0, 1.0);
        Get_Subpage_Boundaries(xmin, xmax, ymin, ymax);
        Set_Character_Height(0.0, 0.8);
        ycharacter_scale := (1.0 - 0.0) / (ymax - ymin);

        -- Factor should be 0.5, but heuristically it turns out to be larger.
        Get_Character_Height(chardef, charht);
        yoffset := 1.0 * charht * ycharacter_scale;

        -- Draw the grid using Box_Around_Viewport
        Set_Pen_Color(Yellow);
        deltax := 1.0 / Long_Float(nxcells(page));
        deltay := 1.0 / Long_Float(nycells(page));
        Box_Around_Viewport("bcg", deltax, 0, "bcg", deltay, 0);
        Set_Pen_Color(White);
        length := hi(page) - lo(page);
        slice := 0;
        for j in reverse -1 .. nycells(page) - 1 loop
            y := (0.5 + Long_Float(j)) * deltay;
            for i in 0 .. nxcells(page) - 1 loop
                x  := (0.5 + Long_Float(i)) * deltax;
                if slice < length then
                    if page = 0 then
                        cmdString := To_Unbounded_String(Greek(slice));

                    elsif page >= 1 and page <= 3 then
                        cmdString := To_Unbounded_String("#[0x" & Hex_Image_4(Type1(offset(page)+slice)) & "]");

                    elsif page >= 4 then
                        cmdString := To_Unbounded_String("#[0x" & Hex_Image_4(lo(page)+slice) & "]");
                    end if;
                    Write_Text_World(x, y + yoffset, 1.0, 0.0, 0.5, To_String(cmdString));
                    Write_Text_World(x, y - yoffset, 1.0, 0.0, 0.5, "#" & To_String(cmdString));
                end if;
                slice := slice + 1;
            end loop; -- i
        end loop; -- j

        Set_Character_Height(0.0, 1.0);

        -- Page title
        Write_Text_Viewport("t", 1.5, 0.5, 0.5, title(page));
    end loop; --page

    -- Demonstrate methods of getting the current fonts
    Get_Font_Characterization_Integer(fci_old);
    Get_Font(ifamily, istyle, iweight);
    Put("For example 23 prior to page 12 the FCI is ");

    -- The following hack in outputting the hexadecimal value of fci_old in "C"
    -- style, e.g. 0x00000000 instead of "Ada" style 16#00000000#, is done in
    -- order to create the same textual output as the C example x23c.
    -- Put(fci_old, Base => 16, Width => 0); -- Outputs "Ada" style.
    Put("0x" & Hex_Image_8(fci_old));        -- Outputs "C" style.
    New_Line;
    Put("For example 23 prior to page 12 the font family, style and weight are "
        & family(ifamily) & " " & style(istyle) & " " & weight(iweight));
    New_Line;

    for page in 11 .. 15 loop
        Advance_To_Subpage(Next_Subpage);
        Set_Viewport_Normalized(0.02, 0.98, 0.02, 0.90);
        Set_Viewport_World(0.0, 1.0, 0.0, 1.0);
        Set_Font_Characterization_Integer(0);
        if page = 11 then
            Write_Text_Viewport("t", 1.5, 0.5, 0.5,
            "#<0x10>PLplot Example 23 - " &
            "Set Font with plsfci");
        elsif page = 12 then
            Write_Text_Viewport("t", 1.5, 0.5, 0.5,
            "#<0x10>PLplot Example 23 - " &
            "Set Font with plsfont");
        elsif page = 13 then
            Write_Text_Viewport("t", 1.5, 0.5, 0.5,
            "#<0x10>PLplot Example 23 - " &
            "Set Font with ##<0x8nnnnnnn> construct");
        elsif page = 14 then
            Write_Text_Viewport("t", 1.5, 0.5, 0.5,
            "#<0x10>PLplot Example 23 - " &
            "Set Font with ##<0xmn> constructs");
        elsif page = 15 then
            Write_Text_Viewport("t", 1.5, 0.5, 0.5,
            "#<0x10>PLplot Example 23 - " &
            "Set Font with ##<FCI COMMAND STRING/> constructs");
        end if;
        Set_Character_Height(0.0, 0.75);
        for i in 0 .. FCI_COMBINATIONS - 1 loop
            family_index := i mod 5;
            style_index  := (i / 5) mod 3;
            weight_index := ((i / 5) / 3) mod 2;
            if page = 11 then
                Set_Font_Characterization_Integer(fci(i));
                a_string := To_Unbounded_String(
                    "Page 12, " &
                    family(family_index) & ", "  &
                    style (style_index)  & ", "  &
                    weight(weight_index) & ":  " &
                    "The quick brown fox jumps over the lazy dog");
            elsif page = 12 then
                Set_Font(family_index, style_index, weight_index);
                a_string := To_Unbounded_String(
                    "Page 13, " &
                    family(family_index) & ", "  &
                    style (style_index)  & ", "  &
                    weight(weight_index) & ":  " &
                    "The quick brown fox jumps over the lazy dog");
            elsif page = 13 then
                a_string := To_Unbounded_String(
                    "Page 14, " &
                    family(family_index) & ", "  &
                    style (style_index)  & ", "  &
                    weight(weight_index) & ":  " &
                    "#<0x" & Hex_Image_8(fci(i)) & ">" &
                    "The quick brown fox jumps over the lazy dog");
            elsif page = 14 then
                a_string := To_Unbounded_String(
                    "Page 15, " &
                    family(family_index) & ", "  &
                    style (style_index)  & ", "  &
                    weight(weight_index) & ":  " &
                    "#<0x"   & Trim(Integer'Image(family_index), Left) &
                    "0>#<0x" & Trim(Integer'Image(style_index),  Left) &
                    "1>#<0x" & Trim(Integer'Image(weight_index), Left) & "2>" &
                    "The quick brown fox jumps over the lazy dog");
            elsif page = 15 then
                a_string := To_Unbounded_String(
                    "Page 16, " &
                    family(family_index) & ", "  &
                    style (style_index)  & ", "  &
                    weight(weight_index) & ":  " &
                    "#<" & family(family_index) & "/>#<" & style (style_index) &
                    "/>#<" & weight(weight_index) & "/>" &
                    "The quick brown fox jumps over the lazy dog");
            end if;
            Write_Text_World (0.0, 1.0 - (Long_Float(i) + 0.5) * dy, 1.0, 0.0, 0.0, To_String(a_string));
        end loop; -- i

        Set_Character_Height(0.0, 1.0);
    end loop; -- page

    -- Restore defaults
    Set_Pen_Color(Red);

    End_PLplot;
end xstandard23a;
