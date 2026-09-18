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
    Drawable* mButtonDrawable;
    cdroid::RefPtr<ColorStateList>mButtonTintList;
    /* AOSP: BlendMode mButtonBlendMode. CDROID has no BlendMode class; the
       PorterDuff::Mode union covers the legacy modes and NOOP stands in for null. */
    PorterDuffMode mButtonBlendMode;
    bool mChecked;
    bool mBroadcasting;
    bool mHasButtonTint;
    bool mHasButtonBlendMode;
    // Indicates whether the toggle state was set from resources or dynamically, so it can be used
    // to sanitize autofill requests.
    bool mCheckedFromResource;
    OnCheckedChangeListener mOnCheckedChangeListener;
    OnCheckedChangeListener mOnCheckedChangeWidgetListener;

    std::string mCustomStateDescription;
    void initCompoundButton();
    void applyButtonTint();
protected:
    std::vector<int>onCreateDrawableState(int)override;
    int getHorizontalOffsetForDrawables()const override;
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
    void onDetachedFromWindow()override;
    void onDraw(Canvas&canvas)override;
    void setDefaultStateDescription();
public:
    CompoundButton(Context*ctx);   // AOSP CompoundButton(Context)
    CompoundButton(Context*ctx,const AttributeSet*attrs);
    CompoundButton(Context*ctx,const AttributeSet* attrs,int defStyleAttr);
    ~CompoundButton()override;
    void setButtonDrawable(int resid);
    void setButtonDrawable(Drawable*d);
    bool performClick()override;
    Drawable* getButtonDrawable()const;
    void jumpDrawablesToCurrentState()override;
    void setButtonTintList(const cdroid::RefPtr<ColorStateList>& tint);
    const cdroid::RefPtr<ColorStateList> getButtonTintList()const;
    void setButtonTintMode(PorterDuffMode tintMode);
    PorterDuffMode getButtonTintMode()const;
    void setButtonTintBlendMode(PorterDuffMode tintMode);
    PorterDuffMode getButtonTintBlendMode()const;
    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    int getCompoundPaddingLeft()const override;
    int getCompoundPaddingRight()const override;

    //inerited from Checkable
    void setChecked(bool checked)override;
    bool isChecked()const override;
    void toggle()override;

    void setStateDescription(const std::string&stateDescription)override;
    virtual std::string getButtonStateDescription();
    void setOnCheckedChangeListener(const OnCheckedChangeListener& listener);
    /*OnCheckedChangeWidgetListener internal use(for radiogroup...)*/
    void setOnCheckedChangeWidgetListener(const OnCheckedChangeListener& listener);
    void drawableHotspotChanged(float x,float y)override;
};

}
#endif
