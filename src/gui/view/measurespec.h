#ifndef __MEASURE_SPEC_H__
#define __MEASURE_SPEC_H__
#include <string>

namespace cdroid{
class MeasureSpec{
public:
    enum{
        MODE_SHIFT = 30,
        MODE_MASK  = 3 << MODE_SHIFT,
        UNSPECIFIED= 0,
        EXACTLY = 1 << MODE_SHIFT,
        AT_MOST = 2 << MODE_SHIFT
    };
public:
    static int makeMeasureSpec(int size,int mode);
    static int makeSafeMeasureSpec(int size, int mode);
    static int getMode(int measureSpec);
    static int getSize(int measureSpec);
    static int adjust(int measureSpec, int delta);
    static const std::string toString(int measureSpec) ;
};
}
#endif
