#ifndef __SOFT_AP_CONFIGURATION_H__
#define __SOFT_AP_CONFIGURATION_H__

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <macaddress.h>
#include <wifi/scanresult.h>
#include <wifi/softapinfo.h>
#include <wifi/wifissid.h>

namespace cdroid {

class WifiConfiguration;

/**
 * Port of android.net.wifi.SoftApConfiguration (android-36): configuration
 * for a soft access point (Soft AP / hotspot). Immutable; built through
 * SoftApConfiguration::Builder.
 *
 * Trims and translations (the module has no binder/parcel layer):
 *   - SparseIntArray  -> std::map<int,int> (kept sorted: "keyAt(0)" is the
 *     lowest band, matching the SparseIntArray ordering the AOSP getters
 *     rely on).
 *   - Parcelable      -> omitted (no Parcel in cdnet).
 *   - OuiKeyedData vendorData -> TODO(porting): type not ported.
 *   - SdkLevel/CompatChanges gates resolve to the android-36 runtime
 *     behavior (always at least T/B): the pre-T passphrase length check is
 *     documented where it lived, the REMOVE_ZERO timeout rewrite and the
 *     BSSID/randomization exclusivity check apply in their final form.
 *   - Java null fields use value sentinels: WifiSsid bytes empty, MacAddress
 *     empty, passphrase empty string (PSK never legitimately is).
 */
class SoftApConfiguration {
public:
    static constexpr int PSK_MIN_LEN = 8;    /* @VisibleForTesting in AOSP */
    static constexpr int PSK_MAX_LEN = 63;   /* @VisibleForTesting in AOSP */

    /* Operating bands (bit mask). */
    static constexpr int BAND_2GHZ  = 1 << 0;
    static constexpr int BAND_5GHZ  = 1 << 1;
    static constexpr int BAND_6GHZ  = 1 << 2;
    static constexpr int BAND_60GHZ = 1 << 3;
    /* @deprecated in AOSP; still the "framework picks" value. */
    static constexpr int BAND_ANY = BAND_2GHZ | BAND_5GHZ | BAND_6GHZ;

    /* Shutdown timeout: framework default (per-country/carrier). */
    static constexpr int64_t DEFAULT_TIMEOUT = -1;

    /* BSSID MAC randomization level. */
    static constexpr int RANDOMIZATION_NONE          = 0;
    static constexpr int RANDOMIZATION_PERSISTENT    = 1;
    static constexpr int RANDOMIZATION_NON_PERSISTENT = 2;

    /* Security types. */
    static constexpr int SECURITY_TYPE_OPEN                = 0;
    static constexpr int SECURITY_TYPE_WPA2_PSK            = 1;
    static constexpr int SECURITY_TYPE_WPA3_SAE_TRANSITION = 2;
    static constexpr int SECURITY_TYPE_WPA3_SAE            = 3;
    static constexpr int SECURITY_TYPE_WPA3_OWE_TRANSITION = 4;
    static constexpr int SECURITY_TYPE_WPA3_OWE            = 5;

    /* AOSP has no public no-arg constructor: in Java an unset configuration
     * is a null reference. The C++ port keeps unset members as defaulted
     * instances paired with a validity flag (WifiManager::mSoftApConfig +
     * mSoftApConfigSet, SoftApConfigStore::Record::hasConfig), which needs
     * this constructor public — so a defaulted instance carries an EMPTY
     * mChannels and every accessor must tolerate it (getBand()/getChannel()
     * return the documented defaults on empty instead of dereferencing
     * end(); read the flag before believing the values). */
    SoftApConfiguration() = default;
    SoftApConfiguration(const SoftApConfiguration&) = default;
    SoftApConfiguration& operator=(const SoftApConfiguration&) = default;

    /* --- getters ---------------------------------------------------------- */

    /* @deprecated UTF-8 SSID text; WifiManager.UNKNOWN_SSID ("<unknown
     * ssid>") when the SSID bytes are not valid UTF-8, empty when no SSID
     * was configured (Java null). */
    std::string getSsid() const;
    /* Configured SSID; empty bytes = not configured (Java null). */
    WifiSsid getWifiSsid() const { return mWifiSsid; }
    /* Configured BSSID; empty MacAddress = not configured (Java null). */
    MacAddress getBssid() const { return mBssid; }
    /* WPA2/WPA3 passphrase; empty = none (Java null). */
    std::string getPassphrase() const { return mPassphrase; }
    bool isHiddenSsid() const { return mHiddenSsid; }

