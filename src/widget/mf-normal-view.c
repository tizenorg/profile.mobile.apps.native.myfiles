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

#include "mf-object-conf.h"
#include "mf-callback.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-gengrid.h"
#include "mf-util.h"
#include "mf-ta.h"
#include "mf-resource.h"
#include "mf-launch.h"
#include "mf-tray-item.h"
#include "mf-navi-bar.h"
#include "mf-object.h"
#include "mf-search-view.h"

static Evas_Object *__mf_normal_view_create_content(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retvm_if(data == NULL, NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap->mf_Status.path == NULL, NULL, "ap->mf_Status.path is NULL");
	mf_retvm_if(ap->mf_Status.path->str == NULL, NULL, "ap->mf_Status.path->str is NULL");

	Evas_Object *pContent = NULL;
	Eina_List *file_list = NULL;
	Eina_List *dir_list = NULL;
	int dir_list_len = 0;
	int file_list_len = 0;
	int error_code = 0;

	error_code = mf_util_generate_file_list(ap);
	if (error_code != MYFILE_ERR_NONE) {
		ap->mf_Status.flagNoContent = EINA_TRUE;
		pContent = mf_object_create_no_content(ap->mf_MainWindow.pNaviBar);
		mf_object_text_set(pContent, MF_LABEL_NO_FILES, "elm.text");
		/*Todo: we need to free all the Eina_List*/
		MF_TRACE_END;
		return pContent;
	}

	/*	sort the list by sort_type*/
	mf_util_sort_the_file_list(ap);

	file_list = ap->mf_FileOperation.file_list;
	dir_list = ap->mf_FileOperation.folder_list;

	dir_list_len = eina_list_count(dir_list);
	file_list_len = eina_list_count(file_list);

	if ((dir_list_len + file_list_len) > 0) {
		ap->mf_Status.flagNoContent = EINA_FALSE;
	} else {
		ap->mf_Status.flagNoContent = EINA_TRUE;
	}

	pContent = mf_navi_bar_create_view_content(ap);

	t_end;

	MF_TRACE_END;
	return pContent;

}

void mf_normal_view_create(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;

	mf_retm_if(ap->mf_MainWindow.pNaviBar == NULL, "ap->mf_MainWindow.pNaviBar is NULL");
	GString *title = NULL;
	Evas_Object *newContent = NULL;
	Evas_Object *pathinfo = NULL;

	mf_navi_bar_reset_navi_obj(ap);

	ap->mf_Status.pPreNaviItem = ap->mf_MainWindow.pNaviItem;
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", EDJ_NAME);
	free(path);

	ap->mf_MainWindow.pNaviLayout = mf_object_create_layout(ap->mf_MainWindow.pNaviBar, edj_path, "view_layout");

	mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_normal);

	ap->mf_MainWindow.pNaviBox = mf_object_create_box(ap->mf_MainWindow.pNaviLayout);
	newContent = __mf_normal_view_create_content(ap);
	evas_object_show(newContent);

	int location = mf_fm_svc_wrapper_is_root_path(ap->mf_Status.path->str);
	pathinfo = mf_navi_bar_create_normal_pathinfo(ap);
	switch (location) {
	case MYFILE_PHONE:
		title = g_string_new(MF_UG_DETAIL_LABEL_DEVICE_MEMORY);
		break;
	case MYFILE_MMC:
		title = g_string_new(MF_LABEL_SD_CARD);
		break;
	default:
		title = mf_fm_svc_wrapper_get_file_name(ap->mf_Status.path);
		break;
	}

	if (title != NULL) {
		title = g_string_new(LABEL_MYFILE_CHAP);
		ap->mf_MainWindow.naviframe_title = g_strdup(title->str);
		//elm_object_part_text_set(ap->mf_MainWindow.pNaviLayout, "Device_storage_text", ap->mf_MainWindow.naviframe_title);
		g_string_free(title, TRUE);
		title = NULL;
	}
	elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "pathinfo", pathinfo);
	elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);
	mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, ap->mf_MainWindow.pNaviBox);

	if (ap->mf_Status.pPreNaviItem) {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar, ap->mf_Status.pPreNaviItem, "", NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	} else {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar, NULL, NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	}

//	if(ap->mf_Status.more == MORE_DEFAULT) {
//		Evas_Object *search_image = elm_image_add(ap->mf_MainWindow.pNaviLayout);
//		elm_image_file_set(search_image, EDJ_IMAGE, MF_TITLE_ICON_SEARCH);
//		elm_image_resizable_set(search_image, EINA_TRUE, EINA_TRUE);
//		evas_object_show(search_image);
//
//		Evas_Object *btn1 = elm_button_add(ap->mf_MainWindow.pNaviLayout);
//		elm_object_content_set(btn1, search_image);
//		evas_object_smart_callback_add(btn1, "clicked", mf_search_bar_enter_search_routine, ap);
//		elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "search_icon", btn1);
//		ap->mf_MainWindow.pButton = btn1;
//	}
//	Evas_Object *pImage = elm_image_add(ap->mf_MainWindow.pNaviLayout);
//	elm_image_file_set(pImage, EDJ_IMAGE, MF_ICON_SOFT_BACK);
//	elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
//	evas_object_show(pImage);

//	Evas_Object *btn = elm_button_add(ap->mf_MainWindow.pNaviLayout);
//	elm_object_content_set(btn,pImage);
//	elm_object_style_set(btn, "transparent");
//	evas_object_smart_callback_add(btn, "clicked", mf_callback_backbutton_clicked_cb, ap);
//	//elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "back_key", btn);
//	elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, "title_left_btn", btn);

	mf_navi_add_back_button(ap, mf_callback_navi_backbutton_clicked_cb);

	/*add control bar for navigation bar*/
	mf_navi_bar_set_ctrlbar(data);
	if (ap->mf_Status.more == MORE_INTERNAL_COPY) {
		mf_naviframe_left_cancel_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_cancel_cb, ap);
		mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_paste_here_cb, ap);
		mf_navi_bar_title_content_set(ap, LABEL_COPY_HERE);
	} else if (ap->mf_Status.more == MORE_INTERNAL_MOVE) {
		mf_naviframe_left_cancel_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_cancel_cb, ap);
		mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_move_here_cb, ap);
		mf_navi_bar_title_content_set(ap, LABEL_MOVE_TO);
	} else if (ap->mf_Status.more == MORE_EDIT_RENAME) {
		elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_FALSE);
		mf_navi_bar_title_content_set(ap, LABEL_RENAME);
	} else {
		mf_navi_bar_title_content_set(ap, ap->mf_MainWindow.naviframe_title);
		elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_TRUE);
	}
	//mf_navi_bar_title_set(ap);
	t_end;
	/*temp data free*/
	MF_TRACE_END;
}


