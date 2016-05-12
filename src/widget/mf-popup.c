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

#include <bundle.h>
#include <notification.h>

#include "mf-util.h"
#include "mf-object-conf.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-callback.h"
#include "mf-genlist.h"
#include "mf-view.h"
#include "mf-object.h"
#include "mf-launch.h"
#include "mf-popup.h"
#include "mf-media.h"
#include "mf-resource.h"

#include "mf-ug-detail.h"
#include "mf-file-util.h"

#define ITEM_COUNT	6
#define ITEM_MAX_COUNT	7
#define ITEM_MAX_COUNT_LANDSCAPE	3
#define MF_POPUP_MENUSTYLE_WIDTH 240
#define MF_POPUP_MENUSTYLE_HEIGHT(x) (52*x)
#define RADIO_POPUP_TIMEOUT	0.5
#define VALUE 1000

static Evas_Object *second_popup = NULL;
//static Evas_Object *first_popup_window = NULL;
//static Evas_Object *first_popup = NULL;
static Evas_Object *ok_button = NULL;
static char *entry_string = NULL;
static char *operation_rename_item_name = NULL;

//1  Sort by
static Evas_Object *sort_group_radio = NULL;
static Evas_Object *order_group_radio = NULL;
static int sort_type_index = 0;
static int order_index = 0;
static int g_sort_type = MYFILE_SORT_BY_NONE;


typedef struct {
	int index;
	Elm_Object_Item *item;
	struct appdata *ap;
	char *title;
} ListByData_s;

static Evas_Smart_Cb rename_save_cb = NULL;
static Evas_Smart_Cb rename_cancel_cb = NULL;
static void *rename_save_params = NULL;
static void *rename_cancel_params = NULL;

static void __mf_popup_view_as_genlist_change(ListByData_s *params);

char *mf_popup_rename_item_name_get()
{
	return operation_rename_item_name;
}

void mf_popup_rename_cancel()
{
	if (rename_cancel_cb && rename_cancel_params) {
		rename_cancel_cb(rename_cancel_params, NULL, NULL);
	}
}

void mf_popup_second_popup_destory()
{
	SAFE_FREE_OBJ(second_popup);
}
void mf_popup_rename_func_set(Evas_Smart_Cb save_cb, void *save_params, Evas_Smart_Cb cancel_cb, void *cancel_params)
{
	rename_save_cb = save_cb;
	rename_cancel_cb = cancel_cb;
	rename_save_params = save_params;
	rename_cancel_params = cancel_params;

}

void mf_popup_rename_func_reset()
{
	rename_save_cb = NULL;
	rename_cancel_cb = NULL;
	rename_save_params = NULL;
	rename_cancel_params = NULL;
}

void mf_popup_ok_button_set(Evas_Object *obj)
{
	ok_button = obj;
}

void mf_popup_entry_string_set(char *string)
{
	SAFE_FREE_CHAR(entry_string);
	if (string) {
		entry_string = g_strdup(string);
	}
}

char *mf_popup_entry_string_get()
{
	return entry_string;
}

/*static int __mf_popup_get_list_by_selected_item()
{
	int iSortTypeValue = 0;
	mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &iSortTypeValue);

	int ret = -1;
	switch (iSortTypeValue) {
	case	MYFILE_SORT_BY_DATE_O2R:
		ret = 3;
		break;
	case	MYFILE_SORT_BY_DATE_R2O:
		ret = 2;
		break;
	case	MYFILE_SORT_BY_TYPE_A2Z:
		ret = 5;
		break;
	case	MYFILE_SORT_BY_NAME_A2Z:
		ret = 0;
		break;
	case	MYFILE_SORT_BY_NAME_Z2A:
		ret = 1;
		break;
	case	MYFILE_SORT_BY_SIZE_L2S:
		ret = 4;
		break;
	case	MYFILE_SORT_BY_SIZE_S2L:
	case	MYFILE_SORT_BY_TYPE_Z2A:
	default:
		ret = 0;
		break;
	}
	return ret;
}*/

static int __mf_popup_get_view_as_type(int index)
{
	eMfViewStyle ret = MF_VIEW_STYLE_LIST;
	switch (index) {
	case 0:
		ret = MF_VIEW_STYLE_LIST;
		break;
	case 1:
		ret = MF_VIEW_SYTLE_LIST_DETAIL;
		break;
	case 2:
		ret = MF_VIEW_STYLE_THUMBNAIL;
		break;
	default:
		ret = MF_VIEW_STYLE_LIST;
		break;
	}
	return ret;
}


static char *__mf_popup_view_as_genlist_label_get(void *data, Evas_Object * obj, const char *part)
{
	ListByData_s *params = (ListByData_s *) data;
	assert(params);
	struct appdata *ap = params->ap;
	assert(ap);
	assert(part);

	eMfViewStyle view_as_type = MF_VIEW_STYLE_LIST;

	char *ret = NULL;
	if (!strcmp(part, "elm.text")) {
		view_as_type = __mf_popup_get_view_as_type(params->index);
		switch (view_as_type) {
		case MF_VIEW_STYLE_LIST:			      /**< Sort by file name ascending */
			ret = g_strdup(mf_util_get_text(MF_LABEL_LIST));
			break;
		case MF_VIEW_SYTLE_LIST_DETAIL:			      /**< Sort by file size ascending */
			ret = g_strdup(mf_util_get_text(LABEL_LIST_DETAIL_VIEW));
			break;
		case MF_VIEW_STYLE_THUMBNAIL:			      /**< Sort by file date ascending */
			ret = g_strdup(mf_util_get_text(MF_LABEL_THUMBNAILS));
			break;
		default:
			break;
		}
		return ret;
	}
	return NULL;
}

static void __gl_popup_viewby_radio_smart_cb(void *data,
        Evas_Object *obj,
        void *vi)
{
	MF_TRACE_BEGIN;
	mf_debug("radio show has finished by smart callback.");
	ListByData_s *params = (ListByData_s *) data;
	if (params == NULL) {
		return;
	}
	__mf_popup_view_as_genlist_change(params);
	MF_TRACE_END;
}

static void __gl_popup_viewby_radio_signal_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MF_TRACE_BEGIN;
	mf_debug("radio show has finished by signal callback.");
	ListByData_s *params = (ListByData_s *) data;
	if (params == NULL) {
		return;
	}
	__mf_popup_view_as_genlist_change(params);
	MF_TRACE_END;
}

static Evas_Object *__mf_popup_view_as_genlist_icon_get(void *data, Evas_Object * obj, const char *part)
{
	ListByData_s *params = (ListByData_s *) data;
	assert(params);
	struct appdata *ap = params->ap;
	assert(ap);
	assert(part);

	if (!strcmp(part, "elm.swallow.end")) {
		Evas_Object *radio = NULL;
		radio = elm_radio_add(obj);
		elm_object_focus_set(radio, EINA_FALSE);
		elm_object_focus_allow_set(radio, EINA_FALSE);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		elm_radio_state_value_set(radio, params->index);
		elm_radio_group_add(radio, ap->mf_Status.pRadioGroup);
		elm_radio_value_set(radio, ap->mf_Status.iRadioValue);


		evas_object_smart_callback_add(radio, "changed", __gl_popup_viewby_radio_smart_cb, params);
		elm_object_signal_callback_add(radio, "elm,action,radio,toggle", "",
		                               __gl_popup_viewby_radio_signal_cb, params);
		evas_object_show(radio);
		return radio;
	}
	return NULL;
}

/*
static void __gl_popup_sortby_radio_cb(void *data,
				       Evas_Object *obj,
				       void *vi)
{
	MF_TRACE_BEGIN;
	mf_debug("sort by radio show has finished.");
	ListByData_s *params = (ListByData_s *) data;
	if (params == NULL)
		return;
	__mf_popup_sort_by_genlist_change(params);
	MF_TRACE_END;
}*/
/*static Evas_Object *__mf_popup_sort_by_genlist_icon_get(void *data, Evas_Object * obj, const char *part)
{
	ListByData_s *params = (ListByData_s *) data;
	assert(params);
	struct appdata *ap = params->ap;
	assert(ap);
	assert(part);

	if (!strcmp(part, "elm.icon")) {
		Evas_Object *radio = NULL;
		radio = elm_radio_add(obj);
		elm_object_focus_set(radio, EINA_FALSE);
		elm_object_focus_allow_set(radio, EINA_FALSE);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		elm_radio_state_value_set(radio, params->index);
		elm_radio_group_add(radio, ap->mf_Status.pRadioGroup);
		elm_radio_value_set(radio, ap->mf_Status.iRadioValue);

		evas_object_smart_callback_add(radio, "changed",
					       __gl_popup_sortby_radio_cb, params);
		elm_object_signal_callback_add(radio, "elm,action,radio,toggle", "",
				 (Edje_Signal_Cb)__gl_popup_sortby_radio_cb,
					       params);
		evas_object_show(radio);
		return radio;
	}
	return NULL;
}*/

/*static Evas_Object *__mf_popup_get_genlist_icon(void *data, Evas_Object * obj, const char *part)
{
	ListByData_s *params = (ListByData_s *) data;
	assert(params);
	struct appdata *ap = params->ap;
	assert(ap);
	assert(part);

	if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *layout = elm_layout_add(obj);
		elm_layout_theme_set(layout, "layout",
							"list/B/type.4", "default");

		Evas_Object *radio = NULL;
		radio = elm_radio_add(layout);
		elm_object_focus_set(radio, EINA_FALSE);
		elm_radio_state_value_set(radio, params->index);
		elm_radio_group_add(radio, ap->mf_Status.pRadioGroup);
		elm_radio_value_set(radio, ap->mf_Status.iRadioValue);
		evas_object_size_hint_align_set(radio,
					EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio,
					EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_content_set(layout,
								"elm.swallow.content", radio);
		evas_object_show(radio);
		return layout;
	}
	return NULL;
}*/

Ecore_Timer *g_popup_timer = NULL;

void mf_popup_timer_del()
{
	SAFE_TIMER_DEL(g_popup_timer);
}


void mf_popup_view_as_response(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	if (ap->mf_MainWindow.pNormalPopup != NULL) {
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
	}
	ap->mf_MainWindow.pNormalPopup = NULL;

	if (ap->mf_Status.flagNoContent != EINA_TRUE) {
		ap->mf_Status.flagViewAsRefreshView = EINA_TRUE;
		mf_view_update(ap);
	}
}

static Eina_Bool
__mf_popup_view_as_response_cb(void *user_data)
{
	MF_TRACE_BEGIN;
	mf_retv_if(user_data == NULL, EINA_FALSE);
	struct appdata *ap = (struct appdata *)user_data;

	mf_popup_view_as_response(ap);
	g_popup_timer = NULL;
	MF_TRACE_END;
	return ECORE_CALLBACK_CANCEL;
}

