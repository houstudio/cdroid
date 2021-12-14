//      PLplot driver for PDF based on the haru library http://www.libharu.org.
//
//      Copyright (C) 2006, 2008  Werner Smekal
//
//      This file is part of PLplot.
//
//      PLplot is free software; you can redistribute it and/or modify
//      it under the terms of the GNU Library General Public License as published
//      by the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      PLplot is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU Library General Public License for more details.
//
//      You should have received a copy of the GNU Library General Public License
//      along with PLplot; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//

// TODO:
// - page orientation
// - text clipping
//

//--------------------------------------------------------------------------
// Header files, defines and local variables
//--------------------------------------------------------------------------
#include "plDevs.h"

#ifdef PLD_pdf

#include <stdarg.h>
#include <math.h>
#include <setjmp.h>
#include <strings.h>

#include "hpdf.h"

// PLplot header files
//#define DEBUG
//#define NEED_PLDEBUG
#include "plplotP.h"
#include "drivers.h"
#include "plunicode-type1.h"
#include "plfci-type1.h"

// Workaround for caseless string comparison
#ifndef _WIN32
        #define stricmp     strcasecmp
        #define strnicmp    strncasecmp
#endif

// constants

// We define a virtual page and scale it down to the
// paper size chosen by the user (A4 is default).
//

// Default dimensions of the canvas (in inches) and DPI
#define CANVAS_WIDTH              ( 50.0 )
#define CANVAS_HEIGHT             ( 37.5 )
#define DEVICE_PIXELS_PER_INCH    ( 72 )

// mm per inch
#define MM_PER_INCH               ( 25.4 )

// pixels per mm
#define DEVICE_PIXELS_PER_MM      ( DEVICE_PIXELS_PER_INCH / MM_PER_INCH )

// maximum string length for own font handling
#define MAX_STRING_LEN            1000

// container for device specific data
typedef struct
{
    HPDF_Doc       pdf;
    HPDF_Page      page;
    HPDF_PageSizes pageSize;
    FILE                         *pdfFile;
    PLFLT          scalex, scaley;

    // font variables
    HPDF_Font      m_font;
    int            nlookup, if_symbol_font;
    const Unicode_to_Type1_table *lookup;
    HPDF_REAL      fontSize;
    HPDF_REAL      fontScale;
    HPDF_REAL      textWidth, textHeight;
    HPDF_REAL      yOffset;
    HPDF_REAL      textRed, textGreen, textBlue;
} pdfdev;

// local variables
PLDLLIMPEXP_DRIVER const char* plD_DEVICE_INFO_pdf = "pdf:Portable Document Format PDF:1:pdf:58:pdf\n";
static jmp_buf env;

//--------------------------------------------------------------------------
// function declarations
//--------------------------------------------------------------------------

// General
//static short desired_offset( short, double );
static void poly_line( PLStream *pls, short *xa, short *ya, PLINT npts, short fill );

// String processing
static void process_string( PLStream *, EscText * );

// PLplot interface functions
void plD_dispatch_init_pdf( PLDispatchTable *pdt );
void plD_init_pdf( PLStream * );
void plD_line_pdf( PLStream *, short, short, short, short );
void plD_polyline_pdf( PLStream *, short *, short *, PLINT );
void plD_eop_pdf( PLStream * );
void plD_bop_pdf( PLStream * );
void plD_tidy_pdf( PLStream * );
void plD_state_pdf( PLStream *, PLINT );
void plD_esc_pdf( PLStream *, PLINT, void * );
void error_handler( HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data );
void PSDrawTextToCanvas( pdfdev* dev, unsigned char* type1_string, short drawText );
void PSSetFont( pdfdev* dev, PLUNICODE fci );
void PSDrawText( pdfdev* dev, PLUNICODE* ucs4, int ucs4Len, short drawText );

