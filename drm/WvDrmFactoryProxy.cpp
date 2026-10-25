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

#define LOG_TAG "WvDrmFactoryProxy"

#include "WvDrmFactoryProxy.h"

#include "WvDrmPluginProxy.h"

#include <android-base/logging.h>

namespace android {
namespace hardware {
namespace drm {
namespace widevine {

using ::android::hardware::Void;
using ::android::hardware::drm::V1_0::Status;

typedef ::android::hardware::drm::V1_0::IDrmPlugin IDrmPlugin_V1_0;

Return<bool> WvDrmFactoryProxy::isCryptoSchemeSupported(
        const hidl_array<uint8_t, 16>& uuid) {
    return mDelegate->isCryptoSchemeSupported(uuid);
}

Return<bool> WvDrmFactoryProxy::isContentTypeSupported(
        const hidl_string& mimeType) {
    return mDelegate->isContentTypeSupported(mimeType);
}

Return<bool> WvDrmFactoryProxy::isCryptoSchemeSupported_1_2(
        const hidl_array<uint8_t, 16>& uuid,
        const hidl_string& mimeType,
        SecurityLevel securityLevel) {
    if (static_cast<uint32_t>(securityLevel) >
            static_cast<uint32_t>(SecurityLevel::SW_SECURE_CRYPTO)) {
        return false;
    }

    Return<bool> schemeSupported = mDelegate->isCryptoSchemeSupported(uuid);
    if (!schemeSupported.isOk() || !static_cast<bool>(schemeSupported)) {
        return false;
    }

    if (mimeType.size() > 0) {
        Return<bool> typeSupported = mDelegate->isContentTypeSupported(mimeType);
        if (!typeSupported.isOk() || !static_cast<bool>(typeSupported)) {
            return false;
        }
    }

    return true;
}

Return<void> WvDrmFactoryProxy::createPlugin(
        const hidl_array<uint8_t, 16>& uuid,
        const hidl_string& appPackageName,
        createPlugin_cb _hidl_cb) {
    return mDelegate->createPlugin(
        uuid, appPackageName,
        [&](Status status, const sp<IDrmPlugin_V1_0>& plugin) {
            if (status != Status::OK || plugin == nullptr) {
                _hidl_cb(status, nullptr);
                return;
            }
            sp<WvDrmPluginProxy> wrapped = new WvDrmPluginProxy(plugin);
            _hidl_cb(status, wrapped);
        });
}

}
}
}
}
