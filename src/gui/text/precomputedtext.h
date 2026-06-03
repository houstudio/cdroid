#ifndef __PRE_COMPUTED_TEXT_H__
#define __PRE_COMPUTED_TEXT_H__
#include <core/rect.h>
#include <text/spannablestring.h>
namespace cdroid{
class MeasuredParagraph;
class TextDirectionHeuristic;
class PrecomputedText :public Spannable {
private:
    static constexpr char LINE_FEED = '\n';
public:
    class Params {
    public:
        static constexpr int UNUSABLE = 0;
        static constexpr int NEED_RECOMPUTE = 1;
        static constexpr int USABLE = 2;
    private:
        friend PrecomputedText;
        TextPaint mPaint;
        const TextDirectionHeuristic* mTextDir;
        int mBreakStrategy;
        int mHyphenationFrequency;
    public:
        Params(const TextPaint& paint,const TextDirectionHeuristic* textDir,int strategy, int frequency);
        const TextPaint& getTextPaint() const{
            return mPaint;
        }
        const TextDirectionHeuristic* getTextDirection() const{
            return mTextDir;
        }
        int getBreakStrategy() const{
            return mBreakStrategy;
        }
        int getHyphenationFrequency() const{
            return mHyphenationFrequency;
        }
        int checkResultUsable(const TextPaint& paint,const TextDirectionHeuristic* textDir, int strategy,  int frequency) const;
    };

    class ParagraphInfo {
    public:
        int paragraphEnd;
        MeasuredParagraph* measured;
        ParagraphInfo(int paraEnd, MeasuredParagraph* measured) {
            this->paragraphEnd = paraEnd;
            this->measured = measured;
        }
    };
private:
    SpannableString* mText;
    int mStart;
    int mEnd;
    Params mParams;
    std::vector<ParagraphInfo> mParagraphInfo;

    static std::vector<ParagraphInfo> createMeasuredParagraphsFromPrecomputedText(
            PrecomputedText* pct, const Params& params, bool computeLayout);
    PrecomputedText(CharSequence* text, int start,  int end,const Params& params, const std::vector<ParagraphInfo>& paraInfo);
public:
    static PrecomputedText* create(CharSequence* text,const Params& params);

    static std::vector<ParagraphInfo> createMeasuredParagraphs(CharSequence* text,const Params& params, int start, int end, bool computeLayout);

    CharSequence* getText() const{
        return mText;
    }
    int getStart() const{
        return mStart;
    }
    int getEnd() const{
        return mEnd;
    }
    const Params& getParams() const{
        return mParams;
    }
    int getParagraphCount() const{
        return mParagraphInfo.size();
    }
    int getParagraphStart(int paraIndex) const{
        return paraIndex == 0 ? mStart : getParagraphEnd(paraIndex - 1);
    }
    int getParagraphEnd(int paraIndex) const{
        return mParagraphInfo[paraIndex].paragraphEnd;
    }
    MeasuredParagraph* getMeasuredParagraph(int paraIndex) const{
        return mParagraphInfo[paraIndex].measured;
    }
    std::vector<ParagraphInfo> getParagraphInfo() const{
        return mParagraphInfo;
    }

    int checkResultUsable(int start, int end, const TextDirectionHeuristic* textDir,const TextPaint& paint, int strategy, int frequency)const;

    int findParaIndex(int pos) const;
    float getWidth(int start,int end) const;
    void getBounds(int start, int end, Rect& bounds) const;
    float getCharWidthAt(int offset) const;
    int getMemoryUsage() const;
    void setSpan(ParcelableSpan* what, int start, int end, int flags) override;
    void removeSpan(ParcelableSpan* what) override;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Spanned overrides

    std::vector<ParcelableSpan*> getSpans(int start, int end,const SpanFilter& type)const override{
        return mText->getSpans(start, end, type);
    }
    int getSpanStart(ParcelableSpan* tag)const override{
        return mText->getSpanStart(tag);
    }
    int getSpanEnd(ParcelableSpan* tag)const override{
        return mText->getSpanEnd(tag);
    }
    int getSpanFlags(ParcelableSpan* tag)const override{
        return mText->getSpanFlags(tag);
    }
    int nextSpanTransition(int start, int limit,const SpanFilter& type)const override{
        return mText->nextSpanTransition(start, limit, type);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void getChars(int start, int end, std::vector<char32_t>& dest, int destPos) const override;
    size_t length() const override{
        return mText->length();
    }
    int charAt(int index) const override{
        return mText->charAt(index);
    }
    CharSequence* subSequence(int start, int end) const override{
        return PrecomputedText::create(mText->subSequence(start, end), mParams);
    }
    std::string toString() const override;
    std::wstring toWString() const override;
};
}/*endof namespace*/
#endif/*__PRE_COMPUTED_TEXT_H__*/
