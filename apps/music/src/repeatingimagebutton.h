/*********************************************************************************
 * Port of com.android.music.RepeatingImageButton — a button that repeatedly
 * calls a listener for as long as it is held (prev/next seek in the playback
 * screen). Faithful logic; listeners are std::function instead of interfaces.
 *********************************************************************************/
#ifndef CDROID_MUSIC_REPEATINGIMAGEBUTTON_H
#define CDROID_MUSIC_REPEATINGIMAGEBUTTON_H

#include <functional>

#include <widget/imagebutton.h>

namespace cdroid {
namespace music {

class RepeatingImageButton : public ImageButton {
public:
    // void onRepeat(View& v, long durationMs, int repeatcount /* -1 = final */)
    using RepeatListener = std::function<void(View&, long, int)>;

    RepeatingImageButton(Context* ctx);
    RepeatingImageButton(Context* ctx, const AttributeSet* attrs);
    RepeatingImageButton(Context* ctx, const AttributeSet* attrs, int defStyleAttr);

    void setRepeatListener(const RepeatListener& l, long interval);

    bool performLongClick() override;
    bool onTouchEvent(MotionEvent& event) override;
    bool onKeyDown(int keyCode, KeyEvent& event) override;
    bool onKeyUp(int keyCode, KeyEvent& event) override;

private:
    void doRepeat(bool last);

    long mStartTime = 0;
    int mRepeatCount = 0;
    RepeatListener mListener;
    long mInterval = 500;
    Runnable mRepeater;
};

} // namespace music
} // namespace cdroid

#endif // CDROID_MUSIC_REPEATINGIMAGEBUTTON_H
