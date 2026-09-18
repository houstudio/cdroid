/*********************************************************************************
 * Port of com.android.music.TrackBrowserActivity — the Songs tab and every
 * drilled-in track list (album / artist / playlist / "nowplaying" queue).
 * The original binds a MediaStore cursor (NowPlayingCursor for the queue);
 * here the row set is a std::vector<Track> resolved at onCreate from the
 * intent extras — bind logic (title/artist/duration/now-playing indicator)
 * follows TrackListAdapter verbatim. Drag-to-reorder is NOT ported
 * (TouchInterceptor is a plain ListView — see touchinterceptor.h).
 *********************************************************************************/
#include <cstdlib>

#include <R.h>
#include <core/activityfactory.h>
#include <core/app.h>
#include <core/context.h>
#include <core/intent.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <porting/cdlog.h>
#include <view/layoutinflater.h>
#include <widget/cdwindow.h>
#include <widget/imageview.h>
#include <widget/internal_R.h>
#include <widget/listview.h>
#include <widget/textview.h>

#include <mediaplaybackservice.h>
#include <musicdb.h>
#include <musicutils.h>

using namespace ::music;

namespace cdroid {
namespace music {

class TrackBrowserActivity : public Window {
private:
    // TrackListAdapter.ViewHolder. `owner` guards against recycled rows whose
    // tag still points into a previous adapter instance (the RecycleBin keeps
    // old views around; a stale tag would be use-after-free).
    struct ViewHolder {
        void* owner = nullptr;
        TextView* line1;
        TextView* line2;
        TextView* duration;
        ImageView* play_indicator;
        ImageView* icon;
    };

    class TrackListAdapter : public Adapter {
    public:
        TrackListAdapter(Context* ctx, std::vector<Track> tracks, bool isNowPlaying,
                bool disableNowPlayingIndicator)
            : mContext(ctx), mTracks(std::move(tracks)), mIsNowPlaying(isNowPlaying),
              mDisableNowPlayingIndicator(disableNowPlayingIndicator) {}
        ~TrackListAdapter() override {
            for (ViewHolder* h : mHolders) delete h;   // View doesn't own its tag
        }

        int getCount() const override { return (int)mTracks.size(); }
        void* getItem(int position) const override {
            return (void*)(uintptr_t)mTracks[position].id;
        }
        long getItemId(int position) const override { return mTracks[position].id; }
        View* getView(int position, View* convertView, ViewGroup* parent) override;

    private:
        Context* mContext;
        std::vector<Track> mTracks;
        bool mIsNowPlaying;
        bool mDisableNowPlayingIndicator;
        std::vector<ViewHolder*> mHolders;
    };

    std::vector<Track> mTracks;
    TrackListAdapter* mAdapter = nullptr;
    ListView* mList = nullptr;
    std::string mPlaylist;      // playlist name or "nowplaying"; empty = library query
    long mAlbumId = -1;
    long mArtistId = -1;

public:
    TrackBrowserActivity() : Window(0, 0, -1, -1) {}

    ~TrackBrowserActivity()override{
        delete mAdapter;
    }
protected:
    void onCreate(Bundle*) override {
        Intent intent = getIntent();
        mPlaylist = intent.getStringExtra("playlist");
        const std::string album = intent.getStringExtra("album");
        const std::string artist = intent.getStringExtra("artist");
        if (!album.empty()) mAlbumId = strtoll(album.c_str(), nullptr, 10);
        if (!artist.empty()) mArtistId = strtoll(artist.c_str(), nullptr, 10);

        setBackgroundColor(0xff000000);   // opaque backdrop (see MusicBrowserActivity)
        LayoutInflater::from(getContext())
                ->inflate(R::layout::media_picker_activity, this, true);
        // Songs tab chrome only when entered as a tab (drilled-in lists hide it,
        // like the original updateButtonBar "withtabs" logic)
        if (!updateButtonBar(*this, R::id::songtab)) addBackButton(*this);

        mList = dynamic_cast<ListView*>(findViewById(internal::R::id::list));
        resolveTracks();
        mAdapter = new TrackListAdapter(getContext(), mTracks,
                mPlaylist == "nowplaying", mPlaylist.empty());
        mList->setAdapter(mAdapter);

        mList->setOnItemClickListener([this](AdapterView&, View&, int position, long) {
            MediaPlaybackService& s = MediaPlaybackService::getInstance();
            if (mPlaylist == "nowplaying") {
                // original: setQueuePosition + play, staying on this screen
                std::vector<long> queue = s.getQueue();
                s.open(queue, position);
                s.play();
                mAdapter->notifyDataSetChanged();
            } else {
                std::vector<long> ids;
                for (const Track& t : mTracks) ids.push_back(t.id);
                playAll(*this, ids, position, false);
            }
        });

        MediaPlaybackService::getInstance().addListener(this,
                [this](const std::string&) {
                    if (mList != nullptr) mAdapter->notifyDataSetChanged();
                    updateNowPlaying(*this);
                });
    }

