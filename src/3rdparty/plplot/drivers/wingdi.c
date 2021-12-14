//      PLplot WIN32 GDI driver.
//
// Copyright (C) 2004  Andrew Roach
//
// This file is part of PLplot.
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
//
#include "plDevs.h"

#ifdef PLD_wingdi

#include <string.h>
#include <windows.h>
#include <windowsx.h>       // GET_X_LPARAM/GET_Y_LPARAM
#include <commctrl.h>       // For status bars
#if !defined ( __CYGWIN__ )
#include <tchar.h>
#else
#include <winnt.h>
#define _T( a )    __TEXT( a )
#endif

#define NEED_PLDEBUG
#define DEBUG
#include "plplotP.h"
#include "drivers.h"
#include "plevent.h"

#define ARRAY_SIZE( x )    ( ( sizeof x ) / ( sizeof *x ) )
#define INITIAL_HEAP_SIZE    16384  // Initial size of heap in bytes

// Driver viewer types
enum _dev_viewer
{
    VIEWER_MINIMAL = 0,       // Display just a plot area window
    VIEWER_FULL,              // Display the full function window
    VIEWER_PLOT               // A child only plot area
};

// Enumerated type for the device states
enum _dev_state
{
    DEV_WAITING = 0,  // Device is idle
    DEV_ACTIVE,       // Device is ready for next plot
    DEV_SIZEMOVE,     // Device might be sizing or moving the window
    DEV_RESIZE,       // Device is resizing the window
    DEV_DRAWING       // Device is actively drawing
};

// Enumerated type used to indicate the device type
// for updating page metrics
enum _dev_type
{
    DEV_WINDOW,          // Setup page metrics for a window
    DEV_PRINTER          // Setup page metrics for a printer
};

// Data structure used to track the fonts that are created.
// This avoids the overhead of recreating the font everytime
// a string is rendered.
struct _font_entry
{
    LOGFONT            info;
    HDC                hdc;         // Device context used to create font
    HFONT              font;
    struct _font_entry *next;       // Next entry in the font list
};

// Device-specific info per stream
struct wingdi_Dev
{
    //
    // Members that are common to interactive GUI devices
    //
    PLFLT            xdpmm;  // Device x pixel per mm
    PLFLT            ydpmm;  // Device y pixel per mm
    PLFLT            xscale; // Virtual x pixels to device pixel scaling
    PLFLT            yscale; // Virtual y pixels to device pixel scaling
    PLINT            width;  // Window Width (which can change)
    PLINT            height; // Window Height

    enum _dev_viewer viewer;
    enum _dev_type   type;
    enum _dev_state  state;             // Current state of the device
    enum _dev_state  prev_state;        // Previous state of the device
    // Used to restore after redraw
    union
    {
        unsigned int status_bar : 1;
        unsigned int menu_bar   : 1;
    } feature;

    //
    // WIN32 API variables
    //
    HDC      hdc;                 // Plot window device context
    HPEN     pen;                 // Current pen used for drawing
    COLORREF color;               // Current color
    HDC      hdc_bmp;             // Bitmap device context
    HBITMAP  bitmap;              // Bitmap of current display
    HWND     frame;               // Handle for the main window.
    HWND     plot;                // Handle for the plot area
    HWND     status_bar;          // Handle for the status bar

    //
    // Image rasterization variables
    HDC     save_hdc;                          // Save the original plot window DC
    HBITMAP raster_bmp;                        // Bitmap for the raster image
    RECT    raster_rect;                       // Location of the image
};

// Device info
PLDLLIMPEXP_DRIVER const char* plD_DEVICE_INFO_wingdi = "wingdi:Windows GDI:1:wingdi:11:wingdi\n";

// Counter used to indicate the number of streams in use
static int wingdi_streams = 0;

// Window class for the overall frame
static TCHAR      * frame_window_class_name = _T( "PLplotFrame" );
static WNDCLASSEX frame_window_class;

// Window class for the plot area
static TCHAR      * plot_area_class_name = _T( "PLplotArea" );
static WNDCLASSEX plot_area_class;

// Popup menu used by a plot window
static HMENU plot_popup_menu;

// Private heap used to allocate memory for the driver
static HANDLE wingdi_heap;

// Font tracking list
struct _font_entry *font_list = NULL;

PLDLLIMPEXP_DRIVER void plD_dispatch_init_wingdi( PLDispatchTable *pdt );
void plD_init_wingdi( PLStream * );

//--------------------------------------------------------------------------
// Graphics primitive implementation functions
//--------------------------------------------------------------------------
static void plD_line_wingdi( PLStream *, short, short, short, short );
static void plD_polyline_wingdi( PLStream *, short *, short *, PLINT );
static void plD_fill_polygon_wingdi( PLStream *pls );
static void plD_clear_wingdi( PLStream *pls,
                              PLINT x1, PLINT y1, PLINT x2, PLINT y2 );
static void plD_eop_wingdi( PLStream * );
static void plD_bop_wingdi( PLStream * );
static void plD_tidy_wingdi( PLStream * );
static void plD_wait_wingdi( PLStream * );
static void plD_state_wingdi( PLStream *, PLINT );
static void plD_esc_wingdi( PLStream *, PLINT, void * );

enum commands
{
    CommandPrint    = 0x08A1,
    CommandNextPage = 0x08A2,
    CommandQuit     = 0x08A3
};
#define PlotAreaId     0x08F0
#define StatusBarId    0x08F1

void plD_dispatch_init_wingdi( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = "Win32/64 GDI device";
    pdt->pl_DevName = "wingdi";
#endif
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 11;
    pdt->pl_init     = (plD_init_fp) plD_init_wingdi;
    pdt->pl_line     = (plD_line_fp) plD_line_wingdi;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_wingdi;
    pdt->pl_eop      = (plD_eop_fp) plD_eop_wingdi;
    pdt->pl_bop      = (plD_bop_fp) plD_bop_wingdi;
    pdt->pl_tidy     = (plD_tidy_fp) plD_tidy_wingdi;
    pdt->pl_state    = (plD_state_fp) plD_state_wingdi;
    pdt->pl_esc      = (plD_esc_fp) plD_esc_wingdi;
    pdt->pl_wait     = (plD_wait_fp) plD_wait_wingdi;
}

static HCURSOR
CrossHairCursor( struct wingdi_Dev * dev )
{
    HCURSOR cursor;

    cursor = LoadCursor( NULL, IDC_CROSS );
    SetClassLongPtr( dev->plot, GCL_HCURSOR, (long) cursor );
    return SetCursor( cursor );
}

static void
NormalCursor( struct wingdi_Dev * dev )
{
    HCURSOR cursor;

    cursor = LoadCursor( NULL, IDC_ARROW );
    SetClassLongPtr( dev->plot, GCL_HCURSOR, (LONG_PTR) cursor );
    SetCursor( cursor );
}

static void
BusyCursor( struct wingdi_Dev * dev )
{
    HCURSOR cursor;

    cursor = LoadCursor( NULL, IDC_WAIT );
    SetClassLongPtr( dev->plot, GCL_HCURSOR, (LONG_PTR) cursor );
    SetCursor( cursor );
}

static void
update_status_bar( struct wingdi_Dev * dev )
{
    LPSTR status_text[] =
    {
        TEXT( "Waiting" ),
        TEXT( "Active" ),
        TEXT( "Size/Move" ),
        TEXT( "Resize" ),
        TEXT( "Drawing" )
    };

    if ( dev->status_bar == NULL )
        return;

    SendMessage( dev->status_bar,
        SB_SETTEXT,
        (WPARAM) 0,
        (LPARAM) status_text[dev->state] );
}

//--------------------------------------------------------------------------
//  static void UpdatePageMetrics ( PLStream *pls, char flag )
//
//      UpdatePageMetrics is a simple function which simply gets new values
//      for a changed DC, be it swapping from printer to screen or vice-versa.
//      The flag variable is used to tell the function if it is updating
//      from the printer (1) or screen (0).
//--------------------------------------------------------------------------
static void UpdatePageMetrics( PLStream *pls, enum _dev_type dev_type )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;

    dev->type = dev_type;
    if ( dev_type == DEV_PRINTER )
    {
        // Get the page size from the printer
        PLINT vsize, hsize;
        PLFLT plot_aspect, print_aspect;
        hsize = GetDeviceCaps( dev->hdc, HORZRES );
        vsize = GetDeviceCaps( dev->hdc, VERTRES );

        // Determine which orientation best matches the aspect
        // ratio of the plot window
        plot_aspect  = (PLFLT) dev->width / (PLFLT) dev->height;
        print_aspect = (PLFLT) hsize / (PLFLT) vsize;
        if ( plot_aspect > 1.0 )
        {
            // Wider than tall
            dev->width  = hsize;
            dev->height = (PLINT) ( (PLFLT) hsize / plot_aspect );
        }
        else
        {
            // Taller than wide
            dev->width  = (PLINT) ( (PLFLT) vsize * plot_aspect );
            dev->height = vsize;
        }
    }
    else
    {
        RECT rect;

        GetClientRect( dev->plot, &rect );
        dev->width  = rect.right;
        dev->height = rect.bottom;

        pldebug( "wingdi", "Page size [%d %d] [%d %d]\n",
            rect.left, rect.top,
            rect.right, rect.bottom );
    }

    // We need the -1 because some of the coordinates
    // are signed and PIXEL_X/PIXEL_Y can exceed the
    // maximum value of a positive signed integer, which
    // results in a negative value.
    dev->xscale = (PLFLT) ( PIXELS_X - 1 ) / dev->width;
    dev->yscale = (PLFLT) ( PIXELS_Y - 1 ) / dev->height;

    pldebug( "wingdi", "Scale = (%f %f) (FLT)\n",
        dev->xscale, dev->yscale );

    // Need to get the DPI information from Windows
    // HORZRES/VERTRES = Width/Height in pixels
    // HORZSIZE/VERTSIZE = Width/Height in millimeters
    pldebug( "wingdi", "Original xdpi = %f ydpi = %f\n",
        pls->xdpi, pls->ydpi );
    //dev->xdpmm = GetDeviceCaps( dev->hdc, HORZRES )
    //			/ GetDeviceCaps( dev->hdc, HORZSIZE );
    //pls->xdpi = dev->xdpmm * 25.4;
    //dev->ydpmm = GetDeviceCaps( dev->hdc, VERTRES )
    //			/ GetDeviceCaps( dev->hdc, VERTSIZE );
    //pls->ydpi = dev->ydpmm * 25.4;
    pls->xdpi  = GetDeviceCaps( dev->hdc, LOGPIXELSX );
    dev->xdpmm = pls->xdpi / 25.4;
    pls->ydpi  = GetDeviceCaps( dev->hdc, LOGPIXELSY );
    dev->ydpmm = pls->ydpi / 25.4;

    pldebug( "wingdi", "New xdpi = %f ydpi = %f\n",
        pls->xdpi, pls->ydpi );
    pldebug( "wingdi", "Windows reports xdpi = %d ydpi = %d\n",
        GetDeviceCaps( dev->hdc, LOGPIXELSX ),
        GetDeviceCaps( dev->hdc, LOGPIXELSY ) );

    // Set the mapping from pixels to mm
    plP_setpxl( dev->xscale * pls->xdpi / 25.4,
        dev->yscale * pls->ydpi / 25.4 );

    // Set the physical limits for this device.  See the
    // previous comment about the -1.
    plP_setphy( 0, PIXELS_X - 1,
        0, PIXELS_Y - 1 );
}

