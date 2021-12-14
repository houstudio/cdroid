//        PLplot PostScript device driver using LASi to provide fonts
//        based on original ps.c PostScript driver
//
//  Copyright (C) 1992-2001 Geoffrey Furnish
//  Copyright (C) 1992-2001 Maurice LeBrun
//  Copyright (C) 2000-2018 Alan W. Irwin
//  Copyright (C) 2001-2002 Joao Cardoso
//  Copyright (C) 2001-2004 Rafael Laboissiere
//  Copyright (C) 2004-2005 Thomas J. Duck
//  Copyright (C) 2005-2006 Andrew Ross
//
//  This file is part of PLplot.
//
//  PLplot is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  PLplot is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public License
//  along with PLplot; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//

#include "plDevs.h"

//#define NEED_PLDEBUG
#include "plplotP.h"
#include "drivers.h"
#include "ps.h"

#include <string.h>
#include <time.h>
#include "plfreetype.h"
#include <iostream>
#include <fstream>
#include <LASi.h>

// Define macro to truncate small values to zero - prevents
//  * printf printing -0.000
#define TRMFLT( a )    ( ( fabs( a ) < 5.0e-4 ) ? 0.0 : ( a ) )

using namespace LASi;
using namespace std;

// Device info

PLDLLIMPEXP_DRIVER const char* plD_DEVICE_INFO_psttf =
#ifdef PLD_psttf
    "psttf:PostScript File (monochrome):0:psttf:55:psttf\n"
#endif
#ifdef PLD_psttfc
    "psttfc:PostScript File (color):0:psttf:56:psttfc\n"
#endif
;


// Prototypes for functions in this file.

#ifdef PLD_psttf
void plD_dispatch_init_psttf( PLDispatchTable *pdt );
#endif
#ifdef PLD_psttfc
void plD_dispatch_init_psttfc( PLDispatchTable *pdt );
#endif

static char  *ps_getdate( void );
static void  ps_init( PLStream * );
static void  fill_polygon( PLStream *pls );
static void  proc_str( PLStream *, EscText * );
//static void  esc_purge( char *, char * );
static void psttf_dispatch_init_helper( PLDispatchTable *pdt,
                                        const char *menustr, const char *devnam,
                                        int type, int seq, plD_init_fp init );

#define OUTBUF_LEN    128
static char outbuf[OUTBUF_LEN];
static int  text = 1;
static int  color;
static int  hrshsym = 0;

// Font style and weight lookup tables

#define N_Pango_Lookup    5

const char * DefaultFamilyLookup[N_Pango_Lookup] = {
    "sans",
    "serif",
    "monospace",
    "sans,serif",
    "sans,serif"
};

const char * EnvFamilyLookup[N_Pango_Lookup] = {
    "PLPLOT_FREETYPE_SANS_FAMILY",
    "PLPLOT_FREETYPE_SERIF_FAMILY",
    "PLPLOT_FREETYPE_MONO_FAMILY",
    "PLPLOT_FREETYPE_SCRIPT_FAMILY",
    "PLPLOT_FREETYPE_SYMBOL_FAMILY"
};

#define FAMILY_LOOKUP_LEN    1024
char             FamilyLookup[N_Pango_Lookup][FAMILY_LOOKUP_LEN];

const FontWeight WeightLookup[2] = {
    NORMAL_WEIGHT,
    BOLD
};

const FontStyle  StyleLookup[3] = {
    NORMAL_STYLE,
    ITALIC,
    OBLIQUE
};

static DrvOpt    ps_options[] = { { "text",    DRV_INT, &text,    "Use Postscript text (text=0|1)"       },
                                  { "color",   DRV_INT, &color,   "Use color (color=0|1)"                },
                                  { "hrshsym", DRV_INT, &hrshsym, "Use Hershey symbol set (hrshsym=0|1)" },
                                  { NULL,      DRV_INT, NULL,     NULL                                   } };

// text > 0 uses some postscript tricks, namely a transformation matrix
//   that scales, rotates (with slanting) and offsets text strings.
//   It has yet some bugs for 3d plots.


static void psttf_dispatch_init_helper( PLDispatchTable *pdt,
                                        const char *menustr, const char *devnam,
                                        int type, int seq, plD_init_fp init )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = (char *) menustr;
    pdt->pl_DevName = (char *) devnam;
#else
    (void) menustr;   // Cast to void to silence compiler warnings about unused parameters
    (void) devnam;
#endif
    pdt->pl_type     = type;
    pdt->pl_seq      = seq;
    pdt->pl_init     = init;
    pdt->pl_line     = (plD_line_fp) plD_line_psttf;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_psttf;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_psttf;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_psttf;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_psttf;
    pdt->pl_state    = (plD_state_fp) plD_state_psttf;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_psttf;
}

#ifdef PLD_psttf
void plD_dispatch_init_psttf( PLDispatchTable *pdt )
{
    psttf_dispatch_init_helper( pdt,
        "PostScript File (monochrome)", "psttf",
        plDevType_FileOriented, 55,
        (plD_init_fp) plD_init_psttf );
}

//--------------------------------------------------------------------------
// plD_init_psttf()
//
// Initialize device.
//--------------------------------------------------------------------------

void
plD_init_psttf( PLStream *pls )
{
    color      = 0;
    pls->color = 0;             // Not a color device

    plParseDrvOpts( ps_options );
    if ( color )
        pls->color = 1;         // But user wants color
    ps_init( pls );
}
#endif //#ifdef PLD_psttf

#ifdef PLD_psttfc
void plD_dispatch_init_psttfc( PLDispatchTable *pdt )
{
    psttf_dispatch_init_helper( pdt,
        "PostScript File (color)", "psttfc",
        plDevType_FileOriented, 56,
        (plD_init_fp) plD_init_psttfc );
}

