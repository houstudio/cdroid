/* NavigationView sample: exercises the material presenter behaviors —
 * group separators, icon-column alignment (needsEmptyIcon), inline submenus
 * (subheader + flattened children) and exclusive checked state. The menu is
 * built programmatically, no menu resource needed. */
#include <cdroid.h>
#include <widgetEx/navigationview/navigationview.h>
#include <menu/menubuilder.h>
#include <menu/menuitem.h>
#include <drawable/gradientdrawable.h>
#include <core/handler.h>
#include <core/looper.h>

static Drawable* colorIcon(Context* ctx, int color){
    GradientDrawable* d = new GradientDrawable();
    d->setShape(GradientDrawable::OVAL);
    d->setColor(color);
    return d;
}

class SelectionLogger : public cdroid::NavigationView::OnNavigationItemSelectedListener {
public:
    bool onNavigationItemSelected(cdroid::MenuItem* item) override {
        LOGD("selected %d '%s'", item->getItemId(), item->getTitle().c_str());
        return true;
    }
};

int main(int argc, const char* argv[]){
    App app(argc, argv);
    Window* w = new Window(0, 0, 480, 640);
    w->setBackgroundColor(0xFF202020);

    NavigationView* nv = new NavigationView(&app, nullptr, 0);
    nv->setId(0x1000);   // needed: saveHierarchyState only stores views with an id
    nv->setLayoutParams(new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
    w->addView(nv);

    Menu* menu = nv->getMenu();
    // Group 1: every item has an icon.
    menu->add(1, 1001, 0, "Inbox")->setIcon(colorIcon(&app, 0xFF26A69A)).setCheckable(true);
    menu->add(1, 1002, 1, "Outbox")->setIcon(colorIcon(&app, 0xFF42A5F5)).setCheckable(true);
    menu->setGroupCheckable(1, true, true);
    // Group 2: no icons at all (no icon column reserved).
    menu->add(2, 2001, 2, "Settings")->setCheckable(true);
    menu->add(2, 2002, 3, "Help & feedback")->setCheckable(true);
    // Submenu: rendered as a subheader followed by its flattened children.
    SubMenu* sub = menu->addSubMenu("Labels");
    sub->add(3, 3001, 4, "Work")->setIcon(colorIcon(&app, 0xFFEF5350)).setCheckable(true);
    sub->add(3, 3002, 5, "Family")->setIcon(colorIcon(&app, 0xFFFFCA28));
    sub->add(3, 3003, 6, "No icon child");

    nv->setNavigationItemSelectedListener(new SelectionLogger());
    nv->setCheckedItem(1001);

    // Self-test: move the checked item after a delay — drives the menu
    // update path (prepareMenuItems + full rebind through the presenter),
    // the same flow a tap on an item takes — then exercises the saved-state
    // round trip (recreate-style) through the public hierarchy APIs.
    static cdroid::Handler sRebindDriver(cdroid::Looper::getMainLooper());
    sRebindDriver.postDelayed([nv](){
        nv->setCheckedItem(1002);

        cdroid::SparseArray<cdroid::Parcelable*> saved;
        nv->saveHierarchyState(saved);
        nv->setCheckedItem(1001);
        nv->restoreHierarchyState(saved);
        const int restored = nv->getCheckedItem() ? nv->getCheckedItem()->getItemId() : -1;
        LOGD("saved-state round trip: checked=%d (expect 1002)", restored);
    }, 1200);

    w->requestLayout();
    return app.exec();
}