//--------------------------------------------------------------------------
// error_handler( HPDF_STATUS error_no, HPDF_STATUS detail_no,
//                void *user_data )
//
// Error handler for haru library.
//--------------------------------------------------------------------------
#ifdef HPDF_DLL
void __stdcall
#else
void
#endif
error_handler( HPDF_STATUS error_no, HPDF_STATUS detail_no, void * PL_UNUSED( user_data ) )
{
    // invoke longjmp() when an error has occurred
    printf( "ERROR: error_no=%04X, detail_no=%d\n", (unsigned int) error_no, (int) detail_no );
    longjmp( env, 1 );
}


//--------------------------------------------------------------------------
// plD_dispatch_init_pdf( PLDispatchTable *pdt )
//
// Initialize device dispatch table.
//--------------------------------------------------------------------------
void plD_dispatch_init_pdf( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Portable Document Format PDF";
    pdt->pl_DevName = "pdf";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 58;
    pdt->pl_init     = (plD_init_fp) plD_init_pdf;
    pdt->pl_line     = (plD_line_fp) plD_line_pdf;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_pdf;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_pdf;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_pdf;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_pdf;
    pdt->pl_state    = (plD_state_fp) plD_state_pdf;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_pdf;
}


// driver specific options
static PLINT text       = 1;
static PLINT compress   = 1;
static PLINT hrshsym    = 1;
static PLINT color      = 1;
static char  * pageSize = NULL;

DrvOpt       pdf_options[] = {
    { "text",     DRV_INT, &text,     "Use own text routines (text=0|1)"         },
    { "color",    DRV_INT, &color,    "Use color (color=0|1)"                    },
    { "compress", DRV_INT, &compress, "Compress pdf output (compress=0|1)"       },
    { "hrshsym",  DRV_INT, &hrshsym,  "Use Hershey symbol set (hrshsym=0|1)"     },
    { "pagesize", DRV_STR, &pageSize, "Set page size (pagesize=A4|letter|A3|A5)" },
    { NULL,       DRV_INT, NULL,      NULL                                       }
};


//--------------------------------------------------------------------------
//  plD_init_pdf( PLStream *pls )
//
// Initialize device.
//--------------------------------------------------------------------------
void plD_init_pdf( PLStream *pls )
{
    pdfdev* dev;

    // allocate memory for the device storage
    dev = (pdfdev *) calloc( 1, sizeof ( pdfdev ) );
    if ( dev == NULL )
        plexit( "Insufficient memory\n" );
    pls->dev = (void *) dev;

    // Check for and set up driver options
    plParseDrvOpts( pdf_options );

    pls->termin = 0;                    // not an interactive device
    if ( color )
        pls->color = 1;                 // supports color
    else
        pls->color = 0;                 // monochrome
    pls->width   = 1;
    pls->bytecnt = 0;

    if ( text )
    {
        pls->dev_text    = 1;     // handles text
        pls->dev_unicode = 1;     // wants text as unicode
        if ( hrshsym )
            pls->dev_hrshsym = 1;
    }

    pls->page      = 0;
    pls->dev_fill0 = 1;         // supports hardware solid fills
    pls->dev_fill1 = 0;         // Use PLplot core fallback for pattern fills

    pls->graphx = GRAPHICS_MODE;

    if ( !pls->colorset )
        pls->color = 1;

    // Set the (virtual) page size. The geometry option is
    // neglected. Page sizes are set with the pagesize option.
    plspage( DEVICE_PIXELS_PER_INCH, DEVICE_PIXELS_PER_INCH,
        (PLINT) ( CANVAS_WIDTH * DEVICE_PIXELS_PER_INCH ), (PLINT) ( CANVAS_HEIGHT * DEVICE_PIXELS_PER_INCH ), 0, 0 );

    // Set up physical limits of plotting device (in drawing units)
    plP_setphy( 0, (PLINT) ( CANVAS_WIDTH * DEVICE_PIXELS_PER_INCH ),
        0, (PLINT) ( CANVAS_HEIGHT * DEVICE_PIXELS_PER_INCH ) );

    // Set the number of pixels per mm
    plP_setpxl( (PLFLT) DEVICE_PIXELS_PER_MM, (PLFLT) DEVICE_PIXELS_PER_MM );

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

    // Initialize family file info
    plFamInit( pls );

    // Prompt for a file name if not already set
    plOpenFile( pls );
    dev->pdfFile = pls->OutFile;

    dev->pdf = HPDF_New( error_handler, NULL );
    if ( !dev->pdf )
        plexit( "ERROR: cannot create pdf object.\n" );

    if ( compress )
        HPDF_SetCompressionMode( dev->pdf, HPDF_COMP_ALL );

    // determine size of pdf page - A4 is default
    dev->pageSize = HPDF_PAGE_SIZE_EOF;
    if ( pageSize == NULL )
        dev->pageSize = HPDF_PAGE_SIZE_A4;
    else if ( !stricmp( pageSize, "letter" ) )
        dev->pageSize = HPDF_PAGE_SIZE_LETTER;
    else if ( !stricmp( pageSize, "A3" ) )
        dev->pageSize = HPDF_PAGE_SIZE_A3;
    else if ( !stricmp( pageSize, "A4" ) )
        dev->pageSize = HPDF_PAGE_SIZE_A4;
    else if ( !stricmp( pageSize, "A5" ) )
        dev->pageSize = HPDF_PAGE_SIZE_A5;

    if ( dev->pageSize == HPDF_PAGE_SIZE_EOF )
        plexit( "ERROR: Unknown page size. Allowed strings are: letter, A3, A4, A5.\n" );

    if ( setjmp( env ) )
    {
        // HPDF_Free segfaults after error so skip this nicety.
        //HPDF_Free( dev->pdf );
        // can't call plexit because that appears to be circular via
        // what happens with plend.  Therefore, print out an error message
        // and exit.
        fprintf( stderr, "ERROR in haru library\n" );
        exit( 1 );
    }
}

