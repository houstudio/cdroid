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
#ifndef __RADIOGROUP_H__
#define __RADIOGROUP_H__
#include <widget/linearlayout.h>
#include <widget/radiobutton.h>
namespace cdroid{


class RadioGroup: public LinearLayout{
public:
    class LayoutParams;
private:
    int mCheckedId;
    bool mInitialCheckedId;
    bool mProtectFromCheckedChange;
    CompoundButton::OnCheckedChangeListener mChildOnCheckedChangeListener;
    CompoundButton::OnCheckedChangeListener mOnCheckedChangeListener;
    ViewGroup::OnHierarchyChangeListener mOnHierarchyChangeListener;
private:
    void init();
    void onChildViewAdded(View& parent, View* child);
    void onChildViewRemoved(View& parent, View* child);
    void setCheckedId(int id);
    void setCheckedStateForView(int viewId, bool checked);
    void onRadioChecked(CompoundButton&c,bool checked);
    int getVisibleChildWithTextCount()const;
    bool isVisibleWithText(RadioButton* button)const;
protected:
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LinearLayout::LayoutParams* generateDefaultLayoutParams()const override;
    void onFinishInflate()override;
public:
    RadioGroup(int w,int h);
    RadioGroup(Context* context,const AttributeSet& attrs);
    LinearLayout::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    void addView(View* child, int index,ViewGroup::LayoutParams* params)override;
    int getCheckedRadioButtonId()const;
    std::string getAccessibilityClassName()const override;
    void setOnHierarchyChangeListener(const ViewGroup::OnHierarchyChangeListener& listener)override;
    void setOnCheckedChangeListener(const CompoundButton::OnCheckedChangeListener& listener);
    void check(int id);
    void clearCheck();
    void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info)override;
    int getIndexWithinVisibleButtons(View* child)const;
};
class RadioGroup::LayoutParams:public LinearLayout::LayoutParams{
public:
    LayoutParams(Context*c,const AttributeSet&);
    LayoutParams(int w,int h);
    LayoutParams(int w, int h, float initWeight);
    LayoutParams(const ViewGroup::LayoutParams& p);
    LayoutParams(const MarginLayoutParams& source);
};
}/*endof namepace*/

#endif
