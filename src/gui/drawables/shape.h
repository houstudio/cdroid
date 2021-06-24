#ifndef __SHAPE_H__
#define __SHAPE_H__
#include <rect.h>
#include <canvas.h>
namespace cdroid{

class Shape{
protected:
    int mWidth,mHeight;
    int mStrokeColor;
    float mStrokeWidth;
    int mDashWidth;
    int mDashGap;
    int mGradientType;//0-solid,1--linear,2--radial 3--sweep
    float mRadius;
    float mAngle;
    float mCenterX,mCenterY;
    bool mRebuildGradient;
    bool bUseLevel;
    std::vector<int>mGradientColors;//size 0:nofill, 1:solid fill 2,3:gradient fill
    RefPtr<Pattern>mPaint;//used to fill
    void rebuildPattern(const RECT&r);
    virtual void onResize(int width,int height){}
    void fill_stroke(Canvas&canvas);
    virtual const RECT&rect()const=0;
    Shape(const Shape&o);
public:
    Shape();
    int getWidth()const{return mWidth;}
    int getHeight()const{return mHeight;}
    void resize(int width, int height);
    void setUseLevel(bool);
    void setStrokeColor(int color);
    void setStrokeSize(float width);
    void setStrokeDash(int width,int gap);
    int getStroke(int &color)const;//return line width,color
    void setSolidColor(int color);
    void setGradientColors(const std::vector<int>&cls);//gradient fill colors
    void setGradientAngle(float angle);
    void setGradientRadius(float radius);
    void setGradientType(int gt);
    void setGradientCenterX(float x);
    void setGradientCenterY(float y);
    virtual void draw(Canvas&canvas)=0;
    virtual Shape*clone()const=0;
};

class RectShape:public Shape{
private:
    RECT mRect;
protected:
    RectShape(const RectShape&o);
    const RECT& rect()const override{return mRect;}
    void onResize(int width,int height)override;
public:
    RectShape();
    void draw(Canvas&canvas)override;
    Shape*clone()const override;
};

class ArcShape:public RectShape{
private:
    float mStartAngle;
    float mSweepAngle;
    ArcShape(const ArcShape&o);
public:
    ArcShape(float startAngle,float sweepAngle);
    float getStartAngle()const{ return mStartAngle; }
    float getSweepAngle()const{ return mSweepAngle; }
    Shape*clone()const override;
    void draw(Canvas&canvas)override;
};

class OvalShape:public RectShape{
public:
    void draw(Canvas&canvas)override;
};

class RoundRectShape:public RectShape{
private:
    std::vector<int>mOuterRadii;
    std::vector<int>mInnerRadii;
    RECT mInset;
    RECT mInnerRect;
protected:
    RoundRectShape(const RoundRectShape&o);
    void onResize(int w, int h)override;
public:
    RoundRectShape();
    RoundRectShape(const std::vector<int>&outRadii,const RECT&inset,const std::vector<int>&innerRadii);
    void setRadius(int radius);
    void draw(Canvas&canvas)override;
    Shape*clone()const override;
};
}
#endif
