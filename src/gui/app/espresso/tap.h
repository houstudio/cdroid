#ifndef CDROID_ESPRESSO_TAP_H
#define CDROID_ESPRESSO_TAP_H

/*
 * android.support.test.espresso.action.Tap — executes different click types
 * to given position. AOSP enum → static Tapper instances.
 */

#include <app/espresso/actioninterfaces.h>

#include <string>

namespace cdroid {
namespace espresso {

class Tap {
public:
    static TapperPtr SINGLE();
    static TapperPtr LONG();
    static TapperPtr DOUBLE();

private:
    Tap() = default;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_TAP_H*/
