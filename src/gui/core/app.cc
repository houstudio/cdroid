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
#include <core/LocaleList.h>
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
#include "data_resource.h"
#include <core/cxxopts.h>
#include <core/inputeventsource.h>
#include <core/windowmanager.h>
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

App::App(int argc,const char*argv[]):mQuitFlag(false),mExitCode(0){
    int alpha = 255, rotation = 0, density = 0, frameDelay = 0;
    bool debug= false,showFPS = false, help = false;
    std::string logo, monkey, record, datapath;
    LogParseModules(argc,argv);
    mInst = this;
    cxxopts::Options options("cdroid","cdroid application");
    options.add_options()
        ("d,debug","enable debuig mode",cxxopts::value<bool>(debug))
        ("h,help","print helps",cxxopts::value<bool>(help))
        ("fps", "show fps info",cxxopts::value<bool>(showFPS))
        ("a,alpha","UI layer global alpha[0,255]",cxxopts::value<int>(alpha)->default_value("255"))
        ("f,framedelay","animation frame delay",cxxopts::value<int>(frameDelay))
        ("density","UI Density",cxxopts::value<int>(density))
        ("R,rotate","display rotate(90*n)",cxxopts::value<int>(rotation)->default_value("0"))
        ("l,logo","show logo",cxxopts::value<std::string>(logo))
        ("m,monkey","events playback path",cxxopts::value<std::string>(monkey))
        ("r,record","events record path",cxxopts::value<std::string>(record))
        ("data","data directory",cxxopts::value<std::string>(datapath));

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
        std::cout<<options.help()<<std::endl;
        exit(EXIT_SUCCESS);
        LogSetModuleLevel(nullptr,LOG_FATAL);
        mQuitFlag = true;
        return;
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
    setTheme(mApplicationTheme ? mApplicationTheme
                               : (int)cdroid::internal::R::style::Theme_Material);
    // AOSP: the system starts the manifest's launcher activity — app main()
    // never does. CDROID's App plays that side on the message queue (like
    // ActivityThread: bindApplication/launchActivity are messages): a posted
    // runnable fires once exec()'s loop is turning, after anything main() set
    // up synchronously. Skipped when a window is already up or the manifest
    // declares no activity (app-driven windows keep working as before).
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
    Typeface::loadFaceFromResource(this);

    InputEventSource*inputsource=&InputEventSource::getInstance();//(getArg("record",""));
    addEventHandler(inputsource);
    if(!monkey.empty()){
        inputsource->playback(monkey);
    }
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
    delete &WindowManager::getInstance();
    delete Looper::getMainLooper();
    delete &GraphDevice::getInstance();
    delete &InputEventSource::getInstance();
    LOGD("~App %p",this);
}

void App::onInit(){
    LOGD("onInit");
    GFXInit();
    mDisplayMetrics.setToDefaults();
    // Locate a shared pak (cdroid.pak / widgetex.pak): data path first, then
    // the executable's directory (build-tree layout puts the app binary in
    // apps/<name>/ with cdroid.pak at the binary-root, so walk up a couple of
    // levels), then the cwd. Without the framework pak every framework style
    // resolves empty — a themed app silently loses its parent chain and the
    // overflow menu renders with no background style at all.
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
        for (const auto& c : cands)
            if (0 == access(c.c_str(), F_OK)) return c;
        return std::string();
    };
    const std::string pak = findSharedPak("cdroid.pak");
    if (!pak.empty()) addResource(pak, "cdroid");
    else addResource("cdroid.pak", "cdroid");   // keep the old failure log
    // i18n data: load raw/i18n.dat from cdroid.pak into a process-lifetime
    // buffer so DataResource::Init reads from RAM (no fd/lseek/read per format
    // class). The string is heap-allocated and never deleted — its buffer is
    // the backing store for DataResource's static pointer. Falls back to the
    // ./i18n.dat sidecar when the pak entry is absent.
    if (auto stream = getInputStream("cdroid:raw/i18n.dat")) {
        auto* data = new std::string((std::istreambuf_iterator<char>(*stream)),
                                     std::istreambuf_iterator<char>());
        if (!data->empty()) {
            i18n::DataResource::SetData(data->data(), data->size());
            LOGD("i18n.dat from pak: %zu bytes (buffer-based)", data->size());
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
    mQuitFlag = true;
    mExitCode = code;
    // With the blocking loop above, exit() must wake the loop or it stays parked
    // in next()/pollInner and never notices mQuitFlag. quit() clears the queue
    // and calls nativeWake() -> Looper::wake(), unblocking next() to return null.
    MessageQueue* q = Looper::getMainLooper()->getQueue();
    if(q){
        q->quit(false);
    }
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
    XmlPullParser parser(this, std::move(stream));

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
    while ((type = parser.next()) != XmlPullParser::END_DOCUMENT) {
        if (type == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "application") {
                mApplicationLabel = attrValueByName(parser,
                        attrId(R::styleable::AndroidManifestApplication, R::styleable::AndroidManifestApplication_label), "label");
                mApplicationTheme = resIdFromRef(attrValueByName(parser,
                        attrId(R::styleable::AndroidManifestApplication, R::styleable::AndroidManifestApplication_theme), "theme"), "style");
            } else if (tag == "activity") {
                inActivity = true;
                current = ActivityInfo();
                current.name = attrValueByName(parser,
                        attrId(R::styleable::AndroidManifestActivity, R::styleable::AndroidManifestActivity_name), "name");
                current.theme = resIdFromRef(attrValueByName(parser,
                        attrId(R::styleable::AndroidManifestActivity, R::styleable::AndroidManifestActivity_theme), "theme"), "style");
                current.label = attrValueByName(parser,
                        attrId(R::styleable::AndroidManifestActivity, R::styleable::AndroidManifestActivity_label), "label");
                current.configChanges = parseConfigChanges(attrValueByName(parser,
                        attrId(R::styleable::AndroidManifestActivity, R::styleable::AndroidManifestActivity_configChanges), "configChanges"));
                sawMainAction = sawLauncherCategory = false;
            } else if (inActivity && tag == "action") {
                if (attrValueByName(parser,
                        attrId(R::styleable::AndroidManifestAction, R::styleable::AndroidManifestAction_name), "name")
                        == "android.intent.action.MAIN")
                    sawMainAction = true;
            } else if (inActivity && tag == "category") {
                if (attrValueByName(parser,
                        attrId(R::styleable::AndroidManifestCategory, R::styleable::AndroidManifestCategory_name), "name")
                        == "android.intent.category.LAUNCHER")
                    sawLauncherCategory = true;
            }
        } else if (type == XmlPullParser::END_TAG) {
            const std::string tag = parser.getName();
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

// App-wide setTheme: Assets::setTheme rebuilds the live theme; the override
// also records the choice as the manifest-equivalent default so windows
// launched afterwards (startActivity / Window::recreate) build their themed
// context under it — the app-wide dynamic-theming contract (toggle the theme,
// recreate(), the new activity inflates re-colored).
void App::setTheme(int resid) {
    mApplicationTheme = resid;
    Assets::setTheme(resid);
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

