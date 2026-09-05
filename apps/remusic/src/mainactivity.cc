// Port of com.wm.remusic.activity.MainActivity — DrawerLayout + top tab bar
// (在线 stub / 本地) + CustomViewPager + bottom quick-controls. The online
// page shows the offline notice (Baidu-ting backend is gone); the local page
// hosts MainFragment exactly like the original page 1.
#include <cdroid.h>
#include <R.h>
#include <core/activityfactory.h>
#include <core/intent.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragment.h>
#include <fragment/fragmentpageradapter.h>
#include <widget/imageview.h>
#include <widget/adapter.h>
#include <widget/listview.h>
#include <widget/textview.h>
#include <widget/viewpager.h>

#include "mainfragment.h"
#include "quickcontrols.h"
#include "themestore.h"

using namespace cdroid;
using namespace remusic;

namespace {

// MenuItemAdapter, local cut: the drawer rows are bare TextViews, which is
// exactly ArrayAdapter's mFieldId==0 shape.
using MenuAdapter = ArrayAdapter<std::string>;

class OnlineStubFragment : public Fragment {
public:
    View* onCreateView(LayoutInflater* inflater, ViewGroup* /*container*/, Bundle*) override {
        auto* tv = new TextView(inflater->getContext());
        tv->setText("在线服务已下线\n(百度 ting 接口 2017 年停服)\n请使用本地音乐");
        tv->setTextSize(18);
        tv->setGravity(Gravity::CENTER);
        return tv;
    }
};

class MainPagerAdapter : public FragmentPagerAdapter {
public:
    MainPagerAdapter(FragmentManager* fm) : FragmentPagerAdapter(fm) {}
    Fragment* getItem(int position) override {
        return position == 0 ? (Fragment*) new OnlineStubFragment() : (Fragment*) new MainFragment();
    }
    int getCount() override { return 2; }
};

class MainActivity : public FragmentActivity {
public:
    MainActivity() : FragmentActivity(0, 0, -1, -1) {}

    void onCreate(Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        LayoutInflater::from(getContext())->inflate(R::layout::activity, this, true);

        setupDrawer();
        setupPager();
        setupTopBar();
        QuickControls::get().attachTo(*this);
    }

private:
    void setupDrawer() {
        // MenuItemAdapter's four entries; only exit is wired so far.
        std::vector<std::string> items = {"主题色", "定时关闭", "设置", "退出应用"};
        auto* menu = (ListView*) findViewById(R::id::id_lv_left_menu);
        if (menu) {
            auto* adapter = new MenuAdapter(getContext(), R::layout::design_drawer_item, 0);
            adapter->addAll(items);
            menu->setAdapter(adapter);
            menu->setOnItemClickListener([this](AdapterView&, View&, int position, long) {
                if (position == 3) { close(); return; }
                // TODO(remusic): theme picker / sleep timer / settings screens.
            });
        }
        // The hamburger opens the drawer (the original gestures on the toolbar).
        if (auto* toolbar = findViewById(R::id::toolbar))
            toolbar->setOnLongClickListener([this](View&) {
                if (auto* drawer = (ViewGroup*) findViewById(R::id::fd))
                    drawer->requestLayout();
                return true;
            });
    }

    // CardPickerDialog, lean: the 8-color palette over a scrim.
    void showThemePicker() {
        if (mThemeSheet) { removeView(mThemeSheet); mThemeSheet = nullptr; }
        auto* wrap = new FrameLayout(getContext());
        wrap->setBackgroundColor(0x88000000u);
        auto* panel = new LinearLayout(getContext());
        panel->setOrientation(LinearLayout::VERTICAL);
        panel->setBackgroundColor(0xFFFFFFFFu);
        auto* title = new TextView(getContext());
        title->setText("选择主题色");
        title->setTextSize(18);
        title->setTextColor(0xFF333333);
        title->setPadding(24, 18, 24, 12);
        panel->addView(title, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 56));
        for (auto& entry : ThemeStore::palette()) {
            auto* row = new TextView(getContext());
            row->setText(std::string("  ● ") + entry.first);
            row->setTextSize(16);
            row->setTextColor(0xFF333333);
            row->setPadding(24, 14, 24, 14);
            row->setClickable(true);
            const uint32_t color = entry.second;
            row->setOnClickListener([this, wrap, color](View&) {
                ThemeStore::get().setAccent(color);
                removeView(wrap);
                mThemeSheet = nullptr;
                recreate();
            });
            panel->addView(row, new LinearLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, 52));
        }
        wrap->addView(panel, new FrameLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT,
                Gravity::BOTTOM));
        addView(wrap, new FrameLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
        mThemeSheet = wrap;
    }

    void setupPager() {
        auto* pager = (ViewPager*) findViewById(R::id::main_viewpager);
        pager->setAdapter(new MainPagerAdapter(getSupportFragmentManager()));
        pager->setCurrentItem(1);   // open on the local page like the original default
        mPager = pager;
    }

    void setupTopBar() {
        if (auto* net = (ImageView*) findViewById(R::id::bar_net))
            net->setOnClickListener([this](View&) { mPager->setCurrentItem(0); });
        if (auto* music = (ImageView*) findViewById(R::id::bar_music))
            music->setOnClickListener([this](View&) { mPager->setCurrentItem(1); });
        if (auto* search = findViewById(R::id::bar_search))
            search->setOnClickListener([](View&) {
                Intent intent;
                intent.setClassName("cdroid.remusic", "LocalSearchActivity");
                App::getInstance().startActivity(intent);
            });
    }

    ViewPager* mPager = nullptr;
    FrameLayout* mThemeSheet = nullptr;
};
REGISTER_ACTIVITY(MainActivity);

} // namespace
