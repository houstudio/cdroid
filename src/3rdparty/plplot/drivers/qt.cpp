//
//
// This software is provided under the LGPL in March 2009 by the
// Cluster Science Centre
// QSAS team,
// Imperial College, London
//
// Copyright (C) 2009 Imperial College, London
// Copyright (C) 2009-2019 Alan W. Irwin
// Copyright (C) 2009 Werner Smekal
// Copyright (C) 2009-2012 Andrew Ross
// Copyright (C) 2009-2011 Hazen Babcock
// Copyright (C) 2010 Hezekiah M. Carty
// Copyright (C) 2015 Jim Dishaw
// Copyright (C) 2019 António R. Tomé
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Lesser Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// To received a copy of the GNU Library General Public License
// write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// History:
//
//
// March 2009:  v1.00
// Initial release.
//
//


#include "qt.h"
#include <QMutexLocker>

// Global variables used in initQtApp (which is called by all
// plD_init_* routines other than plD_init_extqt) and used in
// closeQtApp (which is called by all plD_tidy_* routines other than
// plD_tidy_extqt).  The use of these global variables is kept thread-safe by
// a QMutexLocker used by both initQtApp and closeQtApp.
static int  argc;
static char **argv;
static int  internal_qAppCounter = 0;
static bool internal_qApp        = false;

// Drivers declaration
extern "C" {
PLDLLIMPEXP_DRIVER_DATA( const char* ) plD_DEVICE_INFO_qt =
#if defined ( PLD_bmpqt )
    "bmpqt:Qt Windows bitmap driver:0:qt:66:bmpqt\n"
#endif
#if defined ( PLD_jpgqt )
    "jpgqt:Qt jpg driver:0:qt:67:jpgqt\n"
#endif
#if defined ( PLD_pngqt )
    "pngqt:Qt png driver:0:qt:68:pngqt\n"
#endif
#if defined ( PLD_ppmqt )
    "ppmqt:Qt ppm driver:0:qt:69:ppmqt\n"
#endif
#if defined ( PLD_tiffqt )
    "tiffqt:Qt tiff driver:0:qt:70:tiffqt\n"
#endif
#if defined ( PLD_svgqt ) && QT_VERSION >= 0x040300
    "svgqt:Qt SVG driver:0:qt:71:svgqt\n"
#endif
#if defined ( PLD_qtwidget )
    "qtwidget:Qt Widget:1:qt:72:qtwidget\n"
#endif
#if defined ( PLD_epsqt )
    "epsqt:Qt EPS driver:0:qt:73:epsqt\n"
#endif
#if defined ( PLD_pdfqt )
    "pdfqt:Qt PDF driver:0:qt:74:pdfqt\n"
#endif
#if defined ( PLD_extqt )
    "extqt:External Qt driver:0:qt:75:extqt\n"
#endif
#if defined ( PLD_memqt )
    "memqt:Memory Qt driver:0:qt:76:memqt\n"
#endif
;
}  // extern "C"

static DrvOpt qt_options[] = { { "text_vectorize",     DRV_INT, &vectorize, "Vectorize fonts on output (0|1)"     },
                               { "lines_antialiasing", DRV_INT, &lines_aa,  "Toggles antialiasing on lines (0|1)" },
                               { NULL,                 DRV_INT, NULL,       NULL                                  } };

// The purpose of this routine is to create an internal qApp and the argc and argv data
// that qApp needs if an external or internal qApp does not exist already.

bool initQtApp( bool isGUI )
{
    QMutexLocker locker( &QtPLDriver::mutex );
    bool         res = false;

    if ( qApp == NULL && internal_qAppCounter == 0 )
    {
        internal_qApp = true;
        argc          = 1;
        argv          = new char*[2];
        argv[0]       = new char[10];
        argv[1]       = new char[1];
        snprintf( argv[0], 10, "qt_driver" );
        argv[1][0] = '\0';
#ifdef Q_WS_X11
        // On X11 if DISPLAY is not set then cannot open GUI. This allows non-interactive devices to still work in this case.
        if ( getenv( "DISPLAY" ) == NULL )
            isGUI = false;
#endif
        new QApplication( argc, argv, isGUI );
        res = true;
    }

    // Keep track of the number of uses of the internal qApp.
    if ( internal_qApp )
        ++internal_qAppCounter;

    return res;
}

// The purpose of this routine is to delete qApp and associated argv
// if those were created internally by initQtApp and if it is the last
// use (kept track of by internal_qAppCounter) of that qApp.
void closeQtApp()
{
    QMutexLocker locker( &QtPLDriver::mutex );
    if ( internal_qApp )
    {
        --internal_qAppCounter;
        if ( qApp != NULL && internal_qAppCounter == 0 )
        {
            delete qApp;
            delete[] argv[0];
            delete[] argv[1];
            delete[] argv;
            argv = NULL;
        }
    }
}

//--------------------------------------------------------------------------
// qt_family_check ()
//
// support function to help supress more than one page if family file
// output not specified by the user  (e.g., with the -fam command-line option).
// Adapted directly from svg.c
//--------------------------------------------------------------------------
static int already_warned = 0;
static int qt_family_check( PLStream *pls )
{
    if ( pls->family || pls->page == 1 )
    {
        return 0;
    }
    else
    {
        if ( !already_warned )
        {
            already_warned = 1;
            plwarn( "All pages after the first skipped because family file output not specified.\n" );
        }
        return 1;
    }
}

// Declaration of the driver-specific interface functions
#if defined ( PLD_bmpqt ) || defined ( PLD_jpgqt ) || defined ( PLD_pngqt ) || defined ( PLD_ppmqt ) || defined ( PLD_tiffqt ) || defined ( PLD_memqt )
void plD_init_rasterqt( PLStream * );
void plD_eop_rasterqt( PLStream * );
void plD_line_rasterqt( PLStream *, short, short, short, short );
void plD_polyline_rasterqt( PLStream *, short*, short*, PLINT );
void plD_tidy_rasterqt( PLStream * );
void plD_state_rasterqt( PLStream *, PLINT );
void plD_esc_rasterqt( PLStream *, PLINT, void* );
#endif

#if defined ( PLD_bmpqt )
void plD_dispatch_init_bmpqt( PLDispatchTable *pdt );
void plD_bop_bmpqt( PLStream * );
#endif

#if defined ( PLD_jpgqt )
void plD_dispatch_init_jpgqt( PLDispatchTable *pdt );
void plD_bop_jpgqt( PLStream * );
#endif

#if defined ( PLD_pngqt )
void plD_dispatch_init_pngqt( PLDispatchTable *pdt );
void plD_bop_pngqt( PLStream * );
#endif

#if defined ( PLD_ppmqt )
void plD_dispatch_init_ppmqt( PLDispatchTable *pdt );
void plD_bop_ppmqt( PLStream * );
#endif

