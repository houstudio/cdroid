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
#include <core/queuedwork.h>   // exit-path flush of async writes
#include <content/typedarray.h>   // TypedArray (constructed in obtainStyledAttributes)
#include <content/typedvalue.h>   // TypedValue (typed currency of this layer)
#include <content/androidfw/restable.h> // ResTable engine + Res_value (boundary lookups)
#include <content/assetmanager.h>   // AssetManager
#include <content/resources.h> // cdroid::Resources
#include <algorithm>
#include <cdtypes.h>
#include <cdlog.h>
#include <ziparchive.h>
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
#include <private/ziparchive.h>
#include <widget/framework_styleable.h>
#include <core/xmlpullparser.h>
#include <core/build.h>
#include <core/messagequeue.h>
#include <core/intent.h>
#include <core/activityfactory.h>
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
    std::string autoTest, autoTestRecord, testScript, orientation;
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
         cxxopts::value<std::string>(testScript));

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
        std::string pakPath =getDataPath()+name+std::string(".pak");
        if(0==access(pakPath.c_str(),F_OK)) {
            addResource(pakPath,getName());
            appPakPath = pakPath;
        }
        else {
            addResource(name+".pak",getName());
            appPakPath = name+".pak";
        }
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
    // levels), then the cwd, then system install paths ($CDROID_PAK_PATH,
    // /usr/share/cdroid, /opt/cdroid) for pm-installed apps. Without the
    // framework pak every framework style resolves empty — a themed app
    // silently loses its parent chain and the overflow menu renders with no
    // background style at all.
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
        cands.push_back(name);   // cwd
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
    if (!pak.empty()) addResource(pak, "cdroid");
    else addResource("cdroid.pak", "cdroid");   // keep the old failure log
    // i18n data: load raw/i18n.dat from cdroid.pak into an App-lifetime buffer
    // so DataResource::Init reads from RAM (no fd/lseek/read per format class).
    // The buffer backs DataResource's static pointer — filled once, never
    // modified, and detached in ~App before the member frees. Falls back to
    // the ./i18n.dat sidecar when the pak entry is absent.
    if (auto stream = getInputStream("cdroid:raw/i18n.dat")) {
        mI18nData.assign(std::istreambuf_iterator<char>(*stream),
                         std::istreambuf_iterator<char>());
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

void App::parsePackageManifest(const std::string& pakPath) {
    using namespace cdroid::internal;
    ZIPArchive pak(pakPath);
    std::istream* stm = pak.getInputStream("AndroidManifest.xml");
    if (stm == nullptr) return;   // no manifest (synthesized paks carry none)
    auto stream = std::unique_ptr<std::istream>(stm);
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
            if (tag == "application") {
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

void App::startActivity(const Intent& intent){
    // Resolve the Intent's ComponentName.className via ActivityFactory (REGISTER_ACTIVITY) and `new`
    // the Window (its ctor self-registers with WindowManager -> it appears on screen), then stamp the
    // Intent on it. CDROID's className is the bare C++ class name matching the REGISTER_ACTIVITY key.
    const std::string className = intent.getComponent().getClassName();
    if(className.empty()){
        LOGW("App::startActivity: intent has no component class name");
        return;
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
            return;
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
                    return;
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
                return;
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
        mLastStartedWindow = window;
    } // else: ActivityFactory::instantiate already logged "no Window registered".
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

void App::startActivityForResultInternal(Window* caller, const Intent& intent, int requestCode){
    mLastStartedWindow = nullptr;
    startActivity(intent); // creates target + sets mLastStartedWindow
    if(mLastStartedWindow != nullptr){
        mPendingResults.push_back({caller, requestCode, mLastStartedWindow});
    }
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

// androidfw glue (same seam as typedarray.cc): fill a TypedValue from the raw
// Res_value handed out by ResTable lookups. AOSP does this fill in the native
// layer; core code speaks TypedValue from here on.
static TypedValue tvOf(const Res_value& rv) {
    TypedValue tv; tv.type = rv.dataType; tv.data = rv.data; return tv;
}
// mArscTheme is stored opaque in the header (void*) to keep androidfw out of
// assets.h; cast at the engine boundary.
static ResTable::Theme* asTheme(void* t) { return (ResTable::Theme*)t; }


// Resolve a resource ID through the loaded arsc.
bool App::arscResolveId(uint32_t resId, TypedValue* out) const {
    if (!mResTable || resId == 0 || resId == 0xFFFFFFFF) return false;
    Res_value rv;
    if (mResTable->getResource(resId, &rv) < 0) return false;
    *out = tvOf(rv);
    return true;
}

// Get a string from the arsc string pool by resource ID.
const char16_t* App::arscStringAt(uint32_t resId, size_t* outLen) const {
    if (!mResTable || resId == 0) return nullptr;
    return mResTable->getResourceString(resId, outLen);
}

// Render a resource ID as "@type/key" (text-XML reference form) so binary-AXML
// references flow through the same resolution paths as text XML. Returns "" if
// the arsc can't name the resource (caller falls back to "@0x..").
std::string App::getResourceName(uint32_t resId) const {
    if (!mResTable || resId == 0) return "";
    std::string pkg, type, key;
    if (mResTable->getResourceName(resId, &pkg, &type, &key) && !type.empty() && !key.empty()) {
        // Framework resources need the explicit package prefix ("@android:...");
        // app resources resolve under the default package, so omit it (matches
        // text-XML conventions).
        if (pkg == "android") return "@android:" + type + "/" + key;
        return "@" + type + "/" + key;
    }
    return "";
}

// Resolve a theme-attribute reference (?attr/<id>) through the arsc Theme.
bool App::arscThemeAttribute(uint32_t attrId, TypedValue* out, ssize_t* outBlock) const {
    if (!mArscTheme || !out) return false;
    ResTable::Theme* theme = (ResTable::Theme*)mArscTheme;
    Res_value rv;
    ssize_t blk = theme->getAttribute(attrId, &rv);
    if (blk < 0) return false;
    // Flatten ?attr / @ref chains to a concrete value.
    blk = theme->resolveAttributeReference(&rv, blk);
    if (blk < 0) return false;
    *out = tvOf(rv);
    if (outBlock) *outBlock = blk;
    return true;
}

// Try to resolve a "@0xPPtteeee" hex resource ID string through the arsc.
bool App::arscResolveHexRef(const std::string& s, TypedValue* out) const {
    if (!mResTable || s.empty()) return false;
    // Accept "@0x...", "0x...", or a bare hex tail after the last '@'.
    size_t at = s.rfind('@');
    std::string hex = (at != std::string::npos) ? s.substr(at + 1) : s;
    if (hex.compare(0, 2, "0x") != 0 && hex.compare(0, 2, "0X") != 0) return false;
    char* end = nullptr;
    errno = 0;
    unsigned long id = strtoul(hex.c_str() + 2, &end, 16);
    if (errno || end == hex.c_str() + 2 || id == 0 || id == 0xFFFFFFFF) return false;
    Res_value rv;
    if (mResTable->getResource((uint32_t)id, &rv) < 0) return false;
    *out = tvOf(rv);
    return true;
}


// complexToFloat is provided inline by <androidfw/resourcetypes.h>.


void App::destroyResourceState(){
    delete mCdroidResources;   // holds mAssetManager as a borrowed pointer
    delete mAssetManager;
    delete asTheme(mArscTheme);
    delete mResTable;

    for(auto it=mResources.begin(); it!=mResources.end(); it++) {
        delete it->second;
    }
    mResources.clear();
    LOGD("~App resource state %p!",this);
}

// --- Lazy ID-based resource layer (AOSP Resources/AssetManager) ---
// Built on first use from the pak paths recorded in addResource(); the legacy
// string-based mResTable path is untouched.
void App::ensureCdroidResources() const {
    if (mCdroidResources != nullptr) return;
    if (mAssetManager == nullptr) {
        mAssetManager = new AssetManager();
        for (const auto& p : mPakPaths) {
            mAssetManager->addAssetPath(p, nullptr);
        }
        // Share the arsc table already parsed (addResource reads each
        // pak's resources.arsc into mResTable once). Without this, the lazy
        // AssetManager would re-read and re-parse the very same arsc a second
        // time when getResources() first touches it. mResTable is borrowed here
        // and freed by destroyResourceState after the AssetManager is destroyed. The Header
        // cookie differs (-1 here vs 1-based in appendPathToResTable) but the
        // engine never reads the cookie, and raw files are opened by path
        // (no-cookie openNonAsset), so sharing is safe.
        if (mResTable) mAssetManager->setResTable(mResTable);
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

const std::string App::getPackageName()const {
    return mName;
}

Resources::Theme App::getTheme() {
    // Lazily build an arsc theme if none has been applied yet, so the returned
    // view's engine is valid (AOSP getTheme() never returns a null theme). Binary
    // mode always has mResTable; the static fallback covers text-only paks.
    if (!mArscTheme && mResTable) {
        mArscTheme = new ResTable::Theme(*mResTable);
    }
    ResTable::Theme* engine = asTheme(mArscTheme);
    if (engine == nullptr) {
        static ResTable sEmptyTable;
        static ResTable::Theme sEmptyTheme(sEmptyTable);
        engine = &sEmptyTheme;
    }
    return Resources::Theme(getResources(), engine);
}

void App::applyTheme(int resid) {
    // AOSP Context.setTheme(@StyleRes int): rebuild the arsc-backed theme from
    // the style resource id (applyStyle follows the style's parent chain).
    delete asTheme(mArscTheme);
    mArscTheme = nullptr;
    if (mResTable && resid) {
        mArscTheme = new ResTable::Theme(*mResTable);
        if (asTheme(mArscTheme)->applyStyle((uint32_t)resid) != 0) {
            LOGW("arsc Theme applyStyle(resId=0x%08x) failed", resid);
            delete asTheme(mArscTheme);
            mArscTheme = nullptr;
        } else {
            LOGD("arsc Theme built from %s (resId=0x%08x, gen=%u)",
                 getResourceName((uint32_t)resid).c_str(), resid,
                 asTheme(mArscTheme)->cacheGeneration());
        }
    }
}

int App::addResource(const std::string&path,const std::string&name) {
    mPakPaths.push_back(path);   // recorded for the lazy ID-based AssetManager
    // If the lazy AssetManager was already built — which happens when an earlier
    // pak's addResource triggered ensureCdroidResources() via the pending
    // color-state-list resolve (getColorStateList → getResources) — register this
    // pak with it too. Otherwise files in later paks (e.g. app layouts in
    // uidemo1.pak, added after cdroid.pak) are invisible to openNonAsset, and
    // every app layout inflate returns null.
    LOGD("Loaded %s",name.c_str());
    if (mAssetManager) mAssetManager->addAssetPath(path, nullptr);
    ZIPArchive*pak = new ZIPArchive(path);
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
    // Load resources.arsc if present. Try getInputStream directly rather than
    // hasEntry: cdroid.pak carries duplicate color/ entries (SDK + own), and
    // libzip's zip_name_locate (used by hasEntry) fails to resolve some names
    // in such archives, while zip_fopen (getInputStream) still works. Each pak
    // is one add() = one owning Header; copyData=true makes ResTable malloc its
    // own copy, so the local buffer can be freed safely across multiple paks.
    auto stream = std::unique_ptr<std::istream>(pak->getInputStream("resources.arsc"));
    if (stream && *stream) {
        std::string data((std::istreambuf_iterator<char>(*stream)),
                         std::istreambuf_iterator<char>());
        if (!mResTable) {
            mResTable = new ResTable();
            // Seed the requested config from the device metrics (AOSP
            // ResourcesManager applies the device configuration to every
            // Resources). densityDpi drives config-variant selection (hdpi vs
            // default buckets); with no LCD_DENSITY override it is 160 and
            // selection behaves exactly as before.
            ResTable_config cfg = {};
            cfg.density = (uint16_t)mDisplayMetrics.densityDpi;
            mResTable->setParameters(&cfg);
        }
        // NOTE: the 4-arg form is required so `true` binds to copyData, not to
        // the int32_t cookie of the 3-arg overload — otherwise copyData defaults
        // to false, hdr->data aliases the local buffer, and freeing it on return
        // leaves every Package type/key pointer dangling (UAF).
        mResTable->add(data.data(), data.size(), /*cookie*/-1, /*copyData*/true);
        LOGD("Loaded resources.arsc from %s (%zu bytes, error=%d)",
             path.c_str(), data.size(), mResTable->getError());
    }
    // The default theme is applied by App's bootstrap (single AOSP-like point
    // after every pak is loaded), not here.
    LOGI("[%s] loaded %d files, %d theme attrs, used %dms",
         package.c_str(), count, mArscTheme!=nullptr,
         int(SystemClock::uptimeMillis()-sttm));
    return pak?0:-1;
}

int App::getNextAutofillId(){
    return mNextAutofillViewId++;
}

// Orientation into the arsc request config so -land/-port resource variants
// select. AOSP owns the effective orientation in WMS (rotation + per-
// activity locks) and ResourcesManager applies it to every Resources; CDROID
// has no rotation, so it is resolved once here at startup, before the theme
// build and any inflate: --orientation switch > launcher activity's
// android:screenOrientation > device screen shape.
void App::applyOrientationConfig(const std::string& forced) {
    if (mResTable == nullptr) return;
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
    // sync, locale best-match, arsc reselect, cache flush. Writing mResTable
    // directly would bypass all that and desync ResourcesImpl::mConfig.
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
