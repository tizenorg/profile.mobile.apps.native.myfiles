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
#include "mf-share.h"
#include "mf-search-view.h"
#include "mf-object-item.h"
#include "mf-context-popup.h"
#include "mf-tray-item.h"
#include "mf-media.h"
#include "mf-search-view.h"
#include "mf-resource.h"
#include "mf-edit-view.h"
#include "mf-fm-svc-wrapper.h"


#define TOOLBAR_H	108
static Evas_Coord touch_x;
static Evas_Coord touch_y;
static Evas_Object *searchfilter_ctxpopup = NULL;

//Fixed P131021-03040  Added by jian12.li, for fixing the context popup is rotated problem, when rotate context popup, press the back key, it will be show again.
#define MF_CTXPOPUP_OBJ_DATA_KEY "mf_ctxpopup_data_key"
#define MF_CTXPOPUP_OBJ_MORE_BTN_KEY "mf_ctxpopup_more_btn_key"
#define MF_CTXPOPUP_OBJ_ROTATE_KEY "mf_ctxpopup_rotate_key"
#define MF_CTXPOPUP_STYLE_MORE "more/default"

enum __context_popup_type {
    CONTEXT_POPUP_TYPE_NONE,
    CONTEXT_POPUP_TYPE_MORE,
    CONTEXT_POPUP_TYPE_SEARCH_FILTER,
    CONTEXT_POPUP_TYPE_MAX
};

static void _move_ctxpopup_all(void *data, Evas_Object *parent, Evas_Object *ctxpopup)
{
	mf_retm_if(!data, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_RIGHT, ELM_CTXPOPUP_DIRECTION_UP);

	if (parent) {
		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;
		//give the coordinate values to show popup, begin to fix [P131029-05812]
		/*comment out the previous method(the following two line codes) to get coordinate values*/
		evas_object_geometry_get(parent, &x, &y, &w, &h);
		int changed_angle = elm_win_rotation_get(ap->mf_MainWindow.pWindow);
		mf_error("changed_angle is [%d]", changed_angle);
		switch (changed_angle) {
		case APP_DEVICE_ORIENTATION_270:
		case APP_DEVICE_ORIENTATION_90: {
			//landscape
			x =  x + (w / 2) + 7;
			y =  y + (h / 2);
			mf_error("x is [%d] y is [%d]", x, y);
			break;
		}
		case APP_DEVICE_ORIENTATION_180:
		case APP_DEVICE_ORIENTATION_0 : {
			//portrait

			x =  x + (w / 2);
			//y=  y + h - 4;
			y = y + h + 12;
			mf_error("x is [%d] y is [%d]", x, y);
			break;
		}
		default:
			break;
		}
		evas_object_move(ctxpopup, x , y);
		elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN,
		                                    ELM_CTXPOPUP_DIRECTION_LEFT,
		                                    ELM_CTXPOPUP_DIRECTION_RIGHT,
		                                    ELM_CTXPOPUP_DIRECTION_UP);
		//end*/
	} else {
		evas_object_move(ctxpopup, touch_x, touch_y);
	}
}


static void _move_more_ctxpopup(void *data, Evas_Object *win, Evas_Object *ctxpopup)
{
	mf_retm_if(data == NULL, "data is NULL");
	Evas_Coord w = 0, h = 0;
	int pos = -1;
	Evas_Coord x = 0;
	Evas_Coord y = 0;
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);
	switch (pos) {
	case 0:
	case 180:
		x = w / 2;
		y = h;
		break;
	case 90:
		x = h / 2;
		y = w;
		break;
	case 270:
		x = h / 2;
		y = w;
		break;
	}
	//elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	evas_object_move(ctxpopup, x, y);
}


