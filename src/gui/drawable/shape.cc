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
#include <drawable/shape.h>
#include <cairomm/mesh_pattern.h>
#include <core/color.h>
#include <porting/cdlog.h>
using namespace Cairo;
namespace cdroid{

Shape::Shape(){
    mStrokeWidth=0;
    mWidth=mHeight=0;
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
    mWidth=o.mWidth;
    mHeight=o.mHeight;
    mGradientType=o.mGradientType;
    mGradientAngle=o.mGradientAngle;
    mGradientRadius=o.mGradientRadius;
    mDashGap=o.mDashGap;
    mGradientColors=o.mGradientColors;
    mStrokeColor=o.mStrokeColor;
    mGradientCenterX=o.mGradientCenterX;
    mGradientCenterY=o.mGradientCenterY;
    mPaint=nullptr;
}

int Shape::getWidth()const{
    return mWidth;
}

int Shape::getHeight()const{
    return mHeight;
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
}

void Shape::setSolidColor(int color){
    mGradientColors.resize(1);
    mGradientColors[0]=color;
}

void Shape::setGradientRadius(float r){
    mGradientRadius=r;
}

void Shape::setGradientAngle(float a){
    mGradientAngle=a;
}

void Shape::setGradientType(int gt){
    mGradientType=gt;
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
        LOGV("gradient.colors[%d/%d]=%x",i,mGradientColors.size(),cls[i].toArgb());
    }
    if(mPaint->get_type()!=Pattern::Type::MESH){
        RefPtr<Cairo::Gradient>pat=std::make_shared<Cairo::Gradient>(mPaint->cobj(),false);
        switch(mGradientColors.size()){
        case 2:
           pat->add_color_stop_rgba(.0f,cls[0].red(),cls[0].green(),cls[0].blue(),cls[0].alpha());
           pat->add_color_stop_rgba(1.f,cls[1].red(),cls[1].green(),cls[1].blue(),cls[1].alpha());
           break;
        case 3:
           pat->add_color_stop_rgba(.0f,cls[0].red(),cls[0].green(),cls[0].blue(),cls[0].alpha());
           pat->add_color_stop_rgba(.5f,cls[1].red(),cls[1].green(),cls[1].blue(),cls[1].alpha());
           pat->add_color_stop_rgba(1.f,cls[2].red(),cls[2].green(),cls[2].blue(),cls[2].alpha());
        break;
       }
    }else{
        RefPtr<Cairo::SweepGradient>swp=std::dynamic_pointer_cast<Cairo::SweepGradient>(mPaint);
        const uint32_t*cc=mGradientColors.data();
        const size_t ccnum=mGradientColors.size();
        double angle=0;
        for(int i=0;i<ccnum;i++){
            //swp->add_sector(angle,mGradientColors[i],angle+M_PI*2./ccnum,mGradientColors[(i+1)%ccnum]);
            LOGV("add_sector_patch(%.2f) color:%x->%x",angle,mGradientColors[i],mGradientColors[(i+1)%ccnum]);
            angle+=M_PI*2./ccnum;
        }
    }
}
void Shape::rebuildPattern(int x,int y){
    const float cx=mGradientCenterX*mWidth+x;
    const float cy=mGradientCenterY*mHeight+y;
    switch(mGradientType){
    case Gradient::SOLID:{
           const Color c(mGradientColors[0]);
           mPaint=SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
        }break;
    case Gradient::LINEAR:{
           const int angleIndex=int(mGradientAngle/45);
           const int wh=std::max(mWidth,mHeight);
           switch(angleIndex%8){
           case 0: mPaint=LinearGradient::create(0,0,mWidth,0) ; break;
           case 1: mPaint=LinearGradient::create(0,wh,wh,0); break;
           case 2: mPaint=LinearGradient::create(0,mHeight,0,0) ; break;
           case 3: mPaint=LinearGradient::create(wh,wh,0,0); break; 
           case 4: mPaint=LinearGradient::create(mWidth,0,0,0) ; break;
           case 5: mPaint=LinearGradient::create(wh,0,0,wh); break;
           case 6: mPaint=LinearGradient::create(0,0,0,mHeight) ; break;
           case 7: mPaint=LinearGradient::create(0,0,wh,wh); break;
           }
           LOGV("angleIndex=%d size=%dx%d %d colors",angleIndex,mWidth,mHeight,mGradientColors.size());
           applyGradients();
        }break;
    case Gradient::RADIAL:{
           const double pd=mGradientAngle*M_PI/180.f;
           LOGV("center=%.2f,%.2f cx,cy=%.2f,%.2f mRadius=%f mAngle=%.2f xy=%d,%d",mGradientCenterX,mGradientCenterY,cx,cy,mGradientRadius,mGradientAngle,x,y);
           mPaint=RadialGradient::create(cx,cy,0,cx,cy,mGradientRadius);
           applyGradients();
        }break;
    case Gradient::SWEEP:{
           std::vector<Cairo::ColorStop>stops;
           for(int i=0;i<mGradientColors.size();i++){
               Color c(mGradientColors[i]);
               stops.push_back({double(i)/mGradientColors.size(),c.red(),c.green(),c.blue(),c.alpha()});
           }
           mPaint=SweepGradient::create(mGradientCenterX*mWidth,mGradientCenterY*mHeight,mGradientRadius,M_PI*2.f,stops);
           LOGV("SWEEP(%.2f,%.2f:%.2f",mGradientCenterX*mWidth,mGradientCenterY*mHeight,mGradientRadius);
           applyGradients();
        }break;
    default:  
        LOGE("unsupported gradient type %d",mGradientType);
        break;
    }
}