static void __mf_popup_view_as_genlist_select(void *data, Evas_Object * obj, void *event_info)
{
	assert(data);
	assert(event_info);
	ListByData_s *params = NULL;
	int iViewAsType = 0;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	if (item != NULL) {
		params = (ListByData_s *) elm_object_item_data_get(item);
		if (params != NULL && params->ap != NULL) {
			elm_radio_value_set(params->ap->mf_Status.pRadioGroup, params->index);
			iViewAsType = __mf_popup_get_view_as_type(params->index);
			params->ap->mf_Status.flagViewType = iViewAsType;
			mf_util_set_view_style(iViewAsType);

			SAFE_TIMER_DEL(g_popup_timer);
			g_popup_timer = ecore_timer_add(RADIO_POPUP_TIMEOUT, __mf_popup_view_as_response_cb, params->ap);//Fixed P131207-01294
			__mf_popup_view_as_genlist_change(params);

			mf_popup_view_as_response(params->ap);
		}
	}
}
static int __mf_popup_get_view_as_selected_item()
{
	int iViewAs = 0;

	mf_util_get_pref_value(PREF_TYPE_VIEW_STYLE, &iViewAs);

	int ret = -1;
	switch (iViewAs) {
	case	MF_VIEW_STYLE_LIST:
		ret = 0;
		break;
	case	MF_VIEW_SYTLE_LIST_DETAIL:
		ret = 1;
		break;
	case	MF_VIEW_STYLE_THUMBNAIL:
		ret = 2;
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}
Ecore_Timer *mf_progress_bar_timer = NULL;

static void __mf_popup_progress_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.more == MORE_SEARCH ||
	        (ap->mf_Status.view_type == mf_view_root_category && ap->mf_Status.more != MORE_SEARCH)) {
		if (ap->mf_Status.flagSearchStart) {
			//mf_search_bar_stop(ap);
		}
	}
	if (mf_progress_bar_timer != NULL) {
		ecore_timer_del(mf_progress_bar_timer);
		mf_progress_bar_timer = NULL;
	}
	ap->mf_MainWindow.pProgressPopup = NULL;

}

static void __mf_popup_pb_popup_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_MainWindow.pNaviGenlist) {
		evas_object_data_set(ap->mf_MainWindow.pNaviGenlist, "popup", NULL); // Set popup data as NULL when popup is deleted.
		elm_object_scroll_freeze_pop(ap->mf_MainWindow.pNaviGenlist);	   // Enable scrolling
	}

	ap->mf_MainWindow.pProgressPopup = NULL;

}

static void __mf_popup_normal_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_MainWindow.pNormalPopup) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
	}
	if (ap->mf_MainWindow.pPopupBox) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pPopupBox);
	}
	if (ap->mf_MainWindow.pDeleteConfirmPopup) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pDeleteConfirmPopup);
	}
	if (ap->mf_MainWindow.pNaviGenlist) {
		evas_object_data_set(ap->mf_MainWindow.pNaviGenlist, "popup", NULL); // Set popup data as NULL when popup is deleted.
		elm_object_scroll_freeze_pop(ap->mf_MainWindow.pNaviGenlist);      // Enable scrolling
	}
	SAFE_FREE_OBJ(sort_group_radio);
	SAFE_FREE_OBJ(order_group_radio);
}

static void __mf_popup_second_popup_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	SAFE_FREE_OBJ(second_popup);
}

static void __mf_popup_longpress_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	ap->mf_MainWindow.pLongpressPopup = NULL;
	if (ap->mf_MainWindow.pNaviGenlist) {
		evas_object_data_set(ap->mf_MainWindow.pNaviGenlist, "popup", NULL); // Set popup data as NULL when popup is deleted.
		elm_object_scroll_freeze_pop(ap->mf_MainWindow.pNaviGenlist);      // Enable scrolling
	}
}

void mf_popup_clear_btn_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *entry = (Evas_Object *)data;
	if (entry) {
		elm_entry_cursor_end_set(entry);
	}

}

void mf_popup_new_folder_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME) {
		mf_view_state_reset_state_with_pre(ap);
	}

	ap->mf_MainWindow.pNewFolderPopup = NULL;
	SAFE_FREE_CHAR(operation_rename_item_name);
	SAFE_FREE_CHAR(entry_string);
	SAFE_FREE_OBJ(second_popup);
	SAFE_FREE_CHAR(ap->nTemp_entry);
	ok_button = NULL;
	mf_popup_rename_func_reset();
}

static void __mf_popup_new_folder_cancel_cb(void *data, Evas_Object * obj, void *event_info)
{
	mf_error();
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
	SAFE_FREE_OBJ(second_popup);
	SAFE_FREE_CHAR(ap->nTemp_entry);
}
/******************************
** Prototype    : mf_popup_create_pb_popup
** Description  :
** Input        : void *data
**                char *context
**                int file_count
**                void*func
**                void* param
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
Evas_Object *mf_popup_create_pb_popup(void *data, char *title, char *context, int file_count, void *func, void *param)
{

	struct appdata *ap;

	ap = (struct appdata *)data;
	assert(ap);

	char count[256] = {0,};
	Evas_Object *popup = NULL;
	Evas_Object *progressbar = NULL;
	Evas_Object *layout = NULL;

	snprintf(count, sizeof(count), "%s0/%d", count, file_count);
	popup = elm_popup_add(ap->mf_MainWindow.pMainLayout);

	elm_object_focus_set(popup, EINA_FALSE);

	/*if (title) {
		mf_object_text_set(popup, title, "title,text");
	}*/
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", EDJ_NAME);
	free(path);

	layout = elm_layout_add(popup);
	elm_object_focus_set(layout, EINA_FALSE);
	elm_layout_file_set(layout, edj_path, "popup_center_progressview");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ap->mf_MainWindow.pProgressLayout = layout;

	progressbar = elm_progressbar_add(popup);
	elm_object_focus_set(progressbar, EINA_FALSE);
	ap->mf_FileOperation.progress_bar = progressbar;
	elm_object_style_set(progressbar, "wheel");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//elm_progressbar_value_set(progressbar, 0.0);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_show(progressbar);

	elm_object_part_content_set(layout, "elm.swallow.progressbar", progressbar);
	mf_object_text_set(layout, context, "elm.title");
	/*{//Fixing the P140801-06774
		char *fileName = NULL;
		char *name = NULL;
		if (ap->mf_FileOperation.pSourceList != NULL) {
			fileName = (char*)strdup((char *)ap->mf_FileOperation.pSourceList->data);
			if (fileName) {
				name = (char*)strdup(mf_file_get(fileName));
				free(fileName);
			}
		}
		mf_debug("name=%s", name);
		if (name) {
			mf_object_text_set(layout, name, "elm.title.filename");
			free(name);
		}
	}
	elm_object_part_text_set(layout, "elm.text.left", "0%");
	elm_object_part_text_set(layout, "elm.text.right", count);*/

	elm_object_content_set(popup, layout);

	/*Evas_Object *btn1 = mf_object_create_button(popup,
						    NULL, //"popup_button/default",
						    LABEL_CANCEL,
						    NULL,
						    func,
						    param,
						    EINA_FALSE);
	elm_object_part_content_set(popup, "button1", btn1);*/

	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_pb_popup_del_cb, ap);
	if (func) {
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, func, param);
	} else {
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	}

	return popup;
}

Evas_Object *mf_popup_entry_create(Evas_Object *parent)
{
	Evas_Object *en = NULL;

	en = elm_entry_add(parent);//Using the style to instead of the entry, it will include the other style.
	eext_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,
	                        ELM_SCROLLER_POLICY_AUTO);
	elm_object_signal_emit(en, "elm,action,hide,search_icon", "");
	elm_object_part_text_set(en, "elm.guide", "");

	elm_entry_single_line_set(en, EINA_TRUE);
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_entry_prediction_allow_set(en, EINA_FALSE);
	evas_object_show(en);

	//Evas_Object *clear_button= elm_object_part_content_get(en, "elm.swallow.clear");
	//evas_object_event_callback_add(clear_button, EVAS_CALLBACK_SHOW, mf_popup_clear_btn_show_cb, en);

	return en;
}

Evas_Object *mf_popup_entry_layout_create(Evas_Object *parent)
{
	Evas_Object *layout = elm_layout_add(parent);
	elm_object_focus_set(layout, EINA_TRUE);
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", EDJ_NAME);
	free(path);

	elm_layout_file_set(layout, edj_path, "popup_new_folder");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return layout;
}

void mf_popup_create_folder_imf_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is null");
	struct appdata *ap = (struct appdata *)data;

	const char *entry_data = NULL;
	char new_str[MYFILE_FILE_NAME_LEN_MAX] = { '\0', };

	entry_data = elm_entry_entry_get(ap->mf_MainWindow.pEntry);
	mf_retm_if(entry_data == NULL, "entry_data is null");
	char *name = elm_entry_markup_to_utf8(entry_data);
	mf_retm_if(name == NULL, "name is null");

	SECURE_DEBUG("name is [%s]", name);
	if (mf_file_attr_is_valid_name(name) != MYFILE_ERR_NONE) {
		strncpy(new_str, name, MYFILE_FILE_NAME_LEN_MAX - 1);
		int position = elm_entry_cursor_pos_get(ap->mf_MainWindow.pEntry);
		elm_entry_entry_set(ap->mf_MainWindow.pEntry, elm_entry_utf8_to_markup(ap->nTemp_entry));
		elm_entry_cursor_begin_set(ap->mf_MainWindow.pEntry);
		elm_entry_cursor_pos_set(ap->mf_MainWindow.pEntry, position - 1);
		mf_popup_indicator_popup(data, mf_util_get_text(MF_MSG_RENAME_ILLEGAL_CHAR));//Fixed P131115-04660
		SAFE_FREE_CHAR(name);
		return;
	}

	SAFE_FREE_CHAR(ap->nTemp_entry);
	ap->nTemp_entry = strdup(name);
	Eina_Bool entry_empty = elm_entry_is_empty(ap->mf_MainWindow.pEntry);
	if (entry_empty) {
		mf_error("elm,state,eraser,hide");
		elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
		if (ok_button) {
			elm_object_disabled_set(ok_button, EINA_TRUE);
		}
	} else {
		mf_error("elm,state,eraser,show");
		elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		if (ok_button) {
			if (entry_string && g_strcmp0(entry_data, entry_string) == 0) {
				elm_object_disabled_set(ok_button, EINA_TRUE);
			} else {
				elm_object_disabled_set(ok_button, EINA_FALSE);
			}
		}
	}
	SAFE_FREE_CHAR(name);
	MF_TRACE_END;
}