//--------------------------------------------------------------------------
// plD_bop_pdf( PLStream *pls )
//
// Set up for the next page.
//--------------------------------------------------------------------------
void plD_bop_pdf( PLStream *pls )
{
    pdfdev    * dev = (pdfdev *) pls->dev;
    HPDF_REAL width, height;

    pls->page++;

    // add page and set size (default is A4)
    dev->page = HPDF_AddPage( dev->pdf );
    if ( pls->portrait )
        HPDF_Page_SetSize( dev->page, dev->pageSize, HPDF_PAGE_PORTRAIT );
    else
        HPDF_Page_SetSize( dev->page, dev->pageSize, HPDF_PAGE_LANDSCAPE );

    // Determine scaling parameters.
    width       = HPDF_Page_GetWidth( dev->page );  // in pixels/dots
    height      = HPDF_Page_GetHeight( dev->page ); // in pixels/dots
    dev->scalex = (PLFLT) ( width / ( CANVAS_WIDTH * DEVICE_PIXELS_PER_INCH ) );
    dev->scaley = (PLFLT) ( height / ( CANVAS_HEIGHT * DEVICE_PIXELS_PER_INCH ) );
    HPDF_Page_Concat( dev->page, (HPDF_REAL) ( dev->scalex ), 0, 0, (HPDF_REAL) ( dev->scaley ), 0, 0 );

    // Set the background by drawing a rectangle that is the size of
    // of the canvas and filling it with the background color.
    HPDF_Page_SetRGBFill( dev->page, (HPDF_REAL) ( pls->cmap0[0].r / 255.0 ),
        (HPDF_REAL) ( pls->cmap0[0].g / 255.0 ), (HPDF_REAL) ( pls->cmap0[0].b / 255.0 ) );
    width  /= (HPDF_REAL) ( dev->scalex );
    height /= (HPDF_REAL) ( dev->scaley );
    HPDF_Page_MoveTo( dev->page, (HPDF_REAL) 0.0, (HPDF_REAL) 0.0 );
    HPDF_Page_LineTo( dev->page, width, (HPDF_REAL) 0.0 );
    HPDF_Page_LineTo( dev->page, width, (HPDF_REAL) height );
    HPDF_Page_LineTo( dev->page, 0.0, (HPDF_REAL) height );
    HPDF_Page_Fill( dev->page );
}


