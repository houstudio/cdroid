/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <widget/plotview.h>

#include <math.h>
#include <widget/plotaxis.h>
#include <widget/plotobject.h>
#include <widget/plotpoint.h>
#include <cdtypes.h>
#include <cdlog.h>

#define XPADDING 20
#define YPADDING 20
#define BIGTICKSIZE 10
#define SMALLTICKSIZE 4
#define TICKOFFSET 0

//kf5plotting: https://invent.kde.org/frameworks/kplotting
namespace cdroid{

DECLARE_WIDGET(PlotView)

class PlotView::Private
{
public:
    Private(PlotView *qq)
        : q(qq)
        , cForeground(cdroid::Color::WHITE)
        , cGrid(cdroid::Color::GRAY)
        , showGrid(false)
        , showObjectToolTip(true)
        , useAntialias(false)
        , autoDelete(true)
    {
        // create the axes and setting their default properties
        PlotAxis *leftAxis = new PlotAxis();
        leftAxis->setTickLabelsShown(true);
        axes.insert({LeftAxis, leftAxis});
        PlotAxis *bottomAxis = new PlotAxis();
        bottomAxis->setTickLabelsShown(true);
        axes.insert({BottomAxis, bottomAxis});
        PlotAxis *rightAxis = new PlotAxis();
        axes.insert({RightAxis, rightAxis});
        PlotAxis *topAxis = new PlotAxis();
        axes.insert({TopAxis, topAxis});
        tickLabelSize = 16;
    }

    ~Private()
    {
        if (autoDelete) {
           for(auto o:objectList)delete o;
        }
        for(auto x:axes)delete x.second;
    }

    PlotView *q;

    void calcDataRectLimits(double x1, double x2, double y1, double y2);
    /**
     * @return a value indicating how well the given rectangle is
     * avoiding masked regions in the plot.  A higher returned value
     * indicates that the rectangle is intersecting a larger portion
     * of the masked region, or a portion of the masked region which
     * is weighted higher.
     * @param r The rectangle to be tested
     */
    float rectCost(const RectF &r) const;

