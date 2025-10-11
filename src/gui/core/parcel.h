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
#ifndef __PARCEL_H__
#define __PARCEL_H__
#include <stdint.h>
#include <string>
#include <vector>
#include <core/bundle.h>
namespace cdroid{
class Parcelable;
class Parcel{
private:
    static constexpr int POOL_SIZE = 6;
    //static constexpr Parcel *sOwnedPool;//[POOL_SIZE];
    //static constexpr Parcel *sHolderPool;//[POOL_SIZE];

    // Keep in sync with frameworks/native/include/private/binder/ParcelValTypes.h.
    static constexpr int VAL_NULL = -1;
    static constexpr int VAL_STRING = 0;
    static constexpr int VAL_INTEGER = 1;
    static constexpr int VAL_MAP = 2;
    static constexpr int VAL_BUNDLE = 3;
    static constexpr int VAL_PARCELABLE = 4;
    static constexpr int VAL_SHORT = 5;
    static constexpr int VAL_LONG = 6;
    static constexpr int VAL_FLOAT = 7;
    static constexpr int VAL_DOUBLE = 8;
    static constexpr int VAL_BOOLEAN = 9;
    static constexpr int VAL_CHARSEQUENCE = 10;
    static constexpr int VAL_LIST  = 11;
    static constexpr int VAL_SPARSEARRAY = 12;
    static constexpr int VAL_BYTEARRAY = 13;
    static constexpr int VAL_STRINGARRAY = 14;
    static constexpr int VAL_IBINDER = 15;
    static constexpr int VAL_PARCELABLEARRAY = 16;
    static constexpr int VAL_OBJECTARRAY = 17;
    static constexpr int VAL_INTARRAY = 18;
    static constexpr int VAL_LONGARRAY = 19;
    static constexpr int VAL_BYTE = 20;
    static constexpr int VAL_SERIALIZABLE = 21;
    static constexpr int VAL_SPARSEBOOLEANARRAY = 22;
    static constexpr int VAL_BOOLEANARRAY = 23;
    static constexpr int VAL_CHARSEQUENCEARRAY = 24;
    static constexpr int VAL_PERSISTABLEBUNDLE = 25;
    static constexpr int VAL_SIZE = 26;
    static constexpr int VAL_SIZEF = 27;
    static constexpr int VAL_DOUBLEARRAY = 28;

    // The initial int32 in a Binder call's reply Parcel header:
    // Keep these in sync with libbinder's binder/Status.h.
    static constexpr int EX_SECURITY = -1;
    static constexpr int EX_BAD_PARCELABLE = -2;
    static constexpr int EX_ILLEGAL_ARGUMENT = -3;
    static constexpr int EX_NULL_POINTER = -4;
    static constexpr int EX_ILLEGAL_STATE = -5;
    static constexpr int EX_NETWORK_MAIN_THREAD = -6;
    static constexpr int EX_UNSUPPORTED_OPERATION = -7;
    static constexpr int EX_SERVICE_SPECIFIC = -8;
    static constexpr int EX_PARCELABLE = -9;
    static constexpr int EX_HAS_REPLY_HEADER = -128;  // special; see below
    // EX_TRANSACTION_FAILED is used exclusively in native code.
    // see libbinder's binder/Status.h
    static constexpr int EX_TRANSACTION_FAILED = -129;
private:

public:
    static Parcel* obtain();
    void recycle();
    uint8_t readByte();
    int32_t readInt();
    int64_t readLong();
    bool readBoolean();
    float readFloat();
    double readDouble();
    std::string readString();
    std::string readCharSequence();
    Bundle* readBundle();
    std::vector<std::string>createStringArrayList();

    void writeByte(uint8_t val);
    void writeInt(int32_t val);
    void writeLong(int64_t);
    void writeBoolean(bool);
    void writeFloat(float val);
    void writeDouble(double val);
    void writeString(const std::string& val);
    void writeCharSequence(const std::string& val);
    void writeBundle(const Bundle&);
    void writeParcelable(Parcelable*p,int parcelableFlags);
    void writeStringArrayList(const std::vector<std::string>&);
};
}/*endof namespace*/
#endif/*__PARCEL_H__*/