/******************************
** Prototype    : mf_context_popup_mousedown_cb
** Description  : callback function for the mouse down operation in  context
                  popup
** Input        : void *data
**                int type
**                void *event
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
void mf_context_popup_get_position(Evas_Object *obj, const char *label, int *x, int *y)
{
	mf_retm_if(obj == NULL, "obj is NULL");
	mf_retm_if(label == NULL, "label is NULL");
	Evas_Object *pCtrlBar = obj;

	Elm_Object_Item *pItem = NULL;
	int x_position = 0;
	int y_position = 0;
	int w = 0;

	pItem = elm_toolbar_first_item_get(pCtrlBar);

	while (pItem) {
		const char *button_label = elm_object_item_text_get(pItem);
		if (g_strcmp0(button_label, label) == 0) {
			Evas_Object *icon = elm_toolbar_item_object_get(pItem);
			evas_object_geometry_get(icon, &x_position, &y_position, &w, NULL);
			*x = x_position + w / 2;
			*y = y_position;
			return;
		}
		pItem = elm_toolbar_item_next_get(pItem);
	}
}

int mf_context_popup_mousedown_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Button *ev = event;

	touch_x = ev->x;
	touch_y = ev->y;
	return 0;
}

void mf_context_popup_position_get(int *x, int *y)
{
	*x = touch_x;
	*y = touch_y;
}
/******************************
** Prototype    : mfContextPopupCreate
** Description  : Create the context popup
** Input        : void *data
**                eContextPopMode popupMode
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
static void __mf_context_popup_item_append(Evas_Object *parent, void *data, mf_context_popup_item_type_e type)
{
	MF_TRACE_BEGIN;
	mf_retm_if(parent == NULL, "parent is NULL");
	Evas_Object *ctxpopup = parent;
	Evas_Object *image = NULL;
	Elm_Object_Item *item = NULL;
	int hiden_state;
	switch (type) {
	case mf_context_popup_item_setting:
		item = elm_ctxpopup_item_append(ctxpopup, MF_LABEL_SETTINGS, image, mf_callback_setting_cb, data);
		mf_object_item_translate_set(item, MF_LABEL_SETTINGS);
		break;
	case mf_context_popup_item_storage_usage:
		item = elm_ctxpopup_item_append(ctxpopup, MF_LABEL_STORAGE_USAGE, image, mf_callback_item_storage_usage_cb, data);
		mf_object_item_translate_set(item, MF_LABEL_STORAGE_USAGE);
		break;
	case mf_context_popup_item_view_by:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_VIEW_AS, image, mf_callback_view_as_cb, data);
		mf_object_item_translate_set(item, LABEL_VIEW_AS);
		break;
	case mf_context_popup_item_sort_by:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_SORT_BY, image, mf_callback_list_by_view_cb, data);
		mf_object_item_translate_set(item, LABEL_SORT_BY);
		break;
	case mf_context_popup_item_search:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_SEARCH, image, mf_search_bar_enter_search_routine, data);
		mf_object_item_translate_set(item, LABEL_SEARCH);
		break;
	case mf_context_popup_item_edit:
		break;
		//prevent fix
		/*item = elm_ctxpopup_item_append(ctxpopup, MF_LABEL_SELECT_ITEMS, image, mf_callback_edit_cb, data);
		mf_object_item_translate_set(item, MF_LABEL_EDIT);
		break;*/
	case mf_context_popup_item_share:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_SHARE, image, mf_callback_share_button_cb, data);
		mf_object_item_translate_set(item, LABEL_SHARE);
		break;
	case mf_context_popup_item_new_folder:
		item = elm_ctxpopup_item_append(ctxpopup, MF_LABEL_CREATE_FOLDER, image, mf_callback_new_folder_create_cb, data);
		mf_object_item_translate_set(item, MF_LABEL_CREATE_FOLDER);
		break;
	case mf_context_popup_item_details:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_DETAIL, image, mf_callback_edit_details_cb, data);
		mf_object_item_translate_set(item, LABEL_DETAIL);
		break;
	case mf_context_popup_item_show_hide_hidden:
		mf_util_get_pref_value(PREF_TYPE_HIDEN_STATE, &hiden_state);
		if (hiden_state != MF_HIDEN_SHOW) {
			item = elm_ctxpopup_item_append(ctxpopup, mf_util_get_text(LABEL_SHOW_HIDDEN), image, mf_callback_show_hidden_items_cb, data);
		} else {
			item = elm_ctxpopup_item_append(ctxpopup, mf_util_get_text(LABEL_HIDE_HIDDEN), image, mf_callback_show_hidden_items_cb, data);
		}
		break;
	case mf_context_popup_item_copy:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_COPY, image, mf_callback_edit_copy_cb, data);
		mf_object_item_translate_set(item, LABEL_COPY);
		break;
	case mf_context_popup_item_move:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_MOVE, image, mf_callback_edit_move_cb, data);
		mf_object_item_translate_set(item, LABEL_MOVE);
		break;
	case mf_context_popup_item_delete:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_DELETE, image, mf_callback_edit_delete_cb, data);
		mf_object_item_translate_set(item, LABEL_DELETE);
		break;
	case mf_context_popup_item_rename:
		item = elm_ctxpopup_item_append(ctxpopup, LABEL_RENAME, image, mf_callback_edit_rename_cb, data);
		mf_object_item_translate_set(item, LABEL_RENAME);
		break;
	case mf_context_popup_item_remove_recent:
		item = elm_ctxpopup_item_append(ctxpopup, MF_LABEL_REMOVE, image, mf_callback_edit_delete_recent_cb, data);
		mf_object_item_translate_set(item, MF_LABEL_REMOVE);
		break;
	case mf_context_popup_item_uninstall:
		item = elm_ctxpopup_item_append(ctxpopup, MF_LABEL_UNINSTALL, image, mf_callback_edit_unintall_cb, data);
		mf_object_item_translate_set(item, MF_LABEL_UNINSTALL);
		break;
	default:
		break;
	}
}
//Added by jian12.li, for fixing the context popup is rotated problem, when rotate context popup, press the back key, it will be show again.

