// plplotcapi.i contains the common API (API used by the many computer
// language interfaces for the PLplot library) of PLplot in a form that
// is readily understood by swig.
//
//
//
// Copyright (C) 2002-2019 Alan W. Irwin
// Copyright (C) 2004-2005 Rafael Laboissiere
// Copyright (C) 2004-2013 Andrew Ross
// Copyright (C) 2009 Werner Smekal
// Copyright (C) 2009-2010 Hazen Babcock
// Copyright (C) 2011 Hezekiah M. Carty
//
//This file is part of PLplot.
//
//PLplot is free software; you can redistribute it and/or modify
//it under the terms of the GNU Library General Public License as published by
//the Free Software Foundation; version 2 of the License.
//
//PLplot is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Library General Public License for more details.
//
//You should have received a copy of the GNU Library General Public License
//along with the file PLplot; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
//

// For passing user data, as with X's XtPointer

typedef void*  PLPointer;

// The following numerical #defines should not be independently edited
// since they are generated from include/plplot.h by the
// check_plplotcapi_defines target.  If that target complains of diff
// issues (i.e., non whitespace differences between these next
// numerical #defines and what is generated, then in simple cases edit
// the following numerical #defines to resolve the issue (i.e., the
// check_plplotcapi_defines target runs without diff issues) or in
// complicated cases replace the entirety of the following numerical
// #defines with the contents of the file,
// bindings/swig-support/generated_plplotcapi_defines which is
// generated in the build tree as a result of running the
// check_plplotcapi_defines target.

// N.B. after such an update of this system should style this
// file with the
//
// scripts/style_source.sh --apply
//
// command since the uncrustify command that executes at the heart of that script
// introduces whitespace changes (for some unknown reason, perhaps the context
// of the #defines below is different than the original?) in the following
// compared to the original whitespace in include/plplot.h.

// N.B. for an overview of the following numerical #defines see the
// documentation in include/plplot.h.

