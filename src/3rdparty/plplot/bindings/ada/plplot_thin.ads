-- Thin Ada binding to PLplot

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

--------------------------------------------------------------------------------

-- SOME NOTES ABOUT THE ADA BINDINGS (JB)
-- Some C-arguments of the form PLINT *a are supposed to point at 8-bit unsigned chars
-- and are said to carry information out of the function, according to the PLplot docs.
-- I hope that this is wrong and that they are really PLINTs like the API says.
-- I converted these as a : out PLINT. These are the only *-ed quantities that I set to "outs".

-- I made the final arg in plscmap1l, alt_hue_path, an array of PLBOOLs, not an in out
-- parameter; impossible to tell from C syntax which is intended, but I think
-- this is right.

with
    Interfaces.C,
    Interfaces.C.Strings,
    Interfaces.C.Pointers,
    PLplot_Auxiliary,
    System,
    Ada.Text_IO,
    Ada.Strings.Bounded,
    Ada.Strings.Unbounded;
use
    Interfaces.C,
    Interfaces.C.Strings,
    PLplot_Auxiliary,
    Ada.Text_IO,
    Ada.Strings.Bounded,
    Ada.Strings.Unbounded;

package PLplot_Thin is
    subtype PLINT  is Integer;
    subtype PLFLT  is Long_Float;
    PLfalse : constant Integer := 0;
    PLtrue  : constant Integer := 1;
    subtype PLBOOL is Integer range PLfalse..PLtrue;
    type PLUNICODE is mod 2**32;

    type Boolean_Array_1D is array (Integer range <>) of Boolean;
    type Integer_Array_1D is array (Integer range <>) of Integer;

    subtype PL_Integer_Array  is Integer_Array_1D;
    subtype PL_Float_Array    is Real_Vector;
    subtype PL_Float_Array_2D is Real_Matrix;
    type PL_Bool_Array        is array (Integer range <>) of PLBOOL;
    subtype PL_Pointer        is System.Address; -- non-specific pointer to something
    -- The following deprecated spelling is not used anywhere in the Ada bindings
    -- or examples but is provided for backwards compatibility.
    subtype PLpointer is System.Address; -- DEPRECATED non-specific pointer to something

    -- Types to pass 2D arrays, Real_Matrix, defined in plplot_auxiliary.ads, to C functions. Have
    -- only Long_Float capability for now. See function Matrix_To_Pointers for the conversion.
    type Long_Float_Pointer is access all Long_Float;
    type Long_Float_Pointer_Array is array (Integer range <>) of aliased Long_Float_Pointer;


--------------------------------------------------------------------------------
-- Utility for passing matrices to C                                          --
--------------------------------------------------------------------------------

    function Matrix_To_Pointers(x : Real_Matrix) return Long_Float_Pointer_Array;


--------------------------------------------------------------------------------
-- Transpose a Matrix.                                                        --
--------------------------------------------------------------------------------

    function PL_Transpose(A : Real_Matrix) return Real_Matrix;


--------------------------------------------------------------------------------
-- PLplot-specific things                                                     --
--------------------------------------------------------------------------------

    -- Make a string and array therefore for stripchart legends that is compatible with
    -- the C code of plstripc which creates a strip chart. The C code will accept
    -- any length of string and each of the four strings can be different lengths.
    -- These types are a bit of a hack whereby each of the legend strings is the
    -- same length, so that they can be accessed as an array. They are Max_Stripchart_Label_Length
    -- characters long here including a null terminator; I suppose any length
    -- will work since the stripchart program doesn't seem to mind. The user
    -- will probably never see this type since he is allowed to set up the
    -- legend strings as an array of unbounded strings. Only in preparing to
    -- call the underlying C are the fixed-length strings used. If the user
    -- specifies unbounded legend strings that are longer than allowed here,
    -- they are truncated to meet the required length. Although that length
    -- is now Max_Stripchart_Label_Length, it should be OK to make it
    -- longer or shorter simply by changing the following line, since everything
    -- else (truncation, conversion to C-style) works automatically.
    -- The current implementation uses char_array for the strings and uses
    -- System.Address as the pointer which are assigned to an array of type
    -- PL_Stripchart_String_Array (see plstripc). I think it is also possible
    -- to subtype String and use access variables assigned to an array of
    -- suitable access types.
    -- One very important note is that in forming the C string, the null
    -- terminator will normally come in the middle of the array, right after
    -- the "significant" part of the string. This is handled in plstripc.
    Max_Stripchart_Label_Length : constant Integer := 100;
    subtype PL_Stripchart_String is
        Interfaces.C.char_array(0 .. size_t(Max_Stripchart_Label_Length));
    type PL_Stripchart_String_Array is array (1 .. 4) of PL_Pointer;
    C_Stripchart_String_Array : array(1 .. 4) of PL_Stripchart_String;


    -- Make a string and array therefore for plot (not stripchart) legends that
    -- is compatible with the C code of pllegend. This is modelled on the way
    -- that legends for stripcharts are handled except there, the maximum
    -- number of legends was four and here we don't know until runtime what the
    -- maximum number of legends will be. The user will have to define this is
    -- his code; see e.g. Example 4.
    Max_Legend_Label_Length : constant Integer := 100;
    subtype PL_Legend_String is
        Interfaces.C.char_array(0 .. size_t(Max_Legend_Label_Length));
    type PL_Legend_String_Array is array (Integer range <>) of PL_Pointer;


    -- Access-to-procedure type for Draw_Vector_Plot and its kin.
    type Transformation_Procedure_Pointer_Type is access
        procedure (x, y : PLFLT; tx, ty : out PLFLT; pltr_data : PL_Pointer);
    pragma Convention (Convention => C, Entity => Transformation_Procedure_Pointer_Type);


    -- Access-to-procedure to e.g. to plfill, used by plshade and plshades.
    -- Needs C calling convention because it is passed eventually to plfill aka
    -- c_plfill in plplot.h. plfill is said in the documentation to be possibly
    -- supplemented in the future.
    type Fill_Procedure_Pointer_Type is access
        procedure(length : Integer; x, y : Real_Vector);
    pragma Convention(Convention => C, Entity => Fill_Procedure_Pointer_Type);


    -- Access-to-function type for Shade_Regions (aka plshades).
    -- Returns 1 if point is to be plotted, 0 if not.
    type Mask_Function_Pointer_Type is access
        function (x, y : PLFLT) return Integer;
    pragma Convention(Convention => C, Entity => Mask_Function_Pointer_Type);

    -- Make a constrained array for use by plmap (Draw_Map) and
    -- plmeridians (Draw_Latitude_Longitude). The reason that this is necessary
    -- is that the callback structure for those routines requires that a C
    -- subprogram calls an Ada subroutine *with an unconstrained array*. This
    -- seems to be unique in the PLplot bindings. For a discussion of this, see
    -- http://groups.google.com/group/comp.lang.ada/browse_thread/thread/0e5a3abec221df39?hl=en#
    -- If the user needs to have a larger array, it will require making this
    -- array longer, but we try to anticipate the most extreme situation here
    -- without being too wasteful. The user will typically use only a portion of
    -- the array, beginning at index 0.
    subtype Map_Form_Constrained_Array is Real_Vector(0 .. 2000);

    -- Access-to-procedure type for plotting map outlines (continents).
    -- Length_Of_x is x'Length or y'Length; this is the easiest way to match the
    -- C formal arguments.
    type Map_Form_Function_Pointer_Type is access
        procedure (Length_Of_x : Integer; x, y : in out Map_Form_Constrained_Array);
    pragma Convention(Convention => C, Entity => Map_Form_Function_Pointer_Type);

    -- Access-to-function type for making contour plots from irregularly
    -- spaced data which is stored in a 2D array accessed by a single pointer.
    -- Examples of such functions are in plplot.h and are called plf2eval,
    -- plf2eval2, plf2evalr.
    type Function_Evaluator_Pointer_Type is access
        function (ix, iy : PLINT; Irregular_Data : PL_Pointer) return PLFLT;
    pragma Convention(Convention => C, Entity => Function_Evaluator_Pointer_Type);


    -- Fixed-length string for use with custom label procedures
    -- in Custom_Label_Procedure_Pointer_Type, and plslabelfunc (Set_Custom_Label).
    -- This length, 0 .. 40, is hardwired in the PLplot C code; this type will
    -- fail if that length is ever changed.
    Max_Label_String_Length : constant Integer := 40;
    subtype Label_String_Type is Interfaces.C.char_array (0 .. size_t(Max_Label_String_Length));

    -- Access-to-function type for making custom labels such as with plslabelfunc.
    type Custom_Label_Procedure_Pointer_Type is access
        procedure
           (axis    : Integer;
            a_value : PLFLT;
            label   : out Label_String_Type;
            length  : size_t;
            data    : PL_Pointer);
    pragma Convention(Convention => C, Entity => Custom_Label_Procedure_Pointer_Type);

    -- Access-to-procedure type for setting a global custom coordinate tranformation.
    type Coordinate_Transform_Procedure_Pointer_Type is access
        procedure
           (x_In, y_In                   : PLFLT;
            x_Transformed, y_Transformed : out PLFLT;
            data                         : PL_Pointer);
    pragma Convention(Convention => C, Entity => Coordinate_Transform_Procedure_Pointer_Type);


--------------------------------------------------------------------------------
-- Argument parsing things                                                    --
--------------------------------------------------------------------------------

    Gnat_argc : aliased Integer;
    pragma Import (C, Gnat_argc);

    Gnat_argv : System.Address; -- Just get the base address to argv, like C
    pragma Import (C, Gnat_argv);