    // Colors
    uint32_t cForeground, cGrid;
    // draw options
    bool showGrid;
    bool showObjectToolTip;
    bool useAntialias;
    bool autoDelete;
    int tickLabelSize;
    // padding
    // hashmap with the axes we have
    std::unordered_map<Axis, PlotAxis *> axes;
    // List of PlotObjects
    std::vector<PlotObject *> objectList;
    // Limits of the plot area in data units
    RectF dataRect, secondDataRect;
    // Limits of the plot area in pixel units
    Rect pixRect;
    // Array holding the mask of "used" regions of the plot
    Cairo::RefPtr<Cairo::ImageSurface> plotMask;
};

PlotView::PlotView(int w,int h):View(w,h),d(new Private(this)){
    d->secondDataRect.set(0,0,0,0); // default: no secondary data rect
    // sets the default limits
    d->calcDataRectLimits(0.0, 1.0, 0.0, 1.0);
    d->pixRect.set(0,0,getWidth(),getHeight());
}

PlotView::PlotView(cdroid::Context*ctx,const cdroid::AttributeSet&atts)
    : View(ctx,atts) , d(new Private(this))
{
    d->secondDataRect.set(0,0,0,0); // default: no secondary data rect
    // sets the default limits
    d->calcDataRectLimits(0.0, 1.0, 0.0, 1.0);
    const std::unordered_map<std::string,int>dirs={{"left",1},{"top",2},{"right",4},{"bottom",8},
	    {"horizontal",5},{"vertical",10},{"all",15}};
    d->showGrid = atts.getBoolean("showGrid",false);
    d->cGrid = atts.getInt("gridColor",d->cGrid);
    d->tickLabelSize = atts.getDimensionPixelSize("tickLabelSize",d->tickLabelSize);

    const int tickMarks = atts.getInt("tickMarks",dirs,15);
    const int tickLabels= atts.getInt("tickLabels",dirs,0);

    axis(LeftAxis)->setTickmarkVisible(tickMarks&1);
    axis(TopAxis)->setTickmarkVisible(tickMarks&2);
    axis(RightAxis)->setTickmarkVisible(tickMarks&4);
    axis(BottomAxis)->setTickmarkVisible(tickMarks&8);

    axis(LeftAxis)->setTickLabelsShown(tickLabels&1);
    axis(TopAxis)->setTickLabelsShown(tickLabels&2);
    axis(RightAxis)->setTickLabelsShown(tickLabels&4);
    axis(BottomAxis)->setTickLabelsShown(tickLabels&8);
    //d->tickMarkTextColor= atts.getInt("tickMarkColor",d->tickMarkTextColor);
}

PlotView::~PlotView()
{
    delete d;
}

/*QSize PlotView::minimumSizeHint() const
{
    return QSize(150, 150);
}

QSize PlotView::sizeHint() const
{
    return size();
}*/

void PlotView::setLimits(double x1, double x2, double y1, double y2)
{
    d->calcDataRectLimits(x1, x2, y1, y2);
    invalidate();
}

void PlotView::Private::calcDataRectLimits(double x1, double x2, double y1, double y2)
{
    double XA1;
    double XA2;
    double YA1;
    double YA2;
    if (x2 < x1) {
        XA1 = x2;
        XA2 = x1;
    } else {
        XA1 = x1;
        XA2 = x2;
    }
    if (y2 < y1) {
        YA1 = y2;
        YA2 = y1;
    } else {
        YA1 = y1;
        YA2 = y2;
    }

    if (XA2 == XA1) {
        LOG(WARN) << "x1 and x2 cannot be equal. Setting x2 = x1 + 1.0";
        XA2 = XA1 + 1.0;
    }
    if (YA2 == YA1) {
        LOG(WARN) << "y1 and y2 cannot be equal. Setting y2 = y1 + 1.0";
        YA2 = YA1 + 1.0;
    }
    dataRect = RectF::Make(XA1, YA1, XA2 - XA1, YA2 - YA1);

    q->axis(LeftAxis)->setTickMarks(dataRect.top, dataRect.height);
    q->axis(BottomAxis)->setTickMarks(dataRect.left, dataRect.width);

    if (secondDataRect.empty()){//isNull()) {
        q->axis(RightAxis)->setTickMarks(dataRect.top, dataRect.height);
        q->axis(TopAxis)->setTickMarks(dataRect.left, dataRect.width);
    }
}

void PlotView::setSecondaryLimits(double x1, double x2, double y1, double y2)
{
    double XA1;
    double XA2;
    double YA1;
    double YA2;
    if (x2 < x1) {
        XA1 = x2;
        XA2 = x1;
    } else {
        XA1 = x1;
        XA2 = x2;
    }
    if (y2 < y1) {
        YA1 = y2;
        YA2 = y1;
    } else {
        YA1 = y1;
        YA2 = y2;
    }

    if (XA2 == XA1) {
        LOG(WARN) << "x1 and x2 cannot be equal. Setting x2 = x1 + 1.0";
        XA2 = XA1 + 1.0;
    }
    if (YA2 == YA1) {
        LOG(WARN) << "y1 and y2 cannot be equal. Setting y2 = y1 + 1.0";
        YA2 = YA1 + 1.0;
    }
    d->secondDataRect = RectF::Make(XA1, YA1, XA2 - XA1, YA2 - YA1);

    axis(RightAxis)->setTickMarks(d->secondDataRect.top, d->secondDataRect.height);
    axis(TopAxis)->setTickMarks(d->secondDataRect.left, d->secondDataRect.width);

    invalidate();
}

void PlotView::clearSecondaryLimits(){
    d->secondDataRect.set(0,0,0,0);
    axis(RightAxis)->setTickMarks(d->dataRect.top, d->dataRect.height);
    axis(TopAxis)->setTickMarks(d->dataRect.left, d->dataRect.width);

    invalidate();
}

RectF PlotView::dataRect() const{
    return d->dataRect;
}

RectF PlotView::secondaryDataRect() const{
    return d->secondDataRect;
}

void PlotView::addPlotObject(PlotObject *object){
    // skip null pointers
    if (!object) {
        return;
    }
    d->objectList.push_back(object);
    invalidate();
}

void PlotView::addPlotObjects(const std::list<PlotObject *> &objects){
    bool addedsome = false;
    for (PlotObject *o : objects) {
        if (!o) {
            continue;
        }

        d->objectList.push_back(o);
        addedsome = true;
    }
    if (addedsome) {
        invalidate();
    }
}

std::vector<PlotObject *> PlotView::plotObjects() const{
    return d->objectList;
}

void PlotView::setAutoDeletePlotObjects(bool autoDelete){
    d->autoDelete = autoDelete;
}

void PlotView::removeAllPlotObjects(){
    if (d->objectList.empty()) {
        return;
    }

    if (d->autoDelete) {
        for(auto o:d->objectList)delete o;
    }
    d->objectList.clear();
    invalidate();
}

void PlotView::resetPlotMask(){
    Rect rc = pixRect();
    if(rc.empty())return;
    d->plotMask = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,rc.width,rc.height);//QImage(pixRect().size(), QImage::Format_ARGB32);
    Cairo::RefPtr<Cairo::Context>ctx=Cairo::Context::create(d->plotMask);
    ctx->set_source_rgba(0,0,0,0.5);
    ctx->rectangle(0,0,rc.width,rc.height);
    ctx->set_operator(Cairo::Context::Operator::SOURCE);
    ctx->fill();
}

