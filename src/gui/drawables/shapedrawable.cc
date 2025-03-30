#include <drawables/shapedrawable.h>
namespace cdroid{

ShapeDrawable::ShapeState::ShapeState(){
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
    mIntrinsicWidth = orig.mIntrinsicWidth;
    mIntrinsicHeight= orig.mIntrinsicHeight;
    mPadding= orig.mPadding;
    mAlpha  = orig.mAlpha;
    if(mShape)
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
    //delete mTint;mTint cant be destroied
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
    delete mTintFilter;
}

void ShapeDrawable::getOutline(Outline& outline) {
    if (mShapeState->mShape != nullptr) {
        LOGD("TODO");// mShapeState->mShape->getOutline(outline);
        outline.setAlpha(getAlpha() / 255.0f);
    }
}

std::shared_ptr<Drawable::ConstantState>ShapeDrawable::getConstantState(){
    return mShapeState;
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

int ShapeDrawable::getOpacity(){
    switch(mShapeState->mAlpha){
    case 255: return PixelFormat::OPAQUE;
    case   0: return PixelFormat::TRANSPARENT;
    default : return PixelFormat::TRANSLUCENT; 
    }
}

void ShapeDrawable::setTintList(const ColorStateList*tint){
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
        if(mTintFilter){
            canvas.save();
            canvas.rectangle(0,0,r.width,r.height);
            canvas.clip();
            canvas.push_group();
        }
        mShapeState->mShape->draw(canvas,r.left,r.top);
        if(mTintFilter){
            mTintFilter->apply(canvas,r);
            canvas.pop_group_to_source();
            canvas.paint();
            canvas.restore();
        }
        canvas.translate(-r.left,-r.top);
    }
}

void ShapeDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    Drawable::inflate(parser,atts);
    updateStateFromTypedArray(atts);

    int type;
    XmlPullParser::XmlEvent event;
    const int outerDepth = parser.getDepth();
    while (((type = parser.next(event)) != XmlPullParser::END_DOCUMENT)
            && (type != XmlPullParser::END_TAG || parser.getDepth() > outerDepth)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();
        // call our subclass
        if (!inflateTag(name,parser,event.attributes)) {
            LOGW("Unknown element: %s for ShapeDrawable %p",name.c_str(),this);
        }
    }

    // Update local properties.
    updateLocalState();
#if 0
    const std::string type = atts.getString("shape");//rectangle,line,oval,ring
    Shape*shape = nullptr;
    if(type.compare("rectangle")==0)  shape = new RoundRectShape();
    else if(type.compare("ring")==0||type.compare("oval")==0){
        OvalShape*oval = new OvalShape();
        shape = oval;
        oval->setThickness(atts.getInt("thickness"));
        oval->setThicknessRatio(atts.getFloat("thicknessRatio",.0f));
        oval->setInnerRadius(atts.getInt("innerRadius",0)); 
        oval->setInnerRadiusRatio(atts.getFloat("innerRadiusRatio",.0f));
    }else if(type.compare( "arc")==0){
        const float start= atts.getFloat("startAngle",0);
        const float end  = atts.getFloat("endAngle",360);
        shape = new ArcShape(start,end);
    }
    ShapeDrawable*d = new ShapeDrawable();
    d->setShape(shape);
#endif
}

void ShapeDrawable::updateStateFromTypedArray(const AttributeSet&a) {
    auto state = mShapeState;

    // Account for any configuration changes.
    //state.mChangingConfigurations |= a.getChangingConfigurations();

    // Extract the theme attributes, if any.
    //state.mThemeAttrs = a.extractThemeAttrs();

    //int color = paint.getColor();
    //color = a.getColor("color", color);

    //boolean dither = paint.isDither();
    state->mDither = a.getBoolean("dither", state->mDither);

    state->mIntrinsicWidth = (int) a.getDimension("width", state->mIntrinsicWidth);
    state->mIntrinsicHeight = (int) a.getDimension("height", state->mIntrinsicHeight);

    const int tintMode = a.getInt("tintMode", -1);
    if (tintMode != -1) {
        //state->mBlendMode = Drawable::parseBlendMode(tintMode, BlendMode::SRC_IN);
    }

    ColorStateList* tint = a.getColorStateList("tint");
    if (tint != nullptr) {
        state->mTint = tint;
    }
}

int ShapeDrawable::inflateTag(const std::string&name,XmlPullParser&parser,const AttributeSet&a){
    if (name.compare("padding")==0) {
        setPadding(a.getDimensionPixelOffset("left", 0),
                a.getDimensionPixelOffset("top", 0),
                a.getDimensionPixelOffset("right", 0),
                a.getDimensionPixelOffset("bottom", 0));
        return true;
    }
    return false;
}

}
