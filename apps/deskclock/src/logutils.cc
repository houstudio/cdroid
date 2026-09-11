#include <logutils.h>

#include <cstdarg>
#include <cstdio>

#include <porting/cdlog.h>

namespace cdroid {
namespace deskclock {
namespace LogUtils {

// CDROID: no Build.TYPE on this platform; debug builds log everything.
const bool DEBUG_BUILD = true;

namespace {

void logLine(const char* tag, const char* message, va_list args) {
    char buf[512];
    vsnprintf(buf, sizeof(buf), message, args);
    LOGD("[%s] %s", tag, buf);
}

} // namespace
#ifndef DEBUG
#define DEBUG (!NDEBUG)
#endif
bool Logger::isVerboseLoggable() const { return DEBUG; }
bool Logger::isDebugLoggable() const { return DEBUG; }
bool Logger::isInfoLoggable() const { return DEBUG; }
bool Logger::isWarnLoggable() const { return DEBUG; }
bool Logger::isErrorLoggable() const { return DEBUG; }
bool Logger::isWtfLoggable() const { return DEBUG; }

void Logger::v(const char* message, ...) const {
    va_list args; va_start(args, message); logLine(mLogTag.c_str(), message, args); va_end(args);
}
void Logger::d(const char* message, ...) const {
    va_list args; va_start(args, message); logLine(mLogTag.c_str(), message, args); va_end(args);
}
void Logger::i(const char* message, ...) const {
    va_list args; va_start(args, message); logLine(mLogTag.c_str(), message, args); va_end(args);
}
void Logger::w(const char* message, ...) const {
    va_list args; va_start(args, message); logLine(mLogTag.c_str(), message, args); va_end(args);
}
void Logger::e(const char* message, ...) const {
    va_list args; va_start(args, message); logLine(mLogTag.c_str(), message, args); va_end(args);
}
void Logger::wtf(const char* message, ...) const {
    va_list args; va_start(args, message); logLine(mLogTag.c_str(), message, args); va_end(args);
}

const Logger& getDefaultLogger() {
    static const Logger DEFAULT_LOGGER("AlarmClock");
    return DEFAULT_LOGGER;
}

void v(const char* message, ...) {
    va_list args; va_start(args, message);
    logLine("AlarmClock", message, args);
    va_end(args);
}
void d(const char* message, ...) {
    va_list args; va_start(args, message);
    logLine("AlarmClock", message, args);
    va_end(args);
}
void i(const char* message, ...) {
    va_list args; va_start(args, message);
    logLine("AlarmClock", message, args);
    va_end(args);
}
void w(const char* message, ...) {
    va_list args; va_start(args, message);
    logLine("AlarmClock", message, args);
    va_end(args);
}
void e(const char* message, ...) {
    va_list args; va_start(args, message);
    logLine("AlarmClock", message, args);
    va_end(args);
}
void wtf(const char* message, ...) {
    va_list args; va_start(args, message);
    logLine("AlarmClock", message, args);
    va_end(args);
}

} // namespace LogUtils
} // namespace deskclock
} // namespace cdroid
