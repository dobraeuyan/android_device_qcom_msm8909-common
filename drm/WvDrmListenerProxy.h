/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WV_DRM_LISTENER_PROXY_H_
#define WV_DRM_LISTENER_PROXY_H_

#include <android/hardware/drm/1.0/IDrmPluginListener.h>
#include <android/hardware/drm/1.2/IDrmPluginListener.h>

#include <android-base/logging.h>

namespace android {
namespace hardware {
namespace drm {
namespace widevine {

using ::android::sp;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::drm::V1_0::EventType;
using ::android::hardware::drm::V1_0::KeyStatus;

typedef ::android::hardware::drm::V1_0::IDrmPluginListener IDrmPluginListener_V1_0;
typedef ::android::hardware::drm::V1_2::IDrmPluginListener IDrmPluginListener_V1_2;
typedef ::android::hardware::drm::V1_2::KeyStatus KeyStatus_V1_2;

struct WvDrmListenerProxy : public IDrmPluginListener_V1_2 {
    explicit WvDrmListenerProxy(const sp<IDrmPluginListener_V1_0>& appListener)
        : mAppListener(appListener),
          mAppListenerV1_2(IDrmPluginListener_V1_2::castFrom(appListener)) {}
    virtual ~WvDrmListenerProxy() {}

    Return<void> sendEvent(
        EventType eventType,
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& data) override {
        if (mAppListenerV1_2 != nullptr) {
            mAppListenerV1_2->sendEvent(eventType, sessionId, data);
        } else if (mAppListener != nullptr) {
            mAppListener->sendEvent(eventType, sessionId, data);
        } else {
            LOG(ERROR) << "WvDrmListenerProxy: no app listener, event dropped";
        }
        return Void();
    }

    Return<void> sendExpirationUpdate(
        const hidl_vec<uint8_t>& sessionId,
        int64_t expiryTimeInMS) override {
        if (mAppListenerV1_2 != nullptr) {
            mAppListenerV1_2->sendExpirationUpdate(sessionId, expiryTimeInMS);
        } else if (mAppListener != nullptr) {
            mAppListener->sendExpirationUpdate(sessionId, expiryTimeInMS);
        }
        return Void();
    }

    Return<void> sendKeysChange(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<KeyStatus>& keyStatusList,
        bool hasNewUsableKey) override {
        if (mAppListenerV1_2 != nullptr) {
            mAppListenerV1_2->sendKeysChange(sessionId, keyStatusList,
                                             hasNewUsableKey);
        } else if (mAppListener != nullptr) {
            mAppListener->sendKeysChange(sessionId, keyStatusList,
                                         hasNewUsableKey);
        }
        return Void();
    }

    Return<void> sendSessionLostState(
        const hidl_vec<uint8_t>& sessionId) override {
        if (mAppListenerV1_2 != nullptr) {
            mAppListenerV1_2->sendSessionLostState(sessionId);
        }
        return Void();
    }

    Return<void> sendKeysChange_1_2(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<KeyStatus_V1_2>& keyStatusList,
        bool hasNewUsableKey) override {
        if (mAppListenerV1_2 != nullptr) {
            mAppListenerV1_2->sendKeysChange_1_2(sessionId, keyStatusList,
                                                 hasNewUsableKey);
        }
        return Void();
    }

private:
    sp<IDrmPluginListener_V1_0> mAppListener;
    sp<IDrmPluginListener_V1_2> mAppListenerV1_2;

    WvDrmListenerProxy(const WvDrmListenerProxy&) = delete;
    void operator=(const WvDrmListenerProxy&) = delete;
};

}
}
}
}

#endif
