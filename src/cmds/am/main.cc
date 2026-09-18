/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the License holder; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *********************************************************************************/
// am — the AOSP /system/bin/am role for CDROID: `am start` resolves a package
// (and optional activity shorthand) through the pm registry and fork/execs the
// installed binary. There is no system_server ActivityManagerService to bind
// to; the launch IS the exec. The child detaches (setsid + /dev/null stdio)
// so the adb shell channel it came from can close without SIGPIPE-ing it.
#include <cmds/packagedb.h>

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

using cmds::PackageDB;

static void usage() {
    fprintf(stderr,
        "usage: am <subcommand>\n"
        "  am start [-n] <package>[/<activity>]   launch installed package\n"
        "        activity shorthand: '.Act' expands to '<package>.Act';\n"
        "        a full activity name passes through\n"
        "  am start --exec [-n] <package>         replace THIS process with the\n"
        "        app (no fork, no detach) — for init respawn supervisors\n"
        "  am force-stop <package>                SIGKILL the package's processes\n");
}

// ComponentName.unflattenFromString shorthand: "pkg/.Act" -> "pkg/pkg.Act"?
// AOSP expands ".Act" against the PACKAGE, not a path: ".Act" -> "pkg.Act".
static std::string expandComponent(const std::string& pkg, const std::string& act) {
    if (act.empty()) return std::string();
    if (act[0] == '.') return pkg + act;
    return act;
}

static int cmdStart(int argc, char** argv) {
    std::string comp;
    bool execSelf = false;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--exec")) execSelf = true;
        else if (!strcmp(argv[i], "-n") && i + 1 < argc) comp = argv[++i];
        else if (argv[i][0] != '-') comp = argv[i];   // bare "am start pkg/.Act"
    }
    const size_t slash = comp.find('/');
    std::string pkg = slash == std::string::npos ? comp : comp.substr(0, slash);
    std::string act = slash == std::string::npos ? std::string() : comp.substr(slash + 1);

    PackageDB db;
    db.load();
    const cmds::PackageEntry* e = db.find(pkg);
    if (!e) {
        fprintf(stderr, "Error: package %s is not installed\n", pkg.c_str());
        return 1;
    }
    if (act.empty()) act = e->launcher;   // default: the MAIN/LAUNCHER activity
    if (act.empty()) {
        fprintf(stderr, "Error: package %s declares no launcher activity\n", pkg.c_str());
        return 1;
    }
    const std::string activity = expandComponent(pkg, act);

    // ActivityManagerService's startProcess: the exec replaces the binder
    // handoff. CDROID's App parses the manifest itself and posts the launcher
    // activity once its loop turns, so argv stays bare.
    if (execSelf) {
        // Supervisor mode: replace THIS process with the app (no fork, no
        // detach, stdio untouched). Under `init`-style respawn this makes the
        // supervisor own the whole lifecycle for free — the app exiting IS am
        // exiting, so ::respawn/systemd Restart re-launches it (AOSP
        // persistent-app restart + Home-return in one line of init config).
        execl(e->exePath().c_str(), e->exeName.c_str(), (char*)nullptr);
        fprintf(stderr, "Error: cannot exec %s\n", e->exePath().c_str());
        return 127;
    }
    const pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        setsid();
        int nullfd = open("/dev/null", O_RDWR);
        if (nullfd >= 0) {
            dup2(nullfd, 0); dup2(nullfd, 1); dup2(nullfd, 2);
            if (nullfd > 2) close(nullfd);
        }
        execl(e->exePath().c_str(), e->exeName.c_str(), (char*)nullptr);
        _exit(127);
    }
    // Reap any immediate exec failure (127) without waiting for the GUI.
    int status = 0;
    waitpid(pid, &status, WNOHANG);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
        fprintf(stderr, "Error: cannot exec %s\n", e->exePath().c_str());
        return 1;
    }
    printf("Starting: Intent { cmp=%s/%s }\n", pkg.c_str(), activity.c_str());
    return 0;
}

// kill processes whose /proc/<pid>/exe (fallback: comm, truncated to 15
// chars by the kernel) matches the package's binary. /proc/<pid>/cmdline
// argv[0] carries the full path — prefer that.
static int cmdForceStop(const std::string& pkg) {
    PackageDB db;
    db.load();
    const cmds::PackageEntry* e = db.find(pkg);
    if (!e) {
        fprintf(stderr, "Error: package %s is not installed\n", pkg.c_str());
        return 1;
    }
    int killed = 0;
    if (DIR* d = opendir("/proc")) {
        while (struct dirent* de = readdir(d)) {
            if (de->d_name[0] < '0' || de->d_name[0] > '9') continue;
            char link[512];
            snprintf(link, sizeof(link), "/proc/%s/cmdline", de->d_name);
            char name[512] = {0};
            const int fd = open(link, O_RDONLY);
            if (fd < 0) continue;
            const ssize_t n = read(fd, name, sizeof(name) - 1);
            close(fd);
            if (n <= 0) continue;
            if (e->exePath() == name) {
                kill(atoi(de->d_name), SIGKILL);
                killed++;
            }
        }
        closedir(d);
    }
    printf("%s: killed %d process(es)\n", pkg.c_str(), killed);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) { usage(); return 1; }
    const std::string cmd = argv[1];
    if (cmd == "start")  return cmdStart(argc - 2, argv + 2);
    if (cmd == "force-stop") {
        if (argc < 3) { usage(); return 1; }
        return cmdForceStop(argv[2]);
    }
    usage();
    return 1;
}
