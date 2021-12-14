//  Copyright (C) 1991, 1992, 1993, 1994, 1995  Geoffrey Furnish
//  Copyright (C) 1991, 1992, 1993, 1994, 1995  Maurice LeBrun
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//--------------------------------------------------------------------------
//
//  This is a metafile writer for PLplot.
//
//
#include "plDevs.h"

//#define DEBUG

#ifdef PLD_plmeta

#define NEED_PLDEBUG
#include "plplotP.h"
#include "drivers.h"
#include "metadefs.h"
#include <string.h>

// Device info
PLDLLIMPEXP_DRIVER const char* plD_DEVICE_INFO_plmeta = "plmeta:PLplot Native Meta-File:0:plmeta:26:plm\n";


void plD_dispatch_init_plm( PLDispatchTable *pdt );

void plD_init_plm( PLStream * );
void plD_line_plm( PLStream *, short, short, short, short );
void plD_polyline_plm( PLStream *, short *, short *, PLINT );
void plD_eop_plm( PLStream * );
void plD_bop_plm( PLStream * );
void plD_tidy_plm( PLStream * );
void plD_state_plm( PLStream *, PLINT );
void plD_esc_plm( PLStream *, PLINT, void * );

// Struct to hold device-specific info.

// Used for constructing error messages

#define BUFFER_LEN    256

// Function prototypes

static void WriteFileHeader( PLStream *pls );
static void UpdatePrevPagehdr( PLStream *pls );
static void WritePageInfo( PLStream *pls, FPOS_T pp_offset );
static void UpdateIndex( PLStream *pls, FPOS_T cp_offset );
static void plm_fill( PLStream *pls );
static void plm_swin( PLStream *pls );
static void plm_text( PLStream *pls, EscText *args );

// A little function to help with debugging

#ifdef DEBUG
#define DEBUG_PRINT_LOCATION( a )    PrintLocation( pls, a )

static void PrintLocation( PLStream *pls, char *tag )
{
    int isfile = ( pls->output_type == 0 );
    if ( isfile )
    {
        FILE   *file = pls->OutFile;
        FPOS_T current_offset;

        if ( pl_fgetpos( file, &current_offset ) )
            plexit( "PrintLocation (plmeta.c): fgetpos call failed" );

        pldebug( tag, "at offset %d in file %s\n",
            (int) current_offset, pls->FileName );
    }
}
#else
#define DEBUG_PRINT_LOCATION( a )
#endif

void plD_dispatch_init_plm( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "PLplot Native Meta-File";
    pdt->pl_DevName = "plmeta";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 26;
    pdt->pl_init     = (plD_init_fp) plD_init_plm;
    pdt->pl_line     = (plD_line_fp) plD_line_plm;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_plm;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_plm;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_plm;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_plm;
    pdt->pl_state    = (plD_state_fp) plD_state_plm;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_plm;
}

//--------------------------------------------------------------------------
// plD_init_plm()
//
// Initialize device.
//--------------------------------------------------------------------------

void
plD_init_plm( PLStream *pls )
{
    PLmDev *dev;

    dbug_enter( "plD_init_plm" );

    pls->color     = 1;         // Is a color device
    pls->dev_fill0 = 1;         // Handle solid fills
    pls->dev_fill1 = 1;         // Driver handles pattern fills

    if ( strncmp( PLMETA_VERSION, "2005", 4 ) == 0 )
    {
        pls->dev_text    = 0;       // Disable text handling by the driver
        pls->dev_unicode = 0;       // Disable unicode support
        pls->dev_hrshsym = 0;
    }
    else
    {
        // NOTE:  This breaks compatibility with the 2005 version of
        // the plot metafile format
        // Unicode support is not needed because the plmeta driver
        // stores the unprocessed string data that was passed to PLplot.
        // However, we turn it on to force unicode representation of the
        // plot symbols in plsym.c rather than vectorization.
        pls->dev_text    = 1;       // Enable text handling by the driver
        pls->dev_unicode = 1;       // Enable unicode support
        pls->dev_hrshsym = 0;       // Disable vectorizaton of Hershey symbols
    }

// Initialize family file info

    plFamInit( pls );

// Prompt for a file name if not already set

    plOpenFile( pls );
    pls->pdfs = pdf_finit( pls->OutFile );

// Allocate and initialize device-specific data

    pls->dev = calloc( 1, (size_t) sizeof ( PLmDev ) );
    if ( pls->dev == NULL )
        plexit( "plD_init_plm: Out of memory." );

    dev = (PLmDev *) pls->dev;

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;

    dev->xmin = 0;
    dev->xmax = PIXELS_X - 1;
    dev->ymin = 0;
    dev->ymax = PIXELS_Y - 1;

    dev->pxlx = (double) PIXELS_X / (double) LPAGE_X;
    dev->pxly = (double) PIXELS_Y / (double) LPAGE_Y;

    plP_setpxl( dev->pxlx, dev->pxly );
    plP_setphy( dev->xmin, dev->xmax, dev->ymin, dev->ymax );

// Write Metafile header.

    WriteFileHeader( pls );

// Write color map state info

    plD_state_plm( pls, PLSTATE_CMAP0 );
    plD_state_plm( pls, PLSTATE_CMAP1 );

// Write initialization command.

    DEBUG_PRINT_LOCATION( "before init" );
    plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) INITIALIZE ) );
}

