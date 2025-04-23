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
