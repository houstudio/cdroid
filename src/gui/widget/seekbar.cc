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
#include <widget/seekbar.h>
#include <widget/R.h>
namespace cdroid{

DECLARE_WIDGET2(SeekBar,"cdroid:attr/seekBarStyle")

SeekBar::SeekBar(Context*ctx,const AttributeSet& attrs)
  :AbsSeekBar(ctx,attrs){
}

SeekBar::SeekBar(int w,int h):AbsSeekBar(w,h){
}

void SeekBar::onProgressRefresh(float scale, bool fromUser, int progress){
    AbsSeekBar::onProgressRefresh(scale, fromUser, progress);
    if (mOnSeekBarChangeListener.onProgressChanged) {
         mOnSeekBarChangeListener.onProgressChanged(*this, progress, fromUser);
    }
}

void SeekBar::onStartTrackingTouch() {
    AbsSeekBar::onStartTrackingTouch();
    if (mOnSeekBarChangeListener.onStartTrackingTouch) {
        mOnSeekBarChangeListener.onStartTrackingTouch(*this);
    }
}

void SeekBar::onStopTrackingTouch() {
    AbsSeekBar::onStopTrackingTouch();
    if (mOnSeekBarChangeListener.onStopTrackingTouch) {
        mOnSeekBarChangeListener.onStopTrackingTouch(*this);
    }
}

void SeekBar::setOnSeekBarChangeListener(const OnSeekBarChangeListener& l){
     mOnSeekBarChangeListener = l;
}

std::string SeekBar::getAccessibilityClassName()const{
    return "SeekBar";
}

void SeekBar::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    AbsSeekBar::onInitializeAccessibilityNodeInfoInternal(info);
    if (canUserSetProgress()) {
        info.addAction(R::id::accessibilityActionSetProgress);//AccessibilityNodeInfo::ACTION_SET_PROGRESS);
    }
}

}
