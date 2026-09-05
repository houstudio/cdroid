// Port of com.wm.remusic.activity.PlayingActivity — the now-playing screen:
// album carousel (AlbumViewPager→ViewPager alias), needle, lyrics view,
// transport controls + progress. Lean cut: needle rotation animation and the
// comment/download tools land with the polish pass.
#include <algorithm>

#include <cdroid.h>
#include <cmath>
#include <functional>
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
#include <widget/toolbar.h>
#include <view/keyevent.h>
#include <widget/adapter.h>

#include <widget/viewpager.h>

#include "lrcview.h"
#include "mediaplaybackservice.h"
#include "musicplayer.h"
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
        // Tool row: favorite/download/comment/more — offline stubs for now.
        for (int id : {R::id::playing_fav, R::id::playing_down, R::id::playing_cmt, R::id::playing_more})
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

        MediaPlaybackService::getInstance().addListener(this,
                [this](const std::string& what) {
            if (what == MediaServiceActions::META_CHANGED) updateTrackInfo();
            if (what == MediaServiceActions::PLAYSTATE_CHANGED) updatePlayState();
        });
        updateTrackInfo();
        updatePlayState();
    }

    void onDestroy() override {
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

    void updateTrackInfo() {
        // Lyrics: sibling .lrc if present.
        mLrc->setLrcRows(parseLrc(loadLrcFor(MusicPlayer::getPath())));
        const std::string cover = MusicPlayer::currentTrackInfo()
                ? MusicPlayer::currentTrackInfo()->albumData : std::string();
        if (auto* art = (ImageView*) findViewById(R::id::albumArt)) {
            // Static backdrop: the cover fill; plain dark when absent (the
            // disc placeholder here painted giant concentric rings).
            if (!cover.empty()) art->setImageURIAsync("file://" + cover);
            else {
                art->setImageDrawable(nullptr);
                art->setBackgroundColor(0xFF232323);
            }
        }
        if (mDisc) {
            if (!cover.empty()) mDisc->setImageURIAsync("file://" + cover);
            else mDisc->setImageResource(R::drawable::placeholder_disk_210);
        }
    }

    // The original spins the ALBUM DISC while playing and lifts/lowers the
    // tonearm. Geometry is RESOLUTION-INDEPENDENT by construction:
    //  - the disc takes its bounds from view_pager's laid-out frame (the
    //    XML sizes/centers that area in dp for every screen);
    //  - the needle's intrinsic drawable is raw pixels (no density buckets),
    //    so the arm is rescaled onto the 360dp design grid and the pivot is
    //    set as FRACTIONS of the view (15.1dp of 92x138dp = 0.164 / 0.109),
    //    which stay put under uniform scaling.
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
            post([this, pagerView] {
                if (mDisc == nullptr || pagerView == nullptr) return;
                // Disc = the album area the XML already laid out (match_parent
                // x 263dp, centered): square on its height, centered on its
                // own center — correct at any resolution.
                const int side = pagerView->getHeight();
                auto* lp = (ViewGroup::MarginLayoutParams*) mDisc->getLayoutParams();
                lp->width = side;
                lp->height = side;
                lp->leftMargin = pagerView->getLeft() + (pagerView->getWidth() - side) / 2;
                lp->topMargin = pagerView->getTop();
                mDisc->setLayoutParams(lp);
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
                    auto* nlp = new ViewGroup::MarginLayoutParams(
                            (int)(side * 92.f / 263.f), (int)(side * 138.f / 263.f));
                    const float pivotX = nlp->width * 0.170f;
                    const float pivotY = nlp->height * 0.096f;
                    const int discCx = lp->leftMargin + side / 2;
                    nlp->leftMargin = (int)(discCx + side * 0.0114f - pivotX);
                    nlp->topMargin = (int)(lp->topMargin - side * 0.232f - pivotY);
                    mNeedle->setLayoutParams(nlp);
                    mNeedle->setPivotX(pivotX);
                    mNeedle->setPivotY(pivotY);
                }
            });
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

    // PlayQueueFragment, lean port: a scrimmed bottom sheet listing the
    // queue; tap a row to jump, tap the scrim to dismiss.
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
            auto* list = new ListView(getContext());
            sheet->addView(list, new LinearLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, 420));
            mQueueScrim->addView(sheet, new FrameLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT,
                    Gravity::BOTTOM));
            addView(mQueueScrim, new FrameLayout::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));

            std::vector<std::string> rows;
            const auto& queue = MusicPlayer::getQueue();
            const auto& infos = MusicPlayer::getPlayinfos();
            const int current = MusicPlayer::getQueuePosition();
            for (size_t i = 0; i < queue.size(); i++) {
                auto it = infos.find(queue[i]);
                std::string row = it != infos.end()
                        ? (it->second.musicName + " - " + it->second.artist)
                        : std::to_string(queue[i]);
                if ((int)i == current) row = "▶ " + row;
                rows.push_back(row);
            }
            auto* adapter = new ArrayAdapter<std::string>(
                    getContext(), R::layout::design_drawer_item, 0);
            adapter->addAll(rows);
            list->setAdapter(adapter);
            list->setOnItemClickListener([this, queue](AdapterView&, View&, int position, long) {
                if (position >= (int)queue.size()) return;
                MusicPlayer::setQueuePosition(position);
                hideQueueSheet();
            });
            mQueueSheet = list;
        }
        mQueueScrim->setVisibility(View::VISIBLE);
    }
    void hideQueueSheet() {
        if (mQueueScrim) mQueueScrim->setVisibility(View::GONE);
    }

    bool onKeyDown(int keyCode, KeyEvent& event) override {
        if (keyCode == KeyEvent::KEYCODE_BACK || keyCode == KeyEvent::KEYCODE_ESCAPE) {
            close();
            return true;
        }
        return FragmentActivity::onKeyDown(keyCode, event);
    }

    ImageView* mNeedle = nullptr;
    ImageView* mDisc = nullptr;
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
    ListView* mQueueSheet = nullptr;
    std::function<bool()> mRefresh;
};
REGISTER_ACTIVITY(PlayingActivity);

} // namespace
