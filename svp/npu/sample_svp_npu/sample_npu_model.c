/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <math.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ot_common_svp.h"
#include "sample_common_svp.h"
#include "sample_npu_model.h"

static npu_acl_model_t g_npu_acl_model[MAX_THREAD_NUM] = {0};

td_s32 sample_npu_load_model_with_mem(const char *model_path, td_u32 model_index)
{
    if (g_npu_acl_model[model_index].is_load_flag) {
        sample_svp_trace_err("has already loaded a model\n");
        return TD_FAILURE;
    }

    td_s32 ret = aclmdlQuerySize(model_path, &g_npu_acl_model[model_index].model_mem_size,
        &g_npu_acl_model[model_index].model_weight_size);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("query model failed, model file is %s\n", model_path);
        return TD_FAILURE;
    }

    ret = aclrtMalloc(&g_npu_acl_model[model_index].model_mem_ptr, g_npu_acl_model[model_index].model_mem_size,
        ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("malloc buffer for mem failed, require size is %lu\n",
            g_npu_acl_model[model_index].model_mem_size);
        return TD_FAILURE;
    }

    ret = aclrtMalloc(&g_npu_acl_model[model_index].model_weight_ptr, g_npu_acl_model[model_index].model_weight_size,
        ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("malloc buffer for weight fail, require size is %lu\n",
            g_npu_acl_model[model_index].model_weight_size);
        return TD_FAILURE;
    }

    ret = aclmdlLoadFromFileWithMem(model_path, &g_npu_acl_model[model_index].model_id,
        g_npu_acl_model[model_index].model_mem_ptr, g_npu_acl_model[model_index].model_mem_size,
        g_npu_acl_model[model_index].model_weight_ptr, g_npu_acl_model[model_index].model_weight_size);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("load model from file failed, model file is %s\n", model_path);
        return TD_FAILURE;
    }

    sample_svp_trace_info("load mem_size:%lu weight_size:%lu id:%d\n", g_npu_acl_model[model_index].model_mem_size,
        g_npu_acl_model[model_index].model_weight_size, g_npu_acl_model[model_index].model_id);

    g_npu_acl_model[model_index].is_load_flag = TD_TRUE;
    sample_svp_trace_info("load model %s success\n", model_path);

    return TD_SUCCESS;
}

td_s32 sample_npu_load_model_with_mem_cached(const char *model_path, td_u32 model_index)
{
    if (g_npu_acl_model[model_index].is_load_flag) {
        sample_svp_trace_err("has already loaded a model\n");
        return TD_FAILURE;
    }

    td_s32 ret = aclmdlQuerySize(model_path, &g_npu_acl_model[model_index].model_mem_size,
        &g_npu_acl_model[model_index].model_weight_size);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("query model failed, model file is %s\n", model_path);
        return TD_FAILURE;
    }

    ret = ss_mpi_sys_mmz_alloc_cached(&g_npu_acl_model[model_index].model_mem_phy_addr,
        &g_npu_acl_model[model_index].model_mem_ptr, "model_mem", NULL, g_npu_acl_model[model_index].model_mem_size);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("malloc buffer for mem failed\n");
        return TD_FAILURE;
    }
    memset_s(g_npu_acl_model[model_index].model_mem_ptr, g_npu_acl_model[model_index].model_mem_size, 0,
        g_npu_acl_model[model_index].model_mem_size);
    ss_mpi_sys_flush_cache(g_npu_acl_model[model_index].model_mem_phy_addr, g_npu_acl_model[model_index].model_mem_ptr,
        g_npu_acl_model[model_index].model_mem_size);

    ret = ss_mpi_sys_mmz_alloc_cached(&g_npu_acl_model[model_index].model_weight_phy_addr,
        &g_npu_acl_model[model_index].model_weight_ptr, "model_weight",
        NULL, g_npu_acl_model[model_index].model_weight_size);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("malloc buffer for weight fail\n");
        return TD_FAILURE;
    }
    memset_s(g_npu_acl_model[model_index].model_weight_ptr, g_npu_acl_model[model_index].model_weight_size, 0,
        g_npu_acl_model[model_index].model_weight_size);
    ss_mpi_sys_flush_cache(g_npu_acl_model[model_index].model_weight_phy_addr,
        g_npu_acl_model[model_index].model_weight_ptr, g_npu_acl_model[model_index].model_weight_size);

    ret = aclmdlLoadFromFileWithMem(model_path, &g_npu_acl_model[model_index].model_id,
        g_npu_acl_model[model_index].model_mem_ptr, g_npu_acl_model[model_index].model_mem_size,
        g_npu_acl_model[model_index].model_weight_ptr, g_npu_acl_model[model_index].model_weight_size);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("load model from file failed, model file is %s\n", model_path);
        return TD_FAILURE;
    }

    sample_svp_trace_info("load mem_size:%lu weight_size:%lu id:%d\n", g_npu_acl_model[model_index].model_mem_size,
        g_npu_acl_model[model_index].model_weight_size, g_npu_acl_model[model_index].model_id);

    g_npu_acl_model[model_index].is_load_flag = TD_TRUE;
    sample_svp_trace_info("load model %s success\n", model_path);

    return TD_SUCCESS;
}