#define PLESC_SET_RGB                    1  // obsolete
#define PLESC_ALLOC_NCOL                 2  // obsolete
#define PLESC_SET_LPB                    3  // obsolete
#define PLESC_EXPOSE                     4  // handle window expose
#define PLESC_RESIZE                     5  // handle window resize
#define PLESC_REDRAW                     6  // handle window redraw
#define PLESC_TEXT                       7  // switch to text screen
#define PLESC_GRAPH                      8  // switch to graphics screen
#define PLESC_FILL                       9  // fill polygon
#define PLESC_DI                         10 // handle DI command
#define PLESC_FLUSH                      11 // flush output
#define PLESC_EH                         12 // handle Window events
#define PLESC_GETC                       13 // get cursor position
#define PLESC_SWIN                       14 // set window parameters
#define PLESC_DOUBLEBUFFERING            15 // configure double buffering
#define PLESC_XORMOD                     16 // set xor mode
#define PLESC_SET_COMPRESSION            17 // AFR: set compression
#define PLESC_CLEAR                      18 // RL: clear graphics region
#define PLESC_DASH                       19 // RL: draw dashed line
#define PLESC_HAS_TEXT                   20 // driver draws text
#define PLESC_IMAGE                      21 // handle image
#define PLESC_IMAGEOPS                   22 // plimage related operations
#define PLESC_PL2DEVCOL                  23 // convert PLColor to device color
#define PLESC_DEV2PLCOL                  24 // convert device color to PLColor
#define PLESC_SETBGFG                    25 // set BG, FG colors
#define PLESC_DEVINIT                    26 // alternate device initialization
#define PLESC_GETBACKEND                 27 // get used backend of (wxWidgets) driver - no longer used
#define PLESC_BEGIN_TEXT                 28 // get ready to draw a line of text
#define PLESC_TEXT_CHAR                  29 // render a character of text
#define PLESC_CONTROL_CHAR               30 // handle a text control character (super/subscript, etc.)
#define PLESC_END_TEXT                   31 // finish a drawing a line of text
#define PLESC_START_RASTERIZE            32 // start rasterized rendering
#define PLESC_END_RASTERIZE              33 // end rasterized rendering
#define PLESC_ARC                        34 // render an arc
#define PLESC_GRADIENT                   35 // render a gradient
#define PLESC_MODESET                    36 // set drawing mode
#define PLESC_MODEGET                    37 // get drawing mode
#define PLESC_FIXASPECT                  38 // set or unset fixing the aspect ratio of the plot
#define PLESC_IMPORT_BUFFER              39 // set the contents of the buffer to a specified byte string
#define PLESC_APPEND_BUFFER              40 // append the given byte string to the buffer
#define PLESC_FLUSH_REMAINING_BUFFER     41 // flush the remaining buffer e.g. after new data was appended
#define PLTEXT_FONTCHANGE                0  // font change in the text stream
#define PLTEXT_SUPERSCRIPT               1  // superscript in the text stream
#define PLTEXT_SUBSCRIPT                 2  // subscript in the text stream
#define PLTEXT_BACKCHAR                  3  // back-char in the text stream
#define PLTEXT_OVERLINE                  4  // toggle overline in the text stream
#define PLTEXT_UNDERLINE                 5  // toggle underline in the text stream
#define ZEROW2B                          1
#define ZEROW2D                          2
#define ONEW2B                           3
#define ONEW2D                           4
#define PLSWIN_DEVICE                    1      // device coordinates
#define PLSWIN_WORLD                     2      // world coordinates
#define PL_X_AXIS                        1      // The x-axis
#define PL_Y_AXIS                        2      // The y-axis
#define PL_Z_AXIS                        3      // The z-axis
#define PL_OPT_ENABLED                   0x0001 // Obsolete
#define PL_OPT_ARG                       0x0002 // Option has an argument
#define PL_OPT_NODELETE                  0x0004 // Don't delete after processing
#define PL_OPT_INVISIBLE                 0x0008 // Make invisible
#define PL_OPT_DISABLED                  0x0010 // Processing is disabled
#define PL_OPT_FUNC                      0x0100 // Call handler function
#define PL_OPT_BOOL                      0x0200 // Set *var = 1
#define PL_OPT_INT                       0x0400 // Set *var = atoi(optarg)
#define PL_OPT_FLOAT                     0x0800 // Set *var = atof(optarg)
#define PL_OPT_STRING                    0x1000 // Set var = optarg
#define PL_PARSE_PARTIAL                 0x0000 // For backward compatibility
#define PL_PARSE_FULL                    0x0001 // Process fully & exit if error
#define PL_PARSE_QUIET                   0x0002 // Don't issue messages
#define PL_PARSE_NODELETE                0x0004 // Don't delete options after
#define PL_PARSE_SHOWALL                 0x0008 // Show invisible options
#define PL_PARSE_OVERRIDE                0x0010 // Obsolete
#define PL_PARSE_NOPROGRAM               0x0020 // Program name NOT in *argv[0]..
#define PL_PARSE_NODASH                  0x0040 // Set if leading dash NOT required
#define PL_PARSE_SKIP                    0x0080 // Skip over unrecognized args
#define PL_FCI_MARK                      0x80000000
#define PL_FCI_IMPOSSIBLE                0x00000000
#define PL_FCI_HEXDIGIT_MASK             0xf
#define PL_FCI_HEXPOWER_MASK             0x7
#define PL_FCI_HEXPOWER_IMPOSSIBLE       0xf
#define PL_FCI_FAMILY                    0x0
#define PL_FCI_STYLE                     0x1
#define PL_FCI_WEIGHT                    0x2
#define PL_FCI_SANS                      0x0
#define PL_FCI_SERIF                     0x1
#define PL_FCI_MONO                      0x2
#define PL_FCI_SCRIPT                    0x3
#define PL_FCI_SYMBOL                    0x4
#define PL_FCI_UPRIGHT                   0x0
#define PL_FCI_ITALIC                    0x1
#define PL_FCI_OBLIQUE                   0x2
#define PL_FCI_MEDIUM                    0x0
#define PL_FCI_BOLD                      0x1
#define PL_MAXKEY                        16
#define PL_MASK_SHIFT                    0x1    // ( 1 << 0 )
#define PL_MASK_CAPS                     0x2    // ( 1 << 1 )
#define PL_MASK_CONTROL                  0x4    // ( 1 << 2 )
#define PL_MASK_ALT                      0x8    // ( 1 << 3 )
#define PL_MASK_NUM                      0x10   // ( 1 << 4 )
#define PL_MASK_ALTGR                    0x20   //  ( 1 << 5 )
#define PL_MASK_WIN                      0x40   // ( 1 << 6 )
#define PL_MASK_SCROLL                   0x80   // ( 1 << 7 )
#define PL_MASK_BUTTON1                  0x100  // ( 1 << 8 )
#define PL_MASK_BUTTON2                  0x200  // ( 1 << 9 )
#define PL_MASK_BUTTON3                  0x400  // ( 1 << 10 )
#define PL_MASK_BUTTON4                  0x800  // ( 1 << 11 )
#define PL_MASK_BUTTON5                  0x1000 // ( 1 << 12 )
#define PL_MAXWINDOWS                    64     // Max number of windows/page tracked
#define PL_NOTSET                        ( -42 )
#define PL_DEFAULT_NCOL0                 16
#define PL_DEFAULT_NCOL1                 128
#define MIN_PLINT_RGB                    0
#define MAX_PLINT_RGB                    255
#define MIN_PLFLT_CMAP1                  0.
#define MAX_PLFLT_CMAP1                  1.
#define MIN_PLFLT_ALPHA                  0.
#define MAX_PLFLT_ALPHA                  1.
#define PLESC_DOUBLEBUFFERING_ENABLE     1
#define PLESC_DOUBLEBUFFERING_DISABLE    2
#define PLESC_DOUBLEBUFFERING_QUERY      3
#define PL_BIN_DEFAULT                   0x0
#define PL_BIN_CENTRED                   0x1
#define PL_BIN_NOEXPAND                  0x2
#define PL_BIN_NOEMPTY                   0x4
#define GRID_CSA                         1 // Bivariate Cubic Spline approximation
#define GRID_DTLI                        2 // Delaunay Triangulation Linear Interpolation
#define GRID_NNI                         3 // Natural Neighbors Interpolation
#define GRID_NNIDW                       4 // Nearest Neighbors Inverse Distance Weighted
#define GRID_NNLI                        5 // Nearest Neighbors Linear Interpolation
#define GRID_NNAIDW                      6 // Nearest Neighbors Around Inverse Distance Weighted
#define PL_HIST_DEFAULT                  0x00
#define PL_HIST_NOSCALING                0x01
#define PL_HIST_IGNORE_OUTLIERS          0x02
#define PL_HIST_NOEXPAND                 0x08
#define PL_HIST_NOEMPTY                  0x10
#define PL_POSITION_NULL                 0x0
#define PL_POSITION_LEFT                 0x1
#define PL_POSITION_RIGHT                0x2
#define PL_POSITION_TOP                  0x4
#define PL_POSITION_BOTTOM               0x8
#define PL_POSITION_INSIDE               0x10
#define PL_POSITION_OUTSIDE              0x20
#define PL_POSITION_VIEWPORT             0x40
#define PL_POSITION_SUBPAGE              0x80
#define PL_LEGEND_NULL                   0x0
#define PL_LEGEND_NONE                   0x1
#define PL_LEGEND_COLOR_BOX              0x2
#define PL_LEGEND_LINE                   0x4
#define PL_LEGEND_SYMBOL                 0x8
#define PL_LEGEND_TEXT_LEFT              0x10
#define PL_LEGEND_BACKGROUND             0x20
#define PL_LEGEND_BOUNDING_BOX           0x40
#define PL_LEGEND_ROW_MAJOR              0x80
#define PL_COLORBAR_NULL                 0x0
#define PL_COLORBAR_LABEL_LEFT           0x1
#define PL_COLORBAR_LABEL_RIGHT          0x2
#define PL_COLORBAR_LABEL_TOP            0x4
#define PL_COLORBAR_LABEL_BOTTOM         0x8
#define PL_COLORBAR_IMAGE                0x10
#define PL_COLORBAR_SHADE                0x20
#define PL_COLORBAR_GRADIENT             0x40
#define PL_COLORBAR_CAP_NONE             0x80
#define PL_COLORBAR_CAP_LOW              0x100
#define PL_COLORBAR_CAP_HIGH             0x200
#define PL_COLORBAR_SHADE_LABEL          0x400
#define PL_COLORBAR_ORIENT_RIGHT         0x800
#define PL_COLORBAR_ORIENT_TOP           0x1000
#define PL_COLORBAR_ORIENT_LEFT          0x2000
#define PL_COLORBAR_ORIENT_BOTTOM        0x4000
#define PL_COLORBAR_BACKGROUND           0x8000
#define PL_COLORBAR_BOUNDING_BOX         0x10000
#define PL_DRAWMODE_UNKNOWN              0x0
#define PL_DRAWMODE_DEFAULT              0x1
#define PL_DRAWMODE_REPLACE              0x2
#define PL_DRAWMODE_XOR                  0x4
#define DRAW_LINEX                       0x001 // draw lines parallel to the X axis
#define DRAW_LINEY                       0x002 // draw lines parallel to the Y axis
#define DRAW_LINEXY                      0x003 // draw lines parallel to both the X and Y axis
#define MAG_COLOR                        0x004 // draw the mesh with a color dependent of the magnitude
#define BASE_CONT                        0x008 // draw contour plot at bottom xy plane
#define TOP_CONT                         0x010 // draw contour plot at top xy plane
#define SURF_CONT                        0x020 // draw contour plot at surface
#define DRAW_SIDES                       0x040 // draw sides
#define FACETED                          0x080 // draw outline for each square that makes up the surface
#define MESH                             0x100 // draw mesh

