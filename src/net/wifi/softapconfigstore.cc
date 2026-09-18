/* Soft AP settings persistence (WifiApConfigStore storage half). */
#include <wifi/softapconfigstore.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

namespace cdroid {

namespace {
const char* kStoreFile = "/tmp/cdroid-softap/softap.conf";

/* Comma-separated MAC list for the store file ("aa:..,bb:.."); empty list
 * writes no value. fromString's Java-null contract is an all-zero default
 * here, so malformed entries are skipped on the way back in. */
std::string joinMacs(const std::vector<MacAddress>& macs) {
    std::string out;
    for (const MacAddress& mac : macs) {
        if (!out.empty()) out += ',';
        out += mac.toString();
    }
    return out;
}

std::vector<MacAddress> parseMacList(const std::string& value) {
    std::vector<MacAddress> macs;
    size_t pos = 0;
    while (pos <= value.size()) {
        const size_t comma = value.find(',', pos);
        const std::string token = value.substr(pos,
                comma == std::string::npos ? std::string::npos : comma - pos);
        if (!token.empty()) {
            const MacAddress mac = MacAddress::fromString(token);
            if (!mac.getBytes().empty()) macs.push_back(mac);
        }
        if (comma == std::string::npos) break;
        pos = comma + 1;
    }
    return macs;
}
} // namespace

const char* SoftApConfigStore::storePath() {
    return kStoreFile;
}

bool SoftApConfigStore::load(Record* record, const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string ssid;                 /* WifiSsid::toString form */
    std::string passphrase;
    int securityType = SoftApConfiguration::SECURITY_TYPE_OPEN;
    int band = SoftApConfiguration::BAND_2GHZ;
    int channel = 0;
    bool hidden = false;
    int maxClients = 0;
    bool clientIsolation = false;
    bool autoShutdown = true;
    int64_t shutdownTimeout = SoftApConfiguration::DEFAULT_TIMEOUT;
    /* defaults mirror the Builder's so a store from an older build (which
     * did not persist these) loads with its previous behavior */
    bool clientControlByUser = false;
    std::vector<MacAddress> blockedClients;
    std::vector<MacAddress> allowedClients;
    MacAddress bssid;                 /* empty = not configured */
    int macRandomization = SoftApConfiguration::RANDOMIZATION_NON_PERSISTENT;
    bool ieee80211ax = true;
    bool ieee80211be = true;
    bool sawSsid = false;

    std::string line;
    while (std::getline(in, line)) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos || eq == 0) continue;
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        if (key == "iface") record->iface = value;
        else if (key == "ssid") { ssid = value; sawSsid = true; }
        else if (key == "passphrase") passphrase = value;
        else if (key == "securityType") securityType = atoi(value.c_str());
        else if (key == "band") band = atoi(value.c_str());
        else if (key == "channel") channel = atoi(value.c_str());
        else if (key == "hidden") hidden = value == "true";
        else if (key == "maxClients") maxClients = atoi(value.c_str());
        else if (key == "clientIsolation") clientIsolation = value == "true";
        else if (key == "autoShutdown") autoShutdown = value == "true";
        else if (key == "shutdownTimeoutMillis") shutdownTimeout = atoll(value.c_str());
        else if (key == "clientControlByUser") clientControlByUser = value == "true";
        else if (key == "blockedClients") blockedClients = parseMacList(value);
        else if (key == "allowedClients") allowedClients = parseMacList(value);
        else if (key == "bssid") {
            const MacAddress mac = MacAddress::fromString(value);
            if (!mac.getBytes().empty()) bssid = mac;
        }
        else if (key == "macRandomization") macRandomization = atoi(value.c_str());
        else if (key == "ieee80211ax") ieee80211ax = value == "true";
        else if (key == "ieee80211be") ieee80211be = value == "true";
    }

    if (!sawSsid) return false;   /* partial/corrupt store: treat as absent */
    SoftApConfiguration::Builder builder;
    /* The store file lives in a world-writable place (/tmp is 1777 and the
     * directory mkdir below does not vet ownership), so its contents are
     * untrusted input. ANY Builder setter can throw std::invalid_argument
     * (a passphrase with securityType absent->OPEN, an out-of-range
     * channel/band pair...); an escaping exception took down the whole
     * networking process — a pre-planted file was a local DoS. Guard the
     * whole build: a rejected file is "no stored config", exactly the
     * corrupt-store path above. */
    try {
        /* stored form is WifiSsid::toString output (quoted text / hex) */
        builder.setWifiSsid(WifiSsid::fromString(ssid));
    } catch (const std::invalid_argument&) {
        return false;
    }
    try {
        if (!passphrase.empty())
            builder.setPassphrase(passphrase, securityType);
        else
            builder.setPassphrase(std::string(),
                    securityType == SoftApConfiguration::SECURITY_TYPE_OPEN
                            ? securityType
                            : SoftApConfiguration::SECURITY_TYPE_OPEN);
        if (channel > 0)
            builder.setChannel(channel, band);
        else
            builder.setBand(band);
        builder.setHiddenSsid(hidden);
        builder.setMaxNumberOfClients(maxClients);
        builder.setClientIsolationEnabled(clientIsolation);
        builder.setAutoShutdownEnabled(autoShutdown);
        builder.setShutdownTimeoutMillis(shutdownTimeout);
        /* the full persistable set (AOSP's WifiApConfigStore parcels all of
         * these): the client-control lists, BSSID, MAC randomization and
         * 11ax/be flags used to be silently dropped, so a blocked client
         * rejoined after a process restart with the list emptied. */
        builder.setClientControlByUserEnabled(clientControlByUser);
        if (!blockedClients.empty()) builder.setBlockedClientList(blockedClients);
        if (!allowedClients.empty()) builder.setAllowedClientList(allowedClients);
        if (!bssid.getBytes().empty()) builder.setBssid(bssid);
        builder.setMacRandomizationSetting(macRandomization);
        builder.setIeee80211axEnabled(ieee80211ax);
        builder.setIeee80211beEnabled(ieee80211be);
        record->config = builder.build();
    } catch (const std::invalid_argument& e) {
        /* Untrusted file contents rejected by a setter or build() (bad
         * passphrase/securityType pair, out-of-range channel/band): treat
         * as no stored config — same as the corrupt-store path above. */
        fprintf(stderr, "SoftApConfigStore: rejected stored config: %s\n", e.what());
        return false;
    }
    record->hasConfig = true;
    return true;
}

