#include <view/measurespec.h>
#include <sstream>
#include <cdtypes.h>
#include <cdlog.h>


namespace cdroid{

int MeasureSpec::makeMeasureSpec(int size,int mode){
    return (size & ~MODE_MASK) | (mode & MODE_MASK);
}

int MeasureSpec::makeSafeMeasureSpec(int size, int mode){
    return makeMeasureSpec(size,mode);
}

int MeasureSpec::getMode(int measureSpec){
    //noinspection ResourceType
    return (measureSpec & MODE_MASK);
}

int MeasureSpec::getSize(int measureSpec){
    return (measureSpec & ~MODE_MASK);
}

int MeasureSpec::adjust(int measureSpec, int delta) {
    int mode = getMode(measureSpec);
    int size = getSize(measureSpec);
    if (mode == UNSPECIFIED) {
        // No need to adjust size for UNSPECIFIED mode.
        return makeMeasureSpec(size, UNSPECIFIED);
    }
    size += delta;
    if (size < 0) {
        LOGE("MeasureSpec.adjust: new size would be negative! (%d) spec:%s  delta:%d",size,
				toString(measureSpec).c_str(),delta);
        size = 0;
    }
    return makeMeasureSpec(size, mode);
}

const std::string MeasureSpec::toString(int measureSpec) {
    int mode = getMode(measureSpec);
    int size = getSize(measureSpec);

	std::ostringstream sb;
    if (mode == UNSPECIFIED)  sb<<"UNSPECIFIED ";
    else if (mode == EXACTLY) sb<<"EXACTLY ";
    else if (mode == AT_MOST) sb<<"AT_MOST ";
    else     sb<<mode<<" ";
    sb<<size;
    return sb.str();
}

}