#if defined ( PLD_tiffqt )
void plD_dispatch_init_tiffqt( PLDispatchTable *pdt );
void plD_bop_tiffqt( PLStream * );
#endif

#if defined ( PLD_svgqt ) && QT_VERSION >= 0x040300
void plD_dispatch_init_svgqt( PLDispatchTable *pdt );
void plD_init_svgqt( PLStream * );
void plD_bop_svgqt( PLStream * );
void plD_eop_svgqt( PLStream * );
void plD_line_svgqt( PLStream *, short, short, short, short );
void plD_polyline_svgqt( PLStream *, short*, short*, PLINT );
void plD_tidy_svgqt( PLStream * );
void plD_state_svgqt( PLStream *, PLINT );
void plD_esc_svgqt( PLStream *, PLINT, void* );
#endif

#if defined ( PLD_epsqt ) || defined ( PLD_pdfqt )
void plD_init_epspdfqt( PLStream * );
void plD_bop_epspdfqt_helper( PLStream *, int ifeps );
void plD_eop_epspdfqt( PLStream * );
void plD_line_epspdfqt( PLStream *, short, short, short, short );
void plD_polyline_epspdfqt( PLStream *, short*, short*, PLINT );
void plD_tidy_epspdfqt( PLStream * );
void plD_state_epspdfqt( PLStream *, PLINT );
void plD_esc_epspdfqt( PLStream *, PLINT, void* );
#endif
#if defined ( PLD_epsqt )
void plD_dispatch_init_epsqt( PLDispatchTable *pdt );
void plD_bop_epsqt( PLStream * );
#endif
#if defined ( PLD_pdfqt )
void plD_dispatch_init_pdfqt( PLDispatchTable *pdt );
void plD_bop_pdfqt( PLStream * );
#endif

#if defined ( PLD_qtwidget )
void plD_dispatch_init_qtwidget( PLDispatchTable *pdt );
void plD_init_qtwidget( PLStream * );
void plD_eop_qtwidget( PLStream * );
void plD_wait_qtwidget( PLStream * );
void plD_line_qtwidget( PLStream *, short, short, short, short );
void plD_polyline_qtwidget( PLStream *, short*, short*, PLINT );
void plD_tidy_qtwidget( PLStream * );
void plD_state_qtwidget( PLStream *, PLINT );
void plD_esc_qtwidget( PLStream *, PLINT, void* );
void plD_bop_qtwidget( PLStream * );
#endif

#if defined ( PLD_extqt )
void plD_dispatch_init_extqt( PLDispatchTable *pdt );
void plD_init_extqt( PLStream * );
void plD_eop_extqt( PLStream * );
void plD_line_extqt( PLStream *, short, short, short, short );
void plD_polyline_extqt( PLStream *, short*, short*, PLINT );
void plD_tidy_extqt( PLStream * );
void plD_state_extqt( PLStream *, PLINT );
void plD_esc_extqt( PLStream *, PLINT, void* );
void plD_bop_extqt( PLStream * );
#endif

#if defined ( PLD_memqt )
void plD_dispatch_init_memqt( PLDispatchTable *pdt );
void plD_init_memqt( PLStream * );
void plD_bop_memqt( PLStream * );
void plD_eop_memqt( PLStream * );
#endif

////////////////// Raster driver-specific definitions: class and interface functions /////////
#if defined ( PLD_bmpqt ) || defined ( PLD_jpgqt ) || defined ( PLD_pngqt ) || defined ( PLD_ppmqt ) || defined ( PLD_tiffqt ) || defined ( PLD_memqt )
void plD_init_rasterqt( PLStream * pls )
{
    double dpi;

    vectorize = 0;
    lines_aa  = 1;
    plParseDrvOpts( qt_options );

    // Stream setup
    pls->color        = 1;
    pls->plbuf_write  = 0;
    pls->dev_fill0    = 1;
    pls->dev_fill1    = 0;
    pls->dev_gradient = 1;      // driver renders gradient
    // Let the PLplot core handle dashed lines since
    // the driver results for this capability have a number of issues.
    // pls->dev_dash=1;
    pls->dev_dash  = 0;
    pls->dev_flush = 1;
    // Driver does not have a clear capability so use (good) PLplot core
    // fallback for that instead.
    pls->dev_clear         = 0;
    pls->termin            = 0;
    pls->page              = 0;
    pls->dev_text          = 1; // want to draw text
    pls->dev_unicode       = 1; // want unicode
    pls->has_string_length = 1; // Driver supports string length calculations

    // Needs to be true only because of multi-stream case
    bool isMaster = initQtApp( true );

    if ( pls->xdpi <= 0. )
        dpi = DEFAULT_DPI;
    else
        dpi = pls->xdpi;

    // Shamelessly copied on the Cairo stuff :)
    if ( pls->xlength <= 0 || pls->ylength <= 0 )
    {
        pls->dev     = new QtRasterDevice;
        pls->xlength = (PLINT) ( ( (QtRasterDevice *) ( pls->dev ) )->m_dWidth );
        pls->ylength = (PLINT) ( ( (QtRasterDevice *) ( pls->dev ) )->m_dHeight );
    }
    else
    {
        pls->dev = new QtRasterDevice( pls->xlength, pls->ylength );
    }
    ( (QtRasterDevice *) pls->dev )->setPLStream( pls );

    if ( isMaster )
        handler.setMasterDevice( (QtRasterDevice *) ( pls->dev ) );

    if ( pls->xlength > pls->ylength )
        ( (QtRasterDevice *) ( pls->dev ) )->downscale = (PLFLT) pls->xlength / (PLFLT) ( PIXELS_X - 1 );
    else
        ( (QtRasterDevice *) ( pls->dev ) )->downscale = (PLFLT) pls->ylength / (PLFLT) PIXELS_Y;

    plP_setphy( (PLINT) 0, (PLINT) ( pls->xlength / ( (QtRasterDevice *) ( pls->dev ) )->downscale ), (PLINT) 0, (PLINT) ( pls->ylength / ( (QtRasterDevice *) ( pls->dev ) )->downscale ) );

    plP_setpxl( dpi / 25.4 / ( (QtRasterDevice *) ( pls->dev ) )->downscale, dpi / 25.4 / ( (QtRasterDevice *) ( pls->dev ) )->downscale );

    ( (QtRasterDevice *) ( pls->dev ) )->setResolution( dpi );

    // Initialize family file info
    plFamInit( pls );

    plOpenFile( pls );
}

void plD_eop_rasterqt( PLStream *pls )
{
    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtRasterDevice *) pls->dev )->savePlot();
    handler.DeviceChangedPage( (QtRasterDevice *) pls->dev );
}

void plD_line_rasterqt( PLStream * pls, short x1a, short y1a, short x2a, short y2a )
{
    QtRasterDevice* widget = (QtRasterDevice *) pls->dev;

    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    widget->QtPLDriver::setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawLine( x1a, y1a, x2a, y2a );
}

