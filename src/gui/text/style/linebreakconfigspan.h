#ifndef __LINE_BREAK_CONFIG_SPAN_H__
#define __LINE_BREAK_CONFIG_SPAN_H__
#include <text/parcelablespan.h>
#include <text/linebreakconfig.h>
namespace cdroid{
// Ported from android-36 android/text/style/LineBreakConfigSpan.java
// Implements ParcelableSpan; queried via make_span_filter<LineBreakConfigSpan>().
class LineBreakConfigSpan : public ParcelableSpan {
public:
    LineBreakConfigSpan(const LineBreakConfig& lineBreakConfig)
        : mLineBreakConfig(lineBreakConfig) {
    }

    const LineBreakConfig& getLineBreakConfig() const{
        return mLineBreakConfig;
    }

    static LineBreakConfigSpan* createNoBreakSpan() {
        return new LineBreakConfigSpan(LineBreakConfig::Builder()
                .setLineBreakStyle(LineBreakConfig::LINE_BREAK_STYLE_NO_BREAK).build());
    }

    static LineBreakConfigSpan* createNoHyphenationSpan() {
        return new LineBreakConfigSpan(LineBreakConfig::Builder()
                .setHyphenation(LineBreakConfig::HYPHENATION_DISABLED).build());
    }

    bool equals(const LineBreakConfigSpan& o) const{
        return mLineBreakConfig == o.mLineBreakConfig;
    }

    LineBreakConfigSpan* clone() const override { return new LineBreakConfigSpan(*this); }

private:
    LineBreakConfig mLineBreakConfig;
};
}/* end namespace */
#endif/*__LINE_BREAK_CONFIG_SPAN_H__*/