//--------------------------------------------------------------------------
// plD_line_plm()
//
// Draw a line in the current color from (x1,y1) to (x2,y2).
//--------------------------------------------------------------------------

void
plD_line_plm( PLStream *pls, short x1, short y1, short x2, short y2 )
{
    PLmDev  *dev = (PLmDev *) pls->dev;
    U_SHORT xy[4];

    // dbug_enter("plD_line_plm");

    // Failsafe check

#ifdef DEBUG
    if ( x1 < dev->xmin || x1 > dev->xmax ||
         x2 < dev->xmin || x2 > dev->xmax ||
         y1 < dev->ymin || y1 > dev->ymax ||
         y2 < dev->ymin || y2 > dev->ymax )
    {
        pldebug( "plD_line_plm",
            "coordinates out of bounds -- \nActual: (%i,%i), (%i,%i) Bounds: (%i,%i,%i,%i)\n",
            x1, y1, x2, y2, dev->xmin, dev->xmax, dev->ymin, dev->ymax );
    }
#endif

// If continuation of previous line send the LINETO command, which uses
// the previous (x,y) point as it's starting location.  This results in a
// storage reduction of not quite 50%, since the instruction length for
// a LINETO is 5/9 of that for the LINE command, and given that most
// graphics applications use this command heavily.
//
// Still not quite as efficient as tektronix format since we also send the
// command each time (so shortest command is 25% larger), but a lot easier
// to implement than the tek method.
//
    if ( x1 == dev->xold && y1 == dev->yold )
    {
        plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) LINETO ) );

        xy[0] = x2;
        xy[1] = y2;
        plm_wr( pdf_wr_2nbytes( pls->pdfs, xy, 2 ) );
    }
    else
    {
        plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) LINE ) );

        xy[0] = x1;
        xy[1] = y1;
        xy[2] = x2;
        xy[3] = y2;
        plm_wr( pdf_wr_2nbytes( pls->pdfs, xy, 4 ) );
    }
    dev->xold = x2;
    dev->yold = y2;
}

//--------------------------------------------------------------------------
// plD_polyline_plm()
//
// Draw a polyline in the current color.
//--------------------------------------------------------------------------

void
plD_polyline_plm( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    PLmDev *dev = (PLmDev *) pls->dev;

    dbug_enter( "plD_polyline_plm" );

    plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) POLYLINE ) );

    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) npts ) );

    plm_wr( pdf_wr_2nbytes( pls->pdfs, (U_SHORT *) xa, npts ) );
    plm_wr( pdf_wr_2nbytes( pls->pdfs, (U_SHORT *) ya, npts ) );

    dev->xold = xa[npts - 1];
    dev->yold = ya[npts - 1];
}

//--------------------------------------------------------------------------
// plD_eop_plm()
//
// End of page.
//--------------------------------------------------------------------------

void
plD_eop_plm( PLStream *pls )
{
    plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) EOP ) );
}

//--------------------------------------------------------------------------
// plD_bop_plm()
//
// Set up for the next page.
//
// Page header layout as follows:
//
// BOP			(U_CHAR)
// page number		(U_SHORT)
// prev page offset	(U_LONG)
// next page offset	(U_LONG)
//
// Each call after the first is responsible for updating the table of
// contents and the next page offset from the previous page.
//--------------------------------------------------------------------------

