// Copyright (C) 2015-2017 Phil Rosenberg
// Copyright (C) 2017 Alan W. Irwin
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
#include <wx/menu.h>
#include <wx/msgdlg.h>
// Needed for cerr and endl below
#include <iostream>

// If want to test IPC speed without plotting anything.
//#define PLPLOT_WX_NOPLOT

const int wxPlFrame::ID_FILE_EXIT      = ::wxNewId();
const int wxPlFrame::ID_HELP_ABOUT     = ::wxNewId();
const int wxPlFrame::ID_PAGE_NEXT      = ::wxNewId();
const int wxPlFrame::ID_PAGE_PREV      = ::wxNewId();
const int wxPlFrame::ID_PAGE_FIXASPECT = ::wxNewId();
const int wxPlFrame::ID_CHECK_TIMER    = ::wxNewId();


BEGIN_EVENT_TABLE( wxPlFrame, wxPLplotwindow<wxFrame> )
EVT_MENU( ID_FILE_EXIT, wxPlFrame::OnExit )
EVT_MENU( ID_HELP_ABOUT, wxPlFrame::OnAbout )
EVT_MENU( ID_PAGE_FIXASPECT, wxPlFrame::OnToggleFixAspect )
EVT_MENU( ID_PAGE_NEXT, wxPlFrame::OnNextPage )
EVT_MENU( ID_PAGE_PREV, wxPlFrame::OnPrevPage )
EVT_TIMER( ID_CHECK_TIMER, wxPlFrame::OnCheckTimer )
EVT_KEY_DOWN( wxPlFrame::OnKey )
EVT_LEFT_UP( wxPlFrame::OnMouse )
EVT_MOUSE_EVENTS( wxPlFrame::OnMouse )
END_EVENT_TABLE()


wxPlFrame::wxPlFrame( wxWindow *parent, wxWindowID id, const wxString &title, wxString file, long fileSize,
                      const wxPoint &pos, const wxSize &size, long style, const wxString &name )
    : wxPLplotwindow<wxFrame>( true, size, 1000 ), m_checkTimer( this, ID_CHECK_TIMER )
{
    wxFrame::Create( parent, id, title, pos, wxDefaultSize, style, name );
    setupMenus();

    m_viewingPage          = 0;
    m_writingPage          = 0;
    m_file                 = file;
    m_fileSize             = fileSize;
    m_inCheckTimerFunction = false;
    m_nothingToDoCounter   = 0;

    if ( file.length() > 0 )
    {
        m_memoryMap.create( file.mb_str(), m_fileSize, true, false );
        if ( m_memoryMap.isValid() )
        {
#ifdef PL_WXWIDGETS_IPC3
            m_memoryMap.initializeSemaphoresToValid( file.mb_str() );

#else           // #ifdef PL_WXWIDGETS_IPC3
            wxString mutexName = file + wxT( "mut" );
            m_mutex.create( mutexName.mb_str() );
            if ( !m_mutex.isValid() )
                m_memoryMap.close();
#endif
        }
    }
    if ( !m_memoryMap.isValid() )
    {
        throw ( "Error initializing the shared memory and/or mutex needed for the application. The application will close" );
    }

    m_locateModePage      = -1;
    m_transferComplete    = false;
    m_plottedBufferAmount = 0;
    //signal that we have opened the shared memory
#ifdef PL_WXWIDGETS_IPC3
    try
    {
        // Initialize all of m_header to beat valgrind uninitialized noise with
        // shared memory definitions of m_header.

        // This is actually an invalid value that if not overwritten
        // by the -dev wxwidgets side would throw an exception later on.
        m_header.transmissionType = transmissionRegular;
        // Sensible default
        m_header.plbufAmountToTransmit = 0;
        // This one let's -dev wxwidgets know we have made contact
        m_header.viewerOpenFlag = 1;
        // Sensible default
        m_header.locateModeFlag = 0;
        // Sensible default
        m_header.completeFlag = 0;
        // We leave uninitialized, the graphicsIn and textSizeInfo structs
        // that are part of m_header.
#ifdef PLPLOT_WX_DEBUG_OUTPUT
        std::cerr << "Before transmitBytes" << std::endl;
        std::cerr << "transmissionType = " << static_cast<unsigned int>( m_header.transmissionType ) << std::endl;
        std::cerr << "plbufAmountToTransmit = " << m_header.plbufAmountToTransmit << std::endl;
        std::cerr << "viewerOpenFlag = " << m_header.viewerOpenFlag << std::endl;
        std::cerr << "locateModeFlag = " << m_header.locateModeFlag << std::endl;
        std::cerr << "completeFlag = " << m_header.completeFlag << std::endl;
#endif  // #ifdef PLPLOT_WX_DEBUG_OUTPUT
        m_memoryMap.transmitBytes( true, &m_header, sizeof ( MemoryMapHeader ) );
    }
    catch ( const char *message )
    {
        std::cerr << "Caught exception when attempting to signal that we have opened the shared memory.  The message was " << message << std::endl;
        throw( 1 );
    }

    catch ( ... )
    {
        std::cerr << "Caught unknown exception when attempting to signal that we have opened the shared memory" << std::endl;
        throw( 1 );
    }
#else // #ifdef PL_WXWIDGETS_IPC3
    MemoryMapHeader *header = (MemoryMapHeader *) ( m_memoryMap.getBuffer() );
    header->viewerOpenFlag = 1;
#endif // #ifdef PL_WXWIDGETS_IPC3
    m_checkTimer.Start( m_idleTimerInterval );
}

