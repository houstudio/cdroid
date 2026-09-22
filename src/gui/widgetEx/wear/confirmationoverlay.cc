#include <algorithm>
#include <stdexcept>
#include <widget/internal_R.h>
#include <widgetEx/widgetex_styleable.h>
#include <widget/textview.h>
#include <widget/imageview.h>
#include <widget/cdwindow.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitymanager.h>
#include <widgetEx/wear/resourcesutil.h>
#include <widgetEx/wear/confirmationoverlay.h>
namespace cdroid{
using namespace cdroid::internal;

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
    // Java GC reclaims a still-attached overlay; C++ must detach it from its
    // host ViewGroup first or the host is left with a dangling child.
    if (mOverlayView != nullptr && mOverlayView->getParent() != nullptr) {
        // While the fade-out is running, ViewGroup::removeView parks the view
        // in mDisappearingChildren (animation attached) and the delete below
        // would leave that list dangling. Cancel the fade first so removeView
        // takes the plain detach branch.
        mOverlayView->clearAnimation();
        ((ViewGroup*)mOverlayView->getParent())->removeView(mOverlayView);
    }
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

ConfirmationOverlay& ConfirmationOverlay::setOnAnimationFinishedListener(const OnAnimationFinishedListener& listener) {
    // HEAD rename (ConfirmationOverlay.java:214-218); the deprecated name forwards.
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
    setUpForAccessibility();
    animateAndHideAfterDelay();
}

void ConfirmationOverlay::showOn(Window* activity) {
    if (mIsShowing) {
        return;
    }
    mIsShowing = true;

    updateOverlayView(activity->getContext());
    activity->addView(mOverlayView,mOverlayView->getLayoutParams());
    setUpForAccessibility();
    animateAndHideAfterDelay();
}

// java:254-258
void ConfirmationOverlay::setUpForAccessibility() {
    mOverlayView->setContentDescription(getAccessibilityText());
    mOverlayView->requestFocus();
    mOverlayView->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_FOCUSED);
}

// java:264-269 — CDROID's AccessibilityManager is the in-process singleton
// (no getSystemService).
int ConfirmationOverlay::getDurationMillis() {
    if (AccessibilityManager::getInstance(mOverlayView->getContext()).isEnabled()) {
        return std::max(A11Y_ANIMATION_DURATION_MS, mDurationMillis);
    }
    return mDurationMillis;
}

void ConfirmationOverlay::animateAndHideAfterDelay() {
    auto animatable = dynamic_cast<Animatable*>(mOverlayDrawable);
    if (animatable!=nullptr) {
        animatable->start();
    }
    mMainThreadHandler->postDelayed(mHideRunnable, getDurationMillis());
}

/**
 * Starts a fadeout animation and removes the view once finished. This is invoked by {@link
 * #mHideRunnable} after {@link #mDurationMillis} milliseconds.
 */
void ConfirmationOverlay::hide() {
    Animation* fadeOut = AnimationUtils::loadAnimation(mOverlayView->getContext(), R::anim::fade_out);
    Animation::AnimationListener al;
    // java:293-296 calls clearAnimation() inside onAnimationStart; CDROID's
    // View::clearAnimation deletes the animation, so firing it inside the
    // fade's own start callback would delete the fade mid-fire. Clearing any
    // previous animation eagerly at hide() entry is the same cleanup without
    // the self-destruction.
    mOverlayView->clearAnimation();
    al.onAnimationStart=[this](Animation& animation){
        //mOverlayView->clearAnimation();/*see the comment above*/
    };
    al.onAnimationEnd = [this](Animation& animation){
        ViewGroup* parent = (ViewGroup*) mOverlayView->getParent();
        parent->removeView(mOverlayView);
        mIsShowing = false;
        mOverlayView->clearFocus();
        // java:302-304 invokes the listener inline (before clearFocus());
        // posted to the host instead because this callback fires mid-draw
        // (View::draw -> getTransformation), and a listener that destroys its
        // ConfirmationOverlay — the natural fire-and-forget cleanup — would
        // free mOverlayView while the draw stack above still walks it. One
        // message-loop hop lets that stack unwind first; Java relies on GC.
        OnAnimationFinishedListener listener = mListener;
        if (listener != nullptr) {
            parent->post([listener](){ listener(); });
        }
    };
    fadeOut->setAnimationListener(al);
    mOverlayView->startAnimation(fadeOut);
}