//--------------------------------------------------------------------------
//  static void PrintPage ( PLStream *pls )
//
//     Function brings up a standard printer dialog and, after the user
//     has selected a printer, replots the current page to the windows
//     printer.
//--------------------------------------------------------------------------
static void PrintPage( PLStream *pls )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    PRINTDLGEX        Printer;
    DOCINFO           docinfo;
    DEVMODE           * hDevMode;
    struct wingdi_Dev *push;      // A copy of the entire structure

    // Reset the docinfo structure to 0 and set it's fields up
    // This structure is used to supply a name to the print queue
    ZeroMemory( &docinfo, sizeof ( docinfo ) );
    docinfo.cbSize      = sizeof ( docinfo );
    docinfo.lpszDocName = _T( "Plplot Page" );

    // Set the defaults for the printer device
    // Allocate a moveable block of memory.  Must use GlobalAlloc because
    // HeapAlloc is not moveable.
    hDevMode = GlobalAlloc( GHND, sizeof ( DEVMODE ) );
    if ( hDevMode == NULL )
    {
        plwarn( "wingdi:  Failed to allocate memory for printer defaults\n" );
        return;
    }
    ZeroMemory( hDevMode, sizeof ( DEVMODE ) );
    hDevMode->dmSpecVersion = DM_SPECVERSION;
    hDevMode->dmSize        = sizeof ( DEVMODE );
    hDevMode->dmFields      = DM_ORIENTATION;
    hDevMode->dmOrientation = DMORIENT_LANDSCAPE;

    // Reset out printer structure to zero and initialise it
    ZeroMemory( &Printer, sizeof ( PRINTDLGEX ) );
    Printer.lStructSize = sizeof ( PRINTDLGEX );
    Printer.hwndOwner   = dev->plot;
    Printer.hDevMode    = hDevMode;
    // Disable page ranges, default to collated output,
    // and return the device context (used to generate the output)
    Printer.Flags = PD_NOPAGENUMS | PD_NOCURRENTPAGE | PD_NOSELECTION
                    | PD_COLLATE | PD_RETURNDC;
    // Currently, page ranges is disabled.  This code
    // is left as a placeholder in case print ranges
    // is allowed in the future. There is no mechanism
    // implemented that facilitates the user interaction
    // on selecting pages, so it is best to turn it off
    // for now.
    Printer.nPageRanges    = 0;
    Printer.nMaxPageRanges = 0;
    Printer.lpPageRanges   = NULL;
    Printer.nMinPage       = 0;
    Printer.nMaxPage       = 0;
    // Other print parameter defaults
    Printer.nCopies = 1;
    //Printer.nPropertyPages = ARRAY_SIZE( hPrintPropSheetList ),
    //Printer.lphPropertyPages = hPrintPropSheetList;
    Printer.nStartPage = START_PAGE_GENERAL;

    // Call the printer dialog function.
    // If the user has clicked on "Print", then we will continue
    // processing and print out the page.
    if ( PrintDlgEx( &Printer ) == S_OK
         && Printer.dwResultAction == PD_RESULT_PRINT )
    {
        // Before doing anything, we will take some backup copies
        // of the existing values for page size and the like, because
        // all we are going to do is a quick and dirty modification
        // of plplot's internals to match the new page size and hope
        // it all works out ok. After that, we will manip the values,
        // and when all is done, restore them.
        push = HeapAlloc( wingdi_heap, 0, sizeof ( struct wingdi_Dev ) );
        if ( push != NULL )
        {
            BusyCursor( dev );

            // Save all the state information of this device
            memcpy( push, dev, sizeof ( struct wingdi_Dev ) );

            // Change the device context to the printer
            dev->hdc = Printer.hDC;
            UpdatePageMetrics( pls, DEV_PRINTER );

            // Now the stuff that actually does the printing !!
            StartDoc( dev->hdc, &docinfo );
            plRemakePlot( pls );
            EndDoc( dev->hdc );

            // Now to undo everything back to what it was for the screen
            memcpy( dev, push, sizeof ( struct wingdi_Dev ) );
            UpdatePageMetrics( pls, DEV_WINDOW );

            HeapFree( wingdi_heap, 0, push );
            NormalCursor( dev );

            // Force a redraw to make sure the plot area is clean of
            // the leftovers from the print menu
            //RedrawWindow( dev->plot,
            //			  NULL, NULL,
            //			  RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW );
        }
        else
        {
            plwarn( "wingdi: Unable to save state for print" );
        }
    }

    // Cleanup after printing
    if ( Printer.hDC != NULL )
        DeleteDC( Printer.hDC );
    if ( Printer.hDevMode != NULL )
        DeleteDC( Printer.hDevMode );
    if ( Printer.hDevNames != NULL )
        DeleteDC( Printer.hDevNames );
    // Free allocated memory
    if ( hDevMode )
        GlobalFree( hDevMode );
}

static void
wait_for_user_input( PLStream *pls )
{
    struct wingdi_Dev * dev = (struct wingdi_Dev *) pls->dev;
    MSG msg;

    pldebug( "wingdi", "Waiting for user input\n" );

    // Update the state and the message in the status bar
    dev->state = DEV_WAITING;
    update_status_bar( dev );

    // Process messages in the queue or until we are no longer waiting
    while ( GetMessage( &msg, NULL, 0, 0 ) && dev->state != DEV_ACTIVE )
    {
        TranslateMessage( &msg );
        switch ( (int) msg.message )
        {
        case WM_CONTEXTMENU:
        case WM_RBUTTONDOWN:
            TrackPopupMenu( plot_popup_menu,
                TPM_CENTERALIGN | TPM_RIGHTBUTTON,
                LOWORD( msg.lParam ),
                HIWORD( msg.lParam ),
                0,
                dev->plot,
                NULL );
            break;

        case WM_CHAR:
            if ( ( (TCHAR) ( msg.wParam ) == 32 ) ||
                 ( (TCHAR) ( msg.wParam ) == 13 ) )
            {
                dev->state = DEV_ACTIVE;
                update_status_bar( dev );
            }
            else if ( ( (TCHAR) ( msg.wParam ) == 27 ) ||
                      ( (TCHAR) ( msg.wParam ) == 'q' ) ||
                      ( (TCHAR) ( msg.wParam ) == 'Q' ) )
            {
                dev->state = DEV_ACTIVE;
                update_status_bar( dev );
                PostQuitMessage( 0 );
            }
            break;

        case WM_LBUTTONDBLCLK:
            pldebug( "wingdi", "WM_LBUTTONDBLCLK\n" );
            dev->state = DEV_ACTIVE;
            update_status_bar( dev );
            break;

        case WM_COMMAND:
            switch ( LOWORD( msg.wParam ) )
            {
            case CommandPrint:
                pldebug( "wingdi", "CommandPrint\n" );
                PrintPage( pls );
                break;
            case CommandNextPage:
                pldebug( "wingdi", "CommandNextPage\n" );
                dev->state = DEV_ACTIVE;
                update_status_bar( dev );
                break;
            case CommandQuit:
                pldebug( "wingdi", "CommandQuit\n" );
                dev->state = DEV_ACTIVE;
                update_status_bar( dev );
                PostQuitMessage( 0 );
                break;
            }
            break;

        default:
            DispatchMessage( &msg );
            break;
        }
    }

    pldebug( "wingdi", "Done waiting\n" );
    dev->state = DEV_ACTIVE;
}

//--------------------------------------------------------------------------
// GetCursorCmd()
//
// Handle events connected to selecting points (modelled after xwin)
//--------------------------------------------------------------------------

