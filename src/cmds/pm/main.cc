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
// pm — the AOSP /system/bin/pm role for CDROID: installs adb-installable pak
// bundles (manifest + resources + the embedded bin/<exe> entry), keeps the
// packages.xml registry, and answers pm list/path for the launcher and AS.
//
// `adb install foo.pak` reaches this binary twice: once as
//   pm install-create -r -t ...        → "Success: created install session [N]"
//   pm install-write -S <size> N base -   (session file from stdin)
//   pm install-commit N
// and on older adb clients directly as `pm install -r <path>`. Both paths are
// implemented; the legacy single-shot form is what scripts and CI use.
#include <cmds/packagedb.h>

#include <private/ziparchive.h>
#include <porting/cdlog.h>

using cdroid::ZIPArchive;

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using cmds::PackageDB;
using cmds::PackageEntry;
using cmds::PakInfo;

static void usage() {
    fprintf(stderr,
        "usage: pm <subcommand>\n"
        "  pm install [-r] <pak>            install bundle (replaces existing with -r)\n"
        "  pm install-create [-r] [-t] ...  create a staged session (prints session id)\n"
        "  pm install-write <id> <name> (- | <path>)\n"
        "                                   stream data into session <id>\n"
        "  pm install-commit <id>           apply session <id>\n"
        "  pm install-abandon <id>          drop session <id>\n"
        "  pm uninstall <package>           remove bundle + registry entry\n"
        "  pm list packages [-f]            list installed (paths with -f)\n"
        "  pm path <package>                print installed pak path\n"
        "data root: $CDROID_DATA_DIR (default /data)\n");
}

// rm -r (dirs + files), no symlink following concerns on the install tree.
static bool removeTree(const std::string& path) {
    DIR* d = opendir(path.c_str());
    if (d) {
        while (struct dirent* de = readdir(d)) {
            if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
            if (!removeTree(path + "/" + de->d_name)) { closedir(d); return false; }
        }
        closedir(d);
        return rmdir(path.c_str()) == 0;
    }
    return unlink(path.c_str()) == 0 || errno == ENOENT;
}

// Streams one zip entry out to a file (the bin/<exe> extraction), then chmods
// it executable. 0755, like installd does for native bridge binaries.
static bool extractEntry(const std::string& pakPath, const std::string& entry,
                         const std::string& outPath, mode_t mode) {
    ZIPArchive pak(pakPath);
    std::istream* stm = pak.getInputStream(entry);
    if (!stm) {
        fprintf(stderr, "Error: %s has no %s entry\n", pakPath.c_str(), entry.c_str());
        return false;
    }
    std::unique_ptr<std::istream> hold(stm);
    std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    char buf[64 * 1024];
    while (stm->read(buf, sizeof(buf)) || stm->gcount() > 0)
        out.write(buf, stm->gcount());
    out.flush();
    if (!out) return false;
    return ::chmod(outPath.c_str(), mode) == 0;
}

static bool copyFile(const std::string& from, const std::string& to) {
    std::ifstream in(from, std::ios::binary);
    std::ofstream out(to, std::ios::binary | std::ios::trunc);
    if (!in || !out) return false;
    out << in.rdbuf();
    return (bool)out;
}

