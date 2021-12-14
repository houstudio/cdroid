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
    System,
    Interfaces.C.Pointers,
    Ada.Strings.Unbounded,
    Ada.Strings.Maps;
use
    PLplot_Thin,
    PLplot_Auxiliary,
    Ada.Strings.Unbounded;

package PLplot_Traditional is

    -- Used with plsdidev.
    PL_NOTSET : Long_Float renames PLplot_Thin.PL_NOTSET;

    -- "Rename" some useful types from the thin binding.
    subtype Boolean_Array_1D is PLplot_Thin.Boolean_Array_1D;
    subtype Integer_Array_1D is PLplot_Thin.Integer_Array_1D;

    -- "Rename" some types mainly used for contour plots and the like so that
    -- user programs can see them without with-ing PLplot_thin. It might be
    -- good to remove "use PLplot_Thin" for clarity since it is used in only
    -- a few places.
    subtype PL_Pointer                 is Plplot_Thin.PL_Pointer;
    -- The following deprecated spelling is not used anywhere in the Ada bindings
    -- or examples but is provided for backwards compatibility.
    subtype PLpointer                  is Plplot_Thin.PLpointer; -- DEPRECATED
    subtype Unsigned_Int               is Interfaces.C.unsigned;
    subtype Transformation_Data_Type   is Plplot_Thin.Transformation_Data_Type;
    subtype Transformation_Data_Type_2 is Plplot_Thin.Transformation_Data_Type_2;
    subtype Graphics_Input_Record_Type is Plplot_Thin.PLGraphicsIn;
    -- subtype Data_Ops_2D_Type           is Plplot_Thin.PLf2ops_t; -- Manipulate 2D data

    -- "Rename" the unicode type for characters.
    subtype Unicode is PLplot_Thin.PLUNICODE;

    -- Rename Plplot_Thin.plfill so that a pointer to it can be passed to
    -- procedures such as plshade and plshades and called with C
    -- conventions. Note that Plplot_Thin.plfill is already in C so that it does
    -- not have to be re-written like e.g. pltr1 was for plcont. The Ada
    -- procedure plfill in this package can't be used because it does not carry
    -- the array dimensions which will be needed by the time it is finally
    -- called deep within the plplot calling chain; this is why the version
    -- from this package (PLplot_Traditional) can't be used. Thus, this also
    -- overloads the name plfill. It might never occur to the Ada programmer
    -- that this is happening, which is good.
    -- Suppress these warnings compatibly with pre-4.3 GNAT:
    --    "foreign caller must pass bounds explicitly"
    --    "type of argument ""plfill.x"" is unconstrained array"
    --    "type of argument ""plfill.y"" is unconstrained array"
    pragma Warnings(Off);
    procedure plfill(length : Integer; x, y : Real_Vector) renames PLplot_Thin.plfill;
    pragma Warnings(On);

    -- Make Mask_Function_Pointer_Type available to the user so that s/he doesn't
    -- have to "with" PLplot_Thin. Note that it is also used herein.
    subtype Mask_Function_Pointer_Type is PLplot_Thin.Mask_Function_Pointer_Type;

    -- "Rename" callback for map functions plmap and plmeridians.
    subtype Map_Form_Function_Pointer_Type is Plplot_Thin.Map_Form_Function_Pointer_Type;

    -- "Rename" the necessarily constrained array for mapping longitudes and latitudes.
    subtype Map_Form_Constrained_Array is PLplot_Thin.Map_Form_Constrained_Array;

    -- This is a fixed-length string for use with custom label procedures
    -- in Custom_Label_Procedure_Pointer_Type, and plslabelfunc (Set_Custom_Label).
    -- This length, 0 .. 40, is hardwired in the PLplot C code; this type will
    -- fail if that length is ever changed.
    Max_Label_String_Length : Integer renames PLplot_Thin.Max_Label_String_Length;
    subtype Label_String_Type is PLplot_Thin.Label_String_Type;

    -- "Rename" callback for custom label functions.
    subtype Custom_Label_Procedure_Pointer_Type is
        PLplot_Thin.Custom_Label_Procedure_Pointer_Type;

    -- "Rename" callback for custom coordinate transform procedure.
    subtype Coordinate_Transform_Procedure_Pointer_Type is
        PLplot_Thin.Coordinate_Transform_Procedure_Pointer_Type;

