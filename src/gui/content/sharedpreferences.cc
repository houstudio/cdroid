/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <content/sharedpreferences.h>
#include <core/queuedwork.h>
#include <core/xmlpullparser.h>

#include <porting/cdlog.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace cdroid {

/*============================================================================*
 * On-disk location
 *============================================================================*/

// CDROID data directory for preference files. AOSP stores these under
// /data/data/<pkg>/shared_prefs/; CDROID has no per-app sandbox, so the
// convention is $HOME/.cdroid/prefs/.
static std::string prefsDirectory() {
    const char* home = getenv("HOME");
    std::string dir = (home && *home) ? std::string(home) : std::string("/tmp");
    dir += "/.cdroid/prefs";
    return dir;
}

/*============================================================================*
 * Typed value model (the Map<String, ?> entry role)
 *============================================================================*/

// AOSP boxes String/Integer/Long/Float/Boolean/HashSet<String> into the map
// (null never survives into mMap). C++14 has no variant, so a tagged struct
// plays the box; Type::Null is only ever a mutation marker in mModified —
// the "this is the magic value for a removal" role of EditorImpl.
struct PrefValue {
    enum Type { Null, String, StringSet, Int, Long, Float, Boolean };
    Type type = Null;
    std::string str;                 // String
    std::set<std::string> set;       // StringSet
    int64_t i = 0;                   // Int / Long / Boolean
    float f = 0.f;                   // Float

    static PrefValue nullValue() { return PrefValue(); }
    static PrefValue of(const std::string& v) { PrefValue r; r.type = String; r.str = v; return r; }
    static PrefValue of(const std::set<std::string>& v) { PrefValue r; r.type = StringSet; r.set = v; return r; }
    static PrefValue ofInt(int64_t v) { PrefValue r; r.type = Int; r.i = v; return r; }
    static PrefValue ofLong(int64_t v) { PrefValue r; r.type = Long; r.i = v; return r; }
    static PrefValue ofFloat(float v) { PrefValue r; r.type = Float; r.f = v; return r; }
    static PrefValue ofBoolean(bool v) { PrefValue r; r.type = Boolean; r.i = v ? 1 : 0; return r; }

    bool operator==(const PrefValue& o) const {
        if (type != o.type) return false;
        switch (type) {
            case String:    return str == o.str;
            case StringSet: return set == o.set;
            case Float:     return f == o.f;
            default:        return i == o.i;   // Null / Int / Long / Boolean
        }
    }
};

using PrefMap = std::map<std::string, PrefValue>;

/*============================================================================*
 * XML format — XmlUtils.writeMapXml / readMapXml, byte-compatible output
 *============================================================================*/

// FastXmlSerializer's escapeAndAppendString table: control chars become
// numeric character references, plus '"' '&' '<' '>'. Single quotes pass
// through. The same table is used for attribute values and element text.
// (Control chars other than \t \n \r are not valid XML 1.0 even as
// references — expat rejects them on read-back, the same hole AOSP's
// KXmlParser-based round trip has.)
static void escapeXml(std::string& out, const std::string& s) {
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "&quot;"; break;
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            default:
                if (c < 32) {
                    char buf[8];
                    snprintf(buf, sizeof buf, "&#%u;", (unsigned)c);
                    out += buf;
                } else {
                    out += (char)c;
                }
        }
    }
}

// Java's Float.toString: the shortest decimal that round-trips, always with
// a decimal point ("1.0"). Find the shortest round-tripping %g form, then
// restore the ".0". (Exponent formatting may differ in padding; round-trip
// fidelity is what matters.)
static std::string javaFloatString(float v) {
    char buf[64] = {0};
    for (int prec = 1; prec <= 9; prec++) {
        snprintf(buf, sizeof buf, "%.*g", prec, (double)v);
        if (strtof(buf, nullptr) == v) break;
    }
    std::string s(buf);
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos
            && s.find('E') == std::string::npos && s.find("inf") == std::string::npos
            && s.find("nan") == std::string::npos) {
        s += ".0";
    }
    return s;
}

