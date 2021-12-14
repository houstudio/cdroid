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

#if !defined ( WXPLPLOTWINDOW_H__INCLUDED_ )
#define WXPLPLOTWINDOW_H__INCLUDED_

#include "plplot.h"
#include "wxPLplotstream.h"
#include <wx/window.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/dcgraph.h>
#include <wx/dcbuffer.h>

// A plplot wxWindow template. To create an actual plplot wxWindow use
// the type of wxWindow you wish to inherit from at the template parameter
// For example to create a plplot wxFrame create a wxPLplotwindow<wxFrame>
// then call the base wxWindow's Create method to initialise.
template <class WXWINDOW>
class wxPLplotwindow : public WXWINDOW
{
public:
    wxPLplotwindow( bool useGraphicsContext = true, wxSize clientSize = wxDefaultSize,
                    int resizeRenderDelay_ms = 0 );                       //!< Constructor.
    virtual ~wxPLplotwindow( void );                                      //!< Destructor.

    void RenewPlot( void );                                               //!< Redo plot.
    bool SavePlot( const wxString& driver, const wxString& filename );    //!< Save plot using a different driver.
    wxPLplotstream* GetStream()  { return m_created ? &m_stream : NULL; } //!< Get pointer to wxPLplotstream of this widget.
    void setUseGraphicsContext( bool useGraphicsContext );
    void setCanvasColour( const wxColour &colour );
    bool IsReady() { return GetStream() != NULL; }

protected:
    virtual void OnPaint( wxPaintEvent& event );         //!< Paint event
    virtual void OnSize( wxSizeEvent & event );          //!< Size event
    virtual void OnErase( wxEraseEvent &event );         //!< Background erase event
    virtual void OnCreate( wxWindowCreateEvent &event ); //!< Window created event
    void OnRenderTimer( wxTimerEvent &event );           //!< Timer used in delayed rendering after resize
    void OnMouse( wxMouseEvent &event );                 //!< Mouse events
    wxPLplotstream m_stream;                             //!< The wxPLplotstream which belongs to this plot widget
    bool           m_created;                            //!< Flag to indicate the window has been Created

private:
    bool     m_useGraphicsContext;                       //!< Flag to indicate whether we should use a wxGCDC
    wxBitmap m_bitmap;
    // The memory dc and wrapping gc dc for drawing. Note we
    //use pointers and reallocate them whenever the bitmap is
    //resized because reusing either of these causes problems
    //for rendering on a wxGCDC - at least on Windows.
    wxMemoryDC *m_memoryDc;
    wxSize     m_initialSize;
#ifdef wxUSE_GRAPHICS_CONTEXT
    wxGCDC     *m_gcDc;
#endif
    wxColour   m_canvasColour;
    virtual void OnLocate( const PLGraphicsIn &graphicsIn ){}

    //these are to allow delayed repainting on resize. This causes
    //jerky resizing for large plots
    bool             m_resizing;
    bool             m_completedFirstRender;
    size_t           m_resizeRenderDelay;
    wxTimer          m_renderTimer;
    static const int ID_RENDERTIMER;
};

template<class WXWINDOW>
const int wxPLplotwindow<WXWINDOW>::ID_RENDERTIMER = ::wxNewId();

//! Constructor initialises variables, creates the wxStream and
//! connects methods with events. The WXWINDOW default constructor is
//! used.
//!

template<class WXWINDOW>
wxPLplotwindow<WXWINDOW>::wxPLplotwindow( bool useGraphicsContext, wxSize clientSize, int resizeRenderDelay_ms )
    : m_created( false ), m_initialSize( clientSize ), m_resizing( false ), m_completedFirstRender( false ), m_renderTimer( this, ID_RENDERTIMER ), m_resizeRenderDelay( resizeRenderDelay_ms )

