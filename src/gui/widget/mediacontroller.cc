#if 10
#include <widget/mediacontroller.h>
#include <widget/cdwindow.h>
#include <widget/R.h>
namespace cdroid{

DECLARE_WIDGET(MediaController)

MediaController::MediaController(Context* context,const AttributeSet& attrs)
 :FrameLayout(context,attrs){
    mRoot = this;
    mUseFastForward = true;
    mFromXml = true;
}

MediaController::MediaController(Context* context, bool useFastForward)
  :FrameLayout(context,AttributeSet()){
    mContext = context;
    mUseFastForward = useFastForward;
    initFloatingWindowLayout();
    initFloatingWindow();
}

void MediaController::onFinishInflate() {
    if (mRoot)
        initControllerView(mRoot);
}

void MediaController::initFloatingWindow() {
    //mWindowManager = (WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE);
    mWindow = new Window(0,0,-1,-1);
    //mWindow.setWindowManager(mWindowManager, null, null);
    //mWindow.requestFeature(Window.FEATURE_NO_TITLE);
    //mDecor = mWindow.getDecorView();
    //mDecor.setOnTouchListener(mTouchListener);
    //mDecor.addOnAttachStateChangeListener(mAttachStateListener);
    //mWindow.setContentView(this);
    mWindow->setBackgroundResource("@null");

    // While the media controller is up, the volume control keys should
    // affect the media stream type
    //mWindow.setVolumeControlStream(AudioManager.STREAM_MUSIC);

    setFocusable(true);
    setFocusableInTouchMode(true);
    //setDescendantFocusability(ViewGroup::FOCUS_AFTER_DESCENDANTS);
    requestFocus();
}

void MediaController::initFloatingWindowLayout() {
    /*mDecorLayoutParams = new WindowManager.LayoutParams();
    WindowManager.LayoutParams p = mDecorLayoutParams;
    p.gravity = Gravity::TOP | Gravity::LEFT;
    p.height = LayoutParams::WRAP_CONTENT;
    p.x = 0;
    p.format = PixelFormat.TRANSLUCENT;
    p.type = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;
    p.flags |= WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM
            | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
            | WindowManager.LayoutParams.FLAG_SPLIT_TOUCH;
    p.token = null;
    p.windowAnimations = 0; // android.R.style.DropDownAnimationDown;*/
}

void MediaController::updateFloatingWindowLayout() {
    int anchorPos[2];
    mAnchor->getLocationOnScreen(anchorPos);

    // we need to know the size of the controller so we can properly position it
    // within its space
    mDecor->measure(MeasureSpec::makeMeasureSpec(mAnchor->getWidth(), MeasureSpec::AT_MOST),
            MeasureSpec::makeMeasureSpec(mAnchor->getHeight(), MeasureSpec::AT_MOST));
#if 0
    WindowManager.LayoutParams p = mDecorLayoutParams;
    p.width = mAnchor->getWidth();
    p.x = anchorPos[0] + (mAnchor.getWidth() - p.width) / 2;
    p.y = anchorPos[1] + mAnchor->getHeight() - mDecor->getMeasuredHeight();
    p.token = mAnchor.getWindowToken();
#endif
}


void MediaController::setMediaPlayer(MediaPlayerControl player) {
    mPlayer = player;
    updatePausePlay();
}

void MediaController::setAnchorView(View* view) {
    if (mAnchor != nullptr) {
        mAnchor->removeOnLayoutChangeListener(mLayoutChangeListener);
    }
    mAnchor = view;
    if (mAnchor) {
        mAnchor->addOnLayoutChangeListener(mLayoutChangeListener);
    }

    FrameLayout::LayoutParams* frameParams = new FrameLayout::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT,
            ViewGroup::LayoutParams::MATCH_PARENT
    );

    removeAllViews();
    View* v = makeControllerView();
    addView(v, frameParams);
}