char *mf_popup_rename_text_get(const char *fullpath, char **suffix, Eina_Bool suffix_flag)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(fullpath == NULL, NULL, "fullpath is NULL");

	/* set guide text */
	char *guide_text = NULL;
	const char *filename = mf_file_get(fullpath);

	if (suffix_flag) {
		mf_debug();
		char *ext = NULL;
		char *name_without_ext = NULL;
		name_without_ext = g_strdup(fullpath);
		mf_file_attr_get_file_ext(fullpath, &ext);
		mf_debug("ext is %s", ext);
		if (ext && strlen(ext) != 0) {
			mf_debug();
			name_without_ext[strlen(name_without_ext) - strlen(ext) - 1] = '\0';
			*suffix = strdup(ext);
			SECURE_DEBUG("name_without_ext is [%s]\n", name_without_ext);
			if (strlen(name_without_ext)) {
				guide_text = elm_entry_utf8_to_markup(mf_file_get(name_without_ext));
			} else {
				guide_text = elm_entry_utf8_to_markup(filename);
			}
		} else {
			guide_text = elm_entry_utf8_to_markup(filename);
		}

		SAFE_FREE_CHAR(ext);
		SAFE_FREE_CHAR(name_without_ext);
	} else {
		guide_text = elm_entry_utf8_to_markup(filename);
	}
	MF_TRACE_END;

	return guide_text;
}

Evas_Object *mf_popup_create_rename_popup(void *data, char *context)
{

	mfItemData_s *params = (mfItemData_s *) data;
	mf_retvm_if(params == NULL, NULL, "param is NULL");
	mf_retvm_if(params->m_ItemName == NULL, NULL, "m_ItemName is NULL");

	operation_rename_item_name = g_strdup(params->m_ItemName->str);

	struct appdata *ap = (struct appdata *)params->ap;
	mf_retvm_if(ap == NULL, NULL, "input parameter data error");

	Evas_Object *popup;
	char *text = NULL;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	popup = elm_popup_add(ap->mf_MainWindow.pMainLayout);
	elm_object_signal_emit(popup, "elm,action,center_popup,entry", "");

	elm_object_focus_set(popup, EINA_FALSE);

	Evas_Object *layout = mf_popup_entry_layout_create(popup);
	elm_object_content_set(popup, layout);

	Evas_Object *en = NULL;
	en = mf_popup_entry_create(layout);

	if (ap->mf_FileOperation.to_rename != NULL) {
		g_string_free(ap->mf_FileOperation.to_rename, TRUE);
		ap->mf_FileOperation.to_rename = NULL;
	}
	char *file_name = NULL;
	file_name = g_strdup(params->m_ItemName->str);
	ap->mf_FileOperation.to_rename = g_string_new(file_name);
	SAFE_FREE_CHAR(ap->mf_FileOperation.file_name_suffix);

	//text = mf_popup_rename_text_get(file_name, &ap->mf_FileOperation.file_name_suffix, EINA_TRUE);
	//limit_filter_data.max_char_count = (MYFILE_FILE_NAME_CHAR_COUNT_MAX);
	if (params->file_type != FILE_TYPE_DIR) {
		text = mf_popup_rename_text_get(file_name, &ap->mf_FileOperation.file_name_suffix, EINA_TRUE);
	} else {
		text = mf_popup_rename_text_get(file_name, &ap->mf_FileOperation.file_name_suffix, EINA_FALSE);
	}
	limit_filter_data.max_char_count = (MYFILE_FILE_NAME_CHAR_COUNT_MAX);
	SAFE_FREE_CHAR(file_name);

	elm_entry_entry_set(en, text);
	elm_entry_select_all(en);
	entry_string = g_strdup(text);
	SAFE_FREE_CHAR(text);
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size, &limit_filter_data);
	ap->mf_MainWindow.pEntry = en;

	mf_object_text_set(popup, context, "title,text");

	evas_object_smart_callback_add(en, "maxlength,reached", (Evas_Smart_Cb)mf_callback_max_len_reached_cb, ap);
	evas_object_smart_callback_add(en, "clicked", (Evas_Smart_Cb)mf_callback_clicked_cb, ap);
	evas_object_smart_callback_add(en, "changed", (Evas_Smart_Cb)mf_popup_create_folder_imf_changed_cb, ap);
	evas_object_smart_callback_add(en, "activated", (Evas_Smart_Cb)mf_callback_rename_save_cb, rename_save_params);
	evas_object_smart_callback_add(en, "preedit,changed", (Evas_Smart_Cb)mf_popup_create_folder_imf_changed_cb/*mf_callback_new_folder_changed_cb*/, ap);
	evas_object_smart_callback_add(en, "longpressed", (Evas_Smart_Cb)mf_callback_long_clicked_cb, ap);
	elm_entry_input_panel_return_key_type_set(en, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	elm_object_part_content_set(layout, "elm.swallow.content", en);

//	elm_object_signal_emit(en, "elm,action,hide,search_icon", "");

	Evas_Object *btn1 = mf_object_create_button(popup,
	                    NULL, //"popup_button/default",
	                    LABEL_CANCEL,
	                    NULL,
	                    (Evas_Smart_Cb)rename_cancel_cb,
	                    rename_cancel_params,
	                    EINA_FALSE);

	Evas_Object *btn2 = mf_object_create_button(popup,
	                    NULL, //"popup_button/default",
	                    LABEL_RENAME,//Change MF_LABEL_DONE to MF_BUTTON_LABEL_OK for fixing(P131029-02752).
	                    NULL,
	                    (Evas_Smart_Cb)mf_callback_rename_save_cb,
	                    rename_save_params,
	                    EINA_FALSE);
	ok_button = btn2;

	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);

	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, mf_popup_new_folder_del_cb, ap);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, rename_cancel_cb, rename_cancel_params);
	elm_object_disabled_set(ok_button, EINA_TRUE);
	evas_object_data_set(popup, "item_data", params);
	elm_entry_cursor_end_set(en);
	return popup;
}

Evas_Object *mf_popup_create_new_folder_popup(void *data, char *context)
{

	struct appdata *ap;

	ap = (struct appdata *)data;
	assert(ap);

	Evas_Object *popup;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	popup = elm_popup_add(ap->mf_MainWindow.pMainLayout);
	elm_object_signal_emit(popup, "elm,action,center_popup,entry", "");

	Evas_Object *layout = mf_popup_entry_layout_create(popup);
	elm_object_content_set(popup, layout);

	Evas_Object *en = NULL;
	en = mf_popup_entry_create(layout);
	mf_object_text_set(en, MF_POP_ENTER_FOLDER_NAME, "elm.guide");
	limit_filter_data.max_char_count = MYFILE_FILE_NAME_CHAR_COUNT_MAX;
	//elm_entry_entry_set(en, text);//Fixed P131029-02752
	//SAFE_FREE_CHAR(text);
	//elm_object_signal_emit(en, "elm,action,hide,search_icon", "");
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size, &limit_filter_data);
	ap->mf_MainWindow.pEntry = en;


	evas_object_smart_callback_add(en, "maxlength,reached", (Evas_Smart_Cb)mf_callback_max_len_reached_cb, ap);
	evas_object_smart_callback_add(en, "changed", (Evas_Smart_Cb)mf_popup_create_folder_imf_changed_cb, ap);
	evas_object_smart_callback_add(en, "preedit,changed", (Evas_Smart_Cb)mf_popup_create_folder_imf_changed_cb/*mf_callback_new_folder_changed_cb*/, ap);
	evas_object_smart_callback_add(en, "activated", (Evas_Smart_Cb)mf_callback_new_folder_save_cb, ap);
	evas_object_show(en);
	elm_entry_input_panel_return_key_type_set(en, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	Evas_Object *btn1 = mf_object_create_button(popup,
	                    NULL, //"popup_button/default",
	                    LABEL_CANCEL,
	                    NULL,
	                    (Evas_Smart_Cb)__mf_popup_new_folder_cancel_cb,
	                    ap,
	                    EINA_FALSE);
	Evas_Object *btn2 = mf_object_create_button(popup,
	                    NULL, //"popup_button/default",
	                    MF_LABEL_CREATE,//Change MF_LABEL_DONE to MF_BUTTON_LABEL_OK for fixing(P131029-02752).
	                    NULL,
	                    (Evas_Smart_Cb)mf_callback_new_folder_save_cb,
	                    ap,
	                    EINA_FALSE);
	ok_button = btn2;

	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);
	mf_object_text_set(popup, context, "title,text");

	elm_object_part_content_set(layout, "elm.swallow.content", en);

	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, mf_popup_new_folder_del_cb, ap);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __mf_popup_new_folder_cancel_cb, ap);
	Eina_Bool entry_empty = elm_entry_is_empty(en);
	if (entry_empty) {
		elm_object_disabled_set(ok_button, EINA_TRUE);
	}
	elm_entry_cursor_end_set(en);
	return popup;
}

/*static Eina_Bool
progressbar_timer_cb(void *data)
{
	Evas_Object *popup = data;
	Evas_Object *progressbar = evas_object_data_get(popup, "progressbar");
	double value = 0.0;

	value = elm_progressbar_value_get(progressbar);
	if (value >= 0.999) {
		//evas_object_data_del(popup, "timer");
		//evas_object_del(popup);
		//return ECORE_CALLBACK_CANCEL;
		value = 0.1;
	}
	value = value + 0.1;
	elm_progressbar_value_set(progressbar, value);

	return ECORE_CALLBACK_RENEW;
}*/