{
    PLPLOT_wxLogDebug( "wxPLplotwindow::wxPLplotwindow" );
    m_memoryDc = NULL;
#ifdef wxUSE_GRAPHICS_CONTEXT
    m_gcDc = NULL;
    // Force initialization of m_useGraphicsContext and
    // drawDc when setUseGraphicsContext is called below.
    m_useGraphicsContext = !useGraphicsContext;
#endif
    setUseGraphicsContext( useGraphicsContext );
    m_canvasColour = *wxBLACK;

    //We use connect instead of Bind for compatibility with wxWidgets 2.8
    //but should move to bind in the future.
    WXWINDOW::Connect( wxEVT_SIZE, wxSizeEventHandler( wxPLplotwindow<WXWINDOW>::OnSize ) );
    WXWINDOW::Connect( wxEVT_PAINT, wxPaintEventHandler( wxPLplotwindow<WXWINDOW>::OnPaint ) );
    WXWINDOW::Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( wxPLplotwindow<WXWINDOW>::OnErase ) );
    WXWINDOW::Connect( wxEVT_CREATE, wxWindowCreateEventHandler( wxPLplotwindow<WXWINDOW>::OnCreate ) );
    WXWINDOW::Connect( wxEVT_MOTION, wxMouseEventHandler( wxPLplotwindow<WXWINDOW>::OnMouse ) );
    WXWINDOW::Connect( wxEVT_LEFT_UP, wxMouseEventHandler( wxPLplotwindow<WXWINDOW>::OnMouse ) );
    WXWINDOW::Connect( wxEVT_MIDDLE_UP, wxMouseEventHandler( wxPLplotwindow<WXWINDOW>::OnMouse ) );
    WXWINDOW::Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( wxPLplotwindow<WXWINDOW>::OnMouse ) );
    WXWINDOW::Connect( ID_RENDERTIMER, wxEVT_TIMER, wxTimerEventHandler( wxPLplotwindow<WXWINDOW>::OnRenderTimer ) );
}


//! Destructor - delete the dc and gcdc if needed.
//!

template<class WXWINDOW>
wxPLplotwindow<WXWINDOW>::~wxPLplotwindow( void )
{
    if ( m_memoryDc )
        delete m_memoryDc;
    if ( m_gcDc )
        delete m_gcDc;
}

//! In the OnPaint Method we check if the Windows was resized (will be
//! moved to OnSize() sometimes later), we also implement our own
//! double buffering here (since the PLplot wxWidgets driver draws
//! into a wxMemoryDC).
//!

template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::OnPaint( wxPaintEvent &WXUNUSED( event ) )
{
    //Really this should be in the constructor, but it caused a segfault
    //on at least one system (CentOS with intel compiler and wxWidgets 2.8.12).
    //Moving it here after WXWINDOW::Create has been called stops this and
    //the call does nothing if the style is the same as previous calls so
    //should be safe to call here.
    //WXWINDOW::SetBackgroundStyle( wxBG_STYLE_CUSTOM );

    //wxAutoBufferedPaintDC dc( (WXWINDOW*)this );
    int       width  = WXWINDOW::GetClientSize().GetWidth();
    int       height = WXWINDOW::GetClientSize().GetHeight();

    wxPaintDC paintDc( this );

    //if we are still resizing (i.e. the useris still messing
    //with the size) then just fill the invalidated area with
    //the background colour for now. A full rerender will
    //happen once the size has stopped changing.
    if ( m_resizing )
    {
        //fill the area with the background colour
        paintDc.SetBackground( wxBrush( m_canvasColour ) );
        paintDc.Clear();
        //also blit on the previous image - it may have
        //been invalidated, e.g. by shrinking then expanding
        //the window.
        paintDc.Blit( 0, 0, width, height, m_memoryDc, 0, 0 );

        return;
    }

    //resize the plot if needed
    bool needResize = width != m_bitmap.GetWidth() || height != m_bitmap.GetHeight();
    if ( needResize )
    {
        m_bitmap.Create( width, height, 32 );
        if ( m_memoryDc )
            delete m_memoryDc;
        m_memoryDc = new wxMemoryDC;
        m_memoryDc->SelectObject( m_bitmap );
        wxDC *drawDc = m_memoryDc;
#ifdef wxUSE_GRAPHICS_CONTEXT
        if ( m_useGraphicsContext )
        {
            if ( m_gcDc )
                delete m_gcDc;
            m_gcDc = new wxGCDC( *m_memoryDc );
            drawDc = m_gcDc;
        }
#endif
        if ( IsReady() )
            m_stream.SetDC( drawDc );
        drawDc->SetBackground( wxBrush( m_canvasColour ) );
        drawDc->Clear();
        if ( IsReady() )
            m_stream.SetSize( width, height );
    }

    paintDc.Blit( 0, 0, width, height, m_memoryDc, 0, 0 );

    if ( width > 0 && height > 0 )
        m_completedFirstRender = true;
}

