//
//
// This software is provided under the LGPL in March 2009 by the
// Cluster Science Centre
// QSAS team,
// Imperial College, London
//
// Copyright (C) 2009 Imperial College, London
// Copyright (C) 2009-2019 Alan W. Irwin
// Copyright (C) 2009 Andrew Ross
// Copyright (C) 2010 Hezekiah M. Carty

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
//

#include "qt_PlotWindow.h"

PlotWindow::PlotWindow( int argc, char** argv, QWidget* parent ) :
    QMainWindow( parent )
{
    // According to
    // <https://stackoverflow.com/questions/3153155/when-setting-the-wa-deleteonclose-attribute-on-a-qt-mainwindow-the-program-cras>
    // "Remember that your main window destructor should run only
    // once. That is to say that it should run either because of a stack
    // unwind, or because of WA_DeleteOnClose, not both."
    // Since qt_example uses the stack unwind method we comment out
    // setting the WA_DeleteOnClose attribute below to avoid double frees.
    // setAttribute( Qt::WA_DeleteOnClose );

    QMenu * plotMenu = menuBar()->addMenu( "Plot" );
    plotMenu->addAction( "Curves", this, SLOT( plotCurves() ) );
    plotMenu->addAction( "Histogram", this, SLOT( plotHistogram() ) );
    plotMenu->addAction( "Interactive Selection", this, SLOT( interactive() ) );

    plot = new QtExtWidget( QT_DEFAULT_X, QT_DEFAULT_Y, this );
    setCentralWidget( plot );

    plsdev( "extqt" );

    // Get all the application arguments as argc, argv

    if ( argc == 0 || argv == NULL )
    {
        plsetqtdev( plot );
    }
    else
    {
        plsetqtdev( plot, argc, argv );
    }
    plinit();

    resize( 600, 600 );

    pladv( 0 );
}

PlotWindow::~PlotWindow()
{
    plend();
    plfreeqtdev();
}


void PlotWindow::plotCurves()
{
    plot->clearWidget();

    PLFLT sine[360];
    PLFLT cosine[360];
    PLFLT indexes[360];
    PLFLT square[360];
    PLFLT cubic[360];

    // 1st plot

    for ( int i = 0; i < 360; ++i )
    {
        sine[i]    = sin( (PLFLT) i / 180. * M_PI );
        cosine[i]  = cos( (PLFLT) i / 180. * M_PI );
        indexes[i] = (PLFLT) i;
    }

    pladv( 0 );
    plvpor( 0.05, 0.95, 0.05, 0.45 );
    plwind( 0., 360., -1.2, 1.2 );

    plcol0( 2 );
    plbox( "bcnst", 0., 0, "bcnst", 0., 0 );

    plcol0( 1 );
    plwidth( 2 );
    plline( 360, indexes, sine );

    plcol0( 3 );
    plwidth( 1 );
    pllsty( 2 );
    plline( 360, indexes, cosine );
    pllsty( 1 );

    plcol0( 2 );

    plmtex( "t", 1., 0.5, 0.5, "Sines" );

    // 2nd plot

    for ( int i = -180; i < 180; ++i )
    {
        square[i + 180]  = ( ( (PLFLT) i ) / 180. * ( (PLFLT) i ) / 180. );
        cubic[i + 180]   = square[i + 180] * ( (PLFLT) i ) / 180.;
        indexes[i + 180] = ( (PLFLT) i ) / 180.;
    }

    plvpor( 0.05, 0.95, 0.55, 0.95 );
    plwind( -1., 1., -1., 1. );

    plcol0( 2 );
    plbox( "bcnst", 0., 0, "bcnst", 0., 0 );

    plcol0( 1 );
    plwidth( 2 );
    plline( 360, indexes, square );

    plcol0( 3 );
    plwidth( 1 );
    pllsty( 2 );
    plline( 360, indexes, cubic );
    pllsty( 1 );

    plcol0( 2 );

    plmtex( "t", 1., 0.5, 0.5, "Square & Cubic" );
}

void
plfbox( PLFLT x0, PLFLT y0 )
{
    PLFLT x[4], y[4];

    x[0] = x0;
    y[0] = 0.;
    x[1] = x0;
    y[1] = y0;
    x[2] = x0 + 1.;
    y[2] = y0;
    x[3] = x0 + 1.;
    y[3] = 0.;
    plfill( 4, x, y );
    plcol0( 1 );
    pllsty( 1 );
    plline( 4, x, y );
}

void PlotWindow::plotHistogram()
{
    PLFLT        y0[10];
    int          i;
    static PLFLT pos[]   = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    static PLFLT red[]   = { 0.0, 0.25, 0.5, 1.0, 1.0 };
    static PLFLT green[] = { 1.0, 0.5, 0.5, 0.5, 1.0 };
    static PLFLT blue[]  = { 1.0, 1.0, 0.5, 0.25, 0.0 };
    char         string[20];

    plot->clearWidget();

    pladv( 0 );
    plvsta();
    plcol0( 2 );
    plwind( 1980.0, 1990.0, 0.0, 35.0 );
    plbox( "bc", 1.0, 0, "bcnv", 10.0, 0 );
    plcol0( 2 );
    pllab( "Year", "Widget Sales (millions)", "#frPLplot Example 12" );

    y0[0] = 5;
    y0[1] = 15;
    y0[2] = 12;
    y0[3] = 24;
    y0[4] = 28;
    y0[5] = 30;
    y0[6] = 20;
    y0[7] = 8;
    y0[8] = 12;
    y0[9] = 3;

    plscmap1l( 1, 5, pos, red, green, blue, NULL );

    for ( i = 0; i < 10; i++ )
    {
        //plcol0(i + 1);
        plcol1( i / 9.0 );
        plpsty( 0 );
        plfbox( ( 1980. + i ), y0[i] );
        sprintf( string, "%.0f", y0[i] );
        plptex( ( 1980. + i + .5 ), ( y0[i] + 1. ), 1.0, 0.0, .5, string );
        sprintf( string, "%d", 1980 + i );
        plmtex( "b", 1.0, ( ( i + 1 ) * .1 - .05 ), 0.5, string );
    }
}

void PlotWindow::interactive()
{
    PLFLT x, y;
    plot->captureMousePlotCoords( &x, &y );

    char buf[100];
    sprintf( buf, "Selection: (%f, %f)", x, y );

    QMessageBox msgBox;
    msgBox.setText( buf );
    msgBox.exec();
}

