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

#ifndef _MF_TRAY_H_
#define _MF_TRAY_H_

#include <Elementary.h>

#include "mf-conf.h"
#include "mf-fs-util.h"


typedef struct {
	const char *category_item;
	const char *thumb_path;
	struct appdata *ap;
} mfCategoryData_s;


#ifdef MYFILE_TRAY_FEATURE

typedef enum __mf_tray_item_category mf_tray_item_category;
enum __mf_tray_item_category{
	mf_tray_item_category_none = 0x0000,
	mf_tray_item_category_image = 0x0001,
	mf_tray_item_category_video = 0x0002,
	mf_tray_item_category_sounds = 0x0004,
	mf_tray_item_category_document = 0x0008,
	mf_tray_item_category_others = 0x0010,
	mf_tray_item_category_voice = 0x0020,
	mf_tray_item_category_download_app = 0x0040,
	mf_tray_item_category_recent = 0x0080,
	mf_tray_item_category_max = 0xffff
};

typedef enum {
	MF_STORAGE_SLEEP = 0,
	MF_STORAGE_IMAGE,
	MF_STORAGE_VIDEO,
	MF_STORAGE_SOUND,
	MF_STORAGE_DOCUMENT,
	MF_STORAGE_RECENT,
	MF_STORAGE_MAX,
} mf_update_type;

typedef struct _update_info_t {
	double total_size;
	mf_update_type type;
} Update_Info;

typedef void(* mf_storage_update_cb)(mf_tray_item_category category, Update_Info* update_info, void *pUserData);

Evas_Object *mf_tray_create(Evas_Object *parent, void *data);
int mf_tray_item_type(const char *path);
Evas_Object *mf_category_create(void *data);
void mf_category_refresh(void *data);
void mf_tray_item_search(void *data, int type);

#endif
void mf_category_widgets_lang_changed(void *data, Evas_Object *obj, void *event_info);
Evas_Object *mf_category_widgets_create(void *data, Evas_Object *parent);
void mf_category_storage_size_reset(mf_tray_item_category category);
void mf_category_storage_size_refresh(mf_tray_item_category category, Update_Info* update_info, void *pUserData);
int mf_tray_item_category_type_get_by_file_type(fsFileType type);
void mf_category_size_update(void *data);

#endif //_MF_TRAY_H_
