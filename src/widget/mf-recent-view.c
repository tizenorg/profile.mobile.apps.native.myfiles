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
#include "mf-edit-view.h"
#include "mf-search-view.h"

Evas_Object *g_mf_recent_content = NULL;

void mf_recent_view_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap->mf_MainWindow.pNaviBar == NULL, "navibar is NULL");

	if (ap->mf_Status.more == MORE_DEFAULT) {
		ap->mf_Status.view_type = mf_view_root;
		mf_view_update(ap);
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}
}

Eina_Bool mf_recent_view_navi_back_cb(void *data, Elm_Object_Item *it)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap->mf_MainWindow.pNaviBar == NULL, EINA_FALSE, "navibar is NULL");

	mf_recent_view_back_cb(data, NULL, NULL);

	return EINA_FALSE;
}

Evas_Object *mf_recent_view_content_create(void *data)
{
    MF_TRACE_BEGIN;
    mf_retvm_if(data == NULL, NULL, "data is NULL");

    Eina_List *file_list = NULL;
    Evas_Object *content = NULL;
    struct appdata *ap = (struct appdata *)data;

    mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.recent_list), MYFILE_TYPE_FSNODE);
    mf_util_generate_saved_files_list(ap, mf_list_recent_files);

    if (eina_list_count(ap->mf_FileOperation.recent_list) == 0) 
    {
        mf_debug("No recent lists");

        content = mf_object_create_multi_no_content(ap->mf_MainWindow.pNaviLayout);
        mf_object_text_set(content, MF_LABEL_NO_FILES, "elm.text");
        ap->mf_MainWindow.pNaviGengrid = NULL;
        ap->mf_MainWindow.pNaviGenlist= NULL;
        ap->mf_Status.flagNoContent = EINA_TRUE;
    } else {
        int iSortTypeValue = 0;
        mf_debug("Recent lists exists");

        content = mf_object_create_genlist(ap->mf_MainWindow.pNaviLayout);
        evas_object_smart_callback_add(content, "language,changed", mf_genlist_gl_lang_changed, data);
        // evas_object_smart_callback_add(content, "longpressed", mf_genlist_gl_longpress, ap);
        evas_object_smart_callback_add(content, "selected", mf_edit_list_item_sel_cb, ap);
        mf_genlist_create_itc_style(&ap->mf_gl_style.itc, mf_item_itc_type_recent);

        mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &iSortTypeValue);

        mf_fs_oper_sort_list(&ap->mf_FileOperation.recent_list, iSortTypeValue);
        file_list = ap->mf_FileOperation.recent_list;
        mf_genlist_create_list_default_style(content, ap, NULL, file_list);
        ap->mf_MainWindow.pNaviGengrid = NULL;
        ap->mf_MainWindow.pNaviGenlist = content;
        ap->mf_Status.flagNoContent = EINA_FALSE;
    }

    MF_TRACE_END;
    g_mf_recent_content = content;
    return content;
}

void mf_recent_view_content_refresh(void *data)
{
   struct appdata *ap = (struct appdata *)data;
   SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
   if (g_mf_recent_content != NULL) {
	   elm_box_unpack(ap->mf_MainWindow.pNaviBox, g_mf_recent_content);
	   if (ap->mf_MainWindow.pNaviGenlist == g_mf_recent_content) {
		   ap->mf_MainWindow.pNaviGenlist = NULL;//avoid the invalid pointer.
	   }
	   evas_object_del(g_mf_recent_content);
	   g_mf_recent_content = NULL;
   }
   Evas_Object *newContent = mf_recent_view_content_create(ap);
   evas_object_show(newContent);
   elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);
   evas_object_show(ap->mf_MainWindow.pNaviBox);
}

