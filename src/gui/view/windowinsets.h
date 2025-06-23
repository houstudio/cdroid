#ifndef __WINDOW_INSETS_H__
#define __WINDOW_INSETS_H__
#include <core/rect.h>
namespace cdroid{
class DisplayCutout;
class WindowInsets {
public:
    class Side {
    public:
        static constexpr int LEFT = 1 << 0;
        static constexpr int TOP = 1 << 1;
        static constexpr int RIGHT = 1 << 2;
        static constexpr int BOTTOM = 1 << 3;
    private:
        Side() {
        }
    public:
        static int all() {
            return LEFT | TOP | RIGHT | BOTTOM;
        }
    };
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
}/*endof namespace*/
#endif/*__WINDOW_INSETS_H__*/
