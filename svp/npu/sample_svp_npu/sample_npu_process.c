/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include "sample_npu_process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <limits.h>

#include "ot_common_svp.h"
#include "sample_common_svp.h"
#include "sample_npu_model.h"

static td_u32 g_npu_dev_id = 0;
static td_s32 sample_svp_npu_fill_input_data(td_void *dev_buf, size_t buf_size)
{
    td_s32 ret;
    td_char path[PATH_MAX] = { 0 };
    size_t file_size;

    if (realpath("testyuv.sp420", path) == TD_NULL) {
        sample_svp_trace_err("Invalid file!.\n");
        return TD_FAILURE;
    }

    FILE *fp = fopen(path, "rb");
    sample_svp_check_exps_return(fp == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "open image file failed!\n");

    ret = fseek(fp, 0L, SEEK_END);
    sample_svp_check_exps_goto(ret == -1, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "Fseek failed!\n");
    file_size = ftell(fp);
    sample_svp_check_exps_goto(file_size == 0, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "Ftell failed!\n");
    ret = fseek(fp, 0L, SEEK_SET);
    sample_svp_check_exps_goto(ret == -1, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "Fseek failed!\n");

    file_size = (file_size > buf_size) ? buf_size : file_size;
    ret = fread(dev_buf, file_size, 1, fp);
    sample_svp_check_exps_goto(ret != 1, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "Read file failed!\n");

    if (fp != TD_NULL) {
        fclose(fp);
    }
    return TD_SUCCESS;

end:
    if (fp != TD_NULL) {
        fclose(fp);
    }
    return TD_FAILURE;
}

static td_void sample_svp_npu_destroy_resource(td_void)
{
    aclError ret;

    ret = aclrtResetDevice(g_npu_dev_id);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("reset device fail\n");
    }
    sample_svp_trace_info("end to reset device is %d\n", g_npu_dev_id);

    ret = aclFinalize();
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("finalize acl fail\n");
    }
    sample_svp_trace_info("end to finalize acl\n");
}

static td_s32 sample_svp_npu_init_resource(td_void)
{
    /* ACL init */
    const char *acl_config_path = "";
    aclrtRunMode run_mode;
    td_s32 ret;

    ret = aclInit(acl_config_path);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("acl init fail.\n");
        return TD_FAILURE;
    }
    sample_svp_trace_info("acl init success.\n");

    /* open device */
    ret = aclrtSetDevice(g_npu_dev_id);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("acl open device %d fail.\n", g_npu_dev_id);
        return TD_FAILURE;
    }
    sample_svp_trace_info("open device %d success.\n", g_npu_dev_id);

    /* get run mode */
    ret = aclrtGetRunMode(&run_mode);
    if ((ret != ACL_ERROR_NONE) || (run_mode != ACL_DEVICE)) {
        sample_svp_trace_err("acl get run mode fail.\n");
        return TD_FAILURE;
    }
    sample_svp_trace_info("get run mode success\n");

    return TD_SUCCESS;
}

static td_void sample_svp_npu_acl_resnet50_stop(td_void)
{
    sample_svp_npu_destroy_resource();
    printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
}

static td_s32 sample_svp_npu_acl_prepare_init()
{
    td_s32 ret;

    ret = sample_svp_npu_init_resource();
    if (ret != TD_SUCCESS) {
        sample_svp_npu_destroy_resource();
    }

    return ret;
}

static td_void sample_svp_npu_acl_prepare_exit(td_u32 thread_num)
{
    for (td_u32 model_index = 0; model_index < thread_num; model_index++) {
        sample_npu_destroy_desc(model_index);
        sample_npu_unload_model(model_index);
    }
    sample_svp_npu_destroy_resource();
}

static td_s32 sample_svp_npu_load_model(const char* om_model_path, td_u32 model_index, td_bool is_cached)
{
    td_char path[PATH_MAX] = { 0 };
    td_s32 ret;

    if (sizeof(om_model_path) > PATH_MAX) {
        sample_svp_trace_err("pathname too long!.\n");
        return TD_NULL;
    }
    if (realpath(om_model_path, path) == TD_NULL) {
        sample_svp_trace_err("invalid file!.\n");
        return TD_NULL;
    }

    if (is_cached == TD_TRUE) {
        ret = sample_npu_load_model_with_mem_cached(path, model_index);
    } else {
        ret = sample_npu_load_model_with_mem(path, model_index);
    }

    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("execute load model fail, model_index is:%d.\n", model_index);
        goto acl_prepare_end1;
    }
    ret = sample_npu_create_desc(model_index);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("execute create desc fail.\n");
        goto acl_prepare_end2;
    }

    return TD_SUCCESS;

acl_prepare_end2:
    sample_npu_destroy_desc(model_index);