void plD_polyline_rasterqt( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    QtRasterDevice * widget = (QtRasterDevice *) pls->dev;

    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    widget->QtPLDriver::setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawPolyline( xa, ya, npts );
}

void plD_esc_rasterqt( PLStream * pls, PLINT op, void* ptr )
{
    short          *xa, *ya;
    unsigned char  *r, *g, *b;
    PLFLT          *alpha;
    PLINT          i;
    QtRasterDevice * widget = (QtRasterDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    switch ( op )
    {
    //     case PLESC_DASH:
    //       widget->setDashed(pls->nms, pls->mark, pls->space);
    //       widget->QtPLDriver::setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
    //       widget->drawPolyline(pls->dev_x, pls->dev_y, pls->dev_npts);
    //       widget->setSolid();
    //       break;

    case PLESC_FILL:
        xa = new short[pls->dev_npts];
        ya = new short[pls->dev_npts];

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->QtPLDriver::setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        break;

    case PLESC_GRADIENT:
        xa    = new short[pls->dev_npts];
        ya    = new short[pls->dev_npts];
        r     = new unsigned char[pls->ncol1];
        g     = new unsigned char[pls->ncol1];
        b     = new unsigned char[pls->ncol1];
        alpha = new PLFLT[pls->ncol1];

        for ( i = 0; i < pls->ncol1; i++ )
        {
            r[i]     = pls->cmap1[i].r;
            g[i]     = pls->cmap1[i].g;
            b[i]     = pls->cmap1[i].b;
            alpha[i] = pls->cmap1[i].a;
        }
        widget->setGradient( pls->xgradient[0], pls->xgradient[1], pls->ygradient[0], pls->ygradient[1], r, g, b, alpha, pls->ncol1 );

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        delete[] r;
        delete[] g;
        delete[] b;
        delete[] alpha;
        break;

    case PLESC_HAS_TEXT:
        //  call the generic ProcessString function
        //  ProcessString( pls, (EscText *)ptr );
        widget->QtPLDriver::setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawText( (EscText *) ptr );
        break;

    default: break;
    }
}

void plD_state_rasterqt( PLStream * pls, PLINT op )
{
    QtRasterDevice * widget = (QtRasterDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    switch ( op )
    {
    case PLSTATE_WIDTH:
        widget->setWidthF( pls->width );
        break;

    case PLSTATE_COLOR0:
        ( (QtPLDriver *) widget )->QtPLDriver::setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;

    case PLSTATE_COLOR1:
        ( (QtPLDriver *) widget )->QtPLDriver::setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;


    default: break;
    }
}

void plD_tidy_rasterqt( PLStream * pls )
{
    QtRasterDevice * widget = (QtRasterDevice *) pls->dev;

    if ( widget != NULL )
    {
        handler.DeviceClosed( widget );
        delete widget;
        pls->dev = NULL;
    }
    plCloseFile( pls );

    closeQtApp();
}
#endif

#if defined ( PLD_bmpqt )
void plD_dispatch_init_bmpqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt Windows bitmap Driver";
    pdt->pl_DevName = "bmpqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 66;
    pdt->pl_init     = (plD_init_fp) plD_init_rasterqt;
    pdt->pl_line     = (plD_line_fp) plD_line_rasterqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_rasterqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_rasterqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_bmpqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_rasterqt;
    pdt->pl_state    = (plD_state_fp) plD_state_rasterqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_rasterqt;
}

void plD_bop_bmpqt( PLStream *pls )
{
    // Plot familying stuff. Not really understood, just copying gd.c
    plGetFam( pls );

    pls->famadv = 1;
    pls->page++;
    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtRasterDevice *) pls->dev )->definePlotName( pls->FileName, "BMP" );
    ( (QtRasterDevice *) pls->dev )->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}
#endif

#if defined ( PLD_jpgqt )
void plD_dispatch_init_jpgqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt jpg Driver";
    pdt->pl_DevName = "jpgqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 67;
    pdt->pl_init     = (plD_init_fp) plD_init_rasterqt;
    pdt->pl_line     = (plD_line_fp) plD_line_rasterqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_rasterqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_rasterqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_jpgqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_rasterqt;
    pdt->pl_state    = (plD_state_fp) plD_state_rasterqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_rasterqt;
}

void plD_bop_jpgqt( PLStream *pls )
{
    // Plot familying stuff. Not really understood, just copying gd.c
    plGetFam( pls );

    pls->famadv = 1;
    pls->page++;
    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtRasterDevice *) pls->dev )->definePlotName( pls->FileName, "JPG" );
    ( (QtRasterDevice *) pls->dev )->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}
#endif

#if defined ( PLD_pngqt )
void plD_dispatch_init_pngqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt png Driver";
    pdt->pl_DevName = "pngqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 68;
    pdt->pl_init     = (plD_init_fp) plD_init_rasterqt;
    pdt->pl_line     = (plD_line_fp) plD_line_rasterqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_rasterqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_rasterqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_pngqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_rasterqt;
    pdt->pl_state    = (plD_state_fp) plD_state_rasterqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_rasterqt;
}

void plD_bop_pngqt( PLStream *pls )
{
    // Plot familying stuff. Not really understood, just copying gd.c
    plGetFam( pls );

    pls->famadv = 1;
    pls->page++;
    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtRasterDevice *) pls->dev )->definePlotName( pls->FileName, "PNG" );
    ( (QtRasterDevice *) pls->dev )->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}
#endif

#if defined ( PLD_ppmqt )
void plD_dispatch_init_ppmqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt ppm Driver";
    pdt->pl_DevName = "ppmqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 69;
    pdt->pl_init     = (plD_init_fp) plD_init_rasterqt;
    pdt->pl_line     = (plD_line_fp) plD_line_rasterqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_rasterqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_rasterqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_ppmqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_rasterqt;
    pdt->pl_state    = (plD_state_fp) plD_state_rasterqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_rasterqt;
}

void plD_bop_ppmqt( PLStream *pls )
{
    // Plot familying stuff. Not really understood, just copying gd.c
    plGetFam( pls );

    pls->famadv = 1;
    pls->page++;
    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtRasterDevice *) pls->dev )->definePlotName( pls->FileName, "PPM" );
    ( (QtRasterDevice *) pls->dev )->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}
#endif

#if defined ( PLD_tiffqt )
void plD_dispatch_init_tiffqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt tiff Driver";
    pdt->pl_DevName = "tiffqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 70;
    pdt->pl_init     = (plD_init_fp) plD_init_rasterqt;
    pdt->pl_line     = (plD_line_fp) plD_line_rasterqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_rasterqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_rasterqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_tiffqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_rasterqt;
    pdt->pl_state    = (plD_state_fp) plD_state_rasterqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_rasterqt;
}

