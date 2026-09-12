#ifndef __NETWORK_INFO_H__
#define __NETWORK_INFO_H__

#include <string>

namespace cdroid {

/**
 * Port of android.net.NetworkInfo (android-36): coarse connectivity state of
 * a network of a given type. The @hide setters are public like the original
 * — the wifi/ethernet aggregation path fills them in exactly as the
 * framework's ConnectivityService does.
 *
 * Java's State/DetailedState enums are scoped per-type; the C++ form nests
 * an unscoped enum one level deeper (NetworkInfo::DetailedState::CONNECTED)
 * so both sets of names coexist, matching the SupplicantState port style.
 */
class NetworkInfo {
public:
    /* Coarse network state. */
    struct State {
        enum Type { CONNECTING, CONNECTED, SUSPENDED, DISCONNECTING, DISCONNECTED, UNKNOWN };
    };
    /* Fine-grained network state. */
    struct DetailedState {
        enum Type {
            IDLE, SCANNING, CONNECTING, AUTHENTICATING, OBTAINING_IPADDR, CONNECTED,
            SUSPENDED, DISCONNECTING, DISCONNECTED, FAILED, BLOCKED, VERIFYING_POOR_LINK,
        };
    };

    explicit NetworkInfo(int type);

    int getType() const;
    void setType(int type);
    void setSubtype(int subtype, const std::string& subtypeName);
    int getSubtype() const;
    std::string getSubtypeName() const;
    std::string getTypeName() const;

    /* DetailedState -> State per the AOSP stateMap. */
    static State::Type stateFromDetailedState(DetailedState::Type detailedState);
    void setDetailedState(DetailedState::Type detailedState, const std::string& reason,
                          const std::string& extraInfo);
    DetailedState::Type getDetailedState() const;
    State::Type getState() const;
    std::string getReason() const;
    void setExtraInfo(const std::string& extraInfo);
    std::string getExtraInfo() const;

    bool isConnected() const;
    bool isConnectedOrConnecting() const;
    bool isFailover() const;
    void setFailover(bool isFailover);
    bool isAvailable() const;
    void setIsAvailable(bool isAvailable);
    bool isRoaming() const;
    void setRoaming(bool isRoaming);

    std::string toString() const;

private:
    int mNetworkType;
    int mSubtype = 0;
    std::string mSubtypeName;
    State::Type mState = State::UNKNOWN;
    DetailedState::Type mDetailedState = DetailedState::IDLE;
    std::string mReason;
    std::string mExtraInfo;
    bool mIsFailover = false;
    bool mIsAvailable = false;
    bool mIsRoaming = false;
};

} // namespace cdroid

#endif /* __NETWORK_INFO_H__ */
