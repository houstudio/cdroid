/*********************************************************************************
 * Port of com.android.music.MediaPlaybackActivity — the now-playing screen
 * (R.layout.audio_player). Transport wiring, seek/scan behavior, shuffle and
 * repeat cycling, the 1-second progress refresh loop and the blink-when-
 * paused counter all follow the original. NOT ported (facade): the
 * label-drag scrolling gesture, the album-art worker thread (static default
 * art), the search long-press chooser and the audio-effects panel hook.
 *********************************************************************************/
#include <R.h>
#include <core/activityfactory.h>
#include <core/app.h>
#include <core/context.h>
#include <core/intent.h>
#include <core/systemclock.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <porting/cdlog.h>
#include <view/layoutinflater.h>
#include <widget/cdwindow.h>
#include <widget/imagebutton.h>
#include <widget/imageview.h>
#include <widget/internal_R.h>
#include <widget/progressbar.h>
#include <widget/seekbar.h>
#include <widget/textview.h>
#include <widget/toast.h>

#include <mediaplaybackservice.h>
#include <musicutils.h>
#include <repeatingimagebutton.h>

using namespace ::music;

namespace cdroid {
namespace music {

using Service = MediaPlaybackService;

class MediaPlaybackActivity : public Window {
public:
    MediaPlaybackActivity() : Window(0, 0, -1, -1) {}

protected:
    void onCreate(Bundle*) override {
        setBackgroundColor(0xff000000);   // opaque backdrop (see MusicBrowserActivity)
        LayoutInflater::from(getContext())->inflate(R::layout::audio_player, this, true);
        addBackButton(*this);   // no BACK key on the embedded targets

        Context* ctx = getContext();
        mCurrentTime = dynamic_cast<TextView*>(findViewById(R::id::currenttime));
        mTotalTime = dynamic_cast<TextView*>(findViewById(R::id::totaltime));
        mProgress = dynamic_cast<ProgressBar*>(findViewById(internal::R::id::progress));
        mAlbum = dynamic_cast<ImageView*>(findViewById(R::id::album));
        mArtistName = dynamic_cast<TextView*>(findViewById(R::id::artistname));
        mAlbumName = dynamic_cast<TextView*>(findViewById(R::id::albumname));
        mTrackName = dynamic_cast<TextView*>(findViewById(R::id::trackname));

        mPrevButton = dynamic_cast<RepeatingImageButton*>(findViewById(R::id::prev));
        mPrevButton->setOnClickListener([this](View&) {
            Service& s = Service::getInstance();
            if (s.position() < 2000) {
                s.prev();
            } else {
                s.seek(0);
                s.play();
            }
        });
        mPrevButton->setRepeatListener(
                [this](View&, long howlong, int repcnt) {
                    scanBackward(repcnt, howlong);
                }, 260);

        mPauseButton = dynamic_cast<ImageButton*>(findViewById(R::id::pause));
        mPauseButton->setOnClickListener([this](View&) { doPauseResume(); });

        mNextButton = dynamic_cast<RepeatingImageButton*>(findViewById(R::id::next));
        mNextButton->setOnClickListener([this](View&) { Service::getInstance().next(); });
        mNextButton->setRepeatListener(
                [this](View&, long howlong, int repcnt) {
                    scanForward(repcnt, howlong);
                }, 260);

        mQueueButton = dynamic_cast<ImageButton*>(findViewById(R::id::curplaylist));
        mQueueButton->setOnClickListener([](View&) {
            Intent intent;
            intent.setClassName("cdroid.music", "TrackBrowserActivity")
                  .setAction(Intent::ACTION_EDIT)
                  .putExtra("playlist", std::string("nowplaying"));
            App::getInstance().startActivity(intent);
        });
        mShuffleButton = dynamic_cast<ImageButton*>(findViewById(R::id::shuffle));
        mShuffleButton->setOnClickListener([this](View&) { toggleShuffle(); });
        mRepeatButton = dynamic_cast<ImageButton*>(findViewById(R::id::repeat));
        mRepeatButton->setOnClickListener([this](View&) { cycleRepeat(); });

        if (auto* seeker = dynamic_cast<SeekBar*>(mProgress)) {
            SeekBar::OnSeekBarChangeListener l;
            l.onStartTrackingTouch = [this](SeekBar&) {
                mLastSeekEventTime = 0;
                mFromTouch = true;
            };
            l.onProgressChanged = [this](SeekBar&, int progress, bool fromUser) {
                if (!fromUser) return;
                long now = SystemClock::elapsedRealtime();
                if ((now - mLastSeekEventTime) > 250) {
                    mLastSeekEventTime = now;
                    mPosOverride = mDuration * progress / 1000;
                    Service::getInstance().seek(mPosOverride);
                }
            };
            l.onStopTrackingTouch = [this](SeekBar&) {
                mPosOverride = -1;
                mFromTouch = false;
            };
            seeker->setOnSeekBarChangeListener(l);
        }
        mProgress->setMax(1000);

        // Service events replace the original's BroadcastReceivers
        Service::getInstance().addListener(this, [this](const std::string& what) {
            if (what == Service::META_CHANGED) {
                updateTrackInfo();
                setPauseButtonImage();
                queueNextRefresh(1);
            } else if (what == Service::PLAYSTATE_CHANGED) {
                setPauseButtonImage();
                queueNextRefresh(1);
            }
        });
    }

