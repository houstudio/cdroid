/* SharedPreferences (AOSP SharedPreferencesImpl port) — typed XML store.
 *
 * Coverage: type fidelity across reload, the shared_prefs XML byte format,
 * corrupt-file and .bak recovery semantics, commit/apply write paths,
 * change listeners, and the Context per-name instance cache.
 *
 * Every case gets its own $HOME (mkdtemp) and prefs name, so the on-disk
 * store of one case can never leak into another (prefsDirectory() reads
 * $HOME per call). */
#include <gtest/gtest.h>
#include <content/sharedpreferences.h>
#include <core/queuedwork.h>
#include <core/app.h>
#include <core/context.h>
#include <guienvironment.h>

#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <map>
#include <string>

using namespace cdroid;

class SHARED_PREFS: public testing::Test {
protected:
    std::string mHome;
    std::string mOrigHome;
    int mSeq = 0;

    void SetUp() override {
        char tmpl[] = "/tmp/sprefs_test_XXXXXX";
        char* dir = mkdtemp(tmpl);
        ASSERT_NE(dir, nullptr);
        mHome = dir;
        const char* orig = getenv("HOME");
        mOrigHome = orig ? orig : "";
        setenv("HOME", mHome.c_str(), 1);
        static int seq = 0;
        mSeq = ++seq;
    }

    void TearDown() override {
        setenv("HOME", mOrigHome.c_str(), 1);
    }

    // Unique prefs name per case (fresh file per case).
    std::string name() { return "prefs" + std::to_string(mSeq); }

    std::string filePath(const std::string& prefsName) {
        return mHome + "/.cdroid/prefs/" + prefsName + ".xml";
    }

    static std::string readFile(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
    }
};

/* Type fidelity: values keep their type on disk, so a cross-type read
 * returns the default (AOSP: ClassCastException; CDROID seam: default). */
TEST_F(SHARED_PREFS, TypedRoundTrip) {
    {
        SharedPreferencesImpl sp(name(), 0);
        sp.edit()
            .putString("s", "hello")
            .putInt("i", -42)
            .putLong("l", 9007199254740993LL)
            .putFloat("f", 1.5f)
            .putBoolean("b", true)
            .commit();
    }
    SharedPreferencesImpl sp(name(), 0);   // fresh instance: reload from disk
    EXPECT_EQ(sp.getString("s", ""), "hello");
    EXPECT_EQ(sp.getInt("i", 0), -42);
    EXPECT_EQ(sp.getLong("l", 0), 9007199254740993LL);
    EXPECT_FLOAT_EQ(sp.getFloat("f", 0.f), 1.5f);
    EXPECT_TRUE(sp.getBoolean("b", false));
    EXPECT_TRUE(sp.contains("s"));

    // Cross-type reads return the default, not a coerced value.
    EXPECT_EQ(sp.getInt("s", 7), 7);
    EXPECT_EQ(sp.getString("i", "d"), "d");
    EXPECT_FLOAT_EQ(sp.getFloat("i", 0.25f), 0.25f);
    EXPECT_FALSE(sp.getBoolean("f", false));
}

/* getStringSet survives values that the old INI backend's '\n' join
 * destroyed, plus XML-special characters. */
TEST_F(SHARED_PREFS, StringSetRoundTrip) {
    std::set<std::string> in = {"one", "two\nlines", "a<b&c>d\"e'f", "trailing ", " leading"};
    {
        SharedPreferencesImpl sp(name(), 0);
        sp.edit().putStringSet("set", in).commit();
    }
    SharedPreferencesImpl sp(name(), 0);
    const std::set<std::string> out = sp.getStringSet("set", {});
    EXPECT_EQ(out, in);

    // XML-special strings too.
    {
        SharedPreferencesImpl sp2(name(), 0);
        sp2.edit().putString("xml", "<tag attr=\"x\">&amp;'</tag>\n\tsecond").commit();
    }
    SharedPreferencesImpl sp2(name(), 0);
    EXPECT_EQ(sp2.getString("xml", ""), "<tag attr=\"x\">&amp;'</tag>\n\tsecond");
}

/* The file on disk is AOSP's shared_prefs XML (XmlUtils.writeMapXml through
 * FastXmlSerializer: header, 4-space indent, value= attributes, escaping). */