void wxPlFrame::setupMenus()
{
    wxMenuBar* mbar = new wxMenuBar();

    wxMenu   * fileMenu = new wxMenu( wxT( "" ) );
    fileMenu->Append( ID_FILE_EXIT, wxT( "E&xit\tAlt+F4" ), wxT( "Exit the application" ) );
    mbar->Append( fileMenu, wxT( "&File" ) );


    wxMenu     * pageMenu      = new wxMenu( wxT( "" ) );
    wxMenuItem *aspectMenuItem = new wxMenuItem( pageMenu, ID_PAGE_FIXASPECT, wxT( "Fix Aspect" ),
        wxT( "Fix the aspect ratio of the plot" ), wxITEM_CHECK );
    pageMenu->Append( aspectMenuItem );
    aspectMenuItem->Check( true );
    pageMenu->Append( ID_PAGE_PREV, wxT( "Previous\tPgUp" ), wxT( "Move to the previous page" ) );
    pageMenu->Append( ID_PAGE_NEXT, wxT( "Next\tEnter" ), wxT( "Move to the next page" ) );
    mbar->Append( pageMenu, wxT( "&Page" ) );

    wxMenu* helpMenu = new wxMenu( wxT( "" ) );
    helpMenu->Append( ID_HELP_ABOUT, wxT( "&About\tF1" ), wxT( "Show info about this application" ) );
    mbar->Append( helpMenu, wxT( "&Help" ) );

    SetMenuBar( mbar );

    m_stream.SetFixedAspectRatio( aspectMenuItem->IsChecked() );
}

void wxPlFrame::OnExit( wxCommandEvent& event )
{
    Close();
}

void wxPlFrame::OnAbout( wxCommandEvent& event )
{
    wxMessageBox( wxT( "wxPlViewer version 1.00.0" ), wxT( "About wxPlViewer..." ) );
}

wxPlFrame::~wxPlFrame()
{
}

void wxPlFrame::OnCheckTimer( wxTimerEvent &event )
{
#ifdef PL_WXWIDGETS_IPC3
    try
    {
        // call ReadTransmission once which should stop timer
        // and return false
        if ( ReadTransmission() )
            throw ( "wxPlFrame::OnCheckTimer (internal error): ReadTransmission should always return false" );
    }
    catch ( const char *message )
    {
        std::cerr << "Caught exception in wxPlFrame::OnCheckTimer.  The message was " << message << std::endl;
        throw( 1 );
    }

    catch ( ... )
    {
        std::cerr << "Caught unknown exception in wxPlFrame::OnCheckTimer" << std::endl;
        throw( 1 );
    }

#else // #ifdef PL_WXWIDGETS_IPC3
      //repeatedly call ReadTransmission until there is nothing
      //left to read
    PLINT nFalses = 0;
    while ( nFalses < 100 )
    {
        if ( ReadTransmission() )
            nFalses = 0;
        else
        {
            ++nFalses;
            wxMilliSleep( 1 );
        }
    }
#endif // #ifdef PL_WXWIDGETS_IPC3
}

