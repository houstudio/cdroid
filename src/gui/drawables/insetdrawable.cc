#include <drawables/insetdrawable.h>
#include <cdlog.h>
namespace cdroid{

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

void InsetDrawable::onBoundsChange(const Rect&bounds){
    Rect r=bounds;
    Rect&mInset=mState->mInset;
    r.x += mInset.x;
    r.y += mInset.y;
    r.width -=(mInset.x + mInset.width);
    r.height-=(mInset.y + mInset.height);
    DrawableWrapper::onBoundsChange(r);
    LOGD("inset=%d,%d,%d,%d",mInset.x,mInset.y,mInset.width,mInset.height);
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

