#ifndef __MEDIA_CONTROLLER_H__
#define __MEDIA_CONTROLLER_H__
#include <widget/framelayout.h>
#include <widget/textview.h>
#include <widget/imagebutton.h>
#include <widget/seekbar.h>

namespace cdroid{
class MediaController :public FrameLayout{
public:
    struct MediaPlayerControl {
        std::function<void()>start;
        std::function<void()>pause;
        std::function<int()> getDuration;
        std::function<int()>getCurrentPosition;
        std::function<void(int)>seekTo;
        std::function<bool()> isPlaying;
        std::function<int()>getBufferPercentage;
        std::function<bool()>canPause;
        std::function<bool()>canSeekBackward;
        std::function<bool()>canSeekForward;
        /** Get the audio session id for the player used by this VideoView. This can be used to
         * apply audio effects to the audio track of a video.
         * @return The audio session, or 0 if there was an error.*/
        std::function<int()>getAudioSessionId;
    };
private:
    static constexpr int sDefaultTimeout = 3000;
    MediaPlayerControl mPlayer;
    View* mAnchor;
    View* mRoot;
    Window* mWindow;
    View* mDecor;
    //WindowManager.LayoutParams mDecorLayoutParams;
    ProgressBar* mProgress;
    TextView* mEndTime;
    TextView* mCurrentTime;
    bool mShowing;
    bool mDragging;
    bool mUseFastForward;
    bool mFromXml;
    bool mListenersSet;
    OnClickListener mPauseListener;
    OnClickListener mNextListener, mPrevListener;
    OnClickListener mRewListener, mFfwdListener;
    OnLayoutChangeListener mLayoutChangeListener;
    SeekBar::OnSeekBarChangeListener mSeekListener;
    OnTouchListener mTouchListener;
    Runnable mFadeOut;
    Runnable mShowProgress;
    //StringBuilder mFormatBuilder;
    //Formatter mFormatter;
    ImageButton* mPauseButton;
    ImageButton* mFfwdButton;
    ImageButton* mRewButton;
    ImageButton* mNextButton;
    ImageButton* mPrevButton;
    std::string mPlayDescription;
    std::string mPauseDescription;
    bool mBackCallbackRegistered;
    /** Handles back invocation */
private:
    void initFloatingWindow();
    void initFloatingWindowLayout();
    void updateFloatingWindowLayout();
    void initControllerView(View* v);
    void disableUnsupportedButtons();
    int setProgress();
    void updatePausePlay();
    void doPauseResume();
    void installPrevNextListeners();
    void unregisterOnBackInvokedCallback();
    void registerOnBackInvokedCallback();
protected:
    View* makeControllerView();
public:
    MediaController(Context* context,const AttributeSet& attrs);
    MediaController(Context* context, bool useFastForward);
    void onFinishInflate()override;
    void setMediaPlayer(MediaPlayerControl player);
    void setAnchorView(View* view);
    void show();
    void show(int timeout);
    bool isShowing()const;
    void hide();
    bool onTouchEvent(MotionEvent& event)override;
    bool onTrackballEvent(MotionEvent& ev)override;
    bool dispatchKeyEvent(KeyEvent& event)override;
    void setEnabled(bool enabled)override;
    void setPrevNextListeners(const OnClickListener& next,const OnClickListener& prev);
};
}/*endof namespace*/
#endif//__MEDIA_CONTROLLER_H__
