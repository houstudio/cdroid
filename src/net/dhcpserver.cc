/*
 * Soft AP DHCP server over dnsmasq. See dhcpserver.h for the AOSP mapping.
 */
#include <dhcpserver.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <ipapplicator.h>
#include <linkaddress.h>
#include <staticipconfiguration.h>

#define DHCP_LOGI(...) do { fprintf(stdout, "DhcpServer: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#define DHCP_LOGE(...) do { fprintf(stderr, "DhcpServer E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

namespace cdroid {

namespace {
constexpr int SPAWN_TIMEOUT_MS = 3000;
constexpr int KILL_TIMEOUT_MS = 3000;
}

DhcpServer* DhcpServer::create(const Config& config) {
    return new DhcpServer(config);
}

DhcpServer::DhcpServer(const Config& config) : mConfig(config) {}

DhcpServer::~DhcpServer() {
    stop();
}

std::string DhcpServer::pidFile() const {
    return mConfig.workDir + "/dnsmasq.pid";
}

std::string DhcpServer::logFile() const {
    return mConfig.workDir + "/dnsmasq.log";
}

bool DhcpServer::readPid(pid_t* pid) const {
    std::ifstream in(pidFile());
    if (!in.is_open()) return false;
    *pid = 0;
    in >> *pid;
    return *pid > 0 && kill(*pid, 0) == 0;
}

bool DhcpServer::start() {
    if (mStarted) return true;
    if (mConfig.iface.empty()) return false;

    /* Own the server address first: dnsmasq drops requests on an interface
     * with no address inside the range subnet ("no address range
     * available") — same lesson the bench script bakes in. */
    if (!bringInterfaceUp(mConfig.iface, true)) {
        DHCP_LOGE("cannot bring %s up: %s", mConfig.iface.c_str(), strerror(errno));
        return false;
    }
    StaticIpConfiguration ip;
    ip.setIpAddress(LinkAddress(mConfig.serverIp, mConfig.prefixLength));
    ip.addDnsServer(mConfig.serverIp);
    if (!applyIpConfiguration(mConfig.iface, ip)) {
        DHCP_LOGE("cannot apply %s/%d to %s", mConfig.serverIp.c_str(),
                  mConfig.prefixLength, mConfig.iface.c_str());
        return false;
    }

    if (!spawnDaemon()) {
        /* roll our address back off the interface on failure? AOSP IpServer
         * tears the whole interface config down; the next start re-applies
         * and stopSoftAp clears — leaving it is harmless on a hidden iface. */
        return false;
    }
    mStarted = true;
    return true;
}

bool DhcpServer::spawnDaemon() {
    mkdir(mConfig.workDir.c_str(), 0755);
    unlink(pidFile().c_str());
    const pid_t pid = fork();
    if (pid < 0) {
        DHCP_LOGE("fork failed: %s", strerror(errno));
        return false;
    }
    if (pid == 0) {
        /* every value option in the --opt=value form: dnsmasq's parser
         * accepts the space form for most options but rejects it for
         * --pid-file ("junk found in command line") */
        const std::string range = mConfig.rangeStart + "," + mConfig.rangeEnd
                + "," + mConfig.netmask + "," + std::to_string(mConfig.leaseTimeSec) + "s";
        const std::string ifaceOpt = "--interface=" + mConfig.iface;
        const std::string rangeOpt = "--dhcp-range=" + range;
        const std::string dnsOpt = "--dhcp-option=option:dns-server," + mConfig.serverIp;
        const std::string logOpt = "--log-facility=" + logFile();
        const std::string pidOpt = "--pid-file=" + pidFile();
        execlp("dnsmasq", "dnsmasq",
               "--bind-interfaces", "--except-interface=lo",
               ifaceOpt.c_str(),
               rangeOpt.c_str(),
               dnsOpt.c_str(),
               "--dhcp-authoritative",
               "--log-dhcp",
               logOpt.c_str(),
               pidOpt.c_str(),
               static_cast<char*>(nullptr));
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);   /* dnsmasq daemonizes after the bind check */
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        /* port busy (another dnsmasq owns :67 on this iface) or bad args —
         * the log file says which. */
        DHCP_LOGE("dnsmasq exited %d (see %s)", WEXITSTATUS(status),
                  logFile().c_str());
        return false;
    }
    for (int waited = 0; waited < SPAWN_TIMEOUT_MS; waited += 50) {
        pid_t daemonPid = 0;
        if (readPid(&daemonPid)) return true;
        usleep(50 * 1000);
    }
    DHCP_LOGE("dnsmasq pid file never appeared (see %s)", logFile().c_str());
    return false;
}

void DhcpServer::stop() {
    if (!mStarted) return;
    mStarted = false;
    pid_t pid = 0;
    if (!readPid(&pid)) {
        unlink(pidFile().c_str());
        return;
    }
    if (kill(pid, SIGTERM) != 0 && errno != ESRCH) {
        DHCP_LOGE("SIGTERM %d failed: %s", pid, strerror(errno));
        return;
    }
    for (int waited = 0; waited < KILL_TIMEOUT_MS; waited += 50) {
        if (kill(pid, 0) != 0 && errno == ESRCH) {
            unlink(pidFile().c_str());
            return;
        }
        usleep(50 * 1000);
    }
    DHCP_LOGE("daemon %d did not exit, SIGKILL", pid);
    kill(pid, SIGKILL);
    usleep(100 * 1000);
    unlink(pidFile().c_str());
}

bool DhcpServer::isRunning() const {
    pid_t pid = 0;
    return readPid(&pid);
}

} // namespace cdroid