void
plD_init_psttfc( PLStream *pls )
{
    color      = 1;
    pls->color = 1;             // Is a color device
    plParseDrvOpts( ps_options );

    if ( !color )
        pls->color = 0;         // But user does not want color
    ps_init( pls );
}
#endif //#ifdef PLD_psttfc

#define MAX_NUM_TRIES    10
static void
ps_init( PLStream *pls )
{
    int   i;
    char  *a;
    PSDev *dev;
    PostscriptDocument *doc;
    PLFLT pxlx, pxly;

    // Set default values - 7.5 x 10 [inches] (72 points = 1 inch)
    if ( pls->xlength <= 0 || pls->ylength <= 0 )
    {
        pls->xlength = 540;
        pls->ylength = 720;
    }
    if ( pls->xdpi <= 0 )
        pls->xdpi = 72.;
    if ( pls->ydpi <= 0 )
        pls->ydpi = 72.;


    pxlx = YPSSIZE / LPAGE_X;
    pxly = XPSSIZE / LPAGE_Y;

    if ( text )
    {
        pls->dev_text    = 1;     // want to draw text
        pls->dev_unicode = 1;     // want unicode
        if ( hrshsym )
            pls->dev_hrshsym = 1; // want Hershey symbols
    }

    pls->dev_fill0 = 1;         // Can do solid fills

// Initialize family file info

    plFamInit( pls );

// Prompt for a file name if not already set

    plOpenFile( pls );

// Create postscript document object
    if ( pls->psdoc != NULL )
        delete (PostscriptDocument *) pls->psdoc;

    pls->psdoc = new PostscriptDocument();
    doc        = (PostscriptDocument *) ( pls->psdoc );
    doc->osBody() << fixed;
    doc->osBody().precision( 3 );

// Allocate and initialize device-specific data

    if ( pls->dev != NULL )
        free( (void *) pls->dev );

    pls->dev = calloc( 1, (size_t) sizeof ( PSDev ) );
    if ( pls->dev == NULL )
        plexit( "ps_init: Out of memory." );

    dev = (PSDev *) pls->dev;

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;

    plP_setpxl( pxlx, pxly );

    dev->llx   = XPSSIZE;
    dev->lly   = YPSSIZE;
    dev->urx   = 0;
    dev->ury   = 0;
    dev->ptcnt = 0;

// Rotate by 90 degrees since portrait mode addressing is used

    dev->xmin = 0;
    dev->ymin = 0;
    dev->xmax = PSY;
    dev->ymax = PSX;
    dev->xlen = dev->xmax - dev->xmin;
    dev->ylen = dev->ymax - dev->ymin;

    plP_setphy( dev->xmin, dev->xmax, dev->ymin, dev->ymax );

// If portrait mode is specified, then set up an additional rotation
// transformation with aspect ratio allowed to adjust via freeaspect.
// Default orientation is landscape (ORIENTATION == 3 or 90 deg rotation
// counter-clockwise from portrait).  (Legacy PLplot used seascape
// which was equivalent to ORIENTATION == 1 or 90 deg clockwise rotation
// from portrait.)

    if ( pls->portrait )
    {
        plsdiori( (PLFLT) ( 4 - ORIENTATION ) );
        pls->freeaspect = 1;
    }

    // File table for font families using either environment variables
    // or defaults.
    for ( i = 0; i < N_Pango_Lookup; i++ )
    {
        if ( ( a = getenv( EnvFamilyLookup[i] ) ) != NULL )
        {
            strncpy( FamilyLookup[i], a, FAMILY_LOOKUP_LEN - 1 );
            FamilyLookup[i][FAMILY_LOOKUP_LEN - 1] = '\0';
        }
        else
        {
            strncpy( FamilyLookup[i], DefaultFamilyLookup[i], FAMILY_LOOKUP_LEN );
            FamilyLookup[i][FAMILY_LOOKUP_LEN - 1] = '\0';
        }
    }
}


//--------------------------------------------------------------------------
// writeHeader()
//
// Write plplot postscript commands into the header
//--------------------------------------------------------------------------

