/*
 * Flow demo — widgets wrap into rows inside a narrow container.
 * Build: make flowdemo -j44   Run: ./flowdemo
 */
#include <cdroid.h>
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/helpers/flow.h>

using namespace cdroid;

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* win = new Window(0, 0, -1, -1);

    ConstraintLayout* cl = new ConstraintLayout(-1, -1);
    cl->setBackgroundColor(0xFF1B1B2F);
    win->addView(cl);

    // Create 6 colorful boxes.
    uint32_t colors[] = {0xFFEF5350, 0xFF66BB6A, 0xFF42A5F5, 0xFFFFCA28, 0xFFAB47BC, 0xFF26C6DA};
    const char* labels[] = {"A", "B", "C", "D", "E", "F"};
    for (int i = 0; i < 6; i++) {
        auto* tv = new TextView(labels[i], 120, 80);
        tv->setId(i + 1);
        tv->setBackgroundColor(colors[i]);
        tv->setGravity(Gravity::CENTER);
        tv->setTextColor(0xFFFFFFFF);
        tv->setTextSize(20);
        cl->addView(tv, new ConstraintLayout::LayoutParams(120, 80));
    }

    // Flow: 350 wide, wraps the 6 boxes (2 per row).
    Flow* flow = new Flow(350, 300);
    flow->setId(100);
    flow->setReferencedIds({1, 2, 3, 4, 5, 6});
    flow->setWrapMode(Flow::WRAP_CHAIN);
    flow->setHorizontalGap(8);
    flow->setVerticalGap(8);
    auto* lp = new ConstraintLayout::LayoutParams(350, 300);
    lp->leftToLeft = ConstraintLayout::PARENT_ID;
    lp->rightToRight = ConstraintLayout::PARENT_ID;
    lp->topToTop = ConstraintLayout::PARENT_ID;
    lp->verticalBias = 0.1f;
    cl->addView(flow, lp);

    return app.exec();
}
