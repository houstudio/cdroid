#include <app/espresso/rootsoracle.h>

#include <core/windowmanager.h>
#include <widget/cdwindow.h>

namespace cdroid {
namespace espresso {

std::vector<Root> RootsOracle::listActiveRoots() {
    std::vector<Root> roots;
    std::vector<Window*> windows;
    WindowManager::getInstance().getWindows(windows);
    for (Window* window : windows) {
        // AOSP Root.Builder().withDecorView(view).withWindowLayoutParams(lp)
        roots.push_back(Root::Builder()
                .withDecorView(window)  // a CDROID Window IS its root view
                .withWindow(window)
                .withWindowLayoutParams(&window->getAttributes())
                .build());
    }
    return roots;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
