#ifndef __SOFT_AP_CAPABILITY_H__
#define __SOFT_AP_CAPABILITY_H__

#include <cstdint>
#include <string>

namespace cdroid {

/**
 * Port of android.net.wifi.SoftApCapability (android-36), trimmed to the
 * feature set the hostapd backend can speak. Delivered through
 * WifiManager.SoftApCallback#onCapabilityChanged.
 *
 * The AOSP parceling and the "override-able features" bit map are TODO
 * (no Parcel, no carrier config); areFeaturesSupported() works on the same
 * feature bits as the original.
 */
class SoftApCapability {
public:
    /* Feature bits (SoftApCapability#SOFTAP_FEATURE_*). */
    static constexpr int64_t SOFTAP_FEATURE_ACS_OFFLOAD = 1 << 0;
    static constexpr int64_t SOFTAP_FEATURE_CLIENT_FORCE_DISCONNECT = 1 << 1;
    static constexpr int64_t SOFTAP_FEATURE_MAC_ADDRESS_CUSTOMIZATION = 1 << 2;
    static constexpr int64_t SOFTAP_FEATURE_IEEE80211_AX = 1 << 3;
    static constexpr int64_t SOFTAP_FEATURE_IEEE80211_BE = 1 << 4;
    static constexpr int64_t SOFTAP_FEATURE_STA_AP_CONCURRENCY = 1 << 5;

    /* AOSP default: no maximum (0); the hostapd backend caps at its
     * max_num_sta default unless the configuration narrows it. */
    static constexpr int32_t MAX_SUPPORTED_CLIENTS_NO_LIMIT = 0;

    SoftApCapability() = default;
    explicit SoftApCapability(int64_t supportedFeatures)
        : mSupportedFeatures(supportedFeatures) {}

    int64_t getSupportedFeatures() const { return mSupportedFeatures; }
    /* AOSP: areFeaturesSupported(long features) — false when any requested
     * feature is unsupported. */
    bool areFeaturesSupported(int64_t features) const {
        return (mSupportedFeatures & features) == features;
    }
    int32_t getMaxSupportedClients() const { return mMaxSupportedClients; }
    void setMaxSupportedClients(int32_t maxSupportedClients) {
        mMaxSupportedClients = maxSupportedClients;
    }

    bool operator==(const SoftApCapability& other) const {
        return mSupportedFeatures == other.mSupportedFeatures
                && mMaxSupportedClients == other.mMaxSupportedClients;
    }

    std::string toString() const {
        return "SoftApCapability{mSupportedFeatures = "
                + std::to_string(mSupportedFeatures)
                + ", mMaxSupportedClients = " + std::to_string(mMaxSupportedClients)
                + "}";
    }

private:
    int64_t mSupportedFeatures = 0;
    int32_t mMaxSupportedClients = MAX_SUPPORTED_CLIENTS_NO_LIMIT;
};

} // namespace cdroid

#endif /* __SOFT_AP_CAPABILITY_H__ */