void plD_bop_tiffqt( PLStream *pls )
{
    // Plot familying stuff. Not really understood, just copying gd.c
    plGetFam( pls );

    pls->famadv = 1;
    pls->page++;
    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtRasterDevice *) pls->dev )->definePlotName( pls->FileName, "TIFF" );
    ( (QtRasterDevice *) pls->dev )->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}
#endif

#if defined ( PLD_svgqt ) && QT_VERSION >= 0x040300
void plD_dispatch_init_svgqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt SVG Driver";
    pdt->pl_DevName = "svgqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 71;
    pdt->pl_init     = (plD_init_fp) plD_init_svgqt;
    pdt->pl_line     = (plD_line_fp) plD_line_svgqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_svgqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_svgqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_svgqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_svgqt;
    pdt->pl_state    = (plD_state_fp) plD_state_svgqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_svgqt;
}

void plD_init_svgqt( PLStream * pls )
{
    vectorize = 1;
    lines_aa  = 1;
    plParseDrvOpts( qt_options );

    // Stream setup
    pls->color        = 1;
    pls->plbuf_write  = 0;
    pls->dev_fill0    = 1;
    pls->dev_fill1    = 0;
    pls->dev_gradient = 1;      // driver renders gradient
    // Let the PLplot core handle dashed lines since
    // the driver results for this capability have a number of issues.
    // pls->dev_dash=1;
    pls->dev_dash  = 0;
    pls->dev_flush = 1;
    // Driver does not have a clear capability so use (good) PLplot core
    // fallback for that instead.
    pls->dev_clear         = 0;
    pls->termin            = 0;
    pls->page              = 0;
    pls->dev_text          = 1; // want to draw text
    pls->dev_unicode       = 1; // want unicode
    pls->has_string_length = 1; // Driver supports string length calculations

    // Needs to be true only because of multi-stream case
    bool isMaster = initQtApp( true );

    if ( pls->xlength <= 0 || pls->ylength <= 0 )
    {
        pls->dev     = new QtSVGDevice;
        pls->xlength = (int) ( ( (QtSVGDevice *) ( pls->dev ) )->m_dWidth );
        pls->ylength = (int) ( ( (QtSVGDevice *) ( pls->dev ) )->m_dHeight );
    }
    else
    {
        pls->dev = new QtSVGDevice( pls->xlength, pls->ylength );
    }
    ( (QtSVGDevice *) pls->dev )->setPLStream( pls );

    if ( isMaster )
        handler.setMasterDevice( (QtSVGDevice *) ( pls->dev ) );

    if ( pls->xlength > pls->ylength )
        ( (QtSVGDevice *) ( pls->dev ) )->downscale = (PLFLT) pls->xlength / (PLFLT) ( PIXELS_X - 1 );
    else
        ( (QtSVGDevice *) ( pls->dev ) )->downscale = (PLFLT) pls->ylength / (PLFLT) PIXELS_Y;

    plP_setphy( (PLINT) 0, (PLINT) ( pls->xlength / ( (QtSVGDevice *) ( pls->dev ) )->downscale ), (PLINT) 0, (PLINT) ( pls->ylength / ( (QtSVGDevice *) ( pls->dev ) )->downscale ) );

    plP_setpxl( POINTS_PER_INCH / 25.4 / ( (QtSVGDevice *) ( pls->dev ) )->downscale, POINTS_PER_INCH / 25.4 / ( (QtSVGDevice *) ( pls->dev ) )->downscale );

    // Initialize family file info
    plFamInit( pls );

    plOpenFile( pls );
}

void plD_bop_svgqt( PLStream *pls )
{
    // Plot familying stuff. Not really understood, just copying gd.c
    plGetFam( pls );

    pls->famadv = 1;
    pls->page++;
    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtSVGDevice *) pls->dev )->definePlotName( pls->FileName );
    ( (QtSVGDevice *) pls->dev )->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}

void plD_eop_svgqt( PLStream *pls )
{
    double downscale;
    QSize  s;

    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtSVGDevice *) pls->dev )->savePlot();
    // Once saved, we have to create a new device with the same properties
    // to be able to plot another page.
    downscale = ( (QtSVGDevice *) pls->dev )->downscale;
    s         = ( (QtSVGDevice *) pls->dev )->size();
    bool isMaster = ( handler.isMasterDevice( (QtSVGDevice *) pls->dev ) );
    delete ( (QtSVGDevice *) pls->dev );

    pls->dev = new QtSVGDevice( s.width(), s.height() );
    ( (QtSVGDevice *) pls->dev )->downscale = downscale;
    ( (QtSVGDevice *) pls->dev )->setPLStream( pls );

    if ( isMaster )
        handler.setMasterDevice( (QtSVGDevice *) pls->dev );
    handler.DeviceChangedPage( (QtSVGDevice *) pls->dev );
}

