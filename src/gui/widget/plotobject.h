/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KPLOTOBJECT_H__
#define __KPLOTOBJECT_H__

#include <core/rect.h>
#include <core/canvas.h>
#include <view/view.h>
#include <string>
#include <list>

namespace cdroid{

using cdroid::PointF;
using Cairo::RefPtr;
using Cairo::Pattern;
class PlotView;
class PlotPoint;

/**
 * @class PlotObject
 * @short Encapsulates a data set to be plotted in a PlotView.
 *
 * Think of a PlotObject as a set of data displayed as a group in the plot.
 * Each PlotObject consists of a list of PlotPoints, a "type" controlling
 * how the data points are displayed (some combination of Points, Lines, or
 * Bars), a color, and a size. There is also a parameter which controls the
 * shape of the points used to display the PlotObject.
 *
 * @note PlotObject will take care of the points added to it, so when clearing
 * the points list (eg with clearPoints()) any previous reference to a PlotPoint
 * already added to a PlotObject will be invalid.
 *
 * @author Jason Harris
 * @version 1.1
 */
class PlotObject
{
public:
    /**
     * The type classification of the PlotObject.
     *
     * These are bitmask values that can be OR'd together, so that a set
     * of points can be represented in the plot in multiple ways.
     *
     * @note points should be added in order of increasing x-coordinate
     * when using Bars.
     */
    enum PlotType {
        UnknownType = 0,
        Points = 1, ///< each PlotPoint is represented with a drawn point
        Lines = 2, ///< each PlotPoint is connected with a line
        Bars = 4, ///< each PlotPoint is shown as a vertical bar
    };
    typedef PlotType PlotTypes;//Q_DECLARE_FLAGS(PlotTypes, PlotType)

    /**
     * The available shape styles for plotted points.
     */
    enum PointStyle {
        NoPoints = 0,
        Circle = 1,
        Letter = 2,
        Triangle = 3,
        Square = 4,
        Pentagon = 5,
        Hexagon = 6,
        Asterisk = 7,
        Star = 8,
        UnknownPoint,
    };

    /**
     * Constructor.
     * @param color The color for plotting this object. By default this sets
     * the color for Points, Lines and Bars, but there are functions to
     * override any of these.
     * @param otype the PlotType for this object (Points, Lines or Bars)
     * @param size the size to use for plotted points, in pixels
     * @param ps The PointStyle describing the shape for plotted points
     */
    explicit PlotObject(const uint32_t &color = 0xFFFFFFFF, PlotType otype = Points, double size = 2.0, PointStyle ps = Circle);

    /**
     * Destructor.
     */
    ~PlotObject();

    /**
     * @return the plot flags of the object
     */
    PlotTypes plotTypes() const;

    /**
     * Set whether points will be drawn for this object
     * @param b if true, points will be drawn
     */
    void setShowPoints(bool b);

    /**
     * Set whether lines will be drawn for this object
     * @param b if true, lines will be drawn
     */
    void setShowLines(bool b);

    /**
     * Set whether bars will be drawn for this object
     * @param b if true, bars will be drawn
     */
    void setShowBars(bool b);

    /**
     * @return the size of the plotted points in this object, in pixels
     */
    double size() const;

    /**
     * Set the size for plotted points in this object, in pixels
     * @param s the new size
     */
    void setSize(double s);

    /**
     * @return the style used for drawing the points in this object
     */
    PointStyle pointStyle() const;

    /**
     * Set a new style for drawing the points in this object
     * @param p the new style
     */
    void setPointStyle(PointStyle p);

    /**
     * @return the default pen for this Object.
     * If no other pens are set, this pen will be used for
     * points, lines, bars and labels (this pen is always used for points).
     */
    const RefPtr<Pattern> &pen() const;

    /**
     * Set the default pen for this object
     * @p The pen to use
     */
    void setPen(const RefPtr<Pattern> &p);

    /**
     * @return the pen to use for drawing lines for this Object.
     */
    const RefPtr<Pattern> &linePen() const;

    /**
     * Set the pen to use for drawing lines for this object
     * @p The pen to use
     */
    void setLinePen(const RefPtr<Pattern> &p);

    /**
     * @return the pen to use for drawing bars for this Object.
     */
    const RefPtr<Pattern> &barPen() const;

    /**
     * Set the pen to use for drawing bars for this object
     * @p The pen to use
     */
    void setBarPen(const RefPtr<Pattern> &p);

    /**
     * @return the pen to use for drawing labels for this Object.
     */
    const RefPtr<Pattern> &labelPen() const;

    /**
     * Set the pen to use for labels for this object
     * @p The pen to use
     */
    void setLabelPen(const RefPtr<Pattern> &p);

    /**
     * @return the default Brush to use for this Object.
     */
    const RefPtr<Pattern> brush() const;

    /**
     * Set the default brush to use for this object
     * @b The brush to use
     */
    void setBrush(const RefPtr<Pattern> &b);

    /**
     * @return the brush to use for filling bars for this Object.
     */
    const RefPtr<Pattern> barBrush() const;

    /**
     * Set the brush to use for drawing bars for this object
     * @b The brush to use
     */
    void setBarBrush(const RefPtr<Pattern> &b);

    /**
     * @return the list of PlotPoints that make up this object
     */
    std::list<PlotPoint *> points() const;

    /**
     * Add a point to the object's list of points, using input data to construct a PlotPoint.
     * @param p the QPointF to add.
     * @param label the optional text label for this point
     * @param barWidth the width of the bar, if this object is to be drawn with bars
     * @note if @param barWidth is left at its default value of 0.0, then the width will be
     * automatically set to the distance between this point and the one to its right.
     */
    void addPoint(const PointF &p, const std::string &label = std::string(), double barWidth = 0.0);

    /**
     * Add a given PlotPoint to the object's list of points.
     * @overload
     * @param p pointer to the PlotPoint to add.
     */
    void addPoint(PlotPoint *p);

    /**
     * Add a point to the object's list of points, using input data to construct a PlotPoint.
     * @overload
     * @param x the X-coordinate of the point to add.
     * @param y the Y-coordinate of the point to add.
     * @param label the optional text label
     * @param barWidth the width of the bar, if this object is to be drawn with bars
     * @note if @param barWidth is left at its default value of 0.0, then the width will be
     * automatically set to the distance between this point and the one to its right.
     */
    void addPoint(double x, double y, const std::string &label = std::string(), double barWidth = 0.0);

    /**
     * Remove the QPointF at position index from the list of points
     * @param index the index of the point to be removed.
     */
    void removePoint(int index);

    /**
     * Remove and destroy the points of this object
     */
    void clearPoints();

    /**
     * Draw this PlotObject on the given QPainter
     * @param p The QPainter to draw on
     * @param pw the PlotView to draw on (this is needed
     * for the PlotView::mapToWidget() function)
     */
    void draw(cdroid::Canvas&,PlotView*);
private:
    class Private;
    Private *const d;
};
}//endof namespace
#endif
