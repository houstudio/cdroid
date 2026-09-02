#include <app/espresso/tap.h>

#include <app/espresso/motionevents.h>
#include <app/espresso/uicontroller.h>

#include <porting/cdlog.h>
#include <view/motionevent.h>
#include <view/viewconfiguration.h>

namespace cdroid {
namespace espresso {

static const char* TAG = "Tap";

namespace {

Tapper::Status sendSingleTap(UiController& uiController, const FloatCoord& coordinates,
        const FloatCoord& precision) {
    MotionEvents::DownResultHolder res = MotionEvents::sendDown(uiController, coordinates, precision);
    Tapper::Status status = res.longPress ? Tapper::Status::WARNING : Tapper::Status::SUCCESS;
    try {
        if (!MotionEvents::sendUp(uiController, *res.down)) {
            LOGD("Injection of up event as part of the click failed. Send cancel event.");
            MotionEvents::sendCancel(uiController, *res.down);
            return Tapper::Status::FAILURE;
        }
    } catch (...) {
        res.down->recycle();
        throw;
    }
    res.down->recycle();
    return status;
}

class SingleTap : public Tapper {
public:
    std::string toString() const override { return "SINGLE"; }
    Tapper::Status sendTap(UiController& uiController, const FloatCoord& coordinates,
            const FloatCoord& precision) override {
        Tapper::Status stat = sendSingleTap(uiController, coordinates, precision);
        if (stat == Tapper::Status::SUCCESS) {
            // Wait until the touch event was processed by the main thread.
            const int64_t singlePressTimeout =
                    (int64_t)(ViewConfiguration::getTapTimeout() * 1.5f);
            uiController.loopMainThreadForAtLeast(singlePressTimeout);
        }
        return stat;
    }
};

class LongTap : public Tapper {
public:
    std::string toString() const override { return "LONG"; }
    Tapper::Status sendTap(UiController& uiController, const FloatCoord& coordinates,
            const FloatCoord& precision) override {
        MotionEvents::DownResultHolder res =
                MotionEvents::sendDown(uiController, coordinates, precision);
        try {
            // Duration before a press turns into a long press.
            // Factor 1.5 is needed, otherwise a long press is not safely detected.
            // See android.test.TouchUtils longClickView
            const int64_t longPressTimeout =
                    (int64_t)(ViewConfiguration::getLongPressTimeout() * 1.5f);
            uiController.loopMainThreadForAtLeast(longPressTimeout);

            if (!MotionEvents::sendUp(uiController, *res.down)) {
                MotionEvents::sendCancel(uiController, *res.down);
                return Tapper::Status::FAILURE;
            }
        } catch (...) {
            res.down->recycle();
            throw;
        }
        res.down->recycle();
        return Tapper::Status::SUCCESS;
    }
};

class DoubleTap : public Tapper {
public:
    std::string toString() const override { return "DOUBLE"; }
    Tapper::Status sendTap(UiController& uiController, const FloatCoord& coordinates,
            const FloatCoord& precision) override {
        Tapper::Status stat = sendSingleTap(uiController, coordinates, precision);
        if (stat == Tapper::Status::FAILURE) {
            return Tapper::Status::FAILURE;
        }

        // AOSP reflects ViewConfiguration.getDoubleTapMinTime() (hidden until
        // API 18); the CDROID port exposes it directly.
        const int64_t doubleTapMinTimeout = ViewConfiguration::getDoubleTapMinTime();
        if (0 < doubleTapMinTimeout) {
            uiController.loopMainThreadForAtLeast(doubleTapMinTimeout);
        }

        Tapper::Status secondStat = sendSingleTap(uiController, coordinates, precision);
        if (secondStat == Tapper::Status::FAILURE) {
            return Tapper::Status::FAILURE;
        }

        if (secondStat == Tapper::Status::WARNING || stat == Tapper::Status::WARNING) {
            return Tapper::Status::WARNING;
        }
        return Tapper::Status::SUCCESS;
    }
};

} // namespace

TapperPtr Tap::SINGLE() { static TapperPtr tapper = std::make_shared<SingleTap>(); return tapper; }
TapperPtr Tap::LONG() { static TapperPtr tapper = std::make_shared<LongTap>(); return tapper; }
TapperPtr Tap::DOUBLE() { static TapperPtr tapper = std::make_shared<DoubleTap>(); return tapper; }

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
