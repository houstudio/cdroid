#include <stdexcept>
#include <core/parcel.h>
#include <view/dragevent.h>

namespace cdroid{

int DragEvent::gRecyclerUsed = 0;
DragEvent* DragEvent::gRecyclerTop = nullptr;

DragEvent::DragEvent() {
}

void DragEvent::init(int action, float x, float y, ClipDescription* description, ClipData* data,void* localState, bool result) {
    mAction = action;
    mX = x;
    mY = y;
    mClipDescription = description;
    mClipData = data;
    //this.mDragAndDropPermissions = dragAndDropPermissions;
    mLocalState = localState;
    mDragResult = result;
}

DragEvent* DragEvent::obtain() {
    return DragEvent::obtain(0, 0.f, 0.f, nullptr, nullptr, nullptr, false);
}

DragEvent* DragEvent::obtain(int action, float x, float y, void* localState,
        ClipDescription* description, ClipData* data, bool result) {
    DragEvent* ev;
    //synchronized (gRecyclerLock)
    {
        if (gRecyclerTop == nullptr) {
            ev = new DragEvent();
            ev->init(action, x, y, description, data, localState,result);
            return ev;
        }
        ev = gRecyclerTop;
        gRecyclerTop = ev->mNext;
        gRecyclerUsed -= 1;
    }
    //ev->mRecycledLocation = nullptr;
    ev->mRecycled = false;
    ev->mNext = nullptr;

    ev->init(action, x, y, description, data, localState, result);

    return ev;
}

DragEvent* DragEvent::obtain(DragEvent& source) {
    return obtain(source.mAction, source.mX, source.mY, source.mLocalState,
            source.mClipDescription, source.mClipData,source.mDragResult);
}

int DragEvent::getAction() const{
    return mAction;
}

float DragEvent::getX() const{
    return mX;
}

float DragEvent::getY() const{
    return mY;
}

ClipData* DragEvent::getClipData() const{
    return mClipData;
}

ClipDescription* DragEvent::getClipDescription()const {
    return mClipDescription;
}

void* DragEvent::getLocalState() const{
    return mLocalState;
}

bool DragEvent::getResult() const{
    return mDragResult;
}

void DragEvent::recycle() {
    // Ensure recycle is only called once!
    /*if (TRACK_RECYCLED_LOCATION) {
        if (mRecycledLocation != nullptr) {
            throw std::runtime_error(" recycled twice!", mRecycledLocation);
        }
        mRecycledLocation = new RuntimeException("Last recycled here");
    } else */{
        if (mRecycled) {
            throw std::runtime_error(" recycled twice!");
        }
        mRecycled = true;
    }

    mClipData = nullptr;
    mClipDescription = nullptr;
    mLocalState = nullptr;
    mEventHandlerWasCalled = false;

    //synchronized (gRecyclerLock)
    {
        if (gRecyclerUsed < MAX_RECYCLED) {
            gRecyclerUsed++;
            mNext = gRecyclerTop;
            gRecyclerTop = this;
        }
    }
}

int DragEvent::describeContents() {
    return 0;
}

void DragEvent::writeToParcel(Parcel& dest, int flags) {
    dest.writeInt(mAction);
    dest.writeFloat(mX);
    dest.writeFloat(mY);
    dest.writeInt(mDragResult ? 1 : 0);
    /*if (mClipData == nullptr) {
        dest.writeInt(0);
    } else {
        dest.writeInt(1);
        mClipData.writeToParcel(dest, flags);
    }
    if (mClipDescription == nullptr) {
        dest.writeInt(0);
    } else {
        dest.writeInt(1);
        mClipDescription.writeToParcel(dest, flags);
    }
    if (mDragAndDropPermissions == nullptr) {
        dest.writeInt(0);
    } else {
        dest.writeInt(1);
        dest.writeStrongBinder(mDragAndDropPermissions.asBinder());
    }*/
}

}/*endof namespace*/