void plD_line_svgqt( PLStream * pls, short x1a, short y1a, short x2a, short y2a )
{
    QtSVGDevice* widget = (QtSVGDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawLine( x1a, y1a, x2a, y2a );
}

void plD_polyline_svgqt( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    QtSVGDevice * widget = (QtSVGDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawPolyline( xa, ya, npts );
}

void plD_esc_svgqt( PLStream * pls, PLINT op, void* ptr )
{
    short         *xa, *ya;
    unsigned char *r, *g, *b;
    PLFLT         *alpha;
    PLINT         i;
    QtSVGDevice   * widget = (QtSVGDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    switch ( op )
    {
    case PLESC_FILL:
        xa = new short[pls->dev_npts];
        ya = new short[pls->dev_npts];

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        break;

    case PLESC_GRADIENT:
        xa    = new short[pls->dev_npts];
        ya    = new short[pls->dev_npts];
        r     = new unsigned char[pls->ncol1];
        g     = new unsigned char[pls->ncol1];
        b     = new unsigned char[pls->ncol1];
        alpha = new PLFLT[pls->ncol1];

        for ( i = 0; i < pls->ncol1; i++ )
        {
            r[i]     = pls->cmap1[i].r;
            g[i]     = pls->cmap1[i].g;
            b[i]     = pls->cmap1[i].b;
            alpha[i] = pls->cmap1[i].a;
        }
        widget->setGradient( pls->xgradient[0], pls->xgradient[1], pls->ygradient[0], pls->ygradient[1], r, g, b, alpha, pls->ncol1 );

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        delete[] r;
        delete[] g;
        delete[] b;
        delete[] alpha;
        break;

    case PLESC_HAS_TEXT:
        // call the generic ProcessString function
        //  ProcessString( pls, (EscText *)ptr );
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawText( (EscText *) ptr );
        break;

    default:
        break;
    }
}

void plD_state_svgqt( PLStream * pls, PLINT op )
{
    QtSVGDevice * widget = (QtSVGDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    switch ( op )
    {
    case PLSTATE_WIDTH:
        widget->setWidthF( pls->width );
        break;

    case PLSTATE_COLOR0:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;

    case PLSTATE_COLOR1:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;

    default: break;
    }
}

void plD_tidy_svgqt( PLStream * pls )
{
    QtSVGDevice * widget = (QtSVGDevice *) pls->dev;

    if ( widget != NULL )
    {
        handler.DeviceClosed( widget );
        delete widget;
        pls->dev = NULL;
    }
    plCloseFile( pls );

    closeQtApp();
}
#endif

#if defined ( PLD_epsqt )
void plD_dispatch_init_epsqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt EPS Driver";
    pdt->pl_DevName = "epsqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 73;
    pdt->pl_init     = (plD_init_fp) plD_init_epspdfqt;
    pdt->pl_line     = (plD_line_fp) plD_line_epspdfqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_epspdfqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_epspdfqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_epsqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_epspdfqt;
    pdt->pl_state    = (plD_state_fp) plD_state_epspdfqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_epspdfqt;
}
#endif

#if defined ( PLD_pdfqt )
void plD_dispatch_init_pdfqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt PDF Driver";
    pdt->pl_DevName = "pdfqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 74;
    pdt->pl_init     = (plD_init_fp) plD_init_epspdfqt;
    pdt->pl_line     = (plD_line_fp) plD_line_epspdfqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_epspdfqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_epspdfqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_pdfqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_epspdfqt;
    pdt->pl_state    = (plD_state_fp) plD_state_epspdfqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_epspdfqt;
}
#endif

#if defined ( PLD_epsqt ) || defined ( PLD_pdfqt )
void plD_init_epspdfqt( PLStream * pls )
{
    vectorize = 0;
    lines_aa  = 1;
    plParseDrvOpts( qt_options );

    // Stream setup
    pls->color        = 1;
    pls->plbuf_write  = 0;
    pls->dev_fill0    = 1;
    pls->dev_fill1    = 0;
    pls->dev_gradient = 1;      // driver renders gradient
    pls->dev_arc      = 1;      // driver renders arcs
    // Let the PLplot core handle dashed lines since
    // the driver results for this capability have a number of issues.
    // pls->dev_dash=1;
    pls->dev_dash  = 0;
    pls->dev_flush = 1;
    // Driver does not have a clear capability so use (good) PLplot core
    // fallback for that instead.
    pls->dev_clear         = 0;
    pls->termin            = 0;
    pls->page              = 0;
    pls->dev_text          = 1; // want to draw text
    pls->dev_unicode       = 1; // want unicode
    pls->has_string_length = 1; // Driver supports string length calculations

    // QPrinter devices won't create if there is no QApplication declared...
    // Needs to be true only because of multi-stream case
    bool isMaster = initQtApp( true );

    if ( pls->xlength <= 0 || pls->ylength <= 0 )
    {
        pls->dev     = new QtEPSDevice;
        pls->xlength = (int) ( ( (QtEPSDevice *) ( pls->dev ) )->m_dWidth );
        pls->ylength = (int) ( ( (QtEPSDevice *) ( pls->dev ) )->m_dHeight );
    }
    else
    {
        pls->dev = new QtEPSDevice( pls->xlength, pls->ylength );
    }
    ( (QtEPSDevice *) pls->dev )->setPLStream( pls );

    if ( isMaster )
        handler.setMasterDevice( (QtEPSDevice *) ( pls->dev ) );

    if ( pls->xlength > pls->ylength )
        ( (QtEPSDevice *) ( pls->dev ) )->downscale = (PLFLT) pls->xlength / (PLFLT) ( PIXELS_X - 1 );
    else
        ( (QtEPSDevice *) ( pls->dev ) )->downscale = (PLFLT) pls->ylength / (PLFLT) PIXELS_Y;

    plP_setphy( (PLINT) 0, (PLINT) ( pls->xlength / ( (QtEPSDevice *) ( pls->dev ) )->downscale ), (PLINT) 0, (PLINT) ( pls->ylength / ( (QtEPSDevice *) ( pls->dev ) )->downscale ) );

    plP_setpxl( POINTS_PER_INCH / 25.4 / ( (QtEPSDevice *) ( pls->dev ) )->downscale, POINTS_PER_INCH / 25.4 / ( (QtEPSDevice *) ( pls->dev ) )->downscale );

    // Initialize family file info
    plFamInit( pls );

    plOpenFile( pls );
}

void plD_bop_epspdfqt_helper( PLStream *pls, int ifeps )
{
    // Plot familying stuff. Not really understood, just copying gd.c
    plGetFam( pls );

    pls->famadv = 1;
    pls->page++;
    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtEPSDevice *) pls->dev )->definePlotName( pls->FileName, ifeps );
    ( (QtEPSDevice *) pls->dev )->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}

void plD_eop_epspdfqt( PLStream *pls )
{
    double downscale;

    if ( qt_family_check( pls ) )
    {
        return;
    }
    ( (QtEPSDevice *) pls->dev )->savePlot();
    // Once saved, we have to create a new device with the same properties
    // to be able to plot another page.
    downscale = ( (QtEPSDevice *) pls->dev )->downscale;
    bool isMaster = handler.isMasterDevice( (QtEPSDevice *) pls->dev );
    delete ( (QtEPSDevice *) pls->dev );

    pls->dev = new QtEPSDevice;
    ( (QtEPSDevice *) pls->dev )->downscale = downscale;

    if ( isMaster )
        handler.setMasterDevice( (QtEPSDevice *) pls->dev );
    handler.DeviceChangedPage( (QtEPSDevice *) pls->dev );
}

