#ifndef __TEXT_PAINT_H__
#define __TEXT_PAINT_H__
namespace cdroid{
class Paint{
public:
    enum StartHyphenEdit{
        START_HYPHEN_EDIT_NO_EDIT = 0x00,
        START_HYPHEN_EDIT_INSERT_HYPHEN = 0x01,
        START_HYPHEN_EDIT_INSERT_ZWJ = 0x02
    };
    enum EndHyphenEdit {
        END_HYPHEN_EDIT_NO_EDIT = 0x00,
        END_HYPHEN_EDIT_REPLACE_WITH_HYPHEN = 0x01,
        END_HYPHEN_EDIT_INSERT_HYPHEN = 0x02,
        END_HYPHEN_EDIT_INSERT_ARMENIAN_HYPHEN = 0x03,
        END_HYPHEN_EDIT_INSERT_MAQAF = 0x04,
        END_HYPHEN_EDIT_INSERT_UCAS_HYPHEN = 0x05,
        END_HYPHEN_EDIT_INSERT_ZWJ_AND_HYPHEN = 0x06
    };
    struct FontMetricsInt {
        int top;
        int ascent;
        int descent;
        int bottom;
        int leading;
    };
public:
    Paint();
    Paint(int){};
    Paint(const Paint&){}
    virtual ~Paint()=default;
    void set(const Paint&){}
    bool hasEqualAttributes(const Paint&other)const{return false;}
    float getUnderlineThickness()const{
        return 1.0;
    }
    void setStartHyphenEdit(int);
    void setEndHyphenEdit(int);
};
class TextPaint:public Paint {
public:
    int bgColor;
    int baselineShift;
    int linkColor;
    std::vector<int> drawableState;
    float density = 1.0f;
    int underlineColor = 0;
    float underlineThickness;

    TextPaint():Paint(){
    }

    TextPaint(int flags):Paint(flags){
    }

    TextPaint(const Paint& p):Paint(p){
    }

    void set(const TextPaint& tp) {
        Paint::set(tp);
        bgColor = tp.bgColor;
        baselineShift = tp.baselineShift;
        linkColor = tp.linkColor;
        drawableState = tp.drawableState;
        density = tp.density;
        underlineColor = tp.underlineColor;
        underlineThickness = tp.underlineThickness;
    }

    bool hasEqualAttributes(const TextPaint& other) const{
        return bgColor == other.bgColor
                && baselineShift == other.baselineShift
                && linkColor == other.linkColor
                && drawableState == other.drawableState
                && density == other.density
                && underlineColor == other.underlineColor
                && underlineThickness == other.underlineThickness
                && Paint::hasEqualAttributes((Paint&) other);
    }

    void setUnderlineText(int color, float thickness) {
        underlineColor = color;
        underlineThickness = thickness;
    }

    float getUnderlineThickness() const{
        if (underlineColor != 0) { // Return custom thickness only if underline color is set.
            return underlineThickness;
        } else {
            return Paint::getUnderlineThickness();
        }
    }
};
}/*endof namespace*/
#endif/*__TEXT_PAINT_H__*/
