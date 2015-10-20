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

#ifndef __MF_EDIT_VIEW_H_DEF__
#define __MF_EDIT_VIEW_H_DEF__

typedef void (*mf_edit_select_info) (void *);

void mf_edit_view_create(void *data);
void mf_edit_gengrid_item_sel_cb(void *data, Evas_Object * obj, void *event_info);
void mf_edit_list_item_sel_cb(void *data, Evas_Object * obj, void *event_info);
void mf_edit_item_sel_all_cb(void *data, Evas_Object * obj, void *event_info);
int mf_edit_file_count_get();
int mf_edit_folder_list_get_length();
int mf_edit_file_list_get_length();
Eina_List *mf_edit_get_all_selected_files();
void mf_edit_select_info_func_set(mf_edit_select_info func);
void mf_edit_select_all_callback_set(Evas_Smart_Cb func);
void mf_edit_folder_list_clear();
void mf_edit_file_list_clear();
void *mf_edit_file_list_item_get(int index);
void mf_edit_view_select_all_check(int count);
Eina_List *mf_edit_get_selected_folder_list();
Eina_List *mf_edit_get_selected_file_list();
void mf_edit_view_refresh(void *data, Eina_List **file_list, Eina_List **folder_list);
Eina_List * mf_edit_folder_list_get();
Eina_List * mf_edit_file_list_get();
bool mf_edit_file_list_item_exists(void *data);
void mf_edit_folder_list_item_remove(void *data);
bool mf_edit_folder_list_item_exists(void *data);
void mf_edit_file_list_append(void *data);
void mf_edit_folder_list_append(void *data);
void mf_edit_view_select_info_create(void *data);
void mf_edit_count_set(int count);
void mf_edit_select_all_set(Eina_Bool select_all_state);
void mf_edit_view_ctrlbar_state_set(void *data);
void *mf_edit_folder_list_item_get(int index);
void mf_edit_view_select_all_layout_remove(void *data);
void mf_edit_view_title_button_set(void *data);
void mf_edit_list_item_reset(void *data);
void mf_edit_select_all_check_set(Eina_Bool state);
void mf_edit_view_select_all_layout_prepend(void *data);
void mf_edit_list_item_sel_by_list_data(mf_list_data_t *selected, Evas_Object * obj,  Eina_Bool is_update_checkbox);
void mf_edit_file_list_item_remove(void *data);

#endif //__MF_EDIT_VIEW_H_DEF__