td_s32 sample_npu_create_desc(td_u32 model_index)
{
    td_s32 ret;

    g_npu_acl_model[model_index].model_desc = aclmdlCreateDesc();
    if (g_npu_acl_model[model_index].model_desc == TD_NULL) {
        sample_svp_trace_err("create model description failed\n");
        return TD_FAILURE;
    }

    ret = aclmdlGetDesc(g_npu_acl_model[model_index].model_desc, g_npu_acl_model[model_index].model_id);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("get model description failed\n");
        return TD_FAILURE;
    }

    sample_svp_trace_info("create model description success\n");

    return TD_SUCCESS;
}

td_void sample_npu_destroy_desc(td_u32 model_index)
{
    if (g_npu_acl_model[model_index].model_desc != TD_NULL) {
        (td_void)aclmdlDestroyDesc(g_npu_acl_model[model_index].model_desc);
        g_npu_acl_model[model_index].model_desc = TD_NULL;
    }

    sample_svp_trace_info("destroy model description success\n");
}

td_s32 sample_npu_get_input_size_by_index(const td_u32 index, size_t *input_size, td_u32 model_index)
{
    if (g_npu_acl_model[model_index].model_desc == TD_NULL) {
        sample_svp_trace_err("no model description, create input failed\n");
        return TD_FAILURE;
    }

    *input_size = aclmdlGetInputSizeByIndex(g_npu_acl_model[model_index].model_desc, index);

    return TD_SUCCESS;
}

td_s32 sample_npu_create_input_dataset(td_u32 model_index)
{
    /* om used in this sample has only one input */
    if (g_npu_acl_model[model_index].model_desc == TD_NULL) {
        sample_svp_trace_err("no model description, create input failed\n");
        return TD_FAILURE;
    }

    g_npu_acl_model[model_index].input_dataset = aclmdlCreateDataset();
    if (g_npu_acl_model[model_index].input_dataset == TD_NULL) {
        sample_svp_trace_err("can't create dataset, create input failed\n");
        return TD_FAILURE;
    }

    sample_svp_trace_info("create model input dataset success\n");
    return TD_SUCCESS;
}

td_void sample_npu_destroy_input_dataset(td_u32 model_index)
{
    if (g_npu_acl_model[model_index].input_dataset == TD_NULL) {
        return;
    }

    aclmdlDestroyDataset(g_npu_acl_model[model_index].input_dataset);
    g_npu_acl_model[model_index].input_dataset = TD_NULL;

    sample_svp_trace_info("destroy model input dataset success\n");
}

