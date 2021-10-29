#include <drawables/shape.h>
#include <color.h>
#include <cdlog.h>

namespace cdroid{

Shape::Shape(){
    mStrokeWidth=0;
    mWidth=mHeight=0;
    mRebuildGradient=false;
    mGradientType=0;
    mAngle=0;
    mRadius=0;
    mDashGap=0;
    mDashWidth=0;
    bUseLevel=false;
}

Shape::Shape(const Shape&o){
    mStrokeWidth=o.mStrokeWidth;
    mWidth=o.mWidth;
    mHeight=o.mHeight;
    mRebuildGradient=o.mRebuildGradient;
    mGradientType=o.mGradientType;
    mAngle=o.mAngle;
    mRadius=o.mRadius;
    mDashGap=o.mDashGap;
    mGradientColors=o.mGradientColors;
    mStrokeColor=o.mStrokeColor;
    mRebuildGradient=true;
    mCenterX=o.mCenterX;
    mCenterY=o.mCenterY;
    mPaint=nullptr;
}

void Shape::resize(int w,int h){
    if(w<0)w=0;
    if(h<0)h=0;
    if(mWidth!=w||mHeight!=h){
        mWidth=w;
        mHeight=h;
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
    mRebuildGradient=(mRadius!=r);
    mRadius=r;
}

void Shape::setGradientAngle(float a){
    mRebuildGradient=(mAngle!=a);
    mAngle=a;
}

void Shape::setGradientType(int gt){
    mRebuildGradient=(mGradientType!=gt);
    mGradientType=gt;
    LOGD("mGradientType=%d",mGradientType);
}

void Shape::setGradientCenterX(float x){
    mCenterX=x;
}

void Shape::setGradientCenterY(float y){
    mCenterY=y;
}

void Shape::rebuildPattern(const Rect&r){
    mRebuildGradient=false;
    Color*cls[3]={nullptr,nullptr,nullptr};
    for(int i=0;i<mGradientColors.size();i++)
        cls[i]=new Color(mGradientColors[i]);
    switch(mGradientType){
    case 0:{
           const Color*c=cls[0];
           mPaint=SolidPattern::create_rgba(c->red(),c->green(),c->blue(),c->alpha());
        }break;
    case 1:{
           float cx=mCenterX*mWidth;
           float cy=mCenterY*mHeight;
           float pd=mAngle*M_PI*2/360.;
           RefPtr<Gradient>g=LinearGradient::create(r.left*sin(pd),r.top*cos(pd),r.right()*sin(pd),r.right()*cos(pd));
           g->add_color_stop_rgba(.0,cls[0]->red(),cls[0]->green(),cls[0]->blue(),cls[0]->alpha());
           g->add_color_stop_rgba(.5,cls[1]->red(),cls[1]->green(),cls[1]->blue(),cls[1]->alpha());
           g->add_color_stop_rgba(1.,cls[2]->red(),cls[2]->green(),cls[2]->blue(),cls[2]->alpha());
           mPaint=g;
        }break;
    case 2:{
           float cx=mCenterX*mWidth+r.left;
           float cy=mCenterY*mHeight+r.top;
           float pd=mAngle*M_PI*2/360.;
           LOGV("center=%.2f,%.2f cx,cy=%.2f,%.2f mRadius=%f mAngle=%.2d",mCenterX,mCenterY,cx,cy,mRadius,mAngle);
           RefPtr<Gradient>g=RadialGradient::create(cx,cy,0,cx,cy,mRadius);
           g->add_color_stop_rgba(.0,cls[0]->red(),cls[0]->green(),cls[0]->blue(),cls[0]->alpha());
           g->add_color_stop_rgba(.5,cls[1]->red(),cls[1]->green(),cls[1]->blue(),cls[1]->alpha());
           g->add_color_stop_rgba(1.,cls[2]->red(),cls[2]->green(),cls[2]->blue(),cls[2]->alpha());
           mPaint=g;
        }
    }
    for(int i=0;i<mGradientColors.size();i++){
        delete cls[i];
        Color c(mGradientColors[i]);
        LOGV("GradientColors[%d]=%x rgba=%.2f,%.2f,%.2f,%.2f",i,mGradientColors[i],c.red(),c.green(),c.blue(),c.alpha());
    }
}

void Shape::fill_stroke(Canvas&canvas){
    const bool bstroke=(mStrokeWidth>0) && (mStrokeColor&0x80000000);
    if(mGradientColors.size()){
        rebuildPattern(rect());
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

Shape*RectShape::clone()const{
    return new RectShape(*this);
}

void RectShape::onResize(int width,int height){
    mRect.set(0,0,width,height);

}

void RectShape::draw(Canvas&canvas){
    if(mPaint)canvas.set_source(mPaint);
    canvas.rectangle(0,0,mWidth,mHeight);
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

void OvalShape::draw(Canvas&canvas){
    Rect r=rect();

    canvas.translate(r.left+r.width/2,r.top+r.height/2);
    canvas.scale(1.,(float)r.height/r.width);
    canvas.arc(0,0,r.width/2,0,M_PI*2);
    canvas.scale(1.,(float)r.width/r.height);

    fill_stroke(canvas);
    canvas.translate(-(r.left+r.width)/2,-(r.top+r.height/2));
}

RoundRectShape::RoundRectShape():RectShape(){
    mOuterRadii.resize(8);
    mInnerRadii.resize(8);
    for(int i=0;i<8;i++)mOuterRadii[i]=5;
    for(int i=0;i<8;i++)mInnerRadii[i]=5;
}
RoundRectShape::RoundRectShape(const std::vector<int>&outRadii,const Rect&inset,const std::vector<int>&innerRadii){
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

void RoundRectShape::setRadius(int radius){
    
}

void RoundRectShape::onResize(int w, int h){
    RectShape::onResize(w,h);
    Rect r=rect();
    mInnerRect.set(r.left+mInset.left,r.top+mInset.top,
                r.right()-mInset.left-mInset.width,r.bottom()-mInset.top-mInset.height);
}

void RoundRectShape::draw(Canvas&canvas){
    const double degree = M_PI/180;
    const Rect r=rect();
    const int *radius=mOuterRadii.data();
    const float pts[8]={
        (float)r.left+radius[0]        ,  (float)r.top+radius[1],           
        (float)r.left+mWidth-radius[2] ,  (float)r.top+radius[3],
        (float)r.left+mWidth-radius[4] ,  (float)r.top+mHeight-radius[5],     
        (float)r.left+radius[6]        ,  (float)r.top+mHeight-radius[7] 
    };

    float db=180;
    canvas.begin_new_sub_path();
    for(int i=0;i<8;i+=2){
        canvas.translate(pts[i],pts[i+1]);
        canvas.scale(1,(float)radius[i+1]/radius[i]);
        canvas.arc(0,0,radius[i],db*degree,(db+90)*degree);
        canvas.scale(1,(float)radius[i]/radius[i+1]);
        canvas.translate(-pts[i],-pts[i+1]);
        db+=90.;
    }
    canvas.line_to(pts[0]-radius[0],pts[1]);
    canvas.close_path();
#if 0
    radius=mInnerRadii.data();
    canvas.begin_new_sub_path();
    for(int i=0;i<8;i+=2){
        canvas.translate(pts[i],pts[i+1]);
        canvas.scale(1,(float)radius[i+1]/radius[i]);
        canvas.arc(0,0,radius[i],db*degree,(db+90)*degree);
        canvas.scale(1,(float)radius[i]/radius[i+1]);
        canvas.translate(-pts[i],-pts[i+1]);
        db+=90.;
    }
    canvas.line_to(pts[0]-radius[0],pts[1]);
    canvas.close_path();
#endif
    fill_stroke(canvas);
}
}
