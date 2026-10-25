#define LOG_TAG "izatapi_shim"
#include <log/log.h>

extern "C" {

int _ZN9izat_core10IzatApiV0221injectXtraData_legacyEPKcj(void* thiz,
                                                          const char* data,
                                                          unsigned int len);

int _ZN9izat_core10IzatApiV0214injectXtraDataEPKcj(void* thiz,
                                                   const char* data,
                                                   unsigned int len) {
    ALOGI("injectXtraData(len=%u) -> legacy QMI_LOC_INJECT_PREDICTED_ORBITS_DATA",
          len);
    int rc = _ZN9izat_core10IzatApiV0221injectXtraData_legacyEPKcj(thiz, data,
                                                                   len);
    ALOGI("injectXtraData_legacy rc=%d", rc);
    return rc;
}

}
