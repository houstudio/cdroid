#include <widgetEx/wear/backbuttondismisscontroller.h>
#include <widgetEx/wear/dismissableframelayout.h>
namespace cdroid{

BackButtonDismissController::BackButtonDismissController(Context* context, DismissibleFrameLayout* layout)
    :DismissController(context, layout){

    // set this to true will also ensure that this view is focusable
    layout->setFocusableInTouchMode(true);
    // Dismiss upon back button press
    layout->requestFocus();
    layout->setOnKeyListener([this](View&e,int keyCode,KeyEvent&event){
        return (keyCode==KeyEvent::KEYCODE_BACK) &&
            (event.getAction() == KeyEvent::ACTION_UP) &&
            dismiss();
    });
}

void BackButtonDismissController::disable(DismissibleFrameLayout* layout) {
    setOnDismissListener({});//null);
    layout->setOnKeyListener({});//null);
    // setting this to false will also ensure that this view is not focusable in touch mode
    layout->setFocusable(false);
    layout->clearFocus();
}

bool BackButtonDismissController::dismiss() {
    if ((mDismissListener.onDismissStarted==nullptr)||
            (mDismissListener.onDismissed==nullptr)) return false;

    Animation* exitAnimation = ActivityAnimationUtil.getStandardActivityAnimation(
            mContext, ActivityAnimationUtil.CLOSE_EXIT,
            /* scaled by TRANSITION_ANIMATION_SCALE */true);
    if (exitAnimation != nullptr) {
        Animation::AnimationListener al;
        al.onAnimationStart=[this](Animation& animation){
            mDismissListener.onDismissStarted();
        };
        al.onAnimationEnd = [this](Animation& animation){
            mDismissListener.onDismissed();
        };
        exitAnimation->setAnimationListener(al);
        mLayout->startAnimation(exitAnimation);
    } else {
        mDismissListener.onDismissStarted();
        mDismissListener.onDismissed();
    }
    return true;
}
}/*endof namespace*/