Evas_Object *mf_popup_center_processing(void *data,
                                        const char *context,
                                        const char *first_btn_text,
                                        Evas_Smart_Cb func,
                                        void *param,
                                        Eina_Bool flag_backwork)
{
	mf_error("==================================");
	Evas_Object *popup;
	struct appdata *ap;


	ap = (struct appdata *) data;
	popup = elm_popup_add(ap->mf_MainWindow.pWindow);

	Evas_Object *box = elm_box_add(popup);
	elm_box_horizontal_set(box, EINA_TRUE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_align_set(box, 0.5, 0.5);
	elm_box_padding_set(box, 16, 0);
	//elm_object_part_text_set(popup, "title,text", "Title");

	// [UI] progress icon
	Evas_Object *progressbar = elm_progressbar_add(popup);
	elm_object_style_set(progressbar, "process_large");/* "toolbar_process" or "pending_list" or "list_prosess" */
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_show(progressbar);
	elm_box_pack_end(box, progressbar);
	//mf_progress_bar_timer = ecore_timer_add(0.1, progressbar_timer_cb, popup);

	/*Evas_Object *btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_text_set(btn, "OK");
	elm_object_part_content_set(popup, "button1", btn);*/

	// [UI] text
	Evas_Object *label = elm_label_add(popup);
	//elm_object_text_set(label, context);/* "Loading..." */
	mf_object_text_set(label, context, NULL);
	evas_object_show(label);
	elm_box_pack_end(box, label);


	// [UI] add table
	Evas_Object *table = elm_table_add(popup);
	evas_object_show(table);
	elm_table_homogeneous_set(table, EINA_FALSE);

	Evas_Object *rect_up;// rect as a padding
	rect_up = evas_object_rectangle_add(evas_object_evas_get(popup));
	evas_object_size_hint_min_set(rect_up, ELM_SCALE_SIZE(100), ELM_SCALE_SIZE(20));

	Evas_Object *rect_down;// rect as a padding
	rect_down = evas_object_rectangle_add(evas_object_evas_get(popup));
	evas_object_size_hint_min_set(rect_down, ELM_SCALE_SIZE(100), ELM_SCALE_SIZE(20));

	// box
	elm_table_pack(table, rect_up, 0, 0, 2, 1);// rect as a padding
	elm_table_pack(table, box, 0, 1, 2, 1);
	elm_table_pack(table, rect_down, 0, 2, 2, 1);// rect as a padding
	evas_object_show(box);
	elm_object_content_set(popup, table);
	if (flag_backwork) {
		if (func) {
			eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, func, param);
		} else {
			eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
		}
	}

	//evas_object_data_set(popup, "progressbar", progressbar);
	//evas_object_data_set(popup, "timer", mf_progress_bar_timer);
	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_progress_del_cb, ap);
	return popup;
}

Evas_Object *mf_popup_text(void *data,
                           const char *context, Evas_Object_Event_Cb func,
                           void *param)
{
	mf_error("==================================");
	Evas_Object *popup;
	struct appdata *ap;

	ap = (struct appdata *) data;
	popup = elm_popup_add(ap->mf_MainWindow.pMainLayout);

	Evas_Object *box = elm_box_add(popup);
	elm_box_horizontal_set(box, EINA_TRUE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_align_set(box, 0.5, 0.5);
	elm_box_padding_set(box, 16, 0);

	// [UI] progress icon
	Evas_Object *progressbar = elm_progressbar_add(popup);
	elm_object_style_set(progressbar, "process_large");/* "toolbar_process" or "pending_list" or "list_prosess" */
	//evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	//evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_show(progressbar);
	elm_box_pack_end(box, progressbar);

	// [UI] text
	Evas_Object *label = elm_label_add(popup);
	//elm_object_text_set(label, context);/* "Loading..." */
	mf_object_text_set(label, context, NULL);
	evas_object_show(label);
	elm_box_pack_end(box, label);


	// [UI] add table
	Evas_Object *table = elm_table_add(popup);
	evas_object_show(table);
	elm_table_homogeneous_set(table, EINA_FALSE);

	Evas_Object *rect_up;// rect as a padding
	rect_up = evas_object_rectangle_add(evas_object_evas_get(popup));
	evas_object_size_hint_min_set(rect_up, ELM_SCALE_SIZE(100), ELM_SCALE_SIZE(20));

	Evas_Object *rect_down;// rect as a padding
	rect_down = evas_object_rectangle_add(evas_object_evas_get(popup));
	evas_object_size_hint_min_set(rect_down, ELM_SCALE_SIZE(100), ELM_SCALE_SIZE(20));

	// box
	elm_table_pack(table, rect_up, 0, 0, 2, 1);// rect as a padding
	elm_table_pack(table, box, 0, 1, 2, 1);
	elm_table_pack(table, rect_down, 0, 2, 2, 1);// rect as a padding
	evas_object_show(box);
	elm_object_content_set(popup, table);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);

	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, (Evas_Object_Event_Cb)func, param);
	return popup;
}

/******************************
** Prototype    : mf_popup_create_popup
** Description  :
** Input        : void *data
**                ePopMode popupMode
**                char *title
**                char *context
**                char *first_btn_text
**                char *second_btn_text
**                char *third_btn_text
**                Evas_Smart_Cb func
**                void* param
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
static Evas_Object *__mf_popup_sort_by_box_set(Evas_Object *parent, Evas_Object *content, int item_cnt, bool is_landscape)
{
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	mf_retvm_if(content == NULL, NULL, "content is NULL");
	mf_retvm_if(item_cnt < 0, NULL, "content is NULL");

	Evas_Object *box = elm_box_add(parent);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	// get the rotation
	//int pos = -1;
	//pos = elm_win_rotation_get(mf_get_appdata()->mf_MainWindow.pWindow);

	int max_item_num = item_cnt;
	if (max_item_num > ITEM_MAX_COUNT) {
		max_item_num = ITEM_MAX_COUNT;
	}
	if (is_landscape) {
		if (max_item_num > ITEM_MAX_COUNT_LANDSCAPE) {
			max_item_num = ITEM_MAX_COUNT_LANDSCAPE;
		}
	}
	evas_object_size_hint_min_set(box, -1,
	                              ELM_SCALE_SIZE(MF_POPUP_MENUSTYLE_HEIGHT(max_item_num)));

	evas_object_show(content);
	elm_box_pack_end(box, content);
	return box;
}
static Evas_Object *__mf_popup_box_set(Evas_Object *parent, Evas_Object *content, int item_cnt)
{
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	mf_retvm_if(content == NULL, NULL, "content is NULL");
	mf_retvm_if(item_cnt < 0, NULL, "content is NULL");

	Evas_Object *box = elm_box_add(parent);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	int max_item_num = item_cnt;
	if (max_item_num > ITEM_MAX_COUNT) {
		max_item_num = ITEM_MAX_COUNT;
	}
	evas_object_size_hint_min_set(box, ELM_SCALE_SIZE(MF_POPUP_MENUSTYLE_WIDTH),
	                              ELM_SCALE_SIZE(MF_POPUP_MENUSTYLE_HEIGHT(max_item_num)));

	evas_object_show(content);
	elm_box_pack_end(box, content);
	return box;
}

#if 1
int g_popup_item_index = 0;

void
mf_elm_popup_item_append(Evas_Object *obj,
                         const char *label,
                         Evas_Object *icon,
                         Evas_Smart_Cb func,
                         const void *data)
{
	struct appdata * ap = (struct appdata *) mf_get_appdata();
	Elm_Object_Item *it = NULL;
	ListByData_s *item_data = calloc(sizeof(ListByData_s), sizeof(char));
	if (item_data != NULL) {
		memset(item_data, 0x00, sizeof(ListByData_s));
		item_data->index = g_popup_item_index;
		item_data->ap = ap;
		item_data->title = g_strdup(label);
		it = elm_genlist_item_append(obj, ap->mf_gl_style.popup_itc, (void *)item_data, NULL,
		                             ELM_GENLIST_ITEM_NONE, func, data);

		item_data->item = it;

		g_popup_item_index++;
	}
}

static void __mf_popup_operation_item_create(Evas_Object *genlist, void *data, mf_operation_item_type_e type)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	switch (type) {
	case mf_operation_item_rename:
		mf_elm_popup_item_append(genlist, mf_util_get_text(LABEL_RENAME), NULL, mf_callback_longpress_rename_cb, data);
		break;
	case mf_operation_item_details:
		mf_elm_popup_item_append(genlist, mf_util_get_text(LABEL_DETAIL), NULL, mf_callback_detail_button_cb, data);
		break;
	case mf_operation_item_goto_folder:
		mf_elm_popup_item_append(genlist, mf_util_get_text(MF_LABEL_GOTO_FOLDER), NULL, mf_callback_entry_cb, data);
		break;
	case mf_operation_item_share:
		mf_elm_popup_item_append(genlist, mf_util_get_text(LABEL_SHARE), NULL, mf_launch_item_share, data);
		break;
	case mf_operation_item_copy:
		mf_elm_popup_item_append(genlist, mf_util_get_text(LABEL_COPY), NULL, mf_callback_item_copy_cb, data);
		break;
	case mf_operation_item_move:
		mf_elm_popup_item_append(genlist, mf_util_get_text(LABEL_MOVE), NULL, mf_callback_item_move_cb, data);
		break;
	case mf_operation_item_delete:
		mf_elm_popup_item_append(genlist, mf_util_get_text(LABEL_DELETE), NULL, mf_callback_delete_button_cb, data);
		break;
	case mf_operation_item_download:
		mf_elm_popup_item_append(genlist, mf_util_get_text(MF_LABEL_DOWNLOAD), NULL, NULL, data);
		break;
	case mf_operation_item_remove_recent:
		mf_elm_popup_item_append(genlist, mf_util_get_text(MF_LABEL_REMOVE), NULL, mf_callback_item_remove_from_recent_files_cb, data);
		break;

	default:
		break;
	}
}

static void __mf_popup_operation_items_generate(void *data, Evas_Object *genlist, int type)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_list_data_t * item_data = (mf_list_data_t *)data;
	struct appdata *ap = item_data->ap;
	mf_error("ap->mf_Status.view_type  is [%d] type is [%d]", ap->mf_Status.view_type, type);

	if (ap->mf_Status.more == MORE_SEARCH) {
		{
			if (item_data->file_type == FILE_TYPE_DIR) {
				//1 Delete
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_delete);

				//1 Rename
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_rename);

				//1 Move
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_move);

				//1 Copy
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_copy);

				//1 Details
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_details);
			} else {
				//1 Go to folder
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_goto_folder);

				//1 Share
				{
					__mf_popup_operation_item_create(genlist, data, mf_operation_item_share);
				}


				//1 Delete
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_delete);

				//1 Rename
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_rename);

				//1 Move
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_move);

				//1 Copy
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_copy);

				//1 Details
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_details);
			}
		}
	} else if (ap->mf_Status.view_type == mf_view_root) {
		switch (type) {
		case mf_list_recent_files:
			//1 Go to folder
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_goto_folder);

			//1 Share
			{
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_share);
			}

			//1 Remove
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_remove_recent);

			//1 Details
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_details);

			break;
		case mf_list_normal:
			//1 Details
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_details);
			break;
		default:
			break;
		}
	} else if (ap->mf_Status.view_type == mf_view_root_category) {
		{
			//1 Go to folder
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_goto_folder);

			//1 Share
			{
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_share);
			}


			//1 Delete
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_delete);

			//1 Rename
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_rename);

			//1 Move
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_move);

			//1 Copy
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_copy);

			//1 Details
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_details);
		}

	} else {
		if (item_data->file_type == FILE_TYPE_DIR) {

			//1 Delete
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_delete);

			//1 Rename
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_rename);

			//1 Move
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_move);

			//1 Copy
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_copy);

			//1 Compress
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_compress);

			//1 Details
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_details);
		} else {
			//1 Share
			{
				__mf_popup_operation_item_create(genlist, data, mf_operation_item_share);
			}


			//1 Delete
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_delete);

			//1 Rename
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_rename);

			//1 Move
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_move);

			//1 Copy
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_copy);

			//1 Details
			__mf_popup_operation_item_create(genlist, data, mf_operation_item_details);
		}
	}
	MF_TRACE_END;
}

static void __mf_popup_block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	evas_object_del(obj);
	MF_TRACE_END;
}

static char *__mf_popup_genlist_label_get(void *data, Evas_Object * obj, const char *part)
{
	ListByData_s *params = (ListByData_s *) data;
	assert(params);
	struct appdata *ap = params->ap;
	assert(ap);
	assert(part);

	//eMfViewStyle view_as_type = MF_VIEW_STYLE_LIST;

	//char *ret = NULL;
	if (!strcmp(part, "elm.text.main.left")) {

		return strdup(params->title);;
	}
	return NULL;
}


static Evas_Object *__mf_popup_genlist_icon_get(void *data, Evas_Object * obj, const char *part)
{
	ListByData_s *params = (ListByData_s *) data;
	assert(params);
	struct appdata *ap = params->ap;
	assert(ap);
	assert(part);

	if (!strcmp(part, "elm.icon")) {
		Evas_Object *radio = NULL;
		radio = elm_radio_add(obj);
		elm_object_focus_set(radio, EINA_FALSE);
		elm_object_focus_allow_set(radio, EINA_FALSE);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		elm_radio_state_value_set(radio, params->index);
		elm_radio_group_add(radio, ap->mf_Status.pRadioGroup);
		elm_radio_value_set(radio, ap->mf_Status.iRadioValue);

		/* elm_object_signal_callback_add(radio, "elm,action,show,finished", "elm", */
		/* __gl_popup_viewby_radio_cb, params); */

		evas_object_show(radio);
		return radio;
	}
	return NULL;
}

