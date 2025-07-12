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
#ifndef __TRANSFORMATION_H__
#define __TRANSFORMATION_H__
#include <core/rect.h>
#include <core/insets.h>
#include <cairomm/matrix.h>
#include <animation/interpolators.h>
#include <core/attributeset.h>

typedef Cairo::Matrix Matrix;
namespace cdroid{

class Transformation{
public:
    static constexpr int TYPE_IDENTITY = 0x0;
    static constexpr int TYPE_ALPHA = 0x1;;
    static constexpr int TYPE_MATRIX = 0x2;;
    static constexpr int TYPE_BOTH = TYPE_ALPHA | TYPE_MATRIX;
protected:
    Matrix mMatrix;
    float mAlpha;
    int mTransformationType;
    bool mHasClipRect;
    Rect mClipRect ;
    Insets mInsets;
public:
    Transformation();
    void operator=(const Transformation&);
    void clear();
    int getTransformationType()const;
    void setTransformationType(int transformationType);
    void set(const Transformation& t);
    void compose(const Transformation& t);
    void postCompose(const Transformation& t);
    const Matrix& getMatrix()const;
    Matrix& getMatrix();
    void setAlpha(float alpha);
    float getAlpha()const;
    void setClipRect(const Rect& r);
    void setClipRect(int l, int t, int w, int h);
    Rect getClipRect()const;
    bool hasClipRect()const;
    void setInsets(int left,int top,int right,int bottom);
    const Insets&getInsets()const;
};

}/*endof namespace*/
#endif