void mf_recent_view_create(void *data)
{
    MF_TRACE_BEGIN;
    MF_TA_ACUM_ITEM_BEGIN("12345 mf_root_view_create", 0);
    MF_TA_ACUM_ITEM_BEGIN("123456 create root view layout", 0);

    struct appdata *ap = (struct appdata *)data;
    mf_retm_if(ap == NULL, "ap is NULL");
    mf_retm_if(ap->mf_MainWindow.pNaviBar == NULL, "ap->mf_MainWindow.pNaviBar is NULL");

    mf_navi_bar_reset_navi_obj(ap);
    Evas_Object *pathinfo = NULL;

    ap->mf_Status.pPreNaviItem = ap->mf_MainWindow.pNaviItem;
    ap->mf_MainWindow.pNaviLayout = mf_object_create_layout(ap->mf_MainWindow.pNaviBar, EDJ_NAME, "view_layout");
    mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_normal);
    ap->mf_MainWindow.pNaviBox = mf_object_create_box(ap->mf_MainWindow.pNaviLayout);
    mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, ap->mf_MainWindow.pNaviBox);
    mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.shortcut_list), MYFILE_TYPE_FSNODE);

    Evas_Object *newContent = mf_recent_view_content_create(ap);
    evas_object_show(newContent);
    pathinfo = mf_navi_bar_create_normal_pathinfo(ap);
    elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "pathinfo", pathinfo);
    elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);

    MF_TA_ACUM_ITEM_BEGIN("123456 push naviframe item", 0);
    if (ap->mf_Status.pPreNaviItem) {
        ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar, 
                ap->mf_Status.pPreNaviItem, "", NULL, NULL, ap->mf_MainWindow.pNaviLayout, 
                MF_NAVI_STYLE_ENABLE);
    } else {
        ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar, NULL, NULL, NULL, 
                ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
    }
    MF_TA_ACUM_ITEM_END("123456 push naviframe item", 0);

    GString *title = g_string_new(MF_LABEL_RECTENT_FILES);
    if (title != NULL) {
        SAFE_FREE_CHAR(ap->mf_MainWindow.naviframe_title);
        ap->mf_MainWindow.naviframe_title = g_strdup(title->str);
        g_string_free(title, TRUE);
        title = NULL;
    }
//    Evas_Object *pImage = elm_image_add(ap->mf_MainWindow.pNaviBar);
//    elm_image_file_set(pImage, EDJ_IMAGE, MF_ICON_SOFT_BACK);
//    elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
//    evas_object_show(pImage);
//
//    Evas_Object *btn = elm_button_add(ap->mf_MainWindow.pNaviBar);
//    elm_object_content_set(btn, pImage);
//    elm_object_style_set(btn, "transparent");
//    evas_object_smart_callback_add(btn, "clicked", mf_recent_view_back_cb, ap);
//    elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, "title_left_btn", btn);

//    Evas_Object *search_image = elm_image_add(ap->mf_MainWindow.pNaviLayout);
//    elm_image_file_set(search_image, EDJ_IMAGE, MF_TITLE_ICON_SEARCH);
//    elm_image_resizable_set(search_image, EINA_TRUE, EINA_TRUE);
//    evas_object_show(search_image);
//
//    Evas_Object *btn1 = elm_button_add(ap->mf_MainWindow.pNaviLayout);
//    elm_object_content_set(btn1, search_image);
//    evas_object_smart_callback_add(btn1, "clicked", mf_search_bar_enter_search_routine, ap);
//    elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "search_icon", btn1);
//    ap->mf_MainWindow.pButton = btn1;

    MF_TA_ACUM_ITEM_BEGIN("123456 mf_navi_add_back_button", 0);
    mf_navi_add_back_button(ap, mf_recent_view_navi_back_cb);
    MF_TA_ACUM_ITEM_END("123456 mf_navi_add_back_button", 0);

    /* Add control bar for navigation bar */
    MF_TA_ACUM_ITEM_BEGIN("123456 mf_navi_bar_set_ctrlbar", 0);
    mf_navi_bar_set_ctrlbar(data);
    MF_TA_ACUM_ITEM_END("123456 mf_navi_bar_set_ctrlbar", 0);

    mf_navi_bar_title_content_set(ap, LABEL_MYFILE_CHAP);

    //mf_navi_bar_title_set(ap);

    MF_TRACE_END;
}