--------------------------------------------------------------------------------
--        Types and constants for thick binding                               --
--------------------------------------------------------------------------------

    -- Default dummy arrays for various plotters which take multiple inputs.
    subtype Length_One_Real_Vector is Real_Vector(1..1);
    Dont_Plot_This : Length_One_Real_Vector := (1..1 => Long_Float'small);

    -- Default colors for Color Map 0.
    -- These are hard-wired to the current colors of color map 0; if that
    -- colors map has been changed, then these colors will return surprising
    -- results. Color map 0 can always be restored to its default state using
    -- Restore_Default_Snapshot_Of_Color_Map_0.
    subtype Plot_Color_Type is Natural; -- Remember that user can expand color map 0.
    Black      : constant Plot_Color_Type := 0;
    Red        : constant Plot_Color_Type := 1;
    Yellow     : constant Plot_Color_Type := 2;
    Green      : constant Plot_Color_Type := 3;
    Aquamarine : constant Plot_Color_Type := 4;
    Pink       : constant Plot_Color_Type := 5;
    Wheat      : constant Plot_Color_Type := 6;
    Grey       : constant Plot_Color_Type := 7;
    Brown      : constant Plot_Color_Type := 8;
    Blue       : constant Plot_Color_Type := 9;
    BlueViolet : constant Plot_Color_Type := 10;
    Cyan       : constant Plot_Color_Type := 11;
    Turquoise  : constant Plot_Color_Type := 12;
    Magenta    : constant Plot_Color_Type := 13;
    Salmon     : constant Plot_Color_Type := 14;
    White      : constant Plot_Color_Type := 15;

    Max_Lines_For_Multiplot : constant := 5;

    -- Pre-defined line styles, e.g., continuous, dashed, etc.
    subtype Line_Style_Type is Integer range 1..8;

    -- Arrays for various line appearances.
    type Color_Array_Type      is array (1..Max_Lines_For_Multiplot) of Plot_Color_Type;
    Default_Color_Array : Color_Array_Type := (Red, Blue, Green, Salmon, BlueViolet);

    type Line_Width_Array_Type is array (1..Max_Lines_For_Multiplot) of Long_Float;
    Default_Line_Width_Array : Line_Width_Array_Type := (1.0, 1.0, 1.0, 1.0, 1.0);

    type Line_Style_Array_Type is array (1..Max_Lines_For_Multiplot) of Line_Style_Type;
    Default_Line_Style_Array : Line_Style_Array_Type := (1, 1, 1, 1, 1);

    -- Things for label strings
    Default_Label_String : constant Unbounded_String := To_Unbounded_String("");
    type Label_String_Array_Type is array (1..Max_Lines_For_Multiplot) of Unbounded_String;
    Default_Label_String_Array : Label_String_Array_Type := (others => Default_Label_String);

    -- Things for stripcharts
    Maximum_Number_Of_Stripcharts : Integer := 4; -- Limited by PLplot designers.
    type Stripchart_Label_String_Array_Type is array (1 .. Maximum_Number_Of_Stripcharts) of Unbounded_String;

    -- Things for legends
    -- The user program must declare one of these with definite bounds, one
    -- element for each legend label.
    type Legend_String_Array_Type is array (Integer range <>) of Unbounded_String;

    --Flags used for position argument of both pllegend and plcolorbar
    -- (duplicated from plplot_thin.ads)
    PL_Position_Left     : constant Legend_Colorbar_Position_Type := 1;
    PL_Position_Right    : constant Legend_Colorbar_Position_Type := 2;
    PL_Position_Top      : constant Legend_Colorbar_Position_Type := 4;
    PL_Position_Bottom   : constant Legend_Colorbar_Position_Type := 8;
    PL_Position_Inside   : constant Legend_Colorbar_Position_Type := 16;
    PL_Position_Outside  : constant Legend_Colorbar_Position_Type := 32;
    PL_Position_Viewport : constant Legend_Colorbar_Position_Type := 64;
    PL_Position_Subpage  : constant Legend_Colorbar_Position_Type := 128;

    -- Renamed flags used for position argument of both pllegend and plcolorbar
    Legend_Position_Left     : constant Legend_Colorbar_Position_Type := PL_Position_Left;
    Legend_Position_Right    : constant Legend_Colorbar_Position_Type := PL_Position_Right;
    Legend_Position_Top      : constant Legend_Colorbar_Position_Type := PL_Position_Top;
    Legend_Position_Bottom   : constant Legend_Colorbar_Position_Type := PL_Position_Bottom;
    Legend_Position_Inside   : constant Legend_Colorbar_Position_Type := PL_Position_Inside;
    Legend_Position_Outside  : constant Legend_Colorbar_Position_Type := PL_Position_Outside;
    Legend_Position_Viewport : constant Legend_Colorbar_Position_Type := PL_Position_Viewport;
    Legend_Position_Subpage  : constant Legend_Colorbar_Position_Type := PL_Position_Subpage;

    -- Flags for pllegend (duplicated from plplot_thin.ads)
    PL_Legend_None         : constant Legend_Flag_Type := 1;
    PL_Legend_Color_Box    : constant Legend_Flag_Type := 2;
    PL_Legend_Line         : constant Legend_Flag_Type := 4;
    PL_Legend_Symbol       : constant Legend_Flag_Type := 8;
    PL_Legend_Text_Left    : constant Legend_Flag_Type := 16;
    PL_Legend_Background   : constant Legend_Flag_Type := 32;
    PL_Legend_Bounding_Box : constant Legend_Flag_Type := 64;
    PL_Legend_Row_Major    : constant Legend_Flag_Type := 128;

    -- Renamed flags for pllegend
    Legend_None         : constant Legend_Flag_Type := PL_Legend_None;
    Legend_Color_Box    : constant Legend_Flag_Type := PL_Legend_Color_Box;
    Legend_Line         : constant Legend_Flag_Type := PL_Legend_Line;
    Legend_Symbol       : constant Legend_Flag_Type := PL_Legend_Symbol;
    Legend_Text_Left    : constant Legend_Flag_Type := PL_Legend_Text_Left;
    Legend_Background   : constant Legend_Flag_Type := PL_Legend_Background;
    Legend_Bounding_Box : constant Legend_Flag_Type := PL_Legend_Bounding_Box;
    Legend_Row_Major    : constant Legend_Flag_Type := PL_Legend_Row_Major;

    -- Flags for plcolorbar (duplicated from plplot_thin.ads)
    PL_Colorbar_Label_Left    : constant Colorbar_Flag_Type := 1;
    PL_Colorbar_Label_Right   : constant Colorbar_Flag_Type := 2;
    PL_Colorbar_Label_Top     : constant Colorbar_Flag_Type := 4;
    PL_Colorbar_Label_Bottom  : constant Colorbar_Flag_Type := 8;
    PL_Colorbar_Image         : constant Colorbar_Flag_Type := 16;
    PL_Colorbar_Shade         : constant Colorbar_Flag_Type := 32;
    PL_Colorbar_Gradient      : constant Colorbar_Flag_Type := 64;
    PL_Colorbar_Cap_None      : constant Colorbar_Flag_Type := 128;
    PL_Colorbar_Cap_Low       : constant Colorbar_Flag_Type := 256;
    PL_Colorbar_Cap_High      : constant Colorbar_Flag_Type := 512;
    PL_Colorbar_Shade_Label   : constant Colorbar_Flag_Type := 1024;
    PL_Colorbar_Orient_Right  : constant Colorbar_Flag_Type := 2048;
    PL_Colorbar_Orient_Top    : constant Colorbar_Flag_Type := 4096;
    PL_Colorbar_Orient_Left   : constant Colorbar_Flag_Type := 8192;
    PL_Colorbar_Orient_Bottom : constant Colorbar_Flag_Type := 16384;
    PL_Colorbar_Background    : constant Colorbar_Flag_Type := 32768;
    PL_Colorbar_Bounding_Box  : constant Colorbar_Flag_Type := 65536;

    -- Renamed flags for plcolorbar
    Colorbar_Label_Left    : constant Colorbar_Flag_Type := PL_Colorbar_Label_Left;
    Colorbar_Label_Right   : constant Colorbar_Flag_Type := PL_Colorbar_Label_Right;
    Colorbar_Label_Top     : constant Colorbar_Flag_Type := PL_Colorbar_Label_Top;
    Colorbar_Label_Bottom  : constant Colorbar_Flag_Type := PL_Colorbar_Label_Bottom;
    Colorbar_Image         : constant Colorbar_Flag_Type := PL_Colorbar_Image;
    Colorbar_Shade         : constant Colorbar_Flag_Type := PL_Colorbar_Shade;
    Colorbar_Gradient      : constant Colorbar_Flag_Type := PL_Colorbar_Gradient;
    Colorbar_Cap_None      : constant Colorbar_Flag_Type := PL_Colorbar_Cap_None;
    Colorbar_Cap_Low       : constant Colorbar_Flag_Type := PL_Colorbar_Cap_Low;
    Colorbar_Cap_High      : constant Colorbar_Flag_Type := PL_Colorbar_Cap_High;
    Colorbar_Shade_Label   : constant Colorbar_Flag_Type := PL_Colorbar_Shade_Label;
    Colorbar_Orient_Right  : constant Colorbar_Flag_Type := PL_Colorbar_Orient_Right;
    Colorbar_Orient_Top    : constant Colorbar_Flag_Type := PL_Colorbar_Orient_Top;
    Colorbar_Orient_Left   : constant Colorbar_Flag_Type := PL_Colorbar_Orient_Left;
    Colorbar_Orient_Bottom : constant Colorbar_Flag_Type := PL_Colorbar_Orient_Bottom;
    Colorbar_Background    : constant Colorbar_Flag_Type := PL_Colorbar_Background;
    Colorbar_Bounding_Box  : constant Colorbar_Flag_Type := PL_Colorbar_Bounding_Box;

    -- Justification for plots
    subtype Justification_Type is Integer range -1..2;
    User_Justified       : constant Justification_Type := -1;
    Not_Justified        : constant Justification_Type := 0;
    Justified            : constant Justification_Type := 1;
    Justified_Square_Box : constant Justification_Type := 2;

    -- Axis styles
    subtype Axis_Style_Type is Integer range -2..73; -- Not all values are used. fix this in docs

    No_Box            : constant Axis_Style_Type := -2;
    Box               : constant Axis_Style_Type := -1;

    Linear_Box_Plus   : constant Axis_Style_Type := 0;
    Linear_Zero_Axes  : constant Axis_Style_Type := 1;
    Linear_Major_Grid : constant Axis_Style_Type := 2;
    Linear_Minor_Grid : constant Axis_Style_Type := 3;

    Log_X_Box_Plus    : constant Axis_Style_Type := 10;
    Log_X_Zero_Axes   : constant Axis_Style_Type := 11;
    Log_X_Major_Grid  : constant Axis_Style_Type := 12;
    Log_X_Minor_Grid  : constant Axis_Style_Type := 13;

    Log_Y_Box_Plus    : constant Axis_Style_Type := 20;
    Log_Y_Zero_Axes   : constant Axis_Style_Type := 21;
    Log_Y_Major_Grid  : constant Axis_Style_Type := 22;
    Log_Y_Minor_Grid  : constant Axis_Style_Type := 23;

    Log_XY_Box_Plus   : constant Axis_Style_Type := 30;
    Log_XY_Zero_Axes  : constant Axis_Style_Type := 31;
    Log_XY_Major_Grid : constant Axis_Style_Type := 32;
    Log_XY_Minor_Grid : constant Axis_Style_Type := 33;

    Date_Time_X_Linear_Box_Plus    : constant Axis_Style_Type := 40;
    Date_Time_X_Linear_Zero_Axes   : constant Axis_Style_Type := 41;
    Date_Time_X_Linear_Major_Grid  : constant Axis_Style_Type := 42;
    Date_Time_X_Linear_Minor_Grid  : constant Axis_Style_Type := 43;

    Date_Time_Y_Linear_Box_Plus    : constant Axis_Style_Type := 50;
    Date_Time_Y_Linear_Zero_Axes   : constant Axis_Style_Type := 51;
    Date_Time_Y_Linear_Major_Grid  : constant Axis_Style_Type := 52;
    Date_Time_Y_Linear_Minor_Grid  : constant Axis_Style_Type := 53;

    Date_Time_XY_Linear_Box_Plus   : constant Axis_Style_Type := 60;
    Date_Time_XY_Linear_Zero_Axes  : constant Axis_Style_Type := 61;
    Date_Time_XY_Linear_Major_Grid : constant Axis_Style_Type := 62;
    Date_Time_XY_Linear_Minor_Grid : constant Axis_Style_Type := 63;

    Custom_Labels_Linear_Box_Plus   : constant Axis_Style_Type := 70;
    Custom_Labels_Linear_Zero_Axes  : constant Axis_Style_Type := 71;
    Custom_Labels_Linear_Major_Grid : constant Axis_Style_Type := 72;
    Custom_Labels_Linear_Minor_Grid : constant Axis_Style_Type := 73;

    -- Integer constrained to 0..255
    subtype Integer_0_255_Type is Integer range 0 .. 255;
    type Integer_0_255_Array is array (Integer range <>) of Integer_0_255_Type;

    -- Long_Float constrained to 0.0 .. 1.0.
    subtype Long_Float_0_1_Type is Long_Float range 0.0 .. 1.0;

    -- Escape characters for text strings
    Escape_Character_Set : Ada.Strings.Maps.Character_Set;

    -- Generic font styles
    subtype Font_Style_Type is Integer range 1..4;
    Normal_Font : constant Font_Style_Type := 1;
    Roman_Font  : constant Font_Style_Type := 2;
    Italic_Font : constant Font_Style_Type := 3;
    Script_Font : constant Font_Style_Type := 4;
    -- Bold_Font?

    -- Character sets
    subtype Character_Set_Type is Integer range 0..1;
    Standard_Character_Set : constant Character_Set_Type := 0;
    Extended_Character_Set : constant Character_Set_Type := 1;

    -- Plot orientation
    type Orientation_Type is (Landscape, Portrait);

    -- Constant for several procedures which don't change a parameter if used
    Unchanged : constant Integer := 0;

    -- Pre-defined fill patterns
    subtype Fill_Pattern_Type is Integer range 0..8; -- Guessing; not documented

    -- Modes for parsing command line arguments.
    Parse_Partial    : constant Parse_Mode_Type := 0;   -- For backward compatibility
    Parse_Full       : constant Parse_Mode_Type := 1;   -- Process fully & exit if error
    Parse_Quiet      : constant Parse_Mode_Type := 2;   -- Don't issue messages
    Parse_No_Delete  : constant Parse_Mode_Type := 4;   -- Don't delete options after processing
    Parse_Show_All   : constant Parse_Mode_Type := 8;   -- Show invisible options
    Parse_Override   : constant Parse_Mode_Type := 16;  -- Obsolete
    Parse_No_Program : constant Parse_Mode_Type := 32;  -- Program name NOT in *argv[0]..
    Parse_No_Dash    : constant Parse_Mode_Type := 64;  -- Set if leading dash NOT required
    Parse_Skip       : constant Parse_Mode_Type := 128; -- Skip over unrecognized args

    -- FCI (font characterization integer) related constants.
    FCI_Mark                : constant Integer := 16#10000000#;
    FCI_Impossible          : constant Integer := 16#00000000#;
    FCI_Hexdigit_Mask       : constant Integer := 16#f#;
    FCI_Hexpower_Mask       : constant Integer := 16#7#;
    FCI_Hexpower_Impossible : constant Integer := 16#f#;
    -- These define hexpower values corresponding to each font attribute.
    FCI_Family : constant Integer := 16#0#;
    FCI_Style  : constant Integer := 16#1#;
    FCI_Weight : constant Integer := 16#2#;
    -- These are legal values for font family attribute
    FCI_Sans   : constant Integer := 16#0#;
    FCI_Serif  : constant Integer := 16#1#;
    FCI_Mono   : constant Integer := 16#2#;
    FCI_Script : constant Integer := 16#3#;
    FCI_Symbol : constant Integer := 16#4#;
    -- These are legal values for font style attribute
    FCI_Upright : constant Integer := 16#0#;
    FCI_Italic  : constant Integer := 16#1#;
    FCI_Oblique : constant Integer := 16#2#;
    -- These are legal values for font weight attribute
    FCI_Medium : constant Integer := 16#0#;
    FCI_Bold   : constant Integer := 16#1#;

    -- Descriptions of map outlines for continents, countries, and US states.
    type Map_Type is (Continents, USA_and_States, Continents_and_Countries, USA_States_and_Continents);

    -- definitions for the opt argument in plot3dc() and plsurf3d()
    -- DRAW_LINEX *must* be 1 and DRAW_LINEY *must* be 2, because of legacy code!
    -- fix this Why is there no type declared for these?
    No_3D_Options             : constant Integer := 0;   -- None of the options
    Lines_Parallel_To_X       : constant Integer := 1;   -- draw lines parallel to the X axis
    Lines_Parallel_To_Y       : constant Integer := 2;   -- draw lines parallel to the Y axis
    Lines_Parallel_To_X_And_Y : constant Integer := 3;   -- draw lines parallel to both the X and Y axis
    Magnitude_Color           : constant Integer := 4;   -- draw the mesh with a color dependent of the magnitude
    Base_Contour              : constant Integer := 8;   -- draw contour plot at bottom xy plane
    Top_Contour               : constant Integer := 16;  -- draw contour plot at top xy plane
    Surface_Contour           : constant Integer := 32;  -- draw contour plot at surface
    Sides                     : constant Integer := 64;  -- draw sides
    Facets                    : constant Integer := 128; -- draw outline for each square that makes up the surface
    Meshed                    : constant Integer := 256; -- draw mesh

    subtype Gridding_Algorithm_Type is Integer range 1..6;

    -- Type of gridding algorithm for plgriddata()
    -- "Long form" gridding algorithm names
    Grid_Bivariate_Cubic_Spline_Approximation               : constant Gridding_Algorithm_Type := 1; -- GRID_CSA
    Grid_Delaunay_Triangulation_Linear_Interpolation        : constant Gridding_Algorithm_Type := 2; -- GRID_DTLI
    Grid_Natural_Neighbors_Interpolation                    : constant Gridding_Algorithm_Type := 3; -- GRID_NNI
    Grid_Nearest_Neighbors_Inverse_Distance_Weighted        : constant Gridding_Algorithm_Type := 4; -- GRID_NNIDW
    Grid_Nearest_Neighbors_Linear_Interpolation             : constant Gridding_Algorithm_Type := 5; -- GRID_NNLI
    Grid_Nearest_Neighbors_Around_Inverse_Distance_Weighted : constant Gridding_Algorithm_Type := 6; -- GRID_NNAIDW

    -- Axis label tags
    x_axis : constant Integer := 1; -- The x-axis
    y_axis : constant Integer := 2; -- The y-axis
    z_axis : constant Integer := 3; -- The z-axis

--------------------------------------------------------------------------------
-- Constants copied from PLplot_Thin.ads so that it doesn't have to be seen.  --
-- These are replicated herein with other names. Either name may be used,     --
-- e.g., Parse_Full is the same as PL_PARSE_FULL.                             --
--------------------------------------------------------------------------------

    -- Modes for parsing command line arguments (redux).
    PL_PARSE_PARTIAL   : constant Parse_Mode_Type := 16#0000#; -- For backward compatibility
    PL_PARSE_FULL      : constant Parse_Mode_Type := 16#0001#; -- Process fully & exit if error
    PL_PARSE_QUIET     : constant Parse_Mode_Type := 16#0002#; -- Don't issue messages
    PL_PARSE_NODELETE  : constant Parse_Mode_Type := 16#0004#; -- Don't delete options after processing
    PL_PARSE_SHOWALL   : constant Parse_Mode_Type := 16#0008#; -- Show invisible options
    PL_PARSE_OVERRIDE  : constant Parse_Mode_Type := 16#0010#; -- Obsolete
    PL_PARSE_NOPROGRAM : constant Parse_Mode_Type := 16#0020#; -- Program name NOT in *argv[0]..
    PL_PARSE_NODASH    : constant Parse_Mode_Type := 16#0040#; -- Set if leading dash NOT required
    PL_PARSE_SKIP      : constant Parse_Mode_Type := 16#0080#; -- Skip over unrecognized args

    -- FCI (font characterization integer) related constants.
    PL_FCI_MARK                : constant Integer := 16#10000000#;
    PL_FCI_IMPOSSIBLE          : constant Integer := 16#00000000#;
    PL_FCI_HEXDIGIT_MASK       : constant Integer := 16#f#;
    PL_FCI_HEXPOWER_MASK       : constant Integer := 16#7#;
    PL_FCI_HEXPOWER_IMPOSSIBLE : constant Integer := 16#f#;
    -- These define hexpower values corresponding to each font attribute.
    PL_FCI_FAMILY : constant Integer := 16#0#;
    PL_FCI_STYLE  : constant Integer := 16#1#;
    PL_FCI_WEIGHT : constant Integer := 16#2#;
    -- These are legal values for font family attribute
    PL_FCI_SANS   : constant Integer := 16#0#;
    PL_FCI_SERIF  : constant Integer := 16#1#;
    PL_FCI_MONO   : constant Integer := 16#2#;
    PL_FCI_SCRIPT : constant Integer := 16#3#;
    PL_FCI_SYMBOL : constant Integer := 16#4#;
    -- These are legal values for font style attribute
    PL_FCI_UPRIGHT : constant Integer := 16#0#;
    PL_FCI_ITALIC  : constant Integer := 16#1#;
    PL_FCI_OBLIQUE : constant Integer := 16#2#;
    -- These are legal values for font weight attribute
    PL_FCI_MEDIUM : constant Integer := 16#0#;
    PL_FCI_BOLD   : constant Integer := 16#1#;

    -- Definitions for the opt argument in plot3dc() and plsurf3d()
    DRAW_LINEX  : constant Integer := 1;   -- draw lines parallel to the X axis
    DRAW_LINEY  : constant Integer := 2;   -- draw lines parallel to the Y axis
    DRAW_LINEXY : constant Integer := 3;   -- draw lines parallel to both the X and Y axis
    MAG_COLOR   : constant Integer := 4;   -- draw the mesh with a color dependent of the magnitude
    BASE_CONT   : constant Integer := 8;   -- draw contour plot at bottom xy plane
    TOP_CONT    : constant Integer := 16;  -- draw contour plot at top xy plane
    SURF_CONT   : constant Integer := 32;  -- draw contour plot at surface
    DRAW_SIDES  : constant Integer := 64;  -- draw sides
    FACETED     : constant Integer := 128; -- draw outline for each square that makes up the surface
    MESH        : constant Integer := 256; -- draw mesh

    -- Type of gridding algorithm for plgriddata()
    GRID_CSA    : constant Gridding_Algorithm_Type := 1;
    GRID_DTLI   : constant Gridding_Algorithm_Type := 2;
    GRID_NNI    : constant Gridding_Algorithm_Type := 3;
    GRID_NNIDW  : constant Gridding_Algorithm_Type := 4;
    GRID_NNLI   : constant Gridding_Algorithm_Type := 5;
    GRID_NNAIDW : constant Gridding_Algorithm_Type := 6;

    -- Axis label tags
    -- "PLplot style" names
    -- This version for custom labels in plslabelfunc. Compare with e.g. "x_axis" above.
    PL_X_AXIS : constant Integer := 1; -- The x-axis
    PL_Y_AXIS : constant Integer := 2; -- The y-axis
    PL_Z_AXIS : constant Integer := 3; -- The z-axis
    -- "Ada style" names
    Label_X_Axis : constant Integer := 1; -- The x-axis
    Label_Y_Axis : constant Integer := 2; -- The y-axis
    Label_Z_Axis : constant Integer := 3; -- The z-axis

--------------------------------------------------------------------------------


    -- fix this Why is there no style here?
    -- Flags for the opt argument in plhist                                   --
    Histogram_Default         : constant Integer := 0;
    Histogram_Noscaling       : constant Integer := 1;
    Histogram_Ignore_Outliers : constant Integer := 2;
    Histogram_Noexpand        : constant Integer := 8;
    Histogram_Noempty         : constant Integer := 16;


    -- Flags for the opt argument in plbin                                    --
    Bin_Default   : constant Integer := 0;
    Bin_Centered  : constant Integer := 1;
    Bin_Noexpand  : constant Integer := 2;
    Bin_Noempty   : constant Integer := 4;


--------------------------------------------------------------------------------
-- A convenient string function                                               --
--------------------------------------------------------------------------------

    -- Short name for To_Unbounded_String
    function TUB(arg : String) return Ada.Strings.Unbounded.Unbounded_String renames Ada.Strings.Unbounded.To_Unbounded_String;


--------------------------------------------------------------------------------
--        High-Level subroutines for thick binding                            --
--------------------------------------------------------------------------------

    -- When asked to draw white lines on black background, do it.
    -- This is the default.
    procedure Draw_On_Black;


    -- When asked to draw black lines on white background, reverse black and white.
    -- This might look better on anti-aliased displays.
    procedure Draw_On_White;


    -- Plotter for up to five x-y pairs and settable axis style, plot
    -- line colors, widths, and styles, justification, zoom, and labels.
    -- Can be used directly or as part of a "simple" plotter
    -- such as those that follow or are made by the user.
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
        y_Max_Zoom    : Long_Float              := Long_Float'large);


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
        Title_Label : String := To_String(Default_Label_String));


    -- fix this Add x, y, title labels; make default " " or see predefined default.
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
        Title_Label : String := To_String(Default_Label_String));


    -- Simple log x plotter for single x array and multiple y arrays
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
        Log_Base : Long_Float := 10.0); -- Should this default to e?


    -- Simple log y plotter for multiple x arrays and single y array
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
        Log_Base : Long_Float := 10.0); -- Should this default to e?


    -- Simple log x - log y plotter
    procedure Simple_Plot_Log_XY
       (x, y        : Real_Vector;
        X_Label     : String := To_String(Default_Label_String);
        Y_Label     : String := To_String(Default_Label_String);
        Title_Label : String := To_String(Default_Label_String);
        x_Log_Base, y_Log_Base : Long_Float := 10.0); -- Should this default to e?


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
        Title_Label : String := To_String(Default_Label_String));


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
        y_Max_Zoom    : Long_Float         := Long_Float'large);