//! This is called when the plot is resized
//!

template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::OnSize( wxSizeEvent& event )
{
    //Once the window has been displayed properly for the first time we
    //want to delay rendering on resize, this ensures that complex plots
    //don't cause unresponsive or even nearly impossible resizing.

    //Don't delay the first render or if the size hasn't actually
    //changed (this can be caused by a resize call from PlPlot but
    //with the size the same).
    int width     = WXWINDOW::GetClientSize().GetWidth();
    int height    = WXWINDOW::GetClientSize().GetHeight();
    int oldWidth  = m_bitmap.IsOk() ? m_bitmap.GetWidth() : 0;
    int oldHeight = m_bitmap.IsOk() ? m_bitmap.GetHeight() : 0;
    if ( !m_completedFirstRender || ( m_resizeRenderDelay == 0 ) ||
         ( width == oldWidth && height == oldHeight ) )
    {
        //++m_nResizes;
        //Invalidate the whole window so it is all redrawn, otherwise only
        //newly exposed parts of the window get redrawn
        RenewPlot();
        return;
    }

    //If we wish to delay rerendering while the size settles down then
    //invalidate the newly exposed area and flag that we are still resizing
    //(this tells OnPaint to just fill the new area with the background
    //colour), then set up a timer to trigger a proper rerender once the
    //size has stopped changing.
    m_resizing = true;
    if ( width > oldWidth )
        WXWINDOW::RefreshRect( wxRect( width, 0, width - oldWidth, height ) );
    if ( height > oldHeight )
        WXWINDOW::RefreshRect( wxRect( 0, oldHeight, oldWidth, height - oldHeight ) );

    //Start the timer, note that this resets the timer to 0 if it is still running
    m_renderTimer.Start( m_resizeRenderDelay, true );
}

//! This is called before each paint event
//!

template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::OnErase( wxEraseEvent& WXUNUSED( event ) )
{
    //Do nothing. This stops screen flicker.
}

//! This is called when the window is created, i.e., after
//! WXWINDOW::Create has been called. We note that this has been
//! called to avoid attempting to redraw a plot on a window that
//! hasn't been created yet.
//!

template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::OnCreate( wxWindowCreateEvent &event )
{
    PLPLOT_wxLogDebug( "wxPLplotwindow::OnCreate" );
    if ( !m_created )
    {
        //set the client size if requested
        if ( m_initialSize != wxDefaultSize )
            WXWINDOW::SetClientSize( m_initialSize );
        //create the stream
        int width  = WXWINDOW::GetClientSize().GetWidth();
        int height = WXWINDOW::GetClientSize().GetHeight();
        m_bitmap.Create( width, height );
        if ( m_memoryDc )
            delete m_memoryDc;
        m_memoryDc = new wxMemoryDC;
        m_memoryDc->SelectObject( m_bitmap );
        wxDC * drawDc = m_memoryDc;
#ifdef wxUSE_GRAPHICS_CONTEXT
        if ( m_useGraphicsContext )
        {
            if ( m_gcDc )
                delete m_gcDc;
            m_gcDc = new wxGCDC( *m_memoryDc );
            drawDc = m_gcDc;
        }
#endif
        if ( !m_stream.IsValid() )
            m_stream.Create( drawDc, width, height, wxPLPLOT_DRAW_TEXT );
        else
            m_stream.SetDC( drawDc );
        drawDc->SetBackground( wxBrush( m_canvasColour ) );
        drawDc->Clear();

        m_created = true;
        RenewPlot();
    }
}

template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::OnRenderTimer( wxTimerEvent &event )
{
    if ( m_resizing )
    {
        m_resizing = false;
        RenewPlot();
    }
}