td_s32 sample_npu_create_input_databuf(td_void *data_buf, size_t data_len, td_u32 model_index)
{
    /* om used in this sample has only one input */
    if (g_npu_acl_model[model_index].model_desc == TD_NULL) {
        sample_svp_trace_err("no model description, create input failed\n");
        return TD_FAILURE;
    }

    size_t input_size = aclmdlGetInputSizeByIndex(g_npu_acl_model[model_index].model_desc, 0);
    if (data_len != input_size) {
        sample_svp_trace_err("input image size[%zu] != model input size[%zu]\n", data_len, input_size);
        return TD_FAILURE;
    }

    aclDataBuffer *input_data = aclCreateDataBuffer(data_buf, data_len);
    if (input_data == TD_NULL) {
        sample_svp_trace_err("can't create data buffer, create input failed\n");
        return TD_FAILURE;
    }

    aclError ret = aclmdlAddDatasetBuffer(g_npu_acl_model[model_index].input_dataset, input_data);
    if (ret != ACL_SUCCESS) {
        sample_svp_trace_err("add input dataset buffer failed, ret is %d\n", ret);
        (void)aclDestroyDataBuffer(input_data);
        input_data = TD_NULL;
        return TD_FAILURE;
    }
    sample_svp_trace_info("create model input success\n");

    return TD_SUCCESS;
}

td_void sample_npu_destroy_input_databuf(td_u32 model_index)
{
    td_u32 i;

    if (g_npu_acl_model[model_index].input_dataset == TD_NULL) {
        return;
    }

    for (i = 0; i < aclmdlGetDatasetNumBuffers(g_npu_acl_model[model_index].input_dataset); ++i) {
        aclDataBuffer *data_buffer = aclmdlGetDatasetBuffer(g_npu_acl_model[model_index].input_dataset, i);
        aclDestroyDataBuffer(data_buffer);
    }

    sample_svp_trace_info("destroy model input data buf success\n");
}

td_s32 sample_npu_create_cached_input(td_u32 model_index)
{
    td_u32 input_size;
    if (g_npu_acl_model[model_index].model_desc == TD_NULL) {
        sample_svp_trace_err("no model description, create input failed\n");
        return TD_FAILURE;
    }

    g_npu_acl_model[model_index].input_dataset = aclmdlCreateDataset();
    if (g_npu_acl_model[model_index].input_dataset == TD_NULL) {
        sample_svp_trace_err("can't create dataset, create input failed\n");
        return TD_FAILURE;
    }

    input_size = aclmdlGetNumInputs(g_npu_acl_model[model_index].model_desc);
    for (td_u32 i = 0; i < input_size; i++) {
        td_u32 buffer_size = aclmdlGetInputSizeByIndex(g_npu_acl_model[model_index].model_desc, i);
        td_void *input_buffer = TD_NULL;

        td_s32 ret = ss_mpi_sys_mmz_alloc_cached(&g_npu_acl_model[model_index].input_phy_addr[i],
            &input_buffer, "input", NULL, buffer_size);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("can't malloc buffer, size is %u, create input failed\n", buffer_size);
            return TD_FAILURE;
        }
        memset_s(input_buffer, buffer_size, 0, buffer_size);
        ss_mpi_sys_flush_cache(g_npu_acl_model[model_index].input_phy_addr[i], input_buffer, buffer_size);

        aclDataBuffer *input_data = aclCreateDataBuffer(input_buffer, buffer_size);
        if (input_data == TD_NULL) {
            sample_svp_trace_err("can't create data buffer, create input failed\n");
            aclrtFree(input_buffer);
            return TD_FAILURE;
        }

        ret = aclmdlAddDatasetBuffer(g_npu_acl_model[model_index].input_dataset, input_data);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("can't add data buffer, create input failed\n");
            aclrtFree(input_buffer);
            aclDestroyDataBuffer(input_data);
            return TD_FAILURE;
        }
    }

    sample_svp_trace_info("create model input cached TD_SUCCESS\n");
    return TD_SUCCESS;
}

