/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
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

    Animation* exitAnimation = nullptr;
     //ActivityAnimationUtil.getStandardActivityAnimation(
     //    mContext, ActivityAnimationUtil.CLOSE_EXIT,
     //    /*scaled by TRANSITION_ANIMATION_SCALE */true);
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
