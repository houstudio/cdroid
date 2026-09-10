#include <cdroid.h>
#include <cdlog.h>
#include <widget/internal_R.h>

using namespace cdroid;

/* Repro v3 for the R5 widgetsDemo definite leak (96B AnimationState vector /
 * 692KB nine-patch surfaces, via switch_thumb_material_anim): real Switches in
 * a real window so the transitions' ObjectAnimator actually ticks (frames
 * materialize), the theme/tint path applies, and view callbacks run — the
 * plain no-exec-loop variant did not leak. */
int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* w = new Window(0, 0, -1, -1);
    LinearLayout* box = new LinearLayout(&app);
    box->setOrientation(LinearLayout::VERTICAL);
    for (int i = 0; i < 4; i++) {          // four switches, as on the widgets page
        Switch* s = new Switch(&app);
        s->setText("switch");
        box->addView(s);
    }
    w->addView(box);

    int round = 0;
    std::function<void()> tick = [&]() {
        if (round >= 12) { app.exit(0); return; }
        const bool on = (round & 1) != 0;  // play the 12-frame on/off transitions
        for (int i = 0; i < box->getChildCount(); i++)
            ((Switch*)box->getChildAt(i))->setChecked(on);
        round++;
        box->postDelayed(tick, 150);
    };
    box->postDelayed(tick, 150);

    app.exec();
    return 0;
}
