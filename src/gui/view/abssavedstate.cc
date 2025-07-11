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
 *********************************************************************************/
#include <view/abssavedstate.h>
#include <porting/cdlog.h>

namespace cdroid{
    AbsSavedState AbsSavedState::EMPTY_STATE;

    AbsSavedState::AbsSavedState() {
        mSuperState = nullptr;
    }

    AbsSavedState::AbsSavedState(Parcelable* superState) {
        LOGE_IF(superState == nullptr,"superState must not be null");
        mSuperState = superState != &EMPTY_STATE ? superState : nullptr;
    }

    AbsSavedState::AbsSavedState(Parcel& source) {
        //this(source, null);
    }

    /*AbsSavedState(Parcel& source, ClassLoader loader) {
        Parcelable superState = source.readParcelable(loader);
        mSuperState = superState != null ? superState : EMPTY_STATE;
    }*/

    Parcelable* AbsSavedState::getSuperState() {
        return mSuperState;
    }

    int AbsSavedState::describeContents() {
        return 0;
    }
    void AbsSavedState::writeToParcel(Parcel& dest, int flags) {
        //dest.writeParcelable(mSuperState, flags);
    }
}