void
plD_bop_plm( PLStream *pls )
{
    PLmDev *dev      = (PLmDev *) pls->dev;
    int    isfile    = ( pls->output_type == 0 );
    FPOS_T pp_offset = dev->lp_offset;

    dbug_enter( "plD_bop_plm" );

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;

// Update previous page header

    if ( isfile )
        UpdatePrevPagehdr( pls );

// Start next family file if necessary.

    pls->bytecnt = pls->pdfs->bp;
    plGetFam( pls );

// Update page counter

    pls->page++;

// Update table of contents info & write new page header.

    WritePageInfo( pls, pp_offset );
}

//--------------------------------------------------------------------------
// plD_tidy_plm()
//
// Close graphics file
//--------------------------------------------------------------------------

void
plD_tidy_plm( PLStream *pls )
{
    dbug_enter( "plD_tidy_plm" );

    plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) CLOSE ) );
    pdf_close( pls->pdfs );
    free_mem( pls->dev );
}

//--------------------------------------------------------------------------
// plD_state_plm()
//
// Handle change in PLStream state (color, pen width, fill attribute, etc).
//--------------------------------------------------------------------------

void
plD_state_plm( PLStream *pls, PLINT op )
{
    int i;

    dbug_enter( "plD_state_plm" );

    plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) CHANGE_STATE ) );
    plm_wr( pdf_wr_1byte( pls->pdfs, op ) );

    switch ( op )
    {
    case PLSTATE_WIDTH:
        plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) ( pls->width ) ) );
        break;

    case PLSTATE_COLOR0:
        plm_wr( pdf_wr_2bytes( pls->pdfs, (short) pls->icol0 ) );

        if ( pls->icol0 == PL_RGB_COLOR )
        {
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->curcolor.r ) );
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->curcolor.g ) );
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->curcolor.b ) );
        }
        break;

    case PLSTATE_COLOR1:
        plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->icol1 ) );
        break;

    case PLSTATE_FILL:
        plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) pls->patt ) );
        break;

    case PLSTATE_CMAP0:
        plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->ncol0 ) );
        for ( i = 0; i < pls->ncol0; i++ )
        {
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->cmap0[i].r ) );
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->cmap0[i].g ) );
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->cmap0[i].b ) );
        }
        break;

    case PLSTATE_CMAP1:
        plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->ncol1 ) );
        for ( i = 0; i < pls->ncol1; i++ )
        {
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->cmap1[i].r ) );
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->cmap1[i].g ) );
            plm_wr( pdf_wr_1byte( pls->pdfs, pls->cmap1[i].b ) );
        }
        break;

    case PLSTATE_CHR:
        // save the chrdef and chrht parameters
        if ( strncmp( PLMETA_VERSION, "2005", 4 ) != 0 )
        {
            plm_wr( pdf_wr_ieeef( pls->pdfs, (float) pls->chrdef ) );
            plm_wr( pdf_wr_ieeef( pls->pdfs, (float) pls->chrht ) );
        }
        break;

    case PLSTATE_SYM:
        // save the symdef and symht parameters
        if ( strncmp( PLMETA_VERSION, "2005", 4 ) != 0 )
        {
            plm_wr( pdf_wr_ieeef( pls->pdfs, (float) pls->symdef ) );
            plm_wr( pdf_wr_ieeef( pls->pdfs, (float) pls->symht ) );
        }
        break;
    }
}

//--------------------------------------------------------------------------
// plD_esc_plm()
//
// Escape function.  Note that any data written must be in device
// independent form to maintain the transportability of the metafile.
//
// Functions:
//
//	PLESC_FILL	Fill polygon
//	PLESC_SWIN	Set window parameters
//
//--------------------------------------------------------------------------