static void __mf_ctxpopup_parent_resize_cb(void *data, Evas *e,
        Evas_Object *obj, void *ei)
{
	MF_TRACE_BEGIN;
	mf_retm_if(!data, "data is NULL");
	evas_object_data_set((Evas_Object *)data, MF_CTXPOPUP_OBJ_ROTATE_KEY,
	                     (void*)true);
}

static void __mf_ctxpopup_search_filter_rotate_cb(void *data, Evas_Object *obj, void *ei)
{
	MF_TRACE_BEGIN;
	mf_retm_if(!data, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (searchfilter_ctxpopup) {
		_move_ctxpopup_all(ap, ap->mf_MainWindow.pSearchCategoryBtn, searchfilter_ctxpopup);
		evas_object_show(searchfilter_ctxpopup);
	}
}

static void __mf_ctxpopup_search_filter_hide_cb(void *data, Evas_Object *obj, void *ei)
{
	MF_TRACE_BEGIN;
	mf_retm_if(!data, "data is NULL");
	mf_retm_if(!obj, "obj is NULL");
	struct appdata *ap = (struct appdata *)data;

	bool ct_rotate = (bool)evas_object_data_get(obj,
	                 MF_CTXPOPUP_OBJ_ROTATE_KEY);
	if (!ct_rotate) {
		mf_debug("ctxpopup is dismissed");
		evas_object_del(obj);
		searchfilter_ctxpopup = NULL;
		ap->mf_MainWindow.pContextPopup = NULL;
	} else {
		mf_debug("ctxpopup is not dismissed");
		/* when "dismissed" cb is called next time,
		  * ctxpopup should be dismissed if device is not rotated. */
		evas_object_data_set(obj, MF_CTXPOPUP_OBJ_ROTATE_KEY,
		                     (void*)false);
		/* If ctxpopup is not dismissed, then it must be shown again.
		  * Otherwise "dismissed" cb will be called one more time. */
		if (searchfilter_ctxpopup) {
			_move_ctxpopup_all(ap, ap->mf_MainWindow.pSearchCategoryBtn, searchfilter_ctxpopup);
			evas_object_show(searchfilter_ctxpopup);
		}
	}
}

static void __mf_ctxpopup_hide_cb(void *data, Evas_Object *obj, void *ei)
{
	MF_TRACE_BEGIN;
	mf_retm_if(!data, "data is NULL");
	mf_retm_if(!obj, "obj is NULL");
	struct appdata *ap = (struct appdata *)data;

	bool ct_rotate = (bool)evas_object_data_get(obj,
	                 MF_CTXPOPUP_OBJ_ROTATE_KEY);

	if (!ct_rotate) {
		mf_debug("ctxpopup is dismissed");
		evas_object_del(obj);
		ap->mf_MainWindow.pContextPopup = NULL;
	} else {
		mf_debug("ctxpopup is not dismissed");
		/* when "dismissed" cb is called next time,
		  * ctxpopup should be dismissed if device is not rotated. */
		evas_object_data_set(obj, MF_CTXPOPUP_OBJ_ROTATE_KEY,
		                     (void*)false);
		/* If ctxpopup is not dismissed, then it must be shown again.
		  * Otherwise "dismissed" cb will be called one more time. */
		if (ap->mf_MainWindow.pContextPopup) {
			_move_more_ctxpopup(ap, ap->mf_MainWindow.pWindow, ap->mf_MainWindow.pContextPopup);
			evas_object_show(ap->mf_MainWindow.pContextPopup);
		}
	}
}

static void __mf_ctxpopup_rotate_cb(void *data, Evas_Object *obj, void *ei)
{
	MF_TRACE_BEGIN;
	mf_retm_if(!data, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	/*Evas_Object *more_btn = NULL;
	more_btn = (Evas_Object *)evas_object_data_get(ap->mf_MainWindow.pContextPopup,
						       MF_CTXPOPUP_OBJ_MORE_BTN_KEY);
	mf_retm_if(!more_btn, "more_btn is NULL");*/
	if (ap->mf_MainWindow.pContextPopup) {
		_move_more_ctxpopup(ap, ap->mf_MainWindow.pWindow, ap->mf_MainWindow.pContextPopup);
		evas_object_show(ap->mf_MainWindow.pContextPopup);
	}
}

static void __mf_ctxpopup_del_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	MF_TRACE_BEGIN;
	mf_retm_if(!data, "data is NULL");
	mf_retm_if(!obj, "obj is NULL");
	Evas_Object *ctxpopup = obj;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(!ap->mf_MainWindow.pWindow, "ap->mf_MainWindow.pWindow is NULL");

	evas_object_data_del(ctxpopup, MF_CTXPOPUP_OBJ_MORE_BTN_KEY);
	evas_object_data_del(ctxpopup, MF_CTXPOPUP_OBJ_ROTATE_KEY);
	evas_object_smart_callback_del(ctxpopup, "dismissed",
	                               __mf_ctxpopup_hide_cb);
	evas_object_event_callback_del(ap->mf_MainWindow.pWindow,
	                               EVAS_CALLBACK_RESIZE,
	                               __mf_ctxpopup_parent_resize_cb);

	evas_object_smart_callback_del(elm_object_top_widget_get(ctxpopup),
	                               "rotation,changed",
	                               __mf_ctxpopup_rotate_cb);
	evas_object_event_callback_del(ctxpopup, EVAS_CALLBACK_DEL,
	                               __mf_ctxpopup_del_cb);
	/*evas_object_smart_callback_del(ap->maininfo.naviframe,
				       "ctxpopup,items,update",
				       __mf_ctxpopup_items_update_cb);*/

	mf_debug("done");
}

static int __mf_ctxpopup_add_callbacks(void *data, Evas_Object *ctxpopup)
{
	mf_retvm_if(!data, -1, "data is NULL");
	mf_retvm_if(!ctxpopup, -1, "ctxpopup is NULL");
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(!ap->mf_MainWindow.pWindow, -1, "ap->mf_MainWindow.pWindow is NULL");

	//evas_object_event_callback_del(ap->mf_MainWindow.pWindow, EVAS_CALLBACK_RESIZE, mf_ug_resize_more_ctxpopup_cb);
	//evas_object_event_callback_add(ap->mf_MainWindow.pWindow, EVAS_CALLBACK_RESIZE, (Evas_Object_Event_Cb)mf_ug_resize_more_ctxpopup_cb, ap);
	evas_object_smart_callback_add(ctxpopup, "dismissed",
	                               __mf_ctxpopup_hide_cb, data);
	evas_object_smart_callback_add(elm_object_top_widget_get(ctxpopup),
	                               "rotation,changed",
	                               __mf_ctxpopup_rotate_cb, data);
	evas_object_event_callback_add(ap->mf_MainWindow.pWindow,
	                               EVAS_CALLBACK_RESIZE,
	                               __mf_ctxpopup_parent_resize_cb,
	                               ctxpopup);
	evas_object_event_callback_add(ctxpopup, EVAS_CALLBACK_DEL,
	                               __mf_ctxpopup_del_cb, data);
	/*evas_object_smart_callback_add(ap->mf_MainWindow.pWindow,
				       "ctxpopup,items,update",
				       __mf_ctxpopup_items_update_cb, ctxpopup);*/
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
	mf_debug("done");
	return 0;
}
//End by jian12.li

void mf_context_popup_create_more(void *data, Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata* ap = (struct appdata *)data;
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	Evas_Object *ctxpopup = NULL;
	ctxpopup = elm_ctxpopup_add(ap->mf_MainWindow.pWindow);
	elm_object_style_set(ctxpopup, "more/default");

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
	                                    ELM_CTXPOPUP_DIRECTION_UNKNOWN,
	                                    ELM_CTXPOPUP_DIRECTION_UNKNOWN,
	                                    ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	ap->mf_MainWindow.pContextPopup = ctxpopup;

	if (ap->mf_Status.more == MORE_DEFAULT) {
		if (ap->mf_Status.view_type == mf_view_root) {
			//1 Search
			//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_search);

			//1 Storage Usage
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_storage_usage);
			//1 Setting
			//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_setting);
		} else if (ap->mf_Status.view_type == mf_view_storage) {
			//1 Search
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_search);

			//1 Storage Usage
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_storage_usage);

			//1 Setting
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_setting);
		} else if (ap->mf_Status.view_type == mf_view_root_category) {
			//1 Search
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_search);
			if (ap->mf_Status.flagNoContent == EINA_FALSE) {
				//1 Share
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_share);

				//1 Delete
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_delete);
				//1 Move
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_move);
				//1 Copy
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_copy);
				//1 Rename
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_rename);
				//1 Edit
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_edit);
			}
			//1 View by
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_view_by);
			//1 Sort by
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_sort_by);
			//1 Details
			if (ap->mf_Status.flagNoContent == EINA_FALSE) {
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_details);
			}
			//1 Storage Usage
			//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_storage_usage);

			//1 Setting

			//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_setting);

		} else if (ap->mf_Status.view_type == mf_view_recent) {
			if (eina_list_count(ap->mf_FileOperation.recent_list) > 0) {
				ap->mf_Status.flagNoContent = EINA_FALSE;
			} else {
				ap->mf_Status.flagNoContent = EINA_TRUE;
			}
			//1 Search
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_search);

			if (ap->mf_Status.flagNoContent == EINA_FALSE) {
				//1 Share
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_share);
				//1 Remove
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_remove_recent);
				//view as
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_view_by);
				//1 Sort by
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_sort_by);
				//1 Details
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_details);
			} else {
				//1 Sort by
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_sort_by);
				//1 Storage Usage
				//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_storage_usage);

				//1 Setting
				//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_setting);
			}
		} else {

			//1 Search
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_search);

			if (ap->mf_Status.flagNoContent == EINA_FALSE) {
				//1 Edit
				//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_edit);
				if (!(ap->mf_FileOperation.file_list == NULL || eina_list_count(ap->mf_FileOperation.file_list) == 0)) {
					//1 Share
					__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_share);
				}
				//1 Delete
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_delete);
				//1 Move
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_move);
				//1 Copy
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_copy);
			}

			if (ap->mf_Status.flagNoContent == EINA_FALSE) {
				//1 Create
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_new_folder);
				//1 Rename
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_rename);
				//1 Edit
				//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_edit);

				//1 View by
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_view_by);
				//1 Sort by
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_sort_by);
				//1 Details
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_details);

			} else {
				//1 View by
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_view_by);
				//1 Sort by
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_sort_by);
			}
			// hidden
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_show_hide_hidden);
			//1 Setting

			//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_setting);
		}
	} else if (ap->mf_Status.more == MORE_SEARCH) {
		if (ap->mf_Status.flagNoContent == EINA_FALSE) {
			//delete
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_delete);
			//1 Share
			if (!(ap->mf_FileOperation.search_result_file_list == NULL ||
			        eina_list_count(ap->mf_FileOperation.search_result_file_list) == 0)) {
				__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_share);
			}
			//1 Move
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_move);
			//1 Copy
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_copy);
			//1 Rename
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_rename);
			//1 Details
			__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_details);
		} else {
			return;
		}

	} else if (ap->mf_Status.more == MORE_EDIT) {
		return;
		//1 Copy
		//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_copy); //fix P130916-02998 by ray

		//1 Move

		//__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_move); // fix P130916-02998  by ray

	} else if (ap->mf_Status.more == MORE_INTERNAL_MOVE ||
	           ap->mf_Status.more == MORE_INTERNAL_COPY  ||
	           ap->mf_Status.more == MORE_INTERNAL_DECOMPRESS) {
		//1 Create
		__mf_context_popup_item_append(ctxpopup, ap, mf_context_popup_item_new_folder);
	}

	if (ctxpopup) {
		__mf_ctxpopup_add_callbacks(ap, ctxpopup);
	}
	_move_more_ctxpopup(ap, ap->mf_MainWindow.pWindow, ctxpopup);
	evas_object_show(ctxpopup);
	MF_TRACE_END;
}