static void
GetCursorCmd( PLStream *pls, PLGraphicsIn *gin )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    MSG msg;
    HCURSOR           previous;

    plGinInit( gin );

    previous = CrossHairCursor( dev );

    while ( gin->pX < 0 )
    {
        GetMessage( &msg, NULL, 0, 0 );
        TranslateMessage( &msg );
        switch ( (int) msg.message )
        {
        case WM_LBUTTONDOWN:
            gin->pX = msg.pt.x;
            gin->pY = msg.pt.y;
            gin->dX = (PLFLT) gin->pX / ( dev->width - 1 );
            gin->dY = 1.0 - (PLFLT) gin->pY / ( dev->height - 1 );

            if ( msg.wParam & MK_LBUTTON )
            {
                pldebug( "wingdi", "Left button down\n" );
                // Left button was pressed
                gin->button = 1;
                gin->state  = 0;
                //gin->keysym = 0x20;
            }
            else if ( msg.wParam & MK_MBUTTON )
            {
                pldebug( "wingdi", "Middle button down\n" );
                // Middle button was pressed
                gin->button = 3;
                gin->state  = 0;
            }
            else if ( msg.wParam & MK_RBUTTON )
            {
                pldebug( "wingdi", "Right button down\n" );
                // Right button was pressed
                gin->button = 2;
                gin->state  = 0;
            }
            break;

        case WM_LBUTTONUP:
            gin->pX = msg.pt.x;
            gin->pY = msg.pt.y;
            gin->dX = (PLFLT) gin->pX / ( dev->width - 1 );
            gin->dY = 1.0 - (PLFLT) gin->pY / ( dev->height - 1 );

            if ( msg.wParam & MK_LBUTTON )
            {
                pldebug( "wingdi", "Left button up\n" );
                // Left button was pressed
                gin->button = 1;
                gin->state  = 0x100;
            }
            else if ( msg.wParam & MK_MBUTTON )
            {
                pldebug( "wingdi", "Middle button up\n" );
                // Middle button was pressed
                gin->button = 3;
                gin->state  = 0x10000;
            }
            else if ( msg.wParam & MK_RBUTTON )
            {
                pldebug( "wingdi", "Right button up\n" );
                // Right button was pressed
                gin->button = 2;
                gin->state  = 0x1000;
            }
            break;

        case WM_CHAR:
            gin->pX = msg.pt.x;
            gin->pY = msg.pt.y;
            gin->dX = (PLFLT) gin->pX / ( dev->width - 1 );
            gin->dY = 1.0 - (PLFLT) gin->pY / ( dev->height - 1 );

            gin->button = 0;
            gin->state  = 0;
            gin->keysym = msg.wParam;

            break;
        }
    }

    // Restore the previous cursor
    SetCursor( previous );
}

//--------------------------------------------------------------------------
//    static void CopySCRtoBMP(PLStream *pls)
//       Function copies the screen contents into a bitmap which is
//       later used for fast redraws of the screen (when it gets corrupted)
//       rather than remaking the plot from the plot buffer.
//--------------------------------------------------------------------------
static void CopySCRtoBMP( PLStream *pls )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    RECT rect;
    HGDIOBJ           previous;

    // Delete the existing bitmap before creating a new one
    if ( dev->hdc_bmp != NULL )
        DeleteDC( dev->hdc_bmp );
    if ( dev->bitmap != NULL )
        DeleteObject( dev->bitmap );

    dev->hdc_bmp = CreateCompatibleDC( dev->hdc );
    GetClientRect( dev->plot, &rect );
    dev->bitmap = CreateCompatibleBitmap( dev->hdc,
        rect.right, rect.bottom );
    previous = SelectObject( dev->hdc_bmp, dev->bitmap );
    BitBlt( dev->hdc_bmp, 0, 0, rect.right, rect.bottom, dev->hdc, 0, 0, SRCCOPY );
    SelectObject( dev->hdc_bmp, previous );
}

//--------------------------------------------------------------------------
// static void Erase ( PLStream *pls )
//
// This function erases the client area of a window
//--------------------------------------------------------------------------
static void
Erase( PLStream *pls )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    COLORREF          previous_color;
    RECT rect;

    pldebug( "wingdi", "  Erasing window\n" );
    //
    //    This is a new "High Speed" way of filling in the background.
    //    supposedly this executes faster than creating a brush and
    //    filling a rectangle - go figure ?
    //
    if ( dev->type == DEV_WINDOW )
    {
        // NOTE:  Should GetUpdateRect be used instead?
        GetClientRect( dev->plot, &rect );
        previous_color = SetBkColor( dev->hdc,
            RGB( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b ) );
        ExtTextOut( dev->hdc, 0, 0, ETO_OPAQUE, &rect, _T( "" ), 0, 0 );
        SetBkColor( dev->hdc, previous_color );
    }
    else
    {
        rect.left   = 0;
        rect.top    = 0;
        rect.right  = GetDeviceCaps( dev->hdc, HORZRES );
        rect.bottom = GetDeviceCaps( dev->hdc, VERTRES );
    }
}

//--------------------------------------------------------------------------
// static void Resize( PLStream *pls )
//
// This function regenerates a plot by updating the page metrics and then
// using the plot buffer to recreate the plot.  This function assumes that is
// being called from a WM_PAINT message, thus it does not do anything to force
// a redraw.
//--------------------------------------------------------------------------
static void Resize( PLStream *pls )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    RECT rect;
    enum _dev_state   current;

    pldebug( "wingdi", "Resizing\n" );

    // Only resize the window IF plplot is not busy
    if ( dev->state == DEV_WAITING
         || dev->state == DEV_ACTIVE
         || dev->state == DEV_DRAWING )
    {
        GetClientRect( dev->plot, &rect );
        pldebug( "wingdi", "  Size = [%d %d] [%d %d]\n",
            rect.left, rect.top,
            rect.right, rect.bottom );

        // Check to make sure it isn't just minimised (i.e. zero size)
        if ( ( rect.right > 0 ) && ( rect.bottom > 0 ) )
        {
            UpdatePageMetrics( pls, DEV_WINDOW );

            // Save the current state because remaking the plot
            // will change it when the BOP is executed.
            current = dev->state;
            plRemakePlot( pls );
            dev->state = current;
            update_status_bar( dev );
        }
    }
    else
    {
        pldebug( "wingdi", "  No action taken, state = %d\n", dev->state );
    }
    pldebug( "wingdi", "Resizing done\n" );
}

//--------------------------------------------------------------------------
// This is the window function for the frame window.
//--------------------------------------------------------------------------
LRESULT CALLBACK PlplotFrameProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam )   // second message parameter
{
    struct wingdi_Dev *dev = NULL;

    // Try to get the address to the device data for this window
#ifdef _WIN64
    dev = (struct wingdi_Dev *) GetWindowLongPtr( hwnd, GWLP_USERDATA );
#else
    dev = (struct wingdi_Dev *) GetWindowLongPtr( hwnd, GWL_USERDATA );
#endif

    switch ( uMsg )
    {
    case WM_CREATE:
        // Initialize the window.
    {
        HWND hStatus;

        // Create the status bar only when the frame is being created
        hStatus = CreateWindowEx(
            0,
            STATUSCLASSNAME,
            NULL,
            WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0,
            hwnd,
            (HMENU) StatusBarId,
            GetModuleHandle( NULL ),
            NULL );
        if ( hStatus != NULL )
        {
            int status_widths[] = { 100, 200, 300, -1 };

            SendMessage( hStatus, SB_SETPARTS,
                (WPARAM) ARRAY_SIZE( status_widths ),
                (LPARAM) status_widths );
            SendMessage( hStatus, SB_SETTEXT,
                (WPARAM) 0,
                (LPARAM) (LPSTR) TEXT( "Active" ) );
        }
        else
        {
            MessageBox( hwnd, "Could not create status bar.",
                "Error", MB_OK | MB_ICONERROR );
        }
    }
        return 0;

    case WM_SIZE:
    {
        // Set the size and position of the window, including the
        // child controls
        HWND hStatus, hPlot;
        RECT rStatus, rFrame;
        int  plot_height;

        // Get the client area of the frame
        GetClientRect( hwnd, &rFrame );

        // Get the status bar size
        hStatus = GetDlgItem( hwnd, StatusBarId );
        SendMessage( hStatus, WM_SIZE, 0, 0 );
        GetWindowRect( hStatus, &rStatus );

        // Set the size of the plot area
        hPlot       = GetDlgItem( hwnd, PlotAreaId );
        plot_height = rFrame.bottom - ( rStatus.bottom - rStatus.top );
        SetWindowPos( hPlot,                            // Handle to the plot area window
            NULL,
            0, 0,
            rFrame.right, plot_height,
            SWP_NOZORDER | SWP_SHOWWINDOW );
    }
        return 0;

    case WM_DESTROY:
        // Clean up window-specific data objects.
        return 0;

    //
    // Process other messages.
    //
    default:
        return DefWindowProc( hwnd, uMsg, wParam, lParam );
    }

    return 0;
}

