/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __AUDIO_DL_ADP_H__
#define __AUDIO_DL_ADP_H__

#include "ot_type.h"

td_s32 audio_dlpath(td_char *lib_path);

td_s32 audio_dlopen(td_void **lib_handle, td_char *lib_name);

td_s32 audio_dlsym(td_void **func_handle, td_void *lib_handle, td_char *func_name);

td_s32 audio_dlclose(td_void *lib_handle);

#endif
