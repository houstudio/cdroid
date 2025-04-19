#ifndef __JUSTIFY_CONTENT_H__
#define __JUSTIFY_CONTENT_H__
namespace cdroid{
enum JustifyContent {

    /** Flex items are packed toward the start line. */
    FLEX_START = 0,

    /** Flex items are packed toward the end line. */
    FLEX_END = 1,

    /** Flex items are centered along the flex line where the flex items belong. */
    CENTER = 2,

    /**
     * Flex items are evenly distributed along the flex line, first flex item is on the
     * start line, the last flex item is on the end line.
     */
    SPACE_BETWEEN = 3,

    /**
     * Flex items are evenly distributed along the flex line with the same amount of spaces between
     * the flex lines.
     */
    SPACE_AROUND = 4,

    /**
     * Flex items are evenly distributed along the flex line. The difference between
     * {@link #SPACE_AROUND} is that all the spaces between items should be the same as the
     * space before the first item and after the last item.
     * See
     * <a href="https://developer.mozilla.org/en-US/docs/Web/CSS/justify-content">the document on MDN</a>
     * for more details.
     */
    SPACE_EVENLY = 5
};
}/*endof namespace*/
#endif/*__JUSTIFY_CONTENT_H__*/
