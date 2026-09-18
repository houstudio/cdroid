#ifndef __DESKCLOCK_FLOATINGACTIONBUTTON_H__
#define __DESKCLOCK_FLOATINGACTIONBUTTON_H__
/*********************************************************************************
 * Port of com.google.android.material.floatingactionbutton.FloatingActionButton
 * (subset used by DeskClock, app-local until a second consumer needs it in
 * widgetEx): a circular ImageView whose background is a tinted oval
 * GradientDrawable. android:backgroundTint (default ?android:attr/colorAccent)
 * supplies the tint; app:borderWidth adds a stroke ring. Elevation has no
 * rendering effect on cdroid and is accepted but unused (matches upstream's
 * visual role being decorative here).
 *********************************************************************************/
#include <widget/imageview.h>

namespace cdroid {
namespace deskclock {

class FloatingActionButton : public ImageView {
private:
    int mBackgroundTint;
    int mBorderWidth;
    float mRippleColor = 0;

public:
    FloatingActionButton(Context* context, const AttributeSet* attrs);

    /** Sets the background tint color; recreates the circle background. */
    void setBackgroundTintColor(int color);

    /** @return the current background tint color. */
    int getBackgroundTintColor() const { return mBackgroundTint; }
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_FLOATINGACTIONBUTTON_H__