void PlotView::resetPlot(){
    if (d->autoDelete) {
        for(auto o:d->objectList)delete o;
    }
    d->objectList.clear();
    clearSecondaryLimits();
    d->calcDataRectLimits(0.0, 1.0, 0.0, 1.0);
    PlotAxis *a = axis(RightAxis);
    a->setLabel(std::string());
    a->setTickLabelsShown(false);
    a = axis(TopAxis);
    a->setLabel(std::string());
    a->setTickLabelsShown(false);
    axis(PlotView::LeftAxis)->setLabel(std::string());
    axis(PlotView::BottomAxis)->setLabel(std::string());
    resetPlotMask();
}

void PlotView::replacePlotObject(int i, PlotObject *o){
    // skip null pointers and invalid indexes
    if ((o==nullptr) || (i < 0) || (i >= d->objectList.size())) {
        return;
    }
    if (d->objectList.at(i) == o) {
        return;
    }
    if (d->autoDelete) {
        delete d->objectList.at(i);
    }
    d->objectList[i] = o;
    invalidate();
}

uint32_t PlotView::gridColor() const{
    return d->cGrid;
}

void PlotView::setGridColor(const uint32_t &gc){
    d->cGrid = gc;
    invalidate();
}


uint32_t PlotView::foregroundColor()const{
    return d->cForeground;
}

void PlotView::setForegroundColor(const uint32_t fg){
    d->cForeground = fg;
    invalidate();
}

bool PlotView::isGridShown() const{
    return d->showGrid;
}

bool PlotView::isObjectToolTipShown() const{
    return d->showObjectToolTip;
}

bool PlotView::antialiasing() const{
    return d->useAntialias;
}

void PlotView::setAntialiasing(bool b){
    d->useAntialias = b;
    invalidate();
}

void PlotView::setShowGrid(bool show){
    d->showGrid = show;
    invalidate();
}

void PlotView::setObjectToolTipShown(bool show){
    d->showObjectToolTip = show;
}

PlotAxis *PlotView::axis(Axis type){
    auto it = d->axes.find(type);
    return it != d->axes.end() ? it->second : nullptr;
}

const PlotAxis *PlotView::axis(Axis type) const{
    auto it = d->axes.find(type);
    return it != d->axes.end() ? it->second : nullptr;
}

Rect PlotView::pixRect() const{
    return d->pixRect;
}

std::list<PlotPoint *> PlotView::pointsUnderPoint(const Point &p) const
{
    std::list<PlotPoint *> pts;
    for (const PlotObject *po : d->objectList) {
        const auto pointsList = po->points();
        for (PlotPoint *pp : pointsList) {
#if 0
            if ((p - mapToWidget(pp->position()).toPoint()).manhattanLength() <= 4) {
                pts.push_back(pp);//pts << pp;
            }
#else
	    PointF p1 = mapToWidget(pp->position());
	    if(std::abs(float(p.x)-p1.x)<2||std::abs(float(p.x)-p1.y)<2)
		pts.push_back(pp);
#endif
        }
    }

    return pts;
}

/*bool PlotView::event(QEvent *e){
    if (e->type() == QEvent::ToolTip) {
        if (d->showObjectToolTip) {
            QHelpEvent *he = static_cast<QHelpEvent *>(e);
            QList<PlotPoint *> pts = pointsUnderPoint(he->pos() - Point(leftPadding(), topPadding()) - contentsRect().topLeft());
            if (!pts.isEmpty()) {
                QToolTip::showText(he->globalPos(), pts.front()->label(), this);
            }
        }
        e->accept();
        return true;
    } else {
        return QFrame::event(e);
    }
}*/

void PlotView::onSizeChanged(int w,int h,int oldw,int oldh){
    View::onSizeChanged(w,h,oldw,oldh);
    setPixRect();
    resetPlotMask();
}

void PlotView::setPixRect(){
    const int newWidth = /*contentsRect().width*/getWidth() - getPaddingLeft() - getPaddingRight();
    const int newHeight= /*contentsRect().height*/getHeight()- getPaddingTop() - getPaddingBottom();
    // PixRect starts at (0,0) because we will translate by leftPadding(), topPadding()
    d->pixRect.set(0,0, newWidth, newHeight);
}

PointF PlotView::mapToWidget(const PointF &p) const{
    const float px = d->pixRect.left + float(d->pixRect.width) * (p.x - d->dataRect.left) / d->dataRect.width;
    const float py = d->pixRect.top + float(d->pixRect.height) * (d->dataRect.top + d->dataRect.height -p.y) / d->dataRect.height;
    return PointF{px, py};
}

