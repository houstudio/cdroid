#ifndef __SHAPE_H__
#define __SHAPE_H__
#include <core/canvas.h>
#include <core/path.h>
#include <core/outline.h>
namespace cdroid{

class Shape{
protected:
    int mWidth,mHeight;
    int mStrokeColor;
    float mStrokeWidth;
    int mDashWidth;
    int mDashGap;
    int mGradientType;//0-solid,1--linear,2--radial 3--sweep
    float mGradientRadius;
    float mGradientAngle;
    float mGradientCenterX;
    float mGradientCenterY;
    bool  bUseLevel;
    std::vector<uint32_t>mGradientColors;//size 0:nofill, 1:solid fill 2,3:gradient fill
    Cairo::RefPtr<Cairo::Pattern>mPaint;//used to fill
    void rebuildPattern(int x,int y);
    virtual void onResize(int width,int height){}
    void fill_stroke(Canvas&canvas,int x,int y);
    Shape(const Shape&o);
    void applyGradients();
public:
    enum Gradient{
        SOLID =0,
        LINEAR=1,
        RADIAL=2,
        SWEEP =3
    };
    Shape();
    int getWidth()const;
    int getHeight()const;
    void resize(int width, int height);
    void setUseLevel(bool);
    void setStrokeColor(int color);
    void setStrokeSize(float width);
    void setStrokeDash(int width,int gap);
    int  getStroke(int &color)const;//return line width,color
    void setSolidColor(int color);
    void setGradientColors(const std::vector<uint32_t>&cls);//gradient fill colors
    void setGradientAngle(float angle);
    void setGradientRadius(float radius);
    void setGradientType(int gt);
    void setGradientCenterX(float x);
    void setGradientCenterY(float y);
    virtual void draw(Canvas&canvas,int x,int y)=0;
    virtual void getOutline(Outline&outline){}
    virtual Shape*clone()const=0;
};

class RectShape:public Shape{
protected:
    std::vector<float>mOuterRadii;
    std::vector<float>mInnerRadii;
    Rect mInset;
    Rect mInnerRect;
    Cairo::RefPtr<cdroid::Path>mPath;
protected:
    RectShape(const RectShape&o);
    void onResize(int width,int height)override;
public:
    RectShape();
    void setOuterRadii(const std::vector<float>&);
    void setInnerRadii(const std::vector<float>&);
    void setRadius(float radius);
    void draw(Canvas&canvas,int x=0,int y=0)override;
    void getOutline(Outline&)override;
    RectShape*clone()const override;
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
    ArcShape*clone()const override;
    void draw(Canvas&canvas,int x=0,int y=0)override;
};

class OvalShape:public RectShape{
private:
    float mInnerRadius;
    float mInnerRadiusRatio;
    float mThickness;
    float mThicknessRatio;
public:
    OvalShape();
    void setInnerRadius(float);
    float getInnerRadius()const;
    void setInnerRadiusRatio(float);
    float getInnerRadiusRatio()const;
    void setThickness(float);
    float getThickness()const;
    void setThicknessRatio(float);
    float getThicknessRatio()const;
    void draw(Canvas&canvas,int x=0,int y=0)override;
};

class RoundRectShape:public RectShape{
private:
    void drawRound(Canvas&canvas,const Rect&r,const std::vector<float>&radii);
protected:
    RoundRectShape(const RoundRectShape&o);
    void onResize(int w, int h)override;
public:
    RoundRectShape();
    RoundRectShape(const std::vector<float>&outRadii,const Rect&inset,const std::vector<float>&innerRadii);
    void draw(Canvas&canvas,int x=0,int y=0)override;
    RoundRectShape*clone()const override;
};

class PathShape:public Shape{
private:
    float mStdWidth;
    float mStdHeight;
    float mScaleX;
    float mScaleY;
    Path*mPath;
public:
    PathShape(Path*,float stdWidth,float stdHeight);
    void onResize(int w, int h)override;
    void draw(Canvas&canvas,int x=0,int y=0)override;
    Shape*clone()const override;
};
}
#endif
