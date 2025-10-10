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
#include <drawables/drawable.h>
#include <porting/cdlog.h>
#include <core/windowmanager.h>
#include <drawables/drawableinflater.h>

using namespace Cairo;
namespace cdroid {

Drawable::ConstantState::~ConstantState() {
}

Drawable::Drawable() {
    mLevel = 0;
    mChangingConfigurations = 0;
    mVisible = true;
    mLayoutDirection = LayoutDirection::LTR;
    mCallback = nullptr;
    mBounds.set(0,0,0,0);
    mColorFilter = nullptr;
}

Drawable::~Drawable() {
    delete mColorFilter;
}

void Drawable::setBounds(const Rect&r) {
    setBounds(r.left,r.top,r.width,r.height);
}

void Drawable::setBounds(int x,int y,int w,int h) {
    if((mBounds.left!=x)||(mBounds.top!=y)||(mBounds.width!=w)||(mBounds.height!=h)) {
        if(!mBounds.empty())
            invalidateSelf();
        mBounds.set(x,y,w,h);
        onBoundsChange(mBounds);
    }
}

bool Drawable::getPadding(Rect&padding) {
    padding.set(0,0,0,0);
    return false;
}

Insets Drawable::getOpticalInsets() {
    return Insets();
}

void Drawable::getOutline(Outline&outline){
    outline.setRect(getBounds());
    outline.setAlpha(0);
}

const Rect&Drawable::getBounds()const {
    return mBounds;
}

void Drawable::copyBounds(Rect&bounds)const{
    bounds = mBounds;
}

Rect Drawable::getDirtyBounds() const{
    return mBounds;
}

Drawable*Drawable::mutate() {
    return this;
}

void Drawable::clearMutated() {
}

void Drawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    mVisible = atts.getBoolean("visible", mVisible);
}

void Drawable::inflateWithAttributes(XmlPullParser&parser,const AttributeSet&atts){
    mVisible = atts.getBoolean("visible",mVisible);
}

Drawable* Drawable::createFromXmlInner(XmlPullParser&parser,const AttributeSet&atts){
    return DrawableInflater::inflateFromXml(parser.getName(),parser,atts);
}

Drawable* Drawable::createFromXmlInnerForDensity(XmlPullParser&parser,const AttributeSet&atts,int){
    return DrawableInflater::inflateFromXml(parser.getName(),parser,atts);
}

/*int Drawable::getDimensionOrFraction(const std::string&value,int base,int def){
    if(value.find("%")!=std::string::npos){
    LOGD("%d %s[%.f]=%.2f",base,value.c_str(),std::stof(value),base*std::stof(value)/100);
        return base*std::stof(value)/100;
    }else if(value.find("px")!=std::string::npos){
        return std::stoi(value);
    }
    return def;
}*/

int Drawable::getOpacity() {
    return UNKNOWN;
}

void Drawable::setHotspot(float x,float y) {
}

void Drawable::setHotspotBounds(int left,int top,int width,int height) {
}

void Drawable::getHotspotBounds(Rect&outRect)const{
    outRect = mBounds;
}

bool Drawable::isProjected()const{
    return false;
}

std::shared_ptr<Drawable::ConstantState>Drawable::getConstantState() {
    return nullptr;
}

void Drawable::setAutoMirrored(bool mirrored) {
}

bool Drawable::isAutoMirrored() const{
    return false;
}

void Drawable::setDither(bool){
}

void Drawable::setFilterBitmap(bool filter){
}

bool Drawable::isFilterBitmap()const{
    return false;
}
void Drawable::setColorFilter(ColorFilter*cf) {
    delete mColorFilter;
    mColorFilter = cf;
    invalidateSelf();
    LOGV("setColorFilter %p:%p",this,cf);
}

ColorFilter*Drawable::getColorFilter(){
    return nullptr;
}

void Drawable::clearColorFilter(){
    setColorFilter(nullptr);
}

void Drawable::setColorFilter(int color,PorterDuffMode mode) {
    setColorFilter(new PorterDuffColorFilter(color,mode));
}

void Drawable::setTint(int color) {
    setTintList(ColorStateList::valueOf(color));
}

PorterDuffColorFilter *Drawable::updateTintFilter(PorterDuffColorFilter* tintFilter,const ColorStateList* tint,int tintMode) {
    if ( (tint == nullptr) || (tintMode == PorterDuff::Mode::NOOP) ) {
        return nullptr;
    }

    const int color = tint->getColorForState(getState(), Color::TRANSPARENT);
    if (tintFilter == nullptr) {
        return new PorterDuffColorFilter(color, tintMode);
    }

    tintFilter->setColor(color);
    tintFilter->setMode(tintMode);
    return tintFilter;
}

void Drawable::setTintList(const ColorStateList* tint) {
}

