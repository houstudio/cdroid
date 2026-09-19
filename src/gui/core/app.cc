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
#include <sys/stat.h>
#include <core/app.h>
#include <transition/transition.h>  // App-exit clone sweep (deleteOrphanedClones)
#include <core/queuedwork.h>   // exit-path flush of async writes
#include <content/typedarray.h>   // TypedArray (constructed in obtainStyledAttributes)
#include <content/typedvalue.h>   // TypedValue (typed currency of this layer)
#include <content/androidfw/assetmanager2.h> // AM2 engine + cdroid::Theme (boundary lookups)
#include <content/assetmanager.h>   // AssetManager
#include <content/asset.h>          // Asset (openAsset face: getBuffer/getLength)
#include <content/resources.h> // cdroid::Resources
#include <algorithm>
#include <cdtypes.h>
#include <cdlog.h>
#include <zip.h>
#include <iostreams.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <text/textutils.h>
#include <core/systemclock.h>
#include <drawable/drawables.h>
#include <drawable/drawableinflater.h>
#include <image-decoders/imagedecoder.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <thread>
#include <mutex>
#include <porting/cdlog.h>
#include <porting/cdgraph.h>
#include <core/app.h>
#include <content/LocaleList.h>
#include <widget/framework_styleable.h>
#include <core/xmlpullparser.h>
#include <core/build.h>
#include <core/messagequeue.h>
#include <core/intent.h>
#include <core/activityfactory.h>
#include <widget/activityoptions.h>
#include <widget/cdwindow.h>
#include <widget/internal_R.h>
#include <gui_features.h>
#include <cstdio>
#include <iterator>
#include <content/i18n/data_resource.h>
#include <core/cxxopts.h>
#include <core/inputeventsource.h>
#include <core/windowmanager.h>
#include <app/autotest.h>
#include <core/inputmethodmanager.h>
#include <widget/internal_R.h>

#if defined(__linux__)||defined(__unix__)
#include <sys/auxv.h>
extern "C" char *__progname;
#define PATH_SEP '/'
#elif defined(_WIN32)||defined(_WIN64)
#define PATH_SEP '\\'
extern "C" unsigned long  GetModuleFileNameA(void* hModule, char* lpFilename, unsigned long nSize);
#endif

