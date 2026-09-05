// Navigation endpoints reachable from the local home: 最近播放 / 下载管理 /
// 歌单. Recent shows the play-history tail from the playback service;
// Down/Playlist are honest offline placeholders until their stores land.
#include <cdroid.h>
#include <R.h>
#include <text/String.h>
#include <core/activityfactory.h>
#include <fragment/fragmentactivity.h>
#include <widget/adapter.h>
#include <widget/edittext.h>
#include <widget/framelayout.h>
#include <widget/linearlayout.h>
#include <widget/listview.h>
#include <widget/textview.h>

#include "mediaplaybackservice.h"
#include "musicplayer.h"
#include "musicprovider.h"
#include "themestore.h"
#include "playliststore.h"

using namespace cdroid;
using namespace remusic;

namespace {

class HeaderActivity : public FragmentActivity {
public:
    HeaderActivity(const char* title) : FragmentActivity(0, 0, -1, -1), mTitle(title) {}

    void onCreate(Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        setBackgroundColor(0xFFF2F2F2);
        auto* root = new LinearLayout(getContext());
        root->setOrientation(LinearLayout::VERTICAL);
        auto* header = new TextView(getContext());
        header->setText(mTitle);
        header->setTextSize(22);
        header->setTextColor(0xFFFFFFFF);
        header->setGravity(Gravity::CENTER_VERTICAL);
        header->setBackgroundColor(ThemeStore::get().accent());
        header->setPadding(20, 16, 20, 16);
        header->setClickable(true);
        header->setOnClickListener([this](View&) { close(); });
        root->addView(header, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 56));
        mList = new ListView(getContext());
        root->addView(mList, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
        addView(root);
        onContentReady();
    }

protected:
    virtual void onContentReady() {}
    ListView* mList = nullptr;

private:
    const char* mTitle;
};

class RecentActivity : public HeaderActivity {
public:
    RecentActivity() : HeaderActivity("最近播放") {}

    void onContentReady() override {
        auto* menu = mList;
        // TopTracksLoader.recentSongs: most-recently-played ids from the
        // store, resolved through the provider.
        mSongs.clear();
        for (long id : MediaPlaybackService::recentSongIds()) {
            if (const auto* info = MusicProvider::get().find(id)) mSongs.push_back(*info);
        }
        std::vector<std::string> rows;
        for (auto& s : mSongs) rows.push_back(s.artist + " - " + s.musicName);
        if (rows.empty()) rows.push_back("暂无播放记录");
        auto* adapter = new ArrayAdapter<std::string>(
                getContext(), R::layout::design_drawer_item, 0);
        adapter->addAll(rows);
        menu->setAdapter(adapter);
        menu->setOnItemClickListener([this](AdapterView&, View&, int position, long) {
            if (position >= (int)mSongs.size()) return;
            std::map<long, MusicInfo> infos;
            std::vector<long> list;
            for (auto& s : mSongs) { infos[s.songId] = s; list.push_back(s.songId); }
            MusicPlayer::playAll(infos, list, position, false);
        });
    }
    std::vector<MusicInfo> mSongs;
};
REGISTER_ACTIVITY(RecentActivity);

class DownActivity : public HeaderActivity {
public:
    DownActivity() : HeaderActivity("下载管理") {}
    void onContentReady() override {
        auto* menu = mList;
        std::vector<std::string> rows = {"在线下载已离线"};
        auto* adapter = new ArrayAdapter<std::string>(
                getContext(), R::layout::design_drawer_item, 0);
        adapter->addAll(rows);
        menu->setAdapter(adapter);
    }
};
REGISTER_ACTIVITY(DownActivity);

// PlaylistActivity serves three shapes of the original flow:
//  - playlist detail (extra "playlist" = name): the playlist's songs, tap to play
//  - NEW_PLAYLIST action: the AddPlaylistDialog (name input)
//  - ADD_TO action + extra "songId": PlaylistSelectActivity (pick a target list)
class PlaylistActivity : public HeaderActivity {
public:
    PlaylistActivity() : HeaderActivity("歌单") {}