//Capture Mouse events and pass the
template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::OnMouse( wxMouseEvent &event )
{
    PLGraphicsIn graphicsIn;
    wxPoint      cursorPosition = event.GetPosition();
    wxSize       clientSize     = WXWINDOW::GetClientSize();

    graphicsIn.pX        = cursorPosition.x;
    graphicsIn.pY        = cursorPosition.y;
    graphicsIn.dX        = PLFLT( cursorPosition.x + 0.5 ) / PLFLT( clientSize.GetWidth() );
    graphicsIn.dY        = 1.0 - PLFLT( cursorPosition.y + 0.5 ) / PLFLT( clientSize.GetHeight() );
    graphicsIn.keysym    = 0x20;
    graphicsIn.state     = 0;
    graphicsIn.subwindow = -1;
    graphicsIn.type      = 0;
    graphicsIn.string[0] = '\0';
    if ( event.LeftUp() )
    {
        graphicsIn.button = 1;
        graphicsIn.state |= PL_MASK_BUTTON1;
    }
    else if ( event.MiddleUp() )
    {
        graphicsIn.button = 2;
        graphicsIn.state |= PL_MASK_BUTTON2;
    }
    else if ( event.RightUp() )
    {
        graphicsIn.button = 3;
        graphicsIn.state |= PL_MASK_BUTTON3;
    }
    else if ( event.Aux1Up() )
    {
        graphicsIn.button = 4;
        graphicsIn.state |= PL_MASK_BUTTON4;
    }
    else if ( event.Aux2Up() )
    {
        graphicsIn.button = 5;
        graphicsIn.state |= PL_MASK_BUTTON5;
    }
    else
    {
        //If we get here we have just captured motion
        //not a click
        graphicsIn.button = 0;
        graphicsIn.state  = 0;
        graphicsIn.keysym = 0;
    }

    if ( wxGetKeyState( WXK_SHIFT ) )
        graphicsIn.state |= PL_MASK_SHIFT;
    if ( wxGetKeyState( WXK_CAPITAL ) )
        graphicsIn.state |= PL_MASK_CAPS;
    if ( wxGetKeyState( WXK_ALT ) && wxGetKeyState( WXK_CONTROL ) )
        graphicsIn.state |= PL_MASK_ALTGR;
    else if ( wxGetKeyState( WXK_CONTROL ) )
        graphicsIn.state |= PL_MASK_CONTROL;
    else if ( wxGetKeyState( WXK_ALT ) )
        graphicsIn.state |= PL_MASK_ALT;
    if ( wxGetKeyState( WXK_NUMLOCK ) )
        graphicsIn.state |= PL_MASK_NUM;
    if ( wxGetKeyState( WXK_SCROLL ) )
        graphicsIn.state |= PL_MASK_SCROLL;
    //Note I can't find a way to catch the windows key

    if ( IsReady() )
        m_stream.translatecursor( &graphicsIn );
    this->OnLocate( graphicsIn );
}

//! Redo the whole plot, only if the window has been Created
//!

template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::RenewPlot( void )
{
    if ( m_created )
    {
        WXWINDOW::Refresh();
    }
}


//! Save plot.
//!

template<class WXWINDOW>
bool wxPLplotwindow<WXWINDOW>::SavePlot( const wxString& devname, const wxString& filename )
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

//! Set whether we wish to use wxGCDC instead of a wxDC.
//!

template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::setUseGraphicsContext( bool useGraphicsContext )
{
    wxDC *drawDc;
#ifdef wxUSE_GRAPHICS_CONTEXT
    m_useGraphicsContext = useGraphicsContext;
    drawDc = m_useGraphicsContext ? (wxDC *) m_gcDc : (wxDC *) m_memoryDc;
#else
    drawDc = &m_memoryDc;
    m_useGraphicsContext = false;
#endif
    if ( IsReady() )
    {
        m_stream.SetDC( drawDc );
        RenewPlot();
    }
}

template<class WXWINDOW>
void wxPLplotwindow<WXWINDOW>::setCanvasColour( const wxColour &colour )
{
    m_canvasColour = colour;
    RenewPlot();
}

#endif // !defined( WXPLPLOTWINDOW_H__INCLUDED_ )
