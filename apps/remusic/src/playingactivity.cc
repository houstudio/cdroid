// Port of com.wm.remusic.activity.PlayingActivity — the now-playing screen:
// album carousel (AlbumViewPager→ViewPager alias), needle, lyrics view,
// transport controls + progress. Lean cut: needle rotation animation and the
// comment/download tools land with the polish pass.
#include <algorithm>

#include <cdroid.h>
#include <cmath>
#include <functional>
#include <thread>
#include <R.h>
#include <text/String.h>
#include <core/activityfactory.h>
#include <fragment/fragmentactivity.h>
#include <widget/imageview.h>
#include <widget/seekbar.h>
#include <widget/textview.h>
#include <widget/framelayout.h>
#include <widget/linearlayout.h>
#include <widget/listview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/itemtouchhelper.h>
#include <widget/toolbar.h>
#include <widget/toast.h>
#include <view/keyevent.h>
#include <widget/adapter.h>

#include <widget/viewpager.h>

#include "lrcview.h"
#include "themestore.h"
#ifdef REMUSIC_ONLINE
#include <core/handler.h>
#include <core/looper.h>
#include "lrclib.h"
#include "faviconcache.h"
#endif
#include "mediaplaybackservice.h"
#include "musicplayer.h"
#include "downloadmanager.h"
#include "quickcontrols.h"
#include "musicprovider.h"

using namespace cdroid;
using namespace remusic;

namespace {

std::string mmss(long ms) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%02ld:%02ld", ms / 60000, (ms / 1000) % 60);
    return buf;
}

// Look for a sibling .lrc (song.mp3 -> song.lrc), like the original's
// /remusic/lrc/<id> cache fallback path.
#ifdef REMUSIC_ONLINE
// Worker -> UI handoff, same idiom as the online clients: a Handler on the
// main looper (View::post in this file is only ever called on the UI thread).
static void postToMain(std::function<void()> fn) {
    static cdroid::Handler sMain(cdroid::Looper::getMainLooper());
    sMain.post(std::move(fn));
}
#endif

std::string loadLrcFor(const std::string& audioPath) {
    if (audioPath.empty()) return "";
    const size_t dot = audioPath.rfind('.');
    const std::string stem = dot == std::string::npos ? audioPath : audioPath.substr(0, dot);
    for (const char* suf : {".lrc", ".LRC"}) {
        FILE* f = fopen((stem + suf).c_str(), "rb");
        if (f == nullptr) continue;
        std::string data;
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) data.append(buf, n);
        fclose(f);
        return data;
    }
    return "";
}

// com.wm.remusic.widget.AlbumViewPager — the disc carousel that captures
// single taps (to flip to lyrics) while keeping page swipes.
class AlbumViewPager : public ViewPager {
public:
    AlbumViewPager(Context* ctx, const AttributeSet* attrs) : ViewPager(ctx, attrs) {}
    bool onTouchEvent(MotionEvent& ev) override {
        switch (ev.getActionMasked()) {
        case MotionEvent::ACTION_DOWN:
            mDownX = ev.getX(); mDownY = ev.getY(); mMoved = false;
            break;
        case MotionEvent::ACTION_MOVE:
            if (std::abs(ev.getX() - mDownX) > 20 || std::abs(ev.getY() - mDownY) > 20)
                mMoved = true;
            break;
        case MotionEvent::ACTION_UP:
            if (!mMoved && mToggle) mToggle();
            break;
        }
        return ViewPager::onTouchEvent(ev);
    }
    void setTapListener(std::function<void()> fn) { mToggle = std::move(fn); }
private:
    float mDownX = 0, mDownY = 0;
    bool mMoved = false;
    std::function<void()> mToggle;
};
static const bool sAlbumPagerRegistered = (LayoutInflater::registerInflater("AlbumViewPager", 0,
        [](Context* ctx, const AttributeSet& attr) -> View* {
    return new AlbumViewPager(ctx, &attr);
}), true);

