#include <widget/space.h>

namespace cdroid{

Space::Space(int w,int h):View(w,h){
    if (getVisibility() == VISIBLE) {
        setVisibility(INVISIBLE);
    }
}

Space::Space(Context*context,const AttributeSet& attrs):View(context,attrs){
    if (getVisibility() == VISIBLE) {
        setVisibility(INVISIBLE);
    }
}

void Space::draw(Canvas& canvas) {
}

int Space::getDefaultSize2(int size, int measureSpec) {
    int result = size;
    int specMode = MeasureSpec::getMode(measureSpec);
    int specSize = MeasureSpec::getSize(measureSpec);

    switch (specMode) {
    case MeasureSpec::UNSPECIFIED:
        result = size;
        break;
    case MeasureSpec::AT_MOST:
        result = std::min(size, specSize);
        break;
    case MeasureSpec::EXACTLY:
        result = specSize;
        break;
    }
    return result;
}
void Space::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    setMeasuredDimension(
            getDefaultSize2(getSuggestedMinimumWidth(), widthMeasureSpec),
            getDefaultSize2(getSuggestedMinimumHeight(), heightMeasureSpec));
}

}
