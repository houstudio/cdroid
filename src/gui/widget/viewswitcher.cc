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
#include <widget/viewswitcher.h>

namespace cdroid{

DECLARE_WIDGET(ViewSwitcher)

ViewSwitcher::ViewSwitcher(int w,int h)
   :ViewAnimator(w,h){
}

ViewSwitcher::ViewSwitcher(Context*ctx,const AttributeSet&atts)
  :ViewAnimator(ctx,atts){
}

void ViewSwitcher::addView(View* child, int index, ViewGroup::LayoutParams* params){
    if (getChildCount() >= 2) {
        throw std::runtime_error("Can't add more than 2 views to a ViewSwitcher");
    }
    ViewAnimator::addView(child, index, params);
}

View* ViewSwitcher::getNextView() {
    int which = mWhichChild == 0 ? 1 : 0;
    return getChildAt(which);
}

View* ViewSwitcher::obtainView() {
    View* child = mFactory();
    ViewGroup::LayoutParams* lp = (ViewGroup::LayoutParams*) child->getLayoutParams();
    if (lp == nullptr) {
        lp = new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT);
    }
    addView(child,getChildCount(), lp);
    return child;
}

void ViewSwitcher::setFactory(ViewFactory factory) {
    mFactory = factory;
    obtainView();
    obtainView();
}

void ViewSwitcher::reset() {
    mFirstTime = true;
    View* v= getChildAt(0);
    if (v) v->setVisibility(View::GONE);
    v = getChildAt(1);
    if (v) v->setVisibility(View::GONE);
}

}//namespace

