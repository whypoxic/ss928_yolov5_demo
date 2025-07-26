/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_COMMON_SVP_NPU_MODEL_H
#define SAMPLE_COMMON_SVP_NPU_MODEL_H
#include "ot_type.h"
#include "svp_acl_mdl.h"
#include "sample_common_svp.h"
#include "sample_common_svp_npu.h"

td_s32 sample_common_svp_npu_get_input_data(const td_char *src[], td_u32 file_num,
    const sample_svp_npu_task_info *task);

td_s32 sample_common_svp_npu_load_model(const td_char *model_path, td_u32 model_index, td_bool is_cached);
td_void sample_common_svp_npu_unload_model(td_u32 model_index);

td_s32 sample_common_svp_npu_create_input(sample_svp_npu_task_info *task);
td_s32 sample_common_svp_npu_create_output(sample_svp_npu_task_info *task);
td_void sample_common_svp_npu_destroy_input(sample_svp_npu_task_info *task);
td_void sample_common_svp_npu_destroy_output(sample_svp_npu_task_info *task);

td_s32 sample_common_svp_npu_create_task_buf(sample_svp_npu_task_info *task);
td_s32 sample_common_svp_npu_create_work_buf(sample_svp_npu_task_info *task);
td_void sample_common_svp_npu_destroy_task_buf(sample_svp_npu_task_info *task);
td_void sample_common_svp_npu_destroy_work_buf(sample_svp_npu_task_info *task);

td_s32 sample_common_svp_npu_get_work_buf_info(const sample_svp_npu_task_info *task,
    td_u32 *work_buf_size, td_u32 *work_buf_stride);
td_s32 sample_common_svp_npu_share_work_buf(const sample_svp_npu_shared_work_buf *shared_work_buf,
    const sample_svp_npu_task_info *task);

td_s32 sample_common_svp_npu_set_dynamic_batch(const sample_svp_npu_task_info *task);
td_s32 sample_common_svp_npu_model_execute(const sample_svp_npu_task_info *task);
td_void sample_common_svp_npu_output_classification_result(const sample_svp_npu_task_info *task);

td_void sample_common_svp_npu_dump_task_data(const sample_svp_npu_task_info *task);

td_s32 sample_common_svp_npu_set_threshold(sample_svp_npu_threshold threshold[], td_u32 threshold_num,
    const sample_svp_npu_task_info *task);

td_s32 sample_common_svp_npu_roi_to_rect(const sample_svp_npu_task_info *task,
    sample_svp_npu_detection_info *detection_info, const ot_video_frame_info *proc_frame,
    const ot_video_frame_info *show_frame, ot_sample_svp_rect_info *rect_info);

td_s32 sample_common_svp_npu_update_input_data_buffer_info(td_u8 *virt_addr, td_u32 size, td_u32 stride, td_u32 idx,
    const sample_svp_npu_task_info *task);

td_s32 sample_common_svp_npu_get_input_data_buffer_info(const sample_svp_npu_task_info *task, td_u32 idx,
    td_u8 **virt_addr, td_u32 *size, td_u32 *stride);

td_s32 sample_common_svp_npu_check_has_aicpu_task(const sample_svp_npu_task_info *task, td_bool *has_aicpu_task);

sample_svp_npu_model_info* sample_common_svp_npu_get_model_info(td_u32 model_idx);
#endif