//--------------------------------------------------------------------------
// pdf_line()
//
// Draw a line in the current color from (x1,y1) to (x2,y2).
//--------------------------------------------------------------------------
void plD_line_pdf( PLStream *pls, short x1a, short y1a, short x2a, short y2a )
{
    short xa[2], ya[2];

    xa[0] = x1a; xa[1] = x2a;
    ya[0] = y1a; ya[1] = y2a;

    poly_line( pls, xa, ya, 2, 0 );
}


//--------------------------------------------------------------------------
// pdf_polyline()
//
// Draw a polyline in the current color.
//--------------------------------------------------------------------------
void plD_polyline_pdf( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    poly_line( pls, xa, ya, npts, 0 );
}


//--------------------------------------------------------------------------
// pdf_eop()
//
// End of page
//--------------------------------------------------------------------------
void plD_eop_pdf( PLStream * PL_UNUSED( pls ) )
{
    // nothing to be done here
}


//--------------------------------------------------------------------------
// pdf_tidy()
//
// Close graphics file or otherwise clean up.
//--------------------------------------------------------------------------
void plD_tidy_pdf( PLStream *pls )
{
    pdfdev* dev = (pdfdev *) pls->dev;

    // save the document to a stream
    HPDF_SaveToStream( dev->pdf );

    // rewind the stream.
    HPDF_ResetStream( dev->pdf );

    // get the data from the stream and output it to stdout.
    for (;; )
    {
        HPDF_BYTE   buf[4096]; // TODO: not good
        HPDF_UINT32 size = 4096;
        // HPDF_STATUS ret  = HPDF_ReadFromStream( dev->pdf, buf, &size );
        HPDF_ReadFromStream( dev->pdf, buf, &size );

        if ( size == 0 )
            break;

        if ( fwrite( buf, size, 1, dev->pdfFile ) != 1 )
            plexit( "ERROR: Cannot write to file!" );
    }

    plCloseFile( pls );

    // cleanup
    HPDF_Free( dev->pdf );
}


//--------------------------------------------------------------------------
// plD_state_pdf()
//
// Handle change in PLStream state (color, pen width, fill attribute, etc).
//
// Nothing is done here because these attributes are aquired from
// PLStream for each element that is drawn.
//--------------------------------------------------------------------------
void plD_state_pdf( PLStream * PL_UNUSED( pls ), PLINT PL_UNUSED( op ) )
{
    // Nothing to be done here.
}


//--------------------------------------------------------------------------
// pdf_esc()
//
// Escape function.
//--------------------------------------------------------------------------
void plD_esc_pdf( PLStream *pls, PLINT op, void *ptr )
{
    switch ( op )
    {
    case PLESC_FILL:    // fill polygon
        poly_line( pls, pls->dev_x, pls->dev_y, pls->dev_npts, 1 );
        break;
    case PLESC_HAS_TEXT: // render text
        process_string( pls, (EscText *) ptr );
        break;
    }
}


