#ifndef __TEXT_PAINT_H__
#define __TEXT_PAINT_H__
#include <vector>
namespace cdroid{
class CharSequence;
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
    enum CursorOption{
        CURSOR_AFTER=0,
        CURSOR_AT_OR_AFTER=1,
        CURSOR_BEFORE=2,
        CURSOR_AT_OR_BEFORE=3,
        CURSOR_AT=4,
        CURSOR_OPT_MAX_VALUE=CURSOR_AT
    };
    enum Style{
        NONE = 0,
        STROKE = 1,
        FILL = 2,
        STROKE_AND_FILL = 3
    };
public:
    Paint();
    Paint(int flags){};
    Paint(const Paint&){}
    virtual ~Paint()=default;
    virtual void set(const Paint&){}
    bool hasEqualAttributes(const Paint&other)const{return false;}
    int getFlags()const{return 0;}
    Style getStyle()const{return NONE;}
    void setStyle(Style);
    int getColor()const{return 0;}
    void setColor(int);
    float getStrokeWidth()const{return 0;}
    void setStrokeWidth(float);
    void setTextSize(float);
    float getTextSize()const{return 0;}
    int getTextAlign()const{return 0;}
    void setTextAlign(int);
    bool isAntiAlias()const{return false;}
    void setAntiAlias(bool);
    int getStrokeCap()const{return 0;}
    void setStrokeCap(int);
    int getStrokeJoin()const{return 0;}
    void setStrokeJoin(int);
    void setStrokeMiter(float);
    float getStrokeMiter()const{return 0;}
    bool isStrikeThruText()const{return false;}
    void setStrikeThruText(bool);
    float getUnderlinePosition() const;
    void setUnderlinePosition(float);
    float getUnderlineThickness() const;
    void setUnderlineThickness(float);
    float getStrikeThruPosition() const;
    void setStrikeThruPosition(float);
    float getStrikeThruThickness() const;
    void setStrikeThruThickness(float);
    void setStartHyphenEdit(int);
    int getStartHyphenEdit()const{return 0;}
    void setEndHyphenEdit(int);
    int getEndHyphenEdit()const{return 0;}
    int getLetterSpacing()const{return 0;}
    int getWordSpacing()const{return 0;}
    void setWordSpacing(int);
    bool isElegantTextHeight()const{return false;}
    void getFontMetricsInt(CharSequence* text, int start, int count,
        int contextStart, int contextCount,bool isRtl,FontMetricsInt& outMetrics){
    }
    int getFontMetricsInt(FontMetricsInt& fmi)const{return 0;}
    float getTextRunAdvances(std::vector<char16_t>& chars, int index, int count, int contextIndex,
            int contextCount, bool isRtl, std::vector<float>* advances, int advancesIndex){
        return 80;
    }
    float getRunAdvance(const CharSequence* text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset);
    float getRunAdvance(const std::vector<char16_t>& text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset);
    float getTextRunCursor(const CharSequence* text, int start, int count, bool isRtl, int offset, int cursorOpt);
    float getTextRunCursor(const std::vector<char16_t>& text, int start, int count, bool isRt, int offset, int cursorOpt);
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

    void set(const Paint& tp) override{
        Paint::set(tp);
        bgColor = ((TextPaint&)tp).bgColor;
        baselineShift = ((TextPaint&)tp).baselineShift;
        linkColor = ((TextPaint&)tp).linkColor;
        drawableState = ((TextPaint&)tp).drawableState;
        density = ((TextPaint&)tp).density;
        underlineColor = ((TextPaint&)tp).underlineColor;
        underlineThickness = ((TextPaint&)tp).underlineThickness;
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
    void setUnderlineText(bool underline) {
    }
    void setUnderlineText(int color, float thickness) {
        underlineColor = color;
        underlineThickness = thickness;
    }
    bool isUnderlineText()const{return false;}
    bool equalsForTextMeasurement(const TextPaint&)const{
        return false;
    }
};
}/*endof namespace*/
#endif/*__TEXT_PAINT_H__*/