// XmlUtils.writeValueXml: named entry at the given indent level. Numbers and
// booleans are empty elements carrying value=; strings carry element text;
// sets hold nested name-less <string> children. indent-output feature =
// 4 spaces per level, tags closed with ">\n" / " />".
static void writeValueXml(std::string& out, const std::string& name, const PrefValue& v, int indent) {
    out.append(indent * 4, ' ');
    switch (v.type) {
        case PrefValue::String:
            out += "<string name=\"";
            escapeXml(out, name);
            out += "\">";
            escapeXml(out, v.str);
            out += "</string>\n";
            break;
        case PrefValue::Int:
            out += "<int name=\""; escapeXml(out, name);
            out += "\" value=\"" + std::to_string(v.i) + "\" />\n";
            break;
        case PrefValue::Long:
            out += "<long name=\""; escapeXml(out, name);
            out += "\" value=\"" + std::to_string(v.i) + "\" />\n";
            break;
        case PrefValue::Float:
            out += "<float name=\""; escapeXml(out, name);
            out += "\" value=\"" + javaFloatString(v.f) + "\" />\n";
            break;
        case PrefValue::Boolean:
            out += "<boolean name=\""; escapeXml(out, name);
            out += std::string("\" value=\"") + (v.i ? "true" : "false") + "\" />\n";
            break;
        case PrefValue::StringSet:
            out += "<set name=\"";
            escapeXml(out, name);
            out += "\">\n";
            for (const auto& e : v.set) {
                out.append((indent + 1) * 4, ' ');
                out += "<string>";
                escapeXml(out, e);
                out += "</string>\n";
            }
            out.append(indent * 4, ' ');
            out += "</set>\n";
            break;
        case PrefValue::Null:
            break;   // unreachable: nulls are removals and never enter mMap
    }
}

// XmlUtils.writeMapXml through Xml.newFastSerializer(): header line + <map>
// root + one entry per value.
static std::string writeMapXml(const PrefMap& map) {
    std::string out = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n";
    out += "<map>\n";
    for (const auto& e : map) writeValueXml(out, e.first, e.second, 1);
    out += "</map>\n";
    return out;
}

// --- reading ----------------------------------------------------------------

// Terminal events: expat parse errors surface as BAD_DOCUMENT, a clean EOF
// as END_DOCUMENT (xmlpullparser.cc acquire(BAD_DOCUMENT)); both end the
// parse — anything else must keep the loops advancing.
static bool isEofEvent(int ev) {
    return ev == XmlPullParser::END_DOCUMENT || ev == XmlPullParser::BAD_DOCUMENT;
}

static bool parseLongStrict(const std::string& s, int64_t& out) {
    if (s.empty()) return false;
    errno = 0;
    char* end = nullptr;
    const long long v = strtoll(s.c_str(), &end, 10);
    if (errno != 0 || end != s.c_str() + s.size()) return false;
    out = v;
    return true;
}

static bool parseFloatStrict(const std::string& s, float& out) {
    if (s.empty()) return false;
    char* end = nullptr;
    const float v = strtof(s.c_str(), &end);
    if (end != s.c_str() + s.size()) return false;
    out = v;
    return true;
}

// readThisValueXml for one entry. Entered with the current event at the
// entry's START_TAG; consumes events through the entry's END_TAG (the
// "Skip through to end tag" tail every branch runs). Returns false on any
// foreign shape.
static bool readValueXml(XmlPullParser& parser, const std::string& tag, PrefValue& out) {
    PrefValue v;
    if (tag == "string") {
        std::string text;
        int ev = parser.next();
        while (!isEofEvent(ev)) {
            if (ev == XmlPullParser::END_TAG) {
                if (parser.getName() != "string") return false;   // unexpected end tag
                out = PrefValue::of(text);
                return true;
            }
            if (ev == XmlPullParser::START_TAG) return false;   // "Unexpected start tag"
            if (ev == XmlPullParser::TEXT) text += parser.getText();
            ev = parser.next();
        }
        return false;   // unexpected end of document
    }
    if (tag == "set") {
        // AOSP's set children are generic readThisValueXml values; an Editor
        // only ever writes <string> children, and PrefValue can only hold a
        // set<string> — a foreign mixed set rejects the file (CDROID seam).
        v.type = PrefValue::StringSet;
        int ev = parser.next();
        while (!isEofEvent(ev)) {
            if (ev == XmlPullParser::END_TAG) {
                if (parser.getName() != "set") return false;
                out = v;
                return true;
            }
            if (ev == XmlPullParser::START_TAG) {
                if (parser.getName() != "string") return false;
                PrefValue child;
                if (!readValueXml(parser, "string", child)) return false;
                v.set.insert(child.str);
            }
            ev = parser.next();
        }
        return false;
    }
    if (tag == "int" || tag == "long") {
        int64_t n = 0;
        if (!parseLongStrict(parser.getAttributeValue("", "value"), n)) return false;
        if (tag == "int" && (n < INT32_MIN || n > INT32_MAX)) return false;   // parseInt overflow
        v = (tag == "int") ? PrefValue::ofInt(n) : PrefValue::ofLong(n);
    } else if (tag == "float") {
        float n = 0.f;
        if (!parseFloatStrict(parser.getAttributeValue("", "value"), n)) return false;
        v = PrefValue::ofFloat(n);
    } else if (tag == "boolean") {
        // Boolean.valueOf semantics: anything not "true" (case-insensitive)
        // is false; never fails.
        const std::string b = parser.getAttributeValue("", "value");
        v = PrefValue::ofBoolean(b.size() == 4
                && (b[0] | 0x20) == 't' && (b[1] | 0x20) == 'r'
                && (b[2] | 0x20) == 'u' && (b[3] | 0x20) == 'e');
    } else {
        // double / *-array / byte-array / null / nested map / list: AOSP's
        // generic reader would load them, but an Editor can never write
        // them. CDROID seam: reject the whole file (empty map) instead of
        // partially loading foreign values.
        return false;
    }

    // Skip through to end tag.
    int ev = parser.next();
    while (!isEofEvent(ev)) {
        if (ev == XmlPullParser::END_TAG) {
            if (parser.getName() != tag) return false;
            out = v;
            return true;
        }
        ev = parser.next();
    }
    return false;
}