//--------------------------------------------------------------------------
// poly_line()
//
// Handles drawing filled and unfilled polygons
//--------------------------------------------------------------------------
void poly_line( PLStream *pls, short *xa, short *ya, PLINT npts, short fill )
{
    pdfdev* dev = (pdfdev *) pls->dev;
    PLINT i;

    HPDF_Page_SetLineWidth( dev->page, (HPDF_REAL) ( pls->width ) );
    HPDF_Page_SetLineCap( dev->page, HPDF_ROUND_END );
    HPDF_Page_SetLineJoin( dev->page, HPDF_ROUND_JOIN );
    HPDF_Page_SetRGBStroke( dev->page, (HPDF_REAL) ( pls->curcolor.r / 255.0 ),
        (HPDF_REAL) ( pls->curcolor.g / 255.0 ), (HPDF_REAL) ( pls->curcolor.b / 255.0 ) );
    HPDF_Page_SetRGBFill( dev->page, (HPDF_REAL) ( pls->curcolor.r / 255.0 ),
        (HPDF_REAL) ( pls->curcolor.g / 255.0 ), (HPDF_REAL) ( pls->curcolor.b / 255.0 ) );

    HPDF_Page_MoveTo( dev->page, (HPDF_REAL) xa[0], (HPDF_REAL) ya[0] );
    for ( i = 1; i < npts; i++ )
        HPDF_Page_LineTo( dev->page, (HPDF_REAL) xa[i], (HPDF_REAL) ya[i] );

    if ( fill == 1 )
    {
        if ( pls->dev_eofill )
            HPDF_Page_EofillStroke( dev->page );
        else
            HPDF_Page_FillStroke( dev->page );
    }
    else
    {
        HPDF_Page_Stroke( dev->page );
    }
}


//--------------------------------------------------------------------------
//  unsigned char plunicode2type1 (const PLUNICODE index,
//   const Unicode_to_Type1_table lookup[], const int number_of_entries)
//
//  Function takes an input unicode index, looks through the lookup
//  table (which must be sorted by PLUNICODE Unicode), then returns the
//  corresponding Type1 code in the lookup table.  If the Unicode index
//  is not present the returned value is 32 (which is normally a blank
//  for Type 1 fonts).
//--------------------------------------------------------------------------
static unsigned char plunicode2type1( const PLUNICODE index,
                                      const Unicode_to_Type1_table lookup[],
                                      const int nlookup )
{
    int jlo = -1, jmid, jhi = nlookup;

    while ( jhi - jlo > 1 )
    {
        // Note that although jlo or jhi can be just outside valid
        // range (see initialization above) because of while condition
        // jlo < jmid < jhi and jmid must be in valid range.
        //
        jmid = ( jlo + jhi ) / 2;
        if ( index > lookup[jmid].Unicode )
            jlo = jmid;
        else if ( index < lookup[jmid].Unicode )
            jhi = jmid;
        else
            // We have found it!
            // index == lookup[jmid].Unicode
            //
            return ( lookup[jmid].Type1 );
    }
    // jlo is invalid or it is valid and index > lookup[jlo].Unicode.
    // jhi is invalid or it is valid and index < lookup[jhi].Unicode.
    // All these conditions together imply index cannot be found in lookup.
    // Mark with ' ' (which is normally the index for blank in type 1 fonts).
    //
    return ( ' ' );
}


//--------------------------------------------------------------------------
// PSDrawTextToCanvas( pdfdev* dev, unsigned char* type1_string, short drawText )
//
// This function determines the extent of the string and does
// the actual drawing to the page if drawText is true.
//--------------------------------------------------------------------------
void PSDrawTextToCanvas( pdfdev* dev, unsigned char* type1_string, short drawText )
{
    HPDF_REAL th;

    // write text to page
    if ( drawText )
    {
        HPDF_Page_BeginText( dev->page );
        HPDF_Page_SetTextRenderingMode( dev->page, HPDF_FILL );
        HPDF_Page_SetRGBFill( dev->page, dev->textRed, dev->textGreen, dev->textBlue );
        HPDF_Page_MoveTextPos( dev->page, dev->textWidth, dev->yOffset );
        HPDF_Page_ShowText( dev->page, (char *) type1_string ); // TODO: this conversion must be wrong
        HPDF_Page_EndText( dev->page );
    }

    // determine text width and height
    dev->textWidth += HPDF_Page_TextWidth( dev->page, (char *) type1_string ); // TODO: this conversion must be wrong
    th              = (HPDF_REAL) ( (HPDF_REAL) HPDF_Font_GetCapHeight( dev->m_font ) * dev->fontSize * dev->fontScale / 1000.0 );
    dev->textHeight = dev->textHeight > ( th + dev->yOffset ) ? dev->textHeight : ( th + dev->yOffset );

    // clear string
    memset( type1_string, '\0', MAX_STRING_LEN );
}


