#include <widget/R.h>
#include <widget/textview.h>
#include <widget/imageview.h>
#include <widget/cdwindow.h>
#include <widgetEx/wear/confirmationoverlay.h>
namespace cdroid{

ConfirmationOverlay::ConfirmationOverlay(){
    mIsShowing = false;
    mType = SUCCESS_ANIMATION;
    mDurationMillis = DEFAULT_ANIMATION_DURATION_MS;
    mOverlayView = nullptr;
    mOverlayDrawable = nullptr;
    mMainThreadHandler = new Handler();//Looper::getMainLooper());
    mHideRunnable = [this](){hide();};
}

ConfirmationOverlay::ConfirmationOverlay(const std::string &message)
    :ConfirmationOverlay(){
    mMessage = message;
}

ConfirmationOverlay::~ConfirmationOverlay(){
    delete mOverlayView;
    delete mMainThreadHandler;
}

ConfirmationOverlay& ConfirmationOverlay::setMessage(const std::string&message) {
    mMessage = message;
    return *this;
}

ConfirmationOverlay& ConfirmationOverlay::setType(int type) {
    mType = type;
    return *this;
}

ConfirmationOverlay& ConfirmationOverlay::setDuration(int millis) {
    mDurationMillis = millis;
    return *this;
}

ConfirmationOverlay& ConfirmationOverlay::setFinishedAnimationListener(const OnAnimationFinishedListener& listener) {
    mListener = listener;
    return *this;
}

void ConfirmationOverlay::showAbove(View* view) {
    if (mIsShowing) {
        return;
    }
    mIsShowing = true;

    updateOverlayView(view->getContext());
    ((ViewGroup*) view->getRootView())->addView(mOverlayView);
    animateAndHideAfterDelay();
}

void ConfirmationOverlay::showOn(Window* activity) {
    if (mIsShowing) {
        return;
    }
    mIsShowing = true;

    updateOverlayView(activity->getContext());
    //activity.getWindow().addContentView(mOverlayView, mOverlayView->getLayoutParams());
    activity->addView(mOverlayView,mOverlayView->getLayoutParams());
    animateAndHideAfterDelay();
}

void ConfirmationOverlay::animateAndHideAfterDelay() {
    if (dynamic_cast<Animatable*>(mOverlayDrawable)) {
        Animatable* animatable = (Animatable*) mOverlayDrawable;
        animatable->start();
        LOGD("animatable=%p",animatable);
    }
    mMainThreadHandler->postDelayed(mHideRunnable, mDurationMillis);
}

/**
 * Starts a fadeout animation and removes the view once finished. This is invoked by {@link
 * #mHideRunnable} after {@link #mDurationMillis} milliseconds.
 */
void ConfirmationOverlay::hide() {
    Animation* fadeOut = AnimationUtils::loadAnimation(mOverlayView->getContext(), "cdroid:anim/fade_out");
    Animation::AnimationListener al;
    mOverlayView->clearAnimation();
    al.onAnimationStart=[this](Animation& animation){
        //mOverlayView->clearAnimation();/*this line maybe google's bug*/
    };
    al.onAnimationEnd = [this](Animation& animation){
        ((ViewGroup*) mOverlayView->getParent())->removeView(mOverlayView);
        mIsShowing = false;
        if (mListener != nullptr) {
            mListener/*.onAnimationFinished*/();
        }
    };
    fadeOut->setAnimationListener(al);
    mOverlayView->startAnimation(fadeOut);
}

void ConfirmationOverlay::updateOverlayView(Context* context) {
    if (mOverlayView == nullptr) {
        //noinspection InflateParams
        mOverlayView = LayoutInflater::from(context)->inflate("cdroid:layout/ws_overlay_confirmation", nullptr);
    }
    mOverlayView->setOnTouchListener([](View& v, MotionEvent& event) {
        return true;
    });
    mOverlayView->setLayoutParams(new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT));

    updateImageView(context, mOverlayView);
    updateMessageView(context, mOverlayView);
}

void ConfirmationOverlay::updateMessageView(Context* context, View* overlayView) {
    TextView* messageView =  (TextView*)overlayView->findViewById(R::id::wearable_support_confirmation_overlay_message);

    if (!mMessage.empty()) {
        const int screenWidthPx = context->getDisplayMetrics().widthPixels;// ResourcesUtil.getScreenWidthPx(context);
        const int topMarginPx = 0;//ResourcesUtil.getFractionOfScreenPx(context, screenWidthPx, R.fraction.confirmation_overlay_margin_above_text);
        const int sideMarginPx= 0;//ResourcesUtil.getFractionOfScreenPx(context, screenWidthPx, R.fraction.confirmation_overlay_margin_side);

        MarginLayoutParams* layoutParams = (MarginLayoutParams*) messageView->getLayoutParams();
        layoutParams->topMargin = topMarginPx;
        layoutParams->leftMargin = sideMarginPx;
        layoutParams->rightMargin = sideMarginPx;

        messageView->setLayoutParams(layoutParams);
        messageView->setText(mMessage);
        messageView->setVisibility(View::VISIBLE);

    } else {
        messageView->setVisibility(View::GONE);
    }
}

void ConfirmationOverlay::updateImageView(Context* context, View* overlayView) {
    switch (mType) {
    case SUCCESS_ANIMATION:
        mOverlayDrawable = context->getDrawable("@cdroid:drawable/generic_confirmation_animation");
        break;
    case FAILURE_ANIMATION:
        mOverlayDrawable = context->getDrawable("@cdroid:drawable/ws_full_sad");
        break;
    case OPEN_ON_PHONE_ANIMATION:
        mOverlayDrawable = context->getDrawable("@cdroid:drawable/ws_open_on_phone_animation");
        break;
    default:
        LOGE("Invalid ConfirmationOverlay type [%d]", mType);
        break;
    }

    ImageView* imageView = (ImageView*)overlayView->findViewById(R::id::wearable_support_confirmation_overlay_image);
    imageView->setImageDrawable(mOverlayDrawable);
}
}/*endof namespace*/
