/*********************************************************************************
 * Port of com.android.music.ArtistAlbumBrowserActivity — the Artists tab.
 *
 * DEVIATION: the original is an ExpandableListView (artist groups expanding
 * to album children); CDROID has no ExpandableListView, so this facade is a
 * flat artist list (same rows, same R.id.artisttab highlight) that drills
 * into AlbumBrowserActivity with the "artist" extra — the navigation the
 * original uses when expansion is unavailable. Everything else follows the
 * original (row layout, label formatting, menus).
 *********************************************************************************/
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

class ArtistAlbumBrowserActivity : public Window {
private:
    // ArtistAlbumListAdapter.ViewHolder (group rows)
    struct ViewHolder {
        void* owner = nullptr;   // guards recycled rows with a stale tag (see getView)
        TextView* line1;
        TextView* line2;
        ImageView* play_indicator;
        ImageView* icon;
    };

    class ArtistListAdapter : public Adapter {
    public:
        ArtistListAdapter(Context* ctx) : mContext(ctx) {}
        ~ArtistListAdapter() override {
            for (ViewHolder* h : mHolders) delete h;   // View doesn't own its tag
        }

        int getCount() const override { return (int)MusicDB::get().artists().size(); }
        void* getItem(int position) const override {
            return (void*)(uintptr_t)MusicDB::get().artists()[position].id;
        }
        long getItemId(int position) const override {
            return MusicDB::get().artists()[position].id;
        }
        View* getView(int position, View* convertView, ViewGroup* parent) override;

    private:
        Context* mContext;
        std::vector<ViewHolder*> mHolders;
    };

    ArtistListAdapter* mAdapter = nullptr;
    ListView* mList = nullptr;

public:
    ArtistAlbumBrowserActivity() : Window(0, 0, -1, -1) {}

    ~ArtistAlbumBrowserActivity() override {
        delete mAdapter;   // AdapterView doesn't own its adapter
    }

protected:
    void onCreate(Bundle*) override {
        setBackgroundColor(0xff000000);   // opaque backdrop (see MusicBrowserActivity)
        LayoutInflater::from(getContext())
                ->inflate(R::layout::media_picker_activity, this, true);
        if (!updateButtonBar(*this, R::id::artisttab)) addBackButton(*this);

        mList = dynamic_cast<ListView*>(findViewById(internal::R::id::list));
        mAdapter = new ArtistListAdapter(getContext());
        mList->setAdapter(mAdapter);

        // Drill into the artist's albums (original expands in place; see header)
        mList->setOnItemClickListener([](AdapterView&, View&, int position, long id) {
            Intent intent;
            intent.setClassName("cdroid.music", "AlbumBrowserActivity")
                  .setAction(Intent::ACTION_PICK)
                  .putExtra("artist", std::to_string(id));
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

View* ArtistAlbumBrowserActivity::ArtistListAdapter::getView(int position, View* convertView,
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
        vh->icon->setPadding(0, 0, 1, 0);
        v->setTag(vh);
        mHolders.push_back(vh);
    }

    const Artist& artist = MusicDB::get().artists()[position];
    MusicDB& db = MusicDB::get();
    const int numalbums = db.albumCountForArtist(artist.id);
    const int numsongs = db.trackCountForArtist(artist.id);

    // bindGroupView: line1 = artist, line2 = "N albums\n"
    vh->line1->setText(artist.name);
    vh->line2->setText(makeAlbumsLabel(*mContext, numalbums, numsongs, false));

    // group rows carry no album art (original leaves the icon empty)
    vh->icon->setImageDrawable(nullptr);

    const long currentartistid = MediaPlaybackService::getInstance().getArtistId();
    vh->play_indicator->setImageResource(
            currentartistid == artist.id ? R::drawable::indicator_ic_mp_playing_list : 0);
    return v;
}

REGISTER_ACTIVITY(ArtistAlbumBrowserActivity);

} // namespace music
} // namespace cdroid
