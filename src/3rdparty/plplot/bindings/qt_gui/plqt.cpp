// This software was donated under the LGPL to the PLplot project in
// March 2009 by the
// Cluster Science Centre
// QSAS team,
// Imperial College, London
//
// Copyright (C) 2009  Imperial College, London
// Copyright (C) 2009-2019  Alan W. Irwin
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

#include "qt.h"

// Global variables for Qt driver.
PLDLLIMPEXP_QT_DATA( int ) vectorize = 0;
PLDLLIMPEXP_QT_DATA( int ) lines_aa  = 1;
PLDLLIMPEXP_QT_DATA( MasterHandler ) handler;

// Master Device Handler for multiple streams
// Only handles multiple Qt devices
MasterHandler::MasterHandler() : QObject()
{
    masterDevice = NULL;
}

bool MasterHandler::isMasterDevice( QtPLDriver* d )
{
    return d == masterDevice;
}

void MasterHandler::setMasterDevice( QtPLDriver* d )
{
    masterDevice = d;
}

void MasterHandler::DeviceChangedPage( QtPLDriver* d )
{
    if ( d == masterDevice )
    {
        emit MasterChangedPage();
    }
}

void MasterHandler::DeviceClosed( QtPLDriver* d )
{
    if ( d == masterDevice )
    {
        emit MasterClosed();
        masterDevice = NULL;
    }
}

///////////// Generic Qt driver class /////////////////
QMutex QtPLDriver::mutex;

QtPLDriver::QtPLDriver( PLINT i_iWidth, PLINT i_iHeight )
{
    m_dWidth  = i_iWidth;
    m_dHeight = i_iHeight;
}

QtPLDriver::~QtPLDriver()
{
}

void QtPLDriver::setPLStream( PLStream *p )
{
    pls = p;
}

void QtPLDriver::drawArc( short x, short y, short a, short b, PLFLT angle1, PLFLT angle2, PLFLT rotate, bool fill )
{
    if ( !m_painterP->isActive() )
        return;
    QRectF rect( (PLFLT) ( x - a ) * downscale,
                 m_dHeight - (PLFLT) ( y + b ) * downscale,
                 (PLFLT) a * downscale * 2,
                 (PLFLT) b * downscale * 2
                 );
    if ( rotate != 0.0 )
    {
        m_painterP->save();
        m_painterP->translate( (PLFLT) x * downscale, m_dHeight - (PLFLT) y * downscale );
        m_painterP->rotate( -rotate );
        m_painterP->translate( -(PLFLT) x * downscale, -m_dHeight + (PLFLT) y * downscale );
    }

    if ( fill )
        m_painterP->drawPie( rect, (int) ( angle1 * 16 ), (int) ( ( angle2 - angle1 ) * 16 ) );
    else
        m_painterP->drawArc( rect, (int) ( angle1 * 16 ), (int) ( ( angle2 - angle1 ) * 16 ) );

    if ( rotate != 0.0 )
    {
        m_painterP->restore();
    }
}

void QtPLDriver::drawLine( short x1, short y1, short x2, short y2 )
{
    if ( !m_painterP->isActive() )
        return;
    QLineF line( (PLFLT) x1 * downscale,
                 m_dHeight - (PLFLT) y1 * downscale,
                 (PLFLT) x2 * downscale,
                 m_dHeight - (PLFLT) y2 * downscale
                 );

    m_painterP->drawLine( line );
}

void QtPLDriver::drawPolyline( short * x, short * y, PLINT npts )
{
    if ( !m_painterP->isActive() )
        return;
    QPointF * polyline = new QPointF[npts];
    for ( int i = 0; i < npts; ++i )
    {
        polyline[i].setX( (PLFLT) x[i] * downscale );
        polyline[i].setY( m_dHeight - (PLFLT) y[i] * downscale );
    }
    m_painterP->drawPolyline( polyline, npts );
    delete[] polyline;
}

void QtPLDriver::drawPolygon( short * x, short * y, PLINT npts )
{
    if ( !m_painterP->isActive() )
        return;
    QPointF * polygon = new QPointF[npts];
    for ( int i = 0; i < npts; ++i )
    {
        polygon[i].setX( (PLFLT) x[i] * downscale );
        polygon[i].setY( m_dHeight - (PLFLT) y[i] * downscale );
    }
    if ( plsc->dev_eofill )
        m_painterP->drawPolygon( polygon, npts, Qt::OddEvenFill );
    else
        m_painterP->drawPolygon( polygon, npts, Qt::WindingFill );
    delete[] polygon;
}


QFont QtPLDriver::getFont( PLUNICODE unicode )
{
    // Get new font parameters
    unsigned char fontFamily, fontStyle, fontWeight;

    plP_fci2hex( unicode, &fontFamily, PL_FCI_FAMILY );
    plP_fci2hex( unicode, &fontStyle, PL_FCI_STYLE );
    plP_fci2hex( unicode, &fontWeight, PL_FCI_WEIGHT );

    QFont f;

    f.setPointSizeF( currentFontSize * currentFontScale < 4 ? 4 : currentFontSize * currentFontScale );

    switch ( fontFamily )
    {
    case 1:
        f.setStyleHint( QFont::Serif );
        break;
    case 2:
        f.setStyleHint( QFont::TypeWriter );
        break;
    case 0: case 3: case 4: default:
        f.setStyleHint( QFont::SansSerif );
        break;
    }

    // Use a bogus family name here to force Qt to find fonts using setStyleHint instead.
    // N.B. specifying an empty string which you would think would do this same job,
    // does not work as well as a non-empty string pointing to a bogus family name
    // for both the opensuse Qt5 version as well as the Debian Testing version of
    // that library.
    f.setFamily( "bogusxxxxxxxxxxxxxxxxxxxxbogus" );

    if ( fontStyle )
        f.setItalic( true );
    if ( fontWeight )
        f.setWeight( QFont::Bold );
    else
        f.setWeight( QFont::Normal );

    f.setUnderline( underlined );
    f.setOverline( overlined );

    return f;
}

void QtPLDriver::drawTextInPicture( QPainter* p, const QString& text )
{
    QRectF   rect( 0., 0., 0., 0. );
    QRectF   bounding;
    QPicture tempPic;
    QPainter tempPainter( &tempPic );
    tempPainter.setFont( p->font() );

    if ( vectorize )
    {
        bounding = tempPainter.boundingRect( rect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip, text );

        tempPainter.save();
//         QPen savePen=tempPainter.pen();
        QPen pen = QPen( Qt::NoPen );
        tempPainter.setPen( pen );

        double       offset = QFontMetrics( tempPainter.font(), &tempPic ).boundingRect( text ).top(); // Distance between the baseline and the top of the bounding box

        QPainterPath path;
        path.addText( QPointF( bounding.left(), bounding.top() - offset ), tempPainter.font(), text );
        tempPainter.drawPath( path );
        tempPainter.restore();
    }
    else
    {
        tempPainter.drawText( rect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip, text, &bounding );
    }

    tempPainter.end();

    p->drawPicture( (int) ( xOffset + bounding.width() / 2. ), (int) -yOffset, tempPic );

    xOffset += bounding.width();
}

