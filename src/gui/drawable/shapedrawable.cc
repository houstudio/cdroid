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
#include <porting/cdlog.h>
#include <drawable/shapedrawable.h>
#include <widget/framework_styleable.h>
namespace cdroid{

ShapeDrawable::ShapeState::ShapeState(){
    mChangingConfigurations = 0;
    mAlpha= 255;
    mShape= nullptr;
    mTint = nullptr;
    mTintMode = PorterDuff::Mode::NOOP;
    mIntrinsicWidth = 0;
    mIntrinsicHeight= 0;
    mPadding.set(0,0,0,0);
}

ShapeDrawable::ShapeState::ShapeState(const ShapeState&orig)
      :ShapeDrawable::ShapeState::ShapeState(){
    // androidx ShapeState copy ctor (ShapeDrawable.java:565): propagate the cached config so a
    // copy-on-write clone made by mutate() preserves the snapshot taken by getConstantState().
    mChangingConfigurations = orig.mChangingConfigurations;
    mIntrinsicWidth = orig.mIntrinsicWidth;
    mIntrinsicHeight= orig.mIntrinsicHeight;
    mPadding= orig.mPadding;
    mAlpha  = orig.mAlpha;
    if(orig.mShape)
        mShape  = orig.mShape->clone();
    if(orig.mTint)
        mTint = orig.mTint;
    mTintMode = orig.mTintMode;
}

ShapeDrawable* ShapeDrawable::ShapeState::newDrawable(){
    return new ShapeDrawable(shared_from_this());
}

ShapeDrawable::ShapeState::~ShapeState(){
    delete mShape;
}

int ShapeDrawable::ShapeState::getChangingConfigurations()const{
    return mChangingConfigurations;
}

////////////////////////////////////////////////////////////////////////////////////////////////

ShapeDrawable::ShapeDrawable(std::shared_ptr<ShapeState>state){
    mShapeState = state;
    mMutated = false;
    mTintFilter = nullptr;
}

ShapeDrawable::ShapeDrawable(){
    mShapeState = std::make_shared<ShapeState>();
    mMutated = false;
    mTintFilter = nullptr;
}

ShapeDrawable::~ShapeDrawable(){
}

void ShapeDrawable::getOutline(Outline& outline) {
    if (mShapeState->mShape != nullptr) {
        mShapeState->mShape->getOutline(outline);
        outline.setAlpha(getAlpha() / 255.0f);
    }
}

std::shared_ptr<Drawable::ConstantState>ShapeDrawable::getConstantState(){
    // androidx ShapeDrawable.getConstantState (line 512): sync the drawable's config into the state.
    mShapeState->mChangingConfigurations = getChangingConfigurations();
    return mShapeState;
}

int ShapeDrawable::getChangingConfigurations()const{
    // androidx ShapeDrawable.getChangingConfigurations (line 268): OR the state's config in.
    return Drawable::getChangingConfigurations() | mShapeState->getChangingConfigurations();
}

void ShapeDrawable::setShape(Shape*shape){
    if(mShapeState->mShape)
       delete mShapeState->mShape;
    mShapeState->mShape = shape;
    updateShape();
}

void ShapeDrawable::onBoundsChange(const Rect&bounds){
    Drawable::onBoundsChange(bounds);
    updateShape();
}

bool ShapeDrawable::onStateChange(const std::vector<int>&stateset){
    if(mShapeState->mTint && mShapeState->mTintMode != PorterDuff::Mode::NOOP){
        mTintFilter = updateTintFilter(mTintFilter,mShapeState->mTint,mShapeState->mTintMode);
        return true;
    }
    return false;
}

bool ShapeDrawable::isStateful()const{
    return Drawable::isStateful()||(mShapeState->mTint&&mShapeState->mTint->isStateful());
}

bool ShapeDrawable::hasFocusStateSpecified()const{
    return mShapeState->mTint&&mShapeState->mTint->hasFocusStateSpecified();
}

Shape*ShapeDrawable::getShape()const{
    return mShapeState->mShape;
}

void ShapeDrawable::updateShape(){
    if (mShapeState->mShape != nullptr) {
        const Rect& r = getBounds();
        mShapeState->mShape->resize(r.width,r.height);
    }
    invalidateSelf();
}

bool ShapeDrawable::getPadding(Rect&padding){
    padding=mShapeState->mPadding;
    return true;
}

void ShapeDrawable::setPadding(const Rect& padding){
    mShapeState->mPadding = padding;
}

void ShapeDrawable::setPadding(int left, int top, int right, int bottom){
    if ((left | top | right | bottom) == 0) {
        mShapeState->mPadding.set(0,0,0,0);
    } else {
        mShapeState->mPadding.set(left, top, right, bottom);
    }
}

void ShapeDrawable::setAlpha(int alpha){
    mShapeState->mAlpha = alpha;
    invalidateSelf();
}

int ShapeDrawable::ShapeDrawable::getAlpha()const{
    return mShapeState->mAlpha;
}

int ShapeDrawable::getOpacity()const{
    switch(mShapeState->mAlpha){
    case 255: return PixelFormat::OPAQUE;
    case   0: return PixelFormat::TRANSPARENT;
    default : return PixelFormat::TRANSLUCENT; 
    }
}

void ShapeDrawable::setTintList(const cdroid::RefPtr<ColorStateList>&tint){
    if( mShapeState->mTint!=tint ){
        mShapeState->mTint = tint;
        mTintFilter = updateTintFilter(mTintFilter,tint,mShapeState->mTintMode); 
        invalidateSelf();
    }
}

void ShapeDrawable::setTintMode(int tintMode){
    mShapeState->mTintMode = tintMode;
    mTintFilter= updateTintFilter(mTintFilter,mShapeState->mTint,tintMode);
    invalidateSelf();
}

void ShapeDrawable::setColorFilter(const cdroid::RefPtr<ColorFilter>&colorFilter){
    Drawable::setColorFilter(colorFilter);  // androidx stores the filter; getColorFilter() reads it
    invalidateSelf();
}

int ShapeDrawable::getIntrinsicWidth()const{
    return mShapeState->mIntrinsicWidth;
}

int ShapeDrawable::getIntrinsicHeight()const{
    return mShapeState->mIntrinsicHeight;
}

void ShapeDrawable::setIntrinsicWidth(int width){
    mShapeState->mIntrinsicWidth = width;
    invalidateSelf();
}

void ShapeDrawable::setIntrinsicHeight(int height){
    mShapeState->mIntrinsicHeight = height;
    invalidateSelf();
}

void ShapeDrawable::updateLocalState(){
    mTintFilter = updateTintFilter(mTintFilter, mShapeState->mTint, mShapeState->mTintMode);
}

ShapeDrawable*ShapeDrawable::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        mShapeState = std::make_shared<ShapeState>(*mShapeState);
        updateLocalState();
        mMutated = true;
    }
    return this;
}

