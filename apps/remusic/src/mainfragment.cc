#include "mainfragment.h"

#include <R.h>
#include <core/intent.h>
#include <widget/imageview.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>

#include "mediaplaybackservice.h"
#include "musicprovider.h"
#include "playliststore.h"

using namespace cdroid;
using namespace remusic;

namespace {

struct Row {
    enum Kind { MENU, HEADER, PLAYLIST } kind;
    std::string title;
    int count = 0;
    int iconRes = 0;
};

class HomeAdapter : public RecyclerView::Adapter {
public:
    explicit HomeAdapter(std::vector<Row> rows) : mRows(std::move(rows)) {}

    int getItemViewType(int position) override {
        return mRows.at(position).kind == Row::MENU ? 0
             : mRows.at(position).kind == Row::HEADER ? 1 : 2;
    }

    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override {
        // recycler_header.xml is a 270dp image header, not a section title —
        // the playlist row layout doubles as the section header.
        const int layout = viewType == 0 ? R::layout::fragment_main_item
                                          : R::layout::fragment_main_playlist_first_item;
        View* v = LayoutInflater::from(parent->getContext())->inflate(layout, parent, false);
        return new RecyclerView::ViewHolder(v);
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
        const Row& row = mRows.at(position);
        if (row.kind == Row::MENU) {
            auto* title = (TextView*) holder.itemView->findViewById(R::id::fragment_main_item_title);
            auto* count = (TextView*) holder.itemView->findViewById(R::id::fragment_main_item_count);
            auto* img = (ImageView*) holder.itemView->findViewById(R::id::fragment_main_item_img);
            if (title) title->setText(row.title);
            if (count) count->setText("(" + std::to_string(row.count) + ")");
            if (img) img->setImageResource(row.iconRes);
            holder.itemView->setOnClickListener([this, position](View&) {
                const Row& r = mRows.at(position);
                Intent intent;
                if (r.title == "本地音乐" || r.title == "我的歌手") {
                    intent.setClassName("cdroid.remusic", "TabActivity");
                } else if (r.title == "最近播放") {
                    intent.setClassName("cdroid.remusic", "RecentActivity");
                } else {
                    intent.setClassName("cdroid.remusic", "DownActivity");
                }
                App::getInstance().startActivity(intent);
            });
        } else if (row.kind == Row::HEADER) {
            // 创建的歌单 header: tap = 新建歌单 (AddPlaylistDialog, handled
            // by PlaylistActivity's NEW_PLAYLIST action with an input dialog).
            if (auto* tv = (TextView*) holder.itemView->findViewById(R::id::fragment_main_playlist_item_title))
                tv->setText(row.title);
            if (auto* sub = (TextView*) holder.itemView->findViewById(R::id::fragment_main_playlist_item_count))
                sub->setText("点击新建歌单");
            holder.itemView->setOnClickListener([](View&) {
                Intent intent;
                intent.setClassName("cdroid.remusic", "PlaylistActivity")
                      .setAction("com.wm.remusic.action.NEW_PLAYLIST");
                App::getInstance().startActivity(intent);
            });
        } else {
            if (auto* tv = (TextView*) holder.itemView->findViewById(R::id::fragment_main_playlist_item_title))
                tv->setText(row.title);
            if (auto* sub = (TextView*) holder.itemView->findViewById(R::id::fragment_main_playlist_item_count))
                sub->setText(std::to_string(row.count) + "首");
            std::string name = row.title;
            holder.itemView->setOnClickListener([name](View&) {
                Intent intent;
                intent.setClassName("cdroid.remusic", "PlaylistActivity")
                      .putExtra("playlist", name);
                App::getInstance().startActivity(intent);
            });
        }
    }

    int getItemCount() override { return (int)mRows.size(); }

private:
    std::vector<Row> mRows;
};

} // namespace

View* MainFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                 Bundle* /*savedInstanceState*/) {
    View* view = inflater->inflate(R::layout::fragment_main, container, false);
    auto* recyclerView = (RecyclerView*) view->findViewById(R::id::recyclerview);
    recyclerView->setLayoutManager(new LinearLayoutManager(getContext(), LinearLayout::VERTICAL, false));
    reloadAdapter();
    recyclerView->setAdapter(mAdapter);
    return view;
}

void MainFragment::onResume() {
    Fragment::onResume();
    reloadAdapter();
}

void MainFragment::reloadAdapter() {
    MusicProvider::get().scanIfNeeded();
    const int songs = (int)MusicProvider::get().queryMusic(MusicProvider::SORT_ORDER_A_Z).size();
    const int artists = (int)MusicProvider::get().queryArtist("").size();

    std::vector<Row> rows;
    rows.push_back({Row::MENU, "本地音乐", songs, R::drawable::music_icn_local});
    rows.push_back({Row::MENU, "最近播放",
            (int)MediaPlaybackService::recentSongIds().size(), R::drawable::music_icn_recent});
    rows.push_back({Row::MENU, "下载管理", 0, R::drawable::music_icn_dld});
    rows.push_back({Row::MENU, "我的歌手", artists, R::drawable::music_icn_artist});
    rows.push_back({Row::HEADER, "创建的歌单", 0, 0});
    for (auto& pl : PlaylistStore::get().getPlaylists())
        rows.push_back({Row::PLAYLIST, pl.name, (int)pl.songIds.size(), 0});
    mAdapter = new HomeAdapter(std::move(rows));
    if (getView() != nullptr) {
        auto* recyclerView = (RecyclerView*) getView()->findViewById(R::id::recyclerview);
        recyclerView->setAdapter(mAdapter);
        mAdapter->notifyDataSetChanged();
    }
}