void plD_line_epspdfqt( PLStream * pls, short x1a, short y1a, short x2a, short y2a )
{
    QtEPSDevice* widget = (QtEPSDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawLine( x1a, y1a, x2a, y2a );
}

void plD_polyline_epspdfqt( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    QtEPSDevice * widget = (QtEPSDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawPolyline( xa, ya, npts );
}

void plD_esc_epspdfqt( PLStream * pls, PLINT op, void* ptr )
{
    short         *xa, *ya;
    unsigned char *r, *g, *b;
    PLFLT         *alpha;
    PLINT         i;
    QtEPSDevice   * widget  = (QtEPSDevice *) pls->dev;
    arc_struct    *arc_info = (arc_struct *) ptr;

    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    switch ( op )
    {
    case PLESC_FILL:
        xa = new short[pls->dev_npts];
        ya = new short[pls->dev_npts];

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        break;

    case PLESC_GRADIENT:
        xa    = new short[pls->dev_npts];
        ya    = new short[pls->dev_npts];
        r     = new unsigned char[pls->ncol1];
        g     = new unsigned char[pls->ncol1];
        b     = new unsigned char[pls->ncol1];
        alpha = new PLFLT[pls->ncol1];

        for ( i = 0; i < pls->ncol1; i++ )
        {
            r[i]     = pls->cmap1[i].r;
            g[i]     = pls->cmap1[i].g;
            b[i]     = pls->cmap1[i].b;
            alpha[i] = pls->cmap1[i].a;
        }
        widget->setGradient( pls->xgradient[0], pls->xgradient[1], pls->ygradient[0], pls->ygradient[1], r, g, b, alpha, pls->ncol1 );

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        delete[] r;
        delete[] g;
        delete[] b;
        delete[] alpha;
        break;

    case PLESC_HAS_TEXT:
        // call the generic ProcessString function
        //  ProcessString( pls, (EscText *)ptr );
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawText( (EscText *) ptr );
        break;

    case PLESC_ARC:
        widget->drawArc( arc_info->x, arc_info->y, arc_info->a, arc_info->b, arc_info->angle1, arc_info->angle2, arc_info->rotate, arc_info->fill );
        break;

    default: break;
    }
}

void plD_state_epspdfqt( PLStream * pls, PLINT op )
{
    QtEPSDevice * widget = (QtEPSDevice *) pls->dev;
    if ( widget != NULL && qt_family_check( pls ) )
    {
        return;
    }
    if ( widget == NULL )
        return;

    switch ( op )
    {
    case PLSTATE_WIDTH:
        widget->setWidthF( pls->width );
        break;

    case PLSTATE_COLOR0:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;

    case PLSTATE_COLOR1:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;


    default: break;
    }
}

void plD_tidy_epspdfqt( PLStream * pls )
{
    QtEPSDevice * widget = (QtEPSDevice *) pls->dev;

    if ( widget != NULL )
    {
        handler.DeviceClosed( widget );
        delete widget;
        pls->dev = NULL;
    }
    plCloseFile( pls );

    closeQtApp();
}
#endif

#if defined ( PLD_epsqt )
void plD_bop_epsqt( PLStream *pls )
{
    plD_bop_epspdfqt_helper( pls, 1 );
}
#endif

#if defined ( PLD_pdfqt )
void plD_bop_pdfqt( PLStream *pls )
{
    plD_bop_epspdfqt_helper( pls, 0 );
}
#endif

#if defined ( PLD_qtwidget )
void plD_dispatch_init_qtwidget( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt Widget";
    pdt->pl_DevName = "qtwidget";
#endif
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 72;
    pdt->pl_init     = (plD_init_fp) plD_init_qtwidget;
    pdt->pl_line     = (plD_line_fp) plD_line_qtwidget;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qtwidget;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_qtwidget;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_qtwidget;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_qtwidget;
    pdt->pl_state    = (plD_state_fp) plD_state_qtwidget;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_qtwidget;
    pdt->pl_wait     = (plD_wait_fp) plD_wait_qtwidget;
}

void plD_init_qtwidget( PLStream * pls )
{
    vectorize = 0;
    lines_aa  = 1;
    plParseDrvOpts( qt_options );

    bool      isMaster = initQtApp( true );
    QtPLWidget* widget;

    if ( pls->xlength <= 0 || pls->ylength <= 0 )
    {
        widget       = new QtPLWidget;
        pls->dev     = (void *) widget;
        pls->xlength = (int) widget->m_dWidth;
        pls->ylength = (int) widget->m_dHeight;
    }
    else
    {
        widget   = new QtPLWidget( pls->xlength, pls->ylength );
        pls->dev = (void *) widget;
    }
    widget->setPLStream( pls );

    if ( isMaster )
        handler.setMasterDevice( widget );

    if ( plsc->xlength > plsc->ylength )
        widget->downscale = (PLFLT) plsc->xlength / (PLFLT) ( PIXELS_X - 1 );
    else
        widget->downscale = (PLFLT) plsc->ylength / (PLFLT) PIXELS_Y;

    plP_setphy( (PLINT) 0, (PLINT) ( plsc->xlength / widget->downscale ), (PLINT) 0, (PLINT) ( plsc->ylength / widget->downscale ) );

    QPicture temp;
    QPainter tempPainter( &temp );

    plP_setpxl( temp.logicalDpiX() / 25.4 / widget->downscale, temp.logicalDpiY() / 25.4 / widget->downscale );

    pls->color        = 1;      // Is a color device
    pls->plbuf_write  = 1;      // Store commands to device in core buffer
    pls->dev_fill0    = 1;      // Handle solid fills
    pls->dev_fill1    = 0;
    pls->dev_gradient = 1;      // driver renders gradient
    pls->dev_arc      = 1;      // driver renders arcs
    // Let the PLplot core handle dashed lines since
    // the driver results for this capability have a number of issues.
    // pls->dev_dash=1;
    pls->dev_dash  = 0;
    pls->dev_flush = 1;
    // Driver does not have a clear capability so use (good) PLplot core
    // fallback for that instead.
    pls->dev_clear         = 0;
    pls->termin            = 1;
    pls->dev_text          = 1; // want to draw text
    pls->dev_unicode       = 1; // want unicode
    pls->has_string_length = 1; // Driver supports string length calculations

    widget->setVisible( true );
    widget->resize( plsc->xlength, plsc->ylength );
    widget->move( plsc->xoffset, plsc->yoffset );

    widget->setWindowTitle( pls->plwindow );

    qApp->connect( &handler, SIGNAL( MasterChangedPage() ), widget, SLOT( nextPage() ) );
    qApp->connect( &handler, SIGNAL( MasterClosed() ), widget, SLOT( close() ) );
}

void plD_eop_qtwidget( PLStream *pls )
{
    QtPLWidget* widget = ( (QtPLWidget *) pls->dev );

    widget->flush();
    widget->raise();
}

void plD_wait_qtwidget( PLStream *pls )
{
    QtPLWidget* widget    = ( (QtPLWidget *) pls->dev );
    int       currentPage = widget->pageNumber;

    widget->raise();
    while ( currentPage == widget->pageNumber && handler.isMasterDevice( widget ) )
    {
        qApp->processEvents( QEventLoop::WaitForMoreEvents );
    }
}

void plD_bop_qtwidget( PLStream *pls )
{
    QtPLWidget* widget = ( (QtPLWidget *) pls->dev );
    widget->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}

void plD_line_qtwidget( PLStream * pls, short x1a, short y1a, short x2a, short y2a )
{
    QtPLWidget* widget = (QtPLWidget *) pls->dev;
    if ( widget == NULL )
        return;

    widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawLine( x1a, y1a, x2a, y2a );
}

void plD_polyline_qtwidget( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    QtPLWidget * widget = (QtPLWidget *) pls->dev;
    if ( widget == NULL )
        return;

    widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawPolyline( xa, ya, npts );
}

void plD_esc_qtwidget( PLStream * pls, PLINT op, void* ptr )
{
    short         *xa, *ya;
    PLINT         i;
    unsigned char *r, *g, *b;
    PLFLT         *alpha;
    QtPLWidget    * widget  = (QtPLWidget *) pls->dev;
    arc_struct    *arc_info = (arc_struct *) ptr;
    if ( widget == NULL )
        return;

    switch ( op )
    {
    case PLESC_FILL:
        xa = new short[pls->dev_npts];
        ya = new short[pls->dev_npts];

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }

        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        break;

    case PLESC_GRADIENT:
        xa    = new short[pls->dev_npts];
        ya    = new short[pls->dev_npts];
        r     = new unsigned char[pls->ncol1];
        g     = new unsigned char[pls->ncol1];
        b     = new unsigned char[pls->ncol1];
        alpha = new PLFLT[pls->ncol1];

        for ( i = 0; i < pls->ncol1; i++ )
        {
            r[i]     = pls->cmap1[i].r;
            g[i]     = pls->cmap1[i].g;
            b[i]     = pls->cmap1[i].b;
            alpha[i] = pls->cmap1[i].a;
        }
        widget->setGradient( pls->xgradient[0], pls->xgradient[1], pls->ygradient[0], pls->ygradient[1], r, g, b, alpha, pls->ncol1 );

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        delete[] r;
        delete[] g;
        delete[] b;
        delete[] alpha;
        break;

    case PLESC_HAS_TEXT:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawText( (EscText *) ptr );
        break;

    case PLESC_ARC:
        widget->drawArc( arc_info->x, arc_info->y, arc_info->a, arc_info->b, arc_info->angle1, arc_info->angle2, arc_info->rotate, arc_info->fill );
        break;

    case PLESC_FLUSH:
        widget->flush();
        break;
    case PLESC_GETC:
        widget->getCursorCmd( (PLGraphicsIn *) ptr );
        break;
    default: break;
    }
}

void plD_state_qtwidget( PLStream * pls, PLINT op )
{
    QtPLWidget * widget = (QtPLWidget *) pls->dev;
    if ( widget == NULL )
        return;

    switch ( op )
    {
    case PLSTATE_WIDTH:
        widget->setWidthF( pls->width );
        break;

    case PLSTATE_COLOR0:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;

    case PLSTATE_COLOR1:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;


    default: break;
    }
}

void plD_tidy_qtwidget( PLStream * pls )
{
    QtPLWidget * widget = (QtPLWidget *) pls->dev;

    if ( widget != NULL )
    {
        handler.DeviceClosed( widget );
        delete widget;
        pls->dev = NULL;
    }

    closeQtApp();
}
#endif

#if defined ( PLD_extqt )
void plD_dispatch_init_extqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "External Qt Widget";
    pdt->pl_DevName = "extqt";
#endif
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 75;
    pdt->pl_init     = (plD_init_fp) plD_init_extqt;
    pdt->pl_line     = (plD_line_fp) plD_line_extqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_extqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_extqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_extqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_extqt;
    pdt->pl_state    = (plD_state_fp) plD_state_extqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_extqt;
}

