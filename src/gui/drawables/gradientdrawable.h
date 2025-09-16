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
#ifndef __GRADIENT_DRAWABLE_H__
#define __GRADIENT_DRAWABLE_H__
#include <drawables/drawable.h>
//#include <core/path.h>
namespace cdroid{

class GradientDrawable:public Drawable{
public:
    enum Orientation {
        /** draw the gradient from the top to the bottom */
        TOP_BOTTOM,
        /** draw the gradient from the top-right to the bottom-left */
        TR_BL,
        /** draw the gradient from the right to the left */
        RIGHT_LEFT,
        /** draw the gradient from the bottom-right to the top-left */
        BR_TL,
        /** draw the gradient from the bottom to the top */
        BOTTOM_TOP,
        /** draw the gradient from the bottom-left to the top-right */
        BL_TR,
        /** draw the gradient from the left to the right */
        LEFT_RIGHT,
        /** draw the gradient from the top-left to the bottom-right */
        TL_BR,
    };
    static constexpr bool sWrapNegativeAngleMeasurements = true;
    /*Shape is a rectangle, possibly with rounded corners*/
    static constexpr int RECTANGLE = 0;
    static constexpr int OVAL      = 1;
    static constexpr int LINE      = 2;
    static constexpr int RING      = 3;
    
    /* Gradient is linear (default.)*/
    static constexpr int LINEAR_GRADIENT= 0;
    static constexpr int RADIAL_GRADIENT= 1;
    static constexpr int SWEEP_GRADIENT = 2;
    static constexpr int BITMAP_PATTERN = 3;
private:
    static constexpr int RADIUS_TYPE_PIXELS  =0;
    static constexpr int RADIUS_TYPE_FRACTION=1;
    static constexpr int RADIUS_TYPE_FRACTION_PARENT = 2;
    static constexpr Orientation DEFAULT_ORIENTATION = Orientation::TOP_BOTTOM;
    static constexpr float DEFAULT_INNER_RADIUS_RATIO = 3.0f;
    static constexpr float DEFAULT_THICKNESS_RATIO = 9.0f;
private:
    class GradientState:public std::enable_shared_from_this<GradientState>,public ConstantState{
    public:
       int mChangingConfigurations;
       int mShape;
       int mGradient;// = LINEAR_GRADIENT
       int mAngle;
       int mSolidColor;
       int mStrokeColor;
       Orientation mOrientation;
       const ColorStateList* mSolidColors;
       const ColorStateList* mStrokeColors;
       std::vector<int>mGradientColors;
       std::vector<float>mPositions;
       Cairo::RefPtr<Cairo::ImageSurface>mImagePattern;
       int mStrokeWidth;
       float mStrokeDashWidth = 0.0f;
       float mStrokeDashGap = 0.0f;
       float mRadius = 0.0f;
       std::vector<float>mRadiusArray;
       Rect mPadding;
       int mWidth=-1;
       int mHeight=-1;
       float mInnerRadiusRatio;
       float mThicknessRatio;
       int mInnerRadius=-1;
       int mThickness;
       Insets mOpticalInsets;
       float mCenterX,mCenterY;
       float mGradientRadius = 0.5f;
       int mGradientRadiusType;
       int mGradientType;
       bool mDither = false;
       bool mUseLevel;
       bool mUseLevelForShape;
       bool mOpaqueOverBounds;
       bool mOpaqueOverShape;
       const ColorStateList*mTint;
       int mTintMode;
       int mDensity;
       std::vector<int>mAttrSize;
       std::vector<int>mAttrGradient;
       std::vector<int>mAttrSolid;
       std::vector<int>mAttrStroke;
       std::vector<int>mAttrCorners;
       std::vector<int>mAttrPadding;

