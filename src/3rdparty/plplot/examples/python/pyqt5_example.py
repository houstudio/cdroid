#!/usr/bin/env python
#
# A simple PLplot/PyQt example
#
# Copyright (C) 2016 Hazen Babcock
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

import sys
import numpy

import plplot_python_start
import plplot

# An indirect side effect of this import statement is a consistent set
# of Qt4 libraries are loaded (e.g., if those are from some downloaded
# non-system version in a non-standard location).
import plplot_pyqt5

# This import statement must go after the one above to insure a
# consistent set of Qt4 libraries are used for the case of the
# downloaded, non-standard location of those libraries.
from PyQt5 import QtCore, QtGui, QtWidgets


class QPlot(QtWidgets.QMainWindow):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self, None)

        # UI setup
        self.curvesAct = QtWidgets.QAction("Curves", self)
        self.histogramAct = QtWidgets.QAction("Histogram", self)
        self.interactiveAct = QtWidgets.QAction("Interactive Selection", self)
        self.plotMenu = self.menuBar().addMenu("Plot")
        self.plotMenu.addAction(self.curvesAct)
        self.plotMenu.addAction(self.histogramAct)
        self.plotMenu.addAction(self.interactiveAct)
        self.curvesAct.triggered.connect(self.plotCurves)
        self.histogramAct.triggered.connect(self.plotHistogram)
        self.interactiveAct.triggered.connect(self.interactive)

        # PLplot setup
        self.plot = plplot_pyqt5.QtExtWidget(842, 595, self)
        self.setCentralWidget(self.plot)
        plplot_pyqt5.plsetqtdev(self.plot)
        plplot.plsdev("extqt")
        plplot.plinit()

        self.resize(600,600)
        plplot.pladv(0)

    def cleanup(self):
        plplot.plend()
        plplot_pyqt5.plfreeqtdev()

    def paintEvent(self, event):
#        plplot.pladv(0)
#        plplot.plenv(0, 10, 0, 10, 0, 0)
#        plplot.pllab("X", "Y", "Y vs X")
        self.plot.show()

    def plotCurves(self):
        self.plot.clearWidget()

        # 1st plot
        indexes = numpy.arange(0, 360.1, 1.0)
        sine = numpy.sin(indexes * 3.14159 / 180.0)
        cosine = numpy.cos(indexes * 3.14159 / 180.0)

        plplot.pladv(0)
        plplot.plvpor(0.05, 0.95, 0.05, 0.45)
        plplot.plwind(0.0, 360.0, -1.2, 1.2)

        plplot.plcol0(2)
        plplot.plbox("bcnst", 0., 0, "bcnst", 0., 0)

        plplot.plcol0(1)
        plplot.plwidth(2)
        plplot.plline(indexes, sine)

        plplot.plcol0(3)
        plplot.plwidth(1)
        plplot.pllsty(2)
        plplot.plline(indexes, cosine)
        plplot.pllsty(1)

        plplot.plcol0(2)

        plplot.plmtex("t", 1., 0.5, 0.5, "Sines")

        # 2nd plot
        indexes = numpy.arange(-1.0, 1.0, 0.01)
        square = indexes * indexes
        cubic = square * indexes

        plplot.plvpor(0.05, 0.95, 0.55, 0.95)
        plplot.plwind(-1., 1., -1., 1.)

        plplot.plcol0(2)
        plplot.plbox("bcnst", 0., 0, "bcnst", 0., 0)

        plplot.plcol0(1)
        plplot.plwidth(2)
        plplot.plline(indexes, square)

        plplot.plcol0(3)
        plplot.plwidth(1)
        plplot.pllsty(2)
        plplot.plline(indexes, cubic)
        plplot.pllsty(1)

        plplot.plcol0(2)

        plplot.plmtex("t", 1., 0.5, 0.5, "Square & Cubic")

        self.update()

    def plotHistogram(self):

        def plfbox(x0, y0):
            x = numpy.array([x0, x0, x0 + 1.0, x0 + 1.0])
            y = numpy.array([0.0, y0, y0, 0.0])
            plplot.plfill(x, y)
            plplot.plcol0(1)
            plplot.pllsty(1)
            plplot.plline(x, y)

        self.plot.clearWidget();

        y0 = numpy.array([5, 15, 12, 24, 28, 30, 20, 8, 12, 3])
        pos = numpy.array([0.0, 0.25, 0.5, 0.75, 1.0])
        red = numpy.array([0.0, 0.25, 0.5, 1.0, 1.0])
        green = numpy.array([1.0, 0.5, 0.5, 0.5, 1.0])
        blue = numpy.array([1.0, 1.0, 0.5, 0.25, 0.0])

        plplot.pladv(0)
        plplot.plvsta()

        plplot.plcol0(2)

        plplot.plwind(1980.0, 1990.0, 0.0, 35.0)
        plplot.plbox("bc", 1.0, 0, "bcnv", 10.0, 0)
        plplot.plcol0(2)
        plplot.pllab("Year", "Widget Sales (millions)", "#frPLplot Example 12")

        plplot.plscmap1l(1,pos,red,green,blue)

        for i in range(10):
            plplot.plcol1(i/9.0)
            plplot.plpsty(0)
            plfbox((1980. + i), y0[i])
            string = "%.0f" % (y0[i])
            plplot.plptex((1980. + i + .5), (y0[i] + 1.), 1.0, 0.0, .5, string)
            string = "%d" % (1980 + i)
            plplot.plmtex("b", 1.0, ((i + 1) * .1 - .05), 0.5, string)

        self.update()

    def interactive(self):
        [mx, my] = self.plot.captureMousePlotCoords()
        msg_box = QtWidgets.QMessageBox()
        msg_box.setText("Selection: (%f, %f)" % (mx, my))
        msg_box.exec_()


app = QtWidgets.QApplication(sys.argv)
plot = QPlot()
plot.show()
app.exec_()
plot.cleanup()

