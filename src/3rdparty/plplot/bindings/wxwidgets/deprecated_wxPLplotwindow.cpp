// Copyright (C) 2005  Werner Smekal
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

#include <wx/window.h>
#include <wx/dcclient.h>

//#include "plplotP.h"
#include "deprecated_wxPLplotwindow.h"
#include "deprecated_wxPLplotstream.h"


BEGIN_EVENT_TABLE( wxPLplotwindow, wxWindow )
EVT_SIZE( wxPLplotwindow::OnSize )
EVT_PAINT( wxPLplotwindow::OnPaint )
EVT_ERASE_BACKGROUND( wxPLplotwindow::OnErase )
END_EVENT_TABLE()


//! Constructor allocates wxMemoryDC, a wxPLplotstream and initializes parameters.
//
wxPLplotwindow::wxPLplotwindow( wxWindow* parent, wxWindowID id, const wxPoint& pos,
                                const wxSize& size, long style, int pl_style ) :
    wxWindow( parent, id, pos, size, style | wxFULL_REPAINT_ON_RESIZE )
{
    // create MemoryDC and set size - if size not set (-1, -1) than
    // set size to (640,400)
    MemPlotDC = new wxMemoryDC;
    if ( size.GetWidth() < 0 || size.GetHeight() < 0 )
    {
        m_width  = 640;
        m_height = 400;
    }
    else
    {
        m_width  = size.GetWidth();
        m_height = size.GetHeight();
    }
    bitmapWidth  = m_width;
    bitmapHeight = m_height;

    MemPlotDCBitmap = new wxBitmap( bitmapWidth, bitmapHeight, -1 );
    MemPlotDC->SelectObject( *MemPlotDCBitmap );

    m_stream = new wxPLplotstream( (wxDC *) MemPlotDC, m_width, m_height, pl_style );

    m_stream->cmd( PLESC_GETBACKEND, &m_backend );
    m_backend = 1 << ( m_backend + 2 );

    // tell wxWidgets to leave the background painting to this control
    SetBackgroundStyle( wxBG_STYLE_CUSTOM );
}


//! Deconstructor takes care that all is deleted in the correct order.
//
wxPLplotwindow::~wxPLplotwindow( void )
{
    MemPlotDC->SelectObject( wxNullBitmap );

    if ( MemPlotDCBitmap )
        delete MemPlotDCBitmap;

    if ( m_stream )
        delete m_stream;

    if ( MemPlotDC )
        delete MemPlotDC;
}


//! In the OnPaint Method we check if the Windows was resized (will be moved to OnSize() sometimes
//  later), we also implement our own double buffering here (since the PLplot wxWidgets driver draws
//  into a wxMemoryDC)
//
void wxPLplotwindow::OnPaint( wxPaintEvent &WXUNUSED( event ) )
{
    wxPaintDC dc( this );
    dc.Blit( 0, 0, m_width, m_height, MemPlotDC, 0, 0 );
}


void wxPLplotwindow::OnSize( wxSizeEvent& WXUNUSED( event ) )
{
    int width, height;
    GetClientSize( &width, &height );

    // Check if we window was resized
    if ( ( m_width != width ) || ( m_height != height ) )
    {
        if ( ( width > bitmapWidth ) || ( height > bitmapHeight ) )
        {
            bitmapWidth  = bitmapWidth > width ? bitmapWidth : width;
            bitmapHeight = bitmapHeight > height ? bitmapHeight : height;

            MemPlotDC->SelectObject( wxNullBitmap );
            if ( MemPlotDCBitmap )
                delete MemPlotDCBitmap;
            MemPlotDCBitmap = new wxBitmap( bitmapWidth, bitmapHeight, -1 );
            MemPlotDC->SelectObject( *MemPlotDCBitmap );
        }

        m_stream->SetSize( width, height );
        m_stream->RenewPlot();

        m_width  = width;
        m_height = height;
    }
    else
    {
        m_stream->Update();
        Refresh( false );
    }
}


//! Together with "SetBackgroundStyle( wxBG_STYLE_CUSTOM );" in the constructor this method
//  is responsible that the background is not erased in order to prevent flickering.
//
void wxPLplotwindow::OnErase( wxEraseEvent &WXUNUSED( event ) )
{
}


//! Redo the whole plot.
//
void wxPLplotwindow::RenewPlot( void )
{
    if ( m_stream )
    {
        m_stream->RenewPlot();
        Refresh( false );
    }
}


//! Save plot.
//
bool wxPLplotwindow::SavePlot( const wxString& devname, const wxString& filename )
{
    int  pls, pls_save;
    FILE *sfile;

    if ( ( sfile = fopen( filename.mb_str(), "wb+" ) ) == NULL )
    {
        return false;
    }

    plgstrm( &pls );
    plmkstrm( &pls_save );
    if ( pls_save < 0 )
    {
        fclose( sfile );
        return false;
    }
    plsdev( devname.mb_str() );
    plsfile( sfile );

    plspage( 0., 0., 800, 600, 0, 0 );
    plcpstrm( pls, 0 );
    pladv( 0 );
    plreplot();
    plend1();
    plsstrm( pls );

    return true;
}
