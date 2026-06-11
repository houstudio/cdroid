#ifndef __TEXT_PAINT_H__
#define __TEXT_PAINT_H__
#include <memory>
#include <string>
#include <vector>
namespace minikin{
    class MinikinPaint;
}
namespace cdroid{
class CharSequence;
class Typeface;
class Canvas;
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
    struct FontMetrics {
        float top;
        float ascent;
        float descent;
        float bottom;
        float leading;
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
private:
    Typeface*mTypeface;
    std::shared_ptr<minikin::MinikinPaint>mMinikinPaint;
    int mColor;
    int mTextAlign;
    int mStartHyphenEdit;
    int mEndHyphenEdit;
    Style mStyle;
    bool mAntialias;
    bool mFakeBoldText;
    bool mStrikeThruText;
    float mStrokeWidth;
    float mTextSize;
    float mTextSkewX;
    float mTextScaleX;
    float mWordSpace;
    float mLetterSpacing;
    float mUnderlinePosition;
    float mUnderlineThickness;
    float mStrikeThruPosition;
    float mStrikeThruThickness;
public:
    Paint();
    Paint(int flags);
    Paint(const Paint&);
    virtual ~Paint();
    Typeface* getTypeface()const{return mTypeface;}
    void setTypeface(Typeface*face){mTypeface=face;}
    minikin::MinikinPaint* getMinikinPaint()const{return mMinikinPaint.get();}
    virtual void set(const Paint&);
    bool hasEqualAttributes(const Paint&other)const;
    void setFlags(int v){}
    int getFlags()const{return 0;}
    Style getStyle()const{return mStyle;}
    void setStyle(Style v){mStyle=v;}
    void setFakeBoldText(bool v){mFakeBoldText=v;}
    int getColor()const{return mColor;}
    void setColor(int v){mColor=v;}
    float getStrokeWidth()const{return mStrokeWidth;}
    void setStrokeWidth(float v){mStrokeWidth=v;}
    void setTextSize(float v){mTextSize=v;}
    float getTextSize()const{return mTextSize;}
    void setTextScaleX(float v){mTextScaleX=v;}
    float getTextScaleX()const{return mTextScaleX;}
    void setTextSkewX(float v){mTextSkewX=v;}
    float getTextSkewX()const{return mTextSkewX;}
    int getTextAlign()const{return 0;}
    void setTextAlign(int);
    bool isAntiAlias()const{return mAntialias;}
    void setAntiAlias(bool v){mAntialias=v;}
    int getStrokeCap()const{return 0;}
    void setStrokeCap(int);
    int getStrokeJoin()const{return 0;}
    void setStrokeJoin(int);
    void setStrokeMiter(float);
    float getStrokeMiter()const{return 0;}
    bool isStrikeThruText()const{return mStrikeThruText;}
    void setStrikeThruText(bool v){mStrikeThruText=v;}
    float getUnderlinePosition() const{ return mUnderlinePosition;}
    void setUnderlinePosition(float v){mUnderlinePosition=v;}
    float getUnderlineThickness() const{return mUnderlineThickness;}
    void setUnderlineThickness(float v){mUnderlineThickness=v;}
    float getStrikeThruPosition() const{return mStrikeThruPosition;}
    void setStrikeThruPosition(float v){mStrikeThruPosition=v;}
    float getStrikeThruThickness() const{return mStrikeThruThickness;}
    void setStrikeThruThickness(float v){mStrikeThruThickness=v;}
    void setStartHyphenEdit(int v){mStartHyphenEdit=v;}
    int getStartHyphenEdit()const{return mStartHyphenEdit;}
    void setEndHyphenEdit(int v){mEndHyphenEdit=v;}
    int getEndHyphenEdit()const{return mEndHyphenEdit;}
    void setLetterSpacing(float v){mLetterSpacing=v;}
    float getLetterSpacing()const{return mLetterSpacing;}
    int getWordSpacing()const{return mWordSpace;}
    void setWordSpacing(float v){mWordSpace=v;}
    void setElegantTextHeight(bool){}
    bool isElegantTextHeight()const{return false;}
    float measureText(const std::string& text)const;
    float measureText(const std::string& text, int start, int end)const;
    float measureText(const CharSequence* text, int start, int end)const;
    float measureText(const char16_t* text, int index, int count)const;
    void getFontMetricsInt(const CharSequence* text, int start, int count,
        int contextStart, int contextCount,bool isRtl,FontMetricsInt& outMetrics)const;
    int getFontMetricsInt(FontMetricsInt* fmi)const;
    FontMetricsInt getFontMetricsInt()const;
    float getTextRunAdvances(const char16_t* chars, int index, int count, int contextIndex,
            int contextCount, bool isRtl, std::vector<float>* advances, int advancesIndex)const;
    float getRunAdvance(const CharSequence* text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset)const;
    float getRunAdvance(const char16_t* text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset)const;
    float getTextRunCursor(const CharSequence* text, int start, int count, bool isRtl, int offset, int cursorOpt)const;
    float getTextRunCursor(const char16_t* text, int start, int count, bool isRt, int offset, int cursorOpt)const;
    void drawTextRun(Canvas&c,const char16_t*chars,int start,int count,
        int contextStart,int contextCount,float x,float y,bool runIsRtl)const;
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

    TextPaint();
    TextPaint(int flags);
    TextPaint(const Paint& p);

    void set(const Paint& tp) override;

    bool hasEqualAttributes(const TextPaint& other) const;
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
