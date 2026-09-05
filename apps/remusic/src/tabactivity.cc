// Port of com.wm.remusic.activity.TabActivity + fragment.TabPagerFragment —
// the 本地音乐 four-tab screen (单曲/歌手/专辑/文件夹). 单曲 is the full song
// list; the other three browse groups and open a detail screen (the original
// pushes ArtistDetailActivity etc. — one DetailActivity serves all here).
#include <cdroid.h>
#include <R.h>
#include <core/activityfactory.h>
#include <core/intent.h>
#include <fragment/fragment.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentpageradapter.h>
#include <widget/adapter.h>
#include <widget/linearlayout.h>
#include <widget/listview.h>
#include <widget/textview.h>
#include <widget/viewpager.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/tablayout/tablayout.h>

#include "musicfragment.h"
#include "musicplayer.h"
#include "musicprovider.h"
#include "themestore.h"
#include "quickcontrols.h"

using namespace cdroid;
using namespace remusic;

namespace {

class GroupListFragment : public Fragment {
public:
    enum Kind { ARTIST, ALBUM, FOLDER };
    explicit GroupListFragment(Kind kind) : mKind(kind) {}

    View* onCreateView(LayoutInflater* inflater, ViewGroup* /*container*/, Bundle*) override {
        auto* recyclerView = new RecyclerView(inflater->getContext());
        recyclerView->setLayoutManager(new LinearLayoutManager(inflater->getContext()));
        recyclerView->setAdapter(new GroupAdapter(this));
        return recyclerView;
    }

    void onResume() override {
        Fragment::onResume();
        mEntries.clear();
        MusicProvider::get().scanIfNeeded();
        if (mKind == ARTIST) {
            std::map<long, int> counts;
            for (auto& s : MusicProvider::get().queryMusic("")) counts[s.artistId]++;
            for (auto& kv : MusicProvider::get().queryArtist(""))
                mEntries.push_back({kv.second, kv.second, counts[kv.first]});
        } else if (mKind == ALBUM) {
            std::map<long, int> counts;
            for (auto& s : MusicProvider::get().queryMusic("")) counts[s.albumId]++;
            for (auto& kv : MusicProvider::get().queryAlbums(""))
                mEntries.push_back({kv.second, kv.second, counts[kv.first]});
        } else {
            for (auto& kv : MusicProvider::get().queryFolder()) {
                const size_t slash = kv.first.find_last_of('/');
                const std::string name = slash == std::string::npos ? kv.first
                                                                    : kv.first.substr(slash + 1);
                mEntries.push_back({name.empty() ? kv.first : name, kv.first, kv.second});
            }
        }
        if (getView() != nullptr && ((RecyclerView*) getView())->getAdapter())
            ((RecyclerView*) getView())->getAdapter()->notifyDataSetChanged();
    }

    std::string modeName() const {
        return mKind == ARTIST ? "artist" : mKind == ALBUM ? "album" : "folder";
    }

private:
    struct Entry { std::string name; std::string detailKey; int count; };

    class GroupAdapter : public RecyclerView::Adapter {
    public:
        explicit GroupAdapter(GroupListFragment* frag) : mFrag(frag) {}
        RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int /*viewType*/) override {
            View* v = LayoutInflater::from(parent->getContext())
                    ->inflate(R::layout::fragment_musci_common_item, parent, false);
            return new RecyclerView::ViewHolder(v);
        }
        void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
            const Entry& e = mFrag->mEntries.at(position);
            if (auto* t = (TextView*) holder.itemView->findViewById(R::id::viewpager_list_toptext))
                t->setText(e.name);
            if (auto* b = (TextView*) holder.itemView->findViewById(R::id::viewpager_list_bottom_text))
                b->setText(std::to_string(e.count) + "首");
            const std::string key = e.detailKey;
            GroupListFragment::Kind kind = mFrag->mKind;
            holder.itemView->setOnClickListener([key, kind](View&) {
                Intent intent;
                intent.setClassName("cdroid.remusic", "DetailActivity")
                      .putExtra("mode", kind == GroupListFragment::ARTIST ? "artist"
                                : kind == GroupListFragment::ALBUM ? "album" : "folder")
                      .putExtra("key", key)
                      .putExtra("name", key);
                App::getInstance().startActivity(intent);
            });
        }
        int getItemCount() override { return (int)mFrag->mEntries.size(); }
    private:
        GroupListFragment* mFrag;
    };
    friend class GroupAdapter;

    Kind mKind;
    std::vector<Entry> mEntries;
};

