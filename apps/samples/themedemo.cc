// Dynamic theme switching, the AOSP way: Activity.setTheme() + recreate().
//
// Flow (mirrors android.app.Activity):
//   1. Window(&app, ...) wraps the context in a ContextThemeWrapper — CDROID's
//      Window IS the Activity, and an AOSP Activity IS a themed context.
//   2. setTheme(resid) routes into the wrapper: the style is applied to the
//      live Theme, so everything inflated afterwards (and every lazy ?attr
//      resolution) sees it.
//   3. recreate() closes this window and runs the REGISTER_ACTIVITY factory
//      again — a fresh instance inflates under the (already switched) theme.
//      Already-inflated views are never re-themed in place; AOSP behaves the
//      same. The theme selection itself is re-read by the new instance from
//      wherever the app persists it (here: the static below).
#include <cdroid.h>
#include <cdlog.h>
#include <core/activityfactory.h>
#include <widget/internal_R.h>
#include <widget/button.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>

using namespace cdroid;

// App-persisted theme choice (AOSP parity: onCreate re-reads the persisted
// selection after recreate()).
static int sThemeResId = (int)cdroid::internal::R::style::Theme_Material_Light;

class ThemeWindow : public Window {
public:
    ThemeWindow():Window(&App::getInstance(),0,0,640,480){
        // AOSP Activity.setTheme, called before the content is set up.
        setTheme(sThemeResId);

        AttributeSet atts(getContext(), "cdroid");
        LinearLayout* root = new LinearLayout(getContext(), &atts);
        root->setOrientation(LinearLayout::VERTICAL);

        TextView* title = new TextView(getContext(), &atts);
        title->setText("Dynamic theme switching");
        title->setTextSize(24);
        // ?attr resolution goes through THIS window's theme overlay.
        TypedValue tv;
        if (getContext()->getTheme().resolveAttribute(
                    (int)cdroid::internal::R::attr::colorPrimary, &tv, true)) {
            title->setBackgroundColor(tv.data);
        }

        Button* btn = new Button(getContext(), &atts);
        btn->setText("Toggle theme + recreate");
        btn->setOnClickListener([this](View&){
            sThemeResId = (sThemeResId == (int)cdroid::internal::R::style::Theme_Material_Light)
                        ?  (int)cdroid::internal::R::style::Theme_Material
                        :  (int)cdroid::internal::R::style::Theme_Material_Light;
            recreate();   // AOSP Activity.recreate()
        });

        // AOSP night mode: a uiMode configuration change routed through the
        // "system" (App::handleConfigurationChanged = ActivityThread) — every
        // activity is either dispatched (configChanges) or recreated.
        Button* night = new Button(getContext(), &atts);
        night->setText("Toggle night mode (uiMode config change)");
        static bool sNight = false;
        night->setOnClickListener([](View&){
            sNight = !sNight;
            App& app = App::getInstance();
            Configuration c = app.getResources().getConfiguration();
            c.uiMode = (c.uiMode & ~Configuration::UI_MODE_NIGHT_MASK)
                     | (sNight ? Configuration::UI_MODE_NIGHT_YES : Configuration::UI_MODE_NIGHT_NO);
            app.handleConfigurationChanged(c);   // → -night resource variants + recreate
        });

        root->addView(title);
        root->addView(btn);
        root->addView(night);
        addView(root);
    }
};
REGISTER_ACTIVITY(ThemeWindow);

int main(int argc,const char* argv[]){
    App app(argc,argv);
    Intent intent("");
    intent.setComponent(ComponentName("","ThemeWindow"));
    app.startActivity(intent);
    return app.exec();
}
