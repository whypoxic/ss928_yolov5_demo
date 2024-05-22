/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_MCF_VI_H__
#define __SS_MPI_MCF_VI_H__

#include "ot_common_mcf_vi.h"
#include "ot_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

td_s32 ss_mpi_mcf_set_vi_attr(ot_mcf_id mcf_id, const ot_mcf_vi_attr *mcf_vi_attr);
td_s32 ss_mpi_mcf_get_vi_attr(ot_mcf_id mcf_id, ot_mcf_vi_attr *mcf_vi_attr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SS_MPI_MCF_VI_H__ */