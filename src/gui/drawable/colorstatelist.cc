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
#include <drawable/colorstatelist.h>
#include <drawable/stateset.h>
#include <core/color.h>
#include <core/app.h>
#include <core/resources.h>
#include <core/sparsearray.h>
#include <core/xmlpullparser.h>
#include <attributeset.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <algorithm>
#include <stdexcept>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

std::vector<std::vector<int>> ColorStateList::EMPTY={{}};

ColorStateList::ColorStateList(){
    mChangingConfigurations=0;
}

ColorStateList::ColorStateList(const ColorStateList&other)
	:ColorStateList(other.mStateSpecs,other.mColors){
    mIsOpaque = other.mIsOpaque;
}

ColorStateList::ColorStateList(const std::vector<std::vector<int>>&states,const std::vector<int>&colors){
    mStateSpecs = states;
    mColors = colors;
    mChangingConfigurations = 0;
    onColorsChanged();
}

ColorStateList::~ColorStateList(){
    LOGV("%p",this);
}

int ColorStateList::getChangingConfigurations()const{
    return mChangingConfigurations;
}

int ColorStateList::modulateColor(int baseColor, float alphaMod, float lStar){
    const bool validLStar = lStar >= 0.0f && lStar <= 100.0f;
    if (alphaMod == 1.0f && !validLStar)  return baseColor;

    const int baseAlpha = Color::alpha(baseColor);
    const int alpha = std::max(0, std::min(255, (int)(baseAlpha * alphaMod + 0.5f)));

    if (validLStar) {
        // DEFERRED: AOSP uses android.graphics.cam.Cam (ColorUtils.colorToCAM/
        // CAMToColor) to set perceptual luminance; Cam is not ported to CDROID,
        // so lStar modulation is a no-op until Cam lands.
    }
    return (baseColor & 0xFFFFFF) | ((uint32_t)alpha << 24);
}

cdroid::RefPtr<ColorStateList>ColorStateList::withAlpha(int alpha)const{
    std::vector<int>colors = mColors;
    for(int i = 0 ; i < (int)colors.size();i++)
        colors[i] = (colors[i] & 0x00FFFFFF) | ( alpha & 0xFF000000 );
    return std::make_shared<ColorStateList>(mStateSpecs,colors);
}

void ColorStateList::inflate(const Resources&r,XmlPullParser& parser,const AttributeSet&attrs,ResTable::Theme* theme){
    // AOSP private inflate(Resources, XmlPullParser, AttributeSet, Theme): resolve
    // each <item> through the arsc via obtainStyledAttributes(R.styleable.
    // ColorStateListItem) and walk the raw AttributeSet by attribute resource id
    // to collect state specifiers -- no string attribute lookup.
    // NOTE: `theme` is threaded for AOSP arity but currently unused --
    // Resources::obtainStyledAttributes(Theme) is not ported (DEFERRED).
    (void)theme;

    const int innerDepth = parser.getDepth()+1;
    int depth, type;

    int defaultColor = (int)DEFAULT_COLOR;

    while ((type = parser.next()) != XmlPullParser::END_DOCUMENT
           && ((depth = parser.getDepth()) >= innerDepth || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG || depth > innerDepth
                || parser.getName().compare("item")) {
            continue;
        }

        // AOSP: Resources.obtainAttributes(r, theme, attrs, R.styleable.ColorStateListItem).
        auto a = r.obtainStyledAttributes(attrs, R::styleable::ColorStateListItem);
        const int baseColor = (int)a->getColor(R::styleable::ColorStateListItem_color, Color::MAGENTA);
        const float alphaMod = a->getFloat(R::styleable::ColorStateListItem_alpha, 1.0f);
        const float lStar = a->getFloat(R::styleable::ColorStateListItem_lStar, -1.0f);

        // Parse all unrecognized attributes as state specifiers (AOSP inflate).
        const int numAttrs = (int)attrs.getAttributeCount();
        std::vector<int>stateSpec;
        for (int i = 0; i < numAttrs; i++) {
            const int stateResId = attrs.getAttributeNameResource(i);
            if (stateResId == R::attr::lStar) {
                continue;
            }
            switch (stateResId) {
            case R::attr::color:
            case R::attr::alpha:
                // Recognized item attribute, ignore.
                break;
            default:
                stateSpec.push_back(attrs.getAttributeBooleanValue(i, false)
                        ? stateResId : -stateResId);
            }
        }

        // Apply alpha/lStar modulation. lStar is a no-op until Cam lands
        // (see modulateColor); the defaults leave enough information to
        // modulate again later.
        const int color = modulateColor(baseColor, alphaMod, lStar);
        if (mColors.size() == 0 || stateSpec.size() == 0) {
            defaultColor = color;
        }
        mStateSpecs.push_back(stateSpec);
        mColors.push_back(color);
    }

    mDefaultColor = defaultColor;

    onColorsChanged();
}