td_s32 sample_npu_create_cached_output(td_u32 model_index)
{
    td_u32 output_size;

    if (g_npu_acl_model[model_index].model_desc == TD_NULL) {
        sample_svp_trace_err("no model description, create output failed\n");
        return TD_FAILURE;
    }

    g_npu_acl_model[model_index].output_dataset = aclmdlCreateDataset();
    if (g_npu_acl_model[model_index].output_dataset == TD_NULL) {
        sample_svp_trace_err("can't create dataset, create output failed\n");
        return TD_FAILURE;
    }

    output_size = aclmdlGetNumOutputs(g_npu_acl_model[model_index].model_desc);
    for (td_u32 i = 0; i < output_size; ++i) {
        td_u32 buffer_size = aclmdlGetOutputSizeByIndex(g_npu_acl_model[model_index].model_desc, i);
        td_void *output_buffer = TD_NULL;

        td_s32 ret = ss_mpi_sys_mmz_alloc_cached(&g_npu_acl_model[model_index].output_phy_addr[i],
            &output_buffer, "output", NULL, buffer_size);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("can't malloc buffer, size is %u, create output failed\n", buffer_size);
            return TD_FAILURE;
        }
        memset_s(output_buffer, buffer_size, 0, buffer_size);
        ss_mpi_sys_flush_cache(g_npu_acl_model[model_index].output_phy_addr[i], output_buffer, buffer_size);

        aclDataBuffer *output_data = aclCreateDataBuffer(output_buffer, buffer_size);
        if (output_data == TD_NULL) {
            sample_svp_trace_err("can't create data buffer, create output failed\n");
            aclrtFree(output_buffer);
            return TD_FAILURE;
        }

        ret = aclmdlAddDatasetBuffer(g_npu_acl_model[model_index].output_dataset, output_data);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("can't add data buffer, create output failed\n");
            aclrtFree(output_buffer);
            aclDestroyDataBuffer(output_data);
            return TD_FAILURE;
        }
    }

    sample_svp_trace_info("create model output cached TD_SUCCESS\n");
    return TD_SUCCESS;
}

td_void sample_npu_destroy_cached_input(td_u32 model_index)
{
    if (g_npu_acl_model[model_index].input_dataset == TD_NULL) {
        return;
    }

    for (td_u32 i = 0; i < aclmdlGetDatasetNumBuffers(g_npu_acl_model[model_index].input_dataset); ++i) {
        aclDataBuffer *data_buffer = aclmdlGetDatasetBuffer(g_npu_acl_model[model_index].input_dataset, i);
        td_void *data = aclGetDataBufferAddr(data_buffer);
        ss_mpi_sys_mmz_free(g_npu_acl_model[model_index].input_phy_addr[i], data);
        (td_void)aclDestroyDataBuffer(data_buffer);
    }

    (td_void)aclmdlDestroyDataset(g_npu_acl_model[model_index].input_dataset);
    g_npu_acl_model[model_index].input_dataset = TD_NULL;
}

td_void sample_npu_destroy_cached_output(td_u32 model_index)
{
    if (g_npu_acl_model[model_index].output_dataset == TD_NULL) {
        return;
    }

    for (td_u32 i = 0; i < aclmdlGetDatasetNumBuffers(g_npu_acl_model[model_index].output_dataset); ++i) {
        aclDataBuffer *data_buffer = aclmdlGetDatasetBuffer(g_npu_acl_model[model_index].output_dataset, i);
        td_void *data = aclGetDataBufferAddr(data_buffer);
        ss_mpi_sys_mmz_free(g_npu_acl_model[model_index].output_phy_addr[i], data);
        (td_void)aclDestroyDataBuffer(data_buffer);
    }

    (td_void)aclmdlDestroyDataset(g_npu_acl_model[model_index].output_dataset);
    g_npu_acl_model[model_index].output_dataset = TD_NULL;
}

td_s32 sample_npu_create_output(td_u32 model_index)
{
    td_u32 output_size;

    if (g_npu_acl_model[model_index].model_desc == TD_NULL) {
        sample_svp_trace_err("no model description, create output failed\n");
        return TD_FAILURE;
    }

    g_npu_acl_model[model_index].output_dataset = aclmdlCreateDataset();
    if (g_npu_acl_model[model_index].output_dataset == TD_NULL) {
        sample_svp_trace_err("can't create dataset, create output failed\n");
        return TD_FAILURE;
    }

    output_size = aclmdlGetNumOutputs(g_npu_acl_model[model_index].model_desc);
    for (td_u32 i = 0; i < output_size; ++i) {
        td_u32 buffer_size = aclmdlGetOutputSizeByIndex(g_npu_acl_model[model_index].model_desc, i);

        td_void *output_buffer = TD_NULL;
        td_s32 ret = aclrtMalloc(&output_buffer, buffer_size, ACL_MEM_MALLOC_NORMAL_ONLY);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("can't malloc buffer, size is %u, create output failed\n", buffer_size);
            return TD_FAILURE;
        }

        aclDataBuffer *output_data = aclCreateDataBuffer(output_buffer, buffer_size);
        if (output_data == TD_NULL) {
            sample_svp_trace_err("can't create data buffer, create output failed\n");
            aclrtFree(output_buffer);
            return TD_FAILURE;
        }

        ret = aclmdlAddDatasetBuffer(g_npu_acl_model[model_index].output_dataset, output_data);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("can't add data buffer, create output failed\n");
            aclrtFree(output_buffer);
            aclDestroyDataBuffer(output_data);
            return TD_FAILURE;
        }
    }

    sample_svp_trace_info("create model output TD_SUCCESS\n");
    return TD_SUCCESS;
}

