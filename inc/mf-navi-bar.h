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

#ifndef __MF_NAVI_BAR_H_DEF__
#define __MF_NAVI_BAR_H_DEF__

#include <Elementary.h>
#include "mf-object-conf.h"

void mf_navi_bar_reset_navi_obj(void *data);
Evas_Object *mf_navi_bar_create(Evas_Object * parent);
Evas_Object *mf_navi_bar_create_normal_pathinfo(void *data);
Evas_Object *mf_navi_bar_create_view_content(void *data);
void mf_navi_bar_clean_content(void *data, Evas_Object *pLayout);
void mf_navi_bar_set_content(void *data, Evas_Object *pLayout, Evas_Object *NaviContent);
void mf_navi_bar_title_content_set(void *data, const char *title);
void mf_navi_bar_title_set(void *data);
void mf_navi_add_back_button(void *data, Eina_Bool (*func)(void *data, Elm_Object_Item *it));
void mf_navi_bar_set_ctrlbar(void *data);
void mf_navi_bar_reset_ctrlbar(void *data);
void mf_navi_bar_layout_state_set(Evas_Object *layout, int type);
void mf_navi_bar_remove_info_box(void *data);
void mf_navi_bar_recover_info_box(void *data);
void mf_edit_view_create(void *data);
void mf_navi_bar_create_path_select_view(void *data);
bool mf_navi_bar_remove_list_item_by_label(void *data, const char *pNaviLabel);
void mf_navi_bar_recover_list(void *data);
void mf_navi_bar_set_ctrlbar_item_disable(Elm_Object_Item *navi_it, int disable_item, bool disable);
int mf_navi_bar_get_disable_ctrlbar_item(Elm_Object_Item *navi_it);
Evas_Object *mf_navi_bar_content_create(void *data);
Evas_Object *mf_navi_bar_home_button_create(Evas_Object *parent, Evas_Smart_Cb func, void *user_data);
Evas_Object *mf_navi_bar_upper_button_create(Evas_Object *parent, Evas_Smart_Cb func, void *user_data);
Evas_Object *mf_navi_bar_select_all_button_create(Evas_Object *parent, Evas_Smart_Cb func, void *user_data);
void mf_navi_bar_reset(void *data);
void mf_navi_bar_recover_state(void *data);
void mf_navi_bar_layout_content_set(Evas_Object *layout, Evas_Object *content);
char *mf_navi_bar_path_info_get(void *data, mf_navi_pathinfo_type type);
void mf_navi_bar_pathinfo_refresh(void *data);
Evas_Object *mf_navi_bar_search_button_create(Evas_Object *parent, Evas_Smart_Cb func, void *user_data);
Evas_Object *mf_naviframe_right_save_button_create(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
	Evas_Smart_Cb pFunc, void *pUserData);
Evas_Object *mf_naviframe_left_cancel_button_create(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
	Evas_Smart_Cb pFunc, void *pUserData);
void mf_naviframe_title_button_delete(Elm_Object_Item *navi_it);
Evas_Object *mf_navi_bar_create_download_view_content(void *data);

#endif //__MF_NAVI_BAR_H_DEF__
