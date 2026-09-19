#include <app/espresso/adapterviewprotocol.h>

#include <stdexcept>

#include <cstdio>

namespace cdroid {
namespace espresso {

namespace {
/** The AOSP anonymous "return the fixed data" DataFunction. */
class FixedDataFunction : public DataFunction {
public:
    explicit FixedDataFunction(void* data) : mData(data) {}
    void* getData() override { return mData; }
private:
    void* mData;
};
} // namespace

AdaptedData::AdaptedData(void* /*data*/, void* opaqueToken,
        std::shared_ptr<DataFunction> dataFunction)
    : opaqueToken(opaqueToken), mDataFunction(std::move(dataFunction)) {
    // AOSP ctor: checkNotNull(opaqueToken / dataFunction). The token check
    // cannot survive this port's encoding: the standard protocol stores the
    // adapter POSITION as the token, and position 0 encodes to a null void*.
    // Token equality (what DisplayDataMatcher relies on) is unaffected.
    if (mDataFunction == nullptr) {
        throw std::invalid_argument("dataFunction cannot be null");
    }
}

AdaptedData AdaptedData::Builder::build() {
    if (mDataFunction == nullptr) {
        mDataFunction = std::make_shared<FixedDataFunction>(mData);
    }
    return AdaptedData(mData, mOpaqueToken, std::move(mDataFunction));
}

std::string AdaptedData::toString() const {
    // AOSP: "Data: %s (class: %s) token: %s" via toString(); with void*
    // data there is no toString — print the pointers (see file header).
    char buffer[96];
    snprintf(buffer, sizeof(buffer), "Data: %p token: %p", getData(), opaqueToken);
    return buffer;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