//--------------------------------------------------------------------------
// This is the window function for the plot area. Whenever a message is
// dispatched using DispatchMessage (or sent with SendMessage) this function
// gets called with the contents of the message.
//--------------------------------------------------------------------------
LRESULT CALLBACK PlplotPlotAreaProc( HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    PLStream          *pls = NULL;
    struct wingdi_Dev *dev = NULL;

    // During WM_CREATE message, the PLStream pointer is not set, thus
    // we need to handle this message without attempting to getting the user data
    if ( nMsg == WM_CREATE )
    {
        // Signal that the message was handled
        return ( 0 );
    }

    // Try to get the address to pls for this window
#ifdef _WIN64
    pls = (PLStream *) GetWindowLongPtr( hwnd, GWLP_USERDATA );
#else
    pls = (PLStream *) GetWindowLongPtr( hwnd, GWL_USERDATA );
#endif

    // If we did not get a valid pointer, pass the message to the
    // default message handler
    if ( pls == NULL )
    {
        return DefWindowProc( hwnd, nMsg, wParam, lParam );
    }

    dev = (struct wingdi_Dev *) pls->dev;

    //
    // Process the windows messages
    //
    // Everything except WM_CREATE is done here and it is generally hoped that
    // pls and dev are defined already by this stage.
    // That will be true MOST of the time. Some times WM_PAINT will be called
    // before we get to initialise the user data area of the window with the
    // pointer to the windows plplot stream
    //

    switch ( nMsg )
    {
    case WM_DESTROY:
        pldebug( "wingdi", "WM_DESTROY\n" );
        PostQuitMessage( 0 );
        return ( 0 );
        break;

    case WM_PAINT:
        // A WM_PAINT message gets sent on an expose, resize, and move
        // events.  On expose and move events, a blit of a bitmap will
        // be performed.  On a resize, the plot needs to be regenerated.
        // Because a WM_PAINT is sent after the triggering event
        // (e.g. a move), the triggering event is responsible for ensuring
        // a valid bitmap exists.  Thus, this message handler only needs
        // to blit the bitmap to the window.
        pldebug( "wingdi", "WM_PAINT state = %d\n", dev->state );

        // Check to see if there is an area that needs to be redrawn.
        // Per the MSDN document, calling GetUpdateRect with a NULL RECT
        // value will determine if there is an update region.  BeginPaint()
        // will provide the update region in rcPaint.
        if ( GetUpdateRect( dev->plot, NULL, FALSE ) )
        {
            // Yes there is an update region, start the redraw
            PAINTSTRUCT ps;
            HGDIOBJ     previous;

            BusyCursor( dev );
            BeginPaint( dev->plot, &ps );

            pldebug( "wingdi", "  Need to redraw area (%d,%d) (%d,%d)\n",
                ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right, ps.rcPaint.bottom );
            pldebug( "wingdi", "  Erase status = %d\n", ps.fErase );
            pldebug( "wingdi", "  Device state = %d\n", dev->state );

            // If we have a valid bitmap and are not currently drawing
            // a new plot, then we can blit the bitmap to handle the
            // redraw.  On a resize, this will result in the bitmap being
            // clipped during the resize.  A StretchBlt could be used to
            // rescale the bitmap; however, not all devices support
            // stretch blits.
            if ( dev->bitmap != NULL && dev->state != DEV_DRAWING )
            {
                // A bitmap exists, thus only a blit is required
                pldebug( "wingdi", "  Blit image\n" );
                previous = SelectObject( dev->hdc_bmp, dev->bitmap );
                BitBlt( dev->hdc,
                    ps.rcPaint.left, ps.rcPaint.top,
                    ps.rcPaint.right, ps.rcPaint.bottom,
                    dev->hdc_bmp,
                    ps.rcPaint.left, ps.rcPaint.top,
                    SRCCOPY );
                SelectObject( dev->hdc_bmp, previous );
            }
            else
            {
                pldebug( "wingdi",
                    "  No paint action bitmap = %lx state = \n",
                    dev->bitmap, dev->state );
            }

            EndPaint( dev->plot, &ps );
            NormalCursor( dev );
        }
        else
        {
            pldebug( "wingdi", "  No update area to paint\n" );
        }
        pldebug( "wingdi", "WM_PAINT exit\n" );
        // Signal that the message was processed
        return ( 0 );
        break;

    case WM_SIZE:
        pldebug( "wingdi", "WM_SIZE wParam = %d\n", wParam );
        // The size might have changed.  We only care about
        // Maximized events, a drag resize, or a restore.
        if ( wParam == SIZE_MAXIMIZED )
        {
            // The window was maximized, which is a resize
            pldebug( "wingdi", "  Window maximized\n" );
            Resize( pls );
        }
        else if ( dev->state == DEV_SIZEMOVE )
        {
            // This is a drag resize.  Must check before the SIZE_RESTORED
            // because a drag resize is also a SIZE_RESTORED.
            pldebug( "wingdi", "  Window size/moved\n" );

            // Change the state to indicate that the window has changed
            // size and not just moved.
            dev->state = DEV_RESIZE;
            pldebug( "wingdi", "  New state %d\n", dev->state );
            update_status_bar( dev );
        }
        else if ( wParam == SIZE_RESTORED )
        {
            // This could be a restore from a maximized or minimized state.
            // Unless code is added to detect the difference (i.e. look for
            // an earlier SIZE_MINIMIZED), just treat it as a resize
            pldebug( "wingdi", "  Window restored\n" );
            Resize( pls );
        }
        else
        {
            pldebug( "wingdi", "  Unknowing sizing action\n" );
        }
        pldebug( "wingdi", "WM_SIZE exit\n" );
        // Indicate that this message was processed
        return ( 0 );
        break;

    case WM_ENTERSIZEMOVE:
        pldebug( "wingdi", "WM_ENTERSIZEMOVE\n" );
        pldebug( "wingdi", "  Save state %d\n", dev->state );
        dev->prev_state = dev->state;
        // Indicate that we might be sizing or moving the window
        dev->state = DEV_SIZEMOVE;
        update_status_bar( dev );
        return ( 0 );
        break;

    case WM_EXITSIZEMOVE:
        pldebug( "wingdi", "WM_EXITSIZEMOVE\n" );
        // If the window has been resized, regenerate the plot
        // for the new window dimensions
        if ( dev->state == DEV_RESIZE || dev->state == DEV_SIZEMOVE )
        {
            pldebug( "wingdi", "  Restore state %d\n", dev->prev_state );
            // Restore the previous state before handling the resize
            // The resize routine needs the original state to preserve
            // the state when the buffer is replayed.
            dev->state = dev->prev_state;
            update_status_bar( dev );
            Resize( pls );
        }
        return ( 0 );
        break;

    case WM_ERASEBKGND:
        // Determine if the window needs to be erased based
        // on the current state.
        // DEV_WAITING = No erase necessary, the next WM_PAINT
        //    will repaint the affected area
        pldebug( "wingdi", "WM_ERASEBKGND state = %d\n", dev->state );

        if ( dev->state != DEV_WAITING )
        {
            Erase( pls );
            // Indicate that the client area was erased
            return ( 1 );
        }

        // Indicate no action was taken
        pldebug( "wingdi", "  No erase action taken\n" );
        return ( 0 );
        break;

    case WM_MOUSEMOVE:
    {
        char   mesg[80];
        int    xw, yw;
        double x, y;

        xw = GET_X_LPARAM( lParam );
        yw = GET_Y_LPARAM( lParam );
        x  = (double) xw * dev->xscale;
        y  = (double) ( dev->height - yw ) * dev->yscale;

        snprintf( mesg, sizeof ( mesg ), "%5.1lf", x );
        if ( dev->status_bar != NULL )
            SendMessage( dev->status_bar,
                SB_SETTEXT,
                (WPARAM) 1,
                (LPARAM) mesg );

        snprintf( mesg, sizeof ( mesg ), "%5.1lf", y );
        if ( dev->status_bar != NULL )
            SendMessage( dev->status_bar,
                SB_SETTEXT,
                (WPARAM) 2,
                (LPARAM) mesg );
    }

        // Indicate that we did not process this message
        return ( 1 );
        break;

    case WM_COMMAND:
        pldebug( "wingdi", "WM_COMMAND\n" );
        return ( 0 );
        break;
    }

    // If we don't handle a message completely we hand it to the system
    // provided default window function.
    return DefWindowProc( hwnd, nMsg, wParam, lParam );
}

//--------------------------------------------------------------------------
// wingdi_module_initialize()
//
// Handles the initialization of window classes and module global
// variables.
//--------------------------------------------------------------------------
static void
wingdi_module_initialize( void )
{
    INITCOMMONCONTROLSEX init_controls;

    // Return if the module has been initialized
    // Use a postfix increment so that wingdi_streams is incremented
    // every time this function is called. That ensures a valid count
    // for the number of streams that are created.  This allows the
    // module cleanup to free resources when the last stream is closed.
    if ( wingdi_streams++ > 0 )
        return;

    pldebug( "wingdi", "module init\n" );

    // Initialize common controls
    init_controls.dwSize = sizeof ( INITCOMMONCONTROLSEX );
    init_controls.dwICC  = ICC_BAR_CLASSES;
    if ( !InitCommonControlsEx( &init_controls ) )
    {
        plwarn( "wingdi:  Failed to initialize common Window controls\n" );
    }

    //
    // Initialize the frame window class
    //

    // Initialize the entire structure to zero.
    memset( &frame_window_class, 0, sizeof ( WNDCLASSEX ) );

    // Set the name of the plot window class
    frame_window_class.lpszClassName = frame_window_class_name;

    // Set the size of the window class structure, to include
    // any extra data that might be passed
    frame_window_class.cbSize = sizeof ( WNDCLASSEX );

    // All windows of this class redraw when resized.
    frame_window_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS
                               | CS_OWNDC;

    // Set the callback function.
    frame_window_class.lpfnWndProc = PlplotFrameProc;

    // This class is used with the current program instance.
    frame_window_class.hInstance = GetModuleHandle( NULL );

    // Use standard application icon and arrow cursor provided by the OS
    frame_window_class.hIcon   = LoadIcon( NULL, IDI_APPLICATION );
    frame_window_class.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
    frame_window_class.hCursor = LoadCursor( NULL, IDC_ARROW );

    // Generic solid background for the frame window
    frame_window_class.hbrBackground = (HBRUSH) COLOR_WINDOW;

    // Do not allocate extra space for the callback
    frame_window_class.cbWndExtra = sizeof ( struct wingdi_Dev * );

    // Now register the window class for use.
    RegisterClassEx( &frame_window_class );

    //
    // Initialize the plot area class
    //

    // Initialize the entire structure to zero.
    memset( &plot_area_class, 0, sizeof ( WNDCLASSEX ) );

    // Set the name of the plot window class
    plot_area_class.lpszClassName = plot_area_class_name;

    // Set the size of the window class structure, to include
    // any extra data that might be passed
    plot_area_class.cbSize = sizeof ( WNDCLASSEX );

    // All windows of this class redraw when resized.
    plot_area_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS
                            | CS_OWNDC;

    // Set the callback function.
    plot_area_class.lpfnWndProc = PlplotPlotAreaProc;

    // This class is used with the current program instance.
    plot_area_class.hInstance = GetModuleHandle( NULL );

    // Use standard application icon and arrow cursor provided by the OS
    plot_area_class.hIcon   = LoadIcon( NULL, IDI_APPLICATION );
    plot_area_class.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
    plot_area_class.hCursor = LoadCursor( NULL, IDC_ARROW );

    // Handle the erase background in the callback function
    plot_area_class.hbrBackground = NULL;

    // Allocate extra space for a window instance to store the
    // pointer to the plot stream (PLStream)
    plot_area_class.cbWndExtra = sizeof ( PLStream * );

    // Now register the window class for use.
    RegisterClassEx( &plot_area_class );

    //
    // Create the popup menu used by the plot window
    //
    plot_popup_menu = CreatePopupMenu();
    AppendMenu( plot_popup_menu, MF_STRING, CommandPrint, _T( "Print" ) );
    AppendMenu( plot_popup_menu, MF_STRING, CommandNextPage, _T( "Next Page" ) );
    AppendMenu( plot_popup_menu, MF_STRING, CommandQuit, _T( "Quit" ) );

    //
    // Create a private heap to use for memory allocation.
    // This will keep memory separate from the overall program
    // and does not have the overhead of GlobalAlloc().  Requires
    // a minimum of Windows XP or Windows Server 2003
    //
    wingdi_heap = HeapCreate( 0, INITIAL_HEAP_SIZE, 0 );
    if ( wingdi_heap == NULL )
    {
        //plexit("wingdi: Unable to allocate heap of size %d bytes",
        //	   INITIAL_HEAP_SIZE);
        plexit( "wingdi: Unable to allocate heap" );
    }
}

