#ifndef __TEXT_DIRECTION_HERISTICS_H__
#define __TEXT_DIRECTION_HERISTICS_H__
namespace cdroid{
class CharSequence;
struct TextDirectionHeuristic {
    virtual bool isRtl(const char* array, int start, int count)=0;
    virtual bool isRtl(CharSequence* cs, int start, int count)=0;
};
class TextDirectionHeuristics {
public:
    /**
     * Always decides that the direction is left to right.
     */
    static const TextDirectionHeuristic*const LTR;

    /**
     * Always decides that the direction is right to left.
     */
    static const TextDirectionHeuristic*const RTL;

    /**
     * Determines the direction based on the first strong directional character, including bidi
     * format chars, falling back to left to right if it finds none. This is the default behavior
     * of the Unicode Bidirectional Algorithm.
     */
    static const TextDirectionHeuristic*const FIRSTSTRONG_LTR;

    /**
     * Determines the direction based on the first strong directional character, including bidi
     * format chars, falling back to right to left if it finds none. This is similar to the default
     * behavior of the Unicode Bidirectional Algorithm, just with different fallback behavior.
     */
    static const TextDirectionHeuristic*const FIRSTSTRONG_RTL;

    /**
     * If the text contains any strong right to left non-format character, determines that the
     * direction is right to left, falling back to left to right if it finds none.
     */
    static const TextDirectionHeuristic*const ANYRTL_LTR;

    /**
     * Force the paragraph direction to the Locale direction. Falls back to left to right.
     */
    static const TextDirectionHeuristic*const  LOCALE;
};
}/*endof namespace*/
#endif/*__TEXT_DIRECTION_HERISTICS_H__*/
