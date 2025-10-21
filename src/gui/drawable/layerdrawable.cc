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
#include <drawable/layerdrawable.h>
#include <drawable/ninepatchdrawable.h>
#include <cdlog.h>
#include <limits.h>

namespace cdroid{
#define INSET_UNDEFINED INT_MIN

LayerDrawable::ChildDrawable::ChildDrawable(int density){
    mDensity= density;
    mInsetL = mInsetT =0;
    mInsetR = mInsetB =0;
    mInsetS = mInsetE =INSET_UNDEFINED;
    mGravity= Gravity::NO_GRAVITY;
    mWidth  = mHeight = -1;
    mDrawable= nullptr;
    mId=-1;
}

LayerDrawable::ChildDrawable::ChildDrawable(ChildDrawable* orig,LayerDrawable*owner):ChildDrawable(160){
    Drawable*dr = orig->mDrawable;
    Drawable*clone=nullptr;
    if(dr){
        auto cs=dr->getConstantState();
        if(cs==nullptr){
            clone=dr;
            LOGW_IF((dr->getCallback()==nullptr),"Invalid drawable added to LayerDrawable Drawable already "
                            "belongs to another owner but does not expose a constant state");
        }else
            clone=cs->newDrawable();
        clone->setLayoutDirection(dr->getLayoutDirection());
        clone->setBounds(dr->getBounds());
        clone->setLevel(dr->getLevel());
        clone->setCallback(owner);
    }
    mDrawable=clone;
    mInsetL = orig->mInsetL;
    mInsetT = orig->mInsetT;
    mInsetR = orig->mInsetR;
    mInsetB = orig->mInsetB;
    mInsetS = orig->mInsetS;
    mInsetE = orig->mInsetE;
    mWidth  = orig->mWidth;
    mHeight = orig->mHeight;
    mGravity = orig->mGravity;
    mId = orig->mId;
}

LayerDrawable::ChildDrawable::~ChildDrawable(){
    delete mDrawable;
}

void LayerDrawable::ChildDrawable::setDensity(int targetDensity){
    if (mDensity != targetDensity) {
        const int sourceDensity = mDensity;
        mDensity = targetDensity;
        applyDensityScaling(sourceDensity, targetDensity);
    }       
}

void LayerDrawable::ChildDrawable::applyDensityScaling(int sourceDensity, int targetDensity){
    mInsetL = Drawable::scaleFromDensity(mInsetL, sourceDensity, targetDensity, false);
    mInsetT = Drawable::scaleFromDensity(mInsetT, sourceDensity, targetDensity, false);
    mInsetR = Drawable::scaleFromDensity(mInsetR, sourceDensity, targetDensity, false);
    mInsetB = Drawable::scaleFromDensity(mInsetB, sourceDensity, targetDensity, false);

    if (mInsetS != INSET_UNDEFINED) mInsetS = Drawable::scaleFromDensity(mInsetS,sourceDensity,targetDensity, false);
    if (mInsetE != INSET_UNDEFINED) mInsetE = Drawable::scaleFromDensity(mInsetE,sourceDensity,targetDensity, false);
    if (mWidth > 0)  mWidth = Drawable::scaleFromDensity(mWidth, sourceDensity, targetDensity,true);
    if (mHeight > 0) mHeight = Drawable::scaleFromDensity(mHeight, sourceDensity, targetDensity,true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LayerDrawable::LayerState::LayerState(){
#if 0
    if(attrs){
        mPaddingTop  = attrs->getInt("paddingTop");
        mPaddingLeft = attrs->getInt("paddingLeft");
        mPaddingRight= attrs->getInt("paddingRight");
        mPaddingBottom=attrs->getInt("paddingBottom");
        mPaddingStart =attrs->getInt("paddingStart");
        mPaddingEnd  = attrs->getInt("paddingEnd");
        mPaddingMode = attrs->getInt("paddingMode",std::unordered_map<std::string,int>{
	        {"nest",PADDING_MODE_NEST},
	        {"inner",PADDING_MODE_INNER},
	        {"stack",PADDING_MODE_STACK}
        },PADDING_MODE_NEST);
        mAutoMirrored= attrs->getBoolean("paddingMode");
    }else
#endif
    mDensity = -1;
    mPaddingTop  = mPaddingBottom= -1;
    mPaddingLeft = mPaddingRight = -1;
    mPaddingStart= mPaddingEnd   = -1;
    mPaddingMode = PADDING_MODE_NEST;

    mOpacity = 0xFF;
    mAutoMirrored= false;
    mCheckedOpacity  = false;
    mOpacityOverride = PixelFormat::UNKNOWN;
}

LayerDrawable::LayerState::LayerState(const LayerState*orig,LayerDrawable*owner):LayerState(){
    mDensity = Drawable::resolveDensity( orig ? orig->mDensity : 0);
    if (orig == nullptr) return;

    mChangingConfigurations = orig->mChangingConfigurations;
    mChildrenChangingConfigurations = orig->mChildrenChangingConfigurations;
    mChildren.reserve(orig->mChildren.size());
    for (auto child:orig->mChildren){
        mChildren.push_back(new ChildDrawable(child, owner));
    }
    mDensity = orig->mDensity;
    mCheckedOpacity = orig->mCheckedOpacity;
    mOpacity = orig->mOpacity;
    mCheckedStateful = orig->mCheckedStateful;
    mIsStateful = orig->mIsStateful;
    mAutoMirrored = orig->mAutoMirrored;
    mPaddingMode = orig->mPaddingMode;
    mThemeAttrs = orig->mThemeAttrs;
    mPaddingTop = orig->mPaddingTop;
    mPaddingBottom = orig->mPaddingBottom;
    mPaddingLeft = orig->mPaddingLeft;
    mPaddingRight = orig->mPaddingRight;
    mPaddingStart = orig->mPaddingStart;
    mPaddingEnd = orig->mPaddingEnd;
    mOpacityOverride = orig->mOpacityOverride;

    if (orig->mDensity != mDensity) 
        applyDensityScaling(orig->mDensity, mDensity);
}

LayerDrawable::LayerState::~LayerState(){
    for (auto it = mChildren.begin(); it != mChildren.end();it++)
       delete *it;
    mChildren.clear();
}

LayerDrawable*LayerDrawable::LayerState::newDrawable(){
    return new LayerDrawable(shared_from_this());
}

int LayerDrawable::LayerState::getChangingConfigurations()const{
   return mChangingConfigurations | mChildrenChangingConfigurations;
}

int LayerDrawable::LayerState::getOpacity(){
    if (mCheckedOpacity) {
         return mOpacity;
     }

     // Seek to the first non-null drawable.
     int firstIndex = -1;
     const size_t N = mChildren.size();
     for (int i = 0; i < N; i++) {
         if (mChildren[i]->mDrawable) {
             firstIndex = i;
             break;
         }
     }

     int op;
     if (firstIndex >= 0) {
         op = mChildren[firstIndex]->mDrawable->getOpacity();
     } else {
         op = PixelFormat::TRANSPARENT;
     }

     // Merge all remaining non-null drawables.
     for (int i = firstIndex + 1; i < N; i++) {
         Drawable* dr = mChildren[i]->mDrawable;
         if (dr != nullptr) {
             op = Drawable::resolveOpacity(op, dr->getOpacity());
         }
     }

     mOpacity = op;
     mCheckedOpacity = true;
     return op;
}

bool LayerDrawable::LayerState::isStateful()const{
    bool isStateful = false;
    for (auto child:mChildren) {
        Drawable*dr = child->mDrawable;
        if (dr && dr->isStateful()) {
            isStateful = true;
            break;
        }
    }
    return isStateful;
}

bool LayerDrawable::LayerState::hasFocusStateSpecified()const{
    for (auto child:mChildren) {
       Drawable* dr = child->mDrawable;
       if (dr!= nullptr && dr->hasFocusStateSpecified())
           return true;
    }
    return false;
}

bool LayerDrawable::LayerState::canConstantState() const{
    for (auto child:mChildren) {
        Drawable* dr = child->mDrawable;
        if (dr  && dr->getConstantState() == nullptr) {
            return false;
        }
    }
    // Don't cache the result, this method is not called very often.
    return true;
}

void  LayerDrawable::LayerState::setDensity(int targetDensity) {
    if (mDensity != targetDensity) {
        int sourceDensity = mDensity;
        mDensity = targetDensity;
        onDensityChanged(sourceDensity, targetDensity);
    }
}

void LayerDrawable::LayerState::onDensityChanged(int sourceDensity, int targetDensity){
    applyDensityScaling(sourceDensity, targetDensity);
}

void LayerDrawable::LayerState::applyDensityScaling(int sourceDensity, int targetDensity){
    if (mPaddingLeft  >0) mPaddingLeft  = Drawable::scaleFromDensity( mPaddingLeft, sourceDensity, targetDensity, false);
    if (mPaddingTop   >0) mPaddingTop   = Drawable::scaleFromDensity( mPaddingTop, sourceDensity, targetDensity, false);
    if (mPaddingRight >0) mPaddingRight = Drawable::scaleFromDensity( mPaddingRight, sourceDensity, targetDensity, false);
    if (mPaddingBottom>0) mPaddingBottom= Drawable::scaleFromDensity( mPaddingBottom, sourceDensity, targetDensity, false);
    if (mPaddingStart >0) mPaddingStart = Drawable::scaleFromDensity( mPaddingStart, sourceDensity, targetDensity, false);
    if (mPaddingEnd   >0) mPaddingEnd   = Drawable::scaleFromDensity( mPaddingEnd, sourceDensity, targetDensity, false);
}

void LayerDrawable::LayerState::invalidateCache(){
    mCheckedOpacity = false;
    mCheckedStateful = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LayerDrawable::LayerDrawable():LayerDrawable(std::make_shared<LayerState>()){
}

LayerDrawable::LayerDrawable(const std::vector<Drawable*>&drawables)
        :LayerDrawable(){
    mHotspotBounds.set(0,0,0,0);
    mLayerState->mChildren.reserve(drawables.size());
    for(auto d:drawables){
        ChildDrawable*child=new ChildDrawable(0);
        child->mDrawable=d;
        mLayerState->mChildren.push_back(child);
        d->setCallback(this);
    }
    ensurePadding();
    refreshPadding();
}

LayerDrawable::LayerDrawable(std::shared_ptr<LayerState>state){
    mMutated = false;
    mChildRequestedInvalidation = 0;
    mSuspendChildInvalidation = false;
    mLayerState = createConstantState(state.get(),nullptr);
    if (mLayerState->mChildren.size()) {
        ensurePadding();
        refreshPadding();
    }
}

std::shared_ptr<LayerDrawable::LayerState> LayerDrawable::createConstantState(LayerState* state,const AttributeSet*){
    return std::make_shared<LayerState>(state, this);
}

void LayerDrawable::setLayerSize(int index, int w, int h){
    ChildDrawable* childDrawable = mLayerState->mChildren.at(index);
    childDrawable->mWidth = w;
    childDrawable->mHeight = h;
}

int LayerDrawable::getLayerWidth(int index)const{
    ChildDrawable* childDrawable=mLayerState->mChildren.at(index);
    return childDrawable?childDrawable->mWidth:0;
}

void  LayerDrawable::setLayerWidth(int index, int w){
    ChildDrawable* childDrawable = mLayerState->mChildren.at(index);
    childDrawable->mWidth = w;
}

int LayerDrawable::getLayerHeight(int index)const{
    ChildDrawable* childDrawable=mLayerState->mChildren.at(index);
    return childDrawable?childDrawable->mHeight:0;
}

void LayerDrawable::setLayerHeight(int index, int h){
    ChildDrawable* childDrawable = mLayerState->mChildren.at(index);
    childDrawable->mHeight = h;
}

int LayerDrawable::getLayerGravity(int index)const{
    ChildDrawable* childDrawable = mLayerState->mChildren.at(index);
    return childDrawable->mGravity;
}

void LayerDrawable::setLayerGravity(int index, int gravity){
    ChildDrawable* childDrawable = mLayerState->mChildren.at(index);
    childDrawable->mGravity=gravity;
}

void LayerDrawable::setLayerInsetInternal(int index, int l, int t, int r, int b, int s, int e){
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    childDrawable->mInsetL = l;
    childDrawable->mInsetT = t;
    childDrawable->mInsetR = r;
    childDrawable->mInsetB = b;
    childDrawable->mInsetS = s;
    childDrawable->mInsetE = e;
    LOGV("%d:(%d,%d,%d,%d,0x%x,0x%x)",index,l,t,r,b,s,e);
}

void LayerDrawable::setLayerAttributes(int idx,const AttributeSet&atts){
    const int left = atts.getDimensionPixelOffset("left",0);
    const int top = atts.getDimensionPixelOffset("top",0);
    const int right= atts.getDimensionPixelOffset("right",0);
    const int bottom= atts.getDimensionPixelOffset("bottom",0);
    const int start= atts.getDimensionPixelOffset("start",INSET_UNDEFINED);
    const int end  = atts.getDimensionPixelOffset("end",INSET_UNDEFINED);
    const int id = atts.getResourceId("id",-1);
    if((start!=INSET_UNDEFINED)||(end!=INSET_UNDEFINED))
        setLayerInsetRelative(idx,start,top,end,bottom);
    else
        setLayerInset(idx,left,top,right,bottom);
    setLayerGravity(idx,atts.getGravity("gravity",Gravity::NO_GRAVITY));
    setLayerWidth(idx,atts.getDimensionPixelOffset("width",-1));
    setLayerHeight(idx,atts.getDimensionPixelOffset("height",-1));
    if(id!=-1)setId(idx,id);
}

void LayerDrawable::setLayerInset(int index, int l, int t, int r, int b){
    setLayerInsetInternal(index, l, t, r, b, INSET_UNDEFINED, INSET_UNDEFINED);
}

void LayerDrawable::setLayerInsetRelative(int index, int s, int t, int e, int b){
    setLayerInsetInternal(index, 0, t, 0, b, s, e);
}

void LayerDrawable::setLayerInsetLeft(int index, int l) {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    childDrawable->mInsetL = l;
}

int LayerDrawable::getLayerInsetLeft(int index)const {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    return childDrawable->mInsetL;
}

void LayerDrawable::setLayerInsetRight(int index, int r) {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    childDrawable->mInsetR = r;
}

int LayerDrawable::getLayerInsetRight(int index)const {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    return childDrawable->mInsetR;
}

void LayerDrawable::setLayerInsetTop(int index, int t) {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    childDrawable->mInsetT = t;
}

int LayerDrawable::getLayerInsetTop(int index)const {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    return childDrawable->mInsetT;
}

void LayerDrawable::setLayerInsetBottom(int index, int b) {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    childDrawable->mInsetB = b;
}

int LayerDrawable::getLayerInsetBottom(int index)const {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    return childDrawable->mInsetB;
}

void LayerDrawable::setLayerInsetStart(int index, int s) {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    childDrawable->mInsetS = s;
}

int LayerDrawable::getLayerInsetStart(int index)const {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    return childDrawable->mInsetS;
}

void LayerDrawable::setLayerInsetEnd(int index, int e) {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    childDrawable->mInsetE = e;
}

int LayerDrawable::getLayerInsetEnd(int index)const {
    ChildDrawable* childDrawable = mLayerState->mChildren[index];
    return childDrawable->mInsetE;
}

LayerDrawable::ChildDrawable* LayerDrawable::createLayer(Drawable* dr){
    ChildDrawable* layer = new ChildDrawable(mLayerState->mDensity);
    layer->mDrawable = dr;
    return layer;
}

int LayerDrawable::addLayer(ChildDrawable* layer){
    const int i = (int)mLayerState->mChildren.size();
    mLayerState->mChildren.push_back(layer);
    mLayerState->invalidateCache();
    if(layer&&layer->mDrawable){
        layer->setDensity(160);
        layer->mDrawable->setCallback(this);
    }
    return i;
}

int LayerDrawable::addLayer(Drawable* dr){
    LayerDrawable::ChildDrawable* layer = createLayer(dr);
    const int index = addLayer(layer);
    ensurePadding();
    refreshChildPadding(index, layer);
    return index;
}

LayerDrawable::ChildDrawable* LayerDrawable::addLayer(Drawable* dr,const std::vector<int>&themeAttrs,int id,int left,int top,int right,int bottom){
    LayerDrawable::ChildDrawable*childDrawable = createLayer(dr);
    childDrawable->mId = id;
    childDrawable->mThemeAttrs = themeAttrs;
    childDrawable->mDrawable->setAutoMirrored(isAutoMirrored());
    childDrawable->mInsetL = left;
    childDrawable->mInsetT = top;
    childDrawable->mInsetR = right;
    childDrawable->mInsetB = bottom;

    addLayer(childDrawable);

    mLayerState->mChildrenChangingConfigurations |= dr->getChangingConfigurations();
    dr->setCallback(this);
    return childDrawable;
}

int LayerDrawable::getId(int index)const{
    if (index >= mLayerState->mChildren.size())
        return -1;//throw new IndexOutOfBoundsException();
    return mLayerState->mChildren[index]->mId;
}

void LayerDrawable::setId(int index, int id){
    mLayerState->mChildren[index]->mId=id;
}

Drawable* LayerDrawable::findDrawableByLayerId(int id){
    for (auto c:mLayerState->mChildren) {
        if (c->mId == id){
            return c->mDrawable;
        }
    }
    return nullptr;
}

int LayerDrawable::getNumberOfLayers()const{
    return (int)mLayerState->mChildren.size();
}

bool LayerDrawable::setDrawableByLayerId(int id, Drawable* drawable){
    for (int i=0;i<mLayerState->mChildren.size();i++) {
        LayerDrawable::ChildDrawable* c = mLayerState->mChildren[i];
        if (c->mId == id) {
            setDrawable(i,drawable);
            return true;
        }
    }
    return false;
}

int LayerDrawable::findIndexByLayerId(int id)const{
    for (int i=0;i<mLayerState->mChildren.size();i++) {
        LayerDrawable::ChildDrawable* c = mLayerState->mChildren[i];
        if (c->mId == id) return i;
    }
    return -1;
}

Drawable* LayerDrawable::getDrawable(int index)const{
    if(index>=mLayerState->mChildren.size())return nullptr;
    return mLayerState->mChildren[index]->mDrawable;
}

void LayerDrawable::setDrawable(int index, Drawable* drawable){
    if (index >= mLayerState->mChildren.size())return;
    LayerDrawable::ChildDrawable* childDrawable =mLayerState->mChildren.at(index);
    if (childDrawable->mDrawable != nullptr) {
       if (drawable != nullptr) {
           Rect bounds = childDrawable->mDrawable->getBounds();
           drawable->setBounds(bounds);
       }
       childDrawable->mDrawable->setCallback(nullptr);
    }

    if (drawable != nullptr)
        drawable->setCallback(this);

    childDrawable->mDrawable = drawable;
    mLayerState->invalidateCache();
    refreshChildPadding(index, childDrawable);    
}

void LayerDrawable::computeNestedPadding(Rect& padding){
    padding.set(0,0,0,0);

    // Add all the padding.
    const int N = (int)mLayerState->mChildren.size();
    for (int i = 0; i < N; i++) {
        LayerDrawable::ChildDrawable*child=mLayerState->mChildren.at(i);
        refreshChildPadding(i, child);
        padding.left  += mPaddingL[i];
        padding.top   += mPaddingT[i];
        padding.width += mPaddingR[i];
        padding.height+= mPaddingB[i];
    }
}

void LayerDrawable::computeStackedPadding(Rect& padding){
    const int N = (int)mLayerState->mChildren.size();
    padding.set(0,0,0,0);
    for (int i = 0; i < N; i++) {
        LayerDrawable::ChildDrawable*child=mLayerState->mChildren.at(i);
        refreshChildPadding(i, child);

        padding.left  = std::max(padding.left, mPaddingL[i]);
        padding.top   = std::max(padding.top, mPaddingT[i]);
        padding.width = std::max(padding.width, mPaddingR[i]);
        padding.height= std::max(padding.height, mPaddingB[i]);
    }
}

bool LayerDrawable::getPadding(Rect& padding){
    if (mLayerState->mPaddingMode == PADDING_MODE_NEST) {
        computeNestedPadding(padding);
    } else {
        computeStackedPadding(padding);
    }

    const int paddingT = mLayerState->mPaddingTop;
    const int paddingB = mLayerState->mPaddingBottom;

    // Resolve padding for RTL. Relative padding overrides absolute  padding.
    const bool isLayoutRtl = getLayoutDirection() == LayoutDirection::RTL;
    const int paddingRtlL = isLayoutRtl ? mLayerState->mPaddingEnd  : mLayerState->mPaddingStart;
    const int paddingRtlR = isLayoutRtl ? mLayerState->mPaddingStart: mLayerState->mPaddingEnd;
    const int paddingL = paddingRtlL >= 0 ? paddingRtlL : mLayerState->mPaddingLeft;
    const int paddingR = paddingRtlR >= 0 ? paddingRtlR : mLayerState->mPaddingRight;

        // If padding was explicitly specified (e.g. not -1) then override the
        // computed padding in that dimension.
    if (paddingL >= 0) padding.left = paddingL;
    if (paddingT >= 0) padding.top  = paddingT;
    if (paddingR >= 0) padding.width = paddingR;
    if (paddingB >= 0) padding.height = paddingB;
    return padding.left != 0 || padding.top != 0 || padding.width != 0 || padding.height != 0;
}

void LayerDrawable::setPadding(const AttributeSet&atts){
    const int left = atts.getDimensionPixelOffset("paddingLeft",0);
    const int top = atts.getDimensionPixelOffset("paddingTop",0);
    const int right= atts.getDimensionPixelOffset("paddingRight",0);
    const int bottom= atts.getDimensionPixelOffset("paddingBottom",0);
    const int start= atts.getDimensionPixelOffset("paddingStart",INSET_UNDEFINED);
    const int end  = atts.getDimensionPixelOffset("paddingEnd",INSET_UNDEFINED);
    if((start==INSET_UNDEFINED)&&(end==INSET_UNDEFINED)) 
        setPadding(left,top,right,bottom);
    else
        setPaddingRelative(start,top,end,bottom);
}

void LayerDrawable::setPadding(int left, int top, int right, int bottom){
    mLayerState->mPaddingLeft = left;
    mLayerState->mPaddingTop = top;
    mLayerState->mPaddingRight = right;
    mLayerState->mPaddingBottom = bottom;

    // Clear relative padding values.
    mLayerState->mPaddingStart = -1;
    mLayerState->mPaddingEnd = -1;
}

void LayerDrawable::setPaddingRelative(int start, int top, int end, int bottom) {
    mLayerState->mPaddingStart = start;
    mLayerState->mPaddingTop = top;
    mLayerState->mPaddingEnd = end;
    mLayerState->mPaddingBottom = bottom;
    // Clear absolute padding values.
    mLayerState->mPaddingLeft = -1;
    mLayerState->mPaddingRight = -1;
}

int LayerDrawable::getLeftPadding()const{
    return mLayerState->mPaddingLeft;
}

int LayerDrawable::getRightPadding()const{
    return mLayerState->mPaddingRight;
}

int LayerDrawable::getStartPadding()const{
    return mLayerState->mPaddingStart;
}

int LayerDrawable::getEndPadding()const{
    return mLayerState->mPaddingEnd;
}

int LayerDrawable::getTopPadding()const{
    return mLayerState->mPaddingTop;
}

int LayerDrawable::getBottomPadding()const{
    return mLayerState->mPaddingBottom;
}

void LayerDrawable::setHotspot(float x,float y){
    for (ChildDrawable*child:mLayerState->mChildren){
       Drawable* dr = child->mDrawable;
       if (dr != nullptr) {
           dr->setHotspot(x, y);
       }
    }
}

void LayerDrawable::setHotspotBounds(int left,int top,int width,int height){
    for (ChildDrawable*child:mLayerState->mChildren) {
        Drawable* dr = child->mDrawable;
        if (dr != nullptr) {
            dr->setHotspotBounds(left, top, width,height);
        }
    }
    mHotspotBounds.set(left, top, width,height);
}

void LayerDrawable::getHotspotBounds(Rect& outRect)const{
    if (!mHotspotBounds.empty()) {
         outRect = mHotspotBounds;
    } else {
	 Drawable::getHotspotBounds(outRect);
    }
}

void LayerDrawable::setTintList(const ColorStateList* tint){
    for (ChildDrawable*child:mLayerState->mChildren) {
        Drawable* dr = child->mDrawable;
        if (dr != nullptr) {
            dr->setTintList(tint);
        }
    }
}

void LayerDrawable::setTintMode(int tintMode){
    for (ChildDrawable*child:mLayerState->mChildren) {
        Drawable* dr = child->mDrawable;
        if (dr != nullptr) {
            dr->setTintMode(tintMode);
        }
    }
}

void LayerDrawable::ensurePadding() {
    const size_t N = mLayerState->mChildren.size();
    if (mPaddingL.size()>N) return;

    mPaddingL.resize(N);
    mPaddingT.resize(N);
    mPaddingR.resize(N);
    mPaddingB.resize(N);
}

void LayerDrawable::refreshPadding() {
    const int N = (int)mLayerState->mChildren.size();
    for (int i = 0; i < N; i++)
        refreshChildPadding(i, mLayerState->mChildren[i]);
}

bool LayerDrawable::refreshChildPadding(int i, ChildDrawable* r) {
    if (r->mDrawable != nullptr) {
        Rect rect={0,0,0,0};
        r->mDrawable->getPadding(rect);
        if (rect.left != mPaddingL[i] || rect.top != mPaddingT[i]
                || rect.width != mPaddingR[i] || rect.height != mPaddingB[i]) {
            mPaddingL[i] = rect.left;
            mPaddingT[i] = rect.top;
            mPaddingR[i] = rect.width;
            mPaddingB[i] = rect.height;
            LOGV("layer[%d].padding=(%d,%d,%d,%d)",i,rect.left,rect.top,rect.width,rect.height);
            return true;
        }
    }
    return false;
}

void LayerDrawable::setPaddingMode(int mode) {
    if (mLayerState->mPaddingMode != mode) {
        mLayerState->mPaddingMode = mode;
    }
}

int LayerDrawable::getPaddingMode()const{
  return mLayerState->mPaddingMode;
}

void LayerDrawable::onBoundsChange(const Rect& bounds){
    updateLayerBounds(bounds);
}

bool LayerDrawable::onLevelChange(int level){
    bool changed = false;
    const int N = (int)mLayerState->mChildren.size();
    for (int i = 0; i < N; i++) {
        LayerDrawable::ChildDrawable*child=mLayerState->mChildren[i];
        Drawable* dr = child->mDrawable;
        if (dr != nullptr && dr->setLevel(level)) {
            refreshChildPadding(i, child);
            changed = true;
        }
    }
    if (changed)updateLayerBounds(getBounds());
    return changed;
}

bool LayerDrawable::onStateChange(const std::vector<int>& state){
    bool changed = false;
    const int N = (int)mLayerState->mChildren.size();
    for (int i = 0; i < N; i++) {
        LayerDrawable::ChildDrawable*child=mLayerState->mChildren[i];
        Drawable* dr = child->mDrawable;
        if (dr != nullptr && dr->isStateful() && dr->setState(state)) {
            refreshChildPadding(i,child);
            changed = true;
        }
    }
    if (changed) updateLayerBounds(getBounds());
    return changed;
}

void LayerDrawable::suspendChildInvalidation(){
    mSuspendChildInvalidation = true;
}

void LayerDrawable::resumeChildInvalidation(){
    mSuspendChildInvalidation = false;
    if (mChildRequestedInvalidation) {
        mChildRequestedInvalidation = false;
        invalidateSelf();
    }
}

void LayerDrawable::updateLayerBounds(const Rect& bounds) {
    suspendChildInvalidation();
    updateLayerBoundsInternal(bounds);
    resumeChildInvalidation();
}

bool LayerDrawable::onLayoutDirectionChanged(int layoutDirection){
    bool changed = false;
    for (auto child:mLayerState->mChildren) {
        Drawable* dr = child->mDrawable;
        if (dr != nullptr) {
            changed |= dr->setLayoutDirection(layoutDirection);
        }
    }
    updateLayerBounds(getBounds());
    return changed;
}

void LayerDrawable::updateLayerBoundsInternal(const Rect& bounds){
    int paddingL = 0;
    int paddingT = 0;
    int paddingR = 0;
    int paddingB = 0;

    Rect outRect;
    const int layoutDirection = getLayoutDirection();
    const bool isLayoutRtl = layoutDirection == LayoutDirection::RTL;
    const bool isPaddingNested = mLayerState->mPaddingMode == PADDING_MODE_NEST;

    for (size_t i = 0, count = mLayerState->mChildren.size(); i < count; i++) {
        ChildDrawable*r=mLayerState->mChildren[i];
        Drawable* d = r->mDrawable;
        if (d == nullptr)  continue;

        const int insetT = r->mInsetT;
        const int insetB = r->mInsetB;

        // Resolve insets for RTL. Relative insets override absolute insets.
        const int insetRtlL = isLayoutRtl ? r->mInsetE : r->mInsetS;
        const int insetRtlR = isLayoutRtl ? r->mInsetS : r->mInsetE;
        const int insetL = insetRtlL == INSET_UNDEFINED ? r->mInsetL : insetRtlL;
        const int insetR = insetRtlR == INSET_UNDEFINED ? r->mInsetR : insetRtlR;

        // Establish containing region based on aggregate padding and
        // requested insets for the current layer.
        Rect container;
        container.set(bounds.left + insetL + paddingL, bounds.top + insetT + paddingT,
            bounds.width - insetL -insetR - paddingL -paddingR, bounds.height - insetT -insetB -paddingT -paddingB);

        // Compute a reasonable default gravity based on the intrinsic and
        // explicit dimensions, if specified.
        const int intrinsicW = d->getIntrinsicWidth();
        const int intrinsicH = d->getIntrinsicHeight();
        const int layerW = r->mWidth;
        const int layerH = r->mHeight;
        const int gravity =resolveGravity(r->mGravity, layerW, layerH, intrinsicW, intrinsicH);

        // Explicit dimensions override intrinsic dimensions.
        const int resolvedW = layerW < 0 ? intrinsicW : layerW;
        const int resolvedH = layerH < 0 ? intrinsicH : layerH;

        Gravity::apply(gravity, resolvedW, resolvedH, container, outRect, layoutDirection);
        d->setBounds(outRect);
        LOGV("paddingLTRB=%d,%d-%d,%d container=%d,%d-%d,%d resolvedSize=%dx%d",paddingL,paddingT,paddingR,paddingB,
                container.left,container.top,container.width,container.height,resolvedW,resolvedH);
        LOGV("r.mInsetLTRB=%d,%d-%d,%d insetLTRB=%d,%d-%d,%d r->mGravity=%x size=%dx%d",r->mInsetL,r->mInsetT,r->mInsetR,r->mInsetB,
                insetL,insetT,insetR,insetB,r->mGravity,r->mWidth,r->mHeight);
        LOGV("child[%d] bounds=%d,%d-%d,%d isPaddingNested=%d mPaddingLTRB=%d,%d-%d,%d",i, outRect.left,outRect.top,
                outRect.width,outRect.height,isPaddingNested,mPaddingL[i], mPaddingT[i],mPaddingR[i],mPaddingB[i]);
        if (isPaddingNested) {
            paddingL += mPaddingL[i];
            paddingR += mPaddingR[i];
            paddingT += mPaddingT[i];
            paddingB += mPaddingB[i];
        }
    }
}

int LayerDrawable::resolveGravity(int gravity, int width, int height,
            int intrinsicWidth, int intrinsicHeight){
    if (!Gravity::isHorizontal(gravity)) {
        if (width < 0)
            gravity |= Gravity::FILL_HORIZONTAL;
        else
            gravity |= Gravity::START;
    }

    if (!Gravity::isVertical(gravity)) {
        if (height < 0) 
            gravity |= Gravity::FILL_VERTICAL;
        else
            gravity |= Gravity::TOP;
    }

    // If a dimension if not specified, either implicitly or explicitly,
    // force FILL for that dimension's gravity. This ensures that colors
    // are handled correctly and ensures backward compatibility.
    if (width < 0 && intrinsicWidth < 0)
        gravity |= Gravity::FILL_HORIZONTAL;

    if (height < 0 && intrinsicHeight < 0) 
        gravity |= Gravity::FILL_VERTICAL;

    return gravity;
}

int LayerDrawable::getIntrinsicWidth(){
    int width = -1;
    int padL = 0;
    int padR = 0;

    const bool nest = mLayerState->mPaddingMode == PADDING_MODE_NEST;
    const bool isLayoutRtl = getLayoutDirection() == LayoutDirection::RTL;
    const size_t N = mLayerState->mChildren.size();
    for (size_t i = 0; i < N; i++) {
        ChildDrawable* r = mLayerState->mChildren.at(i);
        if (r->mDrawable == nullptr) continue;
        // Take the resolved layout direction into account. If start / end
        // padding are defined, they will be resolved (hence overriding) to
        // left / right or right / left depending on the resolved layout
        // direction. If start / end padding are not defined, use the
        // left / right ones.
        const int insetRtlL = isLayoutRtl ? r->mInsetE : r->mInsetS;
        const int insetRtlR = isLayoutRtl ? r->mInsetS : r->mInsetE;
        const int insetL = insetRtlL == INSET_UNDEFINED ? r->mInsetL : insetRtlL;
        const int insetR = insetRtlR == INSET_UNDEFINED ? r->mInsetR : insetRtlR;

        // Don't apply padding and insets for children that don't have
        // an intrinsic dimension.
        const int minWidth = r->mWidth < 0 ? r->mDrawable->getIntrinsicWidth() : r->mWidth;
        const int w = minWidth < 0 ? -1 : minWidth + insetL + insetR + padL + padR;
        if (w > width) width = w;

        if (nest) {
            padL += mPaddingL[i];
            padR += mPaddingR[i];
        }
    }
    return width;
}

int LayerDrawable::getIntrinsicHeight() {
    int height = -1;
    int padT = 0;
    int padB = 0;

    const bool nest = mLayerState->mPaddingMode == PADDING_MODE_NEST;
    const size_t N = mLayerState->mChildren.size();
    for (size_t i = 0; i < N; i++) {
        ChildDrawable* r = mLayerState->mChildren.at(i);
        if (r->mDrawable == nullptr) continue;
        // Don't apply padding and insets for children that don't have
        // an intrinsic dimension.
        const int minHeight = r->mHeight < 0 ? r->mDrawable->getIntrinsicHeight() : r->mHeight;
        const int h = minHeight < 0 ? -1 : minHeight + r->mInsetT + r->mInsetB + padT + padB;
        if (h > height)  height = h;

        if (nest) {
            padT += mPaddingT[i];
            padB += mPaddingB[i];
        }
    }
    return height;
}

bool LayerDrawable::isStateful()const{
    return mLayerState->isStateful();
}

bool LayerDrawable::hasFocusStateSpecified()const{
    return mLayerState->hasFocusStateSpecified();
}

void LayerDrawable::setAutoMirrored(bool mirrored){
    mLayerState->mAutoMirrored = mirrored;

    for (ChildDrawable*dr:mLayerState->mChildren) {
         if (dr->mDrawable) dr->mDrawable->setAutoMirrored(mirrored);
    }
}

bool LayerDrawable::isAutoMirrored()const{
    return mLayerState->mAutoMirrored;
}

void LayerDrawable::jumpToCurrentState(){
    for (ChildDrawable*dr:mLayerState->mChildren) {
        if (dr->mDrawable) dr->mDrawable->jumpToCurrentState();
    }
}

std::shared_ptr<Drawable::ConstantState>LayerDrawable::getConstantState(){
    if (mLayerState->canConstantState()) {
        mLayerState->mChangingConfigurations = getChangingConfigurations();
        return mLayerState;
    }
    return nullptr;
}

void LayerDrawable::invalidateDrawable(Drawable& who){
    if (mSuspendChildInvalidation) {
        mChildRequestedInvalidation = true;
    } else {
        // This may have been called as the result of a tint changing, in
        // which case we may need to refresh the cached statefulness or
        // opacity.
        mLayerState->invalidateCache();
        invalidateSelf();
    }
}
void LayerDrawable::scheduleDrawable(Drawable& who,const Runnable& what, int64_t when){
    scheduleSelf(what, when);
}

void LayerDrawable::unscheduleDrawable(Drawable& who,const Runnable& what){
    unscheduleSelf(what);
}

bool LayerDrawable::setVisible(bool visible,bool restart){
    const bool changed = Drawable::setVisible(visible,restart);
    for(auto child:mLayerState->mChildren){
        Drawable*dr = child->mDrawable;
        if(dr)dr->setVisible(visible,restart);
    }
    return changed;
}

void LayerDrawable::setDither(bool dither) {
    for (auto child:mLayerState->mChildren) {
        Drawable* dr = child->mDrawable;
        if (dr) dr->setDither(dither);
    }
}

void LayerDrawable::setAlpha(int alpha){
    for(auto child:mLayerState->mChildren){
        Drawable*dr = child->mDrawable;
        if(dr) dr->setAlpha(alpha);    
    }
}

int  LayerDrawable::getAlpha()const{
    Drawable*dr = getFirstNonNullDrawable();
    if(dr) return dr->getAlpha();
    return Drawable::getAlpha();
}

void LayerDrawable::setOpacity(int opacity) {
    mLayerState->mOpacityOverride = opacity;
}

int LayerDrawable::getOpacity() {
    if (mLayerState->mOpacityOverride != PixelFormat::UNKNOWN) {
        return mLayerState->mOpacityOverride;
    }
    return mLayerState->getOpacity();
}

Drawable* LayerDrawable::getFirstNonNullDrawable()const{
    for(auto child:mLayerState->mChildren){
        Drawable*dr = child->mDrawable;
        if(dr)return dr;
    }
    return nullptr;
}

LayerDrawable*LayerDrawable::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        mLayerState = createConstantState(mLayerState.get(),nullptr);
        for (auto child:mLayerState->mChildren) {
            Drawable*dr = child->mDrawable;
            if (dr)dr->mutate();
        }
        mMutated = true;
    }
    return this;
}

void LayerDrawable::clearMutated() {
    Drawable::clearMutated();

    for (auto child:mLayerState->mChildren) {
        Drawable* dr = child->mDrawable;
        if (dr)dr->clearMutated();
    }
    mMutated = false;
}

void LayerDrawable::draw(Canvas&canvas){
    for (auto child:mLayerState->mChildren) {
        Drawable* dr = child->mDrawable;
        if (dr != nullptr){
            dr->draw(canvas);
        }
    }
}

void LayerDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    Drawable::inflate(parser,atts);
    const int density = Drawable::resolveDensity( 0);
    mLayerState->setDensity(density);

    updateStateFromTypedArray(atts);
    for (ChildDrawable*layer:mLayerState->mChildren) {
        layer->setDensity(density);
    }
    inflateLayers(parser,atts);
    ensurePadding();
    refreshPadding();
}

void LayerDrawable::inflateLayers(XmlPullParser& parser,const AttributeSet& atts){
    int type,depth,low = 0;
    const int innerDepth = parser.getDepth()+1;
    while (((type = parser.next()) != XmlPullParser::END_DOCUMENT)
            && ((depth=parser.getDepth()) >= innerDepth|| type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        if ((depth > innerDepth) || parser.getName().compare("item")) {
            continue;
        }

        ChildDrawable*layer = new ChildDrawable(mLayerState->mDensity);
        updateLayerFromTypedArray(layer,atts);

        if (layer->mDrawable==nullptr) {
            while ((type = parser.next()) == XmlPullParser::TEXT) {
            }
            if (type != XmlPullParser::START_TAG){
                throw std::logic_error(parser.getPositionDescription()+
                                ": <item> tag requires a 'drawable' attribute or "
                                "child tag defining a drawable");
            }
            layer->mDrawable = Drawable::createFromXmlInner(parser,atts);
            layer->mDrawable->setCallback(this);
        }
        addLayer(layer);
    }
}

void LayerDrawable::updateStateFromTypedArray(const AttributeSet&a) {
    auto state = mLayerState;

    // Account for any configuration changes.
    //state->mChangingConfigurations |= a.getChangingConfigurations();
    // Extract the theme attributes, if any.
    //state->mThemeAttrs = a.extractThemeAttrs();

    state->mOpacityOverride = a.getInt("opacity", state->mOpacityOverride);
    state->mPaddingTop = a.getDimensionPixelOffset("paddingTop", state->mPaddingTop);
    state->mPaddingBottom = a.getDimensionPixelOffset("paddingBottom", state->mPaddingBottom);
    state->mPaddingLeft = a.getDimensionPixelOffset("paddingLeft", state->mPaddingLeft);
    state->mPaddingRight = a.getDimensionPixelOffset("paddingRight", state->mPaddingRight);
    state->mPaddingStart = a.getDimensionPixelOffset("paddingStart", state->mPaddingStart);
    state->mPaddingEnd = a.getDimensionPixelOffset("paddingEnd", state->mPaddingEnd);
    state->mAutoMirrored = a.getBoolean("autoMirrored", state->mAutoMirrored);
    state->mPaddingMode = a.getInt("paddingMode", state->mPaddingMode);
}

void LayerDrawable::updateLayerFromTypedArray(ChildDrawable*layer,const AttributeSet&atts){
    auto state = mLayerState;

    // Account for any configuration changes.
    //state->mChildrenChangingConfigurations |= a.getChangingConfigurations();
    //layer->mThemeAttrs = a.extractThemeAttrs();

    layer->mInsetL = atts.getDimensionPixelOffset("left", layer->mInsetL);
    layer->mInsetT = atts.getDimensionPixelOffset("top", layer->mInsetT);
    layer->mInsetR = atts.getDimensionPixelOffset("right", layer->mInsetR);
    layer->mInsetB = atts.getDimensionPixelOffset("bottom", layer->mInsetB);
    layer->mInsetS = atts.getDimensionPixelOffset("start", layer->mInsetS);
    layer->mInsetE = atts.getDimensionPixelOffset("end", layer->mInsetE);
    layer->mWidth  = atts.getDimensionPixelSize("width", layer->mWidth);
    layer->mHeight = atts.getDimensionPixelSize("height", layer->mHeight);
    layer->mGravity= atts.getGravity("gravity", layer->mGravity);
    layer->mId = atts.getResourceId("id", layer->mId);

    Drawable* dr = atts.getDrawable("drawable");
    if (dr != nullptr) {
        if (layer->mDrawable != nullptr) {
            // It's possible that a drawable was already set, in which case
            // we should clear the callback. We may have also integrated the
            // drawable's changing configurations, but we don't have enough
            // information to revert that change.
            layer->mDrawable->setCallback(nullptr);
        }

        // Take ownership of the new drawable.
        layer->mDrawable = dr;
        layer->mDrawable->setCallback(this);
        state->mChildrenChangingConfigurations |= layer->mDrawable->getChangingConfigurations();
    }
}

}