TEST_F(SHARED_PREFS, DiskFormatIsAospXml) {
    {
        SharedPreferencesImpl sp(name(), 0);
        sp.edit()
            .putInt("count", 42)
            .putBoolean("flag", true)
            .putFloat("ratio", 1.0f)
            .putString("esc", "a<b&c>d\"e'f\n")
            .putStringSet("subs", {"x", "y"})
            .commit();
    }
    const std::string xml = readFile(filePath(name()));
    EXPECT_NE(xml.find("<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n<map>\n"),
            std::string::npos);
    EXPECT_NE(xml.find("    <int name=\"count\" value=\"42\" />\n"), std::string::npos);
    EXPECT_NE(xml.find("    <boolean name=\"flag\" value=\"true\" />\n"), std::string::npos);
    EXPECT_NE(xml.find("    <float name=\"ratio\" value=\"1.0\" />\n"), std::string::npos);   // Java Float.toString shape
    EXPECT_NE(xml.find("    <string name=\"esc\">a&lt;b&amp;c&gt;d&quot;e'f&#10;</string>\n"),
            std::string::npos);
    EXPECT_NE(xml.find("    <set name=\"subs\">\n        <string>x</string>\n"
                       "        <string>y</string>\n    </set>\n"), std::string::npos);
    EXPECT_NE(xml.find("</map>\n"), std::string::npos);
}

/* A corrupt/foreign file means an empty store (AOSP: failed read → empty
 * map), never a crash and never stale partial data. */
TEST_F(SHARED_PREFS, CorruptFileStartsEmpty) {
    const std::string path = filePath(name());
    mkdir((mHome + "/.cdroid").c_str(), 0755);
    mkdir((mHome + "/.cdroid/prefs").c_str(), 0755);
    { std::ofstream f(path, std::ios::trunc); f << "this is not xml at all [section]\nkey=1\n"; }
    SharedPreferencesImpl sp(name(), 0);
    EXPECT_TRUE(sp.getAll().empty());
    EXPECT_FALSE(sp.contains("key"));

    // A well-formed file with a foreign value tag is rejected wholesale.
    { std::ofstream f(path, std::ios::trunc);
      f << "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n<map>\n"
        << "    <double name=\"x\" value=\"1.5\" />\n</map>\n"; }
    SharedPreferencesImpl sp2(name(), 0);
    EXPECT_TRUE(sp2.getAll().empty());
}

/* writeToFile's backup protocol: a leftover .bak (the last write failed
 * mid-flight) is restored over the partial file on load. */
TEST_F(SHARED_PREFS, BackupRestoredOnLoad) {
    const std::string path = filePath(name());
    {
        SharedPreferencesImpl sp(name(), 0);
        sp.edit().putString("k", "good").commit();
    }
    // Simulate the crash window: rename the good file to .bak, leave a
    // partial main file behind.
    rename(path.c_str(), (path + ".bak").c_str());
    { std::ofstream f(path, std::ios::trunc); f << "<?xml version='1.0' encoding='ut"; }

    SharedPreferencesImpl sp(name(), 0);
    EXPECT_EQ(sp.getString("k", ""), "good");
    // The backup was consumed by the restore.
    EXPECT_NE(access((path + ".bak").c_str(), F_OK), 0);
}

/* commit() is synchronous: it returns only after the file is on disk. */
TEST_F(SHARED_PREFS, CommitIsSynchronous) {
    SharedPreferencesImpl sp(name(), 0);
    EXPECT_TRUE(sp.edit().putString("k", "v").commit());
    EXPECT_EQ(access(filePath(name()).c_str(), F_OK), 0);
    EXPECT_EQ(sp.getString("k", ""), "v");
}

/* apply() makes the change visible in memory immediately and queues the
 * disk write; QueuedWork::waitToFinish (the exit checkpoint) flushes it. */
TEST_F(SHARED_PREFS, ApplyFlushesThroughQueuedWork) {
    SharedPreferencesImpl sp(name(), 0);
    sp.edit().putString("k1", "v1").apply();
    sp.edit().putString("k2", "v2").apply();
    // Memory state is already visible to readers.
    EXPECT_EQ(sp.getString("k1", ""), "v1");
    EXPECT_EQ(sp.getString("k2", ""), "v2");
    QueuedWork::waitToFinish();
    const std::string xml = readFile(filePath(name()));
    EXPECT_NE(xml.find("v1"), std::string::npos);
    EXPECT_NE(xml.find("v2"), std::string::npos);

    // And a fresh instance sees both keys — the queued writes landed.
    SharedPreferencesImpl sp2(name(), 0);
    EXPECT_EQ(sp2.getString("k1", ""), "v1");
    EXPECT_EQ(sp2.getString("k2", ""), "v2");
}

/* Change notification: commit/apply fire listeners per modified key (in
 * reverse order, AOSP EditorImpl.notifyListeners), clear fires the "" key
 * (the C++ stand-in for AOSP's null), and unregister stops delivery. */
