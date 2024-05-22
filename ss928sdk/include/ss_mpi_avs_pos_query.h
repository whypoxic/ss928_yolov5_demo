/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_AVS_POS_QUERY_H__
#define __SS_MPI_AVS_POS_QUERY_H__

#include "ot_common_avs_pos_query.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * generates the lookup table about the position between output image and source image.
 * avs_config:     output image config
 * mesh_vir_addr:  the address of mesh data to save.
 */
td_s32 ss_mpi_avs_pos_mesh_generate(const ot_avs_pos_cfg *cfg, const td_u64 mesh_addr[OT_AVS_MAX_INPUT_NUM]);

/*
 * query the position in source image space from the output image space
 * dst_size:    the resolution of destination image;
 * window_size: the windows size of position mesh data, should be same as generating the position mesh.
 * mesh_addr:   the virtual address of position mesh data, the memory size should be same as the mesh file.
 * dst_point:   the input position in destination image space.
 * src_point:   the output position in source image space.
 */
td_s32 ss_mpi_avs_pos_query_dst_to_src(const ot_size *dst_size, td_u32 window_size, td_u64 mesh_addr,
    const ot_point *dst_point, ot_point *src_point);

/*
 * query the position in output stitch image space from the source image space
 * src_size:    the resolution of source image;
 * window_size: the windows size of position mesh data, should be same as generating the position mesh.
 * mesh_addr:   the virtual address of position mesh data, the memory size should be same as the mesh file.
 * src_point:   the input position in source image space.
 * dst_point:   the output position in destination image space.
 */
td_s32 ss_mpi_avs_pos_query_src_to_dst(const ot_size *src_size, td_u32 window_size, td_u64 mesh_addr,
    const ot_point *src_point, ot_point *dst_point);

#ifdef __cplusplus
}
#endif

#endif /* ss_mpi_avs_pos_query.h */