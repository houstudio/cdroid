/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __KPLOTVIEW_H__
#define __KPLOTVIEW_H__

#include <view/view.h>
#include <list>
#include <vector>

namespace cdroid{

class PlotAxis;
class PlotObject;
class PlotPoint;

/**
 *@class PlotView
 *
 *@short Generic data plotting widget.
 *
 *Widget for drawing plots. The basic idea behind PlotView is that
 *you don't have to worry about any transformation from your data's
 *natural units to screen pixel coordinates; this is handled internally
 *by the widget.
 *
 *Data to be plotted are represented by one or more instances of
 *PlotObject.  PlotObject contains a list of PointFs to be plotted
 *(again, in the data's natural units), as well as information about how
 *the data are to be rendered in the plot (i.e., as separate points or
 *connected by lines?  With what color and point style? etc).  See
 *PlotObject for more information.
 *
 *PlotView automatically adds axis labels with tickmarks and tick
 *labels.  These are encapsulated in the PlotAxis class.  All you have
 *to do is set the limits of the plotting area in data units, and
 *PlotView will figure out the optimal positions and labels for the
 *tickmarks on the axes.
 *
 *Example of usage:
 *
 * @code
PlotView *kpw = new PlotView( parent );
// setting our limits for the plot
kpw->setLimits( 1.0, 5.0, 1.0, 25.0 );

// creating a plot object whose points are connected by red lines ...
PlotObject *kpo = new PlotObject( Qt::red, PlotObject::Lines );
// ... adding some points to it ...
for ( float x = 1.0; x <= 5.0; x += 0.1 )
    kpo->addPoint( x, x*x );

// ... and adding the object to the plot widget
kpw->addPlotObject( kpo );
 * @endcode
 *
 *@note PlotView will take ownership of the objects added to it, so when
 *clearing the objects list (eg with removeAllPlotObjects()) any previous
 *reference to a PlotObject already added to a PlotView will be invalid.
 *You can disable this behavior by using setAutoDelete(false).
 *
 *@author Jason Harris
 */
class PlotView : public cdroid::View
{
public:
    /**
     *@short Constructor.
     *@param parent the parent widget
     */
    PlotView(int w,int h);
    explicit PlotView(cdroid::Context*,const cdroid::AttributeSet&atts);

    /**
     *@short Destructor.
     */
    ~PlotView() override;

    /**
     * The four types of plot axes.
     */
    enum Axis {
        LeftAxis = 0, ///< the left axis
        BottomAxis, ///< the bottom axis
        RightAxis, ///< the right axis
        TopAxis, ///< the top axis
    };

    /**
     *@return suggested minimum size for the plot widget
     */
    //QSize minimumSizeHint() const override;

    /**
     *@return suggested size for the plot widget
     */
    //QSize sizeHint() const override;

    /**
     * Set new data limits for the plot.
     * @param x1 the minimum X value in data units
     * @param x2 the maximum X value in data units
     * @param y1 the minimum Y value in data units
     * @param y2 the maximum Y value in data units
     */
    void setLimits(double x1, double x2, double y1, double y2);

    /**
     * @short Reset the secondary data limits, which control the
     * values displayed along the top and right axes.
     *
     * All data points are *plotted* using the coordinates
     * defined by setLimits(), so this function is only useful for
     * showing alternate tickmark labels along the top and right
     * edges.  For example, if you were plotting temperature on the
     * X-axis, you could use Centigrade units for the primary
     * (bottom) axis, using setLimits( 0.0, 100.0, 0.0, 1.0 ).  If
     * you also wanted to show Fahrenheit units along the secondary
     * (top) axis, you would additionally use
     * setSecondaryLimits( 32.0, 212.0, 0.0, 1.0 ).  The data
     * added to the plot would have x-coordinates in Centigrade degrees.
     *
     * @param x1 the minimum X value in secondary data units
     * @param x2 the maximum X value in secondary data units
     * @param y1 the minimum Y value in secondary data units
     * @param y2 the maximum Y value in secondary data units
     * @sa setLimits()
     */
    void setSecondaryLimits(double x1, double x2, double y1, double y2);

    /**
     * Unset the secondary limits, so the top and right axes
     * show the same tickmarks as the bottom and left axes (no tickmark
     * labels will be drawn for the top and right axes in this case)
     */
    void clearSecondaryLimits();

    /**
     * @return the rectangle representing the boundaries of the current plot,
     * in natural data units.
     * @sa setLimits()
     */
    RectF dataRect() const;