class PlayingActivity : public FragmentActivity {
public:
    PlayingActivity() : FragmentActivity(0, 0, -1, -1) {}

    void onCreate(Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        LayoutInflater::from(getContext())->inflate(R::layout::activity_playing, this, true);

        mPlay = (ImageView*) findViewById(R::id::playing_play);
        mPre = (ImageView*) findViewById(R::id::playing_pre);
        mNext = (ImageView*) findViewById(R::id::playing_next);
        mMode = (ImageView*) findViewById(R::id::playing_mode);
        mSeek = (SeekBar*) findViewById(R::id::play_seek);
        mPlayed = (TextView*) findViewById(R::id::music_duration_played);
        mDuration = (TextView*) findViewById(R::id::music_duration);
        mLrc = (LrcView*) findViewById(R::id::lrcview);
        mLrc->setOnSeekListener([](int ms) { MusicPlayer::seek(ms); });

        mPlay->setOnClickListener([](View&) { MusicPlayer::playOrPause(); });
        mNext->setOnClickListener([](View&) { MusicPlayer::next(); });
        mPre->setOnClickListener([](View&) { MusicPlayer::previous(false); });
        mMode->setOnClickListener([](View&) { MusicPlayer::cycleRepeat(); });
        if (auto* playlist = findViewById(R::id::playing_playlist))
            playlist->setOnClickListener([this](View&) { showQueueSheet(); });
        // Tool row: favorite/download/comment are offline stubs; the overflow
        // (⋮) opens the sleep timer (TimingFragment in the original, reached
        // through the same overflow menu).
        if (auto* more = findViewById(R::id::playing_more))
            more->setOnClickListener([this](View&) { showTimingSheet(*this); });
        // Download arrow: AddDownTask for online (http) tracks — the file
        // lands in ~/Music and the library picks it up on the next scan.
        if (auto* down = findViewById(R::id::playing_down))
            down->setOnClickListener([this](View&) {
                const std::string url = MusicPlayer::getPath();
                const bool online = url.compare(0, 7, "http://") == 0
                        || url.compare(0, 8, "https://") == 0;
                if (!online) {
                    Toast::makeText(getContext(), "仅在线曲目可下载")->show();
                    return;
                }
                const std::string name = MusicPlayer::getTrackName()
                        + " - " + MusicPlayer::getArtistName();
                if (DownloadManager::get().addTask(url, name) != 0)
                    Toast::makeText(getContext(), "已加入下载队列,见下载管理")->show();
            });
        for (int id : {R::id::playing_fav, R::id::playing_cmt})
            if (auto* v = findViewById(id)) v->setOnClickListener([](View&) {});

        if (auto* toolbar = (Toolbar*) findViewById(R::id::toolbar)) {
            toolbar->setNavigationIcon(getContext()->getDrawable(R::drawable::actionbar_back));
            toolbar->setNavigationOnClickListener([this](View&) { close(); });
        }

        // Disc <-> lyrics toggle: single tap on the disc (AlbumViewPager).
        if (auto* pager = findViewById(R::id::view_pager))
            ((AlbumViewPager*) pager)->setTapListener([this] { toggleLrc(true); });
        if (auto* lrc = findViewById(R::id::lrcviewContainer))
            lrc->setOnClickListener([this](View&) { toggleLrc(false); });

        wireNeedleAndDisc();
        wireSeek();
        applyPlayTheme();
        // The 360x640 design column scales to the window height: the page
        // keeps the original's phone proportions at any window size (the
        // needle/disc geometry sync works in pre-scale coordinates and the
        // column scales as a whole; touch dispatch inverse-transforms).
        if (auto* col = findViewById(R::id::design_column)) {
            post([this, col] {
                if (!*mAlive || col->getParent() == nullptr) return;
                auto* root = (View*) col->getParent();
                const float s = (float) root->getHeight() / 640.f;
                if (s > 0.1f) {
                    col->setScaleX(s);
                    col->setScaleY(s);
                }
            });
        }

        MediaPlaybackService::getInstance().addListener(this,
                [this](const std::string& what) {
            if (what == MediaServiceActions::META_CHANGED) updateTrackInfo();
            if (what == MediaServiceActions::PLAYSTATE_CHANGED) updatePlayState();
        });
        updateTrackInfo();
        updatePlayState();
    }

