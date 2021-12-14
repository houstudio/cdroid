//
//
// This software is provided under the LGPL in March 2009 by the
// Cluster Science Centre
// QSAS team,
// Imperial College, London
//
// Copyright (C) 2009  Imperial College, London
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

int main( int argc, char** argv )
{
    int res;

    // Command-line options are only to be interpreted by PLplot.  Thus,
    // make a deep copy of the arguments for PLplot use before QApplication
    // has a chance to alter them.
    int Argc = argc;
    char** Argv;

    Argv = new char*[argc];
    for ( int i = 0; i < Argc; ++i )
    {
        int len = strlen( argv[i] ) + 1;
        Argv[i] = new char[len];
        strncpy( Argv[i], argv[i], len );
    }

    // Limit QApplication's interpretation of argv to just the first
    // argument (the application name) so that all command-line
    // options such as the PLplot -bg command-line option (which would
    // generate a warning message when misinterpreted by QApplication)
    // are completely ignored.
    argc = 1;
    QApplication a( argc, argv );
    // Must construct an instance of PlotWindow after QApplication.
    PlotWindow   * win = new PlotWindow( Argc, Argv );

//#define PLPLOT_QT_EXAMPLE_EXPERIMENTAL
#undef PLPLOT_QT_EXAMPLE_EXPERIMENTAL
#ifdef PLPLOT_QT_EXAMPLE_EXPERIMENTAL
    // Temporary change so that can test PlotWindow constructor and
    // destructor with valgrind for simpler case where qApp
    // has not used PlotWindow.
    res = 0;
#else
    a.setActiveWindow( win );
    win->setVisible( true );

    res = a.exec();
#endif
    // Clean up.

    // According to QApplication documentation at
    // <http://doc.qt.io/qt-5/qapplication.html#exec> there is no
    // guarantee on some platforms (they use Windows as an example)
    // that code executed after the exec method is executed will ever
    // be executed.  As a result they recommend connecting "clean-up
    // code to the aboutToQuit() signal, instead of putting it in your
    // application's main() function." But for now we will stick with
    // this cleanup method which does work properly on Linux according
    // to the printf statement below.

    for ( int i = 0; i < Argc; ++i )
    {
        delete[] Argv[i];
    }
    delete[] Argv;

    delete win;

    printf( "%s\n", "Completed cleanup of qt_example" );

    return res;
}
