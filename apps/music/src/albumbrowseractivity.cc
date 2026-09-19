/*********************************************************************************
 * Port of com.android.music.AlbumBrowserActivity — the Albums tab.
 * A plain ListView of albums (optionally scoped to one artist via the
 * "artist" intent extra); rows are R.layout.track_list_item with the
 * default-album-art icon and the now-playing overlay indicator.
 * Cursor/AsyncQueryHandler plumbing → MusicDB; that is the only structural
 * change (bind logic follows AlbumListAdapter verbatim).
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

class AlbumBrowserActivity : public Window {
private:
    // AlbumListAdapter.ViewHolder. `owner` guards against recycled rows whose
    // tag still points into a previous adapter instance (stale tag = UAF).
    struct ViewHolder {
        void* owner = nullptr;
        TextView* line1;
        TextView* line2;
        ImageView* play_indicator;
        ImageView* icon;
    };

    class AlbumListAdapter : public Adapter {
    public:
        AlbumListAdapter(Context* ctx, std::vector<Album> albums)
            : mContext(ctx), mAlbums(std::move(albums)) {}
        ~AlbumListAdapter() override {
            for (ViewHolder* h : mHolders) delete h;   // View doesn't own its tag
        }

        int getCount() const override { return (int)mAlbums.size(); }
        void* getItem(int position) const override {
            return (void*)(uintptr_t)mAlbums[position].id;
        }
        long getItemId(int position) const override { return mAlbums[position].id; }
        View* getView(int position, View* convertView, ViewGroup* parent) override;

    private:
        Context* mContext;
        std::vector<Album> mAlbums;
        std::vector<ViewHolder*> mHolders;
    };

    std::vector<Album> mAlbums;
    AlbumListAdapter* mAdapter = nullptr;
    ListView* mList = nullptr;
    long mArtistId = -1;

public:
    AlbumBrowserActivity() : Window(0, 0, -1, -1) {}

    ~AlbumBrowserActivity() override {
        delete mAdapter;   // AdapterView doesn't own its adapter
    }

protected:
    void onCreate(Bundle*) override {
        const std::string artist = getIntent().getStringExtra("artist");
        if (!artist.empty()) mArtistId = strtoll(artist.c_str(), nullptr, 10);

        setBackgroundColor(0xff000000);   // opaque backdrop (see MusicBrowserActivity)
        LayoutInflater::from(getContext())
                ->inflate(R::layout::media_picker_activity, this, true);
        if (!updateButtonBar(*this, R::id::albumtab)) addBackButton(*this);

        mList = dynamic_cast<ListView*>(findViewById(internal::R::id::list));
        mAlbums = mArtistId >= 0 ? MusicDB::get().albumsForArtist(mArtistId)
                                 : MusicDB::get().albums();
        mAdapter = new AlbumListAdapter(getContext(), mAlbums);
        mList->setAdapter(mAdapter);

        // original onListItemClick: drill into the album's track list
        mList->setOnItemClickListener([this](AdapterView&, View&, int position, long id) {
            Intent intent;
            intent.setClassName("cdroid.music", "TrackBrowserActivity")
                  .setAction(Intent::ACTION_PICK)
                  .putExtra("album", std::to_string(id))
                  .putExtra("artist",
                          mArtistId >= 0 ? std::to_string(mArtistId) : std::string());
            App::getInstance().startActivity(intent);
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
        menu.add(0, Defs::PARTY_SHUFFLE, 0,
                getContext()->getString(R::string::party_shuffle));
        menu.add(0, Defs::SHUFFLE_ALL, 0,
                getContext()->getString(R::string::shuffle_all))
                ->setIcon(R::drawable::ic_menu_shuffle);
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
        }
        return Window::onOptionsItemSelected(item);
    }
};

View* AlbumBrowserActivity::AlbumListAdapter::getView(int position, View* convertView,
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
        vh->play_indicator = dynamic_cast<ImageView*>(v->findViewById(R::id::play_indicator));
        vh->icon = dynamic_cast<ImageView*>(v->findViewById(R::id::icon));
        vh->icon->setBackgroundResource(R::drawable::albumart_mp_unknown_list);
        vh->icon->setPadding(0, 0, 1, 0);
        v->setTag(vh);
        mHolders.push_back(vh);
    }

    const Album& album = mAlbums[position];
    const Artist* artist = MusicDB::get().artist(album.artistId);
    // line1 = album (unknown → resource string), line2 = artist
    vh->line1->setText(album.name);
    vh->line2->setText(artist ? artist->name
            : mContext->getString(R::string::unknown_artist_name));

    // no per-album artwork in the facade: background default art only
    vh->icon->setImageDrawable(nullptr);

    const long currentalbumid = MediaPlaybackService::getInstance().getAlbumId();
    vh->play_indicator->setImageResource(
            currentalbumid == album.id ? R::drawable::indicator_ic_mp_playing_list : 0);
    return v;
}

REGISTER_ACTIVITY(AlbumBrowserActivity);

} // namespace music
} // namespace cdroid
