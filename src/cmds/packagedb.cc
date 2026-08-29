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
// PackageDB + readPakInfo implementation. The XML reads go through the
// framework's own XmlPullParser (binary AXML for the pak manifest, text mode
// for packages.xml) with a null Context: attribute lookups here are all
// by bare name (binaryAttrIndex), which never touches the context — only
// resource-reference rendering does, and manifest package/versionCode/name
// values are plain strings/ints.
#include <cmds/packagedb.h>

#include <private/ziparchive.h>
#include <core/xmlpullparser.h>

using cdroid::XmlPullParser;
using cdroid::ZIPArchive;

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>

namespace cmds {

std::string PackageDB::dataRoot() {
    const char* env = getenv("CDROID_DATA_DIR");
    return (env && *env) ? std::string(env) : std::string("/data");
}

void PackageDB::load() {
    mEntries.clear();
    std::unique_ptr<std::istream> in(new std::ifstream(dbPath()));
    if (!static_cast<std::ifstream*>(in.get())->is_open())
        return;   // fresh device — empty registry
    auto parser = XmlPullParser::detectAndCreate(nullptr, std::move(in));
    int type;
    while ((type = parser->next()) != XmlPullParser::END_DOCUMENT) {
        if (type != XmlPullParser::START_TAG || parser->getName() != "package")
            continue;
        PackageEntry e;
        e.name = parser->getAttributeValue("", "name");
        e.codePath = parser->getAttributeValue("", "codePath");
        e.exeName = parser->getAttributeValue("", "exe");
        e.launcher = parser->getAttributeValue("", "launcher");
        e.versionCode = atoi(parser->getAttributeValue("", "versionCode").c_str());
        e.installTime = atol(parser->getAttributeValue("", "installTime").c_str());
        if (!e.name.empty() && !e.exeName.empty()) mEntries.push_back(e);
    }
}

bool PackageDB::save() const {
    const std::string path = dbPath();
    mkpath(path.substr(0, path.rfind('/')));
    std::ostringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<packages>\n";
    for (const auto& e : mEntries) {
        ss << "  <package name=\"" << e.name << "\" versionCode=\"" << e.versionCode
           << "\" exe=\"" << e.exeName << "\" launcher=\"" << e.launcher
           << "\"\n           codePath=\"" << e.codePath
           << "\" installTime=\"" << e.installTime << "\"/>\n";
    }
    ss << "</packages>\n";
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) return false;
        out << ss.str();
        if (!out) return false;
    }
    return ::rename(tmp.c_str(), path.c_str()) == 0;
}

const PackageEntry* PackageDB::find(const std::string& pkg) const {
    for (const auto& e : mEntries)
        if (e.name == pkg) return &e;
    return nullptr;
}

PackageEntry* PackageDB::find(const std::string& pkg) {
    for (auto& e : mEntries)
        if (e.name == pkg) return &e;
    return nullptr;
}

void PackageDB::add(const PackageEntry& entry) {
    PackageEntry* existing = find(entry.name);
    if (existing) *existing = entry;
    else mEntries.push_back(entry);
}

bool PackageDB::remove(const std::string& pkg) {
    for (size_t i = 0; i < mEntries.size(); i++) {
        if (mEntries[i].name == pkg) {
            mEntries.erase(mEntries.begin() + i);
            return true;
        }
    }
    return false;
}

bool readPakInfo(const std::string& pakPath, PakInfo& out) {
    ZIPArchive pak(pakPath.c_str());

    // Sole embedded binary: bin/<name> (the bundle contract).
    std::vector<std::string> entries;
    pak.getEntries(entries);
    for (const auto& e : entries) {
        if (e.compare(0, 4, "bin/") != 0 || e.size() <= 4) continue;
        if (!out.exeName.empty()) {
            fprintf(stderr, "Error: bundle carries more than one bin/ entry "
                    "(%s and %s)\n", out.exeName.c_str(), e.c_str() + 4);
            return false;
        }
        out.exeName = e.substr(4);
    }
    if (out.exeName.empty()) {
        fprintf(stderr, "Error: no bin/ entry in %s — not an installable bundle "
                "(rebuild with CreatePAK EMBED_EXE)\n", pakPath.c_str());
        return false;
    }

    std::istream* stm = pak.getInputStream("AndroidManifest.xml");
    if (!stm) {
        fprintf(stderr, "Error: no AndroidManifest.xml in %s\n", pakPath.c_str());
        return false;
    }
    auto parser = XmlPullParser::detectAndCreate(nullptr,
            std::unique_ptr<std::istream>(stm));

    // Same walk as App::parsePackageManifest, bare-name attribute lookups.
    bool inActivity = false, sawMain = false, sawLauncher = false;
    std::string activityName;
    int type;
    while ((type = parser->next()) != XmlPullParser::END_DOCUMENT) {
        if (type == XmlPullParser::START_TAG) {
            const std::string tag = parser->getName();
            if (tag == "manifest") {
                out.package = parser->getAttributeValue("", "package");
                out.versionCode = atoi(parser->getAttributeValue("", "versionCode").c_str());
            } else if (tag == "activity") {
                inActivity = true;
                activityName = parser->getAttributeValue("", "name");
                sawMain = sawLauncher = false;
            } else if (inActivity && tag == "action") {
                if (parser->getAttributeValue("", "name") == "android.intent.action.MAIN")
                    sawMain = true;
            } else if (inActivity && tag == "category") {
                if (parser->getAttributeValue("", "name") == "android.intent.category.LAUNCHER")
                    sawLauncher = true;
            }
        } else if (type == XmlPullParser::END_TAG && parser->getName() == "activity") {
            if (inActivity && sawMain && sawLauncher && out.launcher.empty())
                out.launcher = activityName;
            inActivity = false;
        }
    }
    if (out.package.empty()) {
        fprintf(stderr, "Error: manifest in %s declares no package\n", pakPath.c_str());
        return false;
    }
    return true;
}

bool mkpath(const std::string& dir) {
    if (dir.empty()) return false;
    std::string cur;
    size_t pos = 0;
    while (pos <= dir.size()) {
        const size_t slash = dir.find('/', pos + 1);
        cur = dir.substr(0, slash == std::string::npos ? std::string::npos : slash);
        if (!cur.empty() && ::mkdir(cur.c_str(), 0755) != 0 && errno != EEXIST)
            return false;
        if (slash == std::string::npos) break;
        pos = slash;
    }
    return true;
}

} // namespace cmds
