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

#ifndef __MF_DOWNLOAD_APPS_VIEW_H_DEF__
#define __MF_DOWNLOAD_APPS_VIEW_H_DEF__

#include "mf-object-conf.h"
#include "mf-main.h"

typedef struct {
	INHERIT_MF_LIST
	char *size;
	char *pkgid;
	char *icon_path;
	char *pkg_label;
	char *mainappid;
	char *pkg_type;
	int total_size;
} mf_download_item_data_s;
void mf_download_app_pkgmgr_subscribe(struct appdata *data);
void mf_download_app_uninstall_cb(void *data, Evas_Object * obj, void *event_info);
void mf_download_app_exit_unstall(void *data);

#endif //__MF_DOWNLOAD_APPS_VIEW_H_DEF__
