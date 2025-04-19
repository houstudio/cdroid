#ifndef __FLEX_DIRECTION_H__
#define __FLEX_DIRECTION_H__
namespace cdroid{
enum FlexDirection {

    /**
     * Main axis direction -> horizontal. Main start to
     * main end -> Left to right (in LTR languages).
     * Cross start to cross end -> Top to bottom
     */
    ROW = 0,

    /**
     * Main axis direction -> horizontal. Main start
     * to main end -> Right to left (in LTR languages). Cross start to cross end ->
     * Top to bottom.
     */
    ROW_REVERSE = 1,

    /**
     * Main axis direction -> vertical. Main start
     * to main end -> Top to bottom. Cross start to cross end ->
     * Left to right (In LTR languages).
     */
    COLUMN = 2,

    /**
     * Main axis direction -> vertical. Main start
     * to main end -> Bottom to top. Cross start to cross end -> Left to right
     * (In LTR languages)
     */
    COLUMN_REVERSE = 3
};
}
#endif