// 0.8 mimics the offset of first superscript/subscript level implemented
// in plstr (plsym.c) for Hershey fonts.  Indeed when comparing with
// -dev xwin results this factor appears to offset the centers of the
// letters appropriately (but not their edges since different font sizes
// are involved).
# define RISE_FACTOR    0.8

QPicture QtPLDriver::getTextPicture( PLUNICODE fci, PLUNICODE* text, int len, PLFLT chrht )
{
    char plplotEsc;
    plgesc( &plplotEsc );

    QPicture res;
    QPainter p( &res );

    QString  currentString;
    PLFLT    old_sscale, sscale, old_soffset, soffset;
    PLINT    level    = 0;
    PLFLT    dyOffset = 0.;
#ifdef PLPLOT_USE_QT5
    // Empirical Y offset of text (needed for Qt5 for some reason).
    // Note this was derived using the test_circle.py python script
    // with the vertical alignment of the light diagonal cross
    // optimized.  That example shows that other glyphs (except for
    // the well-known asterisk case which is vertically misaligned by
    // font design) do not have the best vertical alignment.  This is
    // contrary to the results for cairo and qt with Qt4 which need no
    // special empirical Y offset to get good vertical alignment for
    // most glyphs of that example (other than the asterisk).  An
    // additional issue which confuses the decision concerning the
    // best vertical alignment for qt with Qt5 is the font selection
    // for qt with Qt5 is quite different than that for qt with Qt4 or
    // cairo.  I assume all these issues are due to Qt5 version 5.2.1
    // font selection and font alignment bugs which will be addressed
    // for future Qt5 versions.
    // PLFLT empirical_yOffset = -0.63 * chrht * POINTS_PER_INCH / 25.4;
    // With Debian Jessie Qt5 version 5.3.2, that font alignment bug
    // finally appears to be fixed.
    PLFLT empirical_yOffset = 0.;
#else
    PLFLT empirical_yOffset = 0.;
#endif
    yOffset = empirical_yOffset;
    xOffset = 0.;

    // Scaling factor of 1.45 determined empirically to make all qt results
    // have the same character size as cairo results (taking into account
    // the slightly different actual glyph sizes for the different
    // default fonts found by cairo and qt).
    currentFontSize  = chrht * POINTS_PER_INCH / 25.4 * 1.45;
    currentFontScale = 1.;
    underlined       = false;
    overlined        = false;

    p.setFont( getFont( fci ) );

    int i = 0;
    while ( i < len )
    {
        if ( text[i] < PL_FCI_MARK ) // Not a font change
        {
            if ( text[i] != (PLUNICODE) plplotEsc )
            {
                currentString.append( QString( QChar( text[i] ) ) );
                ++i;
                continue;
            }
            ++i; // Now analyse the escaped character
            switch ( text[i] )
            {
            case 'd': //subscript
                drawTextInPicture( &p, currentString );
                currentString.clear();
                plP_script_scale( FALSE, &level,
                    &old_sscale, &sscale, &old_soffset, &soffset );
                currentFontScale = sscale;

                // The correction for the difference in magnitude
                // between the baseline and middle coordinate systems
                // for subscripts should be
                // -0.5*(fontSize - superscript/subscript fontSize).
                // dyOffset = -0.5 * currentFontSize * ( 1.0 - sscale );
                // But empirically this change in offset should not be applied
                // so leave it at its initial value of zero.
                yOffset = empirical_yOffset - ( currentFontSize * RISE_FACTOR * soffset + dyOffset );

                p.setFont( getFont( fci ) );
                break;

            case 'u':  //superscript
                drawTextInPicture( &p, currentString );

                currentString.clear();
                plP_script_scale( TRUE, &level,
                    &old_sscale, &sscale, &old_soffset, &soffset );
                currentFontScale = sscale;

                // The correction for the difference in magnitude
                // between the baseline and middle coordinate systems
                // for superscripts should be
                // 0.5*(fontSize - superscript/subscript fontSize).
                // dyOffset = 0.5 * currentFontSize * ( 1.0 - sscale );
                // But empirically this change in offset should not be applied
                // so leave it at its initial value of zero.
                yOffset = empirical_yOffset + currentFontSize * RISE_FACTOR * soffset + dyOffset;

                p.setFont( getFont( fci ) );
                break;

            case '-':
                drawTextInPicture( &p, currentString );

                currentString.clear();
                underlined = !underlined;
                p.setFont( getFont( fci ) );
                break;

            case '+':
                drawTextInPicture( &p, currentString );

                currentString.clear();
                overlined = !overlined;
                p.setFont( getFont( fci ) );
                break;


            case '#':
                currentString.append( QString( (QChar *) &( text[i] ), 1 ) );
                break;

            default:
                std::cout << "unknown escape char " << ( (QChar) text[i] ).toLatin1() << std::endl;
                break;
            }
        }
        else // Font change
        {
            drawTextInPicture( &p, currentString );

            currentString.clear();
            fci = text[i];
            p.setFont( getFont( fci ) );
        }
        ++i;
    }
    drawTextInPicture( &p, currentString );

    p.end();

    return res;
}

void QtPLDriver::drawText( EscText* txt )
{
    if ( !m_painterP->isActive() )
        return;

    // Check that we got unicode, warning message and return if not
    if ( txt->unicode_array_len == 0 )
    {
        printf( "Non unicode string passed to a Qt driver, ignoring\n" );
        return;
    }

    // Check that unicode string isn't longer then the max we allow
    if ( txt->unicode_array_len >= 500 )
    {
        printf( "Sorry, the Qt drivers only handle strings of length < %d\n", 500 );
        return;
    }

    PLFLT rotation, shear, stride;
    plRotationShear( txt->xform, &rotation, &shear, &stride );

    double    picDpi;
    PLUNICODE fci;
    plgfci( &fci );
    QPicture  picText = getTextPicture( fci, txt->unicode_array, txt->unicode_array_len, pls->chrht );
    picDpi = picText.logicalDpiY();

    if ( pls->get_string_length )
    {
        pls->string_length = ( (PLFLT) xOffset / picDpi ) * 25.4;
        return;
    }

    m_painterP->setClipping( true );
    m_painterP->setClipRect( QRect( (int) ( pls->clpxmi * downscale ), (int) ( m_dHeight - pls->clpyma * downscale ), (int) ( ( pls->clpxma - pls->clpxmi ) * downscale ), (int) ( ( pls->clpyma - pls->clpymi ) * downscale ) ), Qt::ReplaceClip );

    rotation -= pls->diorot * M_PI / 2.0;
    m_painterP->translate( txt->x * downscale, m_dHeight - txt->y * downscale );
    QMatrix rotShearMatrix( cos( rotation ) * stride, -sin( rotation ) * stride, cos( rotation ) * sin( shear ) + sin( rotation ) * cos( shear ), -sin( rotation ) * sin( shear ) + cos( rotation ) * cos( shear ), 0., 0. );

    m_painterP->setWorldMatrix( rotShearMatrix, true );

    m_painterP->translate( -txt->just * xOffset * m_painterP->device()->logicalDpiY() / picDpi, 0. );

    m_painterP->drawPicture( 0, 0, picText );

    m_painterP->resetTransform();;
    m_painterP->setClipping( false );
}