void
writeHeader( PLStream *pls )
{
    PostscriptDocument *doc = (PostscriptDocument *) ( pls->psdoc );

    doc->osHeader() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

    doc->osHeader() << "%%Title: PLplot Graph\n";
    doc->osHeader() << "%%Creator: PLplot Version " << PLPLOT_VERSION << "\n";
    doc->osHeader() << "%%CreationDate: " << ps_getdate() << "\n";
    doc->osHeader() << "%%Pages: (atend)\n";
    doc->osHeader() << "%%EndComments\n\n";

// Definitions
// Save VM state

    doc->osHeader() << "/PSSave save def\n";

// Define a dictionary and start using it

    doc->osHeader() << "/PSDict 200 dict def\n";
    doc->osHeader() << "PSDict begin\n";

    doc->osHeader() << "/@restore /restore load def\n";
    doc->osHeader() << "/restore\n";
    doc->osHeader() << "   {vmstatus pop\n";
    doc->osHeader() << "    dup @VMused lt {pop @VMused} if\n";
    doc->osHeader() << "    exch pop exch @restore /@VMused exch def\n";
    doc->osHeader() << "   } def\n";
    doc->osHeader() << "/@pri\n";
    doc->osHeader() << "   {\n";
    doc->osHeader() << "    ( ) print\n";
    doc->osHeader() << "    (                                       ) cvs print\n";
    doc->osHeader() << "   } def\n";

// n @copies -

    doc->osHeader() << "/@copies\n";
    doc->osHeader() << "   {\n";
    doc->osHeader() << "    /#copies exch def\n";
    doc->osHeader() << "   } def\n";

// - @start -  -- start everything

    doc->osHeader() << "/@start\n";
    doc->osHeader() << "   {\n";
    doc->osHeader() << "    vmstatus pop /@VMused exch def pop\n";
    doc->osHeader() << "   } def\n";

// - @end -  -- finished

    doc->osHeader() << "/@end\n";
    doc->osHeader() << "   {flush\n";
    doc->osHeader() << "    end\n";
    doc->osHeader() << "    PSSave restore\n";
    doc->osHeader() << "   } def\n";

// bop -  -- begin a new page
// Only fill background if we are using color and if the bg isn't white

    doc->osHeader() << "/bop\n";
    doc->osHeader() << "   {\n";
    doc->osHeader() << "    /SaveImage save def\n";
    doc->osHeader() << "   } def\n";

// - eop -  -- end a page

    doc->osHeader() << "/eop\n";
    doc->osHeader() << "   {\n";
    doc->osHeader() << "    showpage\n";
    doc->osHeader() << "    SaveImage restore\n";
    doc->osHeader() << "   } def\n";

// Set line parameters

    doc->osHeader() << "/@line\n";
    doc->osHeader() << "   {0 setlinecap\n";
    doc->osHeader() << "    0 setlinejoin\n";
    doc->osHeader() << "    1 setmiterlimit\n";
    doc->osHeader() << "   } def\n";

// d @hsize -  horizontal clipping dimension

    doc->osHeader() << "/@hsize   {/hs exch def} def\n";
    doc->osHeader() << "/@vsize   {/vs exch def} def\n";

// d @hoffset - shift for the plots

    doc->osHeader() << "/@hoffset {/ho exch def} def\n";
    doc->osHeader() << "/@voffset {/vo exch def} def\n";

// Set line width

    doc->osHeader() << "/lw " << (int) (
        ( pls->width < MIN_WIDTH ) ? DEF_WIDTH :
        ( pls->width > MAX_WIDTH ) ? MAX_WIDTH : pls->width ) << " def\n";

// Setup user specified offsets, scales, sizes for clipping

    doc->osHeader() << "/@SetPlot\n";
    doc->osHeader() << "   {\n";
    doc->osHeader() << "    ho vo translate\n";
    doc->osHeader() << "    XScale YScale scale\n";
    doc->osHeader() << "    lw setlinewidth\n";
    doc->osHeader() << "   } def\n";

// Setup x & y scales

    doc->osHeader() << "/XScale\n";
    doc->osHeader() << "   {hs " << YPSSIZE << " div} def\n";
    doc->osHeader() << "/YScale\n";
    doc->osHeader() << "   {vs " << XPSSIZE << " div} def\n";

// Macro definitions of common instructions, to keep output small

    doc->osHeader() << "/M {moveto} def\n";
    doc->osHeader() << "/D {lineto} def\n";
    doc->osHeader() << "/A {0.5 0 360 arc} def\n";
    doc->osHeader() << "/S {stroke} def\n";
    doc->osHeader() << "/Z {stroke newpath} def\n";
    if ( pls->dev_eofill )
        doc->osHeader() << "/F {closepath gsave eofill grestore stroke} def\n";
    else
        doc->osHeader() << "/F {closepath gsave fill grestore stroke} def\n";
    doc->osHeader() << "/N {newpath} def\n";
    doc->osHeader() << "/C {setrgbcolor} def\n";
    doc->osHeader() << "/G {setgray} def\n";
    doc->osHeader() << "/W {setlinewidth} def\n";
    doc->osHeader() << "/R {rotate} def\n";
    doc->osHeader() << "/B {Z " << 0 << " " << 0 << " M " << 0 << " " << PSY << " D " << PSX << " " << PSY << " D " << PSX << " " << 0 << " D " << 0 << " " << 0 << " closepath} def\n";
    doc->osHeader() << "/CL {newpath M D D D closepath clip} def\n";

// End of dictionary definition

    doc->osHeader() << "end\n\n";

// Set up the plots

    doc->osHeader() << "PSDict begin\n";
    doc->osHeader() << "@start\n";
    doc->osHeader() << COPIES << " @copies\n";
    doc->osHeader() << "@line\n";
    doc->osHeader() << YSIZE << " @hsize\n";
    doc->osHeader() << XSIZE << " @vsize\n";
    doc->osHeader() << YOFFSET << " @hoffset\n";
    doc->osHeader() << XOFFSET << " @voffset\n";

    doc->osHeader() << "@SetPlot\n" << endl;
}

//--------------------------------------------------------------------------
// plD_line_psttf()
//
// Draw a line in the current color from (x1,y1) to (x2,y2).
//--------------------------------------------------------------------------