--------- Simple Contour Plotter ------

    procedure Simple_Contour
       (z             : Real_Matrix;
        Number_Levels : Integer := 10;
        X_Label       : String  := To_String(Default_Label_String);
        Y_Label       : String  := To_String(Default_Label_String);
        Title_Label   : String  := To_String(Default_Label_String));


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
        Z_Label  : String := "z");


--------- Simple 3D Surface Plotter ------

    procedure Simple_Surface_3D
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
        Z_Label  : String := "z");


--------- Simple color table manipulatons -----

    -- Things for manipulating color map 0 --

    -- Current default number of colors provided by PLplot. There is no way to
    -- get this number under program control. The actual number can be set by
    -- the user with Set_Number_Of_Colors_Map_0.
    Number_Of_Default_Colors : constant Integer := 16;

    -- The default color map 0 is captured at initialization of PLplot.adb with
    -- a call to Make_Snapshot_Of_Color_Map_0 stored here.
    Default_Red_Components   : Integer_Array_1D(0 .. Number_Of_Default_Colors - 1);
    Default_Green_Components : Integer_Array_1D(0 .. Number_Of_Default_Colors - 1);
    Default_Blue_Components  : Integer_Array_1D(0 .. Number_Of_Default_Colors - 1);


    -- Make a snapshot of color map 0 for possible later full or partial restoration.
    -- This is automatically called at package initialization with results stored
    -- in Default_Red_Components, Default_Green_Components, Default_Blue_Components.
    procedure Make_Snapshot_Of_Color_Map_0
       (Reds, Greens, Blues : out Integer_Array_1D);


       -- Restore an arbitray snapshot of color map 0.
    procedure Restore_Snapshot_Of_Color_Map_0
       (Reds, Greens, Blues : Integer_Array_1D);


    -- Restore the default colors of color map 0 taken as a snapshot at initialization.
    procedure Restore_Default_Snapshot_Of_Color_Map_0;


    -- Functions which correspond to the default colors of color map 0. Calling
    -- one of these (1) resets the corresponding slot in color map 0 to its
    -- default value, and (2) returns the correct integer value for the default
    -- color specified. Thus, using plcol0(Reset_Red) instead of
    -- plcol0(Red) guarantees that the color will be set to Red even if
    -- there have been prior manipulations of color 1.

    function Reset_Black return Integer;

    function Reset_Red return Integer;

    function Reset_Yellow return Integer;

    function Reset_Green return Integer;

    function Reset_Aquamarine return Integer;

    function Reset_Pink return Integer;

    function Reset_Wheat return Integer;

    function Reset_Grey return Integer;

    function Reset_Brown return Integer;

    function Reset_Blue return Integer;

    function Reset_BlueViolet return Integer;

    function Reset_Cyan return Integer;

    function Reset_Turquoise return Integer;

    function Reset_Magenta return Integer;

    function Reset_Salmon return Integer;

    function Reset_White return Integer;


    -- Things for manipulating color map 1 --

    type Color_Themes_For_Map_1_Type is (Gray, Blue_Green_Red, Red_Green_Blue,
        Red_Cyan_Blue, Blue_Black_Red, Red_Blue_Green, Red_Yellow);
    type Alt_Hue_Path_Type is (Alt_Hue_Path_None, Alt_Hue_Path_All);


    -- Quick application of pre-fabricated color schemes to color map 1.
    procedure Quick_Set_Color_Map_1(Color_Theme : Color_Themes_For_Map_1_Type);