cdroid::RefPtr<ColorStateList> ColorStateList::createFromXml(const Resources& r,XmlPullParser& parser){
    return createFromXml(r, parser, nullptr);
}

cdroid::RefPtr<ColorStateList> ColorStateList::createFromXml(const Resources& r,XmlPullParser& parser,ResTable::Theme* theme){
    const AttributeSet& attrs = parser; // AOSP Xml.asAttributeSet(parser)
    int type;
    while ((type = parser.next()) != XmlPullParser::START_TAG
             && type != XmlPullParser::END_DOCUMENT) {
        // Seek parser to start tag.
    }
    if (type != XmlPullParser::START_TAG) {
        throw std::runtime_error("No start tag found");
    }
    return createFromXmlInner(r, parser, attrs, theme);
}

cdroid::RefPtr<ColorStateList> ColorStateList::createFromXmlInner(const Resources& r,XmlPullParser& parser,const AttributeSet& attrs,ResTable::Theme* theme){
    const std::string name = parser.getName();
    if (name.compare("selector")) {
        throw std::runtime_error(parser.getPositionDescription()
                + ": invalid color state list tag " + name);
    }
    auto colorStateList = std::make_shared<ColorStateList>();
    colorStateList->inflate(r, parser, attrs, theme);
    return colorStateList;
}

bool ColorStateList::canApplyTheme()const{
    // AOSP: mThemeAttrs != null. Theme-attribute preloading (mThemeAttrs) is not
    // ported to CDROID, so a ColorStateList never carries unresolved theme attrs.
    return false;
}

bool ColorStateList::isOpaque()const{
    return mIsOpaque;
}

bool ColorStateList::isStateful()const{
    return  mStateSpecs.size()&&mStateSpecs[0].size();
}

bool ColorStateList::hasFocusStateSpecified()const{
    return StateSet::containsAttribute(mStateSpecs,(int)cdroid::internal::R::attr::state_focused);
}

int ColorStateList::getDefaultColor()const{
    return mDefaultColor;
}

const std::vector<std::vector<int>>& ColorStateList::getStates()const{
    return mStateSpecs;
}

void ColorStateList::onColorsChanged(){
    int defaultColor = (int)DEFAULT_COLOR;
    bool isOpaque = true;
    const int N=(int)mStateSpecs.size();
    if ( N> 0) {
        defaultColor = mColors[0];

        for (int i = N - 1; i > 0; i--) {
            if (mStateSpecs[i].size() == 0) {
                defaultColor = mColors[i];
                break;
            }
        }

        for (int i = 0; i < N; i++) {
            if (Color::alpha(mColors[i]) != 0xFF) {
                isOpaque = false;
                break;
            }
        }
    }else if (mColors.size()){
        defaultColor = mColors[0];
    }
    mDefaultColor = defaultColor;
    mIsOpaque = isOpaque;
}

int ColorStateList::getColorForState(const std::vector<int>&stateSet, int defaultColor)const{
    const int setLength = (int)mStateSpecs.size();
    for (int i = 0; i < setLength; i++) {
        if (StateSet::stateSetMatches(mStateSpecs[i], stateSet)) {
            return mColors[i];
        }
    }
    return defaultColor;
}

cdroid::RefPtr<ColorStateList> ColorStateList::valueOf(int color){
    static SparseArray<std::weak_ptr<ColorStateList>>sCache;
    const int index = sCache.indexOfKey(color);
    if(index >= 0){
        auto cls = sCache.valueAt(index).lock();
        if(cls) return cls;
        sCache.removeAt(index);
    }
    auto cls = std::make_shared<ColorStateList>(EMPTY, std::vector<int>{color});
    sCache.put(color,cls);
    return cls;
}

const std::vector<int>& ColorStateList::getColors()const{
    return mColors;
}

bool ColorStateList::hasState(int state)const{
    for(auto ss:mStateSpecs){
        for(auto s:ss){
            if(s==state||s==-state)return true;
        }
    }
    return false;
}

std::string ColorStateList::toString()const{
    // AOSP toString (mThemeAttrs omitted -- not ported).
    std::ostringstream oss;
    oss << "ColorStateList{mChangingConfigurations=" << mChangingConfigurations
        << " mStateSpecs=[";
    for (size_t i = 0; i < mStateSpecs.size(); i++) {
        oss << "[";
        for (size_t j = 0; j < mStateSpecs[i].size(); j++) {
            oss << mStateSpecs[i][j];
            if (j + 1 < mStateSpecs[i].size()) oss << ",";
        }
        oss << "]";
        if (i + 1 < mStateSpecs.size()) oss << ",";
    }
    oss << "] mColors=[";
    for (size_t i = 0; i < mColors.size(); i++) {
        oss << std::hex << (unsigned int)mColors[i];
        if (i + 1 < mColors.size()) oss << ",";
    }
    oss << "] mDefaultColor=" << std::hex << (unsigned int)mDefaultColor << "}";
    return oss.str();
}

}
