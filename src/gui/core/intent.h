#ifndef __INTENT_H__
#define __INTENT_H__
#include <set>
#include <core/rect.h>
#include <core/parcel.h>
#include <core/bundle.h>

namespace cdroid{
class Uri;
class ClipData;
class ComponentName;
class Context;
class XmlPullParser;
class AttributeSet;
class Intent{ 
public:
    static constexpr int FLAG_GRANT_READ_URI_PERMISSION = 0x00000001;
    static constexpr int FLAG_GRANT_WRITE_URI_PERMISSION = 0x00000002;
    static constexpr int FLAG_DEBUG_LOG_RESOLUTION = 0x00000008;
    static constexpr int FLAG_EXCLUDE_STOPPED_PACKAGES = 0x00000010;
    static constexpr int FLAG_INCLUDE_STOPPED_PACKAGES = 0x00000020;
    static constexpr int FLAG_GRANT_PERSISTABLE_URI_PERMISSION = 0x00000040;
    static constexpr int FLAG_GRANT_PREFIX_URI_PERMISSION = 0x00000080;
    static constexpr int FLAG_DEBUG_TRIAGED_MISSING = 0x00000100;
    static constexpr int FLAG_IGNORE_EPHEMERAL = 0x00000200;
    static constexpr int FLAG_ACTIVITY_NO_HISTORY = 0x40000000;
    static constexpr int FLAG_ACTIVITY_SINGLE_TOP = 0x20000000;
    static constexpr int FLAG_ACTIVITY_NEW_TASK = 0x10000000;
    static constexpr int FLAG_ACTIVITY_MULTIPLE_TASK = 0x08000000;
    static constexpr int FLAG_ACTIVITY_CLEAR_TOP = 0x04000000;
    static constexpr int FLAG_ACTIVITY_FORWARD_RESULT = 0x02000000;
    static constexpr int FLAG_ACTIVITY_PREVIOUS_IS_TOP = 0x01000000;
    static constexpr int FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS = 0x00800000;
    static constexpr int FLAG_ACTIVITY_BROUGHT_TO_FRONT = 0x00400000;
    static constexpr int FLAG_ACTIVITY_RESET_TASK_IF_NEEDED = 0x00200000;
    static constexpr int FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY = 0x00100000;
    static constexpr int FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET = 0x00080000;
    static constexpr int FLAG_ACTIVITY_NEW_DOCUMENT = FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET;
    static constexpr int FLAG_ACTIVITY_NO_USER_ACTION = 0x00040000;
    static constexpr int FLAG_ACTIVITY_REORDER_TO_FRONT = 0x00020000;
    static constexpr int FLAG_ACTIVITY_NO_ANIMATION = 0x00010000;
    static constexpr int FLAG_ACTIVITY_CLEAR_TASK = 0x00008000;
    static constexpr int FLAG_ACTIVITY_TASK_ON_HOME = 0x00004000;
    static constexpr int FLAG_ACTIVITY_RETAIN_IN_RECENTS = 0x00002000;
    static constexpr int FLAG_ACTIVITY_LAUNCH_ADJACENT = 0x00001000;
    static constexpr int FLAG_ACTIVITY_MATCH_EXTERNAL = 0x00000800;
    static constexpr int FLAG_RECEIVER_REGISTERED_ONLY = 0x40000000;
    static constexpr int FLAG_RECEIVER_REPLACE_PENDING = 0x20000000;
    static constexpr int FLAG_RECEIVER_FOREGROUND = 0x10000000;
    static constexpr int FLAG_RECEIVER_NO_ABORT = 0x08000000;
    static constexpr int FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT = 0x04000000;
    static constexpr int FLAG_RECEIVER_BOOT_UPGRADE = 0x02000000;
    static constexpr int FLAG_RECEIVER_INCLUDE_BACKGROUND = 0x01000000;
    static constexpr int FLAG_RECEIVER_EXCLUDE_BACKGROUND = 0x00800000;
    static constexpr int FLAG_RECEIVER_FROM_SHELL = 0x00400000;
    static constexpr int FLAG_RECEIVER_VISIBLE_TO_INSTANT_APPS = 0x00200000;
    static constexpr int IMMUTABLE_FLAGS = FLAG_GRANT_READ_URI_PERMISSION
            | FLAG_GRANT_WRITE_URI_PERMISSION | FLAG_GRANT_PERSISTABLE_URI_PERMISSION
            | FLAG_GRANT_PREFIX_URI_PERMISSION;

