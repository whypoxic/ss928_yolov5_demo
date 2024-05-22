/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_NPU_MODEL_H
#define SAMPLE_NPU_MODEL_H
#include <stddef.h>
#include "ot_type.h"
#include "acl.h"
#include "ss_mpi_sys.h"

#define MAX_THREAD_NUM 20
#define MAX_INPUT_NUM 5
#define MAX_OUTPUT_NUM 5

typedef struct npu_acl_model {
    td_u32 model_id;
    td_ulong model_mem_size;
    td_ulong model_weight_size;
    td_void *model_mem_ptr;
    td_void *model_weight_ptr;
    td_phys_addr_t model_mem_phy_addr;
    td_phys_addr_t model_weight_phy_addr;
    td_bool is_load_flag;
    aclmdlDesc *model_desc;
    aclmdlDataset *input_dataset;
    aclmdlDataset *output_dataset;
    td_phys_addr_t output_phy_addr[MAX_OUTPUT_NUM];
    td_phys_addr_t input_phy_addr[MAX_INPUT_NUM];
} npu_acl_model_t;

td_s32 sample_npu_load_model_with_mem(const char *model_path, td_u32 model_index);
td_s32 sample_npu_load_model_with_mem_cached(const char *model_path, td_u32 model_index);

td_s32 sample_npu_create_desc(td_u32 model_index);
td_void sample_npu_destroy_desc(td_u32 model_index);

td_s32 sample_npu_get_input_size_by_index(const td_u32 index, size_t *inputSize, td_u32 model_index);
td_s32 sample_npu_create_input_dataset(td_u32 model_index);
td_void sample_npu_destroy_input_dataset(td_u32 model_index);
td_s32 sample_npu_create_input_databuf(td_void *data_buf, size_t data_len, td_u32 model_index);
td_void sample_npu_destroy_input_databuf(td_u32 model_index);

td_s32 sample_npu_create_output(td_u32 model_index);
td_void sample_npu_output_model_result(td_u32 model_index);
td_void sample_npu_destroy_output(td_u32 model_index);

td_s32 sample_npu_model_execute(td_u32 model_index);

td_void sample_npu_unload_model(td_u32 model_index);
td_void sample_npu_unload_model_cached(td_u32 model_index);

td_s32 sample_npu_create_cached_input(td_u32 model_index);
td_s32 sample_npu_create_cached_output(td_u32 model_index);
td_void sample_npu_destroy_cached_input(td_u32 model_index);
td_void sample_npu_destroy_cached_output(td_u32 model_index);

td_s32 sample_svp_npu_loop_execute_dynamicbatch(td_u32 model_index);
#endif
