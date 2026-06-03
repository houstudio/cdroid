#include <functional>
#include <core/spannablestring.h>
#include <view/view.h>
#include <text/textdirectionheuristics.h>
namespace cdroid{
using TextDirectionAlgorithm =std::function<int(CharSequence*,int,int)>;// checkRtl(CharSequence* cs, int start, int count);

static constexpr int STATE_TRUE = 0;
static constexpr int STATE_FALSE = 1;
static constexpr int STATE_UNKNOWN = 2;

static int isRtlCodePoint(int codePoint) {
#if 0
    switch (Character::getDirectionality(codePoint)) {
        case Character::DIRECTIONALITY_LEFT_TO_RIGHT:
            return STATE_FALSE;
        case Character::DIRECTIONALITY_RIGHT_TO_LEFT:
        case Character::DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC:
            return STATE_TRUE;
        case Character::DIRECTIONALITY_UNDEFINED:
            // Unassigned characters still have bidi direction, defined at:
            // http://www.unicode.org/Public/UCD/latest/ucd/extracted/DerivedBidiClass.txt

            if ((0x0590 <= codePoint && codePoint <= 0x08FF) ||
                    (0xFB1D <= codePoint && codePoint <= 0xFDCF) ||
                    (0xFDF0 <= codePoint && codePoint <= 0xFDFF) ||
                    (0xFE70 <= codePoint && codePoint <= 0xFEFF) ||
                    (0x10800 <= codePoint && codePoint <= 0x10FFF) ||
                    (0x1E800 <= codePoint && codePoint <= 0x1EFFF)) {
                // Unassigned RTL character
                return STATE_TRUE;
            } else if (
                    // Potentially-unassigned Default_Ignorable. Ranges are from unassigned
                    // characters that have Unicode property Other_Default_Ignorable_Code_Point
                    // plus some enlargening to cover bidi isolates and simplify checks.
                    (0x2065 <= codePoint && codePoint <= 0x2069) ||
                    (0xFFF0 <= codePoint && codePoint <= 0xFFF8) ||
                    (0xE0000 <= codePoint && codePoint <= 0xE0FFF) ||
                    // Non-character
                    (0xFDD0 <= codePoint && codePoint <= 0xFDEF) ||
                    ((codePoint & 0xFFFE) == 0xFFFE) ||
                    // Currency symbol
                    (0x20A0 <= codePoint && codePoint <= 0x20CF) ||
                    // Unpaired surrogate
                    (0xD800 <= codePoint && codePoint <= 0xDFFF)) {
                return STATE_UNKNOWN;
            } else {
                // Unassigned LTR character
                return STATE_FALSE;
            }
        default:
            return STATE_UNKNOWN;
    }
#else
    return 2;
#endif
}
class TextDirectionHeuristicImpl:public TextDirectionHeuristic {
private:
    TextDirectionAlgorithm mAlgorithm;
    bool doCheck(CharSequence* cs, int start, int count) const{
        switch(mAlgorithm/*.checkRtl*/(cs, start, count)) {
        case STATE_TRUE:  return true;
        case STATE_FALSE: return false;
        default: return defaultIsRtl();
        }
    }
protected:
    virtual bool defaultIsRtl()const =0;
public:
    TextDirectionHeuristicImpl(const TextDirectionAlgorithm& algorithm) {
        mAlgorithm = algorithm;
    }

public:
    bool isRtl(const char32_t* array, int start, int count)const override{
        return 0;//isRtl(CharBuffer.wrap(array), start, count);
    }

