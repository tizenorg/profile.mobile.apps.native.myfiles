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

#ifndef __MF_DOWNLOAD_APP_H__
#define __MF_DOWNLOAD_APP_H__

//add include files
#include <glib.h>

#include "mf-fs-util.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _downloadapp_listinfo {

	char *pkgid;
	char *icon_path;
	char *pkg_label;
	char *mainappid;
	char *pkg_type;

	int total_size;
	bool valid_size;
	bool is_preload;
	bool is_update;
	GList *runinfos;
} downloadapp_listinfo;

typedef struct _download_app_data {
	void *list_worker;
	GList* app_list;
	void* data;
} downloadapp_data;

typedef int (*async_fn)(downloadapp_data *ad);
typedef void (*callback_fn)(int fn_result, downloadapp_data *ad);

//add functions 
int mf_download_app_list_get(downloadapp_data *da);
void mf_download_app_destroy_data(downloadapp_data *da);
GList *mf_download_app_data_list_get();
void mf_download_app_main(void *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__MF_DOWNLOAD_APP_H__

