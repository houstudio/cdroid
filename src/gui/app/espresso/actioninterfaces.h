#ifndef CDROID_ESPRESSO_ACTIONINTERFACES_H
#define CDROID_ESPRESSO_ACTIONINTERFACES_H

/*
 * The android.support.test.espresso.action leaf interfaces: CoordinatesProvider,
 * Tapper (+ Status), Swiper (+ Status), PrecisionDescriber. Java float[]
 * coordinates map onto std::array<float, 2>.
 */

#include <array>
#include <memory>
#include <string>

namespace cdroid {
class View;

namespace espresso {

class UiController;

/** AOSP float[] {x, y} coordinate pair. */
using FloatCoord = std::array<float, 2>;

/** android.support.test.espresso.action.CoordinatesProvider. */
class CoordinatesProvider {
public:
    virtual ~CoordinatesProvider() = default;
    /** Calculates coordinates of a view. */
    virtual FloatCoord calculateCoordinates(View& view) = 0;
};

using CoordinatesProviderPtr = std::shared_ptr<CoordinatesProvider>;

/** android.support.test.espresso.action.Tapper. */
class Tapper {
public:
    enum class Status { SUCCESS, FAILURE, WARNING };

    virtual ~Tapper() = default;
    /** Sends a single tap to the given coordinates. */
    virtual Status sendTap(UiController& uiController, const FloatCoord& coordinates,
            const FloatCoord& precision) = 0;
    /** Java enum name ("SINGLE"/"LONG"/"DOUBLE") — feeds action descriptions. */
    virtual std::string toString() const = 0;
};

using TapperPtr = std::shared_ptr<Tapper>;

/** android.support.test.espresso.action.Swiper. */
class Swiper {
public:
    enum class Status { SUCCESS, FAILURE };

    virtual ~Swiper() = default;
    /** Sends a swipe from the start to the end coordinates. */
    virtual Status sendSwipe(UiController& uiController, const FloatCoord& startCoordinates,
            const FloatCoord& endCoordinates, const FloatCoord& precision) = 0;
    /** Java enum name ("FAST"/"SLOW") — feeds action descriptions. */
    virtual std::string toString() const = 0;
};

using SwiperPtr = std::shared_ptr<Swiper>;

/** android.support.test.espresso.action.PrecisionDescriber. */
class PrecisionDescriber {
public:
    virtual ~PrecisionDescriber() = default;
    /** Describes the precision of the touch. */
    virtual FloatCoord describePrecision() = 0;
};

using PrecisionDescriberPtr = std::shared_ptr<PrecisionDescriber>;

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ACTIONINTERFACES_H*/