// XmlUtils.readMapXml: skip to the first START_TAG, require the <map> root,
// then readThisMapXml's do-while over entries. Any anomaly returns false —
// the caller then treats the file as unreadable (AOSP: exception → null
// map → start empty).
static bool readMapXml(const std::string& path, PrefMap& outMap) {
    std::unique_ptr<std::istream> stream(new std::ifstream(path, std::ios::binary));
    if (!stream->good()) return false;
    XmlPullParser parser(nullptr, std::move(stream));

    int ev = parser.getEventType();
    while (ev != XmlPullParser::START_TAG) {
        if (isEofEvent(ev)) return false;
        ev = parser.next();
    }
    if (parser.getName() != "map") return false;
    ev = parser.next();   // first child (readThisValueXml's map branch)

    do {
        if (ev == XmlPullParser::START_TAG) {
            const std::string name = parser.getAttributeValue("", "name");
            if (name.empty()) return false;
            PrefValue v;
            if (!readValueXml(parser, parser.getName(), v)) return false;
            outMap[name] = v;   // later duplicates win, HashMap#put semantics
        } else if (ev == XmlPullParser::END_TAG) {
            // Expected </map> end tag — anything else is a nesting error.
            return parser.getName() == "map";
        } else if (isEofEvent(ev)) {
            return false;   // "Document ended before end tag"
        }
        ev = parser.next();
    } while (true);
}

/*============================================================================*
 * Private state
 *============================================================================*/

// AOSP SharedPreferencesImpl's fields. Kept alive by shared_ptr so async
// tasks (load, queued writes, finishers) outlive a dropped impl — the GC
// role.
struct SharedPreferencesImpl::Private {
    std::string file;         // mFile
    std::string backupFile;   // makeBackupFile(mFile) = <path>.bak
    int mode = 0;

    // Lock ordering rules (AOSP):
    //  - acquire mLock before EditorImpl's mEditorLock
    //  - acquire mWritingToDiskLock before mEditorLock
    std::mutex mLock;                    // mMap/mLoaded/mDiskWritesInFlight/mListeners/stat/generation
    std::mutex mWritingToDiskLock;       // writeToFile + mDiskStateGeneration
    std::condition_variable mLoadedCv;   // with mLock — awaitLoadedLocked's wait/notifyAll

    PrefMap mMap;                 // guarded by mLock
    bool mLoaded = false;         // guarded by mLock
    int mDiskWritesInFlight = 0;  // guarded by mLock

    // stat snapshot of the file as loaded/written (hasFileChangedUnexpectedly
    // bookkeeping for MODE_MULTI_PROCESS; recorded for fidelity).
    struct timespec mStatTimestamp {};
    off_t mStatSize = 0;          // guarded by mLock

    int64_t mCurrentMemoryStateGeneration = 0;   // guarded by mLock ("this")
    int64_t mDiskStateGeneration = 0;            // guarded by mWritingToDiskLock

    std::vector<SharedPreferences::OnSharedPreferenceChangeListener> mListeners;  // guarded by mLock
};

// AOSP MemoryCommitResult — the return value of EditorImpl#commitToMemory().
// The CountDownLatch becomes a bool + condvar.
struct MemoryCommitResult {
    const int64_t memoryStateGeneration;
    const bool keysCleared;
    const std::vector<std::string> keysModified;   // filled only when listeners existed
    const std::vector<SharedPreferences::OnSharedPreferenceChangeListener> listeners;
    PrefMap mapToWriteToDisk;                      // by value: the writer's frozen snapshot

    std::mutex latchMutex;
    std::condition_variable latchCv;
    bool writtenToDisk = false;   // latch state
    bool writeToDiskResult = false;
    bool wasWritten = false;

