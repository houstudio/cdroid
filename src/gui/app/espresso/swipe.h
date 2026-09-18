#ifndef CDROID_ESPRESSO_SWIPE_H
#define CDROID_ESPRESSO_SWIPE_H

/*
 * android.support.test.espresso.action.Swipe — executes different swipe
 * types to given positions. AOSP enum → static Swiper instances.
 */

#include <app/espresso/actioninterfaces.h>

namespace cdroid {
namespace espresso {

class Swipe {
public:
    /** Swipes quickly between the co-ordinates. */
    static SwiperPtr FAST();
    /** Swipes deliberately slowly between the co-ordinates, to aid in visual debugging. */
    static SwiperPtr SLOW();

private:
    Swipe() = default;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_SWIPE_H*/