    /* @deprecated lowest configured band. */
    int getBand() const;
    /* Sorted configured bands (map key order == AOSP keyAt order). */
    std::vector<int> getBands() const;
    /* @deprecated channel of the lowest configured band (0 = auto). */
    int getChannel() const;
    /* band -> channel map (0 = auto-select); copy, like mChannels.clone(). */
    std::map<int, int> getChannels() const { return mChannels; }

    int getSecurityType() const { return mSecurityType; }
    int getMaxNumberOfClients() const { return mMaxNumberOfClients; }
    bool isAutoShutdownEnabled() const { return mAutoShutdownEnabled; }
    /* DEFAULT_TIMEOUT passes through as-is (the REMOVE_ZERO compat rewrite
     * is resolved away on android-36). */
    int64_t getShutdownTimeoutMillis() const { return mShutdownTimeoutMillis; }
    bool isClientControlByUserEnabled() const { return mClientControlByUser; }
    std::vector<MacAddress> getBlockedClientList() const { return mBlockedClientList; }
    std::vector<MacAddress> getAllowedClientList() const { return mAllowedClientList; }
    int getMacRandomizationSetting() const { return mMacRandomizationSetting; }
    bool isBridgedModeOpportunisticShutdownEnabled() const {
        return mBridgedModeOpportunisticShutdownEnabled;
    }
    bool isIeee80211axEnabled() const { return mIeee80211axEnabled; }
    bool isIeee80211beEnabled() const { return mIeee80211beEnabled; }
    bool isUserConfiguration() const { return mIsUserConfiguration; }
    MacAddress getPersistentRandomizedMacAddress() const {
        return mPersistentRandomizedMacAddress;
    }
    int64_t getBridgedModeOpportunisticShutdownTimeoutMillis() const {
        return mBridgedModeOpportunisticShutdownTimeoutMillis;
    }
    std::vector<ScanResult::InformationElement> getVendorElements() const {
        return mVendorElements;
    }
    std::set<int> getAllowedAcsChannels(int band) const;
    int getMaxChannelBandwidth() const { return mMaxChannelBandwidth; }
    bool isClientIsolationEnabled() const { return mIsClientIsolationEnabled; }

    /**
     * @return a WifiConfiguration representation, or nullptr when the
     * configuration cannot be represented (unsupported security type or
     * band). Caller owns the result — Java @Nullable.
     */
    WifiConfiguration* toWifiConfiguration() const;

    bool operator==(const SoftApConfiguration& other) const;
    bool operator!=(const SoftApConfiguration& other) const;
    std::string toString() const;

    static bool isBandValid(int band);
    /* 2.4/5/6/60GHz channel number validity per band (AOSP private helper,
     * exposed for tests mirroring @VisibleForTesting). */
    static bool isChannelBandPairValid(int channel, int band);

    /**
     * Builder — all fields optional; defaults: no SSID/BSSID, open network,
     * BAND_2GHZ with auto channel (0), auto-shutdown enabled.
     */
    class Builder {
    public:
        Builder();
        Builder(const SoftApConfiguration& other);

        Builder& setSsid(const std::string& ssid);          /* @deprecated */
        Builder& setWifiSsid(const WifiSsid& wifiSsid);
        Builder& setBssid(const MacAddress& bssid);
        Builder& setPassphrase(const std::string& passphrase, int securityType);
        Builder& setHiddenSsid(bool hiddenSsid);
        Builder& setBand(int band);
        Builder& setBands(const std::vector<int>& bands);
        Builder& setChannel(int channel, int band);
        Builder& setChannels(const std::map<int, int>& channels);
        Builder& setMaxNumberOfClients(int maxNumberOfClients);
        Builder& setAutoShutdownEnabled(bool enable);
        Builder& setShutdownTimeoutMillis(int64_t timeoutMillis);
        Builder& setClientControlByUserEnabled(bool enabled);
        Builder& setAllowedClientList(const std::vector<MacAddress>& allowedClientList);
        Builder& setBlockedClientList(const std::vector<MacAddress>& blockedClientList);
        Builder& setMacRandomizationSetting(int macRandomizationSetting);
        Builder& setBridgedModeOpportunisticShutdownEnabled(bool enable);
        Builder& setIeee80211axEnabled(bool enable);
        Builder& setIeee80211beEnabled(bool enable);
        Builder& setUserConfiguration(bool isUserConfigured);
        Builder& setBridgedModeOpportunisticShutdownTimeoutMillis(int64_t timeoutMillis);
        Builder& setVendorElements(
                const std::vector<ScanResult::InformationElement>& vendorElements);
        Builder& setAllowedAcsChannels(int band, const std::vector<int>& channels);
        Builder& setMaxChannelBandwidth(int maxChannelBandwidth);
        Builder& setClientIsolationEnabled(bool isClientIsolationEnabled);