    void onContentReady() override {
        // HeaderActivity::onCreate (already run) invokes onContentReady, so
        // the intent is only safe to read here — a subclass onCreate body
        // would run too late.
        mAction = getIntent().getAction();
        mPlaylist = getIntent().getStringExtra("playlist");
        mSongId = getIntent().getLongExtra("songId", -1);
        if (mAction == "com.wm.remusic.action.NEW_PLAYLIST") {
            mList->setVisibility(View::GONE);
            showNewPlaylistDialog();
            return;
        }
        if (mAction == "com.wm.remusic.action.ADD_TO") {
            bindPlaylistPicker(mSongId);
            return;
        }
        // detail: the playlist's songs, resolved through the provider.
        mSongs.clear();
        for (const auto& pl : PlaylistStore::get().getPlaylists()) {
            if (pl.name != mPlaylist) continue;
            for (long id : pl.songIds)
                if (const auto* info = MusicProvider::get().find(id)) mSongs.push_back(*info);
            break;
        }
        std::vector<std::string> rows;
        for (auto& s : mSongs) rows.push_back(s.artist + " - " + s.musicName);
        if (rows.empty()) rows.push_back(mPlaylist.empty() ? "暂无歌单" : "歌单为空,去单曲页加歌");
        auto* adapter = new ArrayAdapter<std::string>(
                getContext(), R::layout::design_drawer_item, 0);
        adapter->addAll(rows);
        mList->setAdapter(adapter);
        mList->setOnItemClickListener([this](AdapterView&, View&, int position, long) {
            if (position >= (int)mSongs.size()) return;
            std::map<long, MusicInfo> infos;
            std::vector<long> list;
            for (auto& s : mSongs) { infos[s.songId] = s; list.push_back(s.songId); }
            MusicPlayer::playAll(infos, list, position, false);
        });
    }

private:

    void bindPlaylistPicker(long songId) {
        std::vector<std::string> rows;
        auto lists = PlaylistStore::get().getPlaylists();
        for (auto& pl : lists) rows.push_back(pl.name + " (" + std::to_string(pl.songIds.size()) + "首)");
        rows.push_back("+ 新建歌单");
        auto* adapter = new ArrayAdapter<std::string>(
                getContext(), R::layout::design_drawer_item, 0);
        adapter->addAll(rows);
        mList->setAdapter(adapter);
        mList->setOnItemClickListener([this, lists, songId](AdapterView&, View&, int position, long) {
            if (position < (int)lists.size()) {
                PlaylistStore::get().addSong(lists[position].name, songId);
            } else {
                showNewPlaylistDialog([this, songId](const std::string& name) {
                    PlaylistStore::get().addSong(name, songId);
                });
                return;
            }
            close();
        });
    }

    void showNewPlaylistDialog(std::function<void(const std::string&)> onDone = nullptr) {
        // AddPlaylistDialog, lean: a titled single-entry panel on this window.
        auto* panel = new LinearLayout(getContext());
        panel->setOrientation(LinearLayout::VERTICAL);
        panel->setBackgroundColor(0xFFFFFFFFu);
        auto* title = new TextView(getContext());
        title->setText("新建歌单");
        title->setTextSize(20);
        title->setTextColor(0xFF333333);
        title->setPadding(24, 20, 24, 12);
        auto* input = new EditText(getContext());
        input->setText("新建歌单" + std::to_string(PlaylistStore::get().getPlaylists().size() + 1));
        input->setPadding(24, 8, 24, 16);
        auto* ok = new TextView(getContext());
        ok->setText("创建");
        ok->setTextSize(18);
        ok->setTextColor(ThemeStore::get().accent());
        ok->setGravity(Gravity::CENTER);
        ok->setPadding(24, 16, 24, 16);
        ok->setOnClickListener([this, input, onDone](View&) {
            String* value = input->getText().toString();
            const std::string name = value ? value->str() : std::string();
            delete value;
            PlaylistStore::get().createPlaylist(name);
            if (onDone) onDone(name);
            else close();
        });
        panel->addView(title, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 64));
        panel->addView(input, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 96));
        panel->addView(ok, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 64));
        auto* wrap = new FrameLayout(getContext());
        wrap->setBackgroundColor(0x88000000u);
        wrap->setOnClickListener([this](View&) { close(); });
        wrap->addView(panel, new FrameLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT,
                Gravity::CENTER));
        addView(wrap, new FrameLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
    }

    std::string mAction;
    std::string mPlaylist;
    long mSongId = -1;
    std::vector<MusicInfo> mSongs;
};
REGISTER_ACTIVITY(PlaylistActivity);

} // namespace