bool SoftApConfigStore::save(const Record& record, const std::string& path) {
    /* the parent of the target path must exist (the runtime one is created
     * by WifiManager; test paths live beside it). 0700 + chmod: the store
     * carries the hotspot passphrase in cleartext and ofstream's default
     * 0644 left it world-readable under the 1777 /tmp. */
    const size_t slash = path.find_last_of('/');
    if (slash != std::string::npos && slash > 0) {
        std::string dir = path.substr(0, slash);
        mkdir(dir.c_str(), 0700);
        chmod(dir.c_str(), 0700);
    }
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) return false;
    const SoftApConfiguration& config = record.config;
    out << "iface=" << record.iface << "\n";
    out << "ssid=" << config.getWifiSsid().toString() << "\n";
    out << "passphrase=" << config.getPassphrase() << "\n";
    out << "securityType=" << config.getSecurityType() << "\n";
    out << "band=" << config.getBand() << "\n";
    out << "channel=" << config.getChannel() << "\n";
    out << "hidden=" << (config.isHiddenSsid() ? "true" : "false") << "\n";
    out << "maxClients=" << config.getMaxNumberOfClients() << "\n";
    out << "clientIsolation=" << (config.isClientIsolationEnabled() ? "true" : "false") << "\n";
    out << "autoShutdown=" << (config.isAutoShutdownEnabled() ? "true" : "false") << "\n";
    out << "shutdownTimeoutMillis=" << config.getShutdownTimeoutMillis() << "\n";
    /* the client-control lists / BSSID / randomization / 11ax flags — the
     * rest of the persistable set (see load()). */
    out << "clientControlByUser="
            << (config.isClientControlByUserEnabled() ? "true" : "false") << "\n";
    out << "blockedClients=" << joinMacs(config.getBlockedClientList()) << "\n";
    out << "allowedClients=" << joinMacs(config.getAllowedClientList()) << "\n";
    if (!config.getBssid().getBytes().empty())
        out << "bssid=" << config.getBssid().toString() << "\n";
    out << "macRandomization=" << config.getMacRandomizationSetting() << "\n";
    out << "ieee80211ax=" << (config.isIeee80211axEnabled() ? "true" : "false") << "\n";
    out << "ieee80211be=" << (config.isIeee80211beEnabled() ? "true" : "false") << "\n";
    out.flush();
    if (!out.good()) return false;
    /* 0600: passphrase-bearing file (ofstream creates 0644 under umask 022). */
    chmod(path.c_str(), 0600);
    return true;
}

} // namespace cdroid