class TabPagerAdapter : public FragmentPagerAdapter {
public:
    TabPagerAdapter(FragmentManager* fm) : FragmentPagerAdapter(fm) {}
    Fragment* getItem(int position) override {
        switch (position) {
            case 0: return new MusicFragment();
            case 1: return new GroupListFragment(GroupListFragment::ARTIST);
            case 2: return new GroupListFragment(GroupListFragment::ALBUM);
            default: return new GroupListFragment(GroupListFragment::FOLDER);
        }
    }
    int getCount() override { return 4; }
    std::string getPageTitle(int position) override {
        const char* titles[] = {"单曲", "歌手", "专辑", "文件夹"};
        return titles[position];
    }
};

class TabActivity : public FragmentActivity {
public:
    TabActivity() : FragmentActivity(0, 0, -1, -1) {}

    void onCreate(Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        // tabs/viewpager live in fragment_tab (activity_tab only hosts a container).
        LayoutInflater::from(getContext())->inflate(R::layout::fragment_tab, this, true);
        MusicProvider::get().scanIfNeeded();

        auto* tabs = (TabLayout*) findViewById(R::id::tabs);
        auto* pager = (ViewPager*) findViewById(R::id::viewpager);
        auto* adapter = new TabPagerAdapter(getSupportFragmentManager());
        pager->setAdapter(adapter);
        tabs->setupWithViewPager(pager);

        auto* text = (TextView*) findViewById(R::id::toolbar_text);
        if (text) text->setText("本地音乐");
        if (auto* back = findViewById(R::id::toolbar))
            back->setOnClickListener([this](View&) { close(); });

        QuickControls::get().attachTo(*this);
    }
};
REGISTER_ACTIVITY(TabActivity);

// Artist/Album/Folder detail: the group's songs, tap to play (the original
// spreads these over ArtistDetailActivity / AlbumDetailActivity /
// FolderFragment).
class DetailActivity : public FragmentActivity {
public:
    DetailActivity() : FragmentActivity(0, 0, -1, -1) {}

    void onCreate(Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        const std::string mode = getIntent().getStringExtra("mode");
        const std::string key = getIntent().getStringExtra("key");
        auto* root = new LinearLayout(getContext());
        root->setOrientation(LinearLayout::VERTICAL);
        auto* header = new TextView(getContext());
        header->setText(getIntent().getStringExtra("name"));
        header->setTextSize(20);
        header->setTextColor(0xFFFFFFFF);
        header->setBackgroundColor(ThemeStore::get().accent());
        header->setPadding(20, 16, 20, 16);
        header->setClickable(true);
        header->setOnClickListener([this](View&) { close(); });
        root->addView(header, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 56));

        std::vector<MusicInfo> songs;
        if (mode == "artist") songs = MusicProvider::get().songsByArtist(atol(key.c_str()));
        else if (mode == "album") songs = MusicProvider::get().songsByAlbum(atol(key.c_str()));
        else songs = MusicProvider::get().songsByFolder(key);

        std::vector<std::string> rows;
        for (auto& s : songs) rows.push_back(s.musicName + " - " + s.artist);
        if (rows.empty()) rows.push_back("暂无歌曲");
        auto* list = new ListView(getContext());
        auto* adapter = new ArrayAdapter<std::string>(
                getContext(), R::layout::design_drawer_item, 0);
        adapter->addAll(rows);
        list->setAdapter(adapter);
        list->setOnItemClickListener([this, songs](AdapterView&, View&, int position, long) {
            if (position >= (int)songs.size()) return;
            std::map<long, MusicInfo> infos;
            std::vector<long> ids;
            for (auto& s : songs) { infos[s.songId] = s; ids.push_back(s.songId); }
            MusicPlayer::playAll(infos, ids, position, false);
        });
        root->addView(list, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
        addView(root);
    }
};
REGISTER_ACTIVITY(DetailActivity);

} // namespace
