/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __CDROID_PATH_H__
#define __CDROID_PATH_H__
#include <core/rect.h>
#include <cairomm/context.h>
#include <functional>
#include <vector>
namespace cdroid{

class Path{
private:
    Cairo::RefPtr<Cairo::Context>mCTX;     
public:
    typedef cairo_path_t cobject;
    Path();
    Path(Cairo::Context*);
    Path(const Path&);
    void append_to_context(Cairo::Context*)const;
    void append_to_context(const Cairo::RefPtr<Cairo::Context>&to)const;
    cobject* copy_path()const;
    int fromSVGPathData(const std::string&pathData,std::function<void(int8_t,const std::vector<float>&)>);
    void move_to(double x, double y);
    void line_to(double x, double y);
    void rel_move_to(double x,double y);
    void rel_line_to(double,double);
    void set_fill_rule(Cairo::Context::FillRule fill_rule);
    void curve_to(double x1, double y1, double x2, double y2, double x3, double y3);
    void quad_to(double x1, double y1, double x2, double y2);
    void rel_curve_to(double x1, double y1, double x2, double y2, double x3, double y3);;
    void rel_quad_to(double dx1, double dy1, double dx2, double dy2);
    void arc(double xc, double yc, double radius, double angle1, double angle2);
    void arc_to(double x1, double y1, double x2, double y2, double radius);
    void arc_to(const RectF&, double startAngle, double sweepAngle, bool forceMoveTo);
    void arc_to(double left, double top, double width, double height, double startAngle, double sweepAngle, bool forceMoveTo);
    void arc_to(double rx, double ry, double angle, bool largeArc, bool sweepFlag, double x, double y);
    void add_oval(const RectF&,bool isClockWise);
    void add_oval(int left,int top,int width,int height,bool isClockWise);
    void rectangle(double x, double y, double width, double height);
    void round_rectangle(double x,double y,double width,double height,const std::vector<float>& radii);
    void round_rectangle(const RectF&rect,const std::vector<float>& radii);
    void reset();
    void begin_new_sub_path();
    void close_path();
    void append_path(const Path&);
    void append_path(const Path&path,double dx,double dy);
    void compute_bounds(RectF&, bool include_stroke);
    bool is_convex()const;
    void approximate(std::vector<float>&,float acceptableError);
};
}
#endif
