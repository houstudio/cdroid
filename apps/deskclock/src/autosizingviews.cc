#include <autosizingviews.h>


namespace cdroid {
namespace deskclock {

AutoSizingTextClock::AutoSizingTextClock(Context* context, const AttributeSet* attrs)
    : TextClock(context, attrs) {
    mTextSizeHelper.reset(new TextSizeHelper(*this));
}

void AutoSizingTextClock::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    mTextSizeHelper->onMeasure(widthMeasureSpec, heightMeasureSpec);
    TextClock::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

void AutoSizingTextClock::requestLayout() {
    if (!mTextSizeHelper->shouldIgnoreRequestLayout()) {
        TextClock::requestLayout();
    }
}

AutoSizingTextView::AutoSizingTextView(Context* context, const AttributeSet* attrs)
    : TextView(context, attrs) {
    mTextSizeHelper.reset(new TextSizeHelper(*this));
}

void AutoSizingTextView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    mTextSizeHelper->onMeasure(widthMeasureSpec, heightMeasureSpec);
    TextView::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

void AutoSizingTextView::requestLayout() {
    if (!mTextSizeHelper->shouldIgnoreRequestLayout()) {
        TextView::requestLayout();
    }
}

} // namespace deskclock

typedef cdroid::deskclock::AutoSizingTextClock AutoSizingTextClock;
DECLARE_WIDGET2(AutoSizingTextClock, "AutoSizingTextClock");
typedef cdroid::deskclock::AutoSizingTextView AutoSizingTextView;
DECLARE_WIDGET2(AutoSizingTextView, "AutoSizingTextView");

} // namespace cdroid
