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

#ifndef __MF_MOVE_H_DEF__
#define __MF_MOVE_H_DEF__

#include <glib.h>
#include "mf-cancel.h"
#include "mf-request.h"
#include "mf-fo-common.h"

/**
 * mf_move_move_items:
 * @item_list: a GList of strings containing item path to move
 * @dst_dir: destination directory to move
 * @request_callback: callback to handle duplicated file/directory name,
 *                              if NULL is set for requset_callback, default action is merge.
 *                              for detail, see "mf_request.h"
 * @msg_callback: callback for reporting progress, for detail, see "mf_fo_common.h"
 * @cancel :  a handle for cancelling move operation, for detail, see "mf_cancel.h"
 * @sync: a variable for requesting file system sync, if TRUE is set, sync() function is called after move done.
 * @u_data: user data
 *
 * Start move items in given @item_list to @dst_dir, @msg_callback will be called repeatly with @u_data
 * in certain interval to report current progress.
 * if duplicated file/directory is found, @request_callback is called with @u_data,
 * and move operation will be blocked until user's decision is decided.
 * if someone want to cancel operation, call mf_cancel_do_cancel() with @cancel.
 * if @sync is set TRUE, sync() function is called after move operation is done to flush out file system cache.
 * Return value: This function returns zero on success, or negative value.
 **/

int mf_move_move_items(GList *item_list, const char *dst_dir, mf_cancel *cancel, gboolean sync, void *u_data);

#endif //__MF_MOVE_H_DEF__
