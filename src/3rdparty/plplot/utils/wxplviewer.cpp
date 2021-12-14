// Copyright (C) 2015  Phil Rosenberg
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

#include "wxplframe.h"
#include <wx/wx.h>
#include <vector>
// Needed for cerr and endl below
#include <iostream>

class wxPlViewerApp : public wxApp
{
public:
    virtual bool OnInit();
};

IMPLEMENT_APP( wxPlViewerApp );

bool wxPlViewerApp::OnInit()
{
    try
    {
        wxInitAllImageHandlers();
        wxString file;
        long     fileSize = 0;
        long     width    = wxDefaultCoord;
        long     height   = wxDefaultCoord;
        if ( argc >= 3 )
        {
            file = argv[1];
            wxString( argv[2] ).ToLong( &fileSize );
        }
        if ( argc == 5 )
        {
            wxString( argv[3] ).ToLong( &width );
            wxString( argv[4] ).ToLong( &height );
        }
#ifdef __WXGTK__
        //This is to deal with the bug described at
        //https://forums.wxwidgets.org/viewtopic.php?t=37151&p=151356
        //Although this bug has been fixed according to
        //http://trac.wxwidgets.org/ticket/16202 and
        //http://trac.wxwidgets.org/changeset/76398/svn-wx
        //I currently don't know which wxWidgets version it is
        //included in so I am applying this to all wxGTK
        //builds
        wxMilliSleep( 100 );
#endif
        wxPlFrame* frame = new wxPlFrame( NULL, wxID_ANY, wxT( "wxPlViewer" ), file,
            PLINT( fileSize ), wxDefaultPosition, wxSize( width, height ) );
        frame->fixAspect( false );
        //frame->SetClientSize( width, height );
        frame->fixAspect( true );
        frame->Show();
    }
    catch ( const char* message )
    {
        std::cerr << "wxPlViewerApp::OnInit: error when creating wxPlFrame instance.  " << "The message was " << message << std::endl;
        return false;
    }
    catch ( ... )
    {
        std::cerr << "wxPlViewerApp::OnInit: unknown error when creating wxPlFrame instance." << std::endl;
        return false;
    }
    return true;
}
