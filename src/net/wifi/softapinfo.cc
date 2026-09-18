/* Port of android.net.wifi.SoftApInfo (android-36), operating-state core. */
#include <wifi/softapinfo.h>

namespace cdroid {

std::string SoftApInfo::toString() const {
    /* Same vocabulary as the AOSP toString(): frequency/bandwidth/standard. */
    return "SoftApInfo{frequency = " + std::to_string(frequency)
            + ", bandwidth = " + std::to_string(bandwidth)
            + ", wifiStandard = " + std::to_string(wifiStandard) + "}";
}

} // namespace cdroid