    /**
     * @return the rectangle representing the boundaries of the secondary
     * data limits, if they have been set.  Otherwise, this function
     * behaves the same as dataRect().
     * @sa setSecondaryLimits()
     */
    RectF secondaryDataRect() const;

    /**
     * @return the rectangle representing the boundaries of the current plot,
     * in screen pixel units.
     */
    Rect pixRect() const;

    /**
     * Add an item to the list of PlotObjects to be plotted.
     * The widget takes ownership of the plot object, unless auto-delete was disabled.
     * @param object the PlotObject to be added
     */
    void addPlotObject(PlotObject *object);

    /**
     * Add more than one PlotObject at one time.
     * The widget takes ownership of the plot object, unless auto-delete was disabled.
     * @param objects the list of PlotObjects to be added
     */
    void addPlotObjects(const std::list<PlotObject *> &objects);

    /**
     * @return the current list of plot objects
     */
    std::vector<PlotObject *> plotObjects() const;

    /**
     * Enables auto-deletion of plot objects if autoDelete is true; otherwise auto-deletion is disabled.
     * Auto-deletion is enabled by default.
     * @since 5.12
     */
    void setAutoDeletePlotObjects(bool autoDelete);

    /**
     * Removes all plot objects that were added to the widget.
     * If auto-delete was not disabled, the plot objects are deleted.
     */
    void removeAllPlotObjects();

    /**
     * Reset the mask used for non-overlapping labels so that all
     * regions of the plot area are considered empty.
     */
    void resetPlotMask();

    /**
     * Clear the object list, reset the data limits, and remove axis labels
     * If auto-delete was not disabled, the plot objects are deleted.
     */
    void resetPlot();

    /**
     * Replace an item in the PlotObject list.
     * @param i the index of the item to be replaced
     * @param o pointer to the replacement PlotObject
     *
     * @since 5.12, if auto-deletion is enabled, the previous plot object is deleted.
     * Call setAutoDeletePlotObjects(false) if you want to swap between available plot objects
     * and therefore you want to handle deletion externally.
     */
    void replacePlotObject(int i, PlotObject *o);

    /**
     * @return the background color of the plot.
     *
     * The default color is black.
     */
    uint32_t backgroundColor() const;

    /**
     * @return the foreground color, used for axes, tickmarks and associated
     * labels.
     *
     * The default color is white.
     */
    uint32_t foregroundColor() const;

    /**
     * @return the grid color.
     *
     * The default color is gray.
     */
    uint32_t gridColor() const;

    /**
     * Set the background color
     * @param bg the new background color
     */
    void setBackgroundColor(const uint32_t bg);

    /**
     * Set the foreground color
     * @param fg the new foreground color
     */
    void setForegroundColor(const uint32_t fg);

    /**
     * Set the grid color
     * @param gc the new grid color
     */
    void setGridColor(const uint32_t &gc);

    /**
     * @return whether the grid lines are shown
     * Grid lines are not shown by default.
     */
    bool isGridShown() const;

    /**
     * @return whether the tooltip for the point objects is shown.
     * Tooltips are enabled by default.
     */
    bool isObjectToolTipShown() const;

    /**
     * @return whether the antialiasing is active
     * Antialiasing is not active by default.
     */
    bool antialiasing() const;

    /**
     * Toggle antialiased drawing.
     * @param b if true, the plot graphics will be antialiased.
     */
    void setAntialiasing(bool b);

    /**
     * @return the number of pixels to the left of the plot area.
     *
     * Padding values are set to -1 by default; if unchanged, this
     * function will try to guess a good value, based on whether
     * ticklabels and/or axis labels need to be drawn.
     */
    int leftPadding() const;

    /**
     * @return the number of pixels to the right of the plot area.
     * Padding values are set to -1 by default; if unchanged, this
     * function will try to guess a good value, based on whether
     * ticklabels and/or axis labels are to be drawn.
     */
    int rightPadding() const;

    /**
     * @return the number of pixels above the plot area.
     * Padding values are set to -1 by default; if unchanged, this
     * function will try to guess a good value, based on whether
     * ticklabels and/or axis labels are to be drawn.
     */
    int topPadding() const;

    /**
     * @return the number of pixels below the plot area.
     * Padding values are set to -1 by default; if unchanged, this
     * function will try to guess a good value, based on whether
     * ticklabels and/or axis labels are to be drawn.
     */
    int bottomPadding() const;

