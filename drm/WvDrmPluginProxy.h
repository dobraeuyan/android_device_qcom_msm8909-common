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

#ifndef WV_DRM_PLUGIN_PROXY_H_
#define WV_DRM_PLUGIN_PROXY_H_

#include <android/hardware/drm/1.0/IDrmPlugin.h>
#include <android/hardware/drm/1.0/IDrmPluginListener.h>
#include <android/hardware/drm/1.2/IDrmPlugin.h>
#include <android/hardware/drm/1.2/IDrmPluginListener.h>

namespace android {
namespace hardware {
namespace drm {
namespace widevine {

using ::android::sp;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

using ::android::hardware::drm::V1_0::EventType;
using ::android::hardware::drm::V1_0::KeyStatus;
using ::android::hardware::drm::V1_0::KeyType;
using ::android::hardware::drm::V1_0::KeyValue;
using ::android::hardware::drm::V1_0::SecureStop;
using ::android::hardware::drm::V1_0::SecureStopId;
using ::android::hardware::drm::V1_0::SessionId;
using ::android::hardware::drm::V1_0::Status;
using ::android::hardware::drm::V1_1::DrmMetricGroup;
using ::android::hardware::drm::V1_1::HdcpLevel;
using ::android::hardware::drm::V1_1::SecureStopRelease;
using ::android::hardware::drm::V1_1::SecurityLevel;
using ::android::hardware::drm::V1_2::IDrmPlugin;
using ::android::hardware::drm::V1_2::KeySetId;
using ::android::hardware::drm::V1_2::OfflineLicenseState;

typedef ::android::hardware::drm::V1_0::IDrmPlugin IDrmPlugin_V1_0;
typedef ::android::hardware::drm::V1_0::IDrmPluginListener IDrmPluginListener_V1_0;
typedef ::android::hardware::drm::V1_0::KeyRequestType KeyRequestType_V1_0;
typedef ::android::hardware::drm::V1_1::KeyRequestType KeyRequestType_V1_1;
typedef ::android::hardware::drm::V1_2::IDrmPluginListener IDrmPluginListener_V1_2;
typedef ::android::hardware::drm::V1_2::KeyStatus KeyStatus_V1_2;
typedef ::android::hardware::drm::V1_2::Status Status_V1_2;
typedef ::android::hardware::drm::V1_2::HdcpLevel HdcpLevel_V1_2;

struct WvDrmPluginProxy : public IDrmPlugin {
    explicit WvDrmPluginProxy(const sp<IDrmPlugin_V1_0>& delegate)
        : mDelegate(delegate) {}
    virtual ~WvDrmPluginProxy() {}

    Return<void> openSession(openSession_cb _hidl_cb) override;

    Return<Status> closeSession(const hidl_vec<uint8_t>& sessionId) override;

    Return<void> getKeyRequest(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_cb _hidl_cb) override;

    Return<void> provideKeyResponse(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& response,
        provideKeyResponse_cb _hidl_cb) override;

    Return<Status> removeKeys(const hidl_vec<uint8_t>& sessionId) override;

    Return<Status> restoreKeys(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keySetId) override;

    Return<void> queryKeyStatus(
        const hidl_vec<uint8_t>& sessionId,
        queryKeyStatus_cb _hidl_cb) override;

    Return<void> getProvisionRequest(
        const hidl_string& certificateType,
        const hidl_string& certificateAuthority,
        getProvisionRequest_cb _hidl_cb) override;

    Return<void> provideProvisionResponse(
        const hidl_vec<uint8_t>& response,
        provideProvisionResponse_cb _hidl_cb) override;

    Return<void> getSecureStops(getSecureStops_cb _hidl_cb) override;

    Return<void> getSecureStop(
        const hidl_vec<uint8_t>& secureStopId,
        getSecureStop_cb _hidl_cb) override;

    Return<Status> releaseAllSecureStops() override;

    Return<Status> releaseSecureStop(
        const hidl_vec<uint8_t>& secureStopId) override;

    Return<void> getPropertyString(
        const hidl_string& propertyName,
        getPropertyString_cb _hidl_cb) override;

    Return<void> getPropertyByteArray(
        const hidl_string& propertyName,
        getPropertyByteArray_cb _hidl_cb) override;

