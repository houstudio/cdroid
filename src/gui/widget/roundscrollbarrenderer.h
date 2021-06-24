#ifndef __ROUND_SCROLLBAR_RENDERER__
#include <widget/view.h>
namespace cdroid{
class RoundScrollbarRenderer{
private:
    View* mParent;
    RECT mRect;
    int mThumbColor;
    int mTrackColor;
    static float clamp(float val, float min, float max);
    static int applyAlpha(int color, float alpha);
    void setThumbColor(int thumbColor);
    void setTrackColor(int trackColor);
public:
    RoundScrollbarRenderer(View* parent);
    void drawRoundScrollbars(Canvas& canvas, float alpha,const RECT& bounds);
};
}//namespace
#define __ROUND_SCROLLBAR_RENDERER__
#endif
