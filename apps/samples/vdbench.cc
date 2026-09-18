/*********************************************************************************
 * VectorDrawable rasterization benchmark — measures the three cost phases of
 * hwui::Tree (the rasterizer behind every cdroid VectorDrawable):
 *
 *   inflate : App.getDrawable(id) (XML parse + tree build)
 *   cold    : first draw (cache bitmap alloc + CLEAR + full path rasterization)
 *   warm    : repeated draws with no change (cache hit: pure OVER blit)
 *   dirty   : draw after an alpha tick (AVD-style: full re-raster each frame)
 *
 * Usage: vdbench [size [warm-iters [dirty-iters]]]   (defaults 256 200 100)
 *********************************************************************************/
#include <cdroid.h>
#include <drawable/vectordrawable.h>
#include <widget/internal_R.h>

#include <chrono>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using cdroid::App;
using cdroid::Canvas;
using cdroid::Drawable;

namespace {

using Clock = std::chrono::steady_clock;

double msSince(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

struct Case {
    const char* name;
    int drawableId;
    int pathCount;   // informationally: #pathData entries in the XML
    bool isBitmap = false;   // raster drawable (tinted, for the BitmapDrawable path)
    bool noTint = false;     // the untinted floor for the same raster
};

const std::vector<Case> kCases = {
    {"ic_arrow_forward (2 paths)",        cdroid::internal::R::drawable::ic_arrow_forward, 2},
    {"btn_checkbox_unchecked_mtrl",       cdroid::internal::R::drawable::btn_checkbox_unchecked_mtrl, 2},
    {"btn_radio_on_mtrl (rings)",         cdroid::internal::R::drawable::btn_radio_on_mtrl, 4},
    {"lockscreen_selected (9 paths)",     cdroid::internal::R::drawable::lockscreen_selected, 9},
    {"sym_def_app_icon_background (21)",  cdroid::internal::R::drawable::sym_def_app_icon_background, 21},
    {"ic_menu_add png +tint (bitmap)",    cdroid::internal::R::drawable::ic_menu_add, 0, true},
    {"ic_menu_add png notint (floor)",    cdroid::internal::R::drawable::ic_menu_add, 0, true, true},
};

void benchmark(App& app, const Case& c, int size, int warmIters, int dirtyIters,
        const char* phase = "all") {
    // --- inflate ---
    const Clock::time_point tInflate = Clock::now();
    Drawable* d = app.getDrawable(c.drawableId);
    const double inflateMs = msSince(tInflate);
    if (d == nullptr) {
        printf("%-38s  FAILED to load\n", c.name);
        return;
    }
    if (c.isBitmap && !c.noTint) {
        // The raster twin of the vector tint scenario: same tint, applied by
        // the BitmapDrawable path under test.
        d->setTint(0xFF3366FF);
    }
    d->setBounds(0, 0, size, size);

    // Offscreen target the same size as the expected cache bitmap.
    Canvas canvas(Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, size, size));

    // --- cold: first draw rasterizes and fills the cache ---
    Clock::time_point t0 = Clock::now();
    d->draw(canvas);
    const double coldMs = msSince(t0);

    if (strcmp(phase, "dump") == 0) {
        // Regression harness: one draw, write the target to PNG.
        d->draw(canvas);
        std::string file = "/tmp/vdbench_" + std::to_string(c.drawableId & 0xffff) + "_"
                + std::to_string(size) + ".png";
        canvas.get_target()->write_to_png(file.c_str());
        printf("%-38s %5d dumped %s\n", c.name, size, file.c_str());
        return;
    }
    if (strcmp(phase, "tintstates") == 0) {
        // Re-bake verification: a tint change must produce a different cache
        // (filter object change -> dirty -> re-bake), not the stale bitmap.
        d->setTint(0xFF3366FF);
        Canvas blue(Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, size, size));
        d->draw(blue);
        blue.get_target()->write_to_png("/tmp/vdbench_tint_blue.png");
        d->setTint(0xFFFF6633);
        Canvas red(Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, size, size));
        d->draw(red);
        red.get_target()->write_to_png("/tmp/vdbench_tint_red.png");
        printf("%-38s %5d tint states dumped blue+red\n", c.name, size);
        return;
    }
    if (strcmp(phase, "warm") == 0) {
        // profiling mode: loop the clean-draw phase for ~seconds
        t0 = Clock::now();
        for (int i = 0; i < warmIters; i++) d->draw(canvas);
        printf("%-38s %5d warm-only %8.4f ms/draw (%d iters, %.0f ms total)\n",
               c.name, size, msSince(t0) / warmIters, warmIters, msSince(t0));
        return;
    }
    if (strcmp(phase, "dirty") == 0) {
        t0 = Clock::now();
        for (int i = 0; i < dirtyIters; i++) {
            d->setAlpha((i & 1) ? 255 : 254);
            d->draw(canvas);
        }
        printf("%-38s %5d dirty-only %8.3f ms/frame (%d iters, %.0f ms total)\n",
               c.name, size, msSince(t0) / dirtyIters, dirtyIters, msSince(t0));
        return;
    }

    // --- warm: no property change; Tree::drawStaging blits the cache ---
    t0 = Clock::now();
    for (int i = 0; i < warmIters; i++) d->draw(canvas);
    const double warmTotalMs = msSince(t0);

    // --- dirty: alpha tick per frame (AVD-style full re-raster via staging) ---
    t0 = Clock::now();
    for (int i = 0; i < dirtyIters; i++) {
        d->setAlpha((i & 1) ? 255 : 254);   // staging property change -> cache dirty
        d->draw(canvas);
    }
    const double dirtyTotalMs = msSince(t0);

    printf("%-38s %5dx%-5d inflate %8.2f ms | cold %8.2f ms | warm %8.4f ms/draw"
           " | dirty %8.3f ms/frame\n",
           c.name, size, size, inflateMs, coldMs,
           warmTotalMs / warmIters, dirtyTotalMs / dirtyIters);
}

} // namespace

int main(int argc, const char* argv[]) {
    App app(argc, argv);

    int size = argc > 1 ? atoi(argv[1]) : 256;
    int warmIters = argc > 2 ? atoi(argv[2]) : 200;
    int dirtyIters = argc > 3 ? atoi(argv[3]) : 100;
    const int caseIdx = argc > 4 ? atoi(argv[4]) : -1;
    const char* phase = argc > 5 ? argv[5] : "all";
    if (warmIters <= 0) warmIters = 200;
    if (dirtyIters <= 0) dirtyIters = 100;

    printf("== VectorDrawable rasterization benchmark (warm=%d dirty=%d) ==\n",
           warmIters, dirtyIters);
    if (caseIdx >= 0) {
        benchmark(app, kCases[caseIdx], size, warmIters, dirtyIters, phase);
    } else {
        for (int s : {64, size == 64 ? 256 : size, 512}) {
            printf("-- size %d --\n", s);
            for (const Case& c : kCases) benchmark(app, c, s, warmIters, dirtyIters);
        }
    }
    fflush(stdout);

    // Benchmark only: skip the windowless App teardown (X/input thread joins
    // block it); measurement is done, flush and bail.
    std::_Exit(0);
    return app.exec();
}