namespace cdroid{

namespace {
// App-option registrations collected at static-init time (App::addAppOptions,
// DECLARE_WIDGET-style) and replayed into App's cxxopts table in the ctor,
// before parse — so --help lists the application's options beside the
// framework's. Function-local static: no static-init-order dependency on
// whichever TU registers first.
struct AppOptionEntry {
    std::string group;
    std::function<void(cxxopts::OptionAdder&)> adder;
};
std::vector<AppOptionEntry>& appOptionRegistry() {
    static std::vector<AppOptionEntry> registry;
    return registry;
}
} // namespace

bool App::addAppOptions(const std::string& group,
        const std::function<void(cxxopts::OptionAdder&)>& adder) {
    if (!adder) return false;
    appOptionRegistry().push_back({group, adder});
    return true;
}

App::App(int argc,const char*argv[]):mQuitFlag(false),mExitCode(0){
    int alpha = 255, rotation = 0, density = 0, frameDelay = 0;
    bool debug= false,showFPS = false, help = false;
    std::string autoTest, autoTestRecord, testScript, orientation, inputMode;
    std::string logo, datapath;
    LogParseModules(argc,argv);
    mInst = this;
    cxxopts::Options options("cdroid","cdroid application");
    options.add_options()
        ("d,debug","enable debug mode",cxxopts::value<bool>(debug))
        ("h,help","print this help, then exit",cxxopts::value<bool>(help))
        ("fps", "show fps info",cxxopts::value<bool>(showFPS))
        ("a,alpha","UI layer global alpha[0,255]",cxxopts::value<int>(alpha)->default_value("255"))
        ("f,framedelay","animation frame delay",cxxopts::value<int>(frameDelay))
        ("density","UI Density",cxxopts::value<int>(density))
        ("orientation","force resource orientation (land|landscape|port|portrait; "
         "default: launcher manifest's screenOrientation > screen shape)",
         cxxopts::value<std::string>(orientation))
        ("R,rotate","display rotate(90*n)",cxxopts::value<int>(rotation)->default_value("0"))
        ("l,logo","show logo",cxxopts::value<std::string>(logo))
        ("data","data directory",cxxopts::value<std::string>(datapath))
        ("auto-test","a11y semantic UI sweep (clicks every on-screen clickable and verifies "
         "events); bare = deterministic per-page traversal, =SEED = monkey-style random walk",
         cxxopts::value<std::string>(autoTest)->implicit_value("1"))
        ("auto-test-record","record operations into a replayable --test-script: manual "
         "touches/keys always (click/long-click/drag/tap-by-coordinate/key/back); "
         "combined with --auto-test the sweep's steps join the same script",
         cxxopts::value<std::string>(autoTestRecord))
        ("test-script","line-based a11y test script (wait/click/assert/dump; exit code = failures)",
         cxxopts::value<std::string>(testScript))
        ("input-mode","input reader mode: thread (dedicated reader thread, default) | "
         "choreographer (per-frame CALLBACK_INPUT polling, no reader thread)",
         cxxopts::value<std::string>(inputMode)->default_value("thread"));

    Looper::prepareMainLooper();
    options.allow_unrecognised_options();
#if defined(__linux__)||defined(__unix__)
    //mName = std::string(argc?argv[0]:__progname);
    mName = (const char*)getauxval(AT_EXECFN);
#elif (defined(_WIN32)||defined(_WIN64))
    char progName[260];
    GetModuleFileNameA(nullptr,progName,sizeof(progName));
    mName = progName;
#endif
    // Replay the application's registered options (App::addAppOptions) into
    // the same table before parsing, so --help shows them too.
    for (const auto& entry : appOptionRegistry()) {
        try {
            // add_options returns the OptionAdder by value; hold it in an
            // lvalue so the registered callback can take it by reference.
            cxxopts::OptionAdder adder = options.add_options(entry.group);
            entry.adder(adder);
        } catch (const std::exception& e) {
            LOGE("app option group '%s': %s", entry.group.c_str(), e.what());
        }
    }
    try{
        if((argc == 0) || (argv == nullptr)){
            const char*dummy[] = {mName.c_str(), nullptr};
            mArgsResult = std::make_unique<cxxopts::ParseResult>(std::move(options.parse(1,dummy)));
        }else{
            mArgsResult = std::make_unique<cxxopts::ParseResult>(std::move(options.parse(argc,argv)));
        }
    }catch(std::exception&e){
        LOGE("%s",e.what());
    }
    if(help){
        // std::exit, qualified: a bare exit(int) here resolves to App::exit
        // (post the quit sentinel), which does NOT terminate — the ctor would
        // return early with fonts/resources uninitialized and main would keep
        // running into a null-Typeface crash. The code path below was written
        // as if unreachable; with a real exit it actually is, so it is gone.
        std::cout<<options.help()<<std::endl;
        std::exit(EXIT_SUCCESS);
    }
    Typeface::setContext(this);
    onInit();
    std::string appPakPath;
    const size_t pos = mName.rfind(PATH_SEP);
    if(pos!=std::string::npos){
        const std::string name = mName.substr(pos+1);
        /* The app's own pak lives beside the executable (getDataPath, overridable
         * with --data). No cwd probing: launching from a different directory
         * must not silently pick up some other directory's pak. */
        std::string pakPath =getDataPath()+name+std::string(".pak");
        addResource(pakPath,getName());
        appPakPath = pakPath;
    }
    // AOSP: the application theme comes from the manifest (android:theme) and
    // falls back to the platform default; applyStyle follows the style's parent
    // chain. Every pak is loaded before the theme so the manifest parse and the
    // theme's resource references both resolve.
    if (!appPakPath.empty()) parsePackageManifest(appPakPath);
    // Orientation must land before the first inflate (and ideally before the
    // theme build below) so -land/-port variants resolve on first lookup.
    applyOrientationConfig(orientation);
    setTheme(mApplicationTheme ? mApplicationTheme
                               : (int)cdroid::internal::R::style::Theme_Material);
    // AOSP: the system starts the manifest's launcher activity — app main()
    // never does. CDROID's App plays that side on the message queue (like
    // ActivityThread: bindApplication/launchActivity are messages): a posted
    // runnable fires once exec()'s loop is turning, after anything main() set
    // up synchronously. Skipped when a window is already up or the manifest
    // declares no activity (app-driven windows keep working as before).
    // --auto-test / --auto-test-record / --test-script: the App-level semantic
    // UI driver — any app gets a coordinate-free smoke test over the a11y node
    // tree, and the manual-operation recorder.
    if (!testScript.empty()) {
        static Handler sAutoTestHandler(Looper::getMainLooper());
        sAutoTestHandler.postDelayed([scriptPath = testScript]() {
            UiAutoTest::getInstance().runScript(scriptPath);
        }, 3000);  // let the launcher window come up first
    } else {
        // --auto-test-record=<file> opens the SHARED recording sink and hooks
        // the WindowManager input observer IMMEDIATELY (manual recording must
        // catch the very first click, not the ones after a 3s delay). Alone
        // it records manual operations only; combined with --auto-test the
        // sweep's steps join the same script in chronological order.
        if (!autoTestRecord.empty()) {
            UiAutoTest::getInstance().setScriptRecorder(autoTestRecord);
            WindowManager::setInputEventObserver([](const InputEvent& e) {
                UiAutoTest::getInstance().observeInput(e);
            });
        }
        if (!autoTest.empty()) {
            // --auto-test[=SEED]: seed >= 0 selects the Monkey-style seeded-
            // random walk, anything else the deterministic per-page traversal
            // (see UiAutoTest::start).
            long autoSeed = -1;
            const std::string seedSrc = (autoTest != "1" && autoTest != "true")
                    ? autoTest : "";
            if (!seedSrc.empty()) {
                char* end = nullptr;
                const long s = strtol(seedSrc.c_str(), &end, 10);
                if (end != nullptr && *end == '\0' && s >= 0) autoSeed = s;
                else LOGW("--auto-test=%s: not a non-negative seed, using deterministic sweep",
                          seedSrc.c_str());
            }
            static Handler sAutoTestHandler(Looper::getMainLooper());
            sAutoTestHandler.postDelayed([autoSeed]() {
                UiAutoTest::getInstance().start(2500, autoSeed);
            }, 3000);  // let the launcher window come up first
        }
    }
    static Handler sLaunchHandler(Looper::getMainLooper());
    sLaunchHandler.post([this](){
        std::vector<Window*> windows;
        WindowManager::getInstance().getWindows(windows);
        if (!windows.empty()) return;
        const std::string launcher = getLauncherActivity();
        if (launcher.empty()) return;
        Intent intent("");
        intent.setComponent(ComponentName("", launcher));
        startActivity(intent);
    });
    LOGI("\033[1;35m          ┏━┓┏┓╋╋╋┏┓┏┓");
    LOGI("\033[1;35m          ┃┏╋┛┣┳┳━╋╋┛┃");
    LOGI("\033[1;35m          ┃┗┫╋┃┏┫╋┃┃╋┃");
    LOGI("\033[1;35m          ┗━┻━┻┛┗━┻┻━┛");

    LOGI("cdroid %s on %s [%s] Build:%d Commit:%s",Build::VERSION::Release,Build::VERSION::BASE_OS,
            Build::VERSION::CODENAME,Build::VERSION::BuildNumber,Build::VERSION::CommitID);
    LOGI("https://www.gitee.com/houstudio/cdroid");

    GraphDevice& graph =GraphDevice::getInstance();
    if(rotation){
        rotation = (rotation/90)%4;
        WindowManager::getInstance().setDisplayRotation(0,rotation);
        graph.setRotation(rotation);
    }
    if(!logo.empty()) graph.setLogo(logo);
    graph.showFPS(showFPS).init();
    View::VIEW_DEBUG = debug;
    DisplayMetrics::DENSITY_DEVICE = DisplayMetrics::getDeviceDensity();
    if(alpha!=255) setOpacity(alpha);
    if(density) DisplayMetrics::DENSITY_DEVICE = density;
    if(frameDelay) Choreographer::setFrameDelay(frameDelay);
    // --input-mode: fix the InputEventSource reader backend before its lazy
    // init (the first checkEvents) — thread is the stock behavior;
    // choreographer polls InputGetEvents(0ms) per frame on CALLBACK_INPUT
    // with no reader thread (see InputEventSource::Mode).
    if (inputMode == "choreographer") {
        InputEventSource::setMode(InputEventSource::Mode::Choreographer);
    } else if (inputMode != "thread") {
        LOGW("unknown --input-mode=%s, using thread", inputMode.c_str());
    }
    Typeface::loadPreinstalledSystemFontMap();

    InputEventSource*inputsource=&InputEventSource::getInstance();
    addEventHandler(inputsource);
    AnimationHandler::getInstance();
    // IMM is a process-wide service (Android: created early, peekInstance() then
    // returns non-null once the app is up). Create it here so the View focus path
    // (View::onFocusChanged uses peekInstance -> focusIn/focusOut) and TextView's
    // editor show/hide actually engage, instead of silently no-oping on a null
    // singleton.
    InputMethodManager::getInstance();
}

std::atomic<App*>App::mInst;

App::~App(){
    LOGD("~App %p",this);
    auto inst = InputMethodManager::peekInstance();
    if(inst)inst->shutDown();
    // Detach the i18n buffer BEFORE mI18nData (member) frees: anything touching
    // DataResource after this falls back to the sidecar-load path instead of
    // reading freed memory.
    i18n::DataResource::SetData(nullptr, 0);
    delete &WindowManager::getInstance();
    // Ended-but-orphaned transition clones: their deferred self-delete post is
    // dropped when the main queue is already quitting at window-sweep time —
    // nobody else would ever free them (only that post deletes the clone).
    Transition::deleteOrphanedClones();
    // InputEventSource unregisters itself from the main Looper in its dtor, so it
    // must die BEFORE the Looper — the old order (Looper first) left its
    // removeEventHandler() call reading a freed Looper (valgrind UAF).
    delete &InputEventSource::getInstance();
    delete &GraphDevice::getInstance();
    // Covers a plain return from exec() (no App::exit): flush any queued
    // async writes, then park the QueuedWork thread (idempotent).
    QueuedWork::waitToFinish();
    QueuedWork::quitSafely();
    // The main Looper goes last: every other subsystem above still talks to it.
    delete Looper::getMainLooper();
    LOGD("~App %p",this);    destroyResourceState();
}

void App::onInit(){
    LOGD("onInit");
    InputInit();
    GFXInit();
    mDisplayMetrics.setToDefaults();
    // Locate a shared pak (cdroid.pak / widgetex.pak): data path first, then
    // the executable's directory (build-tree layout puts the app binary in
    // apps/<name>/ with cdroid.pak at the binary-root, so walk up a couple of
    // levels), then system install paths ($CDROID_PAK_PATH, /usr/share/cdroid,
    // /opt/cdroid) for pm-installed apps. The cwd is deliberately NOT probed:
    // where the process happens to be launched from must not decide which pak
    // wins. Without the framework pak every framework style resolves empty —
    // a themed app silently loses its parent chain and the overflow menu
    // renders with no background style at all.
    auto findSharedPak = [this](const std::string& name) -> std::string {
        std::vector<std::string> cands;
        cands.push_back(getDataPath() + name);
        // Resolve the executable to an absolute path first — argv[0] may be
        // relative ("./printerdemo") and a naive dirname walk would stall on ".".
        char rp[PATH_MAX] = {0};
        std::string dir = realpath(mName.c_str(), rp) ? std::string(rp) : mName;
        for (int up = 0; up < 3 && !dir.empty(); up++) {
            const size_t pos = dir.rfind(PATH_SEP);
            if (pos == std::string::npos) break;
            dir = dir.substr(0, pos);
            if (dir.empty()) dir = "/";
            cands.push_back(dir + PATH_SEP + name);
        }
        // $CDROID_PAK_PATH: optional extra search dir(s), colon-separated,
        // probed before the standard install locations. No cwd probing: where
        // the process happens to be launched from must not decide which pak
        // wins.
        if (const char* extra = getenv("CDROID_PAK_PATH")) {
            const std::string dirs(extra);
            size_t start = 0;
            while (start <= dirs.size()) {
                const size_t colon = dirs.find(':', start);
                const std::string dir = dirs.substr(start,
                        colon == std::string::npos ? std::string::npos : colon - start);
                if (!dir.empty()) cands.push_back(dir + PATH_SEP + name);
                if (colon == std::string::npos) break;
                start = colon + 1;
            }
        }
        // System search paths: an installed app (pm install layout:
        // /data/app/cdroid/<pkg>/...) can't reach the out-tree root by walking
        // up, so probe the standard install locations.
        cands.push_back(std::string("/usr/share/cdroid") + PATH_SEP + name);
        cands.push_back(std::string("/opt/cdroid") + PATH_SEP + name);
        for (const auto& c : cands)
            if (0 == access(c.c_str(), F_OK)) return c;
        return std::string();
    };
    const std::string pak = findSharedPak("cdroid.pak");
    if (!pak.empty()) addResource(pak, "cdroid", /* isSystemAsset */ true);
    else addResource("cdroid.pak", "cdroid", /* isSystemAsset */ true);   // keep the old failure log
    // i18n data: load raw/i18n.dat from cdroid.pak into an App-lifetime buffer
    // so DataResource::Init reads from RAM (no fd/lseek/read per format class).
    // The buffer backs DataResource's static pointer — filled once, never
    // modified, and detached in ~App before the member frees. Falls back to
    // the ./i18n.dat sidecar when the pak entry is absent.
    if (Asset* asset = openAsset("cdroid:raw/i18n.dat")) {
        const char* base = (const char*)asset->getBuffer(false);
        mI18nData.assign(base, base + asset->getLength());
        delete asset;
        if (!mI18nData.empty()) {
            i18n::DataResource::SetData(mI18nData.data(), mI18nData.size());
            LOGD("i18n.dat from pak: %zu bytes (buffer-based)", mI18nData.size());
        }
    } else {
        LOGW("cdroid:raw/i18n.dat not found in pak — i18n falls back to ./i18n.dat");
    }
    // widgetEx shared resource pak (package-id 0x02 — ConstraintLayout/TabLayout/
    // RecyclerView/etc. custom attrs). Built once, shared by all apps.
    const std::string wpak = findSharedPak("widgetex.pak");
    if (!wpak.empty()) addResource(wpak, "widgetex");
}

const std::string App::getDataPath()const{
    const size_t pos =mName.rfind(PATH_SEP);
    std::string path;
    if(pos!=std::string::npos)
        path = getArg("data",mName.substr(0,pos + 1));
    else
        path = getArg("data",std::string(".")+PATH_SEP);
    return path;
}

App& App::getInstance(){
    if(mInst == nullptr)
        mInst = new App;
    return *mInst;
}

const std::string App::getArg(const std::string&key,const std::string&fallback)const{
    if(mArgsResult && mArgsResult->count(key)) {
        return (*mArgsResult)[key].as<std::string>();
    }
    return fallback;
}

bool App::hasArg(const std::string&key)const{
    return mArgsResult->count(key)!=0;
}

bool App::hasSwitch(const std::string&key)const{
    return mArgsResult && mArgsResult->count(key)!=0;
}

int App::getArgAsInt(const std::string&key,int fallback)const{
    if(mArgsResult&&mArgsResult->count(key)){
        return (*mArgsResult)[key].as<int>();
    }
    return fallback;
}

float App::getArgAsFloat(const std::string&key,float fallback)const{
    if(mArgsResult && mArgsResult->count(key)){
        return (*mArgsResult)[key].as<float>();
    }
    return fallback;
}

double App::getArgAsDouble(const std::string&key,double fallback)const{
    if(mArgsResult->count(key)){
        return (*mArgsResult)[key].as<double>();
    }
    return fallback;
}

size_t App::getParamCount()const{
    return mArgsResult?mArgsResult->arguments().size():0;
}

std::string App::getParam(int idx,const std::string&def)const{
    const auto& args = mArgsResult->arguments();
    if((idx < args.size()) && (idx >= 0)){
        const std::string  key = args[idx].key();
        return (*mArgsResult)[key].as<std::string>();
    }
    return def;
}

void App::setOpacity(unsigned char alpha){
    auto primarySurface = GraphDevice::getInstance().getPrimarySurface();
    if(primarySurface){
        GFXSurfaceSetOpacity(primarySurface,alpha);
        LOGD("alpha=%d",alpha);
    }
}

void App::addEventHandler(const EventHandler*handler){
    Looper::getMainLooper()->addEventHandler(handler);
}

void App::removeEventHandler(const EventHandler*handler){
    Looper::getMainLooper()->removeEventHandler(handler);
}

int App::exec(){
    Looper*looper = Looper::getMainLooper();
    // Event-driven loop: loopOnce() -> MessageQueue::next() -> nativePollOnce()
    // blocks until a message is due, an fd event fires, or wake() is called.
    // next() internally lands in pollInner(), whose tail runs drainMessageQueue()
    // + doEventHandlers() (handleIdle = the frame driver), so frames advance on
    // every wake and the loop sleeps when idle. This needs a working wake()
    // channel on every platform (eventfd / pipe / socket pair) -- the old 1ms
    // pollAll(1) busy-spin was only the fallback for when wake() was a no-op.
    looper->wake();
    while(!mQuitFlag){ if(!looper->loopOnce()) break; }
    return mExitCode;
}

void App::exit(int code){
    mExitCode = code;
    // Sentinel quit: posting AFTER everything already queued lets the message
    // queue's FIFO run the windows' posted teardown deletes (finishClose)
    // BEFORE the loop stops — messages sort by when, so due deletes stay ahead
    // of this sentinel and not-yet-due work sorts behind it and is dropped.
    // A direct quit(false) here would discard the backlog outright; quit(true)
    // (quitSafely) only drains already-due messages — the sentinel covers both
    // and supersedes the quitSafely detour (3cc609130).
    static Handler sExitHandler(Looper::getMainLooper());
    sExitHandler.post([this]() {
        // AOSP flushes QueuedWork (async SharedPreferences writes) at the
        // lifecycle checkpoints (Activity.onPause etc.); the exit sentinel
        // is CDROID's checkpoint — all already-queued UI work has run by
        // now, drain the outstanding disk work before the loop stops.
        QueuedWork::waitToFinish();
        mQuitFlag = true;   // exec()'s loop stops on this iteration
        MessageQueue* q = Looper::getMainLooper()->getQueue();
        if (q) q->quit(false);
    });
}

// AOSP ActivityThread.handleConfigurationChanged(Configuration): update the
// resources' live configuration (resource-variant reselection + cache
// invalidation), then route each activity — every changed bit declared in the
// activity's configChanges → dispatchConfigurationChanged; anything undeclared
// → recreate (AOSP relaunchActivity semantics).

// ============================================================================
// PackageManager role: parse the compiled AndroidManifest.xml carried by the
// app pak (aapt2 compiles the manifest into binary AXML; a PackageParser
// micro-port — AOSP parses this in system_server and ships ActivityInfo over
// Binder, CDROID has no system side so the App fills in).
// ============================================================================
const ActivityInfo* App::getActivityInfo(const std::string& name) const {
    auto it = mActivityInfos.find(name);
    return it == mActivityInfos.end() ? nullptr : &it->second;
}

std::string App::getLauncherActivity() const {
    for (const auto& kv : mActivityInfos) {
        if (kv.second.launchable) return kv.first;
    }
    return std::string();
}

// --- pak entry readers (the former ZIPArchive face) -------------------------
// Entry opens go through zip_fopen: on paks with duplicate entries (cdroid.pak's
// doubled color/ set) libzip's name-locate can fail to resolve names that
// zip_fopen still opens.

static bool slurpZipEntry(struct zip* pak, const char* entry, std::string& data) {
    zip_file_t* zf = zip_fopen(pak, entry, ZIP_RDONLY);
    if (zf == nullptr) return false;
    char buf[65536];
    zip_int64_t n;
    while ((n = zip_fread(zf, buf, sizeof(buf))) > 0)
        data.append(buf, (size_t)n);
    zip_fclose(zf);
    return true;
}

void App::parsePackageManifest(const std::string& pakPath) {
    using namespace cdroid::internal;
    int zerr = 0;
    struct zip* pak = zip_open(pakPath.c_str(), ZIP_CHECKCONS | ZIP_RDONLY, &zerr);
    if (pak == nullptr) return;   // not a pak/zip at all
    std::string data;
    const bool have = slurpZipEntry(pak, "AndroidManifest.xml", data);
    zip_close(pak);
    if (!have) return;   // no manifest (synthesized paks carry none)
    auto stream = std::make_unique<std::istringstream>(data);
    auto parser = XmlPullParser::detectAndCreate(this, std::move(stream));

    // Manifest attribute ids come from the AOSP attrs_manifest.xml
    // declare-styleables (generated framework_styleable.h).
    auto attrId = [](const uint32_t* styleable, int index) -> uint32_t {
        return styleable[index];
    };
    auto attrValueByName = [&](const AttributeSet& atts, uint32_t attrId,
                               const char* bareName) -> std::string {
        for (int i = 0; i < atts.getAttributeCount(); i++) {
            if ((uint32_t)atts.getAttributeNameResource(i) == attrId) {
                // aapt2 compiles references to typed values; the binary bridge
                // renders them back to "@type/name" strings.
                return atts.getAttributeValue(i);
            }
        }
        return bareName ? atts.getAttributeValue(std::string(), bareName) : std::string();
    };

    // configChanges flag names -> Configuration::CONFIG_* bits.
    auto parseConfigChanges = [](const std::string& value) -> int {
        static const std::pair<const char*, int> flags[] = {
            {"mcc", Configuration::CONFIG_MCC}, {"mnc", Configuration::CONFIG_MNC},
            {"locale", Configuration::CONFIG_LOCALE},
            {"touchscreen", Configuration::CONFIG_TOUCHSCREEN},
            {"keyboard", Configuration::CONFIG_KEYBOARD},
            {"keyboardHidden", Configuration::CONFIG_KEYBOARD_HIDDEN},
            {"navigation", Configuration::CONFIG_NAVIGATION},
            {"orientation", Configuration::CONFIG_ORIENTATION},
            {"screenLayout", Configuration::CONFIG_SCREEN_LAYOUT},
            {"uiMode", Configuration::CONFIG_UI_MODE},
            {"screenSize", Configuration::CONFIG_SCREEN_SIZE},
            {"smallestScreenSize", Configuration::CONFIG_SMALLEST_SCREEN_SIZE},
            {"density", Configuration::CONFIG_DENSITY},
            {"layoutDirection", Configuration::CONFIG_LAYOUT_DIRECTION},
            {"colorMode", Configuration::CONFIG_COLOR_MODE},
            {"fontScale", Configuration::CONFIG_FONT_SCALE},
        };
        int bits = 0;
        size_t pos = 0;
        while (pos < value.size()) {
            const size_t bar = value.find('|', pos);
            const std::string tok = value.substr(pos,
                    bar == std::string::npos ? std::string::npos : bar - pos);
            for (const auto& f : flags) {
                if (tok == f.first) { bits |= f.second; break; }
            }
            if (bar == std::string::npos) break;
            pos = bar + 1;
        }
        return bits;
    };

    // Resolve a compiled "@style/X" reference value to a resource id.
    auto resIdFromRef = [&](const std::string& value, const char* type) -> int {
        if (value.empty() || value[0] != '@') return 0;
        std::string entry = value.substr(1);
        const size_t slash = entry.rfind('/');
        if (slash == std::string::npos) return 0;
        const std::string name = entry.substr(slash + 1);
        return getResources().getIdentifier(name, type, "");
    };

    std::vector<ActivityInfo> stack;   // open <activity> elements
    ActivityInfo current;
    bool inActivity = false;
    bool sawMainAction = false, sawLauncherCategory = false;
    int type;
    while ((type = parser->next()) != XmlPullParser::END_DOCUMENT) {
        if (type == XmlPullParser::START_TAG) {
            const std::string tag = parser->getName();
            if (tag == "manifest") {
                // Root package attr: plain string (no namespace, no resource id —
                // aapt2 keeps it in the string pool), so read by bare name
                // directly (an id-0 walk would hit whichever id-less attribute
                // comes first, e.g. platformBuildVersionCode).
                mPackageName = parser->getAttributeValue(std::string(), "package");
            } else if (tag == "application") {
                mApplicationLabel = attrValueByName(*parser,
                        attrId(R::styleable::AndroidManifestApplication, R::styleable::AndroidManifestApplication_label), "label");
                mApplicationTheme = resIdFromRef(attrValueByName(*parser,
                        attrId(R::styleable::AndroidManifestApplication, R::styleable::AndroidManifestApplication_theme), "theme"), "style");
            } else if (tag == "activity") {
                inActivity = true;
                current = ActivityInfo();
                current.name = attrValueByName(*parser,
                        attrId(R::styleable::AndroidManifestActivity, R::styleable::AndroidManifestActivity_name), "name");
                current.theme = resIdFromRef(attrValueByName(*parser,
                        attrId(R::styleable::AndroidManifestActivity, R::styleable::AndroidManifestActivity_theme), "theme"), "style");
                current.label = attrValueByName(*parser,
                        attrId(R::styleable::AndroidManifestActivity, R::styleable::AndroidManifestActivity_label), "label");
                current.configChanges = parseConfigChanges(attrValueByName(*parser,
                        attrId(R::styleable::AndroidManifestActivity, R::styleable::AndroidManifestActivity_configChanges), "configChanges"));
                // android:screenOrientation: aapt2 compiles the enum to a typed
                // int (landscape=0, portrait=1), read through the indexed int
                // getter — the string rendering ("landscape") would be rejected
                // by the string-based parsers as a non-numeric value.
                for (int i = 0; i < parser->getAttributeCount(); i++) {
                    if ((uint32_t)parser->getAttributeNameResource(i) == attrId(
                            R::styleable::AndroidManifestActivity,
                            R::styleable::AndroidManifestActivity_screenOrientation)) {
                        current.screenOrientation = parser->getAttributeIntValue(i, -1);
                        break;
                    }
                }
                sawMainAction = sawLauncherCategory = false;
            } else if (inActivity && tag == "action") {
                if (attrValueByName(*parser,
                        attrId(R::styleable::AndroidManifestAction, R::styleable::AndroidManifestAction_name), "name")
                        == "android.intent.action.MAIN")
                    sawMainAction = true;
            } else if (inActivity && tag == "category") {
                if (attrValueByName(*parser,
                        attrId(R::styleable::AndroidManifestCategory, R::styleable::AndroidManifestCategory_name), "name")
                        == "android.intent.category.LAUNCHER")
                    sawLauncherCategory = true;
            }
        } else if (type == XmlPullParser::END_TAG) {
            const std::string tag = parser->getName();
            if (tag == "activity" && inActivity) {
                current.launchable = sawMainAction && sawLauncherCategory;
                if (!current.name.empty())
                    mActivityInfos[current.name] = current;
                inActivity = false;
            }
        }
    }
    LOGI("manifest: appTheme=0x%x label='%s' activities=%zu launcher='%s'",
         mApplicationTheme, mApplicationLabel.c_str(), mActivityInfos.size(),
         getLauncherActivity().c_str());
}

void App::handleConfigurationChanged(const Configuration& newConfig){
    Resources& res = getResources();
    // AOSP ActivityThread first notifies the Application itself
    // (ComponentCallbacks), then routes each activity.
    onConfigurationChanged(newConfig);
    const int changes = res.calcConfigChanges(&newConfig);
    if (changes & Configuration::CONFIG_LOCALE) {
        // AOSP ActivityThread.handleConfigurationChangedInner: a locale change
        // first moves the process-wide default (LocaleList.setDefault), so
        // Locale::getDefault() and locale-defaulted formatters observe the NEW
        // locale from here on; already-created formatters keep their captured
        // locale until their view is recreated / re-dispatched below.
        LocaleList::setDefault(newConfig.getLocales());
    }
    res.updateConfiguration(&newConfig, nullptr);
    if (changes == 0) return;

    std::vector<Window*> windows;
    WindowManager::getInstance().getWindows(windows);
    for (Window* w : windows) {
        if (w == nullptr) continue;
        // AOSP performActivityConfigurationChanged: keep the activity alive only
        // when every changed bit is declared in its configChanges.
        if ((changes & ~w->getConfigChanges()) == 0) {
            w->dispatchConfigurationChanged(const_cast<Configuration&>(res.getConfiguration()));
        } else {
            w->recreate();
        }
    }
}

// performLaunchActivity's create path (everything App::startActivity routes through): resolve
// the Intent's ComponentName.className via ActivityFactory (REGISTER_ACTIVITY) and `new` the
// Window (its ctor self-registers with WindowManager -> it appears on screen), then stamp the
// Intent on it. CDROID's className is the bare C++ class name matching the REGISTER_ACTIVITY
// key. Returns the created window, or nullptr when nothing was created: no component name, no
// registered class, or a reuse path (singleTop / CLEAR_TOP-singleTop / REORDER_TO_FRONT) that
// delivered the Intent to an existing instance instead.
Window* App::performLaunch(const Intent& intent){
    const std::string className = intent.getComponent().getClassName();
    if(className.empty()){
        LOGW("App::startActivity: intent has no component class name");
        return nullptr;
    }
    // Android FLAG_ACTIVITY_NO_HISTORY: existing noHistory windows are auto-finished when
    // navigating to another Activity.
    {
        std::vector<Window*> windows;
        WindowManager::getInstance().getWindows(windows);
        for(Window* w : windows){
            if(w->isNoHistory()) w->close();
        }
    }
    // singleTop reuse (androidx FLAG_ACTIVITY_SINGLE_TOP): if the topmost Window already has this
    // className, deliver the new Intent to it (setIntent = onNewIntent) instead of creating a new
    // instance. Matches Android singleTop launch-mode.
    if((intent.getFlags() & Intent::FLAG_ACTIVITY_SINGLE_TOP) != 0){
        Window* active = WindowManager::getInstance().getActiveWindow();
        if(active && active->getIntent().getComponent().getClassName() == className){
            active->setIntent(intent);
            active->onNewIntent(intent);
            WindowManager::getInstance().bringToFront(active);
            return nullptr;
        }
    }
    // FLAG_ACTIVITY_CLEAR_TOP: if target exists, close everything above it. With SINGLE_TOP, reuse
    // (onNewIntent); without, still create new (Android standard launch mode).
    if((intent.getFlags() & Intent::FLAG_ACTIVITY_CLEAR_TOP) != 0){
        std::vector<Window*> windows;
        WindowManager::getInstance().getWindows(windows);
        for(int i = (int)windows.size() - 1; i >= 0; --i){
            if(windows[i]->getIntent().getComponent().getClassName() == className){
                for(int j = (int)windows.size() - 1; j > i; --j) windows[j]->close();
                if((intent.getFlags() & Intent::FLAG_ACTIVITY_SINGLE_TOP) != 0){
                    windows[i]->setIntent(intent);
                    windows[i]->onNewIntent(intent);
                    WindowManager::getInstance().bringToFront(windows[i]);
                    return nullptr;
                }
                break; // pop done; fall through to create new (standard mode)
            }
        }
    }
    // FLAG_ACTIVITY_REORDER_TO_FRONT: if target exists anywhere, bring to front (no pop, no new).
    if((intent.getFlags() & Intent::FLAG_ACTIVITY_REORDER_TO_FRONT) != 0){
        std::vector<Window*> windows;
        WindowManager::getInstance().getWindows(windows);
        for(Window* w : windows){
            if(w->getIntent().getComponent().getClassName() == className){
                w->setIntent(intent);
                w->onNewIntent(intent);
                WindowManager::getInstance().bringToFront(w);
                return nullptr;
            }
        }
    }
    // AOSP performLaunchActivity: the activity's theme (from the manifest's
    // ActivityInfo, else the application theme) is applied before the class
    // instantiates — CDROID routes it through a pending slot the Window's
    // Context ctor consumes while building its ContextThemeWrapper overlay.
    const ActivityInfo* info = getActivityInfo(className);
    mPendingActivityTheme = info && info->theme ? info->theme : mApplicationTheme;
    ActivityFactory factory;
    Window* window = factory.instantiate(className);
    mPendingActivityTheme = 0;
    if(window != nullptr){
        // AOSP ActivityInfo.configChanges drives dispatch vs recreate.
        if (info && info->configChanges) window->setConfigChanges(info->configChanges);
        window->setIntent(intent);
        if((intent.getFlags() & Intent::FLAG_ACTIVITY_NO_HISTORY) != 0) window->setNoHistory(true);
    } // else: ActivityFactory::instantiate already logged "no Window registered".
    return window;
}

// AOSP Context.startActivity(Intent): the plain launch — flags still route to the reuse
// paths inside performLaunch.
void App::startActivity(const Intent& intent){
    performLaunch(intent);
}

// App-wide setTheme: applyTheme rebuilds the live theme; the override
// also records the choice as the manifest-equivalent default so windows
// launched afterwards (startActivity / Window::recreate) build their themed
// context under it — the app-wide dynamic-theming contract (toggle the theme,
// recreate(), the new activity inflates re-colored).
void App::setTheme(int resid) {
    mApplicationTheme = resid;
    applyTheme(resid);
}

// Shared tail of the options-carrying launches: stamp a scene-transition ActivityOptions on
// the freshly created window and consume it. A null `target` (a reuse path delivered the
// Intent to an existing instance, or nothing was created) means nothing is stamped and no
// flight plays — the "existing instance already visible" semantics.
static void stampSceneTransitionOptions(Window* target, ActivityOptions* options) {
    if (options == nullptr) return;
    if (target != nullptr && options->hasSceneTransition() && options->getActivity() != nullptr) {
        target->setSharedElementEnter(options->getActivity(), options->getSharedElements());
    }
    delete options;  // consumed (Android hands the Bundle to the system; it dies here)
}

void App::startActivityForResultInternal(Window* caller, const Intent& intent, int requestCode,
                                          ActivityOptions* options){
    Window* target = performLaunch(intent);
    stampSceneTransitionOptions(target, options);
    if(target != nullptr){
        mPendingResults.push_back({caller, requestCode, target});
    }
}

// AOSP Context.startActivity(Intent, Bundle): the launch returns the created window directly
// (no last-started side channel — a nested startActivity inside an onActivityResult cannot
// steal the stamp), and the options are consumed on it in the same message.
void App::startActivity(const Intent& intent, ActivityOptions* options){
    stampSceneTransitionOptions(performLaunch(intent), options);
}

void App::dispatchPendingResult(Window* target){
    for(auto it = mPendingResults.begin(); it != mPendingResults.end(); ++it){
        if(it->target == target){
            it->caller->onActivityResult(it->requestCode, target->getResultCode(), target->getResultData());
            mPendingResults.erase(it);
            return;
        }
    }
}

const std::string App::getName()const{
    const size_t pos = mName.rfind(PATH_SEP);
    return (pos!=std::string::npos)?mName.substr(pos+1):mName;
}

}

