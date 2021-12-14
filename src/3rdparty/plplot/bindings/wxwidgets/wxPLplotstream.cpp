// Copyright (C) 2015  Phil Rosenberg
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

#include "wxPLplotstream.h"

//! Constructor of wxPLplotstream class which is inherited from plstream.
//  Here we set the driver (wxwidgets :), and tell plplot in which dc to
//  plot to and the size of the canvas. We also check and set several
//  device style options.
//
wxPLplotstream::wxPLplotstream( wxDC *dc, int width, int height, int style ) : plstream()
{
    m_created = false;
    Create( dc, width, height, style );
}


wxPLplotstream::wxPLplotstream() : plstream()
{
    m_created = false;
}

//! Called from the constructor or can be called by the user if the default constructor is used
//  We set the driver to be wxwidgets, set the page size, set the driver options and initialize
//  the plot.
void wxPLplotstream::Create( wxDC *dc, int width, int height, int style )
{
    if ( m_created )
    {
        plabort( "wxPLplotstream::Create - Stream already created" );
        return;
    }
    const size_t bufferSize = 256;

    m_width  = width;
    m_height = height;
    m_style  = style;

    sdev( "wxwidgets" );
    spage( 90.0, 90.0, m_width, m_height, 0, 0 );

    char drvopt[bufferSize], buffer[bufferSize];
    drvopt[0] = '\0';

    sprintf( buffer, "hrshsym=%d,text=%d",
        m_style & wxPLPLOT_USE_HERSHEY_SYMBOLS ? 1 : 0,
        m_style & wxPLPLOT_DRAW_TEXT ? 1 : 0 );
    strncat( drvopt, buffer, bufferSize - strlen( drvopt ) );

    setopt( "-drvopt", drvopt );

    sdevdata( (void *) dc );

    init();
}

//! Set the DC to be used by the stream. This will initiate a replot, unless
//  the device is NULL
void wxPLplotstream::SetDC( wxDC *dc )
{
    set_stream();
    cmd( PLESC_DEVINIT, (void *) dc );
}

//! Destructor, although we have no resources to free
wxPLplotstream::~wxPLplotstream()
{
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
    wxSize size( width, height );
    cmd( PLESC_RESIZE, (void *) &size );
    m_width  = width;
    m_height = height;
}


//! Replot everything.
//
void wxPLplotstream::RenewPlot()
{
    replot();
}

void wxPLplotstream::ImportBuffer( void *buffer, size_t size )
{
    plbuffer buf;
    buf.buffer = buffer;
    buf.size   = size;
    cmd( PLESC_IMPORT_BUFFER, &buf );
    RenewPlot();
}

void wxPLplotstream::AppendBuffer( void *buffer, size_t size )
{
    plbuffer buf;
    buf.buffer = buffer;
    buf.size   = size;
    cmd( PLESC_APPEND_BUFFER, &buf );
    cmd( PLESC_FLUSH_REMAINING_BUFFER, NULL );
}

void wxPLplotstream::SetFixedAspectRatio( bool fixed )
{
    PLBOOL f = fixed ? 1 : 0;
    cmd( PLESC_FIXASPECT, &f );
}

bool wxPLplotstream::IsValid()
{
    return m_created;
}