static void __mf_search_filter_selected(void *data, int category, const char *icon_buf)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata* ap = (struct appdata *)data;
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", EDJ_IMAGE);
	if (ap->mf_Status.search_category != category) {
		ap->mf_Status.search_category = category;
		if (ap->mf_MainWindow.pSearchCategoryBtn) {
			elm_image_file_set(ap->mf_MainWindow.pSearchCategoryBtn, edj_path, icon_buf);
		}
		mf_search_bar_view_update(ap);
	}
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	MF_TRACE_END;
}

static void __mf_search_filter_img_selected(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(event_info == NULL, "event_info is NULL");

	struct appdata* ap = (struct appdata *)data;
	int category = mf_tray_item_category_image;
	const char *icon_buf = IMG_ICON_SEARCH_CATEGORY_IMG;
	__mf_search_filter_selected(ap, category, icon_buf);
}

static void __mf_search_filter_snd_selected(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(event_info == NULL, "event_info is NULL");

	struct appdata* ap = (struct appdata *)data;
	int category = mf_tray_item_category_sounds;
	const char *icon_buf = IMG_ICON_SEARCH_CATEGORY_SND;
	__mf_search_filter_selected(ap, category, icon_buf);
}

static void __mf_search_filter_video_selected(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(event_info == NULL, "event_info is NULL");

	struct appdata* ap = (struct appdata *)data;
	int category = mf_tray_item_category_video;
	const char *icon_buf = IMG_ICON_SEARCH_CATEGORY_VIDEO;
	__mf_search_filter_selected(ap, category, icon_buf);
}
static void __mf_search_filter_doc_selected(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(event_info == NULL, "event_info is NULL");
	struct appdata* ap = (struct appdata *)data;
	int category = mf_tray_item_category_document;
	const char *icon_buf = IMG_ICON_SEARCH_CATEGORY_DOC;
	__mf_search_filter_selected(ap, category, icon_buf);
}