    MemoryCommitResult(int64_t gen, bool cleared, std::vector<std::string> keys,
            std::vector<SharedPreferences::OnSharedPreferenceChangeListener> l, PrefMap m)
        : memoryStateGeneration(gen), keysCleared(cleared), keysModified(std::move(keys)),
          listeners(std::move(l)), mapToWriteToDisk(std::move(m)) {}

    void setDiskWriteResult(bool wasW, bool result) {
        std::lock_guard<std::mutex> lk(latchMutex);
        wasWritten = wasW;
        writeToDiskResult = result;
        writtenToDisk = true;
        latchCv.notify_all();
    }

    void awaitWrittenToDisk() {
        std::unique_lock<std::mutex> lk(latchMutex);
        latchCv.wait(lk, [this] { return writtenToDisk; });
    }
};

/*============================================================================*
 * Loading
 *============================================================================*/

// AOSP sLoadExecutor: a process-wide ThreadPoolExecutor(0, 1, 10s idle) —
// one shared loader thread that leaves after 10s idle. A lazily started
// detached thread with an idle timeout gives the same shape.
static void sLoadExecutorPost(std::function<void()> task) {
    struct Loader {
        std::mutex mu;
        std::condition_variable cv;
        std::deque<std::function<void()>> tasks;
        bool running = false;
    };
    // Leaked on purpose (the JVM-daemon role — AOSP's executor threads are
    // process-lifetime and never joined): a destroyed static would race the
    // idle-waiting loader thread at exit (cond dtor under a waiter).
    static Loader* loader = new Loader();

    std::unique_lock<std::mutex> lk(loader->mu);
    loader->tasks.push_back(std::move(task));
    if (loader->running) {
        loader->cv.notify_all();
        return;
    }
    loader->running = true;
    std::thread([] {
        while (true) {
            std::function<void()> t;
            {
                std::unique_lock<std::mutex> lk(loader->mu);
                if (loader->tasks.empty()) {
                    // allowCoreThreadTimeOut: leave after 10s idle
                    if (!loader->cv.wait_for(lk, std::chrono::seconds(10),
                            [] { return !loader->tasks.empty(); })
                            && loader->tasks.empty()) {
                        loader->running = false;
                        return;
                    }
                }
                t = std::move(loader->tasks.front());
                loader->tasks.pop_front();
            }
            if (t) t();
        }
    }).detach();
    loader->cv.notify_all();
}

// AOSP loadFromDisk: restore a leftover backup first (the last write failed
// mid-flight), then read the file. An unreadable/corrupt file means an empty
// map, never a load error surfaced to callers.
static void loadFromDisk(std::shared_ptr<SharedPreferencesImpl::Private> p) {
    {
        std::lock_guard<std::mutex> lk(p->mLock);
        if (p->mLoaded) return;
        struct stat bs;
        if (::stat(p->backupFile.c_str(), &bs) == 0) {
            // Backup exists: the last write crashed; the backup is the only
            // trustworthy copy. Restore it over the (partial) file.
            ::unlink(p->file.c_str());
            ::rename(p->backupFile.c_str(), p->file.c_str());
        }
    }

    PrefMap map;
    struct stat st {};
    bool haveStat = false;
    if (::stat(p->file.c_str(), &st) == 0) {
        haveStat = true;
        PrefMap loaded;
        if (readMapXml(p->file, loaded)) {
            map = std::move(loaded);
        } else {
            LOGW("Cannot read %s (corrupt or foreign format); starting empty",
                    p->file.c_str());
        }
    }

    std::lock_guard<std::mutex> lk(p->mLock);
    p->mLoaded = true;
    p->mMap = std::move(map);
    if (haveStat) {
        p->mStatTimestamp = st.st_mtim;
        p->mStatSize = st.st_size;
    }
    // It's important that we always signal waiters, even if we'll make them
    // fail with an exception (mThrowable has no C++ counterpart here).
    p->mLoadedCv.notify_all();
}

static void startLoadFromDisk(const std::shared_ptr<SharedPreferencesImpl::Private>& p) {
    {
        std::lock_guard<std::mutex> lk(p->mLock);
        p->mLoaded = false;
    }
    sLoadExecutorPost([p] { loadFromDisk(std::move(p)); });
}

// awaitLoadedLocked: call with a unique_lock on p->mLock held.
static void awaitLoadedLocked(SharedPreferencesImpl::Private* p, std::unique_lock<std::mutex>& lk) {
    p->mLoadedCv.wait(lk, [p] { return p->mLoaded; });
}

/*============================================================================*
 * Writing
 *============================================================================*/