        SoftApConfiguration build();
        /* @VisibleForTesting in AOSP: build without the validation pass. */
        SoftApConfiguration buildWithoutCheck();

    private:
        WifiSsid mWifiSsid;
        MacAddress mBssid;
        std::string mPassphrase;
        bool mHiddenSsid = false;
        std::map<int, int> mChannels;
        int mMaxNumberOfClients = 0;
        int mSecurityType = SECURITY_TYPE_OPEN;
        bool mAutoShutdownEnabled = true;
        int64_t mShutdownTimeoutMillis = DEFAULT_TIMEOUT;
        bool mClientControlByUser = false;
        std::vector<MacAddress> mBlockedClientList;
        std::vector<MacAddress> mAllowedClientList;
        int mMacRandomizationSetting = RANDOMIZATION_NON_PERSISTENT;
        bool mBridgedModeOpportunisticShutdownEnabled = true;
        bool mIeee80211axEnabled = true;
        bool mIeee80211beEnabled = true;
        bool mIsUserConfiguration = true;
        int64_t mBridgedModeOpportunisticShutdownTimeoutMillis = DEFAULT_TIMEOUT;
        std::vector<ScanResult::InformationElement> mVendorElements;
        MacAddress mPersistentRandomizedMacAddress;
        std::set<int> mAllowedAcsChannels2g;
        std::set<int> mAllowedAcsChannels5g;
        std::set<int> mAllowedAcsChannels6g;
        int mMaxChannelBandwidth = SoftApInfo::CHANNEL_WIDTH_AUTO;
        bool mIsClientIsolationEnabled = false;
    };

private:
    /* Mirrors the (26-arg) private AOSP constructor; defaults live in
     * Builder like the original. */
    SoftApConfiguration(const WifiSsid& ssid, const MacAddress& bssid,
            const std::string& passphrase, bool hiddenSsid,
            const std::map<int, int>& channels, int securityType,
            int maxNumberOfClients, bool shutdownTimeoutEnabled,
            int64_t shutdownTimeoutMillis, bool clientControlByUser,
            const std::vector<MacAddress>& blockedList,
            const std::vector<MacAddress>& allowedList,
            int macRandomizationSetting,
            bool bridgedModeOpportunisticShutdownEnabled,
            bool ieee80211axEnabled, bool ieee80211beEnabled,
            bool isUserConfiguration,
            int64_t bridgedModeOpportunisticShutdownTimeoutMillis,
            const std::vector<ScanResult::InformationElement>& vendorElements,
            const MacAddress& persistentRandomizedMacAddress,
            const std::set<int>& allowedAcsChannels24g,
            const std::set<int>& allowedAcsChannels5g,
            const std::set<int>& allowedAcsChannels6g,
            int maxChannelBandwidth, bool isClientIsolationEnabled);

    WifiSsid mWifiSsid;                 /* empty = Java null */
    MacAddress mBssid;                  /* empty = Java null */
    std::string mPassphrase;            /* empty = Java null */
    bool mHiddenSsid = false;
    /* band -> channel (0 = auto); never empty — defaults to {2GHZ: 0}
     * exactly like the AOSP ctor's else-branch. */
    std::map<int, int> mChannels;
    int mMaxNumberOfClients = 0;
    int mSecurityType = SECURITY_TYPE_OPEN;
    bool mAutoShutdownEnabled = true;
    int64_t mShutdownTimeoutMillis = DEFAULT_TIMEOUT;
    bool mClientControlByUser = false;
    std::vector<MacAddress> mBlockedClientList;
    std::vector<MacAddress> mAllowedClientList;
    int mMacRandomizationSetting = RANDOMIZATION_NON_PERSISTENT;
    bool mBridgedModeOpportunisticShutdownEnabled = true;
    bool mIeee80211axEnabled = true;
    bool mIeee80211beEnabled = true;
    bool mIsUserConfiguration = true;
    int64_t mBridgedModeOpportunisticShutdownTimeoutMillis = DEFAULT_TIMEOUT;
    std::vector<ScanResult::InformationElement> mVendorElements;
    MacAddress mPersistentRandomizedMacAddress;
    std::set<int> mAllowedAcsChannels2g;
    std::set<int> mAllowedAcsChannels5g;
    std::set<int> mAllowedAcsChannels6g;
    int mMaxChannelBandwidth = SoftApInfo::CHANNEL_WIDTH_AUTO;
    bool mIsClientIsolationEnabled = false;
};

} // namespace cdroid

#endif /* __SOFT_AP_CONFIGURATION_H__ */
