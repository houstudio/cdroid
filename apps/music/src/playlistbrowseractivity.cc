/*********************************************************************************
 * Port of com.android.music.PlaylistBrowserActivity — the Playlists tab.
 * Rows follow PlaylistListAdapter (icon + name, line2 hidden). "New playlist"
 * from the options menu shows the create-playlist dialog inline (the
 * original launches the CreatePlaylist dialog activity); the facade builds
 * the playlist in MusicDB.
 *********************************************************************************/
#include <algorithm>

#include <R.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <core/activityfactory.h>
#include <core/app.h>
#include <core/context.h>
#include <core/intent.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <porting/cdlog.h>
#include <view/layoutinflater.h>
#include <widget/cdwindow.h>
#include <widget/edittext.h>
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

class PlaylistBrowserActivity : public Window {
private:
    // PlaylistListAdapter.ViewHolder
    struct ViewHolder {
        void* owner = nullptr;   // guards recycled rows with a stale tag (see getView)
        TextView* line1;
        TextView* line2;
        ImageView* play_indicator;
        ImageView* icon;
    };

    class PlaylistListAdapter : public Adapter {
    public:
        PlaylistListAdapter(Context* ctx) : mContext(ctx) {}
        ~PlaylistListAdapter() override {
            for (ViewHolder* h : mHolders) delete h;   // View doesn't own its tag
        }

        int getCount() const override { return (int)MusicDB::get().playlists().size(); }
        void* getItem(int position) const override {
            return (void*)(uintptr_t)MusicDB::get().playlists()[position].id;
        }
        long getItemId(int position) const override {
            return MusicDB::get().playlists()[position].id;
        }
        View* getView(int position, View* convertView, ViewGroup* parent) override;

        void refresh() { notifyDataSetChanged(); }

    private:
        Context* mContext;
        std::vector<ViewHolder*> mHolders;
    };

    PlaylistListAdapter* mAdapter = nullptr;
    ListView* mList = nullptr;

public:
    PlaylistBrowserActivity() : Window(0, 0, -1, -1) {}

    ~PlaylistBrowserActivity() override {
        delete mAdapter;   // AdapterView doesn't own its adapter
    }

protected:
    void onCreate(Bundle*) override {
        setBackgroundColor(0xff000000);   // opaque backdrop (see MusicBrowserActivity)
        LayoutInflater::from(getContext())
                ->inflate(R::layout::media_picker_activity, this, true);
        if (!updateButtonBar(*this, R::id::playlisttab)) addBackButton(*this);

        mList = dynamic_cast<ListView*>(findViewById(internal::R::id::list));
        mAdapter = new PlaylistListAdapter(getContext());
        mList->setAdapter(mAdapter);

        // onListItemClick: open the playlist's track list
        mList->setOnItemClickListener([](AdapterView&, View&, int position, long id) {
            Intent intent;
            intent.setClassName("cdroid.music", "TrackBrowserActivity")
                  .setAction(Intent::ACTION_EDIT)
                  .putExtra("playlist", std::to_string(id));
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
        menu.add(0, Defs::NEW_PLAYLIST, 0,
                getContext()->getString(R::string::new_playlist));
        menu.add(0, Defs::PARTY_SHUFFLE, 0,
                getContext()->getString(R::string::party_shuffle));
        return true;
    }

    bool onOptionsItemSelected(MenuItem& item) override {
        switch (item.getItemId()) {
        case Defs::NEW_PLAYLIST:
            showDialog();
            return true;
        case Defs::PARTY_SHUFFLE:
            togglePartyShuffle();
            return true;
        }
        return Window::onOptionsItemSelected(item);
    }

private:
    // CreatePlaylist dialog (facade: inline AlertDialog instead of a dialog
    // activity; default name + OK create the playlist in the mock DB)
    void showDialog() {
        Context* ctx = getContext();
        auto* builder = new AlertDialog::Builder(ctx);
        builder->setTitle(ctx->getString(R::string::create_playlist_create_text));
        EditText* input = new EditText(ctx);
        // original seeds the field with new_playlist_name_template; the facade
        // starts empty (the template carries a %d counter we don't track)
        input->setText("");
        builder->setView(input);
        builder->setPositiveButton(ctx->getString(R::string::create_playlist_create_text),
                [this, input](Dialog&, int) {
                    const std::string name = input->getText().toUTF8();
                    if (!name.empty()) {
                        MusicDB::get().createPlaylist(name);
                        mAdapter->refresh();
                    }
                });
        builder->setNegativeButton(
                ctx->getString(internal::R::string::cancel), nullptr);
        builder->create()->show();
    }
};

View* PlaylistBrowserActivity::PlaylistListAdapter::getView(int position, View* convertView,
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
        // original newView: line2 gone, icon = playlist art
        vh->line2->setVisibility(View::GONE);
        vh->icon->setImageResource(R::drawable::ic_mp_playlist_list);
        mHolders.push_back(vh);
    }

    const Playlist& pl = MusicDB::get().playlists()[position];
    vh->line1->setText(pl.name);

    const long currentaudio = MediaPlaybackService::getInstance().getAudioId();
    const bool playingThis =
            MediaPlaybackService::getInstance().isInitialized() && !pl.trackIds.empty()
            && std::find(pl.trackIds.begin(), pl.trackIds.end(), currentaudio)
                       != pl.trackIds.end();
    vh->play_indicator->setImageResource(
            playingThis ? R::drawable::indicator_ic_mp_playing_list : 0);
    return v;
}

REGISTER_ACTIVITY(PlaylistBrowserActivity);

} // namespace music
} // namespace cdroid
