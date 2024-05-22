/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_OTP_H__
#define __SS_MPI_OTP_H__

#include "ot_common_otp.h"

#ifdef __cplusplus
extern "C" {
#endif

td_s32 ss_mpi_otp_init(td_void);

td_s32 ss_mpi_otp_deinit(td_void);

td_s32 ss_mpi_otp_set_user_data(const td_char *field_name,
    td_u32 offset, const td_u8 *value, td_u32 value_len);

td_s32 ss_mpi_otp_get_user_data(const td_char *field_name,
    td_u32 offset, td_u8 *value, td_u32 value_len);

td_s32 ss_mpi_otp_burn_product_pv(const ot_otp_burn_pv_item *pv, td_u32 num);

td_s32 ss_mpi_otp_read_product_pv(ot_otp_burn_pv_item *pv, td_u32 num);

td_s32 ss_mpi_otp_get_key_verify_status(const td_char *key_name, td_bool *status);

td_s32 ss_mpi_otp_set_user_data_lock(const td_char *field_name,
    td_u32 offset, td_u32 value_len);

td_s32 ss_mpi_otp_get_user_data_lock(const td_char *field_name,
    td_u32 offset, td_u32 value_len, ot_otp_lock_status *lock);

#ifdef __cplusplus
}
#endif

#endif /* __SS_MPI_OTP_H__ */
