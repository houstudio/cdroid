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
#include <core/xmlpullparser.h>
#include <attributeset.h>
#include <exception>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>

namespace cdroid{

std::vector<std::vector<int>> ColorStateList::EMPTY={{}};

ColorStateList::ColorStateList(){
    mChangingConfigurations=0;
}

ColorStateList::ColorStateList(int color)
    :ColorStateList(EMPTY,{color}){
}

int ColorStateList::addStateColor(cdroid::Context*ctx,const AttributeSet&atts){
    std::vector<int>states;
    int color = atts.getColor("color",0);
    int alpha = 0;
    if(atts.hasAttribute("alpha")){
        alpha = atts.getInt("alpha",255);
        color&=0xFFFFFF;
    }
    StateSet::parseState(states,atts);
    return addStateColor(states,color|(alpha<<24));
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

void ColorStateList::dump()const{
    std::ostringstream oss;
    for(int i=0;i<mColors.size();i++){
	const std::vector<int>&state=mStateSpecs[i];
        oss<<"[";
        for(auto s=state.begin();s!=state.end();s++){
	    oss<<*s;
	    if(s<state.end()-1)oss<<",";
	}
        oss<<"]="<<std::hex<<(unsigned int)mColors.at(i)<<" ";
    }
    LOG(DEBUG)<<oss.str();
}

int ColorStateList::getChangingConfigurations()const{
    return mChangingConfigurations;
}

int ColorStateList::addStateColor(const std::vector<int>&stateSet,int color){
    mStateSpecs.push_back(stateSet);
    mColors.push_back(color);
    return mColors.size()-1;
}

int ColorStateList::modulateColorAlpha(int baseColor, float alphaMod)const{
    if (alphaMod == 1.0f)  return baseColor;

    const int baseAlpha = Color::alpha(baseColor);
    const int alpha = (int)(baseAlpha * alphaMod + 0.5f)&0xFF;
    //MathUtils::constrain((int) (baseAlpha * alphaMod + 0.5f), 0, 255);
    return (baseColor & 0xFFFFFF) | ((unsigned int)alpha << 24);    
}

ColorStateList*ColorStateList::withAlpha(int alpha)const{
    std::vector<int>colors = mColors;
    for(int i = 0 ; i < colors.size();i++)
        colors[i] = (colors[i] & 0x00FFFFFF) | ( alpha & 0xFF000000 );
    return new ColorStateList(mStateSpecs,colors);
}

void ColorStateList::inflate(XmlPullParser& parser,const AttributeSet&attrs){
    const int innerDepth = parser.getDepth()+1;
    int depth, type;

    int changingConfigurations = 0;
    int defaultColor = DEFAULT_COLOR;

    bool hasUnresolvedAttrs = false;

    while ((type = parser.next()) != XmlPullParser::END_DOCUMENT
           && ((depth = parser.getDepth()) >= innerDepth || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG || depth > innerDepth
                || parser.getName().compare("item")) {
            continue;
        }
        //changingConfigurations |= a.getChangingConfigurations();

        // Parse all unrecognized attributes as state specifiers.
        const int baseColor = attrs.getColor("color",Color::MAGENTA);
        const float alphaMod = attrs.getFloat("alpha",1.f);
    
        std::vector<int>stateSpec;
        StateSet::parseState(stateSpec,attrs);

        // Apply alpha modulation. If we couldn't resolve the color or
        // alpha yet, the default values leave us enough information to
        // modulate again during applyTheme().
        const int color = modulateColorAlpha(baseColor, alphaMod);
        if (mColors.size() == 0 || mStateSpecs.size() == 0) {
            defaultColor = color;
        }
        addStateColor(stateSpec,color);
    }
    
    mDefaultColor = defaultColor;

    onColorsChanged();
}

ColorStateList* ColorStateList::createFromXmlInner(XmlPullParser& parser,const AttributeSet& attrs){
    const std::string name = parser.getName();
    if (name.compare("selector")) {
        LOGE("invalid color state list tag %s" ,name.c_str());
    }

    ColorStateList* colorStateList = new ColorStateList(DEFAULT_COLOR);
    colorStateList->inflate(parser, attrs);
    return colorStateList;
}

ColorStateList* ColorStateList::createFromXml(XmlPullParser& parser) {
    int type;
    const AttributeSet& attrs = parser;
    while ((type = parser.next()) != XmlPullParser::START_TAG
               && type != XmlPullParser::END_DOCUMENT) {
        // Seek parser to start tag.
    }

    if (type != XmlPullParser::START_TAG) {
        throw std::logic_error("No start tag found");
    }

    return createFromXmlInner(parser, attrs);
}

ColorStateList&ColorStateList::operator=(const ColorStateList&other){
    mStateSpecs = other.mStateSpecs;
    mColors = other.mColors;
    mIsOpaque = other.mIsOpaque;
    onColorsChanged();
    return *this;
}

bool ColorStateList::operator!=(const ColorStateList&other)const{
    return (mColors != other.mColors) || (mStateSpecs != other.mStateSpecs);
}

bool ColorStateList::operator==(const ColorStateList&other)const{
    return (mColors == other.mColors) && (mStateSpecs != other.mStateSpecs);
}

bool ColorStateList::isOpaque()const{
    return mIsOpaque;
}

bool ColorStateList::isStateful()const{
    return  mStateSpecs.size()&&mStateSpecs[0].size();
}

bool ColorStateList::hasFocusStateSpecified()const{
    return StateSet::containsAttribute(mStateSpecs,StateSet::FOCUSED);
}

int ColorStateList::getDefaultColor()const{
    return mDefaultColor;
}

const std::vector<std::vector<int>>& ColorStateList::getStates()const{
    return mStateSpecs;
}

void ColorStateList::onColorsChanged(){
    int defaultColor = DEFAULT_COLOR;
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

ColorStateList*ColorStateList::valueOf(int color){
    char buf[32];
    sprintf(buf,"#%06x",color);
    return App::getInstance().getColorStateList(buf); 
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

ColorStateList*ColorStateList::inflate(Context*ctx,const std::string&resname){
    XmlPullParser parser(ctx,resname);
    int type;
    ColorStateList *colorStateList = nullptr;
    const int depth = parser.getDepth();
    const AttributeSet& atts = parser;
    if(resname.size()&&(resname[0]=='#'||(resname.find("/")!=std::string::npos))){
        const int color = Color::parseColor(resname);
        return new ColorStateList(color);
    }

    while((type=parser.next())!=XmlPullParser::END_DOCUMENT){
        const std::string tagName = parser.getName();
        if((type!=XmlPullParser::START_TAG)||tagName.compare("item")){
            continue;
        }
        std::vector<int>states;
        const int color = atts.getColor("color");
        StateSet::parseState(states,atts);
        if(colorStateList==nullptr)
            colorStateList = new ColorStateList();
        colorStateList->addStateColor(states,color);
    }
    return colorStateList;
}

}
