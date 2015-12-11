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

Eina_Bool
mf_detail_view_navi_back_cb(void *data, Elm_Object_Item *it)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	mf_detail_data_destroy(ap);

	mf_retv_if(ap->mf_MainWindow.pNaviBar == NULL, EINA_FALSE);
	if (ap->mf_Status.more == MORE_DEFAULT) {
		ap->mf_Status.view_type = ap->mf_Status.preViewType;
		mf_view_update(ap);
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}

	return EINA_FALSE;
}

void
mf_detail_view_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	mf_detail_data_destroy(ap);

	mf_retm_if(ap->mf_MainWindow.pNaviBar == NULL, "ap->mf_MainWindow.pNaviBar is NULL");
	if (ap->mf_Status.more == MORE_DEFAULT) {
		ap->mf_Status.view_type = ap->mf_Status.preViewType;
		mf_view_update(ap);
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}

	return;
}

Evas_Object *
mf_detail_view_content_create(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	Evas_Object *content = NULL;

	if (ap->mf_Status.detail == NULL) {
		content = mf_object_create_multi_no_content(ap->mf_MainWindow.pNaviLayout);
		mf_object_text_set(content, MF_LABEL_FILE_NOT_EXIST, "elm.text");
		ap->mf_MainWindow.pNaviGengrid = NULL;
		ap->mf_MainWindow.pNaviGenlist = NULL;
	} else {
		Evas_Object *genlist = mf_ug_detail_view_create_genlist(ap->mf_MainWindow.pNaviLayout,
		                       ap->mf_Status.detail);
		mf_ug_detail_view_process_genlist(ap->mf_Status.detail, genlist);
		evas_object_show(genlist);

		content = genlist;
		evas_object_smart_callback_add(content, "language,changed", mf_genlist_gl_lang_changed, data);
		ap->mf_MainWindow.pNaviGengrid = NULL;
		ap->mf_MainWindow.pNaviGenlist = content;
	}
	MF_TRACE_END;
	return content;
}

void mf_detail_view_create(void *data)
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

	mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_content_only);
	ap->mf_MainWindow.pNaviBox = mf_object_create_box(ap->mf_MainWindow.pNaviLayout);
	mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, ap->mf_MainWindow.pNaviBox);

	Evas_Object *newContent = mf_detail_view_content_create(ap);
	evas_object_show(newContent);

	elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);

	MF_TA_ACUM_ITEM_BEGIN("123456 push naviframe item", 0);
	if (ap->mf_Status.pPreNaviItem) {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar,
		                              ap->mf_Status.pPreNaviItem,
		                              "", NULL, NULL,
		                              ap->mf_MainWindow.pNaviLayout,
		                              MF_NAVI_STYLE_ENABLE);
	} else {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar,
		                              NULL, NULL, NULL,
		                              ap->mf_MainWindow.pNaviLayout,
		                              MF_NAVI_STYLE_ENABLE);
	}

//	Evas_Object *pImage = elm_image_add(ap->mf_MainWindow.pNaviBar);
//	elm_image_file_set(pImage, EDJ_IMAGE, MF_ICON_SOFT_BACK);
//	elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
//	evas_object_show(pImage);
//
//	Evas_Object *btn = elm_button_add(ap->mf_MainWindow.pNaviBar);
//	elm_object_content_set(btn, pImage);
//	elm_object_style_set(btn, "transparent");
//	evas_object_smart_callback_add(btn, "clicked", mf_detail_view_back_cb, ap);
//	elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, "title_left_btn", btn);

	mf_navi_add_back_button(ap, mf_detail_view_navi_back_cb);
	MF_TA_ACUM_ITEM_END("123456 mf_navi_add_back_button", 0);

	/*add control bar for navigation bar*/
	MF_TA_ACUM_ITEM_BEGIN("123456 mf_navi_bar_set_ctrlbar", 0);
	//mf_navi_bar_set_ctrlbar(data);
	MF_TA_ACUM_ITEM_END("123456 mf_navi_bar_set_ctrlbar", 0);
	//Evas_Object *pathinfo = mf_genlist_create_path_info(ap->mf_MainWindow.pNaviLayout,
	//						    mf_util_get_text(MF_TITLE_LABEL_DETAILS),
	//						    EINA_FALSE);

	//elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "pathinfo", pathinfo);
	elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_FALSE);
	mf_navi_bar_title_content_set(ap, MF_TITLE_LABEL_DETAILS);

	//mf_navi_bar_title_set(ap);
	t_end;
	/*temp data free*/
	MF_TRACE_END;
}