void PlotView::maskRect(const RectF &rf, float fvalue){
    const int value = int(fvalue);
    Rect r;//= rf.toRect().intersected(d->pixRect);
    r.set(rf.left,rf.top,rf.width,rf.height);
    r.intersect(d->pixRect);
    uint8_t*pixels = (uint8_t*)d->plotMask->get_data();
    const uint32_t stride = d->plotMask->get_stride();
    for (int ix = r.left; ix < r.right(); ++ix) {
        for (int iy = r.top; iy < r.bottom(); ++iy) {
	    uint8_t*pixel = pixels+(iy*stride+ix*4);
	    pixel[0]=200;
	    pixel[1]=std::min(pixel[1]+value,255);
            //newColor = uint32_t(d->plotMask.pixel(ix, iy));
            //newColor.setAlpha(200);
            //newColor.setRed(qMin(newColor.red() + value, 255));
            //d->plotMask.setPixel(ix, iy, newColor.rg
        }
    }
}

void PlotView::maskAlongLine(const PointF &p1, const PointF &p2, float fvalue){
    if (!d->pixRect.contains({int(p1.x),int(p1.y)}) && !d->pixRect.contains({int(p2.x),int(p2.y)})) {
        return;
    }

    const int value = int(fvalue);

    // Determine slope and zeropoint of line
    double m = (p2.y - p1.y) / (p2.x - p1.x);
    double y0 = p1.y - m * p1.x;

    uint8_t*pixels = (uint8_t*)d->plotMask->get_data();
    const uint32_t stride = d->plotMask->get_stride();
    // Mask each pixel along the line joining p1 and p2
    if (m > 1.0 || m < -1.0) { // step in y-direction
        int y1 = int(p1.y);
        int y2 = int(p2.y);
        if (y1 > y2) {
            y1 = int(p2.y);
            y2 = int(p1.y);
        }
        for (int y = y1; y <= y2; ++y) {
            int x = int((y - y0) / m);
            if (d->pixRect.contains(x, y)) {
                uint8_t*pixel = pixels+(y*stride+x*4);
                pixel[0]=100;
                pixel[1]=std::min(pixel[1]+value,255);
                //uint32_t newColor = uint32_t(d->plotMask.pixel(x, y));
                //newColor.setAlpha(uint8_t(100));
                //newColor.setRed(std::min(newColor.red() + value, 255)));
                //d->plotMask.setPixel(x, y, newColor);
            }
        }

    } else { // step in x-direction
        int x1 = int(p1.x);
        int x2 = int(p2.x);
        if (x1 > x2) {
            x1 = int(p2.x);
            x2 = int(p1.x);
        }

        for (int x = x1; x <= x2; ++x) {
            int y = int(y0 + m * x);
            if (d->pixRect.contains(x, y)) {
                uint8_t*pixel = pixels+(y*stride+x*4);
                pixel[0]=100;
                pixel[1]=std::min(pixel[1]+value,255);
                uint32_t newColor = 0;//uint32_t(d->plotMask.pixel(x, y));
                //newColor.setAlpha(uint8_t(100));
                //newColor.setRed(uint8_t(std::min(newColor.red() + value, 255)));
                //d->plotMask.setPixel(x, y, newColor.rgba());
            }
        }
    }
}

static void drawRound(Canvas&canvas,const RectF&r,float radii) {
    float pts[8];
    double radian = M_PI;
    pts[0] = r.left + radii;
    pts[1] = r.top + radii;
    pts[2] = r.right() - radii;
    pts[3] = r.top + radii;
    pts[4] = r.right() - radii;
    pts[5] = r.bottom()- radii;
    pts[6] = r.left + radii;
    pts[7] = r.bottom()- radii;
    canvas.begin_new_sub_path();
    for(int i = 0,j = 0; i < 8; i += 2,j++) {
       canvas.arc(pts[i],pts[i+1],radii,radian,radian+M_PI/2.0);
        radian += M_PI/2.0;
    }
    canvas.close_path();
}

