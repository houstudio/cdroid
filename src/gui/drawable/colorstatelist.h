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
#include <core/complexcolor.h>
#include <core/xmlpullparser.h>

namespace cdroid{

class ColorStateList:public ComplexColor{
private:
    static constexpr int DEFAULT_COLOR = 0xFFFF0000;
    static std::vector<std::vector<int>>EMPTY;
    int mDefaultColor;
    bool mIsOpaque;
    int mChangingConfigurations;
    std::vector<int>mColors;
    std::vector<std::vector<int>>mStateSpecs;
private:
    void inflate(XmlPullParser& parser,const AttributeSet&atts);
    void onColorsChanged();
    int modulateColorAlpha(int baseColor, float alphaMod)const;
protected:
    static ColorStateList* createFromXmlInner(XmlPullParser& parser,const AttributeSet& attrs);
public:
    ColorStateList();
    ColorStateList(int color);
    ColorStateList(const ColorStateList&other);
    ColorStateList(const std::vector<std::vector<int>>&states,const std::vector<int>&colors);
    ~ColorStateList();
    int addStateColor(const std::vector<int>&stateSet,int color);
    int addStateColor(cdroid::Context*,const AttributeSet&);
    int getDefaultColor()const override;
    bool isOpaque()const;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const;
    ColorStateList*withAlpha(int alpha)const;
    ColorStateList&operator=(const ColorStateList&other);
    bool operator!=(const ColorStateList&other)const;
    bool operator==(const ColorStateList&other)const;
    int getChangingConfigurations()const;
    int getColorForState(const std::vector<int>&stateSet, int defaultColor)const;
    const std::vector<std::vector<int>>& getStates()const;
    const std::vector<int>& getColors()const;
    bool hasState(int state)const ;
    void dump()const;
    static ColorStateList* valueOf(int color);
    static ColorStateList* createFromXml(XmlPullParser& parser);
    static ColorStateList* inflate(Context*ctx,const std::string&resname);
};

}
#endif
