#ifndef MINIKIN_LOG_LOG_H
#define MINIKIN_LOG_LOG_H

// Suppress all logging code.
// Original log library: https://android.googlesource.com/platform/system/logging/+/refs/heads/main-kernel/liblog/include/log/log.h

#define ALOGD(...)
#define ALOGE(...)
#define ALOGI(...)
#define ALOGW(...)
#define LOG_ALWAYS_FATAL_IF(...)

#define android_errorWriteLog(...)

#endif // MINIKIN_LOG_LOG_H