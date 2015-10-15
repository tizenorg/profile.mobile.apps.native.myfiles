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

#ifndef __MF_VIEW_H_DEF__
#define __MF_VIEW_H_DEF__

#include <stdio.h>
#include <assert.h>
#include <glib.h>

#include <Elementary.h>
#include <media_content.h>
#include "mf-object-conf.h"
#include "mf-main.h"

typedef enum __mf_list_data_type_e mf_list_data_type_e;
enum __mf_list_data_type_e {
	mf_list_data_fullpath,
	mf_list_data_max
};

typedef enum __mf_category_list_type_e mf_category_list_type_e;
enum __mf_category_list_type_e {
	mf_category_list_image,
	mf_category_list_video,
	mf_category_list_audio,
	mf_category_list_document,
	mf_category_list_max
};

void mf_view_phone_storage_init(void *data);
void mf_view_refresh(void *data);
void mf_view_set_main_layout_content(void *data);
void mf_view_split_create(void *data);
void mf_view_unset_main_layout_content(Evas_Object *layout);
void mf_view_items_remove(Evas_Object *parent, int storage, int type);
void mf_view_item_remove(Evas_Object *parent, const char *path, int type);
bool mf_view_same_item_exist(Evas_Object *obj, const char *check_path, mf_obj_type_e type);
Elm_Object_Item *mf_view_item_append(Evas_Object *parent, fsNodeInfo *pNode, void *data);
void mf_root_view_create(void *data);
void mf_normal_view_create(void *data);
void mf_category_view_create(void *data, bool flag_show);
void mf_view_update(void *data);
char *mf_view_item_data_get(void *data, int data_type);
void mf_view_item_remove_by_type(Evas_Object *parent, int storage_type, int view_type);
void mf_view_reset_record(oper_record *record);
int mf_view_style_get(void *data);
Elm_Object_Item *mf_view_item_append_with_data(Evas_Object *parent, mfItemData_s *item_data, void *data, void *itc, Evas_Smart_Cb func,void *user_data);
void mf_category_list_update_cb(media_content_error_e error,
					int pid,
					media_content_db_update_item_type_e update_item,
					media_content_db_update_type_e update_type,
					media_content_type_e media_type,
					char *uuid,
					char *path,
					char *mime_type,
					void *user_data);
void mf_category_list_destory();
Evas_Object *mf_category_get_from_media_db(void *data, int category, bool is_use_previous_state);

Evas_Object *mf_root_view_create_content(void *data);
Eina_Bool mf_root_view_back_cb(void *data, Elm_Object_Item *it);
void mf_view_state_set_with_pre(void *data, MORE_TYPE state);
void mf_view_state_reset_state_with_pre(void *data);
MORE_TYPE mf_view_get_pre_state(void *data);
Eina_Bool mf_view_is_root_view(void *data);
Eina_Bool mf_view_is_operating(void *data);
void mf_view_search_item_update(void *data, const char *path, char *new_path);
Eina_Bool mf_view_item_popup_check(void *data, char *path);
void mf_view_item_delete_by_name(void *data, const char *name);
void mf_view_item_delete_by_exists(void *data);
void mf_search_bar_content_object_create(void *data);
void mf_view_resume(void *data);
void mf_storage_view_create(void *data);
void mf_recent_view_create(void *data);
void mf_download_app_view_create(void *data);
void mf_detail_view_create(void *data);
void mf_root_view_append_mmc_item_after_phone(Evas_Object *parent, fsNodeInfo *pNode, void *data);
void mf_detail_data_destroy(void *data);
void mf_mw_root_category_item_update(void *data);
Eina_Bool mf_view_is_item_exists_by_name(void *data, char *name);
#endif