    bool isRtl(CharSequence* cs, int start, int count)const override{
        if (cs == nullptr || start < 0 || count < 0 || cs->length() - count < start) {
            //throw new IllegalArgumentException();
        }
        if (mAlgorithm == nullptr) {
            return defaultIsRtl();
        }
        return doCheck(cs, start, count);
    }
};

class TextDirectionHeuristicInternal:public TextDirectionHeuristicImpl {
private:
    bool mDefaultIsRtl;
protected:
    bool defaultIsRtl()const override{
        return mDefaultIsRtl;
    }
public:
    TextDirectionHeuristicInternal(const TextDirectionAlgorithm& algorithm, bool defaultIsRtl)
        :TextDirectionHeuristicImpl(algorithm){
        mDefaultIsRtl = defaultIsRtl;
    }
};

//class FirstStrong :public TextDirectionAlgorithm {
static int FirstString_checkRtl(CharSequence* cs, int start, int count) {
    int result = STATE_UNKNOWN;
    int openIsolateCount = 0;
    /*for (int cp, i = start, end = start + count;
            i < end && result == STATE_UNKNOWN;
            i += Character.charCount(cp)) {
        cp = Character.codePointAt(cs, i);
        if (0x2066 <= cp && cp <= 0x2068) { // Opening isolates
            openIsolateCount += 1;
        } else if (cp == 0x2069) { // POP DIRECTIONAL ISOLATE (PDI)
            if (openIsolateCount > 0) openIsolateCount -= 1;
        } else if (openIsolateCount == 0) {
            // Only consider the characters outside isolate pairs
            result = isRtlCodePoint(cp);
        }
    }*/
    return result;
}


//class AnyStrong implements TextDirectionAlgorithm {

static int AnyStrong_checkRtl(CharSequence* cs, int start, int count,bool mLookForRtl) {
    bool haveUnlookedFor = false;
    int openIsolateCount = 0;
    for (int cp, i = start, end = start + count; i < end; i += 1/*Character.charCount(cp)*/) {
        cp = 0;//Character.codePointAt(cs, i);
        if (0x2066 <= cp && cp <= 0x2068) { // Opening isolates
            openIsolateCount += 1;
        } else if (cp == 0x2069) { // POP DIRECTIONAL ISOLATE (PDI)
            if (openIsolateCount > 0) openIsolateCount -= 1;
        } else if (openIsolateCount == 0) {
            // Only consider the characters outside isolate pairs
            switch (isRtlCodePoint(cp)) {
                case STATE_TRUE:
                    if (mLookForRtl) {
                        return STATE_TRUE;
                    }
                    haveUnlookedFor = true;
                    break;
                case STATE_FALSE:
                    if (!mLookForRtl) {
                        return STATE_FALSE;
                    }
                    haveUnlookedFor = true;
                    break;
                default:
                    break;
            }
        }
    }
    if (haveUnlookedFor) {
        return mLookForRtl ? STATE_FALSE : STATE_TRUE;
    }
    return STATE_UNKNOWN;
}

/**
 * Algorithm that uses the Locale direction to force the direction of a paragraph.
 */
class TextDirectionHeuristicLocale :public TextDirectionHeuristicImpl {
public:
    TextDirectionHeuristicLocale() :TextDirectionHeuristicImpl(nullptr){}
protected:
    bool defaultIsRtl() const override{
        const int dir = View::LAYOUT_DIRECTION_LTR;//TextUtils.getLayoutDirectionFromLocale(java.util.Locale.getDefault());
        return (dir == View::LAYOUT_DIRECTION_RTL);
    }
};

namespace{
    TextDirectionAlgorithm FIRSTSTRONG_INSTANCE = FirstString_checkRtl;
    TextDirectionAlgorithm ANYSTRONG_RTL = [](CharSequence* cs, int start, int count){
        return AnyStrong_checkRtl(cs,start,count,true);
    };

    TextDirectionAlgorithm ANYSTRONG_LTR =  [](CharSequence* cs, int start, int count){
        return AnyStrong_checkRtl(cs,start,count,true);
    };

    const TextDirectionHeuristicInternal TDH_LTR{nullptr /* no algorithm */, false};
    const TextDirectionHeuristicInternal TDH_RTL{nullptr /* no algorithm */, true};
    const TextDirectionHeuristicInternal TDH_FIRSTSTRONG_LTR{FIRSTSTRONG_INSTANCE,false};
    const TextDirectionHeuristicInternal TDH_FIRSTSTRONG_RTL{FIRSTSTRONG_INSTANCE,true};
    const TextDirectionHeuristicInternal TDH_ANYRTL_LTR{ANYSTRONG_RTL,false};
    const TextDirectionHeuristicLocale   TDH_LOCALE;
}
const TextDirectionHeuristic*const TextDirectionHeuristics::LTR=&TDH_LTR;
const TextDirectionHeuristic*const TextDirectionHeuristics::RTL=&TDH_RTL;
const TextDirectionHeuristic*const TextDirectionHeuristics::FIRSTSTRONG_LTR=&TDH_FIRSTSTRONG_LTR;
const TextDirectionHeuristic*const TextDirectionHeuristics::FIRSTSTRONG_RTL=&TDH_FIRSTSTRONG_RTL;
const TextDirectionHeuristic*const TextDirectionHeuristics::ANYRTL_LTR=&TDH_ANYRTL_LTR;
const TextDirectionHeuristic*const TextDirectionHeuristics::LOCALE=&TDH_LOCALE;
}/*endof namespace*/
