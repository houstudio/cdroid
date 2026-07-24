#ifndef MINIKIN_LOG_LOG_H
#define MINIKIN_LOG_LOG_H
#include <cstdio>
#include <cstdarg>
// Suppress all logging code.
// Original log library: https://android.googlesource.com/platform/system/logging/+/refs/heads/main-kernel/liblog/include/log/log.h
static void custom_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args); // 使用 vprintf 处理可变参数
    printf("\n");       // 添加换行符
    va_end(args);
}
#define ALOGD(...) custom_log(__VA_ARGS__)
#define ALOGE(...) custom_log(__VA_ARGS__)
#define ALOGI(...) custom_log(__VA_ARGS__)
#define ALOGW(...) custom_log(__VA_ARGS__)
#define LOG_ALWAYS_FATAL_IF(...)

#define android_errorWriteLog(...)

#endif // MINIKIN_LOG_LOG_H
