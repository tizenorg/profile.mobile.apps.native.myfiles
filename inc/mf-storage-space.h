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

#ifndef _MF_STORAGE_SPACE_H_
#define _MF_STORAGE_SPACE_H_

#include <stdio.h>
#include <Elementary.h>
#include <glib-object.h>
#include <sys/stat.h>
#include <dirent.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <pthread.h>
#include <media_content.h>
#include <media_content_type.h>
#include "mf-util.h"
#include "mf-callback.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-fs-util.h"
#include "mf-tray-item.h"


typedef struct _mfStorageStatus mfStorageStatus;
struct _mfStorageStatus {
	pthread_mutex_t exit_mutex;
	pthread_mutex_t update_mutex;
	pthread_mutex_t update_data_mutex;
	pthread_cond_t wait_cond;

	Ecore_Pipe *pipe;
	int exit_flag;
	mf_update_type type;
	bool is_update_data;
	pthread_t tid;
	double total_size;
	mf_storage_update_cb update_cb;

	Update_Info image_size_info;
	Update_Info video_size_info;
	Update_Info sound_size_info;
	Update_Info document_size_info;
	Update_Info recent_size_info;
	Update_Info download_app_size_info;

	void *pUserData;
};

int mf_storage_create(void* app_data);
int mf_storage_destroy(void* app_data);
int mf_storage_set_update_cb(void* app_data, mf_storage_update_cb update_cb);
mfStorageStatus* mf_storage_get_status(void* app_data);
int mf_storage_set_update_data(void* app_data, bool is_update);
int mf_storage_refresh(void* app_data);
void mf_category_view_refresh_space_size_set(Eina_Bool flag);
Eina_Bool mf_category_view_refresh_space_size_get();
void mf_storage_get_recent_files_size();

#endif