Evas_Object *mf_popup_create_operation_item_pop(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	Evas_Object *popup = NULL;
	mf_list_data_t *item_data = NULL;

	item_data = (mf_list_data_t *)data;
	struct appdata *ap = item_data->ap;

	char *title = NULL;

	if (item_data->list_type == mf_list_normal
	        || item_data->list_type == mf_list_recent_files) {
		if (item_data->storage_type == MYFILE_PHONE
		        || item_data->storage_type == MYFILE_MMC
		   ) {
			if (!mf_file_exists(((mfItemData_s *)item_data)->m_ItemName->str)) {
				mf_popup_indicator_popup(NULL, mf_util_get_text(MF_LABEL_FILE_NOT_EXIST));
				elm_object_item_del(item_data->item);
				return NULL;
			}
		}
		if (g_strcmp0(((mfItemData_s *)item_data)->m_ItemName->str, PHONE_FOLDER) == 0) {
			title = g_strdup(MF_LABEL_DEVICE_MEMORY);
		} else if (g_strcmp0(((mfItemData_s *)item_data)->m_ItemName->str, MEMORY_FOLDER) == 0) {
			title = g_strdup(MF_LABEL_SD_CARD);
		} else {
			char *temp_utf8 = elm_entry_utf8_to_markup(mf_file_get(((mfItemData_s *)item_data)->m_ItemName->str));//Fixed P140321-05456 by jian12.li
			if (temp_utf8) {
				title = g_strdup(temp_utf8);
				SAFE_FREE_CHAR(temp_utf8);
				temp_utf8 = NULL;
			}
		}
	} else {
		SAFE_FREE_OBJ(popup);
		return NULL;
	}

	popup = elm_popup_add(ap->mf_MainWindow.pMainLayout);
	if (popup == NULL) {
		mf_error("popup is NULL");
		SAFE_FREE_CHAR(title);
		return NULL;
	}

	elm_object_style_set(popup, "indicator_norepeatevent");
	elm_object_focus_set(popup, EINA_FALSE);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title) {
		mf_object_text_set(popup, title, "title,text");//Fixed P140321-05456 by jian12.li
		/*char *temp_utf8 = elm_entry_utf8_to_markup(title);
		  if (temp_utf8) {
		  elm_object_part_text_set(popup, "title,text", temp_utf8);
		  free(temp_utf8);
		  }*/
		SAFE_FREE_CHAR(title);
	}

	//elm_object_style_set(popup,"content_no_vhpad");
	mf_genlist_item_class_free(ap->mf_gl_style.popup_itc);

	ap->mf_gl_style.popup_itc = elm_genlist_item_class_new();
	if (ap->mf_gl_style.popup_itc) {
		ap->mf_gl_style.popup_itc->item_style = "1line";
		ap->mf_gl_style.popup_itc->decorate_all_item_style = NULL;
		ap->mf_gl_style.popup_itc->decorate_item_style = NULL;
		ap->mf_gl_style.popup_itc->func.text_get = __mf_popup_genlist_label_get;
		ap->mf_gl_style.popup_itc->func.content_get = __mf_popup_genlist_icon_get;
		ap->mf_gl_style.popup_itc->func.del	    = NULL;
	}
	if (ap->mf_Status.pRadioGroup) {
		evas_object_del(ap->mf_Status.pRadioGroup);
		ap->mf_Status.pRadioGroup = NULL;
	}

	Evas_Object *radio_group = elm_radio_add(ap->mf_MainWindow.pWindow);
	elm_object_focus_set(radio_group, EINA_FALSE);
	elm_radio_value_set(radio_group, 0);
	evas_object_hide(radio_group);
	ap->mf_Status.pRadioGroup = radio_group;

	int listby_selected = 0;
	ap->mf_Status.iRadioValue = listby_selected;

	Evas_Object * genlist = elm_genlist_add(popup);
	//elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_object_focus_set(genlist, EINA_FALSE);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	g_popup_item_index = 0;

	__mf_popup_operation_items_generate(item_data, genlist, item_data->list_type);
	evas_object_smart_callback_add(genlist, "language,changed", mf_genlist_gl_lang_changed, item_data);

	Evas_Object * box = __mf_popup_box_set(popup, genlist, g_popup_item_index);
	//elm_object_part_content_set(layout, "elm.swallow.content" , box);
	elm_object_content_set(popup, box);
	//evas_object_smart_callback_add(popup, "response", func, param);
	//evas_object_smart_callback_add(popup, "block,clicked", (Evas_Smart_Cb)__mf_popup_block_clicked_cb, NULL);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, (Evas_Object_Event_Cb)__mf_popup_longpress_del_cb, ap);
	evas_object_smart_callback_add(popup, "block,clicked", (Evas_Smart_Cb)__mf_popup_block_clicked_cb, NULL);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	mf_error("popup is [%p]", popup);

	if (ap->mf_MainWindow.pNaviGenlist) {
		evas_object_data_set(ap->mf_MainWindow.pNaviGenlist, "popup", popup); // Set popup data when popup is created.
		elm_object_scroll_freeze_push(ap->mf_MainWindow.pNaviGenlist);	  // If you want to disable scrolling (elm_object_scroll_hold_push is not needed)
	}
	evas_object_show(popup);
	evas_object_data_set(popup, "item_data", data);

	return popup;
}
#endif
Evas_Object *mf_popup_share_as_video_or_image(void *func_for_video, void *func_for_image, void *data)
{
	MF_TRACE_BEGIN
	mfItemData_s *item_data = (mfItemData_s *)data;
	mf_retvm_if(item_data == NULL, NULL, "input data is NULL");
	struct appdata *ap = (struct appdata *)item_data->ap;
	mf_retvm_if(ap == NULL, NULL, "input ap is NULL");

	Evas_Object *popup = NULL;
	popup = elm_popup_add(ap->mf_MainWindow.pWindow);
	mf_retvm_if(popup == NULL, NULL, "popup is NULL");
	elm_object_style_set(popup, "indicator_norepeatevent");
	elm_object_focus_set(popup, EINA_FALSE);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", mf_util_get_text(MF_LABEL_SHARE_SOUND_AND_SHOT_PICS));

	elm_popup_item_append(popup, mf_util_get_text(MF_LABEL_SHARE_AS_VIDEO_FILES), NULL, func_for_video, data);
	elm_popup_item_append(popup, mf_util_get_text(MF_LABEL_SHARE_AS_IMAGE_FILES) , NULL, func_for_image, data);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, (Evas_Object_Event_Cb)__mf_popup_longpress_del_cb, ap);
	evas_object_smart_callback_add(popup, "block,clicked", (Evas_Smart_Cb)__mf_popup_block_clicked_cb, NULL);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	//evas_object_smart_callback_add(popup, "popup,selected", func, (void *)data);

	mf_info("popup is [%p]", popup);
	evas_object_show(popup);
	MF_TRACE_END;
	return popup;
}

static void __mf_popup_listby_gl_style_set(void *data, int type)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	mf_genlist_item_class_free(ap->mf_gl_style.listby_itc);

	ap->mf_gl_style.listby_itc = elm_genlist_item_class_new();
	if (type == POPMODE_VIEW_AS_LIST) {
		if (ap->mf_gl_style.listby_itc) {
			ap->mf_gl_style.listby_itc->item_style = "type1";
			ap->mf_gl_style.listby_itc->decorate_all_item_style = NULL;
			ap->mf_gl_style.listby_itc->decorate_item_style = NULL;
			ap->mf_gl_style.listby_itc->func.text_get = __mf_popup_view_as_genlist_label_get;
			ap->mf_gl_style.listby_itc->func.content_get = __mf_popup_view_as_genlist_icon_get;
			ap->mf_gl_style.listby_itc->func.del	    = NULL;
		}

	}
}

void mf_ea_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	if (obj) {
		eext_popup_back_cb(data, obj, event_info);
	}
	if (data) {
		Evas_Smart_Cb func = data;
		struct appdata * app_data = mf_get_appdata();
		func(app_data, obj, event_info);
	}
	MF_TRACE_END;
}

void mf_popup_del_by_timeout(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	assert(ap);
	if (ap->mf_MainWindow.pNormalPopup) {
		//Add the protection
		evas_object_del(ap->mf_MainWindow.pNormalPopup);
		ap->mf_MainWindow.pNormalPopup = NULL;
	}
	if (ap->mf_MainWindow.pWindow) {
		elm_object_focus_set(ap->mf_MainWindow.pWindow, EINA_TRUE);
	}
	MF_TRACE_END;

}