void Drawable::setTintMode(int mode) {
}

void Drawable::setSrcDensityOverride(int density){
    mSrcDensityOverride = density;
}

bool Drawable::isStateful()const {
    return false;
}

bool Drawable::hasFocusStateSpecified()const {
    return false;
}

bool Drawable::setState(const std::vector<int>&states) {
    mStateSet=states;
    return onStateChange(states);
}

const std::vector<int>& Drawable::getState()const {
    return mStateSet;
}

bool Drawable::setLevel(int level) {
    if(mLevel!=level) {
        mLevel=level;
        return onLevelChange(level);
    }
    return false;
}

int Drawable::getMinimumWidth() {
    const int intrinsicWidth = getIntrinsicWidth();
    return intrinsicWidth > 0 ? intrinsicWidth : 0;
}

int Drawable::getMinimumHeight() {
    const int intrinsicHeight = getIntrinsicHeight();
    return intrinsicHeight > 0 ? intrinsicHeight : 0;
}

bool Drawable::setLayoutDirection (int dir) {
    if (mLayoutDirection != dir) {
        mLayoutDirection = dir;
        return onLayoutDirectionChanged(dir);
    }
    return false;
}

int Drawable::getLayoutDirection()const {
    return mLayoutDirection;
}

int Drawable::getIntrinsicWidth() {
    return -1;
}

int Drawable::getIntrinsicHeight() {
    return -1;
}

bool Drawable::isVisible()const {
    return mVisible;
}

bool Drawable::setVisible(bool visible, bool restart) {
    const bool changed = mVisible != visible;
    if (changed) {
        mVisible = visible;
        invalidateSelf();
    }
    return changed;
}

int Drawable::getChangingConfigurations()const {
    return mChangingConfigurations;
}

void Drawable::setChangingConfigurations(int configs) {
    mChangingConfigurations =configs;
}

void Drawable::setCallback(Drawable::Callback*cbk) {
    mCallback = cbk;
}

Drawable::Callback* Drawable::getCallback()const {
    return mCallback;
}

void Drawable::scheduleSelf(const Runnable& what, int64_t when) {
    if(mCallback)mCallback->scheduleDrawable(*this, what, when);
}

void Drawable::unscheduleSelf(const Runnable& what) {
    if(mCallback)mCallback->unscheduleDrawable(*this, what);
}

void Drawable::invalidateSelf() {
    if(mCallback)mCallback->invalidateDrawable(*this);
}

void Drawable::jumpToCurrentState() {
}

Drawable*Drawable::getCurrent() {
    return this;
}

Cairo::RefPtr<Cairo::Region>Drawable::getTransparentRegion(){
    return nullptr;
}

int Drawable::resolveOpacity(int op1,int op2){
    if (op1 == op2) {
        return op1;
    }
    if (op1 == PixelFormat::UNKNOWN || op2 == PixelFormat::UNKNOWN) {
        return PixelFormat::UNKNOWN;
    }
    if (op1 == PixelFormat::TRANSLUCENT || op2 == PixelFormat::TRANSLUCENT) {
        return PixelFormat::TRANSLUCENT;
    }
    if (op1 == PixelFormat::TRANSPARENT || op2 == PixelFormat::TRANSPARENT) {
        return PixelFormat::TRANSPARENT;
    }
    return PixelFormat::OPAQUE;
}

int Drawable::resolveDensity(int parentDensity){
    DisplayMetrics metrics;
    WindowManager::getInstance().getDefaultDisplay().getMetrics(metrics);
    const int densityDpi = /*r == null ? parentDensity :*/metrics.densityDpi;
    return densityDpi == 0 ? DisplayMetrics::DENSITY_DEFAULT : densityDpi;
}

PorterDuff::Mode Drawable::parseTintMode(int value, PorterDuff::Mode defaultMode) {
    switch (value) {
    case 3: return PorterDuff::Mode::SRC_OVER;
    case 5: return PorterDuff::Mode::SRC_IN;
    case 9: return PorterDuff::Mode::SRC_ATOP;
    case 14: return PorterDuff::Mode::MULTIPLY;
    case 15: return PorterDuff::Mode::SCREEN;
    case 16: return PorterDuff::Mode::ADD;
    default: return defaultMode;
    }
}

float Drawable::scaleFromDensity(float pixels, int sourceDensity, int targetDensity) {
    return pixels * targetDensity / sourceDensity;
}

int Drawable::scaleFromDensity(int pixels, int sourceDensity, int targetDensity, bool isSize) {
    if (pixels == 0 || sourceDensity == targetDensity)
        return pixels;

    const float result = pixels * targetDensity / (float) sourceDensity;
    if (!isSize)  return (int) result;

    const int rounded = round(result);
    if (rounded != 0)     return rounded;
    else if (pixels > 0) return 1;
    else return -1;
}

}