    void onResume() override {
        Window::onResume();
        mPaused = false;
        updateTrackInfo();
        setPauseButtonImage();
        setShuffleButtonImage();
        setRepeatButtonImage();
        long next = refreshNow();
        queueNextRefresh(next);
    }

    void onPause() override {
        mPaused = true;
        removeCallbacks(mRefreshRunnable);
        Window::onPause();
    }

    void onDestroy() override {
        Service::getInstance().removeListener(this);
        Window::onDestroy();
    }

    bool onKeyDown(int keyCode, KeyEvent& event) override {
        switch (keyCode) {
        case KeyEvent::KEYCODE_DPAD_CENTER:
        case KeyEvent::KEYCODE_SPACE:
            doPauseResume();
            return true;
        case KeyEvent::KEYCODE_S:
            toggleShuffle();
            return true;
        }
        return Window::onKeyDown(keyCode, event);
    }

    bool onCreateOptionsMenu(Menu& menu) override {
        if (Service::getInstance().getAudioId() >= 0) {
            menu.add(0, Defs::GOTO_START, 0,
                    getContext()->getString(R::string::goto_start))
                    ->setIcon(R::drawable::ic_menu_music_library);
            menu.add(0, Defs::PARTY_SHUFFLE, 0,
                    getContext()->getString(R::string::party_shuffle));
        }
        return true;
    }

    bool onOptionsItemSelected(MenuItem& item) override {
        switch (item.getItemId()) {
        case Defs::GOTO_START: {
            Intent intent;
            intent.setClassName("cdroid.music", "MusicBrowserActivity")
                  .addFlags(Intent::FLAG_ACTIVITY_CLEAR_TOP | Intent::FLAG_ACTIVITY_NEW_TASK);
            App::getInstance().startActivity(intent);
            close();
            return true;
        }
        case Defs::PARTY_SHUFFLE:
            togglePartyShuffle();
            setShuffleButtonImage();
            return true;
        }
        return Window::onOptionsItemSelected(item);
    }

private:
    void doPauseResume() {
        Service& s = Service::getInstance();
        if (s.isPlaying()) {
            s.pause();
        } else {
            s.play();
        }
        refreshNow();
        setPauseButtonImage();
    }

    void toggleShuffle() {
        Service& s = Service::getInstance();
        const int shuffle = s.getShuffleMode();
        if (shuffle == Service::SHUFFLE_NONE) {
            s.setShuffleMode(Service::SHUFFLE_NORMAL);
            if (s.getRepeatMode() == Service::REPEAT_CURRENT) {
                s.setRepeatMode(Service::REPEAT_ALL);
                setRepeatButtonImage();
            }
            showToast(R::string::shuffle_on_notif);
        } else if (shuffle == Service::SHUFFLE_NORMAL || shuffle == Service::SHUFFLE_AUTO) {
            s.setShuffleMode(Service::SHUFFLE_NONE);
            showToast(R::string::shuffle_off_notif);
        }
        setShuffleButtonImage();
    }

