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

#include <stdio.h>
#include <dbus/dbus.h>
#include <pthread.h>
#include <storage.h>
#include "mf-main.h"
#include "mf-conf.h"
#include "mf-util.h"
#include "mf-callback.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-search-view.h"
#include "mf-fs-util.h"
#include "mf-launch.h"
#include "mf-dlog.h"
#include "mf-inotify-handle.h"
#include "mf-copy.h"
#include "mf-delete.h"
#include "mf-move.h"
#include "mf-resource.h"
#include "mf-share.h"
#include "mf-object-conf.h"
#include "mf-genlist.h"
#include "mf-gengrid.h"
#include "mf-navi-bar.h"
#include "mf-setting-view.h"
#include "mf-view.h"
#include "mf-object.h"
#include "mf-object-item.h"
#include "mf-popup.h"
#include "mf-context-popup.h"
#include "mf-util.h"
#include "mf-tray-item.h"
#include "mf-media-db.h"
#include "mf-media.h"
#include "mf-media-data.h"
#include "mf-edit-view.h"
#include "mf-thumb-gen.h"
#include "mf-fs-monitor.h"
#include "mf-search-view.h"
#include "mf-media-content.h"

#include "mf-ug-detail.h"
#include "mf-file-util.h"

#define MF_SHARE_ITEM_COUNT_MAX 500
#define MF_POPUP_MENUSTYLE_HEIGHT(x) (52*x)
#define MF_SORT_BY_ITEM 6
struct appdata *temp_data;
/* mutex for refresh */
/* mutex for dbus message */
static Eina_Bool monitor_internal_update_flag = EINA_FALSE;
static Eina_Bool monitor_media_db_update_flag = EINA_FALSE;

Ecore_Idler *g_thumbnail_download_update_idle = NULL;
static Evas_Object *max_length_entry = NULL;
int g_mf_create_thumbnail_count = 0;
#define MF_MAX_MAKE_THUNBNAIL_COUNT 10

void mf_callback_cancel_cb(void *data, Evas_Object *obj, void *event_info);
static void __mf_callback_mmc_removed(void *data, MF_STORAGE storage);
Eina_Bool mf_callback_is_duplicated_without_case(Eina_List *folder_list, char *name);

/* For saving the storage remove state for operation failed message */
static Eina_Bool mf_storage_remove_flag = EINA_FALSE;
static int mf_storage_remove_more = MORE_DEFAULT;
/* For saving the storage remove state for operation failed message */
void mf_recent_view_content_refresh(void *data);

#ifdef MYFILE_DETAILS
static void
__mf_load_detail_data(void *data, const char *path, bool is_multi)
{
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(path == NULL, "path is NULL");

	struct appdata *ap = (struct appdata *) data;
	mf_detail_data_destroy(ap);

	if (is_multi) {
		ap->mf_Status.detail = mf_ug_detail_multi_info_extract(path);
	} else {
		ap->mf_Status.detail = mf_ug_detail_common_info_extract(path);
	}
}
#endif

Eina_Bool mf_callback_storage_remove_flag_get(int *more)
{
	*more = mf_storage_remove_more;
	return mf_storage_remove_flag;
}

void mf_callback_storage_remove_flag_set(Eina_Bool flag, int more)
{
	mf_storage_remove_more = more;
	mf_storage_remove_flag = flag;
}

Eina_Bool mf_callback_monitor_internal_update_flag_get()
{
	return monitor_internal_update_flag;
}

void mf_callback_monitor_internal_update_flag_set(Eina_Bool flag)
{
	monitor_internal_update_flag = flag;
}

Eina_Bool mf_callback_monitor_media_db_update_flag_get()
{
	return monitor_media_db_update_flag;
}

void mf_callback_monitor_media_db_update_flag_set(Eina_Bool flag)
{
	monitor_media_db_update_flag = flag;
}

char *mf_callback_entry_text_get(Evas_Object *entry)
{
	mf_retvm_if(entry == NULL, NULL, "entry is NULL");
	char *entry_text = NULL;

	const char *entry_data = NULL;
	if (elm_entry_is_empty(entry)) {
		return NULL;
	}
	entry_data = elm_entry_entry_get(entry);

	if (entry_data) {
		entry_text = elm_entry_markup_to_utf8(entry_data);
	}
	return entry_text;
}

/******************************
** Prototype    : mf_callback_warning_popup_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_warning_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);

	if (g_strcmp0(label, mf_util_get_text(MF_BUTTON_LABEL_OK)) == 0) {
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
		ap->mf_MainWindow.pNormalPopup = NULL;
	}
}

/******************************
** Prototype    : mf_callback_app_rotate_cb
** Description  :
** Input        : enum appcore_rm mode
**                void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_app_rotate_cb(void *data, Evas_Object *obj, void *event)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = data;
	assert(ap);

	int changed_angle = elm_win_rotation_get(obj);
	/*
		enum ug_event new_event = UG_EVENT_ROTATE_PORTRAIT;
		switch (changed_angle) {
		case APP_DEVICE_ORIENTATION_0:
			new_event = UG_EVENT_ROTATE_PORTRAIT;
			break;
		case APP_DEVICE_ORIENTATION_90:
			new_event = UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN;
			break;
		case APP_DEVICE_ORIENTATION_180:
			new_event = UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN;
			break;
		case APP_DEVICE_ORIENTATION_270:
			new_event = UG_EVENT_ROTATE_LANDSCAPE;
			break;
		}
		ug_send_event(new_event);
	*/
	const char *config = elm_config_profile_get();
	if (!strcmp(config, "desktop")) {
		return;
	}


	if (mf_view_is_root_view(ap)) {
		if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
			//mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_root_all_horizon);
			//mf_category_refresh(ap);
		} else {
			//mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_root_all);
			//mf_category_refresh(ap);
		}
		mf_mw_root_category_item_update(ap);
	} else {
		if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
			evas_object_size_hint_min_set(ap->mf_MainWindow.pPopupBox, -1,
			                              ELM_SCALE_SIZE(MF_POPUP_MENUSTYLE_HEIGHT(MF_SORT_BY_ITEM / 2)));
		} else {
			evas_object_size_hint_min_set(ap->mf_MainWindow.pPopupBox, -1,
			                              ELM_SCALE_SIZE(MF_POPUP_MENUSTYLE_HEIGHT(MF_SORT_BY_ITEM)));
		}
		if (ap->mf_Status.view_type != mf_view_root_category && ap->mf_Status.view_type != mf_view_recent && ap->mf_Status.view_type != mf_view_storage) {
			mf_navi_bar_pathinfo_refresh(ap);
		}
	}


	if (ap->mf_MainWindow.pNaviGengrid) {
		mf_gengrid_align_set(ap->mf_MainWindow.pNaviGengrid, elm_gengrid_items_count(ap->mf_MainWindow.pNaviGengrid));
	}
	MF_TRACE_END;
}

/******************************
** Prototype    : click_callback
** Description  :
** Input        : struct appdata* data
**                mfAction key
**                GString* path
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_upper_click_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	assert(data);
	struct appdata *ap = data;

	mf_fs_monitor_remove_dir_watch();

	GString *parent_path = NULL;
	mf_error("ap->mf_Status.preViewType is [%d] view_type is [%d] ", ap->mf_Status.preViewType, ap->mf_Status.view_type);
	if (ap->mf_Status.view_type == mf_view_storage) {
		ap->mf_Status.view_type = mf_view_root;
		g_string_free(ap->mf_Status.path, TRUE);
		ap->mf_Status.path = g_string_new(PHONE_FOLDER);
	} else if (ap->mf_Status.view_type == mf_view_normal && mf_fm_svc_wrapper_is_root_path(ap->mf_Status.path->str)) {
		ap->mf_Status.view_type = mf_view_root;
		g_string_free(ap->mf_Status.path, TRUE);
		ap->mf_Status.path = g_string_new(PHONE_FOLDER);
	} else if (ap->mf_Status.view_type == mf_view_root_category) {
		ap->mf_Status.view_type = mf_view_root;
		g_string_free(ap->mf_Status.path, TRUE);
		ap->mf_Status.path = g_string_new(PHONE_FOLDER);
	} else if (ap->mf_Status.view_type == mf_view_recent) {
		ap->mf_Status.view_type = mf_view_root;
		g_string_free(ap->mf_Status.path, TRUE);
		ap->mf_Status.path = g_string_new(PHONE_FOLDER);
	} else {
		parent_path = mf_fm_svc_wrapper_get_file_parent_path(ap->mf_Status.path);
		g_string_free(ap->mf_Status.path, TRUE);
		ap->mf_Status.path = NULL;
		ap->mf_Status.path = parent_path;
	}
	SAFE_FREE_CHAR(ap->mf_Status.entry_path);
	ap->mf_Status.entry_more = MORE_DEFAULT;

	if (ap->mf_Status.more == MORE_DEFAULT && ap->mf_Status.view_type == mf_view_normal) {
		char *fullpath = mf_util_first_item_get(ap->mf_Status.path->str);
		mf_error("===================== fullpath is [%s]", fullpath);
		if (fullpath) {
			SAFE_FREE_CHAR(ap->mf_Status.entry_path);
			ap->mf_Status.EnterFrom = g_strdup(fullpath);
		}
	}

	mf_view_refresh_thumbnail_destroy();
	mf_view_update(ap);
	MF_TRACE_END;
}

void mf_callback_click_cb(struct appdata *data, mfAction key, GString *path)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	mf_retm_if(ap->mf_Status.path == NULL, "ap->mf_Status.path is NULL");
	mf_retm_if(ap->mf_Status.path->str == NULL, "ap->mf_Status.path->str is NULL");
	mf_retm_if(path == NULL, "path is NULL");
	mf_retm_if(path->str == NULL, "path->str is NULL");


	if (mf_fm_svc_wrapper_is_dir(path)) {
		if (ap->mf_Status.view_type == mf_view_storage || ap->mf_Status.view_type == mf_view_root) {
			if (path->str != NULL) {
				mf_debug("~~~~~~~~~~~  path->str [%s]", path->str);
				SAFE_FREE_GSTRING(ap->mf_Status.path);
				ap->mf_Status.path = g_string_new(path->str);
				ap->mf_Status.view_type = mf_view_normal;
				mf_view_refresh(ap);
			}
		} else {
			GString *new_path = NULL;
			if (ap->mf_Status.more == MORE_SEARCH) {
				if (ap->mf_FileOperation.search_result_folder_list) {
					mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.search_result_folder_list), MYFILE_TYPE_ITEM_DATA);
				}
				if (ap->mf_FileOperation.search_result_file_list) {
					mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.search_result_file_list), MYFILE_TYPE_ITEM_DATA);
				}
				elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
				ap->mf_Status.more = MORE_DEFAULT;
			}
			new_path = g_string_new(path->str);

			SAFE_FREE_GSTRING(ap->mf_Status.path);
			ap->mf_Status.path = new_path;
			mf_view_update(ap);

		}
	} else {
		struct timeval mytime;
		gettimeofday(&mytime, NULL);
		mf_debug("[myfiles click time] %ld sec %ld usec \n", mytime.tv_sec, mytime.tv_usec);

		int ret = -1;
		ret = mf_launch_service(ap, path->str);
		mf_debug("ret is %d\n", ret);
	}
}

/******************************
** Prototype    : mf_callback_illegal_char_popup_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_illegal_char_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);
	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);

	if ((g_strcmp0(label, mf_util_get_text(MF_BUTTON_LABEL_YES)) == 0) || (g_strcmp0(label, mf_util_get_text(MF_BUTTON_LABEL_OK)) == 0)) {
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
		ap->mf_MainWindow.pNormalPopup = NULL;

		Evas_Object *entry = ap->mf_MainWindow.pEntry;
		if (entry != NULL) {
			elm_object_focus_set(entry, EINA_TRUE);
		}
	} else if ((g_strcmp0(label, mf_util_get_text(MF_BUTTON_LABEL_NO)) == 0) || (g_strcmp0(label, mf_util_get_text(LABEL_CANCEL)) == 0)) {
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
		ap->mf_MainWindow.pNormalPopup = NULL;
		mf_callback_cancel_cb(ap, NULL, NULL);
	} else {//Fix the bug when pressing the back key.
		Evas_Object *entry = ap->mf_MainWindow.pEntry;
		if (entry != NULL) {
			elm_object_focus_set(entry, EINA_TRUE);
		}
	}
}

/******************************
** Prototype    : mf_callback_cancel_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	mf_retm_if(ap->mf_Status.path == NULL, "ap->mf_Status.path is NULL");
	mf_retm_if(ap->mf_Status.path->str == NULL, "ap->mf_Status.path->str is NULL");

	mf_error("more is [%d]", ap->mf_Status.more);

	Evas_Object *newContent;
	//mf_object_enable_virtualkeypad();

	switch (ap->mf_Status.more) {
	case MORE_INTERNAL_COPY:
	case MORE_INTERNAL_MOVE:
	case MORE_INTERNAL_COPY_MOVE:
		SAFE_FREE_GSTRING(ap->mf_FileOperation.source);
		/*1.    state set */
		/*1.1   status set to default */
		/*1.2   free the path to set it to the one just when edit clicked later */
		/*2.    navigation bar state recovery */

		SAFE_FREE_GSTRING(ap->mf_Status.path);
		ap->mf_Status.path = g_string_new(ap->mf_MainWindow.record.path);
		ap->mf_Status.view_type = ap->mf_MainWindow.record.view_type;
		mf_view_state_reset_state_with_pre(ap);
		ap->mf_Status.more = MORE_DEFAULT;
		mf_view_update(ap);
		break;
	case MORE_RENAME:
		if (mf_view_get_pre_state(ap) == MORE_EDIT) {
			mf_view_state_reset_state_with_pre(ap);

			Evas_Object *btn = elm_object_item_part_content_get(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN);
			if (btn) {
				elm_object_disabled_set(btn, EINA_FALSE);
			}
		}
		break;
		/* when cancle rename, just destory the rename relative, and then the mode will change to Edit
		   then do what cancle edit do, so here not need "break" */
	case MORE_SHARE_EDIT:
		ap->mf_Status.more = MORE_DEFAULT;
		SAFE_FREE_CHAR(ap->mf_Status.entry_path);
		ap->mf_Status.entry_more = MORE_DEFAULT;
		/*3.    refresh the content of the view */
		if (ap->mf_Status.view_type != mf_view_root_category && ap->mf_Status.more != MORE_SEARCH) {
			mf_navi_bar_recover_info_box(ap);
		}
		mf_naviframe_title_button_delete(ap->mf_MainWindow.pNaviItem);
		mf_view_update(ap);
		break;
	case MORE_EDIT:
	case MORE_EDIT_COPY:
	case MORE_EDIT_MOVE:
	case MORE_EDIT_DELETE:
	case MORE_EDIT_DETAIL:
		/*1     pop edit view */
		if (ap->mf_Status.extra == MORE_SEARCH) {
			ap->mf_Status.more = MORE_SEARCH;
			mf_search_view_create(ap);
		} else {
			ap->mf_Status.more = MORE_DEFAULT;
			SAFE_FREE_CHAR(ap->mf_Status.entry_path);
			ap->mf_Status.entry_more = MORE_DEFAULT;
			/*3.    refresh the content of the view */
			if (ap->mf_Status.view_type != mf_view_root_category && ap->mf_Status.more != MORE_SEARCH) {
				mf_navi_bar_recover_info_box(ap);
			}
			mf_naviframe_title_button_delete(ap->mf_MainWindow.pNaviItem);
			mf_view_update(ap);
		}
		/*4.    set tab enable */
		//mf_navi_bar_title_set(ap);
		break;
	case MORE_EDIT_DELETE_RECENT:
		ap->mf_Status.more = MORE_DEFAULT;
		SAFE_FREE_CHAR(ap->mf_Status.entry_path);
		ap->mf_Status.entry_more = MORE_DEFAULT;
		mf_view_update(ap);
		break;
	case MORE_EDIT_UNINSTALL:
		mf_error();
		ap->mf_Status.more = MORE_DEFAULT;
		mf_view_update(ap);
		break;
	case MORE_SEARCH:
		if (ap->mf_FileOperation.search_IME_hide_timer != NULL) {
			ecore_timer_del(ap->mf_FileOperation.search_IME_hide_timer);
			ap->mf_FileOperation.search_IME_hide_timer = NULL;
		}

		if (ap->mf_FileOperation.search_result_folder_list) {
			mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.search_result_folder_list), MYFILE_TYPE_ITEM_DATA);
		}
		if (ap->mf_FileOperation.search_result_file_list) {
			mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.search_result_file_list), MYFILE_TYPE_ITEM_DATA);
		}

		if (ap->mf_Status.search_handler > 0) {
			mf_search_stop(ap->mf_Status.search_handler);
			mf_search_finalize(&ap->mf_Status.search_handler);
		}
		ap->mf_Status.more = MORE_DEFAULT;
		elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
		SAFE_FREE_OBJ(ap->mf_MainWindow.pSearchBar);
		ap->mf_MainWindow.pSearchEntry = NULL;

		mf_view_update(ap);
		if (ap->mf_Status.view_type == mf_view_root_category) {
			mf_object_item_part_content_remove(ap->mf_MainWindow.pNaviItem, TITLE_LEFT_BTN);
			mf_object_item_part_content_remove(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN);
			//Evas_Object *search_btn = mf_navi_bar_search_button_create(ap->mf_MainWindow.pNaviBar, mf_search_bar_enter_search_routine, ap);
			//elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN, search_btn);
		}
		SAFE_FREE_CHAR(ap->mf_Status.search_filter);
		ap->mf_Status.flagSearchStart = EINA_FALSE;

		break;
	case MORE_THUMBNAIL_RENAME:
		if (ap->mf_Status.extra == MORE_SEARCH) {
			elm_object_focus_set(ap->mf_MainWindow.pEntry, EINA_FALSE);
			evas_object_del(ap->mf_MainWindow.pEntry);
			ap->mf_MainWindow.pEntry = NULL;
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
			ap->mf_Status.more = MORE_SEARCH;
			mf_search_view_create(ap);
		} else {
			mf_view_state_reset_state_with_pre(ap);
			ap->mf_Status.more = MORE_DEFAULT;
			elm_object_focus_set(ap->mf_MainWindow.pEntry, EINA_FALSE);
			evas_object_del(ap->mf_MainWindow.pEntry);
			ap->mf_MainWindow.pEntry = NULL;
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
		}
		break;
	case MORE_EDIT_RENAME:
		if (ap->mf_Status.extra == MORE_SEARCH) {
			elm_object_focus_set(ap->mf_MainWindow.pEntry, EINA_FALSE);
			evas_object_del(ap->mf_MainWindow.pEntry);
			ap->mf_MainWindow.pEntry = NULL;
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
			ap->mf_Status.more = MORE_SEARCH;
			mf_search_view_create(ap);
		} else {
			ap->mf_Status.more = MORE_DEFAULT;
			newContent = mf_navi_bar_content_create(ap);
			evas_object_show(newContent);
			mf_navi_bar_set_content(ap, ap->mf_MainWindow.pNaviLayout, newContent);
			mf_navi_bar_reset_ctrlbar(ap);
			mf_navi_add_back_button(ap, mf_callback_navi_backbutton_clicked_cb);
		}
		break;
	case MORE_SETTING:
		ap->mf_Status.more = MORE_DEFAULT;
		mf_view_refresh(ap);
		break;

	default:
		ap->mf_Status.more = MORE_DEFAULT;
		/*4.    set tab enable */
		break;
	}

	if (ap->mf_Status.view_type != mf_view_root) {
		mf_navi_add_back_button(ap, mf_callback_navi_backbutton_clicked_cb);
//		Evas_Object *pImage = elm_image_add(ap->mf_MainWindow.pNaviLayout);
//		elm_image_file_set(pImage, EDJ_IMAGE, MF_ICON_SOFT_BACK);
//		elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
//		evas_object_show(pImage);
//
//		Evas_Object *btn = elm_button_add(ap->mf_MainWindow.pNaviLayout);
//		elm_object_content_set(btn,pImage);
//		elm_object_style_set(btn, "transparent");
//		evas_object_smart_callback_add(btn, "clicked", mf_callback_backbutton_clicked_cb, ap);
//		//elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "back_key", btn);
//		elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, "title_left_btn", btn);
	}

	if (ap->mf_Status.more == MORE_DEFAULT) {
		if (ap->mf_Status.view_type == mf_view_root_category) {
			mf_navi_bar_title_content_set(ap, ap->mf_Status.categorytitle);
		} else if (ap->mf_Status.view_type == mf_view_recent) {
			mf_navi_bar_title_content_set(ap, MF_LABEL_RECTENT_FILES);
		} else {
			mf_navi_bar_title_content_set(ap, ap->mf_MainWindow.naviframe_title);
			elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_TRUE);
		}
	} else if (ap->mf_Status.more != MORE_EDIT_RENAME) {
		elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_FALSE, EINA_FALSE);
	}
}