void plD_init_extqt( PLStream * pls )
{
    // N.B. initQtApp() not called here because internal_qApp is always false
    // for this device.
    vectorize = 0;
    lines_aa  = 1;

    if ( pls->dev == NULL /* || pls->xlength <= 0 || pls->ylength <= 0*/ )
    {
        printf( "Error: use plsetqtdev to set up the Qt device before calling plinit()\n" );
        return;
    }

    QtExtWidget* widget = (QtExtWidget *) ( pls->dev );

    if ( widget->m_dWidth > widget->m_dHeight )
        widget->downscale = (PLFLT) widget->m_dWidth / (PLFLT) ( PIXELS_X - 1 );
    else
        widget->downscale = (PLFLT) widget->m_dHeight / (PLFLT) PIXELS_Y;

    plP_setphy( (PLINT) 0, (PLINT) ( widget->m_dWidth / widget->downscale ), (PLINT) 0, (PLINT) ( widget->m_dHeight / widget->downscale ) );

    QPicture temp;
    QPainter tempPainter( &temp );

    plP_setpxl( temp.logicalDpiX() / 25.4 / widget->downscale, temp.logicalDpiY() / 25.4 / widget->downscale );

    pls->color        = 1;      // Is a color device
    pls->plbuf_write  = 0;
    pls->dev_fill0    = 1;      // Handle solid fills
    pls->dev_fill1    = 0;
    pls->dev_gradient = 1;      // driver renders gradient
    pls->dev_arc      = 1;      // driver renders arcs
    // Let the PLplot core handle dashed lines since
    // the driver results for this capability have a number of issues.
    // pls->dev_dash=1;
    pls->dev_dash  = 0;
    pls->dev_flush = 1;
    // Driver does not have a clear capability so use (good) PLplot core
    // fallback for that instead.
    pls->dev_clear         = 0;
    pls->termin            = 0;
    pls->dev_text          = 1; // want to draw text
    pls->dev_unicode       = 1; // want unicode
    pls->has_string_length = 1; // Driver supports string length calculations
}

//
// These functions are separated out (instead of using dynamic_cast)
// for the benefit of the PyQt4 bindings. C++ QtExtWidgets created
// by PyQt4 are not properly type resolved.
//

void plD_line_extqt( PLStream * pls, short x1a, short y1a, short x2a, short y2a )
{
    QtExtWidget * widget = NULL;

    widget = (QtExtWidget *) pls->dev;
    widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawLine( x1a, y1a, x2a, y2a );
}

void plD_polyline_extqt( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    QtExtWidget * widget = NULL;

    widget = (QtExtWidget *) pls->dev;
    widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
    widget->drawPolyline( xa, ya, npts );
}

void plD_esc_extqt( PLStream * pls, PLINT op, void* ptr )
{
    short         *xa, *ya;
    PLINT         i;
    unsigned char *r, *g, *b;
    PLFLT         *alpha;
    QtExtWidget   * widget  = NULL;
    arc_struct    *arc_info = (arc_struct *) ptr;

    widget = (QtExtWidget *) pls->dev;
    switch ( op )
    {
    case PLESC_FILL:
        xa = new short[pls->dev_npts];
        ya = new short[pls->dev_npts];

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        break;

    case PLESC_GRADIENT:
        xa    = new short[pls->dev_npts];
        ya    = new short[pls->dev_npts];
        r     = new unsigned char[pls->ncol1];
        g     = new unsigned char[pls->ncol1];
        b     = new unsigned char[pls->ncol1];
        alpha = new PLFLT[pls->ncol1];

        for ( i = 0; i < pls->ncol1; i++ )
        {
            r[i]     = pls->cmap1[i].r;
            g[i]     = pls->cmap1[i].g;
            b[i]     = pls->cmap1[i].b;
            alpha[i] = pls->cmap1[i].a;
        }
        widget->setGradient( pls->xgradient[0], pls->xgradient[1], pls->ygradient[0], pls->ygradient[1], r, g, b, alpha, pls->ncol1 );

        for ( i = 0; i < pls->dev_npts; i++ )
        {
            xa[i] = pls->dev_x[i];
            ya[i] = pls->dev_y[i];
        }
        widget->drawPolygon( xa, ya, pls->dev_npts );

        delete[] xa;
        delete[] ya;
        delete[] r;
        delete[] g;
        delete[] b;
        delete[] alpha;
        break;

    case PLESC_HAS_TEXT:
        // call the generic ProcessString function
        //  ProcessString( pls, (EscText *)ptr );
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        widget->drawText( (EscText *) ptr );
        break;

    case PLESC_ARC:
        widget->drawArc( arc_info->x, arc_info->y, arc_info->a, arc_info->b, arc_info->angle1, arc_info->angle2, arc_info->rotate, arc_info->fill );
        break;

    default: break;
    }
}

