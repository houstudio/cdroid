#ifndef __DESKCLOCK_CIRCLEBUTTONSLAYOUT_H__
#define __DESKCLOCK_CIRCLEBUTTONSLAYOUT_H__
/*********************************************************************************
 * Port of com.android.deskclock.CircleButtonsLayout — aligns the child buttons
 * and label of the timer/stopwatch circle: left/right buttons at the bottom of
 * the circle, stop button and label inside, label max-width from circle math.
 *********************************************************************************/
#include <widget/framelayout.h>
#include <widget/button.h>
#include <widget/textview.h>

namespace cdroid {
namespace deskclock {

class CircleButtonsLayout : public FrameLayout {
private:
    float mDiamOffset;
    View* mCircleView = nullptr;
    Button* mResetAddButton = nullptr;
    TextView* mLabel = nullptr;

public:
    CircleButtonsLayout(Context* context, const AttributeSet* attrs);

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;

private:
    void remeasureViews();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CIRCLEBUTTONSLAYOUT_H__
