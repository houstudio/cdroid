/*
 * Transport to wpa_supplicant's control interface, on top of the vendor
 * wpa_ctrl library. See supplicantclient.h for the AOSP mapping notes.
 */
#include <wifi/supplicantclient.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <wpa_ctrl.h>

/* The module stays self-contained (no cdroid dependency), so logging is a
 * plain fprintf shim with the same severity names as cdlog. */
#define WIFI_LOGI(...) do { fprintf(stdout, "SupplicantClient: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#define WIFI_LOGE(...) do { fprintf(stderr, "SupplicantClient E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

static constexpr int CTRL_RECV_TIMEOUT_SEC = 10;   /* hung-daemon guard on request() */
static constexpr int MONITOR_POLL_MS        = 300; /* monitor loop wakeup granularity */
static constexpr int RECONNECT_BACKOFF_MS   = 1000;

namespace cdroid {

const char* SupplicantClient::defaultCtrlPath() {
    return "/var/run/wpa_supplicant/wlan0";
}

SupplicantClient::SupplicantClient(const std::string& ctrlPath)
    : mCtrlPath(ctrlPath),
      mCtrl(nullptr),
      mMonitor(nullptr),
      mRunning(false),
      mConnected(false),
      mCallback(nullptr) {
}

SupplicantClient::~SupplicantClient() {
    close();
}

bool SupplicantClient::connect() {
    std::lock_guard<std::mutex> lock(mCtrlMutex);
    if (mRunning.load()) return mConnected.load();
    /* Even when the daemon is not up yet, start the monitor thread: it owns
     * the reconnect backoff, so a supplicant started later is picked up. */
    const bool opened = openConnections();
    mRunning.store(true);
    mMonitorThread = std::thread(&SupplicantClient::monitorLoop, this);
    return opened;
}

void SupplicantClient::close() {
    if (mMonitorThread.joinable()) {
        mRunning.store(false);
        mMonitorThread.join();
    }
    std::lock_guard<std::mutex> lock(mCtrlMutex);
    closeConnections();
}

void SupplicantClient::setCtrlPath(const std::string& ctrlPath) {
    if (!mConnected.load())
        mCtrlPath = ctrlPath;
}

void SupplicantClient::setEventCallback(EventCallback* callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mCallback = callback;
}

void SupplicantClient::setDispatcher(std::function<void(std::function<void()>)> dispatcher) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mDispatcher = std::move(dispatcher);
}

void SupplicantClient::dispatch(std::function<void()> runnable) {
    std::function<void(std::function<void()>)> dispatcher;
    {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        dispatcher = mDispatcher;
    }
    if (dispatcher) dispatcher(std::move(runnable));
    else runnable();
}

bool SupplicantClient::request(const std::string& cmd, std::string& reply) {
    std::lock_guard<std::mutex> lock(mCtrlMutex);
    if (!mConnected.load() && !openConnections())
        return false;
    char buf[4096];
    size_t len = sizeof(buf) - 1;
    const int rc = wpa_ctrl_request(mCtrl, cmd.c_str(), cmd.size(), buf, &len, nullptr);
    if (rc != 0) {
        WIFI_LOGE("request('%s') failed rc=%d errno=%d — connection lost", cmd.c_str(), rc, errno);
        mConnected.store(false);
        closeConnections();
        return false;
    }
    reply.assign(buf, len);
    return true;
}

std::string SupplicantClient::request(const std::string& cmd) {
    std::string reply;
    request(cmd, reply);
    return reply;
}

/* --- connection management (mCtrlMutex held or single-threaded) --------- */

