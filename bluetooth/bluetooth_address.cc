//
// Copyright 2016 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "bluetooth_address.h"

#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <utils/Log.h>

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

void BluetoothAddress::bytes_to_string(const uint8_t* addr, char* addr_str) {
  sprintf(addr_str, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2],
          addr[3], addr[4], addr[5]);
}

bool BluetoothAddress::string_to_bytes(const char* addr_str, uint8_t* addr) {
  if (addr_str == NULL) return false;
  if (strnlen(addr_str, kStringLength) != kStringLength) return false;
  unsigned char trailing_char = '\0';

  return (sscanf(addr_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx%1c",
                 &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5],
                 &trailing_char) == kBytes);
}

static bool get_address_from_nv(uint8_t* addr) {
  static const char* const kNvPaths[] = {
      "/mnt/per/bluetooth/.bt_nv.bin",
      "/mnt/per/.bt_nv.bin",
  };
  int fd = -1;
  for (const char* path : kNvPaths) {
    fd = open(path, O_RDONLY);
    if (fd != -1) break;
  }
  if (fd == -1) return false;

  uint8_t nv[9] = {0};
  int n = read(fd, nv, sizeof(nv));
  close(fd);
  if (n != static_cast<int>(sizeof(nv))) return false;
  if (nv[0] != 0x01 || (nv[1] != 0x01 && nv[1] != 0x00) || nv[2] != 0x06)
    return false;

  for (size_t i = 0; i < BluetoothAddress::kBytes; i++) addr[i] = nv[8 - i];

  const uint8_t zero_bdaddr[BluetoothAddress::kBytes] = {0, 0, 0, 0, 0, 0};
  if (memcmp(addr, zero_bdaddr, BluetoothAddress::kBytes) == 0) return false;

  static char device[PROPERTY_VALUE_MAX] = {0};
  static bool device_loaded = false;

  if (!device_loaded) {
    property_get("ro.product.device", device, "");
    device_loaded = true;
  }

  if (strncmp(device, "a16", 3) == 0) {
    if (addr[0] == 0 && addr[1] == 0 && addr[2] == 0) {
      ALOGW("%s: NV blob has a null OUI (%02x:%02x:%02x:%02x:%02x:%02x) - "
            "treating it as unprogrammed for a16", __func__, addr[0], addr[1], addr[2],
            addr[3], addr[4], addr[5]);
      return false;
    }
  }
  return true;
}

bool BluetoothAddress::get_local_address(uint8_t* local_addr) {
  char property[PROPERTY_VALUE_MAX] = {0};

  if (get_address_from_nv(local_addr)) {
    ALOGI("%s: Got factory BDA from NV blob", __func__);
    return true;
  }

  if (property_get(PROPERTY_BT_BDADDR_PATH, property, NULL)) {
    ALOGD("%s: Trying %s", __func__, property);

    int addr_fd = open(property, O_RDONLY);
    if (addr_fd != -1) {
      char address[kStringLength + 1] = {0};
      int bytes_read = read(addr_fd, address, kStringLength);
      if (bytes_read == -1) {
        ALOGE("%s: Error reading address from %s: %s", __func__, property,
              strerror(errno));
      }
      close(addr_fd);

      address[kStringLength] = '\0';

      const uint8_t zero_bdaddr[kBytes] = {0, 0, 0, 0, 0, 0};
      if ((string_to_bytes(address, local_addr)) &&
          (memcmp(local_addr, zero_bdaddr, kBytes) != 0)) {
        ALOGD("%s: Got Factory BDA %s", __func__, address);
        return true;
      } else {
        ALOGE("%s: Got Invalid BDA '%s' from %s", __func__, address, property);
      }
    }
  }

  if (property_get(FACTORY_BDADDR_PROPERTY, property, NULL) &&
      string_to_bytes(property, local_addr)) {
    return true;
  }

  if (property_get(PERSIST_BDADDR_PROPERTY, property, NULL) &&
      string_to_bytes(property, local_addr)) {
    return true;
  }

  return false;
}

}
}
}
}
}
