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
#include "mf-search-view.h"
#include "mf-dlog.h"
#include "mf-navi-bar.h"
#include "mf-view.h"
#include "mf-object.h"
#include "mf-tray-item.h"
#include "mf-context-popup.h"
#include "mf-file-util.h"

static Eina_Bool __mf_search_view_add_filter_ctxpopup(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, EINA_FALSE, "The ap is NULL");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	ap->mf_MainWindow.pContextPopup = mf_context_popup_search_filter(ap->mf_MainWindow.pNaviBar, ap, ap->mf_MainWindow.pSearchCategoryBtn);
	ap->mf_MainWindow.pPopupTimer = NULL;
	return EINA_FALSE;
}

void mf_callback_category_filter_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
	ap->mf_MainWindow.pPopupTimer = ecore_timer_add(0.2, (Ecore_Task_Cb)__mf_search_view_add_filter_ctxpopup, ap);
}

static void __mf_search_entry_text_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	if (!elm_entry_is_empty(ap->mf_MainWindow.pSearchEntry)) {
		elm_entry_entry_set(ap->mf_MainWindow.pSearchEntry, "");
	}
}

/*
static void __mf_category_btn_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	mf_error("================= Delete");
	ap->mf_MainWindow.pSearchCategoryBtn = NULL;
}*/

static void __mf_search_entry_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *searchbar = ap->mf_MainWindow.pSearchBar;
	MF_CHECK(searchbar);
	Evas_Object *entry = obj;
	MF_CHECK(entry);
	const char *signal = NULL;
	if (elm_entry_is_empty(entry)) {
		elm_object_part_content_unset(searchbar, "elm.swallow.cross");
		evas_object_hide(ap->mf_MainWindow.pSearchLayout);
		signal = "elm,state,eraser,hide";
	} else {
		elm_object_part_content_set(searchbar, "elm.swallow.cross", ap->mf_MainWindow.pSearchLayout);
		evas_object_show(ap->mf_MainWindow.pSearchLayout);
		signal = "elm,state,eraser,show";
	}

	elm_object_signal_emit(searchbar, signal, "elm");
}

int mf_search_bar_unfocused_callback_unselect(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, EINA_FALSE, "Invalid app instance");

	mf_callback_cancel_cb(ap, NULL, NULL);

	return ECORE_CALLBACK_CANCEL;
}

static void mf_search_bar_unfocused_callback(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "Invalid app instance");

	elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);

	ecore_timer_add(0.05, (Ecore_Task_Cb)mf_search_bar_unfocused_callback_unselect, data);
}

void mf_search_view_orientation_get(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *layout = ap->mf_MainWindow.pSearchBar;
	int changed_angle = elm_win_rotation_get(obj);
	mf_debug("changed_angle is [%d]", changed_angle);

	if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
		elm_object_signal_emit(layout, "set_landscape", "bg");
	} else {
		elm_object_signal_emit(layout, "set_portrait", "bg");
	}
}


Evas_Object *mf_search_view_create_search_bar(Evas_Object * parent, void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retv_if(ap == NULL, NULL);
	mf_retv_if(parent == NULL, NULL);

	Evas_Object *sb = NULL;
	Evas_Object *en = NULL;

	sb = elm_layout_add(parent);

	elm_layout_file_set(sb, EDJ_NAME, "myfile_search_layout");
	//elm_layout_theme_set(sb, "layout", "searchbar", "cancel_button");

	en = elm_entry_add(sb);
	elm_object_domain_translatable_part_text_set(en, "elm.guide", "sys_string", "IDS_COM_BODY_SEARCH");

	elm_entry_input_panel_return_key_type_set(en, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);

	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_entry_single_line_set(en, EINA_TRUE);
	elm_entry_cnp_mode_set(en, ELM_CNP_MODE_PLAINTEXT);
	elm_object_part_content_set(sb, "elm.swallow.content", en);

	elm_entry_prediction_allow_set(en, EINA_FALSE);

	elm_entry_input_panel_layout_set(en, ELM_INPUT_PANEL_LAYOUT_NORMAL);

	evas_object_size_hint_weight_set(sb, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(sb, EVAS_HINT_FILL, 0.0);
	elm_entry_autocapital_type_set(en, ELM_AUTOCAPITAL_TYPE_SENTENCE);

	evas_object_smart_callback_add(en, "changed", __mf_search_entry_changed_cb, ap);
	evas_object_smart_callback_add(en, "maxlength,reached", mf_callback_max_len_reached_cb, ap);
	evas_object_smart_callback_add(en, "activated", mf_search_bar_search_started_callback, ap);
	elm_object_signal_callback_add(sb, "elm,eraser,clicked", "elm", mf_callback_eraser_clicked_cb, en);

	Evas_Object *back_btn = elm_image_add(sb);
	elm_image_file_set(back_btn, EDJ_IMAGE, MF_ICON_SOFT_SEARCH_CANCEL);
	elm_image_resizable_set(back_btn, EINA_TRUE, EINA_TRUE);
	elm_image_prescale_set(back_btn, 100);
	elm_image_fill_outside_set(back_btn, TRUE);
	elm_image_smooth_set(back_btn, TRUE);
	evas_object_size_hint_weight_set(back_btn, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(back_btn, EVAS_HINT_FILL, 0.0);
	evas_object_show(back_btn);
	evas_object_smart_callback_add(back_btn, "clicked", (Evas_Smart_Cb)__mf_search_entry_text_del_cb, ap);
	ap->mf_MainWindow.pSearchLayout = back_btn;

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_char_count = MYFILE_FILE_NAME_CHAR_COUNT_MAX;
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size, &limit_filter_data);

	Evas_Object *pImage = elm_image_add(sb);
	elm_image_file_set(pImage, EDJ_IMAGE, MF_ICON_SOFT_SEARCH_BACK);
	elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
	evas_object_show(pImage);

	Evas_Object *btn = elm_button_add(sb);
	elm_object_style_set(btn, "transparent");
	elm_object_content_set(btn, pImage);
	evas_object_smart_callback_add(btn, "clicked", mf_search_bar_unfocused_callback, ap);
	elm_object_part_content_set(sb, "back_button", btn);
	evas_object_show(btn);

	evas_object_show(sb);
	elm_object_focus_set(en, EINA_TRUE);
	ap->mf_MainWindow.pSearchEntry = en;
	ap->mf_MainWindow.pSearchBar = sb;
	int changed_angle = elm_win_rotation_get(ap->mf_MainWindow.pWindow);
	mf_debug("changed_angle is [%d]", changed_angle);
	if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
		elm_object_signal_emit(ap->mf_MainWindow.pSearchBar, "set_landscape", "bg");
	} else {
		elm_object_signal_emit(ap->mf_MainWindow.pSearchBar, "set_portrait", "bg");
	}

	evas_object_smart_callback_add(ap->mf_MainWindow.pWindow, "rotation,changed", mf_search_view_orientation_get, ap);

	MF_TRACE_END;
	return sb;
}