// Determine optimal placement for a text label for point pp.  We want
// the label to be near point pp, but we don't want it to overlap with
// other labels or plot elements.  We will use a "downhill simplex"
// algorithm to find a label position that minimizes the pixel values
// in the plotMask image over the label's rect().  The sum of pixel
// values in the label's rect is the "cost" of placing the label there.
//
// Because a downhill simplex follows the local gradient to find low
// values, it can get stuck in local minima.  To mitigate this, we will
// iteratively attempt each of the initial path offset directions (up,
// down, right, left) in the order of increasing cost at each location.
void PlotView::placeLabel(cdroid::Canvas&painter, PlotPoint *pp){
    const int textFlags = Gravity::CENTER;//Qt::TextSingleLine | Qt::AlignCenter;

    PointF pos = mapToWidget(pp->position());
    if (!d->pixRect.contains(int(pos.x),int(pos.y))) {
        return;
    }

    Cairo::TextExtents ext;
    //QFontMetricsF fm(painter->font(), painter->device());
    RectF bestRect;
    painter.get_text_extents(pp->label(),ext);
    bestRect.set(int(pos.x),int(pos.y),int(ext.width),int(ext.height)) ;
    float xStep = 0.5 * bestRect.width;
    float yStep = 0.5 * bestRect.height;
    float maxCost = 0.05 * bestRect.width * bestRect.height;
    float bestCost = d->rectCost(bestRect);

    // We will travel along a path defined by the maximum decrease in
    // the cost at each step.  If this path takes us to a local minimum
    // whose cost exceeds maxCost, then we will restart at the
    // beginning and select the next-best path.  The indices of
    // already-tried paths are stored in the TriedPathIndex list.
    //
    // If we try all four first-step paths and still don't get below
    // maxCost, then we'll adopt the local minimum position with the
    // best cost (designated as bestBadCost).
    int iter = 0;
    std::vector<int> TriedPathIndex;
    float bestBadCost = 10000;
    RectF bestBadRect;

    // needed to halt iteration from inside the switch
    bool flagStop = false;

    while (bestCost > maxCost) {
        // Displace the label up, down, left, right; determine which
        // step provides the lowest cost
	RectF bestRectF={bestRect.left,bestRect.top,bestRect.width,bestRect.height};	
        RectF upRect = bestRectF;
        upRect.top += yStep;//upRect.moveTop(upRect.top + yStep);
        float upCost = d->rectCost(upRect);
        RectF downRect = bestRectF;
        downRect.top  -= yStep;//downRect.moveTop(downRect.top - yStep);
        float downCost = d->rectCost(downRect);
        RectF leftRect = bestRectF;
        leftRect.left -= xStep;//leftRect.moveLeft(leftRect.left - xStep);
        float leftCost = d->rectCost(leftRect);
        RectF rightRect = bestRectF;
        rightRect.left += xStep;//rightRect.moveLeft(rightRect.left + xStep);
        float rightCost = d->rectCost(rightRect);

        // which direction leads to the lowest cost?
	std::vector<float> costList={upCost,downCost,leftCost,rightCost};
        int imin = -1;
        for (int i = 0; i < costList.size(); ++i) {
	    auto it=std::find(TriedPathIndex.begin(),TriedPathIndex.end(),i);
            if (iter == 0 && it!=TriedPathIndex.end()){//TriedPathIndex.contains(i)) {
                continue; // Skip this first-step path, we already tried it!
            }

            // If this first-step path doesn't improve the cost,
            // skip this direction from now on
            if (iter == 0 && costList[i] >= bestCost) {
                TriedPathIndex.push_back(i);//append(i);
                continue;
            }

            if (costList[i] < bestCost && (imin < 0 || costList[i] < costList[imin])) {
                imin = i;
            }
        }

        // Make a note that we've tried the current first-step path
        if (iter == 0 && imin >= 0) {
            TriedPathIndex.push_back(imin);
        }

        // Adopt the step that produced the best cost
        switch (imin) {
        case 0: // up
            bestRect.top = upRect.top;//moveTop(upRect.top);
            bestCost = upCost;
            break;
        case 1: // down
            bestRect.top = downRect.top;//moveTop(downRect.top);
            bestCost = downCost;
            break;
        case 2: // left
            bestRect.left = leftRect.left;//moveLeft(leftRect.left);
            bestCost = leftCost;
            break;
        case 3: // right
            bestRect.left = rightRect.left;//moveLeft(rightRect.left);
            bestCost = rightCost;
            break;
        case -1: // no lower cost found!
            // We hit a local minimum.  Keep the best of these as bestBadRect
            if (bestCost < bestBadCost) {
                bestBadCost = bestCost;
                bestBadRect.set(bestRect.left,bestRect.top,bestRect.width,bestRect.height);// = bestRect;
            }

            // If all of the first-step paths have now been searched, we'll
            // have to adopt the bestBadRect
            if (TriedPathIndex.size() == 4) {
                bestRect.set(bestBadRect.left,bestBadRect.top,bestBadRect.width,bestBadRect.height);// = bestBadRect;
                flagStop = true; // halt iteration
                break;
            }

            // If we haven't yet tried all of the first-step paths, start over
            if (TriedPathIndex.size() < 4) {
                iter = -1; // anticipating the ++iter below
                //bestRect = fm.boundingRect(QRectF(pos.x, pos.y, 1, 1), textFlags, pp->label());
		bestRect.set(int(pos.x),int(pos.y), pp->label().size()*32,32);
                bestCost = d->rectCost(RectF::Make(bestRect.left,bestRect.top,bestRect.width,bestRect.height));
            }
            break;
        }

        // Halt iteration, because we've tried all directions and
        // haven't gotten below maxCost (we'll adopt the best
        // local minimum found)
        if (flagStop) {
            break;
        }

        ++iter;
    }

    Rect rctxt = Rect::Make(int(bestRect.left),int(bestRect.top),int(bestRect.width),int(bestRect.height));
    painter.draw_text(rctxt/*bestRect*/,pp->label(), textFlags);

    // Is a line needed to connect the label to the point?
    float deltax = pos.x - bestRect.centerX();
    float deltay = pos.y - bestRect.centerY();
    float rbest = sqrt(deltax * deltax + deltay * deltay);
    if (rbest > 20.0) {
        // Draw a rectangle around the label
        //painter.setBrush(QBrush());
        // QPen pen = painter->pen();
        // pen.setStyle( Qt::DotLine );
        // painter->setPen( pen );
        drawRound(painter,bestRect,std::min(bestRect.width,bestRect.height)/4);
        //painter.stroke();
        // Now connect the label to the point with a line.
        // The line is drawn from the center of the near edge of the rectangle
        /*float xline = bestRect.centerX();
        if (bestRect.left > pos.x) {
            xline = bestRect.left;
        }
        if (bestRect.right() < pos.x) {
            xline = bestRect.right();
        }

        float yline = bestRect.centerY();
        if (bestRect.top > pos.y) {
            yline = bestRect.top;
        }
        if (bestRect.bottom() < pos.y) {
            yline = bestRect.bottom();
        }*/

        /*painter.move_to(xline,yline);
        painter.line_to(pos.x,pos.y);
        painter.close_path();
        painter.stroke();*/
    }

    // Mask the label's rectangle so other labels won't overlap it.
    maskRect(RectF::Make(bestRect.left,bestRect.top,bestRect.width,bestRect.height));
}