void
plD_esc_plm( PLStream *pls, PLINT op, void *ptr )
{
    dbug_enter( "plD_esc_plm" );

    plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) ESCAPE ) );
    plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) op ) );

    switch ( op )
    {
    case PLESC_FILL:
        plm_fill( pls );
        break;

    case PLESC_SWIN:
        plm_swin( pls );
        break;

    // Unicode and non-Unicode text handling
    case PLESC_HAS_TEXT:
        plm_text( pls, (EscText *) ptr );
        break;

    // Alternate unicode text handling
    case PLESC_BEGIN_TEXT:
    case PLESC_TEXT_CHAR:
    case PLESC_CONTROL_CHAR:
    case PLESC_END_TEXT:
        // NOP these for now until a decision is made
        // which method should be implemented for metafiles
        plwarn( "plmeta: Alternate Unicode text handling is not implemented" );
        break;
    }
}

//--------------------------------------------------------------------------
// Private functions
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// plm_fill()
//
// Fill polygon described in points pls->dev_x[] and pls->dev_y[].
//--------------------------------------------------------------------------

static void
plm_fill( PLStream *pls )
{
    PLmDev *dev = (PLmDev *) pls->dev;

    dbug_enter( "plm_fill" );

    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->dev_npts ) );

    plm_wr( pdf_wr_2nbytes( pls->pdfs, (U_SHORT *) pls->dev_x, pls->dev_npts ) );
    plm_wr( pdf_wr_2nbytes( pls->pdfs, (U_SHORT *) pls->dev_y, pls->dev_npts ) );

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
}

//--------------------------------------------------------------------------
// plm_swin()
//
// Set window parameters.
// Each parameter or group of parameters is tagged to make backward
// compatibility easier.
//--------------------------------------------------------------------------

static void
plm_swin( PLStream *pls )
{
    dbug_enter( "plm_swin" );
}

//--------------------------------------------------------------------------
// plm_text()
//
// Stores the text into the metafile.
//--------------------------------------------------------------------------

static void
plm_text( PLStream *pls, EscText *args )
{
    PLmDev *dev = (PLmDev *) pls->dev;
    size_t len;

    // Write state information needed to render the text

    plm_wr( pdf_wr_ieeef( pls->pdfs, pls->chrht ) );
    plm_wr( pdf_wr_ieeef( pls->pdfs, pls->diorot ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, pls->clpxmi ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, pls->clpxma ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, pls->clpymi ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, pls->clpyma ) );

    //  Write the text layout information

    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) args->base ) );
    plm_wr( pdf_wr_ieeef( pls->pdfs, (float) args->just ) );

    // Do we have a rotation shear that needs to be saved
    if ( args->xform != NULL )
    {
        plm_wr( pdf_wr_ieeef( pls->pdfs, (float) args->xform[0] ) );
        plm_wr( pdf_wr_ieeef( pls->pdfs, (float) args->xform[1] ) );
        plm_wr( pdf_wr_ieeef( pls->pdfs, (float) args->xform[2] ) );
        plm_wr( pdf_wr_ieeef( pls->pdfs, (float) args->xform[3] ) );
    }
    else
    {
        plwarn( "plmeta: transformation matrix undefined, using a guess" );
        plm_wr( pdf_wr_ieeef( pls->pdfs, (float) 1.0 ) );
        plm_wr( pdf_wr_ieeef( pls->pdfs, (float) 0.0 ) );
        plm_wr( pdf_wr_ieeef( pls->pdfs, (float) 0.0 ) );
        plm_wr( pdf_wr_ieeef( pls->pdfs, (float) 1.0 ) );
    }

    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) args->x ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) args->y ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) args->refx ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) args->refy ) );
    plm_wr( pdf_wr_1byte( pls->pdfs, (U_CHAR) args->font_face ) );
    plm_wr( pdf_wr_4bytes( pls->pdfs, (int) args->text_type ) );

    // Was a text string passed or a plot symbol?
    if ( args->text_type == PL_STRING_TEXT )
    {
        // Text string
        len = strlen( args->string );
        plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) len ) );
        if ( len > 0 )
            plm_wr( pdf_wr_string( pls->pdfs, args->string ) );
    }
    else
    {
        // Plot symbol
        plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) args->symbol ) );
    }

    // Clear the last known position
    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
}

//--------------------------------------------------------------------------
// Provide a consistent method for handling a failed fsetpos.
//--------------------------------------------------------------------------
static void
handle_fsetpos_failed( const char *where, const char *which, FPOS_T position )
{
    char buffer[BUFFER_LEN];

    // Format a standard message detailing the failure location
    snprintf( buffer, BUFFER_LEN,
        "%s: fsetpos to %s (%lu) failed",
        where,
        which,
        (unsigned long) position );

    plexit( buffer );
}