    // ---------------------------------------------------------------------
    // toUri() and parseUri() options.

    static constexpr int URI_INTENT_SCHEME = 1<<0;
    static constexpr int URI_ANDROID_APP_SCHEME = 1<<1;
    static constexpr int URI_ALLOW_UNSAFE = 1<<2;
    ///////////////////////////////////////////////////////////////////////
    static constexpr const char*const ACTION_MAIN = "android.intent.action.MAIN";
    static constexpr const char*const ACTION_VIEW = "android.intent.action.VIEW";
    static constexpr const char*const EXTRA_FROM_STORAGE = "android.intent.extra.FROM_STORAGE";
    static constexpr const char*const ACTION_DEFAULT = ACTION_VIEW;
    static constexpr const char*const ACTION_QUICK_VIEW = "android.intent.action.QUICK_VIEW";
    static constexpr const char*const ACTION_ATTACH_DATA = "android.intent.action.ATTACH_DATA";
    static constexpr const char*const ACTION_EDIT = "android.intent.action.EDIT";
    static constexpr const char*const ACTION_INSERT_OR_EDIT = "android.intent.action.INSERT_OR_EDIT";
    static constexpr const char*const ACTION_PICK = "android.intent.action.PICK";
    static constexpr const char*const ACTION_CREATE_REMINDER = "android.intent.action.CREATE_REMINDER";
    static constexpr const char*const ACTION_CREATE_SHORTCUT = "android.intent.action.CREATE_SHORTCUT";

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Standard intent categories (see addCategory()).

    static constexpr const char* const CATEGORY_DEFAULT = "android.intent.category.DEFAULT";
    static constexpr const char* const CATEGORY_BROWSABLE = "android.intent.category.BROWSABLE";
    static constexpr const char* const CATEGORY_VOICE = "android.intent.category.VOICE";
    static constexpr const char* const CATEGORY_ALTERNATIVE = "android.intent.category.ALTERNATIVE";
    static constexpr const char* const CATEGORY_SELECTED_ALTERNATIVE = "android.intent.category.SELECTED_ALTERNATIVE";
    static constexpr const char* const CATEGORY_TAB = "android.intent.category.TAB";
    static constexpr const char* const CATEGORY_LAUNCHER = "android.intent.category.LAUNCHER";
    static constexpr const char* const CATEGORY_LEANBACK_LAUNCHER = "android.intent.category.LEANBACK_LAUNCHER";
    static constexpr const char* const CATEGORY_CAR_LAUNCHER = "android.intent.category.CAR_LAUNCHER";
    static constexpr const char* const CATEGORY_COMMUNAL_MODE = "android.intent.category.COMMUNAL_MODE";
    static constexpr const char* const CATEGORY_LEANBACK_SETTINGS = "android.intent.category.LEANBACK_SETTINGS";
    static constexpr const char* const CATEGORY_INFO = "android.intent.category.INFO";
    static constexpr const char* const CATEGORY_HOME = "android.intent.category.HOME";
    static constexpr const char* const CATEGORY_HOME_MAIN = "android.intent.category.HOME_MAIN";
    static constexpr const char* const CATEGORY_SECONDARY_HOME = "android.intent.category.SECONDARY_HOME";
    static constexpr const char* const CATEGORY_SETUP_WIZARD = "android.intent.category.SETUP_WIZARD";
    static constexpr const char* const CATEGORY_LAUNCHER_APP = "android.intent.category.LAUNCHER_APP";
    static constexpr const char* const CATEGORY_PREFERENCE = "android.intent.category.PREFERENCE";
    static constexpr const char* const CATEGORY_DEVELOPMENT_PREFERENCE = "android.intent.category.DEVELOPMENT_PREFERENCE";
    static constexpr const char* const CATEGORY_EMBED = "android.intent.category.EMBED";
    static constexpr const char* const CATEGORY_APP_MARKET = "android.intent.category.APP_MARKET";
    static constexpr const char* const CATEGORY_MONKEY = "android.intent.category.MONKEY";
    static constexpr const char* const CATEGORY_TEST = "android.intent.category.TEST";
    static constexpr const char* const CATEGORY_UNIT_TEST = "android.intent.category.UNIT_TEST";
    static constexpr const char* const CATEGORY_SAMPLE_CODE = "android.intent.category.SAMPLE_CODE";
    static constexpr const char* const CATEGORY_OPENABLE = "android.intent.category.OPENABLE";
    static constexpr const char* const CATEGORY_TYPED_OPENABLE = "android.intent.category.TYPED_OPENABLE";
    static constexpr const char* const CATEGORY_FRAMEWORK_INSTRUMENTATION_TEST = "android.intent.category.FRAMEWORK_INSTRUMENTATION_TEST";
    static constexpr const char* const CATEGORY_CAR_DOCK = "android.intent.category.CAR_DOCK";
    static constexpr const char* const CATEGORY_DESK_DOCK = "android.intent.category.DESK_DOCK";
    static constexpr const char* const CATEGORY_LE_DESK_DOCK = "android.intent.category.LE_DESK_DOCK";
    static constexpr const char* const CATEGORY_HE_DESK_DOCK = "android.intent.category.HE_DESK_DOCK";
    static constexpr const char* const CATEGORY_CAR_MODE = "android.intent.category.CAR_MODE";
    static constexpr const char* const CATEGORY_VR_HOME = "android.intent.category.VR_HOME";
    static constexpr const char* const CATEGORY_ACCESSIBILITY_SHORTCUT_TARGET = "android.intent.category.ACCESSIBILITY_SHORTCUT_TARGET";

