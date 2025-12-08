#ifndef __WINDOW_INSETS_H__
#define __WINDOW_INSETS_H__
#include <core/rect.h>
namespace cdroid{
class DisplayCutout;
class WindowInsets {
public:
    class Type;
    class Side;
private:
    Rect mSystemWindowInsets;
    Rect mWindowDecorInsets;
    Rect mStableInsets;
    bool mIsRound;
    DisplayCutout* mDisplayCutout;
    bool mAlwaysConsumeNavBar;
    bool mSystemWindowInsetsConsumed = false;
    bool mWindowDecorInsetsConsumed = false;
    bool mStableInsetsConsumed = false;
    bool mDisplayCutoutConsumed = false;
private:
    static Rect insetInsets(const Rect& insets, int left, int top, int right, int bottom);
public:
    static WindowInsets CONSUMED;

    WindowInsets(const Rect* systemWindowInsets,const Rect* windowDecorInsets, const Rect* stableInsets,
                   bool isRound, bool alwaysConsumeNavBar, const DisplayCutout* displayCutout);
    WindowInsets(const WindowInsets& src);
    WindowInsets(const Rect& systemWindowInsets);

    Rect getSystemWindowInsets() const;
    int getSystemWindowInsetLeft() const;
    int getSystemWindowInsetTop() const;
    int getSystemWindowInsetRight() const;
    int getSystemWindowInsetBottom() const;

    int getWindowDecorInsetLeft() const;
    int getWindowDecorInsetTop() const;
    int getWindowDecorInsetRight() const;
    int getWindowDecorInsetBottom() const;

    bool hasSystemWindowInsets() const;
    bool hasWindowDecorInsets() const;
    bool hasInsets() const;

    DisplayCutout* getDisplayCutout()const;

    WindowInsets consumeDisplayCutout();

    bool isConsumed() const;
    bool isRound() const;

    WindowInsets consumeSystemWindowInsets();
    WindowInsets consumeSystemWindowInsets(bool left, bool top, bool right, bool bottom);
    WindowInsets replaceSystemWindowInsets(int left, int top,int right, int bottom);
    WindowInsets replaceSystemWindowInsets(const Rect& systemWindowInsets);

    WindowInsets consumeWindowDecorInsets();
    WindowInsets consumeWindowDecorInsets(bool left, bool top,bool right, bool bottom);
    WindowInsets replaceWindowDecorInsets(int left, int top, int right, int bottom);

    int getStableInsetTop() const;
    int getStableInsetLeft() const;
    int getStableInsetRight() const;
    int getStableInsetBottom() const;
    bool hasStableInsets() const;
    WindowInsets consumeStableInsets();

    bool shouldAlwaysConsumeNavBar() const;

    WindowInsets inset(const Rect& r);

    WindowInsets inset(int left, int top, int right, int bottom);
    bool operator==(const WindowInsets& o)const;
    bool operator!=(const WindowInsets& o)const;
    bool isSystemWindowInsetsConsumed() const;
};

class WindowInsets::Type{
    static constexpr int FIRST = 1 << 0;
    static constexpr int STATUS_BARS = FIRST;
    static constexpr int NAVIGATION_BARS = 1 << 1;
    static constexpr int CAPTION_BAR = 1 << 2;

    static constexpr int IME = 1 << 3;

    static constexpr int SYSTEM_GESTURES = 1 << 4;
    static constexpr int MANDATORY_SYSTEM_GESTURES = 1 << 5;
    static constexpr int TAPPABLE_ELEMENT = 1 << 6;

    static constexpr int DISPLAY_CUTOUT = 1 << 7;

    static constexpr int WINDOW_DECOR = 1 << 8;

    static constexpr int SYSTEM_OVERLAYS = 1 << 9;
    static constexpr int LAST = SYSTEM_OVERLAYS;
    static constexpr int SIZE = 10;

    static constexpr int DEFAULT_VISIBLE = ~IME;
private:
    Type() = default;
protected:
    static int indexOf(int type);
public:
    static int statusBars(){return STATUS_BARS;}
    static int navigationBars() { return NAVIGATION_BARS; }
    static int captionBar() { return CAPTION_BAR; }
    static int ime() { return IME; }
    static int systemGestures() { return SYSTEM_GESTURES; }
    static int mandatorySystemGestures() { return MANDATORY_SYSTEM_GESTURES; }
    static int displayCutout() { return DISPLAY_CUTOUT; }
    static int systemOverlays() { return SYSTEM_OVERLAYS; }
    static int systemBars() {
        return STATUS_BARS | NAVIGATION_BARS | CAPTION_BAR | SYSTEM_OVERLAYS;
    }
    static int defaultVisible() { return DEFAULT_VISIBLE; }
    static int all() { return 0xFFFFFFFF; }

    static bool hasCompatSystemBars(int types) {
        return (types & (STATUS_BARS | NAVIGATION_BARS)) != 0;
    }
};

class WindowInsets::Side {
public:
    static constexpr int LEFT = 1 << 0;
    static constexpr int TOP = 1 << 1;
    static constexpr int RIGHT = 1 << 2;
    static constexpr int BOTTOM = 1 << 3;
private:
    Side() = default;
public:
    static int all() {
        return LEFT | TOP | RIGHT | BOTTOM;
    }
};

}/*endof namespace*/
#endif/*__WINDOW_INSETS_H__*/
