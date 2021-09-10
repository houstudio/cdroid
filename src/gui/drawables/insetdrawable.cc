#include <drawables/insetdrawable.h>
#include <cdlog.h>
namespace cdroid{

int InsetDrawable::InsetValue::getDimension(int boundSize)const{
    return (int) (boundSize * mFraction) + mDimension;
}

InsetDrawable::InsetState::InsetState():DrawableWrapperState(){
    mInset.set(0,0,0,0);
}

InsetDrawable::InsetState::InsetState(const InsetState& orig)
    :DrawableWrapperState(orig){
    mInset=orig.mInset;
}

void InsetDrawable::InsetState::applyDensityScaling(int sourceDensity, int targetDensity){
}

void InsetDrawable::InsetState::onDensityChanged(int sourceDensity, int targetDensity){
}

Drawable*InsetDrawable::InsetState::newDrawable(){
    return new InsetDrawable(std::dynamic_pointer_cast<InsetState>(shared_from_this()));
}

InsetDrawable::InsetDrawable(){
    mState=std::make_shared<InsetState>();
}

InsetDrawable::InsetDrawable(std::shared_ptr<InsetState>state):DrawableWrapper(state){
    mState=state;
}

InsetDrawable::InsetDrawable(Drawable*drawable,int inset)
    :InsetDrawable(std::make_shared<InsetState>()){
    setDrawable(drawable);
    mState->mInset.set(inset,inset,inset,inset);
}

InsetDrawable::InsetDrawable(Drawable* drawable,int insetLeft,int insetTop,int insetRight,int insetBottom)
    :InsetDrawable(drawable,0){
    mState->mInset.set(insetLeft,insetTop,insetRight,insetBottom);
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState> InsetDrawable::mutateConstantState(){
    mState=std::make_shared<InsetState>(*mState);
    return mState;
}

void InsetDrawable::getInsets(Rect& out) {
    Rect b = getBounds();
    out.left  = mState->mInsetLeft.getDimension(b.width);
    out.width = mState->mInsetRight.getDimension(b.width);
    out.top   = mState->mInsetTop.getDimension(b.height);
    out.height= mState->mInsetBottom.getDimension(b.height);
}

bool InsetDrawable::getPadding(Rect& padding) {
    bool pad = DrawableWrapper::getPadding(padding);
    Rect tmp;
    getInsets(tmp);
    padding.left  += tmp.left;
    padding.width += tmp.width;
    padding.top   += tmp.top;
    padding.height+= tmp.height;

    return pad || (tmp.left | tmp.width | tmp.top | tmp.height) != 0;
}

Insets InsetDrawable::getOpticalInsets() {
    const Insets contentInsets = DrawableWrapper::getOpticalInsets();
    Rect tmp;
    getInsets(tmp);
    return Insets::of(
            contentInsets.left + tmp.left,
            contentInsets.top + tmp.top,
            contentInsets.right + tmp.width,
            contentInsets.bottom + tmp.height);
}

int InsetDrawable::getOpacity() {
    int opacity = getDrawable()->getOpacity();
    Rect tmp;
    getInsets(tmp);
    if (opacity == OPAQUE && (tmp.left > 0 || tmp.top > 0 || tmp.width > 0 || tmp.height > 0)) {
        return TRANSLUCENT;
    }
    return opacity;
}

void InsetDrawable::onBoundsChange(const Rect&bounds){
    Rect r=bounds;
  
    r.left  += mState->mInsetLeft.getDimension(bounds.width);
    r.top   += mState->mInsetTop.getDimension(bounds.height);
    r.width -= mState->mInsetRight.getDimension(bounds.width);
    r.height-= mState->mInsetBottom.getDimension(bounds.height);
    DrawableWrapper::onBoundsChange(r);
}

std::shared_ptr<Drawable::ConstantState>InsetDrawable::getConstantState(){
    return mState;
}

Drawable*InsetDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string src=atts.getString("src");;
    int inset =atts.getInt("inset");
    int insetLeft =atts.getInt("insetLeft");
    int insetTop  =atts.getInt("insetTop");
    int insetRight =atts.getInt("insetRight");
    int insetBottom =atts.getInt("insetBottom");
    Drawable*d=Drawable::inflate(ctx,src);
    if(inset>0)
        return new InsetDrawable(d,inset);
    else
        return new InsetDrawable(d,insetLeft,insetTop,insetRight,insetBottom);
}

}

