#pragma once
#include <widget/view.h>

namespace cdroid{

class Space:public View{
private:
    static int getDefaultSize2(int size, int measureSpec);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    Space(int w,int h);
    Space(Context*context,const AttributeSet& attrs);
    void draw(Canvas&)override;
};

}//endof namespace
