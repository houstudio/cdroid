#include <memory>
#include <iostream>
#include <text/textpaint.h>
#include <text/textutils.h>
#include <text/spannablestring.h>
#include <minikin/GraphemeBreak.h>
#include <minikin/LocaleList.h>
#include <minikin/Measurement.h>
#include <minikin/MeasuredText.h>
#include <minikin/MinikinFont.h>
#include <core/typeface.h>
#include <core/canvas.h>
#include <porting/cdlog.h>
#include <hb.h>
#include <hb-ft.h>

namespace cdroid{

TextPaint::TextPaint():Paint(){
}

TextPaint::TextPaint(int flags):Paint(flags){
}

TextPaint::TextPaint(const Paint& p):Paint(p){
}

void TextPaint::set(const Paint& tp) {
    Paint::set(tp);
    bgColor = ((TextPaint&)tp).bgColor;
    baselineShift = ((TextPaint&)tp).baselineShift;
    linkColor = ((TextPaint&)tp).linkColor;
    drawableState = ((TextPaint&)tp).drawableState;
    density = ((TextPaint&)tp).density;
    underlineColor = ((TextPaint&)tp).underlineColor;
    underlineThickness = ((TextPaint&)tp).underlineThickness;
}

bool TextPaint::hasEqualAttributes(const TextPaint& other) const{
    return bgColor == other.bgColor
            && baselineShift == other.baselineShift
            && linkColor == other.linkColor
            && drawableState == other.drawableState
            && density == other.density
            && underlineColor == other.underlineColor
            && underlineThickness == other.underlineThickness
            && Paint::hasEqualAttributes((Paint&) other);
}

}/*endof namespace*/
