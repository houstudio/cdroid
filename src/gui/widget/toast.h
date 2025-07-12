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
#ifndef __TOAST_H__
#define __TOAST_H__
#include <view/view.h>
#include <widget/cdwindow.h>

namespace cdroid{

class Toast{
private:
    int mGravity;
    int mX,mY;
    int mHorizontalMargin;
    int mVerticalMargin;
    Window*mWindow;
protected:
    Context*mContext;
    int mDuration;
    View*mNextView;
public:
    static constexpr int LENGTH_SHORT= 2000;
    static constexpr int LENGTH_LONG = 4000;
public:
    Toast(Context*context);
    void show();
    void cancel();
    Toast& setView(View*);
    View*getView()const;
    Toast& setDuration(int duration);
    int getDuration()const;
    Toast& setMargin(int horizontalMargin,int verticalMargin);
    int getHorizontalMargin()const;
    int getVerticalMargin()const;
    Toast& setGravity(int gravity,int xoffset,int yoffset);
    int getGravity()const;
    int getXOffset()const;
    int getYOffset()const;
    Toast& setText(const std::string&);
    static Toast*makeText(Context*,const std::string&text,int duration= LENGTH_SHORT);
};
}//endof namespace

#endif