--------------------------------------------------------------------------------
-- More PLplot-specific things                                                --
--------------------------------------------------------------------------------

    -- Switches for escape function call.
    -- Some of these are obsolete but are retained in order to process
    -- old metafiles

    PLESC_SET_RGB         : constant Integer := 1;	-- obsolete
    PLESC_ALLOC_NCOL      : constant Integer := 2;	-- obsolete
    PLESC_SET_LPB         : constant Integer := 3;	-- obsolete
    PLESC_EXPOSE          : constant Integer := 4;	-- handle window expose
    PLESC_RESIZE          : constant Integer := 5;	-- handle window resize
    PLESC_REDRAW          : constant Integer := 6;	-- handle window redraw
    PLESC_TEXT            : constant Integer := 7;	-- switch to text screen
    PLESC_GRAPH           : constant Integer := 8;	-- switch to graphics screen
    PLESC_FILL            : constant Integer := 9;	-- fill polygon
    PLESC_DI              : constant Integer := 10;	-- handle DI command
    PLESC_FLUSH           : constant Integer := 11;	-- flush output
    PLESC_EH              : constant Integer := 12; -- handle Window events
    PLESC_GETC            : constant Integer := 13;	-- get cursor position
    PLESC_SWIN            : constant Integer := 14;	-- set window parameters
    PLESC_DOUBLEBUFFERING : constant Integer := 15;	-- configure double buffering
    PLESC_XORMOD          : constant Integer := 16;	-- set xor mode
    PLESC_SET_COMPRESSION : constant Integer := 17;	-- AFR: set compression
    PLESC_CLEAR           : constant Integer := 18; -- RL: clear graphics region
    PLESC_DASH            : constant Integer := 19;	-- RL: draw dashed line
    PLESC_HAS_TEXT        : constant Integer := 20;	-- driver draws text
    PLESC_IMAGE           : constant Integer := 21;	-- handle image
    PLESC_IMAGEOPS        : constant Integer := 22; -- plimage related operations
    PLESC_PL2DEVCOL       : constant Integer := 23;	-- convert PLColor to device color
    PLESC_DEV2PLCOL       : constant Integer := 24;	-- convert device color to PLColor
    PLESC_SETBGFG         : constant Integer := 25;	-- set BG, FG colors
    PLESC_DEVINIT         : constant Integer := 26;	-- alternate device initialization

    -- image operations
    ZEROW2B : constant Integer := 1;
    ZEROW2D : constant Integer := 2;
    ONEW2B  : constant Integer := 3;
    ONEW2D  : constant Integer := 4;

    -- Window parameter tags

    PLSWIN_DEVICE : constant Integer := 1; -- device coordinates
    PLSWIN_WORLD  : constant Integer := 2; -- world coordinates

    -- Axis label tags
    PL_X_AXIS : constant Integer := 1; -- The x-axis
    PL_Y_AXIS : constant Integer := 2; -- The y-axis
    PL_Z_AXIS : constant Integer := 3; -- The z-axis

    -- PLplot Option table & support constants

    -- Option-specific settings

    PL_OPT_ENABLED   : constant Integer := 16#0001#; -- Obsolete
    PL_OPT_ARG       : constant Integer := 16#0002#; -- Option has an argument
    PL_OPT_NODELETE  : constant Integer := 16#0004#; -- Don't delete after processing
    PL_OPT_INVISIBLE : constant Integer := 16#0008#; -- Make invisible
    PL_OPT_DISABLED  : constant Integer := 16#0010#; -- Processing is disabled

    -- Option-processing settings -- mutually exclusive

    PL_OPT_FUNC   : constant Integer := 16#0100#; -- Call handler function
    PL_OPT_BOOL   : constant Integer := 16#0200#; -- Set *var = 1
    PL_OPT_INT    : constant Integer := 16#0400#; -- Set *var = atoi(optarg)
    PL_OPT_FLOAT  : constant Integer := 16#0800#; -- Set *var = atof(optarg)
    PL_OPT_STRING : constant Integer := 16#1000#; -- Set var = optarg

    -- Global mode settings
    -- These override per-option settings

    type Parse_Mode_Type is range 16#0000# .. 16#0001# + 16#0002# + 16#0004# +
        16#0008# + 16#0010# + 16#0020# + 16#0040# + 16#0080#;

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

    -- fix this
    --type PLOptionTable is
    --    record
    --        opt : char_array;
    --        int  (*handler)	(char *, char *, void *);
    --        void *client_data;
    --        void *var;
    --        mode : long;
    --        syntax : char_array;
    --        desc : char_array;
    --    end record;

    -- PLplot Graphics Input structure


    PL_MAXKEY : constant Integer := 16;

    type PLGraphicsIn is
        record
            Event_Type : Integer; -- of event (CURRENTLY UNUSED)
            state      : unsigned; -- key or button mask
            keysym     : unsigned; -- key selected
            button     : unsigned; -- mouse button selected
            subwindow  : PLINT;    -- subwindow (alias subpage, alias subplot) number
            T_String   : String(1 .. PL_MAXKEY);
            pX, pY     : integer;  -- absolute device coordinates of pointer
            dX, dY     : PLFLT;    -- relative device coordinates of pointer
            wX, wY     : PLFLT;    -- world coordinates of pointer
        end record;


    -- Structure for describing the plot window

    PL_MAXWINDOWS : constant Integer := 64;	-- Max number of windows/page tracked

    type PLWindow is
        record
            dxmi, dxma, dymi, dyma : PLFLT;	-- min, max window rel dev coords
            wxmi, wxma, wymi, wyma : PLFLT;	-- min, max window world coords
        end record;

    -- Structure for doing display-oriented operations via escape commands
    -- May add other attributes in time

    type PLDisplay is
        record
            x, y          : unsigned; -- upper left hand corner
            width, height : unsigned; -- window dimensions
        end record;

    -- Macro used (in some cases) to ignore value of argument
    -- I don't plan on changing the value so you can hard-code it

    -- Long_Float for Ada binding. Used with plsdidev which requires Long_Float.
    -- C doesn't care if it is an integer, apparently converting it to a float.
    PL_NOTSET : constant Long_Float := -42.0;

    -- See plcont.c for examples of the following


    -- PLfGrid is for passing (as a pointer to the first element) an arbitrarily
    -- dimensioned array.  The grid dimensions MUST be stored, with a maximum of 3
    -- dimensions assumed for now.

    -- PLplot appears to not use this. The documentation is unclear what this it supposed to do and
    -- thus it can't be ascertained how to dimension it.
    -- typedef struct
    -- {
    --     const PLFLT *f;
    --     PLINT       nx, ny, nz;
    -- } PLfGrid;

    -- PLfGrid2 is for passing (as an array of pointers) a 2d function array.  The
    -- grid dimensions are passed for possible bounds checking.

    -- I don't think Ada needs this since we use the procedure Matrix_To_Pointers to connect an Ada
    -- Real_Matrix to C's mess of pointers. It's just a matrix. But it's here if we need it.
    type PLfGrid2(x_Last, y_Last : Natural) is
        record
            f : Real_Matrix(0 .. x_Last, 0 .. y_Last);
            nx : Natural := x_Last + 1;
            ny : Natural := y_Last + 1;
        end record;

    -- NOTE: a PLfGrid3 is a good idea here but there is no way to exploit it yet
    -- so I'll leave it out for now.

    -- PLcGrid is for passing (as a pointer to the first element) arbitrarily
    -- dimensioned coordinate transformation arrays.  The grid dimensions MUST be
    -- stored, with a maximum of 3 dimensions assumed for now.

    -- This variant record emulates PLcGrid. Use it in plvect, plcont, plfcont,
    -- plshade, c_plshades, plfshade, plf2eval2, plf2eval, plf2evalr,
    -- PLcGrid and Ada counterparts thereof.
    type Transformation_Data_Type (x_Last, y_Last, z_Last : Natural) is
        record
            xg : PL_Float_Array(0 .. x_Last);
            yg : PL_Float_Array(0 .. y_Last);
            zg : PL_Float_Array(0 .. z_Last);
            nx : Natural := x_Last + 1;
            ny : Natural := y_Last + 1;
            nz : Natural := z_Last + 1;
        end record;

    -- Provide Transformation_Data_Type as named in the documentation.
    subtype PLcGrid is Transformation_Data_Type;

    -- PLcGrid2 is for passing (as arrays of pointers) 2d coordinate
    -- transformation arrays.  The grid dimensions are passed for possible bounds
    -- checking.

    -- This variant record emulates PLcGrid2.
    type Transformation_Data_Type_2 (x_Last, y_Last : Natural) is
        record
            xg : PL_Float_Array_2D(0 .. x_Last, 0 .. y_Last);
            yg : PL_Float_Array_2D(0 .. x_Last, 0 .. y_Last);
            zg : PL_Float_Array_2D(0 .. x_Last, 0 .. y_Last);
            nx : Natural := x_Last + 1;
            ny : Natural := y_Last + 1;
        end record;

    -- Provide Transformation_Data_Type_2 as named in the documentation.
    subtype PLcGrid2 is Transformation_Data_Type_2;


    --NOTE: a PLcGrid3 is a good idea here but there is no way to exploit it yet
    --so I'll leave it out for now.


    -- PLColor is the usual way to pass an rgb color value.
    -- fix this
    -- This can't be converted to Ada without knowing what "name" is used for.
    -- There is nothing in the documentation about this structure nor is it used
    -- in any of the examples.
    --type PLColor is
    --    record
    --        r : unsigned_char; -- red fix this surely these can be mod 2**3
    --        g : unsigned_char; -- green
    --        b : unsigned_char; -- blue
    --        name : char_array;
    --    end record;

    -- PLControlPt is how cmap1 control points are represented.

    type PLControlPt is
        record
            h   : PLFLT;   -- hue
            l   : PLFLT;   -- lightness
            s   : PLFLT;   -- saturation
            p   : PLFLT;   -- position
            alt_hue_path : Integer; -- if set, interpolate through h=0
        end record;

    -- A PLBufferingCB is a control block for interacting with devices
    -- that support double buffering.

    type PLBufferingCB is
        record
            cmd    : PLINT;
            result : PLINT;
        end record;

    PLESC_DOUBLEBUFFERING_ENABLE  : constant Integer := 1;
    PLESC_DOUBLEBUFFERING_DISABLE : constant Integer := 2;
    PLESC_DOUBLEBUFFERING_QUERY   : constant Integer := 3;