void mf_callback_unsupported_app_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
	MF_TRACE_END;
}

/******************************
** Prototype    : __mf_popup_show_vk_cb
** Description  : Samsung
** Input        : void *data
**                Evas_Object * obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_popup_show_vk_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	mf_popup_second_popup_destory();
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
	mf_callback_entry_focus();
	MF_TRACE_END;

}

static int __mf_callback_idle_rename_search_refresh(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	mf_retvm_if(ap == NULL, 0, "ap is NULL");
	mf_search_bar_search_started_callback(ap, NULL, NULL);
	ap->mf_Status.operation_refresh_idler = NULL;
	ap->mf_Status.rename_timer = NULL;

	MF_TRACE_END;
	return ECORE_CALLBACK_CANCEL;
}

static void __mf_callback_refresh_rename(void *data, GString *pre_name, GString *new_name)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(pre_name == NULL, "pre_name is NULL");
	mf_retm_if(pre_name->str == NULL, "pre_name->str is NULL");
	mf_retm_if(new_name == NULL, "new_name is NULL");
	mf_retm_if(new_name->str == NULL, "new_name->str is NULL");
	struct appdata *ap = (struct appdata *)data;

	mfItemData_s *item_data = NULL;
	int view_style = mf_view_style_get(ap);

	item_data = elm_object_item_data_get(ap->mf_FileOperation.rename_item);
	if (item_data == NULL) {
		ap->mf_FileOperation.rename_item = NULL;
		return;
	}
#if 1
	if (mf_view_get_pre_state(ap) == MORE_SEARCH) {
		if (mf_is_dir(new_name->str)) {
			mf_ecore_idler_del(ap->mf_Status.operation_refresh_idler);
			ap->mf_Status.operation_refresh_idler = ecore_idler_add((Ecore_Task_Cb)__mf_callback_idle_rename_search_refresh, ap);
			return;
		}
	}
#endif
	if (ap->mf_Status.more == MORE_RENAME && mf_view_get_pre_state(ap) == MORE_EDIT) {
		Evas_Object *btn = NULL;
		btn = elm_object_item_part_content_get(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN);
		if (btn) {
			elm_object_disabled_set(btn, EINA_FALSE);
		}
	}

	if (g_string_equal(item_data->m_ItemName, pre_name)) {

		int hiden_state = 0;
		mf_util_get_pref_value(PREF_TYPE_HIDEN_STATE, &hiden_state);
		if (hiden_state == MF_HIDEN_HIDE) {
			if ((strncmp(new_name->str, ".", strlen(".")) == 0)) {
				elm_object_item_del(ap->mf_FileOperation.rename_item);
				ap->mf_FileOperation.rename_item = NULL;
			} else {
				g_string_free(item_data->m_ItemName, TRUE);
				item_data->m_ItemName = g_string_new(new_name->str);

				i18n_udate date = 0;
				int ret = 0;
				ret = mf_file_attr_get_file_mdate(item_data->m_ItemName->str, &date);
				if (ret == MYFILE_ERR_NONE) {
					item_data->modify_time = date;
				}

				if (!mf_is_dir(item_data->m_ItemName->str)) {
					SAFE_FREE_CHAR(item_data->thumb_path);
					item_data->real_thumb_flag = EINA_FALSE;
					mf_genlist_get_thumbnail(item_data);
				}

				elm_object_item_data_set(ap->mf_FileOperation.rename_item, item_data);
				if (view_style == MF_VIEW_STYLE_LIST || view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
					if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME || ap->mf_Status.more == MORE_EDIT_RENAME) {
						elm_genlist_item_update(ap->mf_FileOperation.rename_item);
					}
				} else {
					elm_gengrid_item_update(ap->mf_FileOperation.rename_item);
				}
				if (ap->mf_Status.view_type == mf_view_root_category) {
					mf_util_update_item_from_list_by_name(&ap->mf_FileOperation.category_list, pre_name->str, new_name->str);
				}
			}
		} else {
			g_string_free(item_data->m_ItemName, TRUE);
			item_data->m_ItemName = g_string_new(new_name->str);

			i18n_udate date = 0;
			int ret = 0;
			ret = mf_file_attr_get_file_mdate(item_data->m_ItemName->str, &date);
			if (ret == MYFILE_ERR_NONE) {
				item_data->modify_time = date;
			}

			if (!mf_is_dir(item_data->m_ItemName->str)) {
				SAFE_FREE_CHAR(item_data->thumb_path);
				item_data->real_thumb_flag = EINA_FALSE;
				mf_genlist_get_thumbnail(item_data);
			}

			elm_object_item_data_set(ap->mf_FileOperation.rename_item, item_data);
			if (view_style == MF_VIEW_STYLE_LIST || view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
				if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME || ap->mf_Status.more == MORE_EDIT_RENAME) {
					elm_genlist_item_update(ap->mf_FileOperation.rename_item);
				}
			} else {
				elm_gengrid_item_update(ap->mf_FileOperation.rename_item);
			}
			if (ap->mf_Status.view_type == mf_view_root_category) {
				mf_util_update_item_from_list_by_name(&ap->mf_FileOperation.category_list, pre_name->str, new_name->str);
			}
		}
	}
	if (mf_view_get_pre_state(ap) == MORE_SEARCH) {
		if (!mf_util_NFD_strstr(new_name->str, ap->mf_Status.search_filter)) {
			elm_object_item_del(ap->mf_FileOperation.rename_item);
			ap->mf_FileOperation.rename_item = NULL;
		}
	}
}

/******************************
** Prototype    : mf_callback_rename_save_cb
** Description  :
** Input        : void *ad
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static int __mf_callback_idle_rename_refresh(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, (ap->mf_Status.operation_refresh_idler = NULL, ECORE_CALLBACK_CANCEL), "ap is NULL");

	mf_view_update(ap);
	ap->mf_Status.operation_refresh_idler = NULL;
	mf_callback_monitor_internal_update_flag_set(EINA_FALSE);

	return ECORE_CALLBACK_CANCEL;
}

void mf_callback_rename_save_cb(void *ad, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mfItemData_s *item_data = (mfItemData_s *)ad;
	mf_retm_if(item_data == NULL, "ap is NULL");
	struct appdata *ap = (struct appdata *)item_data->ap;
	mf_retm_if(ap == NULL, "ap is NULL");

	char *name = NULL;
	Evas_Object *pEntry = NULL;
	const char *entry_data = NULL;
	int ret = MYFILE_ERR_NONE;
	const char *message = NULL;
	GString *from = NULL;
	GString *to = NULL;
	char *pFullPath = NULL;
	char *pName = NULL;
	char *strstrip_name = NULL;
	if (ap->mf_Status.more != MORE_RENAME && ap->mf_Status.more != MORE_THUMBNAIL_RENAME  && ap->mf_Status.more != MORE_EDIT_RENAME) {
		MF_TRACE_END;
		return;
	}

	pEntry = ap->mf_MainWindow.pEntry;
	entry_data = elm_entry_entry_get(pEntry);
	if (entry_data) {
		char *entry_string = mf_popup_entry_string_get();
		if (entry_string && g_strcmp0(entry_data, entry_string) == 0) {
			if (mf_is_dir(ap->mf_FileOperation.to_rename->str)) {
				mf_popup_indicator_popup(ap, MF_MSG_FOLDER_DIR_IN_USE);
			} else {
				mf_popup_indicator_popup(ap, MF_MSG_FILE_NAME_IN_USE);
			}
			return;
		}
		strstrip_name = elm_entry_markup_to_utf8(entry_data);
		name = g_strstrip(strstrip_name);
		if (name == NULL) {
			message = MF_RENAME_MSG_GET_NAME_FAILED;
			goto INVAILD_NAME_EXIT;
		}
		if (strlen(name) == 0) {
			message = MF_POPUP_MSG_NAME_INVALID;	/*TODO */
			goto INVAILD_NAME_EXIT;
		}
		if (strlen(name) == 0 && (ap->mf_Status.more == MORE_THUMBNAIL_RENAME || ap->mf_Status.more == MORE_EDIT_RENAME)) {
			SAFE_FREE_CHAR(strstrip_name);
			goto INVAILD_NAME_EXIT;
		}
	} else {
		message = MF_RENAME_MSG_GET_NAME_FAILED;
		goto INVAILD_NAME_EXIT;
	}

	if (strlen(name)) {

		if (ap->mf_FileOperation.file_name_suffix != NULL && strlen(ap->mf_FileOperation.file_name_suffix) > 0) {
			pName = g_strconcat(name, ".", ap->mf_FileOperation.file_name_suffix, NULL);
			CHAR_CHECK_NULL_GOTO(pName, ALLOC_FAILED_EXIT);
		} else {
			pName = g_strdup(name);
			CHAR_CHECK_NULL_GOTO(pName, ALLOC_FAILED_EXIT);
		}

		GString *parent_path = mf_fm_svc_wrapper_get_file_parent_path(ap->mf_FileOperation.to_rename);
		if ((parent_path) == NULL || (parent_path->str) == NULL) {
			goto ALLOC_FAILED_EXIT;
		}
		pFullPath = g_strconcat(parent_path->str, "/", pName, NULL);
		CHAR_CHECK_NULL_GOTO(pFullPath, ALLOC_FAILED_EXIT);
		SAFE_FREE_GSTRING(parent_path);

		switch (mf_util_is_valid_name_check(pName)) {
		case MF_INTERNAL_FILE_NAME_EMPTY:
		case MF_INTERNAL_FILE_NAME_IGNORE:
			message = MF_MSG_SET_NAME_DOT;
			goto INVAILD_NAME_EXIT;
		case MF_INTERNAL_FILE_NAME_CHUG:
			message = MF_MSG_SET_NAME_ALL_SPACE;
			goto INVAILD_NAME_EXIT;
		case MF_INTERNAL_FILE_NAME_MAX_LENGTH:
			message = MF_LABEL_MAX_CHARACTER_REACHED;
			goto INVAILD_NAME_EXIT;
		case MF_INTERNAL_FILE_NAME_INVALID_CHAR:
			message = MF_MSG_ILLEGAL_CHAR;
			goto INVAILD_NAME_EXIT;
		case MF_INTERNAL_FILE_NAME_NULL:
			message = MF_MSG_NO_NAME_WARNING;
			goto INVAILD_NAME_EXIT;
		default:
			break;
		}

		if (mf_is_dir(ap->mf_FileOperation.to_rename->str)) {
			if (strncmp(name, ".", strlen(".")) == 0 || strncmp(name, "..", strlen("..")) == 0) {
				message = MF_MSG_SET_NAME_DOT;
				SAFE_FREE_CHAR(strstrip_name);
				SAFE_FREE_CHAR(pName);
				SAFE_FREE_CHAR(pFullPath);
				mf_callback_monitor_media_db_update_flag_set(EINA_FALSE);
				mf_callback_monitor_internal_update_flag_set(EINA_FALSE);
				mf_popup_indicator_popup(ap, message);
				return;
			}
			if (mf_callback_is_duplicated_without_case(ap->mf_FileOperation.folder_list, pName)) {
				message = MF_MSG_FOLDER_DIR_IN_USE;
				goto INVAILD_NAME_EXIT;
			}
		}
		if (strlen(pFullPath) > MYFILE_FILE_PATH_LEN_MAX) {
			message = MF_LABEL_MAX_CHARACTER_REACHED;
			goto INVAILD_NAME_EXIT;
		}

		from = g_string_new(ap->mf_FileOperation.to_rename->str);
		GSTRING_CHECK_NULL_GOTO(from, ALLOC_FAILED_EXIT);
		to = g_string_new(pFullPath);
		GSTRING_CHECK_NULL_GOTO(from, ALLOC_FAILED_EXIT);

		if (!g_string_equal(from, to)) {
			mf_callback_monitor_internal_update_flag_set(EINA_TRUE);
			ret = mf_fm_svc_wrapper_rename_service(ap, from, to);
			if (ret != MYFILE_ERR_NONE) {
				if (errno == EROFS) {
					message = MF_MSG_OPER_READ_ONLY;
					goto INVAILD_NAME_EXIT;
				} else if (ret == MYFILE_ERR_INVALID_DIR_NAME || ret == MYFILE_ERR_INVALID_FILE_NAME) {
					message = MF_MSG_ILLEGAL_CHAR;
					goto INVAILD_NAME_EXIT;
				} else if (ret == MYFILE_ERR_DUPLICATED_NAME) {
					if (mf_is_dir(from->str)) {
						message = MF_MSG_FOLDER_DIR_IN_USE;
					} else {
						message = MF_MSG_FILE_NAME_IN_USE;
					}
					goto INVAILD_NAME_EXIT;
				} else {
					message = MF_MSG_UNKNOW_REASON_RENAME_FAILED;
					goto INVAILD_NAME_EXIT;
				}
			} else {
				mf_callback_monitor_media_db_update_flag_set(EINA_TRUE);
				if (item_data->file_type != FILE_TYPE_DIR) {
					if (strstr(from->str, "/.") != NULL && strstr(to->str, "/.") == NULL) {
						mf_media_content_scan_file(to->str);
					} else if (strstr(from->str, "/.") == NULL && strstr(to->str, "/.") != NULL) {
						mf_media_content_scan_file(from->str);
					} else if (strstr(from->str, "/.") == NULL && strstr(to->str, "/.") == NULL) {
						mf_media_content_scan_file(from->str);
						mf_media_content_scan_file(to->str);
					}
				}
				if (ap->mf_Status.more == MORE_RENAME) {
					__mf_callback_refresh_rename(ap, from, to);
				} else if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME || ap->mf_Status.more == MORE_EDIT_RENAME) {
					elm_object_focus_set(ap->mf_MainWindow.pEntry, EINA_FALSE);
					evas_object_del(ap->mf_MainWindow.pEntry);
					ap->mf_MainWindow.pEntry = NULL;
					if (ap->mf_Status.view_type == mf_view_root_category) {
						mf_util_update_item_from_list_by_name(&ap->mf_FileOperation.category_list, from->str, to->str);
						if (ap->mf_Status.more == MORE_DEFAULT) {
							__mf_callback_refresh_rename(ap, from, to);
							SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
							goto NORMAL_EXIT;
						}
					}
					if (ap->mf_Status.view_type == mf_view_root) {
						mf_update_recent_files_name(ap->mf_MainWindow.mfd_handle, to->str, from->str);
					}

					if (mf_view_get_pre_state(ap) == MORE_SEARCH) {// || mf_view_get_pre_state(ap) == MORE_DEFAULT) {//ap->mf_Status.view_type == mf_view_root_category) {
						__mf_callback_refresh_rename(ap, from, to);
						SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
						goto NORMAL_EXIT;
					}
					// int view_style = mf_view_style_get(ap);  /* blocked becoz not used */
					SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
					ap->mf_Status.EnterFrom = strdup(to->str);
					mf_error("ap->mf_Status.EnterFrom = %s", ap->mf_Status.EnterFrom);
					mf_view_state_reset_state_with_pre(ap);
					mf_ecore_idler_del(ap->mf_Status.operation_refresh_idler);
					ap->mf_Status.operation_refresh_idler = ecore_idler_add((Ecore_Task_Cb)__mf_callback_idle_rename_refresh, ap);
				}
				goto NORMAL_EXIT;
			}
		} else {

			if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME || ap->mf_Status.more == MORE_EDIT_RENAME) {
				mf_callback_cancel_cb(ap, NULL, NULL);
			}
			goto NORMAL_EXIT;
		}
	} else {
		goto NORMAL_EXIT;
	}
