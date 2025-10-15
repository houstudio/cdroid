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
#include <drawable/clipdrawable.h>
#include <porting/cdlog.h>

namespace cdroid{

ClipDrawable::ClipState::ClipState():DrawableWrapperState(){
    mGravity = Gravity::LEFT;
    mOrientation = HORIZONTAL;
}

ClipDrawable::ClipState::ClipState(const ClipState& state)
    :DrawableWrapperState(state){
    mGravity = state.mGravity;
    mOrientation =state.mOrientation;
}

ClipDrawable*ClipDrawable::ClipState::newDrawable(){
    return new ClipDrawable(std::dynamic_pointer_cast<ClipState>(shared_from_this()));
}

///////////////////////////////////////////////////////////////////////////////////////////

ClipDrawable::ClipDrawable()
    :ClipDrawable(std::make_shared<ClipState>()){
}

ClipDrawable::ClipDrawable(std::shared_ptr<ClipState>state):DrawableWrapper(state){
    mState = state;
}

ClipDrawable::ClipDrawable(Drawable* drawable, int gravity,int orientation)
    :ClipDrawable(std::make_shared<ClipState>()){
    mState->mGravity = gravity;
    mState->mOrientation = orientation;
    setDrawable(drawable);
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState> ClipDrawable::mutateConstantState(){
    return std::make_shared<ClipState>(*mState);
}

int ClipDrawable::getGravity()const{
    return mState->mGravity;
}

int ClipDrawable::getOrientation()const{
    return mState->mOrientation;
}

int ClipDrawable::getOpacity(){
    Drawable* dr = getDrawable();
    if (dr->getOpacity() == PixelFormat::TRANSPARENT || dr->getLevel() == 0) {
        return PixelFormat::TRANSPARENT;
    }

    if (getLevel() >= MAX_LEVEL) {
        return dr->getOpacity();
    }
    // Some portion of non-transparent drawable is showing.
    return PixelFormat::TRANSLUCENT;
}

bool ClipDrawable::onLevelChange(int level){
    DrawableWrapper::onLevelChange(level);
    invalidateSelf();
    return true;
}

void ClipDrawable::draw(Canvas& canvas){
    Drawable* dr = getDrawable();
    if (dr->getLevel() == 0) return;
    Rect r ={0,0,0,0};
    Rect bounds = getBounds();
    const int level = getLevel();

    int w = bounds.width;
    const int iw = 0;
    if (mState->mOrientation&HORIZONTAL){
        w -= (w - iw) * (MAX_LEVEL - level) / MAX_LEVEL;
    }

    int h = bounds.height;
    const int ih = 0;
    if (mState->mOrientation&VERTICAL){
        h -= (h - ih) * (MAX_LEVEL - level) / MAX_LEVEL;
    }

    const int layoutDirection = getLayoutDirection();
    Gravity::apply(mState->mGravity, w, h, bounds, r, layoutDirection);
    LOGV("%p lvl=%d rect=%d,%d-%d,%d gravity=%d bounds=%d,%d-%d,%d  wh=%d,%d",
            this,level,r.left,r.top,r.width,r.height, mState->mGravity,bounds.left,bounds.top,bounds.width,bounds.height,w,h);
    if ((dr!=nullptr) && (w > 0) && (h > 0)) {
        canvas.save();
        canvas.rectangle(r.left,r.top,r.width,r.height);
        canvas.clip();
        dr->draw(canvas);
        canvas.restore();
    }
}

void ClipDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    updateStateFromTypedArray(atts);
    DrawableWrapper::inflate(parser,atts);
}

void ClipDrawable::updateStateFromTypedArray(const AttributeSet&atts){
    mState->mOrientation = atts.getInt("clipOrientation",std::unordered_map<std::string,int>{
            {"horizontal",(int)HORIZONTAL},
            {"vertical",(int)VERTICAL}
        }, mState->mOrientation);
    mState->mGravity = atts.getGravity("gravity", mState->mGravity);
}

}/*endof namespace*/
