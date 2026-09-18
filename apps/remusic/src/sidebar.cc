// Port of com.wm.remusic.widget.SideBar — see sidebar.h.
#include "sidebar.h"

#include <core/canvas.h>
#include <view/motionevent.h>
#include <view/gravity.h>
#include <widget/textview.h>

using namespace cdroid;

namespace remusic {

const std::vector<std::string>& SideBar::letters() {
    static const std::vector<std::string> b = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
        "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "#"};
    return b;
}

SideBar::SideBar(Context* ctx) : View(ctx) {}

void SideBar::onDraw(Canvas& canvas) {
    const auto& b = letters();
    const int single = getHeight() / (int) b.size();
    canvas.set_font_size(std::max(9, single * 2 / 3));
    for (int i = 0; i < (int) b.size(); i++) {
        // Unselected letters stay readable on any page background.
        canvas.set_color(i == mChoose ? 0xFF3399FF : 0xFFFFFFFF);
        canvas.draw_text(Rect::Make(0, single * i, getWidth(), single), b[i],
                Gravity::CENTER);
    }
}

bool SideBar::onTouchEvent(MotionEvent& event) {
    const auto& b = letters();
    const int c = (int) (event.getY() / getHeight() * b.size());
    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_UP:
        mChoose = -1;
        if (mTextDialog != nullptr) mTextDialog->setVisibility(View::INVISIBLE);
        invalidate();
        return true;
    default:
        if (c >= 0 && c < (int) b.size() && c != mChoose) {
            mChoose = c;
            invalidate();
            if (mTextDialog != nullptr) {
                mTextDialog->setText(b[c]);
                mTextDialog->setVisibility(View::VISIBLE);
            }
            if (mOnTouch) mOnTouch(b[c]);
        }
        return true;
    }
}

void SideBar::setSelected(const std::string& letter) {
    const auto& b = letters();
    mChoose = -1;
    for (int i = 0; i < (int) b.size(); i++)
        if (b[i] == letter) { mChoose = i; break; }
    invalidate();
}

} // namespace remusic
