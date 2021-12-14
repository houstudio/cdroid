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

// wxwidgets headers
#include "wx/wx.h"

// plplot headers
#include "plplotP.h"

#include "deprecated_wxPLplotstream.h"

//! Constructor of wxPLplotstream class which is inherited from plstream.
//  Here we set the driver (wxwidgets :), and tell plplot in which dc to
//  plot to and the size of the canvas. We also check and set several
//  device style options.
//
wxPLplotstream::wxPLplotstream( wxDC *dc, int width, int height, int style ) : plstream()
{
    Create( dc, width, height, style );
}


wxPLplotstream::wxPLplotstream() : plstream()
{
}


void wxPLplotstream::Create( wxDC *dc, int width, int height, int style )
{
    const size_t bufferSize = 256;

    m_dc     = dc;
    m_width  = width;
    m_height = height;
    m_style  = style;
    m_image  = NULL;

    sdev( "wxwidgets" );
    spage( 0.0, 0.0, m_width, m_height, 0, 0 );

    // use freetype, antialized canvas?
    char drvopt[bufferSize], buffer[bufferSize];
    drvopt[0] = '\0';
#ifdef WX_TEMP_PL_HAVE_FREETYPE_IS_ON
    sprintf( buffer, "freetype=%d,smooth=%d,",
        m_style & wxPLPLOT_FREETYPE ? 1 : 0,
        m_style & wxPLPLOT_SMOOTH_TEXT ? 1 : 0 );
    strcat( drvopt, buffer );
#endif

    int backend;
    if ( m_style & wxPLPLOT_BACKEND_GC )
        backend = 2;
    else if ( m_style & wxPLPLOT_BACKEND_AGG )
        backend = 1;
    else
        backend = 0;

    sprintf( buffer, "hrshsym=%d,text=%d,backend=%d",
        m_style & wxPLPLOT_USE_HERSHEY_SYMBOLS ? 1 : 0,
        m_style & wxPLPLOT_DRAW_TEXT ? 1 : 0,
        backend );
    strncat( drvopt, buffer, bufferSize - strlen( drvopt ) );

    setopt( "-drvopt", drvopt );

    init();

    cmd( PLESC_GETBACKEND, &m_backend );
    m_backend = 1 << ( m_backend + 2 );

    if ( m_backend == wxPLPLOT_BACKEND_AGG )
    {
        m_image = new wxImage( m_width, m_height );
        cmd( PLESC_DEVINIT, (void *) m_image );
    }
    else
        cmd( PLESC_DEVINIT, (void *) m_dc );
}


wxPLplotstream::~wxPLplotstream()
{
    if ( m_image )
        delete m_image;
}


//! This is the overloaded set_stream() function, where we could have some
//  code processed before every call of a plplot functions, since set_stream()
//  is called before every plplot function. Not used in the moment.
//
void wxPLplotstream::set_stream()
{
    plstream::set_stream();
}


//! Call this method if the size of the dc changed (because of resizing)
//  to set the new size. You need to call RenewPlot afterwards.
//
void wxPLplotstream::SetSize( int width, int height )
{
    // For the AGG backend it is important to set first the new image buffer
    //       and tell the driver the new size if the buffer size increases and
    //       the other way round if the buffer size decreases. There is no impact
    //       for the other backends. This is kind of hacky, but I have no better
    //       idea in the moment
    if ( width * height > m_width * m_height )
    {
        if ( m_image )
        {
            delete m_image;
            m_image = new wxImage( width, height );
            cmd( PLESC_DEVINIT, (void *) m_image );
        }
        wxSize size( width, height );
        cmd( PLESC_RESIZE, (void *) &size );
    }
    else
    {
        wxSize size( width, height );
        cmd( PLESC_RESIZE, (void *) &size );
        if ( m_image )
        {
            delete m_image;
            m_image = new wxImage( width, height );
            cmd( PLESC_DEVINIT, (void *) m_image );
        }
    }

    m_width  = width;
    m_height = height;
}


//! Replot everything.
//
void wxPLplotstream::RenewPlot()
{
    replot();
    Update();
}


// After calling plot commands it is not sure, that the dc
// gets updated properly, therefore you need to call this function.
//
void wxPLplotstream::Update()
{
    if ( m_image )
    {
        wxMemoryDC MemoryDC;
        wxBitmap   bitmap( *m_image, -1 );
        MemoryDC.SelectObject( bitmap );
        m_dc->Blit( 0, 0, m_width, m_height, &MemoryDC, 0, 0 );
        MemoryDC.SelectObject( wxNullBitmap );
    }
}
