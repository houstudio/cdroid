// Port of com.wm.remusic.activity.MainActivity — DrawerLayout + top tab bar
// (在线 stub / 本地) + CustomViewPager + bottom quick-controls. The online
// page shows the offline notice (Baidu-ting backend is gone); the local page
// hosts MainFragment exactly like the original page 1.
#include <cdroid.h>
#include <functional>
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
#include "musicplayer.h"
#include "quickcontrols.h"
#ifdef REMUSIC_ONLINE
#include "radiobrowser.h"
#include <widget/horizontalscrollview.h>
#endif
#include "themestore.h"

using namespace cdroid;
using namespace remusic;

namespace {

// MenuItemAdapter, local cut: the drawer rows are bare TextViews, which is
// exactly ArrayAdapter's mFieldId==0 shape.
using MenuAdapter = ArrayAdapter<std::string>;

#ifdef REMUSIC_ONLINE
// The original's page-0 net experience, rebuilt on the key-free
// radio-browser.info directory: genre chips over a station list; tapping a
// station plays its live stream through the same queue/player pipeline
// (FFmpeg opens http(s) stream URLs natively; duration 0 = live, the tick
// never auto-advances it).
class RadioFragment : public Fragment {
public:
    View* onCreateView(LayoutInflater* inflater, ViewGroup* /*container*/, Bundle*) override {
        auto* ctx = inflater->getContext();
        auto* root = new LinearLayout(ctx);
        root->setOrientation(LinearLayout::VERTICAL);
        root->setBackgroundColor(0xFFF2F2F2);

        auto* chipsScroll = new HorizontalScrollView(ctx);
        mChips = new LinearLayout(ctx);
        mChips->setOrientation(LinearLayout::HORIZONTAL);
        mChips->setPadding(8, 8, 8, 8);
        for (const char* tag : {"chinese", "pop", "rock", "classical", "jazz",
                                "dance", "news", "oldies", "80s", "electronic"}) {
            auto* chip = new TextView(ctx);
            chip->setText(tag);
            chip->setTextSize(15);
            chip->setPadding(28, 12, 28, 12);
            chip->setClickable(true);
            std::string t = tag;
            chip->setOnClickListener([this, t](View&) { selectTag(t); });
            mChips->addView(chip, new LinearLayout::LayoutParams(
                    ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT));
            mChipViews.push_back(chip);
        }
        chipsScroll->addView(mChips);
        root->addView(chipsScroll, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT));

        mStatus = new TextView(ctx);
        mStatus->setText("加载电台中…");
        mStatus->setTextSize(14);
        mStatus->setTextColor(0xFF888888);
        mStatus->setPadding(20, 8, 20, 8);
        root->addView(mStatus, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT));

        mList = new ListView(ctx);
        root->addView(mList, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
        selectTag("chinese");
        return root;
    }

private:
    void selectTag(const std::string& tag) {
        mTag = tag;
        for (size_t i = 0; i < mChipViews.size(); i++) {
            const bool sel = mChipTags(i) == tag;
            mChipViews[i]->setBackgroundColor(sel ? 0xFFD43C33 : 0xFFDDDDDD);
            mChipViews[i]->setTextColor(sel ? 0xFFFFFFFF : 0xFF444444);
        }
        mStatus->setText("加载 " + tag + " 电台中…");
        RadioBrowser::searchByTag(tag, [this](std::vector<RadioStation> stations) {
            if (getView() == nullptr) return;
            mStations = std::move(stations);
            bindList();
        });
    }

    std::string mChipTags(size_t i) const {
        const char* tags[] = {"chinese", "pop", "rock", "classical", "jazz",
                              "dance", "news", "oldies", "80s", "electronic"};
        return tags[i];
    }

    void bindList() {
        std::vector<std::string> rows;
        for (auto& st : mStations) {
            std::string row = st.name;
            if (st.bitrate > 0) row += "  " + std::to_string(st.bitrate) + "kbps";
            if (!st.country.empty()) row += "  ·  " + st.country;
            rows.push_back(row);
        }
        if (rows.empty()) {
            rows.push_back("加载失败或无电台(检查网络)");
            mStatus->setText("无结果");
        } else {
            mStatus->setText(std::to_string(rows.size()) + " 个电台 · 点击播放直播流");
        }
        auto* adapter = new ArrayAdapter<std::string>(
                getContext(), R::layout::design_drawer_item, 0);
        adapter->addAll(rows);
        mList->setAdapter(adapter);
        mList->setOnItemClickListener([this](AdapterView&, View&, int position, long) {
            if (position >= (int)mStations.size()) return;
            const RadioStation& st = mStations[position];
            MusicInfo info;
            info.songId = std::hash<std::string>{}(st.url) & 0x7fffffff;
            info.musicName = st.name;
            info.artist = "电台 · " + mTag;
            info.albumName = st.country.empty() ? mTag : st.country;
            info.data = st.url;
            info.islocal = true;
            info.duration = 0;   // live: the tick never auto-advances
            std::map<long, MusicInfo> infos{{info.songId, info}};
            std::vector<long> ids{info.songId};
            MusicPlayer::playAll(infos, ids, 0, false);
        });
    }

    LinearLayout* mChips = nullptr;
    TextView* mStatus = nullptr;
    ListView* mList = nullptr;
    std::vector<TextView*> mChipViews;
    std::vector<RadioStation> mStations;
    std::string mTag;
};
#else
class OnlineStubFragment : public Fragment {
public:
    View* onCreateView(LayoutInflater* inflater, ViewGroup* /*container*/, Bundle*) override {
        auto* tv = new TextView(inflater->getContext());
        tv->setText("在线服务未编入\n(需要 curl+jsoncpp)");
        tv->setTextSize(18);
        tv->setGravity(Gravity::CENTER);
        return tv;
    }
};
#endif

class MainPagerAdapter : public FragmentPagerAdapter {
public:
    MainPagerAdapter(FragmentManager* fm) : FragmentPagerAdapter(fm) {}
    Fragment* getItem(int position) override {
#ifdef REMUSIC_ONLINE
        return position == 0 ? (Fragment*) new RadioFragment() : (Fragment*) new MainFragment();
#else
        return position == 0 ? (Fragment*) new OnlineStubFragment() : (Fragment*) new MainFragment();
#endif
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
