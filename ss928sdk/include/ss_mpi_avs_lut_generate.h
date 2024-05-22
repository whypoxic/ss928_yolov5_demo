/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_AVS_LUT_GENERATE_H__
#define __SS_MPI_AVS_LUT_GENERATE_H__

#include "ot_type.h"
#include "ot_common_avs_lut_generate.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * generates mesh(LUT) files.
 * lut_input: the input of lut generates.
 * lut_output_addr: memory for saving each output lookup tables, the memory size for each one should be 4MB.
 */
ot_avs_status ss_mpi_avs_lut_generate(ot_avs_lut_generate_input *lut_input,
    td_u64 lut_output_addr[OT_AVS_MAX_CAMERA_NUM]);

/*
 * get rotation matrix for each camera.
 * file_input_addr: the memory address of input avs calibration file;
 * rotation_matrix: the output rotation matrix for each camera.
 */
ot_avs_status ss_mpi_avs_get_rotation_matrix(const td_u64 file_input_addr,
    td_double rotation_matrix[OT_AVS_MAX_CAMERA_NUM][OT_AVS_MATRIX_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* __ss_mpi_avs_lut_generate_H__ */
