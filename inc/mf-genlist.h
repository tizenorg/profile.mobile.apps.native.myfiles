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

#ifndef __MF_GENLIST_H_DEF__
#define __MF_GENLIST_H_DEF__

#include <Elementary.h>
#include "mf-object-conf.h"

#define MF_GENLIST_THUMBNAIL_SIZE		46

typedef enum __mf_item_itc_type mf_item_itc_type_e;
enum __mf_item_itc_type {
	mf_item_itc_type_none,
	mf_item_itc_type_search,
	mf_item_itc_type_recent,
	mf_item_itc_type_normal_list,
	mf_item_itc_type_normal_list_details,
	mf_item_itc_type_category,
	mf_item_itc_type_max
};
Evas_Object *mf_genlist_create_list(void *data, Evas_Object *parent);
Evas_Object *mfSearchResultListView(void *data);
Evas_Object *mf_genlist_create_list_new_folder_style(void *data);
void mf_genlist_create_list_default_style(Evas_Object *pGenlist, void *data, Eina_List *dir_list,
					  Eina_List *file_list);
int mf_util_generate_list_data(const char *path, Eina_List **dir_list ,Eina_List ** file_list);

void mf_genlist_clear(Evas_Object *genlist);
Evas_Object *mf_genlist_create_list_rename_style(void *data);
void mf_genlist_create_itc_style(Elm_Genlist_Item_Class **itc, int itc_type);
void mf_genlist_get_list_selected_items(Evas_Object * pGenlist, Eina_List **list);
void mf_genlist_gl_sel(void *data, Evas_Object * obj, void *event_info);
void mf_genlist_create_data(mfItemData_s **m_TempItem, const char *name, void *data);
Evas_Object *mf_genlist_create_path_info(Evas_Object *parent, char *info, Eina_Bool slide_flag);

void mf_genlist_set_folder_edit_style(void *data);
void mf_genlist_gl_lang_changed(void *data, Evas_Object *obj, void *event_info);
void mf_genlist_disable_items(Evas_Object *genlist, Eina_Bool disable);
void mf_genlist_get_thumbnail(mfItemData_s *params);
void mf_genlist_gl_edit_item_selected(void *data);
void mf_genlist_gl_selected(void *data, Evas_Object * obj, void *event_info);
void mf_genlist_gl_longpress(void *data, Evas_Object *obj, void *event_info);
char *mf_genlist_first_item_name_get(Evas_Object *genlist);
Evas_Object *mf_genlist_create_path_tab(Evas_Object *parent, char *info, void *data);
char *mf_genlist_group_index_label_get(void *data, Evas_Object * obj, const char *part);
void mf_genlist_group_index_del(void *data, Evas_Object * obj);
void mf_genlist_cloud_content_set(void *data, Evas_Object *genlist, Eina_List *file_list);
void mf_genlist_cloud_item_content_set(void *data, Evas_Object *genlist, Eina_List *file_list);

#endif //__MF_GENLIST_H_DEF__