// mkdir -p for the prefs directory chain. AOSP's createFileOutputStream only
// ever needs one level (the /data/data/<pkg> tree pre-exists); CDROID's
// $HOME may not have .cdroid/ at all, so walk the whole chain — including
// the final component.
static bool ensureDirectories(const std::string& dir) {
    for (size_t i = 1; i <= dir.size(); i++) {
        if (dir[i] == '/' || i == dir.size()) {
            if (mkdir(dir.substr(0, i).c_str(), 0755) != 0 && errno != EEXIST) return false;
        }
    }
    return true;
}

// AOSP createFileOutputStream: on open failure, mkdir the parent and retry.
static FILE* createFileOutputStream(const std::string& path) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) {
        const size_t slash = path.find_last_of('/');
        if (slash != std::string::npos && slash > 0) {
            if (!ensureDirectories(path.substr(0, slash))) {
                LOGE("Couldn't create directory for SharedPreferences file %s", path.c_str());
                return nullptr;
            }
            f = fopen(path.c_str(), "wb");
            if (!f) LOGE("Couldn't create SharedPreferences file %s", path.c_str());
        }
    }
    return f;
}

// AOSP writeToFile, called with mWritingToDiskLock held. Attempts to write
// the file, delete the backup and return true as atomically as possible;
// on failure deletes the new file — the next load restores from the backup.
static void writeToFile(SharedPreferencesImpl::Private* p, MemoryCommitResult* mcr,
        bool isFromSyncCommit) {
    struct stat st;
    const bool fileExists = ::stat(p->file.c_str(), &st) == 0;

    if (fileExists) {
        bool needsWrite = false;
        // Only need to write if the disk state is older than this commit
        if (p->mDiskStateGeneration < mcr->memoryStateGeneration) {
            if (isFromSyncCommit) {
                needsWrite = true;
            } else {
                std::lock_guard<std::mutex> lk(p->mLock);
                // No need to persist intermediate states. Just wait for the
                // latest state to be persisted.
                if (p->mCurrentMemoryStateGeneration == mcr->memoryStateGeneration) {
                    needsWrite = true;
                }
            }
        }
        if (!needsWrite) {
            mcr->setDiskWriteResult(false, true);
            return;
        }

        // Rename the current file so it may be used as a backup during the
        // next read.
        if (::access(p->backupFile.c_str(), F_OK) != 0) {
            if (::rename(p->file.c_str(), p->backupFile.c_str()) != 0) {
                LOGE("Couldn't rename file %s to backup file %s",
                        p->file.c_str(), p->backupFile.c_str());
                mcr->setDiskWriteResult(false, false);
                return;
            }
        } else {
            ::unlink(p->file.c_str());
        }
    }

    FILE* str = createFileOutputStream(p->file);
    if (!str) {
        mcr->setDiskWriteResult(false, false);
        return;
    }
    const std::string doc = writeMapXml(mcr->mapToWriteToDisk);
    const bool wrote = fwrite(doc.data(), 1, doc.size(), str) == doc.size();
    fflush(str);
    fsync(fileno(str));   // FileUtils.sync — durability is the point on power-cut targets
    fclose(str);
    // ContextImpl.setFilePermissionsFromMode: MODE_PRIVATE => owner-only.
    chmod(p->file.c_str(), p->mode == 0 ? (mode_t)0600 : (mode_t)0644);

    if (!wrote) {
        // Clean up an unsuccessfully written file
        if (::unlink(p->file.c_str()) != 0) {
            LOGE("Couldn't clean up partially-written file %s", p->file.c_str());
        }
        mcr->setDiskWriteResult(false, false);
        return;
    }

    if (::stat(p->file.c_str(), &st) == 0) {
        std::lock_guard<std::mutex> lk(p->mLock);
        p->mStatTimestamp = st.st_mtim;
        p->mStatSize = st.st_size;
    }

    // Writing was successful, delete the backup file if there is one.
    ::unlink(p->backupFile.c_str());

    p->mDiskStateGeneration = mcr->memoryStateGeneration;
    mcr->setDiskWriteResult(true, true);
}

/*============================================================================*
 * EditorImpl
 *============================================================================*/

class SharedPreferencesImpl::EditorImpl : public SharedPreferences::Editor {
public:
    EditorImpl(SharedPreferencesImpl& parent, std::shared_ptr<Private> p)
        : mParent(parent), mP(std::move(p)) {}

    Editor& putString(const std::string& key, const std::string& value) override {
        std::lock_guard<std::mutex> lk(mEditorLock);
        mModified[key] = PrefValue::of(value);
        return *this;
    }

    Editor& putStringSet(const std::string& key, const std::set<std::string>& values) override {
        // by-value parameter = AOSP's defensive new HashSet<>(values)
        std::lock_guard<std::mutex> lk(mEditorLock);
        mModified[key] = PrefValue::of(values);
        return *this;
    }