TEST_F(SHARED_PREFS, ChangeListeners) {
    SharedPreferencesImpl sp(name(), 0);
    std::vector<std::string> seen;
    SharedPreferences::OnSharedPreferenceChangeListener listener =
            [&seen](SharedPreferences&, const std::string& key) { seen.push_back(key); };
    sp.registerOnSharedPreferenceChangeListener(listener);

    sp.edit().putString("a", "1").putString("b", "2").commit();
    ASSERT_EQ(seen.size(), 2u);
    EXPECT_EQ(seen[0], "b");   // reverse insertion order
    EXPECT_EQ(seen[1], "a");

    // An unchanged value is not a change (AOSP's equals short-circuit).
    seen.clear();
    sp.edit().putString("a", "1").commit();
    EXPECT_TRUE(seen.empty());

    // clear() notifies the "" key (null on AOSP R+).
    seen.clear();
    sp.edit().clear().commit();
    ASSERT_EQ(seen.size(), 1u);
    EXPECT_EQ(seen[0], "");
    EXPECT_TRUE(sp.getAll().empty());

    sp.unregisterOnSharedPreferenceChangeListener(listener);
    seen.clear();
    sp.edit().putString("c", "3").commit();
    EXPECT_TRUE(seen.empty());
}

/* remove() deletes the key (a Null mutation in the batch, AOSP's
 * this-marker); a remove of a missing key is not a change. */
TEST_F(SHARED_PREFS, RemoveKey) {
    SharedPreferencesImpl sp(name(), 0);
    sp.edit().putString("k", "v").commit();
    EXPECT_TRUE(sp.contains("k"));
    sp.edit().remove("k").commit();
    EXPECT_FALSE(sp.contains("k"));

    SharedPreferencesImpl sp2(name(), 0);   // persisted
    EXPECT_FALSE(sp2.contains("k"));
    EXPECT_EQ(sp2.getString("k", "def"), "def");
}

/* Context::getSharedPreferences: one instance per name for the process
 * (AOSP ContextImpl cache) — same shared_ptr identity, live writes shared.
 * The old INI-backed store could not offer this contract plus change
 * notification; this is the alignment payoff. */
TEST_F(SHARED_PREFS, ContextReturnsOneInstancePerName) {
    Context& ctx = App::getInstance();
    auto a = ctx.getSharedPreferences(name(), Context::MODE_PRIVATE);
    auto b = ctx.getSharedPreferences(name(), Context::MODE_PRIVATE);
    EXPECT_EQ(a.get(), b.get());
    a->edit().putInt("shared", 7).commit();
    EXPECT_EQ(b->getInt("shared", 0), 7);
}

/* getAll() keeps its string-pair face (values stringified), the interface
 * contract apps already code against. */
TEST_F(SHARED_PREFS, GetAllStringifies) {
    SharedPreferencesImpl sp(name(), 0);
    sp.edit().putInt("n", 42).putBoolean("t", false).putString("s", "str").commit();
    std::map<std::string, std::string> all;
    for (auto& kv : sp.getAll()) all[kv.first] = kv.second;
    EXPECT_EQ(all["n"], "42");
    EXPECT_EQ(all["t"], "false");
    EXPECT_EQ(all["s"], "str");
    EXPECT_EQ(all.size(), 3u);
}

/* An empty store writes a valid empty <map> document (AOSP shape). */
TEST_F(SHARED_PREFS, EmptyMapDocument) {
    SharedPreferencesImpl sp(name(), 0);
    EXPECT_TRUE(sp.edit().putString("k", "v").commit());
    sp.edit().clear().commit();
    const std::string xml = readFile(filePath(name()));
    EXPECT_EQ(xml, "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n<map>\n</map>\n");
}

/* QueuedWork's daemon lifecycle: quitSafely parks the queued-work
 * HandlerThread; the next queue() lazily creates a fresh one (the AOSP
 * resetHandler shape) and apply() keeps working across the restart. */
TEST_F(SHARED_PREFS, QueuedWorkRestartsAfterQuit) {
    {
        SharedPreferencesImpl sp(name(), 0);
        sp.edit().putString("k", "before").apply();
        QueuedWork::waitToFinish();
    }
    QueuedWork::quitSafely();

    {
        SharedPreferencesImpl sp(name(), 0);
        EXPECT_EQ(sp.getString("k", ""), "before");   // first round landed
        sp.edit().putString("k", "after").apply();
        QueuedWork::waitToFinish();
    }
    SharedPreferencesImpl sp(name(), 0);
    EXPECT_EQ(sp.getString("k", ""), "after");        // second round landed too
}