    // ---------------------------------------------------------------------
    // Application launch intent categories (see addCategory()).

    static constexpr const char* const CATEGORY_APP_BROWSER = "android.intent.category.APP_BROWSER";
    static constexpr const char* const CATEGORY_APP_CALCULATOR = "android.intent.category.APP_CALCULATOR";
    static constexpr const char* const CATEGORY_APP_CALENDAR = "android.intent.category.APP_CALENDAR";
    static constexpr const char* const CATEGORY_APP_CONTACTS = "android.intent.category.APP_CONTACTS";
    static constexpr const char* const CATEGORY_APP_EMAIL = "android.intent.category.APP_EMAIL";
    static constexpr const char* const CATEGORY_APP_GALLERY = "android.intent.category.APP_GALLERY";
    static constexpr const char* const CATEGORY_APP_MAPS = "android.intent.category.APP_MAPS";
    static constexpr const char* const CATEGORY_APP_MESSAGING = "android.intent.category.APP_MESSAGING";
    static constexpr const char* const CATEGORY_APP_MUSIC = "android.intent.category.APP_MUSIC";
    static constexpr const char* const CATEGORY_APP_FILES = "android.intent.category.APP_FILES";
    static constexpr const char* const CATEGORY_APP_WEATHER = "android.intent.category.APP_WEATHER";
    static constexpr const char* const CATEGORY_APP_FITNESS = "android.intent.category.APP_FITNESS";
private:
    // ---------------------------------------------------------------------

    std::string mAction;
    Uri* mData;
    std::string mType;
    std::string mPackage;
    //ComponentName mComponent;
    int mFlags;
    std::set<std::string> mCategories;
    Bundle* mExtras;
    Rect mSourceBounds;
    Intent* mSelector;
    ClipData* mClipData;
    int mContentUserHint;// = UserHandle.USER_CURRENT;
    /** Token to track instant app launches. Local only; do not copy cross-process. */
    std::string mLaunchToken;

    // ---------------------------------------------------------------------
    static constexpr int COPY_MODE_ALL = 0;
    static constexpr int COPY_MODE_FILTER = 1;
    static constexpr int COPY_MODE_HISTORY = 2;
private:
    Intent(const Intent& o, int copyMode);
    static Intent* getIntentOld(const std::string& uri, int flags);
    void toUriFragment(std::ostringstream& uri,const std::string& scheme,const std::string& defAction,const std::string& defPackage, int flags);
    void toUriInner(std::ostringstream& uri, const std::string&scheme,const std::string& defAction,const std::string& defPackage, int flags);
protected:
    Intent(Parcel& in);
public:
    Intent();

    Intent(const Intent& o);

    Intent* cloneFilter();

    Intent(const std::string& action);

    Intent(const std::string& action, Uri* uri);

    //Intent(Context* packageContext, Class<?> cls);
    //Intent(const std::string& action, Uri& uri,Context* packageContext, Class<?> cls);
    //static Intent makeMainActivity(ComponentName& mainActivity);

    static Intent* makeMainSelectorActivity(const std::string& selectorAction,const std::string& selectorCategory);

    //static Intent makeRestartActivityTask(ComponentName mainActivity);

    static Intent* parseUri(const std::string& uri,int flags);

    static Intent* getIntentOld(const std::string& uri);