/*static void __mf_sortby_popup_rotate_cb(void *data, Evas_Object *obj, void *ei)
{
	MF_TRACE_BEGIN;
	mf_retm_if(!data, "data is NULL");

	if (data) {
		Evas_Object *box = (Evas_Object *)data;
		Evas_Object * pWin = mf_get_appdata()->mf_MainWindow.pWindow;
		// get the rotation
		int pos = -1;
		//Evas_Coord x = 0;
		//Evas_Coord y = 0;
		pos = elm_win_rotation_get(pWin);
		switch (pos) {
			case 0:
			case 180: //portait
			{
				if (ITEM_COUNT > ITEM_MAX_COUNT) {
					evas_object_size_hint_min_set(box, MF_POPUP_MENUSTYLE_WIDTH,
							      MF_POPUP_MENUSTYLE_HEIGHT(ITEM_MAX_COUNT));
				} else {
					evas_object_size_hint_min_set(box, MF_POPUP_MENUSTYLE_WIDTH,
								      MF_POPUP_MENUSTYLE_HEIGHT(ITEM_COUNT));
				}
				break;
			}
			case 90:
			case 270:
			{
				evas_object_size_hint_min_set(box, MF_POPUP_MENUSTYLE_WIDTH,
								      MF_POPUP_MENUSTYLE_HEIGHT(5));
				break;
			}
		}
	}
	MF_TRACE_END;
}*/

Evas_Object *mf_popup_create_popup(void *data, ePopMode popupMode, char *title, const char *context, const char *first_btn_text, const char *second_btn_text,
                                   const char *third_btn_text, Evas_Smart_Cb func, void *param)
{
	MF_TRACE_BEGIN;
	Evas_Object *popup = NULL;
	struct appdata *ap = NULL;
	Evas_Object *genlist = NULL;
	Evas_Object *radio_group = NULL;
	Evas_Object *box = NULL;
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	int index = 0;
	int listby_selected = 0;
	ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");

	popup = elm_popup_add(ap->mf_MainWindow.pWindow);
	//elm_object_signal_emit(popup, "elm,action,center_popup,entry", "");

	mf_retvm_if(popup == NULL, NULL, "popup is NULL");
//	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	elm_object_focus_set(popup, EINA_FALSE);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if (title) {
		mf_object_text_set(popup, title, "title,text");
	}
	if (context && popupMode != POPMODE_PROGRESSBAR) {
		/* if it is a delete popup. */
		mf_object_text_set(popup, context, NULL);
	}

	switch (popupMode) {
	case POPMODE_TEXT:
	case POPMODE_TITLE_TEXT:
		elm_popup_timeout_set(popup, 3);
		if (func != NULL) {
			evas_object_smart_callback_add(popup, "timeout", (Evas_Smart_Cb)func, param);
		} else {
			evas_object_smart_callback_add(popup, "timeout", (Evas_Smart_Cb)mf_popup_del_by_timeout, ap);
		}
		break;
	case POPMODE_TEXT_NOT_DISABLED:
		if (func != NULL) {
			evas_object_smart_callback_add(popup, "timeout", (Evas_Smart_Cb)func, param);
		}
		break;

	case POPMODE_TEXT_TWO_BTN:
	case POPMODE_TITLE_TEXT_TWO_BTN:
		btn1 = mf_object_create_button(popup,
		                               NULL, //"popup_button/default",
		                               first_btn_text,
		                               NULL,
		                               (Evas_Smart_Cb)func,
		                               param,
		                               EINA_FALSE);
		btn2 = mf_object_create_button(popup,
		                               NULL, //"popup_button/default",
		                               second_btn_text,
		                               NULL,
		                               (Evas_Smart_Cb)func,
		                               param,
		                               EINA_FALSE);
		elm_object_part_content_set(popup, "button1", btn1);
		elm_object_part_content_set(popup, "button2", btn2);
		break;

	case POPMODE_TEXT_BTN:
	case POPMODE_TITLE_TEXT_BTN:
		btn1 = mf_object_create_button(popup,
		                               NULL, //"popup_button/default",
		                               first_btn_text,
		                               NULL,
		                               (Evas_Smart_Cb)func,
		                               param,
		                               EINA_FALSE);
		elm_object_focus_set(btn1, EINA_TRUE);
		elm_object_part_content_set(popup, "button1", btn1);
		break;

	case POPMODE_VIEW_AS_LIST:

		//elm_object_style_set(popup,"content_no_vhpad");
		//layout = elm_layout_add(popup);
		//elm_layout_theme_set(layout, "layout", "content", "min_menustyle");
		//evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		if (ap->mf_Status.pRadioGroup) {
			evas_object_del(ap->mf_Status.pRadioGroup);
			ap->mf_Status.pRadioGroup = NULL;
		}

		radio_group = elm_radio_add(ap->mf_MainWindow.pWindow);
		elm_object_focus_set(radio_group, EINA_FALSE);
		elm_radio_value_set(radio_group, 0);
		evas_object_hide(radio_group);
		ap->mf_Status.pRadioGroup = radio_group;

		listby_selected = __mf_popup_get_view_as_selected_item();
		ap->mf_Status.iRadioValue = listby_selected;

		__mf_popup_listby_gl_style_set(ap, popupMode);
		genlist = elm_genlist_add(popup);
		//elm_genlist_homogeneous_set(genlist, EINA_TRUE);
		elm_object_focus_set(genlist, EINA_FALSE);
		evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		for (index = 0; index < 2; index++) {
			Elm_Object_Item *it = NULL;
			ListByData_s *item_data = calloc(sizeof(ListByData_s), sizeof(char));
			if (item_data == NULL) {
				continue;
			}
			memset(item_data, 0x00, sizeof(ListByData_s));
			item_data->index = index;
			item_data->ap = ap;
			it = elm_genlist_item_append(genlist, ap->mf_gl_style.listby_itc, (void *)item_data, NULL,
			                             ELM_GENLIST_ITEM_NONE, __mf_popup_view_as_genlist_select, popup);
			item_data->item = it;
			evas_object_smart_callback_add(genlist, "language,changed", mf_genlist_gl_lang_changed, item_data);
		}

		box = __mf_popup_box_set(popup, genlist, 2);
		//elm_object_part_content_set(layout, "elm.swallow.content" , box);
		elm_object_content_set(popup, box);
		evas_object_smart_callback_add(popup, "response", func, param);
		evas_object_smart_callback_add(popup, "block,clicked", (Evas_Smart_Cb)__mf_popup_block_clicked_cb, NULL);
	default:
		break;
	}
	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_normal_del_cb, ap);
	if (func && (func == mf_callback_illegal_char_popup_cb || func == mf_popup_show_vk_cb)) {//Fixed P131031-00367, there is the side effect, only when the focus issue,will call the func whck back key.
		//Fixed the bug(P131011-02665), when pressing the back, no focus at the entry.
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, func, param);
	} else {
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	}
	MF_TRACE_END;
	return popup;
}

Evas_Object *mf_popup_create_delete_confirm_popup(void *data, char *title, const char *context, const char *first_btn_text, const char *second_btn_text, Evas_Smart_Cb func, void *param, int count)
{
	MF_TRACE_BEGIN;
	Evas_Object *popup = NULL;
	struct appdata *ap = NULL;
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");

	popup = elm_popup_add(ap->mf_MainWindow.pWindow);
	//elm_object_signal_emit(popup, "elm,action,center_popup,entry", "");

	mf_retvm_if(popup == NULL, NULL, "popup is NULL");
	elm_object_focus_set(popup, EINA_FALSE);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if (title) {
		mf_object_text_set(popup, title, "title,text");
	}
	if (context) {
		if (count > 1) {
			char *tmp = NULL;
			tmp = mf_util_get_text(context);
			char *label = g_strdup_printf(tmp, count);
			mf_object_text_set(popup, label, NULL);
			SAFE_FREE_CHAR(label);
		} else {
			mf_object_text_set(popup, context, NULL);
		}
	}

	btn1 = mf_object_create_button(popup,
	                               NULL, //"popup_button/default",
	                               first_btn_text,
	                               NULL,
	                               (Evas_Smart_Cb)func,
	                               param,
	                               EINA_FALSE);
	btn2 = mf_object_create_button(popup,
	                               NULL, //"popup_button/default",
	                               second_btn_text,
	                               NULL,
	                               (Evas_Smart_Cb)func,
	                               param,
	                               EINA_FALSE);
	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_normal_del_cb, ap);

	if (func && (func == mf_callback_illegal_char_popup_cb || func == mf_popup_show_vk_cb)) {//Fixed P131031-00367, there is the side effect, only when the focus issue,will call the func whck back key.
		//Fixed the bug(P131011-02665), when pressing the back, no focus at the entry.
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, func, param);
	} else {
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	}
	MF_TRACE_END;
	return popup;
}

Evas_Object *mf_popup_warning_popup_create(void *data, Evas_Object *parent, char *title, const char *context, const char *btn_text, Evas_Smart_Cb func, void *param)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = NULL;
	Evas_Object *btn1 = NULL;
	ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");

	SAFE_FREE_OBJ(second_popup);
	second_popup = elm_popup_add(ap->mf_MainWindow.pWindow);
	mf_retvm_if(second_popup == NULL, NULL, "popup is NULL");
	evas_object_size_hint_weight_set(second_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title) {
		mf_object_text_set(second_popup, title, "title,text");
	}

	if (context) {
		mf_object_text_set(second_popup, context, NULL);
	}

	btn1 = mf_object_create_button(second_popup,
	                               NULL, //"popup_button/default",
	                               btn_text,
	                               NULL,
	                               (Evas_Smart_Cb)func,
	                               param,
	                               EINA_FALSE);
	elm_object_part_content_set(second_popup, "button1", btn1);
	evas_object_show(second_popup);
	evas_object_event_callback_add(second_popup, EVAS_CALLBACK_DEL, __mf_popup_second_popup_del_cb, ap);

	if (func && (func == mf_callback_illegal_char_popup_cb || func == mf_popup_show_vk_cb)) {//Fixed P131031-00367, there is the side effect, only when the focus issue,will call the func whck back key.
		//Fixed the bug(P131011-02665), when pressing the back, no focus at the entry.
		eext_object_event_callback_add(second_popup, EEXT_CALLBACK_BACK, func, param);
	} else {
		eext_object_event_callback_add(second_popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	}
	MF_TRACE_END;
	return second_popup;
}