--------------------------------------------------------------------------------
--        Auxiliary things                                                    --
--------------------------------------------------------------------------------

    -- This is a mask function for Shade_Regions (aka plshades) et al that always
    -- returns 1 so that all points are plotted. Can be used as a template
    -- for other user-written mask functions. This should be the same as
    -- passing null for the second argument in Shade_Regions.
    function Mask_Function_No_Mask(x, y : Long_Float) return Integer;
    pragma Convention(Convention => C, Entity => Mask_Function_No_Mask);


    -- Given an array to hold contour levels and function minimum and maximum,
    -- fill it and return. Useful for contour and shade plots.
    procedure Calculate_Contour_Levels
       (Contour_Levels : in out Real_Vector;
        z_Min, z_Max : Long_Float);


--------------------------------------------------------------------------------
--        Re-define PLplot procedures using Ada style.                        --
--------------------------------------------------------------------------------

-- These correspond to the section in plot.h called "Function Prototypes".

    -- set the format of the contour labels
    procedure pl_setcontlabelformat
       (Limit_Exponent     : Integer := 4;
        Significant_Digits : Integer := 2);


    -- set offset and spacing of contour labels
    procedure pl_setcontlabelparam
       (Label_Offset       : Long_Float := 0.006; -- Units are ???
        Label_Font_Height  : Long_Float := 0.3;   -- Units are ???
        Label_Spacing      : Long_Float := 0.1;  -- Units are???
        Labels_Active      : Boolean := False);


    Next_Subpage : constant Integer := 0;

    -- Advance to subpage "page", or to the next one if "page" = 0.
    procedure pladv(Page : Natural);


    -- Plot an arc.
    procedure plarc
       (x, y, a, b, angle1, angle2, rotate : Long_Float;
        fill : Boolean);


    -- Draw a 2D vector plot.
    procedure plvect
       (u, v                             : Real_Matrix;
        Scale                            : Long_Float;
        Transformation_Procedure_Pointer : Transformation_Procedure_Pointer_Type;
        Transformation_Data_Pointer      : PL_Pointer);


    -- Set the style for the arrow used by plvect to plot vectors.
    procedure plsvect
       (X_Vertices, Y_Vertices : Real_Vector;
        Fill_Arrow             : Boolean);


    -- Set the default style for the arrow used by plvect to plot vectors.
    procedure plsvect
       (X_Vertices, Y_Vertices : PL_Pointer;
        Fill_Arrow : Boolean);


    -- Simple method to set the default style for the arrow used by plvect to plot vectors.
    -- This is not part of the C API and is Ada-specific.
    procedure plsvect;


    -- Another simple method to set the default style for the arrow used by plvect to plot vectors.
    -- This is not part of the C API and is Ada-specific.
    procedure Reset_Vector_Arrow_Style;


    -- This functions similarly to plbox() except that the origin of the axes
    -- is placed at the user-specified point (x0, y0).
    procedure plaxes
       (X_Origin, Y_Origin       : Long_Float;
        X_Option_String          : String;
        X_Major_Tick_Interval    : Long_Float;
        X_Number_Of_Subintervals : Natural;
        Y_Option_String          : String;
        Y_Major_Tick_Interval    : Long_Float;
        Y_Number_Of_Subintervals : Natural);


    -- Plot a histogram using x to store data values and y to store frequencies
    procedure plbin
       (Bin_Values     : Real_Vector; -- "x"
        Bin_Counts     : Real_Vector; -- "y"
        Options        : Integer); -- Options are not defined in plplot.h.


    -- Calculate broken-down time from continuous time for current stream.
    procedure plbtime
       (year, month, day, hour, min : out Integer;
        sec                         : out Long_Float;
        ctime                       : Long_Float);


    -- Start new page. Should only be used with pleop().
    procedure plbop;


    -- This draws a box around the current viewport.
    procedure plbox
       (X_Option_String          : String;
        X_Major_Tick_Interval    : Long_Float;
        X_Number_Of_Subintervals : Natural := 0;
        Y_Option_String          : String;
        Y_Major_Tick_Interval    : Long_Float;
        Y_Number_Of_Subintervals : Natural := 0);

    Auto_Subintervals : Integer := 0; -- Intervals for minor ticks is automatic.


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
        Z_Number_Of_Subintervals : Natural := 0);


    -- Calculate world coordinates and subpage from relative device coordinates.
    procedure plcalc_world
       (x_Relative, y_Relative : Long_Float_0_1_Type;
        x_World,    y_World    : out Long_Float;
        Last_Window_Index      : out Integer);


    -- Clear current subpage.
    procedure plclear;


    -- Set color, map 0. Argument is integer between 0 and 15.
    procedure plcol0(A_Color : Plot_Color_Type);


    -- Set color, map 1. Argument is a float between 0. and 1.
    procedure plcol1(Color : Long_Float_0_1_Type);


    -- Configure transformation between continuous and broken-down time (and
    -- vice versa) for current stream.
    procedure plconfigtime
       (skale, offset1, offset2      : Long_Float;
        ccontrol                     : Integer;
        ifbtime_offset               : Boolean;
        year, month, day, hour, min : Integer;
        sec                          : Long_Float);


    -- Draws a contour plot from data in f(nx,ny). Is just a front-end to plfcont,
    -- with a particular choice for f2eval and f2eval_data.

    procedure plcont
       (z                                : Real_Matrix;
        x_Min_Index, x_Max_Index         : Integer;
        y_Min_Index, y_Max_Index         : Integer;
        Contour_Levels                   : Real_Vector;
        Transformation_Procedure_Pointer : Transformation_Procedure_Pointer_Type;
        Transformation_Data_Pointer      : PL_Pointer);


    -- The procedure plfcont is not documented and is not part of the API.
    -- However, it is a very useful capability to have available.
    -- I have tried to implement it as I think was intended but this may be incorrect.
    -- It appears as though the intent is to pass the arbitrarily organized
    -- data (pointed to by Irregular_Data_Pointer) as a (single) pointer to a
    -- 2D C-style array. Thus, for examaple, it is not possible to pass the data
    -- as triples.

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
        Transformation_Data              : Transformation_Data_Type);


    -- Copies state parameters from the reference stream to the current stream.
    procedure plcpstrm
       (Stream_ID                      : Integer;
        Do_Not_Copy_Device_Coordinates : Boolean);


    -- Calculate continuous time from broken-down time for current stream.
    procedure plctime
       (year, month, day, hour, min : Integer;
        sec                         : Long_Float;
        ctime                       : out Long_Float);


    -- Converts input values from relative device coordinates to relative plot
    -- coordinates.
    procedure pldid2pc;


    -- Converts input values from relative plot coordinates to relative
    -- device coordinates.
    procedure pldip2dc;


    -- End a plotting session for all open streams.
    procedure plend;


    -- End a plotting session for the current stream only.
    procedure plend1;


    -- Simple interface for defining viewport and window.
    procedure plenv
       (x_Min, x_Max, y_Min, y_Max : Long_Float;
        Justification              : Justification_Type;
        Axis_Style                 : Axis_Style_Type);


    -- similar to plenv() above, but in multiplot mode does not advance the subpage,
    -- instead the current subpage is cleared
    procedure plenv0
       (x_Min, x_Max, y_Min, y_Max : Long_Float;
        Justification              : Justification_Type;
        Axis_Style                 : Axis_Style_Type);


    -- End current page. Should only be used with plbop().
    procedure pleop;


    -- Plot horizontal error bars (xmin(i),y(i)) to (xmax(i),y(i))
    procedure plerrx(x_Min, x_Max, y : Real_Vector);


    -- Plot vertical error bars (x,ymin(i)) to (x(i),ymax(i))
    procedure plerry(x, y_Min, y_Max : Real_Vector);


    -- Advance to the next family file on the next new page
    procedure plfamadv;


    -- Other "fill" routines similar to plfill could be written in Ada if
    -- desired, but if they are intended to be used as callbacks in subprograms
    -- such as plshade and plshades, they should be called with C
    -- calling conventions.

    -- Pattern fills the polygon bounded by the input points.
    procedure plfill(x, y : Real_Vector);


    -- Pattern fills the 3d polygon bounded by the input points.
    procedure plfill3 (x, y, z : Real_Vector);


    -- Flushes the output stream. Use sparingly, if at all.
    procedure plflush;


    -- Sets the global font flag to 'ifont'.
    procedure plfont(Font_Style : Font_Style_Type);


    -- Load specified font set.
    procedure plfontld(Character_Set : Character_Set_Type);


    -- Get character default height and current (scaled) height
    procedure plgchr(Default_Height, Current_Height : out Long_Float);


    -- Returns 8 bit RGB values for given color from color map 0
    procedure plgcol0
       (Color_Index      : Integer;
        Red_Component, Green_Component, Blue_Component : out Integer);


    -- Returns 8 bit RGB values for given color from color map 0 and alpha value
    procedure plgcol0a
       (Color_Index                                    : Integer;
        Red_Component, Green_Component, Blue_Component : out Integer;
        Alpha                                          : out Long_Float_0_1_Type);


    -- Returns the background color by 8 bit RGB value
    procedure plgcolbg
       (Red_Component, Green_Component, Blue_Component : out Integer);


    -- Returns the background color by 8 bit RGB value and alpha value
    procedure plgcolbga
       (Red_Component, Green_Component, Blue_Component : out Integer;
        Alpha                                          : out Long_Float_0_1_Type);


    -- Returns the current compression setting
    procedure plgcompression(Compression_Level : out Integer);


    -- Make a function version of plgdev so that the caller can use it whereever
    -- a String type is expected without fooling around with conversions between
    -- Ada string types. See Example 14 for useage.
    -- This _replaces_ the procedure version.
    -- THIS IS NOT IN THE C API.

    -- Get the current device (keyword) name
    function plgdev return String;


    -- Retrieve current window into device space
    procedure plgdidev
       (Relative_Margin_Width : out Long_Float;
        Aspect_Ratio          : out Long_Float;
        x_Justification       : out Long_Float;
        y_Justification       : out Long_Float);


    -- Get plot orientation
    procedure plgdiori(Rotation : out Long_Float);


    -- Retrieve current window into plot space
    procedure plgdiplt
       (x_Min : out Long_Float;
        y_Min : out Long_Float;
        x_Max : out Long_Float;
        y_Max : out Long_Float);


    -- Get FCI (font characterization integer)
    procedure plgfci(Font_Characterization_Integer : out Unicode);


    -- Get family, style and weight of the current font
    procedure plgfont(Family, Style, Weight : out Integer);


    -- Get family file parameters
    procedure plgfam
       (Family_Enabled : out Boolean;
        Family_File_Number : out Integer;
        Maximum_File_Size : out Integer);


    -- Make a function version of plgfnam so that the caller can use it whereever
    -- a String type is expected without fooling around with conversions between
    -- Ada string types. See Example 14 for useage.
    -- This _replaces_ the procedure version.
    -- THIS IS NOT IN THE C API.

    -- Get the (current) output file name.
    function plgfnam return String;

    -- Get the (current) run level.
    procedure plglevel(Run_Level : out Integer);


    -- Get output device parameters.
    procedure plgpage
       (x_Pixels, y_Pixels           : out Long_Float;
        x_Page_Length, y_Page_Length : out Integer;
        x_Page_Offset, y_Page_Offset : out Integer);


    -- Switches to graphics screen.
    procedure plgra;


    -- Draw gradient in polygon.
    procedure plgradient(x, y : Real_Vector; Angle : Long_Float);


    -- Grid irregularly sampled data.
    procedure plgriddata
       (x, y, z                : Real_Vector; -- ungridded x- and y-points; z is height
        x_Grid, y_Grid         : Real_Vector;
        z_Gridded              : in out Real_Matrix;
        Gridding_Algorithm     : Gridding_Algorithm_Type;
        Griding_Algorithm_Data : Long_Float);


    -- Get subpage boundaries in absolute coordinates
    -- Results are millimeters from the lower left corner.
    procedure plgspa
       (Left_Edge, Right_Edge, Bottom_Edge, Top_Edge : out Long_Float);


    -- Get current stream number.
    procedure plgstrm(Stream_Number : out Integer);


    -- Make a function version of plgver so that the caller can use it whereever
    -- a String type is expected without fooling around with conversions between
    -- Ada string types. See Example 14 for useage.
    -- This _replaces_ the procedure version.
    -- THIS IS NOT IN THE C API.

    -- Get the current library version number
    function plgver return String;

    -- Get viewport boundaries in normalized device coordinates
    procedure plgvpd(x_Min, x_Max, y_Min, y_Max : out Long_Float);


    -- Get viewport boundaries in world coordinates
    procedure plgvpw(x_Min, x_Max, y_Min, y_Max : out Long_Float);


    -- Get x axis labeling parameters
    procedure plgxax(Max_Digits, Actual_Digits : out Integer);


    -- Get y axis labeling parameters
    procedure plgyax(Max_Digits, Actual_Digits : out Integer);


    -- Get z axis labeling parameters
    procedure plgzax(Max_Digits, Actual_Digits : out Integer);


    -- Draws a histogram of n values of a variable in array data[0..n-1]
    procedure plhist
       (Data           : Real_Vector;
        Data_Min       : Long_Float; -- left  edge of left-most bin
        Data_Max       : Long_Float; -- right edge of right-most bin
        Number_Of_Bins : Positive; -- equal-sized, between Data_Min and Data_Max
        Options : Integer); -- Options are not defined in plplot.h.


    -- Functions for converting between HLS and RGB color space
    procedure plhlsrgb
       (Hue_Component        : Long_Float;
        Lightness_Component  : Long_Float_0_1_Type;
        Saturation_Component : Long_Float_0_1_Type;
        Red_Component        : out Long_Float_0_1_Type;
        Green_Component      : out Long_Float_0_1_Type;
        Blue_Component       : out Long_Float_0_1_Type);


    -- Initialization. Must be called before starting plot.
    -- Certain other procedures, if used, must be called first.
    -- Initializes PLplot, using preset or default options
    procedure plinit;


    -- Draws a line segment from (x1, y1) to (x2, y2).
    procedure pljoin(x1, y1, x2, y2 : Long_Float);


    -- Simple routine for labelling graphs.
    procedure pllab(X_Label, Y_Label, Title_Label : String := To_String(Default_Label_String));


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
        Symbols                               : Legend_String_Array_Type);


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
        Values                               : Real_Matrix);


    -- Sets position of the light source
    procedure pllightsource
       (x : Long_Float := 1.0;
        y: Long_Float := 1.0;
        z: Long_Float := 1.0);


    -- Draws line segments connecting a series of points.
    procedure plline(x, y : Real_Vector);


    -- Draws a line in 3 space.
    procedure plline3(x, y, z : Real_Vector);


    -- Set line style.
    procedure pllsty(Line_Style : Line_Style_Type);


    -- fix this See comment in Example 19, x19a.adb or xthick19a.adb for how to
    -- possibly eliminate the need to pass array size as the first argument in
    -- the function pointed to by Map_Form_Function_Pointer. Ditto for plmeridians.

    -- plot continental outline in world coordinates
    procedure plmap
       (Map_Form_Function_Pointer            : Map_Form_Function_Pointer_Type;
        Map_Kind                             : Map_Type;
        Minimum_Longitude, Maximum_Longitude : Long_Float;
        Minimum_Latitude,  Maximum_Latitude  : Long_Float);


    -- Plot map fills.
    procedure plmapfill
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries               : Integer_Array_1D);


    -- Plot map fills: overload passes null pointer plotentries to emulate C, match documentaton.
    procedure plmapfill
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries               : PL_Pointer);


    -- Plot map fills: overload that doesn't use plotentries to emulate C, match documentaton.
    procedure Plot_Shapefile_All
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float);


    -- Plot map outlines.
    procedure plmapline
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries               : Integer_Array_1D);


    -- Plot map outlines: overload passes null pointer plotentries to emulate C, match documentaton.
    procedure plmapline
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries               : PL_Pointer);


    -- Plot map outlines: overload that doesn't use plotentries to emulate C, match documentaton.
    procedure Plot_Shapefile_World_All
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float);


    -- Plot map points.
    procedure plmapstring
       (Map_Form_Function_Pointer : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Plot_Entries : Integer_Array_1D);


    -- Plot map text.
    procedure plmaptex
       (Map_Form_Function_Pointer  : Map_Form_Function_Pointer_Type;
        Shapefile_File_Name        : String;
        dx, dy                     : Long_Float;
        Justification              : Long_Float;
        Text                       : String;
        Min_X, Max_X, Min_Y, Max_Y : Long_Float;
        Which_Shapefile_String     : Integer);


    -- fix this See comment for plmap.

    -- Plot the latitudes and longitudes on the background.
    procedure plmeridians
       (Map_Form_Function_Pointer            : Map_Form_Function_Pointer_Type;
        Delta_Longitude, Delta_Latitude      : Long_Float;
        Minimum_Longitude, Maximum_Longitude : Long_Float;
        Minimum_Latitude,  Maximum_Latitude  : Long_Float);


    -- Plots a mesh representation of the function z(x, y).
    procedure plmesh
       (x, y    : Real_Vector; -- surface definition points
        z       : Real_Matrix; -- height of surface at definition points
        Options : Integer);

    -- Plots a mesh representation of the function z(x, y) with contour
    procedure plmeshc
       (x, y           : Real_Vector; -- surface definition points
        z              : in Real_Matrix; -- height of surface at definition points
        Options        : Integer;
        Contour_Levels : Real_Vector); -- levels at which to draw contours


    -- Creates a new stream and makes it the default.
    procedure plmkstrm(New_Stream_Number : out Integer);


    -- Prints out "The_Text" at specified position relative to viewport
    procedure plmtex
       (Side                : String;
        Position_From_Edge  : Long_Float;
        Position_Along_Edge : Long_Float;
        Justification       : Long_Float;
        The_Text            : String);


    -- Prints out "The_Text" at specified position relative to viewport (3D)
    procedure plmtex3
       (Side                : String;
        Position_From_Edge  : Long_Float;
        Position_Along_Edge : Long_Float;
        Justification       : Long_Float;
        The_Text            : String);


    -- Plots a 3-d representation of the function z(x, y).
    procedure plot3d
       (x, y    : Real_Vector; -- surface definition points
        z       : Real_Matrix; -- height of surface at definition points
        Options : Integer;
        Sides : Boolean); -- draw sides?


    -- Plots a 3-d representation of the function z(x, y) with contour.
    procedure plot3dc
       (x, y           : Real_Vector; -- surface definition points
        z              : Real_Matrix; -- height of surface at definition points
        Options        : Integer;
        Contour_Levels : Real_Vector); -- levels at which to draw contours


    -- Plots a 3-d representation of the function z(x, y) with contour and
    -- y index limits.
    procedure plot3dcl -- Lacks documentation in Chapter 17 of Ref. Man.
       (x, y           : Real_Vector; -- surface definition points
        z              : Real_Matrix; -- height of surface at definition points
        Options        : Integer;
        Contour_Levels : Real_Vector;
        ixstart, ixn   : Integer;
        indexymin, indexymax : Integer_Array_1D); -- levels at which to draw contours


       --  valid options for plot3dc():
       --
       --  DRAW_SIDES, BASE_CONT, TOP_CONT (not yet),
       --  MAG_COLOR, DRAW_LINEX, DRAW_LINEY, DRAW_LINEXY.
       --
       --  valid options for plsurf3d():
       --
       --  MAG_COLOR, BASE_CONT, SURF_CONT, FACETED, DRAW_SIDES.


    -- Set fill pattern directly.
    procedure plpat
       (Inclinations : Integer_Array_1D;
        Spacings     : Integer_Array_1D);



    -- Draw a line connecting two points, accounting for coordinate transform
    procedure plpath
       (Num_Segments   : Integer;
        x1, y1, x2, y2 : Long_Float);


    -- Plots array y against x for n points using ASCII code "code".
    procedure plpoin
       (x, y : Real_Vector;
        Symbol_As_Number : Integer);


    -- Draws a series of points in 3 space.
    procedure plpoin3
       (x, y, z          : Real_Vector;
        Symbol_As_Number : Integer);


    -- Draws a polygon in 3 space.
    procedure plpoly3
       (x, y, z : Real_Vector;
        Draw_Segments : Boolean_Array_1D;
        Draw_Counterclockwise : Boolean);


    -- Set the floating point precision (in number of places) in numeric labels.
    procedure plprec
       (Set_Manually : Boolean := True;
        Number_Digits_After_Decimal_Point : Integer := 2);


    -- Set fill pattern, using one of the predefined patterns.
    procedure plpsty(Fill_Pattern : Fill_Pattern_Type);


    -- Prints out "The_Text" at world cooordinate (x, y).
    procedure plptex
       (x, y             : Long_Float;
        Delta_X, Delta_Y : Long_Float;
        Justification    : Long_Float;
        The_Text         : String);


    -- Prints out "The_Text" at world cooordinate (x, y, z).
    procedure plptex3
       (x, y, z                   : Long_Float;
        Delta_X, Delta_Y, Delta_Z : Long_Float;
        Shear_X, Shear_Y, Shear_Z : Long_Float;
        Justification             : Long_Float;
        The_Text                  : String);


    -- Random number generator based on Mersenne Twister.
    -- Obtain real random number in range [0,1].
    function plrandd return Long_Float;


    -- Replays contents of plot buffer to current device/file.
    procedure plreplot;


    -- Functions for converting between HLS and RGB color space
    procedure plrgbhls
       (Red_Component        : Long_Float_0_1_Type;
        Green_Component      : Long_Float_0_1_Type;
        Blue_Component       : Long_Float_0_1_Type;
        Hue_Component        : out Long_Float;
        Lightness_Component  : out Long_Float_0_1_Type;
        Saturation_Component : out Long_Float_0_1_Type);


    -- Set character height.
    procedure plschr(Default_Height, Scale_Factor : Long_Float);


    -- The PLplot docs say that the arguments to this procedure are arrays of 8-bit numbers
    -- but plplot.h says that they are arrays of 32-bit integers.
    -- Set color map 0 colors by 8 bit RGB values
    procedure plscmap0(Red_Components, Green_Components, Blue_Components : Integer_Array_1D);


    -- Set color map 0 colors by 8 bit RGB values and alpha values
    procedure plscmap0a
       (Red_Components, Green_Components, Blue_Components : Integer_Array_1D;
        Alpha                                             : Real_Vector);


    -- Set number of colors in cmap 0
    procedure plscmap0n(Number_Of_Colors : Integer);


    -- Set color map 1 colors by 8 bit RGB values
    procedure plscmap1
       (Red_Component, Green_Component, Blue_Component : Integer_0_255_Array);


    -- Set color map 1 colors by 8 bit RGB and alpha values
    procedure plscmap1a
       (Red_Component, Green_Component, Blue_Component : Integer_0_255_Array;
        Alpha                                          : Real_Vector);


    -- Types for colors for color map 1.
    type Color_Model_Type is (HLS, RGB); -- Red, Green, Blue or Hue, Lightness, Saturation


    -- Set color map 1 colors using a piece-wise linear relationship between
    -- intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.
    procedure plscmap1l
       (Color_Model    : Color_Model_Type; -- HLS or RGB
        Control_Points : Real_Vector; -- range 0.0 .. 1.0; not checked here
        H_Or_R         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        L_Or_G         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        S_Or_B         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alt_Hue_Path   : Boolean_Array_1D);   -- True means use alternative hue interpolation path which always includes the point hue = 0.  False reverses.


    -- Overloaded version of plscmap1l which allows simplified (non-)reversal of
    -- the traversed hue range. This is an Ada-like way of doing what is described
    -- in the PLplot documentation when a C user passes a null pointer as the
    -- final argument instead of an array of booleans to indicate hue reversal.
    procedure plscmap1l
       (Color_Model    : Color_Model_Type; -- HLS or RGB
        Control_Points : Real_Vector; -- range 0.0 .. 1.0; not checked here
        H_Or_R         : Real_Vector; -- range 0.0 .. 1.0; not checked here
                                              -- Note: Hue is 0.0 .. 360.0.
        L_Or_G         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        S_Or_B         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alt_Hue_Path    : Alt_Hue_Path_Type);


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
        Alt_Hue_Path    : Boolean_Array_1D);   -- True means use alternative hue interpolation path which always includes the point hue = 0.  False reverses.


    -- Overloaded version of plscmap1la which allows simplified (non-)reversal of
    -- the traversed hue range. This is an Ada-like way of doing what is described
    -- in the PLplot documentation when a C user passes a null pointer as the
    -- final argument instead of an array of booleans to indicate hue reversal.
    procedure plscmap1la
       (Color_Model    : Color_Model_Type; -- HLS or RGB
        Control_Points : Real_Vector; -- range 0.0 .. 1.0; not checked here
        H_Or_R         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        L_Or_G         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        S_Or_B         : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alpha          : Real_Vector; -- range 0.0 .. 1.0; not checked here
        Alt_Hue_Path    : Alt_Hue_Path_Type);


    -- Set number of colors in cmap 1
    procedure plscmap1n(Number_Of_Colors : Integer);


    -- Set the color map 1 range used in continuous plots.
    procedure plscmap1_range(Min_Color, Max_Color : Long_Float);


    -- Get the color map 1 range used in continuous plots
    procedure plgcmap1_range(Min_Color, Max_Color : out Long_Float);


    -- Set a given color from color map 0 by 8 bit RGB value
    procedure plscol0
       (Plot_Color : Plot_Color_Type;
        Red_Component, Green_Component, Blue_Component : Integer);


    -- Set a given color from color map 0 by 8 bit RGB value and alpha value
    procedure plscol0a
       (Plot_Color                                     : Plot_Color_Type;
        Red_Component, Green_Component, Blue_Component : Integer;
        Alpha                                          : Long_Float_0_1_Type);


    -- Set the background color by 8 bit RGB value
    procedure plscolbg
       (Red_Component, Green_Component, Blue_Component : Integer);


    -- Set the background color by 8 bit RGB value and alpha value
    procedure plscolbga
       (Red_Component, Green_Component, Blue_Component : Integer;
        Alpha                                          : Long_Float_0_1_Type);


    -- Used to globally turn color output on/off
    procedure plscolor(Enable_Color : Boolean);


    -- Set the compression level
    procedure plscompression(Compression_Level : Integer);


    -- Set the device (keyword) name
    procedure plsdev(Device_Name : String);


    -- Set window into device space using margin, aspect ratio, and
    -- justification
    procedure plsdidev
       (Margin          : Long_Float;
        Aspect_Ratio    : Long_Float;
        x_Justification : Long_Float;
        y_Justification : Long_Float);


    -- Set up transformation from metafile coordinates.
    procedure plsdimap
       (dimxmin : Integer;
        dimxmax : Integer;
        dimymin : Integer;
        dimymax : Integer;
        dimxpmm : Long_Float;
        dimypmm : Long_Float);


    -- Set plot orientation, specifying rotation in units of pi/2.
    procedure plsdiori(Rotation : Long_Float);


    -- Set window into plot space
    procedure plsdiplt
       (x_Min : Long_Float := 0.0;
        y_Min : Long_Float := 0.0;
        x_Max : Long_Float := 1.0;
        y_Max : Long_Float := 1.0);


    -- Set window into plot space incrementally (zoom)
    procedure plsdiplz
       (x_Min_Relative : Long_Float;
        y_Min_Relative : Long_Float;
        x_Max_Relative : Long_Float;
        y_Max_Relative : Long_Float);


    -- Set seed for internal random number generator
    procedure plseed(Seed : Unsigned_Int);


    -- Set the escape character for text strings.
    procedure plsesc(Escape_Character : Character);


    -- Set family file parameters
    procedure plsfam
       (Enable_Family : Boolean;
        Family_File_Number : Integer := 1;
        Maximum_File_Size : Integer := 1_000_000);


    -- Set FCI (font characterization integer)
    procedure plsfci(Font_Characterization_Integer : Unicode);


    -- Set the font family, style and weight
    procedure plsfont(Family, Style, Weight : Integer);


    -- Set the output file name.
    procedure plsfnam(Output_File_Name : String);


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
        Transformation_Data_Pointer              : PL_Pointer);


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
        Transformation_Data_Pointer      : PL_Pointer);


    -- The procedure plfshade is not part of the API. If it should be necessary
    -- to make it available to Ada programs, use the binding to plfcont in this
    -- file as a guideline.

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
    --


    -- Setup a user-provided custom labeling function.
    procedure plslabelfunc
       (Custom_Label_Procedure_Pointer : Custom_Label_Procedure_Pointer_Type;
        label_data : PL_Pointer);


    -- Reset to default labeling. Ada only; not part of the C API.
    procedure Use_Default_Labels;


    -- Set up lengths of major tick marks.
    procedure plsmaj(Default_Length, Scale_Factor : Long_Float);


    -- Set the memory area to be plotted (with the 'mem' driver)
    procedure plsmem
       (x_Size, y_Size : Integer;
        Plot_This      : System.Address);


    -- Set up lengths of minor tick marks.
    procedure plsmin(Default_Length, Scale_Factor : Long_Float);


    -- Set orientation. Must be done before calling plinit.
    procedure plsori(Orientation : Orientation_Type);


    -- Set output device parameters. Usually ignored by the driver.
    procedure plspage
       (x_Pixels, y_Pixels : Long_Float;
        x_Length, y_Length : Integer;
        x_Offset, y_Offset : Integer);


    -- Set the colors for color table 0 from a cmap0 file.
    procedure plspal0(Color_File_Name : String);


    -- Set the colors for color table 1 from a cmap1 file.
    procedure plspal1(Color_File_Name : String; Interpolate : Boolean);


    -- Set the pause (on end-of-page) status
    procedure plspause(Pause : Boolean);


    -- Set stream number.
    procedure plsstrm(Stream_Number : Integer);


    -- Set the number of subwindows in x and y
    procedure plssub(x_Number, y_Number : Integer);


    -- Set symbol height.
    procedure plssym(Default_Height, Scale_Factor : Long_Float);


    -- Initialize PLplot, passing in the windows/page settings.
    procedure plstar
       (Number_Horizontal_Subpages, Number_Vertical_Subpages : Integer := 1);


    -- Initialize PLplot, passing the device name and windows/page settings.
    procedure plstart
       (Device_Name                                          : String;
        Number_Horizontal_Subpages, Number_Vertical_Subpages : Integer := 1);


    -- Set the coordinate transform.
    procedure plstransform
       (Coordinate_Transform_Procedure_Pointer : Coordinate_Transform_Procedure_Pointer_Type;
        Coordinate_Transform_Data_Pointer : PL_Pointer);


    -- Clear the coordinate transform. Ada only; not part of the C API.
    procedure Clear_Custom_Coordinate_Transform;


    -- Prints out the same string repeatedly at the n points in world
    -- coordinates given by the x and y arrays.  Supersedes plpoin and
    -- plsymbol for the case where text refers to a unicode glyph either
    -- directly as UTF-8 or indirectly via the standard text escape
    -- sequences allowed for PLplot input strings.
    procedure plstring
       (x, y : Real_Vector;
        Plot_This_String : String);


    -- Prints out the same string repeatedly at the n points in world
    -- coordinates given by the x, y, and z arrays.  Supersedes plpoin3
    -- for the case where text refers to a unicode glyph either directly
    -- as UTF-8 or indirectly via the standard text escape sequences
    -- allowed for PLplot input strings.
    procedure plstring3
       (x, y, z : Real_Vector;
        Plot_This_String : String);


    -- Add a point to a stripchart.
    procedure plstripa
       (ID         : Integer;
        Pen_Number : Integer;
        x, y       : Long_Float);


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
        X_Label, Y_Label, Title_Label        : String := To_String(Default_Label_String));


    -- Deletes and releases memory used by a stripchart.
    procedure plstripd(ID : Integer);


    -- plots a 2d image (or a matrix too large for plshade() )
    procedure plimagefr -- No documentation in Chapter 17 of Programmer's Reference Manual
       (Data                             : Real_Matrix;
        x_Min,     x_Max                 : Long_Float;
        y_Min,     y_Max                 : Long_Float;
        z_Min,     z_Max                 : Long_Float;
        Value_Min, Value_Max             : Long_Float;
        Transformation_Procedure_Pointer : Transformation_Procedure_Pointer_Type;
        Transformation_Data_Pointer      : PL_Pointer);


    -- plots a 2d image (or a matrix too large for plshade() )
    procedure plimage
       (Data : Real_Matrix;
        x_Min, x_Max : Long_Float;
        y_Min, y_Max : Long_Float;
        z_Min, z_Max : Long_Float;
        Dxmin, Dxmax : Long_Float;
        Dymin, Dymax : Long_Float);


    -- Set up a new line style
    procedure plstyl(Marks, Spaces : Integer_Array_1D);


    Default_Continuous_Line : constant Integer := 0;

    -- This is an overloaded procedure equivalent to calling plstyl with its
    -- first argument zero.
    procedure plstyl(Default_Continuous_Line : Integer);

