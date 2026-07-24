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
#include <cairomm/matrix.h>   // Cairo::Matrix for transform()
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
    void moveTo(double x, double y);
    void lineTo(double x, double y);
    void rel_move_to(double x,double y);
    void rel_line_to(double,double);
    void setFillType(Cairo::Context::FillRule fill_rule);
    void cubicTo(double x1, double y1, double x2, double y2, double x3, double y3);
    void quadTo(double x1, double y1, double x2, double y2);
    void rel_curve_to(double x1, double y1, double x2, double y2, double x3, double y3);;
    void rel_quad_to(double dx1, double dy1, double dx2, double dy2);
    void arc(double xc, double yc, double radius, double angle1, double angle2);
    void arc_negative(double xc, double yc, double radius, double angle1, double angle2);
    void arcTo(double x1, double y1, double x2, double y2, double radius);
    void arcTo(const RectF&, double startAngle, double sweepAngle, bool forceMoveTo);
    void arcTo(double left, double top, double width, double height, double startAngle, double sweepAngle, bool forceMoveTo);
    void arcTo(double rx, double ry, double angle, bool largeArc, bool sweepFlag, double x, double y);
    void addOval(const RectF&,bool isClockWise);
    void addOval(int left,int top,int width,int height,bool isClockWise);
    void addRect(double x, double y, double width, double height);
    void addRoundRect(double x,double y,double width,double height,const std::vector<float>& radii);
    void addRoundRect(const RectF&rect,const std::vector<float>& radii);
    void reset();
    void begin_new_sub_path();
    void close();
    void addPath(const Path&);
    void addPath(const Path&path,double dx,double dy);
    void computeBounds(RectF&, bool include_stroke);
    bool isConvex()const;
    void approximate(std::vector<float>&,float acceptableError);
    // android.graphics.Path#transform — apply a matrix to this path's points (in place / into dst).
    void transform(const Cairo::Matrix& matrix);
    void transform(const Cairo::Matrix& matrix, Path& dst);
private:
    // Replays this path's points (matrix-transformed) onto a target context (shared by both transform overloads).
    void transformInto(const Cairo::Matrix& matrix, Cairo::Context* target);
};
}
#endif