//--------------------------------------------------------------------------
// WriteFileHeader()
//
// Writes Metafile header.
//--------------------------------------------------------------------------

static void
WriteFileHeader( PLStream *pls )
{
    PLmDev *dev   = (PLmDev *) pls->dev;
    FILE   *file  = pls->OutFile;
    int    isfile = ( pls->output_type == 0 );

    dbug_enter( "WriteFileHeader(PLStream *pls" );

    plm_wr( pdf_wr_header( pls->pdfs, PLMETA_HEADER ) );
    plm_wr( pdf_wr_header( pls->pdfs, PLMETA_VERSION ) );

    // Write file index info.  Right now only number of pages.
    // The order here is critical

    if ( isfile )
    {
        // Save the position of the pages field so that it can
        // be updated when a new page is created
        if ( pl_fgetpos( file, &dev->index_offset ) )
            plexit( "WriteFileHeader: fgetpos call failed" );
    }

    plm_wr( pdf_wr_header( pls->pdfs, "pages" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) 0 ) );

    // Write initialization info.  Tag via strings to make backward
    // compatibility with old metafiles as easy as possible.

    plm_wr( pdf_wr_header( pls->pdfs, "xmin" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) dev->xmin ) );

    plm_wr( pdf_wr_header( pls->pdfs, "xmax" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) dev->xmax ) );

    plm_wr( pdf_wr_header( pls->pdfs, "ymin" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) dev->ymin ) );

    plm_wr( pdf_wr_header( pls->pdfs, "ymax" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) dev->ymax ) );

    plm_wr( pdf_wr_header( pls->pdfs, "pxlx" ) );
    plm_wr( pdf_wr_ieeef( pls->pdfs, (float) dev->pxlx ) );

    plm_wr( pdf_wr_header( pls->pdfs, "pxly" ) );
    plm_wr( pdf_wr_ieeef( pls->pdfs, (float) dev->pxly ) );

    // Geometry info, needed to properly transmit e.g. aspect ratio, via the
    // length params.  Not sure if the others are useful, but they're included
    // for completeness.

    plm_wr( pdf_wr_header( pls->pdfs, "xdpi" ) );
    plm_wr( pdf_wr_ieeef( pls->pdfs, (float) pls->xdpi ) );

    plm_wr( pdf_wr_header( pls->pdfs, "ydpi" ) );
    plm_wr( pdf_wr_ieeef( pls->pdfs, (float) pls->ydpi ) );

    plm_wr( pdf_wr_header( pls->pdfs, "xlength" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->xlength ) );

    plm_wr( pdf_wr_header( pls->pdfs, "ylength" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->ylength ) );

    plm_wr( pdf_wr_header( pls->pdfs, "xoffset" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->xoffset ) );

    plm_wr( pdf_wr_header( pls->pdfs, "yoffset" ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->yoffset ) );

    plm_wr( pdf_wr_header( pls->pdfs, "" ) );
}

//--------------------------------------------------------------------------
// WritePageInfo()
//
// Update table of contents info & write new page header.
//--------------------------------------------------------------------------

static void
WritePageInfo( PLStream *pls, FPOS_T pp_offset )
{
    PLmDev *dev   = (PLmDev *) pls->dev;
    FILE   *file  = pls->OutFile;
    int    isfile = ( pls->output_type == 0 );
    U_CHAR c;
    FPOS_T cp_offset = 0;

    // Update table of contents.

    if ( isfile )
    {
        // Get the position of this page in order to update the index
        if ( pl_fgetpos( file, &cp_offset ) )
            plexit( "WritePageInfo (plmeta.c): fgetpos call failed" );

        UpdateIndex( pls, cp_offset );
    }

    // Write new page header

    if ( dev->notfirst )
        c = BOP;
    else
    {
        c             = BOP0;
        dev->notfirst = 1;
    }
    plm_wr( pdf_wr_1byte( pls->pdfs, c ) );
    plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->page ) );
    plm_wr( pdf_wr_4bytes( pls->pdfs, (U_LONG) pp_offset ) );
    plm_wr( pdf_wr_4bytes( pls->pdfs, (U_LONG) 0 ) );

    // Update last page offset with current page value

    dev->lp_offset = cp_offset;

    // Write some page state information just to make things nice later on
    // Eventually there will be more

    plD_state_plm( pls, PLSTATE_COLOR0 );
}

