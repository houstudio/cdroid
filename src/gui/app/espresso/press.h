#ifndef CDROID_ESPRESSO_PRESS_H
#define CDROID_ESPRESSO_PRESS_H

/*
 * android.support.test.espresso.action.Press — returns different touch
 * target sizes. AOSP enum → static provider instances.
 */

#include <app/espresso/actioninterfaces.h>

namespace cdroid {
namespace espresso {

class Press {
public:
    static PrecisionDescriberPtr PINPOINT();
    /** Average width of the index finger is 16–20 mm. */
    static PrecisionDescriberPtr FINGER();
    /** Average width of an adult thumb is 25 mm (1 inch). */
    static PrecisionDescriberPtr THUMB();

private:
    Press() = default;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_PRESS_H*/