--==================================================================================================
--==================================================================================================

    -- s_plf2ops_c : Data_Ops_2D_Type :=
    --    (Get => plf2ops_c_get'Access,
    --     Set => plf2ops_c_set'Access,
    --     Add => plf2ops_c_add'Access,
    --     Sub => plf2ops_c_sub'Access,
    --     Mul => plf2ops_c_mul'Access,
    --     Div => plf2ops_c_div'Access,
    --     Is_Nan => plf2ops_c_isnan'Access,
    --     MinMax => plf2ops_c_minmax'Access,
    --     F2Eval => plf2ops_c_f2eval'Access);
--==================================================================================================
--==================================================================================================


    -- Plots the 3d surface representation of the function z(x, y).
    procedure plsurf3d
       (x, y           : Real_Vector; -- surface definition points
        z              : Real_Matrix; -- height of surface at definition points
        Options        : Integer;
        Contour_Levels : Real_Vector); -- levels at which to draw contours


    -- -- Like plsurf3d, but uses an evaluator function to access z data from zp
    -- procedure plfsurf3d
    --    (x , y          : Real_Vector; -- surface definition points
    --     z_Ops          : Data_Ops_2D_Type; -- operations on z
    --     z              : Real_Matrix; -- height of surface at definition points
    --     Options        : Integer;
    --     Contour_Levels : Real_Vector); -- levels at which to draw contours


    -- Plots the 3-d surface representation of the function z(x, y) with limits on x and y.
    procedure plsurf3dl
        (x, y                     : Real_Vector; -- surface definition points
         z                        : Real_Matrix; -- height of surface at definition points
         Options                  : Integer;
         Contour_Levels           : Real_Vector; -- levels at which to draw contours
         Index_x_Min, Index_x_Max : Integer;           -- limits on x
         Index_y_Min, Index_y_Max : Integer_Array_1D); -- limits on y


    -- Sets the edges of the viewport to the specified absolute coordinates
    procedure plsvpa
       (Left_Edge   : Long_Float;  -- millimeters from edge of subpage
        Right_Edge  : Long_Float;  -- millimeters from edge of subpage
        Bottom_Edge : Long_Float;  -- millimeters from edge of subpage
        Top_Edge    : Long_Float); -- millimeters from edge of subpage


    -- Set x axis labeling parameters
    -- "digits" changed to "field_digits".
    procedure plsxax(Max_Digits, Field_Digits : Integer);


    -- Set inferior X window
    procedure plsxwin(Window_ID : Integer);


    -- Set y axis labeling parameters
    -- "digits" changed to "field_digits".
    procedure plsyax(Max_Digits, Field_Digits : Integer);


    -- Plots array y against x for n points using Hershey symbol "code"
    procedure plsym
       (x, y : Real_Vector;
        Hershey_Code : Integer);


    -- Set z axis labeling parameters
    -- "digits" changed to "field_digits".
    procedure plszax(Max_Digits, Field_Digits : Integer);


    -- Switches to text screen.
    procedure pltext;


    -- Set the format for date / time labels
    procedure pltimefmt(Format : String);


    -- Sets the edges of the viewport with the given aspect ratio, leaving
    -- room for labels.
    procedure plvasp(Aspect_Ratio : Long_Float);


    -- Creates the largest viewport of the specified aspect ratio that fits
    -- within the specified normalized subpage coordinates.
    procedure plvpas
       (x_Min, x_Max : Long_Float;
        y_Min, y_Max : Long_Float;
        Aspect_Ratio : Long_Float);


    -- Creates a viewport with the specified normalized subpage coordinates.
    procedure plvpor
       (Left_Edge   : Long_Float := 0.0;
        Right_Edge  : Long_Float := 1.0;
        Bottom_Edge : Long_Float := 0.0;
        Top_Edge    : Long_Float := 1.0);


    -- Defines a "standard" viewport with seven character heights for
    -- the left margin and four character heights everywhere else.
    procedure plvsta;


    -- Set up a window for three-dimensional plotting.
    procedure plw3d
       (X_Box, Y_Box, Z_Box                      : Long_Float;  -- Extents of enclosing box; world coordinates
        X_Min, X_Max, Y_Min, Y_Max, Z_Min, Z_Max : Long_Float;  -- Data limits; user coordinates
        Altitude, Azimuth                        : Long_Float); -- Viewing angles in world coordinates


    -- Set pen width.
    procedure plwidth(Pen_Width : Long_Float);


    -- Set up world coordinates of the viewport boundaries (2d plots).
    procedure plwind
       (Left_Edge   : Long_Float;
        Right_Edge  : Long_Float;
        Bottom_Edge : Long_Float;
        Top_Edge    : Long_Float);

    -- set xor mode; mode = 1-enter, 0-leave, status = 0 if not interactive device
    procedure plxormod
       (Use_XOR : Boolean;
        Supports_XOR : out Boolean);


--------------------------------------------------------------------------------
--         Functions for use from C or C++ only                               --
--         (Not really ;).                                                    --
--------------------------------------------------------------------------------
--  THESE FUNCTIONS ^^^ ARE NOT IMPLEMENTED FOR THE ADA BINDING
--  EXCEPT FOR THE FOLLOWING.

    -- This procedure is currently GNAT-specific, importing Gnat_Argc and Gnat_Argv.
    -- Process options list using current options info.
    procedure plparseopts(Mode : Parse_Mode_Type);


    -- Process input strings, treating them as an option and argument pair.
    procedure plsetopt(Option, Argument : String);


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
    -- remove the extra call level. The argument for the current arrangement is
    -- easier code maintainence.

    -- Identity transformation.
    procedure pltr0
       (x_Grid, y_Grid   : Long_Float;
        x_World, y_World : out Long_Float;
        Data             : PL_Pointer);
    pragma Convention(Convention => C, Entity => pltr0);


    -- Does linear interpolation from singly dimensioned coord arrays.
    procedure pltr1
       (x_Grid, y_Grid   : Long_Float;
        x_World, y_World : out Long_Float;
        Data_Pointer     : PL_Pointer);
    pragma Convention(Convention => C, Entity => pltr1);


    -- Does linear interpolation from doubly dimensioned coord arrays
    -- (column dominant, as per normal C 2d arrays).
    procedure pltr2
       (x_Grid, y_Grid   : Long_Float;
        x_World, y_World : out Long_Float;
        Data_Pointer     : PL_Pointer);
    pragma Convention(Convention => C, Entity => pltr2);


    -- Wait for graphics input event and translate to world coordinates.
    procedure plGetCursor(Graphics_Input : out Graphics_Input_Record_Type);

end PLplot_Traditional;
