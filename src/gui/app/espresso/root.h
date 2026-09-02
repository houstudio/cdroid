#ifndef CDROID_ESPRESSO_ROOT_H
#define CDROID_ESPRESSO_ROOT_H

/*
 * android.support.test.espresso.Root — represents a root view in the
 * application and optionally the layout params of the window holding it.
 *
 * Used internally to determine which view root to run user provided matchers
 * against; it is not part of the public api.
 *
 * CDROID bindings (in place of AOSP's binder tokens):
 *  - a CDROID Window IS the root view (Window : FrameLayout), so the decor
 *    view of a root is the Window itself;
 *  - Java's Optional<WindowManager.LayoutParams> maps onto a nullable
 *    pointer. Window::getAttributes() returns the LIVE attribute object
 *    owned by the window, so the borrowed pointer is valid while the window
 *    lives — Espresso holds roots only for the duration of an interaction;
 *  - AOSP reaches the owning window through decorView.getWindowToken(); with
 *    no tokens, Root carries the cdroid::Window* directly (Builder.withWindow).
 */

#include <string>

#include <core/windowmanager.h>

namespace cdroid {
class View;

namespace espresso {

class Root {
public:
    class Builder;

    Root() = default;

    View* getDecorView() const { return mDecorView; }

    /** Java Optional<WindowManager.LayoutParams> — null when absent. */
    const WindowManager::LayoutParams* getWindowLayoutParams() const { return mWindowLayoutParams; }

    /** CDROID: the owning window (AOSP: decorView.getWindowToken() dance). */
    Window* getWindow() const { return mWindow; }

    std::string toString() const;

private:
    friend class Builder;
    explicit Root(const Builder& builder);
    View* mDecorView = nullptr;
    const WindowManager::LayoutParams* mWindowLayoutParams = nullptr;
    Window* mWindow = nullptr;
};

class Root::Builder {
public:
    Builder& withDecorView(View* view) { mDecorView = view; return *this; }
    Builder& withWindowLayoutParams(const WindowManager::LayoutParams* windowLayoutParams) {
        mWindowLayoutParams = windowLayoutParams;
        return *this;
    }
    Builder& withWindow(Window* window) { mWindow = window; return *this; }
    Root build() const { return Root(*this); }

private:
    friend class Root;
    View* mDecorView = nullptr;
    const WindowManager::LayoutParams* mWindowLayoutParams = nullptr;
    Window* mWindow = nullptr;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ROOT_H*/
