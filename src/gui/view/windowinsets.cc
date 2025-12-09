#include <view/view.h>
#include <view/windowinsets.h>

namespace cdroid{

//CONSUMED = new WindowInsets(null, null, null, false, false, null);

WindowInsets::WindowInsets(const Rect* systemWindowInsets,const Rect* windowDecorInsets,const Rect* stableInsets,
        bool isRound, bool alwaysConsumeNavBar,const DisplayCutout* displayCutout) {
    mSystemWindowInsetsConsumed = systemWindowInsets == nullptr;
    mSystemWindowInsets.setEmpty();
    mWindowDecorInsets.setEmpty();
    mStableInsets.setEmpty();
    if(mSystemWindowInsetsConsumed) mSystemWindowInsets = *systemWindowInsets;

    mWindowDecorInsetsConsumed = windowDecorInsets == nullptr;
    if(mWindowDecorInsetsConsumed) mWindowDecorInsets = *windowDecorInsets;

    mStableInsetsConsumed = stableInsets == nullptr;
    if(mStableInsetsConsumed) mStableInsets = *stableInsets;

    mIsRound = isRound;
    mAlwaysConsumeNavBar = alwaysConsumeNavBar;

    mDisplayCutoutConsumed = displayCutout == nullptr;
    mDisplayCutout = nullptr;//(mDisplayCutoutConsumed || displayCutout.isEmpty()) ? nullptr : displayCutout;
}

WindowInsets::WindowInsets(const WindowInsets& src) {
    mSystemWindowInsets = src.mSystemWindowInsets;
    mWindowDecorInsets = src.mWindowDecorInsets;
    mStableInsets = src.mStableInsets;
    mSystemWindowInsetsConsumed = src.mSystemWindowInsetsConsumed;
    mWindowDecorInsetsConsumed = src.mWindowDecorInsetsConsumed;
    mStableInsetsConsumed = src.mStableInsetsConsumed;
    mIsRound = src.mIsRound;
    mAlwaysConsumeNavBar = src.mAlwaysConsumeNavBar;
    mDisplayCutout = src.mDisplayCutout;
    mDisplayCutoutConsumed = src.mDisplayCutoutConsumed;
}

WindowInsets::WindowInsets(const Rect& systemWindowInsets)
  :WindowInsets(&systemWindowInsets, nullptr, nullptr, false, false, nullptr){
}

Rect WindowInsets::getSystemWindowInsets() const{
    return mSystemWindowInsets;
}

int WindowInsets::getSystemWindowInsetLeft() const{
    return mSystemWindowInsets.left;
}

int WindowInsets::getSystemWindowInsetTop() const{
    return mSystemWindowInsets.top;
}

int WindowInsets::getSystemWindowInsetRight() const{
    return mSystemWindowInsets.width;
}

int WindowInsets::getSystemWindowInsetBottom() const{
    return mSystemWindowInsets.height;
}

int WindowInsets::getWindowDecorInsetLeft() const{
    return mWindowDecorInsets.left;
}

int WindowInsets::getWindowDecorInsetTop() const{
    return mWindowDecorInsets.top;
}

int WindowInsets::getWindowDecorInsetRight() const{
    return mWindowDecorInsets.width;
}

int WindowInsets::getWindowDecorInsetBottom() const{
    return mWindowDecorInsets.height;
}

bool WindowInsets::hasSystemWindowInsets() const{
    return (mSystemWindowInsets.left != 0) || (mSystemWindowInsets.top != 0) ||
            (mSystemWindowInsets.width != 0) || (mSystemWindowInsets.height != 0);
}

bool WindowInsets::hasWindowDecorInsets() const{
    return (mWindowDecorInsets.left != 0) || (mWindowDecorInsets.top != 0) ||
            (mWindowDecorInsets.width != 0) || (mWindowDecorInsets.height != 0);
}

bool WindowInsets::hasInsets() const{
    return hasSystemWindowInsets() || hasWindowDecorInsets() || hasStableInsets()
            || (mDisplayCutout != nullptr);
}

DisplayCutout* WindowInsets::getDisplayCutout()const {
    return mDisplayCutout;
}

WindowInsets WindowInsets::consumeDisplayCutout() {
    WindowInsets result(*this);// = new WindowInsets(this);
    result.mDisplayCutout = nullptr;
    result.mDisplayCutoutConsumed = true;
    return result;
}

bool WindowInsets::isConsumed() const{
    return mSystemWindowInsetsConsumed && mWindowDecorInsetsConsumed 
           && mStableInsetsConsumed && mDisplayCutoutConsumed;
}

bool WindowInsets::isRound() const{
    return mIsRound;
}

WindowInsets WindowInsets::consumeSystemWindowInsets() {
    WindowInsets result(*this);
    result.mSystemWindowInsets.setEmpty();
    result.mSystemWindowInsetsConsumed = true;
    return result;
}

WindowInsets WindowInsets::consumeSystemWindowInsets(bool left, bool top,
        bool right, bool bottom) {
    if (left || top || right || bottom) {
        WindowInsets result(*this);// = new WindowInsets(this);
        result.mSystemWindowInsets.set(
                left ? 0 : mSystemWindowInsets.left,
                top ? 0 : mSystemWindowInsets.top,
                right ? 0 : mSystemWindowInsets.width,
                bottom ? 0 : mSystemWindowInsets.height);
        return result;
    }
    return *this;
}

WindowInsets WindowInsets::replaceSystemWindowInsets(int left, int top,
        int right, int bottom) {
    WindowInsets result(*this);// new WindowInsets(this);
    result.mSystemWindowInsets.set(left, top, right, bottom);
    return result;
}

WindowInsets WindowInsets::replaceSystemWindowInsets(const Rect& systemWindowInsets) {
    WindowInsets result(*this);// = new WindowInsets(this);
    result.mSystemWindowInsets=systemWindowInsets;
    return result;
}

WindowInsets WindowInsets::consumeWindowDecorInsets() {
    WindowInsets result(*this);
    result.mWindowDecorInsets.set(0, 0, 0, 0);
    result.mWindowDecorInsetsConsumed = true;
    return result;
}

WindowInsets WindowInsets::consumeWindowDecorInsets(bool left, bool top, bool right, bool bottom) {
    if (left || top || right || bottom) {
        WindowInsets result(*this);// = new WindowInsets(this);
        result.mWindowDecorInsets.set(left ? 0 : mWindowDecorInsets.left,
                top ? 0 : mWindowDecorInsets.top,
                right ? 0 : mWindowDecorInsets.width,
                bottom ? 0 : mWindowDecorInsets.height);
        return result;
    }
    return *this;
}

WindowInsets WindowInsets::replaceWindowDecorInsets(int left, int top, int right, int bottom) {
    WindowInsets result(*this);
    result.mWindowDecorInsets.set(left, top, right, bottom);
    return result;
}

int WindowInsets::getStableInsetTop() const{
    return mStableInsets.top;
}

int WindowInsets::getStableInsetLeft() const{
    return mStableInsets.left;
}

int WindowInsets::getStableInsetRight() const{
    return mStableInsets.width;
}

int WindowInsets::getStableInsetBottom() const{
    return mStableInsets.height;
}

bool WindowInsets::hasStableInsets() const{
    return (mStableInsets.top != 0) || (mStableInsets.left != 0) || (mStableInsets.width != 0)
            || (mStableInsets.height != 0);
}

WindowInsets WindowInsets::consumeStableInsets() {
    WindowInsets result(*this);// = new WindowInsets(this);
    result.mStableInsets.setEmpty();
    result.mStableInsetsConsumed = true;
    return result;
}

bool WindowInsets::shouldAlwaysConsumeNavBar() const{
    return mAlwaysConsumeNavBar;
}

WindowInsets WindowInsets::inset(const Rect& r) {
    return inset(r.left, r.top, r.width, r.height);
}

WindowInsets WindowInsets::inset(int left, int top, int right, int bottom) {
    /*Preconditions.checkArgumentNonnegative(left);
    Preconditions.checkArgumentNonnegative(top);
    Preconditions.checkArgumentNonnegative(right);
    Preconditions.checkArgumentNonnegative(bottom);*/

    WindowInsets result(*this);
    if (!result.mSystemWindowInsetsConsumed) {
        result.mSystemWindowInsets = insetInsets(result.mSystemWindowInsets, left, top, right, bottom);
    }
    if (!result.mWindowDecorInsetsConsumed) {
        result.mWindowDecorInsets = insetInsets(result.mWindowDecorInsets, left, top, right, bottom);
    }
    if (!result.mStableInsetsConsumed) {
        result.mStableInsets = insetInsets(result.mStableInsets, left, top, right, bottom);
    }
    /*if (mDisplayCutout != nullptr) {
        result.mDisplayCutout = result.mDisplayCutout.inset(left, top, right, bottom);
        if (result.mDisplayCutout.isEmpty()) {
            result.mDisplayCutout = nullptr;
        }
    }*/
    return result;
}

bool WindowInsets::operator==(const WindowInsets&that)const {
    if (this == &that) return true;
    return (mIsRound == that.mIsRound)
            && (mAlwaysConsumeNavBar == that.mAlwaysConsumeNavBar)
            && (mSystemWindowInsetsConsumed == that.mSystemWindowInsetsConsumed)
            && (mWindowDecorInsetsConsumed == that.mWindowDecorInsetsConsumed)
            && (mStableInsetsConsumed == that.mStableInsetsConsumed)
            && (mDisplayCutoutConsumed == that.mDisplayCutoutConsumed)
            && (mSystemWindowInsets==mSystemWindowInsets)
            && (mWindowDecorInsets==that.mWindowDecorInsets)
            && (mStableInsets==that.mStableInsets)
            && (mDisplayCutout==that.mDisplayCutout);
}

bool WindowInsets::operator!=(const WindowInsets& that)const {
    if (this == &that) return false;
    return (mIsRound != that.mIsRound)
        || (mAlwaysConsumeNavBar == that.mAlwaysConsumeNavBar)
        || (mSystemWindowInsetsConsumed == that.mSystemWindowInsetsConsumed)
        || (mWindowDecorInsetsConsumed == that.mWindowDecorInsetsConsumed)
        || ( mStableInsetsConsumed == that.mStableInsetsConsumed)
        || ( mDisplayCutoutConsumed == that.mDisplayCutoutConsumed)
        || (mSystemWindowInsets == mSystemWindowInsets)
        || (mWindowDecorInsets == that.mWindowDecorInsets)
        || (mStableInsets == that.mStableInsets)
        || (mDisplayCutout == that.mDisplayCutout);
}

Rect WindowInsets::insetInsets(const Rect& insets, int left, int top, int right, int bottom) {
    const int newLeft = std::max(0, insets.left - left);
    const int newTop = std::max(0, insets.top - top);
    const int newRight = std::max(0, insets.width - right);
    const int newBottom = std::max(0, insets.height - bottom);
    if ((newLeft == left) && (newTop == top) && (newRight == right) && (newBottom == bottom)) {
        return insets;
    }
    return Rect::Make(newLeft, newTop, newRight, newBottom);
}

bool WindowInsets::isSystemWindowInsetsConsumed() const{
    return mSystemWindowInsetsConsumed;
}

int WindowInsets::Type::indexOf(int type){
    switch (type) {
    case STATUS_BARS:
        return 0;
    case NAVIGATION_BARS:
        return 1;
    case CAPTION_BAR:
        return 2;
    case IME:
        return 3;
    case SYSTEM_GESTURES:
        return 4;
    case MANDATORY_SYSTEM_GESTURES:
        return 5;
    case TAPPABLE_ELEMENT:
        return 6;
    case DISPLAY_CUTOUT:
        return 7;
    case WINDOW_DECOR:
        return 8;
    case SYSTEM_OVERLAYS:
        return 9;
    default:
        throw std::invalid_argument("type needs to be >= FIRST and <= LAST");
    }
}
}/*endof namespace*/