acl_prepare_end1:
    sample_npu_unload_model(model_index);
    return ret;
}
static td_s32 sample_svp_npu_dataset_prepare_init(td_u32 model_index)
{
    td_s32 ret;

    ret = sample_npu_create_input_dataset(model_index);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("execute create input fail.\n");
        return TD_FAILURE;
    }
    ret = sample_npu_create_output(model_index);
    if (ret != TD_SUCCESS) {
        sample_npu_destroy_input_dataset(model_index);
        sample_svp_trace_err("execute create output fail.\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 sample_svp_npu_create_cached_input_output(td_u32 model_index)
{
    td_s32 ret;

    ret = sample_npu_create_cached_input(model_index);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("execute create input fail.\n");
        return TD_FAILURE;
    }
    ret = sample_npu_create_cached_output(model_index);
    if (ret != TD_SUCCESS) {
        sample_npu_destroy_cached_input(model_index);
        sample_svp_trace_err("execute create output fail.\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void sample_svp_npu_dataset_prepare_exit(td_u32 thread_num)
{
    for (td_u32 model_index = 0; model_index < thread_num; model_index++) {
        sample_npu_destroy_output(model_index);
        sample_npu_destroy_input_dataset(model_index);
    }
}

static td_void sample_svp_npu_release_input_data(td_void **data_buf, size_t *data_len, td_u32 thread_num)
{
    for (td_u32 model_index = 0; model_index < thread_num; model_index++) {
        ot_unused(data_len[model_index]);
        (td_void)aclrtFree(data_buf[model_index]);
    }
}

static td_s32 sample_svp_npu_get_input_data(td_void **data_buf, size_t *data_len, td_u32 model_index)
{
    size_t buf_size;
    td_s32 ret;

    ret = sample_npu_get_input_size_by_index(0, &buf_size, model_index);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("execute get input size fail.\n");
        return TD_FAILURE;
    }

    ret = aclrtMalloc(data_buf, buf_size, ACL_MEM_MALLOC_NORMAL_ONLY);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("malloc device buffer fail. size is %zu, errorCode is %d.\n", buf_size, ret);
        return TD_FAILURE;
    }

    ret = sample_svp_npu_fill_input_data(*data_buf, buf_size);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("memcpy_s device buffer fail.\n");
        (td_void)aclrtFree(data_buf);
        return TD_FAILURE;
    }

    *data_len = buf_size;

    sample_svp_trace_info("get input data success\n");

    return TD_SUCCESS;
}

static td_s32 sample_svp_npu_create_input_databuf(td_void *data_buf, size_t data_len, td_u32 model_index)
{
    return sample_npu_create_input_databuf(data_buf, data_len, model_index);
}

static td_void sample_svp_npu_destroy_input_databuf(td_u32 thread_num)
{
    for (td_u32 model_index = 0; model_index < thread_num; model_index++) {
        sample_npu_destroy_input_databuf(model_index);
    }
}

void *sample_svp_execute_func_contious(void *args)
{
    td_s32 ret;
    td_u32 model_index = *(td_u32 *)args;

    ret = aclrtSetDevice(g_npu_dev_id);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("acl open device %d fail.\n", g_npu_dev_id);
        return NULL;
    }

    sample_svp_trace_info("open device %d success.\n", g_npu_dev_id);

    ret = sample_npu_model_execute(model_index);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("execute inference failed of thread[%d].\n", model_index);
    }

    ret = aclrtResetDevice(g_npu_dev_id);
    if (ret != ACL_ERROR_NONE) {
        sample_svp_trace_err("model[%d]reset device failed\n", model_index);
    }
    return NULL;
}

static td_void sample_svp_npu_model_execute_multithread()
{
    pthread_t execute_threads[MAX_THREAD_NUM] = {0};
    td_u32 index[MAX_THREAD_NUM];
    td_u32 model_index;

    for (model_index = 0; model_index < MAX_THREAD_NUM; model_index++) {
        index[model_index] = model_index;
        pthread_create(&execute_threads[model_index], NULL, sample_svp_execute_func_contious,
            (void *)&index[model_index]);
    }

    void *waitret[MAX_THREAD_NUM];
    for (model_index = 0; model_index < MAX_THREAD_NUM; model_index++) {
        pthread_join(execute_threads[model_index], &waitret[model_index]);
    }

    for (model_index = 0; model_index < MAX_THREAD_NUM; model_index++) {
        sample_npu_output_model_result(model_index);
    }
}

/* function : show the sample of npu resnet50_multithread */
td_void sample_svp_npu_acl_resnet50_multithread(td_void)
{
    td_void *data_buf[MAX_THREAD_NUM] = {TD_NULL};
    size_t buf_size[MAX_THREAD_NUM];
    td_u32 model_index;
    td_s32 ret;

    const char *om_model_path = "./data/model/resnet50.om";
    ret = sample_svp_npu_acl_prepare_init();
    if (ret != TD_SUCCESS) {
        return;
    }

    for (model_index = 0; model_index < MAX_THREAD_NUM; model_index++) {
        ret = sample_svp_npu_load_model(om_model_path, model_index, TD_FALSE);
        if (ret != TD_SUCCESS) {
            goto acl_process_end0;
        }

        ret = sample_svp_npu_dataset_prepare_init(model_index);
        if (ret != TD_SUCCESS) {
            goto acl_process_end1;
        }

        ret = sample_svp_npu_get_input_data(&data_buf[model_index], &buf_size[model_index], model_index);
        if (ret != TD_SUCCESS) {
            sample_svp_trace_err("execute create input fail.\n");
            goto acl_process_end2;
        }

        ret = sample_svp_npu_create_input_databuf(data_buf[model_index], buf_size[model_index], model_index);
        if (ret != TD_SUCCESS) {
            sample_svp_trace_err("memcpy_s device buffer fail.\n");
            goto acl_process_end3;
        }
    }

    sample_svp_npu_model_execute_multithread();

acl_process_end3:
    sample_svp_npu_destroy_input_databuf(MAX_THREAD_NUM);
acl_process_end2:
    sample_svp_npu_release_input_data(data_buf, buf_size, MAX_THREAD_NUM);
acl_process_end1:
    sample_svp_npu_dataset_prepare_exit(MAX_THREAD_NUM);
acl_process_end0:
    sample_svp_npu_acl_prepare_exit(MAX_THREAD_NUM);
}