    Editor& putInt(const std::string& key, int value) override {
        std::lock_guard<std::mutex> lk(mEditorLock);
        mModified[key] = PrefValue::ofInt(value);
        return *this;
    }

    Editor& putLong(const std::string& key, int64_t value) override {
        std::lock_guard<std::mutex> lk(mEditorLock);
        mModified[key] = PrefValue::ofLong(value);
        return *this;
    }

    Editor& putFloat(const std::string& key, float value) override {
        std::lock_guard<std::mutex> lk(mEditorLock);
        mModified[key] = PrefValue::ofFloat(value);
        return *this;
    }

    Editor& putBoolean(const std::string& key, bool value) override {
        std::lock_guard<std::mutex> lk(mEditorLock);
        mModified[key] = PrefValue::ofBoolean(value);
        return *this;
    }

    Editor& remove(const std::string& key) override {
        // "this" is the magic value for a removal mutation; Null plays it.
        std::lock_guard<std::mutex> lk(mEditorLock);
        mModified[key] = PrefValue::nullValue();
        return *this;
    }

    Editor& clear() override {
        std::lock_guard<std::mutex> lk(mEditorLock);
        mClear = true;
        return *this;
    }

    bool commit() override {
        std::shared_ptr<MemoryCommitResult> mcr = commitToMemory();
        enqueueDiskWrite(mcr, Runnable() /* sync write on this thread okay */);
        mcr->awaitWrittenToDisk();
        notifyListeners(mcr);
        return mcr->writeToDiskResult;
    }

    void apply() override {
        std::shared_ptr<MemoryCommitResult> mcr = commitToMemory();

        Runnable awaitCommit = [mcr] { mcr->awaitWrittenToDisk(); };
        QueuedWork::addFinisher(awaitCommit);

        Runnable postWriteRunnable = [awaitCommit]() mutable {
            awaitCommit();
            QueuedWork::removeFinisher(awaitCommit);
        };

        enqueueDiskWrite(mcr, postWriteRunnable);

        // Okay to notify the listeners before it's hit disk because the
        // listeners should always get the same SharedPreferences instance
        // back, which has the changes reflected in memory.
        notifyListeners(mcr);
    }

private:
    // Returns the MemoryCommitResult of the changes applied to memory.
    std::shared_ptr<MemoryCommitResult> commitToMemory() {
        int64_t memoryStateGeneration = 0;
        bool keysCleared = false;
        std::vector<std::string> keysModified;
        std::vector<SharedPreferences::OnSharedPreferenceChangeListener> listeners;
        PrefMap mapToWriteToDisk;

        std::lock_guard<std::mutex> lk(mP->mLock);
        // We optimistically don't make a deep copy until a memory commit
        // comes in while we're already writing to disk: AOSP then clones mMap
        // so the in-flight writer keeps the old object; handing the writer a
        // by-value snapshot taken after the mutations gives the same
        // guarantees (later commits never touch it).
        mP->mDiskWritesInFlight++;
        const bool hasListeners = !mP->mListeners.empty();
        if (hasListeners) listeners = mP->mListeners;   // AOSP snapshots the key set

        bool changesMade = false;
        {
            std::lock_guard<std::mutex> el(mEditorLock);   // mLock → mEditorLock (AOSP order)
            if (mClear) {
                if (!mP->mMap.empty()) {
                    changesMade = true;
                    mP->mMap.clear();
                }
                keysCleared = true;
                mClear = false;
            }

            for (const auto& e : mModified) {
                const std::string& k = e.first;
                const PrefValue& v = e.second;
                if (v.type == PrefValue::Null) {
                    // removal mutation; also the putString(key, null) role
                    if (mP->mMap.find(k) == mP->mMap.end()) continue;
                    mP->mMap.erase(k);
                } else {
                    auto it = mP->mMap.find(k);
                    if (it != mP->mMap.end() && it->second == v) continue;   // unchanged
                    mP->mMap[k] = v;
                }
                changesMade = true;
                if (hasListeners) keysModified.push_back(k);
            }

            mModified.clear();

            if (changesMade) mP->mCurrentMemoryStateGeneration++;
            memoryStateGeneration = mP->mCurrentMemoryStateGeneration;

            mapToWriteToDisk = mP->mMap;   // the writer's frozen snapshot
        }
        return std::make_shared<MemoryCommitResult>(memoryStateGeneration, keysCleared,
                std::move(keysModified), std::move(listeners), std::move(mapToWriteToDisk));
    }