--------------------------------------------------------------------------
--       Function Prototypes                                            --
--------------------------------------------------------------------------

    -- set the format of the contour labels

    procedure
    pl_setcontlabelformat(lexp : PLINT; sigdig : PLINT);
    pragma Import(C, pl_setcontlabelformat, "c_pl_setcontlabelformat");


    -- set offset and spacing of contour labels

    procedure
    pl_setcontlabelparam(offset : PLFLT; size : PLFLT; spacing : PLFLT; active : PLINT);
    pragma Import(C, pl_setcontlabelparam, "c_pl_setcontlabelparam");


    -- Advance to subpage "page", or to the next one if "page" = 0.

    procedure
    pladv(page : PLINT);
    pragma Import(C, pladv, "c_pladv");


    -- Plot an arc.
    procedure
    plarc(x : PLFLT; y : PLFLT; a : PLFLT; b : PLFLT; angle1 : PLFLT; angle2 : PLFLT; rotate : PLFLT; fill : PLBOOL);
    pragma Import(C, plarc, "c_plarc");


    -- simple arrow plotter.

    procedure
    plvect(u : Long_Float_Pointer_Array; v : Long_Float_Pointer_Array;
        nx : PLINT; ny : PLINT; scale : PLFLT;
        pltr : Transformation_Procedure_Pointer_Type; pltr_data : PL_Pointer);
    pragma Import(C, plvect, "c_plvect");


    -- Set arrow style for vector plots.

    procedure
    plsvect(arrowx : PL_Float_Array; arrowy : PL_Float_Array; npts : PLINT; fill : PLINT);
    pragma Import(C, plsvect, "c_plsvect");


    -- Ada hack to accomodate the C API that allows this call in C
    -- plsvect( NULL, NULL, 0, 0 );
    -- to reset the arrow style to a default value.

    procedure
    plsvectdefault(arrowx : PL_Pointer; arrowy : PL_Pointer; npts : PLINT; fill : PLINT);
    pragma Import(C, plsvectdefault, "c_plsvect");



    -- This functions similarly to plbox() except that the origin of the axes
    -- is placed at the user-specified point (x0, y0).

    procedure
    plaxes(x0 : PLFLT; y0 : PLFLT; xopt : char_array; xtick : PLFLT; nxsub : PLINT;
         yopt : char_array; ytick : PLFLT; nysub : PLINT);
    pragma Import(C, plaxes, "c_plaxes");


    -- Plot a histogram using x to store data values and y to store frequencies

    -- Flags for the opt argument in plbin
    PL_Bin_Default  : constant Integer := 0;
    PL_Bin_Centred  : constant Integer := 1;
    PL_Bin_Noexpand : constant Integer := 2;
    PL_Bin_Noempty  : constant Integer := 4;

    procedure
    plbin(nbin : PLINT; x : PL_Float_Array; y : PL_Float_Array; center : PLINT);
    pragma Import(C, plbin, "c_plbin");


    -- Calculate broken-down time from continuous time for current stream.
    procedure
    plbtime(year, month, day, hour, min : out PLINT;
        sec : out PLFLT; ctime : PLFLT);
    pragma Import(C, plbtime, "c_plbtime");


    -- Start new page.  Should only be used with pleop().

    procedure
    plbop;
    pragma Import(C, plbop, "c_plbop");


    -- This draws a box around the current viewport.

    procedure
    plbox(xopt : char_array; xtick : PLFLT; nxsub : PLINT;
        yopt : char_array; ytick : PLFLT; nysub : PLINT);
    pragma Import(C, plbox, "c_plbox");


    -- This is the 3-d analogue of plbox().

    procedure
    plbox3(xopt : char_array; xlabel : char_array; xtick : PLFLT; nsubx : PLINT;
           yopt : char_array; ylabel : char_array; ytick : PLFLT; nsuby : PLINT;
           zopt : char_array; zlabel : char_array; ztick : PLFLT; nsubz : PLINT);
    pragma Import(C, plbox3, "c_plbox3");


    -- Calculate world coordinates and subpage from relative device coordinates.

    procedure
    plcalc_world(rx : PLFLT; ry : PLFLT; wx : out PLFLT; wy : out PLFLT; window : out PLINT);
    pragma Import(C, plcalc_world, "c_plcalc_world");


    -- Clear current subpage.

    procedure
    plclear;
    pragma Import(C, plclear, "c_plclear");


    -- Set color, map 0.  Argument is integer between 0 and 15.

    procedure
    plcol0(icol0 : PLINT);
    pragma Import(C, plcol0, "c_plcol0");


    -- Set color, map 1.  Argument is a float between 0. and 1.

    procedure
    plcol1(col1 : PLFLT);
    pragma Import(C, plcol1, "c_plcol1");


    -- Configure transformation between continuous and broken-down time (and
    -- vice versa) for current stream.
    procedure
    plconfigtime(skale, offset1, offset2 : PLFLT; ccontrol : PLINT;
        ifbtime_offset : PLBOOL; year, month, day, hour, min : PLINT; sec : PLFLT);
    pragma Import(C, plconfigtime, "c_plconfigtime");


    -- Draws a contour plot from data in f(nx,ny).  Is just a front-end to
    -- plfcont, with a particular choice for f2eval and f2eval_data.

    procedure
    plcont(z : Long_Float_Pointer_Array; nx, ny : Integer;
        kx, lx, ky, ly : Integer; clevel : PL_Float_Array; nlevel : Integer;
        pltr : Transformation_Procedure_Pointer_Type; pltr_data : PL_Pointer);
    pragma Import(C, plcont, "c_plcont");


    -- Draws a contour plot using the function evaluator f2eval and data stored
    -- by way of the f2eval_data pointer.  This allows arbitrary organizations
    -- of 2d array data to be used.

    procedure
    plfcont(f2eval : Function_Evaluator_Pointer_Type;
        Irregular_Data : PL_Pointer;
        nx, ny : PLINT;
        kx, lx, ky, ly: PLINT; clevel : PL_Float_Array; nlevel : PLINT;
        pltr : Transformation_Procedure_Pointer_Type; pltr_data : PL_Pointer);
    pragma Import(C, plfcont, "plfcont");


    -- Copies state parameters from the reference stream to the current stream.

    procedure
    plcpstrm(iplsr : PLINT; flags : PLINT);
    pragma Import(C, plcpstrm, "c_plcpstrm");


    -- Calculate continuous time from broken-down time for current stream.

    procedure
    plctime(year, month, day, hour, min : PLINT; sec : PLFLT;
        ctime : out PLFLT);
    pragma Import(C, plctime, "c_plctime");


    -- Converts input values from relative device coordinates to relative plot
    -- coordinates.

    procedure
    pldid2pc(xmin : PL_Float_Array; ymin : PL_Float_Array; xmax : PL_Float_Array; ymax : PL_Float_Array);
    pragma Import(C, pldid2pc, "pldid2pc");

    -- Converts input values from relative plot coordinates to relative
    -- device coordinates.

    procedure
    pldip2dc(xmin : PL_Float_Array; ymin : PL_Float_Array; xmax : PL_Float_Array; ymax : PL_Float_Array);
    pragma Import(C, pldip2dc, "pldip2dc");

    -- End a plotting session for all open streams.

    procedure
    plend;
    pragma Import(C, plend, "c_plend");


    -- End a plotting session for the current stream only.

    procedure
    plend1;
    pragma Import(C, plend1, "c_plend1");


    -- Simple interface for defining viewport and window.

    procedure
    plenv(xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT;
        just : PLINT; axis : PLINT);
    pragma Import(C, plenv, "c_plenv");



    -- similar to plenv() above, but in multiplot mode does not advance the subpage,
    -- instead the current subpage is cleared

    procedure
    plenv0(xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT;
        just : PLINT; axis : PLINT);
    pragma Import(C, plenv0, "c_plenv0");


    -- End current page.  Should only be used with plbop().

    procedure
    pleop;
    pragma Import(C, pleop, "c_pleop");


    -- Plot horizontal error bars (xmin(i),y(i)) to (xmax(i),y(i))

    procedure
    plerrx(n : PLINT; xmin : PL_Float_Array; xmax : PL_Float_Array; y : PL_Float_Array);
    pragma Import(C, plerrx, "c_plerrx");


    -- Plot vertical error bars (x,ymin(i)) to (x(i),ymax(i))

    procedure
    plerry(n : PLINT; x : PL_Float_Array; ymin : PL_Float_Array; ymax : PL_Float_Array);
    pragma Import(C, plerry, "c_plerry");


    -- Advance to the next family file on the next new page

    procedure
    plfamadv;
    pragma Import(C, plfamadv, "c_plfamadv");


    -- Pattern fills the polygon bounded by the input points.

    procedure
    plfill(n : PLINT; x : PL_Float_Array; y : PL_Float_Array);
    pragma Import(C, plfill, "c_plfill");


    -- Pattern fills the 3d polygon bounded by the input points.

    procedure
    plfill3(n : PLINT; x : PL_Float_Array; y : PL_Float_Array; z : PL_Float_Array);
    pragma Import(C, plfill3, "c_plfill3");


    -- Flushes the output stream.  Use sparingly, if at all.

    procedure
    plflush;
    pragma Import(C, plflush, "c_plflush");


    -- Sets the global font flag to 'ifont'.

    procedure
    plfont(ifont : PLINT);
    pragma Import(C, plfont, "c_plfont");


    -- Load specified font set.

    procedure
    plfontld(fnt : PLINT);
    pragma Import(C, plfontld, "c_plfontld");


    -- Get character default height and current (scaled) height

    procedure
    plgchr(p_def : out PLFLT; p_ht : out PLFLT);
    pragma Import(C, plgchr, "c_plgchr");


    -- Returns 8 bit RGB values for given color from color map 0

    procedure
    plgcol0(icol0 : PLINT; r : out PLINT; g : out PLINT; b : out PLINT);
    pragma Import(C, plgcol0, "c_plgcol0");


    -- Returns 8 bit RGB values for given color from color map 0 and alpha value

    procedure
    plgcol0a(icol0 : PLINT; r, g, b: out PLINT; a : out PLFLT);
    pragma Import(C, plgcol0a, "c_plgcol0a");


    -- Returns the background color by 8 bit RGB value

    procedure
    plgcolbg(r : out PLINT; g : out PLINT; b : out PLINT);
    pragma Import(C, plgcolbg, "c_plgcolbg");


    -- Returns the background color by 8 bit RGB value and alpha value

    procedure
    plgcolbga(r, g, b : out PLINT; a : out PLFLT);
    pragma Import(C, plgcolbga, "c_plgcolbga");


    -- Returns the current compression setting

    procedure
    plgcompression(compression : out PLINT);
    pragma Import(C, plgcompression, "c_plgcompression");


    -- Get the current device (keyword) name

    procedure
    plgdev(p_dev : out char_array);
    pragma Import(C, plgdev, "c_plgdev");


    -- Retrieve current window into device space

    procedure
    plgdidev(p_mar : out PLFLT; p_aspect : out PLFLT; p_jx : out PLFLT; p_jy : out PLFLT);
    pragma Import(C, plgdidev, "c_plgdidev");


    -- Get plot orientation

    procedure
    plgdiori(p_rot : out PLFLT);
    pragma Import(C, plgdiori, "c_plgdiori");


    -- Retrieve current window into plot space

    procedure
    plgdiplt(p_xmin : out PLFLT; p_ymin : out PLFLT; p_xmax : out PLFLT; p_ymax : out PLFLT);
    pragma Import(C, plgdiplt, "c_plgdiplt");


    -- Get FCI (font characterization integer)

    procedure
    plgfci(pfci : out PLUNICODE);
    pragma Import(C, plgfci, "c_plgfci");


    -- Get family, style and weight of the current font

    procedure
    plgfont(p_family, p_style, p_weight : out PLINT);
    pragma Import(C, plgfont, "c_plgfont");


    -- Get family file parameters

    procedure
    plgfam(p_fam : out PLINT; p_num : out PLINT; p_bmax : out PLINT);
    pragma Import(C, plgfam, "c_plgfam");


    -- Get the (current) output file name.  Must be preallocated to >80 bytes

    procedure
    plgfnam(fnam : out char_array);
    pragma Import(C, plgfnam, "c_plgfnam");


    -- Get the (current) run level.

    procedure
    plglevel(p_level : out PLINT);
    pragma Import(C, plglevel, "c_plglevel");


    -- Get output device parameters.

    procedure
    plgpage(p_xp : out PLFLT; p_yp : out PLFLT;
          p_xleng : out PLINT; p_yleng : out PLINT; p_xoff : out PLINT; p_yoff : out PLINT);
    pragma Import(C, plgpage, "c_plgpage");


    -- Switches to graphics screen.

    procedure
    plgra;
    pragma Import(C, plgra, "c_plgra");


    -- Draw gradient in polygon.

    procedure
    plgradient(n : PLINT; x : PL_Float_Array; y : PL_Float_Array; angle : PLFLT);
    pragma Import(C, plgradient, "c_plgradient");


      -- grid irregularly sampled data
    -- Ada Note: Since z_Gridded is passed as an array of pointers, this is
    -- effectively the same as pass-by-reference so "in out" is not required for
    -- the formal argument zg in plgriddata.

    procedure
    plgriddata(x : PL_Float_Array; y : PL_Float_Array; z : PL_Float_Array; npts : PLINT;
           xg : PL_Float_Array; nptsx : PLINT; yg : PL_Float_Array;  nptsy : PLINT;
           zg : Long_Float_Pointer_Array; grid_type : PLINT; data : PLFLT); --"type" re-named "grid_type" for Ada
    pragma Import(C, plgriddata, "c_plgriddata");


      -- type of gridding algorithm for plgriddata()

    GRID_CSA    : constant Integer := 1; -- Bivariate Cubic Spline approximation
    GRID_DTLI   : constant Integer := 2; -- Delaunay Triangulation Linear Interpolation
    GRID_NNI    : constant Integer := 3; -- Natural Neighbors Interpolation
    GRID_NNIDW  : constant Integer := 4; -- Nearest Neighbors Inverse Distance Weighted
    GRID_NNLI   : constant Integer := 5; -- Nearest Neighbors Linear Interpolation
    GRID_NNAIDW : constant Integer := 6; -- Nearest Neighbors Around Inverse Distance Weighted

    -- Get subpage boundaries in absolute coordinates

    procedure
    plgspa(xmin : out PLFLT; xmax : out PLFLT; ymin : out PLFLT; ymax : out PLFLT);
    pragma Import(C, plgspa, "c_plgspa");


    -- Get current stream number.

    procedure
    plgstrm(p_strm : out PLINT);
    pragma Import(C, plgstrm, "c_plgstrm");


    -- Get the current library version number

    procedure
    plgver(p_ver : out char_array);
    pragma Import(C, plgver, "c_plgver");


    -- Get viewport boundaries in normalized device coordinates

    procedure
    plgvpd(p_xmin : out PLFLT; p_xmax : out PLFLT; p_ymin : out PLFLT; p_ymax : out PLFLT);
    pragma Import(C, plgvpd, "c_plgvpd");


    -- Get viewport boundaries in world coordinates

    procedure
    plgvpw(p_xmin : out PLFLT; p_xmax : out PLFLT; p_ymin : out PLFLT; p_ymax : out PLFLT);
    pragma Import(C, plgvpw, "c_plgvpw");


    -- Get x axis labeling parameters

    procedure
    plgxax(p_digmax : out PLINT; p_digits : out PLINT);
    pragma Import(C, plgxax, "c_plgxax");


    -- Get y axis labeling parameters

    procedure
    plgyax(p_digmax : out PLINT; p_digits : out PLINT);
    pragma Import(C, plgyax, "c_plgyax");


    -- Get z axis labeling parameters

    procedure
    plgzax(p_digmax : out PLINT; p_digits : out PLINT);
    pragma Import(C, plgzax, "c_plgzax");


    -- Draws a histogram of n values of a variable in array data[0..n-1]

    -- Flags for the opt argument in plhist
    PL_Hist_Default         : constant Integer := 0;
    PL_Hist_Noscaling       : constant Integer := 1;
    PL_Hist_Ignore_Outliers : constant Integer := 2;
    PL_Hist_Noexpand        : constant Integer := 8;
    PL_Hist_Noempty         : constant Integer := 16;

    procedure
    plhist(n : PLINT; data : PL_Float_Array; datmin : PLFLT; datmax : PLFLT;
         nbin : PLINT; oldwin : PLINT);
    pragma Import(C, plhist, "c_plhist");


    -- Functions for converting between HLS and RGB color space

    procedure
    plhlsrgb(h : PLFLT; l : PLFLT; s : PLFLT; p_r : out PLFLT; p_g : out PLFLT; p_b : out PLFLT);
    pragma Import(C, plhlsrgb, "c_plhlsrgb");


    -- Initializes PLplot, using preset or default options

    procedure
    plinit;
    pragma Import(C, plinit, "c_plinit");


    -- Draws a line segment from (x1, y1) to (x2, y2).

    procedure
    pljoin(x1 : PLFLT; y1 : PLFLT; x2 : PLFLT; y2 : PLFLT);
    pragma Import(C, pljoin, "c_pljoin");


    -- Simple routine for labelling graphs.

    procedure
    pllab(xlabel : char_array; ylabel : char_array; tlabel : char_array);
    pragma Import(C, pllab, "c_pllab");


    -- Legend and colorbar position options are chosen by adding their values.
    subtype Legend_Colorbar_Position_Type is Integer range
        1 .. 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128;

    --Flags used for position argument of both pllegend and plcolorbar
    PL_Position_Left     : constant Legend_Colorbar_Position_Type := 1;
    PL_Position_Right    : constant Legend_Colorbar_Position_Type := 2;
    PL_Position_Top      : constant Legend_Colorbar_Position_Type := 4;
    PL_Position_Bottom   : constant Legend_Colorbar_Position_Type := 8;
    PL_Position_Inside   : constant Legend_Colorbar_Position_Type := 16;
    PL_Position_Outside  : constant Legend_Colorbar_Position_Type := 32;
    PL_Position_Viewport : constant Legend_Colorbar_Position_Type := 64;
    PL_Position_Subpage  : constant Legend_Colorbar_Position_Type := 128;

    -- Legend characteristics are chosen by adding flag values.
    subtype Legend_Flag_Type is Integer range 1 .. 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128;

    -- Flags for pllegend
    PL_Legend_None         : constant Legend_Flag_Type := 1;
    PL_Legend_Color_Box    : constant Legend_Flag_Type := 2;
    PL_Legend_Line         : constant Legend_Flag_Type := 4;
    PL_Legend_Symbol       : constant Legend_Flag_Type := 8;
    PL_Legend_Text_Left    : constant Legend_Flag_Type := 16;
    PL_Legend_Background   : constant Legend_Flag_Type := 32;
    PL_Legend_Bounding_Box : constant Legend_Flag_Type := 64;
    PL_Legend_Row_Major    : constant Legend_Flag_Type := 128;

    -- Colorbar characteristics are chosen by adding flag values.
    subtype Colorbar_Flag_Type is Integer range 1 .. 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 +
        512 + 1024 + 2048 + 4096 + 8192 + 16384 + 32768 + 65536;

    -- Flags for plcolorbar
    PL_Colorbar_Label_Left    : constant Colorbar_Flag_Type := 1;
    PL_Colorbar_Label_Right   : constant Colorbar_Flag_Type := 2;
    PL_Colorbar_Label_Top     : constant Colorbar_Flag_Type := 4;
    PL_Colorbar_Label_Bottom  : constant Colorbar_Flag_Type := 8;
    PL_Colorbar_Image         : constant Colorbar_Flag_Type := 16;
    PL_Colorbar_Shade         : constant Colorbar_Flag_Type := 32;
    PL_Colorbar_Gradient      : constant Colorbar_Flag_Type := 64;
    PL_Colorbar_Cap_Low       : constant Colorbar_Flag_Type := 256;
    PL_Colorbar_Cap_High      : constant Colorbar_Flag_Type := 512;
    PL_Colorbar_Shade_Label   : constant Colorbar_Flag_Type := 1024;
    PL_Colorbar_Orient_Right  : constant Colorbar_Flag_Type := 2048;
    PL_Colorbar_Orient_Top    : constant Colorbar_Flag_Type := 4096;
    PL_Colorbar_Orient_Left   : constant Colorbar_Flag_Type := 8192;
    PL_Colorbar_Orient_Bottom : constant Colorbar_Flag_Type := 16384;
    PL_Colorbar_Background    : constant Colorbar_Flag_Type := 32768;
    PL_Colorbar_Bounding_Box  : constant Colorbar_Flag_Type := 65536;

     -- Flags for drawing mode
    PL_Drawmode_Unknown : constant Integer := 0;
    PL_Drawmode_Default : constant Integer := 1;
    PL_Drawmode_Replace : constant Integer := 2;
    PL_Drawmode_Xor     : constant Integer := 4;

    -- Routine for drawing discrete line, symbol, or cmap0 legends
    procedure
    pllegend
       (p_legend_width : out PLFLT; p_legend_height : out PLFLT;
        opt : PLINT; plposition : PLINT; x : PLFLT; y : PLFLT; plot_width : PLFLT;
        bg_color : PLINT; bb_color : PLINT; bb_style : PLINT;
        nrow : PLINT; ncolumn : PLINT;
        nlegend : PLINT; opt_array : PL_Integer_Array;
        text_offset : PLFLT; text_scale : PLFLT; text_spacing : PLFLT;
        text_justification : PLFLT;
        text_colors : PL_Integer_Array; -- fixme these are actually colors; map to them in Ada
        text : PL_Legend_String_Array;
        box_colors : PL_Integer_Array; box_patterns : PL_Integer_Array;
        box_scales : PL_Float_Array; box_line_widths : PL_Float_Array;
        line_colors : PL_Integer_Array; line_styles : PL_Integer_Array;
        line_widths : PL_Float_Array;
        symbol_colors : PL_Integer_Array; symbol_scales : PL_Float_Array;
        symbol_numbers : PL_Integer_Array;
        symbols : PL_Legend_String_Array);
    pragma Import(C, pllegend, "c_pllegend");


    -- Routine for drawing continuous colour legends
    procedure
    plcolorbar
       (p_colorbar_width : out PLFLT; p_colorbar_height : out PLFLT;
        opt : PLINT; position : PLINT; x : PLFLT; y : PLFLT;
        x_length : PLFLT; y_length : PLFLT;
        bg_color : PLINT; bb_color : PLINT; bb_style : PLINT;
        low_cap_color : PLFLT; high_cap_color : PLFLT;
        cont_color : PLINT; cont_width : PLFLT;
        n_labels : PLINT; label_opts : PL_Integer_Array; labels : PL_Legend_String_Array;
        n_axes : PLINT; axis_opts : PL_Legend_String_Array;
        ticks : PL_Float_Array; sub_ticks : PL_Integer_Array;
        n_values : PL_Integer_Array; values : Long_Float_Pointer_Array);
    pragma Import(C, plcolorbar, "c_plcolorbar");


    -- Sets position of the light source

    procedure
    pllightsource(x : PLFLT; y : PLFLT; z : PLFLT);
    pragma Import(C, pllightsource, "c_pllightsource");


    -- Draws line segments connecting a series of points.

    procedure
    plline(n : PLINT; x : PL_Float_Array; y : PL_Float_Array);
    pragma Import(C, plline, "c_plline");


    -- Draws a line in 3 space.

    procedure
    plline3(n : PLINT; x : PL_Float_Array; y : PL_Float_Array; z : PL_Float_Array);
    pragma Import(C, plline3, "c_plline3");


    -- Set line style.

    procedure
    pllsty(lin : PLINT);
    pragma Import(C, pllsty, "c_pllsty");


    -- plot continental outline in world coordinates

    procedure
    plmap(mapform : Map_Form_Function_Pointer_Type; Map_Kind_C_String : char_array;
             minlong : PLFLT; maxlong : PLFLT; minlat : PLFLT; maxlat : PLFLT);
    pragma Import(C, plmap, "c_plmap");


    -- Plot map fills.

    procedure
    plmapfill(mapform : Map_Form_Function_Pointer_Type; Map_Kind_C_String : char_array;
        minx : PLFLT; maxx : PLFLT; miny : PLFLT; maxy : PLFLT;
        plotentries : PL_Integer_Array; nplotentries : PLINT);
    pragma Import(C, plmapfill, "c_plmapfill");


    -- Plot map fills: overload passes null pointer plotentries to emulate C, match documentaton.

    procedure
    plmapfill_Overload(mapform : Map_Form_Function_Pointer_Type; name : char_array;
        minx : PLFLT; maxx : PLFLT; miny : PLFLT; maxy : PLFLT;
        plotentries : PL_Pointer; nplotentries : PLINT);
    pragma Import(C, plmapfill_Overload, "c_plmapfill");

    -- Plot map outlines.

    procedure
    plmapline(mapform : Map_Form_Function_Pointer_Type; name : char_array;
        minx : PLFLT; maxx : PLFLT; miny : PLFLT; maxy : PLFLT;
        plotentries : PL_Integer_Array; nplotentries : PLINT);
    pragma Import(C, plmapline, "c_plmapline");


    -- Plot map outlines: overload passes null pointer plotentries to emulate C, match documentaton.

    procedure
    plmapline_Overload(mapform : Map_Form_Function_Pointer_Type; name : char_array;
        minx : PLFLT; maxx : PLFLT; miny : PLFLT; maxy : PLFLT;
        plotentries : PL_Pointer; nplotentries : PLINT);
    pragma Import(C, plmapline_Overload, "c_plmapline");


    -- Plot map points.

    procedure
    plmapstring(mapform : Map_Form_Function_Pointer_Type; name : char_array;
        minx : PLFLT; maxx : PLFLT; miny : PLFLT; maxy : PLFLT;
                   plotentries : PL_Integer_Array; nplotentries : PLINT);
    pragma Import(C, plmapstring, "c_plmapstring");


    -- Plot map text.

    procedure
    plmaptex(mapform : Map_Form_Function_Pointer_Type; name : char_array;
        dx : PLFLT; dy : PLFLT; just : PLFLT; text : char_array;
        minx : PLFLT; maxx : PLFLT; miny : PLFLT; maxy : PLFLT;
        plotentry : PLINT);
    pragma Import(C, plmaptex, "c_plmaptex");


    -- Plot the latitudes and longitudes on the background.

    procedure
    plmeridians(mapform : Map_Form_Function_Pointer_Type;
                   dlong : PLFLT; dlat : PLFLT;
                   minlong : PLFLT; maxlong : PLFLT; minlat : PLFLT; maxlat : PLFLT);
    pragma Import(C, plmeridians, "c_plmeridians");


    -- Plots a mesh representation of the function z(x, y).

    procedure
    plmesh(x : PL_Float_Array; y : PL_Float_Array; z : Long_Float_Pointer_Array; nx : PLINT; ny : PLINT; opt : PLINT);
    pragma Import(C, plmesh, "c_plmesh");


    -- Plots a mesh representation of the function z(x, y) with contour

    procedure
    plmeshc(x : PL_Float_Array; y : PL_Float_Array; z : Long_Float_Pointer_Array; nx : PLINT; ny : PLINT; opt : PLINT;
          clevel : PL_Float_Array; nlevel : PLINT);
    pragma Import(C, plmeshc, "c_plmeshc");


    -- Creates a new stream and makes it the default.

    procedure
    plmkstrm(p_strm : out PLINT);
    pragma Import(C, plmkstrm, "c_plmkstrm");


    -- Prints out "text" at specified position relative to viewport

    procedure
    plmtex(side : char_array; disp : PLFLT; pos : PLFLT; just : PLFLT;
         text : char_array);
    pragma Import(C, plmtex, "c_plmtex");


    -- Prints out "text" at specified position relative to viewport (3D)

    procedure
    plmtex3(side : char_array; disp, pos, just : PLFLT; text : char_array);
    pragma Import(C, plmtex3, "c_plmtex3");


    -- Plots a 3-d representation of the function z(x, y).

    procedure
    plot3d(x : PL_Float_Array; y : PL_Float_Array; z : Long_Float_Pointer_Array;
         nx : PLINT; ny : PLINT; opt : PLINT; side : PLINT);
    pragma Import(C, plot3d, "c_plot3d");


    -- Plots a 3-d representation of the function z(x, y) with contour.

    procedure
    plot3dc(x : PL_Float_Array; y : PL_Float_Array; z : Long_Float_Pointer_Array;
         nx : PLINT; ny : PLINT; opt : PLINT;
         clevel : PL_Float_Array; nlevel : PLINT);
    pragma Import(C, plot3dc, "c_plot3dc");


    -- Plots a 3-d representation of the function z(x, y) with contour and
    -- y index limits.

    procedure
    plot3dcl(x : PL_Float_Array; y : PL_Float_Array; z : Long_Float_Pointer_Array;
         nx : PLINT; ny : PLINT; opt : PLINT;
         clevel : PL_Float_Array; nlevel : PLINT;
         ixstart : PLINT; ixn : PLINT; indexymin : PL_Integer_Array; indexymax : PL_Integer_Array);
    pragma Import(C, plot3dcl, "c_plot3dcl");


    -- definitions for the opt argument in plot3dc() and plsurf3d()
    -- DRAW_LINEX *must* be 1 and DRAW_LINEY *must* be 2, because of legacy code!
    -- fix this Why is there no type declared for these?
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

       --  valid options for plot3dc():
       --
       --  DRAW_SIDES, BASE_CONT, TOP_CONT (not yet),
       --  MAG_COLOR, DRAW_LINEX, DRAW_LINEY, DRAW_LINEXY.
       --
       --  valid options for plsurf3d():
       --
       --  MAG_COLOR, BASE_CONT, SURF_CONT, FACETED, DRAW_SIDES.


    -- Set fill pattern directly.

    procedure
    plpat(nlin : PLINT; inc : PL_Integer_Array; del : PL_Integer_Array);
    pragma Import(C, plpat, "c_plpat");


    -- Draw a line connecting two points, accounting for coordinate transforms.

    procedure
    plpath(n : PLINT; x1 : PLFLT; y1 : PLFLT; x2 : PLFLT; y2 : PLFLT);
    pragma Import(C, plpath, "c_plpath");


    -- Plots array y against x for n points using ASCII code "code".

    procedure
    plpoin(n : PLINT; x : PL_Float_Array; y : PL_Float_Array; code : PLINT);
    pragma Import(C, plpoin, "c_plpoin");


    -- Draws a series of points in 3 space.

    procedure
    plpoin3(n : PLINT; x : PL_Float_Array; y : PL_Float_Array; z : PL_Float_Array; code : PLINT);
    pragma Import(C, plpoin3, "c_plpoin3");


    -- Draws a polygon in 3 space.

    procedure
    plpoly3(n : PLINT; x : PL_Float_Array; y : PL_Float_Array; z : PL_Float_Array; draw : PL_Bool_Array; ifcc : PLBOOL);
    pragma Import(C, plpoly3, "c_plpoly3");


    -- Set the floating point precision (in number of places) in numeric labels.

    procedure
    plprec(setp : PLINT; prec : PLINT);
    pragma Import(C, plprec, "c_plprec");


    -- Set fill pattern, using one of the predefined patterns.

    procedure
    plpsty(patt : PLINT);
    pragma Import(C, plpsty, "c_plpsty");


    -- Prints out "text" at world cooordinate (x,y).

    procedure
    plptex(x : PLFLT; y : PLFLT; dx : PLFLT; dy : PLFLT; just : PLFLT; text : char_array);
    pragma Import(C, plptex, "c_plptex");


    -- Prints out "text" at world cooordinate (x,y,z).

    procedure
    plptex3(wx, wy, wz, dx, dy, dz, sx, sy, sz, just : PLFLT; text : char_array);
    pragma Import(C, plptex3, "c_plptex3");


    -- Random number generator based on Mersenne Twister.
    -- Obtain real random number in range [0,1].

    function
    plrandd return PLFLT;
    pragma Import(C, plrandd, "c_plrandd");


    -- Replays contents of plot buffer to current device/file.

    procedure
    plreplot;
    pragma Import(C, plreplot, "c_plreplot");


    -- Functions for converting between HLS and RGB color space

    procedure
    plrgbhls(r : PLFLT; g : PLFLT; b : PLFLT; p_h : out PLFLT; p_l : out PLFLT; p_s : out PLFLT);
    pragma Import(C, plrgbhls, "c_plrgbhls");


    -- Set character height.

    procedure
    plschr(def : PLFLT; scale : PLFLT);
    pragma Import(C, plschr, "c_plschr");


    -- Set color map 0 colors by 8 bit RGB values

    procedure
    plscmap0(r : PL_Integer_Array; g : PL_Integer_Array; b : PL_Integer_Array; ncol0 : PLINT);
    pragma Import(C, plscmap0, "c_plscmap0");


    -- Set color map 0 colors by 8 bit RGB values and alpha values

    procedure
    plscmap0a(r, g, b : PL_Integer_Array; a : PL_Float_Array; ncol0 : PLINT);
    pragma Import(C, plscmap0a, "c_plscmap0a");


    -- Set number of colors in cmap 0

    procedure
    plscmap0n(ncol0 : PLINT);
    pragma Import(C, plscmap0n, "c_plscmap0n");


    -- Set color map 1 colors by 8 bit RGB values

    procedure
    plscmap1(r : PL_Integer_Array; g : PL_Integer_Array; b : PL_Integer_Array; ncol1 : PLINT);
    pragma Import(C, plscmap1, "c_plscmap1");


    -- Set color map 1 colors by 8 bit RGB and alpha values

    procedure
    plscmap1a(r, g, b : PL_Integer_Array; a : PL_Float_Array; ncol1 : PLINT);
    pragma Import(C, plscmap1a, "c_plscmap1a");


    -- Set color map 1 colors using a piece-wise linear relationship between
    -- intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.

    procedure
    plscmap1l(itype : PLINT; npts : PLINT; intensity : PL_Float_Array;
        coord1 : PL_Float_Array; coord2 : PL_Float_Array; coord3 : PL_Float_Array;
        alt_hue_path : PL_Bool_Array);
    pragma Import(C, plscmap1l, "c_plscmap1l");


    -- Set color map 1 colors using a piece-wise linear relationship between
    -- intensity [0,1] (cmap 1 index) and position in HLS or RGB color space.
    -- Will also linear interpolate alpha values.

    procedure
    plscmap1la(itype : PLINT; npts : PLINT;
        intensity, coord1, coord2, coord3, a : PL_Float_Array;
        alt_hue_path : PL_Bool_Array);
    pragma Import(C, plscmap1la, "c_plscmap1la");


    -- Set number of colors in cmap 1

    procedure
    plscmap1n(ncol1 : PLINT);
    pragma Import(C, plscmap1n, "c_plscmap1n");


    -- Set the color map 1 range used in continuous plots.

    procedure
    plscmap1_range(min_color : PLFLT; max_color : PLFLT);
    pragma Import(C, plscmap1_range, "c_plscmap1_range");


    -- Get the color map 1 range used in continuous plots

    procedure
    plgcmap1_range(min_color, max_color : out PLFLT);
    pragma Import(C, plgcmap1_range, "c_plgcmap1_range");


    -- Set a given color from color map 0 by 8 bit RGB value

    procedure
    plscol0(icol0 : PLINT; r : PLINT; g : PLINT; b : PLINT);
    pragma Import(C, plscol0, "c_plscol0");


    -- Set a given color from color map 0 by 8 bit RGB value and alpha value

    procedure
    plscol0a(icol0 : PLINT; r, g, b : PLINT; a : PLFLT);
    pragma Import(C, plscol0a, "c_plscol0a");


    -- Set the background color by 8 bit RGB value

    procedure
    plscolbg(r : PLINT; g : PLINT; b : PLINT);
    pragma Import(C, plscolbg, "c_plscolbg");


    -- Set the background color by 8 bit RGB value and alpha value

    procedure
    plscolbga(r, g, b : PLINT; a : PLFLT);
    pragma Import(C, plscolbga, "c_plscolbga");


    -- Used to globally turn color output on/off

    procedure
    plscolor(color : PLINT);
    pragma Import(C, plscolor, "c_plscolor");


    -- Set the compression level

    procedure
    plscompression(compression : PLINT);
    pragma Import(C, plscompression, "c_plscompression");


    -- Set the device (keyword) name

    procedure
    plsdev(devname : char_array);
    pragma Import(C, plsdev, "c_plsdev");


    -- Set window into device space using margin, aspect ratio, and
    -- justification

    procedure
    plsdidev(mar : PLFLT; aspect : PLFLT; jx : PLFLT; jy : PLFLT);
    pragma Import(C, plsdidev, "c_plsdidev");


    -- Set up transformation from metafile coordinates.

    procedure
    plsdimap(dimxmin : PLINT; dimxmax : PLINT; dimymin : PLINT; dimymax : PLINT;
           dimxpmm : PLFLT; dimypmm : PLFLT);
    pragma Import(C, plsdimap, "c_plsdimap");


    -- Set plot orientation, specifying rotation in units of pi/2.

    procedure
    plsdiori(rot : PLFLT);
    pragma Import(C, plsdiori, "c_plsdiori");


    -- Set window into plot space

    procedure
    plsdiplt(xmin : PLFLT; ymin : PLFLT; xmax : PLFLT; ymax : PLFLT);
    pragma Import(C, plsdiplt, "c_plsdiplt");


    -- Set window into plot space incrementally (zoom)

    procedure
    plsdiplz(xmin : PLFLT; ymin : PLFLT; xmax : PLFLT; ymax : PLFLT);
    pragma Import(C, plsdiplz, "c_plsdiplz");


    -- Set seed for internal random number generator

    procedure
    plseed(s : unsigned);
    pragma Import(C, plseed, "c_plseed");


    -- Set the escape character for text strings.

    procedure
    plsesc(esc : char);
    pragma Import(C, plsesc, "c_plsesc");


    -- Set family file parameters

    procedure
    plsfam(fam : PLINT; num : PLINT; bmax : PLINT);
    pragma Import(C, plsfam, "c_plsfam");


    -- Set FCI (font characterization integer)

    procedure
    plsfci(fci : PLUNICODE);
    pragma Import(C, plsfci, "c_plsfci");


    -- Set the font family, style and weight

    procedure
    plsfont(family, style, weight : PLINT);
    pragma Import(C, plsfont, "c_plsfont");


    -- Set the output file name.

    procedure
    plsfnam(fnam : char_array);
    pragma Import(C, plsfnam, "c_plsfnam");


    -- Shade region.

    procedure
    plshade(a : Long_Float_Pointer_Array; nx : PLINT; ny : PLINT; defined : Mask_Function_Pointer_Type;
          left : PLFLT; right : PLFLT; bottom : PLFLT; top : PLFLT;
          shade_min : PLFLT; shade_max : PLFLT;
          sh_cmap : PLINT; sh_color : PLFLT; sh_width : PLFLT;
          min_color : PLINT; min_width : PLFLT;
          max_color : PLINT; max_width : PLFLT;
          fill : Fill_Procedure_Pointer_Type; rectangular : PLINT;
          pltr : Transformation_Procedure_Pointer_Type; pltr_data : PL_Pointer);
    pragma Import(C, plshade, "c_plshade");


    procedure
    plshades(z : Long_Float_Pointer_Array; nx : PLINT; ny : PLINT; defined : Mask_Function_Pointer_Type;
          xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT;
          clevel : PL_Float_Array; nlevel : PLINT; fill_width : PLFLT;
          cont_color : PLINT; cont_width : PLFLT;
          fill : Fill_Procedure_Pointer_Type; rectangular : PLINT;
          pltr : Transformation_Procedure_Pointer_Type; pltr_data : PL_Pointer);
    pragma Import(C, plshades, "c_plshades");


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
--    pragma Import(C, plfshade, "plfshade");


    -- Setup a user-provided custom labeling function.

    procedure
    plslabelfunc(label_func : Custom_Label_Procedure_Pointer_Type; label_data : PL_Pointer);
    pragma Import(C, plslabelfunc, "c_plslabelfunc");


    -- Set up lengths of major tick marks.

    procedure
    plsmaj(def : PLFLT; scale : PLFLT);
    pragma Import(C, plsmaj, "c_plsmaj");


    -- Set the memory area to be plotted (with the 'mem' driver)

    procedure
    plsmem(maxx : PLINT; maxy : PLINT; plotmem : PL_Pointer);
    pragma Import(C, plsmem, "c_plsmem");


    -- Set up lengths of minor tick marks.

    procedure
    plsmin(def : PLFLT; scale : PLFLT);
    pragma Import(C, plsmin, "c_plsmin");


    -- Set orientation.  Must be done before calling plinit.

    procedure
    plsori(ori : PLINT);
    pragma Import(C, plsori, "c_plsori");


    -- Set output device parameters.  Usually ignored by the driver.

    procedure
    plspage(xp : PLFLT; yp : PLFLT; xleng : PLINT; yleng : PLINT;
          xoff : PLINT; yoff : PLINT);
    pragma Import(C, plspage, "c_plspage");


    -- Set the colors for color table 0 from a cmap0 file.

    procedure
    plspal0(filename : char_array);
    pragma Import(C, plspal0, "c_plspal0");


    -- Set the colors for color table 1 from a cmap1 file.
    procedure
    plspal1(filename : char_array; interpolate : PLBOOL);
    pragma Import(C, plspal1, "c_plspal1");


    -- Set the pause (on end-of-page) status

    procedure
    plspause(pause : PLINT);
    pragma Import(C, plspause, "c_plspause");


    -- Set stream number.

    procedure
    plsstrm(strm : PLINT);
    pragma Import(C, plsstrm, "c_plsstrm");


    -- Set the number of subwindows in x and y

    procedure
    plssub(nx : PLINT; ny : PLINT);
    pragma Import(C, plssub, "c_plssub");


    -- Set symbol height.

    procedure
    plssym(def : PLFLT; scale : PLFLT);
    pragma Import(C, plssym, "c_plssym");


    -- Initialize PLplot, passing in the windows/page settings.

    procedure
    plstar(nx : PLINT; ny : PLINT);
    pragma Import(C, plstar, "c_plstar");


    -- Initialize PLplot, passing the device name and windows/page settings.

    procedure
    plstart(devname : char_array; nx : PLINT; ny : PLINT);
    pragma Import(C, plstart, "c_plstart");


    --Set the coordinate transform

    procedure
    plstransform(coordinate_transform : Coordinate_Transform_Procedure_Pointer_Type;
        coordinate_transform_data : PL_Pointer);
    pragma Import(C, plstransform, "c_plstransform");


    -- Prints out the same string repeatedly at the n points in world
    -- coordinates given by the x and y arrays.  Supersedes plpoin and
    -- plsymbol for the case where text refers to a unicode glyph either
    -- directly as UTF-8 or indirectly via the standard text escape
    -- sequences allowed for PLplot input strings.

    procedure
    plstring(n : PLINT; x : PL_Float_Array; y : PL_Float_Array; cstring : char_array);
    pragma Import(C, plstring, "c_plstring");


    -- Prints out the same string repeatedly at the n points in world
    -- coordinates given by the x, y, and z arrays.  Supersedes plpoin3
    -- for the case where text refers to a unicode glyph either directly
    -- as UTF-8 or indirectly via the standard text escape sequences
    -- allowed for PLplot input strings.

    procedure
    plstring3(n : PLINT; x : PL_Float_Array; y : PL_Float_Array;
        z : PL_Float_Array; cstring : char_array);
    pragma Import(C, plstring3, "c_plstring3");


    -- Add a point to a stripchart.

    procedure
    plstripa(id : PLINT; pen : PLINT; x : PLFLT; y : PLFLT);
    pragma Import(C, plstripa, "c_plstripa");


    -- Create 1d stripchart

    procedure
    plstripc(id : out PLINT; xspec : char_array; yspec : char_array;
        xmin : PLFLT; xmax : PLFLT; xjump : PLFLT; ymin : PLFLT; ymax : PLFLT;
        xlpos : PLFLT; ylpos : PLFLT;
        y_ascl : PLBOOL; acc : PLBOOL;
        colbox : PLINT; collab : PLINT;
        colline : PL_Integer_Array; styline : PL_Integer_Array; legline : PL_Stripchart_String_Array;
        labx : char_array; laby : char_array; labtop : char_array);
    pragma Import(C, plstripc, "c_plstripc");


    -- Deletes and releases memory used by a stripchart.

    procedure
    plstripd(id : PLINT);
    pragma Import(C, plstripd, "c_plstripd");


    -- plots a 2d image (or a matrix too large for plshade() )

    procedure
    plimagefr(data : Long_Float_Pointer_Array; nx : PLINT; ny : PLINT;
        xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT; zmin : PLFLT; zmax : PLFLT;
        valuemin :PLFLT; valuemax : PLFLT;
        pltr : Transformation_Procedure_Pointer_Type;
        pltr_data : PL_Pointer);
    pragma Import(C, plimagefr, "c_plimagefr");


      -- plots a 2d image (or a matrix too large for plshade() )

    procedure
    plimage(data : Long_Float_Pointer_Array; nx : PLINT; ny : PLINT;
        xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT; zmin : PLFLT; zmax : PLFLT;
        Dxmin : PLFLT; Dxmax : PLFLT; Dymin : PLFLT; Dymax : PLFLT);
    pragma Import(C, plimage, "c_plimage");


    -- Set up a new line style

    procedure
    plstyl(nms : PLINT; mark : PL_Integer_Array; space : PL_Integer_Array);
    pragma Import(C, plstyl, "c_plstyl");