    void onDestroy() override {
        *mAlive = false;   // in-flight lyrics fetch must not touch the corpse
        MediaPlaybackService::getInstance().removeListener(this);
        FragmentActivity::onDestroy();
    }

private:
    void wireSeek() {
        SeekBar::OnSeekBarChangeListener l;
        l.onProgressChanged = [this](SeekBar&, int progress, bool fromUser) {
            if (fromUser) mScrubbingMs = progress;
        };
        l.onStartTrackingTouch = [this](SeekBar&) { mScrubbing = true; };
        l.onStopTrackingTouch = [this](SeekBar&) {
            MusicPlayer::seek(mScrubbingMs);
            mScrubbing = false;
        };
        mSeek->setOnSeekBarChangeListener(l);
        post(mRefresh = [this] {
            refreshProgress();
            return true;
        });
    }

    void refreshProgress() {
        const long dur = MusicPlayer::duration();
        const long pos = MusicPlayer::position();
        mSeek->setMax((int)std::max<long>(1, dur));
        if (!mScrubbing) mSeek->setProgress((int)pos);
        mPlayed->setText(mmss(pos));
        mDuration->setText(mmss(dur));
        if (mLrcVisible) mLrc->seekTo((int)pos, false);
        postDelayed(mRefresh, 500);
    }

    // Day (default — the original's look in the reference screenshots) or
    // night: page scrim, text colors, nav-icon tint, lyric colors.
    void applyPlayTheme() {
        const bool night = ThemeStore::get().night();
        if (auto* scrim = findViewById(R::id::page_scrim))
            scrim->setBackgroundColor(night ? 0x70000000u : 0xE6FFFFFFu);
        const uint32_t main = night ? 0xFFFFFFFFu : 0xFF3B3B3Bu;
        const uint32_t sub = night ? 0xB3FFFFFFu : 0xFF8A8A8Au;
        for (int id : {R::id::play_title, R::id::music_duration_played,
                R::id::music_duration})
            if (auto* v = (TextView*) findViewById(id))
                v->setTextColor(id == R::id::play_title ? main : sub);
        if (auto* artist = (TextView*) findViewById(R::id::play_artist))
            artist->setTextColor(sub);
        if (auto* toolbar = (Toolbar*) findViewById(R::id::toolbar))
            if (Drawable* icon = toolbar->getNavigationIcon())
                icon->setTint(night ? 0xFFFFFFFFu : 0xFF3B3B3Bu);
        mLrc->setColors(night ? 0xFF3333FFu : 0xFFD43C33u,
                        night ? 0xFFAAAAAAu : 0xFF9E9E9Eu);
    }

#ifdef REMUSIC_ONLINE
    // Lyrics for tracks with no sibling .lrc — the on-demand (Audius/
    // ccMixter) songs. Blocking fetch on a worker thread, applied on the UI
    // thread under a generation tag: a result landing after the track
    // switched (gen) or the activity died (mAlive) is dropped, not applied.
    void fetchOnlineLyrics(bool needed) {
        const int gen = ++mLrcGen;   // any track change invalidates in-flight fetches
        if (!needed) return;
        const MusicInfo* info = MusicPlayer::currentTrackInfo();
        if (info == nullptr || info->musicName.empty() || info->duration <= 0) return;
        const std::string title = info->musicName;
        const std::string artist = info->artist;
        const long durationMs = info->duration;
        auto alive = mAlive;
        std::thread([this, alive, gen, title, artist, durationMs] {
            const std::string lrc = LrcLib::fetchSyncedLyrics(title, artist, durationMs);
            postToMain([this, alive, gen, lrc] {
                if (!*alive || gen != mLrcGen || lrc.empty()) return;
                mLrc->setLrcRows(parseLrc(lrc));   // refreshProgress() re-seeks
            });
        }).detach();
    }
#endif

