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

#include "PictureAdjustment.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <unistd.h>

#include <cmath>
#include <string>
#include <vector>

using ::android::base::ReadFileToString;
using ::android::base::Split;
using ::android::base::StringPrintf;
using ::android::base::Trim;
using ::android::base::WriteStringToFile;
using ::vendor::lineage::livedisplay::V2_0::FloatRange;

namespace device {
namespace qcom {
namespace msm8909 {
namespace livedisplay {

namespace {
constexpr const char* kFileHsic = "/sys/class/graphics/fb0/hsic";

constexpr float kUnity = 512.0f;
constexpr float kSicMin = 0.0f;
constexpr float kSicMax = 2.0f;
constexpr float kHueMin = -180.0f;
constexpr float kHueMax = 180.0f;

int32_t sicToNode(float v) {
    if (v < kSicMin) v = kSicMin;
    if (v > kSicMax) v = kSicMax;
    return static_cast<int32_t>(std::lround(v * kUnity));
}
}

bool PictureAdjustment::isSupported() {
    return !access(kFileHsic, R_OK | W_OK);
}

Return<void> PictureAdjustment::getHueRange(getHueRange_cb _hidl_cb) {
    _hidl_cb(FloatRange{kHueMax, kHueMin, 1.0f});
    return Void();
}

Return<void> PictureAdjustment::getSaturationRange(getSaturationRange_cb _hidl_cb) {
    _hidl_cb(FloatRange{kSicMax, kSicMin, 0.01f});
    return Void();
}

Return<void> PictureAdjustment::getIntensityRange(getIntensityRange_cb _hidl_cb) {
    _hidl_cb(FloatRange{kSicMax, kSicMin, 0.01f});
    return Void();
}

Return<void> PictureAdjustment::getContrastRange(getContrastRange_cb _hidl_cb) {
    _hidl_cb(FloatRange{kSicMax, kSicMin, 0.01f});
    return Void();
}

Return<void> PictureAdjustment::getSaturationThresholdRange(
        getSaturationThresholdRange_cb _hidl_cb) {

    _hidl_cb(FloatRange{0.0f, 0.0f, 0.0f});
    return Void();
}

Return<void> PictureAdjustment::getPictureAdjustment(getPictureAdjustment_cb _hidl_cb) {
    HSIC hsic{};
    std::string tmp;

    if (ReadFileToString(kFileHsic, &tmp)) {
        std::vector<std::string> values = Split(Trim(tmp), " ");
        if (values.size() == 4) {
            hsic.hue = std::stof(values[0]);
            hsic.saturation = std::stof(values[1]) / kUnity;
            hsic.intensity = std::stof(values[2]) / kUnity;
            hsic.contrast = std::stof(values[3]) / kUnity;
        }
    }

    _hidl_cb(hsic);
    return Void();
}

Return<void> PictureAdjustment::getDefaultPictureAdjustment(
        getDefaultPictureAdjustment_cb _hidl_cb) {

    _hidl_cb(HSIC{0.0f, 1.0f, 1.0f, 1.0f, 0.0f});
    return Void();
}

Return<bool> PictureAdjustment::setPictureAdjustment(const HSIC& hsic) {
    int32_t hue = static_cast<int32_t>(std::lround(hsic.hue));

    return WriteStringToFile(StringPrintf("%d %d %d %d", hue, sicToNode(hsic.saturation),
                                          sicToNode(hsic.intensity), sicToNode(hsic.contrast)),
                             kFileHsic, true);
}

}
}
}
}