--==================================================================================================
--==================================================================================================
-- Begin inserting definitions and functions for new 3D plotting capability. April 13, 2016



    -- type Function_To_Manipulate_Matrix_A_Type is
    --     access function (p : Real_Matrix; ix, iy : Integer)
    --         return Long_Float;
    -- pragma Convention(C, Function_To_Manipulate_Matrix_A_Type);
    --
    -- type Function_To_Manipulate_Matrix_B_Type is
    --     access function (p : in out Real_Matrix; ix, iy : Integer; z : Long_Float)
    --         return Long_Float;
    -- pragma Convention(C, Function_To_Manipulate_Matrix_B_Type);
    --
    -- type Function_To_Manipulate_Matrix_C_Type is
    --     access function (ix, iy : Integer; p : Real_Matrix) return Long_Float;-- Why not use A form?
    -- pragma Convention(C, Function_To_Manipulate_Matrix_C_Type);
    --
    -- type Procedure_To_Manipulate_Matrix_Type is
    --     access procedure (p : Real_Matrix; nx, ny : Integer; zmin, zmax : out Long_Float);
    -- pragma Convention(C, Procedure_To_Manipulate_Matrix_Type);
    --
    -- type PLf2ops_t is
    --     record
    --         Get    : Function_To_Manipulate_Matrix_A_Type;
    --         Set    : Function_To_Manipulate_Matrix_B_Type;
    --         Add    : Function_To_Manipulate_Matrix_B_Type;
    --         Sub    : Function_To_Manipulate_Matrix_B_Type;
    --         Mul    : Function_To_Manipulate_Matrix_B_Type;
    --         Div    : Function_To_Manipulate_Matrix_B_Type;
    --         Is_Nan : Function_To_Manipulate_Matrix_A_Type;
    --         MinMax : Procedure_To_Manipulate_Matrix_Type;
    --         F2Eval : Function_To_Manipulate_Matrix_C_Type;
    --     end record;
    --
    -- function plf2ops_c_get(p : Real_Matrix; ix, iy : Integer) return Long_Float;
    -- pragma Import(C, plf2ops_c_get, "plf2ops_c_get");
    --
    -- function plf2ops_c_set(p : in out Real_Matrix; ix, iy : Integer; z : Long_Float) return Long_Float;
    -- pragma Import(C, plf2ops_c_set, "plf2ops_c_set");
    --
    -- function plf2ops_c_add(p : in out Real_Matrix; ix, iy : Integer; z : Long_Float) return Long_Float;
    -- pragma Import(C, plf2ops_c_add, "plf2ops_c_add");
    --
    -- function plf2ops_c_sub(p : in out Real_Matrix; ix, iy : Integer; z : Long_Float) return Long_Float;
    -- pragma Import(C, plf2ops_c_sub, "plf2ops_c_sub");
    --
    -- function plf2ops_c_mul(p : in out Real_Matrix; ix, iy : Integer; z : Long_Float) return Long_Float;
    -- pragma Import(C, plf2ops_c_mul, "plf2ops_c_mul");
    --
    -- function plf2ops_c_div(p : in out Real_Matrix; ix, iy : Integer; z : Long_Float) return Long_Float;
    -- pragma Import(C, plf2ops_c_div, "plf2ops_c_div");
    --
    -- function plf2ops_c_isnan(p : Real_Matrix; ix, iy : Integer) return Long_Float;
    -- pragma Import(C, plf2ops_c_isnan, "plf2ops_c_isnan");
    --
    -- procedure plf2ops_c_minmax(p : Real_Matrix; nx, ny : Integer; zmin, zmax : out Long_Float);
    -- pragma Import (C, plf2ops_c_minmax, "plf2ops_c_minmax");
    --
    -- function plf2ops_c_f2eval(ix, iy : Integer; p : Real_Matrix) return Long_Float;
    -- pragma Import(C, plf2ops_c_f2eval, "plf2ops_c_f2eval");
    --






