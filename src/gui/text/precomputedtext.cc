#include <text/layout.h>
#include <text/precomputedtext.h>
#include <text/measuredparagraph.h>
#include <text/textdirectionheuristics.h>
namespace cdroid{
//private static constexpr char LINE_FEED = '\n';

/////////////////////////////////////////////////////////////////////////////////////////////

PrecomputedText::Params::Params(const TextPaint& paint, const TextDirectionHeuristic* textDir,int strategy, int frequency) {
    mPaint = paint;
    mTextDir = textDir;
    mBreakStrategy = strategy;
    mHyphenationFrequency = frequency;
}

int PrecomputedText::Params::checkResultUsable(const TextPaint& paint,
        const TextDirectionHeuristic* textDir, int strategy,  int frequency) const{
    if (mBreakStrategy == strategy && mHyphenationFrequency == frequency
            && mPaint.equalsForTextMeasurement(paint)) {
        return mTextDir == textDir ? USABLE : NEED_RECOMPUTE;
    } else {
        return UNUSABLE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PrecomputedText* PrecomputedText::create(CharSequence* text,const Params& params) {
    std::vector<ParagraphInfo> paraInfo;
    PrecomputedText* hintPct = dynamic_cast<PrecomputedText*>(text);
    if (hintPct != nullptr) {
        const PrecomputedText::Params& hintParams = hintPct->getParams();
        const int checkResult = hintParams.checkResultUsable(params.mPaint, params.mTextDir,
                        params.mBreakStrategy, params.mHyphenationFrequency);
        switch (checkResult) {
            case Params::USABLE:
                return hintPct;
            case Params::NEED_RECOMPUTE:
                // To be able to use PrecomputedText for new params, at least break strategy and
                // hyphenation frequency must be the same.
                if (params.getBreakStrategy() == hintParams.getBreakStrategy()
                        && params.getHyphenationFrequency()
                            == hintParams.getHyphenationFrequency()) {
                    paraInfo = createMeasuredParagraphsFromPrecomputedText(
                            hintPct, params, true /* compute layout */);
                }
                break;
            case Params::UNUSABLE:
                // Unable to use anything in PrecomputedText. Create PrecomputedText as the
                break;// normal text input.
        }

    }
    if (paraInfo.empty()) {
        paraInfo = createMeasuredParagraphs(text, params, 0, text->length(), true /* computeLayout */);
    }
    return new PrecomputedText(text, 0, text->length(), params, paraInfo);
}

std::vector<PrecomputedText::ParagraphInfo> PrecomputedText::createMeasuredParagraphsFromPrecomputedText( PrecomputedText* pct,const Params& params, bool computeLayout) {
    const bool needHyphenation = params.getBreakStrategy() != Layout::BREAK_STRATEGY_SIMPLE
            && params.getHyphenationFrequency() != Layout::HYPHENATION_FREQUENCY_NONE;
    std::vector<ParagraphInfo> result;
    for (int i = 0; i < pct->getParagraphCount(); ++i) {
        const int paraStart = pct->getParagraphStart(i);
        const int paraEnd = pct->getParagraphEnd(i);
        result.push_back(ParagraphInfo(paraEnd, MeasuredParagraph::buildForStaticLayout(
                &params.getTextPaint(), pct, paraStart, paraEnd, params.getTextDirection(),
                needHyphenation, computeLayout, pct->getMeasuredParagraph(i),
                nullptr /* no recycle */)));
    }
    return result;
}

std::vector<PrecomputedText::ParagraphInfo> PrecomputedText::createMeasuredParagraphs(CharSequence* text,const Params& params, int start, int end, bool computeLayout) {
    std::vector<ParagraphInfo> result;

    //Preconditions.checkNotNull(text);
    //Preconditions.checkNotNull(params);
    const bool needHyphenation = params.getBreakStrategy() != Layout::BREAK_STRATEGY_SIMPLE
            && params.getHyphenationFrequency() != Layout::HYPHENATION_FREQUENCY_NONE;

    int paraEnd = 0;
    for (int paraStart = start; paraStart < end; paraStart = paraEnd) {
        paraEnd = TextUtils::indexOf(text, LINE_FEED, paraStart, end);
        if (paraEnd < 0) {
            // No LINE_FEED(U+000A) character found. Use end of the text as the paragraph
            // end.
            paraEnd = end;
        } else {
            paraEnd++;  // Includes LINE_FEED(U+000A) to the prev paragraph.
        }

        result.emplace_back(ParagraphInfo(paraEnd, MeasuredParagraph::buildForStaticLayout(
                &params.getTextPaint(), text, paraStart, paraEnd, params.getTextDirection(),
                needHyphenation, computeLayout, nullptr /* no hint */,
                nullptr /* no recycle */)));
    }
    return result;
}

// Use PrecomputedText.create instead.
PrecomputedText::PrecomputedText(CharSequence* text, int start,  int end, const Params& params,
        const std::vector<ParagraphInfo>& paraInfo)
    : mParams(params) {
    mText = new SpannableString(text, true /* ignoreNoCopySpan */);
    mStart = start;
    mEnd = end;
    mParagraphInfo = paraInfo;
}

int PrecomputedText::checkResultUsable(int start, int end, const TextDirectionHeuristic* textDir,
        const TextPaint& paint, int strategy, int frequency) const{
    if (mStart != start || mEnd != end) {
        return Params::UNUSABLE;
    } else {
        return mParams.checkResultUsable(paint, textDir, strategy, frequency);
    }
}

int PrecomputedText::findParaIndex(int pos) const{
    // TODO: Maybe good to remove paragraph concept from PrecomputedText and add substring
    //       layout support to StaticLayout.
    for (int i = 0; i < mParagraphInfo.size(); ++i) {
        if (pos < mParagraphInfo[i].paragraphEnd) {
            return i;
        }
    }
    //LOGE("pos must be less than %d, gave %d",mParagraphInfo[mParagraphInfo.length - 1].paragraphEnd,pos);
    return 0;
}

float PrecomputedText::getWidth(int start,int end) const{
    //Preconditions.checkArgument(0 <= start && start <= mText.length(), "invalid start offset");
    //Preconditions.checkArgument(0 <= end && end <= mText.length(), "invalid end offset");
    //Preconditions.checkArgument(start <= end, "start offset can not be larger than end offset");

    if (start == end) {
        return 0;
    }
    const int paraIndex = findParaIndex(start);
    const int paraStart = getParagraphStart(paraIndex);
    const int paraEnd = getParagraphEnd(paraIndex);
    if (start < paraStart || paraEnd < end) {
        //LOGE("Cannot measured across the paragraph:para(%d,%d) request(%d,%d)",paraStart,paraEnd,start,end);
    }
    return getMeasuredParagraph(paraIndex)->getWidth(start - paraStart, end - paraStart);
}

void PrecomputedText::getBounds(int start, int end, Rect& bounds) const{
    //Preconditions.checkArgument(0 <= start && start <= mText.length(), "invalid start offset");
    //Preconditions.checkArgument(0 <= end && end <= mText.length(), "invalid end offset");
    //Preconditions.checkArgument(start <= end, "start offset can not be larger than end offset");
    //Preconditions.checkNotNull(bounds);
    if (start == end) {
        bounds.set(0, 0, 0, 0);
        return;
    }
    const int paraIndex = findParaIndex(start);
    const int paraStart = getParagraphStart(paraIndex);
    const int paraEnd = getParagraphEnd(paraIndex);
    if (start < paraStart || paraEnd < end) {
        //LOGE("Cannot measured across the paragraph:para(%d,%d) request(%d,%d)",paraStart,paraEnd,start,end);
    }
    getMeasuredParagraph(paraIndex)->getBounds(start - paraStart, end - paraStart, bounds);
}

float PrecomputedText::getCharWidthAt(int offset) const{
    const int paraIndex = findParaIndex(offset);
    const int paraStart = getParagraphStart(paraIndex);
    const int paraEnd = getParagraphEnd(paraIndex);
    return getMeasuredParagraph(paraIndex)->getCharWidthAt(offset - paraStart);
}

int PrecomputedText::getMemoryUsage() const{
    int r = 0;
    for (int i = 0; i < getParagraphCount(); ++i) {
        r += getMeasuredParagraph(i)->getMemoryUsage();
    }
    return r;
}

void PrecomputedText::setSpan(const ParcelableSpan* what, int start, int end, int flags) {
    if (dynamic_cast<const MetricAffectingSpan*>(what)) {
        //LOGE("MetricAffectingSpan can not be set to PrecomputedText.");
    }
    mText->setSpan(what, start, end, flags);
}

void PrecomputedText::removeSpan(const ParcelableSpan* what) {
    if (dynamic_cast<const MetricAffectingSpan*>(what)) {
        //LOGE("MetricAffectingSpan can not be removed from PrecomputedText.");
    }
    mText->removeSpan(what);
}

void PrecomputedText::getChars(int start, int end, char16_t* dest, int destPos) const{
}

std::string PrecomputedText::toString() const {
    return "";
}
}/*endof namespace*/
