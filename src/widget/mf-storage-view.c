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
#include "mf-genlist.h"
#include "mf-util.h"
#include "mf-ta.h"
#include "mf-resource.h"
#include "mf-navi-bar.h"
#include "mf-object.h"
#include "mf-object-item.h"
#include "mf-view.h"
#include "mf-focus-ui.h"

Eina_Bool mf_storage_view_back_cb(void *data, Elm_Object_Item *it)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	mf_retv_if(ap->mf_MainWindow.pNaviBar == NULL, EINA_FALSE);
	if (ap->mf_Status.more == MORE_DEFAULT) {
		ap->mf_Status.view_type = mf_view_root;
		mf_view_update(ap);
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}

	return EINA_FALSE;

}

Evas_Object *mf_storage_view_create_content(void *data)
{
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	//Eina_List *dir_list = NULL;
	Evas_Object *content = NULL;

	//int view_style = mf_view_style_get(ap);

	if (ap->mf_FileOperation.folder_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.folder_list), MYFILE_TYPE_FSNODE);
	}
	if (ap->mf_FileOperation.file_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.file_list), MYFILE_TYPE_FSNODE);
	}
	mf_util_generate_root_view_file_list(ap, &(ap->mf_FileOperation.folder_list), ap->mf_Status.iStorageState);
	//dir_list = ap->mf_FileOperation.folder_list;

	ap->mf_Status.flagNoContent = EINA_FALSE;

	content = mf_navi_bar_create_view_content(ap);

	return content;
}


void mf_storage_view_create(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	MF_TA_ACUM_ITEM_BEGIN("12345 mf_root_view_create", 0);
	MF_TA_ACUM_ITEM_BEGIN("123456 create root view layout", 0);
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	mf_retm_if(ap->mf_MainWindow.pNaviBar == NULL, "ap->mf_MainWindow.pNaviBar is NULL");
	mf_navi_bar_reset_navi_obj(ap);
	ap->mf_Status.pPreNaviItem = ap->mf_MainWindow.pNaviItem;
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", EDJ_NAME);
	free(path);

	ap->mf_MainWindow.pNaviLayout = mf_object_create_layout(ap->mf_MainWindow.pNaviBar, edj_path, "view_layout");

	mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_normal);

	Evas_Object *newContent = mf_storage_view_create_content(ap);
	evas_object_show(newContent);


	mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, newContent);
	MF_TA_ACUM_ITEM_BEGIN("123456 push naviframe item", 0);
	if (ap->mf_Status.pPreNaviItem) {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar, ap->mf_Status.pPreNaviItem, "", NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	} else {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar, NULL, NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	}
	MF_TA_ACUM_ITEM_END("123456 push naviframe item", 0);

	GString *title = g_string_new(LABEL_MYFILE_CHAP);

	if (title != NULL) {
		SAFE_FREE_CHAR(ap->mf_MainWindow.naviframe_title);
		ap->mf_MainWindow.naviframe_title = g_strdup(title->str);
		g_string_free(title, TRUE);
		title = NULL;
	}
	MF_TA_ACUM_ITEM_BEGIN("123456 mf_navi_add_back_button", 0);

	mf_navi_add_back_button(ap, mf_storage_view_back_cb);
	MF_TA_ACUM_ITEM_END("123456 mf_navi_add_back_button", 0);

	/*add control bar for navigation bar*/
	MF_TA_ACUM_ITEM_BEGIN("123456 mf_navi_bar_set_ctrlbar", 0);
	mf_navi_bar_set_ctrlbar(data);
	MF_TA_ACUM_ITEM_END("123456 mf_navi_bar_set_ctrlbar", 0);
	Evas_Object *pathinfo = mf_navi_bar_create_normal_pathinfo(ap);
	mf_naviframe_left_cancel_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_cancel_cb, ap);
	Evas_Object *btn = mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, NULL, ap);
	elm_object_disabled_set(btn, EINA_TRUE);
	elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "pathinfo", pathinfo);
	mf_navi_bar_title_content_set(ap, MF_LABEL_SELECT_STORAGE);
	elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_TRUE);

	t_end;
	/*temp data free*/
	MF_TRACE_END;

}

