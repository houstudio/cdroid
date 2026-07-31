/*
 * ConstraintLayout demo — exercises the MVP features:
 *  - Centered child (left+right to parent, bias 0.5)
 *  - Match_constraint (0dp) child (fills)
 *  - Pinned-right child (rightToRight to parent)
 *  - Guideline at 33% + a child constrained to it
 *
 * Build: make constraintlayoutdemo -j44 (auto-registered by apps/samples/CMakeLists.txt)
 * Run:   ./constraintlayoutdemo
 */
#include <cdroid.h>
#include <widgetEx/constraintlayout/constraintlayout.h>

using namespace cdroid;

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* win = new Window(0, 0, -1, -1);

    ConstraintLayout* cl = new ConstraintLayout(-1, -1); // fills window
    cl->setBackgroundColor(0xFF1B1B2F);

    // 1. Centered red box (width 200, height 80)
    TextView* centered = new TextView("Centered", 200, 80);
    centered->setBackgroundColor(0xFFEF5350);
    centered->setGravity(Gravity::CENTER);
    centered->setTextColor(0xFFFFFFFF);
    centered->setTextSize(18);
    auto* lp1 = new ConstraintLayout::LayoutParams(200, 80);
    lp1->leftToLeft = ConstraintLayout::PARENT_ID;
    lp1->rightToRight = ConstraintLayout::PARENT_ID;
    lp1->topToTop = ConstraintLayout::PARENT_ID;
    lp1->verticalBias = 0.1f; // near top
    cl->addView(centered, lp1);

    // 2. Match_constraint green bar (0dp, fills width, below the red)
    TextView* fill = new TextView("0dp Fill", 0, 60);
    fill->setBackgroundColor(0xFF66BB6A);
    fill->setGravity(Gravity::CENTER);
    fill->setTextColor(0xFFFFFFFF);
    fill->setTextSize(16);
    auto* lp2 = new ConstraintLayout::LayoutParams(0, 60);
    lp2->leftToLeft = ConstraintLayout::PARENT_ID;
    lp2->rightToRight = ConstraintLayout::PARENT_ID;
    lp2->topToBottom = 1; // below centered
    centered->setId(1);
    cl->addView(fill, lp2);

    // 3. Guideline at 66% vertical + a blue box constrained to it
    View* gl = new View(0, 0);
    gl->setId(10);
    auto* glp = new ConstraintLayout::LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    glp->orientation = ConstraintWidget::VERTICAL;
    glp->guidePercent = 0.66f;
    glp->validate();
    cl->addView(gl, glp);

    TextView* guided = new TextView("Guideline 66%", 0, 60);
    guided->setBackgroundColor(0xFF42A5F5);
    guided->setGravity(Gravity::CENTER);
    guided->setTextColor(0xFFFFFFFF);
    guided->setTextSize(16);
    auto* lp3 = new ConstraintLayout::LayoutParams(0, 60);
    lp3->leftToLeft = 10;        // guideline
    lp3->rightToRight = ConstraintLayout::PARENT_ID;
    lp3->topToTop = ConstraintLayout::PARENT_ID;
    lp3->verticalBias = 0.95f;   // near bottom
    cl->addView(guided, lp3);

    win->addView(cl);
    return app.exec();
}
