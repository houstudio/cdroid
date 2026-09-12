/* Port of android.net.wifi.SupplicantState (android-36). */
#include <wifi/supplicantstate.h>

#include <cstdlib>
#include <stdexcept>

namespace cdroid {

static const char* const kStateNames[] = {
    "DISCONNECTED", "INTERFACE_DISABLED", "INACTIVE", "SCANNING",
    "AUTHENTICATING", "ASSOCIATING", "ASSOCIATED", "FOUR_WAY_HANDSHAKE",
    "GROUP_HANDSHAKE", "COMPLETED", "DORMANT", "UNINITIALIZED", "INVALID",
};

static const int kStateCount = static_cast<int>(sizeof(kStateNames) / sizeof(kStateNames[0]));

bool SupplicantState::isValidState(State state) {
    return state != UNINITIALIZED && state != INVALID;
}

bool SupplicantState::isHandshakeState(State state) {
    switch (state) {
    case AUTHENTICATING:
    case ASSOCIATING:
    case ASSOCIATED:
    case FOUR_WAY_HANDSHAKE:
    case GROUP_HANDSHAKE:
        return true;
    case COMPLETED:
    case DISCONNECTED:
    case INTERFACE_DISABLED:
    case INACTIVE:
    case SCANNING:
    case DORMANT:
    case UNINITIALIZED:
    case INVALID:
        return false;
    }
    throw std::invalid_argument("Unknown supplicant state: " + std::to_string((int) state));
}

bool SupplicantState::isConnecting(State state) {
    switch (state) {
    case AUTHENTICATING:
    case ASSOCIATING:
    case ASSOCIATED:
    case FOUR_WAY_HANDSHAKE:
    case GROUP_HANDSHAKE:
    case COMPLETED:
        return true;
    case DISCONNECTED:
    case INTERFACE_DISABLED:
    case INACTIVE:
    case SCANNING:
    case DORMANT:
    case UNINITIALIZED:
    case INVALID:
        return false;
    }
    throw std::invalid_argument("Unknown supplicant state: " + std::to_string((int) state));
}

bool SupplicantState::isDriverActive(State state) {
    switch (state) {
    case DISCONNECTED:
    case DORMANT:
    case INACTIVE:
    case AUTHENTICATING:
    case ASSOCIATING:
    case ASSOCIATED:
    case SCANNING:
    case FOUR_WAY_HANDSHAKE:
    case GROUP_HANDSHAKE:
    case COMPLETED:
        return true;
    case INTERFACE_DISABLED:
    case UNINITIALIZED:
    case INVALID:
        return false;
    }
    throw std::invalid_argument("Unknown supplicant state: " + std::to_string((int) state));
}

SupplicantState::State SupplicantState::fromString(const std::string& name) {
    /* CTRL-EVENT-STATE-CHANGE carries the numeric wpa_states value
     * (ctrl_iface.c: "id=%d state=%d ..."), and that enum (defs.h) shares
     * its first ten values with this one, in the same order. */
    if (!name.empty() && name.find_first_not_of("0123456789") == std::string::npos) {
        const long value = strtol(name.c_str(), nullptr, 10);
        if (value < 0 || value > COMPLETED)
            throw std::invalid_argument("Unknown supplicant state: " + name);
        return static_cast<State>(value);
    }
    /* wpa_supplicant_state_txt() spells the 4-way state "4WAY_HANDSHAKE"
     * (STATUS wpa_state=); STATUS of an older daemon shape aside, this is
     * the only wire spelling that differs from the enum names. */
    if (name == "4WAY_HANDSHAKE") return FOUR_WAY_HANDSHAKE;
    for (int i = 0; i < kStateCount; i++) {
        if (name == kStateNames[i]) return static_cast<State>(i);
    }
    throw std::invalid_argument("Unknown supplicant state: " + name);
}

std::string SupplicantState::toString(State state) {
    const int index = static_cast<int>(state);
    if (index < 0 || index >= kStateCount)
        return "INVALID";
    return kStateNames[index];
}

} // namespace cdroid
