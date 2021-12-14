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

#if !defined ( WXPLPLOTSTREAM_H__INCLUDED_ )
#define WXPLPLOTSTREAM_H__INCLUDED_

#include "plstream.h"
#include "wxPLplot_nanosec.h"

// forward declarations
class wxImage;
class wxDC;

//! Style options for wxPLplotstream:
//  wxPLPLOT_NONE: no option
//  wxPLPLOT_DRAWTEXT: use wxWidgets routines to draw text
//  wxPLPLOT_USE_HERSHEY_SYMBOLS: use hershey symbols to draw symbols instead of font symbols
//
enum
{
    wxPLPLOT_NONE      = 0,
    wxPLPLOT_DRAW_TEXT = 1 << 5,
        wxPLPLOT_USE_HERSHEY_SYMBOLS = 1 << 6
};

//! wxPLplotstream is inherited from plstream, which is the C++ interface
//  to the plplot API. The documentation of this interface is described in
//  the PLplot manual, not here.
//
class PLDLLIMPEXP_WX wxPLplotstream : public plstream
{
public:
    wxPLplotstream();                                                              //!< Constructor.
    wxPLplotstream( wxDC * dc, int width, int height, int style = wxPLPLOT_NONE ); //!< Constructor.
    void Create( wxDC *dc, int width, int height, int style );
    ~wxPLplotstream();
    //wxPLplotstream( wxImage *buffer, int width, int height, int style );
    void set_stream();                              //!< Calls some code before every PLplot command.
    void SetSize( int width, int height );          //!< Set new size of plot area.
    void RenewPlot();                               //!< Redo plot and update dc
    void SetDC( wxDC *dc );                         //!< Set a new dc to write to
    void ImportBuffer( void *buffer, size_t size ); //!< Import a buffer of plplot commands
    void AppendBuffer( void *buffer, size_t size ); //!< Append a buffer of plplot commands
    void SetFixedAspectRatio( bool fixed );         //!< Set Fixed aspect ratio of the plot
    bool IsValid();

private:
    void InitStream();

private:
    int  m_width;       //!< Width of dc/plot area.
    int  m_height;      //!< Height of dc/plot area.
    int  m_style;       //!< style of this plot
    bool m_created;
};


#endif // !defined( WXPLPLOTSTREAM_H__INCLUDED_ )