static void wingdi_module_cleanup( void )
{
    struct _font_entry *ptr;

    wingdi_streams--;
    if ( wingdi_streams > 0 )
        return;

    DeleteMenu( plot_popup_menu, CommandPrint, 0 );
    DeleteMenu( plot_popup_menu, CommandNextPage, 0 );
    DeleteMenu( plot_popup_menu, CommandQuit, 0 );
    DestroyMenu( plot_popup_menu );

    if ( !UnregisterClass( plot_area_class_name, plot_area_class.hInstance ) )
    {
        plexit( "wingdi: Failed to unregister window class" );
    }
    if ( !UnregisterClass( frame_window_class_name, frame_window_class.hInstance ) )
    {
        plexit( "wingdi: Failed to unregister window class" );
    }

    while ( font_list != NULL )
    {
        ptr = font_list;
        DeleteObject( ptr->font );
        font_list = ptr->next;
        HeapFree( wingdi_heap, 0, ptr );
    }

    if ( HeapDestroy( wingdi_heap ) == 0 )
    {
        plexit( "wingdi: Failed to destroy heap" );
    }
}

//--------------------------------------------------------------------------
// plD_init_wingdi()
//
// Initialize device (terminal).
//--------------------------------------------------------------------------

void
plD_init_wingdi( PLStream *pls )
{
    struct wingdi_Dev *dev;
    int    status_bar       = 0; // Default to no status bar
    int    full_viewer      = 0; // Default to the minimal viewer
    DrvOpt wingdi_options[] = {
        { "full",      DRV_INT, &full_viewer, "Enable full function viewer (0|1)" },
        { "statusbar", DRV_INT, &status_bar,  "Enable status bar (0|1)"           },
        { NULL,        DRV_INT, NULL,         NULL                                }
    };
    TCHAR  *program;
#ifdef UNICODE
    int    programlength;
#endif

    pls->debug = 1;
    pldebug( "wingdi", "Device Init\n" );

    // Initialize the module
    wingdi_module_initialize();

    // Allocate and initialize device-specific data
    if ( pls->dev != NULL )
        free( (void *) pls->dev );

    pls->dev = calloc( 1, (size_t) sizeof ( struct wingdi_Dev ) );
    if ( pls->dev == NULL )
        plexit( "plD_init_wingdi_Dev: Out of memory." );

    // Shortcut to the device specific data structure
    dev = (struct wingdi_Dev *) pls->dev;

    pls->icol0 = 1;                    // Set a fall back pen color in case user doesn't

    pls->termin       = 1;             // interactive device
    pls->graphx       = GRAPHICS_MODE; // No text mode for this driver
    pls->dev_fill0    = 1;             // driver can do solid area fills
    pls->dev_xor      = 1;             // driver supports xor mode
    pls->dev_clear    = 1;             // driver supports clear
    pls->dev_text     = 1;             // driver supports text
    pls->dev_gradient = 0;             // driver not support gradient fills
    pls->dev_dash     = 0;             // driver can not do dashed lines (yet)
    pls->plbuf_write  = 1;             // driver uses the buffer for redraws

    if ( !pls->colorset )
        pls->color = 1;

    // Check for and set up driver options
    plParseDrvOpts( wingdi_options );

    // Set the appropriate viewer type
    if ( full_viewer )
        dev->viewer = VIEWER_FULL;
    else
        dev->viewer = VIEWER_MINIMAL;

    // Determine which features should be enabled
    if ( status_bar )
        dev->feature.status_bar = 1;
    else
        dev->feature.status_bar = 0;

    // Set up the initial device parameters.  This will be updated
    // after the plot window is initialized.
    if ( pls->xlength <= 0 || pls->ylength <= 0 )
    {
        // use default width, height of 800x600 if not specified by
        // -geometry option or plspage
        plspage( 0., 0., 800, 600, 0, 0 );
    }

    dev->width  = pls->xlength - 1;     // should I use -1 or not???
    dev->height = pls->ylength - 1;

#ifdef UNICODE
    //convert the program name to wide char
    programlength = strlen( pls->program ) + 1;
    program       = malloc( programlength * sizeof ( TCHAR ) );
    MultiByteToWideChar( CP_UTF8, 0, pls->program, programlength, program, programlength );
#else
    program = pls->program;
#endif

    if ( dev->viewer == VIEWER_FULL )
    {
        // Create our main window using the plot window class
        dev->frame = CreateWindowEx(
            WS_EX_WINDOWEDGE + WS_EX_LEFT,
            frame_window_class_name,                   // Class name
            program,                                   // Caption
            WS_OVERLAPPEDWINDOW                        // Window style
            | WS_CLIPCHILDREN,                         // Exclude child area from parent
            pls->xoffset,                              // Initial x (use default)
            pls->yoffset,                              // Initial y (use default)
            pls->xlength,                              // Initial x size (use default)
            pls->ylength,                              // Initial y size (use default)
            NULL,                                      // No parent window
            NULL,                                      // No menu
            frame_window_class.hInstance,              // This program instance
            NULL                                       // Creation parameters
            );

        // Create the plot area
        dev->plot = CreateWindowEx(
            0,
            plot_area_class_name,                      // Class name
            NULL,                                      // Caption
            WS_CHILD | WS_VISIBLE,                     // Style
            0, 0, 0, 0,                                // Position information
            dev->frame,                                // Parent window
            (HMENU) PlotAreaId,                        // No menu
            plot_area_class.hInstance,                 // This program instance
            NULL                                       // Creation parameters
            );
    }
    else
    {
        // Create the plot area
        dev->plot = CreateWindowEx(
            WS_EX_WINDOWEDGE + WS_EX_LEFT,
            plot_area_class_name,                      // Class name
            NULL,                                      // Caption
            WS_OVERLAPPEDWINDOW                        // Style
            | WS_VISIBLE,
            pls->xoffset,                              // Initial x (use default)
            pls->yoffset,                              // Initial y (use default)
            pls->xlength,                              // Initial x size (use default)
            pls->ylength,                              // Initial y size (use default)
            NULL,                                      // Parent window
            NULL,                                      // No menu
            plot_area_class.hInstance,                 // This program instance
            NULL                                       // Creation parameters
            );
    }

    if ( dev->plot == NULL )
    {
        plexit( "wingdi:  Failed to create plot area\n" );
    }

#ifdef UNICODE
    free( program );
#endif

    // Attach a pointer to the stream to the window's user area
    // this pointer will be used by the windows call back for
    // process this window
#ifdef _WIN64
    SetWindowLongPtr( dev->plot, GWLP_USERDATA, (LONG_PTR) pls );
    if ( dev->frame )
        SetWindowLongPtr( dev->frame, GWLP_USERDATA, (LONG_PTR) dev );
#else
    SetWindowLongPtr( dev->plot, GWL_USERDATA, (LONG) pls );
    if ( dev->frame )
        SetWindowLongPtr( dev->frame, GWL_USERDATA, (LONG) dev );
#endif

    // Get the device context of the window that was created
    dev->hdc = GetDC( dev->plot );

    // Set the initial color for the drawing pen
    plD_state_wingdi( pls, PLSTATE_COLOR0 );

    // Display the window which we just created (using the nShow
    // passed by the OS, which allows for start minimized and that
    // sort of thing).
    if ( dev->viewer == VIEWER_FULL )
    {
        ShowWindow( dev->frame, SW_SHOWDEFAULT );
        SetForegroundWindow( dev->frame );
    }
    else
    {
        ShowWindow( dev->plot, SW_SHOWDEFAULT );
        SetForegroundWindow( dev->plot );
    }

    if ( dev->feature.status_bar )
    {
        // Get the handle for the status bar.  This will
        // make it easier to update the contents
        dev->status_bar = GetDlgItem( dev->frame, StatusBarId );
    }

    //  Now we have to find out, from windows, just how big our drawing area is
    //  when we specified the page size earlier on, that includes the borders,
    //  title bar etc... so now that windows has done all its initialisations,
    //  we will ask how big the drawing area is, and tell plplot
    UpdatePageMetrics( pls, DEV_WINDOW );
    plspage( pls->xdpi, pls->ydpi, 0, 0, 0, 0 );

    // Set fill rule.
    if ( pls->dev_eofill )
        SetPolyFillMode( dev->hdc, ALTERNATE );
    else
        SetPolyFillMode( dev->hdc, WINDING );

    // Erase the plot window
    Erase( pls );

    // Indicate that the plot window is active
    dev->state = DEV_ACTIVE;
    update_status_bar( dev );
}