// End of data that is derived from numerical #defines in include/plplot.h

//--------------------------------------------------------------------------
// Complex data types and other good stuff
//--------------------------------------------------------------------------

// Structure for describing the plot window
typedef struct
{
    int          type;              // of event (CURRENTLY UNUSED)
    unsigned int state;             // key or button mask
    unsigned int keysym;            // key selected
    unsigned int button;            // mouse button selected
    PLINT        subwindow;         // subwindow (alias subpage, alias subplot) number
    char         string[PL_MAXKEY]; // translated string
    int          pX, pY;            // absolute device coordinates of pointer
    PLFLT        dX, dY;            // relative device coordinates of pointer
    PLFLT        wX, wY;            // world coordinates of pointer
} PLGraphicsIn;


#ifdef SWIG_PYTHON
#define SWIG_OBJECT_DATA        PYOBJECT_DATA
#define SWIG_OBJECT_DATA_img    PYOBJECT_DATA_img
#define pltr_img                pltr
#else
#define SWIG_OBJECT_DATA        OBJECT_DATA
#define SWIG_OBJECT_DATA_img    OBJECT_DATA_img
#endif

#ifdef SWIG_PYTHON

// Non-common API that are included here because they traditionally
// were part of plmodule.c.

void
plsxwin( PLINT window_id );