NORMAL_EXIT:
	SAFE_FREE_CHAR(strstrip_name);
	SAFE_FREE_CHAR(pName);
	SAFE_FREE_CHAR(pFullPath);
	SAFE_FREE_GSTRING(from);
	SAFE_FREE_GSTRING(to);
	mf_callback_monitor_internal_update_flag_set(EINA_FALSE);
	MF_TRACE_END;
	return;

INVAILD_NAME_EXIT:
	SAFE_FREE_CHAR(strstrip_name);
	SAFE_FREE_CHAR(pName);
	SAFE_FREE_CHAR(pFullPath);
	SAFE_FREE_GSTRING(from);
	SAFE_FREE_GSTRING(to);
	if (ap->mf_Status.more == MORE_RENAME && mf_view_get_pre_state(ap) == MORE_EDIT) {
		Evas_Object *btn = NULL;
		btn = elm_object_item_part_content_get(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN);
		if (btn) {
			elm_object_disabled_set(btn, EINA_FALSE);
		}
	}

	if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME || ap->mf_Status.more == MORE_EDIT_RENAME) {
		//ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_TWO_BTN, NULL, message,LABEL_CANCEL, MF_BUTTON_LABEL_OK, NULL, mf_callback_illegal_char_popup_cb, ap);
		mf_callback_entry_unfocus(pEntry);
		mf_popup_second_popup_create(ap, ap->mf_MainWindow.pWindow, message,
		                             MF_BUTTON_LABEL_OK, mf_popup_show_vk_cb, ap);
		mf_callback_monitor_media_db_update_flag_set(EINA_FALSE);
		mf_callback_monitor_internal_update_flag_set(EINA_FALSE);
		return;
	} else {
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL, message, MF_BUTTON_LABEL_OK, NULL, NULL, mf_popup_show_vk_cb, ap);
	}
	mf_callback_monitor_media_db_update_flag_set(EINA_FALSE);
	mf_callback_monitor_internal_update_flag_set(EINA_FALSE);

	MF_TRACE_END;
	return;

ALLOC_FAILED_EXIT:
	SAFE_FREE_CHAR(strstrip_name);
	SAFE_FREE_CHAR(pName);
	SAFE_FREE_CHAR(pFullPath);
	SAFE_FREE_GSTRING(from);
	mf_util_operation_alloc_failed(ap);
	mf_callback_monitor_media_db_update_flag_set(EINA_FALSE);
	mf_callback_monitor_internal_update_flag_set(EINA_FALSE);
	MF_TRACE_END;
	return;
}

/******************************
** Prototype    : mf_callback_rename_save_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_popup_error_exit_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	if (ap->mf_MainWindow.pNormalPopup) {//Add the protection
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
		ap->mf_MainWindow.pNormalPopup = NULL;
	}
	MF_TRACE_END;

}

Eina_Bool mf_callback_is_duplicated_without_case(Eina_List *folder_list, char *name)
{
	mf_retvm_if(folder_list == NULL, EINA_FALSE, "folder_list is NULL");

	if (eina_list_count(folder_list) == 0) {
		return EINA_FALSE;
	}

	fsNodeInfo *pNode = NULL;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(folder_list, l, pNode) {
		if (pNode) {
			if (pNode->name && mf_file_exists(pNode->name)) {
				if (strcasecmp(name, pNode->name) == 0) {
					return EINA_TRUE;
				}
			}
		}
	}
	return EINA_FALSE;
}

void mf_callback_new_folder_save_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);
	int ret = 0;
	//prevent issue
	//int noContentFlag = 0;
	const char *message = NULL;
	char *name = NULL;
	char *test_space = NULL;
	char *fullpath = NULL;
	{
		mf_object_entry_unfocus(ap->mf_MainWindow.pEntry);
		name = mf_callback_entry_text_get(ap->mf_MainWindow.pEntry);

		SECURE_DEBUG("ap is [%p], name is [%s]", ap, name);

		if (name) {
			if (strlen(name)) {
				test_space = g_strstrip(name);
				if (strlen(test_space) == 0) {
					message = MF_POPUP_MSG_NAME_INVALID;	/*TODO */
					goto ERROR_WARNING_EXIT;
				}
				if (mf_file_attr_is_valid_name(test_space) == MYFILE_ERR_INVALID_FILE_NAME) {
					message = MF_MSG_ILLEGAL_CHAR;
					mf_debug("Name contains illegal character!!!");
					goto ERROR_WARNING_EXIT;
				}
				if (strncmp(test_space, ".", strlen(".")) == 0 || strncmp(test_space, "..", strlen("..")) == 0) {
					message = MF_MSG_SET_NAME_DOT;
					SAFE_FREE_CHAR(name);
					SAFE_FREE_CHAR(fullpath);
					mf_popup_indicator_popup(ap, message);
					return;
				}

				fullpath = g_strconcat(ap->mf_Status.path->str, "/", test_space, NULL);
				if (!fullpath) {
					message = MF_MSG_CREATE_DIR_FAILED;
					goto ERROR_EXIT_CREATE;
				}

				/*check whether DIR name is override(DIR name has no extention) */
				/*check whether path length is override */
				if ((strlen(ap->mf_Status.path->str) + strlen(test_space)) > MYFILE_FILE_PATH_LEN_MAX) {

					message = MF_LABEL_MAX_CHARACTER_REACHED;
					goto ERROR_EXIT_CREATE;
				}
				/*check if duplicated name */
				if (mf_file_attr_is_duplicated_name(ap->mf_Status.path->str, test_space) == MYFILE_ERR_DUPLICATED_NAME) {
					message = MF_LABEL_FOLDER_NAME_ALREADY_INUSE;
					goto ERROR_WARNING_EXIT;
				}

				if (mf_callback_is_duplicated_without_case(ap->mf_FileOperation.folder_list, test_space)) {
					message = MF_LABEL_FOLDER_NAME_ALREADY_INUSE;
					goto ERROR_WARNING_EXIT;
				}
				/*check if DIR name is all spaces */
				ret = mf_fm_svc_wrapper_create_service(ap, fullpath);
				/*check whether operate on read only area */
				if (ret == MYFILE_ERR_READ_ONLY) {
					message = MF_MSG_OPER_READ_ONLY;
					goto ERROR_EXIT_CREATE;
				} else if (ret == MYFILE_ERR_NO_FREE_SPACE) {
					message = MF_LABE_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS_AND_TRY_AGAIN;
					goto ERROR_EXIT_CREATE;
				} else if (ret) {
					message = MF_MSG_CREATE_DIR_FAILED;
					goto ERROR_EXIT_CREATE;
				} else {
					ap->mf_Status.more = ap->mf_Status.pre_create_more;
					ap->mf_Status.pre_create_more = MORE_DEFAULT;
					evas_object_del(ap->mf_MainWindow.pEntry);
					ap->mf_MainWindow.pEntry = NULL;
					SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
					mf_view_update(ap);
					//Prevent issue
					/*if (ap->mf_Status.more == MORE_DEFAULT && noContentFlag) {
						mf_navi_bar_set_ctrlbar_item_disable(ap->mf_MainWindow.pNaviItem, CTRL_DISABLE_NOCONTENT_VIEW, false);
					}*/
				}
			} else {
				message = MF_MSG_EMPTY_FOLDER_NAME;
				goto ERROR_WARNING_EXIT;
			}
		} else {
			message = MF_MSG_GET_NAME_FAILED;
			goto ERROR_EXIT_CREATE;
		}
	}

	SAFE_FREE_CHAR(name);
	return;

ERROR_WARNING_EXIT:
	SAFE_FREE_CHAR(name);
	SAFE_FREE_CHAR(fullpath);
	mf_callback_entry_unfocus(ap->mf_MainWindow.pEntry);
	mf_popup_warning_popup_create(ap, ap->mf_MainWindow.pWindow, MF_POP_MAX_WARNING_TITEL, message, MF_BUTTON_LABEL_OK, mf_popup_show_vk_cb, ap);
	return;
ERROR_EXIT_CREATE:
	SAFE_FREE_CHAR(name);
	SAFE_FREE_CHAR(fullpath);
	ap->mf_Status.more = ap->mf_Status.pre_create_more;
	ap->mf_Status.pre_create_more = MORE_DEFAULT;
	evas_object_del(ap->mf_MainWindow.pEntry);
	ap->mf_MainWindow.pEntry = NULL;
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
	ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL, message,
	                                 MF_BUTTON_LABEL_OK, NULL, NULL, mf_popup_error_exit_cb, ap);

	return;

}

void mf_callback_popup_deleted_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	if (ap->mf_MainWindow.pNormalPopup != NULL) {
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
	}
	ap->mf_MainWindow.pNormalPopup = NULL;
}

/******************************
** Prototype    : __mf_callback_list_by_response_cb
** Description  : Samsung
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_list_by_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);

	Evas_Object *playout = ap->mf_MainWindow.pNaviLayout;
	mf_retm_if(playout == NULL, "get conformant failed");
	Evas_Object *newContent = NULL;

	if (ap->mf_Status.flagNoContent != EINA_TRUE) {
		if (ap->mf_Status.view_type == mf_view_recent) {
			mf_view_update(ap);
		} else {
			newContent = mf_navi_bar_content_create(ap);
			mf_navi_bar_set_content(ap, playout, newContent);
		}
	}
	ap->mf_Status.more = MORE_DEFAULT;
}

/******************************
** Prototype    : mf_callback_list_by_view_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_list_by_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	mf_debug();
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	mf_sort_by_respones_func_set(mf_callback_list_by_response_cb);
	ap->mf_MainWindow.pNormalPopup = mf_popup_sort_by_create(LABEL_SORT_BY_CHAP, mf_callback_popup_deleted_cb, ap);
}

/******************************
** Prototype    : mf_callback_edit_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_callback_save_edit_view_status(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata*)data;
	/*backup the edit point for every navigation bar to get recovery */

	SAFE_FREE_CHAR(ap->mf_MainWindow.record.path);
	ap->mf_MainWindow.record.path = g_strdup(ap->mf_Status.path->str);
	ap->mf_MainWindow.record.view_type = ap->mf_Status.view_type;
	ap->mf_MainWindow.record.location = ap->mf_MainWindow.location;
	ap->mf_MainWindow.record.more = ap->mf_Status.more;

	//ap->mf_Status.preViewType = ap->mf_Status.view_type;  todo

	MF_TRACE_END;
}

static void __mf_callback_edit_share_view(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	/*backup the edit point for every navigation bar to get recovery */
	__mf_callback_save_edit_view_status(ap);

	mf_edit_view_create(ap);

	MF_TRACE_END;

}

void mf_callback_share_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	if (ap->mf_MainWindow.pButton) {
		elm_object_part_content_unset(ap->mf_MainWindow.pNaviLayout, "search_icon");
		evas_object_hide(ap->mf_MainWindow.pButton);
	}
	if (ap->mf_Status.more == MORE_SEARCH) {
		if (ap->mf_MainWindow.pNaviSearchBar) {
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "hide", "search_bar");
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "show", "elm.swallow.content");
			//ap->mf_Status.extra = MORE_SHARE_EDIT;
		}
	} else {
		ap->mf_Status.extra = MORE_SHARE_EDIT;
	}

	ap->mf_Status.more = MORE_SHARE_EDIT;
	__mf_callback_edit_share_view(ap);
	MF_TRACE_END;
}

void mf_callback_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);

	ap->mf_Status.more = MORE_EDIT;
	__mf_callback_edit_share_view(ap);

	/*disable all the tab item if tab exists */
	MF_TRACE_END;
}

void mf_callback_edit_copy_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	if (ap->mf_MainWindow.pButton) {
		elm_object_part_content_unset(ap->mf_MainWindow.pNaviLayout, "search_icon");
		evas_object_hide(ap->mf_MainWindow.pButton);
	}

	if (ap->mf_Status.more == MORE_SEARCH) {
		if (ap->mf_MainWindow.pNaviSearchBar) {
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "hide", "search_bar");
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "show", "elm.swallow.content");
			//ap->mf_Status.extra = MORE_SHARE_EDIT;
		}
	} else {
		ap->mf_Status.extra = MORE_SHARE_EDIT;
	}
	ap->mf_Status.more = MORE_EDIT_COPY;
	__mf_callback_edit_share_view(ap);

	/*disable all the tab item if tab exists */
	MF_TRACE_END;
}

void mf_callback_edit_move_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	if (ap->mf_MainWindow.pButton) {
		elm_object_part_content_unset(ap->mf_MainWindow.pNaviLayout, "search_icon");
		evas_object_hide(ap->mf_MainWindow.pButton);
	}
	if (ap->mf_Status.more == MORE_SEARCH) {
		if (ap->mf_MainWindow.pNaviSearchBar) {
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "hide", "search_bar");
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "show", "elm.swallow.content");
			//ap->mf_Status.extra = MORE_EDIT_MOVE;
		}
	} else {
		ap->mf_Status.extra = MORE_EDIT_MOVE;
	}

	ap->mf_Status.more = MORE_EDIT_MOVE;
	__mf_callback_edit_share_view(ap);

	/*disable all the tab item if tab exists */
	MF_TRACE_END;
}

void mf_callback_edit_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	if (ap->mf_MainWindow.pButton) {
		elm_object_part_content_unset(ap->mf_MainWindow.pNaviLayout, "search_icon");
		evas_object_hide(ap->mf_MainWindow.pButton);
	}
	if (ap->mf_Status.more == MORE_SEARCH) {
		if (ap->mf_MainWindow.pNaviSearchBar) {
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "hide", "search_bar");
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "show", "elm.swallow.content");
			//ap->mf_Status.extra = MORE_EDIT_DELETE;
		}
	} else {
		ap->mf_Status.extra = MORE_EDIT_DELETE;
	}

	ap->mf_Status.more = MORE_EDIT_DELETE;
	__mf_callback_edit_share_view(ap);

	/*disable all the tab item if tab exists */
	MF_TRACE_END;
}

void mf_callback_edit_rename_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	if (ap->mf_Status.more == MORE_SEARCH) {
		if (ap->mf_MainWindow.pNaviSearchBar) {
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "hide", "search_bar");
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "show", "elm.swallow.content");
			//ap->mf_Status.extra = MORE_EDIT_RENAME;
		}
	} else {
		ap->mf_Status.extra = MORE_EDIT_RENAME;
	}
	ap->mf_Status.more = MORE_EDIT_RENAME;
	elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_FALSE);
	mf_navi_bar_title_content_set(ap, LABEL_RENAME);
	if (ap->mf_MainWindow.pButton) {
		elm_object_part_content_unset(ap->mf_MainWindow.pNaviLayout, "search_icon");
		evas_object_hide(ap->mf_MainWindow.pButton);
	}
	if (ap->mf_MainWindow.pNaviItem) {
		Evas_Object *unset = elm_object_item_part_content_unset(ap->mf_MainWindow.pNaviItem, "prev_btn");
		evas_object_hide(unset);
	}
	//__mf_callback_edit_share_view(ap);

	/*disable all the tab item if tab exists */
	MF_TRACE_END;
}