//--------------------------------------------------------------------------
// PSSetFont( pdfdev* dev, PLUNICODE fci )
//
// Sets the font.
//--------------------------------------------------------------------------
void PSSetFont( pdfdev* dev, PLUNICODE fci )
{
    const char *font;

    // fci = 0 is a special value indicating the Type 1 Symbol font
    // is desired.  This value cannot be confused with a normal FCI value
    // because it doesn't have the PL_FCI_MARK.
    if ( fci == 0 )
    {
        font                = "Symbol";
        dev->nlookup        = number_of_entries_in_unicode_to_symbol_table;
        dev->lookup         = unicode_to_symbol_lookup_table;
        dev->if_symbol_font = 1;
    }
    else
    {
        // convert the fci to Base14/Type1 font information
        font                = plP_FCI2FontName( fci, Type1Lookup, N_Type1Lookup );
        dev->nlookup        = number_of_entries_in_unicode_to_standard_table;
        dev->lookup         = unicode_to_standard_lookup_table;
        dev->if_symbol_font = 0;
    }

    if ( !( dev->m_font = HPDF_GetFont( dev->pdf, font, NULL ) ) )
        plexit( "ERROR: Couldn't open font\n" );
    //pldebug( "PSSetFont", "HPDF requested font size = %f\n", dev->fontSize * dev->fontScale );
    HPDF_Page_SetFontAndSize( dev->page, dev->m_font, dev->fontSize * dev->fontScale );
}

// 0.8 should mimic the offset of first superscript/subscript level
// implemented in plstr (plsym.c) for Hershey fonts.  However, when
// comparing with -dev xwin and -dev xcairo results changing this
// factor to 0.6 appears to offset the centers of the letters
// appropriately while 0.8 gives much poorer agreement with the
// other devices.
# define RISE_FACTOR    0.6

