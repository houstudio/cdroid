#include <drawables/shapedrawable.h>
namespace cdroid{

ShapeDrawable::ShapeState::ShapeState(){
    mAlpha=255;
    mShape=nullptr;
    mTint =nullptr;
    mTintMode =TintMode::NONOP;
    mIntrinsicWidth=0;
    mIntrinsicHeight=0;
    mPadding.set(0,0,0,0);
}

ShapeDrawable::ShapeState::ShapeState(const ShapeState&orig){
    mIntrinsicWidth = orig.mIntrinsicWidth;
    mIntrinsicHeight = orig.mIntrinsicHeight;
    mPadding=orig.mPadding;
    mAlpha  = orig.mAlpha;
    mShape  = orig.mShape?orig.mShape->clone():nullptr;
    mTint   = new ColorStateList(*orig.mTint);
    mTintMode =orig.mTintMode;
}

Drawable* ShapeDrawable::ShapeState::newDrawable(){
    return new ShapeDrawable(shared_from_this());
}

ShapeDrawable::ShapeState::~ShapeState(){
    delete mShape;
    delete mTint;
}

int ShapeDrawable::ShapeState::getChangingConfigurations()const{
    return mChangingConfigurations;
}

////////////////////////////////////////////////////////////////////////////////////////////////

ShapeDrawable::ShapeDrawable(std::shared_ptr<ShapeState>state){
    mShapeState=state;
    mTintFilter=nullptr;
}

ShapeDrawable::ShapeDrawable(){
    mShapeState=std::make_shared<ShapeState>();
    mTintFilter=nullptr;
}

std::shared_ptr<Drawable::ConstantState>ShapeDrawable::getConstantState(){
    return mShapeState;
}

void ShapeDrawable::setShape(Shape*shape){
    if(mShapeState->mShape)
       delete mShapeState->mShape;
    mShapeState->mShape=shape;
    updateShape();
}

void ShapeDrawable::onBoundsChange(const Rect&bounds){
    Drawable::onBoundsChange(bounds);
    updateShape();
}

bool ShapeDrawable::onStateChange(const std::vector<int>&stateset){
    if(mShapeState->mTint&&mShapeState->mTintMode!=TintMode::NONOP){
        mTintFilter= updateTintFilter(mTintFilter,mShapeState->mTint,mShapeState->mTintMode);
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
    mShapeState->mPadding=padding;
}

void ShapeDrawable::setPadding(int left, int top, int right, int bottom){
    if ((left | top | right | bottom) == 0) {
        mShapeState->mPadding.set(0,0,0,0);
    } else {
        mShapeState->mPadding.set(left, top, right, bottom);
    }
}

void ShapeDrawable::setAlpha(int alpha){
    mShapeState->mAlpha =alpha;
    invalidateSelf();
}

int ShapeDrawable::ShapeDrawable::getAlpha()const{
    return mShapeState->mAlpha;
}

int ShapeDrawable::getOpacity(){
    switch(mShapeState->mAlpha){
    case 255: return PixelFormat::OPAQUE;
    case   0: return PixelFormat::TRANSPARENT;
    default : return PixelFormat::TRANSLUCENT; 
    }
}

void ShapeDrawable::setTintList(ColorStateList*tint){
    mShapeState->mTint = tint;
    mTintFilter=updateTintFilter(mTintFilter,tint,mShapeState->mTintMode); 
    invalidateSelf();
}

void ShapeDrawable::setTintMode(int tintMode){
    mShapeState->mTintMode = tintMode;
    mTintFilter=updateTintFilter(mTintFilter,mShapeState->mTint,tintMode);
    invalidateSelf();
}

void ShapeDrawable::setColorFilter(ColorFilter*colorFilter){
    invalidateSelf();
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
    mTintFilter = updateTintFilter(mTintFilter, mShapeState->mTint, mShapeState->mTintMode);
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
    const Rect&r = getBounds();
    if(mShapeState->mShape!=nullptr){
        canvas.translate(r.left,r.top);
        mShapeState->mShape->draw(canvas);
        canvas.translate(-r.left,-r.top);
    }
}

Drawable*ShapeDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string type=atts.getString("shape");//rectangle,line,oval,ring
    Shape*shape=nullptr;
    if(type.compare("rectangle")==0)  shape = new RoundRectShape();
    else if(type.compare("ring")==0||type.compare("oval")==0){
        OvalShape*oval = new OvalShape();
        shape =oval;
        oval->setThickness(atts.getInt("thickness"));
        oval->setThicknessRatio(atts.getFloat("thicknessRatio",.0f));
        oval->setInnerRadius(atts.getInt("innerRadius",0)); 
        oval->setInnerRadiusRatio(atts.getFloat("innerRadiusRatio",.0f));
    }else if(type.compare( "arc")==0)  shape = new ArcShape(0,0);
    ShapeDrawable*d=new ShapeDrawable();
    d->setShape(shape);
    return d;
}

}
