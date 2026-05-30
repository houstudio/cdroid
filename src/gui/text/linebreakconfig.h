#ifndef __LINE_BREAK_CONFIG_H__
#define __LINE_BREAK_CONFIG_H__
namespace cdroid{
class LineBreakConfig {
public:
    static constexpr int HYPHENATION_UNSPECIFIED = -1;
    static constexpr int HYPHENATION_DISABLED = 0;
    static constexpr int HYPHENATION_ENABLED = 1;

    static constexpr int LINE_BREAK_STYLE_UNSPECIFIED = -1;
    static constexpr int LINE_BREAK_STYLE_NONE = 0;
    static constexpr int LINE_BREAK_STYLE_LOOSE = 1;
    static constexpr int LINE_BREAK_STYLE_NORMAL = 2;
    static constexpr int LINE_BREAK_STYLE_STRICT = 3;
    static constexpr int LINE_BREAK_STYLE_NO_BREAK = 4;

    static constexpr int LINE_BREAK_STYLE_AUTO = 5;

    static constexpr int LINE_BREAK_WORD_STYLE_UNSPECIFIED = -1;
    static constexpr int LINE_BREAK_WORD_STYLE_NONE = 0;
    static constexpr int LINE_BREAK_WORD_STYLE_PHRASE = 1;
    static constexpr int LINE_BREAK_WORD_STYLE_AUTO = 2;
private:
    int mLineBreakStyle;
    int mLineBreakWordStyle;
    int mHyphenation;
public:
    class Builder {
    private:
        int mLineBreakStyle = LineBreakConfig::LINE_BREAK_STYLE_UNSPECIFIED;
        int mLineBreakWordStyle = LineBreakConfig::LINE_BREAK_WORD_STYLE_UNSPECIFIED;
        int mHyphenation = LineBreakConfig::HYPHENATION_UNSPECIFIED;
    public:
        Builder() {
            reset(nullptr);
        }

        Builder& merge(const LineBreakConfig& config) {
            if (config.mLineBreakStyle != LINE_BREAK_STYLE_UNSPECIFIED) {
                mLineBreakStyle = config.mLineBreakStyle;
            }
            if (config.mLineBreakWordStyle != LINE_BREAK_WORD_STYLE_UNSPECIFIED) {
                mLineBreakWordStyle = config.mLineBreakWordStyle;
            }
            if (config.mHyphenation != HYPHENATION_UNSPECIFIED) {
                mHyphenation = config.mHyphenation;
            }
            return *this;
        }

        Builder& reset(const LineBreakConfig* config) {
            if (config == nullptr) {
                mLineBreakStyle = LINE_BREAK_STYLE_UNSPECIFIED;
                mLineBreakWordStyle = LINE_BREAK_WORD_STYLE_UNSPECIFIED;
                mHyphenation = HYPHENATION_UNSPECIFIED;
            } else {
                mLineBreakStyle = config->mLineBreakStyle;
                mLineBreakWordStyle = config->mLineBreakWordStyle;
                mHyphenation = config->mHyphenation;
            }
            return *this;
        }

        Builder& setLineBreakStyle(int lineBreakStyle) {
            mLineBreakStyle = lineBreakStyle;
            return *this;
        }

        Builder& setLineBreakWordStyle(int lineBreakWordStyle) {
            mLineBreakWordStyle = lineBreakWordStyle;
            return *this;
        }

        Builder& setHyphenation(int hyphenation) {
            mHyphenation = hyphenation;
            return *this;
        }

        LineBreakConfig build() const{
            return LineBreakConfig(mLineBreakStyle, mLineBreakWordStyle, mHyphenation);
        }
    };

    static LineBreakConfig getLineBreakConfig(int lineBreakStyle,int lineBreakWordStyle) {
        LineBreakConfig::Builder builder;// = new LineBreakConfig.Builder();
        return builder.setLineBreakStyle(lineBreakStyle)
                .setLineBreakWordStyle(lineBreakWordStyle)
                .build();
    }

    const static LineBreakConfig NONE;
        // =new Builder().setLineBreakStyle(LINE_BREAK_STYLE_NONE)
        //.setLineBreakWordStyle(LINE_BREAK_WORD_STYLE_NONE).build();

    LineBreakConfig(int lineBreakStyle,int lineBreakWordStyle,int hyphenation) {
        mLineBreakStyle = lineBreakStyle;
        mLineBreakWordStyle = lineBreakWordStyle;
        mHyphenation = hyphenation;
    }

    int getLineBreakStyle() const{
        return mLineBreakStyle;
    }

    static int getResolvedLineBreakStyle(const LineBreakConfig* config) {
        int defaultStyle = LINE_BREAK_STYLE_AUTO;
        if (config == nullptr) {
            return defaultStyle;
        }
        return config->mLineBreakStyle == LINE_BREAK_STYLE_UNSPECIFIED
                ? defaultStyle : config->mLineBreakStyle;
    }

    int getLineBreakWordStyle() const{
        return mLineBreakWordStyle;
    }

    static int getResolvedLineBreakWordStyle(const LineBreakConfig* config) {
        const int defaultWordStyle = LINE_BREAK_WORD_STYLE_AUTO;
        if (config == nullptr) {
            return defaultWordStyle;
        }
        return config->mLineBreakWordStyle == LINE_BREAK_WORD_STYLE_UNSPECIFIED
                ? defaultWordStyle : config->mLineBreakWordStyle;
    }

    int getHyphenation() const{
        return mHyphenation;
    }

    static int getResolvedHyphenation(const LineBreakConfig* config) {
        if (config == nullptr) {
            return HYPHENATION_ENABLED;
        }
        return config->mHyphenation == HYPHENATION_UNSPECIFIED
                ? HYPHENATION_ENABLED : config->mHyphenation;
    }


    LineBreakConfig merge(const LineBreakConfig& config) const{
        return LineBreakConfig(
                config.mLineBreakStyle == LINE_BREAK_STYLE_UNSPECIFIED
                        ? mLineBreakStyle : config.mLineBreakStyle,
                config.mLineBreakWordStyle == LINE_BREAK_WORD_STYLE_UNSPECIFIED
                        ? mLineBreakWordStyle : config.mLineBreakWordStyle,
                config.mHyphenation == HYPHENATION_UNSPECIFIED
                        ? mHyphenation : config.mHyphenation);
    }
};
}/*endof namespace*/
#endif/*__LINE_BREAK_CONFIG_H__*/
