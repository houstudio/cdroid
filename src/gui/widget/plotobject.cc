/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <widget/plotobject.h>
#include <widget/plotpoint.h>
#include <widget/plotview.h>
#include <cdtypes.h>
#include <cdlog.h>

namespace cdroid{

class PlotObject::Private
{
public:
    Private(PlotObject *qq)
        :q(qq){
	type = UnknownType;
	pointStyle = NoPoints;
    }

    ~Private(){
        //qDeleteAll(pList);
	for(auto i:pList)delete i;
    }

    PlotObject *q;

    std::list<PlotPoint *> pList;
    int type;
    PointStyle pointStyle;
    double size;
    uint32_t pen, linePen, barPen, labelPen;
    uint32_t brush, barBrush;
};

PlotObject::PlotObject(const uint32_t &c, PlotType t, double size, PointStyle ps)
    : d(new Private(this))
{
    // By default, all pens and brushes are set to the given color
    setBrush(c);
    setBarBrush(c);
    setPen(c);//QPen(brush(), 1));
    setLinePen(pen());
    setBarPen(pen());
    setLabelPen(pen());

    d->type |= t;
    setSize(size);
    setPointStyle(ps);
}

PlotObject::~PlotObject()
{
    delete d;
}

PlotObject::PlotTypes PlotObject::plotTypes() const
{
    return (PlotObject::PlotTypes)d->type;
}

void PlotObject::setShowPoints(bool b)
{
    if (b) {
        d->type |= PlotObject::Points;
    } else {
        d->type &= ~PlotObject::Points;
    }
}

void PlotObject::setShowLines(bool b)
{
    if (b) {
        d->type |= PlotObject::Lines;
    } else {
        d->type &= ~PlotObject::Lines;
    }
}

void PlotObject::setShowBars(bool b)
{
    if (b) {
        d->type |= PlotObject::Bars;
    } else {
        d->type &= ~PlotObject::Bars;
    }
}

double PlotObject::size() const
{
    return d->size;
}

void PlotObject::setSize(double s)
{
    d->size = s;
}

PlotObject::PointStyle PlotObject::pointStyle() const
{
    return d->pointStyle;
}

void PlotObject::setPointStyle(PointStyle p)
{
    d->pointStyle = p;
}

const QPen &PlotObject::pen() const
{
    return d->pen;
}

void PlotObject::setPen(const QPen &p)
{
    d->pen = p;
}

const QPen &PlotObject::linePen() const
{
    return d->linePen;
}

void PlotObject::setLinePen(const QPen &p)
{
    d->linePen = p;
}

const QPen &PlotObject::barPen() const
{
    return d->barPen;
}

void PlotObject::setBarPen(const QPen &p)
{
    d->barPen = p;
}

const QPen &PlotObject::labelPen() const
{
    return d->labelPen;
}

void PlotObject::setLabelPen(const QPen &p)
{
    d->labelPen = p;
}

const QBrush PlotObject::brush() const
{
    return d->brush;
}

void PlotObject::setBrush(const QBrush &b)
{
    d->brush = b;
}

const QBrush PlotObject::barBrush() const
{
    return d->barBrush;
}

void PlotObject::setBarBrush(const QBrush &b)
{
    d->barBrush = b;
}

std::list<PlotPoint *> PlotObject::points() const
{
    return d->pList;
}

void PlotObject::addPoint(const PointF &p, const std::string &label, double barWidth)
{
    addPoint(new PlotPoint(p.x, p.y, label, barWidth));
}

void PlotObject::addPoint(PlotPoint *p)
{
    if (!p) {
        return;
    }
    d->pList.push_back(p);
}

void PlotObject::addPoint(double x, double y, const std::string &label, double barWidth)
{
    addPoint(new PlotPoint(x, y, label, barWidth));
}

void PlotObject::removePoint(int index)
{
    if ((index < 0) || (index >= d->pList.size())) {
        LOG(WARN) << "PlotObject::removePoint(): index " << index << " out of range!";
        return;
    }
    auto it= d->pList.begin();
    std::advance(it,index);
    d->pList.erase(it);
}

void PlotObject::clearPoints()
{
    for(auto i:d->pList)delete i;
    d->pList.clear();
}

void PlotObject::draw(cdroid::Canvas&painter,PlotView*pw)
{
    // Order of drawing determines z-distance: Bars in the back, then lines,
    // then points, then labels.

    if (d->type & Bars) {
        double w = 0;
        for (int i = 0; i < d->pList.size(); ++i) {
	    auto it =d->pList.begin();
	    std::advance(it,i);
            if ((*it)->barWidth() == 0.0) {
                if (i < d->pList.size() - 1) {
		    auto next=it;
		    std::advance(next,1);
                    w = (*next)->x() - (*it)->x();
                }
                // For the last bin, we'll just keep the previous width

            } else {
                w = (*it)->barWidth();
            }

            PointF pp = (*it)->position();
            PointF p1={float(pp.x - 0.5f * w), 0.f};
            PointF p2={float(pp.x + 0.5f * w), pp.y};
            PointF sp1 = pw->mapToWidget(p1);
            PointF sp2 = pw->mapToWidget(p2);

            RectF barRect = RectF::Make(sp1.x, sp1.y, sp2.x - sp1.x, sp2.y - sp1.y);//.normalized();
            painter.rectangle(sp1.x, sp1.y, sp2.x - sp1.x, sp2.y - sp1.y);//drawRect(barRect);
	    painter.set_color(barBrush());
	    painter.fill_preserve();
	    painter.set_color(barPen());
	    painter.stroke();
            pw->maskRect(barRect, 0.25);
        }
    }

    // Draw lines:
    if (d->type & Lines) {
	bool bPrevious = false;
        painter.set_color(linePen());
        PointF Previous; // Initialize to null
        for (const PlotPoint *pp : d->pList) {
            // q is the position of the point in screen pixel coordinates
            PointF q = pw->mapToWidget(pp->position());
            if (bPrevious) {
                //painter->drawLine(Previous, q);
		painter.line_to(q.x,q.y);
                pw->maskAlongLine(Previous, q);
            }else{
	        bPrevious = true;
		painter.move_to(q.x,q.y);
	    }
            Previous = q;
        }
	painter.stroke();
    }

    // Draw points:
    if (d->type & Points) {
        for (const PlotPoint *pp : d->pList) {
            // q is the position of the point in screen pixel coordinates
            PointF q = pw->mapToWidget(pp->position());
            if (pw->pointInView(q.x,q.y,1)) {
                double x1 = q.x - size();
                double y1 = q.y - size();
                RectF qr = RectF::Make(x1, y1, 2 * size(), 2 * size());
                // Mask out this rect in the plot for label avoidance
                pw->maskRect(qr, 2.0);

                switch (pointStyle()) {
                case Circle:
                    //painter->drawEllipse(qr);
		    painter.arc(q.x,q.y,size(),0,M_PI*2.f);
		    painter.stroke();
                    break;

                case Letter:
                    //painter->drawText(qr, Qt::AlignCenter, pp->label().left(1));
		    painter.draw_text(Rect::Make(x1, y1, 2 * size(), 2 * size()),pp->label(),cdroid::Gravity::CENTER);
                    break;

                case Triangle: {
		    painter.set_color(brush());
		    painter.move_to(q.x - size(), q.y + size());
		    painter.line_to(q.x, q.y - size());
		    painter.line_to(q.x + size(), q.y + size());
		    painter.line_to(q.x - size(), q.y + size());
		    painter.line_to(q.x - size(), q.y + size());//line to 1st point(closepath)
		    painter.fill_preserve();
		    painter.set_color(pen());
		    painter.stroke();
                    break;
                }

                case Square:
		    painter.rectangle(qr.left,qr.top,qr.width,qr.height);
		    painter.set_color(brush());
		    painter.fill_preserve();
		    painter.set_color(pen());
		    painter.stroke();
                    break;

                case Pentagon: {
		    painter.move_to(q.x, q.y - size());
		    painter.line_to(q.x + size(), q.y - 0.309 * size());
		    painter.line_to(q.x + 0.588 * size(), q.y + size());
		    painter.line_to(q.x - 0.588 * size(), q.y + size());
		    painter.line_to(q.x - size(), q.y - 0.309 * size());
		    painter.line_to(q.x, q.y - size());//line to 1st point(closepath)

		    painter.set_color(brush());
		    painter.fill_preserve();
		    painter.set_color(pen());
		    painter.stroke();		    
                    break;
                }

                case Hexagon: {
		    painter.move_to(q.x, q.y + size());
		    painter.line_to(q.x + size(), q.y + 0.5 * size());
		    painter.line_to(q.x + size(), q.y - 0.5 * size());
		    painter.line_to(q.x, q.y - size());
		    painter.line_to(q.x - size(), q.y + 0.5 * size());
		    painter.line_to(q.x - size(), q.y - 0.5 * size());
		    painter.line_to(q.x, q.y + size());//line to 1st point(closepath)
		    painter.fill_preserve();
		    painter.set_color(pen());
		    painter.stroke();
                    break;
                }

                case Asterisk:
	            painter.move_to(q.x,q.y);
		    painter.line_to(q.x, q.y + size());
		    painter.move_to(q.x,q.y);
		    painter.line_to(q.x + size(), q.y - 0.5 * size());
		    painter.move_to(q.x,q.y);
		    painter.line_to(q.x + size(), q.y - 0.5 * size());
		    painter.move_to(q.x,q.y);
		    painter.line_to(q.x, q.y - size());
		    painter.move_to(q.x,q.y);
		    painter.line_to(q.x - size(), q.y + 0.5 * size());
		    painter.move_to(q.x,q.y);
		    painter.line_to(q.x - size(), q.y - 0.5 * size());
		    painter.line_to(q.x,q.y);//line to 1st point(closepath)
		    painter.set_color(brush());
		    painter.fill_preserve();
		    painter.set_color(pen());
		    painter.stroke();
                    break;

                case Star: {
		    painter.move_to(q.x, q.y - size());
		    painter.line_to(q.x + 0.2245 * size(), q.y - 0.309 * size());
		    painter.line_to(q.x + size(), q.y - 0.309 * size());
		    painter.line_to(q.x + 0.363 * size(), q.y + 0.118 * size());
		    painter.line_to(q.x + 0.588 * size(), q.y + size());
		    painter.line_to(q.x, q.y + 0.382 * size());
		    painter.line_to(q.x - 0.588 * size(), q.y + size());
		    painter.line_to(q.x - 0.363 * size(), q.y + 0.118 * size());
		    painter.line_to(q.x - size(), q.y - 0.309 * size());
		    painter.line_to(q.x - 0.2245 * size(), q.y - 0.309 * size());
		    painter.line_to(q.x, q.y - size());//line to 1st point(closepath)
		    painter.set_color(brush());
		    painter.fill_preserve();
		    painter.set_color(pen());
		    painter.stroke();
                    break;
                }

                default:
                    break;
                }
            }
        }
    }

    // Draw labels
    painter.set_color(labelPen());

    for (PlotPoint *pp : d->pList) {
        PointF q = pw->mapToWidget(pp->position());//.toPoint();
        if (pw->pointInView(q.x,q.y, 1) && !pp->label().empty()) {
            pw->placeLabel(painter, pp);
        }
    }
}
}//endof namespace