void QtPLDriver::setColor( int r, int g, int b, double alpha )
{
    if ( !m_painterP->isActive() )
        return;

    QPen p = m_painterP->pen();
    p.setColor( QColor( r, g, b, (int) ( alpha * 255 ) ) );
    m_painterP->setPen( p );

    QBrush B = m_painterP->brush();
    B.setColor( QColor( r, g, b, (int) ( alpha * 255 ) ) );
    B.setStyle( Qt::SolidPattern );
    m_painterP->setBrush( B );
}

void QtPLDriver::setGradient( int x1, int x2, int y1, int y2,
                              unsigned char *r, unsigned char *g,
                              unsigned char *b, PLFLT *alpha, PLINT ncol1 )
{
    if ( !m_painterP->isActive() || ncol1 < 2 )
        return;

    int             i;
    qreal           stop_arg;
    QLinearGradient linear_gradient;
    QGradientStops  stops;

    linear_gradient = QLinearGradient(
        QPointF( (qreal) ( x1 * downscale ), (qreal) ( m_dHeight - y1 * downscale ) ),
        QPointF( (qreal) ( x2 * downscale ), (qreal) ( m_dHeight - y2 * downscale ) ) );

    for ( i = 0; i < ncol1; i++ )
    {
        stop_arg = (qreal) i / (qreal) ( ncol1 - 1 );
        stops << QGradientStop( stop_arg, QColor( r[i], g[i],
                b[i], (int) ( alpha[i] * 255 ) ) );
    }
    linear_gradient.setStops( stops );
    m_painterP->setBrush( linear_gradient );
}

void QtPLDriver::setWidthF( PLFLT w )
{
    if ( !m_painterP->isActive() )
        return;

    QPen p = m_painterP->pen();
    p.setWidthF( w );
    m_painterP->setPen( p );
}

// void QtPLDriver::setDashed(PLINT nms, PLINT* mark, PLINT* space)
// {
//     if(!m_painterP->isActive()) return;
//
//     QVector<qreal> vect;
//     for(int i=0; i<nms; ++i)
//     {
//         vect << (PLFLT)mark[i]*m_painterP->device()->logicalDpiX()/25400.;
//         vect << (PLFLT)space[i]*m_painterP->device()->logicalDpiX()/25400.;
//     }
//     QPen p=m_painterP->pen();
//     p.setDashPattern(vect);
//     m_painterP->setPen(p);
// }

void QtPLDriver::setSolid()
{
    if ( !m_painterP->isActive() )
        return;

    QPen p = m_painterP->pen();
    p.setStyle( Qt::SolidLine );
    m_painterP->setPen( p );
}

////////////////// Raster driver-specific definitions: class and interface functions /////////
#if defined ( PLD_bmpqt ) || defined ( PLD_jpgqt ) || defined ( PLD_pngqt ) || defined ( PLD_ppmqt ) || defined ( PLD_tiffqt ) || defined ( PLD_memqt )
QtRasterDevice::QtRasterDevice( int i_iWidth, int i_iHeight ) :
    QtPLDriver( i_iWidth, i_iHeight ),
    QImage( i_iWidth, i_iHeight, QImage::Format_ARGB32 )
{
    // Painter initialised in the constructor contrary
    // to buffered drivers, which paint only in doPlot().
    m_painterP = new QPainter( this );
    fill( Qt::transparent );

    QBrush b = m_painterP->brush();
    b.setStyle( Qt::SolidPattern );
    m_painterP->setBrush( b );
    m_painterP->setRenderHint( QPainter::Antialiasing, (bool) lines_aa );
}

QtRasterDevice::~QtRasterDevice()
{
    delete m_painterP;
}

void QtRasterDevice::definePlotName( const char* fileName, const char* format )
{
    // Avoid buffer overflows
    strncpy( this->format, format, 4 );
    this->format[4] = '\0';

    this->fileName = QString( fileName );
}

void QtRasterDevice::savePlot()
{
    m_painterP->end();
    if ( save( fileName, format, 85 ) )
    {
        m_painterP->begin( this );
        m_painterP->setRenderHint( QPainter::Antialiasing, (bool) lines_aa );
        QBrush b = m_painterP->brush();
        b.setStyle( Qt::SolidPattern );
        m_painterP->setBrush( b );
    }
    else
    {
        printf( "WARNING: %s raster format currently not supported by your Qt installation\n", format );
    }
}

void QtRasterDevice::setBackgroundColor( int r, int g, int b, double alpha )
{
    if ( !m_painterP->isActive() )
        return;


    QBrush brush( QColor( r, g, b, (int) ( alpha * 255 ) ) );
    m_painterP->fillRect( 0, 0, width(), height(), brush );
}
#endif

#if defined ( PLD_svgqt ) && QT_VERSION >= 0x040300
QtSVGDevice::QtSVGDevice( int i_iWidth, int i_iHeight ) :
    QtPLDriver( i_iWidth, i_iHeight )
{
    m_painterP = NULL;
}

QtSVGDevice::~QtSVGDevice()
{
    delete m_painterP;
}

void QtSVGDevice::definePlotName( const char* fileName )
{
    setFileName( QString( fileName ) );
    setResolution( POINTS_PER_INCH );
    setSize( QSize( (int) m_dWidth, (int) m_dHeight ) );
#if QT_VERSION >= 0x040500
    setViewBox( QRect( 0, 0, (int) m_dWidth, (int) m_dHeight ) );
#endif

    m_painterP = new QPainter( this );
}

void QtSVGDevice::savePlot()
{
    m_painterP->end();
}

