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

#ifndef __MF_POPUP_H_DEF__
#define __MF_POPUP_H_DEF__

#include <Elementary.h>
#include <Eina.h>
#include "mf-object-conf.h"

typedef enum __mf_operation_item_type_e mf_operation_item_type_e;
enum __mf_operation_item_type_e {
	mf_operation_item_rename,
	mf_operation_item_details,
	mf_operation_item_goto_folder,
	mf_operation_item_share,
	mf_operation_item_copy,
	mf_operation_item_move,
	mf_operation_item_delete,
	mf_operation_item_download,
	mf_operation_item_addto_shortcut,
	mf_operation_item_compress,
	mf_operation_item_decompress,
	mf_operation_item_decompress_here,
	mf_operation_item_remove,
	mf_operation_item_remove_recent,
};

typedef enum __message_type_e message_type_e;
enum __message_type_e {
	message_type_notification,
	message_type_popup,
};
Evas_Object *mf_popup_create_popup(void *data, ePopMode popupMode, char *title, const char *context, const char *first_btn_text, const char *second_btn_text,
				   const char *third_btn_text, Evas_Smart_Cb func, void *param);
Evas_Object *mf_popup_create_pb_popup(void *data, char *title, char *context, int file_count, void *func, void *param);
void mf_popup_indicator_popup(void *data, const char *text);
Evas_Object *mf_popup_center_processing(void *data,
				   const char *context,
				   const char *first_btn_text,
				   Evas_Smart_Cb func,
				   void *param,
				   Eina_Bool flag_backwork);
Evas_Object *mf_popup_create_new_folder_popup(void *data, char *context);
Evas_Object *mf_popup_create_rename_popup(void *data, char *context);
Evas_Object *mf_popup_create_operation_item_pop(void *data);
Evas_Object *mf_popup_create_compress_popup(void *data, char *context, char *name);
Evas_Object *mf_popup_create_decompress_popup(void *data, char *context, char *name);

Evas_Object *mf_popup_text(void *data, const char *context, Evas_Object_Event_Cb func, void *param);
Evas_Object *mf_popup_warning_popup_create(void *data, Evas_Object *parent, char *title, const char *context, const char *btn_text, Evas_Smart_Cb func, void *param);

Evas_Object *mf_popup_create_confirm_cancel_popup(char *title, const char *context, const char *cancel_btn_text, Evas_Smart_Cb cancel_func,
	 const char *confirm_btn_text, Evas_Smart_Cb confirm_func,void *data);
void mf_popup_create_folder_imf_changed_cb(void *data, Evas_Object *obj, void *event_info);
void mf_popup_new_folder_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
Evas_Object *mf_popup_entry_layout_create(Evas_Object *parent);
void mf_popup_timer_del();
Evas_Object *mf_popup_second_popup_create(void *data, Evas_Object *parent, const char *context, const char *btn_text, Evas_Smart_Cb func, void *param);
void mf_popup_second_popup_destory();
char *mf_popup_rename_item_name_get();
void mf_popup_rename_cancel();
void mf_popup_rename_func_set(Evas_Smart_Cb save_cb, void *save_params, Evas_Smart_Cb cancel_cb, void *cancel_params);
void mf_popup_rename_func_reset();
Evas_Object *mf_popup_share_as_video_or_image(void *func_for_video,void *func_for_image, void *data);
char *mf_popup_rename_text_get(const char *fullpath, char **suffix, Eina_Bool suffix_flag);
Evas_Object *mf_popup_entry_create(Evas_Object *parent);
Evas_Object *mf_popup_create_compress_popup_with_location(void *data, char *context, char *name);
Evas_Object *mf_popup_check_view_popup(void *data,
	const char *title,
	const char *text,
	const char *check_text,
	const char *first_btn_text,
	const char *second_btn_text,
	Evas_Smart_Cb func,
	void *param);
Eina_Bool mf_popup_check_view_flag_get();
void mf_popup_check_view_flag_set(Eina_Bool state);
Evas_Object *mf_popup_create_delete_confirm_popup(void *data, char *title, const char *context, const char *first_btn_text, const char *second_btn_text, Evas_Smart_Cb func, void *param, int count);
Evas_Object *mf_popup_sort_by_create(char *title, Evas_Smart_Cb func, void *param);
void mf_sort_by_respones_func_set(Evas_Smart_Cb func);
void mf_popup_ok_button_set(Evas_Object *obj);
void mf_popup_entry_string_set(char *string);
char *mf_popup_entry_string_get(void);
Evas_Object *mf_popup_replace_create(char *title, char *label_text, Evas_Smart_Cb func2, Evas_Smart_Cb func ,Evas_Smart_Cb func1, void *param);
#endif //__MF_POPUP_H_DEF__
