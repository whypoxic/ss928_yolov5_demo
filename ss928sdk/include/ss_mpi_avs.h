/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_AVS_H__
#define __SS_MPI_AVS_H__

#include "ot_common_avs.h"

#ifdef __cplusplus
extern "C" {
#endif

td_s32 ss_mpi_avs_create_grp(ot_avs_grp grp, const ot_avs_grp_attr *grp_attr);
td_s32 ss_mpi_avs_destroy_grp(ot_avs_grp grp);

td_s32 ss_mpi_avs_start_grp(ot_avs_grp grp);
td_s32 ss_mpi_avs_stop_grp(ot_avs_grp grp);

td_s32 ss_mpi_avs_reset_grp(ot_avs_grp grp);

td_s32 ss_mpi_avs_close_fd(td_void);

td_s32 ss_mpi_avs_get_grp_attr(ot_avs_grp grp, ot_avs_grp_attr *grp_attr);
td_s32 ss_mpi_avs_set_grp_attr(ot_avs_grp grp, const ot_avs_grp_attr *grp_attr);

td_s32 ss_mpi_avs_send_pipe_frame(ot_avs_grp grp, ot_avs_pipe pipe, const ot_video_frame_info *frame_info,
    td_s32 milli_sec);

td_s32 ss_mpi_avs_get_pipe_frame(ot_avs_grp grp, ot_avs_pipe pipe, ot_video_frame_info *frame_info, td_s32 milli_sec);
td_s32 ss_mpi_avs_release_pipe_frame(ot_avs_grp grp, ot_avs_pipe pipe, const ot_video_frame_info *frame_info);

td_s32 ss_mpi_avs_set_chn_attr(ot_avs_grp grp, ot_avs_chn chn, const ot_avs_chn_attr *chn_attr);
td_s32 ss_mpi_avs_get_chn_attr(ot_avs_grp grp, ot_avs_chn chn, ot_avs_chn_attr *chn_attr);

td_s32 ss_mpi_avs_enable_chn(ot_avs_grp grp, ot_avs_chn chn);
td_s32 ss_mpi_avs_disable_chn(ot_avs_grp grp, ot_avs_chn chn);

td_s32 ss_mpi_avs_get_chn_frame(ot_avs_grp grp, ot_avs_chn chn, ot_video_frame_info *frame_info, td_s32 milli_sec);
td_s32 ss_mpi_avs_release_chn_frame(ot_avs_grp grp, ot_avs_chn chn, const ot_video_frame_info *frame_info);

td_s32 ss_mpi_avs_get_mod_param(ot_avs_mod_param *mod_param);
td_s32 ss_mpi_avs_set_mod_param(const ot_avs_mod_param *mod_param);

#ifdef __cplusplus
}
#endif

#endif /* __SS_MPI_AVS_H__ */