    void cycleRepeat() {
        Service& s = Service::getInstance();
        const int mode = s.getRepeatMode();
        if (mode == Service::REPEAT_NONE) {
            s.setRepeatMode(Service::REPEAT_ALL);
            showToast(R::string::repeat_all_notif);
        } else if (mode == Service::REPEAT_ALL) {
            s.setRepeatMode(Service::REPEAT_CURRENT);
            if (s.getShuffleMode() != Service::SHUFFLE_NONE) {
                s.setShuffleMode(Service::SHUFFLE_NONE);
                setShuffleButtonImage();
            }
            showToast(R::string::repeat_current_notif);
        } else {
            s.setRepeatMode(Service::REPEAT_NONE);
            showToast(R::string::repeat_off_notif);
        }
        setRepeatButtonImage();
    }

    void showToast(int resid) {
        Toast::makeText(getContext(), getContext()->getString(resid), Toast::LENGTH_SHORT)
                ->show();
    }

    void scanBackward(int repcnt, long delta) {
        Service& s = Service::getInstance();
        if (repcnt == 0) {
            mStartSeekPos = s.position();
            mLastSeekEventTime = 0;
            return;
        }
        if (delta < 5000) delta *= 10;
        else delta = 50000 + (delta - 5000) * 40;
        long newpos = mStartSeekPos - delta;
        if (newpos < 0) {
            s.prev();
            const long duration = s.duration();
            mStartSeekPos += duration;
            newpos += duration;
        }
        if (((delta - mLastSeekEventTime) > 250) || repcnt < 0) {
            s.seek(newpos);
            mLastSeekEventTime = delta;
        }
        mPosOverride = repcnt >= 0 ? newpos : -1;
        refreshNow();
    }

    void scanForward(int repcnt, long delta) {
        Service& s = Service::getInstance();
        if (repcnt == 0) {
            mStartSeekPos = s.position();
            mLastSeekEventTime = 0;
            return;
        }
        if (delta < 5000) delta *= 10;
        else delta = 50000 + (delta - 5000) * 40;
        long newpos = mStartSeekPos + delta;
        const long duration = s.duration();
        if (newpos >= duration) {
            s.next();
            mStartSeekPos -= duration;
            newpos -= duration;
        }
        if (((delta - mLastSeekEventTime) > 250) || repcnt < 0) {
            s.seek(newpos);
            mLastSeekEventTime = delta;
        }
        mPosOverride = repcnt >= 0 ? newpos : -1;
        refreshNow();
    }

    void setRepeatButtonImage() {
        switch (Service::getInstance().getRepeatMode()) {
        case Service::REPEAT_ALL:
            mRepeatButton->setImageResource(R::drawable::ic_mp_repeat_all_btn);
            break;
        case Service::REPEAT_CURRENT:
            mRepeatButton->setImageResource(R::drawable::ic_mp_repeat_once_btn);
            break;
        default:
            mRepeatButton->setImageResource(R::drawable::ic_mp_repeat_off_btn);
            break;
        }
    }

    void setShuffleButtonImage() {
        switch (Service::getInstance().getShuffleMode()) {
        case Service::SHUFFLE_NONE:
            mShuffleButton->setImageResource(R::drawable::ic_mp_shuffle_off_btn);
            break;
        case Service::SHUFFLE_AUTO:
            mShuffleButton->setImageResource(R::drawable::ic_mp_partyshuffle_on_btn);
            break;
        default:
            mShuffleButton->setImageResource(R::drawable::ic_mp_shuffle_on_btn);
            break;
        }
    }

    void setPauseButtonImage() {
        mPauseButton->setImageResource(Service::getInstance().isPlaying()
                ? internal::R::drawable::ic_media_pause
                : internal::R::drawable::ic_media_play);
    }