// The PackageManagerService install core: validate bundle → stage to
// <root>/app/<pkg>-<serial>/ → register. Existing package survives a failed
// install (stage first, then drop the old dir — AOSP order).
static bool doInstall(const std::string& pakPath, bool replace) {
    PakInfo info;
    if (!cmds::readPakInfo(pakPath, info)) return false;

    PackageDB db;
    db.load();
    const PackageEntry* existing = db.find(info.package);
    if (existing && !replace) {
        fprintf(stderr, "Error: INSTALL_FAILED_ALREADY_EXISTS (package %s; "
                "use -r to replace)\n", info.package.c_str());
        return false;
    }
    if (existing && replace && info.versionCode < existing->versionCode) {
        fprintf(stderr, "Error: INSTALL_FAILED_VERSION_DOWNGRADE "
                "(%d < installed %d)\n", info.versionCode, existing->versionCode);
        return false;
    }

    // <pkg>-<serial>: next free number under the app dir (AOSP layout).
    int serial = 1;
    const std::string prefix = PackageDB::appDir() + "/" + info.package + "-";
    if (DIR* d = opendir(PackageDB::appDir().c_str())) {
        while (struct dirent* de = readdir(d)) {
            if (!strncmp(de->d_name, (info.package + "-").c_str(), info.package.size() + 1)) {
                const int n = atoi(de->d_name + info.package.size() + 1);
                if (n >= serial) serial = n + 1;
            }
        }
        closedir(d);
    }
    const std::string codePath = prefix + std::to_string(serial);
    if (!cmds::mkpath(codePath)) {
        fprintf(stderr, "Error: cannot create %s (%s)\n", codePath.c_str(), strerror(errno));
        return false;
    }

    // Stage: the binary out of bin/<exe> (0755), the pak itself beside it as
    // <exe>.pak — the exact pair App probes in its own directory.
    if (!extractEntry(pakPath, "bin/" + info.exeName, codePath + "/" + info.exeName, 0755)) {
        removeTree(codePath);
        return false;
    }
    if (!copyFile(pakPath, codePath + "/" + info.exeName + ".pak")) {
        fprintf(stderr, "Error: cannot stage pak into %s\n", codePath.c_str());
        removeTree(codePath);
        return false;
    }

    // db.add overwrites the existing entry IN PLACE (existing points into the
    // vector), so capture the old dir before registering — it is removed only
    // after the registry commit succeeds (AOSP keeps the old install alive
    // through a failed upgrade).
    const std::string oldPath = existing ? existing->codePath : std::string();

    PackageEntry entry;
    entry.name = info.package;
    entry.codePath = codePath;
    entry.exeName = info.exeName;
    entry.launcher = info.launcher;
    entry.versionCode = info.versionCode;
    entry.installTime = time(nullptr);
    db.add(entry);
    if (!db.save()) {
        fprintf(stderr, "Error: cannot write %s\n", PackageDB::dbPath().c_str());
        removeTree(codePath);
        return false;
    }

    // Registry committed — the old version dir can go.
    if (!oldPath.empty() && oldPath != codePath) removeTree(oldPath);
    printf("Success\n");
    return true;
}

// ---- staged sessions (modern adb install: create/write/commit) ----

static int nextSessionId() {
    int id = 1;
    if (DIR* d = opendir(PackageDB::sessionsDir().c_str())) {
        while (struct dirent* de = readdir(d)) {
            const int n = atoi(de->d_name);
            if (n >= id) id = n + 1;
        }
        closedir(d);
    }
    return id;
}

// Flags install-create may carry that we tolerate but don't need (the values
// --user/-i/--intent drag along are consumed too, so argv stays in sync).
static bool sessionFlagWithValue(const std::string& f) {
    return f == "--user" || f == "-i" || f == "--intent" || f == "-S"
        || f == "--dev-certifiable-metrics" || f == "--reason";
}

static int cmdInstallCreate(int argc, char** argv) {
    bool replace = false;
    for (int i = 0; i < argc; i++) {
        const std::string a = argv[i];
        if (a == "-r" || a == "--replace") replace = true;
        else if (sessionFlagWithValue(a)) i++;   // skip the value
        // everything else (-t, -d, --fastdeploy, ...): tolerated, ignored
    }
    cmds::mkpath(PackageDB::sessionsDir());
    const int id = nextSessionId();
    const std::string dir = PackageDB::sessionsDir() + "/" + std::to_string(id);
    if (::mkdir(dir.c_str(), 0755) != 0) {
        fprintf(stderr, "Error: cannot create session dir %s\n", dir.c_str());
        return 1;
    }
    std::ofstream flags(dir + "/flags");
    if (replace) flags << "-r\n";
    printf("Success: created install session [%d]\n", id);
    return 0;
}

static int cmdInstallWrite(int argc, char** argv) {
    // argv: [-S <size>] <id> <name> (- | <path>)
    int i = 0;
    if (i < argc && !strcmp(argv[i], "-S")) i += 2;   // size known from the stream
    if (argc - i < 3) {
        fprintf(stderr, "Error: install-write needs <id> <name> (- | <path>)\n");
        return 1;
    }
    const std::string id = argv[i++];
    const std::string name = argv[i++];
    const std::string src = argv[i];
    if (name.find('/') != std::string::npos) {
        fprintf(stderr, "Error: bad session file name %s\n", name.c_str());
        return 1;
    }
    const std::string dst = PackageDB::sessionsDir() + "/" + id + "/" + name;
    if (src == "-") {
        std::ofstream out(dst, std::ios::binary | std::ios::trunc);
        if (!out) { fprintf(stderr, "Error: cannot write %s\n", dst.c_str()); return 1; }
        char buf[64 * 1024];
        while (std::cin.read(buf, sizeof(buf)) || std::cin.gcount() > 0)
            out.write(buf, std::cin.gcount());
        return out ? 0 : 1;
    }
    return copyFile(src, dst) ? 0
        : (fprintf(stderr, "Error: cannot read %s\n", src.c_str()), 1);
}

