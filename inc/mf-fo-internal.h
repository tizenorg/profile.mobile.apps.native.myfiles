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

#ifndef __MF_FO_INTERNAL_H_DEF__
#define __MF_FO_INTERNAL_H_DEF__

#include <stdbool.h>
#include <sys/types.h>
#include "mf-fo-common.h"
#include "mf-util.h"

typedef void (*_mf_fo_msg_cb) (mf_msg_type msg_type, const char *real, unsigned long long size, int error_code, void *data);

bool _mf_fo_check_exist(const char *path);
char *_mf_fo_get_next_unique_dirname(const char *name, int *errcode);
char *_mf_fo_get_next_unique_filename(const char *name, int *errcode);
int _mf_fo_get_total_item_size(const char *item, unsigned long long *size);
int _mf_fo_get_remain_space(const char *path, unsigned long long *size);

int _mf_fo_errno_to_mferr(int err_no);
void _mf_fo_free_directory_hierarchies(GSList **glist);
int _mf_fo_get_file_location(const char *path);
char *_mf_fo_get_next_unique_filename(const char *name, int *errcode);

#endif //__MF_FO_INTERNAL_H_DEF__