void mf_callback_edit_delete_recent_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	if (ap->mf_MainWindow.pButton) {
		elm_object_part_content_unset(ap->mf_MainWindow.pNaviLayout, "search_icon");
		evas_object_hide(ap->mf_MainWindow.pButton);
	}
	if (ap->mf_Status.more == MORE_SEARCH) {
		if (ap->mf_MainWindow.pNaviSearchBar) {
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "hide", "search_bar");
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "show", "elm.swallow.content");
			//ap->mf_Status.extra = MORE_EDIT_DELETE_RECENT;
		}
	} else {
		ap->mf_Status.extra = MORE_EDIT_DELETE_RECENT;
	}
	ap->mf_Status.more = MORE_EDIT_DELETE_RECENT;
	__mf_callback_edit_share_view(ap);
	MF_TRACE_END;
}

void mf_callback_do_delete_recent_files(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);

	SAFE_FREE_OBJ(ap->mf_MainWindow.pDeleteConfirmPopup);

	if (g_strcmp0(label, mf_util_get_text(LABEL_CANCEL)) != 0) {
		ap->mf_MainWindow.pProgressPopup = mf_popup_create_pb_popup(ap, LABEL_DELETE, MF_MSG_REMOVING, 0, NULL, ap);
		elm_popup_align_set(ap->mf_MainWindow.pProgressPopup, 0.5, 0.5);
		Eina_List * selected_list = mf_edit_file_list_get();
		mfItemData_s *item_data = NULL;
		Elm_Object_Item *it = NULL;
		Eina_List * l = NULL;
		EINA_LIST_FOREACH(selected_list, l, it) {
			if (it) {
				item_data = elm_object_item_data_get(it);
				if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
					mf_error("item_data->m_ItemName->str is [%s]", item_data->m_ItemName->str);
					mf_util_db_remove_recent_files(ap->mf_MainWindow.mfd_handle, item_data->m_ItemName->str);
				}
			}
		}
		elm_popup_timeout_set(ap->mf_MainWindow.pProgressPopup, 0.3);
		ap->mf_Status.more = MORE_DEFAULT;
		mf_view_update(ap);
	}
}

void mf_callback_delete_recent_files_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	int count = mf_edit_file_count_get();
	if (count > 1) {
		ap->mf_MainWindow.pDeleteConfirmPopup = mf_popup_create_delete_confirm_popup(ap, MF_LABEL_REMOVE,
		                                        MF_LABEL_DELETE_RECENT_Q,
		                                        LABEL_CANCEL, MF_LABEL_REMOVE, mf_callback_do_delete_recent_files, ap, count);
	} else {
		ap->mf_MainWindow.pDeleteConfirmPopup = mf_popup_create_delete_confirm_popup(ap, MF_LABEL_REMOVE,
		                                        MF_LABEL_DELETE_THIS_RECENT,
		                                        LABEL_CANCEL, MF_LABEL_REMOVE, mf_callback_do_delete_recent_files, ap, count);
	}
}

void mf_callback_edit_unintall_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	ap->mf_Status.more = MORE_EDIT_UNINSTALL;
	__mf_callback_edit_share_view(ap);
	MF_TRACE_END;
}

/******************************
** Prototype    : mf_callback_icu_update_cb
** Description  :
** Input        : void *data
** Output       : int
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2011/06/30
**    Author       : Samsung
**    Modification : Created function
**/
void mf_callback_icu_update_cb(app_event_info_h event_info, void *data)
{
	/*
		ug_send_event(UG_EVENT_REGION_CHANGE);
	*/
	mf_debug("region changed");
	assert(data);
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.flagIcuInit == FALSE) {
		return;
	}
	/* finalize the previous icu session */
	mf_util_icu_finalize(ap);
	/* start a new icu session*/
	mf_util_icu_init(ap);
	/* check if we should refresh the list */
	int iSortTypeValue = 0;
	mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &iSortTypeValue);
	if (iSortTypeValue != MYFILE_SORT_BY_DATE_R2O && iSortTypeValue != MYFILE_SORT_BY_DATE_O2R) {
		return ;
	}

	if (ap->mf_Status.more != MORE_DEFAULT) {
		return ;
	}


	/*get current genlist */
	Evas_Object *currgenlist = ap->mf_MainWindow.pNaviGenlist;
	if (currgenlist == NULL) {
		return ;
	}
	/*create new genlist */
	mf_view_update(ap);
	return ;
}

/******************************
** Prototype    : mf_callback_new_folder_create_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_new_folder_create_cb(void *data, Evas_Object *obj, void *event_info)
{
	mf_debug();
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	{
		int ret = 0;
		if (ap->mf_Status.folder_count >= MAX_FOLDER_COUNT) {
			const char *message = MF_MSG_FOLDER_NUM_MAX;
			ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT, NULL, message, NULL, NULL, NULL, NULL, NULL);
			return;
		}

		ret = mf_util_check_disk_space(ap);
		if (ret == MYFILE_ERR_NO_FREE_SPACE) {
			return;
		}

		ap->mf_Status.pre_create_more = ap->mf_Status.more;
		ap->mf_MainWindow.pNewFolderPopup = mf_popup_create_new_folder_popup(ap, MF_LABEL_CREATE_FOLDER_CHAP);
		elm_object_focus_set(ap->mf_MainWindow.pEntry, EINA_TRUE);
	}
	return;
}

void mf_callback_rename_create_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	mfItemData_s *params = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)params->ap;

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME) {
		return;
	}
	mf_view_state_set_with_pre(ap, MORE_THUMBNAIL_RENAME);

	mf_popup_rename_func_reset();
	mf_popup_rename_func_set(mf_callback_rename_save_cb, params, mf_callback_cancel_cb, ap);

	ap->mf_MainWindow.pNewFolderPopup = mf_popup_create_rename_popup(params, LABEL_RENAME_CHAP);
	elm_object_focus_set(ap->mf_MainWindow.pEntry, EINA_TRUE);
	MF_TRACE_END;
}

void mf_callback_max_len_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.more == MORE_SEARCH) {
		elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_TRUE);
		elm_entry_cursor_end_set(ap->mf_MainWindow.pSearchEntry);

	} else {
		elm_entry_cursor_end_set(ap->mf_MainWindow.pEntry);
		elm_object_focus_set(ap->mf_MainWindow.pEntry, EINA_TRUE);
	}
}

void mf_callback_entry_unfocus(Evas_Object *entry)
{
	if (entry) {
		elm_object_focus_set(entry, EINA_FALSE);
		elm_object_focus_allow_set(entry, EINA_FALSE);
		max_length_entry = entry;
	} else {
		max_length_entry = NULL;
	}
}

void mf_callback_entry_focus()
{
	if (max_length_entry) {
		elm_object_focus_allow_set(max_length_entry, EINA_TRUE);
		elm_entry_cursor_end_set(max_length_entry);
		evas_object_show(max_length_entry);
		elm_object_focus_set(max_length_entry, EINA_TRUE);
	}

}

void mf_callback_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	elm_entry_select_none(ap->mf_MainWindow.pEntry);
	MF_TRACE_END;
}

void mf_callback_long_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	evas_object_smart_callback_del(ap->mf_MainWindow.pEntry, "clicked", (Evas_Smart_Cb) mf_callback_clicked_cb);
//	elm_entry_select_none(ap->mf_MainWindow.pEntry);
	MF_TRACE_END;
}

void mf_callback_max_len_reached_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	char *message = g_strdup_printf(mf_util_get_text(MF_LABEL_MAX_CHARACTER_REACHED), MYFILE_FILE_NAME_CHAR_COUNT_MAX);
	mf_popup_indicator_popup(data, message);
	SAFE_FREE_CHAR(message);
	//mf_callback_entry_unfocus(obj);
	//mf_popup_indicator_popup(ap, MF_LABEL_MAX_CHARACTER_REACHED);
	//mf_popup_warning_popup_create(ap, ap->mf_MainWindow.pWindow, MF_POP_MAX_WARNING_TITEL, MF_LABEL_MAX_CHARACTER_REACHED, MF_BUTTON_LABEL_OK, mf_popup_show_vk_cb, ap);
	MF_TRACE_END;
}

/******************************
** Prototype    : mf_callback_init_operation_cancel
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_init_operation_cancel(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	if (ap->mf_FileOperation.pCancel) {
		mf_cancel_free(ap->mf_FileOperation.pCancel);
		ap->mf_FileOperation.pCancel = NULL;
	}

	ap->mf_FileOperation.pCancel = mf_cancel_new();
	MF_TRACE_END;
}

/******************************
** Prototype    : __mf_callback_idle_refresh
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static int __mf_callback_idle_refresh(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, 0, "ap is NULL");
	mf_util_refresh_screen(ap);
	ap->mf_Status.operation_refresh_idler = NULL;
	MF_TRACE_END;
	return ECORE_CALLBACK_CANCEL;
}

/******************************
** Prototype    : __mf_callback_progress_bar_state_cb
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_progress_bar_state_cb(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	mf_retm_if(ap->mf_MainWindow.pProgressPopup == NULL, "ap->mf_MainWindow.pProgressPopup is NULL");

	mf_fo_msg *msg = ap->mf_FileOperation.pMessage;
	char lable[10] = { '\0', };
	double value = 0.0;
	int count = 0;
	int total_count = 0;
	Evas_Object *layout = ap->mf_MainWindow.pProgressLayout;
	mf_retm_if(layout == NULL, "layout is NULL");

	total_count = ap->mf_FileOperation.iTotalCount;
	count = msg->current_index;
	mf_error("processing ,current_size[%d], total_size[%ld]", msg->current_size,
	         msg->total_size);
	value = (double)msg->current_size / msg->total_size;
	//add protection to avoid the percent passed 100%
	if (value > 1 && count <= total_count) {
		value = (double)count / total_count;
	}
	elm_progressbar_value_set(ap->mf_FileOperation.progress_bar, value);
	//mf_error("processing ,count=%d, total_count=%d, value is [%f]",count, total_count,value);

	snprintf(lable, sizeof(lable), "%d%s", (int)(value * 100), "%");
	/*{//Fixing the P140801-06774
		char *fileName = NULL;
		char *name = NULL;
		if (ap->mf_FileOperation.pSourceList != NULL) {
			GList* listTmp = ap->mf_FileOperation.pSourceList;
			char *listData = (char*) g_list_nth_data(listTmp, count);
			if (listData) {
				fileName = (char*)strdup(listData);
				if (fileName) {
					name = (char*)strdup(mf_file_get(fileName));
					free(fileName);
				}
			}
		}
		mf_error("name=%s", name);

		if (name) {
			edje_object_part_text_set(elm_layout_edje_get(layout), "elm.title.filename", name);
			free(name);
		}
	}
	edje_object_part_text_set(elm_layout_edje_get(layout), "elm.text.left", lable);

	if (count <= total_count) {
		snprintf(count_label, sizeof(count_label), "%d/%d", count, total_count);
	}
	edje_object_part_text_set(elm_layout_edje_get(layout), "elm.text.right", count_label);*/

	MF_TRACE_END;
}

/******************************
** Prototype    : mf_callback_progress_bar_cancel_cb
** Description  : Samsung
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_progress_bar_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	mf_cancel_do_cancel(ap->mf_FileOperation.pCancel);
	mf_msg_request_handled_send();

	MF_TRACE_END;
}

/******************************
** Prototype    : mfCopyCB
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_item_copy_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *item_data = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)item_data->ap;
	mf_retm_if(ap == NULL, "ap is NULL");

	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);

	if (ap->mf_FileRecordList.value_saver != NULL) {
		mf_util_free_eina_list_with_data(&ap->mf_FileRecordList.value_saver, MYFILE_TYPE_GSTRING);
		ap->mf_FileRecordList.value_saver = NULL;
	}
	ap->mf_FileRecordList.value_saver = eina_list_append(ap->mf_FileRecordList.value_saver, g_string_new(item_data->m_ItemName->str));

	if (ap->mf_FileOperation.source != NULL) {
		g_string_free(ap->mf_FileOperation.source, TRUE);
		ap->mf_FileOperation.source = NULL;
	}

	ap->mf_FileOperation.source = mf_fm_svc_wrapper_get_file_parent_path(item_data->m_ItemName);
	__mf_callback_save_edit_view_status(ap);
	ap->mf_Status.view_type = mf_view_storage;
	mf_view_state_set_with_pre(ap, MORE_INTERNAL_COPY);

	mf_view_update(ap);

	MF_TRACE_END;
}

void mf_callback_item_move_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *item_data = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)item_data->ap;
	mf_retm_if(ap == NULL, "ap is NULL");

	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);

	if (ap->mf_FileRecordList.value_saver != NULL) {
		mf_util_free_eina_list_with_data(&ap->mf_FileRecordList.value_saver, MYFILE_TYPE_GSTRING);
		ap->mf_FileRecordList.value_saver = NULL;
	}
	ap->mf_FileRecordList.value_saver = eina_list_append(ap->mf_FileRecordList.value_saver, g_string_new(item_data->m_ItemName->str));

	if (ap->mf_FileOperation.source != NULL) {
		g_string_free(ap->mf_FileOperation.source, TRUE);
		ap->mf_FileOperation.source = NULL;
	}

	ap->mf_FileOperation.source = mf_fm_svc_wrapper_get_file_parent_path(item_data->m_ItemName);
	__mf_callback_save_edit_view_status(ap);

	ap->mf_Status.view_type = mf_view_storage;
	mf_view_state_set_with_pre(ap, MORE_INTERNAL_MOVE);
	mf_view_update(ap);

	MF_TRACE_END;
}

void mf_callback_copy_move_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");


	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	if (ap->mf_FileRecordList.value_saver != NULL) {
		mf_util_free_eina_list_with_data(&ap->mf_FileRecordList.value_saver, MYFILE_TYPE_GSTRING);
		ap->mf_FileRecordList.value_saver = NULL;
	}
	ap->mf_FileRecordList.value_saver = mf_edit_get_all_selected_files();

	if (mf_util_get_eina_list_len(ap->mf_FileRecordList.value_saver) < 1) {
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL, MF_LABEL_NOTHING_SELECTED,
		                                 MF_BUTTON_LABEL_OK, NULL, NULL, mf_callback_warning_popup_cb, ap);
		return;
	}

	if (ap->mf_FileOperation.source != NULL) {
		g_string_free(ap->mf_FileOperation.source, TRUE);
		ap->mf_FileOperation.source = NULL;
	}
	ap->mf_FileOperation.source = g_string_new(ap->mf_Status.path->str);
	if (ap->mf_Status.more == MORE_EDIT_MOVE) {
		ap->mf_Status.more = MORE_INTERNAL_MOVE;
	} else if (ap->mf_Status.more == MORE_EDIT_COPY) {
		ap->mf_Status.more = MORE_INTERNAL_COPY;
	} else {
		return;
	}

	/*pop select view*/
	/*set Navigation Bar for Destination selection */
	ap->mf_Status.view_type = mf_view_storage;
	mf_view_update(ap);
	MF_TRACE_END;
}

void mf_callback_details_cb(void *data, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = mf_get_appdata();

	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	if (mf_edit_file_count_get() == 1) {
		mf_list_data_t *list_data = NULL;
		if (mf_edit_folder_list_get_length()) {
			list_data = (mf_list_data_t *)mf_edit_folder_list_item_get(0);
		} else if (mf_edit_file_list_get_length()) {
			list_data = (mf_list_data_t *)mf_edit_file_list_item_get(0);
		}
		if (list_data) {

			{
				mf_callback_detail_button_cb(list_data, NULL, NULL);
			}
		}
		return;
	} else {
		Eina_List *select_list = mf_edit_get_all_selected_files();
		if (mf_util_get_eina_list_len(select_list) < 1) {
			ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL, MF_LABEL_NOTHING_SELECTED,
			                                 MF_BUTTON_LABEL_OK, NULL, NULL, mf_callback_warning_popup_cb, ap);
			return;
		}

		Eina_List *l = NULL;
		GString *file_path = NULL;
		char *total_path = NULL;
		EINA_LIST_FOREACH(select_list, l, file_path) {
			if (file_path) {
				if (total_path == NULL) {
					total_path = g_strdup(file_path->str);
				} else {
					char *temp = total_path;
					total_path = g_strconcat(total_path, ";", file_path->str, NULL);
					free(temp);
					temp = NULL;
				}
			}
		}
		mf_util_free_eina_list_with_data(&select_list, MYFILE_TYPE_GSTRING);
		mf_error("path is [%s]", total_path);
		//mf_launch_load_ug(ap, total_path, MF_LOAD_UG_DETAIL, EINA_TRUE);
		__mf_load_detail_data(ap, total_path, EINA_TRUE);

		ap->mf_Status.preViewType = ap->mf_Status.view_type;
		ap->mf_Status.view_type = mf_view_detail;
		ap->mf_Status.more = MORE_DEFAULT;
		mf_view_update(ap);
	}
	return;
}

