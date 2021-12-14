/* July 2, 2009

   PyQt4 wrapper file for the PLplot QtExtWidget driver.

   Copyright (C) 2009 Hazen Babcock

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   PLplot is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with PLplot; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

*/

%Module(name = plplot_pyqt4, version = 0)

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class QtPLDriver {

%TypeHeaderCode
#include "qt.h"
%End

  public:
    QtPLDriver(int i_iWidth=QT_DEFAULT_X, int i_iHeight=QT_DEFAULT_Y);
    virtual ~QtPLDriver();
};

class QtPLWidget: QWidget, QtPLDriver {

%TypeHeaderCode
#include "qt.h"
%End

  public:
    QtPLWidget(int i_iWidth=QT_DEFAULT_X, int i_iHeight=QT_DEFAULT_Y, QWidget* parent=0);
    virtual ~QtPLWidget();

    void clearWidget();

  protected:
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
    void nextPage();
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void getPlotParameters(double &io_dXFact, double &io_dYFact, double &io_dXOffset, double &io_dYOffset);
};

class QtExtWidget: QtPLWidget {

%TypeHeaderCode
#include "qt.h"
%End

  public:
    QtExtWidget(int i_iWidth=QT_DEFAULT_X, int i_iHeight=QT_DEFAULT_Y, QWidget* parent=0);
    virtual ~QtExtWidget();

    void captureMousePlotCoords(double *x, double *y);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

  protected:
    void paintEvent(QPaintEvent *event);
};

void plsetqtdev(QtExtWidget* widget);
void plfreeqtdev();
