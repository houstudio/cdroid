
#ifndef __CHARACTER_STYLES_H__
#define __CHARACTER_STYLES_H__
#include <string>
#include <text/textpaint.h>
#include <core/typeface.h>
#include <text/style/metricaffectingspan.h>
namespace cdroid{

class UnderlineSpan : public CharacterStyle {
public:
    void updateDrawState(TextPaint& paint) const override {
        paint.setUnderlineText(true);
    }
};

class StrikethroughSpan : public CharacterStyle {
public:
    void updateDrawState(TextPaint& paint) const override {
        paint.setStrikeThruText(true);
    }
};

class ForegroundColorSpan : public CharacterStyle {
public:
    explicit ForegroundColorSpan(int color) : mColor(color) {}
    int getForegroundColor() const { return mColor; }

    void updateDrawState(TextPaint& paint) const override {
        paint.setColor(mColor);
    }

private:
    int mColor;
};

class BackgroundColorSpan : public CharacterStyle {
public:
    explicit BackgroundColorSpan(int color) : mColor(color) {}
    int getBackgroundColor() const { return mColor; }

    void updateDrawState(TextPaint& paint) const override {
        paint.bgColor = mColor;
    }

private:
    int mColor;
};

}/* namespace cdroid */
#endif/*__CHARACTER_STYLES_H__*/
