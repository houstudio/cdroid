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
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#ifdef __WXMAC__
        #include <Carbon/Carbon.h>
extern "C" { void CPSEnableForegroundOperation( ProcessSerialNumber* psn ); }
#endif

#include "deprecated_wxPLplotwindow.h"
#include <cmath>

#define MAX( a, b )    ( ( a ) < ( b ) ? ( b ) : ( a ) )
#define MIN( a, b )    ( ( a ) < ( b ) ? ( a ) : ( b ) )

// Application icon as XPM
// This free icon was taken from http://2pt3.com/news/twotone-icons-for-free/
static const char *graph[] = {
// columns rows colors chars-per-pixel
    "16 16 4 2",
    "   c black",
    ".  c #BA1825",
    "X  c gray100",
    "UX c None",
// pixels
    "UX. . . . . . . . . . . . . . UX",
    ". . . . . . . . . . . . . . . . ",
    ". . . . . . . . . . . . . . . . ",
    ". . . . . . . . . . . X X . . . ",
    ". . . . . . . . . . . X X . . . ",
    ". . . . . . . . . . . X X . . . ",
    ". . . . . X X . . . . X X . . . ",
    ". . . . . X X . . . . X X . . . ",
    ". . . . . X X . X X . X X . . . ",
    ". . . . . X X . X X . X X . . . ",
    ". . . . . X X . X X . X X . . . ",
    ". . . . . X X . X X . X X . . . ",
    ". . . X X X X X X X X X X . . . ",
    ". . . . . . . . . . . . . . . . ",
    ". . . . . . . . . . . . . . . . ",
    "UX. . . . . . . . . . . . . . UX"
};

//--------------------------------------------------------------------------
// private classes
//--------------------------------------------------------------------------
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};


class MyPlotwindow : public wxPLplotwindow
{
public:
    MyPlotwindow( wxFrame* frame, wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = 0,
                  int pl_style = wxPLPLOT_NONE );

    void OnChar( wxKeyEvent& event );

private:
    wxFrame* mframe;
};


// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    MyFrame( const wxString& title );
    void Plot();

private:
    void OnQuit( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );
    void OnBackgroundColor( wxCommandEvent& event );

private:
    MyPlotwindow* plotwindow;
    bool        bgcolor;
    int         m_backend;

    DECLARE_EVENT_TABLE()
};


//--------------------------------------------------------------------------
// constants
//--------------------------------------------------------------------------
enum { wxPLplotDemo_Quit    = wxID_EXIT, wxPLplotDemo_About = wxID_ABOUT,
       wxPLplotDemo_BGColor = 10000 };

//--------------------------------------------------------------------------
// event tables and other macros for wxWidgets
//--------------------------------------------------------------------------
BEGIN_EVENT_TABLE( MyFrame, wxFrame )
EVT_MENU( wxPLplotDemo_Quit, MyFrame::OnQuit )
EVT_MENU( wxPLplotDemo_About, MyFrame::OnAbout )
EVT_MENU( wxPLplotDemo_BGColor, MyFrame::OnBackgroundColor )
END_EVENT_TABLE()

IMPLEMENT_APP( MyApp )

//--------------------------------------------------------------------------
// implementation
//--------------------------------------------------------------------------

//! This method is called right at the beginning and opens a frame for us.
//
bool MyApp::OnInit()
{
#ifdef __WXMAC__
    // this hack enables to have a GUI on Mac OSX even if the
    // program was called from the command line (and isn't a bundle)
    ProcessSerialNumber psn;

    GetCurrentProcess( &psn );
    CPSEnableForegroundOperation( &psn );
    SetFrontProcess( &psn );
#endif

    MyFrame *frame = new MyFrame( _T( "wxPLplot demo" ) );
    frame->Show( true );

    return true;
}


MyPlotwindow::MyPlotwindow( wxFrame* frame, wxWindow* parent, wxWindowID id, const wxPoint& pos,
                            const wxSize& size, long style, int pl_style ) :
    wxPLplotwindow( parent, id, pos, size, style, pl_style )
{
    mframe = frame;
}


