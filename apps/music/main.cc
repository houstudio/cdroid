/*********************************************************************************
 * CDROID port of the AOSP Music app (packages/apps/Music, java flavor) —
 * a UI-fidelity facade: original layouts/drawables/strings, in-process mock
 * media store (src/musicdb.*), fake playback service (src/mediaplaybackservice.*).
 *
 * The classic Music look is the dark theme; Theme_Material (dark) supplies
 * the text appearances the layouts resolve (?android:attr/textAppearance*).
 *********************************************************************************/
#include <cdroid.h>

#include <R.h>
#include <core/intent.h>
#include <widget/internal_R.h>

int main(int argc, const char* argv[]) {
    cdroid::App app(argc, argv);

    app.setTheme(cdroid::internal::R::style::Theme_Material);

    // Launch through the ActivityFactory registry (REGISTER_ACTIVITY keys),
    // the same path MusicUtils::activateTab uses for the tab screens.
    cdroid::Intent intent;
    intent.setClassName("cdroid.music", "MusicBrowserActivity")
          .setAction(cdroid::Intent::ACTION_MAIN);
    app.startActivity(intent);

    return app.exec();
}
