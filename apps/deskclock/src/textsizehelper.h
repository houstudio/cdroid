#ifndef __DESKCLOCK_TEXTSIZEHELPER_H__
#define __DESKCLOCK_TEXTSIZEHELPER_H__
/*********************************************************************************
 * Port of com.android.deskclock.widget.TextSizeHelper — binary-search the
 * largest text size that fits the TextView's measure constraints.
 *********************************************************************************/
#include <climits>

#include <text/layout.h>
#include <widget/textview.h>

namespace cdroid {
namespace deskclock {

class TextSizeHelper {
private:
    TextView& mTextView;
    TextPaint mMeasurePaint;

    const float mMaxTextSize;

    int mWidthConstraint = INT_MAX;
    int mHeightConstraint = INT_MAX;

    bool mIgnoreRequestLayout = false;

public:
    explicit TextSizeHelper(TextView& textView)
        : mTextView(textView), mMaxTextSize(textView.getTextSize()) {}

    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int widthConstraint = INT_MAX;
        if (View::MeasureSpec::getMode(widthMeasureSpec) != View::MeasureSpec::UNSPECIFIED) {
            widthConstraint = View::MeasureSpec::getSize(widthMeasureSpec)
                    - mTextView.getCompoundPaddingLeft() - mTextView.getCompoundPaddingRight();
        }

        int heightConstraint = INT_MAX;
        if (View::MeasureSpec::getMode(heightMeasureSpec) != View::MeasureSpec::UNSPECIFIED) {
            heightConstraint = View::MeasureSpec::getSize(heightMeasureSpec)
                    - mTextView.getCompoundPaddingTop() - mTextView.getCompoundPaddingBottom();
        }

        if (mTextView.isLayoutRequested()
                || mWidthConstraint != widthConstraint
                || mHeightConstraint != heightConstraint) {
            mWidthConstraint = widthConstraint;
            mHeightConstraint = heightConstraint;
            adjustTextSize();
        }
    }

    bool shouldIgnoreRequestLayout() const { return mIgnoreRequestLayout; }

private:
    void adjustTextSize() {
        // TextView::getText() returns a reference into the view; borrowed here.
        CharSequence& textRef = mTextView.getText();
        const bool hasText = textRef.length() > 0;
        float textSize = mMaxTextSize;
        if (hasText
                && (mWidthConstraint < INT_MAX || mHeightConstraint < INT_MAX)) {
            mMeasurePaint.set(mTextView.getPaint());

            float minTextSize = 1.0f;
            float maxTextSize = mMaxTextSize;
            while (maxTextSize >= minTextSize) {
                const float midTextSize = (float) (int) ((maxTextSize + minTextSize) / 2.0f);
                mMeasurePaint.setTextSize(midTextSize);

                const float width = Layout::getDesiredWidth(&textRef, mMeasurePaint);
                const float height = (float) mMeasurePaint.getFontMetricsInt(nullptr);
                if (width > mWidthConstraint || height > mHeightConstraint) {
                    maxTextSize = midTextSize - 1.0f;
                } else {
                    textSize = midTextSize;
                    minTextSize = midTextSize + 1.0f;
                }
            }
        }

        if (mTextView.getTextSize() != textSize) {
            mIgnoreRequestLayout = true;
            mTextView.setTextSize(TypedValue::COMPLEX_UNIT_PX, textSize);
            mIgnoreRequestLayout = false;
        }
    }
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TEXTSIZEHELPER_H__