void ShapeDrawable::clearMutated(){
    Drawable::clearMutated();
    mMutated = false;
}

void ShapeDrawable::draw(Canvas&canvas){
    const Rect&r = getBounds();
    if(mShapeState->mShape!=nullptr){
        canvas.translate(r.left,r.top);
        ColorFilter* tintFilter = beginTintGroup(canvas, Rect{0,0,r.width,r.height}, mTintFilter.get());
        mShapeState->mShape->draw(canvas,r.left,r.top);
        if(tintFilter) endTintGroup(canvas, r, tintFilter);
        canvas.translate(-r.left,-r.top);
    }
}

int ShapeDrawable::inflateTag(const std::string&name,XmlPullParser&parser,const AttributeSet&a){
    if (name.compare("padding")==0) {
        // AOSP ShapeDrawable.inflateTag: r.obtainAttributes(attrs, R.styleable.ShapeDrawablePadding).
        namespace SP = styleable::ShapeDrawablePadding;
        Context* ctx = a.getContext();
        auto ta = ctx ? ctx->obtainStyledAttributes(a, SP::IDS) : nullptr;
        if (ta) {
            setPadding(ta->getDimensionPixelOffset(SP::left, 0),
                    ta->getDimensionPixelOffset(SP::top, 0),
                    ta->getDimensionPixelOffset(SP::right, 0),
                    ta->getDimensionPixelOffset(SP::bottom, 0));
        }
        return true;
    }
    return false;
}


void ShapeDrawable::inflate(Resources& r,XmlPullParser&parser,const AttributeSet&atts){
    Drawable::inflate(r,parser,atts);

    // AOSP ShapeDrawable.inflate: obtainAttributes(R.styleable.ShapeDrawable) -> updateStateFromTypedArray.
    Context* ctx = atts.getContext();
    auto ta = r.obtainStyledAttributes(&atts, styleable::ShapeDrawable::IDS);
    if (ta) updateStateFromTypedArray(*ta);

    int type;
    const int outerDepth = parser.getDepth();
    while (((type = parser.next()) != XmlPullParser::END_DOCUMENT)
            && (type != XmlPullParser::END_TAG || parser.getDepth() > outerDepth)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
        // call our subclass
        if (!inflateTag(name,parser,atts)) {
            LOGW("Unknown element: %s for ShapeDrawable %p",name.c_str(),this);
        }
    }

    // Update local properties.
    updateLocalState();
}

void ShapeDrawable::updateStateFromTypedArray(const TypedArray& a) {
    // AOSP ShapeDrawable.updateStateFromTypedArray: index-based reads against R.styleable.ShapeDrawable.
    namespace SX = styleable::ShapeDrawable;
    auto state = mShapeState;

    // CDROID's ShapeState has no Paint (the Shape owns drawing), so the AOSP paint color/dither
    // pair reduces to just dither here. dither is read via the styleable index.
    state->mDither = a.getBoolean(SX::dither, state->mDither);

    state->mIntrinsicWidth = (int) a.getDimension(SX::width, state->mIntrinsicWidth);
    state->mIntrinsicHeight = (int) a.getDimension(SX::height, state->mIntrinsicHeight);

    // tintMode is a flag enum aapt2 pre-resolves to a PorterDuff::Mode value; getInt replaces the
    // AttributeSet::getTintMode string decoder. NOOP (-1) sentinel == "not specified".
    const int tintMode = a.getInt(SX::tintMode, PorterDuff::Mode::NOOP);
    if (tintMode != PorterDuff::Mode::NOOP) {
        state->mTintMode = tintMode;
    }

    auto tint = a.getColorStateList(SX::tint);
    if (tint != nullptr) {
        state->mTint = tint;
    }
}

}