Eina_Bool mf_search_view_back_cb(void *data, Elm_Object_Item *it)
{
	mf_retvm_if(data == NULL, EINA_FALSE, "data is NULL");
	struct appdata *ap = (struct appdata *)data;


	SAFE_FREE_OBJ(ap->mf_MainWindow.pProgressPopup);
	SAFE_DEL_ECORE_TIMER(ap->mf_FileOperation.search_IME_hide_timer);
	SAFE_DEL_ECORE_TIMER(ap->mf_MainWindow.pPopupTimer);//Fix the bug(P131205-04843).

	mf_callback_cancel_cb(ap, NULL, NULL);
	return EINA_FALSE;
}

void mf_search_view_create(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	mf_navi_bar_reset_navi_obj(ap);

	Evas_Object *pSearchViewLayout = NULL;

	if (ap->mf_Status.view_type == mf_view_root_category) {
		ap->mf_Status.search_category = ap->mf_Status.category_type;
	} else {
		ap->mf_Status.search_category = mf_tray_item_category_none;
	}

	ap->mf_MainWindow.pNaviLayout =	mf_object_create_layout(ap->mf_MainWindow.pNaviBar, EDJ_NAME, "search_view_layout");
	ap->mf_MainWindow.pNaviSearchBar = mf_search_view_create_search_bar(ap->mf_MainWindow.pNaviLayout, ap);
	Evas_Object *bx = elm_box_add(ap->mf_MainWindow.pNaviLayout);
	elm_box_pack_start(bx, ap->mf_MainWindow.pNaviSearchBar);

	pSearchViewLayout = ap->mf_MainWindow.pNaviLayout;

	elm_object_part_content_set(pSearchViewLayout, "search_bar", bx);

	elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "elm.pathinfo.hide", "elm");
	ap->mf_MainWindow.pNaviBox = mf_object_create_box(ap->mf_MainWindow.pNaviLayout);
	mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, ap->mf_MainWindow.pNaviBox);

	ap->mf_Status.pPreNaviItem = ap->mf_MainWindow.pNaviItem;
	if (ap->mf_Status.pPreNaviItem) {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar, ap->mf_Status.pPreNaviItem, NULL, NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	} else {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar, NULL, NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	}

	mf_navi_add_back_button(ap, mf_search_view_back_cb);

	/*hide Tab Bar in search view*/

	/*add control bar for navigation bar*/
	/*temp data free*/

	SAFE_FREE_CHAR(ap->mf_MainWindow.naviframe_title);
	if (ap->mf_Status.view_type != mf_view_root) {
		if (g_strcmp0(ap->mf_Status.path->str, PHONE_FOLDER) == 0) {
			ap->mf_MainWindow.naviframe_title = g_strdup(MF_LABEL_DEVICE_MEMORY);
		} else if (g_strcmp0(ap->mf_Status.path->str, MEMORY_FOLDER) == 0) {
			ap->mf_MainWindow.naviframe_title = g_strdup(MF_LABEL_SD_CARD);
			//return g_strdup(mf_util_get_text(MF_LABEL_SD_CARD));
		} else {
			const char *name = mf_file_get(ap->mf_Status.path->str);
			if (name) {
				ap->mf_MainWindow.naviframe_title = g_strdup(name);
			} else {
				ap->mf_MainWindow.naviframe_title = g_strdup(MF_LABEL_SEARCH_CHAP);
			}
		}
		//mf_navi_bar_title_set(ap);
	} else {
		//mf_navi_bar_title_content_set(ap, LABEL_MYFILE);
	}
	//
	
	elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_FALSE, EINA_FALSE);
	evas_object_show(ap->mf_MainWindow.pSearchEntry);

	elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_TRUE);
	SAFE_DEL_NAVI_ITEM(&ap->mf_Status.pPreNaviItem);

	MF_TRACE_END;
}