void Shape::fill_stroke(Canvas&canvas,int x,int y){
    const bool bstroke=(mStrokeWidth>0) && (mStrokeColor&0x80000000);
    if(mGradientColors.size()){
        rebuildPattern(x,y);
        if(mGradientType==Shape::Gradient::RADIAL)
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

RectShape::RectShape():Shape(){
}

RectShape::RectShape(const RectShape&o):Shape(o){
}

void RectShape::setOuterRadii(const std::vector<float>&v){
    mOuterRadii=v;
}
void RectShape::setInnerRadii(const std::vector<float>&v){
    mInnerRadii=v;
}

void RectShape::setRadius(float radius){
    mOuterRadii.resize(4);
    for(int i=0;i<4;i++)mOuterRadii[i]=radius; 
}

void RectShape::getOutline(Outline&outline){
    /*RectF
    outline.setRect((int) Math.ceil(rect.left), (int) Math.ceil(rect.top),
                (int) Math.floor(rect.right), (int) Math.floor(rect.bottom));*/
}

RectShape*RectShape::clone()const{
    return new RectShape(*this);
}

void RectShape::onResize(int width,int height){
}

void RectShape::draw(Canvas&canvas,int x,int y){
    rebuildPattern(x,y);
    canvas.rectangle(0,0,mWidth,mHeight);
    fill_stroke(canvas,x,y);
}

ArcShape::ArcShape(float startAngle,float sweepAngle):RectShape(){
    mStartAngle = startAngle;
    mSweepAngle = sweepAngle;
}

ArcShape::ArcShape(const ArcShape&o):RectShape(o){
    mStartAngle=o.mStartAngle;
    mSweepAngle=o.mSweepAngle;
}

ArcShape*ArcShape::clone()const{
    return new ArcShape(*this);
}

void ArcShape::draw(Canvas&canvas,int x,int y){
    canvas.save();
    canvas.scale(1.0 , (float)mHeight/mWidth);
    canvas.arc(mWidth/2 ,mHeight/2, mWidth/2, mStartAngle,mSweepAngle);
    canvas.scale(1.0 , (float)mWidth/mHeight);
    canvas.line_to(mWidth/2,mHeight/2);
    canvas.close_path();
    fill_stroke(canvas,x,y);
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

void OvalShape::draw(Canvas&canvas,int x,int y){
    // outer     
    canvas.translate(mWidth/2,mHeight/2);
    canvas.scale(1.,(float)mHeight/mWidth);
    canvas.arc(0,0,mWidth/2,0,M_PI*2);
    canvas.scale(1.,(float)mWidth/mHeight);
    canvas.translate(-mWidth/2,-mHeight/2);

    //inner
    float innerRadius=mInnerRadius;
    if(mInnerRadius==.0f)
       innerRadius=mWidth/2.f-mThickness;
    if(innerRadius>.0){
        float ratio=(float)mWidth/mHeight;
        if(mInnerRadiusRatio)ratio=mInnerRadiusRatio;
        canvas.translate(mWidth/2,mHeight/2);
        canvas.scale(ratio,1.f);
        canvas.arc(0,0,innerRadius,0,M_PI*2);
        canvas.scale(1.f/ratio,1.f); 
        canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
    }
    canvas.translate(-mWidth/2,-mHeight/2);
    fill_stroke(canvas,x,y);
}

RoundRectShape::RoundRectShape():RectShape(){
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

RoundRectShape*RoundRectShape::clone()const{
    return new RoundRectShape(*this);
}

void RoundRectShape::onResize(int w, int h){
    RectShape::onResize(w,h);
    Rect r={0,0,mWidth,mHeight};
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
        pts[0]=r.left+radii[0];      pts[1]=r.top +radii[0];
        pts[2]=r.right()-radii[1];   pts[3]=r.top+radii[1];
        pts[4]=r.right()-radii[2];   pts[5]=r.bottom()-radii[2];
        pts[6]=r.left+radii[3];      pts[7]=r.bottom()-radii[3];
        for(int i=0,j=0;i<8;i+=2,j++){
            canvas.translate(pts[i],pts[i+1]);
            canvas.arc(0,0,radii[j],db*degree,(db+90)*degree);
            canvas.translate(-pts[i],-pts[i+1]);
            db+=90.f;
        }canvas.line_to(pts[0]-radii[0],pts[1]);
    }
}

void RoundRectShape::draw(Canvas&canvas,int x,int y){
    Rect r={0,0,mWidth,mHeight};
    drawRound(canvas,r,mOuterRadii);
    canvas.set_fill_rule(Cairo::Context::FillRule::WINDING);
    if(mInnerRadii.size()){
        drawRound(canvas,r,mInnerRadii);
        canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
    }
    fill_stroke(canvas,x,y);
}

}
