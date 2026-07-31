/*
 * Barrier demo — a vertical Barrier tracks the rightmost of two boxes.
 * Box A (narrow) + Box B (wide) → Barrier on their right side → Box C pinned to the Barrier.
 * Tap A or B to toggle B's width; C follows the Barrier.
 * Build: make barrierdemo -j44   Run: ./barrierdemo
 */
#include <cdroid.h>
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/helpers/barrier.h>

using namespace cdroid;

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* win = new Window(0, 0, -1, -1);

    ConstraintLayout* cl = new ConstraintLayout(-1, -1);
    cl->setBackgroundColor(0xFF1B1B2F);
    win->addView(cl);

    // Box A (always 120 wide)
    TextView* a = new TextView("A", 120, 80);
    a->setId(1); a->setBackgroundColor(0xFFEF5350);
    a->setGravity(Gravity::CENTER); a->setTextColor(0xFFFFFFFF); a->setTextSize(18);
    auto* lpa = new ConstraintLayout::LayoutParams(120, 80);
    lpa->leftToLeft = ConstraintLayout::PARENT_ID;
    lpa->topToTop = ConstraintLayout::PARENT_ID;
    lpa->topMargin = 50; lpa->leftMargin = 50;
    cl->addView(a, lpa);

    // Box B (starts wide, tap toggles width)
    TextView* b = new TextView("B (tap)", 300, 80);
    b->setId(2); b->setBackgroundColor(0xFF66BB6A);
    b->setGravity(Gravity::CENTER); b->setTextColor(0xFFFFFFFF); b->setTextSize(18);
    auto* lpb = new ConstraintLayout::LayoutParams(300, 80);
    lpb->leftToLeft = ConstraintLayout::PARENT_ID;
    lpb->topToTop = ConstraintLayout::PARENT_ID;
    lpb->topMargin = 150; lpb->leftMargin = 50;
    cl->addView(b, lpb);

    // Right Barrier referencing A + B
    Barrier* barrier = new Barrier(ConstraintLayout::LayoutParams::WRAP_CONTENT,
                                   ConstraintLayout::LayoutParams::WRAP_CONTENT);
    barrier->setId(10);
    barrier->setType(Barrier::RIGHT);
    barrier->setReferencedIds({1, 2});
    cl->addView(barrier, new ConstraintLayout::LayoutParams(
            ConstraintLayout::LayoutParams::WRAP_CONTENT,
            ConstraintLayout::LayoutParams::WRAP_CONTENT));

    // Box C pinned to the Barrier's left → sits right after the widest of A/B.
    TextView* c = new TextView("C", 100, 80);
    c->setId(3); c->setBackgroundColor(0xFF42A5F5);
    c->setGravity(Gravity::CENTER); c->setTextColor(0xFFFFFFFF); c->setTextSize(18);
    auto* lpc = new ConstraintLayout::LayoutParams(100, 80);
    lpc->leftToLeft = 10; // barrier id
    lpc->topToTop = ConstraintLayout::PARENT_ID;
    lpc->topMargin = 100;
    lpc->leftMargin = 20;
    cl->addView(c, lpc);

    // Toggle B's width on tap.
    bool wide = true;
    b->setOnClickListener([b, &wide](View&) {
        wide = !wide;
        auto* lp = static_cast<ConstraintLayout::LayoutParams*>(b->getLayoutParams());
        lp->width = wide ? 300 : 80;
        b->setText(wide ? "B (tap)" : "B*");
        b->requestLayout();
    });

    return app.exec();
}
