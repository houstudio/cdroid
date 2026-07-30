/*
 * ConstraintLayoutStates demo — adaptive (responsive) layout.
 *
 * A <StateSet> maps the layout's width to a ConstraintSet: the default set places a small box at the
 * top-left; a Variant for width>800 places a large box at the bottom-right. Tap the box to toggle the
 * simulated width (400 <-> 1200) and watch the layout swap instantly via setState.
 *
 * (CDROID screens are usually fixed, so the demo drives the dimension with a tap instead of a real
 * resize. The `constraints` refs here are inline <ConstraintSet>s; "@layout/.." layout-resource refs
 * are also supported — ConstraintSet.clone inflates them offscreen.)
 *
 * Build: make constraintlayoutstatesdemo -j44   Run: ./constraintlayoutstatesdemo
 */
#include <sstream>

#include <cdroid.h>
#include <core/xmlpullparser.h>
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/constraintlayoutstates.h>

using namespace cdroid;

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* win = new Window(0, 0, -1, -1);

    ConstraintLayout* cl = new ConstraintLayout(-1, -1);
    cl->setBackgroundColor(0xFF1B1B2F);
    win->addView(cl);

    TextView* box = new TextView("Tap", 100, 100);
    box->setId(42);
    box->setBackgroundColor(0xFF42A5F5);
    box->setGravity(Gravity::CENTER);
    box->setTextColor(0xFFFFFFFF);
    box->setTextSize(18);
    cl->addView(box, new ConstraintLayout::LayoutParams(100, 100));

    // StateSet: default set = small box top-left; Variant width>800 = large box bottom-right.
    const std::string xml =
        "<StateSet xmlns:android=\"http://schemas.android.com/apk/res/android\""
        "          defaultState=\"@+id/base\">"
        "  <State android:id=\"@+id/base\" constraints=\"@+id/small\">"
        "    <Variant region_widthMoreThan=\"800\" constraints=\"@+id/large\"/>"
        "  </State>"
        "  <ConstraintSet android:id=\"@+id/small\">"
        "    <Constraint android:id=\"42\" android:layout_width=\"120dp\" android:layout_height=\"120dp\""
        "                layout_constraintLeft_toLeftOf=\"parent\" layout_constraintTop_toTopOf=\"parent\"/>"
        "  </ConstraintSet>"
        "  <ConstraintSet android:id=\"@+id/large\">"
        "    <Constraint android:id=\"42\" android:layout_width=\"240dp\" android:layout_height=\"240dp\""
        "                layout_constraintRight_toRightOf=\"parent\" layout_constraintBottom_toBottomOf=\"parent\"/>"
        "  </ConstraintSet>"
        "</StateSet>";
    auto stream = std::make_unique<std::stringstream>(xml);
    XmlPullParser parser(&app, std::move(stream));
    while (parser.getEventType() != XmlPullParser::START_TAG &&
           parser.getEventType() != XmlPullParser::END_DOCUMENT) {
        parser.next();
    }
    ConstraintLayoutStates* states = new ConstraintLayoutStates(&app, cl, parser);
    const int baseId = states->getId("@+id/base");

    bool wide = false;
    auto apply = [&]() {
        // setState would route through the layout; call updateConstraints directly (states owns the
        // layout ref). Width drives Variant selection.
        states->updateConstraints(baseId, wide ? 1200.0f : 400.0f, 800.0f);
        cl->requestLayout();
    };
    apply(); // initial layout (narrow)
    box->setOnClickListener([&](View&) { wide = !wide; apply(); });

    return app.exec();
}
