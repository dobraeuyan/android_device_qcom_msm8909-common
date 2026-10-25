# Copyright (C) 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libbt-vendor_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/NOTICE_FILES/src/system/vendor/lib/libbt-vendor.so.txt)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/lib/libbt-vendor.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/symbols/system/vendor/lib/libbt-vendor.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/vendor/lib/libbt-vendor.so)
