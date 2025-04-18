#ifndef __ALIGN_CONTENT_H__
#define __ALIGN_CONTENT_H__
namespace cdroid{
enum class AlignContent {

    /** Flex lines are packed to the start of the flex container. */
    FLEX_START = 0,

    /** Flex lines are packed to the end of the flex container. */
    FLEX_END = 1,

    /** Flex lines are centered in the flex container. */
    CENTER = 2,

    /**
     * Flex lines are evenly distributed in the flex container. The first flex line is
     * placed at the start of the flex container, the last flex line is placed at the
     * end of the flex container.
     */
    SPACE_BETWEEN = 3,

    /**
     * Flex lines are evenly distributed in the flex container with the same amount of spaces
     * between the flex lines.
     */
    SPACE_AROUND = 4,

    /**
     * Flex lines are stretched to fill the remaining space along the cross axis.
     */
    STRETCH = 5
};

enum class AlignItems {

    /** Flex item's edge is placed on the cross start line. */
    FLEX_START = 0,

    /** Flex item's edge is placed on the cross end line. */
    FLEX_END = 1,

    /** Flex item's edge is centered along the cross axis. */
    CENTER = 2,

    /** Flex items are aligned based on their text's baselines. */
    BASELINE = 3,

    /** Flex items are stretched to fill the flex line's cross size. */
    STRETCH = 4
};

enum class AlignSelf {
    /**
     * The default value for the AlignSelf attribute, which means use the inherit
     * the {@link AlignItems} attribute from its parent.
     */
    AUTO = -1,

    /** This item's edge is placed on the cross start line. */
    FLEX_START = 0,//AlignItems::FLEX_START,

    /** This item's edge is placed on the cross end line. */
    FLEX_END = 1,//AlignItems::FLEX_END,

    /** This item's edge is centered along the cross axis. */
    CENTER = 2,//AlignItems::CENTER,

    /** This items is aligned based on their text's baselines. */
    BASELINE = 3,//AlignItems::BASELINE,

    /** This item is stretched to fill the flex line's cross size. */
    STRETCH = 4//AlignItems::STRETCH
};

}
#endif
