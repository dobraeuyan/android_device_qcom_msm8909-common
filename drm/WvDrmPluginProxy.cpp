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

#define LOG_TAG "WvDrmPluginProxy"

#include "WvDrmPluginProxy.h"

#include "WvDrmListenerProxy.h"

#include <cstdlib>

namespace android {
namespace hardware {
namespace drm {
namespace widevine {

Return<void> WvDrmPluginProxy::openSession(openSession_cb _hidl_cb) {
    return mDelegate->openSession(_hidl_cb);
}

Return<Status> WvDrmPluginProxy::closeSession(const hidl_vec<uint8_t>& sessionId) {
    return mDelegate->closeSession(sessionId);
}

Return<void> WvDrmPluginProxy::getKeyRequest(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_cb _hidl_cb) {
    return mDelegate->getKeyRequest(scope, initData, mimeType, keyType,
                                    optionalParameters, _hidl_cb);
}

Return<void> WvDrmPluginProxy::provideKeyResponse(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& response,
        provideKeyResponse_cb _hidl_cb) {
    return mDelegate->provideKeyResponse(scope, response, _hidl_cb);
}

Return<Status> WvDrmPluginProxy::removeKeys(const hidl_vec<uint8_t>& sessionId) {
    return mDelegate->removeKeys(sessionId);
}

Return<Status> WvDrmPluginProxy::restoreKeys(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keySetId) {
    return mDelegate->restoreKeys(sessionId, keySetId);
}

Return<void> WvDrmPluginProxy::queryKeyStatus(
        const hidl_vec<uint8_t>& sessionId,
        queryKeyStatus_cb _hidl_cb) {
    return mDelegate->queryKeyStatus(sessionId, _hidl_cb);
}

Return<void> WvDrmPluginProxy::getProvisionRequest(
        const hidl_string& certificateType,
        const hidl_string& certificateAuthority,
        getProvisionRequest_cb _hidl_cb) {
    return mDelegate->getProvisionRequest(certificateType, certificateAuthority,
                                          _hidl_cb);
}

Return<void> WvDrmPluginProxy::provideProvisionResponse(
        const hidl_vec<uint8_t>& response,
        provideProvisionResponse_cb _hidl_cb) {
    return mDelegate->provideProvisionResponse(response, _hidl_cb);
}

Return<void> WvDrmPluginProxy::getSecureStops(getSecureStops_cb _hidl_cb) {
    return mDelegate->getSecureStops(_hidl_cb);
}

Return<void> WvDrmPluginProxy::getSecureStop(
        const hidl_vec<uint8_t>& secureStopId,
        getSecureStop_cb _hidl_cb) {
    return mDelegate->getSecureStop(secureStopId, _hidl_cb);
}

Return<Status> WvDrmPluginProxy::releaseAllSecureStops() {
    return mDelegate->releaseAllSecureStops();
}

Return<Status> WvDrmPluginProxy::releaseSecureStop(
        const hidl_vec<uint8_t>& secureStopId) {
    return mDelegate->releaseSecureStop(secureStopId);
}

Return<void> WvDrmPluginProxy::getPropertyString(
        const hidl_string& propertyName,
        getPropertyString_cb _hidl_cb) {
    return mDelegate->getPropertyString(propertyName, _hidl_cb);
}

Return<void> WvDrmPluginProxy::getPropertyByteArray(
        const hidl_string& propertyName,
        getPropertyByteArray_cb _hidl_cb) {
    return mDelegate->getPropertyByteArray(propertyName, _hidl_cb);
}

Return<Status> WvDrmPluginProxy::setPropertyString(
        const hidl_string& propertyName, const hidl_string& value) {
    return mDelegate->setPropertyString(propertyName, value);
}

Return<Status> WvDrmPluginProxy::setPropertyByteArray(
        const hidl_string& propertyName, const hidl_vec<uint8_t>& value) {
    return mDelegate->setPropertyByteArray(propertyName, value);
}

Return<Status> WvDrmPluginProxy::setCipherAlgorithm(
        const hidl_vec<uint8_t>& sessionId, const hidl_string& algorithm) {
    return mDelegate->setCipherAlgorithm(sessionId, algorithm);
}

Return<Status> WvDrmPluginProxy::setMacAlgorithm(
        const hidl_vec<uint8_t>& sessionId, const hidl_string& algorithm) {
    return mDelegate->setMacAlgorithm(sessionId, algorithm);
}

Return<void> WvDrmPluginProxy::encrypt(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keyId,
        const hidl_vec<uint8_t>& input,
        const hidl_vec<uint8_t>& iv,
        encrypt_cb _hidl_cb) {
    return mDelegate->encrypt(sessionId, keyId, input, iv, _hidl_cb);
}

Return<void> WvDrmPluginProxy::decrypt(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keyId,
        const hidl_vec<uint8_t>& input,
        const hidl_vec<uint8_t>& iv,
        decrypt_cb _hidl_cb) {
    return mDelegate->decrypt(sessionId, keyId, input, iv, _hidl_cb);
}

Return<void> WvDrmPluginProxy::sign(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keyId,
        const hidl_vec<uint8_t>& message,
        sign_cb _hidl_cb) {
    return mDelegate->sign(sessionId, keyId, message, _hidl_cb);
}

Return<void> WvDrmPluginProxy::verify(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& keyId,
        const hidl_vec<uint8_t>& message,
        const hidl_vec<uint8_t>& signature,
        verify_cb _hidl_cb) {
    return mDelegate->verify(sessionId, keyId, message, signature, _hidl_cb);
}

Return<void> WvDrmPluginProxy::signRSA(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_string& algorithm,
        const hidl_vec<uint8_t>& message,
        const hidl_vec<uint8_t>& wrappedKey,
        signRSA_cb _hidl_cb) {
    return mDelegate->signRSA(sessionId, algorithm, message, wrappedKey, _hidl_cb);
}

Return<void> WvDrmPluginProxy::setListener(
        const sp<IDrmPluginListener_V1_0>& listener) {
    mAppListener = listener;
    mAppListenerV1_2 = IDrmPluginListener_V1_2::castFrom(listener);
    sp<WvDrmListenerProxy> shim = new WvDrmListenerProxy(listener);
    return mDelegate->setListener(shim);
}

Return<void> WvDrmPluginProxy::sendEvent(
        EventType eventType,
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<uint8_t>& data) {
    return mDelegate->sendEvent(eventType, sessionId, data);
}

Return<void> WvDrmPluginProxy::sendExpirationUpdate(
        const hidl_vec<uint8_t>& sessionId,
        int64_t expiryTimeInMS) {
    return mDelegate->sendExpirationUpdate(sessionId, expiryTimeInMS);
}

Return<void> WvDrmPluginProxy::sendKeysChange(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<KeyStatus>& keyStatusList,
        bool hasNewUsableKey) {
    return mDelegate->sendKeysChange(sessionId, keyStatusList, hasNewUsableKey);
}

// ===========================================================================

// ===========================================================================

Return<void> WvDrmPluginProxy::openSession_1_1(
        SecurityLevel securityLevel, openSession_1_1_cb _hidl_cb) {
    if (static_cast<uint32_t>(securityLevel) >
            static_cast<uint32_t>(SecurityLevel::SW_SECURE_CRYPTO)) {
        _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, hidl_vec<uint8_t>());
        return Void();
    }
    return mDelegate->openSession(_hidl_cb);
}

Return<void> WvDrmPluginProxy::getKeyRequest_1_1(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_1_1_cb _hidl_cb) {
    return mDelegate->getKeyRequest(
        scope, initData, mimeType, keyType, optionalParameters,
        [&](Status status, const hidl_vec<uint8_t>& request,
            KeyRequestType_V1_0 requestType, const hidl_string& defaultUrl) {

            _hidl_cb(status, request,
                     static_cast<KeyRequestType_V1_1>(requestType), defaultUrl);
        });
}

Return<void> WvDrmPluginProxy::getHdcpLevels(getHdcpLevels_cb _hidl_cb) {
    _hidl_cb(Status::OK, HdcpLevel::HDCP_NONE, HdcpLevel::HDCP_NO_OUTPUT);
    return Void();
}

Return<void> WvDrmPluginProxy::getNumberOfSessions(
        getNumberOfSessions_cb _hidl_cb) {
    uint32_t currentSessions = 0;
    uint32_t maxSessions = 16;

    mDelegate->getPropertyString(
        "numberOfOpenSessions",
        [&](Status status, const hidl_string& value) {
            if (status == Status::OK && value.size() > 0) {
                currentSessions = static_cast<uint32_t>(atoi(value.c_str()));
            }
        });

    mDelegate->getPropertyString(
        "maxNumberOfSessions",
        [&](Status status, const hidl_string& value) {
            if (status == Status::OK && value.size() > 0) {
                maxSessions = static_cast<uint32_t>(atoi(value.c_str()));
            }
        });

    _hidl_cb(Status::OK, currentSessions, maxSessions);
    return Void();
}

Return<void> WvDrmPluginProxy::getSecurityLevel(
        const hidl_vec<uint8_t>& sessionId,
        getSecurityLevel_cb _hidl_cb) {
    (void)sessionId;

    SecurityLevel level = SecurityLevel::SW_SECURE_CRYPTO;
    mDelegate->getPropertyString(
        "securityLevel",
        [&](Status status, const hidl_string& value) {
            if (status == Status::OK) {
                if (value == "L1") {
                    level = SecurityLevel::HW_SECURE_ALL;
                } else {

                    level = SecurityLevel::SW_SECURE_CRYPTO;
                }
            }
        });

    _hidl_cb(Status::OK, level);
    return Void();
}

Return<void> WvDrmPluginProxy::getMetrics(getMetrics_cb _hidl_cb) {
    _hidl_cb(Status::OK, hidl_vec<DrmMetricGroup>());
    return Void();
}

Return<void> WvDrmPluginProxy::getSecureStopIds(getSecureStopIds_cb _hidl_cb) {

    _hidl_cb(Status::OK, hidl_vec<SecureStopId>());
    return Void();
}

Return<Status> WvDrmPluginProxy::releaseSecureStops(
        const SecureStopRelease& ssRelease) {

    return mDelegate->releaseSecureStop(ssRelease.opaqueData);
}

Return<Status> WvDrmPluginProxy::removeSecureStop(
        const hidl_vec<uint8_t>& secureStopId) {
    (void)secureStopId;

    return Status::ERROR_DRM_CANNOT_HANDLE;
}

Return<Status> WvDrmPluginProxy::removeAllSecureStops() {
    return mDelegate->releaseAllSecureStops();
}

// ===========================================================================

// ===========================================================================

Return<void> WvDrmPluginProxy::getOfflineLicenseKeySetIds(
        getOfflineLicenseKeySetIds_cb _hidl_cb) {
    _hidl_cb(Status::OK, hidl_vec<KeySetId>());
    return Void();
}

Return<Status> WvDrmPluginProxy::removeOfflineLicense(const KeySetId& keySetId) {
    (void)keySetId;
    return Status::ERROR_DRM_CANNOT_HANDLE;
}

Return<void> WvDrmPluginProxy::getOfflineLicenseState(
        const KeySetId& keySetId,
        getOfflineLicenseState_cb _hidl_cb) {
    (void)keySetId;
    _hidl_cb(Status::ERROR_DRM_CANNOT_HANDLE, OfflineLicenseState::UNKNOWN);
    return Void();
}

Return<void> WvDrmPluginProxy::getKeyRequest_1_2(
        const hidl_vec<uint8_t>& scope,
        const hidl_vec<uint8_t>& initData,
        const hidl_string& mimeType,
        KeyType keyType,
        const hidl_vec<KeyValue>& optionalParameters,
        getKeyRequest_1_2_cb _hidl_cb) {
    return mDelegate->getKeyRequest(
        scope, initData, mimeType, keyType, optionalParameters,
        [&](Status status, const hidl_vec<uint8_t>& request,
            KeyRequestType_V1_0 requestType, const hidl_string& defaultUrl) {
            _hidl_cb(static_cast<Status_V1_2>(status), request,
                     static_cast<KeyRequestType_V1_1>(requestType), defaultUrl);
        });
}

Return<void> WvDrmPluginProxy::getProvisionRequest_1_2(
        const hidl_string& certificateType,
        const hidl_string& certificateAuthority,
        getProvisionRequest_1_2_cb _hidl_cb) {
    return mDelegate->getProvisionRequest(
        certificateType, certificateAuthority,
        [&](Status status, const hidl_vec<uint8_t>& request,
            const hidl_string& defaultUrl) {
            _hidl_cb(static_cast<Status_V1_2>(status), request, defaultUrl);
        });
}

Return<void> WvDrmPluginProxy::getHdcpLevels_1_2(getHdcpLevels_1_2_cb _hidl_cb) {
    _hidl_cb(Status_V1_2::OK, HdcpLevel_V1_2::HDCP_NONE,
             HdcpLevel_V1_2::HDCP_NO_OUTPUT);
    return Void();
}

Return<void> WvDrmPluginProxy::sendSessionLostState(
        const hidl_vec<uint8_t>& sessionId) {
    if (mAppListenerV1_2 != nullptr) {
        mAppListenerV1_2->sendSessionLostState(sessionId);
    }
    return Void();
}

Return<void> WvDrmPluginProxy::sendKeysChange_1_2(
        const hidl_vec<uint8_t>& sessionId,
        const hidl_vec<KeyStatus_V1_2>& keyStatusList,
        bool hasNewUsableKey) {
    if (mAppListenerV1_2 != nullptr) {
        mAppListenerV1_2->sendKeysChange_1_2(sessionId, keyStatusList,
                                             hasNewUsableKey);
    }
    return Void();
}

}
}
}
}