/* print the top 5 confidence values with indexes */
#define SHOW_TOP_NUM    5
static td_void ssample_npu_sort_output_result(const td_float *src, td_u32 src_len,
    td_float *dst, td_u32 dst_len)
{
    td_u32 i, j;

    if (src == TD_NULL || dst == TD_NULL || src_len == 0 || dst_len == 0) {
        return;
    }

    for (i = 0; i < src_len; i++) {
        td_bool charge = TD_FALSE;
        td_u32 index;

        for (j = 0; j < dst_len; j++) {
            if (src[i] > dst[j]) {
                index = j;
                charge = TD_TRUE;
                break;
            }
        }

        if (charge == TD_TRUE) {
            for (j = dst_len - 1; j > index; j--) {
                dst[j] = dst[j - 1];
            }
            dst[index] = src[i];
        }
    }
}

#include "wrapperncnn.h"
td_void sample_npu_output_model_result(td_u32 model_index)
{
    aclDataBuffer *data_buffer = TD_NULL;
    td_void *data = TD_NULL;
    td_u32 len;
    td_u32 i, j;
    td_float top[SHOW_TOP_NUM] = { 0.0 };

    for (i = 0; i < aclmdlGetDatasetNumBuffers(g_npu_acl_model[model_index].output_dataset); ++i) {
        data_buffer = aclmdlGetDatasetBuffer(g_npu_acl_model[model_index].output_dataset, i);
        if (data_buffer == TD_NULL) {
            sample_svp_trace_err("get data buffer null.\n");
            continue;
        }

        data = aclGetDataBufferAddr(data_buffer);
        len = aclGetDataBufferSizeV2(data_buffer);
        if (data == TD_NULL || len == 0) {
            sample_svp_trace_err("get data null.\n");
            continue;
        }

        ncnn_result(data,len/sizeof(float));

        // ssample_npu_sort_output_result(data, (len / sizeof(td_float)), top, SHOW_TOP_NUM);

        // for (j = 0; j < SHOW_TOP_NUM; j++) {
        //     sample_svp_trace_info("top %d: value[%lf]\n", j, top[j]);
        // }
    }

    sample_svp_trace_info("output data success\n");
    return;
}

td_void sample_npu_destroy_output(td_u32 model_index)
{
    if (g_npu_acl_model[model_index].output_dataset == TD_NULL) {
        return;
    }

    for (td_u32 i = 0; i < aclmdlGetDatasetNumBuffers(g_npu_acl_model[model_index].output_dataset); ++i) {
        aclDataBuffer *data_buffer = aclmdlGetDatasetBuffer(g_npu_acl_model[model_index].output_dataset, i);
        td_void *data = aclGetDataBufferAddr(data_buffer);
        (td_void)aclrtFree(data);
        (td_void)aclDestroyDataBuffer(data_buffer);
    }

    (td_void)aclmdlDestroyDataset(g_npu_acl_model[model_index].output_dataset);
    g_npu_acl_model[model_index].output_dataset = TD_NULL;
}

td_s32 sample_npu_model_execute(td_u32 model_index)
{
    td_s32 ret;
    ret = aclmdlExecute(g_npu_acl_model[model_index].model_id, g_npu_acl_model[model_index].input_dataset,
        g_npu_acl_model[model_index].output_dataset);
    sample_svp_trace_info("end aclmdlExecute, modelId is %u\n", g_npu_acl_model[model_index].model_id);
    return ret;
}