//This function reads any transmissions from the shared memory and acts upon
//it. If there was nothing to read it returns false, otherwise it returns
//true indicating that the program may want to read some more.
bool wxPlFrame::ReadTransmission()
{
    //avoid re-entrant behaviour if some function yields allowing the
    //timer to call this function again
    if ( m_inCheckTimerFunction )
        return true;
    m_inCheckTimerFunction = true;

    if ( !m_memoryMap.isValid() )
        throw( "wxPlFrame::ReadTransmission: invalid Memory map" );

 #ifdef PL_WXWIDGETS_IPC3
    size_t nbytes;
    // Read the entire plbuf generated by a series of calls on the writer (-dev wxwidgets)
    // side to transmitBuffer, which writes a header and optionally a sequential
    // part or all of plbuf per each call.  The end of that series of calls
    // is signalled by the writer side setting m_header.completeFlag to a non-zero value.

    do
    {
        // First read complete data header to find out everything we need to know
        // about the transmitBuffer call.
        m_memoryMap.receiveBytes( true, &m_header, sizeof ( MemoryMapHeader ) );
#ifdef PLPLOT_WX_DEBUG_OUTPUT
        std::cerr << "After receiveBytes" << std::endl;
        std::cerr << "transmissionType = " << static_cast<unsigned int>( m_header.transmissionType ) << std::endl;
        std::cerr << "plbufAmountToTransmit = " << m_header.plbufAmountToTransmit << std::endl;
        std::cerr << "viewerOpenFlag = " << m_header.viewerOpenFlag << std::endl;
        std::cerr << "locateModeFlag = " << m_header.locateModeFlag << std::endl;
        std::cerr << "completeFlag = " << m_header.completeFlag << std::endl;
#endif  // #ifdef PLPLOT_WX_DEBUG_OUTPUT
        switch ( m_header.transmissionType )
        {
        // Three valid cases which should be handled after plbuf has been received and processed.
        case transmissionEndOfPage:
        case transmissionEndOfPageNoPause:
        case transmissionLocate:
            break;
            // Other valid cases which should be handled before plbuf has been received and processed.
#if 0
        case transmissionRequestTextSize:
            wxClientDC dc( this );
            wxCoord    width;
            wxCoord    height;
            wxCoord    depth;
            wxCoord    leading;
            wxFont     *font = wxTheFontList->FindOrCreateFont( m_header.textSizeInfo.pt, m_header.textSizeInfo.family,
                m_header.textSizeInfo.style, m_header.textSizeInfo.weight, m_header.textSizeInfo.underlined );
            dc.GetTextExtent( wxString( m_header.textSizeInfo.text ), &width, &height, &depth, &leading, font );
            m_header.textSizeInfo.width   = long(width);
            m_header.textSizeInfo.height  = long(height);
            m_header.textSizeInfo.depth   = long(depth);
            m_header.textSizeInfo.leading = long(leading);
            m_header.textSizeInfo.written = true;
            break;
#endif      // #if 0
        // Used only to transmit  m_header.completeFlag != 0 to terminate this loop
        case transmissionComplete:
        // Used only for flush.
        case transmissionFlush:
            break;
        case transmissionClose:
            Close( 0 );
            break;
        case transmissionBeginPage:
            m_pageBuffers.resize( m_pageBuffers.size() + 1 );
            m_bufferValidFlags.push_back( false );
            m_writingPage = m_pageBuffers.size() - 1;
            break;

        // Invalid cases.
        default:
            throw( "wxPlFrame::ReadTransmission: read invalid value of transmissionType" );
            break;
        }

        if ( m_header.plbufAmountToTransmit > 0 )
        {
            char * plbufBuffer = (char *) malloc( m_header.plbufAmountToTransmit );
            if ( plbufBuffer == NULL )
                throw( "wxPlFrame::ReadTransmission: malloc of plbufBuffer failed" );
            m_memoryMap.receiveBytes( false, plbufBuffer, m_header.plbufAmountToTransmit );
#ifdef PLPLOT_WX_DEBUG_OUTPUT
            std::cerr << "Successful read of plbuf" << std::endl;
#endif      // #ifdef PLPLOT_WX_DEBUG_OUTPUT
#ifndef PLPLOT_WX_NOPLOT
            m_pageBuffers[m_writingPage].insert( m_pageBuffers[m_writingPage].end(),
                plbufBuffer, plbufBuffer + m_header.plbufAmountToTransmit );
#endif      // #ifndef PLPLOT_WX_NOPLOT
            m_bufferValidFlags[m_writingPage] = true;
            // Conditionally plot buffer, but I (AWI) have no clue where the 1024 below
            // comes from.
            if ( m_writingPage == m_viewingPage &&
                 ( m_plottedBufferAmount + 1024 ) < m_pageBuffers[ m_writingPage ].size() )
                SetPageAndUpdate();
            free( plbufBuffer );
        }
        if ( m_header.transmissionType == transmissionEndOfPage || m_header.transmissionType == transmissionEndOfPageNoPause )
        {
            // N.B. can receive (e.g., example 2) two transmissionEndOfPage commands in a row
            // with the second one having m_header.plbufAmountToTransmit == 0.  But I assume
            // SetPageAndUpdate should be able to handle this case.
            if ( m_header.transmissionType == transmissionEndOfPage )
                SetPageAndUpdate();
            else
                SetPageAndUpdate( m_writingPage );
        }
        if ( m_header.transmissionType == transmissionLocate )
        {
            SetPageAndUpdate();
            m_locateModePage = m_writingPage;
        }
    }
    while ( !( m_header.transmissionType == transmissionLocate || m_header.completeFlag != 0 || m_header.transmissionType == transmissionRequestTextSize || m_header.transmissionType == transmissionFlush ) );

    if ( m_header.completeFlag != 0 )
    {
        //Once the complete flag is set to non-zero then stop looking
        m_transferComplete = true;
        m_checkTimer.Stop();
    }
    else if ( m_currentTimerInterval != m_busyTimerInterval )
    {
        //If we have something to read then make sure
        //we keep checking regularly for more
        m_checkTimer.Stop();
        m_checkTimer.Start( m_busyTimerInterval );
        m_currentTimerInterval = m_busyTimerInterval;
        m_nothingToDoCounter   = 0;
    }

    if ( m_header.locateModeFlag != 0 )
    {
        //Stop checking for new data if we are in locate mode.
        //This is a workaround for a hang situation which occurs
        //with the IPC3 comms when bot the viewer and the core
        //code end up stuck waiting for new data.
        //This should be removed when that hang situation is
        //removed by including a timeout option for the
        //receiveData function.
        m_checkTimer.Stop();
    }

    // Allow the timer to call this function again
    m_inCheckTimerFunction = false;

    // Always return false.
    return false;

#else // #ifdef PL_WXWIDGETS_IPC3
    MemoryMapHeader    *header = (MemoryMapHeader *) ( m_memoryMap.getBuffer() );
    //lock the mutex for the duration of this scope
    PLNamedMutexLocker locker( &m_mutex );
    //Check if there is anything to read
    if ( header->readLocation == header->writeLocation )
    {
        ++m_nothingToDoCounter;
        if ( header->completeFlag != 0 )
        {
            //if there is nothing to read and the complete flag is set then
            // stop looking
            m_transferComplete = true;
            m_checkTimer.Stop();
        }
        else if ( m_currentTimerInterval != m_idleTimerInterval && m_nothingToDoCounter > m_nothingToDoCounterLimit )
        {
            //if there is nothing to read but we might have more to come
            //then check less frequently
            m_checkTimer.Stop();
            m_checkTimer.Start( m_idleTimerInterval );
            m_currentTimerInterval = m_idleTimerInterval;
        }

        //nothing to do so return
        m_inCheckTimerFunction = false;
        return false;
    }

    if ( m_currentTimerInterval != m_busyTimerInterval )
    {
        //If we have something to read then make sure
        //we keep checking regularly for more
        m_checkTimer.Stop();
        m_checkTimer.Start( m_busyTimerInterval );
        m_currentTimerInterval = m_busyTimerInterval;
        m_nothingToDoCounter   = 0;
    }

    unsigned char transmissionType;
    transmissionType = *( (unsigned char *) ( m_memoryMap.getBuffer() + header->readLocation ) );
    size_t        nRead = sizeof ( unsigned char );

    if ( transmissionType == transmissionSkipFileEnd )
    {
        header->readLocation   = plMemoryMapReservedSpace;
        m_inCheckTimerFunction = false;
        return true;
    }
    else if ( transmissionType == transmissionBeginPage )
    {
        m_pageBuffers.resize( m_pageBuffers.size() + 1 );
        m_bufferValidFlags.push_back( false );
        m_writingPage = m_pageBuffers.size() - 1;
    }
    else if ( transmissionType == transmissionEndOfPage || transmissionType == transmissionEndOfPageNoPause )
    {
        if ( !m_bufferValidFlags[m_writingPage] )
            throw( "Received an end of page transmission after an incomplete or no draw instruction" );
        if ( transmissionType == transmissionEndOfPage )
            SetPageAndUpdate();
        else
            SetPageAndUpdate( m_writingPage );
    }
    else if ( transmissionType == transmissionLocate )
    {
        SetPageAndUpdate();
        m_locateModePage = m_writingPage;
    }
    else if ( transmissionType == transmissionRequestTextSize )
    {
        wxClientDC dc( this );
        wxCoord    width;
        wxCoord    height;
        wxCoord    depth;
        wxCoord    leading;
        wxFont     *font = wxTheFontList->FindOrCreateFont( header->textSizeInfo.pt, header->textSizeInfo.family,
            header->textSizeInfo.style, header->textSizeInfo.weight, header->textSizeInfo.underlined );
        dc.GetTextExtent( wxString( header->textSizeInfo.text ), &width, &height, &depth, &leading, font );
        header->textSizeInfo.width   = long(width);
        header->textSizeInfo.height  = long(height);
        header->textSizeInfo.depth   = long(depth);
        header->textSizeInfo.leading = long(leading);
        header->textSizeInfo.written = true;
    }
    else if ( transmissionType == transmissionPartial || transmissionType == transmissionComplete )
    {
        size_t dataSize;
        dataSize = *(size_t *) ( (unsigned char *) ( m_memoryMap.getBuffer() + header->readLocation +
                                                     sizeof ( transmissionType ) ) );
#ifndef PLPLOT_WX_NOPLOT
        m_pageBuffers[m_writingPage].insert( m_pageBuffers[m_writingPage].end(),
            m_memoryMap.getBuffer() + header->readLocation + sizeof ( transmissionType ) + sizeof ( dataSize ),
            m_memoryMap.getBuffer() + header->readLocation + sizeof ( transmissionType ) + sizeof ( dataSize ) + dataSize );
#endif  // #ifndef PLPLOT_WX_NOPLOT
        nRead += sizeof ( dataSize ) + dataSize;
        if ( transmissionType == transmissionComplete )
            m_bufferValidFlags[m_writingPage] = true;
        else
            m_bufferValidFlags[m_writingPage] = false;

        // If we have a new unplotted buffer, then it is due to either
        // a flush or we have so much data we have wrapped the buffer.
        if ( m_writingPage == m_viewingPage &&
             ( m_plottedBufferAmount + 1024 ) < m_pageBuffers[ m_writingPage ].size() )
            SetPageAndUpdate();
    }
    else if ( transmissionType == transmissionClose )
    {
        Close( 0 );
    }
    header->readLocation += nRead;
    if ( header->readLocation == m_memoryMap.getSize() )
        header->readLocation = plMemoryMapReservedSpace;
    m_inCheckTimerFunction = false;
    return true;
#endif //#ifdef PL_WXWIDGETS_IPC3
}
void wxPlFrame::OnToggleFixAspect( wxCommandEvent &event )
{
    fixAspect( event.IsChecked() );
}

