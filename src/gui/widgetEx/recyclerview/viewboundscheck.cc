#include <widgetEx/recyclerview/viewboundscheck.h> 
namespace cdroid{

ViewBoundsCheck::BoundFlags::BoundFlags(){
    mBoundFlags = 0;
    mRvStart = mRvEnd =0;
    mChildStart = 0;
    mChildEnd = 0;
}

void ViewBoundsCheck::BoundFlags::setBounds(int rvStart, int rvEnd, int childStart, int childEnd) {
    mRvStart = rvStart;
    mRvEnd = rvEnd;
    mChildStart = childStart;
    mChildEnd = childEnd;
}

void ViewBoundsCheck::BoundFlags::setFlags(int flags, int mask) {
    mBoundFlags = (mBoundFlags & ~mask) | (flags & mask);
}

void ViewBoundsCheck::BoundFlags::addFlags(int flags) {
    mBoundFlags |= flags;
}

void ViewBoundsCheck::BoundFlags::resetFlags() {
    mBoundFlags = 0;
}

int ViewBoundsCheck::BoundFlags::compare(int x, int y) {
    if (x > y) return GT;
    if (x == y) return EQ;
    return LT;
}

bool ViewBoundsCheck::BoundFlags::boundsMatch() {
    if ((mBoundFlags & (MASK << CVS_PVS_POS)) != 0) {
        if ((mBoundFlags & (compare(mChildStart, mRvStart) << CVS_PVS_POS)) == 0) {
            return false;
        }
    }

    if ((mBoundFlags & (MASK << CVS_PVE_POS)) != 0) {
       if ((mBoundFlags & (compare(mChildStart, mRvEnd) << CVS_PVE_POS)) == 0) {
           return false;
       }
    }

    if ((mBoundFlags & (MASK << CVE_PVS_POS)) != 0) {
        if ((mBoundFlags & (compare(mChildEnd, mRvStart) << CVE_PVS_POS)) == 0) {
            return false;
        }
    }

    if ((mBoundFlags & (MASK << CVE_PVE_POS)) != 0) {
        if ((mBoundFlags & (compare(mChildEnd, mRvEnd) << CVE_PVE_POS)) == 0) {
            return false;
        }
    }
    return true;
}
 
////////////////////////////////////////////////////////////////////////////////////////////////

ViewBoundsCheck::ViewBoundsCheck(Callback callback) {
    mCallback = callback;
}

View* ViewBoundsCheck::findOneViewWithinBoundFlags(int fromIndex, int toIndex,
     int preferredBoundFlags,int acceptableBoundFlags) {
    const int start = mCallback.getParentStart();
    const int end = mCallback.getParentEnd();
    const int next = toIndex > fromIndex ? 1 : -1;
    View* acceptableMatch = nullptr;
    for (int i = fromIndex; i != toIndex; i += next) {
        View* child = mCallback.getChildAt(i);
        const int childStart = mCallback.getChildStart(child);
        const int childEnd = mCallback.getChildEnd(child);
        mBoundFlags.setBounds(start, end, childStart, childEnd);
        if (preferredBoundFlags != 0) {
            mBoundFlags.resetFlags();
            mBoundFlags.addFlags(preferredBoundFlags);
            if (mBoundFlags.boundsMatch()) {
                // found a perfect match
                return child;
            }
        }
        if (acceptableBoundFlags != 0) {
            mBoundFlags.resetFlags();
            mBoundFlags.addFlags(acceptableBoundFlags);
            if (mBoundFlags.boundsMatch()) {
                acceptableMatch = child;
            }
        }
    }
    return acceptableMatch;
}

bool ViewBoundsCheck::isViewWithinBoundFlags(View* child,int boundsFlags) {
    mBoundFlags.setBounds(mCallback.getParentStart(), mCallback.getParentEnd(),
            mCallback.getChildStart(child), mCallback.getChildEnd(child));
    if (boundsFlags != 0) {
        mBoundFlags.resetFlags();
        mBoundFlags.addFlags(boundsFlags);
        return mBoundFlags.boundsMatch();
    }
    return false;
}

}/*endof namespace*/