View* MediaController::makeControllerView() {
    LayoutInflater* inflate = LayoutInflater::from(mContext);
    mRoot = inflate->inflate("cdroid:layout/media_controller", nullptr);
    initControllerView(mRoot);
    return mRoot;
}

void MediaController::initControllerView(View* v) {
    mPlayDescription = mContext->getString("cdroid:string/lockscreen_transport_play_description");
    mPauseDescription = mContext->getString("cdroid:string/lockscreen_transport_pause_description");
    mPauseButton = (ImageButton*)v->findViewById(cdroid::R::id::pause);
    if (mPauseButton) {
        mPauseButton->requestFocus();
        mPauseButton->setOnClickListener(mPauseListener);
    }

    mFfwdButton = (ImageButton*)v->findViewById(cdroid::R::id::ffwd);
    if (mFfwdButton) {
        mFfwdButton->setOnClickListener(mFfwdListener);
        if (!mFromXml) {
            mFfwdButton->setVisibility(mUseFastForward ? View::VISIBLE : View::GONE);
        }
    }

    mRewButton = (ImageButton*)v->findViewById(cdroid::R::id::rew);
    if (mRewButton) {
        mRewButton->setOnClickListener(mRewListener);
        if (!mFromXml) {
            mRewButton->setVisibility(mUseFastForward ? View::VISIBLE : View::GONE);
        }
    }

    // By default these are hidden. They will be enabled when setPrevNextListeners() is called
    mNextButton = (ImageButton*)v->findViewById(cdroid::R::id::next);
    if (mNextButton && !mFromXml && !mListenersSet) {
        mNextButton->setVisibility(View::GONE);
    }
    mPrevButton = (ImageButton*)v->findViewById(cdroid::R::id::prev);
    if (mPrevButton && !mFromXml && !mListenersSet) {
        mPrevButton->setVisibility(View::GONE);
    }

    mProgress = (ProgressBar*)v->findViewById(cdroid::R::id::mediacontroller_progress);
    if (mProgress) {
        if (dynamic_cast<SeekBar*>(mProgress)) {
            SeekBar* seeker = (SeekBar*) mProgress;
            seeker->setOnSeekBarChangeListener(mSeekListener);
        }
        mProgress->setMax(1000);
    }

    mEndTime = (TextView*)v->findViewById(cdroid::R::id::time);
    mCurrentTime = (TextView*)v->findViewById(cdroid::R::id::time_current);
    //mFormatBuilder = new StringBuilder();
    //mFormatter = new Formatter(mFormatBuilder, Locale.getDefault());

    installPrevNextListeners();
}

void MediaController::show() {
    show(sDefaultTimeout);
}

void MediaController::disableUnsupportedButtons() {
    if (mPauseButton && !mPlayer.canPause()) {
        mPauseButton->setEnabled(false);
    }
    if (mRewButton && !mPlayer.canSeekBackward()) {
        mRewButton->setEnabled(false);
    }
    if (mFfwdButton && !mPlayer.canSeekForward()) {
        mFfwdButton->setEnabled(false);
    }
    // TODO What we really should do is add a canSeek to the MediaPlayerControl interface;
    // this scheme can break the case when applications want to allow seek through the
    // progress bar but disable forward/backward buttons.
    //
    // However, currently the flags SEEK_BACKWARD_AVAILABLE, SEEK_FORWARD_AVAILABLE,
    // and SEEK_AVAILABLE are all (un)set together; as such the aforementioned issue
    // shouldn't arise in existing applications.
    if (mProgress && !mPlayer.canSeekBackward() && !mPlayer.canSeekForward()) {
        mProgress->setEnabled(false);
    }
}

