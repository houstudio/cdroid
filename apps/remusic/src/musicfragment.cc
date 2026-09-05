#include "musicfragment.h"

#include <R.h>
#include <core/intent.h>
#include <widget/framelayout.h>
#include <widget/imageview.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/recyclerview.h>

#include "musicplayer.h"
#include "musicprovider.h"

using namespace cdroid;
using namespace remusic;

namespace {

class SongAdapter : public RecyclerView::Adapter {
public:
    explicit SongAdapter(MusicFragment* frag, std::vector<MusicInfo> songs)
            : mFrag(frag), mSongs(std::move(songs)) {}

    int getItemViewType(int position) override { return position == 0 ? 0 : 1; }

    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override {
        // position 0 = the 播放全部 header (common_item), else a song row.
        View* v = LayoutInflater::from(parent->getContext())->inflate(
                viewType == 0 ? R::layout::common_item : R::layout::fragment_musci_common_item,
                parent, false);
        return new RecyclerView::ViewHolder(v);
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
        if (position == 0) {
            // Header: "(N)" + 播放全部 — clicking plays the whole queue.
            auto* top = (TextView*) holder.itemView->findViewById(R::id::viewpager_list_toptext);
            if (top) top->setText("(" + std::to_string(mSongs.size()) + ")");
            holder.itemView->setOnClickListener([this](View&) {
                std::map<long, MusicInfo> infos;
                std::vector<long> list;
                for (auto& s : mSongs) { infos[s.songId] = s; list.push_back(s.songId); }
                MusicPlayer::playAll(infos, list, 0, false);
            });
            return;
        }
        const MusicInfo& song = mSongs.at(position - 1);
        auto* title = (TextView*) holder.itemView->findViewById(R::id::viewpager_list_toptext);
        auto* sub = (TextView*) holder.itemView->findViewById(R::id::viewpager_list_bottom_text);
        if (title) title->setText(song.musicName);
        char dur[16];
        snprintf(dur, sizeof(dur), "%02d:%02d", song.duration / 60000, (song.duration / 1000) % 60);
        if (sub) sub->setText(song.artist + " - " + song.albumName + "  " + dur);
        if (auto* more = (ImageView*) holder.itemView->findViewById(R::id::viewpager_list_button))
            more->setOnClickListener([this, song](View&) { showMoreSheet(song); });
        holder.itemView->setOnClickListener([this, position](View&) {
            const MusicInfo& s = mSongs.at(position - 1);
            std::map<long, MusicInfo> infos;
            std::vector<long> list;
            for (auto& x : mSongs) { infos[x.songId] = x; list.push_back(x.songId); }
            MusicPlayer::playAll(infos, list, position - 1, false);
        });
    }

    int getItemCount() override { return (int)mSongs.size() + 1; }

private:
    // MoreFragment, lean: the overflow sheet (加入歌单 / 查看歌手 / 从队列移除).
    void showMoreSheet(const MusicInfo& song) {
        Context* ctx = mFrag->getContext();
        auto* wrap = new FrameLayout(ctx);
        wrap->setBackgroundColor(0x88000000u);
        auto* panel = new LinearLayout(ctx);
        panel->setOrientation(LinearLayout::VERTICAL);
        panel->setBackgroundColor(0xFFF5F5F5u);
        auto add = [ctx, panel, &song](const char* label, auto fn) {
            auto* row = new TextView(ctx);
            row->setText(label);
            row->setTextSize(17);
            row->setTextColor(0xFF333333);
            row->setPadding(28, 18, 28, 18);
            row->setClickable(true);
            row->setOnClickListener([fn, song](View&) { fn(song); });
            panel->addView(row, new LinearLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, 58));
        };
        add("加入歌单", [](const MusicInfo& s) {
            Intent intent;
            intent.setClassName("cdroid.remusic", "PlaylistActivity")
                  .setAction("com.wm.remusic.action.ADD_TO")
                  .putExtra("songId", (long)s.songId);
            App::getInstance().startActivity(intent);
        });
        add("查看歌手", [](const MusicInfo& s) {
            Intent intent;
            intent.setClassName("cdroid.remusic", "DetailActivity")
                  .putExtra("mode", "artist")
                  .putExtra("key", std::to_string(s.artistId))
                  .putExtra("name", s.artist);
            App::getInstance().startActivity(intent);
        });
        add("查看专辑", [](const MusicInfo& s) {
            Intent intent;
            intent.setClassName("cdroid.remusic", "DetailActivity")
                  .putExtra("mode", "album")
                  .putExtra("key", std::to_string(s.albumId))
                  .putExtra("name", s.albumName);
            App::getInstance().startActivity(intent);
        });
        add("从队列移除", [](const MusicInfo& s) {
            MusicPlayer::removeTrack(s.songId);
        });
        auto* dismiss = [ctx, wrap]() -> TextView* {
            auto* t = new TextView(ctx);
            t->setText("取消");
            t->setTextSize(17);
            t->setTextColor(0xFF888888);
            t->setGravity(Gravity::CENTER);
            t->setPadding(28, 16, 28, 16);
            return t;
        }();
        dismiss->setOnClickListener([wrap](View&) {
            if (wrap->getParent() != nullptr)
                ((ViewGroup*) wrap->getParent())->removeView(wrap);
        });
        panel->addView(dismiss, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 56));
        wrap->setOnClickListener([wrap](View&) {
            if (wrap->getParent() != nullptr)
                ((ViewGroup*) wrap->getParent())->removeView(wrap);
        });
        wrap->addView(panel, new FrameLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT,
                Gravity::BOTTOM));
        if (mFrag->getView() != nullptr && mFrag->getView()->getParent() != nullptr)
            ((ViewGroup*) mFrag->getView()->getParent())->addView(wrap, new FrameLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
    }

    MusicFragment* mFrag;
    std::vector<MusicInfo> mSongs;
};

} // namespace

View* MusicFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                  Bundle* /*savedInstanceState*/) {
    View* view = inflater->inflate(R::layout::recylerview, container, false);
    auto* recyclerView = (RecyclerView*) view->findViewById(R::id::recyclerview);
    recyclerView->setLayoutManager(new LinearLayoutManager(getContext()));
    auto songs = MusicProvider::get().queryMusic(MusicProvider::SORT_ORDER_A_Z);
    recyclerView->setAdapter(new SongAdapter(this, std::move(songs)));
    if (auto* sidebar = view->findViewById(R::id::sidebar)) sidebar->setVisibility(View::GONE);
    return view;
}