    void updateTrackInfo() {
        // Lyrics: sibling .lrc first; a miss (on-demand tracks ship none)
        // falls through to LRCLIB in the background.
        const std::string lrcData = loadLrcFor(MusicPlayer::getPath());
        mLrc->setLrcRows(parseLrc(lrcData));
#ifdef REMUSIC_ONLINE
        fetchOnlineLyrics(lrcData.empty());
#endif
        // Centered header (the original's): song name + artist, colors per theme.
        const MusicInfo* info = MusicPlayer::currentTrackInfo();
        if (auto* t = (TextView*) findViewById(R::id::play_title))
            t->setText(info ? info->musicName : std::string());
        if (auto* a = (TextView*) findViewById(R::id::play_artist))
            a->setText(info ? info->artist : std::string());
        const std::string cover = MusicPlayer::currentTrackInfo()
                ? MusicPlayer::currentTrackInfo()->albumData : std::string();
        // On-demand tracks carry their cover as a REMOTE url (the listing
        // APIs' artwork) — those route through the disk cache below.
        const bool remote = cover.compare(0, 7, "http://") == 0
                || cover.compare(0, 8, "https://") == 0;
        if (auto* art = (ImageView*) findViewById(R::id::albumArt)) {
            // Static backdrop: the cover fill; plain dark when absent (the
            // disc placeholder here painted giant concentric rings).
            if (!remote) {
                if (!cover.empty()) art->setImageURIAsync("file://" + cover);
                else {
                    art->setImageDrawable(nullptr);
                    art->setBackgroundColor(0xFF232323);
                }
            }
        }
        if (mDisc) {
            if (!remote) {
                if (!cover.empty()) mDisc->setImageURIAsync("file://" + cover);
                else mDisc->setImageResource(R::drawable::placeholder_disk_210);
            }
        }
#ifdef REMUSIC_ONLINE
        if (remote) {
            auto alive = mAlive;
            FaviconCache::load(getContext(), cover,
                    [this, alive](const std::string& file) {
                if (!*alive || file.empty()) return;
                if (auto* art = (ImageView*) findViewById(R::id::albumArt))
                    art->setImageURIAsync("file://" + file);
                if (mDisc) mDisc->setImageURIAsync("file://" + file);
            });
        }
#endif
    }

