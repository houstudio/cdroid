#include <app/espresso/humanreadables.h>

#include <app/espresso/treeiterables.h>

#include <widget/checkable.h>
#include <widget/textview.h>
#include <view/viewgroup.h>

namespace cdroid {
namespace espresso {

std::string HumanReadables::getViewHierarchyErrorMessage(View* rootView,
        const std::vector<View*>& problemViews, const std::string& errorHeader,
        const std::string& problemViewSuffix) {
    std::string errorMessage = errorHeader;
    if (!problemViewSuffix.empty()) {
        errorMessage += "\nProblem views are marked with '" + problemViewSuffix + "' below.";
    }
    errorMessage += "\n\nView Hierarchy:\n";

    // AOSP: Joiner.on("\n").appendTo(errorMessage, Iterables.transform(
    //   depthFirstViewTraversalWithDistance(rootView), viewAndDistance -> ...))
    bool first = true;
    for (const ViewAndDistance& viewAndDistance
            : depthFirstViewTraversalWithDistance(rootView)) {
        if (!first) errorMessage += "\n";
        first = false;

        std::string formatString = "+%s%s ";
        if (std::find(problemViews.begin(), problemViews.end(), viewAndDistance.getView())
                != problemViews.end()) {
            formatString += problemViewSuffix;
        }
        formatString += "\n|";

        // Strings.padStart(">", distanceFromRoot + 1, '-')
        std::string indent(viewAndDistance.getDistanceFromRoot() + 1, '-');
        indent += ">";

        char line[512];
        snprintf(line, sizeof(line), formatString.c_str(), indent.c_str(),
                describe(viewAndDistance.getView()).c_str());
        errorMessage += line;
    }
    return errorMessage;
}

std::string HumanReadables::describe(View* v) {
    if (v == nullptr) {
        return "null";
    }

    // Objects.toStringHelper(v) — a compact name{key=value, ...} rendering.
    std::string out = "View{id=";
    out += std::to_string(v->getId());

    if (v->getId() != View::NO_ID && v->getContext() != nullptr) {
        // AOSP resolves the entry name and swallows Resources.NotFoundException.
        const std::string resName = v->getContext()->getResourceName(v->getId());
        if (!resName.empty()) {
            out += ", res-name=" + resName;
        }
    }
    const std::string desc = v->getContentDescription();
    if (!desc.empty()) {
        out += ", desc=" + desc;
    }

    switch (v->getVisibility()) {
        case View::GONE:     out += ", visibility=GONE"; break;
        case View::INVISIBLE: out += ", visibility=INVISIBLE"; break;
        case View::VISIBLE:  out += ", visibility=VISIBLE"; break;
        default: out += ", visibility=" + std::to_string(v->getVisibility()); break;
    }

    out += ", width=" + std::to_string(v->getWidth());
    out += ", height=" + std::to_string(v->getHeight());
    out += ", has-focus=" + std::string(v->hasFocus() ? "true" : "false");
    out += ", has-focusable=" + std::string(v->hasFocusable() ? "true" : "false");
    // has-window-focus omitted: CDROID's View::hasWindowFocus is protected
    // (public in AOSP); the window-stack probe lives in rootmatchers.h.
    out += ", is-clickable=" + std::string(v->isClickable() ? "true" : "false");
    out += ", is-enabled=" + std::string(v->isEnabled() ? "true" : "false");
    out += ", is-focused=" + std::string(v->isFocused() ? "true" : "false");
    out += ", is-focusable=" + std::string(v->isFocusable() ? "true" : "false");
    out += ", is-layout-requested=" + std::string(v->isLayoutRequested() ? "true" : "false");
    out += ", is-selected=" + std::string(v->isSelected() ? "true" : "false");

    if (v->getRootView() != nullptr) {
        // pretty much only true in unit-tests.
        out += ", root-is-layout-requested="
            + std::string(v->getRootView()->isLayoutRequested() ? "true" : "false");
    }

    // AOSP then probes onCreateInputConnection (has-input-connection /
    // editor-info) — the InputConnection surface is not ported to CDROID.

    out += ", x=" + std::to_string(v->getX());
    out += ", y=" + std::to_string(v->getY());

    if (TextView* textBox = dynamic_cast<TextView*>(v)) {
        // innerDescribe(TextView)
        const std::string text = std::string(textBox->getText());
        if (!text.empty()) out += ", text=" + text;
        // error-text / hint / has-links omitted: getError/getUrls unported.
        CharSequence* hint = textBox->getHint();
        if (hint != nullptr && !std::string(*hint).empty()) {
            out += ", hint=" + std::string(*hint);
        }
        out += ", input-type=" + std::to_string(textBox->getInputType());
        out += ", ime-target=" + std::string(textBox->isInputMethodTarget() ? "true" : "false");
    }
    if (Checkable* checkable = dynamic_cast<Checkable*>(v)) {
        // innerDescribe(Checkable)
        out += ", is-checked=" + std::string(checkable->isChecked() ? "true" : "false");
    }
    if (ViewGroup* viewGroup = dynamic_cast<ViewGroup*>(v)) {
        // innerDescribe(ViewGroup)
        out += ", child-count=" + std::to_string(viewGroup->getChildCount());
    }
    out += "}";
    return out;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
