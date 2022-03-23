#ifndef __RTLSPACINGHELPER_H__
#define __RTLSPACINGHELPER_H__
#include <limits.h>
namespace cdroid{
class RtlSpacingHelper{
public:
    static constexpr int UNDEFINED = INT_MIN;
private:
    int mLeft = 0;
    int mRight = 0;
    int mStart = UNDEFINED;
    int mEnd = UNDEFINED;
    int mExplicitLeft = 0;
    int mExplicitRight = 0;

    bool mIsRtl = false;
    bool mIsRelative = false;
public:
    int getLeft()const;
    int getRight()const;
    int getStart()const;
    int getEnd()const;
    void setRelative(int start, int end);
    void setAbsolute(int left, int right);
    void setDirection(bool isRtl);
};
}
#endif