td_void sample_npu_unload_model(td_u32 model_index)
{
    if (!g_npu_acl_model[model_index].is_load_flag) {
        sample_svp_trace_info("no model had been loaded.\n");
        return;
    }

    td_s32 ret = aclmdlUnload(g_npu_acl_model[model_index].model_id);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("unload model failed, modelId is %u\n", g_npu_acl_model[model_index].model_id);
    }

    if (g_npu_acl_model[model_index].model_desc != TD_NULL) {
        (td_void)aclmdlDestroyDesc(g_npu_acl_model[model_index].model_desc);
        g_npu_acl_model[model_index].model_desc = TD_NULL;
    }

    if (g_npu_acl_model[model_index].model_mem_ptr != TD_NULL) {
        aclrtFree(g_npu_acl_model[model_index].model_mem_ptr);
        g_npu_acl_model[model_index].model_mem_ptr = TD_NULL;
        g_npu_acl_model[model_index].model_mem_size = 0;
    }

    if (g_npu_acl_model[model_index].model_weight_ptr != TD_NULL) {
        aclrtFree(g_npu_acl_model[model_index].model_weight_ptr);
        g_npu_acl_model[model_index].model_weight_ptr = TD_NULL;
        g_npu_acl_model[model_index].model_weight_size = 0;
    }

    g_npu_acl_model[model_index].is_load_flag = TD_FALSE;
    sample_svp_trace_info("unload model SUCCESS, modelId is %u\n", g_npu_acl_model[model_index].model_id);
}

td_void sample_npu_unload_model_cached(td_u32 model_index)
{
    if (!g_npu_acl_model[model_index].is_load_flag) {
        sample_svp_trace_info("no model had been loaded.\n");
        return;
    }

    td_s32 ret = aclmdlUnload(g_npu_acl_model[model_index].model_id);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("unload model failed, modelId is %u\n", g_npu_acl_model[model_index].model_id);
    }

    if (g_npu_acl_model[model_index].model_desc != TD_NULL) {
        (td_void)aclmdlDestroyDesc(g_npu_acl_model[model_index].model_desc);
        g_npu_acl_model[model_index].model_desc = TD_NULL;
    }

    if (g_npu_acl_model[model_index].model_mem_ptr != TD_NULL) {
        ss_mpi_sys_mmz_free(g_npu_acl_model[model_index].model_mem_phy_addr,
            g_npu_acl_model[model_index].model_mem_ptr);
        g_npu_acl_model[model_index].model_mem_ptr = TD_NULL;
        g_npu_acl_model[model_index].model_mem_size = 0;
    }

    if (g_npu_acl_model[model_index].model_weight_ptr != TD_NULL) {
        ss_mpi_sys_mmz_free(g_npu_acl_model[model_index].model_weight_phy_addr,
            g_npu_acl_model[model_index].model_weight_ptr);
        g_npu_acl_model[model_index].model_weight_ptr = TD_NULL;
        g_npu_acl_model[model_index].model_weight_size = 0;
    }

    g_npu_acl_model[model_index].is_load_flag = TD_FALSE;
    sample_svp_trace_info("unload model SUCCESS, modelId is %u\n", g_npu_acl_model[model_index].model_id);
}

#define MAX_BATCH_COUNT 100
static td_s32 sample_svp_npu_get_dynamicbatch(td_u32 model_index, td_u32 *batchs, td_u32 *count)
{
    aclmdlBatch batch;
    if (g_npu_acl_model[model_index].model_desc == TD_NULL) {
        sample_svp_trace_err("no model description, create input failed\n");
        return TD_FAILURE;
    }
    aclError ret = aclmdlGetDynamicBatch(g_npu_acl_model[model_index].model_desc, &batch);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("aclmdlGetDynamicBatch failed, modelId is %u, errorCode is %d\n",
            g_npu_acl_model[model_index].model_id, (int32_t)ret);
        return TD_FAILURE;
    }
    sample_svp_trace_info("aclmdlGetDynamicBatch batch count = %d\n", (int32_t)(batch.batchCount));
    *count = batch.batchCount;
    if (*count >= MAX_BATCH_COUNT) {
        sample_svp_trace_err("dynamic batch count[%u] is larger than max count[%u]\n", *count, MAX_BATCH_COUNT);
        return TD_FAILURE;
    }
    for (td_u32 i = 0; i < batch.batchCount; i++) {
        sample_svp_trace_info("aclmdlGetDynamicBatch index = %d batch = %lu\n", i, batch.batch[i]);
        batchs[i] = batch.batch[i];
    }
    return TD_SUCCESS;
}

