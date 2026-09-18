/*
 * hostapd control plane: ctrl-interface client (over the shared wpa_ctrl
 * transport), hostapd.conf renderer and daemon lifecycle. See
 * hostapdclient.h for the AOSP mapping notes.
 */
#include <wifi/hostapdclient.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define HOSTAPD_LOGI(...) do { fprintf(stdout, "HostapdClient: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#define HOSTAPD_LOGE(...) do { fprintf(stderr, "HostapdClient E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

namespace cdroid {

namespace {
constexpr const char* SOFTAP_ROOT = "/tmp/cdroid-softap";
constexpr int DAEMON_KILL_TIMEOUT_MS = 3000;

/* hex dump of raw bytes, hostapd ssid=<hex> form */
std::string toHexString(const std::string& bytes) {
    static const char* digits = "0123456789abcdef";
    std::string out;
    out.reserve(bytes.size() * 2);
    for (const unsigned char c : bytes) {
        out.push_back(digits[c >> 4]);
        out.push_back(digits[c & 0xf]);
    }
    return out;
}

/* "ieee80211ax" is a hostapd config item only when the daemon was built
 * with CONFIG_IEEE80211AX (distro builds of the same version differ);
 * without it the whole conf file is rejected ("unknown configuration
 * item"). Ground truth over version parsing: feed the installed daemon a
 * one-item config on stdin and watch whether it complains. Result cached;
 * an absent daemon counts as unsupported (the line is omitted, the driver
 * then decides HE capability on its own). */
bool hostapdSupports11axConfig() {
    static int supported = -1;   /* cached probe */
    if (supported >= 0) return supported == 1;
    supported = 0;
    FILE* pipe = popen(
            "printf 'interface=lo\\nieee80211ax=1\\n' | hostapd /dev/stdin 2>&1",
            "r");
    if (!pipe) return false;
    bool sawUnknownItem = false;
    bool sawAnyOutput = false;
    char line[512];
    while (fgets(line, sizeof(line), pipe)) {
        sawAnyOutput = true;
        if (strstr(line, "unknown configuration item")) sawUnknownItem = true;
    }
    pclose(pipe);
    /* no output at all = no hostapd binary; a complaint = unsupported;
     * anything else (it moved past parsing, e.g. "Failed to set up
     * interface") = the item is understood */
    supported = (sawAnyOutput && !sawUnknownItem) ? 1 : 0;
    return supported == 1;
}
} // namespace

/* --- ctrl-interface client (composition over SupplicantClient) ------------- */

HostapdClient::HostapdClient(const std::string& ctrlPath)
    : mTransport(ctrlPath) {
    /* Forward the shared transport's supplicant-shaped events into this
     * client's callback slot (was an internal TransportCallback class —
     * the lambda form keeps it allocation-free on this side). hostapd
     * events carry no priority prefix and a positional tail; the shared
     * parser tolerates both shapes. */
    SupplicantClient::EventCallback transport;
    transport.onSupplicantEvent = [this](const SupplicantEvent& event) {
        EventCallback cb;
        {
            std::lock_guard<std::mutex> lock(mCallbackMutex);
            cb = mCallback;
        }
        cb.onHostapdEvent(event);
    };
    transport.onSupplicantDisconnected = [this]() {
        EventCallback cb;
        {
            std::lock_guard<std::mutex> lock(mCallbackMutex);
            cb = mCallback;
        }
        cb.onHostapdDisconnected();
    };
    transport.onSupplicantReconnected = [this]() {
        EventCallback cb;
        {
            std::lock_guard<std::mutex> lock(mCallbackMutex);
            cb = mCallback;
        }
        cb.onHostapdReconnected();
    };
    mTransport.setEventCallback(transport);
}

HostapdClient::~HostapdClient() {
    close();
    /* Retire the forwarding lambdas (they capture this) — close() already
     * stopped the monitor, this keeps the transport slot from outliving
     * the object it forwards to. */
    mTransport.setEventCallback(SupplicantClient::EventCallback());
}

bool HostapdClient::connect() {
    return mTransport.connect();
}

void HostapdClient::close() {
    mTransport.close();
}

bool HostapdClient::request(const std::string& cmd, std::string& reply) {
    return mTransport.request(cmd, reply);
}

std::string HostapdClient::request(const std::string& cmd) {
    return mTransport.request(cmd);
}

void HostapdClient::setEventCallback(const EventCallback& callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mCallback = callback;
}

void HostapdClient::setDispatcher(
        std::function<void(std::function<void()>)> dispatcher) {
    mTransport.setDispatcher(std::move(dispatcher));
}

const char* HostapdClient::defaultCtrlPath() {
    const char* env = getenv("HOSTAPD_CTRL_PATH");
    if (env && *env) return env;
    return "/var/run/hostapd/ap0";
}

std::string HostapdClient::configDirectory(const std::string& iface) {
    return std::string(SOFTAP_ROOT) + "/" + iface;
}

/* --- hostapd.conf renderer --------------------------------------------------- */

bool HostapdClient::writeConfigFile(const std::string& path,
        const std::string& iface, const std::string& ctrlDirectory,
        const SoftApConfiguration& config, std::string* error) {
    const auto fail = [error](const std::string& why) {
        if (error) *error = why;
        return false;
    };

    /* Bridged (multi-band) APs need a dedicated per-band interface model —
     * rejected until the nl80211 AP-interface work lands. */
    if (config.getChannels().size() != 1)
        return fail("bridged/multi-band SoftApConfiguration not supported");

    const int band = config.getBand();
    const int channel = config.getChannel();
    const char* hwMode;
    switch (band) {
        case SoftApConfiguration::BAND_2GHZ: hwMode = "g"; break;
        case SoftApConfiguration::BAND_5GHZ: hwMode = "a"; break;
        case SoftApConfiguration::BAND_6GHZ:
            /* hw_mode=a covers 5GHz only: rendering a 6GHz configuration
             * through it silently brought the AP up on 5GHz (channel 36 =
             * 5180 MHz) while the caller believed it started a 6GHz hotspot.
             * 6GHz needs the hostapd 6GHz/HE machinery — reject until that
             * backend exists (AOSP checks HAL capabilities the same way). */
            return fail("6GHz SoftApConfiguration not supported by the "
                    "classic hostapd backend (would silently start on 5GHz)");
        default:
            return fail("unsupported band for hostapd backend: "
                    + std::to_string(band));
    }

    std::string ssidLine;
    const std::string ssidBytes = config.getWifiSsid().getBytes();
    if (ssidBytes.empty()) {
        return fail("SoftApConfiguration without an SSID cannot be rendered "
                "(framework-determined SSID is TODO)");
    }
    /* This backend renders through a hostapd.conf FILE — unlike AOSP, which
     * hands the SSID/passphrase bytes to the HAL directly. A line break in
     * either value would inject arbitrary configuration lines (a hostile
     * "open" SSID like "Guest\nwpa_passphrase=evil12345" silently re-secures
     * the network), so anything copied into the file verbatim must be
     * line-safe. The hex SSID branch is safe by construction. */
    const auto lineSafe = [](const std::string& s) {
        return s.find('\n') == std::string::npos
                && s.find('\r') == std::string::npos;
    };
    const std::string utf8 = config.getWifiSsid().getUtf8Text();
    if (!utf8.empty()) {
        if (!lineSafe(utf8))
            return fail("ssid contains a line break (hostapd.conf injection)");
        ssidLine = "ssid=" + utf8;                             /* plain text */
    } else {
        ssidLine = "ssid=" + toHexString(ssidBytes);           /* raw hex */
    }

    const std::string passphrase = config.getPassphrase();
    if (!passphrase.empty() && !lineSafe(passphrase))
        return fail("passphrase contains a line break (hostapd.conf injection)");
    std::string securityLines;
    switch (config.getSecurityType()) {
        case SoftApConfiguration::SECURITY_TYPE_OPEN:
            break;
        case SoftApConfiguration::SECURITY_TYPE_WPA2_PSK:
            if (passphrase.empty())
                return fail("WPA2_PSK requires a passphrase");
            securityLines = "wpa=2\nwpa_key_mgmt=WPA-PSK\n"
                    "rsn_pairwise=CCMP\nwpa_passphrase=" + passphrase + "\n";
            break;
        case SoftApConfiguration::SECURITY_TYPE_WPA3_SAE:
            if (passphrase.empty())
                return fail("WPA3_SAE requires a passphrase");
            securityLines = "wpa=2\nwpa_key_mgmt=SAE\nrsn_pairwise=CCMP\n"
                    "ieee80211w=2\nsae_password=" + passphrase + "\n";
            break;
        case SoftApConfiguration::SECURITY_TYPE_WPA3_SAE_TRANSITION:
            if (passphrase.empty())
                return fail("WPA3_SAE_TRANSITION requires a passphrase");
            securityLines = "wpa=2\nwpa_key_mgmt=WPA-PSK SAE\n"
                    "rsn_pairwise=CCMP\nieee80211w=1\n"
                    "wpa_passphrase=" + passphrase + "\n";
            break;
        default:
            /* OWE / OWE_TRANSITION need the OWE transition-BSS machinery. */
            return fail("security type not supported by the hostapd backend: "
                    + std::to_string(config.getSecurityType()));
    }

    /* Client control: hostapd accept/deny MAC lists (side files beside the
     * conf, exactly what macaddr_acl wants). */
    std::string clientListLines;
    const std::string denyFile = path + ".deny";
    const std::string acceptFile = path + ".accept";
    const std::vector<MacAddress> blocked = config.getBlockedClientList();
    const std::vector<MacAddress> allowed = config.getAllowedClientList();
    if (config.isClientControlByUserEnabled() && !allowed.empty()) {
        std::ofstream accept(acceptFile);
        for (const MacAddress& mac : allowed) accept << mac.toString() << "\n";
        clientListLines = "macaddr_acl=1\naccept_mac_file=" + acceptFile + "\n";
        if (!blocked.empty()) {
            std::ofstream deny(denyFile);
            for (const MacAddress& mac : blocked) deny << mac.toString() << "\n";
            clientListLines += "deny_mac_file=" + denyFile + "\n";
        }
    } else if (!blocked.empty()) {
        std::ofstream deny(denyFile);
        for (const MacAddress& mac : blocked) deny << mac.toString() << "\n";
        clientListLines = "macaddr_acl=0\ndeny_mac_file=" + denyFile + "\n";
    }

    std::ofstream conf(path);
    if (!conf.is_open()) {
        return fail("cannot write " + path + ": " + strerror(errno));
    }
    conf << "interface=" << iface << "\n";
    conf << "driver=nl80211\n";
    conf << "ctrl_interface=" << ctrlDirectory << "\n";
    conf << ssidLine << "\n";
    if (config.getBssid().getBytes().size() == 6)
        conf << "bssid=" << config.getBssid().toString() << "\n";
    conf << "hw_mode=" << hwMode << "\n";
    /* channel 0 = framework/driver auto-select (AOSP ACS offload). The
     * classic-daemon backend has no ACS on e.g. mac80211_hwsim, so the
     * renderer substitutes the band default — the hostapd form of the same
     * "pick a valid channel" the AOSP framework does without ACS. */
    int renderedChannel = channel;
    if (renderedChannel == 0) {
        renderedChannel = (band == SoftApConfiguration::BAND_2GHZ) ? 6 : 36;
        conf << "# channel auto-selected (no ACS offload on this backend)\n";
    }
    conf << "channel=" << renderedChannel << "\n";
    if (config.isHiddenSsid()) conf << "ignore_broadcast_ssid=1\n";
    conf << securityLines;
    if (config.getMaxNumberOfClients() > 0)
        conf << "max_num_sta=" << config.getMaxNumberOfClients() << "\n";
    conf << "ieee80211n=1\n";
    if (config.isIeee80211axEnabled() && hostapdSupports11axConfig())
        conf << "ieee80211ax=1\n";
    if (config.isClientIsolationEnabled()) conf << "ap_isolate=1\n";
    conf << clientListLines;
    conf.flush();
    if (!conf.good()) return fail("short write to " + path);
    /* 0600: this file carries wpa_passphrase/sae_password (ofstream creates
     * it 0644 under umask 022 — world-readable under /tmp's 1777). */
    chmod(path.c_str(), 0600);
    return true;
}

/* --- daemon lifecycle ---------------------------------------------------------- */

pid_t HostapdClient::startDaemon(const std::string& iface,
        const std::string& confPath, const std::string& pidFile,
        const std::string& logFile) {
    const pid_t pid = fork();
    if (pid < 0) {
        HOSTAPD_LOGE("fork failed: %s", strerror(errno));
        return -1;
    }
    if (pid == 0) {
        /* child: hostapd -B daemonizes after interface setup; -f keeps the
         * daemon log observable on the bench. */
        execlp("hostapd", "hostapd", "-B", "-i", iface.c_str(), confPath.c_str(),
               "-P", pidFile.c_str(), "-f", logFile.c_str(),
               static_cast<char*>(nullptr));
        _exit(127);   /* exec failed */
    }
    /* The direct child either becomes the daemon (older hostapd) or exits
     * after forking the background worker (-B); reap it synchronously so no
     * zombie is left behind. */
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
        HOSTAPD_LOGE("execvp(hostapd) failed — hostapd not installed?");
        return -1;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        HOSTAPD_LOGE("hostapd exited with %d before daemonizing",
                     WEXITSTATUS(status));
        return -1;
    }
    return pid;
}

bool HostapdClient::stopDaemon(const std::string& pidFile) {
    std::ifstream pidStream(pidFile);
    if (!pidStream.is_open()) return true;   /* no daemon recorded */
    pid_t pid = 0;
    pidStream >> pid;
    if (pid <= 0 || kill(pid, 0) != 0) {
        unlink(pidFile.c_str());
        return true;   /* stale pid file */
    }
    if (kill(pid, SIGTERM) != 0 && errno != ESRCH) {
        HOSTAPD_LOGE("SIGTERM %d failed: %s", pid, strerror(errno));
        return false;
    }
    for (int waited = 0; waited < DAEMON_KILL_TIMEOUT_MS; waited += 50) {
        if (kill(pid, 0) != 0 && errno == ESRCH) {
            unlink(pidFile.c_str());
            return true;
        }
        usleep(50 * 1000);
    }
    HOSTAPD_LOGE("daemon %d did not exit, SIGKILL", pid);
    kill(pid, SIGKILL);
    usleep(100 * 1000);
    unlink(pidFile.c_str());
    return true;
}

} // namespace cdroid