void QtSVGDevice::setBackgroundColor( int r, int g, int b, double alpha )
{
    if ( !m_painterP->isActive() )
        return;

    QBrush brush( QColor( r, g, b, (int) ( alpha * 255 ) ) );
    m_painterP->fillRect( 0, 0, width(), height(), brush );
}
#endif

#if defined ( PLD_epsqt ) || defined ( PLD_pdfqt )
QtEPSDevice::QtEPSDevice( int i_iWidth, int i_iHeight )
{
#if QT_VERSION < 0x040400
    setPageSize( QPrinter::A4 );
#else
    setFullPage( true );
    setPaperSize( QSizeF( i_iHeight, i_iWidth ), QPrinter::Point );
#endif
    setResolution( POINTS_PER_INCH );
    setColorMode( QPrinter::Color );
    setOrientation( QPrinter::Landscape );
    setPrintProgram( QString( "lpr" ) );

    if ( i_iWidth <= 0 || i_iHeight <= 0 )
    {
        m_dWidth  = pageRect().width();
        m_dHeight = pageRect().height();
    }
    else
    {
        m_dWidth  = i_iWidth;
        m_dHeight = i_iHeight;
    }
    m_painterP = NULL;
}

QtEPSDevice::~QtEPSDevice()
{
    delete m_painterP;
}

void QtEPSDevice::definePlotName( const char* fileName, int ifeps )
{
    setOutputFileName( QString( fileName ) );
    if ( ifeps )
    {
#ifndef PLPLOT_USE_QT5
        setOutputFormat( QPrinter::PostScriptFormat );
#endif
    }
    else
    {
        setOutputFormat( QPrinter::PdfFormat );
    }

    m_painterP = new QPainter( this );
}

void QtEPSDevice::savePlot()
{
    m_painterP->end();
}

void QtEPSDevice::setBackgroundColor( int r, int g, int b, double alpha )
{
    if ( !m_painterP->isActive() )
        return;

    QBrush brush( QColor( r, g, b, (int) ( alpha * 255 ) ) );
    m_painterP->fillRect( 0, 0, width(), height(), brush );
}
#endif

#if defined ( PLD_qtwidget ) || defined ( PLD_extqt )
QtPLWidget::QtPLWidget( int i_iWidth, int i_iHeight, QWidget* parent ) :
    QWidget( parent ), QtPLDriver( i_iWidth, i_iHeight )
{
    m_painterP = new QPainter;

    m_dAspectRatio = m_dWidth / m_dHeight;

    m_pixPixmap = NULL;
//     m_iOldSize=0;
    pageNumber = 0;
    resize( i_iWidth, i_iHeight );
    lastColour.r = -1;
    setVisible( true );
    // Dropping this appears to give more reliable results
    // (QColor::setRgb: RGB parameters out of range warnings go away)
    // according to Jonathan Woithe <jwoithe@just42.net> and according
    // to my own tests does not affect results from the
    // test_interactive target.
    // QApplication::processEvents();
    redrawFromLastFlush = false;
    redrawAll           = true;

    NoPen = QPen( Qt::NoPen );
    NoPen.setWidthF( 0. );

    locate_mode = 0;
}

QtPLWidget::~QtPLWidget()
{
    clearBuffer();
    delete m_pixPixmap;
}

void QtPLWidget::clearWidget()
{
    clearBuffer();
    setBackgroundColor( bgColour.r, bgColour.g, bgColour.b, bgColour.alpha );
    redrawAll = true;
//     m_bAwaitingRedraw=true;
    update();
}

void QtPLWidget::flush()
{
    repaint();
    QApplication::processEvents();
}

void QtPLWidget::clearBuffer()
{
    lastColour.r = -1;
    for ( QLinkedList<BufferElement>::iterator i = m_listBuffer.begin(); i != m_listBuffer.end(); ++i )
    {
        switch ( i->Element )
        {
        case LINE:
            delete i->Data.Line;
            break;
        case RECTANGLE:
            delete i->Data.Rect;
            break;

        case POLYLINE:
        case POLYGON:
            delete i->Data.Polyline;
            break;

        case TEXT:
            delete[] i->Data.TextStruct->text;
            delete i->Data.TextStruct;
            break;

        case SET_COLOUR:
        case SET_BG_COLOUR:
            delete i->Data.ColourStruct;
            break;

        case SET_GRADIENT:
            delete i->Data.LinearGradient;
            break;

        case ARC:
            delete i->Data.ArcStruct->rect;
            delete i->Data.ArcStruct->dx;
            delete i->Data.ArcStruct;

        default:
            break;
        }
    }

    m_listBuffer.clear();
    start_iterator = m_listBuffer.constBegin();
    redrawAll      = true;
}


void QtPLWidget::drawArc( short x, short y, short a, short b, PLFLT angle1, PLFLT angle2, PLFLT rotate, bool fill )
{
    BufferElement el;
    el.Element              = ARC;
    el.Data.ArcStruct       = new struct ArcStruct_;
    el.Data.ArcStruct->rect = new QRectF( (PLFLT) ( x - a ) * downscale,
        m_dHeight - (PLFLT) ( y + b ) * downscale,
        (PLFLT) a * downscale * 2,
        (PLFLT) b * downscale * 2
        );
    el.Data.ArcStruct->startAngle = (int) ( angle1 * 16 );
    el.Data.ArcStruct->spanAngle  = (int) ( ( angle2 - angle1 ) * 16 );
    el.Data.ArcStruct->rotate     = rotate;
    el.Data.ArcStruct->dx         = new QPointF( (PLFLT) x * downscale, m_dHeight - (PLFLT) y * downscale );
    el.Data.ArcStruct->fill       = fill;

    m_listBuffer.append( el );
    redrawFromLastFlush = true;
}


void QtPLWidget::drawLine( short x1, short y1, short x2, short y2 )
{
    BufferElement el;
    el.Element   = LINE;
    el.Data.Line = new QLineF( QPointF( (PLFLT) x1 * downscale, (PLFLT) ( m_dHeight - y1 * downscale ) ), QPointF( (PLFLT) x2 * downscale, (PLFLT) ( m_dHeight - y2 * downscale ) ) );

    m_listBuffer.append( el );
    redrawFromLastFlush = true;
}

void QtPLWidget::drawPolyline( short * x, short * y, PLINT npts )
{
    BufferElement el;
    el.Element       = POLYLINE;
    el.Data.Polyline = new QPolygonF;
    for ( int i = 0; i < npts; ++i )
    {
        ( *el.Data.Polyline ) << QPointF( (PLFLT) ( x[i] ) * downscale, (PLFLT) ( m_dHeight - ( y[i] ) * downscale ) );
    }

    m_listBuffer.append( el );
    redrawFromLastFlush = true;
}

