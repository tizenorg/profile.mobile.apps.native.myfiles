/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef __MF_DELETE_INTERNAL_H_DEF__
#define __MF_DELETE_INTERNAL_H_DEF__

#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mf-cancel.h"
#include "mf-fo-common.h"
#include "mf-fo-internal.h"

int _mf_delete_delete_regfile(const char *file, struct stat *file_statp, mf_cancel *cancel, _mf_fo_msg_cb msg_cb, void *msg_data);

int _mf_delete_delete_directory(const char *dir, mf_cancel *cancel, _mf_fo_msg_cb msg_cb, void *msg_data);

int _mf_delete_del_internal(const char *item, mf_cancel *cancel, _mf_fo_msg_cb msg_callback, void *msg_data);

#endif //__MF_DELETE_INTERNAL_H_DEF__
