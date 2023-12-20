#ifndef __SHAPE_DRAWABLE_H__
#define __SHAPE_DRAWABLE_H__
#include <drawables/drawable.h>
#include <drawables/shape.h>

namespace cdroid{

class ShapeDrawable:public Drawable{
private:
    class ShapeState:public std::enable_shared_from_this<ShapeState>,public ConstantState{
    public:
        Shape*mShape;
        const ColorStateList* mTint;
        int mTintMode;
        Rect mPadding;
        int mChangingConfigurations;
        int mIntrinsicWidth;
        int mIntrinsicHeight;
        int mAlpha;
        ShapeState();
        ShapeState(const ShapeState&orig);
        ~ShapeState();
        ShapeDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    bool mMutated;
    PorterDuffColorFilter*mTintFilter;
    std::shared_ptr<ShapeState>mShapeState;
    void updateShape();
    ShapeDrawable(std::shared_ptr<ShapeState>state);
    void updateLocalState();
protected:
    void onBoundsChange(const Rect&bounds)override;
    bool onStateChange(const std::vector<int>&stateset)override;
public:
    ShapeDrawable();
    ~ShapeDrawable();
    void setShape(Shape*shape);
    Shape*getShape()const;
    bool getPadding(Rect&rect)override;
    void setPadding(const Rect& padding);
    void setPadding(int left, int top, int right, int bottom);
    void setAlpha(int alpha)override;
    int getAlpha()const override;
    int getOpacity()override;
    void setTintList(const ColorStateList*)override;
    void setTintMode(int tintMode)override;
    void setColorFilter(ColorFilter*colorFilter)override;
    int getIntrinsicWidth()const;
    int getIntrinsicHeight()const;
    void setIntrinsicWidth(int width);
    void setIntrinsicHeight(int height);
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    std::shared_ptr<ConstantState>getConstantState()override;
    ShapeDrawable*mutate()override;
    void clearMutated()override;
    void draw(Canvas&canvas)override;
    static Drawable*inflate(Context*ctx,const AttributeSet&atts);
};

}
#endif