void MyPlotwindow::OnChar( wxKeyEvent& event )
{
    int keycode = event.GetKeyCode();

    if ( keycode == WXK_RETURN ||
         keycode == WXK_SPACE ||
         keycode == WXK_RIGHT ||
         keycode == WXK_ESCAPE )
        mframe->Close( true );
    else
        event.Skip();
}


//! Constructor of our custom frame, where the Menu is created and a
//  a wxPLplotwindow is inserted into the frame. We plot some easy functions
//  just to show how it works. wxPLplotwindow takes care of all the setup
//  for the use of PLplot library.
//
MyFrame::MyFrame( const wxString& title ) : wxFrame( NULL, wxID_ANY, title )
{
    bgcolor = false;

    // add menu
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append( wxPLplotDemo_BGColor, _T( "&Change background color...\tAlt-C" ), _T( "Change background color" ) );
    fileMenu->Append( wxPLplotDemo_About, _T( "&About...\tF1" ), _T( "Show about dialog" ) );
    fileMenu->Append( wxPLplotDemo_Quit, _T( "E&xit\tAlt-X" ), _T( "Quit this program" ) );

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append( fileMenu, _T( "&File" ) );
    SetMenuBar( menuBar );
    SetIcon( wxIcon( graph ) );

    // add the wxPLplot
    wxPanel   * panel = new wxPanel( this );
    wxBoxSizer* box   = new wxBoxSizer( wxVERTICAL );
    plotwindow = new MyPlotwindow( this, panel, -1, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS,
#if wxUSE_GRAPHICS_CONTEXT
        wxPLPLOT_BACKEND_GC | wxPLPLOT_DRAW_TEXT );
#else
        wxPLPLOT_BACKEND_AGG | wxPLPLOT_DRAW_TEXT );
#endif
    plotwindow->Connect( wxEVT_CHAR, wxKeyEventHandler( MyPlotwindow::OnChar ) );
    box->Add( plotwindow, 1, wxALL | wxEXPAND, 0 );
    panel->SetSizer( box );
    SetSize( 640, 500 );      // set frame size
    SetSizeHints( 220, 150 ); // set minimum frame size

    wxString m_title = title;
    switch ( plotwindow->getBackend() )
    {
    case wxPLPLOT_BACKEND_DC:
        m_title += wxT( " (basic)" );
        break;
    case wxPLPLOT_BACKEND_GC:
        m_title += wxT( " (wxGC)" );
        break;
    case wxPLPLOT_BACKEND_AGG:
        m_title += wxT( " (AGG)" );
        break;
    default:
        break;
    }
    SetTitle( m_title );

    Plot();
}

void MyFrame::Plot()
{
    wxPLplotstream* pls = plotwindow->GetStream();

    const size_t  np = 500;
    PLFLT         x[np], y[np];
    PLFLT         xmin, xmax;
    PLFLT         ymin = 1e30, ymax = 1e-30;

    xmin = -2.0;
    xmax = 10.0;
    for ( size_t i = 0; i < np; i++ )
    {
        x[i] = ( xmax - xmin ) * i / np + xmin;
        y[i] = 1.0;
        if ( x[i] != 0.0 )
            y[i] = sin( x[i] ) / x[i];
        ymin = MIN( ymin, y[i] );
        ymax = MAX( ymax, y[i] );
    }

    pls->adv( 0 );
    if ( bgcolor )
    {
        pls->scol0( 0, 255, 255, 255 );
        pls->scol0( 15, 0, 0, 0 );
    }
    else
    {
        pls->scol0( 15, 255, 255, 255 );
        pls->scol0( 0, 0, 0, 0 );
    }
    pls->col0( 1 );
    pls->env( xmin, xmax, ymin, ymax, 0, 0 );
    pls->col0( 2 );
    pls->lab( "x", "y", "sin(x)/x" );

    pls->col0( 3 );
    pls->width( 2 );
    pls->line( np, x, y );

    plotwindow->RenewPlot();
}


void MyFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
    Close( true );
}


void MyFrame::OnBackgroundColor( wxCommandEvent& WXUNUSED( event ) )
{
    bgcolor = !bgcolor;
    Plot();
}


//! Show information if Menu entry About was choosen.
//
void MyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
    wxMessageBox( _T( "This is the About dialog of the wxPLplot demo.\n" ), _T( "About wxPLplot" ),
        wxOK | wxICON_INFORMATION, this );
}
