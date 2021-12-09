#include <drawables/shape.h>
#include <color.h>
#include <cdlog.h>

namespace cdroid{

Shape::Shape(){
    mStrokeWidth=0;
    mRect.set(0,0,0,0);
    mRebuildGradient=false;
    mGradientType=0;
    mGradientAngle=0;
    mGradientRadius=0;
    mDashGap=0;
    mDashWidth=0;
    bUseLevel=false;
    mGradientCenterX=0;
    mGradientCenterY=0;
}

Shape::Shape(const Shape&o){
    mStrokeWidth=o.mStrokeWidth;
    mRect=o.mRect;
    mRebuildGradient=o.mRebuildGradient;
    mGradientType=o.mGradientType;
    mGradientAngle=o.mGradientAngle;
    mGradientRadius=o.mGradientRadius;
    mDashGap=o.mDashGap;
    mGradientColors=o.mGradientColors;
    mStrokeColor=o.mStrokeColor;
    mRebuildGradient=true;
    mGradientCenterX=o.mGradientCenterX;
    mGradientCenterY=o.mGradientCenterY;
    mPaint=nullptr;
}

int Shape::getWidth()const{
    return mRect.width;
}

int Shape::getHeight()const{
    return mRect.height;
}

const Rect&Shape::rect()const{
    return mRect;
}

void Shape::resize(int w,int h){
    if(w<0)w=0;
    if(h<0)h=0;
    if(mRect.width!=w||mRect.height!=h){
        mRect.width=w;
        mRect.height=h;
        onResize(w,h);
    }
}

void Shape::setUseLevel(bool b){
    bUseLevel=b;
}
void Shape::setStrokeColor(int color){
    mStrokeColor=color;
}
void Shape::setStrokeSize(float width){
    mStrokeWidth=width;
}

void Shape::setStrokeDash(int width,int gap){
    mDashGap=gap;
    mDashWidth=width;
}
int Shape::getStroke(int& color)const{
    color=mStrokeColor;
    return mStrokeWidth;
}

void Shape::setGradientColors(const std::vector<uint32_t>&cls){
    mGradientColors=cls;
    mRebuildGradient=true;
}

void Shape::setSolidColor(int color){
    mGradientColors.resize(1);
    mGradientColors[0]=color;
    mRebuildGradient=true;
}

void Shape::setGradientRadius(float r){
    mRebuildGradient=(mGradientRadius!=r);
    mGradientRadius=r;
}

void Shape::setGradientAngle(float a){
    mRebuildGradient=(mGradientAngle!=a);
    mGradientAngle=a;
}

void Shape::setGradientType(int gt){
    mRebuildGradient=(mGradientType!=gt);
    mGradientType=gt;
    LOGD("mGradientType=%d",mGradientType);
}

void Shape::setGradientCenterX(float x){
    mGradientCenterX=x;
}

void Shape::setGradientCenterY(float y){
    mGradientCenterY=y;
}

void Shape::applyGradients(){
    std::vector<Color>cls;
    cls.resize(mGradientColors.size());
    for(int i=0;i<mGradientColors.size();i++){
        cls[i]=Color(mGradientColors[i]);
    }
    RefPtr<Cairo::Gradient> g=std::make_shared<Cairo::Gradient>(mPaint->cobj());
    switch(mGradientColors.size()){
    case 2:
        g->add_color_stop_rgba(.0f,cls[0].red(),cls[0].green(),cls[0].blue(),cls[0].alpha());
        g->add_color_stop_rgba(1.f,cls[1].red(),cls[1].green(),cls[1].blue(),cls[1].alpha());
        break;
    case 3:
        g->add_color_stop_rgba(.0f,cls[0].red(),cls[0].green(),cls[0].blue(),cls[0].alpha());
        g->add_color_stop_rgba(.5f,cls[1].red(),cls[1].green(),cls[1].blue(),cls[1].alpha());
        g->add_color_stop_rgba(1.f,cls[2].red(),cls[2].green(),cls[2].blue(),cls[2].alpha());
        break;
    }
}
void Shape::rebuildPattern(const Rect&r){
    mRebuildGradient=false;
    switch(mGradientType){
    case Gradient::SOLID:{
           const Color c(mGradientColors[0]);
           mPaint=SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
        }break;
    case Gradient::LINEAR:{
           const int angleIndex=mGradientAngle/45;
           const int wh=std::max(mRect.width,mRect.height);
           switch(angleIndex%8){
           case 0: mPaint=LinearGradient::create(0,0,wh,0) ; break;
           case 1: mPaint=LinearGradient::create(0,wh,wh,0); break;
           case 2: mPaint=LinearGradient::create(0,wh,0,0) ; break;
           case 3: mPaint=LinearGradient::create(wh,wh,0,0); break; 
           case 4: mPaint=LinearGradient::create(wh,0,0,0) ; break;
           case 5: mPaint=LinearGradient::create(wh,0,0,wh); break;
           case 6: mPaint=LinearGradient::create(0,0,0,wh) ; break;
           case 7: mPaint=LinearGradient::create(0,0,wh,wh); break;
           }
           applyGradients();
        }break;
    case Gradient::RADIAL:{
           const float cx=mGradientCenterX*mRect.width+r.left;
           const float cy=mGradientCenterY*mRect.height+r.top;
           const float pd=mGradientAngle*M_PI/180.f;
           LOGV("center=%.2f,%.2f cx,cy=%.2f,%.2f mRadius=%f mAngle=%.2f",mGradientCenterX,mGradientCenterY,cx,cy,mGradientRadius,mGradientAngle);
           mPaint=RadialGradient::create(cx,cy,mGradientRadius,cx,cy,mGradientRadius);//,cx,cy,std::max(mRect.width,mRect.height)/2.f);
           applyGradients();
        }break;
    case Gradient::SWEEP:
    default:  
        LOGE("unsupported gradient type %d",mGradientType);
        break;
    }
}

void Shape::fill_stroke(Canvas&canvas){
    const bool bstroke=(mStrokeWidth>0) && (mStrokeColor&0x80000000);
    if(mGradientColors.size()){
        rebuildPattern(rect());
        mPaint->set_matrix(canvas.get_matrix());
        canvas.set_source(mPaint);
        if(bstroke)
            canvas.fill_preserve();
        else
            canvas.fill();
    }
    if(bstroke){
        canvas.set_line_width(mStrokeWidth);
        if(mDashWidth&&mDashGap){
            std::vector<double>dashs={(double)mDashWidth,(double)mDashGap};
            canvas.set_dash(dashs,0);
        }
        canvas.set_color(mStrokeColor);
        canvas.stroke();
    }
}

RectShape::RectShape(){
    mRect.set(0,0,0,0);
}

RectShape::RectShape(const RectShape&o):Shape(o){
    mRect=o.mRect;
}

void RectShape::setOuterRadii(const std::vector<float>&v){
    mOuterRadii=v;
}
void RectShape::setInnerRadii(const std::vector<float>&v){
    mInnerRadii=v;
}

void RectShape::setRadius(float radius){
    mOuterRadii.resize(8);
    for(int i=0;i<8;i++)mOuterRadii[i]=radius; 
}

Shape*RectShape::clone()const{
    return new RectShape(*this);
}

void RectShape::onResize(int width,int height){
    mRect.set(0,0,width,height);
}

void RectShape::draw(Canvas&canvas){
    if(mPaint)canvas.set_source(mPaint);
    canvas.rectangle(0,0,mRect.width,mRect.height);
    fill_stroke(canvas);
}

ArcShape::ArcShape(float startAngle,float sweepAngle):RectShape(){
    mStartAngle = startAngle;
    mSweepAngle = sweepAngle;
}

ArcShape::ArcShape(const ArcShape&o):RectShape(o){
    mStartAngle=o.mStartAngle;
    mSweepAngle=o.mSweepAngle;
}

Shape*ArcShape::clone()const{
    return new ArcShape(*this);
}

void ArcShape::draw(Canvas&canvas){
    const Rect r=rect();
    canvas.save();
    canvas.scale(1.0 , (float)r.height/r.width);
    canvas.arc(r.left+r.width/2 , r.top+r.height/2, r.width/2, mStartAngle,mSweepAngle);
    canvas.scale(1.0 , (float)r.width/r.height);
    canvas.line_to(r.left+r.width/2,r.left+r.height/2);
    canvas.close_path();
    fill_stroke(canvas);
    canvas.restore();
}

OvalShape::OvalShape():RectShape(){
    mInnerRadius=.0f;
    mInnerRadiusRatio=.0f;
    mThickness=.0f;
    mThicknessRatio=.0f;
}

void OvalShape::setInnerRadius(float v){
    mInnerRadius=v;
}

float OvalShape::getInnerRadius()const{
    return mInnerRadius;
}

void OvalShape::setInnerRadiusRatio(float v){
    mInnerRadiusRatio=v;
}

float OvalShape::getInnerRadiusRatio()const{
    return mInnerRadiusRatio;
}

void OvalShape::setThickness(float v){
    mThickness=v;
}

float OvalShape::getThickness()const{
    return mThickness;
}

void OvalShape::setThicknessRatio(float v){
    mThicknessRatio=v;
}

float OvalShape::getThicknessRatio()const{
    return mThicknessRatio;
}

void OvalShape::draw(Canvas&canvas){
    Rect r=rect();

    //draw outer     
    canvas.translate(r.left+r.width/2,r.top+r.height/2);
    canvas.scale(1.,(float)r.height/r.width);
    canvas.arc(0,0,r.width/2,0,M_PI*2);
    canvas.scale(1.,(float)r.width/r.height);
    canvas.translate(-r.left-r.width/2,-r.top-r.height/2);

    //draw inner
    float innerRadius=mInnerRadius;
    if(mInnerRadius==.0f)
       innerRadius=r.width/2.f-mThickness;
    if(innerRadius>.0){
        float ratio=(float)r.width/r.height;
        if(mInnerRadiusRatio)ratio=mInnerRadiusRatio;
        canvas.translate(r.left+r.width/2,r.top+r.height/2);
        canvas.scale(ratio,1.f);
        canvas.arc(0,0,innerRadius,0,M_PI*2);
        canvas.scale(1.f/ratio,1.f); 
        canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
    }
    fill_stroke(canvas);
    canvas.translate(-(r.left+r.width)/2,-(r.top+r.height/2));
}

RoundRectShape::RoundRectShape():RectShape(){
    mOuterRadii.resize(8);
    mInnerRadii.resize(8);
    for(int i=0;i<8;i++)mOuterRadii[i]=5;
    for(int i=0;i<8;i++)mInnerRadii[i]=5;
}

RoundRectShape::RoundRectShape(const std::vector<float>&outRadii,const Rect&inset,const std::vector<float>&innerRadii){
    mOuterRadii=outRadii;
    mInnerRadii=innerRadii;
    mInset=inset;
}

RoundRectShape::RoundRectShape(const RoundRectShape&o):RectShape(o){
    mOuterRadii=o.mOuterRadii;
    mInnerRadii=o.mInnerRadii;
    mInset=o.mInset;
}

Shape*RoundRectShape::clone()const{
    return new RoundRectShape(*this);
}

void RoundRectShape::onResize(int w, int h){
    RectShape::onResize(w,h);
    Rect r=rect();
    mInnerRect.set(r.left+mInset.left,r.top+mInset.top,
        r.right()-mInset.left-mInset.width,r.bottom()-mInset.top-mInset.height);
}

void RoundRectShape::drawRound(Canvas&canvas,const Rect&r,const std::vector<float>&radii){
    constexpr double degree = M_PI/180.f;
    if(radii.size()==0)
        canvas.rectangle(r.left,r.top,r.width,r.height);
    else{
        float db=180.f;
        float pts[8];
        pts[0]=r.left+radii[0];      pts[1]=r.top +radii[1];
        pts[2]=r.right()-radii[2];   pts[3]=r.top+radii[3];
        pts[4]=r.right()-radii[4];   pts[5]=r.bottom()-radii[5];
        pts[6]=r.left+radii[6];      pts[7]=r.bottom()-radii[7];
        for(int i=0;i<8;i+=2){
            canvas.translate(pts[i],pts[i+1]);
            canvas.scale(1,(float)radii[i+1]/radii[i]);
            canvas.arc(0,0,radii[i],db*degree,(db+90)*degree);
            canvas.scale(1,(float)radii[i]/radii[i+1]);
            canvas.translate(-pts[i],-pts[i+1]);
            db+=90.f;
        }canvas.line_to(pts[0]-radii[0],pts[1]);
    }
}

void RoundRectShape::draw(Canvas&canvas){
    Rect r=rect();
    drawRound(canvas,r,mOuterRadii);
    if(mInnerRadii.size()){
        r.inflate(-50,-50);
        drawRound(canvas,r,mInnerRadii);
        canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
    }
    fill_stroke(canvas);
}
}