//--------------------------------------------------------------------------
// PSDrawText( pdfdev* dev, PLUNICODE* ucs4, int ucs4Len, short drawText )
//
// This function is called twice, first to determine the extent of the
// text written to the page and then a second time to actually draw
// the text.
//--------------------------------------------------------------------------
void PSDrawText( pdfdev* dev, PLUNICODE* ucs4, int ucs4Len, short drawText )
{
    int           i, s;
    unsigned char type1_string[MAX_STRING_LEN];
    char          plplotEsc;
    PLUNICODE     fci;
    int           last_chance = 0;
    PLFLT         old_sscale, sscale, old_soffset, soffset, dup;
    PLINT         level = 0;

    memset( type1_string, '\0', MAX_STRING_LEN );

    // Get PLplot escape character
    plgesc( &plplotEsc );

    // Get the current font
    dev->fontScale = 1.0;
    dev->yOffset   = 0.0;
    plgfci( &fci );
    PSSetFont( dev, fci );
    dev->textWidth  = 0;
    dev->textHeight = 0;

    i = 0; s = 0;
    while ( i < ucs4Len )
    {
        if ( ucs4[i] < PL_FCI_MARK )                // not a font change
        {
            if ( ucs4[i] != (PLUNICODE) plplotEsc ) // a character to display
            {
                type1_string[s] = plunicode2type1( ucs4[i], dev->lookup, dev->nlookup );
                if ( ucs4[i] != ' ' && type1_string[s] == ' ' )
                {
                    // failed lookup
                    if ( !dev->if_symbol_font )
                    {
                        // failed standard font lookup.  Try "last chance"
                        // symbol font instead.
                        type1_string[s] = '\0';
                        PSDrawTextToCanvas( dev, type1_string, drawText );
                        s           = 0;
                        last_chance = 1;
                        PSSetFont( dev, 0 );
                        continue;
                    }
                    else if ( !last_chance )
                    {
                        // failed symbol font lookup that is not right
                        // after a failed standard font lookup (i.e.,
                        // last_change = 0).  Try standard fonts lookup instead.
                        type1_string[s] = '\0';
                        PSDrawTextToCanvas( dev, type1_string, drawText );
                        s           = 0;
                        last_chance = 0;
                        PSSetFont( dev, fci );
                        continue;
                    }
                    else
                    {
                        // failed "last_chance" symbol font lookup that
                        // has occurred right after a failed standard
                        // fonts lookup.  Just accept blank result and
                        // move on using standard fonts.
                        PSDrawTextToCanvas( dev, type1_string, drawText );
                        s           = 0;
                        last_chance = 0;
                        PSSetFont( dev, fci );
                        i++;
                        continue;
                    }
                }
                else
                {
                    // font lookup succeeded.
                    s++;
                    i++;
                    last_chance = 0;
                    continue;
                }
            }
            i++;
            if ( ucs4[i] == (PLUNICODE) plplotEsc ) // a escape character to display
            {
                type1_string[s] = plunicode2type1( ucs4[i], dev->lookup, dev->nlookup );
                if ( ucs4[i] != ' ' && type1_string[s] == ' ' )
                {
                    // failed lookup
                    if ( !dev->if_symbol_font )
                    {
                        // failed standard font lookup.  Try "last chance"
                        // symbol font instead.
                        type1_string[s] = '\0';
                        PSDrawTextToCanvas( dev, type1_string, drawText );
                        s           = 0;
                        last_chance = 1;
                        PSSetFont( dev, 0 );
                        continue;
                    }
                    else if ( !last_chance )
                    {
                        // failed symbol font lookup that is not right
                        // after a failed standard font lookup (i.e.,
                        // last_change = 0).  Try standard fonts lookup instead.
                        type1_string[s] = '\0';
                        PSDrawTextToCanvas( dev, type1_string, drawText );
                        s           = 0;
                        last_chance = 0;
                        PSSetFont( dev, fci );
                        continue;
                    }
                    else
                    {
                        // failed "last_chance" symbol font lookup that
                        // has occurred right after a failed standard
                        // fonts lookup.  Just accept blank result and
                        // move on using standard fonts.
                        PSDrawTextToCanvas( dev, type1_string, drawText );
                        s           = 0;
                        last_chance = 0;
                        PSSetFont( dev, fci );
                        i++;
                        continue;
                    }
                }
                else
                {
                    // font lookup succeeded.
                    s++;
                    i++;
                    last_chance = 0;
                    continue;
                }
            }
            else
            {
                if ( ucs4[i] == (PLUNICODE) 'u' ) // Superscript
                {
                    // draw string so far
                    PSDrawTextToCanvas( dev, type1_string, drawText );
                    s = 0;

                    plP_script_scale( TRUE, &level,
                        &old_sscale, &sscale, &old_soffset, &soffset );
                    // The correction for the difference in magnitude
                    // between the baseline and middle coordinate systems
                    // for superscripts should be
                    // 0.5*(base font size - superscript/subscript font size).
                    dup            = 0.5 * ( 1.0 - sscale );
                    dev->fontScale = (HPDF_REAL) sscale;
                    PSSetFont( dev, fci );
                    dev->yOffset = (HPDF_REAL) ( dev->fontSize * ( soffset * RISE_FACTOR + dup ) );
                }
                if ( ucs4[i] == (PLUNICODE) 'd' ) // Subscript
                {
                    // draw string so far
                    PSDrawTextToCanvas( dev, type1_string, drawText );
                    s = 0;

                    plP_script_scale( FALSE, &level,
                        &old_sscale, &sscale, &old_soffset, &soffset );
                    // The correction for the difference in magnitude
                    // between the baseline and middle coordinate systems
                    // for subcripts should be
                    // 0.5*(base font size - superscript/subscript font size).
                    dup            = -0.5 * ( 1.0 - sscale );
                    dev->fontScale = (HPDF_REAL) sscale;
                    PSSetFont( dev, fci );
                    dev->yOffset = (HPDF_REAL) ( -dev->fontSize * ( soffset * RISE_FACTOR + dup ) );
                }
                if ( ucs4[i] == (PLUNICODE) '-' ) // underline
                {                                 // draw string so far
                    PSDrawTextToCanvas( dev, type1_string, drawText );
                    s = 0;

                    // dev->underlined = !dev->underlined;
                    PSSetFont( dev, fci );
                }
                if ( ucs4[i] == (PLUNICODE) '+' ) // overline
                {                                 // not implemented yet
                }
                i++;
            }
        }
        else // a font change
        {
            // draw string so far
            PSDrawTextToCanvas( dev, type1_string, drawText );
            s = 0;

            // get new font
            fci = ucs4[i];
            PSSetFont( dev, fci );
            i++;
        }
    }

    PSDrawTextToCanvas( dev, type1_string, drawText );
}