//--------------------------------------------------------------------------
// UpdatePrevPagehdr()
//
// Update previous page header.
//--------------------------------------------------------------------------

static void
UpdatePrevPagehdr( PLStream *pls )
{
    PLmDev *dev      = (PLmDev *) pls->dev;
    FILE   *file     = pls->OutFile;
    FPOS_T cp_offset = 0;

    fflush( file );

    // Determine where we are

    if ( pl_fgetpos( file, &cp_offset ) )
        plexit( "plD_bop_plm: fgetpos call failed" );

    // Seek back to previous page header.

    if ( dev->lp_offset > 0 )
    {
        FPOS_T fwbyte_offset = 0;

        pldebug( "UpdatePrevPagehdr 1 (plmeta.c)",
            "Location: %d, seeking to: %d\n",
            (int) cp_offset, (int) dev->lp_offset );

        // The forward byte offset is located exactly 7 bytes after the BOP
        fwbyte_offset = dev->lp_offset + 7;
        if ( pl_fsetpos( file, &fwbyte_offset ) )
        {
            handle_fsetpos_failed( "UpdatePrevPagehdr",
                "fwbyte_offset", fwbyte_offset );
        }

        // DEBUG: verify current location

#ifdef DEBUG
        if ( pl_fgetpos( file, &fwbyte_offset ) )
            plexit( "UpdatePrevPagehdr (plmeta.c): fgetpos call failed" );

        pldebug( "UpdatePrevPagehdr 2 (plmeta.c)",
            "Now at: %d, to write: %d\n",
            (int) fwbyte_offset, (int) cp_offset );
#endif

        // Write forward byte offset into previous page header.

        plm_wr( pdf_wr_4bytes( pls->pdfs, (U_LONG) cp_offset ) );
        fflush( file );

        // DEBUG: move back to before the write & read it to verify

#ifdef DEBUG
        if ( pl_fsetpos( file, &fwbyte_offset ) )
        {
            handle_fsetpos_failed( "UpdatePrevPagehdr",
                "fwbyte_offset", fwbyte_offset );
        }
        {
            U_LONG read_offset;
            plm_rd( pdf_rd_4bytes( pls->pdfs, &read_offset ) );
            pldebug( "UpdatePrevPagehdr 3 (plmeta.c)",
                "Value read as: %d\n", read_offset );
        }
#endif

        // Return to current page offset

        if ( pl_fsetpos( file, &cp_offset ) )
        {
            handle_fsetpos_failed( "UpdatePrevPagehdr",
                "cp_offset", cp_offset );
        }
    }
}

//--------------------------------------------------------------------------
// UpdateIndex()
//
// Update file index.
//--------------------------------------------------------------------------

static void
UpdateIndex( PLStream *pls, FPOS_T cp_offset )
{
    PLmDev *dev  = (PLmDev *) pls->dev;
    FILE   *file = pls->OutFile;

    // Update file index.  Right now only number of pages.
    // The ordering here is critical

    if ( dev->index_offset > 0 )
    {
        pldebug( "UpdateIndex (plmeta.c)",
            "Location: %d, seeking to: %d\n",
            (int) cp_offset, (int) dev->lp_offset );

        if ( pl_fsetpos( file, &dev->index_offset ) )
        {
            handle_fsetpos_failed( "UpdateIndex",
                "index_offset", dev->index_offset );
        }
        plm_wr( pdf_wr_header( pls->pdfs, "pages" ) );
        plm_wr( pdf_wr_2bytes( pls->pdfs, (U_SHORT) pls->page ) );

        pldebug( "UpdateIndex (plmeta.c)",
            "Location: %d, seeking to: %d\n",
            (int) dev->lp_offset, (int) cp_offset );

        if ( pl_fsetpos( file, &cp_offset ) )
        {
            handle_fsetpos_failed( "UpdateIndex",
                "cp_offset", cp_offset );
        }
    }
}

#else
int
pldummy_plmeta()
{
    return 0;
}

#endif                          // PLD_plmeta
