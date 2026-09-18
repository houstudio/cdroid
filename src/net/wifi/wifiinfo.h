#ifndef __WIFI_INFO_H__
#define __WIFI_INFO_H__

#include <string>

#include <networkinfo.h>
#include <wifi/supplicantstate.h>
#include <wifi/wifissid.h>

namespace cdroid {

/**
 * Port of android.net.wifi.WifiInfo (android-36), trimmed to the fields the
 * supplicant STATUS + SIGNAL_POLL exchange provides. The @hide setters are
 * kept public like the original — the wpa response parsing path fills them
 * in, exactly as the framework does before handing WifiInfo to apps.
 */
class WifiInfo {
public:
    static constexpr const char* DEFAULT_MAC_ADDRESS = "02:00:00:00:00:00";
    static constexpr int INVALID_RSSI = -127;
    static constexpr int UNKNOWN_FREQUENCY = -1;
    static constexpr int MIN_RSSI = -126;
    static constexpr int MAX_RSSI = 200;
    static constexpr int LINK_SPEED_UNKNOWN = -1;
    static constexpr const char* LINK_SPEED_UNITS = "Mbps";
    static constexpr const char* FREQUENCY_UNITS = "MHz";

    static constexpr int SECURITY_TYPE_UNKNOWN = -1;
    static constexpr int SECURITY_TYPE_OPEN = 0;
    static constexpr int SECURITY_TYPE_WEP = 1;
    static constexpr int SECURITY_TYPE_PSK = 2;
    static constexpr int SECURITY_TYPE_EAP = 3;
    static constexpr int SECURITY_TYPE_SAE = 4;
    static constexpr int SECURITY_TYPE_EAP_WPA3_ENTERPRISE_192_BIT = 5;
    static constexpr int SECURITY_TYPE_OWE = 6;
    static constexpr int SECURITY_TYPE_WAPI_PSK = 7;
    static constexpr int SECURITY_TYPE_WAPI_CERT = 8;
    static constexpr int SECURITY_TYPE_EAP_WPA3_ENTERPRISE = 9;
    static constexpr int SECURITY_TYPE_OSEN = 10;
    static constexpr int SECURITY_TYPE_PASSPOINT_R1_R2 = 11;
    static constexpr int SECURITY_TYPE_PASSPOINT_R3 = 12;
    static constexpr int SECURITY_TYPE_DPP = 13;

    /* Public score field shared with AOSP (Q/ns scoring). */
    int score = 0;

    WifiInfo();

    /* Map a supplicant state into a fine-grained network connectivity state
     * (AOSP location: WifiInfo#getDetailedStateOf; map body verbatim
     * android-36). Legal here because android.net.wifi sits on top of
     * android.net in the module layering. */
    static NetworkInfo::DetailedState::Type getDetailedStateOf(
            SupplicantState::State suppState);

    void setSSID(WifiSsid wifiSsid);          /* @hide */
    /* Empty WifiInfo object when unset (Java null). */
    WifiSsid getWifiSsid() const;                   /* @hide */
    /* Quoted SSID per WifiSsid#toString, or WifiManager.UNKNOWN_SSID. */
    std::string getSSID() const;
    void setBSSID(const std::string& BSSID);  /* @hide */
    std::string getBSSID() const;

    int getRssi() const;
    void setRssi(int rssi);                   /* @hide */
    int getLinkSpeed() const;
    void setLinkSpeed(int linkSpeed);         /* @hide */
    int getTxLinkSpeedMbps() const;
    void setTxLinkSpeedMbps(int txLinkSpeed); /* @hide */
    int getMaxSupportedTxLinkSpeedMbps() const;
    void setMaxSupportedTxLinkSpeedMbps(int maxSupportedTxLinkSpeed); /* @hide */
    int getRxLinkSpeedMbps() const;
    void setRxLinkSpeedMbps(int rxLinkSpeed); /* @hide */
    int getMaxSupportedRxLinkSpeedMbps() const;
    void setMaxSupportedRxLinkSpeedMbps(int maxSupportedRxLinkSpeed); /* @hide */

    int getFrequency() const;
    void setFrequency(int frequency);         /* @hide */
    bool is24GHz() const;
    bool is5GHz() const;
    bool is6GHz() const;

    void setMacAddress(const std::string& macAddress); /* @hide */
    std::string getMacAddress() const;
    bool hasRealMacAddress() const;

    int getNetworkId() const;
    void setNetworkId(int networkId);         /* @hide */
    SupplicantState::State getSupplicantState() const;
    void setSupplicantState(SupplicantState::State state); /* @hide */

    /* @hide: dotted-decimal "192.168.1.5"; empty clears. */
    void setInetAddress(const std::string& address);
    /* @deprecated IPv4 in host byte order (a<<24|b<<16|c<<8|d), 0 when unset. */
    int getIpAddress() const;

    bool getHiddenSSID() const;
    void setHiddenSSID(bool isHiddenSsid);    /* @hide */

    void setMeteredHint(bool meteredHint);    /* @hide */
    bool getMeteredHint() const;
    void setEphemeral(bool ephemeral);        /* @hide */
    bool isEphemeral() const;
    void setTrusted(bool trusted);            /* @hide */
    bool isTrusted() const;
    void setRestricted(bool restricted);      /* @hide */
    bool isRestricted() const;
    void setOemPaid(bool oemPaid);            /* @hide */
    bool isOemPaid() const;
    void setOemPrivate(bool oemPrivate);      /* @hide */
    bool isOemPrivate() const;
    void setCarrierMerged(bool carrierMerged); /* @hide */
    bool isCarrierMerged() const;
    void setOsuAp(bool osuAp);                /* @hide */
    bool isOsuAp() const;

    int getScore() const;
    void setScore(int score);
    bool isUsable() const;
    void setUsable(bool isUsable);

    std::string toString() const;

private:
    WifiSsid mWifiSsid;
    std::string mBSSID;
    int mRssi = INVALID_RSSI;
    int mLinkSpeed = LINK_SPEED_UNKNOWN;
    int mTxLinkSpeed = LINK_SPEED_UNKNOWN;
    int mMaxSupportedTxLinkSpeed = LINK_SPEED_UNKNOWN;
    int mRxLinkSpeed = LINK_SPEED_UNKNOWN;
    int mMaxSupportedRxLinkSpeed = LINK_SPEED_UNKNOWN;
    int mFrequency = UNKNOWN_FREQUENCY;
    bool mIsHiddenSsid = false;
    std::string mMacAddress = DEFAULT_MAC_ADDRESS;
    int mNetworkId = -1;
    SupplicantState::State mSupplicantState = SupplicantState::UNINITIALIZED;
    std::string mIpAddress;  /* dotted decimal; empty = not set */
    bool mMeteredHint = false;
    bool mEphemeral = false;
    bool mTrusted = false;
    bool mRestricted = false;
    bool mOemPaid = false;
    bool mOemPrivate = false;
    bool mCarrierMerged = false;
    bool mOsuAp = false;
    int mSecurityType = SECURITY_TYPE_UNKNOWN;
    bool mIsUsable = false;
};

} // namespace cdroid

#endif /* __WIFI_INFO_H__ */
