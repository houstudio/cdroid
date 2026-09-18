#ifndef __DESKCLOCK_AUTOSIZINGVIEWS_H__
#define __DESKCLOCK_AUTOSIZINGVIEWS_H__
/*********************************************************************************
 * Ports of com.android.deskclock.widget.AutoSizingTextClock and
 * AutoSizingTextView — clock/text views whose text size shrinks to fit via
 * TextSizeHelper. (The setText/onTextChanged suppression dance upstream guards
 * the same requestLayout storm the helper's flag already gates here.)
 *********************************************************************************/
#include <memory>

#include <widget/textclock.h>
#include <widget/textview.h>

#include <textsizehelper.h>

namespace cdroid {
namespace deskclock {

class AutoSizingTextClock : public TextClock {
private:
    std::unique_ptr<TextSizeHelper> mTextSizeHelper;

public:
    AutoSizingTextClock(Context* context, const AttributeSet* attrs);

    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void requestLayout() override;
};

class AutoSizingTextView : public TextView {
private:
    std::unique_ptr<TextSizeHelper> mTextSizeHelper;

public:
    AutoSizingTextView(Context* context, const AttributeSet* attrs);

    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void requestLayout() override;
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_AUTOSIZINGVIEWS_H__
