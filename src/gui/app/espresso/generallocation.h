#ifndef CDROID_ESPRESSO_GENERALLOCATION_H
#define CDROID_ESPRESSO_GENERALLOCATION_H

/*
 * android.support.test.espresso.action.GeneralLocation — calculates
 * coordinate positions for general locations. AOSP is an enum implementing
 * CoordinatesProvider; the enum constants become static provider instances.
 */

#include <app/espresso/actioninterfaces.h>

namespace cdroid {
namespace espresso {

class GeneralLocation {
public:
    static CoordinatesProviderPtr TOP_LEFT();
    static CoordinatesProviderPtr TOP_CENTER();
    static CoordinatesProviderPtr TOP_RIGHT();
    static CoordinatesProviderPtr CENTER_LEFT();
    static CoordinatesProviderPtr CENTER();
    static CoordinatesProviderPtr CENTER_RIGHT();
    static CoordinatesProviderPtr BOTTOM_LEFT();
    static CoordinatesProviderPtr BOTTOM_CENTER();
    static CoordinatesProviderPtr BOTTOM_RIGHT();
    static CoordinatesProviderPtr VISIBLE_CENTER();

    /**
     * Translates the given coordinates by the given distances. The distances
     * are given in terms of the view's size — 1.0 means to translate by an
     * amount equivalent to the view's length.
     */
    static CoordinatesProviderPtr translate(CoordinatesProviderPtr coords, float dx, float dy);

    // AOSP private helpers, kept visible for the provider instances below.
    enum class Position { BEGIN, MIDDLE, END };
    static float getPosition(Position position, int viewPos, int viewLength);
    static FloatCoord getCoordinates(View& view, Position vertical, Position horizontal);
    static FloatCoord getCoordinatesOfVisiblePart(View& view, Position vertical,
            Position horizontal);

private:
    GeneralLocation() = default;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_GENERALLOCATION_H*/