--==================================================================================================
--==================================================================================================


    -- Plots the 3d surface representation of the function z(x, y).

    procedure
    plsurf3d(x : PL_Float_Array; y : PL_Float_Array; z : Long_Float_Pointer_Array;
        nx : PLINT; ny :  PLINT; opt : PLINT; clevel : PL_Float_Array; nlevel : PLINT);
    pragma Import(C, plsurf3d, "c_plsurf3d");


    -- -- Like plsurf3d, but uses an evaluator function to access z data from zp
    --
    -- procedure
    -- plfsurf3d(x : PL_Float_Array; y : PL_Float_Array; zops : PLf2ops_t;
    --     zp : Long_Float_Pointer_Array; nx : PLINT; ny : PLINT; opt : PLINT;
    --     clevel : PL_Float_Array; nlevel : PLINT);
    -- pragma Import(C, plfsurf3d, "plfsurf3d");


    -- Plots the 3-d surface representation of the function z(x, y) with limits on x and y.

    procedure
    plsurf3dl(x : PL_Float_Array; y : PL_Float_Array; z : Long_Float_Pointer_Array;
        nx : PLINT; ny : PLINT; opt : PLINT; clevel : PL_Float_Array; nlevel : PLINT;
        indexxmin : PLINT; indexxmax : PLINT;
        indexymin : PL_Integer_Array; indexymax : PL_Integer_Array);
    pragma Import(C, plsurf3dl, "c_plsurf3dl");


    -- Sets the edges of the viewport to the specified absolute coordinates

    procedure
    plsvpa(xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT);
    pragma Import(C, plsvpa, "c_plsvpa");


    -- Set x axis labeling parameters

    procedure
    plsxax(digmax : PLINT; field_digits : PLINT); -- "digits" changed to "field_digits".
    pragma Import(C, plsxax, "c_plsxax");


    -- Set inferior X window

    procedure
    plsxwin(window_id : PLINT);
    pragma Import(C, plsxwin, "plsxwin");

    -- Set y axis labeling parameters

    procedure
    plsyax(digmax : PLINT; field_digits : PLINT); -- "digits" changed to "field_digits".
    pragma Import(C, plsyax, "c_plsyax");


    -- Plots array y against x for n points using Hershey symbol "code"

    procedure
    plsym(n : PLINT; x : PL_Float_Array; y : PL_Float_Array; code : PLINT);
    pragma Import(C, plsym, "c_plsym");


    -- Set z axis labeling parameters

    procedure
    plszax(digmax : PLINT; field_digits : PLINT); -- "digits" changed to "field_digits".
    pragma Import(C, plszax, "c_plszax");


    -- Switches to text screen.

    procedure
    pltext;
    pragma Import(C, pltext, "c_pltext");


    -- Set the format for date / time labels

    procedure
    pltimefmt(fmt : char_array);
    pragma Import(C, pltimefmt, "c_pltimefmt");


    -- Sets the edges of the viewport with the given aspect ratio, leaving
    -- room for labels.

    procedure
    plvasp(aspect : PLFLT);
    pragma Import(C, plvasp, "c_plvasp");


    -- Creates the largest viewport of the specified aspect ratio that fits
    -- within the specified normalized subpage coordinates.

    procedure
    plvpas(xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT; aspect : PLFLT);
    pragma Import(C, plvpas, "c_plvpas");


    -- Creates a viewport with the specified normalized subpage coordinates.

    procedure
    plvpor(xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT);
    pragma Import(C, plvpor, "c_plvpor");


    -- Defines a "standard" viewport with seven character heights for
    -- the left margin and four character heights everywhere else.

    procedure
    plvsta;
    pragma Import(C, plvsta, "c_plvsta");


    -- Set up a window for three-dimensional plotting.

    procedure
    plw3d(basex : PLFLT; basey : PLFLT; height : PLFLT; xmin0 : PLFLT;
        xmax0 : PLFLT; ymin0 : PLFLT; ymax0 : PLFLT; zmin0 : PLFLT;
        zmax0 : PLFLT; alt : PLFLT; az : PLFLT);
    pragma Import(C, plw3d, "c_plw3d");


    -- Set pen width.

    procedure
    plwidth(width : PLFLT);
    pragma Import(C, plwidth, "c_plwidth");


    -- Set up world coordinates of the viewport boundaries (2d plots).

    procedure
    plwind(xmin : PLFLT; xmax : PLFLT; ymin : PLFLT; ymax : PLFLT);
    pragma Import(C, plwind, "c_plwind");


    --  set xor mode; mode = 1-enter, 0-leave, status = 0 if not interactive device

    procedure
    plxormod(mode : PLINT; status : out PLINT);
    pragma Import(C, plxormod, "c_plxormod");


