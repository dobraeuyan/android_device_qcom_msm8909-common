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

#ifndef WV_DRM_FACTORY_PROXY_H_
#define WV_DRM_FACTORY_PROXY_H_

#include <android/hardware/drm/1.0/IDrmFactory.h>
#include <android/hardware/drm/1.2/IDrmFactory.h>

namespace android {
namespace hardware {
namespace drm {
namespace widevine {

using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::hardware::drm::V1_1::SecurityLevel;
using ::android::hardware::drm::V1_2::IDrmFactory;

typedef ::android::hardware::drm::V1_0::IDrmFactory IDrmFactory_V1_0;

struct WvDrmFactoryProxy : public IDrmFactory {
    explicit WvDrmFactoryProxy(const sp<IDrmFactory_V1_0>& delegate)
        : mDelegate(delegate) {}
    virtual ~WvDrmFactoryProxy() {}

    Return<bool> isCryptoSchemeSupported(
        const hidl_array<uint8_t, 16>& uuid) override;

    Return<bool> isContentTypeSupported(const hidl_string& mimeType) override;

    Return<void> createPlugin(
        const hidl_array<uint8_t, 16>& uuid,
        const hidl_string& appPackageName,
        createPlugin_cb _hidl_cb) override;

    Return<bool> isCryptoSchemeSupported_1_2(
        const hidl_array<uint8_t, 16>& uuid,
        const hidl_string& mimeType,
        SecurityLevel securityLevel) override;

private:
    sp<IDrmFactory_V1_0> mDelegate;

    WvDrmFactoryProxy(const WvDrmFactoryProxy&) = delete;
    void operator=(const WvDrmFactoryProxy&) = delete;
};

}
}
}
}

#endif
