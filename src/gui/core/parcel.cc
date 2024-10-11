#include <vector>
#include <string>
#include <core/parcel.h>
namespace cdroid{

int Parcel::readInt(){
    return 0;
}
bool Parcel::readBoolean(){
    return readInt()!=0;
}

std::string Parcel::readString(){
    return std::string();
}

void Parcel::writeInt(int val){
}

void Parcel::writeBoolean(bool){
}

void Parcel::writeString(const std::string&){
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