    /*public interface CommandOptionHandler {
        bool handleOption(std::string opt, ShellCommand cmd);
    }*/

    //static Intent parseCommandArgs(ShellCommand cmd, CommandOptionHandler optionHandler);

    //static void printIntentArgsHelp(PrintWriter pw, std::string prefix);

    std::string getAction()const;

    Uri* getData()const;

    std::string getDataString()const;

    std::string getScheme()const;

    std::string getType()const;

    std::string resolveType(Context* context);

    //std::string resolveType(ContentResolver& resolver);
    //std::string resolveTypeIfNeeded(ContentResolver& resolver);

    bool hasCategory(const std::string&category);

    std::set<std::string> getCategories();

    Intent* getSelector();

    ClipData* getClipData();

    int getContentUserHint();

    std::string getLaunchToken();

    void setLaunchToken(const std::string&launchToken);

    //void setExtrasClassLoader(ClassLoader* loader);

    bool hasExtra(const std::string& name);

    bool hasFileDescriptors();

    void setAllowFds(bool allowFds);

    void setDefusable(bool defusable);

    bool getBooleanExtra(const std::string& name, bool defaultValue);

    uint8_t getByteExtra(const std::string& name, uint8_t defaultValue);

    short getShortExtra(const std::string& name, short defaultValue);

    char getCharExtra(const std::string& name, char defaultValue);

    int getIntExtra(const std::string& name, int defaultValue);

    int64_t getLongExtra(const std::string& name, int64_t defaultValue);

    float getFloatExtra(const std::string& name, float defaultValue);

    double getDoubleExtra(const std::string& name, double defaultValue);

    std::string getStringExtra(const std::string& name);

    Parcelable* getParcelableExtra(const std::string& name);

    std::vector<Parcelable*> getParcelableArrayExtra(const std::string& name);

    //Serializable getSerializableExtra(const std::string&name);

    std::vector<int32_t> getIntegerArrayListExtra(const std::string&name);

    std::vector<std::string> getStringArrayListExtra(const std::string&name);

    std::vector<bool> getBooleanArrayExtra(const std::string&name);

    std::vector<int8_t> getByteArrayExtra(const std::string&name);

    std::vector<int16_t> getShortArrayExtra(const std::string&name);

    std::vector<char> getCharArrayExtra(const std::string&name);

    std::vector<int32_t> getIntArrayExtra(const std::string&name);

    std::vector<int64_t> getLongArrayExtra(const std::string&name);

    std::vector<float> getFloatArrayExtra(const std::string&name);

    std::vector<double> getDoubleArrayExtra(const std::string&name);

    std::vector<std::string> getStringArrayExtra(const std::string&name);

    Bundle* getBundleExtra(const std::string&name);

    Bundle* getExtras();

    void removeUnsafeExtras();

    bool canStripForHistory()const;

    Intent* maybeStripForHistory();

    int getFlags() const;

    bool isExcludingStopped()const;

    std::string getPackage()const;

    ComponentName getComponent();

    Rect getSourceBounds();

    //ComponentName resolveActivity(PackageManager& pm);
    //ActivityInfo resolveActivityInfo(PackageManager& pm,int flags);
    //ComponentName resolveSystemService(PackageManager& pm,int flags);

    Intent& setAction(const std::string& action);

    Intent& setData(Uri* data);

    Intent& setDataAndNormalize(Uri& data);

    Intent& setType(const std::string& type);

    Intent& setTypeAndNormalize(const std::string& type);

    Intent& setDataAndType(Uri* data, const std::string& type);

    Intent& setDataAndTypeAndNormalize(Uri& data, const std::string& type);

    Intent& addCategory(const std::string&category);

    void removeCategory(const std::string&category);

    void setSelector(Intent* selector);

    void setClipData(ClipData* clip);

    void prepareToLeaveUser(int userId);
    Intent& putExtra(const std::string& name, bool value);

    Intent& putExtra(const std::string&name, int8_t value);

    //Intent& putExtra(const std::string&name, char value);

    Intent& putExtra(const std::string&name, short value);

    Intent& putExtra(const std::string&name, int value);

    Intent& putExtra(const std::string&name, int64_t value);

    Intent& putExtra(const std::string&name, float value);

    Intent& putExtra(const std::string&name, double value);

    Intent& putExtra(const std::string&name, const std::string& value);

    Intent& putExtra(const std::string&name, Parcelable* value);

    Intent& putExtra(const std::string&name, const std::vector<Parcelable*>& value);