void ConfirmationOverlay::updateOverlayView(Context* context) {
    if (mOverlayView == nullptr) {
        //noinspection InflateParams
        mOverlayView = LayoutInflater::from(context)->inflate(
                R::layout::ws_overlay_confirmation, nullptr);
    }
    mOverlayView->setOnTouchListener([](View& v, MotionEvent& event) {
        return true;
    });
    mOverlayView->setLayoutParams(new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT));

    updateImageView(context, mOverlayView);
    updateMessageView(context, mOverlayView);
}

void ConfirmationOverlay::updateMessageView(Context* context, View* overlayView) {
    TextView* messageView =  (TextView*)overlayView->findViewById(
            R::id::wearable_support_confirmation_overlay_message);

    // Remove the message view from the view hierarchy if there's no message, as it changes the
    // vertical alignment of other content in the overlay.
    if (mMessage.empty()) {
        messageView->setVisibility(View::GONE);
    } else {
        const int screenWidthPx = ResourcesUtil::getScreenWidthPx(*context);
        const int insetMarginPx = ResourcesUtil::getFractionOfScreenPx(*context, screenWidthPx,
                (int) R::fraction::confirmation_overlay_text_inset_margin);

        MarginLayoutParams* layoutParams = (MarginLayoutParams*) messageView->getLayoutParams();
        layoutParams->leftMargin = insetMarginPx;
        layoutParams->rightMargin = insetMarginPx;

        messageView->setLayoutParams(layoutParams);
        messageView->setText(mMessage);
        messageView->setVisibility(View::VISIBLE);
    }
}

void ConfirmationOverlay::updateImageView(Context* context, View* overlayView) {
    switch (mType) {
    case SUCCESS_ANIMATION:
        mOverlayDrawable = context->getDrawable(R::drawable::confirmation_animation);
        break;
    case FAILURE_ANIMATION:
        mOverlayDrawable = context->getDrawable(R::drawable::failure_animation);
        break;
    case OPEN_ON_PHONE_ANIMATION:
        mOverlayDrawable = context->getDrawable(R::drawable::open_on_phone_animation);
        break;
    default:
        // java:374-376 throws IllegalStateException; the invalid type has no
        // drawable to show.
        throw std::runtime_error(
                "Invalid ConfirmationOverlay type [" + std::to_string(mType) + "]");
    }

    ImageView* imageView = (ImageView*)overlayView->findViewById(
            R::id::wearable_support_confirmation_overlay_image);
    imageView->setImageDrawable(mOverlayDrawable);
}

// java:384-414 — text to be read out if accessibility is turned on. The three
// a11y strings are pinned in wear res public.xml; until the next pak build
// picks them up, getString returns the empty string (safe degradation).
std::string ConfirmationOverlay::getAccessibilityText() {
    if (!mMessage.empty()) {
        return mMessage;
    }
    Context* context = mOverlayView->getContext();
    switch (mType) {
        case SUCCESS_ANIMATION:
            return context->getString((int) R::string::confirmation_overlay_a11y_description_success);
        case FAILURE_ANIMATION:
            return context->getString((int) R::string::confirmation_overlay_a11y_description_fail);
        case OPEN_ON_PHONE_ANIMATION:
            return context->getString((int) R::string::confirmation_overlay_a11y_description_phone);
        default:
            // java:409-411
            throw std::runtime_error(
                    "Invalid ConfirmationOverlay type [" + std::to_string(mType) + "]");
    }
}
}/*endof namespace*/
