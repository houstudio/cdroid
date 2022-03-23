#include <widget/rtlspacinghelper.h>
namespace cdroid{

int RtlSpacingHelper::getLeft()const{
    return mLeft;
}

int RtlSpacingHelper::getRight()const{
     return mRight;
}

int RtlSpacingHelper::getStart()const{
    return mIsRtl ? mRight : mLeft;
}

int RtlSpacingHelper::getEnd()const{
    return mIsRtl ? mLeft : mRight;
}

void RtlSpacingHelper::setRelative(int start, int end) {
    mStart = start;
    mEnd = end;
    mIsRelative = true;
    if (mIsRtl) {
        if (end != UNDEFINED) mLeft = end;
        if (start != UNDEFINED) mRight = start;
    } else {
        if (start != UNDEFINED) mLeft = start;
        if (end != UNDEFINED) mRight = end;
    }
}

void RtlSpacingHelper::setAbsolute(int left, int right) {
    mIsRelative = false;
    if (left != UNDEFINED) mLeft = mExplicitLeft = left;
    if (right != UNDEFINED) mRight = mExplicitRight = right;
}

void RtlSpacingHelper::setDirection(bool isRtl) {
    if (isRtl == mIsRtl) {
        return;
    }
    mIsRtl = isRtl;
    if (mIsRelative) {
        if (isRtl) {
            mLeft = mEnd != UNDEFINED ? mEnd : mExplicitLeft;
            mRight = mStart != UNDEFINED ? mStart : mExplicitRight;
        } else {
            mLeft = mStart != UNDEFINED ? mStart : mExplicitLeft;
            mRight = mEnd != UNDEFINED ? mEnd : mExplicitRight;
        }
    } else {
        mLeft = mExplicitLeft;
        mRight = mExplicitRight;
    }
}

}//endof namespace