static td_void sample_npu_set_input_data(int model_index, td_u32 value)
{
    aclDataBuffer *data_buffer = aclmdlGetDatasetBuffer(g_npu_acl_model[model_index].input_dataset, 0);
    td_void *data = aclGetDataBufferAddr(data_buffer);
    td_u32 buffer_size = aclmdlGetInputSizeByIndex(g_npu_acl_model[model_index].model_desc, 0);
    memset_s(data, buffer_size, value, buffer_size);
    ss_mpi_sys_flush_cache(g_npu_acl_model[model_index].input_phy_addr[0], data, buffer_size);
}

td_s32 sample_svp_npu_loop_execute_dynamicbatch(td_u32 model_index)
{
    td_u32 batchs[MAX_BATCH_COUNT];
    td_u32 count = 0;
    td_s32 ret = sample_svp_npu_get_dynamicbatch(model_index, batchs, &count);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("sample_svp_npu_get_dynamicbatch fail\n");
        return TD_FAILURE;
    }

    /* memset_s input as 1 */
    sample_npu_set_input_data(model_index, 1);

    /* loop execute with every batch num */
    size_t index;
    for (td_u32 i = 0; i < count; i++) {
        ret = aclmdlGetInputIndexByName(g_npu_acl_model[model_index].model_desc, ACL_DYNAMIC_TENSOR_NAME, &index);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("aclmdlGetInputIndexByName failed, modelId is %u, errorCode is %d\n",
                g_npu_acl_model[model_index].model_id, (int32_t)(ret));
            return TD_FAILURE;
        }
        sample_svp_trace_info("aclmdlSetDynamicBatchSize , batchSize is %u\n", batchs[i]);
        ret = aclmdlSetDynamicBatchSize(g_npu_acl_model[model_index].model_id,
            g_npu_acl_model[model_index].input_dataset, index,  batchs[i]);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("aclmdlSetDynamicBatchSize failed, modelId is %u, errorCode is %d\n",
                g_npu_acl_model[model_index].model_id, (int32_t)(ret));
            return TD_FAILURE;
        }

        /* ater set dynamic batch size, flush the mem */
        aclDataBuffer *input_data_buffer = aclmdlGetDatasetBuffer(g_npu_acl_model[model_index].input_dataset, index);
        td_void *data = aclGetDataBufferAddr(input_data_buffer);
        td_u32 buffer_size = aclmdlGetInputSizeByIndex(g_npu_acl_model[model_index].model_desc, index);

        ss_mpi_sys_flush_cache(g_npu_acl_model[model_index].input_phy_addr[index], data, buffer_size);

        ret = aclmdlExecute(g_npu_acl_model[model_index].model_id, g_npu_acl_model[model_index].input_dataset,
            g_npu_acl_model[model_index].output_dataset);
        if (ret != ACL_ERROR_NONE) {
            sample_svp_trace_err("execute model failed, modelId is %u, errorCode is %d\n",
                g_npu_acl_model[model_index].model_id, (int32_t)(ret));
            return TD_FAILURE;
        }
        aclDataBuffer *output_data_buffer = aclmdlGetDatasetBuffer(g_npu_acl_model[model_index].output_dataset, 0);
        data = aclGetDataBufferAddr(output_data_buffer);
        buffer_size = aclmdlGetOutputSizeByIndex(g_npu_acl_model[model_index].model_desc, 0);
        ss_mpi_sys_flush_cache(g_npu_acl_model[model_index].output_phy_addr[0], data, buffer_size);
    }
    sample_svp_trace_info("loop execute dynamicbatch success\n");
    return TD_SUCCESS;
}
