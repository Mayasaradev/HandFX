#include <android/log.h>
#include "utils/logger.hpp"

#define TAG "HandFX"

void logMessage(const char* message)
{
    __android_log_print(
        ANDROID_LOG_INFO,
        TAG,
        "%s",
        message
    );
}