#endif // SWIG_PYTHON

// Complete list of common API (has "c_" suffix version defined in plplot.h)

void
pl_setcontlabelformat( PLINT lexp, PLINT sigdig );

void
pl_setcontlabelparam( PLFLT offset, PLFLT size, PLFLT spacing, PLINT active );

void
pladv( PLINT page );

void
plarc( PLFLT x, PLFLT y, PLFLT a, PLFLT b, PLFLT angle1, PLFLT angle2,
       PLFLT rotate, PLBOOL fill );

void
plaxes( PLFLT x0, PLFLT y0, const char *xopt, PLFLT xtick, PLINT nxsub,
        const char *yopt, PLFLT ytick, PLINT nysub );

void
plbin( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, PLINT center );

void
plbtime( PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT, PLFLT *OUTPUT, PLFLT ctime );

void
plbop( void );

void
plbox( const char *xopt, PLFLT xtick, PLINT nxsub,
       const char *yopt, PLFLT ytick, PLINT nysub );

void
plbox3( const char *xopt, const char *xlabel, PLFLT xtick, PLINT nsubx,
        const char *yopt, const char *ylabel, PLFLT ytick, PLINT nsuby,
        const char *zopt, const char *zlabel, PLFLT ztick, PLINT nsubz );

void
plcalc_world( PLFLT rx, PLFLT ry, PLFLT *OUTPUT, PLFLT *OUTPUT, PLINT *OUTPUT );

void
plclear( void );

void
plcol0( PLINT icol0 );

void
plcol1( PLFLT col1 );

void
plconfigtime( PLFLT scale, PLFLT offset1, PLFLT offset2, PLINT ccontrol, PLBOOL ifbtime_offset, PLINT year, PLINT month, PLINT day, PLINT hour, PLINT min, PLFLT sec );

void
plcont( const PLFLT **Matrix, PLINT nx, PLINT ny, PLINT kx, PLINT lx,
        PLINT ky, PLINT ly, const PLFLT *Array, PLINT n,
        pltr_func pltr,
        PLPointer SWIG_OBJECT_DATA );


void
plctime( PLINT year, PLINT month, PLINT day, PLINT hour, PLINT min, PLFLT sec, PLFLT *OUTPUT );

void
plcpstrm( PLINT iplsr, PLBOOL flags );

void
plend( void );

void
plend1( void );

void
plenv( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
       PLINT just, PLINT axis );

void
plenv0( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
        PLINT just, PLINT axis );

void
pleop( void );

void
plerrx( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, const PLFLT *ArrayCk );

void
plerry( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, const PLFLT *ArrayCk );

void
plfamadv( void );

void
plfill( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk );

void
plfill3( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, const PLFLT *ArrayCk );

void
plgradient( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, PLFLT angle );

void
plflush( void );

void
plfont( PLINT ifont );

void
plfontld( PLINT fnt );