void mf_callback_edit_details_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	elm_object_part_content_unset(ap->mf_MainWindow.pNaviLayout, "search_icon");
	evas_object_hide(ap->mf_MainWindow.pButton);
	if (ap->mf_Status.more == MORE_SEARCH) {
		if (ap->mf_MainWindow.pNaviSearchBar) {
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "hide", "search_bar");
			elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "show", "elm.swallow.content");
			//ap->mf_Status.extra = MORE_EDIT_DETAIL;
		}
	} else {
		ap->mf_Status.extra = MORE_EDIT_DETAIL;
	}

	ap->mf_Status.more = MORE_EDIT_DETAIL;
	__mf_callback_edit_share_view(ap);

	/*disable all the tab item if tab exists */
	MF_TRACE_END;
}

void mf_callback_show_hidden_items_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	int hiden_state;
	mf_util_get_pref_value(PREF_TYPE_HIDEN_STATE, &hiden_state);
	if (hiden_state == MF_HIDEN_SHOW) {
		mf_util_set_hiden_state(MF_HIDEN_HIDE);
	} else {
		mf_util_set_hiden_state(MF_HIDEN_SHOW);
	}
	mf_view_refresh(ap);
}

/******************************
** Prototype    : mf_callback_paste_here_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_paste_here_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	int source_len = 0;
	int dest_len = 0;
	GList *pSourceList = NULL;
	int count = 0;
	gchar *pDestPath = NULL;

	mf_retm_if(ap->mf_FileOperation.source == NULL, "ap->mf_FileOperation.source is NULL");
	mf_retm_if(ap->mf_FileOperation.source->str == NULL, "ap->mf_FileOperation.source is NULL");

	mf_retm_if(ap->mf_Status.path == NULL, "ap->mf_Status.path is NULL");
	mf_retm_if(ap->mf_Status.path->str == NULL, "ap->mf_Status.path->str is NULL");

	ap->mf_FileOperation.destination = ap->mf_Status.path;

	if (ap->mf_FileOperation.source && ap->mf_FileOperation.source->str) {
		source_len = strlen(ap->mf_FileOperation.source->str);
	}

	if (ap->mf_FileOperation.destination && ap->mf_FileOperation.destination->str) {
		dest_len = strlen(ap->mf_FileOperation.destination->str);
	}

	if (source_len == 0 || dest_len == 0) {
		mf_callback_cancel_cb(ap, NULL, NULL);
		MF_TRACE_END;
		return;
	}
	/*glib api change */

	if (!mf_util_check_forbidden_operation(ap)) {
		mf_callback_cancel_cb(ap, NULL, NULL);
		return;
	}
	if (source_len == dest_len && strcmp(ap->mf_FileOperation.destination->str, ap->mf_FileOperation.source->str) == 0) {
		ap->mf_FileOperation.refresh_type = TRUE;
	} else {
		ap->mf_FileOperation.refresh_type = FALSE;
	}

	mf_util_merge_eina_list_to_glist(ap->mf_FileRecordList.value_saver, &pSourceList);

	if (pSourceList) {
		count = g_list_length(pSourceList);
	} else {
		mf_popup_indicator_popup(ap, mf_util_get_text(MF_LABEL_FILE_NOT_EXIST));
		mf_callback_cancel_cb(ap, NULL, NULL);
		MF_TRACE_END;
		return;
	}

	ap->mf_FileOperation.iTotalCount = count;
	pDestPath = g_strdup(ap->mf_Status.path->str);
	if (pDestPath == NULL) {
		g_list_free(pSourceList);
		pSourceList = NULL;
		mf_callback_cancel_cb(ap, NULL, NULL);
		MF_TRACE_END;
		return;
	}
	ap->mf_Status.more = MORE_DATA_COPYING;

	mf_callback_init_operation_cancel(ap);
	ap->mf_FileOperation.iRequestType = MF_REQ_NONE;

	ap->mf_FileOperation.pSourceList = pSourceList;

	int ret = mf_copy_copy_items(pSourceList, pDestPath, ap->mf_FileOperation.pCancel, TRUE, ap);
	if (ret == 0) {
		ap->mf_FileOperation.iOperationSuccessFlag = TRUE;
		mf_fs_monitor_remove_dir_watch();
		ap->mf_MainWindow.pProgressPopup = mf_popup_create_pb_popup(ap, LABEL_COPY, MF_MSG_COPYING, count, mf_callback_progress_bar_cancel_cb, ap);
	} else if (ret == MF_FO_ERR_ARGUMENT) {
		mf_popup_indicator_popup(ap, mf_util_get_text(MF_LABEL_FILE_NOT_EXIST));
		mf_callback_cancel_cb(ap, NULL, NULL);
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}

	g_free(pDestPath);
	pDestPath = NULL;

	MF_TRACE_END;
}

/******************************
** Prototype    : mf_move_here_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_move_here_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	int source_len = 0;
	int dest_len = 0;
	GList *pSourceList = NULL;
	int count = 0;
	gchar *pDestPath = NULL;

	mf_retm_if(ap->mf_FileOperation.source == NULL, "ap->mf_FileOperation.source is NULL");
	mf_retm_if(ap->mf_FileOperation.source->str == NULL, "ap->mf_FileOperation.source is NULL");

	mf_retm_if(ap->mf_Status.path == NULL, "ap->mf_Status.path is NULL");
	mf_retm_if(ap->mf_Status.path->str == NULL, "ap->mf_Status.path->str is NULL");

	ap->mf_FileOperation.destination = ap->mf_Status.path;

	if (ap->mf_FileOperation.source && ap->mf_FileOperation.source->str) {
		source_len = strlen(ap->mf_FileOperation.source->str);
	}

	if (ap->mf_FileOperation.destination && ap->mf_FileOperation.destination->str) {
		dest_len = strlen(ap->mf_FileOperation.destination->str);
	}

	if (source_len == 0 || dest_len == 0) {
		mf_callback_cancel_cb(ap, NULL, NULL);
		MF_TRACE_END;
		return;
	}
	/*glib api change */
	if (!mf_util_check_forbidden_operation(ap)) {
		mf_callback_cancel_cb(ap, NULL, NULL);
		return;
	} else {
		ap->mf_FileOperation.refresh_type = FALSE;
	}

	mf_util_merge_eina_list_to_glist(ap->mf_FileRecordList.value_saver, &pSourceList);

	if (pSourceList) {
		count = g_list_length(pSourceList);
	} else {
		mf_popup_indicator_popup(ap, mf_util_get_text(MF_LABEL_FILE_NOT_EXIST));
		mf_callback_cancel_cb(ap, NULL, NULL);
		MF_TRACE_END;
		return;
	}

	ap->mf_FileOperation.iTotalCount = count;
	pDestPath = g_strdup(ap->mf_Status.path->str);
	if (pDestPath == NULL) {
		g_list_free(pSourceList);
		pSourceList = NULL;
		mf_callback_cancel_cb(ap, NULL, NULL);
		MF_TRACE_END;
		return;
	}
	ap->mf_Status.more = MORE_DATA_MOVING;

	mf_callback_init_operation_cancel(ap);
	ap->mf_FileOperation.iRequestType = MF_REQ_NONE;

	ap->mf_FileOperation.pSourceList = pSourceList;
	if (mf_move_move_items(pSourceList, pDestPath, ap->mf_FileOperation.pCancel, TRUE, ap) == 0) {
		ap->mf_FileOperation.iOperationSuccessFlag = TRUE;
		mf_fs_monitor_remove_dir_watch();
		ap->mf_MainWindow.pProgressPopup = mf_popup_create_pb_popup(ap, LABEL_MOVE, MF_MSG_MOVING, count, mf_callback_progress_bar_cancel_cb, ap);
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}
	g_free(pDestPath);
	pDestPath = NULL;
}

/******************************
** Prototype    : __mf_callback_confirm_delete
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_callback_confirm_delete(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);

	if (g_strcmp0(label, mf_util_get_text(LABEL_DELETE)) == 0) {
		if (ap->mf_MainWindow.pDeleteConfirmPopup) {
			evas_object_del(ap->mf_MainWindow.pDeleteConfirmPopup);
			ap->mf_MainWindow.pDeleteConfirmPopup = NULL;
		}

		ap->mf_FileOperation.refresh_type = TRUE;

		/*setting item check */
		Eina_List *selected_list = mf_edit_get_all_selected_files();

		if (ap->mf_FileRecordList.value_saver != NULL) {
			mf_util_free_eina_list_with_data(&ap->mf_FileRecordList.value_saver, MYFILE_TYPE_GSTRING);
			ap->mf_FileRecordList.value_saver = NULL;
		}
		ap->mf_FileRecordList.value_saver = selected_list;

		if (mf_util_get_eina_list_len(ap->mf_FileRecordList.value_saver) < 1) {
			ap->mf_MainWindow.pNormalPopup =
			    mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL, MF_LABEL_NOTHING_SELECTED,
			                          MF_BUTTON_LABEL_OK, NULL, NULL, mf_callback_warning_popup_cb, ap);
		} else {
			GList *pSourceList = NULL;
			int count = 0;

			mf_util_merge_eina_list_to_glist(ap->mf_FileRecordList.value_saver, &pSourceList);
			if (pSourceList) {
				count = g_list_length(pSourceList);
			} else {
				MF_TRACE_END;
				return;
			}

			ap->mf_FileOperation.iTotalCount = count;
			mf_callback_init_operation_cancel(ap);
			mf_view_state_set_with_pre(ap, MORE_DELETE);
			ap->mf_FileOperation.iRequestType = MF_REQ_NONE;

			ap->mf_FileOperation.pSourceList = pSourceList;
			mf_fs_monitor_remove_dir_watch();

			if (mf_delete_items(pSourceList, ap->mf_FileOperation.pCancel, TRUE, ap) == 0) {
				ap->mf_FileOperation.iOperationSuccessFlag = TRUE;
				mf_fs_monitor_remove_dir_watch();
				ap->mf_MainWindow.pProgressPopup =
				    mf_popup_create_pb_popup(ap, LABEL_DELETE, MF_MSG_DELETING, count, mf_callback_progress_bar_cancel_cb, ap);
			} else {
				mf_util_exception_func(ap);
			}
		}
	} else {
		if (ap->mf_Status.extra == MORE_SEARCH) {
			mf_edit_file_list_clear();
		}
		evas_object_del(ap->mf_MainWindow.pDeleteConfirmPopup);
		ap->mf_MainWindow.pDeleteConfirmPopup = NULL;
		return;
	}
}

