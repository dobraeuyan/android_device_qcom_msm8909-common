/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ******************************************************************************/

#ifndef BT_VENDOR_LIB_H
#define BT_VENDOR_LIB_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {

  BT_VND_OP_POWER_CTRL,

  BT_VND_OP_FW_CFG,

  BT_VND_OP_SCO_CFG,

  BT_VND_OP_USERIAL_OPEN,

  BT_VND_OP_USERIAL_CLOSE,

  BT_VND_OP_GET_LPM_IDLE_TIMEOUT,

  BT_VND_OP_LPM_SET_MODE,

  BT_VND_OP_LPM_WAKE_SET_STATE,

  BT_VND_OP_SET_AUDIO_STATE,

  BT_VND_OP_EPILOG,

  BT_VND_OP_A2DP_OFFLOAD_START,

  BT_VND_OP_A2DP_OFFLOAD_STOP,

} bt_vendor_opcode_t;

typedef enum {
  BT_VND_PWR_OFF,
  BT_VND_PWR_ON,
} bt_vendor_power_state_t;

typedef enum {
  CH_CMD,
  CH_EVT,
  CH_ACL_OUT,
  CH_ACL_IN,

  CH_MAX
} bt_vendor_hci_channels_t;

typedef enum {
  BT_VND_LPM_DISABLE,
  BT_VND_LPM_ENABLE,
} bt_vendor_lpm_mode_t;

typedef enum {
  BT_VND_LPM_WAKE_ASSERT,
  BT_VND_LPM_WAKE_DEASSERT,
} bt_vendor_lpm_wake_state_t;

typedef enum {
  BT_VND_OP_RESULT_SUCCESS,
  BT_VND_OP_RESULT_FAIL,
} bt_vendor_op_result_t;

typedef struct {
  uint16_t handle;
  uint16_t peer_codec;
  uint16_t state;
} bt_vendor_op_audio_state_t;

typedef void (*cfg_result_cb)(bt_vendor_op_result_t result);

typedef void* (*malloc_cb)(int size);

typedef void (*mdealloc_cb)(void* p_buf);

typedef void (*tINT_CMD_CBACK)(void* p_mem);

typedef uint8_t (*cmd_xmit_cb)(uint16_t opcode, void* p_buf,
                               tINT_CMD_CBACK p_cback);

typedef void (*cfg_a2dp_cb)(bt_vendor_op_result_t result, bt_vendor_opcode_t op,
                            uint8_t bta_av_handle);

typedef struct {

  size_t size;

  cfg_result_cb fwcfg_cb;

  cfg_result_cb scocfg_cb;

  cfg_result_cb lpm_cb;

  cfg_result_cb audio_state_cb;

  malloc_cb alloc;

  mdealloc_cb dealloc;

  cmd_xmit_cb xmit_cb;

  cfg_result_cb epilog_cb;

  cfg_a2dp_cb a2dp_offload_cb;
} bt_vendor_callbacks_t;

typedef struct {
  uint8_t bta_av_handle;
  uint16_t xmit_quota;
  uint16_t acl_data_size;
  uint16_t stream_mtu;
  uint16_t local_cid;
  uint16_t remote_cid;
  uint16_t lm_handle;
  uint8_t is_flushable;
  uint32_t stream_source;
  uint8_t codec_info[10];
} bt_vendor_op_a2dp_offload_t;

typedef struct {

  size_t size;

  int (*init)(const bt_vendor_callbacks_t* p_cb, unsigned char* local_bdaddr);

  int (*op)(bt_vendor_opcode_t opcode, void* param);

  void (*cleanup)(void);
} bt_vendor_interface_t;

extern const bt_vendor_interface_t BLUETOOTH_VENDOR_LIB_INTERFACE;

typedef struct {
  uint16_t event;
  uint16_t len;
  uint16_t offset;
  uint16_t layer_specific;
  uint8_t data[];
} HC_BT_HDR;

#ifdef __cplusplus
}
#endif

#endif
