#ifndef __FLEX_WRAP_H__
#define __FLEX_WRAP_H__
namespace cdroid{
enum FlexWrap {

    /** The flex container is single-line. */
    NOWRAP = 0,

    /** The flex container is multi-line. */
    WRAP = 1,

    /**
     * The flex container is multi-line. The direction of the
     * cross axis is opposed to the direction as the {@link #WRAP}
     */
    WRAP_REVERSE = 2
};
}
#endif