/******************************
** Prototype    : mf_callback_delete_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	mf_debug("delete clicked\n");
	int count = mf_edit_file_count_get();
	if (count > 1) {
		ap->mf_MainWindow.pDeleteConfirmPopup = mf_popup_create_delete_confirm_popup(ap, LABEL_DELETE,
		                                        MF_LABEL_ITEMS_WILL_DELETE,
		                                        LABEL_CANCEL, LABEL_DELETE, __mf_callback_confirm_delete, ap, count);
	} else {
		ap->mf_MainWindow.pDeleteConfirmPopup = mf_popup_create_delete_confirm_popup(ap,
		                                        LABEL_DELETE,
		                                        MF_LABEL_THIS_ITEM_WILL_DELETE,
		                                        LABEL_CANCEL, LABEL_DELETE, __mf_callback_confirm_delete, ap, count);
	}
}

/******************************
** Prototype    : mf_callback_thread_pipe_cb
** Description  :
** Input        : void *data
**                void *buffer
**                unsigned int nbyte
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
void mf_callback_operation_request_rename_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	mf_fo_request *request = ap->mf_FileOperation.pRequest;

	if (ap->mf_MainWindow.pProgressPopup) {
		evas_object_show(ap->mf_MainWindow.pProgressPopup);
	}

	ap->mf_FileOperation.iRequestType = MF_REQ_RENAME;
	mf_request_set_result(request, MF_REQ_RENAME);
	MF_TRACE_END;
}

void mf_callback_operation_request_replace_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	mf_fo_request *request = ap->mf_FileOperation.pRequest;

	if (ap->mf_MainWindow.pProgressPopup) {
		evas_object_show(ap->mf_MainWindow.pProgressPopup);
	}

	ap->mf_FileOperation.iRequestType = MF_REQ_MERGE;
	if (ap->mf_FileOperation.refresh_type == TRUE) {
		mf_request_set_result(request, MF_REQ_SKIP);
	} else {
		mf_request_set_result(request, MF_REQ_MERGE);
	}
	MF_TRACE_END;
}

void mf_callback_operation_request_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	mf_fo_request *request = ap->mf_FileOperation.pRequest;

	if (ap->mf_MainWindow.pProgressPopup) {
		evas_object_show(ap->mf_MainWindow.pProgressPopup);
	}

	mf_request_set_result(request, MF_REQ_CANCEL);
	MF_TRACE_END;
}

void mf_callback_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
	mf_request_set_result(ap->mf_FileOperation.pRequest, MF_REQ_CANCEL);
}

void mf_callback_rename_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	ap->mf_Status.req = MF_REQ_RENAME;
	mf_request_set_result(ap->mf_FileOperation.pRequest, MF_REQ_RENAME);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
}

void mf_callback_replace_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	ap->mf_Status.req = MF_REQ_NONE;
	mf_request_set_result(ap->mf_FileOperation.pRequest, MF_REQ_NONE);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
}

void mf_callback_thread_pipe_cb(void *data, void *buffer, unsigned int nbyte)
{
	MF_TRACE_BEGIN;
	mf_debug(":::::::::: Main thread id = %d ::::::::::", (int)pthread_self());
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	mf_fo_msg *pPipeMsg = (mf_fo_msg *) buffer;
	char *message = NULL;
	int error_type = 0;

	if (!MYFILE_MAGIC_CHECK(pPipeMsg, MYFILE_MAGIC_PIPE_DATA)) {
		mf_error(":::::::::::::p_pipe_data ERROR:::::::::::::");
		return;
	}
	ap->mf_FileOperation.pMessage = pPipeMsg;
	ap->mf_FileOperation.pRequest = pPipeMsg->request;
	mf_debug("::::::::::: pPipeMsg->msg_type is [%d]", pPipeMsg->msg_type);

	if (pPipeMsg->msg_type == MF_MSG_REQUEST && ap->mf_Status.check != 1) {
		GString *path = g_string_new(pPipeMsg->current);
		if (mf_fm_svc_wrapper_is_dir(path) == FILE_TYPE_DIR) {
			char *label = gettext(MF_LABEL_SAME_FOLDER_LABEL);
			ap->file_name = mf_fm_svc_get_file_name(path);
			char *message = g_strdup_printf(label, ap->file_name);
			ap->mf_MainWindow.pNormalPopup = mf_popup_replace_create(MF_LABEL_SAME_FOLDER, message, mf_callback_popup_cancel_cb,
			                                 mf_callback_replace_cb, mf_callback_rename_cb, ap);
		} else {
			char *label = gettext(MF_LABEL_SAME_FILE_LABEL);
			ap->file_name = mf_fm_svc_get_file_name(path);
			char *message = g_strdup_printf(label, ap->file_name);
			ap->mf_MainWindow.pNormalPopup = mf_popup_replace_create(MF_LABEL_SAME_FILE, message, mf_callback_popup_cancel_cb,
			                                 mf_callback_replace_cb, mf_callback_rename_cb, ap);
		}
	}
	mf_util_set_pm_lock(ap, EINA_TRUE);
	switch (pPipeMsg->msg_type) {
	case MF_MSG_NONE:
	case MF_MSG_SKIP:
	case MF_MSG_SYNC:
		mf_msg_request_handled_send();
		break;
	case MF_MSG_CANCELLED:
		if (ap->mf_MainWindow.pProgressPopup) {
			evas_object_del(ap->mf_MainWindow.pProgressPopup);
			ap->mf_MainWindow.pProgressPopup = NULL;
		}
		ap->mf_MainWindow.pFinishPopup = mf_popup_create_popup(ap, POPMODE_TEXT_NOT_DISABLED, NULL,
		                                 MF_LABEL_CANCELING, NULL, NULL, NULL, NULL, NULL);
		mf_msg_request_handled_send();
		ap->mf_FileOperation.iOperationSuccessFlag = FALSE;
		break;
	case MF_MSG_DOING:
		mf_callback_progress_bar_state_cb(ap);
		mf_msg_request_handled_send();
		break;
	case MF_MSG_REQUEST:
		if (ap->mf_FileOperation.iRequestType == MF_REQ_RENAME) {
			if (ap->mf_Status.check == 1 && ap->mf_Status.req == MF_REQ_RENAME) {
				mf_request_set_result(pPipeMsg->request, MF_REQ_RENAME);
			}
			if (ap->mf_Status.check == 1 && ap->mf_Status.req == MF_REQ_NONE) {
				mf_request_set_result(pPipeMsg->request, MF_REQ_NONE);
			}
		} else {
			ap->mf_FileOperation.iRequestType = MF_REQ_RENAME;
			if (ap->mf_Status.check == 1 && ap->mf_Status.req == MF_REQ_RENAME) {
				mf_request_set_result(pPipeMsg->request, MF_REQ_RENAME);
			}
			if (ap->mf_Status.check == 1 && ap->mf_Status.req == MF_REQ_NONE) {
				mf_request_set_result(pPipeMsg->request, MF_REQ_NONE);
			}
		}
		break;
	case MF_MSG_ERROR:
		ap->mf_FileOperation.message_type = message_type_notification;
		error_type = FO_ERROR_CHECK(pPipeMsg->error_code);
		mf_error("===================================error is [%d]", error_type);
		switch (error_type) {
		case MF_FO_ERR_PERMISSION:
			message = MF_MSG_PERMISSION_ERR;
			break;
		case MF_FO_ERR_ARGUMENT:
			message = MF_MSG_ARG_ERR;
			break;
		case MF_FO_ERR_FAULT:
			message = MF_MSG_FAULT_ERR;
			break;
		case MF_FO_ERR_TYPE:
			message = MF_MSG_FILE_TYPE_ERR;
			break;
		case MF_FO_ERR_MAX_OPEN:
			message = MF_MSG_MAX_OPEN_ERR;
			break;
		case MF_FO_ERR_RO:
			message = MF_MSG_RO_ERR;
			break;
		case MF_FO_ERR_LOOP:
			message = MF_MSG_LOOP_ERR;
			break;
		case MF_FO_ERR_MEM:
		case MF_FO_ERR_SPACE:
			message = MF_LABE_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS_AND_TRY_AGAIN;
			break;
		case MF_FO_ERR_NOT_EXIST:
			if (ap->mf_Status.more == MORE_DATA_COPYING) {
				message = MF_LABEL_UNALBE_COPY;
			} else if (ap->mf_Status.more == MORE_DATA_MOVING) {
				message = MF_LABEL_UNALBE_MOVE;
			} else {
				message = MF_LABEL_FILE_NOT_EXIST;
			}
			break;
		case MF_FO_ERR_LONG_NAME:
			message = MF_MSG_LONG_NAME_ERR;
			break;
		case MF_FO_ERR_BIG_SIZE:
			message = MF_MSG_BIG_SIZE_ERR;
			break;
		case MF_FO_ERR_UNKNOWN:
			message = MF_MSG_UNKNOWN_ERR;
			break;
		case MF_FO_ERR_IO:
			message = MF_MSG_IO_ERR;
			break;
		default:
			message = MF_MSG_DEFAULT_ERR;
			break;
		}

		if (ap->mf_MainWindow.pProgressPopup) {
			evas_object_del(ap->mf_MainWindow.pProgressPopup);
			ap->mf_MainWindow.pProgressPopup = NULL;
		}

		if (ap->mf_MainWindow.pFinishPopup) {
			evas_object_del(ap->mf_MainWindow.pFinishPopup);
			ap->mf_MainWindow.pFinishPopup = NULL;
		}

		ap->mf_FileOperation.pOperationMsg = NULL;
		if (error_type == MF_FO_ERR_SPACE || error_type == MF_FO_ERR_MEM) {
			ap->mf_FileOperation.message_type = message_type_popup;
			ap->mf_FileOperation.pOperationMsg = message;
		} else if (error_type == MF_FO_ERR_NOT_EXIST) {
			ap->mf_FileOperation.message_type = message_type_notification;
			ap->mf_FileOperation.pOperationMsg = message;
		} else if (error_type == MF_FO_ERR_IO || error_type == MF_FO_ERR_UNKNOWN) {
			int operation_more = MORE_DEFAULT;
			if (mf_callback_storage_remove_flag_get(&operation_more)) {
				mf_callback_storage_remove_flag_set(EINA_FALSE, MORE_DEFAULT);
			}
			if (operation_more == MORE_DEFAULT) {
				operation_more = ap->mf_Status.more;
			}
			if (operation_more == MORE_DATA_COPYING) {
				ap->mf_FileOperation.pOperationMsg = MF_LABEL_UNALBE_COPY;
			} else if (operation_more == MORE_DATA_MOVING) {
				ap->mf_FileOperation.pOperationMsg = MF_LABEL_UNALBE_MOVE;
			} else if (operation_more == MORE_DELETE || operation_more == MORE_IDLE_DELETE) {
				ap->mf_FileOperation.pOperationMsg = MF_LABEL_DELETE_FAILED;
			} else {
				ap->mf_FileOperation.pOperationMsg = message;
			}
		} else {
			if (ap->mf_Status.more == MORE_DATA_COPYING) {
				ap->mf_FileOperation.pOperationMsg = MF_MSG_COPY_FAILED;
			} else if (ap->mf_Status.more == MORE_DATA_MOVING) {
				ap->mf_FileOperation.pOperationMsg = MF_MSG_MOVE_FAILED3;
			} else if (ap->mf_Status.more == MORE_DELETE || ap->mf_Status.more == MORE_IDLE_DELETE) {
				ap->mf_FileOperation.pOperationMsg = MF_LABEL_DELETE_FAILED;
			} else {
				ap->mf_FileOperation.pOperationMsg = message;
			}
		}

		ap->mf_FileOperation.iOperationSuccessFlag = FALSE;
		mf_msg_request_handled_send();
		if (error_type == MF_FO_ERR_SPACE || error_type == MF_FO_ERR_MEM) {
			ap->mf_FileOperation.message_type = message_type_popup;
			ap->mf_FileOperation.pOperationMsg = NULL;
			ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_TWO_BTN, MF_LABEL_UNABLE_TO_SAVE_DATA, MF_LABEL_NO_SPACE_TO_SETTING, LABEL_CANCEL, MF_LABEL_SETTINGS, NULL, mf_callback_setting_popup_cb, ap);
		}
		break;
	case MF_MSG_END:
		ap->mf_Status.check = 0;
		if (ap->mf_FileOperation.source) {
			g_string_free(ap->mf_FileOperation.source, TRUE);
			ap->mf_FileOperation.source = NULL;
		}
		if (ap->mf_FileOperation.pSourceList) {
			g_list_free(ap->mf_FileOperation.pSourceList);
			ap->mf_FileOperation.pSourceList = NULL;
		}
		if (ap->mf_MainWindow.pProgressPopup) {
			evas_object_del(ap->mf_MainWindow.pProgressPopup);
			ap->mf_MainWindow.pProgressPopup = NULL;
		}
		mf_ecore_idler_del(ap->mf_Status.operation_refresh_idler);
		ap->mf_Status.operation_refresh_idler = ecore_idler_add((Ecore_Task_Cb)__mf_callback_idle_refresh, ap);
		if (pPipeMsg->pipe) {
			ecore_pipe_del(pPipeMsg->pipe);
			pPipeMsg->pipe = NULL;
		}
		mf_msg_request_handled_send();

		mf_util_set_pm_lock(ap, EINA_FALSE);
		if (ap->mf_FileRecordList.value_saver != NULL) {
			mf_util_free_eina_list_with_data(&ap->mf_FileRecordList.value_saver, MYFILE_TYPE_GSTRING);
			ap->mf_FileRecordList.value_saver = NULL;
		}
		break;
	default:
		mf_msg_request_handled_send();
		break;
	}

}

void mf_callback_exception_popup_cb(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	if (ap->mf_MainWindow.pNormalPopup) {
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
		ap->mf_MainWindow.pNormalPopup = NULL;
	}

	mf_callback_cancel_cb(ap, NULL, NULL);
}

/******		Memory card connection/removal handler		******/
static void __mf_callback_mmc_connected(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "appdata is NULL");
	mf_retm_if(ap->mf_Status.path == NULL || ap->mf_Status.path->str == NULL, "mf_Status.path is NULL");

	Evas_Object *entry = NULL;
	ap->mf_Status.iStorageState |= MYFILE_MMC;

	if (ap->mf_Status.more == MORE_SEARCH || (ap->mf_Status.more == MORE_RENAME && mf_view_get_pre_state(ap) == MORE_SEARCH)) {
		return;
	}

	if (ap->mf_Status.view_type != mf_view_storage && ap->mf_Status.view_type != mf_view_root) {
		return;
	}

	mf_util_action_storage_insert(ap, mf_util_get_text(MF_LABEL_MMC));
	if (ap->mf_Status.more == MORE_RENAME) {
		entry = ap->mf_MainWindow.pEntry;
		if (entry != NULL) {
			elm_object_focus_set(entry, EINA_TRUE);
		}
	}
	//mf_navi_bar_title_set(ap);
}

void mf_callback_storage_remove_view_operation(void *data, int optStorage)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "appdata is NULL");

	Evas_Object *parent = NULL;
	int view_style = mf_view_style_get(ap);
	if (view_style == MF_VIEW_STYLE_LIST || view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
		parent = ap->mf_MainWindow.pNaviGenlist;
	} else {
		parent = ap->mf_MainWindow.pNaviGengrid;
	}

	mf_view_item_remove_by_type(parent, optStorage, view_style);
}

void mf_callback_storage_remove_category_view_items(void *data, int optStorage)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "appdata is NULL");

	Evas_Object *parent = NULL;
	int view_style = mf_view_style_get(ap);
	if (view_style == MF_VIEW_STYLE_LIST || view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
		parent = ap->mf_MainWindow.pNaviGenlist;
	} else {
		parent = ap->mf_MainWindow.pNaviGengrid;
	}
	mf_view_items_remove(parent, optStorage, view_style);
	mf_util_remove_item_from_list_by_location(&ap->mf_FileOperation.category_list, optStorage);
	int count = elm_genlist_items_count(ap->mf_MainWindow.pNaviGenlist);
	mf_error("================== count is [%d]", count);
	if (count == 0) {
		ap->mf_Status.flagNoContent = EINA_TRUE;
		SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviGenlist);
		if (ap->mf_MainWindow.pNaviBox) {
			Evas_Object *pContent = mf_object_create_no_content(ap->mf_MainWindow.pNaviBar);
			mf_object_text_set(pContent, MF_LABEL_NO_FILES, "elm.text");
			elm_box_clear(ap->mf_MainWindow.pNaviBox);
			elm_box_pack_end(ap->mf_MainWindow.pNaviBox, pContent);
		}
		/*Todo: we need to free all the Eina_List*/
		MF_TRACE_END;
	} else {
		mf_edit_list_item_reset(ap);
	}
}

static void __mf_callback_mmc_removed(void *data, MF_STORAGE storage)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "appdata is NULL");
	mf_retm_if(ap->mf_Status.path == NULL || ap->mf_Status.path->str == NULL, "mf_Status.path is NULL");

	Evas_Object *entry = NULL;
	MF_STORAGE optStorage = MYFILE_NONE;
	char *storage_loc = STORAGE_PARENT;
	optStorage = storage;
	switch (optStorage) {
	case MYFILE_PHONE:
		mf_util_get_text(MF_LABEL_DEVICE_MEMORY);
		break;
	case MYFILE_MMC:
		mf_util_get_text(MF_LABEL_SD_CARD);
		break;
	default:
		break;
	}

	if (ap->mf_SharedGadget.ug && ap->mf_SharedGadget.location == MYFILE_MMC) {
		mf_error("================== destory detail ug");
		//ug_destroy(ap->mf_SharedGadget.ug);
		app_control_destroy(ap->mf_SharedGadget.ug);
		ap->mf_SharedGadget.ug = NULL;
	}
	mf_callback_storage_remove_flag_set(EINA_TRUE, ap->mf_Status.more);

	/*1.2.1 check if mmc is editstar navi */
	mf_debug("before remove is [%d]", ap->mf_Status.iStorageState);
	ap->mf_Status.iStorageState = (ap->mf_Status.iStorageState ^ optStorage);
	mf_debug("after remove is [%d]", ap->mf_Status.iStorageState);

	/*3 factors; */
	/*
	 **     1.      status                          ap->mf_Status.more
	 **     2.      is storage in Use       ap->mf_Status.path
	 **     3.      is source path in storage in inter-storage operation    flagEditStartView
	 */
	mf_media_delete_recent_files_by_type(ap->mf_MainWindow.mfd_handle, MYFILE_MMC);
	if (ap->mf_Status.view_type == mf_view_recent) {
		mf_recent_view_content_refresh(ap);
	}
	if (ap->mf_MainWindow.pNewFolderPopup) {
		mf_list_data_t *item_data = (mf_list_data_t *)evas_object_data_get(ap->mf_MainWindow.pNewFolderPopup, "item_data");
		if (item_data && item_data->storage_type == MYFILE_MMC) {
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
		}
	}
	if (mf_fm_svc_wrapper_get_location(ap->mf_Status.path->str) == MYFILE_MMC) {
		if (ap->mf_MainWindow.pLongpressPopup) {
			SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
		}
		if (ap->mf_MainWindow.pNormalPopup) {
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
		}
	}

	if (ap->mf_Status.view_type == mf_view_root) {
		switch (ap->mf_Status.more) {
		case MORE_DEFAULT://go through
			mf_callback_storage_remove_view_operation(ap, optStorage);
			break;
		case MORE_RENAME:
			if (ap->mf_FileOperation.rename_item) {
				mfItemData_s *item_data = (mfItemData_s *)elm_object_item_data_get(ap->mf_FileOperation.rename_item);
				if (item_data->storage_type == MYFILE_PHONE) {
					mf_callback_storage_remove_view_operation(ap, optStorage);
				} else if (item_data->storage_type == MYFILE_MMC) {
					mf_callback_storage_remove_view_operation(ap, optStorage);
				}
			}
			break;
		case MORE_INTERNAL_COPY_MOVE:
		case MORE_INTERNAL_COPY:
		case MORE_INTERNAL_MOVE:
			if (ap->mf_MainWindow.record.location == optStorage) {
				mf_navi_bar_reset(ap);
				mf_view_update(ap);
			} else {
				mf_callback_storage_remove_view_operation(ap, optStorage);
			}
			break;
		case MORE_SEARCH:
			mf_callback_storage_remove_view_operation(ap, optStorage);
			break;
		default:
			break;
		}
	} else if (ap->mf_Status.view_type == mf_view_root_category) {
		switch (ap->mf_Status.more) {
		case MORE_DEFAULT:
		case MORE_EDIT:
		case MORE_SHARE_EDIT:
		case MORE_RENAME:
		case MORE_EDIT_COPY:
		case MORE_EDIT_MOVE:
		case MORE_EDIT_DELETE:
		case MORE_EDIT_DETAIL:
		case MORE_EDIT_RENAME:
		case MORE_THUMBNAIL_RENAME:
			mf_callback_monitor_media_db_update_flag_set(EINA_TRUE);
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
			mf_callback_storage_remove_category_view_items(ap, optStorage);
			if (ap->mf_Status.flagNoContent) {
				ap->mf_Status.more = MORE_DEFAULT;
				mf_naviframe_title_button_delete(ap->mf_MainWindow.pNaviItem);
				mf_navi_bar_title_content_set(ap, ap->mf_Status.categorytitle);
				elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_FALSE);
			}
			break;
		case MORE_SEARCH:
			mf_callback_storage_remove_view_operation(ap, optStorage);
			break;
		default:
			break;
		}
	} else if (mf_fm_svc_wrapper_get_location(ap->mf_Status.path->str) != optStorage) {
		switch (ap->mf_Status.more) {
		case MORE_DEFAULT:
		case MORE_EDIT:
		case MORE_SHARE_EDIT:
		case MORE_IDLE_DELETE:
		case MORE_DELETE:
		case MORE_RENAME:
		case MORE_EDIT_COPY:
		case MORE_EDIT_MOVE:
		case MORE_EDIT_DELETE:
		case MORE_EDIT_DETAIL:
		case MORE_EDIT_RENAME:
			/*      not a inter-storage operation */
			if (ap->mf_Status.more == MORE_RENAME) {
				entry = ap->mf_MainWindow.pEntry;
				if (entry != NULL) {
					elm_object_focus_set(entry, EINA_TRUE);
				}
			}
			break;
		case MORE_SEARCH:
			mf_callback_storage_remove_view_operation(ap, optStorage);
			break;
		case MORE_THUMBNAIL_RENAME:
			mf_debug("=====================  MMC  remove ==================");
			break;
		case MORE_INTERNAL_COPY:
		case MORE_INTERNAL_MOVE:
		case MORE_INTERNAL_COPY_MOVE:
			mf_debug("=====================  MMC  remove ==================");
			if (ap->mf_MainWindow.record.location == optStorage) {
				mf_navi_bar_reset(ap);
				mf_view_update(ap);
			}

			break;

		case MORE_DATA_COPYING:
		case MORE_DATA_MOVING:
			mf_debug("=====================  MMC  remove ==================");
			if (strncmp(ap->mf_MainWindow.record.path , storage_loc, strlen(storage_loc)) == 0) {
				if (ap->mf_MainWindow.pNormalPopup) {
					evas_object_del(ap->mf_MainWindow.pNormalPopup);
					ap->mf_MainWindow.pNormalPopup = NULL;
				}
				mf_request_set_result(ap->mf_FileOperation.pRequest, MF_REQ_SKIP);
			}
			if (ap->mf_MainWindow.record.location == optStorage) {
				ap->mf_MainWindow.pMmcRemovedPopup = mf_popup_create_popup(ap, POPMODE_TEXT_NOT_DISABLED, NULL,
				                                     MF_LABEL_MMC_REMOVED, NULL, NULL, NULL, NULL, NULL);
			}

			break;
		default:
			break;
		}
	} else {
		switch (ap->mf_Status.more) {
		case MORE_DEFAULT:
			mf_debug("=====================  MMC  remove ==================");
			mf_navi_bar_reset(ap);
			mf_view_update(ap);
			break;
		case MORE_SEARCH:
		case MORE_THUMBNAIL_RENAME:
			mf_debug("=====================  MMC  remove ==================");
			if (ap->mf_FileOperation.search_IME_hide_timer != NULL) {
				ecore_timer_del(ap->mf_FileOperation.search_IME_hide_timer);
				ap->mf_FileOperation.search_IME_hide_timer = NULL;
			}
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
			mf_navi_bar_reset(ap);
			mf_view_update(ap);

			break;
		case MORE_EDIT:
		case MORE_SHARE_EDIT:
		case MORE_EDIT_COPY:
		case MORE_EDIT_MOVE:
		case MORE_EDIT_DELETE:
		case MORE_EDIT_DETAIL:
		case MORE_EDIT_RENAME:
			mf_debug("=====================  MMC  remove ==================");
			mf_navi_bar_reset(ap);
			mf_view_update(ap);
			break;
		case MORE_IDLE_DELETE:
			mf_debug("=====================  MMC  remove ==================");

			ap->mf_MainWindow.pMmcRemovedPopup = mf_popup_create_popup(ap, POPMODE_TEXT_NOT_DISABLED, NULL,
			                                     MF_LABEL_MMC_REMOVED, NULL, NULL, NULL, NULL, NULL);
			/*ToDo: check whether get the error before this; */
			if (ap->mf_FileOperation.pCancel) {
				mf_cancel_do_cancel(ap->mf_FileOperation.pCancel);
			}

			mf_navi_bar_reset(ap);
			mf_view_update(ap);
			break;
		case MORE_RENAME:
			mf_debug("=====================  MMC  remove ==================");
			mf_navi_bar_reset(ap);
			mf_view_update(ap);

			break;
		case MORE_INTERNAL_COPY:
		case MORE_INTERNAL_MOVE:
		case MORE_INTERNAL_COPY_MOVE:
			mf_debug("=====================  MMC  remove ==================");
			if (ap->mf_MainWindow.record.location == optStorage) {
				mf_debug();
				mf_navi_bar_reset(ap);
			} else {
				mf_debug();
				ap->mf_Status.view_type = mf_view_root;
				SAFE_FREE_GSTRING(ap->mf_Status.path);
				ap->mf_Status.path = g_string_new(PHONE_FOLDER);
				ap->mf_MainWindow.location = MYFILE_PHONE;
				mf_navi_bar_reset(ap);
			}
			mf_view_update(ap);

			break;

		case MORE_DATA_COPYING:
		case MORE_DATA_MOVING:
			mf_debug("=====================  MMC  remove ==================");
			if (ap->mf_FileOperation.pCancel) {
				mf_cancel_do_cancel(ap->mf_FileOperation.pCancel);
			}
			if (ap->mf_MainWindow.pProgressPopup) {
				evas_object_del(ap->mf_MainWindow.pProgressPopup);
				ap->mf_MainWindow.pProgressPopup = NULL;
			}
			ap->mf_MainWindow.pMmcRemovedPopup = mf_popup_create_popup(ap, POPMODE_TEXT_NOT_DISABLED, NULL,
			                                     MF_LABEL_MMC_REMOVED, NULL, NULL, NULL, NULL, NULL);
			mf_navi_bar_reset(ap);
			mf_view_update(ap);

			/*check navigation bar count to decide the main layout content*/
			break;
		case MORE_DELETE:
			mf_debug("=====================  MMC  remove ==================");
			ap->mf_MainWindow.pMmcRemovedPopup = mf_popup_create_popup(ap, POPMODE_TEXT_NOT_DISABLED, NULL,
			                                     MF_LABEL_MMC_REMOVED, NULL, NULL, NULL, NULL, NULL);
			if (ap->mf_FileOperation.pCancel) {
				mf_cancel_do_cancel(ap->mf_FileOperation.pCancel);
			}
			mf_navi_bar_reset(ap);

			mf_view_update(ap);
			break;
		default:
			break;
		}
		if (ap->mf_MainWindow.pNewFolderPopup) {//Fix the daily issue at releaseing, when the sdcard is removed, we should dismiss the popup window.
			mf_debug();
			evas_object_del(ap->mf_MainWindow.pNewFolderPopup);
			ap->mf_MainWindow.pNewFolderPopup = NULL;
		}

	}
	if (ap->mf_MainWindow.pMmcRemovedPopup) {
		mf_debug();
		evas_object_del(ap->mf_MainWindow.pMmcRemovedPopup);
		ap->mf_MainWindow.pMmcRemovedPopup = NULL;
	}
	//mf_navi_bar_title_set(ap);

}
static void __mf_callback_storage_changed_cb(int storage_id, storage_state_e state, void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "appdata is NULL");
	mf_retm_if(ap->mf_Status.path == NULL || ap->mf_Status.path->str == NULL, "mf_Status.path is NULL");
	int optStorage = MYFILE_NONE;

	if (STORAGE_STATE_MOUNTED == state) {
		if (!(ap->mf_Status.iStorageState & MYFILE_MMC)) {
			__mf_callback_mmc_connected(ap);
		}
		return;
	}

	if (STORAGE_STATE_REMOVED == state || STORAGE_STATE_UNMOUNTABLE == state) {
		mf_debug("mmc removed");
		optStorage = MYFILE_MMC;
	}

	if (optStorage == MYFILE_NONE) {
		mf_debug("get removed storage failed\n");
		return;
	}
	/*here we handle the remove action */
	if (ap->mf_Status.iStorageState & optStorage) {
		__mf_callback_mmc_removed(ap, optStorage);
	}
	return;
}

