#ifndef __COLLAPSING_TOOLBAR_LAYOUT_H__
#define __COLLAPSING_TOOLBAR_LAYOUT_H__
#include <widget/framelayout.h>
#include <widgetEx/appbar/collapsingtexthelper.h>

namespace cdroid {
class Drawable;
class AppBarLayout;

class CollapsingToolbarLayout : public FrameLayout {
public:
    static constexpr int TITLE_COLLAPSE_MODE_SCALE = 0;
    static constexpr int TITLE_COLLAPSE_MODE_FADE = 1;

    class LayoutParams : public FrameLayout::LayoutParams {
    public:
        static constexpr int COLLAPSE_MODE_OFF = 0;
        static constexpr int COLLAPSE_MODE_PIN = 1;
        static constexpr int COLLAPSE_MODE_PARALLAX = 2;
        int collapseMode = COLLAPSE_MODE_OFF;
        float parallaxMultiplier = 0.5f;
        LayoutParams(Context* context, const AttributeSet* attrs);
        LayoutParams(int width, int height);
        LayoutParams(const ViewGroup::LayoutParams& source);
    };
private:
    CollapsingTextHelper mCollapsingTextHelper;
    std::u16string mTitle;
    Drawable* mContentScrim = nullptr;
    Drawable* mStatusBarScrim = nullptr;
    View* mToolbar = nullptr;
    int mToolbarId = View::NO_ID;
    int mCurrentOffset = 0;
    int mExpandedMarginStart = 0;
    int mExpandedMarginTop = 0;
    int mExpandedMarginEnd = 0;
    int mExpandedMarginBottom = 0;
    int mTitleCollapseMode = TITLE_COLLAPSE_MODE_SCALE;
    bool mTitleEnabled = true;
    bool mScrimsShown = false;
    bool mRefreshToolbar = true;
    int mScrimAlpha = 0;
    void updateToolbar();
    void updateTitleBounds();
    void updateScrimVisibility();
protected:
    void onAttachedToWindow() override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int left, int top, int right, int bottom) override;
    void onDraw(Canvas& canvas) override;
    LayoutParams* generateDefaultLayoutParams() const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* params) const override;
    LayoutParams* generateLayoutParams(const AttributeSet& attrs) const override;
public:
    CollapsingToolbarLayout(Context* context, const AttributeSet& attrs);
    CollapsingToolbarLayout(Context* context, const AttributeSet* attrs, int defStyleAttr = 0);
    ~CollapsingToolbarLayout() override;

    void setTitle(const std::u16string& title);
    const std::u16string& getTitle() const;
    void setTitleCollapseMode(int mode);
    int getTitleCollapseMode() const;
    void setContentScrim(Drawable* scrim);
    Drawable* getContentScrim() const;
    void setStatusBarScrim(Drawable* scrim);
    Drawable* getStatusBarScrim() const;
    void setScrimsShown(bool shown);
    bool isScrimsShown() const;
    void setExpandedTitleMargin(int start, int top, int end, int bottom);
    void setToolbarId(int id);
    int getToolbarId() const;
    void onOffsetChanged(AppBarLayout& appBarLayout, int verticalOffset);
};
}
#endif