td_void sample_svp_npu_acl_mobilenet_v3_dynamicbatch(td_void)
{
    td_s32 ret;
    const char *om_model_path = "./data/model/mobilenet_v3_dynamic_batch.om";
    ret = sample_svp_npu_acl_prepare_init();
    if (ret != TD_SUCCESS) {
        return;
    }

    ret = sample_svp_npu_load_model(om_model_path, 0, TD_TRUE);
    if (ret != TD_SUCCESS) {
        goto acl_process_end0;
    }

    ret = sample_svp_npu_create_cached_input_output(0);
    if (ret != TD_SUCCESS) {
        goto acl_process_end0;
    }

    sample_svp_npu_loop_execute_dynamicbatch(0);

    sample_npu_destroy_cached_input(0);
    sample_npu_destroy_cached_output(0);
acl_process_end0:
    sample_npu_destroy_desc(0);
    sample_npu_unload_model_cached(0);
}

/* function : show the sample of npu resnet50 */
td_void sample_svp_npu_acl_resnet50(td_void)
{
    td_void *data_buf = TD_NULL;
    size_t buf_size;
    td_s32 ret;

    // const char *om_model_path = "./data/model/resnet50.om";
    const char *om_model_path = "yolov5.om";    
    ret = sample_svp_npu_acl_prepare_init(om_model_path);
    if (ret != TD_SUCCESS) {
        return;
    }

    ret = sample_svp_npu_load_model(om_model_path, 0, TD_FALSE);
    if (ret != TD_SUCCESS) {
        goto acl_process_end0;
    }

    ret = sample_svp_npu_dataset_prepare_init(0);
    if (ret != TD_SUCCESS) {
        goto acl_process_end0;
    }

    ret = sample_svp_npu_get_input_data(&data_buf, &buf_size, 0);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("execute create input fail.\n");
        goto acl_process_end1;
    }

    ret = sample_svp_npu_create_input_databuf(data_buf, buf_size, 0);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("memcpy_s device buffer fail.\n");
        goto acl_process_end2;
    }

    ret = sample_npu_model_execute(0);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("execute inference fail.\n");
        goto acl_process_end3;
    }

    sample_npu_output_model_result(0);

acl_process_end3:
    sample_svp_npu_destroy_input_databuf(1);
acl_process_end2:
    sample_svp_npu_release_input_data(&data_buf, &buf_size, 1);
acl_process_end1:
    sample_svp_npu_dataset_prepare_exit(1);
acl_process_end0:
    sample_svp_npu_acl_prepare_exit(1);
}

/* function : npu resnet50 sample signal handle */
td_void sample_svp_npu_acl_resnet50_handle_sig(td_void)
{
    sample_svp_npu_acl_resnet50_stop();
}

/* function : show the sample of npu resnet50 */
td_void nnn_init(td_void)
{
    td_void *data_buf = TD_NULL;
    size_t buf_size;
    td_s32 ret;

    // const char *om_model_path = "./data/model/resnet50.om";
    const char *om_model_path = "yolov5s_v6.2.om";    
    ret = sample_svp_npu_acl_prepare_init(om_model_path);
    if (ret != TD_SUCCESS) {
        return;
    }

    ret = sample_svp_npu_load_model(om_model_path, 0, TD_FALSE);
    if (ret != TD_SUCCESS) {
        goto acl_process_end0;
    }

   return 0;
acl_process_end0:
    sample_svp_npu_acl_prepare_exit(1);
}

td_s32 nnn_execute(td_void* data_buf, size_t data_len)
{
    td_s32 ret;
    ret = sample_svp_npu_dataset_prepare_init(0);

    ret = sample_svp_npu_create_input_databuf(data_buf, data_len, 0);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("memcpy_s device buffer fail.\n");
        return -1;
    }
    ret = sample_npu_model_execute(0);
    if (ret != TD_SUCCESS) {
        sample_svp_trace_err("execute inference fail.\n");
        return -1;
    }
    sample_npu_output_model_result(0);
    sample_npu_destroy_input_databuf(0);
    sample_npu_destroy_output(0);
    sample_npu_destroy_input_dataset(0);
    return 0;
}