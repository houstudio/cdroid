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
#ifndef __INSETS_H__
#define __INSETS_H__
#include <core/rect.h>
namespace cdroid{

class Insets{
public:
    int left;
    int top;
    int right;
    int bottom;
    const static Insets NONE;
private:
    Insets(int left, int top, int right, int bottom);
public:
    Insets();
    void set(int l,int t,int r,int b);
    static Insets of(int left, int top, int right, int bottom);
    static Insets of(const Rect& r);
    const Rect toRect()const;
    static Insets add(const Insets&a,const Insets&b);
    static Insets subtract(const Insets&a,const Insets&b);
    static Insets max(const Insets&a,const Insets&b);
    static Insets min(const Insets&a,const Insets&b);
    bool operator==(const Insets&o)const;
    bool operator!=(const Insets&o)const;
};

}
#endif
