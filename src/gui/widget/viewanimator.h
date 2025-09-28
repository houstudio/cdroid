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
#ifndef __VIEW_ANIMATOR_H__
#define __VIEW_ANIMATOR_H__
#include <widget/framelayout.h>
namespace cdroid{

class ViewAnimator:public FrameLayout{
protected:
    int mWhichChild = 0;
    bool mFirstTime = true;
    bool mAnimateFirstTime = true;
    Animation  *mInAnimation;
    Animation  *mOutAnimation;
private:
    void initViewAnimator(Context* context,const AttributeSet& attrs);
protected:
    void showOnly(int childIndex);
    virtual void showOnly(int childIndex, bool animate);
public:
    ViewAnimator(int w,int h);
    ViewAnimator(Context* context,const AttributeSet& attrs);
    ~ViewAnimator();
    void setDisplayedChild(int whichChild);
    int getDisplayedChild()const;
    virtual void showNext();
    virtual void showPrevious();
    void addView(View* child, int index,ViewGroup::LayoutParams* params)override;
    void removeAllViews()override;
    void removeView(View* view)override;
    void removeViewAt(int index)override;
    void removeViewInLayout(View* view) override;
    void removeViews(int start, int count) override;
    void removeViewsInLayout(int start, int count) override;
    View* getCurrentView()const;
    Animation* getInAnimation()const;
    void setInAnimation(Animation* inAnimation);
    Animation* getOutAnimation()const;
    void setOutAnimation(Animation* outAnimation);
    bool getAnimateFirstView()const;
    void setAnimateFirstView(bool animate);
    int getBaseline()override;
};
}//namespace
#endif