void
plgchr( PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plgcol0( PLINT icol0, PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT );

void
plgcol0a( PLINT icol0, PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT, PLFLT *OUTPUT );

void
plgcolbg( PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT );

void
plgcolbga( PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT, PLFLT *OUTPUT );

void
plgcompression( PLINT *OUTPUT );

void
plgdev( char *OUTPUT );

void
plgdidev( PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plgdiori( PLFLT *OUTPUT );

void
plgdiplt( PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plgfam( PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT );

void
plgfci( PLUNICODE *OUTPUT );

void
plgfnam( char *OUTPUT );

void
plgfont( PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT );

void
plglevel( PLINT *OUTPUT );

void
plgpage( PLFLT *OUTPUT, PLFLT *OUTPUT,
         PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT, PLINT *OUTPUT );

void
plgra( void );

void
plgriddata( const PLFLT *Array, const PLFLT *ArrayCk, const PLFLT *ArrayCk, PLINT n,
            const PLFLT *ArrayX, PLINT nx, const PLFLT *ArrayY, PLINT ny,
            PLFLT **OutMatrixCk, PLINT type, PLFLT data );


void
plgspa( PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plgstrm( PLINT *OUTPUT );

void
plgver( char *OUTPUT );

void
plgvpd( PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plgvpw( PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plgxax( PLINT *OUTPUT, PLINT *OUTPUT );

void
plgyax( PLINT *OUTPUT, PLINT *OUTPUT );

void
plgzax( PLINT *OUTPUT, PLINT *OUTPUT );

void
plhist( PLINT n, const PLFLT *Array, PLFLT datmin, PLFLT datmax,
        PLINT nbin, PLINT oldwin );

void
plhlsrgb( PLFLT h, PLFLT l, PLFLT s, PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plinit( void );

void
pljoin( PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2 );

void
pllab( const char *xlabel, const char *ylabel, const char *tlabel );

void
pllegend( PLFLT *OUTPUT, PLFLT *OUTPUT,
          PLINT opt, PLINT position, PLFLT x, PLFLT y, PLFLT plot_width,
          PLINT bg_color, PLINT bb_color, PLINT bb_style,
          PLINT nrow, PLINT ncolumn,
          PLINT n, const PLINT *Array,
          PLFLT text_offset, PLFLT text_scale, PLFLT text_spacing,
          PLFLT text_justification,
          const PLINT *ArrayCk, const char **ArrayCk,
          const PLINT *ArrayCkNull, const PLINT *ArrayCkNull,
          const PLFLT *ArrayCkNull, const PLFLT *ArrayCkNull,
          const PLINT *ArrayCkNull, const PLINT *ArrayCkNull,
          const PLFLT *ArrayCkNull,
          const PLINT *ArrayCkNull, const PLFLT *ArrayCkNull,
          const PLINT *ArrayCkNull, const char **ArrayCk );

void
plcolorbar( PLFLT *OUTPUT, PLFLT *OUTPUT,
            PLINT opt, PLINT position, PLFLT x, PLFLT y,
            PLFLT x_length, PLFLT y_length,
            PLINT bg_color, PLINT bb_color, PLINT bb_style,
            PLFLT low_cap_color, PLFLT high_cap_color,
            PLINT cont_color, PLFLT cont_width,
            PLINT n, const PLINT *Array, const char **ArrayCk,
            PLINT n, const char **Array,
            const PLFLT *ArrayCk, const PLINT *ArrayCk,
            const PLINT *ArrayN, const PLFLT **MatrixCk );

void
pllightsource( PLFLT x, PLFLT y, PLFLT z );

void
plline( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk );

void
plline3( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, const PLFLT *ArrayCk );

void
pllsty( PLINT lin );

void
plmesh( const PLFLT *ArrayX, const PLFLT *ArrayY, const PLFLT **MatrixCk,
        PLINT nx, PLINT ny, PLINT opt );

void
plmeshc( const PLFLT *ArrayX, const PLFLT *ArrayY, const PLFLT **MatrixCk,
         PLINT nx, PLINT ny, PLINT opt, const PLFLT *Array, PLINT n );

void
plmkstrm( PLINT *OUTPUT );

void
plmtex( const char *side, PLFLT disp, PLFLT pos, PLFLT just,
        const char *text );

void
plmtex3( const char *side, PLFLT disp, PLFLT pos, PLFLT just,
         const char *text );

void
plot3d( const PLFLT *ArrayX, const PLFLT *ArrayY, const PLFLT **MatrixCk,
        PLINT nx, PLINT ny, PLINT opt, PLBOOL side );

void
plot3dc( const PLFLT *ArrayX, const PLFLT *ArrayY, const PLFLT **MatrixCk,
         PLINT nx, PLINT ny, PLINT opt, const PLFLT *Array, PLINT n );

void
plot3dcl( const PLFLT *ArrayX, const PLFLT *ArrayY, const PLFLT **MatrixCk,
          PLINT nx, PLINT ny, PLINT opt, const PLFLT *Array, PLINT n,
          PLINT ixstart, PLINT n, const PLINT *Array, const PLINT *ArrayCk );

void
plsurf3d( const PLFLT *ArrayX, const PLFLT *ArrayY, const PLFLT **MatrixCk,
          PLINT nx, PLINT ny, PLINT opt, const PLFLT *Array, PLINT n );

void
plsurf3dl( const PLFLT *ArrayX, const PLFLT *ArrayY, const PLFLT **MatrixCk,
           PLINT nx, PLINT ny, PLINT opt, const PLFLT *Array, PLINT n,
           PLINT ixstart, PLINT n, const PLINT *Array, const PLINT *ArrayCk );

PLINT
plparseopts( int *p_argc, char **argv, PLINT mode );

void
plpat( PLINT n, const PLINT *Array, const PLINT *ArrayCk );

void
plpath( PLINT n, PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2 );

void
plpoin( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, PLINT code );

void
plpoin3( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, const PLFLT *ArrayCk, PLINT code );

void
plpoly3( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, const PLFLT *ArrayCk, const PLBOOL *ArrayCkMinus1,
         PLBOOL flag );

void
plprec( PLINT setp, PLINT prec );

void
plpsty( PLINT patt );

void
plptex( PLFLT x, PLFLT y, PLFLT dx, PLFLT dy, PLFLT just, const char *text );

void
plptex3( PLFLT x, PLFLT y, PLFLT z, PLFLT dx, PLFLT dy, PLFLT dz, PLFLT sx, PLFLT sy, PLFLT sz, PLFLT just, const char *text );

PLFLT
plrandd();

void
plreplot( void );

void
plrgbhls( PLFLT r, PLFLT g, PLFLT b, PLFLT *OUTPUT, PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plschr( PLFLT def, PLFLT scale );

void
plscmap0( const PLINT *Array, const PLINT *ArrayCk, const PLINT *ArrayCk, PLINT n );

void
plscmap0a( const PLINT *Array, const PLINT *ArrayCk, const PLINT *ArrayCk, const PLFLT *ArrayCk, PLINT n );

void
plscmap0n( PLINT ncol0 );

void
plscmap1( const PLINT *Array, const PLINT *ArrayCk, const PLINT *ArrayCk, PLINT n );

void
plscmap1a( const PLINT *Array, const PLINT *ArrayCk, const PLINT *ArrayCk, const PLFLT *ArrayCk, PLINT n );

void
plscmap1l( PLBOOL itype, PLINT n, const PLFLT *Array,
           const PLFLT *ArrayCk, const PLFLT *ArrayCk, const PLFLT *ArrayCk,
           const PLBOOL *ArrayCkMinus1Null );

void
plscmap1la( PLBOOL itype, PLINT n, const PLFLT *Array,
            const PLFLT *ArrayCk, const PLFLT *ArrayCk, const PLFLT *ArrayCk, const PLFLT *ArrayCk,
            const PLBOOL *ArrayCkMinus1Null );

void
plscmap1n( PLINT ncol1 );

void
plscmap1_range( PLFLT min_color, PLFLT max_color );

void
plgcmap1_range( PLFLT *OUTPUT, PLFLT *OUTPUT );

void
plscol0( PLINT icol0, PLINT r, PLINT g, PLINT b );

void
plscol0a( PLINT icol0, PLINT r, PLINT g, PLINT b, PLFLT a );

void
plscolbg( PLINT r, PLINT g, PLINT b );

void
plscolbga( PLINT r, PLINT g, PLINT b, PLFLT a );

void
plscolor( PLINT color );

void
plscompression( PLINT compression );

void
plsdev( const char *devname );

void
plsdidev( PLFLT mar, PLFLT aspect, PLFLT jx, PLFLT jy );

void
plsdimap( PLINT dimxmin, PLINT dimxmax, PLINT dimymin, PLINT dimymax,
          PLFLT dimxpmm, PLFLT dimypmm );

void
plsdiori( PLFLT rot );

void
plsdiplt( PLFLT xmin, PLFLT ymin, PLFLT xmax, PLFLT ymax );

void
plsdiplz( PLFLT xmin, PLFLT ymin, PLFLT xmax, PLFLT ymax );

void
plseed( unsigned int s );

void
plsesc( char esc );

PLINT
plsetopt( const char *opt, const char *optarg );

void
plsfam( PLINT fam, PLINT num, PLINT bmax );

void
plsfci( PLUNICODE fci );

void
plsfnam( const char *fnam );

void
plsfont( PLINT family, PLINT style, PLINT weight );

void
plshades( const PLFLT **Matrix, PLINT nx, PLINT ny, defined_func df,
          PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
          const PLFLT *Array, PLINT n, PLFLT fill_width,
          PLINT cont_color, PLFLT cont_width,
          fill_func ff, PLBOOL rectangular,
          pltr_func pltr,
          PLPointer SWIG_OBJECT_DATA );

void
plshade( const PLFLT **Matrix, PLINT nx, PLINT ny, defined_func df,
         PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
         PLFLT shade_min, PLFLT shade_max,
         PLINT sh_cmap, PLFLT sh_color, PLFLT sh_width,
         PLINT min_color, PLFLT min_width,
         PLINT max_color, PLFLT max_width,
         fill_func ff, PLBOOL rectangular,
         pltr_func pltr,
         PLPointer SWIG_OBJECT_DATA );

void
plslabelfunc( label_func lf, PLPointer data );

void
plsmaj( PLFLT def, PLFLT scale );

#if defined ( PYTHON_HAVE_PYBUFFER ) && defined ( SWIG_PYTHON )

void
plsmem( PLINT maxx, PLINT maxy, void *plotmem );

void
plsmema( PLINT maxx, PLINT maxy, void *plotmem );

#endif

void
plsmin( PLFLT def, PLFLT scale );

void
plsori( PLINT ori );

void
plspage( PLFLT xp, PLFLT yp, PLINT xleng, PLINT yleng,
         PLINT xoff, PLINT yoff );

void
plspal0( const char *filename );

void
plspal1( const char *filename, PLBOOL interpolate );

void
plspause( PLBOOL pause );

void
plsstrm( PLINT strm );

void
plssub( PLINT nx, PLINT ny );

void
plssym( PLFLT def, PLFLT scale );

void
plstar( PLINT nx, PLINT ny );

void
plstart( const char *devname, PLINT nx, PLINT ny );

void
plstransform( ct_func ctf, PLPointer data );

void
plstring( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, const char *string );

void
plstring3( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, const PLFLT *ArrayCk, const char *string );

void
plstripa( PLINT id, PLINT pen, PLFLT x, PLFLT y );

void
plstripc( PLINT *OUTPUT, const char *xspec, const char *yspec,
          PLFLT xmin, PLFLT xmax, PLFLT xjump, PLFLT ymin, PLFLT ymax,
          PLFLT xlpos, PLFLT ylpos,
          PLBOOL y_ascl, PLBOOL acc,
          PLINT colbox, PLINT collab,
          const PLINT *Array, const PLINT *ArrayCk, const char *legline[4],
          const char *labx, const char *laby, const char *labtop );

void
plstripd( PLINT id );

void
plstyl( PLINT n, const PLINT *Array, const PLINT *ArrayCk );

void
plsvect( const PLFLT *ArrayNull, const PLFLT *ArrayCkNull, PLINT n, PLBOOL deffalse );

void
plsvpa( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax );

void
plsxax( PLINT digmax, PLINT digits );

void
plsyax( PLINT digmax, PLINT digits );

void
plsym( PLINT n, const PLFLT *Array, const PLFLT *ArrayCk, PLINT code );

void
plszax( PLINT digmax, PLINT digits );

void
pltext( void );

void
pltimefmt( const char *fmt );

void
plvasp( PLFLT aspect );

void
plvect( const PLFLT **Matrix, const PLFLT **MatrixCk, PLINT nx, PLINT ny, PLFLT scale,
        pltr_func pltr,
        PLPointer SWIG_OBJECT_DATA );

void
plvpas( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax, PLFLT aspect );

void
plvpor( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax );

void
plvsta( void );

void
plw3d( PLFLT basex, PLFLT basey, PLFLT height, PLFLT xmin0,
       PLFLT xmax0, PLFLT ymin0, PLFLT ymax0, PLFLT zmin0,
       PLFLT zmax0, PLFLT alt, PLFLT az );

void
plwidth( PLFLT width );

void
plwind( PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax );

void
plxormod( PLBOOL mode, PLBOOL *OUTPUT );

//--------------------------------------------------------------------------
//  Functions for use from C or C++ only
//  N.B. If you want these in python, they should be officially put in
//  the common API for all front-ends to the PLplot library with "c_" suffix,
//  DocBook xml documentation in the api.xml chapter, etc.
//--------------------------------------------------------------------------

#if 0

// Draws a contour plot using the function evaluator f2eval and data stored
// by way of the f2eval_data pointer.  This allows arbitrary organizations
// of 2d array data to be used.
//
void
plfcont( f2eval_func f2eval,
         PLPointer SWIG_OBJECT_DATA,
         PLINT nx, PLINT ny, PLINT kx, PLINT lx,
         PLINT ky, PLINT ly, const PLFLT *clevel, PLINT nlevel,
         pltr_func pltr,
         PLPointer SWIG_OBJECT_DATA );

#endif

// plot continental outline in world coordinates

void
plmap( mapform_func mapform, const char *type,
       PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy );

// Plot map outlines

void
plmapline( mapform_func mapform, const char *type,
           PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
           const PLINT *ArrayNull, PLINT n );

// Plot map points

void
plmapstring( mapform_func mapform, const char *type, const char *string,
             PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
             const PLINT *ArrayNull, PLINT n );

// Plot map text

void
plmaptex( mapform_func mapform, const char *type,
          PLFLT dx, PLFLT dy, PLFLT just, const char *text,
          PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
          PLINT plotentry );

// Plot map fills

void
plmapfill( mapform_func mapform, const char *type,
           PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy,
           const PLINT *ArrayNull, PLINT n );

// Plot the latitudes and longitudes on the background.

void
plmeridians( mapform_func mapform,
             PLFLT dlong, PLFLT dlat,
             PLFLT minlong, PLFLT maxlong, PLFLT minlat, PLFLT maxlat );


#if 0

void
plfshade( f2eval_func,
          PLPointer SWIG_OBJECT_DATA,
          c2eval_func,
          PLPointer c2eval_data,
          PLINT nx, PLINT ny,
          PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
          PLFLT shade_min, PLFLT shade_max,
          PLINT sh_cmap, PLFLT sh_color, PLINT sh_width,
          PLINT min_color, PLINT min_width,
          PLINT max_color, PLINT max_width,
          fill_func, PLBOOL rectangular,
          pltr_func,
          PLPointer SWIG_OBJECT_DATA );

// Converts input values from relative device coordinates to relative plot
// coordinates.

void
pldid2pc( PLFLT *INOUT, PLFLT *INOUT, PLFLT *INOUT, PLFLT *INOUT );

// Converts input values from relative plot coordinates to relative
// device coordinates.

void
pldip2dc( PLFLT *INOUT, PLFLT *INOUT, PLFLT *INOUT, PLFLT *INOUT );
#endif

// plots a 2d image (or a matrix too large for plshade() ).

void
plimage( const PLFLT **Matrix, PLINT nx, PLINT ny,
         PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax, PLFLT zmin, PLFLT zmax,
         PLFLT Dxmin, PLFLT Dxmax, PLFLT Dymin, PLFLT Dymax );

// plots a 2d image (or a matrix too large for plshade() ).

void
plimagefr( const PLFLT **Matrix, PLINT nx, PLINT ny,
           PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax, PLFLT zmin, PLFLT zmax,
           PLFLT valuemin, PLFLT valuemax,
           pltr_func pltr_img, PLPointer SWIG_OBJECT_DATA_img );

#ifdef 0
// Returns a list of file-oriented device names and their menu strings
void
plgFileDevs( char ***p_menustr, char ***p_devname, PLINT *p_ndev );

// Returns a list of all device names and their menu strings

void
plgDevs( char ***p_menustr, char ***p_devname, PLINT *p_ndev );

// Set the function pointer for the keyboard event handler

void
plsKeyEH( void ( *KeyEH )( PLGraphicsIn *, void *, PLINT * ), void *KeyEH_data );

// Set the function pointer for the (mouse) button event handler

void
plsButtonEH( void ( *ButtonEH )( PLGraphicsIn *, void *, PLINT * ),
             void *ButtonEH_data );
#endif
// Set the variables to be used for storing error info

#if 0
// Cannot get this to work since plsError is not simply an output
// of an internal integer and character string.
void
plsError( PLINT *OUTPUT, char *OUTPUT );
#endif

// Sets an optional user exit handler.
#if 0
void
plsexit( PLINT ( *handler )( const char * ) );
// Transformation routines
#endif

#if 0
// Just like pltr2() but uses pointer arithmetic to get coordinates from
// 2d grid tables.

void
pltr2p( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, PLPointer pltr_data );
// Identity transformation for plots from Fortran.

void
pltr0f( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data );

// Does linear interpolation from doubly dimensioned coord arrays
// (row dominant, i.e. Fortran ordering).

void
pltr2f( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data );

// Example linear transformation function for contour plotter.
// This is not actually part of the core library any more
//void
//xform(PLFLT x, PLFLT y, PLFLT *OUTPUT, PLFLT *OUTPUT);
// Function evaluators
// Does a lookup from a 2d function array.  Array is of type (PLFLT **),
// and is column dominant (normal C ordering).

PLFLT
plf2eval2( PLINT ix, PLINT iy, PLPointer plf2eval_data );

// Does a lookup from a 2d function array.  Array is of type (PLFLT *),
// and is column dominant (normal C ordering).

PLFLT
plf2eval( PLINT ix, PLINT iy, PLPointer plf2eval_data );

// Does a lookup from a 2d function array.  Array is of type (PLFLT *),
// and is row dominant (Fortran ordering).

PLFLT
plf2evalr( PLINT ix, PLINT iy, PLPointer plf2eval_data );
#endif
// Command line parsing utilities

// Clear internal option table info structure.

void
plClearOpts( void );

// Reset internal option table info structure.

void
plResetOpts( void );

// Merge user option table into internal info structure.
#if 0
PLINT
plMergeOpts( PLOptionTable *options, const char *name, const char **notes );
#endif
// Set the strings used in usage and syntax messages.

void
plSetUsage( const char *program_string, const char *usage_string );

// Print usage & syntax message.

void
plOptUsage( void );

// Miscellaneous
#if 0
// Set the output file pointer

void
plgfile( FILE **p_file );

// Get the output file pointer

void
plsfile( FILE *file );

// Get the escape character for text strings.

void
plgesc( char *p_esc );

// Front-end to driver escape function.

void
pl_cmd( PLINT op, void *ptr );

// Return full pathname for given file if executable

PLINT
plFindName( char *p );

// Looks for the specified executable file according to usual search path.

char *
plFindCommand( const char *fn );

// Gets search name for file by concatenating the dir, subdir, and file
// name, allocating memory as needed.

void
plGetName( const char *dir, const char *subdir, const char *filename, char **filespec );

// Prompts human to input an integer in response to given message.

PLINT
plGetInt( const char *s );

// Prompts human to input a float in response to given message.

PLFLT
plGetFlt( const char *s );

// Nice way to allocate space for a vectored 2d grid

// Allocates a block of memory for use as a 2-d grid of PLFLT's.

void
plAlloc2dGrid( PLFLT ***f, PLINT nx, PLINT ny );

// Frees a block of memory allocated with plAlloc2dGrid().

void
plFree2dGrid( PLFLT **f, PLINT nx, PLINT ny );

// Find the maximum and minimum of a 2d matrix allocated with plAllc2dGrid().

#endif

void
plMinMax2dGrid( const PLFLT **Matrix, PLINT nx, PLINT ny, PLFLT *OUTPUT, PLFLT *OUTPUT );

// Wait for graphics input event and translate to world coordinates

PLINT
plGetCursor( PLGraphicsIn *gin );

// Translates relative device coordinates to world coordinates.

#if 0
// Use plcalc_world instead of plTranslateCursor.
PLINT
plTranslateCursor( PLGraphicsIn *gin );
#endif