// --- ContextImpl face (absorbed from the former Assets) ---
using namespace Cairo;
namespace cdroid{

// mArscTheme is stored opaque in the header (void*) to keep androidfw out of
// assets.h; cast at the engine boundary.
static cdroid::Theme* asTheme(void* t) { return (cdroid::Theme*)t; }

// The AM2 engine behind this App's AssetManager (null before any pak with an
// arsc was registered). ContextImpl::arscEngine face.
AssetManager2* App::arscEngine() const {
    return mAssetManager ? &mAssetManager->getAssetManager2() : nullptr;
}

// Render a resource ID as "@type/key" (text-XML reference form) so binary-AXML
// references flow through the same resolution paths as text XML. Returns "" if
// the arsc can't name the resource (caller falls back to "@0x..").
std::string App::getResourceName(uint32_t resId) const {
    AssetManager2* am2 = arscEngine();
    if (!am2 || resId == 0) return "";
    auto name = am2->GetResourceName(resId);
    if (!name.has_value()) return "";
    // The Utf8 faces are preferred; only when unavailable are the Utf16
    // variants populated (the pak's pools are UTF-16).
    std::string type, entry;
    if (name->type != nullptr) type.assign(name->type, name->type_len);
    else if (name->type16 != nullptr) type = TextUtils::utf16_utf8((const uint16_t*)name->type16, name->type_len);
    if (name->entry != nullptr) entry.assign(name->entry, name->entry_len);
    else if (name->entry16 != nullptr) entry = TextUtils::utf16_utf8((const uint16_t*)name->entry16, name->entry_len);
    if (type.empty() || entry.empty()) return "";
    // Framework resources need the explicit package prefix ("@android:...");
    // app resources resolve under the default package, so omit it (matches
    // text-XML conventions).
    if (name->package != nullptr && strncmp(name->package, "android", name->package_len) == 0
            && name->package_len == 7)
        return "@android:" + type + "/" + entry;
    return "@" + type + "/" + entry;
}

void App::destroyResourceState(){
    delete mCdroidResources;   // holds mAssetManager as a borrowed pointer
    delete mAssetManager;      // owns the ApkAssets + AssetManager2 table
    delete asTheme(mArscTheme);

    for(auto it=mResources.begin(); it!=mResources.end(); it++) {
        if(it->second) zip_close(it->second);
    }
    mResources.clear();
    LOGD("~App resource state %p!",this);
}

// --- Lazy ID-based resource layer (AOSP Resources/AssetManager) ---
// The AssetManager (with the AM2 table) is built eagerly by addResource —
// every pak commits its ApkAssets there as it registers; only the Resources
// wrapper is lazy.
void App::ensureCdroidResources() const {
    if (mCdroidResources != nullptr) return;
    if (mAssetManager == nullptr) {
        mAssetManager = new AssetManager();
        for (const auto& p : mPakPaths) {
            mAssetManager->addAssetPath(p, nullptr);
        }
    }
    mCdroidResources = new cdroid::Resources(mAssetManager, const_cast<App*>(this));
}

Resources& App::getResources() {
    ensureCdroidResources();
    return *mCdroidResources;
}

AssetManager& App::getAssets() {
    ensureCdroidResources();
    return *mAssetManager;
}

// getDrawable/getColorStateList: Context's themed defaults (context.cc) call
// getResources()/getTheme(), both of which ensureCdroidResources() — no
// override needed here anymore (AOSP-final semantics).

const DisplayMetrics& App::getDisplayMetrics()const{
    return mDisplayMetrics;
}

std::string App::getPackageName()const {
    // AOSP: the package id from the parsed manifest (stable). Synthesized paks
    // without a manifest fall back to the executable basename — never the full
    // path (a path-valued name leaks into prefs/dirs as nested directory trees).
    if (!mPackageName.empty()) return mPackageName;
    const size_t pos = mName.rfind(PATH_SEP);
    return (pos == std::string::npos) ? mName : mName.substr(pos + 1);
}

Resources::Theme App::getTheme() {
    // Lazily build a theme if none has been applied yet, so the returned view's
    // engine is valid (AOSP getTheme() never returns a null theme). Binary mode
    // always has the AM2 table; the static fallback covers text-only paks.
    if (!mArscTheme && arscEngine() != nullptr) {
        mArscTheme = mAssetManager->getAssetManager2().NewTheme().release();
    }
    cdroid::Theme* engine = asTheme(mArscTheme);
    if (engine == nullptr) {
        static AssetManager2 sEmptyAm2;
        static std::unique_ptr<cdroid::Theme> sEmptyTheme = sEmptyAm2.NewTheme();
        engine = sEmptyTheme.get();
    }
    return Resources::Theme(getResources(), engine);
}

void App::applyTheme(int resid) {
    // AOSP Context.setTheme(@StyleRes int): rebuild the arsc-backed theme from
    // the style resource id (applyStyle follows the style's parent chain).
    delete asTheme(mArscTheme);
    mArscTheme = nullptr;
    if (arscEngine() != nullptr && resid) {
        mArscTheme = mAssetManager->getAssetManager2().NewTheme().release();
        if (!asTheme(mArscTheme)->ApplyStyle((uint32_t)resid).has_value()) {
            LOGW("arsc Theme applyStyle(resId=0x%08x) failed", resid);
            delete asTheme(mArscTheme);
            mArscTheme = nullptr;
        } else {
            LOGD("arsc Theme built from %s (resId=0x%08x)",
                 getResourceName((uint32_t)resid).c_str(), resid);
        }
    }
}

int App::addResource(const std::string&path,const std::string&name,bool isSystemAsset) {
    mPakPaths.push_back(path);   // recorded for the (lazy) Resources wrapper
    // Build the AssetManager eagerly on the first pak: it owns the AM2 table,
    // and each pak commits its ApkAssets (resources.arsc, zero-copy) as it
    // registers. addAssetPath on an already-built manager (later paks) keeps
    // the table current — SetApkAssets rebuilds the package groups atomically.
    LOGD("Loaded %s",name.c_str());
    if (!mAssetManager) {
        mAssetManager = new AssetManager();
        // Seed the requested config from the device metrics (AOSP
        // ResourcesManager applies the device configuration to every
        // Resources). densityDpi drives config-variant selection (hdpi vs
        // default buckets); with no LCD_DENSITY override it is 160 and
        // selection behaves exactly as before.
        ResTable_config cfg = {};
        cfg.density = (uint16_t)mDisplayMetrics.densityDpi;
        mAssetManager->setConfiguration(cfg);
    }
    mAssetManager->addAssetPath(path, nullptr, /* appAsLib */ false, isSystemAsset);
    struct zip* pak = zip_open(path.c_str(), ZIP_CHECKCONS | ZIP_RDONLY, nullptr);
    std::string package = name;
    if(name.empty()) {
        size_t pos=path.find_last_of('/');
        if(pos != std::string::npos)
            package = path.substr(pos+1);
        pos = package.find('.');
        if( pos != std::string::npos)
            package = package.substr(0,pos);
    }
    mResources.insert({package,pak});

    int count=0;
    auto sttm = SystemClock::uptimeMillis();
    // The arsc itself is read through the AssetManager's ApkAssets (mmap'd,
    // STORED zero-copy); no slurped copy and no second parse here.
    // The default theme is applied by App's bootstrap (single AOSP-like point
    // after every pak is loaded), not here.
    LOGI("[%s] loaded %d files, %d theme attrs, used %dms",
         package.c_str(), count, mArscTheme!=nullptr,
         int(SystemClock::uptimeMillis()-sttm));
    return pak?0:-1;
}
// Orientation into the arsc request config so -land/-port resource variants
// select. AOSP owns the effective orientation in WMS (rotation + per-
// activity locks) and ResourcesManager applies it to every Resources; CDROID
// has no rotation, so it is resolved once here at startup, before the theme
// build and any inflate: --orientation switch > launcher activity's
// android:screenOrientation > device screen shape.
void App::applyOrientationConfig(const std::string& forced) {
    if (arscEngine() == nullptr) return;
    int orientation = ResTable_config::ORIENTATION_ANY;
    const char* source = nullptr;
    if (!forced.empty()) {
        if (forced == "land" || forced == "landscape")
            orientation = ResTable_config::ORIENTATION_LAND;
        else if (forced == "port" || forced == "portrait")
            orientation = ResTable_config::ORIENTATION_PORT;
        else
            LOGW("--orientation='%s' invalid (land|landscape|port|portrait)", forced.c_str());
        if (orientation != ResTable_config::ORIENTATION_ANY) source = "switch";
    }
    if (orientation == ResTable_config::ORIENTATION_ANY) {
        // ActivityInfo numbering (landscape=0, portrait=1) remaps onto
        // ResTable_config::ORIENTATION_* (PORT=1, LAND=2).
        const std::string launcher = getLauncherActivity();
        if (!launcher.empty()) {
            const ActivityInfo* info = getActivityInfo(launcher);
            if (info != nullptr) {
                if (info->screenOrientation == 0) {
                    orientation = ResTable_config::ORIENTATION_LAND;
                    source = "manifest";
                } else if (info->screenOrientation == 1) {
                    orientation = ResTable_config::ORIENTATION_PORT;
                    source = "manifest";
                }
            }
        }
    }
    if (orientation == ResTable_config::ORIENTATION_ANY) {
        orientation = (mDisplayMetrics.widthPixels >= mDisplayMetrics.heightPixels)
                ? ResTable_config::ORIENTATION_LAND : ResTable_config::ORIENTATION_PORT;
        source = "auto";
    }
    // Apply through the AOSP face: the system layer (App, standing in for
    // ActivityThread/WMS) computes the effective Configuration, and
    // Resources.updateConfiguration pushes it via ResourcesImpl — metrics
    // sync, locale best-match, arsc reselect, cache flush. Writing the
    // engine config directly would bypass all that and desync ResourcesImpl::mConfig.
    // Read-modify-write keeps the rest of the live configuration intact.
    Resources& res = getResources();
    Configuration cfg = res.getConfiguration();
    cfg.orientation = (orientation == ResTable_config::ORIENTATION_LAND)
            ? Configuration::ORIENTATION_LANDSCAPE
            : Configuration::ORIENTATION_PORTRAIT;
    res.updateConfiguration(&cfg, nullptr);
    LOGI("orientation=%s (%s)",
         orientation == ResTable_config::ORIENTATION_LAND ? "landscape" : "portrait",
         source);
}
// AOSP Context.obtainStyledAttributes(AttributeSet, int[], defStyleAttr,
// defStyleRes) — delegates to Resources.obtainStyledAttributes (the AOSP
// Resources surface; the resolver logic lives there now). AttributeSet is
// nullable (AOSP @Nullable).
std::unique_ptr<TypedArray> App::obtainStyledAttributes(
    const AttributeSet* attrs, const uint32_t* styleable,
    int32_t defStyleAttr, int32_t defStyleRes)
{
    return getResources().obtainStyledAttributes(attrs, styleable, defStyleAttr, defStyleRes);
}


}