void
plD_line_psttf( PLStream *pls, short x1a, short y1a, short x2a, short y2a )
{
    PSDev *dev = (PSDev *) pls->dev;
    PostscriptDocument *doc = (PostscriptDocument *) pls->psdoc;
    PLINT x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;

// Rotate by 90 degrees

    plRotPhy( ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax, &x1, &y1 );
    plRotPhy( ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax, &x2, &y2 );

    if ( x1 == dev->xold && y1 == dev->yold && dev->ptcnt < 40 )
    {
        if ( pls->linepos + 12 > LINELENGTH )
        {
            doc->osBody() << '\n';
            pls->linepos = 0;
        }
        else
            doc->osBody() << ' ';

        snprintf( outbuf, OUTBUF_LEN, "%d %d D", x2, y2 );
        dev->ptcnt++;
        pls->linepos += 12;
    }
    else
    {
        doc->osBody() << " Z\n";
        pls->linepos = 0;

        if ( x1 == x2 && y1 == y2 ) // must be a single dot, draw a circle
            snprintf( outbuf, OUTBUF_LEN, "%d %d A", x1, y1 );
        else
            snprintf( outbuf, OUTBUF_LEN, "%d %d M %d %d D", x1, y1, x2, y2 );
        dev->llx      = MIN( dev->llx, x1 );
        dev->lly      = MIN( dev->lly, y1 );
        dev->urx      = MAX( dev->urx, x1 );
        dev->ury      = MAX( dev->ury, y1 );
        dev->ptcnt    = 1;
        pls->linepos += 24;
    }
    dev->llx = MIN( dev->llx, x2 );
    dev->lly = MIN( dev->lly, y2 );
    dev->urx = MAX( dev->urx, x2 );
    dev->ury = MAX( dev->ury, y2 );

    doc->osBody() << outbuf;
    pls->bytecnt += 1 + strlen( outbuf );
    dev->xold     = x2;
    dev->yold     = y2;
}

//--------------------------------------------------------------------------
// plD_polyline_psttf()
//
// Draw a polyline in the current color.
//--------------------------------------------------------------------------

void
plD_polyline_psttf( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    PLINT i;

    for ( i = 0; i < npts - 1; i++ )
        plD_line_psttf( pls, xa[i], ya[i], xa[i + 1], ya[i + 1] );
}

//--------------------------------------------------------------------------
// plD_eop_psttf()
//
// End of page.
//--------------------------------------------------------------------------

void
plD_eop_psttf( PLStream *pls )
{
    PostscriptDocument *doc = (PostscriptDocument *) pls->psdoc;
    doc->osBody() << " S\neop\n";
}

//--------------------------------------------------------------------------
// plD_bop_psttf()
//
// Set up for the next page.
// Advance to next family file if necessary (file output).
//--------------------------------------------------------------------------

void
plD_bop_psttf( PLStream *pls )
{
    PSDev *dev = (PSDev *) pls->dev;
    PostscriptDocument *doc = (PostscriptDocument *) pls->psdoc;

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;

    if ( !pls->termin )
        plGetFam( pls );

    pls->page++;

    if ( pls->family )
        doc->osBody() << "%%Page: " << (int) pls->page << " 1\n";
    else
        doc->osBody() << "%%Page: " << (int) pls->page << " " << (int) pls->page << "\n";

    doc->osBody() << "bop\n";
    if ( pls->color )
    {
        PLFLT r, g, b;
        if ( pls->cmap0[0].r != 0xFF ||
             pls->cmap0[0].g != 0xFF ||
             pls->cmap0[0].b != 0xFF )
        {
            r = ( (PLFLT) pls->cmap0[0].r ) / 255.;
            g = ( (PLFLT) pls->cmap0[0].g ) / 255.;
            b = ( (PLFLT) pls->cmap0[0].b ) / 255.;

            doc->osBody() << "B " << r << " " << g << " " << b << " C F\n";
        }
    }
    pls->linepos = 0;

// This ensures the color and line width are set correctly at the beginning of
//   each page

    plD_state_psttf( pls, PLSTATE_COLOR0 );
    plD_state_psttf( pls, PLSTATE_WIDTH );
}

//--------------------------------------------------------------------------
// plD_tidy_psttf()
//
// Close graphics file or otherwise clean up.
//--------------------------------------------------------------------------

void
plD_tidy_psttf( PLStream *pls )
{
    PSDev *dev = (PSDev *) pls->dev;
    PostscriptDocument *doc = (PostscriptDocument *) pls->psdoc;

    dev->llx /= ENLARGE;
    dev->lly /= ENLARGE;
    dev->urx /= ENLARGE;
    dev->ury /= ENLARGE;
    dev->llx += YOFFSET;
    dev->lly += XOFFSET;
    dev->urx += YOFFSET;
    dev->ury += XOFFSET;

// changed for correct Bounding boundaries Jan Thorbecke  okt 1993
// occurs from the integer truncation -- postscript uses fp arithmetic

    dev->urx += 1;
    dev->ury += 1;

    if ( pls->family )
        doc->osFooter() << "%%Pages: 1\n";
    else
        doc->osFooter() << "%%Pages: " << (int) pls->page << "\n";

    doc->osFooter() << "@end" << endl;

// Now write the rest of the header
    writeHeader( pls );

// Write out postscript document to file and close
    // For C++ stream we first need to close the file using
//       the C FILE * handle, then reopen as a ofstream. Yuck!
    if ( !strcmp( pls->FileName, "-" ) )
    {
        doc->write( cout, dev->llx, dev->lly, dev->urx, dev->ury );
    }
    else
    {
        plCloseFile( pls );
        ofstream out;
        out.open( pls->FileName );
        doc->write( out, dev->llx, dev->lly, dev->urx, dev->ury );
        out.close();
    }

    delete doc;
    pls->psdoc = NULL;
}

//--------------------------------------------------------------------------
// plD_state_psttf()
//
// Handle change in PLStream state (color, pen width, fill attribute, etc).
//--------------------------------------------------------------------------

