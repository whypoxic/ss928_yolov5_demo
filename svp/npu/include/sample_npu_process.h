/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_NPU_PROCESS_H
#define SAMPLE_NPU_PROCESS_H

#include "ot_type.h"

/* function : show the sample of acl resnet50 */
td_void sample_svp_npu_acl_resnet50(td_void);

/* function : show the sample of acl resnet50_multithread */
td_void sample_svp_npu_acl_resnet50_multithread(td_void);

/* function : show the sample of acl resnet50 sign handle */
td_void sample_svp_npu_acl_resnet50_handle_sig(td_void);

/* function : show the sample of acl mobilenet_v3 dyanamicbatch with mmz cached */
td_void sample_svp_npu_acl_mobilenet_v3_dynamicbatch(td_void);

#endif
