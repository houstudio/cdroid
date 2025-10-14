#ifndef __COLORSTATELIST_DRAWABLE_H__
#define __COLORSTATELIST_DRAWABLE_H__
#include <drawable/drawable.h>
namespace cdroid{
class ColorStateListDrawable :public Drawable ,public Drawable::Callback {
private:
    class ColorStateListDrawableState;
    ColorDrawable* mColorDrawable;
    std::shared_ptr<ColorStateListDrawableState> mState;
    bool mMutated = false;
private:
    ColorStateListDrawable(@NonNull ColorStateListDrawableState state);
    void initializeColorDrawable();
protected:
    void onBoundsChange(const Rect& bounds) override;
    bool onStateChange(const std::vector<int>&state) override;
public:
    ColorStateListDrawable();
    ColorStateListDrawable(ColorStateList* colorStateList);

    void draw(Canvas& canvas)override;

    int getAlpha() override;

    bool isStateful() override;
    bool hasFocusStateSpecified() override;

    Drawable* getCurrent() override;

    void applyTheme(@NonNull Resources.Theme t) override;
    bool canApplyTheme() override;

    void setAlpha(int alpha) override;
    void clearAlpha();

    void setTintList(ColorStateList* tint) override;
    void setTintBlendMode(@NonNull BlendMode blendMode) override;
    ColorFilter* getColorFilter() override;
    void setColorFilter(ColorFilter colorFilter) override;

    int getOpacity() override;

    void invalidateDrawable(Drawable& who) override;
    void scheduleDrawable(Drawable& who, const Runnable& what, int64_t when) override;
    void unscheduleDrawable(Drawable& who, const Runnable& what) override;

    std::shared_ptr<ConstantState> getConstantState() override;

    ColorStateList* getColorStateList();
    int getChangingConfigurations() override;
    Drawable* mutate() override;
    void clearMutated() override;

    /**
     * Replace this Drawable's ColorStateList. It is not copied, so changes will propagate on the
     * next call to {@link #setState(int[])}.
     *
     * @param colorStateList A color state list to attach.
     */
    void setColorStateList(ColorStateList* colorStateList);
};

static class ColorStateListDrawable::ColorStateListDrawableState :public ConstantState {
    ColorStateList mColor = null;
    ColorStateList mTint = null;
    int mAlpha = -1;
    BlendMode mBlendMode = DEFAULT_BLEND_MODE;
    int mChangingConfigurations = 0;
public:
    ColorStateListDrawableState();
    ColorStateListDrawableState(ColorStateListDrawableState state);

    Drawable newDrawable() override;

    int getChangingConfigurations() override;

    bool isStateful() override;
    bool hasFocusStateSpecified() override;
    bool canApplyTheme() override;
};
}/*endof namespace*/
#endif/*__COLORSTATELIST_DRAWABLE_H__*/
