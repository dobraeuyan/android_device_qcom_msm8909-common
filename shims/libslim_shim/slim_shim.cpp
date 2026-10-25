#include <cinttypes>
#include <cstdio>
#include <string>

namespace android {
namespace frameworks {
namespace sensorservice {
namespace V1_0 {

enum class Result : int32_t {
    OK = 0,
    NOT_EXIST = 1,
    NO_MEMORY = 2,
    NO_INIT = 3,
    PERMISSION_DENIED = 4,
    BAD_VALUE = 5,
    INVALID_OPERATION = 6,
    UNKNOWN_ERROR = 7,
};

std::string toString(Result o);

std::string toString(Result o) {
    switch (o) {
        case Result::OK: return "OK";
        case Result::NOT_EXIST: return "NOT_EXIST";
        case Result::NO_MEMORY: return "NO_MEMORY";
        case Result::NO_INIT: return "NO_INIT";
        case Result::PERMISSION_DENIED: return "PERMISSION_DENIED";
        case Result::BAD_VALUE: return "BAD_VALUE";
        case Result::INVALID_OPERATION: return "INVALID_OPERATION";
        case Result::UNKNOWN_ERROR: return "UNKNOWN_ERROR";
    }
    char buf[19];
    snprintf(buf, sizeof(buf), "0x%" PRIx32, static_cast<int32_t>(o));
    return buf;
}

}
}
}
}
