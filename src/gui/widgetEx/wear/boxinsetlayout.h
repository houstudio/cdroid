#ifndef __WEAR_BOXINSET_LAYOUT_H__
#define __WEAR_BOXINSET_LAYOUT_H__
#include <widget/framelayout.h>
namespace cdroid{
class BoxInsetLayout:public ViewGroup {
public:
    class LayoutParams:public FrameLayout::LayoutParams {
    public:
        static constexpr int BOX_NONE = 0x0;
        static constexpr int BOX_LEFT = 0x01;
        static constexpr int BOX_TOP = 0x02;
        static constexpr int BOX_RIGHT = 0x04;
        static constexpr int BOX_BOTTOM = 0x08;
        static constexpr int BOX_ALL = 0x0F;

        int boxedEdges = BOX_NONE;
    public:
        LayoutParams(Context* context,const AttributeSet& attrs);

        LayoutParams(int width, int height);
        LayoutParams(int width, int height, int gravity);
        LayoutParams(int width, int height, int gravity, int boxed);

        LayoutParams(const ViewGroup::LayoutParams& source);
        LayoutParams(const ViewGroup::MarginLayoutParams& source);

        LayoutParams(const FrameLayout::LayoutParams& source);
        LayoutParams(const LayoutParams& source);
    };
private:
    static constexpr float FACTOR = 0.146447f; //(1 - sqrt(2)/2)/2
    static constexpr int DEFAULT_CHILD_GRAVITY = Gravity::TOP | Gravity::START;

    int mScreenHeight;
    int mScreenWidth;

    bool mIsRound;
    Rect mForegroundPadding;
    Rect mInsets;
    Drawable* mForegroundDrawable;
private:
    void measureChild(int widthMeasureSpec, int heightMeasureSpec, int desiredMinInset,int i);

    int calculateChildLeftMargin(const LayoutParams* lp, int horizontalGravity, int desiredMinInset)const;
    int calculateChildRightMargin(const LayoutParams* lp, int horizontalGravity, int desiredMinInset)const;

    int calculateChildTopMargin(const LayoutParams* lp, int verticalGravity, int desiredMinInset)const;
    int calculateChildBottomMargin(const LayoutParams* lp, int verticalGravity, int desiredMinInset)const;

    int calculateInset(int measuredWidth, int measuredHeight);
protected:
    void onAttachedToWindow() override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int left, int top, int right, int bottom) override;

    bool checkLayoutParams(const ViewGroup::LayoutParams* p) const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p) const override;
public:
    /**
     * Perform inflation from XML and apply a class-specific base style from a theme attribute.
     * This constructor allows subclasses to use their own base style when they are inflating.
     *
     * @param context  The {@link Context} the view is running in, through which it can
     *                 access the current theme, resources, etc.
     * @param attrs    The attributes of the XML tag that is inflating the view.
     * @param defStyle An attribute in the current theme that contains a reference to a style
     *                 resource that supplies default values for the view. Can be 0 to not look for
     *                 defaults.
     */
    BoxInsetLayout(Context* context,const AttributeSet& attrs);

    void setForeground(Drawable* drawable) override;

    LayoutParams* generateLayoutParams(const AttributeSet& attrs) const override;
};
}/*endof namespace*/
#endif/*__WEAR_BOXINSET_LAYOUT_H__*/
