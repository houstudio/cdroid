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
#ifndef __WEAR_DISMISS_CONTROLLER_H__
#define __WEAR_DISMISS_CONTROLLER_H__
#include <core/context.h>
namespace cdroid{
class DismissibleFrameLayout;
class DismissController {
public:
    struct OnDismissListener {
        std::function<void()> onDismissStarted;
        std::function<void()> onDismissCanceled;
        std::function<void()> onDismissed;
    };
protected:
    Context* mContext;
    DismissibleFrameLayout* mLayout;
    OnDismissListener mDismissListener;
public:
    DismissController(Context* context, DismissibleFrameLayout* layout) {
        mContext = context;
        mLayout = layout;
    }
    virtual ~DismissController()=default;
    void setOnDismissListener(const OnDismissListener& listener) {
        mDismissListener = listener;
    }
};
}/*endof namespace*/
#endif/*__WEAR_DISMISS_CONTROLLER_H__*/