void wxPlFrame::fixAspect( bool fix )
{
    m_stream.SetFixedAspectRatio( fix );
    if ( !fix )
        this->Refresh();
}
void wxPlFrame::SetClientSize( int width, int height )
{
    wxFrame::SetClientSize( width, height );
    m_stream.SetSize( width, height );
}

void wxPlFrame::SetClientSize( const wxSize &size )
{
    SetClientSize( size.GetWidth(), size.GetHeight() );
}

void wxPlFrame::SetClientSize( const wxRect &rect )
{
    SetClientSize( rect.GetWidth(), rect.GetHeight() );
}

void wxPlFrame::OnNextPage( wxCommandEvent& event )
{
    SetPageAndUpdate( m_viewingPage + 1 );
}

void wxPlFrame::OnPrevPage( wxCommandEvent& event )
{
    SetPageAndUpdate( m_viewingPage - 1 );
}

void wxPlFrame::OnMouse( wxMouseEvent &event )
{
    //save the mouse position for use in key presses
    m_cursorPosition = event.GetPosition();

    //If the mouse button was clicked then
    if ( m_locateModePage == m_viewingPage && event.ButtonDown() || event.ButtonUp() )
    {
        wxSize clientSize = GetClientSize();
#ifdef PL_WXWIDGETS_IPC3
        m_header.graphicsIn.pX = m_cursorPosition.x;
        m_header.graphicsIn.pY = m_cursorPosition.y;
        m_header.graphicsIn.dX = PLFLT( m_cursorPosition.x + 0.5 ) / PLFLT( clientSize.GetWidth() );
        m_header.graphicsIn.dY = 1.0 - PLFLT( m_cursorPosition.y + 0.5 ) / PLFLT( clientSize.GetHeight() );

        int button = event.GetButton();

        if ( button == wxMOUSE_BTN_LEFT )
            m_header.graphicsIn.button = 1;
        else if ( button == wxMOUSE_BTN_MIDDLE )
            m_header.graphicsIn.button = 2;
        else if ( button == wxMOUSE_BTN_RIGHT )
            m_header.graphicsIn.button = 3;
        else if ( button == wxMOUSE_BTN_AUX1 )
            m_header.graphicsIn.button = 4;
        else if ( button == wxMOUSE_BTN_AUX2 )
            m_header.graphicsIn.button = 5;

        //flags the buttons that were down before this event
        m_header.graphicsIn.state = 0;
        if ( ( event.LeftIsDown() && !event.LeftDown() ) || event.LeftUp() )
            m_header.graphicsIn.state |= ( 1 << 8 );
        if ( ( event.MiddleIsDown() && !event.MiddleDown() ) || event.MiddleUp() )
            m_header.graphicsIn.state |= ( 1 << 9 );
        if ( ( event.RightIsDown() && !event.RightDown() ) || event.RightUp() )
            m_header.graphicsIn.state |= ( 1 << 10 );
        if ( ( event.Aux1IsDown() && !event.Aux1Down() ) || event.Aux1Up() )
            m_header.graphicsIn.state |= ( 1 << 11 );
        if ( ( event.Aux2IsDown() && !event.Aux2Down() ) || event.Aux2Up() )
            m_header.graphicsIn.state |= ( 1 << 12 );

        m_header.graphicsIn.keysym = 0x20;                // keysym for button event from xwin.c

        m_header.locateModeFlag = 0;
        m_memoryMap.transmitBytes( true, &m_header, sizeof ( MemoryMapHeader ) );
        m_locateModePage = -1;

        //Restart checking for new data now locate mode is complete.
        //This is a workaround for a hang situation which occurs
        //with the IPC3 comms when bot the viewer and the core
        //code end up stuck waiting for new data.
        //This should be removed when that hang situation is
        //removed by including a timeout option for the
        //receiveData function.
        m_checkTimer.Start();
#else   // #ifdef PL_WXWIDGETS_IPC3
        PLNamedMutexLocker locker( &m_mutex );
        MemoryMapHeader    *header = (MemoryMapHeader *) m_memoryMap.getBuffer();
        //PLGraphicsIn &     graphicsIn = header->graphicsIn;
        header->graphicsIn.pX = m_cursorPosition.x;
        header->graphicsIn.pY = m_cursorPosition.y;
        header->graphicsIn.dX = PLFLT( m_cursorPosition.x + 0.5 ) / PLFLT( clientSize.GetWidth() );
        header->graphicsIn.dY = 1.0 - PLFLT( m_cursorPosition.y + 0.5 ) / PLFLT( clientSize.GetHeight() );

        int button = event.GetButton();

        if ( button == wxMOUSE_BTN_LEFT )
            header->graphicsIn.button = 1;
        else if ( button == wxMOUSE_BTN_MIDDLE )
            header->graphicsIn.button = 2;
        else if ( button == wxMOUSE_BTN_RIGHT )
            header->graphicsIn.button = 3;
        else if ( button == wxMOUSE_BTN_AUX1 )
            header->graphicsIn.button = 4;
        else if ( button == wxMOUSE_BTN_AUX2 )
            header->graphicsIn.button = 5;

        //flags the buttons that were down before this event
        header->graphicsIn.state = 0;
        if ( ( event.LeftIsDown() && !event.LeftDown() ) || event.LeftUp() )
            header->graphicsIn.state |= ( 1 << 8 );
        if ( ( event.MiddleIsDown() && !event.MiddleDown() ) || event.MiddleUp() )
            header->graphicsIn.state |= ( 1 << 9 );
        if ( ( event.RightIsDown() && !event.RightDown() ) || event.RightUp() )
            header->graphicsIn.state |= ( 1 << 10 );
        if ( ( event.Aux1IsDown() && !event.Aux1Down() ) || event.Aux1Up() )
            header->graphicsIn.state |= ( 1 << 11 );
        if ( ( event.Aux2IsDown() && !event.Aux2Down() ) || event.Aux2Up() )
            header->graphicsIn.state |= ( 1 << 12 );

        header->graphicsIn.keysym = 0x20;                // keysym for button event from xwin.c

        header->locateModeFlag = 0;
        m_locateModePage       = -1;
#endif  // #ifdef PL_WXWIDGETS_IPC3
    }
}

