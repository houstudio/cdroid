/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * Port of com.android.internal.widget.DialogViewAnimator (android-36).
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
#ifndef __DIALOG_VIEW_ANIMATOR_H__
#define __DIALOG_VIEW_ANIMATOR_H__
#include <widget/viewanimator.h>
namespace cdroid{

// Port of com.android.internal.widget.DialogViewAnimator.
// A ViewAnimator with a more reasonable handling of MATCH_PARENT: unlike
// FrameLayout's onMeasure, the max-size accumulation skips MATCH_PARENT children
// and only plain padding (not the foreground padding) is accounted for.
class DialogViewAnimator:public ViewAnimator{
private:
    std::vector<View*> mMatchParentChildren;
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    DialogViewAnimator(Context* context,const AttributeSet& attrs);
    DialogViewAnimator(Context* context,const AttributeSet* attrs,int defStyleAttr=0);
};

}//namespace
#endif