void QtPLWidget::drawPolygon( short * x, short * y, PLINT npts )
{
    BufferElement el;

    bool          isRect = false;
    if ( npts == 4 ) // Check if it's a rectangle. If so, it can be made faster to display
    {
        if ( x[0] == x[1] && x[2] == x[3] && y[0] == y[3] && y[1] == y[2] )
        {
            isRect = true;
        }
        else if ( x[0] == x[3] && x[1] == x[2] && y[0] == y[1] && y[2] == y[3] )
        {
            isRect = true;
        }
    }
    if ( npts == 5 )
    {
        if ( x[0] == x[4] && y[0] == y[4] )
        {
            if ( x[0] == x[1] && x[2] == x[3] && y[0] == y[3] && y[1] == y[2] )
            {
                isRect = true;
            }
            else if ( x[0] == x[3] && x[1] == x[2] && y[0] == y[1] && y[2] == y[3] )
            {
                isRect = true;
            }
        }
    }

    if ( isRect )
    {
        el.Element = RECTANGLE;

        double x1, y1, x2, y2, x0, y0, width, height;
        x1 = (PLFLT) ( x[0] ) * downscale;
        x2 = (PLFLT) ( x[2] ) * downscale;
        y1 = (PLFLT) ( m_dHeight - ( y[0] ) * downscale );
        y2 = (PLFLT) ( m_dHeight - ( y[2] ) * downscale );
        if ( x1 < x2 )
        {
            x0    = x1;
            width = x2 - x1;
        }
        else
        {
            x0    = x2;
            width = x1 - x2;
        }
        if ( y1 < y2 )
        {
            y0     = y1;
            height = y2 - y1;
        }
        else
        {
            y0     = y2;
            height = y1 - y2;
        }
        el.Data.Rect = new QRectF( x0, y0, width, height );
    }
    else
    {
        el.Element       = POLYGON;
        el.Data.Polyline = new QPolygonF;
        for ( int i = 0; i < npts; ++i )
        {
            ( *el.Data.Polyline ) << QPointF( (PLFLT) ( x[i] ) * downscale, (PLFLT) ( m_dHeight - ( y[i] ) * downscale ) );
        }
    }

    m_listBuffer.append( el );
    redrawFromLastFlush = true;
}

void QtPLWidget::setColor( int r, int g, int b, double alpha )
{
    if ( lastColour.r != r || lastColour.g != g || lastColour.b != b || lastColour.alpha != alpha )
    {
        BufferElement el;
        el.Element              = SET_COLOUR;
        el.Data.ColourStruct    = new struct ColourStruct_;
        el.Data.ColourStruct->R = r;
        el.Data.ColourStruct->G = g;
        el.Data.ColourStruct->B = b;
        el.Data.ColourStruct->A = (PLINT) ( alpha * 255. );

        m_listBuffer.append( el );

        lastColour.r     = r;
        lastColour.g     = g;
        lastColour.b     = b;
        lastColour.alpha = alpha;
    }
    // No need to ask for a redraw at this point. The color only affects subsequent items
//     redrawFromLastFlush=true;
}

void QtPLWidget::setGradient( int x1, int x2, int y1, int y2,
                              unsigned char *r, unsigned char *g,
                              unsigned char *b, PLFLT *alpha, PLINT ncol1 )
{
    int            i;
    BufferElement  el;
    qreal          stop_arg;
    QGradientStops stops;

    el.Element = SET_GRADIENT;

    el.Data.LinearGradient  = new QLinearGradient;
    *el.Data.LinearGradient = QLinearGradient(
        QPointF( (qreal) ( x1 * downscale ), (qreal) ( m_dHeight - y1 * downscale ) ),
        QPointF( (qreal) ( x2 * downscale ), (qreal) ( m_dHeight - y2 * downscale ) ) );
    for ( i = 0; i < ncol1; i++ )
    {
        stop_arg = (qreal) i / (qreal) ( ncol1 - 1 );
        stops << QGradientStop( stop_arg, QColor( r[i], g[i],
                b[i], (int) ( alpha[i] * 255 ) ) );
    }
    ( *el.Data.LinearGradient ).setStops( stops );
    m_listBuffer.append( el );

    // No need to ask for a redraw at this point. The gradient only
    // affects subsequent items.
    //redrawFromLastFlush=true;
}

void QtPLWidget::setBackgroundColor( int r, int g, int b, double alpha )
{
    BufferElement el;
    el.Element              = SET_BG_COLOUR;
    el.Data.ColourStruct    = new struct ColourStruct_;
    el.Data.ColourStruct->R = r;
    el.Data.ColourStruct->G = g;
    el.Data.ColourStruct->B = b;
    el.Data.ColourStruct->A = (PLINT) ( alpha * 255. );

    bgColour.r     = r;
    bgColour.g     = g;
    bgColour.b     = b;
    bgColour.alpha = alpha;
    if ( alpha >= 0.999 )
    {
        clearBuffer();
    }
    m_listBuffer.append( el );
    redrawFromLastFlush = true;
}

void QtPLWidget::setWidthF( PLFLT w )
{
    BufferElement el;
    el.Element       = SET_WIDTH;
    el.Data.fltParam = w;
    m_listBuffer.append( el );
//     redrawFromLastFlush=true;
}

void QtPLWidget::drawText( EscText* txt )
{
    if ( pls->get_string_length )
    {
        PLUNICODE fci;
        QPicture  picText;
        double    picDpi;
        PLUNICODE *text;

        plgfci( &fci );
        text = new PLUNICODE[txt->unicode_array_len];
        memcpy( text, txt->unicode_array, txt->unicode_array_len * sizeof ( PLUNICODE ) );
        picText = getTextPicture( fci,
            text,
            txt->unicode_array_len,
            pls->chrht );
        //
        // I'm assuming that y_fact is 1.0 here, as it is impossible
        // to know in advance (or so I believe). When the text is
        // rendered "for real" it will be: pls->chrht * y_fact.
        //
        // Hazen 6/2011
        //
        picDpi             = picText.logicalDpiY();
        pls->string_length = ( (PLFLT) xOffset / picDpi ) * 25.4;
        delete[] text;
        return;
    }

    BufferElement el;

    el.Element                   = TEXT;
    el.Data.TextStruct           = new struct TextStruct_;
    el.Data.TextStruct->x        = txt->x * downscale;
    el.Data.TextStruct->y        = m_dHeight - txt->y * downscale;
    el.Data.TextStruct->clipxmin = pls->clpxmi * downscale;
    el.Data.TextStruct->clipymin = m_dHeight - pls->clpymi * downscale;
    el.Data.TextStruct->clipxmax = pls->clpxma * downscale;
    el.Data.TextStruct->clipymax = m_dHeight - pls->clpyma * downscale;
    PLUNICODE fci;
    plgfci( &fci );
    el.Data.TextStruct->fci = fci;
    PLFLT rotation, shear, stride;
    plRotationShear( txt->xform, &rotation, &shear, &stride );
    rotation -= pls->diorot * M_PI / 2.0;
    el.Data.TextStruct->rotation = rotation;
    el.Data.TextStruct->shear    = shear;
    el.Data.TextStruct->stride   = stride;
    el.Data.TextStruct->just     = txt->just;
    el.Data.TextStruct->text     = new PLUNICODE[txt->unicode_array_len];
    memcpy( el.Data.TextStruct->text, txt->unicode_array, txt->unicode_array_len * sizeof ( PLUNICODE ) );
    el.Data.TextStruct->len   = txt->unicode_array_len;
    el.Data.TextStruct->chrht = pls->chrht;

    m_listBuffer.append( el );
    redrawFromLastFlush = true;
}

