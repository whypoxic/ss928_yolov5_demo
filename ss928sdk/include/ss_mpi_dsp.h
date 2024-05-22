/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef _SS_MPI_DSP_H_
#define _SS_MPI_DSP_H_

#include "ot_debug.h"
#include "ot_common_dsp.h"
#include "ot_common.h"

#ifdef __cplusplus
extern "C" {
#endif

td_s32 ss_mpi_svp_dsp_load_bin(const td_char *bin_file_name, ot_svp_dsp_mem_type mem_type);

td_s32 ss_mpi_svp_dsp_enable_core(ot_svp_dsp_id dsp_id);

td_s32 ss_mpi_svp_dsp_disable_core(ot_svp_dsp_id dsp_id);

td_s32 ss_mpi_svp_dsp_rpc(ot_svp_dsp_handle *handle, const ot_svp_dsp_msg *msg, ot_svp_dsp_id dsp_id,
    ot_svp_dsp_pri pri);

td_s32 ss_mpi_svp_dsp_query(ot_svp_dsp_id dsp_id, ot_svp_dsp_handle handle, td_bool *is_finish, td_bool is_block);

td_s32 ss_mpi_svp_dsp_power_on(ot_svp_dsp_id dsp_id);

td_s32 ss_mpi_svp_dsp_power_off(ot_svp_dsp_id dsp_id);

#ifdef __cplusplus
}
#endif
#endif