    void queueNextRefresh(long delay) {
        if (mPaused) return;
        removeCallbacks(mRefreshRunnable);
        mRefreshRunnable = Runnable([this] {
            long next = refreshNow();
            queueNextRefresh(next);
        });
        postDelayed(mRefreshRunnable, delay);
    }

    long refreshNow() {
        Service& s = Service::getInstance();
        if (!s.isPlaying() && mPosOverride < 0 && s.position() == 0 && mDuration <= 0) {
            return 500;
        }
        const long pos = mPosOverride < 0 ? s.position() : mPosOverride;
        if ((pos >= 0) && (mDuration > 0)) {
            mCurrentTime->setText(makeTimeString(*getContext(), pos / 1000));
            const int progress = (int)(1000 * pos / mDuration);
            mProgress->setProgress(progress);

            if (s.isPlaying()) {
                mCurrentTime->setVisibility(View::VISIBLE);
            } else {
                // blink the counter
                const int vis = mCurrentTime->getVisibility();
                mCurrentTime->setVisibility(
                        vis == View::INVISIBLE ? View::VISIBLE : View::INVISIBLE);
                return 500;
            }
        } else {
            mCurrentTime->setText("--:--");
            mProgress->setProgress(1000);
        }
        // ms until the next full second, so the counter moves at the right time
        long remaining = 1000 - (pos % 1000);
        const int width = mProgress->getWidth();
        if (width == 0) return remaining > 500 ? 500 : remaining;
        long smoothrefreshtime = mDuration / width;
        if (smoothrefreshtime > remaining) return remaining;
        if (smoothrefreshtime < 20) return 20;
        return smoothrefreshtime;
    }

    void updateTrackInfo() {
        Service& s = Service::getInstance();
        if (!s.isInitialized()) {
            // nothing loaded — like the original osc callback when the service
            // has nothing: bounce to the browser
            Intent intent;
            intent.setClassName("cdroid.music", "MusicBrowserActivity")
                  .setAction(Intent::ACTION_MAIN)
                  .addFlags(Intent::FLAG_ACTIVITY_NEW_TASK);
            App::getInstance().startActivity(intent);
            close();
            return;
        }
        mArtistName->getParent()->setVisibility(View::VISIBLE);
        mAlbumName->getParent()->setVisibility(View::VISIBLE);
        mArtistName->setText(s.getArtistName());
        mAlbumName->setText(s.getAlbumName());
        mTrackName->setText(s.getTrackName());
        // facade: static default album art (no art decode worker)
        mAlbum->setImageResource(R::drawable::albumart_mp_unknown);
        mAlbum->setVisibility(View::VISIBLE);
        mDuration = s.duration();
        mTotalTime->setText(makeTimeString(*getContext(), mDuration / 1000));
        mRepeatButton->setVisibility(View::VISIBLE);
        mShuffleButton->setVisibility(View::VISIBLE);
        mQueueButton->setVisibility(View::VISIBLE);
    }

    ImageView* mAlbum = nullptr;
    TextView* mCurrentTime = nullptr;
    TextView* mTotalTime = nullptr;
    TextView* mArtistName = nullptr;
    TextView* mAlbumName = nullptr;
    TextView* mTrackName = nullptr;
    ProgressBar* mProgress = nullptr;
    RepeatingImageButton* mPrevButton = nullptr;
    ImageButton* mPauseButton = nullptr;
    RepeatingImageButton* mNextButton = nullptr;
    ImageButton* mRepeatButton = nullptr;
    ImageButton* mShuffleButton = nullptr;
    ImageButton* mQueueButton = nullptr;

    long mPosOverride = -1;
    bool mFromTouch = false;
    long mDuration = 0;
    long mStartSeekPos = 0;
    long mLastSeekEventTime = 0;
    bool mPaused = false;
    Runnable mRefreshRunnable;
};

REGISTER_ACTIVITY(MediaPlaybackActivity);

} // namespace music
} // namespace cdroid