void QtPLWidget::renderText( QPainter* p, struct TextStruct_* s, double x_fact, double x_offset, double y_fact, double y_offset )
{
    if ( s->len <= 0 || s->len >= 500 )
        return;
    QPicture picText = getTextPicture( s->fci, s->text, s->len, s->chrht * y_fact );

    double   picDpi = picText.logicalDpiY();

    p->setClipping( true );
    p->setClipRect( QRectF( s->clipxmin * x_fact + x_offset, s->clipymax * y_fact + y_offset, ( s->clipxmax - s->clipxmin ) * x_fact, ( -s->clipymax + s->clipymin ) * y_fact ), Qt::ReplaceClip );
    p->translate( s->x * x_fact + x_offset, s->y * y_fact + y_offset );
    QMatrix rotShearMatrix( cos( s->rotation ) * s->stride, -sin( s->rotation ) * s->stride, cos( s->rotation ) * sin( s->shear ) + sin( s->rotation ) * cos( s->shear ), -sin( s->rotation ) * sin( s->shear ) + cos( s->rotation ) * cos( s->shear ), 0., 0. );
    p->setWorldMatrix( rotShearMatrix, true );

    p->translate( -s->just * xOffset * p->device()->logicalDpiY() / picDpi, 0. );

    p->drawPicture( 0, 0, picText );

    p->resetTransform();

    p->setClipping( false );
}

void QtPLWidget::resetPensAndBrushes( QPainter* painter )
{
    SolidPen = QPen();
    hasPen   = true;
    painter->setPen( SolidPen );
    SolidBrush = QBrush( Qt::SolidPattern );
}

void QtPLWidget::lookupButtonEvent( QMouseEvent * event )
{
    Qt::MouseButtons      buttons   = event->buttons();
    Qt::KeyboardModifiers modifiers = event->modifiers();
    gin.pX = event->x();
    gin.pY = height() - event->y();
    gin.dX = (PLFLT) event->x() / width();
    gin.dY = (PLFLT) ( height() - event->y() ) / height();

    switch ( event->button() )
    {
    case Qt::LeftButton:
        gin.button = 1;
        break;
    case Qt::MidButton:
        gin.button = 2;
        break;
    case Qt::RightButton:
        gin.button = 3;
        break;
    default:
        break;
    }

    // Map Qt button and key states to the (X windows) values used
    // by plplot.
    gin.state = 0;
    if ( buttons & Qt::LeftButton )
        gin.state |= 1 << 8;
    if ( buttons & Qt::MidButton )
        gin.state |= 1 << 9;
    if ( buttons & Qt::RightButton )
        gin.state |= 1 << 10;
    if ( modifiers & Qt::ShiftModifier )
        gin.state |= 1 << 0;
    if ( modifiers & Qt::ControlModifier )
        gin.state |= 1 << 2;
    if ( modifiers & Qt::AltModifier )
        gin.state |= 1 << 3;
    if ( modifiers & Qt::MetaModifier )
        gin.state |= 1 << 3;

    gin.keysym = 0x20;
}

void QtPLWidget::locate()
{
    if ( plTranslateCursor( &gin ) )
    {
        if ( locate_mode == 2 )
        {
            pltext();
            if ( gin.keysym < 0xFF && isprint( gin.keysym ) )
                std::cout << gin.wX << " " << gin.wY << " " << (char) gin.keysym << std::endl;
            else
                std::cout << gin.wX << " " << gin.wY << " " << std::hex << gin.keysym << std::endl;

            plgra();
        }
    }
    else
    {
        locate_mode = 0;
        QApplication::restoreOverrideCursor();
    }
}

void QtPLWidget::mouseEvent( QMouseEvent * event )
{
    lookupButtonEvent( event );

    if ( locate_mode )
    {
        if ( event->button() == Qt::LeftButton )
        {
            locate();
        }
    }
    else
    {
        if ( event->button() == Qt::RightButton )
        {
            handler.DeviceChangedPage( this );
        }
    }
}

void QtPLWidget::mousePressEvent( QMouseEvent * event )
{
    mouseEvent( event );
}

void QtPLWidget::mouseReleaseEvent( QMouseEvent * PL_UNUSED( event ) )
{
    //mouseEvent( event );
}

void QtPLWidget::mouseMoveEvent( QMouseEvent * PL_UNUSED( event ) )
{
    //mouseEvent( event );
}

void QtPLWidget::keyPressEvent( QKeyEvent* event )
{
    if ( locate_mode )
    {
        QPoint p = QCursor::pos();
        gin.pX = p.x();
        gin.pY = height() - p.y();
        gin.dX = (PLFLT) p.x() / width();
        gin.dY = (PLFLT) ( height() - p.y() ) / height();

        switch ( event->key() )
        {
        case Qt::Key_Escape:
            locate_mode = 0;
            QApplication::restoreOverrideCursor();
            plGinInit( &gin );
            break;
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Meta:
        case Qt::Key_AltGr:
            plGinInit( &gin );
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
        {
            int x1, y1, dx = 0, dy = 0;
            int xmin = 0, xmax = width() - 1, ymin = 0, ymax = height() - 1;
            switch ( event->key() )
            {
            case Qt::Key_Left:
                dx = -1;
                break;
            case Qt::Key_Right:
                dx = 1;
                break;
            case Qt::Key_Up:
                dy = -1;
                break;
            case Qt::Key_Down:
                dy = 1;
                break;
            }
            if ( event->modifiers() & Qt::ShiftModifier )
            {
                dx *= 5;
                dy *= 5;
            }
            if ( event->modifiers() & Qt::ControlModifier )
            {
                dx *= 5;
                dy *= 5;
            }
            if ( event->modifiers() & Qt::AltModifier )
            {
                dx *= 5;
                dy *= 5;
            }
            x1 = gin.pX + dx;
            y1 = gin.pY + dy;

            if ( x1 < xmin )
                dx = xmin - gin.pX;
            if ( y1 < ymin )
                dy = ymin - gin.pY;
            if ( x1 > xmax )
                dx = xmax - gin.pX;
            if ( y1 > ymax )
                dy = ymax - gin.pY;

            QCursor::setPos( p.x() + dx, p.y() + dy );
            plGinInit( &gin );
            break;
        }
        default:
            locate();
            break;
        }
    }
    else
    {
        if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
        {
            handler.DeviceChangedPage( this );
        }
        if ( event->text() == "Q" )
        {
            // Terminate on a 'Q' (not 'q', since it's too easy to hit by mistake)
            pls->nopause = TRUE;
            plexit( "" );
        }
        else if ( event->text() == "L" )
        // Begin locate mode
        {
            locate_mode = 2;
            QApplication::setOverrideCursor( Qt::CrossCursor );
        }
    }
}

