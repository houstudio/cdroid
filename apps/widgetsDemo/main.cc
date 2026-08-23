#include <cdroid.h>
#include <view/layoutinflater.h>
#include <widgetEx/viewpager2.h>
#include <widgetEx/tablayoutmediator.h>
#include <widget/tablayout.h>
#include <porting/cdlog.h>
#include <R.h>
#include "pageradapter.h"
#include <typeinfo>

using namespace cdroid;

static const char* const TITLES[] = {
    "Buttons", "Progress", "Text", "Images", "Anim", "Lists", "Misc", "DateTime",
    "Constraint", "Motion",
};

static void dumpTree(View* v, int depth) {
    std::string ind(depth * 2, ' ');
    ViewGroup* vg = dynamic_cast<ViewGroup*>(v);
    int childn = vg ? vg->getChildCount() : 0;
    LOGE("WDEMO-DUMP %s%s id=0x%x vis=%d meas=%dx%d real=%dx%d children=%d",
         ind.c_str(), typeid(*v).name(), v->getId(), (int)v->getVisibility(),
         v->getMeasuredWidth(), v->getMeasuredHeight(), v->getWidth(), v->getHeight(), childn);
    if (vg) {
        for (int i = 0; i < vg->getChildCount(); i++) {
            View* c = vg->getChildAt(i);
            if (c) dumpTree(c, depth + 1);
        }
    }
}

int main(int argc, const char* argv[]) {
    LOGE("WDEMO main enter");
    App app(argc, argv);
    LOGE("WDEMO App ctor done");

    Window* w = new Window(0, 0, -1, -1);
    View* root = nullptr;
    {
        // Probe the layout AXML bytes directly — getXml(resid) is what inflate's
        // XmlPullParser uses; null here = the layout file isn't loadable.
        Asset* asset = app.getResources().getXml(widgetsDemo::R::layout::main);
        if (asset) { delete asset; }
        root = LayoutInflater::from(&app)->inflate(widgetsDemo::R::layout::main, w);
    }

    ViewPager2* pager = (ViewPager2*)w->findViewById(widgetsDemo::R::id::pager);
    TabLayout*  tabs  = (TabLayout*) w->findViewById(widgetsDemo::R::id::tabs);

    if (pager) {
        pager->setAdapter(new DemoPagerAdapter());
        if (tabs) {
            TabLayoutMediator* mediator=new TabLayoutMediator(tabs, pager,
                [](TabLayout::Tab& tab, int pos) { tab.setText(TITLES[pos]); });
            mediator->attach();
        }
    }
    return app.exec();
}