void MediaController::show(int timeout) {
    if (!mShowing && mAnchor) {
        setProgress();
        if (mPauseButton) {
            mPauseButton->requestFocus();
        }
        disableUnsupportedButtons();
        updateFloatingWindowLayout();
        //mWindowManager.addView(mDecor, mDecorLayoutParams);
        mShowing = true;
    }
    updatePausePlay();

    // cause the progress bar to be updated even if mShowing
    // was already true.  This happens, for example, if we're
    // paused with the progress bar showing the user hits play.
    post(mShowProgress);

    if (timeout != 0 /*&& !mAccessibilityManager.isTouchExplorationEnabled()*/) {
        removeCallbacks(mFadeOut);
        postDelayed(mFadeOut, timeout);
    }
    registerOnBackInvokedCallback();
}

bool MediaController::isShowing()const{
    return mShowing;
}

void MediaController::hide() {
    if (mAnchor == nullptr)
        return;
    if (mShowing) {
        removeCallbacks(mShowProgress);
        //mWindowManager.removeView(mDecor);
        mShowing = false;
        unregisterOnBackInvokedCallback();
    }
}

static std::string stringForTime(int timeMs) {
    int totalSeconds = timeMs / 1000;

    int seconds = totalSeconds % 60;
    int minutes = (totalSeconds / 60) % 60;
    int hours   = totalSeconds / 3600;
#if 0
    mFormatBuilder.setLength(0);
    if (hours > 0) {
        return mFormatter.format("%d:%02d:%02d", hours, minutes, seconds).toString();
    } else {
        return mFormatter.format("%02d:%02d", minutes, seconds).toString();
    }
#endif
    return "";
}

int MediaController::setProgress() {
    if (/*mPlayer == null ||*/ mDragging) {
        return 0;
    }
    const int position = mPlayer.getCurrentPosition();
    const int duration = mPlayer.getDuration();
    if (mProgress) {
        if (duration > 0) {
            // use long to avoid overflow
            const long pos = 1000L * position / duration;
            mProgress->setProgress( (int) pos);
        }
        int percent = mPlayer.getBufferPercentage();
        mProgress->setSecondaryProgress(percent * 10);
    }
    if (mEndTime)
        mEndTime->setText(stringForTime(duration));
    if (mCurrentTime)
        mCurrentTime->setText(stringForTime(position));
    return position;
}

bool MediaController::onTouchEvent(MotionEvent& event) {
    switch (event.getAction()) {
    case MotionEvent::ACTION_DOWN:
         show(0); // show until hide is called
         break;
    case MotionEvent::ACTION_UP:
         show(sDefaultTimeout); // start timeout
          break;
    case MotionEvent::ACTION_CANCEL:
          hide();
          break;
    default:
          break;
    }
    return true;
}

bool MediaController::onTrackballEvent(MotionEvent& ev) {
    show(sDefaultTimeout);
    return false;
}

bool MediaController::dispatchKeyEvent(KeyEvent& event) {
    const int keyCode = event.getKeyCode();
    bool uniqueDown = event.getRepeatCount() == 0
            && event.getAction() == KeyEvent::ACTION_DOWN;
    if (keyCode ==  KeyEvent::KEYCODE_HEADSETHOOK
            || keyCode == KeyEvent::KEYCODE_MEDIA_PLAY_PAUSE
            || keyCode == KeyEvent::KEYCODE_SPACE) {
        if (uniqueDown) {
            doPauseResume();
            show(sDefaultTimeout);
            if (mPauseButton) {
                mPauseButton->requestFocus();
            }
        }
        return true;
    } else if (keyCode == KeyEvent::KEYCODE_MEDIA_PLAY) {
        if (uniqueDown && !mPlayer.isPlaying()) {
            mPlayer.start();
            updatePausePlay();
            show(sDefaultTimeout);
        }
        return true;
    } else if (keyCode ==KeyEvent::KEYCODE_MEDIA_STOP
            || keyCode == KeyEvent::KEYCODE_MEDIA_PAUSE) {
        if (uniqueDown && mPlayer.isPlaying()) {
            mPlayer.pause();
            updatePausePlay();
            show(sDefaultTimeout);
        }
        return true;
    } else if (keyCode == KeyEvent::KEYCODE_VOLUME_DOWN
            || keyCode == KeyEvent::KEYCODE_VOLUME_UP
            || keyCode == KeyEvent::KEYCODE_VOLUME_MUTE
            || keyCode == KeyEvent::KEYCODE_CAMERA) {
        // don't show the controls for volume adjustment
        return FrameLayout::dispatchKeyEvent(event);
    } else if (keyCode == KeyEvent::KEYCODE_BACK || keyCode == KeyEvent::KEYCODE_MENU) {
        if (uniqueDown) {
            hide();
        }
        return true;
    }

    show(sDefaultTimeout);
    return FrameLayout::dispatchKeyEvent(event);
}

