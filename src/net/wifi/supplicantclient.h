#ifndef __SUPPLICANT_CLIENT_H__
#define __SUPPLICANT_CLIENT_H__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

struct wpa_ctrl;

namespace cdroid {

/**
 * A single unsolicited (event) message received from wpa_supplicant over the
 * monitor connection. Mirrors the "CTRL-EVENT-*" notifications AOSP's
 * WifiMonitor dispatches (frameworks/opt/net/wifi WifiMonitor.java).
 * The priority prefix "<n>" is stripped; key=value arguments are parsed out
 * (quoted values keep their spaces, e.g. SSID="my network").
 */
struct SupplicantEvent {
    std::string name;                                   /* "CTRL-EVENT-SCAN-RESULTS" */
    std::string raw;                                    /* message without the priority prefix */
    std::unordered_map<std::string, std::string> args;  /* key=value pairs, when present */

    bool getArg(const std::string& key, std::string& value) const;
};

/**
 * Blocking request/reply + event-side transport to a running wpa_supplicant
 * daemon over its unix control socket (wpa_ctrl, ../wpa_ctrl.c — vendor file,
 * not modified). AOSP counterpart: the framework reaches wpa_supplicant via
 * SupplicantStaIfaceHal (HIDL); in CDROID's single-process world this class
 * is that transport.
 *
 * Two connections are kept, exactly like wpa_cli / AOSP WifiNative:
 *   - mCtrl:    serialized request/reply (every request() takes mCtrlMutex;
 *               the ctrl socket has SO_RCVTIMEO so a hung daemon cannot
 *               block a caller forever)
 *   - mMonitor: ATTACHed; a dedicated monitor thread (AOSP's WifiMonitor is
 *               also a dedicated thread) drains unsolicited events and hands
 *               them to the EventCallback.
 *
 * Connection loss (daemon restart, socket removed) is reported through the
 * callback and retried in the background; request() additionally attempts one
 * synchronous reopen so a transient daemon bounce does not fail a call.
 */
class SupplicantClient {
public:
    class EventCallback {
    public:
        virtual ~EventCallback() = default;
        /* CTRL-EVENT-* / WPA: / "Trying to ..." unsolicited message */
        virtual void onSupplicantEvent(const SupplicantEvent& event) = 0;
        /* ctrl+monitor connection lost (daemon exit / socket removed) */
        virtual void onSupplicantDisconnected() {}
        /* connection re-established after a loss */
        virtual void onSupplicantReconnected() {}
    };

    explicit SupplicantClient(const std::string& ctrlPath = defaultCtrlPath());
    ~SupplicantClient();

    /* Open both connections, ATTACH the monitor, start the monitor thread. */
    bool connect();
    bool isConnected() const { return mConnected; }
    void close();
    /* Rebind the control socket path — honored only while not connected
     * (a running client keeps its socket until close()). */
    void setCtrlPath(const std::string& ctrlPath);

    /*
     * Serialized request/reply ("PING", "STATUS", "SCAN_RESULTS", ...).
     * Returns false (and clears reply) on transport error/timeout. The
     * wpa_supplicant reply convention is preserved verbatim in the string:
     * "OK\n", "FAIL\n", or a multi-line payload.
     */
    bool request(const std::string& cmd, std::string& reply);
    /* Convenience overload: empty string on transport failure. */
    std::string request(const std::string& cmd);

    /* Not owned. May be installed before connect() or swapped at runtime. */
    void setEventCallback(EventCallback* callback);
    /*
     * Marshals event delivery off the monitor thread (default: direct call
     * from the monitor thread). The cdroid integration installs a
     * post-to-main-looper dispatcher so app callbacks land on the UI thread.
     */
    void setDispatcher(std::function<void(std::function<void()>)> dispatcher);

    /* "/var/run/wpa_supplicant/wlan0" (overridable per interface). */
    static const char* defaultCtrlPath();

    /* Split one wpa_supplicant text message into name + key=value args. */
    static SupplicantEvent parseEventMessage(const std::string& message);

private:
    void monitorLoop();
    bool openConnections();
    void closeConnections();
    /* Releases the monitor thread's claim on mMonitor (see mMonitorInUse). */
    void releaseMonitorClaim();
    void dispatch(std::function<void()> runnable);

    std::string mCtrlPath;
    struct wpa_ctrl* mCtrl;
    struct wpa_ctrl* mMonitor;
    std::mutex mCtrlMutex;      /* serializes every wpa_ctrl_request */
    std::mutex mCallbackMutex;  /* callback/dispatcher vs monitor thread */
    /* The monitor thread uses mMonitor (select/recv) outside mCtrlMutex;
     * closeConnections() must not free the handle under it. The monitor
     * thread sets mMonitorInUse while inside that window (claim taken under
     * mCtrlMutex, released without it), and closeConnections() waits for the
     * claim before closing — bounded by the 300 ms select timeout + one
     * recv. Lock order: mCtrlMutex -> mMonitorUseMutex, never reversed. */
    std::mutex mMonitorUseMutex;
    std::condition_variable mMonitorIdleCv;
    bool mMonitorInUse = false;
    std::thread mMonitorThread;
    std::atomic<bool> mRunning;
    std::atomic<bool> mConnected;
    EventCallback* mCallback;
    std::function<void(std::function<void()>)> mDispatcher;
};

} // namespace cdroid

#endif /* __SUPPLICANT_CLIENT_H__ */
