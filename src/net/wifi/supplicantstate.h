#ifndef __SUPPLICANT_STATE_H__
#define __SUPPLICANT_STATE_H__

#include <string>

namespace cdroid {

/**
 * Port of android.net.wifi.SupplicantState (android-36). AOSP models it as a
 * Java enum whose constant order must match the state constants in
 * wpa_supplicant's defs.h; the C++ form follows the cdroid Gravity pattern
 * (class + nested enum + statics) so call sites read
 * SupplicantState::COMPLETED / SupplicantState::isValidState(s) like Java.
 */
class SupplicantState {
public:
    enum State {
        DISCONNECTED,
        INTERFACE_DISABLED,
        INACTIVE,
        SCANNING,
        AUTHENTICATING,
        ASSOCIATING,
        ASSOCIATED,
        FOUR_WAY_HANDSHAKE,
        GROUP_HANDSHAKE,
        COMPLETED,
        DORMANT,
        UNINITIALIZED,
        INVALID,
    };

    /* True when the state is valid (neither UNINITIALIZED nor INVALID). */
    static bool isValidState(State state);
    /* @hide: supplicant associating/authenticating counts as handshake. */
    static bool isHandshakeState(State state);
    /* @hide */
    static bool isConnecting(State state);
    /* @hide */
    static bool isDriverActive(State state);
    /* Java enum valueOf(String); throws std::invalid_argument when unknown. */
    static State fromString(const std::string& name);
    /* Java enum name()/toString(). */
    static std::string toString(State state);
};

} // namespace cdroid

#endif /* __SUPPLICANT_STATE_H__ */