static void __mf_search_filter_all_selected(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(event_info == NULL, "event_info is NULL");

	struct appdata* ap = (struct appdata *)data;
	int category = mf_tray_item_category_none;
	const char *icon_buf = IMG_ICON_SEARCH_CATEGORY_ALL;
	__mf_search_filter_selected(ap, category, icon_buf);
}

Evas_Object *mf_context_popup_search_filter(Evas_Object *parent, void *user_data, Evas_Object *obj)
{
	struct appdata *ap = (struct appdata *)user_data;

	Elm_Object_Item *item = NULL;
	Evas_Object *ctxpopup = elm_ctxpopup_add(ap->mf_MainWindow.pWindow);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	searchfilter_ctxpopup = ctxpopup;
	Evas_Object *icon = NULL;
	elm_ctxpopup_horizontal_set(ctxpopup, EINA_FALSE);
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", EDJ_IMAGE);
	icon = elm_image_add(ctxpopup);
	elm_image_file_set(icon, edj_path, IMG_ICON_SEARCH_CATEGORY_ALL);
	item = elm_ctxpopup_item_append(ctxpopup, MF_POP_SEARCH_ALL, icon, __mf_search_filter_all_selected, user_data);
	mf_object_item_translate_set(item, MF_POP_SEARCH_ALL);

	icon = elm_image_add(ctxpopup);
	elm_image_file_set(icon, edj_path, IMG_ICON_SEARCH_CATEGORY_IMG);
	item = elm_ctxpopup_item_append(ctxpopup, MF_POP_SEARCH_IMAGES, icon, __mf_search_filter_img_selected, user_data);
	mf_object_item_translate_set(item, MF_POP_SEARCH_IMAGES);

	icon = elm_image_add(ctxpopup);
	elm_image_file_set(icon, edj_path, IMG_ICON_SEARCH_CATEGORY_VIDEO);
	item = elm_ctxpopup_item_append(ctxpopup, MF_POP_SEARCH_VIDEOS, icon, __mf_search_filter_video_selected, user_data);
	mf_object_item_translate_set(item, MF_POP_SEARCH_VIDEOS);

	icon = elm_image_add(ctxpopup);
	elm_image_file_set(icon, edj_path, IMG_ICON_SEARCH_CATEGORY_SND);
	item = elm_ctxpopup_item_append(ctxpopup, MF_POP_SEARCH_SOUNDS, icon, __mf_search_filter_snd_selected, user_data);
	mf_object_item_translate_set(item, MF_POP_SEARCH_SOUNDS);

	icon = elm_image_add(ctxpopup);
	elm_image_file_set(icon, edj_path, IMG_ICON_SEARCH_CATEGORY_DOC);
	item = elm_ctxpopup_item_append(ctxpopup, MF_POP_SEARCH_DOCUMENTS, icon, __mf_search_filter_doc_selected, user_data);
	mf_object_item_translate_set(item, MF_POP_SEARCH_DOCUMENTS);

	_move_ctxpopup_all(user_data, obj, ctxpopup);
	evas_object_show(ctxpopup);

	evas_object_smart_callback_add(ctxpopup, "dismissed",
	                               __mf_ctxpopup_search_filter_hide_cb, user_data);
	evas_object_smart_callback_add(elm_object_top_widget_get(ctxpopup),
	                               "rotation,changed",
	                               __mf_ctxpopup_search_filter_rotate_cb, ap);
	evas_object_event_callback_add(ap->mf_MainWindow.pWindow,
	                               EVAS_CALLBACK_RESIZE,
	                               __mf_ctxpopup_parent_resize_cb,
	                               ctxpopup);

	return ctxpopup;
}