//--------------------------------------------------------------------------
// plD_line_wingdi()
//
// Draw a line in the current color from (x1,y1) to (x2,y2).
//--------------------------------------------------------------------------

static void
plD_line_wingdi( PLStream *pls, short x1a, short y1a, short x2a, short y2a )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    HGDIOBJ           previous_obj;
    POINT             points[2];

    points[0].x = (LONG) ( x1a / dev->xscale );
    points[1].x = (LONG) ( x2a / dev->xscale );
    points[0].y = (LONG) ( dev->height - ( y1a / dev->yscale ) );
    points[1].y = (LONG) ( dev->height - ( y2a / dev->yscale ) );

    previous_obj = SelectObject( dev->hdc, dev->pen );

    if ( points[0].x != points[1].x || points[0].y != points[1].y )
    {
        Polyline( dev->hdc, points, 2 );
    }
    else
    {
        SetPixel( dev->hdc, points[0].x, points[0].y, dev->color );
    }
    SelectObject( dev->hdc, previous_obj );
}

//--------------------------------------------------------------------------
// plD_polyline_wingdi()
//
// Draw a polyline in the current color.
//--------------------------------------------------------------------------

static void
plD_polyline_wingdi( PLStream *pls, short *xa, short *ya, PLINT npts )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    int i;
    POINT             *points = NULL;
    HGDIOBJ           previous_obj;

    if ( npts > 0 )
    {
        points = HeapAlloc( wingdi_heap, HEAP_ZERO_MEMORY,
            (size_t) npts * sizeof ( POINT ) );
        if ( points != NULL )
        {
            for ( i = 0; i < npts; i++ )
            {
                points[i].x = (LONG) ( xa[i] / dev->xscale );
                points[i].y = (LONG) ( dev->height - ( ya[i] / dev->yscale ) );
            }

            previous_obj = SelectObject( dev->hdc, dev->pen );
            Polyline( dev->hdc, points, npts );
            SelectObject( dev->hdc, previous_obj );
            HeapFree( wingdi_heap, 0, points );
        }
        else
        {
            plexit( "Could not allocate memory to \"plD_polyline_wingdi\"\n" );
        }
    }
}

//--------------------------------------------------------------------------
// plD_fill_polygon_wingdi()
//
// Fill polygon described in points pls->dev_x[] and pls->dev_y[].
//--------------------------------------------------------------------------
static void
plD_fill_polygon_wingdi( PLStream *pls )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    int i;
    POINT             *points = NULL;
    HGDIOBJ           previous_brush, previous_pen;
    HPEN hpen;
    HBRUSH            fillbrush;

    // Do nothing if there are no points
    if ( pls->dev_npts == 0 )
        return;

    points = HeapAlloc( wingdi_heap, HEAP_ZERO_MEMORY,
        (size_t) pls->dev_npts * sizeof ( POINT ) );

    if ( points == NULL )
        plexit( "Could not allocate memory to \"plD_fill_polygon_wingdi\"\n" );

    for ( i = 0; i < pls->dev_npts; i++ )
    {
        points[i].x = (PLINT) ( pls->dev_x[i] / dev->xscale );
        points[i].y = (PLINT) ( dev->height - ( pls->dev_y[i] / dev->yscale ) );
    }

    // Create a brush for the fill and a pen for the border
    fillbrush      = CreateSolidBrush( dev->color );
    hpen           = CreatePen( PS_SOLID, 1, dev->color );
    previous_brush = SelectObject( dev->hdc, fillbrush );
    previous_pen   = SelectObject( dev->hdc, hpen );

    // Draw the filled polygon
    Polygon( dev->hdc, points, pls->dev_npts );

    // Restore the previous objects and delete
    SelectObject( dev->hdc, previous_brush );
    DeleteObject( fillbrush );
    SelectObject( dev->hdc, previous_pen );
    DeleteObject( hpen );

    HeapFree( wingdi_heap, 0, points );
}

//--------------------------------------------------------------------------
// plD_clear_wingdi()
//
// Clears the client area of a window.
//--------------------------------------------------------------------------
static void
plD_clear_wingdi( PLStream *pls, PLINT x1, PLINT y1, PLINT x2, PLINT y2 )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    COLORREF          previous_color;
    RECT rect;

    if ( x1 >= 0 && y1 >= 0 && x2 >= 0 && y2 >= 0 )
    {
        rect.left   = (LONG) ( x1 / dev->xscale );
        rect.top    = (LONG) ( dev->height - ( y1 / dev->yscale ) );
        rect.right  = (LONG) ( x2 / dev->xscale );
        rect.bottom = (LONG) ( dev->height - ( y2 / dev->yscale ) );
    }
    else
    {
        // Invalid coordinates, erase the entire area
        GetClientRect( dev->plot, &rect );
    }

    //    This is a new "High Speed" way of filling in the background.
    //    supposedly this executes faster than creating a brush and
    //    filling a rectangle - go figure ?
    previous_color = SetBkColor( dev->hdc,
        RGB( pls->cmap0[0].r, pls->cmap0[0].g, pls->cmap0[0].b ) );
    ExtTextOut( dev->hdc, 0, 0, ETO_OPAQUE, &rect, _T( "" ), 0, 0 );
    SetBkColor( dev->hdc, previous_color );
}

struct _text_state
{
    // Font information
    PLFLT font_height;
    LONG  font_weight;
    BYTE  font_charset;
    BYTE  font_pitch;
    BYTE  font_family;
    BYTE  italic;

    // Font transformation
    PLFLT rotation, shear, stride;

    // Super/subscript state
    PLINT level;                      // super/subscript level
    PLFLT old_sscale, sscale;
    PLFLT old_soffset, soffset;
};

// set_font()
//
// Sets the current font of the plot window device context to one that
// best matches the desired attributes.  The previous font is returned so
// that the caller can restore the original font.  This routine caches the
// fonts that it creates to improve performance.
static HFONT
set_font( struct wingdi_Dev * dev,
          LONG font_height, LONG escapement,
          LONG weight, BYTE italic,
          BYTE charset,
          BYTE pitch, BYTE family )
{
    struct _font_entry *ptr = font_list;

    for ( ptr = font_list; ptr != NULL; ptr = ptr->next )
    {
        if (
            ptr->info.lfHeight == font_height
            && ptr->info.lfEscapement == escapement
            && ptr->info.lfWeight == weight
            && ptr->info.lfItalic == italic
            && ptr->info.lfCharSet == charset
            && ptr->info.lfPitchAndFamily == ( pitch | family )
            && ptr->hdc == dev->hdc
            )
        {
            return SelectObject( dev->hdc, ptr->font );
        }
    }

    // Allocate space for this font entry
    ptr = HeapAlloc( wingdi_heap, 0, sizeof ( struct _font_entry ) );

    // The font was not found, thus we need to create it
    ptr->info.lfHeight         = font_height;
    ptr->info.lfWidth          = 0;
    ptr->info.lfEscapement     = escapement;  // Escapement angle
    ptr->info.lfOrientation    = 0;           // Orientation angle
    ptr->info.lfWeight         = weight;      // Font weight (e.g. bold)
    ptr->info.lfItalic         = italic;
    ptr->info.lfUnderline      = 0;
    ptr->info.lfStrikeOut      = 0;
    ptr->info.lfCharSet        = charset;
    ptr->info.lfOutPrecision   = OUT_OUTLINE_PRECIS;
    ptr->info.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    ptr->info.lfQuality        = ANTIALIASED_QUALITY;
    ptr->info.lfPitchAndFamily = pitch | family;
    ptr->font = CreateFontIndirect( &( ptr->info ) );
    ptr->hdc  = dev->hdc;
    ptr->next = NULL;

    if ( ptr->font == NULL )
    {
        plwarn( "wingdi:  Unable to create a font, using default\n" );
        HeapFree( wingdi_heap, 0, ptr );
        return NULL;
    }
    else
    {
        if ( font_list != NULL )
        {
            ptr->next = font_list;
            font_list = ptr;
        }
        else
        {
            font_list = ptr;
        }
    }

    return SelectObject( dev->hdc, ptr->font );
}

// Find the corresponding ANSI value for a given
// Hershey character
static const char hershey_to_ansi_lookup[] =
{
    0,     0,    0,   0,   0,   0,   0,   0,   0, 0,         //  0
    0,     0,    0,   0,   0,   0,   0,   0,   0, 0,         // 10
    0,     0,    0,   0,   0,   0,   0,   0,   0, 0,         // 20
    0,     0,    0, '!', '"', '#', '$', '%', '&', '\'',      // 30
    '(', ')', '*',  '+', ',', '-', '.', '/', '0', '1',       // 40
    '2', '3', '4',  '5', '6', '7', '8', '9', ':', ';',       // 50
    '<', '=', '>',  '?', '@', 'A', 'B', 'C', 'D', 'E',       // 60
    'F', 'G', 'H',  'I', 'J', 'K', 'L', 'M', 'N', 'O',       // 70
    'P', 'Q', 'R',  'S', 'T', 'U', 'V', 'W', 'X', 'Y',       // 80
    'Z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c',       // 90
    'd', 'e', 'f',  'g', 'h', 'i', 'j', 'k', 'l', 'm',       // 100
    'n', 'o', 'p',  'q', 'r', 's', 't', 'u', 'v', 'w',       // 110
    'x', 'y', 'z',  '{', '|', '}', '~',   0,   0, 0,         // 120
};
static const char hershey_to_symbol_lookup[] =
{
    0,    0xB7, 0x2B, 0x2A, 0, 0, 0, 0,    0, 0,                //  0
    0,       0,    0,    0, 0, 0, 0, 0,    0, 0,                // 10
    0,       0,    0,    0, 0, 0, 0, 0, 0xAC, 0xAE,             // 20
    0xAD, 0xAF
};

static void
process_text_escape( struct wingdi_Dev *dev, EscText *args,
                     int *i, char *buffer, int *j,
                     struct _text_state *state )
{
    int val;