static int cmdInstallCommit(int argc, char** argv) {
    if (argc < 1) {
        fprintf(stderr, "Error: install-commit needs a session id\n");
        return 1;
    }
    const std::string dir = PackageDB::sessionsDir() + "/" + argv[0];
    DIR* d = opendir(dir.c_str());
    if (!d) {
        fprintf(stderr, "Error: no install session %s\n", argv[0]);
        return 1;
    }
    bool replace = false;
    std::string pak;
    while (struct dirent* de = readdir(d)) {
        const std::string n = de->d_name;
        if (n == "flags") continue;
        if (n[0] == '.') continue;
        pak = dir + "/" + n;   // adb writes exactly one staged file
    }
    closedir(d);
    if (pak.empty()) {
        fprintf(stderr, "Error: session %s is empty\n", argv[0]);
        return 1;
    }
    std::ifstream flags(dir + "/flags");
    if (flags) {
        std::string f;
        while (flags >> f) if (f == "-r") replace = true;
    }
    const bool ok = doInstall(pak, replace);
    removeTree(dir);
    return ok ? 0 : 1;
}

static int cmdUninstall(const std::string& pkg) {
    PackageDB db;
    db.load();
    const PackageEntry* e = db.find(pkg);
    if (!e) {
        fprintf(stderr, "Failure [not installed for 0]\n");
        return 1;
    }
    const std::string codePath = e->codePath;
    db.remove(pkg);
    db.save();
    removeTree(codePath);
    printf("Success\n");
    return 0;
}

static int cmdListPackages(bool withPath) {
    PackageDB db;
    db.load();
    for (const auto& e : db.all()) {
        if (withPath) printf("package:%s=%s\n", e.pakPath().c_str(), e.name.c_str());
        else printf("package:%s\n", e.name.c_str());
    }
    return 0;
}

int main(int argc, char** argv) {
    // adb's install machinery greps our stdout/stderr for "Success" and the
    // session id — keep the framework's debug chatter (binary-AXML attr
    // warnings etc.) out of the pipe.
    LogSetModuleLevel(nullptr, LOG_WARN);
    if (argc < 2) { usage(); return 1; }
    const std::string cmd = argv[1];
    char** rest = argv + 2;
    int nrest = argc - 2;

    if (cmd == "install") {
        bool replace = false;
        std::string path;
        for (int i = 0; i < nrest; i++) {
            if (!strcmp(rest[i], "-r")) replace = true;
            else if (rest[i][0] != '-') path = rest[i];
        }
        if (path.empty()) { usage(); return 1; }
        return doInstall(path, replace) ? 0 : 1;
    }
    if (cmd == "install-create") return cmdInstallCreate(nrest, rest);
    if (cmd == "install-write")  return cmdInstallWrite(nrest, rest);
    if (cmd == "install-commit") return cmdInstallCommit(nrest, rest);
    if (cmd == "install-abandon") {
        if (nrest < 1) { usage(); return 1; }
        const std::string dir = PackageDB::sessionsDir() + "/" + rest[0];
        removeTree(dir);
        printf("Success\n");
        return 0;
    }
    if (cmd == "uninstall") {
        if (nrest < 1) { usage(); return 1; }
        return cmdUninstall(rest[nrest - 1]);
    }
    if (cmd == "list") {
        if (nrest < 1 || strcmp(rest[0], "packages")) { usage(); return 1; }
        bool withPath = false;
        for (int i = 1; i < nrest; i++)
            if (!strcmp(rest[i], "-f")) withPath = true;
        return cmdListPackages(withPath);
    }
    if (cmd == "path") {
        if (nrest < 1) { usage(); return 1; }
        PackageDB db;
        db.load();
        const PackageEntry* e = db.find(rest[nrest - 1]);
        if (!e) { fprintf(stderr, "Error: package %s not found\n", rest[nrest - 1]); return 1; }
        printf("package:%s\n", e->pakPath().c_str());
        return 0;
    }
    usage();
    return 1;
}
