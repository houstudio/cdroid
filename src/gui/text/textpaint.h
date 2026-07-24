#ifndef __TEXT_PAINT_H__
#define __TEXT_PAINT_H__
#include <memory>
#include <string>
#include <vector>
#include <text/paint.h>
namespace cdroid{

class TextPaint:public Paint {
public:
    int bgColor;
    int baselineShift;
    int linkColor;
    float density = 1.0f;
    std::vector<int> drawableState;
    int underlineColor = 0;
    float underlineThickness;

    TextPaint();
    TextPaint(int flags);
    TextPaint(const Paint& p);

    void set(const Paint& tp) override;

    bool hasEqualAttributes(const TextPaint& other) const;
    // void setUnderlineText(bool underline) {
    // }
    using Paint::setUnderlineText; // 引入基类 setUnderlineText(bool)，避免被下方 (int,float) 重载隐藏
    void setUnderlineText(int color, float thickness) {
        underlineColor = color;
        underlineThickness = thickness;
    }
    // bool isUnderlineText()const{return false;}
    bool equalsForTextMeasurement(const TextPaint&)const{
        return false;
    }
};
}/*endof namespace*/
#endif/*__TEXT_PAINT_H__*/