       GradientState();
       GradientState(Orientation orientation, const std::vector<int>&gradientColors);
       GradientState(const GradientState& orig);
       ~GradientState();
       void setDensity(int targetDensity);
       bool hasCenterColor()const;
       void applyDensityScaling(int sourceDensity, int targetDensity);
       GradientDrawable* newDrawable()override;
       int getChangingConfigurations()const override;
       void setShape(int shape);
       void setGradientType(int gradient);
       void setGradientCenter(float x, float y);
       void setGradientColors(const std::vector<int>&colors);
       void setSolidColors(const ColorStateList* colors);
       void setStroke(int width,int color,float dashWidth,float dashGap);
       void setStroke(int width,const ColorStateList* colors,float dashWidth,float dashGap);
       void setCornerRadius(float radius);
       void setCornerRadii(const std::vector<float>& radii);
       void setSize(int width, int height);
       void setGradientRadius(float gradientRadius,int type);
       void setImagePattern(Cairo::RefPtr<Cairo::ImageSurface>);
       void computeOpacity();
    };
    int mAlpha;
    float mGradientRadius;
    bool mPathIsDirty;
    bool mGradientIsDirty;
    bool mMutated;
    Cairo::RefPtr<cdroid::Path>mPath;
    Cairo::RefPtr<cdroid::Path>mRingPath;
    RectF mRect;
    Rect mPadding;
    PorterDuffColorFilter*mTintFilter;
    std::shared_ptr<GradientState>mGradientState;
    std::vector<double>mDashArray;
    double mStrokeWidth;
    Cairo::RefPtr<Cairo::Pattern>mStrokePaint;
    Cairo::RefPtr<Cairo::Pattern>mFillPaint;
    bool ensureValidRect();
    void buildPathIfDirty();
    Cairo::RefPtr<cdroid::Path> buildRing(GradientState* st);
    bool isOpaqueForState()const;
    int modulateAlpha(int alpha);
    void setStrokeInternal(int width, int color, float dashWidth, float dashGap);
    GradientDrawable(std::shared_ptr<GradientState>state);
    void updateLocalState();
    void prepareStrokeProps(Canvas&canvas);
    void getPatternAlpha(int& strokeAlpha,int& fillApha);
    void updateStateFromTypedArray(const AttributeSet&atts);
    void inflateChildElements(XmlPullParser&,const AttributeSet&);
    void updateGradientDrawableSize(const AttributeSet&);
    void updateGradientDrawableGradient(const AttributeSet&);
    void updateGradientDrawableSolid(const AttributeSet&);
    void updateGradientDrawableStroke(const AttributeSet&);
    void updateDrawableCorners(const AttributeSet&);
    void updateGradientDrawablePadding(const AttributeSet&);
protected:
    void onBoundsChange(const Rect& r)override;
    bool onLevelChange(int level)override;
    bool onStateChange(const std::vector<int>& stateSet)override;
public:
    GradientDrawable();
    GradientDrawable(Orientation orientation,const std::vector<int>&colors);
    ~GradientDrawable()override;
    static bool isOpaque(int color){return ((color>>24)&0xFF)==0xFF;}
    bool getPadding(Rect& padding)override;
    void setCornerRadii(const std::vector<float>& radii);
    const std::vector<float>&getCornerRadii()const;
    void setCornerRadius(float radius);
    float getCornerRadius()const;
    void setStroke(int width,int color);
    void setStroke(int width, const ColorStateList* colorStateList);
    void setStroke(int width,int color, float dashWidth, float dashGap);
    void setStroke(int width, const ColorStateList* colorStateList, float dashWidth, float dashGap);
    void setInnerRadiusRatio(float innerRadiusRatio);
    float getInnerRadiusRatio()const;
    void setInnerRadius(int innerRadius);
    int  getInnerRadius()const;
    void setThicknessRatio(float thicknessRatio);
    float getThicknessRatio()const;
    void setThickness(int thickness);
    int  getThickness()const;
    void setPadding(int left,int top,int right,int bottom);
    void setShape(int shape);
    int getShape()const;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    Insets getOpticalInsets()override;
    void setSize(int width, int height);
    void setGradientType(int gradient);
    int getGradientType()const;
    void setGradientCenter(float x, float y);
    float getGradientCenterX()const;
    float getGradientCenterY()const;
    void setGradientRadius(float gradientRadius);
    float getGradientRadius();
    void setUseLevel(bool useLevel);
    bool getUseLevel()const;
    Orientation getOrientation()const;
    void setOrientation(Orientation orientation);
    void setColors(const std::vector<int>& colors);
    void setColors(const std::vector<int>&colors,const std::vector<float>&offsets);
    const std::vector<int>&getColors()const;
    void setColor(int argb);
    void setColor(const ColorStateList* colorStateList);
    const ColorStateList* getColor();
    void setImagePattern(Cairo::RefPtr<Cairo::ImageSurface>);
    void setImagePattern(Context*ctx,const std::string&);
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    int getChangingConfigurations()const override;
    void setAlpha(int a)override;
    int getAlpha()const override;
    void setDither(bool dither)override;
    void setColorFilter(ColorFilter*)override;
    ColorFilter*getColorFilter()override;
    void setTintList(const ColorStateList*tint)override;
    void setTintMode(int tintMode)override;
    int getOpacity()const;
    void getGradientCenter(float&x,float&y)const;
    std::shared_ptr<ConstantState>getConstantState()override;
    void getOutline(Outline&)override;
    GradientDrawable*mutate()override;
    void clearMutated()override;
    void draw(Canvas&canvas)override;
    void inflate(XmlPullParser&,const AttributeSet&)override;
};

}/*endof namespace*/
#endif/*__GRADIENT_DRAWABLE_H__*/
