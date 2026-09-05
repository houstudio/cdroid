#include <floatingactionbutton.h>

#include <R.h>

#include <drawable/gradientdrawable.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

namespace {
constexpr int SIZE_NORMAL_DP = 56; // design_fab_size_normal
}

FloatingActionButton::FloatingActionButton(Context* context, const AttributeSet* attrs)
    : ImageView(context, attrs) {
    auto a = context->obtainStyledAttributes(attrs, R::styleable::FloatingActionButton);
    mBorderWidth = (int) a->getDimension(R::styleable::FloatingActionButton_borderWidth, 0);

    // ?android:attr/colorAccent fallback lives in Theme.DeskClock's colorAccent.
    mBackgroundTint = 0xFFDA4336;
    {
        // Resolve the theme colorAccent directly (android attr id).
        TypedValue value;
        if (context->getTheme().resolveAttribute(0x01010435 /* android:colorAccent */, &value, true)) {
            mBackgroundTint = value.data;
        }
    }
    const int tintAttr = attrs->getAttributeResourceValue("http://schemas.android.com/apk/res/android",
                                                          "backgroundTint", 0);
    if (tintAttr != 0) {
        mBackgroundTint = context->getColor(tintAttr);
    }

    const float density = context->getDisplayMetrics().density;
    setMinimumWidth((int) (SIZE_NORMAL_DP * density));
    setMinimumHeight((int) (SIZE_NORMAL_DP * density));
    // Upstream draws the FAB icon at its intrinsic size, centered in the
    // circle (24dp add/globe/stop AND the 48dp play/pause morph icons share
    // one FAB). CENTER keeps intrinsic size; FIT_CENTER would crush the 48dp
    // icons into the content box, and any padding would shrink them further.
    setScaleType(ScaleType::CENTER);

    setBackgroundTintColor(mBackgroundTint);
}

void FloatingActionButton::setBackgroundTintColor(int color) {
    mBackgroundTint = color;

    GradientDrawable* circle = new GradientDrawable();
    circle->setShape(GradientDrawable::OVAL);
    circle->setColor(color);
    if (mBorderWidth > 0) {
        circle->setStroke(mBorderWidth, 0xFFFFFFFF);
    }
    setBackground(circle);
}

} // namespace deskclock

DECLARE_WIDGET3(cdroid::deskclock::FloatingActionButton, FloatingActionButton, 0);

} // namespace cdroid
