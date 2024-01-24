#include <view/abssavedstate.h>
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
