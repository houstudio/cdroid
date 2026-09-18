#ifndef CDROID_ESPRESSO_ADAPTERVIEWPROTOCOLS_H
#define CDROID_ESPRESSO_ADAPTERVIEWPROTOCOLS_H

/*
 * android.support.test.espresso.action.AdapterViewProtocols —
 * implementations of AdapterViewProtocol for standard SDK widgets.
 */

#include <memory>

#include <app/espresso/adapterviewprotocol.h>

namespace cdroid {
namespace espresso {

class AdapterViewProtocols {
public:
    /**
     * Creates an implementation of AdapterViewProtocol that can work with
     * AdapterViews that do not break method contracts on AdapterView.
     */
    static std::shared_ptr<AdapterViewProtocol> standardProtocol();

    AdapterViewProtocols() = delete;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ADAPTERVIEWPROTOCOLS_H*/