Evas_Object *mf_popup_second_popup_create(void *data, Evas_Object *parent, const char *context, const char *btn_text, Evas_Smart_Cb func, void *param)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = NULL;
	Evas_Object *btn1 = NULL;
	ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");

	SAFE_FREE_OBJ(second_popup);
	second_popup = elm_popup_add(ap->mf_MainWindow.pWindow);

	mf_retvm_if(second_popup == NULL, NULL, "popup is NULL");
	elm_object_focus_set(second_popup, EINA_FALSE);
	evas_object_size_hint_weight_set(second_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (context) {
		mf_object_text_set(second_popup, context, NULL);
	}

	btn1 = mf_object_create_button(second_popup,
	                               NULL, //"popup_button/default",
	                               btn_text,
	                               NULL,
	                               (Evas_Smart_Cb)func,
	                               param,
	                               EINA_FALSE);
	elm_object_focus_set(btn1, EINA_TRUE);
	elm_object_part_content_set(second_popup, "button1", btn1);
	evas_object_show(second_popup);
	evas_object_event_callback_add(second_popup, EVAS_CALLBACK_DEL, __mf_popup_second_popup_del_cb, ap);

	if (func && (func == mf_callback_illegal_char_popup_cb || func == mf_popup_show_vk_cb)) {//Fixed P131031-00367, there is the side effect, only when the focus issue,will call the func whck back key.
		//Fixed the bug(P131011-02665), when pressing the back, no focus at the entry.
		eext_object_event_callback_add(second_popup, EEXT_CALLBACK_BACK, func, param);
	} else {
		eext_object_event_callback_add(second_popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	}
	MF_TRACE_END;
	return second_popup;
}

void mf_popup_indicator_popup(void *data, const char *text)
{
	mf_retm_if(text == NULL, "text is NULL");
	char *notification = mf_util_get_text(text);
	int ret = notification_status_message_post(notification);
	if (notification != NULL) {
		mf_debug("indicator popup message : %s", notification);
	}

	mf_debug("notification_status_message_post()... [0x%x]!", ret);
	if (ret != 0) {
		mf_debug("status_message_post()... [0x%x]!", ret);
	}
	return ;
}

static Eina_Bool mf_check_view_show_flag = EINA_TRUE;

Eina_Bool mf_popup_check_view_flag_get()
{
	return mf_check_view_show_flag;
}

void mf_popup_check_view_flag_set(Eina_Bool state)
{
	mf_check_view_show_flag = state;
}

static void _go_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Eina_Bool state = elm_check_state_get(obj);
	if (state) {
		elm_object_signal_emit(obj, "elm,activate,check,off", "elm");
	} else {
		elm_object_signal_emit(obj, "elm,activate,check,on", "elm");
	}

	mf_check_view_show_flag = !state;
}

Evas_Object *mf_popup_check_view_popup(void *data,
                                       const char *title,
                                       const char *text,
                                       const char *check_text,
                                       const char *first_btn_text,
                                       const char *second_btn_text,
                                       Evas_Smart_Cb func,
                                       void *param)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *scroller;
	Evas_Object *label;
	Evas_Object *check;

	struct appdata *ap = NULL;
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");

	popup = elm_popup_add(ap->mf_MainWindow.pWindow);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if (title) {
		mf_object_text_set(popup, title, "title,text");
	}
	/* layout */
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", EDJ_NAME);
	free(path);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, edj_path, "popup_checkview_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(layout, "elm.text", "Description text");

	/* ok button */
	/*btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_text_set(btn, "OK");
	elm_object_part_content_set(popup, "button1", btn);*/
//	evas_object_smart_callback_add(btn, "clicked", popup_btn_clicked_cb, popup);

	/* check */
	check = elm_check_add(popup);
	elm_object_style_set(check, "popup");
	mf_object_text_set(check, check_text, NULL);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "elm.swallow.end", check);
	evas_object_smart_callback_add(check, "changed", _go_check_clicked_cb, NULL);

	/* scroller */
	scroller = elm_scroller_add(layout);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_part_content_set(layout, "elm.swallow.content", scroller);

	/* label */
	label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	mf_object_text_set(label, text, NULL);
	//elm_object_text_set(label, "This is popup description text. To provide information about popup, it is scrollable text.");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);
	elm_object_content_set(scroller, label);

	elm_object_content_set(popup, layout);

	evas_object_show(popup);
	if (first_btn_text && func) {
		btn1 = mf_object_create_button(popup,
		                               "popup_button/default",
		                               first_btn_text,
		                               NULL,
		                               (Evas_Smart_Cb)func,
		                               param,
		                               EINA_FALSE);
		elm_object_part_content_set(popup, "button1", btn1);
	}
	if (second_btn_text && func) {
		btn2 = mf_object_create_button(popup,
		                               "popup_button/default",
		                               second_btn_text,
		                               NULL,
		                               (Evas_Smart_Cb)func,
		                               param,
		                               EINA_FALSE);
		elm_object_part_content_set(popup, "button2", btn2);
	}
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_normal_del_cb, ap);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	evas_object_show(popup);
	return popup;
}

void
__mf_popup_view_as_genlist_change(ListByData_s *params)
{
	if (params == NULL) {
		return;
	}

	elm_radio_value_set(params->ap->mf_Status.pRadioGroup, params->index);
	int iViewAsType = __mf_popup_get_view_as_type(params->index);
	params->ap->mf_Status.flagViewType = iViewAsType;
	mf_util_set_view_style(iViewAsType);

	SAFE_TIMER_DEL(g_popup_timer);
	g_popup_timer = ecore_timer_add(RADIO_POPUP_TIMEOUT, __mf_popup_view_as_response_cb, params->ap);//Fixed P131207-01294
}


//1  Sort by popup

static Evas_Smart_Cb sort_by_respones_func = NULL;

void mf_sort_by_respones_func_set(Evas_Smart_Cb func)
{
	sort_by_respones_func = func;
}

static int __mf_sort_by_type_get(int sort_type)
{
	int ret = 0;
	switch (sort_type) {
	case	MYFILE_SORT_BY_DATE_O2R:
	case	MYFILE_SORT_BY_DATE_R2O:
		ret = 0;
		break;
	case	MYFILE_SORT_BY_TYPE_A2Z:
	case	MYFILE_SORT_BY_TYPE_Z2A:
		ret = 1;
		break;
	case	MYFILE_SORT_BY_NAME_A2Z:
	case	MYFILE_SORT_BY_NAME_Z2A:
		ret = 2;
		break;
	case	MYFILE_SORT_BY_SIZE_L2S:
	case	MYFILE_SORT_BY_SIZE_S2L:
		ret = 3;
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}

static int __mf_sort_by_order_get(int sort_type)
{
	int ret = 0;
	switch (sort_type) {
	case	MYFILE_SORT_BY_TYPE_A2Z:
	case	MYFILE_SORT_BY_NAME_A2Z:
	case	MYFILE_SORT_BY_SIZE_S2L:
	case	MYFILE_SORT_BY_DATE_O2R:
		ret = 4;
		break;
	case	MYFILE_SORT_BY_TYPE_Z2A:
	case	MYFILE_SORT_BY_NAME_Z2A:
	case	MYFILE_SORT_BY_SIZE_L2S:
	case	MYFILE_SORT_BY_DATE_R2O:
		ret = 5;
		break;
	default:
		ret = 4;
		break;
	}
	return ret;
}

static int __mf_sort_by_value_get(int sort_type, int order_type)
{
	int value = MYFILE_SORT_BY_NAME_A2Z;
	if (order_type == 4) {
		switch (sort_type) {
		case 0:
			value = MYFILE_SORT_BY_DATE_O2R;
			break;
		case 1:
			value = MYFILE_SORT_BY_TYPE_A2Z;
			break;
		case 2:
			value = MYFILE_SORT_BY_NAME_A2Z;
			break;
		case 3:
			value = MYFILE_SORT_BY_SIZE_S2L;
			break;
		default:
			break;
		}
	} else {
		switch (sort_type) {
		case 0:
			value = MYFILE_SORT_BY_DATE_R2O;
			break;
		case 1:
			value = MYFILE_SORT_BY_TYPE_Z2A;
			break;
		case 2:
			value = MYFILE_SORT_BY_NAME_Z2A;
			break;
		case 3:
			value = MYFILE_SORT_BY_SIZE_L2S;
			break;
		default:
			break;
		}
	}
	return value;
}
static Evas_Object *__mf_sort_by_gl_icon(void *data, Evas_Object * obj, const char *part)
{
	int index = (int) data;
	struct appdata *ap = mf_get_appdata();
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");

	if (!strcmp(part, "elm.swallow.end")) {
		Evas_Object *radio = NULL;
		radio = elm_radio_add(obj);
		elm_object_focus_set(radio, EINA_FALSE);
		elm_radio_state_value_set(radio, index);
		if (index < 4) {
			elm_radio_group_add(radio, sort_group_radio);
			elm_radio_value_set(radio, sort_type_index);
		} else {
			elm_radio_group_add(radio, order_group_radio);
			elm_radio_value_set(radio, order_index);
		}
		elm_object_signal_emit(radio, "elm,event,pass,enabled", "elm");
		evas_object_size_hint_align_set(radio,
										EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(radio,
										 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(radio);
		return radio;
	}
	return NULL;
}

static char *__mf_sort_by_gl_label(void *data, Evas_Object * obj, const char *part)
{
	int index = (int) data;
	struct appdata *ap = mf_get_appdata();
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");
	mf_error("index is [%d]", index);
	char *ret = NULL;
	if (!strcmp(part, "elm.text")) {
		switch (index) {
		case 0:
			ret = g_strdup(mf_util_get_text(MF_LABEL_TIME));
			break;
		case 1:
			ret = g_strdup(mf_util_get_text(MF_LABEL_TYPE));
			break;
		case 2:
			ret = g_strdup(mf_util_get_text(MF_LABEL_NAME));
			break;
		case 3:
			ret = g_strdup(mf_util_get_text(MF_LABEL_SIZE));
			break;
		case 4:
			ret = g_strdup(mf_util_get_text(MF_LABEL_ASCENDING));
			break;
		case 5:
			ret = g_strdup(mf_util_get_text(MF_LABEL_DESCENDING));
			break;
		default:
			break;
		}
		return ret;
	}
	return NULL;
}

char *mf_sort_by_order_label_get(void *data, Evas_Object * obj, const char *part)
{
	mf_error("part=%s", part);
	if (strcmp(part, "elm.text") == 0) {
		return g_strdup(mf_util_get_text(MF_LABEL_ORDER));
	}
	return g_strdup(_(""));
}

void __mf_sort_by_response_cb(int index)
{
	struct appdata *ap = mf_get_appdata();
	mf_debug("sort genlist");
	//fsSortOption iListBySortType = MYFILE_SORT_BY_NONE;
	ap->mf_Status.iSelectedSortType = index;
	mf_util_set_sort_type(index);
	sort_by_respones_func(ap, NULL, NULL);
}

static void __mf_sort_by_gl_select(void *data, Evas_Object * obj, void *event_info)
{
	int index = (int) data;
	struct appdata *ap = mf_get_appdata();
	mf_retm_if(ap == NULL, "ap is NULL");
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	mf_error("index is [%d]", index);
	if (item != NULL) {
		elm_genlist_item_selected_set(item, FALSE);
	}
	if (index < 4) {
		sort_type_index = index;
		if (elm_radio_value_get(sort_group_radio) != index) {
			elm_radio_value_set(sort_group_radio, index);
		}

		//Fixed P140826-07892.
		mf_debug("sort_type_index is [%d] order_index is [%d]", sort_type_index, order_index);
		g_sort_type = __mf_sort_by_value_get(sort_type_index, order_index);
		mf_debug("g_sort_type is [%d]", g_sort_type);
		//__mf_sort_by_response_cb(value);
		//End P140826-07892.
	} else {
		order_index = index;
		if (elm_radio_value_get(order_group_radio) != index) {
			elm_radio_value_set(order_group_radio, index);
		}
		mf_debug("sort_type_index is [%d] order_index is [%d]", sort_type_index, order_index);
		g_sort_type = __mf_sort_by_value_get(sort_type_index, order_index);
		mf_debug("g_sort_type is [%d]", g_sort_type);
	}
}

static void __mf_listby_gl_style_set(void *data)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	mf_genlist_item_class_free(ap->mf_gl_style.listby_itc);

	ap->mf_gl_style.listby_itc = elm_genlist_item_class_new();
	if (ap->mf_gl_style.listby_itc) {
		ap->mf_gl_style.listby_itc->item_style = "type1";
		ap->mf_gl_style.listby_itc->decorate_all_item_style = NULL;
		ap->mf_gl_style.listby_itc->decorate_item_style = NULL;
		ap->mf_gl_style.listby_itc->func.text_get = __mf_sort_by_gl_label;
		ap->mf_gl_style.listby_itc->func.content_get = __mf_sort_by_gl_icon;
		ap->mf_gl_style.listby_itc->func.del	    = NULL;
	}

	if (ap->mf_gl_style.order_itc == NULL) {
		ap->mf_gl_style.order_itc = elm_genlist_item_class_new();
		if (ap->mf_gl_style.order_itc != NULL) {
			ap->mf_gl_style.order_itc->item_style = "group_index";
			ap->mf_gl_style.order_itc->decorate_all_item_style = NULL;
			ap->mf_gl_style.order_itc->decorate_item_style = NULL;
			ap->mf_gl_style.order_itc->func.text_get = mf_sort_by_order_label_get;
			ap->mf_gl_style.order_itc->func.content_get = NULL;
			ap->mf_gl_style.order_itc->func.del	    = NULL;
		}
	}
}

void __mf_sort_by_button_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ap = mf_get_appdata();
	Evas_Object *btn = (Evas_Object *)obj;
	const char *label = elm_object_text_get(btn);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
	SAFE_FREE_OBJ(ap->mf_MainWindow.pPopupBox);

	if (g_strcmp0(label, mf_util_get_text(MF_BUTTON_LABEL_OK)) == 0) {
		__mf_sort_by_response_cb(g_sort_type);
	}
}

static void __mf_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;

	struct appdata *ap = mf_get_appdata();
	if (elm_check_state_get(obj)) {
		ap->mf_Status.check = 1;
	} else {
		ap->mf_Status.check = 0;
	}

	MF_TRACE_END;
	return;
}