    Return<Status> setPropertyString(
        const hidl_string& propertyName, const hidl_string& value) override;

    Return<Status> setPropertyByteArray(
        const hidl_string& propertyName, const hidl_vec<uint8_t>& value) override;

    Return<Status> setCipherAlgorithm(
        const hidl_vec<uint8_t>& sessionId, const hidl_string& algorithm) override;

    Return<Status> setMacAlgorithm(
        const hidl_vec<uint8_t>& sessionId, const hidl_string& algorithm) override;

    Return<void> encrypt(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keyId,
        const hidl_vec<uint8_t>& input,
        const hidl_vec<uint8_t>& iv,
        encrypt_cb _hidl_cb) override;

    Return<void> decrypt(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keyId,
        const hidl_vec<uint8_t>& input,
        const hidl_vec<uint8_t>& iv,
        decrypt_cb _hidl_cb) override;

    Return<void> sign(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keyId,
        const hidl_vec<uint8_t>& message,
        sign_cb _hidl_cb) override;

    Return<void> verify(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keyId,
        const hidl_vec<uint8_t>& message,
        const hidl_vec<uint8_t>& signature,
        verify_cb _hidl_cb) override;

    Return<void> signRSA(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_string& algorithm,
        const hidl_vec<uint8_t>& message,
        const hidl_vec<uint8_t>& wrappedKey,
        signRSA_cb _hidl_cb) override;

    Return<void> setListener(const sp<IDrmPluginListener_V1_0>& listener) override;

    Return<void> sendEvent(
        EventType eventType,
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& data) override;

    Return<void> sendExpirationUpdate(
        const hidl_vec<uint8_t>& sessionId,
        int64_t expiryTimeInMS) override;

    Return<void> sendKeysChange(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<KeyStatus>& keyStatusList,
        bool hasNewUsableKey) override;

    Return<void> openSession_1_1(
        SecurityLevel securityLevel, openSession_1_1_cb _hidl_cb) override;

    Return<void> getKeyRequest_1_1(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_1_1_cb _hidl_cb) override;

    Return<void> getHdcpLevels(getHdcpLevels_cb _hidl_cb) override;

    Return<void> getNumberOfSessions(getNumberOfSessions_cb _hidl_cb) override;

    Return<void> getSecurityLevel(
        const hidl_vec<uint8_t>& sessionId,
        getSecurityLevel_cb _hidl_cb) override;

    Return<void> getMetrics(getMetrics_cb _hidl_cb) override;

    Return<void> getSecureStopIds(getSecureStopIds_cb _hidl_cb) override;

    Return<Status> releaseSecureStops(const SecureStopRelease& ssRelease) override;

    Return<Status> removeSecureStop(const hidl_vec<uint8_t>& secureStopId) override;

    Return<Status> removeAllSecureStops() override;

    Return<void> getOfflineLicenseKeySetIds(
        getOfflineLicenseKeySetIds_cb _hidl_cb) override;

    Return<Status> removeOfflineLicense(const KeySetId& keySetId) override;

    Return<void> getOfflineLicenseState(
        const KeySetId& keySetId,
        getOfflineLicenseState_cb _hidl_cb) override;

    Return<void> getKeyRequest_1_2(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_1_2_cb _hidl_cb) override;

    Return<void> getProvisionRequest_1_2(
        const hidl_string& certificateType,
        const hidl_string& certificateAuthority,
        getProvisionRequest_1_2_cb _hidl_cb) override;

    Return<void> getHdcpLevels_1_2(getHdcpLevels_1_2_cb _hidl_cb) override;

    Return<void> sendSessionLostState(
        const hidl_vec<uint8_t>& sessionId) override;

    Return<void> sendKeysChange_1_2(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<KeyStatus_V1_2>& keyStatusList,
        bool hasNewUsableKey) override;

private:
    sp<IDrmPlugin_V1_0> mDelegate;
    sp<IDrmPluginListener_V1_0> mAppListener;
    sp<IDrmPluginListener_V1_2> mAppListenerV1_2;

    WvDrmPluginProxy(const WvDrmPluginProxy&) = delete;
    void operator=(const WvDrmPluginProxy&) = delete;
};

}
}
}
}

#endif
