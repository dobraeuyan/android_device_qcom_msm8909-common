#define LOG_TAG "android.hardware.thermal@2.0-service-msm8909"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include "Thermal.h"

using ::android::OK;
using ::android::status_t;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::hardware::thermal::V2_0::IThermal;
using ::android::hardware::thermal::V2_0::implementation::Thermal;

static int shutdown() {
    LOG(ERROR) << "Thermal HAL service is shutting down.";
    return 1;
}

int main(int /* argc */, char** /* argv */) {
    android::sp<IThermal> service = nullptr;

    LOG(INFO) << "Thermal HAL 2.0 (msm8909) starting";

    service = new Thermal();
    if (service == nullptr) {
        LOG(ERROR) << "Failed to create Thermal HAL instance";
        return shutdown();
    }

    configureRpcThreadpool(1, true /* callerWillJoin */);

    status_t status = service->registerAsService();
    if (status != OK) {
        LOG(ERROR) << "Failed to register Thermal HAL: " << status;
        return shutdown();
    }

    LOG(INFO) << "Thermal HAL 2.0 (msm8909) registered";
    joinRpcThreadpool();
    return shutdown();
}