void MediaController::updatePausePlay() {
    if (mRoot == nullptr || mPauseButton == nullptr)
        return;

    if (mPlayer.isPlaying()) {
        mPauseButton->setImageResource("cdroid:drawable/ic_media_pause");
        mPauseButton->setContentDescription(mPauseDescription);
    } else {
        mPauseButton->setImageResource("cdroid:drawable/ic_media_play");
        mPauseButton->setContentDescription(mPlayDescription);
    }
}

void MediaController::doPauseResume() {
    if (mPlayer.isPlaying()) {
        mPlayer.pause();
    } else {
        mPlayer.start();
    }
    updatePausePlay();
}

void MediaController::setEnabled(bool enabled) {
    if (mPauseButton) mPauseButton->setEnabled(enabled);
    if (mFfwdButton) mFfwdButton->setEnabled(enabled);
    if (mRewButton) mRewButton->setEnabled(enabled);
    if (mNextButton) mNextButton->setEnabled(enabled && mNextListener != nullptr);
    if (mPrevButton) mPrevButton->setEnabled(enabled && mPrevListener != nullptr);
    if (mProgress) mProgress->setEnabled(enabled);
    disableUnsupportedButtons();
    FrameLayout::setEnabled(enabled);
}

void MediaController::installPrevNextListeners() {
    if (mNextButton) {
        mNextButton->setOnClickListener(mNextListener);
        mNextButton->setEnabled(mNextListener != nullptr);
    }

    if (mPrevButton) {
        mPrevButton->setOnClickListener(mPrevListener);
        mPrevButton->setEnabled(mPrevListener != nullptr);
    }
}

void MediaController::setPrevNextListeners(const View::OnClickListener& next,const View::OnClickListener& prev) {
    mNextListener = next;
    mPrevListener = prev;
    mListenersSet = true;

    if (mRoot != nullptr) {
        installPrevNextListeners();
        if (mNextButton && !mFromXml) {
            mNextButton->setVisibility(View::VISIBLE);
        }
        if (mPrevButton && !mFromXml) {
            mPrevButton->setVisibility(View::VISIBLE);
        }
    }
}

void MediaController::unregisterOnBackInvokedCallback() {
    if (!mBackCallbackRegistered) {
        return;
    }
#if 0
    ViewRootImpl viewRootImpl = mDecor->getViewRootImpl();
    if (viewRootImpl != null
            && viewRootImpl.getOnBackInvokedDispatcher().isOnBackInvokedCallbackEnabled()) {
        viewRootImpl.getOnBackInvokedDispatcher()
                .unregisterOnBackInvokedCallback(mBackCallback);
    }
#endif
    mBackCallbackRegistered = false;
}

void MediaController::registerOnBackInvokedCallback() {
    if (mBackCallbackRegistered) {
        return;
    }
#if 0
    ViewRootImpl viewRootImpl = mDecor.getViewRootImpl();
    if (viewRootImpl != null
            && viewRootImpl.getOnBackInvokedDispatcher().isOnBackInvokedCallbackEnabled()) {
        viewRootImpl.getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                OnBackInvokedDispatcher.PRIORITY_DEFAULT, mBackCallback);
        mBackCallbackRegistered = true;
    }
#endif
}

}/*endof namespace*/
#endif