    // The original spins the ALBUM DISC while playing and lifts/lowers the
    // tonearm. Geometry is RESOLUTION-INDEPENDENT by construction:
    //  - the disc takes its bounds from view_pager's laid-out frame (the
    //    XML sizes/centers that area in dp for every screen);
    //  - the needle's intrinsic drawable is raw pixels (no density buckets),
    //    so the arm is rescaled onto the 360dp design grid and the pivot is
    //    set as FRACTIONS of the view (15.1dp of 92x138dp = 0.164 / 0.109),
    //    which stay put under uniform scaling.
    // Derive the disc + needle geometry from view_pager's CURRENT frame.
    // Idempotent (re-derives only what actually changed), so the layout
    // listener below can call it on every pager layout without looping.
    void syncDiscAndNeedleGeometry() {
        View* pagerView = findViewById(R::id::view_pager);
        if (mDisc == nullptr || pagerView == nullptr) return;
        // Disc = the album area the XML already laid out (match_parent
        // x 263dp, centered): square on its height, centered on its
        // own center — correct at any resolution.
        const int side = pagerView->getHeight();
        if (side <= 0) return;   // not laid out yet; the listener retries
        auto* lp = (ViewGroup::MarginLayoutParams*) mDisc->getLayoutParams();
        const int discL = pagerView->getLeft() + (pagerView->getWidth() - side) / 2;
        const int discT = pagerView->getTop();
        if (lp->width != side || lp->height != side
                || lp->leftMargin != discL || lp->topMargin != discT) {
            lp->width = side;
            lp->height = side;
            lp->leftMargin = discL;
            lp->topMargin = discT;
            mDisc->setLayoutParams(lp);
        }
        mDisc->setCornerRadii(side / 2);            // circular crop
        // Needle: intrinsic 276x414 raw px would dwarf small screens,
        // and the XML anchors it parent-right+100dp — tuned for the
        // 360dp design width, so on wider windows the post drifts
        // right while the disc stays centered. Lock the arm to the
        // DISC instead: every quantity is a fraction of the disc side
        // (design ratios 92:263 / 138:263; post circle at 0.170/0.096
        // of the art per play_needle.png; post offset from the disc
        // center = +0.0114/-0.732 of the side), so needle and record
        // can never decouple at any resolution.
        if (mNeedle != nullptr) {
            const int w = (int)(side * 92.f / 263.f);
            const int h = (int)(side * 138.f / 263.f);
            const float pivotX = w * 0.170f;
            const float pivotY = h * 0.096f;
            const int nL = (int)(discL + side / 2 + side * 0.0114f - pivotX);
            const int nT = (int)(discT - side * 0.232f - pivotY);
            auto* cur = (ViewGroup::MarginLayoutParams*) mNeedle->getLayoutParams();
            if (cur == nullptr || cur->width != w || cur->height != h
                    || cur->leftMargin != nL || cur->topMargin != nT) {
                // Fresh rule-less params: absolute positioning in the
                // RelativeLayout (replacing, not mutating, drops the XML
                // parent-right anchor rules for good).
                auto* nlp = new ViewGroup::MarginLayoutParams(w, h);
                nlp->leftMargin = nL;
                nlp->topMargin = nT;
                mNeedle->setLayoutParams(nlp);
                mNeedle->setPivotX(pivotX);
                mNeedle->setPivotY(pivotY);
            }
        }
    }

    void wireNeedleAndDisc() {
        mNeedle = (ImageView*) findViewById(R::id::needle);
        // Dedicated disc view, added over the (empty) album pager area.
        View* pagerView = findViewById(R::id::view_pager);
        ViewGroup* pagerArea = pagerView ? (ViewGroup*) pagerView->getParent() : nullptr;
        if (pagerArea != nullptr) {
            mDisc = new ImageView(getContext());
            mDisc->setScaleType(ScaleType::FIT_XY);
            mDisc->setImageResource(R::drawable::placeholder_disk_play_song);
            auto* lp = new ViewGroup::MarginLayoutParams(0, 0);
            mDisc->setLayoutParams(lp);
            // Z-order: above the pager, BELOW the needle (the arm rests on
            // the record, not under it). RelativeLayout draws later children
            // on top, so insert just before the needle.
            int at = pagerArea->getChildCount();
            for (int i = 0; i < pagerArea->getChildCount(); i++)
                if (pagerArea->getChildAt(i)->getId() == R::id::needle) { at = i; break; }
            pagerArea->addView(mDisc, at);
            // Sync off every pager LAYOUT, not a one-shot post(): the post
            // raced the first traversal on some runs (needle stuck at its
            // XML parent-right anchor, disc left 0x0) and never retried.
            // The listener dies with the content view, so `this` is safe.
            // NEVER swap LayoutParams synchronously here — this fires INSIDE
            // the parent RelativeLayout's layout walk, which still holds the
            // needle's old params from measure (the swap frees them → the
            // walk lays the needle out off a dangling pointer → garbage
            // bounds → pixman "Invalid rectangle"). Defer to after the
            // traversal; the sync is idempotent so re-posts are free.
            pagerView->addOnLayoutChangeListener(
                    [this](View&, int, int, int, int, int, int, int, int) {
                post([this] { syncDiscAndNeedleGeometry(); });
            });
            syncDiscAndNeedleGeometry();
        }
        mSpinKeepalive = [this]() -> bool {
            if (MusicPlayer::isPlaying()) {
                if (mDisc) mDisc->setRotation(mDisc->getRotation() + 0.6f);
                if (mNeedle) mNeedle->setRotation(0.f);     // resting on the record
            } else if (mNeedle) {
                mNeedle->setRotation(-30.f);                // lifted (XML initial)
            }
            postDelayed(mSpinKeepalive, 40);
            return true;
        };
        post(mSpinKeepalive);
    }