void
plD_state_psttf( PLStream *pls, PLINT op )
{
    PSDev *dev = (PSDev *) pls->dev;
    PostscriptDocument *doc = (PostscriptDocument *) pls->psdoc;

    switch ( op )
    {
    case PLSTATE_WIDTH: {
        int width = (int) (
            ( pls->width < MIN_WIDTH ) ? DEF_WIDTH :
            ( pls->width > MAX_WIDTH ) ? MAX_WIDTH : pls->width );

        doc->osBody() << " S\n" << width << " W";

        dev->xold = PL_UNDEFINED;
        dev->yold = PL_UNDEFINED;
        break;
    }
    case PLSTATE_COLOR0:
        if ( !pls->color )
        {
            doc->osBody() << " S\n" << ( pls->icol0 ? 0.0 : 1.0 ) << " G";
            // Reinitialize current point location.
            if ( dev->xold != PL_UNDEFINED && dev->yold != PL_UNDEFINED )
                doc->osBody() << " " << (int) dev->xold << " " << (int) dev->yold << " M \n";
            break;
        }
    // else fallthrough
    case PLSTATE_COLOR1:
        if ( pls->color )
        {
            PLFLT r = ( (PLFLT) pls->curcolor.r ) / 255.0;
            PLFLT g = ( (PLFLT) pls->curcolor.g ) / 255.0;
            PLFLT b = ( (PLFLT) pls->curcolor.b ) / 255.0;

            doc->osBody() << " S\n" << r << " " << g << " " << b << " C";
        }
        else
        {
            PLFLT r = ( (PLFLT) pls->curcolor.r ) / 255.0;
            doc->osBody() << " S\n" << 1.0 - r << " G";
        }
        // Reinitialize current point location.
        if ( dev->xold != PL_UNDEFINED && dev->yold != PL_UNDEFINED )
            doc->osBody() << " " << (int) dev->xold << " " << (int) dev->yold << " M \n";
        break;
    }
}

//--------------------------------------------------------------------------
// plD_esc_psttf()
//
// Escape function.
//--------------------------------------------------------------------------

void
plD_esc_psttf( PLStream *pls, PLINT op, void *ptr )
{
    switch ( op )
    {
    case PLESC_FILL:
        fill_polygon( pls );
        break;
    case PLESC_HAS_TEXT:
        proc_str( pls, (EscText *) ptr );
        break;
    }
}

//--------------------------------------------------------------------------
// fill_polygon()
//
// Fill polygon described in points pls->dev_x[] and pls->dev_y[].
// Only solid color fill supported.
//--------------------------------------------------------------------------

static void
fill_polygon( PLStream *pls )
{
    PSDev *dev = (PSDev *) pls->dev;
    PostscriptDocument *doc = (PostscriptDocument *) pls->psdoc;
    PLINT n, ix = 0, iy = 0;
    PLINT x, y;

    doc->osBody() << " Z\n";

    for ( n = 0; n < pls->dev_npts; n++ )
    {
        x = pls->dev_x[ix++];
        y = pls->dev_y[iy++];

// Rotate by 90 degrees

        plRotPhy( ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax, &x, &y );

// First time through start with a x y moveto

        if ( n == 0 )
        {
            snprintf( outbuf, OUTBUF_LEN, "N %d %d M", x, y );
            dev->llx = MIN( dev->llx, x );
            dev->lly = MIN( dev->lly, y );
            dev->urx = MAX( dev->urx, x );
            dev->ury = MAX( dev->ury, y );
            doc->osBody() << outbuf;
            pls->bytecnt += strlen( outbuf );
            continue;
        }

        if ( pls->linepos + 21 > LINELENGTH )
        {
            doc->osBody() << '\n';
            pls->linepos = 0;
        }
        else
            doc->osBody() << ' ';

        pls->bytecnt++;

        snprintf( outbuf, OUTBUF_LEN, "%d %d D", x, y );
        dev->llx = MIN( dev->llx, x );
        dev->lly = MIN( dev->lly, y );
        dev->urx = MAX( dev->urx, x );
        dev->ury = MAX( dev->ury, y );

        doc->osBody() << outbuf;
        pls->bytecnt += strlen( outbuf );
        pls->linepos += 21;
    }
    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
    doc->osBody() << " F ";
}

//--------------------------------------------------------------------------
// ps_getdate()
//
// Get the date and time
//--------------------------------------------------------------------------

static char *
ps_getdate( void )
{
    int    len;
    time_t t;
    char   *p;

    t   = time( (time_t *) 0 );
    p   = ctime( &t );
    len = strlen( p );
    *( p + len - 1 ) = '\0';      // zap the newline character
    return p;
}

// 0.8 should mimic the offset of first superscript/subscript level
// implemented in plstr (plsym.c) for Hershey fonts.  However, when
// comparing with -dev xwin and -dev xcairo results changing this
// factor to 0.6 appears to offset the centers of the letters
// appropriately while 0.8 gives much poorer agreement with the
// other devices.
# define RISE_FACTOR    0.6

//--------------------------------------------------------------------------
// proc_str()
//
// Prints postscript strings.
// N.B. Now unicode only, no string access!
//
//--------------------------------------------------------------------------

