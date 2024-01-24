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
    void writeToParcel(Parcel& dest, int flags);
};
}/*endof namespace*/
#endif