    // Enqueue an already-committed-to-memory result to be written to disk.
    // They will be written to disk one-at-a-time in the order that they're
    // enqueued. A null postWriteRunnable means commit(): the disk write may
    // run on the calling thread.
    void enqueueDiskWrite(const std::shared_ptr<MemoryCommitResult>& mcr,
            Runnable postWriteRunnable) {
        const bool isFromSyncCommit = !postWriteRunnable;

        Runnable writeToDiskRunnable = [p = mP, mcr, postWriteRunnable, isFromSyncCommit]() mutable {
            {
                std::lock_guard<std::mutex> lk(p->mWritingToDiskLock);
                writeToFile(p.get(), mcr.get(), isFromSyncCommit);
            }
            {
                std::lock_guard<std::mutex> lk(p->mLock);
                p->mDiskWritesInFlight--;
            }
            if (postWriteRunnable) postWriteRunnable();
        };

        // Typical #commit() path with fewer allocations, doing a write on
        // the current thread.
        if (isFromSyncCommit) {
            bool wasEmpty = false;
            {
                std::lock_guard<std::mutex> lk(mP->mLock);
                wasEmpty = mP->mDiskWritesInFlight == 1;
            }
            if (wasEmpty) {
                writeToDiskRunnable();
                return;
            }
        }

        QueuedWork::queue(writeToDiskRunnable, !isFromSyncCommit);
    }

    void notifyListeners(const std::shared_ptr<MemoryCommitResult>& mcr) {
        if (mcr->listeners.empty() || (mcr->keysModified.empty() && !mcr->keysCleared)) return;
        // AOSP reposts to the main thread when this runs off it; CDROID's
        // edit()/commit()/apply() callers are the UI thread in practice (the
        // async disk write never notifies), so listeners fire synchronously
        // on the editing thread — the thread AOSP uses for the common case.
        if (mcr->keysCleared) {
            // AOSP (R+, CALLBACK_ON_CLEAR_CHANGE) notifies a null key on
            // clear; the listener face here has no null, "" plays it.
            for (const auto& l : mcr->listeners) {
                if (l) l(mParent, "");
            }
        }
        for (size_t i = mcr->keysModified.size(); i-- > 0; ) {
            const std::string& key = mcr->keysModified[i];
            for (const auto& l : mcr->listeners) {
                if (l) l(mParent, key);
            }
        }
    }

    SharedPreferencesImpl& mParent;
    std::shared_ptr<Private> mP;
    std::mutex mEditorLock;   // guards mModified/mClear (AOSP mEditorLock)
    std::map<std::string, PrefValue> mModified;
    bool mClear = false;
};

/*============================================================================*
 * SharedPreferencesImpl
 *============================================================================*/

SharedPreferencesImpl::SharedPreferencesImpl(const std::string& name, int mode)
    : mP(std::make_shared<Private>()) {
    mP->file = prefsDirectory() + "/" + name + ".xml";
    mP->backupFile = mP->file + ".bak";   // makeBackupFile
    mP->mode = mode;
    if (mode != 0) {   // Context::MODE_PRIVATE
        LOGW("SharedPreferences mode %d unsupported (only MODE_PRIVATE); ignored", mode);
    }
    startLoadFromDisk(mP);
    mEditor = std::unique_ptr<Editor>(new EditorImpl(*this, mP));
}

SharedPreferencesImpl::~SharedPreferencesImpl() {
    // AOSP has no finalizer write: in-flight writes live on the executor and
    // QueuedWork; App::exit's waitToFinish is the checkpoint that flushes
    // them.
}

std::vector<std::pair<std::string, std::string>> SharedPreferencesImpl::getAll() {
    std::unique_lock<std::mutex> lk(mP->mLock);
    awaitLoadedLocked(mP.get(), lk);
    // The interface face maps AOSP's Map<String, ?> onto string pairs
    // (values stringified; sets join with '\n', the legacy shape).
    std::vector<std::pair<std::string, std::string>> all;
    all.reserve(mP->mMap.size());
    for (const auto& e : mP->mMap) {
        std::string v;
        switch (e.second.type) {
            case PrefValue::String: v = e.second.str; break;
            case PrefValue::Int:
            case PrefValue::Long:   v = std::to_string(e.second.i); break;
            case PrefValue::Float:  v = javaFloatString(e.second.f); break;
            case PrefValue::Boolean: v = e.second.i ? "true" : "false"; break;
            case PrefValue::StringSet:
                for (const auto& s : e.second.set) {
                    if (!v.empty()) v += '\n';
                    v += s;
                }
                break;
            case PrefValue::Null: break;
        }
        all.emplace_back(e.first, std::move(v));
    }
    return all;
}