void
proc_str( PLStream *pls, EscText *args )
{
    PLFLT *t = args->xform, tt[4];              // Transform matrices
    PLFLT theta, shear, stride;                 // Rotation angle and shear from the matrix
    PLFLT ft_ht, offset;                        // Font height and offset
    PLFLT cs, sn;
    PSDev *dev = (PSDev *) pls->dev;
    PostscriptDocument *doc = (PostscriptDocument *) pls->psdoc;
    char               *font, esc;
    FontStyle          style;
    FontWeight         weight;
    // Be generous.  Used to store lots of font changes which take
    // 3 characters per change.
  #define PROC_STR_STRING_LENGTH    1000
    char   *strp, str[PROC_STR_STRING_LENGTH], *cur_strp,
            cur_str[PROC_STR_STRING_LENGTH];
    float  font_factor = 1.4;
    PLINT  clxmin, clxmax, clymin, clymax;  // Clip limits
    PLINT  clipx[4], clipy[4];              // Current clip limits

    PLFLT  scale = 1., up = 0.;             // Font scaling and shifting parameters

    double lineSpacing, xAdvance, ymintmp, ymaxtmp, ymin, ymax, xmin, xmax;
    PLINT  xx[4], yy[4];

    // unicode only! so test for it.
    if ( args->unicode_array_len > 0 )
    {
        int             j, s, f;
        char            *fonts[PROC_STR_STRING_LENGTH];
        FontStyle       styles[PROC_STR_STRING_LENGTH];
        FontWeight      weights[PROC_STR_STRING_LENGTH];
        const PLUNICODE *cur_text;
        PLUNICODE       fci;
        unsigned char   fontfamily, fontstyle, fontweight;
        PLFLT           old_sscale, sscale, old_soffset, soffset, dup;
        PLINT           level = 0;

        // translate from unicode into type 1 font index.
        //
        // Choose the font family, style, variant, and weight using
        // the FCI (font characterization integer).
        //

        plgesc( &esc );
        plgfci( &fci );
        plP_fci2hex( fci, &fontfamily, PL_FCI_FAMILY );
        plP_fci2hex( fci, &fontstyle, PL_FCI_STYLE );
        plP_fci2hex( fci, &fontweight, PL_FCI_WEIGHT );
        font   = (char *) FamilyLookup[fontfamily];
        weight = WeightLookup[fontweight];
        style  = StyleLookup[fontstyle];
        // Need to add some error checking here
        if ( false )
        {
            fprintf( stderr, "fci = 0x%x, font name pointer = NULL \n", fci );
            plabort( "proc_str: FCI inconsistent with TrueTypeLookup; "
                "internal PLplot error" );
            return;
        }
        //pldebug("proc_str", "fci = 0x%x, font name = %s\n", fci, font);
        cur_text = args->unicode_array;
        for ( f = s = j = 0; j < args->unicode_array_len; j++ )
        {
            if ( cur_text[j] & PL_FCI_MARK )
            {
                // process an FCI by saving it and escaping cur_str
                // with an escff to make it a 2-character escape
                // that is not used in legacy Hershey code
                //
                if ( ( f < PROC_STR_STRING_LENGTH ) && ( s + 3 < PROC_STR_STRING_LENGTH ) )
                {
                    plP_fci2hex( cur_text[j], &fontfamily, PL_FCI_FAMILY );
                    plP_fci2hex( cur_text[j], &fontstyle, PL_FCI_STYLE );
                    plP_fci2hex( cur_text[j], &fontweight, PL_FCI_WEIGHT );
                    fonts[f]   = (char *) FamilyLookup[fontfamily];
                    weights[f] = WeightLookup[fontweight];
                    styles[f]  = StyleLookup[fontstyle];
                    if ( fonts[f] == NULL )
                    {
                        fprintf( stderr, "string-supplied FCI = 0x%x, font name pointer = NULL \n", cur_text[j] );
                        plabort( "proc_str: string-supplied FCI inconsistent with font lookup;" );
                        return;
                    }
                    //pldebug("proc_str", "string-supplied FCI = 0x%x, font name = %s\n", cur_text[j], fonts[f]);
                    cur_str[s++] = esc;
                    cur_str[s++] = 'f';
                    cur_str[s++] = 'f';
                    f++;
                }
            }
            else if ( s + 1 < PROC_STR_STRING_LENGTH )
            {
                s += ucs4_to_utf8( cur_text[j], &cur_str[s] );
                //pldebug("proc_str", "unicode = 0x%x, type 1 code = %d\n",
//                          cur_text[j], cur_str[j]);
            }
        }
        cur_str[s] = '\0';

        // finish previous polyline

        dev->xold = PL_UNDEFINED;
        dev->yold = PL_UNDEFINED;

        // Determine the font height
        ft_ht = pls->chrht * 72.0 / 25.4; // ft_ht in points, ht is in mm


        // The transform matrix has only rotations and shears; extract them
        plRotationShear( t, &theta, &shear, &stride );
        cs    = cos( theta );
        sn    = sin( theta );
        tt[0] = t[0] * cs + t[2] * sn;
        tt[1] = t[1] * cs + t[3] * sn;
        tt[2] = -t[0] * sn + t[2] * cs;
        tt[3] = -t[1] * sn + t[3] * cs;

        //
        // Reference point conventions:
        //   If base = 0, it is aligned with the center of the text box
        //   If base = 1, it is aligned with the baseline of the text box
        //   If base = 2, it is aligned with the top of the text box
        //
        // Currently plplot only uses base=0
        // Postscript uses base=1
        //
        // We must calculate the difference between the two and apply the offset.
        //

        if ( args->base == 2 )             // not supported by plplot
            offset = ENLARGE * ft_ht / 2.; // half font height
        else if ( args->base == 1 )
            offset = 0.;
        else
            offset = -ENLARGE * ft_ht / 2.;

        // Determine the adjustment for page orientation
        theta   -= PI / 2. * pls->diorot;
        args->y += (int) ( offset * cos( theta ) );
        args->x -= (int) ( offset * sin( theta ) );

        // ps driver is rotated by default
        plRotPhy( ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax,
            &( args->x ), &( args->y ) );

        // Correct for the fact ps driver uses landscape by default
        theta += PI / 2.;

        // Output
        // Set clipping
        clipx[0] = pls->clpxmi;
        clipx[2] = pls->clpxma;
        clipy[0] = pls->clpymi;
        clipy[2] = pls->clpyma;
        clipx[1] = clipx[2];
        clipy[1] = clipy[0];
        clipx[3] = clipx[0];
        clipy[3] = clipy[2];
        difilt( clipx, clipy, 4, &clxmin, &clxmax, &clymin, &clymax );
        plRotPhy( ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax,
            &clipx[0], &clipy[0] );
        plRotPhy( ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax,
            &clipx[1], &clipy[1] );
        plRotPhy( ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax,
            &clipx[2], &clipy[2] );
        plRotPhy( ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax,
            &clipx[3], &clipy[3] );
        doc->osBody() << " gsave " << clipx[0] << " " << clipy[0] << " " <<
            clipx[1] << " " << clipy[1] << " " << clipx[2] << " " <<
            clipy[2] << " " << clipx[3] << " " << clipy[3] << " CL\n";

        // move to string reference point
        doc->osBody() << " " << args->x << " " << args->y << " M\n";

        // Save the current position and set the string rotation
        doc->osBody() << "gsave " << TRMFLT( theta * 180. / PI ) << " R\n";

        doc->osBody() << "[" << TRMFLT( tt[0] ) << " " << TRMFLT( tt[2] ) << " " << TRMFLT( tt[1] )
                      << " " << TRMFLT( tt[3] ) << " 0 0] concat\n";

        xmax = 0;
        // Dummy run through the string first to work out the
        // length, including any font changes
        cur_strp = cur_str;
        f        = 0;
        do
        {
            strp = str;

            if ( *cur_strp == esc )
            {
                cur_strp++;

                if ( *cur_strp == esc ) // <esc><esc>
                {
                    *strp++ = *cur_strp++;
                }
                else if ( *cur_strp == 'f' )
                {
                    cur_strp++;
                    if ( *cur_strp++ != 'f' )
                    {
                        // escff occurs because of logic above. But any suffix
                        // other than "f" should never happen.
                        plabort( "proc_str, internal PLplot logic error;"
                            "wrong escf escape sequence" );
                        return;
                    }
                    font   = fonts[f];
                    style  = styles[f];
                    weight = weights[f];
                    f++;
                    continue;
                }
                else
                    switch ( *cur_strp++ )
                    {
                    case 'd': //subscript
                    case 'D':
                        plP_script_scale( FALSE, &level,
                            &old_sscale, &sscale, &old_soffset, &soffset );
                        scale = sscale;
                        // The correction for the difference in magnitude
                        // between the baseline and middle coordinate systems
                        // for subscripts should be
                        // -0.5*(base font size - superscript/subscript font size).
                        dup = -0.5 * ( 1.0 - sscale );
                        up  = -font_factor * ENLARGE * ft_ht * ( RISE_FACTOR * soffset + dup );
                        break;

                    case 'u': //superscript
                    case 'U':
                        plP_script_scale( TRUE, &level,
                            &old_sscale, &sscale, &old_soffset, &soffset );
                        scale = sscale;
                        // The correction for the difference in magnitude
                        // between the baseline and middle coordinate systems
                        // for superscripts should be
                        // 0.5*(base font size - superscript/subscript font size).
                        dup = 0.5 * ( 1.0 - sscale );
                        up  = font_factor * ENLARGE * ft_ht * ( RISE_FACTOR * soffset + dup );
                        break;

                    // ignore the next sequences

                    case '+':
                    case '-':
                    case 'b':
                    case 'B':
                        plwarn( "'+', '-', and 'b/B' text escape sequences not processed." );
                        break;
                    }
            }

            // copy from current to next token, adding a postscript escape
            // char '\' if necessary
            //
            while ( *cur_strp && *cur_strp != esc )
            {
                *strp++ = *cur_strp++;
            }
            *strp = '\0';

            //	   if(fabs(up)<0.001) up = 0.; /* Watch out for small differences */

            // Set the font size
            doc->setFont( font, style, weight );
            doc->setFontSize( font_factor * ENLARGE * ft_ht * scale );
            doc->get_dimensions( (const char *) str, &lineSpacing, &xAdvance, &ymintmp, &ymaxtmp );
            xmax += xAdvance;
        } while ( *cur_strp );

        // Use the length of the string to calculate offset
        // Also used later for bounding box
        xmin = -xmax * args->just;
        xmax = xmin;
        ymin = 0;
        ymax = 0;

        // Reset parameters
        level = 0;
        scale = 1.0;
        up    = 0.0;

        // Move relative to position to account for justification
        doc->osBody() << " gsave " << TRMFLT( xmin * tt[0] ) << " " <<
            TRMFLT( xmin * tt[2] ) << " rmoveto\n";

        // Parse string for PLplot escape sequences and print everything out

        cur_strp = cur_str;
        f        = 0;
        do
        {
            strp = str;

            if ( *cur_strp == esc )
            {
                cur_strp++;

                if ( *cur_strp == esc ) // <esc><esc>
                {
                    *strp++ = *cur_strp++;
                }
                else if ( *cur_strp == 'f' )
                {
                    cur_strp++;
                    if ( *cur_strp++ != 'f' )
                    {
                        // escff occurs because of logic above. But any suffix
                        // other than "f" should never happen.
                        plabort( "proc_str, internal PLplot logic error;"
                            "wrong escf escape sequence" );
                        return;
                    }
                    font   = fonts[f];
                    style  = styles[f];
                    weight = weights[f];
                    f++;
                    //pldebug("proc_str", "string-specified fci = 0x%x, font name = %s\n", fci, font);
                    continue;
                }
                else
                    switch ( *cur_strp++ )
                    {
                    case 'd': //subscript
                    case 'D':
                        plP_script_scale( FALSE, &level,
                            &old_sscale, &sscale, &old_soffset, &soffset );
                        scale = sscale;
                        // The correction for the difference in magnitude
                        // between the baseline and middle coordinate systems
                        // for subscripts should be
                        // -0.5*(base font size - superscript/subscript font size).
                        dup = -0.5 * ( 1.0 - sscale );
                        up  = -font_factor * ENLARGE * ft_ht * ( RISE_FACTOR * soffset + dup );
                        break;

                    case 'u': //superscript
                    case 'U':
                        plP_script_scale( TRUE, &level,
                            &old_sscale, &sscale, &old_soffset, &soffset );
                        scale = sscale;
                        // The correction for the difference in magnitude
                        // between the baseline and middle coordinate systems
                        // for superscripts should be
                        // 0.5*(base font size - superscript/subscript font size).
                        dup = 0.5 * ( 1.0 - sscale );
                        up  = font_factor * ENLARGE * ft_ht * ( RISE_FACTOR * soffset + dup );
                        break;

                    // ignore the next sequences

                    case '+':
                    case '-':
                    case 'b':
                    case 'B':
                        plwarn( "'+', '-', and 'b/B' text escape sequences not processed." );
                        break;
                    }
            }

            // copy from current to next token, adding a postscript escape
            // char '\' if necessary
            //
            while ( *cur_strp && *cur_strp != esc )
            {
                *strp++ = *cur_strp++;
            }
            *strp = '\0';

            //	   if(fabs(up)<0.001) up = 0.; /* Watch out for small differences */

            // Set the font size
            doc->setFont( font, style, weight );
            doc->setFontSize( font_factor * ENLARGE * ft_ht * scale );
            doc->get_dimensions( (const char *) str, &lineSpacing, &xAdvance, &ymintmp, &ymaxtmp );
            ymin  = MIN( ymintmp + up, ymin );
            ymax  = MAX( ymaxtmp + up, ymax );
            xmax += xAdvance;

            // if up/down escape sequences, save current point and adjust baseline;
            // take the shear into account
            if ( up != 0. )
                doc->osBody() << "gsave " << TRMFLT( up * tt[1] ) << " " << TRMFLT( up * tt[3] ) << " rmoveto\n";

            // print the string
            doc->osBody() << show( (const char *) str );

            // back to baseline
            if ( up != 0. )
                doc->osBody() << "grestore " << TRMFLT( xAdvance * tt[0] ) << " " << TRMFLT( xAdvance * tt[2] ) << " rmoveto\n";
        } while ( *cur_strp );

        doc->osBody() << "grestore\n";
        doc->osBody() << "grestore\n";
        doc->osBody() << "grestore\n";

        //
        // Estimate text bounding box from LASi get_dimensions function.
        // xmin, xmax are text left and right extents,
        // ymin, ymax are top and bottom extents.
        // These need to be rotated / transformed to get the correct values
        //
        xx[0] = (PLINT) ( t[0] * xmin + t[1] * ymin );
        yy[0] = (PLINT) ( t[2] * xmin + t[3] * ymin );
        xx[1] = (PLINT) ( t[0] * xmin + t[1] * ymax );
        yy[1] = (PLINT) ( t[2] * xmin + t[3] * ymax );
        xx[2] = (PLINT) ( t[0] * xmax + t[1] * ymin );
        yy[2] = (PLINT) ( t[2] * xmax + t[3] * ymin );
        xx[3] = (PLINT) ( t[0] * xmax + t[1] * ymax );
        yy[3] = (PLINT) ( t[2] * xmax + t[3] * ymax );

        plRotPhy( ORIENTATION, 0, 0, 0, 0, &xx[0], &yy[0] );
        plRotPhy( ORIENTATION, 0, 0, 0, 0, &xx[1], &yy[1] );
        plRotPhy( ORIENTATION, 0, 0, 0, 0, &xx[2], &yy[2] );
        plRotPhy( ORIENTATION, 0, 0, 0, 0, &xx[3], &yy[3] );


        xmin = MIN( MIN( MIN( xx[0], xx[1] ), xx[2] ), xx[3] ) + args->x;
        xmax = MAX( MAX( MAX( xx[0], xx[1] ), xx[2] ), xx[3] ) + args->x;
        ymin = MIN( MIN( MIN( yy[0], yy[1] ), yy[2] ), yy[3] ) + args->y;
        ymax = MAX( MAX( MAX( yy[0], yy[1] ), yy[2] ), yy[3] ) + args->y;

        dev->llx = (int) ( MIN( dev->llx, xmin ) );
        dev->lly = (int) ( MIN( dev->lly, ymin ) );
        dev->urx = (int) ( MAX( dev->urx, xmax ) );
        dev->ury = (int) ( MAX( dev->ury, ymax ) );
//      doc->osBody() << "Z " << xmin << " " << ymin << " M "
//                    << xmin << " " << ymax << " D "
//                    << xmax << " " << ymax << " D "
//                    << xmax << " " << ymin << " D "
//                    << xmin << " " << ymin << " closepath\n"
//                    << "Z " << args->x << " " << args->y << " A closepath\n";
    }
}

//static void
//esc_purge( char *dstr, char *sstr )
//{
//    char esc;
//
//    plgesc( &esc );
//
//    while ( *sstr )
//    {
//        if ( *sstr != esc )
//        {
//            *dstr++ = *sstr++;
//            continue;
//        }
//
//        sstr++;
//        if ( *sstr == esc )
//        {
//            *dstr++ = *sstr++;
//            continue;
//        }
//
//        else
//        {
//            switch ( *sstr++ )
//            {
//            case 'f':
//                sstr++;
//                break; // two chars sequence
//
//            default:
//                break; // single char escape
//            }
//        }
//    }
//    *dstr = '\0';
//}