    // Get the next character to determine what the escape action is
    switch ( args->string[++( *i )] )
    {
    case 'u':     // Superscript or end of subscript
        plP_script_scale( TRUE, &state->level,
            &state->old_sscale, &state->sscale,
            &state->old_soffset, &state->soffset );

        set_font( dev,
            (LONG) ( state->font_height * state->sscale ),
            (LONG) ( state->rotation ),
            state->font_weight,
            state->italic,
            state->font_charset,
            state->font_pitch, state->font_family );
        break;
    case 'd':     // Subscript or end of superscript
        plP_script_scale( FALSE, &state->level,
            &state->old_sscale, &state->sscale,
            &state->old_soffset, &state->soffset );

        set_font( dev,
            (LONG) ( state->font_height * state->sscale ),
            (LONG) ( state->rotation ),
            state->font_weight,
            state->italic,
            state->font_charset,
            state->font_pitch, state->font_family );
        break;
    case 'f':     // Font switch
        switch ( args->string[++( *i )] )
        {
        case 'n':
            state->font_family  = FF_SWISS;
            state->italic       = 0;
            state->font_charset = ANSI_CHARSET;
            break;
        case 'r':
            state->font_family  = FF_ROMAN;
            state->italic       = 0;
            state->font_charset = ANSI_CHARSET;
            break;
        case 'i':
            state->font_family  = FF_ROMAN;
            state->italic       = 1;
            state->font_charset = ANSI_CHARSET;
            break;
        case 's':
            state->font_family  = FF_SCRIPT;
            state->italic       = 0;
            state->font_charset = ANSI_CHARSET;
            break;
        }
        set_font( dev,
            (LONG) ( state->font_height * state->sscale ),
            (LONG) ( state->rotation ),
            state->font_weight,
            state->italic,
            state->font_charset,
            state->font_pitch, state->font_family );
        break;
    case 'g':     // Greek character font
        state->font_family  = FF_DONTCARE;
        state->italic       = 0;
        state->font_charset = GREEK_CHARSET;

        set_font( dev,
            (LONG) ( state->font_height * state->sscale ),
            (LONG) ( state->rotation ),
            state->font_weight,
            state->italic,
            state->font_charset,
            state->font_pitch, state->font_family );
        break;
    case '(':     // Hershey character specified
        sscanf( args->string + *i, "%d", &val );

        if ( val > 0 && val < ARRAY_SIZE( hershey_to_ansi_lookup )
             && hershey_to_ansi_lookup[val] != 0 )
        {
            buffer[( *j )++] = hershey_to_ansi_lookup[val];
        }
        else
        {
            plwarn( "Unsupported hershey character\n" );
            // Substitute a bullet so something is displayed
            buffer[( *j )++] = 0x95;
        }
        *i += 4;
        break;
    case '[':     // Unicode character specified
        plwarn( "Unicode characters are not supported\n" );
        *i += 4;
        break;
    default:
        plwarn( "Ignoring escape code %c\n" );
        //plwarn( "Ignoring escape code %c\n", args->string[i-1] );
    }
}

//--------------------------------------------------------------------------
// plD_text_wingdi()
//
// Renders text on the string.  Support non-Unicode and Unicode strings.
//--------------------------------------------------------------------------
static void
plD_text_wingdi( PLStream * pls, EscText * args )
{
    struct wingdi_Dev  *dev = (struct wingdi_Dev *) pls->dev;
    struct _text_state state;
    PLFLT    cos_rot, sin_rot;
    PLFLT    cos_shear, sin_shear;
    int      rx, ry;
    UINT     halign, valign;
    HFONT    font = NULL, prev_font = NULL;
    COLORREF prev_color;
    int      prev_bkmode;
    int      text_segments;
    char     esc;

    // Get the escape character used to format strings
    plgesc( &esc );

    // Determine the font characteristics
    state.italic       = 0;
    state.font_weight  = FW_NORMAL;
    state.font_charset = ANSI_CHARSET;
    state.font_pitch   = DEFAULT_PITCH;
    state.font_family  = FF_DONTCARE;
    switch ( pls->cfont )
    {
    case 1:
        // normal = (medium, upright, sans serif)
        state.font_family = FF_SWISS;
        break;
    case 2:
        // roman = (medium, upright, serif)
        state.font_family = FF_ROMAN;
        break;
    case 3:
        // italic = (medium, italic, serif)
        state.font_family = FF_ROMAN;
        state.italic      = 1;
        break;
    case 4:
        // script = (medium, upright, script)
        state.font_family = FF_SCRIPT;
        break;
    }

    // Calculate the font size from mm to device units
    state.font_height = 1.6 * pls->chrht * dev->ydpmm;

    plRotationShear( args->xform,
        &state.rotation, &state.shear, &state.stride );
    state.rotation -= pls->diorot * PI / 2.0;
    cos_rot         = (PLFLT) cos( state.rotation );
    sin_rot         = (PLFLT) sin( state.rotation );
    cos_shear       = (PLFLT) cos( state.shear );
    sin_shear       = (PLFLT) sin( state.shear );
    // Convert from radians to tenths of a degree, which is what
    // CreateFont() uses
    state.rotation = state.rotation * ( 180.0 / M_PI ) * 10.0;

    // Is the page rotated?
    if ( pls->diorot != 0 )
    {
        // The page is rotated.  We need to apply the page rotation because
        // args x,y are not rotated.
        PLFLT x, y, ox, oy;
        PLFLT cos_theta, sin_theta;

        // Translate origin of the virtual coordinates to the midpoint
        ox = (PLFLT) args->x - PIXELS_X / 2;
        oy = (PLFLT) args->y - PIXELS_Y / 2;

        // Apply the orientation rotation
        cos_theta = cos( -pls->diorot * PI / 2.0 );
        sin_theta = sin( -pls->diorot * PI / 2.0 );
        x         = cos_theta * (PLFLT) ox - sin_theta * (PLFLT) oy;
        y         = sin_theta * (PLFLT) ox + cos_theta * (PLFLT) oy;

        // Untranslate origin
        x = x + PIXELS_X / 2;
        y = y + PIXELS_Y / 2;

        // Convert to device coordinates
        rx = (int) ( x / dev->xscale );
        ry = (int) ( dev->height - ( y / dev->yscale ) );
    }
    else
    {
        rx = (int) ( args->x / dev->xscale );
        ry = (int) ( dev->height - ( args->y / dev->yscale ) );
    }

    // Determine the location of the bounding rectangle relative to the
    // reference point (bottom, top, or center)
    // Windows draws the text within a rectangle, thus the rectangle
    // needs to be positioned relative to specified coordinate
    if ( args->base == 2 )
    {
        //  If base = 2, it is aligned with the top of the text box
        valign = TA_TOP;
    }
    else if ( args->base == 1 )
    {
        //  If base = 1, it is aligned with the baseline of the text box
        valign = TA_BOTTOM;
    }
    else
    {
        //  If base = 0, it is aligned with the center of the text box
        valign = TA_TOP;

        // Adjust the reference point by 1/2 of the character height
        ry -= (int) ( 0.5 * pls->chrht * dev->ydpmm );
    }

    // Text justification.  Left (0.0), center (0.5) and right (1.0)
    // justification, which are the more common options, are supported.
    if ( args->just < 0.499 )
    {
        // Looks left aligned
        halign = TA_LEFT;
    }
    else if ( args->just > 0.501 )
    {
        // Looks right aligned
        halign = TA_RIGHT;
    }
    else
    {
        halign = TA_CENTER;
    }

    prev_font = set_font( dev,
        (LONG) state.font_height,
        (LONG) state.rotation,
        state.font_weight,
        state.italic,
        state.font_charset,
        state.font_pitch,
        state.font_family );
    prev_color  = SetTextColor( dev->hdc, dev->color );
    prev_bkmode = SetBkMode( dev->hdc, TRANSPARENT );

    if ( args->unicode_array_len == 0 )
    {
        // Non unicode string
        size_t i, j, len;
        char   * buffer;
        int    height = 0, width = 0;

        // Determine the string length and allocate a buffer to
        // use as a working copy of the output string.  The passed
        // string will always be larger than the formatted version
        len    = strlen( args->string );
        buffer = HeapAlloc( wingdi_heap, 0, ( len + 1 ) * sizeof ( char ) );
        if ( buffer == NULL )
        {
            plexit( "wingdi: Unable to allocate character buffer\n" );
        }

        // First we need to determine the number of text segments in the
        // string.  Text segments are delimited by the escape characters
        text_segments = 1;
        state.level   = 0;
        state.sscale  = 1.0;
        for ( i = j = 0; i < len + 1; i++ )
        {
            if ( args->string[i] != esc && args->string[i] != '\0' )
            {
                // Copy characters into the buffer to build the
                // string segment
                buffer[j++] = args->string[i];
            }
            else if ( i != len && args->string[i + 1] == esc )
            {
                // We have two escape characters in a row, which means
                // to ignore the escape and copy the character
                buffer[j++] = args->string[i];
                i++;
            }
            else
            {
                if ( j > 0 )
                {
                    SIZE segment_size;

                    // NUL terminate what we have copied so far
                    buffer[j] = '\0';

                    // Determine the size of the text segment and add
                    // it to the width of the overall string
                    GetTextExtentPoint32( dev->hdc,
                        buffer, j,
                        &segment_size );

                    // The effect of super/subscripts on the size of the
                    // bounding box is ignored at this time.  This will result
                    // in small positional errors.

                    width += segment_size.cx;
                    if ( segment_size.cy > height )
                        height = segment_size.cy;
                }
                j = 0;
                if ( i != len )
                {
                    text_segments++;
                    process_text_escape( dev, args,
                        &i, buffer, &j,
                        &state );
                }
            }
        }

        // Output the text segments
        if ( text_segments > 1 )
        {
            UINT save_text_align;

            // We have the total width of all the text segments.  Determine
            // the initial reference point based on the desired alignment.
            // When rendering text, we use left alignment and adjust the reference
            // point accordingly.
            switch ( halign )
            {
            case TA_LEFT:
                break;
            case TA_RIGHT:
                rx -= width;
                break;
            case TA_CENTER:
                rx -= (int) ( cos_rot * 0.5 * (PLFLT) width );
                ry += (int) ( sin_rot * 0.5 * (PLFLT) width );
                break;
            }
            save_text_align = SetTextAlign( dev->hdc, TA_LEFT | valign );

            state.level  = 0;
            state.sscale = 1.0;
            for ( i = j = 0; i < len + 1; i++ )
            {
                if ( args->string[i] != esc && args->string[i] != '\0' )
                {
                    // Copy characters into the buffer to build the
                    // string segment
                    buffer[j++] = args->string[i];
                }
                else if ( i != len && args->string[i + 1] == esc )
                {
                    // We have two escape characters in a row, which means
                    // to ignore the escape and copy the character
                    buffer[j++] = args->string[i];
                    i++;
                }
                else
                {
                    SIZE segment_size;
                    int  sx, sy;

                    if ( j > 0 )
                    {
                        // NUL terminate what we have copied so far
                        buffer[j] = '\0';

                        GetTextExtentPoint32( dev->hdc,
                            buffer, j,
                            &segment_size );

                        // Determine the offset due to super/subscripts
                        sx = (int) ( ( 1.0 - cos_rot ) * (PLFLT) state.soffset );
                        sy = (int) ( ( 1.0 - sin_rot ) * (PLFLT) state.soffset );

                        TextOut( dev->hdc,
                            rx + sx,
                            ry + sy,
                            buffer, j );

                        rx += (int) ( cos_rot * (PLFLT) segment_size.cx );
                        ry -= (int) ( sin_rot * (PLFLT) segment_size.cx );
                    }
                    j = 0;

                    if ( i != len )
                        process_text_escape( dev, args,
                            &i, buffer, &j,
                            &state );
                }
            }
        }
        HeapFree( wingdi_heap, 0, buffer );
    }
    else
    {
        // Unicode string
    }

    if ( text_segments == 1 )
    {
        // Only one text segment, thus this is a simple string
        // that can be written to the device in one step.  We
        // check for this because many strings are simple and
        // this approach is faster
        UINT save_text_align;

        save_text_align = SetTextAlign( dev->hdc, halign | valign );

        TextOut( dev->hdc,
            rx, ry,
            args->string, strlen( args->string ) );

        SetTextAlign( dev->hdc, save_text_align );
    }

    // Restore the previous text settings
    if ( prev_font != NULL )
        SelectObject( dev->hdc, prev_font );
    SetTextColor( dev->hdc, prev_color );
    SetBkMode( dev->hdc, prev_bkmode );
}

