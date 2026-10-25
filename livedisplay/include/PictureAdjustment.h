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

#ifndef MSM8909_LIVEDISPLAY_PICTUREADJUSTMENT_H
#define MSM8909_LIVEDISPLAY_PICTUREADJUSTMENT_H

#include <vendor/lineage/livedisplay/2.0/IPictureAdjustment.h>

namespace device {
namespace qcom {
namespace msm8909 {
namespace livedisplay {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::vendor::lineage::livedisplay::V2_0::HSIC;
using ::vendor::lineage::livedisplay::V2_0::IPictureAdjustment;

class PictureAdjustment : public IPictureAdjustment {
  public:
    static bool isSupported();

    Return<void> getHueRange(getHueRange_cb _hidl_cb) override;
    Return<void> getSaturationRange(getSaturationRange_cb _hidl_cb) override;
    Return<void> getIntensityRange(getIntensityRange_cb _hidl_cb) override;
    Return<void> getContrastRange(getContrastRange_cb _hidl_cb) override;
    Return<void> getSaturationThresholdRange(getSaturationThresholdRange_cb _hidl_cb) override;
    Return<void> getPictureAdjustment(getPictureAdjustment_cb _hidl_cb) override;
    Return<void> getDefaultPictureAdjustment(getDefaultPictureAdjustment_cb _hidl_cb) override;
    Return<bool> setPictureAdjustment(const HSIC& hsic) override;
};

}
}
}
}

#endif
