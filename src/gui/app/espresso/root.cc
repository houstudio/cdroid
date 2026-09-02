#include <app/espresso/root.h>

#include <app/espresso/humanreadables.h>
#include <app/espresso/rootmatchers.h>

#include <core/windowmanager.h>
#include <view/view.h>

namespace cdroid {
namespace espresso {

Root::Root(const Builder& builder)
    : mDecorView(builder.mDecorView),
      mWindowLayoutParams(builder.mWindowLayoutParams),
      mWindow(builder.mWindow) {}

std::string Root::toString() const {
    // AOSP: application-window-token / window-token / has-window-focus /
    // layout-params-type / layout-params-string / decor-view-string.
    std::string out = "Root{has-window-focus=";
    out += (mDecorView && viewHasWindowFocus(mDecorView)) ? "true" : "false";
    if (mWindowLayoutParams != nullptr) {
        out += ", layout-params-type=" + std::to_string(mWindowLayoutParams->type);
        out += ", flags=" + std::to_string(mWindowLayoutParams->flags);
    }
    if (mDecorView != nullptr) {
        out += ", decor-view-string=" + HumanReadables::describe(mDecorView);
    }
    out += "}";
    return out;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
