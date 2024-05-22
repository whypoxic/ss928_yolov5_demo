/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */
#ifndef __SS_MPI_HDMI_H__
#define __SS_MPI_HDMI_H__

#include "ot_common_hdmi.h"

#ifdef __cplusplus
extern "C" {
#endif

td_s32 ss_mpi_hdmi_init(td_void);

td_s32 ss_mpi_hdmi_deinit(td_void);

td_s32 ss_mpi_hdmi_open(ot_hdmi_id hdmi);

td_s32 ss_mpi_hdmi_close(ot_hdmi_id hdmi);

td_s32 ss_mpi_hdmi_get_sink_capability(ot_hdmi_id hdmi, ot_hdmi_sink_capability *capability);

td_s32 ss_mpi_hdmi_set_attr(ot_hdmi_id hdmi, const ot_hdmi_attr *attr);

td_s32 ss_mpi_hdmi_get_attr(ot_hdmi_id hdmi, ot_hdmi_attr *attr);

td_s32 ss_mpi_hdmi_start(ot_hdmi_id hdmi);

td_s32 ss_mpi_hdmi_stop(ot_hdmi_id hdmi);

td_s32 ss_mpi_hdmi_force_get_edid(ot_hdmi_id hdmi, ot_hdmi_edid *edid_data);

td_s32 ss_mpi_hdmi_set_infoframe(ot_hdmi_id hdmi, const ot_hdmi_infoframe *infoframe);

td_s32 ss_mpi_hdmi_get_infoframe(ot_hdmi_id hdmi, ot_hdmi_infoframe_type infoframe_type, ot_hdmi_infoframe *infoframe);

td_s32 ss_mpi_hdmi_register_callback(ot_hdmi_id hdmi, const ot_hdmi_callback_func *callback_func);

td_s32 ss_mpi_hdmi_unregister_callback(ot_hdmi_id hdmi, const ot_hdmi_callback_func *callback_func);

td_s32 ss_mpi_hdmi_set_hw_spec(ot_hdmi_id hdmi, const ot_hdmi_hw_spec *hw_spec);

td_s32 ss_mpi_hdmi_get_hw_spec(ot_hdmi_id hdmi, ot_hdmi_hw_spec *hw_spec);

td_s32 ss_mpi_hdmi_set_avmute(ot_hdmi_id hdmi, td_bool avmute_en);

td_s32 ss_mpi_hdmi_set_mod_param(ot_hdmi_id hdmi, const ot_hdmi_mod_param *mod_param);

td_s32 ss_mpi_hdmi_get_mod_param(ot_hdmi_id hdmi, ot_hdmi_mod_param *mod_param);

#ifdef __cplusplus
}
#endif

#endif
