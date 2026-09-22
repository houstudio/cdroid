/*********************************************************************************
 * weardemos: port of androidx SupportWearDemos — the wear-widget demo catalog.
 *
 * Upstream: ~/research/androidx/samples/SupportWearDemos/
 *   src/main/java/com/example/android/support/wear/app/MainDemoActivity.java
 *   src/main/res/layout/*.xml
 * One file per demo activity here, like upstream (one Activity per demo):
 *   simplewearablerecyclerdemo.cc   WearableRecyclerView + WearableLinearLayoutManager
 *   simplerecyclerdemo.cc           plain RecyclerView inside BoxInsetLayout
 *   simplenestedscrollviewdemo.cc   NestedScrollView inside BoxInsetLayout
 *   wearableswitchdemo.cc           centered Switch
 *   circularprogresslayoutdemo.cc   CircularProgressLayout 10 s timer
 *   roundeddrawabledemo.cc          RoundedDrawable (code-built, see rd_demo.xml)
 *   alertdialogdemo.cc              AlertDialog (upstream shows appcompat + framework)
 *   confirmationoverlaydemo.cc      ConfirmationOverlay showOn/showAbove
 *   wearabledrawersdemo.cc         WearableDrawerLayout + navigation/action drawers
 * NOT ported from upstream: AmbientModeDemo (needs AmbientModeSupport, which the
 * CDROID wear module does not carry).
 *
 * This file: the launcher (MainDemoActivity — a WearableRecyclerView of Buttons,
 * one per demo) and main(). Navigation is App's startActivity by REGISTER_ACTIVITY
 * name (upstream navigates by Intent target class; CDROID's ActivityFactory keys
 * play that role).
 *********************************************************************************/
#include <core/app.h>
#include <cdroid.h>
#include <cdlog.h>
#include <core/activityfactory.h>
#include <core/intent.h>
#include <core/componentname.h>
#include <widget/button.h>
#include <widgetEx/wear/wearablerecyclerview.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include "R.h"

using namespace cdroid;

namespace {
// Upstream createContentMap()'s LinkedHashMap entries, minus the two skipped
// demos (drawers/ambient).
struct DemoEntry {
    const char* label;
    const char* activity;   // REGISTER_ACTIVITY key (upstream: Intent target class)
};
const DemoEntry kDemos[] = {
    {"Curved Text (ArcLayout)", "CurvedTextDemo"},
    {"Wearable Drawers",       "WearableDrawersDemo"},
    {"Wearable Recycler View", "SimpleWearableRecyclerViewDemo"},
    {"Recycler View",          "SimpleRecyclerViewDemo"},
    {"NestedScrollView View",  "SimpleNestedScrollViewDemo"},
    {"Wearable Switch",        "WearableSwitchDemo"},
    {"Circular Progress Layout", "CircularProgressLayoutDemo"},
    {"Rounded Drawable",       "RoundedDrawableDemo"},
    {"Alert Dialog (v7)",      "AlertDialogDemo"},
    {"Confirmation Overlay",   "ConfirmationOverlayDemo"},
};
} // namespace

class MainDemoActivity : public Window {
private:
    WearableRecyclerView* mDemoList = nullptr;
    class ViewHolder : public RecyclerView::ViewHolder {
    public:
        Button* mView;
        explicit ViewHolder(Button* itemView) : RecyclerView::ViewHolder(itemView), mView(itemView) {}
    };
    class DemoAdapter : public RecyclerView::Adapter {
    private:
        MainDemoActivity* mHost;
    public:
        explicit DemoAdapter(MainDemoActivity* host) : mHost(host) {}
        RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override {
            Button* view = new Button(parent->getContext(), nullptr);
            view->setLayoutParams(new RecyclerView::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT));
            view->setPadding(10, 10, 10, 10);
            view->setGravity(Gravity::CENTER);
            return new ViewHolder(view);
        }
        void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
            ViewHolder& vh = static_cast<ViewHolder&>(holder);
            vh.mView->setText(kDemos[position].label);
            vh.mView->setOnClickListener([this, position](View&) {
                // Activity.startActivity routes through the Context seam (App
                // implements it — the ActivityThread/SystemServer role).
                Intent intent("");
                intent.setComponent(ComponentName("", kDemos[position].activity));
                mHost->getContext()->startActivity(intent);
            });
        }
        int getItemCount() override {
            return sizeof(kDemos) / sizeof(kDemos[0]);
        }
    };
public:
    MainDemoActivity() : Window(&App::getInstance(), 0, 0, -1, -1) {
        WearableRecyclerView* demoList = new WearableRecyclerView(getContext(), nullptr);
        demoList->setPadding(30, 0, 30, 0);
        demoList->setLayoutManager(new LinearLayoutManager(getContext()));
        demoList->setAdapter(new DemoAdapter(this));
        demoList->setEdgeItemsCenteringEnabled(true);
        // AOSP Activity.setContentView installs MATCH_PARENT on the content
        // view; a bare addView would take the ViewGroup default (wrap) and the
        // list measures 0x0 on the first pass — a blank white window.
        addView(demoList, LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT);
        mDemoList = demoList;
    }
};
REGISTER_ACTIVITY(MainDemoActivity);

int main(int argc, const char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    App app(argc, argv);
    // The launcher activity starts itself via the manifest's MAIN/LAUNCHER
    // (App::exec launches it when no window is up) — printerdemo's shape.
    return app.exec();
}