void QtPLWidget::closeEvent( QCloseEvent* event )
{
    handler.DeviceClosed( this );
    event->ignore();
}

void QtPLWidget::nextPage()
{
    clearWidget();
    pageNumber++;
}

void QtPLWidget::resizeEvent( QResizeEvent * )
{
//     m_bAwaitingRedraw=true;
    redrawAll = true;
    delete m_pixPixmap;
    m_pixPixmap = NULL;
}

void QtPLWidget::paintEvent( QPaintEvent * )
{
    double x_fact, y_fact, x_offset( 0. ), y_offset( 0. ); //Parameters to scale and center the plot on the widget

    getPlotParameters( x_fact, y_fact, x_offset, y_offset );

    if ( redrawAll || m_pixPixmap == NULL )
    {
        if ( m_pixPixmap != NULL )
        {
            delete m_pixPixmap;
        }
        m_pixPixmap = new QPixmap( width(), height() );
        QPainter* painter = new QPainter;
        painter->begin( m_pixPixmap );

        // Draw the margins and the background
        painter->fillRect( 0, 0, width(), height(), QColor( bgColour.r, bgColour.g, bgColour.b ) );

        // Re-initialise pens etc.
        resetPensAndBrushes( painter );

        start_iterator = m_listBuffer.constBegin();

        // Draw the plot
        doPlot( painter, x_fact, y_fact, x_offset, y_offset );
        painter->end();

//             m_iOldSize=m_listBuffer.size();

        delete painter;
    }
    else
    {
        QPainter* painter = new QPainter;
        painter->begin( m_pixPixmap );
        if ( hasPen )
            painter->setPen( SolidPen );
        else
            painter->setPen( NoPen );

        // Draw the plot
        doPlot( painter, x_fact, y_fact, x_offset, y_offset );
        painter->end();
    }

    // draw the current pixmap
    m_painterP->begin( this );

    m_painterP->drawPixmap( 0, 0, *m_pixPixmap );

    m_painterP->end();
}

void QtPLWidget::doPlot( QPainter* p, double x_fact, double y_fact, double x_offset, double y_offset )
{
    QLineF         line;
    QVector<qreal> vect;
    QRectF         rect;


//     QPen SolidPen;
//
//     QPen NoPen(Qt::NoPen);
//     NoPen.setWidthF(0.); // Cosmetic pen
//     p->setPen(SolidPen);
//     bool hasPen=true;

    p->setRenderHints( QPainter::Antialiasing, (bool) lines_aa );

//     QBrush SolidBrush(Qt::SolidPattern);
    p->setBrush( SolidBrush );

    QTransform trans;
    trans = trans.translate( x_offset, y_offset );
    trans = trans.scale( x_fact, y_fact );

    p->setTransform( trans );

    if ( m_listBuffer.empty() )
    {
        p->fillRect( 0, 0, 1, 1, QBrush() );
        return;
    }

    // unrolls the buffer and draws each element accordingly
    for ( QLinkedList<BufferElement>::const_iterator i = start_iterator; i != m_listBuffer.constEnd(); ++i )
    {
        switch ( i->Element )
        {
        case SET_COLOUR:
            SolidPen.setColor( QColor( i->Data.ColourStruct->R, i->Data.ColourStruct->G, i->Data.ColourStruct->B, i->Data.ColourStruct->A ) );
            if ( hasPen )
            {
                p->setPen( SolidPen );
            }
            SolidBrush.setColor( QColor( i->Data.ColourStruct->R, i->Data.ColourStruct->G, i->Data.ColourStruct->B, i->Data.ColourStruct->A ) );
            p->setBrush( SolidBrush );
            break;

        case SET_GRADIENT:
            p->setBrush( *( i->Data.LinearGradient ) );
            break;

        case LINE:
            if ( !hasPen )
            {
                p->setPen( SolidPen );
                hasPen = true;
            }
            p->drawLine( *( i->Data.Line ) );

            break;

        case POLYLINE:
            if ( !hasPen )
            {
                p->setPen( SolidPen );
                hasPen = true;
            }
            p->drawPolyline( *( i->Data.Polyline ) );
            break;

        case RECTANGLE:
            p->setRenderHints( QPainter::Antialiasing, false );
            if ( hasPen )
            {
                p->setPen( NoPen );
                hasPen = false;
            }
            p->drawRect( *( i->Data.Rect ) );
            p->setRenderHints( QPainter::Antialiasing, (bool) lines_aa );
            break;

        case POLYGON:
            p->setRenderHints( QPainter::Antialiasing, false );
            if ( hasPen )
            {
                p->setPen( NoPen );
                hasPen = false;
            }
            if ( plsc->dev_eofill )
                p->drawPolygon( *( i->Data.Polyline ), Qt::OddEvenFill );
            else
                p->drawPolygon( *( i->Data.Polyline ), Qt::WindingFill );
            p->setRenderHints( QPainter::Antialiasing, (bool) lines_aa );
            break;

        case TEXT:
            if ( !hasPen )
            {
                p->setPen( SolidPen );
                hasPen = true;
            }
            p->save();
            p->resetTransform();

            renderText( p, i->Data.TextStruct, x_fact, x_offset, y_fact, y_offset );
            p->restore();
            break;

        case SET_WIDTH:
            SolidPen.setWidthF( i->Data.fltParam );
            if ( hasPen )
            {
                p->setPen( SolidPen );
            }
            break;

        case SET_BG_COLOUR:
            SolidBrush.setColor( QColor( i->Data.ColourStruct->R, i->Data.ColourStruct->G, i->Data.ColourStruct->B, i->Data.ColourStruct->A ) );
            p->fillRect( 0, 0, (int) m_dWidth, (int) m_dHeight, SolidBrush );
            break;

        case ARC:
            if ( !hasPen )
            {
                p->setPen( SolidPen );
                hasPen = true;
            }
            if ( i->Data.ArcStruct->rotate != 0.0 )
            {
                p->save();
                p->translate( *( i->Data.ArcStruct->dx ) );
                p->rotate( -i->Data.ArcStruct->rotate );
                p->translate( -*( i->Data.ArcStruct->dx ) );
            }

            if ( i->Data.ArcStruct->fill )
                p->drawPie( *( i->Data.ArcStruct->rect ), i->Data.ArcStruct->startAngle, i->Data.ArcStruct->spanAngle );
            else
                p->drawArc( *( i->Data.ArcStruct->rect ), i->Data.ArcStruct->startAngle, i->Data.ArcStruct->spanAngle );

            if ( i->Data.ArcStruct->rotate != 0.0 )
            {
                p->restore();
            }

            break;
        default:
            break;
        }
    }

    start_iterator = m_listBuffer.constEnd();
    --start_iterator;
    redrawFromLastFlush = false;
    redrawAll           = false;
}

