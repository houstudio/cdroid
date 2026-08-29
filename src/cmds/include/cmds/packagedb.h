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
#ifndef __CMDS_PACKAGEDB_H__
#define __CMDS_PACKAGEDB_H__
// Device-side package registry shared by pm / am (the PackageManagerService
// data model, minus binder: CDROID has no system_server, so the database is a
// plain XML file both CLIs — and later the launcher — read/write directly).
//
// Layout under the data root (AOSP shapes; CDROID_DATA_DIR overrides for
// desktop testing, default /data):
//   <root>/app/<pkg>-<serial>/<exe>        installed binary (executable)
//   <root>/app/<pkg>-<serial>/<exe>.pak    the app's resources
//   <root>/system/packages.xml             this registry
//   <root>/system/sessions/<id>/...        pm install-create staging
//
// The <exe>.pak naming is deliberate: App probes getDataPath()+<exe>.pak
// (the executable's own directory), so the installed pair runs with ZERO
// runtime changes.
#include <string>
#include <vector>
#include <ctime>

namespace cmds {

struct PackageEntry {
    std::string name;          // manifest package, e.g. "cdroid.preferencedemo"
    std::string codePath;      // install dir, e.g. "/data/app/cdroid.preferencedemo-1"
    std::string exeName;       // binary basename, e.g. "preferencedemo"
    std::string launcher;      // MAIN/LAUNCHER activity (".MainActivity"), "" if none
    int versionCode = 1;
    long installTime = 0;      // time(nullptr) at install

    std::string exePath() const { return codePath + "/" + exeName; }
    std::string pakPath() const { return codePath + "/" + exeName + ".pak"; }
};

// What pm can learn from a bundle pak without starting an App: the compiled
// AndroidManifest.xml (package/versionCode/launcher activity) plus the single
// embedded binary under bin/ (the bundle contract: exactly one entry).
struct PakInfo {
    std::string package;
    std::string launcher;
    std::string exeName;       // basename of the sole bin/ entry
    int versionCode = 1;
};

class PackageDB {
public:
    // Data root: $CDROID_DATA_DIR when set (desktop testing), else /data.
    static std::string dataRoot();
    static std::string dbPath() { return dataRoot() + "/system/packages.xml"; }
    static std::string sessionsDir() { return dataRoot() + "/system/sessions"; }
    static std::string appDir() { return dataRoot() + "/app"; }

    // Loads the registry (missing file = empty db, not an error).
    void load();
    // Writes the registry atomically (temp file + rename).
    bool save() const;

    const PackageEntry* find(const std::string& pkg) const;
    PackageEntry* find(const std::string& pkg);
    void add(const PackageEntry& entry);
    bool remove(const std::string& pkg);

    const std::vector<PackageEntry>& all() const { return mEntries; }

private:
    std::vector<PackageEntry> mEntries;
};

// Parses a bundle pak's compiled AndroidManifest.xml + bin/ listing using the
// framework's own ZIPArchive/XmlPullParser (no App instance, no graphics).
// Returns false with an error line on stderr when the file is not a valid
// installable bundle (no manifest / no embedded binary).
bool readPakInfo(const std::string& pakPath, PakInfo& out);

// mkdir -p. Returns false (errno set) on failure.
bool mkpath(const std::string& dir);

} // namespace cmds
#endif /*__CMDS_PACKAGEDB_H__*/
