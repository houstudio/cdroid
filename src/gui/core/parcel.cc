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
#include <vector>
#include <string>
#include <core/parcel.h>
namespace cdroid{

int Parcel::readInt(){
    return 0;
}

int64_t Parcel::readLong(){
    return 0;
}

bool Parcel::readBoolean(){
    return readInt()!=0;
}

std::string Parcel::readString(){
    return std::string();
}

Bundle* Parcel::readBundle(){
    return nullptr;
}

std::string Parcel::readCharSequence(){
    return std::string();
}

std::vector<std::string>Parcel::createStringArrayList(){
    return std::vector<std::string>();
}

void Parcel::writeInt(int32_t val){
}

void Parcel::writeLong(int64_t){
}

void Parcel::writeBoolean(bool){
}

float Parcel::readFloat(){
    return 0;
}

void Parcel::writeFloat(float){
}

void Parcel::writeString(const std::string&){
}

void Parcel::writeCharSequence(const std::string& val){
}

void Parcel::writeBundle(const Bundle&){
}

void Parcel::writeStringArrayList(const std::vector<std::string>&){
}

void Parcel::writeParcelable(Parcelable* p, int parcelableFlags) {
    if (p == nullptr) {
        writeString("");
        return;
    }
    //writeParcelableCreator(p);
    //p->writeToParcel(this, parcelableFlags);
}

}