    void onResume() override {
        Window::onResume();
        updateNowPlaying(*this);
    }

    void onDestroy() override {
        MediaPlaybackService::getInstance().removeListener(this);
        Window::onDestroy();
    }

    bool onCreateOptionsMenu(Menu& menu) override {
        if (mPlaylist == "nowplaying") {
            menu.add(0, Defs::SAVE_AS_PLAYLIST, 0,
                    getContext()->getString(R::string::save_as_playlist));
            menu.add(0, Defs::CLEAR_PLAYLIST, 0,
                    getContext()->getString(R::string::clear_playlist))
                    ->setIcon(R::drawable::ic_menu_clear_playlist);
        } else {
            menu.add(0, Defs::PARTY_SHUFFLE, 0,
                    getContext()->getString(R::string::party_shuffle));
            menu.add(0, Defs::SHUFFLE_ALL, 0,
                    getContext()->getString(R::string::shuffle_all))
                    ->setIcon(R::drawable::ic_menu_shuffle);
        }
        return true;
    }

    bool onOptionsItemSelected(MenuItem& item) override {
        switch (item.getItemId()) {
        case Defs::PARTY_SHUFFLE:
            togglePartyShuffle();
            return true;
        case Defs::SHUFFLE_ALL:
            shuffleAll(*this);
            return true;
        case Defs::CLEAR_PLAYLIST: {
            // original clearQueue(): removeTracks(0, MAX)
            MediaPlaybackService::getInstance().removeTracks(0, INT_MAX);
            finishListReload();
            return true;
        }
        }
        return Window::onOptionsItemSelected(item);
    }

private:
    void resolveTracks() {
        MusicDB& db = MusicDB::get();
        if (mPlaylist == "nowplaying") {
            for (long id : MediaPlaybackService::getInstance().getQueue())
                if (const Track* t = db.track(id)) mTracks.push_back(*t);
        } else if (!mPlaylist.empty()) {
            long plid = strtoll(mPlaylist.c_str(), nullptr, 10);
            mTracks = db.tracksForPlaylist(plid);
        } else if (mAlbumId >= 0) {
            mTracks = db.tracksForAlbum(mAlbumId);
        } else if (mArtistId >= 0) {
            mTracks = db.tracksForArtist(mArtistId);
        } else {
            mTracks = db.tracks();
        }
    }

    void finishListReload() {
        mTracks.clear();
        resolveTracks();
        delete mAdapter;
        mAdapter = new TrackListAdapter(getContext(), mTracks,
                mPlaylist == "nowplaying", mPlaylist.empty());
        mList->setAdapter(mAdapter);
    }
};

View* TrackBrowserActivity::TrackListAdapter::getView(int position, View* convertView,
        ViewGroup* parent) {
    View* v = convertView;
    if (v == nullptr) {
        v = LayoutInflater::from(mContext)->inflate(R::layout::track_list_item, parent, false);
    }
    // (re)bind the holder: a recycled row may carry a stale tag from a previous
    // adapter instance — the owner check rebuilds instead of dereferencing it
    ViewHolder* vh = (ViewHolder*)v->getTag();
    if (vh == nullptr || vh->owner != this) {
        vh = new ViewHolder();
        vh->owner = this;
        vh->line1 = dynamic_cast<TextView*>(v->findViewById(R::id::line1));
        vh->line2 = dynamic_cast<TextView*>(v->findViewById(R::id::line2));
        vh->duration = dynamic_cast<TextView*>(v->findViewById(R::id::duration));
        vh->play_indicator = dynamic_cast<ImageView*>(v->findViewById(R::id::play_indicator));
        vh->icon = dynamic_cast<ImageView*>(v->findViewById(R::id::icon));
        vh->icon->setPadding(0, 0, 1, 0);
        v->setTag(vh);
        mHolders.push_back(vh);
    }

    const Track& track = mTracks[position];
    MusicDB& db = MusicDB::get();

    // bindView: title / artist / duration / now-playing indicator
    vh->line1->setText(track.title);
    const Artist* artist = db.artist(track.artistId);
    vh->line2->setText(artist ? artist->name
            : mContext->getString(R::string::unknown_artist_name));

    if (mIsNowPlaying) {
        vh->duration->setText("");
    } else {
        vh->duration->setText(makeTimeString(*mContext, track.durationMs / 1000));
    }

    // track rows carry no album art (original leaves the icon empty)
    vh->icon->setImageDrawable(nullptr);

    const long audioid = MediaPlaybackService::getInstance().getAudioId();
    if (!mDisableNowPlayingIndicator && audioid == track.id) {
        vh->play_indicator->setImageResource(R::drawable::indicator_ic_mp_playing_list);
    } else {
        vh->play_indicator->setImageResource(0);
    }
    return v;
}

REGISTER_ACTIVITY(TrackBrowserActivity);

} // namespace music
} // namespace cdroid