//--------------------------------------------------------------------------
// process_string( PLStream* pls, EscText* args )
//
// Handles the output of the text on the page.
//--------------------------------------------------------------------------
void process_string( PLStream* pls, EscText* args )
{
    pdfdev    * dev = (pdfdev *) pls->dev;
    PLFLT     rotation, shear, stride;
    HPDF_REAL cos_rot, sin_rot, cos_shear, sin_shear;

    // Check that we got unicode, warning message and return if not
    if ( args->unicode_array_len == 0 )
    {
        printf( "Non unicode string passed to a pdf driver, ignoring\n" );
        return;
    }

    // Check that unicode string isn't longer then the max we allow
    if ( args->unicode_array_len >= MAX_STRING_LEN )
    {
        printf( "Sorry, the pdf drivers only handles strings of length < %d\n", MAX_STRING_LEN );
        return;
    }

    // Calculate the font size (in pixels)
    dev->fontSize = (HPDF_REAL) ( pls->chrht * DEVICE_PIXELS_PER_INCH / 25.4 * 1.6 );

    // text color
    dev->textRed   = (HPDF_REAL) ( pls->curcolor.r / 255.0 );
    dev->textGreen = (HPDF_REAL) ( pls->curcolor.g / 255.0 );
    dev->textBlue  = (HPDF_REAL) ( pls->curcolor.b / 255.0 );

    // calculate transformation matrix (rotation and shear of text)
    plRotationShear( args->xform, &rotation, &shear, &stride );
    rotation -= pls->diorot * M_PI / 2.0;
    cos_rot   = (HPDF_REAL) cos( rotation );
    sin_rot   = (HPDF_REAL) sin( rotation );
    cos_shear = (HPDF_REAL) cos( shear );
    sin_shear = (HPDF_REAL) sin( shear );

    // calculate text extend -> stored in dev->textWidth and dev->textHeight
    PSDrawText( dev, args->unicode_array, args->unicode_array_len, 0 );

    // apply transformation matrix and draw text
    HPDF_Page_GSave( dev->page );
    HPDF_Page_Concat( dev->page, cos_rot, sin_rot,
        -cos_rot * sin_shear - sin_rot * cos_shear,
        -sin_rot * sin_shear + cos_rot * cos_shear,
        (HPDF_REAL) ( args->x ), (HPDF_REAL) ( args->y ) );
    HPDF_Page_Concat( dev->page, (HPDF_REAL) 1.0, (HPDF_REAL) 0.0, (HPDF_REAL) 0.0, (HPDF_REAL) 1.0,
        (HPDF_REAL) ( -args->just * dev->textWidth ), (HPDF_REAL) ( -0.5 * dev->textHeight ) );
    PSDrawText( dev, args->unicode_array, args->unicode_array_len, 1 );
    HPDF_Page_GRestore( dev->page );
}

#else

//--------------------------------------------------------------------------
// pldummy_pdf()
//
// Dummy function if driver should not be available.
//--------------------------------------------------------------------------
int pldummy_pdf()
{
    return 0;
}

#endif                          // PLD_pdf

