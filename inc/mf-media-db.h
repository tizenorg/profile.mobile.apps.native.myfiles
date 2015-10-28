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

#ifndef _GALLERY_MEDIA_DB_H_
#define _GALLERY_MEDIA_DB_H_

#include "mf-media-types.h"
#include "mf-media-error.h"
#include <sqlite3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int mf_connect_db_with_handle(sqlite3 **db_handle);
int mf_disconnect_db_with_handle(sqlite3 *db_handle);
int mf_insert_recent_file(MFDHandle *mfd_handle, const char *path, const char *name, int storage_type, const char *thumbnail_path);
int mf_delete_recent_files(MFDHandle *mfd_handle, const char *path);
int mf_delete_recent_files_by_type(MFDHandle *mfd_handle, int storage_type);
int mf_update_recent_files_thumbnail(MFDHandle *mfd_handle, const char *thumbnail, const char *new_thumbnail);
int mf_update_recent_files_name(MFDHandle *mfd_handle,const char *new_name, char *old_name);
int mf_foreach_recent_files_list(MFDHandle *mfd_handle, mf_recent_files_item_cb callback, void *user_data);
int mf_get_recent_files_count(MFDHandle *mfd_handle, int *count);
int mf_find_recent_file(MFDHandle *mfd_handle, const char *path);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*_GALLERY_MEDIA_DB_H_*/