    void updatePlayState() {
        mPlay->setImageResource(MusicPlayer::isPlaying()
                ? R::drawable::playbar_btn_pause : R::drawable::playbar_btn_play);
    }

    void toggleLrc(bool showLrc) {
        mLrcVisible = showLrc;
        if (auto* c = findViewById(R::id::lrcviewContainer))
            c->setVisibility(showLrc ? View::VISIBLE : View::GONE);
        if (auto* p = findViewById(R::id::view_pager))
            p->setVisibility(showLrc ? View::GONE : View::VISIBLE);
        if (mDisc) mDisc->setVisibility(showLrc ? View::GONE : View::VISIBLE);
        if (mNeedle) mNeedle->setVisibility(showLrc ? View::GONE : View::VISIBLE);
    }

    // PlayQueueFragment, port: scrimmed bottom sheet listing the queue; tap
    // a row to jump, long-press-drag to reorder (DragSortRecycler's role in
    // the original, done with the ported androidx ItemTouchHelper here).
    void showQueueSheet() {
        if (mQueueSheet == nullptr) {
            mQueueScrim = new FrameLayout(getContext());
            mQueueScrim->setBackgroundColor(0x88000000u);
            mQueueScrim->setOnClickListener([this](View&) { hideQueueSheet(); });
            auto* sheet = new LinearLayout(getContext());
            sheet->setOrientation(LinearLayout::VERTICAL);
            sheet->setBackgroundColor(0xFFF5F5F5u);
            auto* title = new TextView(getContext());
            title->setText("当前播放 (" + std::to_string(MusicPlayer::getQueueSize()) + ")");
            title->setTextSize(18);
            title->setTextColor(0xFF333333);
            title->setPadding(24, 18, 24, 18);
            sheet->addView(title, new LinearLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, 56));
            auto* list = new RecyclerView(getContext());
            list->setLayoutManager(new LinearLayoutManager(getContext()));
            mQueueIds = MusicPlayer::getQueue();
            list->setAdapter(mQueueAdapter = new QueueAdapter(this));
            mQueueTouchCb = new QueueTouchCallback(this);
            mQueueTouch = new ItemTouchHelper(mQueueTouchCb);
            mQueueTouch->attachToRecyclerView(list);
            sheet->addView(list, new LinearLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, 420));
            mQueueScrim->addView(sheet, new FrameLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT,
                    ViewGroup::LayoutParams::WRAP_CONTENT,
                    Gravity::BOTTOM));
            addView(mQueueScrim, new FrameLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT,
                    ViewGroup::LayoutParams::MATCH_PARENT));
            mQueueSheet = list;
        }
        mQueueIds = MusicPlayer::getQueue();
        if (mQueueAdapter) mQueueAdapter->notifyDataSetChanged();
        mQueueScrim->setVisibility(View::VISIBLE);
    }
    void hideQueueSheet() {
        if (mQueueScrim) mQueueScrim->setVisibility(View::GONE);
    }

    // Rows: "name - artist", the playing row marked with a play glyph; tap jumps.
    class QueueAdapter : public RecyclerView::Adapter {
    public:
        explicit QueueAdapter(PlayingActivity* host) : mHost(host) {}
        RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int) override {
            View* v = LayoutInflater::from(parent->getContext())
                    ->inflate(R::layout::design_drawer_item, parent, false);
            return new RecyclerView::ViewHolder(v);
        }
        void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
            if (position >= (int) mHost->mQueueIds.size()) return;
            const long id = mHost->mQueueIds[position];
            const auto& infos = MusicPlayer::getPlayinfos();
            auto it = infos.find(id);
            std::string row = it != infos.end()
                    ? (it->second.musicName + " - " + it->second.artist)
                    : std::to_string(id);
            if (position == MusicPlayer::getQueuePosition()) row = "> " + row;
            ((TextView*) holder.itemView)->setText(row);
            holder.itemView->setOnClickListener([this, position](View&) {
                if (position >= (int) mHost->mQueueIds.size()) return;
                MusicPlayer::setQueuePosition(position);
                mHost->mQueueAdapter->notifyDataSetChanged();
            });
        }
        int getItemCount() override { return (int) mHost->mQueueIds.size(); }
    private:
        PlayingActivity* mHost;
    };
    friend class QueueAdapter;

    // Long-press drag = the service's moveQueueItem; swipe aside removes
    // the row (PlayQueueFragment's delete).
    class QueueTouchCallback : public ItemTouchHelper::SimpleCallback {
    public:
        explicit QueueTouchCallback(PlayingActivity* host)
                : ItemTouchHelper::SimpleCallback(ItemTouchHelper::UP | ItemTouchHelper::DOWN,
                        ItemTouchHelper::LEFT | ItemTouchHelper::RIGHT), mHost(host) {}
        bool onMove(RecyclerView&, RecyclerView::ViewHolder& vh,
                    RecyclerView::ViewHolder& target) override {
            const int from = vh.getLayoutPosition();
            const int to = target.getLayoutPosition();
            if (from < 0 || to < 0 || from == to
                    || from >= (int) mHost->mQueueIds.size()
                    || to >= (int) mHost->mQueueIds.size())
                return false;
            MusicPlayer::moveQueueItem(from, to);
            const long id = mHost->mQueueIds[from];
            mHost->mQueueIds.erase(mHost->mQueueIds.begin() + from);
            mHost->mQueueIds.insert(mHost->mQueueIds.begin() + to, id);
            mHost->mQueueAdapter->notifyItemMoved(from, to);
            return true;
        }
        void onSwiped(RecyclerView::ViewHolder& vh, int) override {
            const int pos = vh.getLayoutPosition();
            if (pos < 0 || pos >= (int) mHost->mQueueIds.size()) return;
            const long id = mHost->mQueueIds[pos];
            MusicPlayer::removeTrackAtPosition(id, pos);
            mHost->mQueueIds.erase(mHost->mQueueIds.begin() + pos);
            mHost->mQueueAdapter->notifyItemRemoved(pos);
        }
    private:
        PlayingActivity* mHost;
    };
    friend class QueueTouchCallback;

    bool onKeyDown(int keyCode, KeyEvent& event) override {
        if (keyCode == KeyEvent::KEYCODE_BACK || keyCode == KeyEvent::KEYCODE_ESCAPE) {
            close();
            return true;
        }
        return FragmentActivity::onKeyDown(keyCode, event);
    }

    ImageView* mNeedle = nullptr;
    ImageView* mDisc = nullptr;
    std::shared_ptr<bool> mAlive = std::make_shared<bool>(true);  // teardown guard
    int mLrcGen = 0;                       // online-lyrics fetch generation tag
    std::function<bool()> mSpinKeepalive;
    ImageView* mPlay = nullptr;
    ImageView* mPre = nullptr;
    ImageView* mNext = nullptr;
    ImageView* mMode = nullptr;
    SeekBar* mSeek = nullptr;
    TextView* mPlayed = nullptr;
    TextView* mDuration = nullptr;
    LrcView* mLrc = nullptr;
    bool mLrcVisible = false;
    bool mScrubbing = false;
    long mScrubbingMs = 0;
    FrameLayout* mQueueScrim = nullptr;
    RecyclerView* mQueueSheet = nullptr;
    std::vector<long> mQueueIds;
    QueueAdapter* mQueueAdapter = nullptr;
    ItemTouchHelper* mQueueTouch = nullptr;
    QueueTouchCallback* mQueueTouchCb = nullptr;
    std::function<bool()> mRefresh;
};
REGISTER_ACTIVITY(PlayingActivity);

} // namespace