void QtPLWidget::getPlotParameters( double & io_dXFact, double & io_dYFact, double & io_dXOffset, double & io_dYOffset )
{
    double w = (double) width();
    double h = (double) height();
    if ( w / h > m_dAspectRatio ) //Too wide, h is the limitating factor
    {
        io_dYFact   = h / m_dHeight;
        io_dXFact   = h * m_dAspectRatio / m_dWidth;
        io_dYOffset = 0.;
        io_dXOffset = ( w - io_dXFact * m_dWidth ) / 2.;
    }
    else
    {
        io_dXFact   = w / m_dWidth;
        io_dYFact   = w / m_dAspectRatio / m_dHeight;
        io_dXOffset = 0.;
        io_dYOffset = ( h - io_dYFact * m_dHeight ) / 2.;
    }
}

void QtPLWidget::getCursorCmd( PLGraphicsIn *ptr )
{
    plGinInit( &gin );

    locate_mode = 1;
    QApplication::setOverrideCursor( Qt::CrossCursor );

    while ( gin.pX < 0 && locate_mode )
        QCoreApplication::processEvents( QEventLoop::AllEvents, 10 );

    QApplication::restoreOverrideCursor();
    *ptr = gin;
}

#endif

#if defined ( PLD_extqt )
QtExtWidget::QtExtWidget( int i_iWidth, int i_iHeight, QWidget* parent ) :
    QtPLWidget( i_iWidth, i_iHeight, parent )
{
    cursorParameters.isTracking = false;
    cursorParameters.cursor_x   = -1.0;
    cursorParameters.cursor_y   = -1.0;
    killed = false;
}

QtExtWidget::~QtExtWidget()
{
    killed = true;
    QCoreApplication::processEvents( QEventLoop::AllEvents, 10 );
    delete m_pixPixmap;
    delete m_painterP;
    m_pixPixmap = NULL;
    m_painterP  = NULL;
}

void QtExtWidget::captureMousePlotCoords( PLFLT* x, PLFLT* y )
{
    setMouseTracking( true );
    cursorParameters.isTracking   = true;
    cursorParameters.cursor_x     =
        cursorParameters.cursor_y = -1.;
    do
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 10 );
    } while ( cursorParameters.isTracking && !killed );

    *x = cursorParameters.cursor_x;
    *y = cursorParameters.cursor_y;
}

void QtExtWidget::mouseMoveEvent( QMouseEvent* event )
{
    if ( !cursorParameters.isTracking )
        return;

    double x_fact, y_fact, x_offset, y_offset; //Parameters to scale and center the plot on the widget

    getPlotParameters( x_fact, y_fact, x_offset, y_offset );

    cursorParameters.cursor_x = (PLFLT) event->x();
    cursorParameters.cursor_y = (PLFLT) event->y();

    double ratio_x;
    double ratio_y;
    ratio_x = ( cursorParameters.cursor_x - x_offset ) / ( width() - 2. * x_offset );
    ratio_y = ( cursorParameters.cursor_y - y_offset ) / ( height() - 2. * y_offset );

    PLFLT a, b;
    PLINT c;
    plcalc_world( ratio_x, 1. - ratio_y, &a, &b, &c );

    if ( c < 0 )
    {
        cursorParameters.cursor_x = -1.;
        cursorParameters.cursor_y = -1.;
    }

    update();
}

void QtExtWidget::mousePressEvent( QMouseEvent*  /* event */ )
{
}

void QtExtWidget::mouseReleaseEvent( QMouseEvent* event )
{
    if ( !cursorParameters.isTracking )
        return;

    double x_fact, y_fact, x_offset, y_offset; //Parameters to scale and center the plot on the widget

    getPlotParameters( x_fact, y_fact, x_offset, y_offset );

    cursorParameters.cursor_x   = (PLFLT) event->x();
    cursorParameters.cursor_y   = (PLFLT) event->y();
    cursorParameters.isTracking = false;
    setMouseTracking( false );

    double ratio_x;
    double ratio_y;
    ratio_x = ( cursorParameters.cursor_x - x_offset ) / ( width() - 2. * x_offset );
    ratio_y = ( cursorParameters.cursor_y - y_offset ) / ( height() - 2. * y_offset );

    PLFLT a, b;
    PLINT c;
    plcalc_world( ratio_x, 1. - ratio_y, &a, &b, &c );

    if ( c < 0 )
    {
        cursorParameters.cursor_x = -1.;
        cursorParameters.cursor_y = -1.;
    }
    else
    {
        cursorParameters.cursor_x = a;
        cursorParameters.cursor_y = b;
    }

    update();
}

void QtExtWidget::paintEvent( QPaintEvent* event )
{
    QtPLWidget::paintEvent( event );

    if ( !cursorParameters.isTracking || cursorParameters.cursor_x < 0 )
        return;

    QPainter p( this );

    p.setPen( QPen( Qt::white ) );

    p.drawLine( (int) cursorParameters.cursor_x, 0, (int) cursorParameters.cursor_x, height() );
    p.drawLine( 0, (int) cursorParameters.cursor_y, width(), (int) cursorParameters.cursor_y );

    p.end();
}

void plsetqtdev( QtExtWidget* widget )
{
    plsc->dev = (void *) widget;
    widget->setPLStream( plsc );
}

void plsetqtdev( QtExtWidget* widget, int argc, char** argv )
{
    plparseopts( &argc, argv, PL_PARSE_FULL );
    plsc->dev = (void *) widget;
    widget->setPLStream( plsc );
}

void plfreeqtdev()
{
    delete ( (QtExtWidget *) plsc->dev );
    plsc->dev = NULL;
}
#endif