void _language_changed(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata*) data;
	char *label = gettext(MF_LABEL_SAME_FILE_LABEL);
	char *message = g_strdup_printf(label, ap->file_name);
	mf_object_text_set(ap->label, message, NULL);
}

Evas_Object *mf_popup_replace_create(char *title, char *label_text, Evas_Smart_Cb func1, Evas_Smart_Cb func2, Evas_Smart_Cb func3, void *param)
{
	struct appdata *ap = mf_get_appdata();
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");
	Evas_Object *popup = elm_popup_add(ap->mf_MainWindow.pWindow);
	mf_retvm_if(popup == NULL, NULL, "POPUP is NULL");
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	ap->mf_Status.check = 0;
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	mf_object_text_set(popup, title, "title,text");
	/* layout */
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", EDJ_NAME);
	free(path);

	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_file_set(layout, edj_path, "popup_confirmation_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(layout, "language,changed", _language_changed, ap);
	/* check */
	Evas_Object *check = elm_check_add(popup);
	mf_retvm_if(check == NULL, NULL, "check is NULL");
	//mf_object_text_set(check, MF_LABEL_APPLY_ALL, NULL);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(check, "changed", __mf_check_clicked_cb, ap);
	elm_object_part_content_set(layout, "elm.swallow.end", check);
	/* label */
	Evas_Object *label = elm_label_add(layout);
	mf_retvm_if(label == NULL, NULL, "label is NULL");
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	mf_object_text_set(label, label_text, NULL);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	ap->label = label;
	evas_object_show(label);
	/*apply all label*/
	Evas_Object *apply_all_label = elm_label_add(layout);
	mf_retvm_if(apply_all_label == NULL, NULL, "apply_all_label is NULL");
	elm_object_style_set(apply_all_label, "popup/default");
	elm_label_line_wrap_set(apply_all_label, ELM_WRAP_MIXED);
	mf_object_text_set(apply_all_label, MF_LABEL_APPLY_ALL, NULL);
	evas_object_size_hint_weight_set(apply_all_label, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(apply_all_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(apply_all_label);

	elm_object_part_content_set(layout, "elm.swallow.content", label);
	elm_object_part_content_set(layout, "elm.swallow.apply", apply_all_label);
	int h,w,x,y;
	bool is_landscape = false;
	evas_object_geometry_get(ap->mf_MainWindow.pWindow, &x, &y, &w, &h);

	if (w > h) {
		is_landscape = true;
	}
	Evas_Object *box = __mf_popup_sort_by_box_set(popup, layout, 3, is_landscape);
	elm_object_content_set(popup, box);
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	Evas_Object *btn3 = NULL;
	btn1 = mf_object_create_button(popup,
	                               NULL,
	                               MF_POPUP_BUTTON_CANCEL,
	                               NULL,
	                               (Evas_Smart_Cb)func1,
	                               ap,
	                               EINA_FALSE);
	btn2 = mf_object_create_button(popup,
	                               NULL,
	                               MF_LABEL_REPLACE,
	                               NULL,
	                               (Evas_Smart_Cb)func2,
	                               param,
	                               EINA_FALSE);
	btn3 = mf_object_create_button(popup,
	                               NULL,
	                               LABEL_RENAME,
	                               NULL,
	                               (Evas_Smart_Cb)func3,
	                               param,
	                               EINA_FALSE);
	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);
	elm_object_part_content_set(popup, "button3", btn3);
	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_normal_del_cb, ap);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, func1, ap);
	return popup;
}

Evas_Object *mf_popup_sort_by_create(char *title, Evas_Smart_Cb func, void *param)
{
	MF_TRACE_BEGIN;
	Evas_Object *popup = NULL;
	struct appdata *ap = mf_get_appdata();
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");
	Evas_Object *genlist = NULL;
	//Evas_Object *radio_group = NULL;
	Evas_Object *box = NULL;
	int index = 0;
	g_sort_type = MYFILE_SORT_BY_NONE;

	popup = elm_popup_add(ap->mf_MainWindow.pWindow);
	//elm_object_signal_emit(popup, "elm,action,center_popup,entry", "");

	mf_retvm_if(popup == NULL, NULL, "popup is NULL");
	elm_object_focus_set(popup, EINA_FALSE);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if (title) {
		mf_object_text_set(popup, title, "title,text");
	}
	SAFE_FREE_OBJ(sort_group_radio);
	SAFE_FREE_OBJ(order_group_radio);

	int sort_value = 0;
	mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &sort_value);

	sort_group_radio = elm_radio_add(ap->mf_MainWindow.pWindow);
	elm_object_focus_set(sort_group_radio, EINA_FALSE);
	elm_radio_state_value_set(sort_group_radio, VALUE);
	elm_radio_value_set(sort_group_radio, 0);
	evas_object_hide(sort_group_radio);
	sort_type_index = __mf_sort_by_type_get(sort_value);

	order_group_radio = elm_radio_add(ap->mf_MainWindow.pWindow);
	elm_object_focus_set(order_group_radio, EINA_FALSE);
	elm_radio_value_set(order_group_radio, 0);
	evas_object_hide(order_group_radio);
	order_index = __mf_sort_by_order_get(sort_value);

	mf_debug("sort_value is [%d] sort_type is [%d] order_type is [%d]", sort_value, sort_type_index, order_index);

	__mf_listby_gl_style_set(ap);

	genlist = elm_genlist_add(popup);
	//elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_object_focus_set(genlist, EINA_FALSE);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "language,changed", mf_genlist_gl_lang_changed, NULL);
	//Elm_Object_Item *it = NULL;
	for (index = 0; index < 4; index++) {
		elm_genlist_item_append(genlist, ap->mf_gl_style.listby_itc, (void *)index, NULL,
		                        ELM_GENLIST_ITEM_NONE, __mf_sort_by_gl_select, (void *)index);

	}

	elm_genlist_item_append(genlist, ap->mf_gl_style.order_itc, NULL, NULL,
	                        ELM_GENLIST_ITEM_NONE, NULL, NULL);
	for (index = 4; index < 6; index++) {
		elm_genlist_item_append(genlist, ap->mf_gl_style.listby_itc, (void *)index, NULL,
		                        ELM_GENLIST_ITEM_NONE, __mf_sort_by_gl_select, (void *)index);
	}
	int h,w,x,y;
	bool is_landscape = false;
	evas_object_geometry_get(ap->mf_MainWindow.pWindow, &x, &y, &w, &h);

	if (w > h) {
		is_landscape = true;
	}
	box = __mf_popup_sort_by_box_set(popup, genlist, 6, is_landscape);
	ap->mf_MainWindow.pPopupBox = box;
	elm_object_content_set(popup, box);
	evas_object_smart_callback_add(popup, "response", func, param);
	evas_object_smart_callback_add(popup, "block,clicked", (Evas_Smart_Cb)__mf_popup_block_clicked_cb, NULL);

	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	btn1 = mf_object_create_button(popup,
	                               "popup",
	                               LABEL_CANCEL,
	                               NULL,
	                               (Evas_Smart_Cb)__mf_sort_by_button_cb,
	                               ap,
	                               EINA_FALSE);
	btn2 = mf_object_create_button(popup,
	                               "popup",
	                               MF_BUTTON_LABEL_OK,
	                               NULL,
	                               (Evas_Smart_Cb)__mf_sort_by_button_cb,
	                               ap,
	                               EINA_FALSE);
	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);

	//evas_object_smart_callback_add(elm_object_top_widget_get(popup),"rotation,changed",__mf_sortby_popup_rotate_cb, box);
	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __mf_popup_normal_del_cb, ap);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	return popup;
}