float PlotView::Private::rectCost(const RectF &r) const{
    RectF pmrc = {0,0,float(plotMask->get_width()),float(plotMask->get_height())};
    if(!pmrc.contains(r)) return 10000.;
    //if (!plotMask.rect().contains(r.toRect())) return 10000.;

    // Compute sum of mask values in the rect r
    Rect ri={int(r.left),int(r.top),int(r.width),int(r.height)};
    Cairo::RefPtr<Cairo::ImageSurface> subMask ;//= plotMask.copy(r.toRect());
    int cost = 0;
    const uint8_t*pixels=(uint8_t*)plotMask->get_data();
    const int stride = plotMask->get_stride();
#if 0
    for (int ix = 0; ix < subMask->get_width(); ++ix) {
        for (int iy = 0; iy < subMask->get_height(); ++iy) {
            //cost += uint32_t(subMask.pixel(ix, iy)).red();
        }
    }
#else
    for(int ix = ri.left;ix<ri.right();ix++){
	for(int iy = ri.top;iy<ri.bottom();iy++){
            uint8_t*pixel = (uint8_t*)(pixels+(iy*stride+ix*4));
	    cost +=pixels[1];
	}
    }
#endif
    return float(cost);
}

void PlotView::onDraw(cdroid::Canvas&p){
    // let QFrame draw its default stuff (like the frame)
    //p.setRenderHint(QPainter::Antialiasing, d->useAntialias);
    View::onDraw(p);
    Rect r = getBound();

    //p.fillRect(rect(), backgroundColor());
    p.translate(getPaddingLeft(), getPaddingTop());
    drawAxes(p);

    setPixRect();
    r= d->pixRect;

    p.save();
    p.rectangle(r.left,r.top,r.width,r.height);
    p.clip();

    resetPlotMask();

    for (PlotObject *po : d->objectList) {
        po->draw(p, this);
    }
    p.restore();//This save/restore used to reset clip area
}

