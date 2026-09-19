#include <app/espresso/press.h>

namespace cdroid {
namespace espresso {

namespace {

class FixedPrecision : public PrecisionDescriber {
public:
    explicit FixedPrecision(FloatCoord precision) : mPrecision(precision) {}
    FloatCoord describePrecision() override { return mPrecision; }
private:
    FloatCoord mPrecision;
};

} // namespace

PrecisionDescriberPtr Press::PINPOINT() {
    return std::make_shared<FixedPrecision>(FloatCoord{1.0f, 1.0f});
}

PrecisionDescriberPtr Press::FINGER() {
    return std::make_shared<FixedPrecision>(FloatCoord{16.0f, 16.0f});
}

PrecisionDescriberPtr Press::THUMB() {
    return std::make_shared<FixedPrecision>(FloatCoord{25.0f, 25.0f});
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
