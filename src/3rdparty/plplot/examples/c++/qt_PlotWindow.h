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

#ifndef QT01_PLOTWINDOW_H
#define QT01_PLOTWINDOW_H

#define PLD_qtwidget

#include <QApplication>
#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include "qt.h"

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// Constructor for the window, holding the widget parameters
    /// \param width Window width (independant of the plot size)
    /// \param height Window height
    /// \param saveWidth Plot actual width. Used to compute the aspect ratio on screen and the size in PS and SVG outputs \see resolution
    /// \param saveHeight Plot actual height.
    /// \param resolution Print resolution
    /// \param pageSize Print size
    /// \param orientation Print orientation
    /// \param parent Parent widget, if any.
    PlotWindow( int argc = 0, char** argv = NULL, QWidget* parent = NULL );

    ~PlotWindow();


public slots:


    void plotCurves();

    void plotHistogram();

    void interactive();


protected:

    PLINT       strm;

    QtExtWidget * plot;
};

#endif
