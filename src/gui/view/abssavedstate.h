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
#ifndef __ABS_SAVED_STATE_H__
#define __ABS_SAVED_STATE_H__
#include <core/parcelable.h>
#include <core/parcel.h>
namespace cdroid{
class AbsSavedState:public Parcelable {
public:
    static AbsSavedState EMPTY_STATE;
private:
    Parcelable* mSuperState;
    AbsSavedState();
protected:
    AbsSavedState(Parcelable* superState);
    AbsSavedState(Parcel& source);
public:
    Parcelable* getSuperState();
    int describeContents();
    virtual void writeToParcel(Parcel& dest, int flags);
};
}/*endof namespace*/
#endif