void wxPlFrame::OnKey( wxKeyEvent &event )
{
#ifndef PL_WXWIDGETS_IPC3
    //This only works on Windows, unfortunately on wxGTK, wxFrames cannot catch key presses
    if ( m_locateModePage = m_viewingPage )
    {
        PLNamedMutexLocker locker( &m_mutex );
        MemoryMapHeader    *header = (MemoryMapHeader *) m_memoryMap.getBuffer();

        wxSize             clientSize = GetClientSize();

        header->graphicsIn.pX = m_cursorPosition.x;
        header->graphicsIn.pY = m_cursorPosition.y;
        header->graphicsIn.dX = PLFLT( m_cursorPosition.x + 0.5 ) / PLFLT( clientSize.GetWidth() );
        header->graphicsIn.dY = 1.0 - PLFLT( m_cursorPosition.y + 0.5 ) / PLFLT( clientSize.GetHeight() );
        // gin->state = keyEvent->state;

        int keycode = event.GetKeyCode();
        header->graphicsIn.string[0] = (char) keycode;
        header->graphicsIn.string[1] = '\0';

        // ESCAPE, RETURN, etc. are already in ASCII equivalent
        header->graphicsIn.keysym = keycode;

        header->locateModeFlag = 0;
        m_locateModePage       = -1;
    }
#endif //#ifndef PL_WXWIDGETS_IPC3
}

