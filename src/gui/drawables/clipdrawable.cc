#include <drawables/clipdrawable.h>
#include <gravity.h>
#include <cdlog.h>


namespace cdroid{

#define MAX_LEVEL 10000

ClipDrawable::ClipState::ClipState():DrawableWrapperState(){
    mOrientation = HORIZONTAL;
    mGravity = Gravity::LEFT;
}

ClipDrawable::ClipState::ClipState(const ClipState& state)
    :DrawableWrapperState(state){
    mOrientation =state.mOrientation;
    mGravity = state.mGravity;
}

Drawable*ClipDrawable::ClipState::newDrawable(){
    return new ClipDrawable(std::dynamic_pointer_cast<ClipState>(shared_from_this()));
}

///////////////////////////////////////////////////////////////////////////////////////////

ClipDrawable::ClipDrawable()
    :ClipDrawable(std::make_shared<ClipState>()){
}

ClipDrawable::ClipDrawable(std::shared_ptr<ClipState>state):DrawableWrapper(state){
    mState=state;
}

ClipDrawable::ClipDrawable(Drawable* drawable, int gravity, int orientation)
    :ClipDrawable(std::make_shared<ClipState>()){
    mState->mGravity = gravity;
    mState->mOrientation = orientation;
    setDrawable(drawable);
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState> ClipDrawable::mutateConstantState(){
    return std::make_shared<ClipState>(*mState);
}

void ClipDrawable::setGravity(int gravity){
    mState->mGravity = gravity;
}

void ClipDrawable::setOrientation(int orientation){
    mState->mOrientation = orientation;
}

int ClipDrawable::getOpacity(){
    Drawable* dr = getDrawable();
    if (dr->getOpacity() == Drawable::TRANSPARENT || dr->getLevel() == 0) {
        return Drawable::TRANSPARENT;
    }

    if (getLevel() >= MAX_LEVEL) {
        return dr->getOpacity();
    }
    // Some portion of non-transparent drawable is showing.
    return Drawable::TRANSLUCENT;
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
    if ((mState->mOrientation & HORIZONTAL) != 0) {
        w -= (w - iw) * (MAX_LEVEL - level) / MAX_LEVEL;
    }

    int h = bounds.height;
    const int ih = 0;
    if ((mState->mOrientation & VERTICAL) != 0) {
        h -= (h - ih) * (MAX_LEVEL - level) / MAX_LEVEL;
    }

    const int layoutDirection = getLayoutDirection();
    Gravity::apply(mState->mGravity, w, h, bounds, r, layoutDirection);
    LOGV("%p lvl=%d rect=%d,%d-%d,%d gravity=%d orient=%d bounds=%d,%d-%d,%d  wh=%d,%d",
            this,level,r.left,r.top,r.width,r.height, mState->mGravity,mState->mOrientation,bounds.left,bounds.top,bounds.width,bounds.height,w,h);
    if (w > 0 && h > 0) {
        canvas.save();
        canvas.rectangle(r.left,r.top,r.width,r.height);
        canvas.clip();
        dr->draw(canvas);
        canvas.restore();
    }
}

Drawable*ClipDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string src=atts.getString("drawable");
    Drawable*d=Drawable::inflate(ctx,src);
    int gravity=atts.getGravity("gravity",Gravity::LEFT);
    const std::string sOrientation=atts.getString("clipOrientation");
    int orientation=(sOrientation.compare("vertical")==0)?VERTICAL:HORIZONTAL;
    d=new ClipDrawable(d,gravity,orientation);
    LOGV("%p gravity=%d orientation=%d",d,gravity,orientation);
    return d;
}

}
