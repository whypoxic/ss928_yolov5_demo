/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_HNR_H__
#define __SS_MPI_HNR_H__

#include "ot_common_vi.h"
#include "ot_common_hnr.h"
#include "ot_common_vb.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

td_s32 ss_mpi_hnr_init(td_void);
td_void ss_mpi_hnr_exit(td_void);

td_s32 ss_mpi_hnr_load_cfg(const ot_hnr_cfg *cfg, td_s32 *cfg_id);
td_s32 ss_mpi_hnr_unload_cfg(td_s32 cfg_id);

td_s32 ss_mpi_hnr_set_alg_cfg(ot_vi_pipe vi_pipe, const ot_hnr_alg_cfg *cfg);
td_s32 ss_mpi_hnr_get_alg_cfg(ot_vi_pipe vi_pipe, ot_hnr_alg_cfg *cfg);

td_s32 ss_mpi_hnr_enable(ot_vi_pipe vi_pipe);
td_s32 ss_mpi_hnr_disable(ot_vi_pipe vi_pipe);

td_s32 ss_mpi_hnr_set_attr(ot_vi_pipe vi_pipe, const ot_hnr_attr *attr);
td_s32 ss_mpi_hnr_get_attr(ot_vi_pipe vi_pipe, ot_hnr_attr *attr);

td_s32 ss_mpi_hnr_set_input_depth(ot_vi_pipe vi_pipe, td_u32 depth);

td_s32 ss_mpi_hnr_set_thread_attr(const ot_hnr_thread_attr *thread_attr);
td_s32 ss_mpi_hnr_get_thread_attr(ot_hnr_thread_attr *thread_attr);

td_s32 ss_mpi_hnr_attach_out_vb_pool(ot_vi_pipe vi_pipe, ot_vb_pool vb_pool);
td_s32 ss_mpi_hnr_detach_out_vb_pool(ot_vi_pipe vi_pipe);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SS_MPI_HNR_H__ */
