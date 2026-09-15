#ifndef __SOFT_AP_CONFIG_STORE_H__
#define __SOFT_AP_CONFIG_STORE_H__

#include <string>

#include <wifi/softapconfiguration.h>

namespace cdroid {

/**
 * Persistence for the Soft AP settings — the storage half of AOSP's
 * WifiApConfigStore (which keeps the default + user configuration in the
 * wifi store xml; this port uses a flat key=value file until the xml
 * settings store exists).
 *
 * Store file: /tmp/cdroid-softap/softap.conf, lines "key=value":
 *   iface, ssid (WifiSsid::toString quoted/hex form), passphrase,
 *   securityType, band, channel, hidden, maxClients, clientIsolation,
 *   autoShutdown, shutdownTimeoutMillis
 *
 * The AP interface name lives here too: AOSP resolves it from the wifi HAL
 * persistently; cross-process CLI tools (wpatest apiface/apstart) need the
 * same memory, and the file is that memory.
 */
class SoftApConfigStore {
public:
    /* Record = stored configuration + the interface it runs on (empty
     * string = follow the STA interface). load() returns false when no
     * store exists (first boot). */
    struct Record {
        bool hasConfig = false;
        std::string iface;
        SoftApConfiguration config;
    };

    /* The path parameter is the @VisibleForTesting seam (the wifitests
     * suite cannot write into a root-owned runtime directory). */
    static bool load(Record* record, const std::string& path = storePath());
    static bool save(const Record& record, const std::string& path = storePath());

    static const char* storePath();
};

} // namespace cdroid

#endif /* __SOFT_AP_CONFIG_STORE_H__ */
