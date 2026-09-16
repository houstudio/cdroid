#ifndef __HOSTAPD_CLIENT_H__
#define __HOSTAPD_CLIENT_H__

#include <string>

#include <core/callbackbase.h>   /* EventSet listener base (header-only) */

#include <wifi/softapconfiguration.h>
#include <wifi/supplicantclient.h>

namespace cdroid {

/**
 * Soft AP control plane: hostapd daemon + its control interface.
 *
 * The ctrl-interface transport is protocol-identical to wpa_supplicant's
 * (same wpa_ctrl library, same request/reply + ATTACHed monitor model), so
 * the proven SupplicantClient transport is reused by composition. This is
 * CDROID's counterpart of AOSP's IHostapd HAL client (HostapdHal.java):
 * addAccessPointInstance/removeAccessPointInstance map to spawning/stopping
 * the daemon with a generated hostapd.conf — the classic-daemon seam
 * cdnet targets, instead of the AIDL service.
 *
 * hostapd events ("AP-STA-CONNECTED <mac>", "AP-STA-DISCONNECTED <mac>")
 * carry a positional argument, not key=value: consumers read event.raw.
 */
class HostapdClient {
public:
    /* Same message shape as the supplicant side (name + raw + args). */
    using HostapdEvent = SupplicantEvent;

    /* Canonical event spellings come from the vendored wpa_ctrl.h macros
     * AP_STA_CONNECTED / AP_STA_DISCONNECTED (they carry a trailing space —
     * match them through SupplicantClient-style length-aware comparison,
     * see WifiManager::onHostapdEvent). */

    /* Same EventSet + std::function shape as SupplicantClient::EventCallback
     * (see there): fill the members with lambdas, hand a copy to
     * setEventCallback, unset slots are no-ops. */
    class EventCallback : public EventSet {
    public:
        std::function<void(const HostapdEvent&)> onHostapdEvent;
        std::function<void()> onHostapdDisconnected;
        std::function<void()> onHostapdReconnected;
    };

    explicit HostapdClient(const std::string& ctrlPath = defaultCtrlPath());
    ~HostapdClient();

    bool connect();
    bool isConnected() const { return mTransport.isConnected(); }
    void close();
    void setCtrlPath(const std::string& ctrlPath) { mTransport.setCtrlPath(ctrlPath); }

    /* Serialized request/reply: "PING" -> "PONG", "STATUS", "DISABLE",
     * "ENABLE", "LIST_CLIENTS"? — reply convention preserved verbatim. */
    bool request(const std::string& cmd, std::string& reply);
    std::string request(const std::string& cmd);

    void setEventCallback(const EventCallback& callback);
    /* Marshals event delivery off the monitor thread (see SupplicantClient). */
    void setDispatcher(std::function<void(std::function<void()>)> dispatcher);

    /* "/var/run/hostapd/<iface>"-style default (HOSTAPD_CTRL_PATH env
     * override mirrors WPA_CTRL_PATH — bench convenience only). */
    static const char* defaultCtrlPath();

    /* --- daemon + configuration (HostapdHal seam) -------------------------- */

    /* Per-interface working directory for the generated hostapd files:
     * /tmp/cdroid-softap/<iface>/{hostapd.conf,pid,log}. */
    static std::string configDirectory(const std::string& iface);

    /**
     * Renders a hostapd.conf for `config` on `iface` (SoftApConfiguration ->
     * hostapd vocabulary: band->hw_mode, SECURITY_TYPE_* -> wpa/key_mgmt
     * lines, max clients, hidden SSID, client lists...). OWE security types
     * and bridged (multi-band) configurations are rejected with error
     * SAP_START_FAILURE_UNSUPPORTED_CONFIGURATION semantics — the caller
     * decides how to surface that.
     * @return false with *error filled when the config is unrenderable.
     */
    static bool writeConfigFile(const std::string& path, const std::string& iface,
            const std::string& ctrlDirectory, const SoftApConfiguration& config,
            std::string* error);

    /* Launches "hostapd -B -i <iface> <conf> -P <pidFile> -f <logFile>".
     * -B backgrounds after interface setup, so success here only means the
     * process spawned — readiness is observed via the ctrl socket.
     * Returns the spawned pid, -1 on fork/exec failure. */
    static pid_t startDaemon(const std::string& iface, const std::string& confPath,
            const std::string& pidFile, const std::string& logFile);

    /* SIGTERM the daemon recorded in pidFile (SIGKILL after 3 s), remove the
     * pid file. true when no daemon was running or it exited. */
    static bool stopDaemon(const std::string& pidFile);

private:
    SupplicantClient mTransport;
    EventCallback mCallback;    /* guarded by mCallbackMutex */
    std::mutex mCallbackMutex; /* forwarding lambdas vs setEventCallback */
};

} // namespace cdroid

#endif /* __HOSTAPD_CLIENT_H__ */