//--------------------------------------------------------------------------
//  End of the page
//--------------------------------------------------------------------------
static void
plD_eop_wingdi( PLStream *pls )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;

    pldebug( "wingdi", "End of the page\n" );

    if ( dev->type == DEV_WINDOW )
        // Save a bitmap of the current screen to help with redraws
        CopySCRtoBMP( pls );
    else
        EndPage( dev->hdc );

    // Set the cursor to normal to indicate that the window is not busy
    NormalCursor( dev );

    // Indicate that the driver is no longer drawing and is ready to continue
    dev->state = DEV_ACTIVE;
}

//--------------------------------------------------------------------------
//  Beginning of the new page
//--------------------------------------------------------------------------
static void
plD_bop_wingdi( PLStream *pls )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;
    pldebug( "wingdi", "Start of Page\n" );

    // Indicate that the device is actively drawing
    dev->state = DEV_DRAWING;

    // Update the status bar
    update_status_bar( dev );

    // Change the cursor to indicate the program is busy
    BusyCursor( dev );

    if ( dev->type == DEV_WINDOW )
    {
        // Invalidate the page so that it gets erased on the WM_PAINT.  It might be
        // better to just clear now and not invalidate.
        // NOTE:  Is RedrawWindow needed only for the window output device type?
        //RedrawWindow( dev->plot, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW );
        SendMessage( dev->plot,
            WM_ERASEBKGND,
            (WPARAM) 0,
            (LPARAM) NULL );
    }
    else
    {
        StartPage( dev->hdc );
    }

    // Reset the pen color
    plD_state_wingdi( pls, PLSTATE_COLOR0 );
}

//--------------------------------------------------------------------------
//  Stream cleanup function
//--------------------------------------------------------------------------
static void
plD_tidy_wingdi( PLStream *pls )
{
    struct wingdi_Dev *dev = NULL;

    pldebug( "wingdi", "plD_tidy_wingdi\n" );

    if ( pls->dev != NULL )
    {
        dev = (struct wingdi_Dev *) pls->dev;

        if ( dev->hdc != NULL )
            ReleaseDC( dev->plot, dev->hdc );

        if ( dev->bitmap != NULL )
            DeleteObject( dev->bitmap );

        if ( dev->plot != NULL )
            DestroyWindow( dev->plot );
        if ( dev->status_bar != NULL )
            DestroyWindow( dev->status_bar );
        if ( dev->frame != NULL )
            DestroyWindow( dev->frame );

        free_mem( pls->dev );

        wingdi_module_cleanup();
    }
}

//--------------------------------------------------------------------------
// plD_wait_wingdi()
//
// Wait for user input.
//--------------------------------------------------------------------------

static void
plD_wait_wingdi( PLStream *pls )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;

    // Wait for the user to indicate the next action
    wait_for_user_input( pls );
}

//--------------------------------------------------------------------------
// plD_state_wingdi()
//
// Handle change in PLStream state (color, pen width, fill attribute, etc).
//--------------------------------------------------------------------------

static void
plD_state_wingdi( PLStream *pls, PLINT op )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;

    switch ( op )
    {
    case PLSTATE_COLOR0:
    case PLSTATE_COLOR1:
        dev->color = RGB( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b );
        break;

    case PLSTATE_CMAP0:
    case PLSTATE_CMAP1:
        dev->color = RGB( pls->curcolor.r, pls->curcolor.g, pls->curcolor.b );
        break;
    }

    if ( dev->pen != NULL )
        DeleteObject( dev->pen );
    dev->pen = CreatePen( PS_SOLID, (int) pls->width, dev->color );
}

//--------------------------------------------------------------------------
// plD_esc_wingdi()
//
// Handle PLplot escapes
//--------------------------------------------------------------------------

#ifndef PLESC_TELLME
//
// Placeholder code from Greg Jung until it gets incorporated
// somewhere else
//
struct passmeup
{
    MSG  *msg;
    HWND *hwnd;
    HDC  *hdc;
    void *roomformore[6];
    char *flags;
};
#define PLESC_TELLME    41
#endif

static void
plD_esc_wingdi( PLStream *pls, PLINT op, void *ptr )
{
    struct wingdi_Dev *dev = (struct wingdi_Dev *) pls->dev;

    switch ( op )
    {
    case PLESC_GETC:
        GetCursorCmd( pls, (PLGraphicsIn *) ptr );
        break;

    case PLESC_FILL:
        plD_fill_polygon_wingdi( pls );
        break;

    case PLESC_CLEAR:
        plD_clear_wingdi( pls,
            pls->sppxmi, pls->sppymi,
            pls->sppxma, pls->sppyma );
        break;

    case PLESC_HAS_TEXT:
        plD_text_wingdi( pls, (EscText *) ptr );
        break;

    case PLESC_DOUBLEBUFFERING:
        // Ignore double buffering requests.  It causes problems with
        // strip charts and most machines are fast enough that it
        // does not make much of a difference.
        break;

    case PLESC_XORMOD:
        if ( *(PLINT *) ( ptr ) == 0 )
            SetROP2( dev->hdc, R2_COPYPEN );
        else
            SetROP2( dev->hdc, R2_XORPEN );
        break;

    case PLESC_START_RASTERIZE:
        //
//        pldebug( "wingdi", "Start Rasterize\n" );
//        // The image rasterization in PLplot uses fills.  It indicates
//        // the start and end with a PLESC_START_RASTERIZE and a
//        // PLESC_END_RASTERIZE escape code.  Thus, when these codes are
//        // received, the wingdi driver will switch from rendering to the
//        // display and render to a bitmap instead
//
//        // Save the original plot area DC
//        dev->save_hdc = dev->hdc;
//
//        // Create a new DC and a bitmap
//    dev->hdc = CreateCompatibleDC( dev->save_hdc );
//        GetClientRect( dev->plot, &dev->raster_rect );
//        dev->raster_bmp = CreateCompatibleBitmap( dev->hdc,
//                                                                                          dev->raster_rect.right,
//                                                                                      dev->raster_rect.bottom );
//    SelectObject( dev->hdc, dev->raster_bmp );
        //
        break;

    case PLESC_END_RASTERIZE:
        //
//        pldebug( "wingdi", "Stop Rasterize\n" );
//        // Display the bitmap
//        BitBlt( dev->save_hdc,
//                        dev->raster_rect.left, dev->raster_rect.top,
//                        dev->raster_rect.right, dev->raster_rect.bottom,
//                        dev->hdc,
//                        dev->raster_rect.left, dev->raster_rect.top,
//                        SRCCOPY );
//
//        // Cleanup
//        if ( dev->raster_bmp != NULL )
//                DeleteObject( dev->raster_bmp );
//        if ( dev->hdc != NULL )
//                DeleteDC( dev->hdc );
//
//        // Restore the original DC
//        dev->hdc = dev->save_hdc;
        //
        break;

    case PLESC_TELLME:
    {
        struct passmeup *pup = ptr;

        pup->hwnd = &( dev->plot );
        pup->hdc  = &( dev->hdc );
    }
    break;
    }
}

#else
int
pldummy_wingdi()
{
    return ( 0 );
}

#endif                          // PLD_wingdidev