--------------------------------------------------------------------------------
--         Functions for use from C or C++ only                               --
--         (Not really ;).                                                    --
--------------------------------------------------------------------------------
-- THESE FUNCTIONS ^^^ ARE NOT IMPLEMENTED FOR THE ADA BINDING
-- EXCEPT FOR THE FOLLOWING.

    -- plparseopts here is an exact copy (exept for the name) of
    -- Parse_Command_Line_Arguments in the thick binding. The reason for
    -- departing from the usual method of simply pragma Import-ing as in
    -- most or all of the other interfaces to C is because of the need to
    -- figure out what the command lines arguments are by also pragma
    -- Import-ing Gnat_Argc and Gnat_Argv. A single-argument version is made
    -- at the request of the development team rather than the three-argument
    -- version of the documetation. The caller specifies only the parse mode.

    -- Process options list using current options info.
    procedure plparseopts(Mode : Parse_Mode_Type);


    -- This is a three-argument version of plparseopts as indicated in the
    -- documentation.

    -- Process options list using current options info.
    procedure plparseopts
       (Gnat_Argc : Integer;
        Gnat_Argv : System.Address;
        Mode      : Parse_Mode_Type);


    -- Process input strings, treating them as an option and argument pair.
    procedure
    plsetopt(opt, optarg : char_array);
    pragma Import(C, plsetopt, "c_plsetopt");


	-- Transformation routines

    -- pltr0, pltr1, and pltr2 had to be re-written in Ada in order to make the
    -- callback work while also passing the data structure along, e.g.
    -- pltr_data in the formal names below. The machinery surrounding this idea
    -- also allows for easy user-defined plot transformation subprograms to be
    -- written.

    -- Identity transformation. Re-write of pltr0 in plcont.c in Ada.
    procedure pltr0
       (x, y      : PLFLT;
        tx, ty    : out PLFLT;
        pltr_data : PLplot_thin.PL_Pointer);
    pragma Convention (Convention => C, Entity => pltr0);


    -- Re-write of pltr1 in Ada.
    procedure pltr1
       (x, y      : PLFLT;
        tx, ty    : out PLFLT;
        pltr_data : PLplot_thin.PL_Pointer);
    pragma Convention (Convention => C, Entity => pltr1);


    -- Re-write of pltr2 in Ada.
    -- Does linear interpolation from doubly dimensioned coord arrays
    -- (column dominant, as per normal C 2d arrays).
    procedure pltr2
       (x, y      : PLFLT;
        tx, ty    : out PLFLT;
        pltr_data : PLplot_thin.PL_Pointer);
    pragma Convention (Convention => C, Entity => pltr2);


    -- Wait for graphics input event and translate to world coordinates.
    procedure plGetCursor(gin : out PLGraphicsIn);
    pragma Import (C, plGetCursor, "plGetCursor");

end PLplot_Thin;