    /**
     * @short Set the number of pixels to the left of the plot area.
     * Set this to -1 to revert to automatic determination of padding values.
     */
    void setLeftPadding(int padding);

    /**
     * @short Set the number of pixels to the right of the plot area.
     * Set this to -1 to revert to automatic determination of padding values.
     */
    void setRightPadding(int padding);

    /**
     * @short Set the number of pixels above the plot area.
     * Set this to -1 to revert to automatic determination of padding values.
     */
    void setTopPadding(int padding);

    /**
     * @short Set the number of pixels below the plot area.
     * Set this to -1 to revert to automatic determination of padding values.
     */
    void setBottomPadding(int padding);

    /**
     * @short Revert all four padding values to -1, so that they will be
     * automatically determined.
     */
    void setDefaultPaddings();

    /**
     * @short Map a coordinate @param p from the data rect to the physical
     * pixel rect.
     * Used mainly when drawing.
     * @param p the point to be converted, in natural data units
     * @return the coordinate in the pixel coordinate system
     */
    PointF mapToWidget(const PointF &p) const;

    /**
     * Indicate that object labels should try to avoid the given
     * rectangle in the plot.  The rectangle is in pixel coordinates.
     *
     * @note You should not normally call this function directly.
     * It is called by PlotObject when points, bars and labels are drawn.
     * @param r the rectangle defining the region in the plot that
     * text labels should avoid (in pixel coordinates)
     * @param value Allows you to determine how strongly the rectangle
     * should be avoided.  Larger values are avoided more strongly.
     */
    void maskRect(const RectF &r, float value = 1.0f);

    /**
     * Indicate that object labels should try to avoid the line
     * joining the two given points (in pixel coordinates).
     *
     * @note You should not normally call this function directly.
     * It is called by PlotObject when lines are drawn in the plot.
     * @param p1 the starting point for the line
     * @param p2 the ending point for the line
     * @param value Allows you to determine how strongly the line
     * should be avoided.  Larger values are avoided more strongly.
     */
    void maskAlongLine(const PointF &p1, const PointF &p2, float value = 1.0f);

    /**
     * Place an object label optimally in the plot.  This function will
     * attempt to place the label as close as it can to the point to which
     * the label belongs, while avoiding overlap with regions of the plot
     * that have been masked.
     *
     * @note You should not normally call this function directly.
     * It is called internally in PlotObject::draw().
     *
     * @param painter Pointer to the painter on which to draw the label
     * @param pp pointer to the PlotPoint whose label is to be drawn.
     */
    void placeLabel(cdroid::Canvas&, PlotPoint *pp);

    /**
     * @return the axis of the specified @p type, or 0 if no axis has been set.
     * @sa Axis
     */
    PlotAxis *axis(Axis type);

    /**
     * @return the axis of the specified @p type, or 0 if no axis has been set.
     * @sa Axis
     */
    const PlotAxis *axis(Axis type) const;

public:// Q_SLOTS:
    /**
     * Toggle whether grid lines are drawn at major tickmarks.
     * @param show if true, grid lines will be drawn.
     * @sa isGridShown()
     */
    void setShowGrid(bool show);

    /**
     * Toggle the display of a tooltip for point objects.
     * @param show whether show the tooltip.
     * @sa isObjectToolTipShown()
     */
    void setObjectToolTipShown(bool show);

protected:
    /**
     * Generic event handler.
     */
    //bool event(QEvent *) override;

    /**
     * The paint event handler, executed when update() or repaint() is called.
     */
    void onDraw(cdroid::Canvas&) override;

    /**
     * The resize event handler, called when the widget is resized.
     */
    void onSizeChanged(int,int,int,int) override;

    /**
     * Draws the plot axes and axis labels.
     * @internal Internal use only; one should simply call update()
     * to draw the widget with axes and all objects.
     * @param p pointer to the painter on which we are drawing
     */
    virtual void drawAxes(cdroid::Canvas&);

    /**
     * Synchronize the PixRect with the current widget size and
     * padding settings.
     */
    void setPixRect();

    /**
     * @return a list of points in the plot which are within 4 pixels
     * of the screen position given as an argument.
     * @param p The screen position from which to check for plot points.
     */
    std::list<PlotPoint *> pointsUnderPoint(const Point &p) const;

private:
    class Private;
    Private *const d;
};
}//endof namespace
#endif
