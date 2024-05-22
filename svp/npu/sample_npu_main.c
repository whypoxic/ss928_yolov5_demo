/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "securec.h"
#include "sample_npu_process.h"

#define SAMPLE_SVP_NPU_ARG_MAX_NUM  3
#define SAMPLE_SVP_NPU_IDX_TWO  2
#define SAMPLE_SVP_NPU_CMP_STR_NUM 2

static char **g_npu_cmd_argv = TD_NULL;

/*
 * function : to process abnormal case
 */
#ifndef __LITEOS__
static td_void sample_svp_npu_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        switch (*g_npu_cmd_argv[1]) {
            case '0':
                sample_svp_npu_acl_resnet50_handle_sig();
                break;
            case '1':
                sample_svp_npu_acl_resnet50_handle_sig();
                break;
            default:
                break;
        }
    }
}
#endif

/*
 * function : show usage
 */
static td_void sample_svp_npu_usage(const td_char *prg_name)
{
    printf("Usage : %s <index>\n", prg_name);
    printf("index:\n");
    printf("\t 0) acl_resnet50.\n");
    printf("\t 1) acl_resnet50_multithread.\n");
    printf("\t 2) acl_mobilenet_v3_dynamicbatch_with_mmzcache.\n");
}

static td_s32 sample_svp_npu_case(int argc, char *argv[])
{
    td_s32 ret = TD_SUCCESS;

    switch (*argv[1]) {
        case '0':
            if (argc != SAMPLE_SVP_NPU_ARG_MAX_NUM - 1) {
                return TD_FAILURE;
            }
            sample_svp_npu_acl_resnet50();
            break;
        case '1':
            if (argc != SAMPLE_SVP_NPU_ARG_MAX_NUM - 1) {
                return TD_FAILURE;
            }
            sample_svp_npu_acl_resnet50_multithread();
            break;
        case '2':
            if (argc != SAMPLE_SVP_NPU_ARG_MAX_NUM - 1) {
                return TD_FAILURE;
            }
            sample_svp_npu_acl_mobilenet_v3_dynamicbatch();
            break;
        default:
            ret = TD_FAILURE;
            break;
    }
    return ret;
}

#include "wrapperncnn.h"

int main(int argc, char* argv[])
{
    remove("image.png");
    struct sigaction sa;
    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sample_svp_npu_handle_sig;
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s [imagepath]\n", argv[0]);
        return -1;
    }
    const char* imagepath = argv[1];
    ncnn_convertimg_yolov5s(imagepath, "testyuv.sp420");
    sample_svp_npu_acl_resnet50();
    system("chmod -R 777 image.png");
    return 0;
}