int mf_callback_set_mmc_state_cb(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	int mmc_state = 0;
	int storage_id = 0;
	mf_retvm_if(ap == NULL, -1, "appdata is NULL");

	mf_util_is_mmc_on(&mmc_state);
	storage_id = mf_util_get_storage_id();

	return storage_set_state_changed_cb(storage_id , __mf_callback_storage_changed_cb, ap);
}

void mf_callback_unregister_mmc_state_cb()
{
	int error_code = -1;
	int storage_id = 0;
	storage_id = mf_util_get_storage_id();
	error_code = storage_unset_state_changed_cb(storage_id, __mf_callback_storage_changed_cb);
	if (error_code != STORAGE_ERROR_NONE) {
		mf_error("storage_unset_state_changed_cb() failed!! for storageid[%d]", storage_id);
	}
}

/******		end of Memory card connection/removal handler	******/

void mf_callback_backbutton_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.more == MORE_DEFAULT) {
		if (ap->mf_Status.entry_path) {
			if (ap->mf_Status.entry_more == MORE_SEARCH) {
				ap->mf_Status.entry_more = MORE_DEFAULT;
				ap->mf_Status.view_type = ap->mf_Status.preViewType;
				SAFE_FREE_GSTRING(ap->mf_Status.path);
				if (ap->mf_Status.view_type == mf_view_root_category) {
					ap->mf_Status.view_type = mf_view_root;
					ap->mf_Status.path = g_string_new(PHONE_FOLDER);
				} else {
					ap->mf_Status.path = g_string_new(ap->mf_Status.entry_path);
				}
				mf_view_update(ap);
				SAFE_FREE_CHAR(ap->mf_Status.entry_path);
			} else if (g_strcmp0(ap->mf_Status.entry_path, ap->mf_Status.path->str) == 0) {
				ap->mf_Status.view_type = ap->mf_Status.preViewType;
				mf_view_update(ap);
				SAFE_FREE_CHAR(ap->mf_Status.entry_path);
				ap->mf_Status.entry_more = MORE_DEFAULT;
			} else {
				mf_callback_upper_click_cb(ap, NULL, NULL);
				SAFE_FREE_CHAR(ap->mf_Status.entry_path);
				ap->mf_Status.entry_more = MORE_DEFAULT;
			}
		} else {
			mf_callback_upper_click_cb(ap, NULL, NULL);
		}
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}
	MF_TRACE_END;
}

Eina_Bool mf_callback_navi_backbutton_clicked_cb(void *data, Elm_Object_Item *it)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, EINA_FALSE, "data is NULL");

	mf_callback_backbutton_clicked_cb(data, NULL, NULL);

	return EINA_FALSE;
}

void mf_callback_new_folder_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is null");

	Eina_Bool entry_empty = elm_entry_is_empty(ap->mf_MainWindow.pEntry);
	if (entry_empty) {
		elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	} else {
		elm_object_signal_emit(obj, "elm,state,clear,visible", "");
	}
	MF_TRACE_END;

}

void mf_callback_genlist_imf_preedit_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is null");
	mfItemData_s *params = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)params->ap;

	Eina_Bool entry_empty = elm_entry_is_empty(ap->mf_MainWindow.pEntry);
	if (elm_object_focus_get(obj)) {
		if (entry_empty) {
			elm_object_item_signal_emit(params->item, "elm,state,eraser,hide", "elm");
		} else {
			elm_object_item_signal_emit(params->item, "elm,state,eraser,show", "elm");
		}
	}

	return;
}

void mf_callback_genlist_imf_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is null");
	mfItemData_s *params = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)params->ap;

	const char *entry_data = NULL;
	char *name = NULL;
	char new_str[MYFILE_FILE_NAME_LEN_MAX] = { '\0', };

	entry_data = elm_entry_entry_get(ap->mf_MainWindow.pEntry);
	mf_retm_if(entry_data == NULL, "entry_data is null");
	name = elm_entry_markup_to_utf8(entry_data);
	mf_retm_if(name == NULL, "name is null");

	Eina_Bool entry_empty = elm_entry_is_empty(ap->mf_MainWindow.pEntry);
	if (elm_object_focus_get(obj)) {
		if (entry_empty) {
			elm_object_item_signal_emit(params->item, "elm,state,eraser,hide", "");
		} else {
			elm_object_item_signal_emit(params->item, "elm,state,eraser,show", "");
		}
	}
	if (mf_file_attr_is_valid_name(name) != MYFILE_ERR_NONE) {
		strncpy(new_str, name, MYFILE_FILE_NAME_LEN_MAX - 1);
		if (strlen(name) > 0) {
			new_str[strlen(name) - 1] = '\0';
		}
		elm_entry_entry_set(ap->mf_MainWindow.pEntry, new_str);
		elm_entry_cursor_end_set(ap->mf_MainWindow.pEntry);
		elm_object_focus_set(ap->mf_MainWindow.pEntry, EINA_FALSE);

		if (ap->mf_MainWindow.pNormalPopup) {
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
		}
		mf_callback_entry_unfocus(ap->mf_MainWindow.pEntry);
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL, MF_MSG_RENAME_ILLEGAL_CHAR, MF_BUTTON_LABEL_OK, NULL,
		                                 NULL, mf_popup_show_vk_cb, ap);
	}
	SAFE_FREE_CHAR(name);
	MF_TRACE_END;
}

int mf_callback_idle_rename(void *data)
{
	MF_TRACE_BEGIN;
	mfItemData_s *params = (mfItemData_s *) data;
	mf_retvm_if(params == NULL, 0, "ap is NULL");
	mf_callback_rename_create_cb(data, NULL, NULL);
	params->ap->mf_Status.rename_timer = NULL;
	MF_TRACE_END;
	return ECORE_CALLBACK_CANCEL;
}

void mf_callback_longpress_rename_cb(void *data, Evas_Object *obj, void *event_info)
{
	mfItemData_s *params = (mfItemData_s *)data;
	mf_retm_if(params == NULL, "input data is NULL");

	struct appdata *ap = (struct appdata *)params->ap;
	mf_retm_if(ap == NULL, "input ap is NULL");
	Elm_Object_Item *it = params->item;
	mf_retm_if(it == NULL, "input item is NULL");

	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	ap->mf_FileOperation.rename_item = it;
	//__mf_callback_idle_rename(data);
	SAFE_DEL_ECORE_TIMER(ap->mf_Status.rename_timer);
	ap->mf_Status.rename_timer = ecore_timer_add(0.05, (Ecore_Task_Cb)mf_callback_idle_rename, data);
	return;

}

#ifdef MYFILE_DETAILS
void mf_callback_detail_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *params = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)params->ap;
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);

	//mf_launch_load_ug(ap, params->m_ItemName->str, MF_LOAD_UG_DETAIL, EINA_FALSE);
	__mf_load_detail_data(ap, params->m_ItemName->str, EINA_FALSE);

	ap->mf_Status.preViewType = ap->mf_Status.view_type;
	ap->mf_Status.view_type = mf_view_detail;
	ap->mf_Status.more = MORE_DEFAULT;
	mf_view_update(ap);
}
#endif

void mf_callback_detail_ctx_cb(void *data, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *itemData = (mfItemData_s *)data;
	struct appdata* ap = (struct appdata *)itemData->ap;
	mf_retm_if(ap == NULL, "ap is NULL");

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	//mf_launch_load_ug(ap, itemData->m_ItemName->str, MF_LOAD_UG_DETAIL, EINA_FALSE);
	__mf_load_detail_data(ap, itemData->m_ItemName->str, EINA_FALSE);

	ap->mf_Status.preViewType = ap->mf_Status.view_type;
	ap->mf_Status.view_type = mf_view_detail;
	mf_view_update(ap);
}

static void __mf_callback_delete_in_idle(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;

	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);

	if (ap->mf_FileOperation.idle_delete_item) {
		mfItemData_s *selected = elm_object_item_data_get(ap->mf_FileOperation.idle_delete_item);
		if (selected && selected->m_ItemName && selected->m_ItemName->str) {
			if (!mf_file_exists(selected->m_ItemName->str)) {
				elm_object_item_del(selected->item);
				ap->mf_FileOperation.idle_delete_item = NULL;
				SAFE_FREE_OBJ(ap->mf_MainWindow.pDeleteConfirmPopup);
				ap->mf_Status.more = MORE_DEFAULT;
				return;
			}
		}
	}

	if (g_strcmp0(label, mf_util_get_text(LABEL_CANCEL)) == 0) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pDeleteConfirmPopup);
		return;
	} else if (g_strcmp0(label, mf_util_get_text(LABEL_DELETE)) == 0) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pDeleteConfirmPopup);

		GList *pSourceList = NULL;
		int count = 0;

		mf_util_merge_eina_list_to_glist(ap->mf_FileRecordList.value_saver, &pSourceList);

		if (pSourceList) {
			count = g_list_length(pSourceList);
		} else {
			MF_TRACE_END;
			return;
		}
		mf_callback_init_operation_cancel(ap);
		ap->mf_FileOperation.iTotalCount = count;
		mf_view_state_set_with_pre(ap, MORE_IDLE_DELETE);
		ap->mf_FileOperation.pSourceList = pSourceList;
		ap->mf_FileOperation.iOperationSuccessFlag = TRUE;
		mf_delete_items(pSourceList, ap->mf_FileOperation.pCancel, TRUE, ap);
		SAFE_FREE_OBJ(ap->mf_MainWindow.pProgressPopup);
		//ap->mf_MainWindow.pProgressPopup = mf_popup_create_pb_popup(ap, LABEL_DELETE, MF_MSG_DELETING, count, mf_callback_progress_bar_cancel_cb, ap);
	}
}

void mf_callback_delete_button_cb(void *data, Evas_Object *obj, void *event_info)
{

	mfItemData_s *params = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)params->ap;

	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);

	if (ap->mf_FileRecordList.value_saver != NULL) {
		mf_util_free_eina_list_with_data(&ap->mf_FileRecordList.value_saver, MYFILE_TYPE_GSTRING);
		ap->mf_FileRecordList.value_saver = NULL;
	}
	ap->mf_FileRecordList.value_saver = eina_list_append(ap->mf_FileRecordList.value_saver, g_string_new(params->m_ItemName->str));
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
	ap->mf_FileOperation.idle_delete_item = params->item;
	ap->mf_MainWindow.pDeleteConfirmPopup = mf_popup_create_delete_confirm_popup(ap, MF_LABEL_DELETE_ITEM,
	                                        MF_LABEL_THIS_ITEM_WILL_DELETE,
	                                        LABEL_CANCEL, LABEL_DELETE,
	                                        __mf_callback_delete_in_idle, ap, 1);
}

void mf_callback_view_as_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_VIEW_AS_LIST, LABEL_VIEW_AS_CHAP, NULL, NULL, NULL, NULL, mf_callback_popup_deleted_cb, ap);
}
void mf_callback_view_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	if (ap->mf_Status.flagViewType == MF_VIEW_STYLE_LIST) {
		ap->mf_Status.flagViewType = MF_VIEW_STYLE_THUMBNAIL;
	} else {
		ap->mf_Status.flagViewType = MF_VIEW_STYLE_LIST;
	}

	mf_util_set_view_style(ap->mf_Status.flagViewType);

	if (ap->mf_Status.flagNoContent != EINA_TRUE) {
		mf_view_update(ap);
	}
	MF_TRACE_END;
}