void PlotView::drawAxes(cdroid::Canvas&p){
    if (d->showGrid) {
        p.set_color(gridColor());//setPen(gridColor());

        // Grid lines are placed at locations of primary axes' major tickmarks
        // vertical grid lines
        const std::list<double>& majMarks = axis(BottomAxis)->majorTickMarks();
        if(axis(BottomAxis)->isTickmarkVisible()|| axis(TopAxis)->isTickmarkVisible()){
            for (const double xx : majMarks) {
                double px = d->pixRect.width * (xx - d->dataRect.left) / d->dataRect.width;
                p.move_to(px,0);
                p.line_to(px,d->pixRect.height);
            }
        }
        // horizontal grid lines
        const std::list<double>&leftTickMarks = axis(LeftAxis)->majorTickMarks();
        if(axis(LeftAxis)->isTickmarkVisible()||axis(RightAxis)->isTickmarkVisible()){
            for (const double yy : leftTickMarks) {
                double py = d->pixRect.height * (1.0 - (yy - d->dataRect.top) / d->dataRect.height);
                p.move_to(0,py);
                p.line_to(double(d->pixRect.width), py);
            }
        }
        if(axis(TopAxis)->isTickmarkVisible()||axis(BottomAxis)->isTickmarkVisible()
            ||axis(LeftAxis)->isTickmarkVisible()||axis(RightAxis)->isTickmarkVisible())
            p.stroke();
    }

    p.set_color(foregroundColor());//p->setPen(foregroundColor());
    //p->setBrush(Qt::NoBrush);

    // set small font for tick labels
    p.set_font_size(d->tickLabelSize);
    //p->setFont(f);

    /*** BottomAxis ***/
    PlotAxis *a = axis(BottomAxis);
    if (a->isVisible()) {
        // Draw axis line
        p.move_to(0, d->pixRect.height);
        p.line_to(d->pixRect.width,d->pixRect.height);

        // Draw major tickmarks
        const std::list<double>& majMarks = a->majorTickMarks();
        for (const double xx : majMarks) {
            double px = d->pixRect.width * (xx - d->dataRect.left) / d->dataRect.width;
            if (px > 0 && px < d->pixRect.width) {
                p.move_to(px, d->pixRect.height - TICKOFFSET);
                p.line_to(px, d->pixRect.height - BIGTICKSIZE - TICKOFFSET);
                p.stroke();
                // Draw ticklabel
                if (a->areTickLabelsShown()) {
                    Rect r={int(px) - BIGTICKSIZE, d->pixRect.height + BIGTICKSIZE, 2 * BIGTICKSIZE, BIGTICKSIZE};
                    p.draw_text(r, a->tickLabel(xx),cdroid::Gravity::CENTER);
                }
            }
        }

        // Draw minor tickmarks
        const std::list<double>& minTickMarks = a->minorTickMarks();
        for (const double xx : minTickMarks) {
            double px = d->pixRect.width * (xx - d->dataRect.left) / d->dataRect.width;
            if (px > 0 && px < d->pixRect.width) {
                p.move_to(px,d->pixRect.height - TICKOFFSET);
                p.line_to(px,d->pixRect.height - SMALLTICKSIZE - TICKOFFSET);
            }
        }
        p.stroke();
        // Draw BottomAxis Label
        if (!a->label().empty()) {
            Rect r = {0, d->pixRect.height + 2 * YPADDING, d->pixRect.width, YPADDING};
            p.draw_text(r,a->label(),cdroid::Gravity::CENTER);
        }
    } // End of BottomAxis

    /*** LeftAxis ***/
    a = axis(LeftAxis);
    if (a->isVisible()) {
        // Draw axis line
        p.move_to(0,0);
        p.line_to(0,d->pixRect.height);

        // Draw major tickmarks
        const std::list<double>& majMarks = a->majorTickMarks();
        for (const double yy : majMarks) {
            double py = d->pixRect.height * (1.0 - (yy - d->dataRect.top) / d->dataRect.height);
            if (py > 0 && py < d->pixRect.height) {
                p.move_to(TICKOFFSET,py);
                p.line_to(TICKOFFSET + BIGTICKSIZE,py);
                p.stroke();
                // Draw ticklabel
                if (a->areTickLabelsShown()) {
                    Rect r = {-2 * BIGTICKSIZE - SMALLTICKSIZE, int(py) - SMALLTICKSIZE, 2 * BIGTICKSIZE, 2 * SMALLTICKSIZE};
                    p.draw_text(r, a->tickLabel(yy),Gravity::RIGHT|Gravity::CENTER_VERTICAL);
                }
            }
        }

        // Draw minor tickmarks
        const std::list<double>& minTickMarks = a->minorTickMarks();
        for (const double yy : minTickMarks) {
            double py = d->pixRect.height * (1.0 - (yy - d->dataRect.top) / d->dataRect.height);
            if (py > 0 && py < d->pixRect.height) {
                p.move_to(TICKOFFSET,py);
                p.line_to(TICKOFFSET + SMALLTICKSIZE,py);
            }
        }p.stroke();

        // Draw LeftAxis Label.  We need to draw the text sideways.
        if (!a->label().empty()) {
            // store current painter translation/rotation state
            p.save();

            // translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
            p.translate(-3 * XPADDING, d->pixRect.height);
            p.rotate_degrees(-90.0);

            Rect r = {0, 0, d->pixRect.height, XPADDING};
            p.draw_text(r,a->label(),Gravity::CENTER); // draw the label, now that we are sideways

            p.restore(); // restore translation/rotation state
        }
    } // End of LeftAxis

    // Prepare for top and right axes; we may need the secondary data rect
    double x0 = d->dataRect.left;
    double y0 = d->dataRect.top;
    double dw = d->dataRect.width;
    double dh = d->dataRect.height;
    if (secondaryDataRect().empty()==false){//isValid()) {
        x0 = secondaryDataRect().left;
        y0 = secondaryDataRect().top;
        dw = secondaryDataRect().width;
        dh = secondaryDataRect().height;
    }

    /*** TopAxis ***/
    a = axis(TopAxis);
    if (a->isVisible()) {
        // Draw axis line
	p.move_to(0,0);
	p.line_to(d->pixRect.width, 0);
        // Draw major tickmarks
        const std::list<double>& majMarks = a->majorTickMarks();
        for (const double xx : majMarks) {
            double px = d->pixRect.width * (xx - x0) / dw;
            if (px > 0 && px < d->pixRect.width) {
                p.move_to(px, TICKOFFSET);
                p.line_to(px,double(BIGTICKSIZE + TICKOFFSET));
                p.stroke();
                // Draw ticklabel
                if (a->areTickLabelsShown()) {
                    Rect r = {int(px) - BIGTICKSIZE, (int)-1.5 * BIGTICKSIZE, 2 * BIGTICKSIZE, BIGTICKSIZE};
                    p.draw_text(r,a->tickLabel(xx),Gravity::CENTER);// Qt::AlignCenter | Qt::TextDontClip, a->tickLabel(xx));
                }
            }
        }

        // Draw minor tickmarks
        const std::list<double>& minMarks = a->minorTickMarks();
        for (const double xx : minMarks) {
            double px = d->pixRect.width * (xx - x0) / dw;
            if (px > 0 && px < d->pixRect.width) {
                p.move_to(px, TICKOFFSET);
                p.line_to(px,SMALLTICKSIZE + TICKOFFSET);
            }
        }
        p.stroke();

        // Draw TopAxis Label
        if (!a->label().empty()) {
            Rect r = {0, 0 - 3 * YPADDING, d->pixRect.width, YPADDING};
            p.draw_text(r, a->label(),Gravity::CENTER);
        }
    } // End of TopAxis

    /*** RightAxis ***/
    a = axis(RightAxis);
    if (a->isVisible()) {
        // Draw axis line
        p.move_to(d->pixRect.width, 0);
        p.line_to(d->pixRect.width, d->pixRect.height);

        // Draw major tickmarks
        const std::list<double>& majMarks = a->majorTickMarks();
        for (const double yy : majMarks) {
            double py = d->pixRect.height * (1.0 - (yy - y0) / dh);
            if (py > 0 && py < d->pixRect.height) {
                p.move_to(d->pixRect.width - TICKOFFSET,py);
                p.line_to(d->pixRect.width - TICKOFFSET - BIGTICKSIZE,py);
                p.stroke();
                // Draw ticklabel
                if (a->areTickLabelsShown()) {
                    Rect r = {d->pixRect.width + SMALLTICKSIZE, int(py) - SMALLTICKSIZE, 2 * BIGTICKSIZE, 2 * SMALLTICKSIZE};
                    p.draw_text(r,a->tickLabel(yy),Gravity::LEFT|Gravity::CENTER_VERTICAL);
                }
            }
        }

        // Draw minor tickmarks
        const std::list<double>& minMarks = a->minorTickMarks();
        for (const double yy : minMarks) {
            double py = d->pixRect.height * (1.0 - (yy - y0) / dh);
            if (py > 0 && py < d->pixRect.height) {
                p.move_to(d->pixRect.width ,py);
                p.line_to(d->pixRect.width - SMALLTICKSIZE,py);
            }
        }
        p.stroke();

        // Draw RightAxis Label.  We need to draw the text sideways.
        if (!a->label().empty()) {
            // store current painter translation/rotation state
            p.save();

            // translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
            p.translate(d->pixRect.width + 2 * XPADDING, d->pixRect.height);
            p.rotate_degrees(-90.0);

            Rect r = {0, 0, d->pixRect.height, XPADDING};
            p.draw_text(r,a->label(),Gravity::CENTER); // draw the label, now that we are sideways

            p.restore(); // restore translation/rotation state
        }
    } // End of RightAxis
}

}//endof namespace
