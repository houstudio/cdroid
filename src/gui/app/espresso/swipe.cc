#include <app/espresso/swipe.h>

#include <app/espresso/motionevents.h>
#include <app/espresso/uicontroller.h>

#include <string>

#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <view/motionevent.h>

namespace cdroid {
namespace espresso {

static const char* TAG = "Swipe";

namespace {

/** The number of motion events to send for each swipe. */
constexpr int SWIPE_EVENT_COUNT = 10;
/** Length of time a "fast" swipe should last for, in milliseconds. */
constexpr int SWIPE_FAST_DURATION_MS = 100;
/** Length of time a "slow" swipe should last for, in milliseconds. */
constexpr int SWIPE_SLOW_DURATION_MS = 1500;

std::vector<FloatCoord> interpolate(const FloatCoord& start, const FloatCoord& end, int steps) {
    std::vector<FloatCoord> res(steps);
    for (int i = 1; i < steps + 1; i++) {
        res[i - 1][0] = start[0] + (end[0] - start[0]) * i / (steps + 2.f);
        res[i - 1][1] = start[1] + (end[1] - start[1]) * i / (steps + 2.f);
    }
    return res;
}

Swiper::Status sendLinearSwipe(UiController& uiController, const FloatCoord& startCoordinates,
        const FloatCoord& endCoordinates, const FloatCoord& precision, int duration) {
    const std::vector<FloatCoord> steps =
            interpolate(startCoordinates, endCoordinates, SWIPE_EVENT_COUNT);
    const int delayBetweenMovements = duration / (int)steps.size();

    MotionEvents::DownResultHolder down =
            MotionEvents::sendDown(uiController, startCoordinates, precision);
    Swiper::Status status = Swiper::Status::SUCCESS;
    try {
        for (size_t i = 0; i < steps.size(); i++) {
            if (!MotionEvents::sendMovement(uiController, *down.down, steps[i])) {
                LOGE("Injection of move event as part of the swipe failed. Sending cancel event.");
                MotionEvents::sendCancel(uiController, *down.down);
                return Swiper::Status::FAILURE;
            }

            const int64_t desiredTime =
                    down.down->getDownTime() + delayBetweenMovements * (int64_t)i;
            const int64_t timeUntilDesired = desiredTime - SystemClock::uptimeMillis();
            if (timeUntilDesired > 10) {
                uiController.loopMainThreadForAtLeast(timeUntilDesired);
            }
        }

        if (!MotionEvents::sendUp(uiController, *down.down, endCoordinates)) {
            LOGE("Injection of up event as part of the swipe failed. Sending cancel event.");
            MotionEvents::sendCancel(uiController, *down.down);
            return Swiper::Status::FAILURE;
        }
    } catch (...) {
        down.down->recycle();
        throw;
    }
    down.down->recycle();
    return status;
}

class FastSwipe : public Swiper {
public:
    std::string toString() const override { return "FAST"; }
    Swiper::Status sendSwipe(UiController& uiController, const FloatCoord& startCoordinates,
            const FloatCoord& endCoordinates, const FloatCoord& precision) override {
        return sendLinearSwipe(uiController, startCoordinates, endCoordinates, precision,
                SWIPE_FAST_DURATION_MS);
    }
};

class SlowSwipe : public Swiper {
public:
    std::string toString() const override { return "SLOW"; }
    Swiper::Status sendSwipe(UiController& uiController, const FloatCoord& startCoordinates,
            const FloatCoord& endCoordinates, const FloatCoord& precision) override {
        return sendLinearSwipe(uiController, startCoordinates, endCoordinates, precision,
                SWIPE_SLOW_DURATION_MS);
    }
};

} // namespace

SwiperPtr Swipe::FAST() { static SwiperPtr swiper = std::make_shared<FastSwipe>(); return swiper; }
SwiperPtr Swipe::SLOW() { static SwiperPtr swiper = std::make_shared<SlowSwipe>(); return swiper; }

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