static void setSocketTimeouts(struct wpa_ctrl* ctrl, int seconds) {
    if (!ctrl) return;
    const int fd = wpa_ctrl_get_fd(ctrl);
    if (fd < 0) return;
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

bool SupplicantClient::openConnections() {
    closeConnections();
    mCtrl = wpa_ctrl_open(mCtrlPath.c_str());
    if (!mCtrl) {
        WIFI_LOGE("cannot open ctrl connection to %s", mCtrlPath.c_str());
        return false;
    }
    setSocketTimeouts(mCtrl, CTRL_RECV_TIMEOUT_SEC);
    mMonitor = wpa_ctrl_open(mCtrlPath.c_str());
    if (!mMonitor) {
        WIFI_LOGE("cannot open monitor connection to %s", mCtrlPath.c_str());
        wpa_ctrl_close(mCtrl);
        mCtrl = nullptr;
        return false;
    }
    if (wpa_ctrl_attach(mMonitor) != 0) {
        WIFI_LOGE("wpa_ctrl_attach failed on %s", mCtrlPath.c_str());
        wpa_ctrl_close(mMonitor);
        wpa_ctrl_close(mCtrl);
        mMonitor = nullptr;
        mCtrl = nullptr;
        return false;
    }
    mConnected.store(true);
    return true;
}

void SupplicantClient::closeConnections() {
    /* mMonitor is detached before close, like wpa_cli does. */
    if (mMonitor) {
        wpa_ctrl_detach(mMonitor);
        wpa_ctrl_close(mMonitor);
        mMonitor = nullptr;
    }
    if (mCtrl) {
        wpa_ctrl_close(mCtrl);
        mCtrl = nullptr;
    }
    mConnected.store(false);
}

/* --- monitor thread ------------------------------------------------------ */

void SupplicantClient::monitorLoop() {
    bool reportedDisconnect = false;
    while (mRunning.load()) {
        if (!mMonitor || !mCtrl) {
            /* never connected / lost: report once, then retry with backoff */
            if (!reportedDisconnect) {
                reportedDisconnect = true;
                EventCallback* cb;
                {
                    std::lock_guard<std::mutex> lock(mCallbackMutex);
                    cb = mCallback;
                }
                if (cb) dispatch([cb] { cb->onSupplicantDisconnected(); });
            }
            usleep(RECONNECT_BACKOFF_MS * 1000);
            if (!mRunning.load()) break;
            std::lock_guard<std::mutex> lock(mCtrlMutex);
            if (openConnections()) {
                reportedDisconnect = false;
                EventCallback* cb;
                {
                    std::lock_guard<std::mutex> lock(mCallbackMutex);
                    cb = mCallback;
                }
                if (cb) dispatch([cb] { cb->onSupplicantReconnected(); });
            }
            continue;
        }
        const int fd = wpa_ctrl_get_fd(mMonitor);
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(fd, &readFds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = MONITOR_POLL_MS * 1000;
        const int rc = select(fd + 1, &readFds, nullptr, nullptr, &tv);
        if (!mRunning.load()) break;
        if (rc == 0) continue;              /* poll timeout: re-check mRunning */
        if (rc < 0) {
            if (errno == EINTR) continue;
            WIFI_LOGE("monitor select failed errno=%d", errno);
            break;
        }
        char buf[4096];
        size_t len = sizeof(buf) - 1;
        if (wpa_ctrl_recv(mMonitor, buf, &len) != 0) {
            WIFI_LOGE("monitor recv failed — supplicant connection lost");
            std::lock_guard<std::mutex> lock(mCtrlMutex);
            closeConnections();
            continue;
        }
        buf[len] = '\0';
        /* One datagram normally carries one event; split defensively. */
        char* saveptr = nullptr;
        for (char* line = strtok_r(buf, "\n", &saveptr); line;
             line = strtok_r(nullptr, "\n", &saveptr)) {
            const SupplicantEvent event = parseEventMessage(line);
            EventCallback* cb;
            {
                std::lock_guard<std::mutex> lock(mCallbackMutex);
                cb = mCallback;
            }
            if (cb && !event.name.empty())
                dispatch([cb, event] { cb->onSupplicantEvent(event); });
        }
    }
}

/* --- event parsing ------------------------------------------------------- */

bool SupplicantEvent::getArg(const std::string& key, std::string& value) const {
    const auto it = args.find(key);
    if (it == args.end()) return false;
    value = it->second;
    return true;
}

SupplicantEvent SupplicantClient::parseEventMessage(const std::string& message) {
    SupplicantEvent event;
    std::string text = message;
    /* strip the priority prefix, e.g. "<3>" */
    if (!text.empty() && text[0] == '<') {
        const size_t close = text.find('>');
        if (close != std::string::npos) text = text.substr(close + 1);
    }
    /* trim leading/trailing whitespace */
    const size_t first = text.find_first_not_of(" \t\r");
    if (first == std::string::npos) return event;
    const size_t last = text.find_last_not_of(" \t\r");
    text = text.substr(first, last - first + 1);
    event.raw = text;

    /* name = leading token up to the first blank */
    const size_t blank = text.find(' ');
    event.name = (blank == std::string::npos) ? text : text.substr(0, blank);
    if (blank == std::string::npos) return event;

    /* Tokenize the remainder keeping quoted spans intact, then accept
     * key=value tokens. Values are stored RAW (quotes kept): the quoted vs
     * hex distinction is meaningful for SSID arguments, so consumers strip
     * quotes explicitly (WifiSsid::fromString handles both forms). */
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;
    for (const char c : text.substr(blank + 1)) {
        if (c == '"') inQuotes = !inQuotes;
        if ((c == ' ' || c == '\t') && !inQuotes) {
            if (!token.empty()) tokens.push_back(token);
            token.clear();
        } else {
            token.push_back(c);
        }
    }
    if (!token.empty()) tokens.push_back(token);
    for (const std::string& rawToken : tokens) {
        /* CTRL-EVENT-CONNECTED tails a bracket group: "[id=1 id_str=]" —
         * the '[' sticks to the first key and ']' to the last value. */
        std::string t = rawToken;
        if (!t.empty() && t.front() == '[') t.erase(0, 1);
        if (!t.empty() && t.back() == ']') t.pop_back();
        const size_t eq = t.find('=');
        if (eq == std::string::npos || eq == 0) continue;
        event.args[t.substr(0, eq)] = t.substr(eq + 1);
    }
    return event;
}

} // namespace cdroid