void wxPlFrame::SetPageAndUpdate( size_t page )
{
    //if we get a page of -1 (which will really be max size_t) then just update the current page
    //otherwise switch to the given page
    if ( page != size_t( -1 ) )
    {
        if ( page >= m_pageBuffers.size() )
        {
            if ( m_transferComplete )
                Close();
            return;
        }
        if ( page != m_viewingPage )
        {
            m_viewingPage         = page;
            m_plottedBufferAmount = 0;
        }
    }

    // If there is an unplotted buffer and the buffer is valid
    // (contains some instructions and ends with a valid instruction)
    // then send that buffer to the driver and replot the page.
    if ( m_bufferValidFlags[m_viewingPage] && m_plottedBufferAmount < m_pageBuffers[m_viewingPage].size() )
    {
        if ( m_plottedBufferAmount == 0 )
            //this ensures that any buffer generated by plplot initialisation is overwritten
            //often there is a begin page already in the buffer by the time we get here
            GetStream()->ImportBuffer( &m_pageBuffers[m_viewingPage][0], m_pageBuffers[m_viewingPage].size() );
        else
            GetStream()->AppendBuffer( &m_pageBuffers[m_viewingPage][m_plottedBufferAmount], m_pageBuffers[m_viewingPage].size() - m_plottedBufferAmount );
        m_plottedBufferAmount = m_pageBuffers[m_viewingPage].size();
        Refresh();
    }
}
