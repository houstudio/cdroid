#include <drawables/shapedrawable.h>
namespace cdroid{

ShapeDrawable::ShapeState::ShapeState(){
    mAlpha=255;
    mShape=nullptr;
    mIntrinsicWidth=0;
    mIntrinsicHeight=0;
    mPadding.set(0,0,0,0);
}

ShapeDrawable::ShapeState::ShapeState(const ShapeState&orig){
    mIntrinsicWidth = orig.mIntrinsicWidth;
    mIntrinsicHeight = orig.mIntrinsicHeight;
    mPadding=orig.mPadding;
    mAlpha = orig.mAlpha;
    mShape =  orig.mShape?orig.mShape->clone():nullptr;
}

Drawable* ShapeDrawable::ShapeState::newDrawable(){
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
    mShapeState=state;
}

ShapeDrawable::ShapeDrawable(){
    mShapeState=std::make_shared<ShapeState>();
}

std::shared_ptr<Drawable::ConstantState>ShapeDrawable::getConstantState(){
    return mShapeState;
}

void ShapeDrawable::setShape(Shape*shape){
    mShapeState->mShape=shape;
    updateShape();
}

void ShapeDrawable::onBoundsChange(const RECT&bounds){
    Drawable::onBoundsChange(bounds);
    updateShape();
}

Shape*ShapeDrawable::getShape()const{
    return mShapeState->mShape;
}

void ShapeDrawable::updateShape(){
    if (mShapeState->mShape != nullptr) {
        const RECT& r = getBounds();
        mShapeState->mShape->resize(r.width,r.height);
    }
    invalidateSelf();
}

bool ShapeDrawable::getPadding(RECT&padding){
    padding=mShapeState->mPadding;
    return true;
}

void ShapeDrawable::setPadding(const RECT& padding){
    mShapeState->mPadding=padding;
}

void ShapeDrawable::setPadding(int left, int top, int right, int bottom){
    if ((left | top | right | bottom) == 0) {
        mShapeState->mPadding.set(0,0,0,0);
    } else {
        mShapeState->mPadding.set(left, top, right, bottom);
    }
}

int ShapeDrawable::getIntrinsicWidth()const{
    return mShapeState->mIntrinsicWidth;
}

int ShapeDrawable::getIntrinsicHeight()const{
    return mShapeState->mIntrinsicHeight;
}

void ShapeDrawable::setIntrinsicWidth(int width){
    mShapeState->mIntrinsicWidth=width;
    invalidateSelf();
}

void ShapeDrawable::setIntrinsicHeight(int height){
    mShapeState->mIntrinsicHeight = height;
    invalidateSelf();
}

void ShapeDrawable::updateLocalState(){
    //mTintFilter = updateTintFilter(mTintFilter, mShapeState.mTint, mShapeState.mTintMode);
}

Drawable*ShapeDrawable::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        mShapeState=std::make_shared<ShapeState>(*mShapeState);
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
    const RECT&r = getBounds();
    if(mShapeState->mShape!=nullptr){
        canvas.translate(r.x,r.y);
        mShapeState->mShape->draw(canvas);
        canvas.translate(-r.x,-r.y);
    }
}

Drawable*ShapeDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string type=atts.getString("shape");
    Shape*shape=nullptr;
    if(type.compare("rectangle")==0)  shape = new RectShape();
    else if(type.compare("roundrect")==0)shape=new RoundRectShape();
    else if(type.compare("oval")==0)  shape = new OvalShape();
    else if(type.compare("ring")==0){
        shape = new OvalShape();
        shape->setStrokeSize(atts.getInt("thickness"));
    }else if(type.compare( "arc")==0)  shape = new ArcShape(0,0);
    ShapeDrawable*d=new ShapeDrawable();
    d->setShape(shape);
    return d;
}

}