void mf_callback_home_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;

	mf_fs_monitor_remove_dir_watch();

	SAFE_FREE_GSTRING(ap->mf_Status.path);
	ap->mf_Status.path = g_string_new(PHONE_FOLDER);
	ap->mf_Status.view_type = mf_view_root;

	SAFE_FREE_CHAR(ap->mf_Status.entry_path);
	ap->mf_Status.entry_more = MORE_DEFAULT;
	mf_view_update(ap);
	MF_TRACE_END;
}

void mf_callback_more_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *more = elm_object_item_part_content_get(ap->mf_MainWindow.pNaviItem, NAVI_MORE_BUTTON_PART);

	mf_debug("ap->mf_Status.more = %d", ap->mf_Status.more);
	if (ap->mf_MainWindow.pContextPopup) {
		elm_ctxpopup_dismiss(ap->mf_MainWindow.pContextPopup);
	} else {
		if (ap->mf_Status.more == MORE_DEFAULT) {
			mf_context_popup_create_more(ap, more);
		} else if (ap->mf_Status.more == MORE_SEARCH) {
			mf_context_popup_create_more(ap, more);
		} else if (ap->mf_Status.more == MORE_INTERNAL_MOVE
		           || ap->mf_Status.more == MORE_DATA_MOVING
		           || ap->mf_Status.more == MORE_INTERNAL_COPY
		           || ap->mf_Status.more == MORE_DATA_COPYING) {
			if (ap->mf_Status.view_type != mf_view_root && ap->mf_Status.view_type != mf_view_storage) {
				mf_context_popup_create_more(ap, more);
			}
		}
	}
	return;
}

void mf_callback_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_entry_entry_set(data, "");
}

void mf_callback_extension_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "user_data is NULL");

	struct appdata *ap = (struct appdata *)data;

	Evas_Object *playout = ap->mf_MainWindow.pNaviLayout;
	mf_retm_if(playout == NULL, "get conformant failed");
	Evas_Object *newContent = NULL;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	const char *label = elm_object_item_text_get(item);
	if (g_strcmp0(label, mf_util_get_text(MF_LABEL_SHOW_EXTENSION)) == 0) {
		mf_util_set_extension_state(MF_EXTENSION_SHOW);
		ap->mf_Status.iExtensionState = MF_EXTENSION_SHOW;
	} else if (g_strcmp0(label, mf_util_get_text(MF_LABEL_HIDE_EXTENSION)) == 0) {
		mf_util_set_extension_state(MF_EXTENSION_HIDE);
		ap->mf_Status.iExtensionState = MF_EXTENSION_HIDE;
	} else {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
		return;
	}
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	if (ap->mf_Status.flagNoContent != EINA_TRUE) {
		newContent = mf_navi_bar_content_create(ap);
		mf_navi_bar_set_content(ap, playout, newContent);
	}
	ap->mf_Status.more = MORE_DEFAULT;
}

void mf_callback_profile_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	const char *profile = elm_config_profile_get();

	if (!strcmp(profile, "desktop")) {
		elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, "prev_btn", NULL);
		elm_win_indicator_mode_set(ap->mf_MainWindow.pWindow, ELM_WIN_INDICATOR_HIDE);
	} else {   // mobile
		elm_win_indicator_mode_set(ap->mf_MainWindow.pWindow, ELM_WIN_INDICATOR_SHOW);
	}
	t_end;
	MF_TRACE_END;
}

void mf_callback_share_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	if (mf_edit_file_count_get() > MF_SHARE_ITEM_COUNT_MAX) {
		//P131118-02783 by wangyan , add detailed maximum number in the popup message while share
		char limit_str[256] = {0};
		snprintf(limit_str, sizeof(limit_str) - 1, mf_util_get_text(MF_LABEL_REACH_MAX_SHARE_COUNT), MF_SHARE_ITEM_COUNT_MAX);
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL, limit_str/*MF_LABEL_REACH_MAX_SHARE_COUNT*/, MF_BUTTON_LABEL_OK, NULL, NULL, mf_callback_warning_popup_cb, ap);
		return;
	}
	mf_launch_share(ap);
}

static Eina_Bool
__mf_callback_thumb_created_idler_cb(void *user_data)
{
	MF_TRACE_BEGIN;
	mf_retv_if(user_data == NULL, EINA_FALSE);
	mfItemData_s *pListData = (mfItemData_s *)user_data;
	mf_retv_if(pListData->item == NULL, EINA_FALSE);

	int view_style = mf_view_style_get(pListData->ap);
	if (view_style == MF_VIEW_STYLE_THUMBNAIL) {
		if (pListData->item) {
			elm_gengrid_item_update(pListData->item);
		}
	} else {
		if (pListData->item) {
			elm_genlist_item_update(pListData->item);
		}
	}
	pListData->thumbnail_create = EINA_FALSE;
	g_thumbnail_download_update_idle = NULL;
	MF_TRACE_END;
	return ECORE_CALLBACK_CANCEL;
}

void mf_download_update_idler_del()
{
	mf_ecore_idler_del(g_thumbnail_download_update_idle);
}

void mf_callback_thumb_created_cb(media_content_error_e error, const char *path, void *user_data)
{
	mf_retm_if(user_data == NULL, "user_data is NULL");
	mf_retm_if(path == NULL, "path is NULL");
	mfItemData_s *pListData = (mfItemData_s *)user_data;
	mf_retm_if(pListData->item == NULL, "pListData->item is NULL");

	g_mf_create_thumbnail_count--;
	if (error == MEDIA_CONTENT_ERROR_NONE && mf_file_exists(path)) {

		mf_ecore_idler_del(g_thumbnail_download_update_idle);

		SAFE_FREE_CHAR(pListData->thumb_path);
		if (pListData->file_type == FILE_TYPE_MUSIC || pListData->file_type == FILE_TYPE_SOUND) {
			if (strcmp(path, MF_MUSIC_DEFAULT_THUMBNAIL_FROM_DB) == 0) {
				mf_debug("if in get the path from db is [%s]", path);
				pListData->thumb_path = g_strdup(MF_ICON_MUSIC_THUMBNAIL);
				pListData->thumbnail_type = MF_THUMBNAIL_DEFAULT;
			} else {
				mf_debug("else in get the path from db is [%s]", path);
				pListData->thumb_path = g_strdup(path);
				pListData->thumbnail_type = MF_THUMBNAIL_THUMB;
			}
		} else {
			pListData->thumb_path = g_strdup(path);
			pListData->thumbnail_type = MF_THUMBNAIL_THUMB;
		}

		pListData->real_thumb_flag = TRUE;
		/*int view_style = mf_view_style_get(pListData->ap);
		if (view_style == MF_VIEW_STYLE_THUMBNAIL) {
			if (pListData->item) {
				elm_gengrid_item_update(pListData->item);
			}
		} else {
			if (pListData->item) {
				elm_genlist_item_update(pListData->item);
			}
		}*/
		mf_debug("Update item with new thumbnail[%s]", path);
		g_thumbnail_download_update_idle = ecore_idler_add(__mf_callback_thumb_created_idler_cb, pListData);//Update the ui in mainUI thread
	} else {
		mf_debug("Invalid thumb path!");
	}
}

bool mf_callback_create_thumbnail(void *data, media_thumbnail_completed_cb callback)
{
	mf_retvm_if(data == NULL, -1, "filter is NULL");
	mfItemData_s *pListData = (mfItemData_s *)data;

	int ret = -1;

	if (g_mf_create_thumbnail_count < MF_MAX_MAKE_THUNBNAIL_COUNT) {//Fixed P140827-07370
		ret = media_info_create_thumbnail(pListData->media, callback,
		                                  pListData);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			mf_debug("Failed to create thumbnail! ret is [%d]", ret);
			if (pListData->file_type == FILE_TYPE_IMAGE || pListData->file_type == FILE_TYPE_VIDEO) {
				pListData->thumb_path = g_strdup(pListData->m_ItemName->str);
			}
			return -1;
		} else {
			g_mf_create_thumbnail_count++;
		}
	}
	return 0;
}

void mf_callback_entry_cb(void *data, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *pItemData = (mfItemData_s *)data;

	struct appdata *ap = pItemData->ap;
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);

	if (ap->mf_Status.path == NULL || ap->mf_Status.path->str == NULL) {
		return;
	}
	if (ap->mf_MainWindow.pSearchEntry) {
		elm_object_focus_allow_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
	}

	GString *path = mf_fm_svc_wrapper_get_file_parent_path(pItemData->m_ItemName);
	mf_retm_if(path == NULL, "path is NULL");
	mf_retm_if(path->str == NULL, "path->str is NULL");
	if (ap->mf_Status.more == MORE_SEARCH) {
		ap->mf_Status.more = MORE_DEFAULT ;
		SAFE_FREE_CHAR(ap->mf_Status.entry_path);
		ap->mf_Status.entry_path = g_strdup(ap->mf_Status.path->str);
		ap->mf_Status.entry_more = MORE_SEARCH;
	} else {
		SAFE_FREE_CHAR(ap->mf_Status.entry_path);
		ap->mf_Status.entry_path = g_strdup(path->str);
		ap->mf_Status.entry_more = MORE_DEFAULT;
	}

	//int error_code = 0;
	//int mmc_card = 0;

	SAFE_FREE_CHAR(ap->mf_Status.EnterFrom);
	ap->mf_Status.EnterFrom = g_strdup(pItemData->m_ItemName->str);

	//error_code = mf_util_is_mmc_on(&mmc_card);
	if (ap->mf_Status.more == MORE_DEFAULT) {
		if (mf_fm_svc_wrapper_is_dir(path)) {

			/*set new path */
			if (ap->mf_Status.path != NULL) {
				g_string_free(ap->mf_Status.path, TRUE);
				ap->mf_Status.path = NULL;
			}
			ap->mf_Status.path = path;
			ap->mf_Status.preViewType = ap->mf_Status.view_type;
			ap->mf_Status.view_type = mf_view_normal;
			mf_view_update(ap);
		}
	}
}

void mf_callback_naviframe_title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata*)data;

	Elm_Object_Item *navi_it = event_info;
	if (!navi_it) {
		return;
	}


	Evas_Object *label = elm_object_item_part_content_get(navi_it, "elm.swallow.title");
	if (!label || (ap->mf_Status.more == MORE_EDIT)) {
		return;
	}
	elm_label_slide_go(label);
}

static void __mf_callback_sip_imf_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");


	Evas_Object *nf = ap->mf_MainWindow.pNaviBar;

	if (!nf) {
		return;
	}

	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(nf);

	if (!navi_it) {
		return;
	}

	elm_object_item_signal_emit(navi_it, "elm,state,toolbar,instant_open", "");
	MF_TRACE_END;
}

static void __mf_callback_sip_imf_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");


	Evas_Object *nf = ap->mf_MainWindow.pNaviBar;

	if (!nf) {
		return;
	}

	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(nf);

	if (!navi_it) {
		return;
	}

	elm_object_item_signal_emit(navi_it, "elm,state,toolbar,instant_close", "");
	MF_TRACE_END;
}

void mf_callback_imf_state_callback_register(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	Evas_Object *conform = ap->mf_MainWindow.pConformant;
	evas_object_smart_callback_add(conform, "virtualkeypad,state,off", __mf_callback_sip_imf_hide_cb, ap);

	evas_object_smart_callback_add(conform, "virtualkeypad,state,on", __mf_callback_sip_imf_show_cb, ap);

	evas_object_smart_callback_add(conform, "clipboard,state,off", __mf_callback_sip_imf_hide_cb, ap);

	evas_object_smart_callback_add(conform, "clipboard,state,on", __mf_callback_sip_imf_show_cb, ap);

	MF_TRACE_END;
}

void mf_callback_imf_state_callback_del(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	Evas_Object *conform = ap->mf_MainWindow.pConformant;
	evas_object_smart_callback_del(conform, "virtualkeypad,state,on", __mf_callback_sip_imf_show_cb);

	evas_object_smart_callback_del(conform, "virtualkeypad,state,off", __mf_callback_sip_imf_hide_cb);

	evas_object_smart_callback_del(conform, "clipboard,state,on", __mf_callback_sip_imf_show_cb);

	evas_object_smart_callback_del(conform, "clipboard,state,off", __mf_callback_sip_imf_hide_cb);

	MF_TRACE_END;
}

void mf_callback_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!elm_entry_is_empty(obj)) {
		elm_object_signal_emit(data, "elm,state,eraser,show", "");
	}
	elm_object_signal_emit(data, "elm,state,rename,hide", "");
}

void mf_callback_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_signal_emit(data, "elm,state,eraser,hide", "");
	elm_object_signal_emit(data, "elm,state,rename,show", "");
}

void mf_callback_mouseup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;

	Evas_Event_Mouse_Up *ev = event_info;
	if (ev->button == 3) {
		elm_naviframe_item_pop_cb_set(ap->mf_MainWindow.pNaviItem, mf_callback_navi_backbutton_clicked_cb, ap);
	}
}

void mf_callback_keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;

	Evas_Event_Key_Down *ev = event_info;
	if (!strcmp(ev->keyname, "Escape")) { // if ESC key is down
		elm_naviframe_item_pop_cb_set(ap->mf_MainWindow.pNaviItem, mf_callback_navi_backbutton_clicked_cb, ap);
	}
}

void mf_callback_more_keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");

}
void mf_callback_hardkey_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.more == MORE_DEFAULT) {
		if (ap->mf_Status.view_type == mf_view_root) {
			Evas_Object *win = ap->mf_MainWindow.pWindow;
			elm_win_lower(win);
		} else if (ap->mf_Status.view_type == mf_view_root_category || ap->mf_Status.view_type == mf_view_storage) {
			ap->mf_Status.view_type = mf_view_root;
			mf_view_update(ap);
		} else {
			if (ap->mf_Status.entry_path && g_strcmp0(ap->mf_Status.entry_path, ap->mf_Status.path->str) == 0) {
				ap->mf_Status.view_type = mf_view_root_category;
				mf_view_update(ap);
				mf_object_item_part_content_remove(ap->mf_MainWindow.pNaviItem, TITLE_LEFT_BTN);
				mf_object_item_part_content_remove(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN);
				mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_content_only);
				SAFE_FREE_CHAR(ap->mf_Status.entry_path);
			} else {
				mf_callback_upper_click_cb(ap, NULL, NULL);
			}
		}
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}
	MF_TRACE_END;
}

void mf_callback_hardkey_more_cb(void *data, Elm_Object_Item *it, const char *emission, const char *source)
{
	mf_callback_more_button_cb(data, NULL, NULL);
}

static void __mf_callback_recent_files_confirm_delete(void *data, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *item_data = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)item_data->ap;
	mf_retm_if(ap == NULL, "ap is NULL");

	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);

	if (g_strcmp0(label, mf_util_get_text(LABEL_CANCEL)) == 0) {
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
		ap->mf_MainWindow.pNormalPopup = NULL;
		return;
	} else {
		if (ap->mf_MainWindow.pNormalPopup) {
			evas_object_del(ap->mf_MainWindow.pNormalPopup);
			ap->mf_MainWindow.pNormalPopup = NULL;
		}

		mf_util_db_remove_recent_files(ap->mf_MainWindow.mfd_handle, item_data->m_ItemName->str);

		elm_object_item_del(item_data->item);
		if (elm_genlist_items_count(ap->mf_MainWindow.pNaviGenlist) == 0) {
			Evas_Object *content = (Evas_Object *)mf_object_create_multi_no_content(ap->mf_MainWindow.pNaviLayout);
			mf_object_text_set(content, MF_LABEL_NO_ITEMS, "elm.text");
			mf_navi_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, content);
			ap->mf_Status.flagNoContent = EINA_TRUE;
			ap->mf_MainWindow.pNaviGenlist = NULL;
			ap->mf_MainWindow.pNaviGengrid = NULL;
		}
	}
}

void mf_callback_item_remove_from_recent_files_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *item_data = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)item_data->ap;
	mf_retm_if(ap == NULL, "ap is NULL");

	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap,
	                                 POPMODE_TEXT_TWO_BTN,
	                                 MF_LABEL_REMOVE_FILE,
	                                 MF_LABEL_REMOVE_FROME_RECENT,
	                                 LABEL_CANCEL,
	                                 MF_LABEL_REMOVE,
	                                 NULL,
	                                 __mf_callback_recent_files_confirm_delete,
	                                 item_data);

	//elm_object_item_del(item_data->item);

	MF_TRACE_END;
}

void mf_callback_setting_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	ap->mf_Status.more = MORE_SETTING;
	mf_fs_monitor_remove_dir_watch();

	mf_view_refresh(ap);

	MF_TRACE_END;
}

void mf_callback_item_storage_usage_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	mf_launch_load_storage(ap);

	MF_TRACE_END;
}

void mf_callback_setting_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);

	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
	if (g_strcmp0(label, mf_util_get_text(MF_LABEL_SETTINGS)) == 0) {
		mf_callback_item_storage_usage_cb(ap, NULL, NULL);
	}
}
