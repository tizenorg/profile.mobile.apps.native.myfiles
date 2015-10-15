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

#ifndef __DEF_MYFILE_CALLBACK_H_
#define __DEF_MYFILE_CALLBACK_H_

#include "mf-main.h"

#define MF_MUSIC_DEFAULT_THUMBNAIL_FROM_DB "/opt/usr/media/.thumb/thumb_default.png"
void mf_callback_click_cb(struct appdata *data, mfAction key, GString * path);
void mf_callback_app_rotate_cb(void *data, Evas_Object *obj, void *event);
void mfNaviBackStatusSet(void *data, Evas_Object *obj, void *event_info);
void mfListByCB(void *data, Evas_Object *obj, void *event_info);
void mf_callback_share_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_cancel_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_share_button_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_rename_save_cb(void *ad, Evas_Object * obj, void *event_info);
void mf_callback_delete_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_max_len_reached_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_new_folder_create_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_list_by_view_cb(void *data, Evas_Object *obj, void *event_info);
void mfMoveCB(void *data, Evas_Object * obj, void *event_info);
void mf_callback_copy_move_cb(void *data, Evas_Object *obj, void *event_info);
void mfSearchbarCancelCallBack(void *data, Evas_Object *obj, void *event_info);

void mf_callback_move_here_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_paste_here_cb(void *data, Evas_Object *obj, void *event_info);
void mfCopyCB(void *data, Evas_Object *obj, void *event_info);
Eina_Bool mf_callback_thumb_timer_cb(void *data);

/*Callbacks defined for external usage*/

int mf_callback_set_mmc_state_cb(void *data);

void mf_callback_init_operation_cancel(void *data);
void mf_callback_progress_bar_cancel_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_thread_pipe_cb(void *data, void *buffer, unsigned int nbyte);
void mf_callback_icu_update_cb(app_event_info_h event_info, void *data);
void mf_callback_exception_popup_cb(void *data);
void mf_popup_show_vk_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_backbutton_clicked_cb(void *data, Evas_Object *obj, void *event_info);
Eina_Bool mf_callback_navi_backbutton_clicked_cb(void *data, Elm_Object_Item *it);
void mf_callback_upper_click_cb(void *data, Evas_Object * obj, void *event_info);
int mf_callback_set_mass_storage_state_cb(void *data);
void mf_callback_show_hidden_items_cb(void *data, Evas_Object * obj, void *event_info);
#ifdef MYFILE_DETAILS
void mf_callback_detail_button_cb(void *data, Evas_Object * obj, void *event_info);
#endif
void mf_callback_delete_button_cb(void *data, Evas_Object * obj, void *event_info);
Eina_Bool mf_callback_gengrid_thumb_timer_cb(void *data);
void mf_callback_view_style_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_home_button_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_more_button_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
void mf_callback_list_by_response_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_send_response_cb(void *data);

void mf_callback_operation_request_cancel_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_operation_request_replace_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_operation_request_rename_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_view_as_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_rename_create_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_new_folder_changed_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_extension_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_operation_timeout_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_nofity_show_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
void mf_callback_nofity_hide_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
void mf_callback_detail_ctx_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_profile_changed_cb(void *data, Evas_Object * obj, void *event_info);

void mf_callback_thumb_created_cb(media_content_error_e error, const char *path, void *user_data);
bool mf_callback_create_thumbnail(void *data, media_thumbnail_completed_cb callback);
void mf_callback_entry_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_naviframe_title_clicked_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_storage_remove_view_operation(void *data, int optStorage);
void mf_callback_storage_remove_category_view_items(void *data, int optStorage);
void mf_callback_imf_state_callback_register(void *data);
void mf_callback_imf_state_callback_del(void *data);
void mf_callback_illegal_char_popup_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_genlist_imf_changed_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_genlist_imf_preedit_change_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_entry_focused_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_mouseup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
void mf_callback_keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
void mf_callback_more_keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
void mf_callback_hardkey_more_cb(void *data, Elm_Object_Item *it, const char *emission, const char *source);
void mf_callback_hardkey_back_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_item_copy_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_item_move_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_item_add_to_shortcut_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_item_remove_from_shortcut_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_setting_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_new_folder_save_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_longpress_rename_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_item_remove_from_recent_files_cb(void *data, Evas_Object * obj, void *event_info);
void mf_download_update_idler_del();
void mf_callback_popup_deleted_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_unregister_mmc_state_cb();
void mf_callback_item_storage_usage_cb(void *data, Evas_Object * obj, void *event_info);
Eina_Bool mf_callback_monitor_internal_update_flag_get();
void mf_callback_monitor_internal_update_flag_set(Eina_Bool flag);
Eina_Bool mf_callback_monitor_media_db_update_flag_get();
void mf_callback_monitor_media_db_update_flag_set(Eina_Bool flag);

char *mf_callback_entry_text_get(Evas_Object *entry);
void mf_callback_progress_bar_state_cb(void *data);
void mf_callback_details_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_details_cb(void *data, Evas_Object *obj, void *event_info);

void mf_callback_storage_remove_flag_set(Eina_Bool flag, int more);
Eina_Bool mf_callback_storage_remove_flag_get(int *more);

void mf_callback_entry_unfocus(Evas_Object *entry);
void mf_callback_entry_focus();
void mf_callback_move_to_private_button_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_remove_from_private_button_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_move_to_private_button_from_edit_view_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_copy_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_move_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_delete_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_shortcut_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_rename_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_add_to_shortcut_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_do_add_to_shortcut_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_warning_popup_cb(void *data, Evas_Object * obj, void *event_info);
int mf_callback_idle_rename(void *data);
void mf_callback_edit_delete_shortcut_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_do_delete_shortcut_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_rename_shortcut_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_delete_recent_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_do_delete_recent_files(void *data, Evas_Object * obj, void *event_info);
void mf_callback_delete_shortcut_confirm_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_delete_recent_files_confirm_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_edit_unintall_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_unsupported_app_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_setting_popup_cb(void *data, Evas_Object * obj, void *event_info);
void mf_callback_clicked_cb(void *data, Evas_Object *obj, void *event_info);
void mf_callback_long_clicked_cb(void *data, Evas_Object *obj, void *event_info);

#endif //__DEF_MYFILE_CALLBACK_H_
