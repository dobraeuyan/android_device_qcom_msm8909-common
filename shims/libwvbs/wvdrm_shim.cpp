#include <android-base/logging.h>

#include <new>

extern "C" void*
_ZN7android4base10LogMessageC1EPKcjNS0_5LogIdENS0_11LogSeverityEi(
        void* thiz, const char* file, unsigned int line,
        android::base::LogId id, android::base::LogSeverity severity,
        int error) {

    new (thiz) android::base::LogMessage(file, line, id, severity,
                                          nullptr, error);
    return thiz;
}
