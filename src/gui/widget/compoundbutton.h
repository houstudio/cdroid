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
#ifndef __COMPOUND_BUTTON_H__
#define __COMPOUND_BUTTON_H__
#include <widget/button.h>
#include <widget/checkable.h>
namespace cdroid{
class CompoundButton:public Button,public Checkable{
public:
    DECLARE_UIEVENT(void,OnCheckedChangeListener,CompoundButton&view,bool);
private: 
    bool mChecked;
    bool mBroadcasting;
    bool mCheckedFromResource;
    int  mButtonTintMode;
    Drawable* mButtonDrawable;
    const ColorStateList*mButtonTintList;
    std::string mCustomStateDescription;
    OnCheckedChangeListener mOnCheckedChangeListener;
    OnCheckedChangeListener mOnCheckedChangeWidgetListener;
    void initCompoundButton();
    void applyButtonTint();
protected:
    std::vector<int>onCreateDrawableState(int)override;
    int getHorizontalOffsetForDrawables()const override;
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
    void onDetachedFromWindow()override;
    void onDraw(Canvas&canvas)override;
    virtual void doSetChecked(bool);
    void setDefaultStateDescription();
public:
    CompoundButton(const std::string&txt,int width,int height);
    CompoundButton(Context*ctx,const AttributeSet&attrs);
    ~CompoundButton()override;
    void setButtonDrawable(const std::string&resid);
    void setButtonDrawable(Drawable*d);
    bool performClick()override;
    Drawable* getButtonDrawable()const;
    void jumpDrawablesToCurrentState()override;
    void setButtonTintList(const ColorStateList* tint);
    const ColorStateList* getButtonTintList()const;
    void setButtonTintMode(PorterDuffMode tintMode);
    PorterDuffMode getButtonTintMode()const;
    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    int getCompoundPaddingLeft()const override;
    int getCompoundPaddingRight()const override;
    //inerited from Checkable
#if !(defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE)
    void setChecked(bool checked)override;
    bool isChecked()const override;
    void toggle()override;
#endif
    void setStateDescription(const std::string&stateDescription)override;
    virtual std::string getButtonStateDescription();
    void setOnCheckedChangeListener(const OnCheckedChangeListener& listener);
    /*OnCheckedChangeWidgetListener internal use(for radiogroup...)*/
    void setOnCheckedChangeWidgetListener(const OnCheckedChangeListener& listener);
    void drawableHotspotChanged(float x,float y)override;
};

}
#endif
