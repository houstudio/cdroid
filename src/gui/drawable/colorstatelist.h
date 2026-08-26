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
#ifndef __COLOR_STATE_LIST_H__
#define __COLOR_STATE_LIST_H__
#include <vector>
#include <iostream>
#include <core/context.h>
#include <core/attributeset.h>
#include <content/complexcolor.h>
#include <core/xmlpullparser.h>
#include <content/androidfw/restable.h>   // ResTable::Theme (createFromXml theme param)

namespace cdroid{

class ColorStateList:public ComplexColor{
private:
    static constexpr int DEFAULT_COLOR = 0xFFFF0000;
    static std::vector<std::vector<int>>EMPTY;
    int mDefaultColor;
    int mChangingConfigurations;
    bool mIsOpaque;
    std::vector<int>mColors;
    std::vector<std::vector<int>>mStateSpecs;
private:
    // AOSP private inflate(Resources, XmlPullParser, AttributeSet, Theme). Theme
    // is threaded for arity but currently a no-op (obtainStyledAttributes(Theme)
    // is not ported); see the DEFERRED note in the .cc. Resources is taken const
    // -- inflate only reads from it (obtainStyledAttributes is const).
    void inflate(const Resources&r,XmlPullParser& parser,const AttributeSet&atts,ResTable::Theme* theme);
    void onColorsChanged();
    static int modulateColor(int baseColor, float alphaMod, float lStar);
public:
    // AOSP marks the default ctor private ("Not publicly instantiable"); kept
    // public so createFromXmlInner can std::make_shared it.
    ColorStateList();
    ColorStateList(const ColorStateList&other);
    ColorStateList(const std::vector<std::vector<int>>&states,const std::vector<int>&colors);
    ~ColorStateList()override;
    int getDefaultColor()const override;
    bool canApplyTheme()const override;
    bool isOpaque()const;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const;
    cdroid::RefPtr<ColorStateList>withAlpha(int alpha)const;
    int getChangingConfigurations()const;
    int getColorForState(const std::vector<int>&stateSet, int defaultColor)const;
    const std::vector<std::vector<int>>& getStates()const;
    const std::vector<int>& getColors()const;
    bool hasState(int state)const ;
    std::string toString()const;
    static cdroid::RefPtr<ColorStateList> valueOf(int color);
    static cdroid::RefPtr<ColorStateList> createFromXml(const Resources& r,XmlPullParser& parser);
    static cdroid::RefPtr<ColorStateList> createFromXml(const Resources& r,XmlPullParser& parser,ResTable::Theme* theme);
    static cdroid::RefPtr<ColorStateList> createFromXmlInner(const Resources& r,XmlPullParser& parser,const AttributeSet& attrs,ResTable::Theme* theme);
};
}
#endif
