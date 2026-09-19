#ifndef __DESKCLOCK_LOGUTILS_H__
#define __DESKCLOCK_LOGUTILS_H__
/*********************************************************************************
 * Port of com.android.deskclock.LogUtils — tag-gated logging facade over cdlog.
 * Java's String.format(message, *args) maps to printf-style varargs.
 *********************************************************************************/
#include <string>
#include <stdexcept>

namespace cdroid {
namespace deskclock {
namespace LogUtils {

// Log everything for debug builds or if running on a dev device.
// (BuildConfig.DEBUG || "eng" == Build.TYPE || "userdebug" == Build.TYPE)
extern const bool DEBUG_BUILD; // named DEBUG upstream; -DDEBUG build define collides

class Logger {
private:
    const std::string mLogTag;
public:
    explicit Logger(const std::string& logTag) : mLogTag(logTag) {}

    bool isVerboseLoggable() const;
    bool isDebugLoggable() const;
    bool isInfoLoggable() const;
    bool isWarnLoggable() const;
    bool isErrorLoggable() const;
    bool isWtfLoggable() const;

    void v(const char* message, ...) const;
    void d(const char* message, ...) const;
    void i(const char* message, ...) const;
    void w(const char* message, ...) const;
    void e(const char* message, ...) const;
    void wtf(const char* message, ...) const;
};

// Default logger used for generic logging when a specific log tag isn't specified.
const Logger& getDefaultLogger();

void v(const char* message, ...);
void d(const char* message, ...);
void i(const char* message, ...);
void w(const char* message, ...);
void e(const char* message, ...);
void wtf(const char* message, ...);

} // namespace LogUtils
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_LOGUTILS_H__