void plD_state_extqt( PLStream * pls, PLINT op )
{
    QtExtWidget * widget = NULL;

    widget = (QtExtWidget *) pls->dev;
    switch ( op )
    {
    case PLSTATE_WIDTH:
        widget->setWidthF( pls->width );
        break;

    case PLSTATE_COLOR0:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;

    case PLSTATE_COLOR1:
        widget->setColor( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a );
        break;


    default: break;
    }
}

void plD_tidy_extqt( PLStream * pls )
{
    QtExtWidget * widget = NULL;

    widget = (QtExtWidget *) pls->dev;
    if ( widget != NULL )
    {
        handler.DeviceClosed( widget );
        delete widget;
        pls->dev = NULL;
    }

    // closeQtApp() not called here because internal_qApp is always false
    // for this device.
}

void plD_eop_extqt( PLStream * /* pls */ )
{
}

void plD_bop_extqt( PLStream *pls )
{
    QtExtWidget * widget = NULL;
    widget = (QtExtWidget *) pls->dev;
    widget->setBackgroundColor( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b, pls->cmap0[0].a );
}
#endif

#if defined ( PLD_memqt )
void plD_dispatch_init_memqt( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Qt Memory Driver";
    pdt->pl_DevName = "memqt";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 76;
    pdt->pl_init     = (plD_init_fp) plD_init_memqt;
    pdt->pl_line     = (plD_line_fp) plD_line_rasterqt;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_rasterqt;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_memqt;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_memqt;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_rasterqt;
    pdt->pl_state    = (plD_state_fp) plD_state_rasterqt;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_rasterqt;
}

void plD_init_memqt( PLStream * pls )
{
    int           i;
    double        dpi;
    unsigned char *qt_mem;
    unsigned char *input_mem;

    vectorize = 0;
    lines_aa  = 1;
    plParseDrvOpts( qt_options );

    // Stream setup
    pls->color        = 1;
    pls->plbuf_write  = 0;
    pls->dev_fill0    = 1;
    pls->dev_fill1    = 0;
    pls->dev_gradient = 1;      // driver renders gradient
    pls->dev_arc      = 1;      // driver renders arcs
    // Let the PLplot core handle dashed lines since
    // the driver results for this capability have a number of issues.
    // pls->dev_dash=1;
    pls->dev_dash  = 0;
    pls->dev_flush = 1;
    // Driver does not have a clear capability so use (good) PLplot core
    // fallback for that instead.
    pls->dev_clear         = 0;
    pls->termin            = 0;
    pls->page              = 0;
    pls->dev_text          = 1; // want to draw text
    pls->dev_unicode       = 1; // want unicode
    pls->has_string_length = 1; // Driver supports string length calculations

    // Needs to be true only because of multi-stream case
    bool isMaster = initQtApp( true );

    if ( pls->xdpi <= 0. )
        dpi = DEFAULT_DPI;
    else
        dpi = pls->xdpi;

    // Set the plot size to the memory buffer size, on the off chance
    // that they are different.
    pls->xlength = pls->phyxma;
    pls->ylength = pls->phyyma;

    // Save a pointer to the user supplied memory
    input_mem = (unsigned char *) pls->dev;

    // Create a appropriately sized raster device
    pls->dev = new QtRasterDevice( pls->xlength, pls->ylength );
    ( (QtRasterDevice *) pls->dev )->setPLStream( pls );
    ( (QtRasterDevice *) pls->dev )->memory = input_mem;

    if ( isMaster )
        handler.setMasterDevice( (QtRasterDevice *) ( pls->dev ) );

    if ( pls->xlength > pls->ylength )
        ( (QtRasterDevice *) ( pls->dev ) )->downscale = (PLFLT) pls->xlength / (PLFLT) ( PIXELS_X - 1 );
    else
        ( (QtRasterDevice *) ( pls->dev ) )->downscale = (PLFLT) pls->ylength / (PLFLT) PIXELS_Y;

    plP_setphy( (PLINT) 0, (PLINT) ( pls->xlength / ( (QtRasterDevice *) ( pls->dev ) )->downscale ), (PLINT) 0, (PLINT) ( pls->ylength / ( (QtRasterDevice *) ( pls->dev ) )->downscale ) );

    plP_setpxl( dpi / 25.4 / ( (QtRasterDevice *) ( pls->dev ) )->downscale, dpi / 25.4 / ( (QtRasterDevice *) ( pls->dev ) )->downscale );

    // Copy the user supplied memory into the QImage.
//       This device assumes that the format of the QImage
//       is RGB32 (or ARGB).

    qt_mem = ( (QtRasterDevice *) pls->dev )->scanLine( 0 );

    for ( i = 0; i < ( pls->xlength * pls->ylength ); i++ )
    {
        qt_mem[2] = input_mem[0]; // R
        qt_mem[1] = input_mem[1]; // G
        qt_mem[0] = input_mem[2]; // B
        if ( pls->dev_mem_alpha == 1 )
        {
            qt_mem[3]  = input_mem[3];
            input_mem += 4;
        }
        else
        {
            input_mem += 3;
        }
        qt_mem += 4;
    }

    ( (QtRasterDevice *) ( pls->dev ) )->setResolution( dpi );

    // This is set so the we'll always make it past the qt_family_check().
    pls->family = true;
}

void plD_bop_memqt( PLStream * /* pls */ )
{
    // Do nothing to preserve user data
}

void plD_eop_memqt( PLStream *pls )
{
    int           i;
    unsigned char *memory;
    unsigned char *qt_mem;

    memory = ( (QtRasterDevice *) pls->dev )->memory;
    qt_mem = ( (QtRasterDevice *) pls->dev )->scanLine( 0 );

    for ( i = 0; i < ( pls->xlength * pls->ylength ); i++ )
    {
        memory[0] = qt_mem[2];           // R
        memory[1] = qt_mem[1];           // G
        memory[2] = qt_mem[0];           // B
        if ( pls->dev_mem_alpha == 1 )
        {
            memory[3] = qt_mem[3];
            memory   += 4;
        }
        else
        {
            memory += 3;
        }
        qt_mem += 4;
    }
}

#endif
