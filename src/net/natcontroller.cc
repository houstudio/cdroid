/*
 * Tethering NAT over classic iptables — netd NatController port. See
 * natcontroller.h for the rule set and the AOSP mapping notes.
 */
#include <natcontroller.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/wait.h>
#include <unistd.h>

#define NAT_LOGI(...) do { fprintf(stdout, "NatController: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#define NAT_LOGE(...) do { fprintf(stderr, "NatController E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

namespace cdroid {

namespace {
constexpr const char* IP_FORWARD_PROC = "/proc/sys/net/ipv4/ip_forward";

/* split a rule string into argv (strtok would clobber its input) */
std::vector<std::string> splitArgs(const std::string& rule) {
    std::vector<std::string> args;
    std::string token;
    for (const char c : rule) {
        if (c == ' ' || c == '\t') {
            if (!token.empty()) args.push_back(token);
            token.clear();
        } else {
            token.push_back(c);
        }
    }
    if (!token.empty()) args.push_back(token);
    return args;
}
} // namespace

std::string NatController::masqueradeRule(const std::string& externalIface) {
    return "-t nat -A POSTROUTING -o " + externalIface + " -j MASQUERADE";
}

std::vector<std::string> NatController::forwardingRules(
        const std::string& internalIface, const std::string& externalIface) {
    return {
        "-A FORWARD -i " + externalIface + " -o " + internalIface +
                " -m state --state RELATED,ESTABLISHED -j ACCEPT",
        "-A FORWARD -i " + internalIface + " -o " + externalIface + " -j ACCEPT",
    };
}

std::string NatController::defaultRouteInterface() {
    /* /proc/net/route: Iface Destination Gateway Flags RefCnt Use Metric ... */
    std::ifstream in("/proc/net/route");
    if (!in.is_open()) return std::string();
    std::string bestIface;
    unsigned long bestMetric = ~0UL;
    std::string line;
    std::getline(in, line);   /* header */
    while (std::getline(in, line)) {
        char iface[64];
        char dest[16];
        unsigned long flags = 0, metric = 0;
        if (sscanf(line.c_str(), "%63s %15s %*s %lu %*s %*s %lu",
                   iface, dest, &flags, &metric) != 4)
            continue;
        const bool isDefault = strcmp(dest, "00000000") == 0;   /* 0.0.0.0 */
        const bool isUp = (flags & 0x2) != 0;                   /* RTF_GATEWAY */
        if (isDefault && isUp && metric < bestMetric) {
            bestMetric = metric;
            bestIface = iface;
        }
    }
    return bestIface;
}

bool NatController::setIpForward(bool enable) {
    std::ofstream out(IP_FORWARD_PROC);
    if (!out.is_open()) {
        NAT_LOGE("cannot write %s: %s", IP_FORWARD_PROC, strerror(errno));
        return false;
    }
    out << (enable ? '1' : '0');
    return out.good();
}

bool NatController::runIptables(const std::vector<std::string>& args) {
    const pid_t pid = fork();
    if (pid < 0) {
        NAT_LOGE("fork failed: %s", strerror(errno));
        return false;
    }
    if (pid == 0) {
        /* args are pre-split; exec the iptables binary by absolute path so a
         * hostile PATH cannot substitute rules */
        std::vector<const char*> argv;
        argv.push_back("/usr/sbin/iptables");
        for (const std::string& arg : args) argv.push_back(arg.c_str());
        argv.push_back(nullptr);
        execv("/usr/sbin/iptables", const_cast<char* const*>(argv.data()));
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
        NAT_LOGE("iptables %s ... failed (%d)", args.empty() ? "" : args[0].c_str(),
                 WIFEXITED(status) ? WEXITSTATUS(status) : -1);
        return false;
    }
    return true;
}

bool NatController::enableNat(const std::string& internalIface,
                              const std::string& externalIface) {
    if (internalIface.empty() || externalIface.empty()) {
        NAT_LOGE("enableNat needs both interfaces (int='%s' ext='%s')",
                 internalIface.c_str(), externalIface.c_str());
        return false;
    }
    /* netd applies the whole rule set or nothing (its error paths unwind).
     * Match that: a mid-sequence failure (missing nft module, busy table)
     * rolls back everything already installed. The caller's failure path
     * tears the Soft AP down and nothing ever comes back for these rules —
     * half a NAT plus ip_forward=1 would stay in the kernel past the
     * process. The -D passes on rules that never landed are idempotent. */
    if (!setIpForward(true)) return false;
    bool ok = runIptables(splitArgs(masqueradeRule(externalIface)));
    if (ok) {
        for (const std::string& rule : forwardingRules(internalIface, externalIface)) {
            if (!runIptables(splitArgs(rule))) { ok = false; break; }
        }
    }
    if (!ok) {
        NAT_LOGE("enableNat incomplete (%s -> %s), rolling back",
                 internalIface.c_str(), externalIface.c_str());
        disableNat(internalIface, externalIface);
        return false;
    }
    NAT_LOGI("NAT up: %s -> %s", internalIface.c_str(), externalIface.c_str());
    return true;
}

bool NatController::disableNat(const std::string& internalIface,
                               const std::string& externalIface) {
    if (internalIface.empty() || externalIface.empty()) return false;
    bool ok = true;
    std::string masq = masqueradeRule(externalIface);
    masq.replace(masq.find("-A "), 3, "-D ");
    ok = runIptables(splitArgs(masq)) && ok;
    for (std::string rule : forwardingRules(internalIface, externalIface)) {
        rule.replace(rule.find("-A "), 3, "-D ");
        ok = runIptables(splitArgs(rule)) && ok;
    }
    /* single-NAT module: forwarding goes off with the last tether (netd's
     * refcount collapses to zero). */
    ok = setIpForward(false) && ok;
    NAT_LOGI("NAT down: %s -> %s", internalIface.c_str(), externalIface.c_str());
    return ok;
}

} // namespace cdroid
