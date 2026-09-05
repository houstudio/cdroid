// Port of com.wm.remusic.widget.SideBar — the A..# index strip dragged to
// jump the list to a section; the bubble TextView (setView) shows the current
// letter mid-drag, exactly like the original. Deviation: always visible (the
// original hid itself 2s after the last draw and re-showed on list drag).
#ifndef __REMUSIC_SIDEBAR_H__
#define __REMUSIC_SIDEBAR_H__

#include <functional>
#include <string>
#include <vector>

#include <view/view.h>

namespace cdroid { class TextView; }

namespace remusic {

class SideBar : public cdroid::View {
public:
    using OnTouchingLetterChanged = std::function<void(const std::string&)>;

    explicit SideBar(cdroid::Context* ctx);

    // The mid-screen letter bubble the sidebar shows/hides itself (setView).
    void setView(cdroid::TextView* textDialog) { mTextDialog = textDialog; }
    void setOnTouchingLetterChangedListener(OnTouchingLetterChanged l) {
        mOnTouch = std::move(l);
    }
    // Highlight the current section while the list scrolls (setSelected).
    void setSelected(const std::string& letter);

    void onDraw(cdroid::Canvas& canvas) override;
    bool onTouchEvent(cdroid::MotionEvent& event) override;

private:
    static const std::vector<std::string>& letters();

    OnTouchingLetterChanged mOnTouch;
    cdroid::TextView* mTextDialog = nullptr;
    int mChoose = -1;
};

} // namespace remusic
#endif
