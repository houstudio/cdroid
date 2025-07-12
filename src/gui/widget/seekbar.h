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
#ifndef __SEEK_BAR_H__
#define __SEEK_BAR_H__
#include <widget/absseekbar.h>

namespace cdroid{
class SeekBar:public AbsSeekBar{
public:
    struct OnSeekBarChangeListener{
        std::function<void(SeekBar&,int,bool)> onProgressChanged;//(SeekBar seekBar, int progress, boolean fromUser);
        std::function<void(SeekBar&)>onStartTrackingTouch;
        std::function<void(SeekBar&)>onStopTrackingTouch;
    };
protected:
    OnSeekBarChangeListener  mOnSeekBarChangeListener;
    void onProgressRefresh(float scale, bool fromUser, int progress)override;
    void onStartTrackingTouch()override;
    void onStopTrackingTouch()override;
public:
    SeekBar(int w,int h);
    SeekBar(Context*ctx,const AttributeSet& attrs);
    void setOnSeekBarChangeListener(const OnSeekBarChangeListener& l);
    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
};

}//namespace
#endif 