    Intent& putParcelableArrayListExtra(const std::string&name,const std::vector<Parcelable*>& value);

    Intent& putIntegerArrayListExtra(const std::string&name, const std::vector<int32_t>& value);

    Intent& putStringArrayListExtra(const std::string&name, const std::vector<std::string>& value);

    //Intent& putExtra(const std::string&name, Serializable value);

    Intent& putExtra(const std::string&name, const std::vector<bool>& value);

    Intent& putExtra(const std::string&name, const std::vector<int8_t>& value);

    Intent& putExtra(const std::string&name, const std::vector<int16_t>& value);

    Intent& putExtra(const std::string&name, const std::vector<char>& value);

    Intent& putExtra(const std::string&name, const std::vector<int32_t>& value);

    Intent& putExtra(const std::string&name, const std::vector<int64_t>& value);

    Intent& putExtra(const std::string&name, const std::vector<float>& value);

    Intent& putExtra(const std::string&name, const std::vector<double>& value);

    Intent& putExtra(const std::string&name, const std::vector<std::string>& value);

    Intent& putExtra(const std::string&name, Bundle* value);

    //Intent& putExtra(const std::string& name, IBinder value);

    Intent& putExtras(const Intent& src);

    Intent& putExtras(const Bundle* extras);

    Intent& replaceExtras(const Intent& src);

    Intent& replaceExtras(const Bundle* extras);

    void removeExtra(const std::string& name);

    Intent& setFlags(int flags);

    Intent& addFlags(int flags);

    void removeFlags(int flags);

    Intent& setPackage(const std::string& packageName);

    Intent& setComponent(ComponentName component);

    Intent& setClassName(Context* packageContext,const std::string& className);

    Intent& setClassName(const std::string& packageName, const std::string& className);

    //Intent& setClass(Context* packageContext, Class<?> cls);

    void setSourceBounds(const Rect* r);

    static constexpr int FILL_IN_ACTION = 1<<0;
    static constexpr int FILL_IN_DATA = 1<<1;
    static constexpr int FILL_IN_CATEGORIES = 1<<2;
    static constexpr int FILL_IN_COMPONENT = 1<<3;
    static constexpr int FILL_IN_PACKAGE = 1<<4;
    static constexpr int FILL_IN_SOURCE_BOUNDS = 1<<5;
    static constexpr int FILL_IN_SELECTOR = 1<<6;
    static constexpr int FILL_IN_CLIP_DATA = 1<<7;

    int fillIn(const Intent& other, int flags);

    /*public static final class FilterComparison {
        private final Intent mIntent;
        private final int mHashCode;

        public FilterComparison(Intent intent) {
            mIntent = intent;
            mHashCode = intent.filterHashCode();
        }

        public Intent getIntent() {
            return mIntent;
        }

        public bool equals(Object obj) {
            if (obj instanceof FilterComparison) {
                Intent other = ((FilterComparison)obj).mIntent;
                return mIntent.filterEquals(other);
            }
            return false;
        }

        public int hashCode() {
            return mHashCode;
        }
    };*/


    bool filterEquals(const Intent* other);

    int filterHashCode();

    std::string toString();

    std::string toInsecureString();

    std::string toInsecureStringWithClip();

    std::string toShortString(bool secure, bool comp, bool extras, bool clip);

    void toShortString(std::ostringstream& b, bool secure, bool comp, bool extras,bool clip);
    //void writeToProto(ProtoOutputStream proto, long fieldId, bool secure, bool comp,bool extras, bool clip);

    std::string toURI();

    std::string toUri(int flags);

    int describeContents();

    void writeToParcel(Parcel& out, int flags);

    void readFromParcel(Parcel& in);

    static Intent* parseIntent(XmlPullParser& parser, const AttributeSet& attrs);

    //void saveToXml(XmlSerializer& out);

    static Intent restoreFromXml(XmlPullParser& in);

    static std::string normalizeMimeType(const std::string& type);

    void prepareToLeaveProcess(Context* context);

    void prepareToLeaveProcess(bool leavingPackage);

    void prepareToEnterProcess();

    bool hasWebURI();

    bool isWebIntent()const;

    void fixUris(int contentUserHint);

    bool migrateExtraStreamToClipData();

    static std::string dockStateToString(int dock);

    //private static ClipData.Item makeClipItem(std::vector<Uri> streams, std::vector<CharSequence> texts,const std::vector<std::string>& htmlTexts, int which);

    bool isDocument() const;
};
}/*endof namespace*/
#endif
