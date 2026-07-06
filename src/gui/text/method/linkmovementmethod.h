/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __LINK_MOVEMENT_METHOD_H__
#define __LINK_MOVEMENT_METHOD_H__
#include <text/method/scrollingmovementmethod.h>
namespace cdroid{
class LinkMovementMethod:public ScrollingMovementMethod {
private:
    static constexpr int CLICK = 1;
    static constexpr int UP = 2;
    static constexpr int DOWN = 3;
    static constexpr int HIDE_FLOATING_TOOLBAR_DELAY_MS = 200;
    static NoCopySpan FROM_BELOW;
private:
    bool action(int what, TextView& widget, Spannable& buffer);
protected:
    bool handleMovementKey(TextView& widget, Spannable& buffer, int keyCode,
            int movementMetaState, KeyEvent& event) override;
    bool up(TextView& widget, Spannable& buffer)override;
    bool down(TextView& widget, Spannable& buffer)override;
    bool left(TextView& widget, Spannable& buffer)override;
    bool right(TextView& widget, Spannable& buffer)override;
public:
    bool canSelectArbitrarily() const override{
        return true;
    }

    bool onTouchEvent(TextView& widget, Spannable& buffer,MotionEvent& event)override;

    void initialize(TextView& widget, Spannable& text)override;

    void onTakeFocus(TextView& view, Spannable& text, int dir)override;

    static MovementMethod* getInstance();
};
}/*endof namespace*/
#endif/*__LINK_MOVEMENT_METHOD_H__*/
