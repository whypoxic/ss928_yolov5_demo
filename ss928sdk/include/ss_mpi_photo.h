/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_PHOTO_H__
#define __SS_MPI_PHOTO_H__

#include "ot_common_photo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

td_s32 ss_mpi_photo_alg_init(ot_photo_alg_type alg_type, const ot_photo_alg_init *photo_init);
td_s32 ss_mpi_photo_alg_deinit(ot_photo_alg_type alg_type);

td_s32 ss_mpi_photo_alg_process(ot_photo_alg_type alg_type, const ot_photo_alg_attr *photo_attr);

td_s32 ss_mpi_photo_set_alg_coef(ot_photo_alg_type alg_type, const ot_photo_alg_coef *alg_coef);
td_s32 ss_mpi_photo_get_alg_coef(ot_photo_alg_type alg_type, ot_photo_alg_coef *alg_coef);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SS_MPI_PHOTO_H__ */