std::string SharedPreferencesImpl::getString(const std::string& key, const std::string& defValue) {
    std::unique_lock<std::mutex> lk(mP->mLock);
    awaitLoadedLocked(mP.get(), lk);
    const auto it = mP->mMap.find(key);
    if (it == mP->mMap.end()) return defValue;
    // AOSP: (String) cast — ClassCastException on a cross-type read. CDROID
    // seam: warn and return the default (the library is not exception-safe).
    if (it->second.type != PrefValue::String) {
        LOGW("SharedPreferences key '%s' is not a String", key.c_str());
        return defValue;
    }
    return it->second.str;
}

std::set<std::string> SharedPreferencesImpl::getStringSet(const std::string& key,
        const std::set<std::string>& defValues) {
    std::unique_lock<std::mutex> lk(mP->mLock);
    awaitLoadedLocked(mP.get(), lk);
    const auto it = mP->mMap.find(key);
    if (it == mP->mMap.end()) return defValues;
    if (it->second.type != PrefValue::StringSet) {
        LOGW("SharedPreferences key '%s' is not a StringSet", key.c_str());
        return defValues;
    }
    return it->second.set;   // returned by value: a copy, as the docs demand
}

int SharedPreferencesImpl::getInt(const std::string& key, int defValue) {
    std::unique_lock<std::mutex> lk(mP->mLock);
    awaitLoadedLocked(mP.get(), lk);
    const auto it = mP->mMap.find(key);
    if (it == mP->mMap.end()) return defValue;
    if (it->second.type != PrefValue::Int) {
        LOGW("SharedPreferences key '%s' is not an Int", key.c_str());
        return defValue;
    }
    return (int)it->second.i;
}

int64_t SharedPreferencesImpl::getLong(const std::string& key, int64_t defValue) {
    std::unique_lock<std::mutex> lk(mP->mLock);
    awaitLoadedLocked(mP.get(), lk);
    const auto it = mP->mMap.find(key);
    if (it == mP->mMap.end()) return defValue;
    if (it->second.type != PrefValue::Long) {
        LOGW("SharedPreferences key '%s' is not a Long", key.c_str());
        return defValue;
    }
    return it->second.i;
}

float SharedPreferencesImpl::getFloat(const std::string& key, float defValue) {
    std::unique_lock<std::mutex> lk(mP->mLock);
    awaitLoadedLocked(mP.get(), lk);
    const auto it = mP->mMap.find(key);
    if (it == mP->mMap.end()) return defValue;
    if (it->second.type != PrefValue::Float) {
        LOGW("SharedPreferences key '%s' is not a Float", key.c_str());
        return defValue;
    }
    return it->second.f;
}

bool SharedPreferencesImpl::getBoolean(const std::string& key, bool defValue) {
    std::unique_lock<std::mutex> lk(mP->mLock);
    awaitLoadedLocked(mP.get(), lk);
    const auto it = mP->mMap.find(key);
    if (it == mP->mMap.end()) return defValue;
    if (it->second.type != PrefValue::Boolean) {
        LOGW("SharedPreferences key '%s' is not a Boolean", key.c_str());
        return defValue;
    }
    return it->second.i != 0;
}

bool SharedPreferencesImpl::contains(const std::string& key) {
    std::unique_lock<std::mutex> lk(mP->mLock);
    awaitLoadedLocked(mP.get(), lk);
    return mP->mMap.find(key) != mP->mMap.end();
}

SharedPreferences::Editor& SharedPreferencesImpl::edit() {
    {
        std::unique_lock<std::mutex> lk(mP->mLock);
        awaitLoadedLocked(mP.get(), lk);
    }
    return *mEditor;
}

void SharedPreferencesImpl::registerOnSharedPreferenceChangeListener(
        const OnSharedPreferenceChangeListener& listener) {
    std::lock_guard<std::mutex> lk(mP->mLock);
    // AOSP uses a WeakHashMap (set semantics): re-registering an identical
    // listener keeps one entry.
    for (const auto& l : mP->mListeners) {
        if (l && listener
                && l.template target<void(SharedPreferences&, const std::string&)>() ==
                   listener.template target<void(SharedPreferences&, const std::string&)>()) {
            return;
        }
    }
    mP->mListeners.push_back(listener);
}

void SharedPreferencesImpl::unregisterOnSharedPreferenceChangeListener(
        const OnSharedPreferenceChangeListener& listener) {
    std::lock_guard<std::mutex> lk(mP->mLock);
    for (auto it = mP->mListeners.begin(); it != mP->mListeners.end(); ++it) {
        // std::function target identity: erase the first callable registered
        // with the same target address (the WeakHashMap#remove role).
        if (*it && listener
                && it->template target<void(SharedPreferences&, const std::string&)>() ==
                   listener.template target<void(SharedPreferences&, const std::string&)>()) {
            mP->mListeners.erase(it);
            return;
        }
    }
}

} // namespace cdroid
