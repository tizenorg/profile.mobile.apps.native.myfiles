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

#include <Eina.h>

#include "mf-object-conf.h"
#include "mf-gengrid.h"
#include "mf-util.h"
#include "mf-dlog.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-launch.h"
#include "mf-tray-item.h"
#include "mf-gengrid.h"
#include "mf-genlist.h"
#include "mf-navi-bar.h"
#include "mf-object.h"
#include "mf-callback.h"
#include "mf-view.h"
#include "mf-search-view.h"
#include "mf-popup.h"
#include "mf-object-item.h"
#include "mf-file-util.h"
#include "mf-edit-view.h"



#define MF_SEARCH_OPTION_DEF (MF_SEARCH_OPT_DIR | MF_SEARCH_OPT_FILE)
#define MF_SEARCH_ROOT_NUM 1
#define MF_SEARCH_TIMER_INTERVAL 0.5
#define MF_CATEGOR_SEARCH_ITEM_COUNT	1000
#define MF_SEARCH_ITEM_COUNT	1000

Elm_Gengrid_Item_Class search_gic;
extern int flagSearchMsg;
extern pthread_mutex_t gLockSearchMsg;
extern pthread_cond_t gCondSearchMsg;
static Eina_Bool search_all_flag = EINA_FALSE;
static Eina_Bool do_search_all = EINA_FALSE;
static Ecore_Idler *entry_focus_allow_idler = NULL;

typedef struct {
	char *size;
	char *create_date;
} mf_search_detail_infor_s;

void mf_search_bar_set_search_all(Eina_Bool flag)
{
	search_all_flag = flag;
}
static void __mf_search_bar_sel_search_all(void *data, Evas_Object * obj, void *event_info)
{
	search_all_flag = EINA_TRUE;
	do_search_all = EINA_TRUE;
	mf_search_bar_search_started_callback(data, NULL, NULL);
}

void mf_search_gengrid_style_set()
{
	search_gic.item_style = "custom/myfile";
	search_gic.func.text_get = mf_gengrid_item_label_get;
	search_gic.func.content_get = mf_gengrid_item_icon_get;
	search_gic.func.state_get = NULL;
	search_gic.func.del = NULL;
}

static Evas_Object *_gl_search_all_content_get(void *data, Evas_Object * obj, const char *part)
{

	mf_debug("part is [%s]", part);
	if (!strcmp(part, "elm.icon")) {

		Evas_Object *btn = mf_object_create_button(obj,
		                   NULL,
		                   MF_LABEL_SEARCH_ALL,
		                   NULL,
		                   __mf_search_bar_sel_search_all,
		                   data,
		                   EINA_FALSE);

		return btn;
	}
	return NULL;
}

void mf_search_bar_search_all_item_append(void *data)
{

	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input data error");

	if (ap->mf_Status.view_type != mf_view_root) {
		Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
		if (itc) {
			itc->item_style = "myfile/1icon/search_all";
			itc->func.text_get = NULL;
			itc->func.content_get = _gl_search_all_content_get;
			itc->func.state_get = NULL;
			itc->func.del = NULL;
			elm_genlist_item_append(ap->mf_MainWindow.pNaviGenlist, itc, ap, NULL,
			                        ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}
}

static void __mf_search_bar_click_item(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *item_data = (mfItemData_s *)data;
	mf_retm_if(item_data->m_ItemName == NULL, "item_data->m_ItemName is NULL");
	mf_retm_if(item_data->m_ItemName->str == NULL, "item_data->m_ItemName->str is NULL");
	struct appdata *ap = item_data->ap;
	mf_retm_if(ap == NULL, "ap is NULL");
	char *path = item_data->m_ItemName->str;

	GString *new_path = NULL;


	if (ap->mf_Status.more == MORE_RENAME) {
		mf_callback_rename_save_cb(ap, NULL, NULL);
		return;
	}
	if (ap->mf_Status.search_handler > 0) {
		mf_search_stop(ap->mf_Status.search_handler);
		mf_search_finalize(&ap->mf_Status.search_handler);
	}


	if (mf_file_attr_is_dir(path)) {
#ifdef MF_SEARCH_UPDATE_COUNT
		ap->mf_Status.flagUpdateSearch = EINA_FALSE;
#endif

		elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
		SAFE_FREE_OBJ(ap->mf_MainWindow.pSearchBar);
		ap->mf_MainWindow.pSearchEntry = NULL;
		ap->mf_Status.view_type = mf_view_normal;
		ap->mf_Status.more = MORE_DEFAULT;
		new_path = g_string_new(path);

		/*set new path*/
		SAFE_FREE_GSTRING(ap->mf_Status.path);

		ap->mf_Status.path = new_path;
		//fix P131203-02347 by wangyan, get the clicked item path before eina list free
		if (ap->mf_FileOperation.search_result_folder_list) {
			mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.search_result_folder_list), MYFILE_TYPE_ITEM_DATA);
		}
		if (ap->mf_FileOperation.search_result_file_list) {
			mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.search_result_file_list), MYFILE_TYPE_ITEM_DATA);
		}

		mf_view_update(ap);
	} else {
		int ret = 0;
		ret = mf_launch_service(ap, path);
		mf_debug("ret is %d\n", ret);
		if (ret) {
			ap->mf_MainWindow.pNormalPopup =
			    mf_popup_create_popup(ap, POPMODE_TEXT, NULL, MF_LABEL_UNSUPPORT_FILE_TYPE, NULL, NULL, NULL, NULL, NULL);
		}
	}
	MF_TRACE_END;
}

static void __mf_search_bar_item_sel(void *data, Evas_Object * obj, void *event_info)
{
	mf_debug("Start");
	assert(data);

	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	if (item != NULL) {
		mfItemData_s *selected = (mfItemData_s *) elm_object_item_data_get(item);
		struct appdata *ap = (struct appdata *)selected->ap;
		//fix P131209-00523 by wangyan,search -> longpress searched file-> back, highlight should be released
		elm_genlist_item_selected_set(item, FALSE);
		if (ap->mf_MainWindow.pLongpressPopup != NULL) {
			return;
		}
		if (ap->mf_Status.more == MORE_EDIT_RENAME) {
			ap->mf_FileOperation.rename_item = item;
			mf_callback_idle_rename(selected);
		} else if (ap->mf_Status.more == MORE_RENAME) {
			mf_callback_rename_save_cb(ap, NULL, NULL);
			return;
		} else if (ap->mf_Status.more == MORE_EDIT || ap->mf_Status.more == MORE_SHARE_EDIT ||
		           ap->mf_Status.more == MORE_EDIT_COPY || ap->mf_Status.more == MORE_EDIT_MOVE ||
		           ap->mf_Status.more == MORE_EDIT_DETAIL || ap->mf_Status.more == MORE_EDIT_DELETE
		           || ap->mf_Status.more == MORE_SHARE_EDIT) {
			if (selected->file_type == FILE_TYPE_DIR) {
				mf_info("select type is DIR");
				if (mf_edit_folder_list_item_exists(selected->item)) {
					selected->m_checked = false;
					mf_edit_folder_list_item_remove(selected->item);
				} else {
					selected->m_checked = true;
					mf_edit_folder_list_append(selected->item);
				}
			} else {
				mf_info("select type is FILE");
				if (mf_edit_file_list_item_exists(selected->item)) {
					mf_debug("select file exists, remove from list");
					selected->m_checked = false;
					mf_edit_file_list_item_remove(selected->item);
				} else {
					mf_debug("select file not exists, add to list");
					selected->m_checked = true;
					mf_edit_file_list_append(selected->item);
				}
			}
			if (ap->mf_Status.more == MORE_EDIT_COPY || ap->mf_Status.more == MORE_EDIT_MOVE) {
				mf_callback_copy_move_cb(ap, NULL, NULL);
			} else if (ap->mf_Status.more == MORE_EDIT_DELETE) {
				mf_callback_delete_cb(ap, NULL, NULL);
			} else if (ap->mf_Status.more == MORE_SHARE_EDIT) {
				mf_callback_share_cb(ap, NULL, NULL);
			} else if (ap->mf_Status.more == MORE_EDIT_DETAIL) {
				mf_debug("detail callback");
				mf_callback_details_cb(ap, NULL, NULL);
			}
		} else {
			__mf_search_bar_click_item(selected);
		}
	}
}

static void __mf_search_bar_grid_item_sel(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	if (item != NULL) {
		mfItemData_s *selected = (mfItemData_s *) elm_object_item_data_get(item);
		mf_retm_if(selected == NULL, "selected is NULL");
		elm_gengrid_item_selected_set(item, FALSE);

		mf_retm_if(selected->m_ItemName == NULL, "selected->m_ItemName is NULL");
		mf_retm_if(selected->m_ItemName->str == NULL, "selected->m_ItemName->str is NULL");

		__mf_search_bar_click_item(selected);
	}
}

mfItemData_s *mf_search_item_normal_data_get(const char *fullpath, void *user_data, int list_type)
{
	mf_retvm_if(fullpath == NULL, NULL, "fullpath error");
	mf_retvm_if(user_data == NULL, NULL, "user_data error");
	struct appdata *ap = (struct appdata *)user_data;
	mfItemData_s *item_data = NULL;

	item_data = (mfItemData_s *) calloc(sizeof(mfItemData_s), sizeof(char));
	if (item_data == NULL) {
		return NULL;
	}
	item_data->m_ItemName = g_string_new(fullpath);

	item_data->m_checked = FALSE;
	item_data->pCheckBox = NULL;
	item_data->thumb_path = NULL;
	item_data->real_thumb_flag = FALSE;
	item_data->media = NULL;
	item_data->ap = ap;
	if (mf_is_dir(fullpath)) {
		item_data->file_type = FILE_TYPE_DIR;
	} else {
		mf_file_attr_get_file_category(fullpath, &item_data->file_type);
	}
	item_data->storage_type = mf_fm_svc_wrapper_get_location(fullpath);
	item_data->list_type = list_type;
	return item_data;

}

static mfItemData_s *__mf_search_bar_item_generate(void *data, const char *fullpath)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data error");
	mf_retvm_if(fullpath == NULL, NULL, "fullpath error");

	struct appdata *ap = (struct appdata *)data;
	mfItemData_s *m_TempItem = NULL;
	m_TempItem = mf_search_item_normal_data_get(fullpath, ap, mf_list_normal);
	if (m_TempItem == NULL) {
		return NULL;
	}
	if (!(m_TempItem->real_thumb_flag && m_TempItem->thumb_path)) {
		mf_genlist_get_thumbnail(m_TempItem);
	}
	return m_TempItem;
}
static void __mf_search_bar_item_append(void *data, mfItemData_s *item_data)
{

	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data error");
	mf_retm_if(item_data == NULL, "fullpath error");

	struct appdata *ap = (struct appdata *)data;

//#ifdef MF_SEARCH_THUMBNAIL
#if 1
	int view_style = mf_view_style_get(ap);
	if (view_style == MF_VIEW_STYLE_THUMBNAIL) {
		Elm_Object_Item *git = NULL;
		git = elm_gengrid_item_append(ap->mf_MainWindow.pNaviGengrid, &search_gic, item_data, __mf_search_bar_grid_item_sel, ap);
		item_data->item = git;
	} else {
		Elm_Object_Item *it = NULL;
		it = elm_genlist_item_append(ap->mf_MainWindow.pNaviGenlist, ap->mf_gl_style.search_itc, item_data, NULL,
		                             ELM_GENLIST_ITEM_NONE, __mf_search_bar_item_sel, ap);
		item_data->item = it;
	}
#else
	Elm_Object_Item *it = NULL;
	it = elm_genlist_item_append(ap->mf_MainWindow.pNaviGenlist, ap->mf_gl_style.search_itc, m_TempItem, NULL,
	                             ELM_GENLIST_ITEM_NONE, __mf_search_bar_item_sel, ap);
	m_TempItem->item = it;
#endif
}
void mf_search_bar_folder_item_append(void *data, void *user_data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)user_data;
	mf_retm_if(ap == NULL, "input data error");
	char *item_name = (char *)data;
	mf_retm_if(item_name == NULL, "input item_name error");

	mfItemData_s *item_data = __mf_search_bar_item_generate(ap, item_name);
	if (item_data) {
		ap->mf_FileOperation.search_result_folder_list = eina_list_append(ap->mf_FileOperation.search_result_folder_list, item_data);
		if (ap->mf_Status.search_category == mf_tray_item_category_none) {
			__mf_search_bar_item_append(ap, item_data);
			ap->mf_Status.count++;
		}
	}

	MF_TRACE_END;

}


void mf_search_bar_file_item_append(void *data, void *user_data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)user_data;
	mf_retm_if(ap == NULL, "input data error");
	char *item_name = (char *)data;
	mf_retm_if(item_name == NULL, "input item_name error");

	mfItemData_s *item_data = __mf_search_bar_item_generate(ap, item_name);
	if (item_data) {
		ap->mf_FileOperation.search_result_file_list = eina_list_append(ap->mf_FileOperation.search_result_file_list, item_data);
		if (ap->mf_Status.search_category != mf_tray_item_category_none) {
			int type = 0;
			type = mf_tray_item_type(item_name);
			if (type == ap->mf_Status.search_category) {
				__mf_search_bar_item_append(ap, item_data);
				ap->mf_Status.count++;
			}
		} else {
			__mf_search_bar_item_append(ap, item_data);
			ap->mf_Status.count++;
		}
	}


	MF_TRACE_END;

}


void mf_search_bar_set_content(void *data, Evas_Object *pLayout, Evas_Object *NaviContent, bool is_no_content)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(pLayout == NULL, "pConform is NULL");
	mf_retm_if(NaviContent == NULL, "NaviContent is NULL");

	elm_box_clear(pLayout);
	elm_box_pack_end(pLayout, NaviContent);
	MF_TRACE_END;
}


void mf_search_bar_content_object_create(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "result is NULL");
	struct appdata *ap = (struct appdata *)data;
	int view_style = mf_view_style_get(ap);
	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		if (ap->mf_MainWindow.pNaviGenlist == NULL) {
			ap->mf_MainWindow.pNaviGenlist = elm_genlist_add(ap->mf_MainWindow.pNaviBar);
			elm_genlist_mode_set(ap->mf_MainWindow.pNaviGenlist, ELM_LIST_COMPRESS);
			elm_object_focus_set(ap->mf_MainWindow.pNaviGenlist, EINA_FALSE);
			elm_genlist_mode_set(ap->mf_MainWindow.pNaviGenlist, ELM_LIST_COMPRESS);
			evas_object_size_hint_weight_set(ap->mf_MainWindow.pNaviGenlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(ap->mf_MainWindow.pNaviGenlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

			evas_object_show(ap->mf_MainWindow.pNaviGenlist);
			//evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGenlist, "longpressed", mf_genlist_gl_longpress, ap);
			mf_search_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, ap->mf_MainWindow.pNaviGenlist, false);
		}
	} else {
		if (ap->mf_MainWindow.pNaviGengrid == NULL) {
			ap->mf_MainWindow.pNaviGengrid = mf_gengrid_create_grid(ap->mf_MainWindow.pNaviBar);

			evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGengrid, "language,changed", mf_gengrid_gl_lang_changed, ap);
			//evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGengrid, "longpressed", mf_gengrid_thumbs_longpressed, ap);
			evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGengrid, "realized", mf_gengrid_realized, ap);
			elm_gengrid_clear(ap->mf_MainWindow.pNaviGengrid);
			mf_search_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, ap->mf_MainWindow.pNaviGengrid, false);
		}
		elm_gengrid_align_set(ap->mf_MainWindow.pNaviGengrid, 0.0, 0.0);
		elm_gengrid_item_size_set(ap->mf_MainWindow.pNaviGengrid, MF_HD_GENGRID_ITEM_WIDTH, MF_HD_GENGRID_ITEM_HEIGTH);
		evas_object_show(ap->mf_MainWindow.pNaviGengrid);
	}


}

void mf_search_set_ctrl_button(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Elm_Object_Item *navi_it = ap->mf_MainWindow.pNaviItem;
	Evas_Object *more_bt = NULL;
	ap->mf_Status.extra = MORE_SEARCH;
	more_bt = mf_object_create_button(ap->mf_MainWindow.pNaviBar, NAVI_BUTTON_EDIT, MF_LABEL_MORE,
	                                  NULL, (Evas_Smart_Cb)mf_callback_more_button_cb, ap, EINA_FALSE);

	if (more_bt) {
		Evas_Object *unset = elm_object_item_part_content_unset(navi_it, NAVI_MORE_BUTTON_PART);
		SAFE_FREE_OBJ(unset);
		elm_object_item_part_content_set(navi_it, NAVI_MORE_BUTTON_PART, more_bt);
		evas_object_event_callback_add(more_bt, EVAS_CALLBACK_KEY_DOWN, mf_callback_more_keydown_cb, ap);
	}
	t_end;
	MF_TRACE_END;
}

void mf_search_bar_content_create(mf_search_result_t *result, void *user_data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(result == NULL, "result is NULL");

	struct appdata *ap = (struct appdata *)user_data;
	mf_retm_if(ap == NULL, "ap is NULL");
	ap->mf_Status.count = 0;
	mf_search_bar_content_object_create(ap);

	mf_error("file is [%d] folder is [%d]", g_list_length(result->file_list), g_list_length(result->dir_list));
	if (result->dir_list != NULL) {
		g_list_foreach(result->dir_list, mf_search_bar_folder_item_append, ap);
	}
	if (result->file_list != NULL) {
		g_list_foreach(result->file_list, mf_search_bar_file_item_append, ap);
	}
	if (ap->mf_Status.count == 0) {
		ap->mf_Status.flagNoContent = true;
		Evas_Object *parent = NULL;
		parent = ap->mf_MainWindow.pNaviBar;
		Evas_Object *no_content = elm_layout_add(parent);
		elm_layout_theme_set(no_content, "layout", "nocontents", "text");
		evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_focus_set(no_content, EINA_FALSE);
		mf_object_text_set(no_content, MF_LABEL_NO_RESULT_FOUND, "elm.text");
		mf_search_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, no_content, true);
		evas_object_show(no_content);
	} else {
		ap->mf_Status.flagNoContent = false;
		mf_search_set_ctrl_button(ap);
	}
}

void mf_search_bar_view_update(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	Eina_List *l = NULL;
	int view_style = mf_view_style_get(ap);
	mfItemData_s *item_data = NULL;

	//mf_object_enable_virtualkeypad();

	mf_search_bar_content_object_create(ap);

	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		elm_genlist_clear(ap->mf_MainWindow.pNaviGenlist);
		EINA_LIST_FOREACH(ap->mf_FileOperation.search_result_folder_list, l, item_data) {
			if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
				if (!mf_file_exists(item_data->m_ItemName->str)) {
					ap->mf_FileOperation.search_result_folder_list = eina_list_remove(ap->mf_FileOperation.search_result_folder_list, item_data);
					mf_util_normal_item_data_free(&item_data);
					continue;
				}
				if (ap->mf_Status.search_category == mf_tray_item_category_none) {
					__mf_search_bar_item_append(ap, item_data);
				}
			}
		}
		EINA_LIST_FOREACH(ap->mf_FileOperation.search_result_file_list, l, item_data) {
			if (item_data) {
				if (!mf_file_exists(item_data->m_ItemName->str)) {
					ap->mf_FileOperation.search_result_file_list = eina_list_remove(ap->mf_FileOperation.search_result_file_list, item_data);
					mf_util_normal_item_data_free(&item_data);
					continue;
				}
				if (ap->mf_Status.search_category != mf_tray_item_category_none) {
					int type = 0;
					type = mf_tray_item_category_type_get_by_file_type(item_data->file_type);
					if (type != ap->mf_Status.search_category) {
						continue;
					} else {
						__mf_search_bar_item_append(ap, item_data);
					}
				} else {
					__mf_search_bar_item_append(ap, item_data);
				}
			}
		}
		if (elm_genlist_items_count(ap->mf_MainWindow.pNaviGenlist) == 0) {
			//Evas_Object *no_content = NULL;

			Evas_Object *parent = NULL;
			parent = ap->mf_MainWindow.pNaviLayout;
			//no_content = mf_object_create_no_content(parent);
			//mf_object_disable_virtualkeypad();
			Evas_Object *no_content = elm_layout_add(parent);
			//elm_layout_theme_set(no_content, "layout", "nocontents", "search");
			elm_layout_theme_set(no_content, "layout", "nocontents", "text");
			evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_object_focus_set(no_content, EINA_FALSE);
			mf_object_text_set(no_content, MF_LABEL_NO_RESULT_FOUND, "elm.text");
			mf_search_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, no_content, true);
			evas_object_show(no_content);
			ap->mf_MainWindow.pNaviGenlist = NULL;
		}
	} else {
		elm_gengrid_clear(ap->mf_MainWindow.pNaviGengrid);
		EINA_LIST_FOREACH(ap->mf_FileOperation.search_result_folder_list, l, item_data) {
			if (item_data) {
				if (!mf_file_exists(item_data->m_ItemName->str)) {
					ap->mf_FileOperation.search_result_folder_list = eina_list_remove(ap->mf_FileOperation.search_result_folder_list, item_data);
					mf_util_normal_item_data_free(&item_data);
					continue;
				}
				if (ap->mf_Status.search_category == mf_tray_item_category_none) {
					__mf_search_bar_item_append(ap, item_data);
				}
			}
		}
		EINA_LIST_FOREACH(ap->mf_FileOperation.search_result_file_list, l, item_data) {
			if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
				if (!mf_file_exists(item_data->m_ItemName->str)) {
					ap->mf_FileOperation.search_result_file_list = eina_list_remove(ap->mf_FileOperation.search_result_file_list, item_data);
					mf_util_normal_item_data_free(&item_data);
					continue;
				}
				if (ap->mf_Status.search_category != mf_tray_item_category_none) {
					int type = 0;
					type = mf_tray_item_category_type_get_by_file_type(item_data->file_type);
					if (type != ap->mf_Status.search_category) {
						continue;
					} else {
						__mf_search_bar_item_append(ap, item_data);
					}
				} else {
					__mf_search_bar_item_append(ap, item_data);
				}
			}
		}
		if (elm_gengrid_items_count(ap->mf_MainWindow.pNaviGengrid) == 0) {
			Evas_Object *parent = NULL;
			parent = ap->mf_MainWindow.pNaviLayout;
			Evas_Object *no_content = elm_layout_add(parent);
			elm_layout_theme_set(no_content, "layout", "nocontents", "search");
			elm_object_focus_set(no_content, EINA_FALSE);
			evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
			//mf_object_disable_virtualkeypad();
			mf_object_text_set(no_content, MF_LABEL_NO_RESULT_FOUND, "elm.text");
			mf_search_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, no_content, true);
			evas_object_show(no_content);
			ap->mf_MainWindow.pNaviGengrid = NULL;
		}

	}
}

static int __mf_search_bar_entry_focus_allow(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, 0, "data is NULL");
	if (data) {
		Evas_Object *entry = (Evas_Object *)data;
		elm_object_focus_allow_set(entry, EINA_TRUE);
	}
	entry_focus_allow_idler = NULL;
	MF_TRACE_END;
	return ECORE_CALLBACK_CANCEL;
}
void mf_entry_focus_allow_idler_destory()
{
	mf_ecore_idler_del(entry_focus_allow_idler);
}

#ifdef MF_SEARCH_UPDATE_COUNT
static void __mf_search_bar_result_update(mf_search_result_t *result, void *user_data, int msg_type)
{
	MF_TRACE_BEGIN;
	mf_retm_if(result == NULL, "result is NULL");

	struct appdata *ap = (struct appdata *)user_data;
	mf_retm_if(ap == NULL, "ap is NULL");
	//mf_object_enable_virtualkeypad();

	if (ap->mf_Status.flagUpdateSearch == EINA_FALSE) {
//#ifdef MF_SEARCH_THUMBNAIL
#if 1
		int view_style = mf_view_style_get(ap);
		if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
			if (ap->mf_MainWindow.pNaviGenlist) {
				elm_genlist_clear(ap->mf_MainWindow.pNaviBar);
			}
		} else {
			if (ap->mf_MainWindow.pNaviGengrid) {
				elm_gengrid_clear(ap->mf_MainWindow.pNaviGengrid);
			}
		}
#else
		if (ap->mf_MainWindow.pNaviGenlist) {
			elm_genlist_clear(ap->mf_MainWindow.pNaviBar);
		}
#endif
		Evas_Object *progressbar = elm_progressbar_add(ap->mf_MainWindow.pNaviBar);
		elm_object_style_set(progressbar, "list_process");
		evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
		evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(progressbar);
		elm_progressbar_pulse(progressbar, EINA_TRUE);
		elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN, progressbar);
	}
	if (msg_type == MF_SEARCH_PIPE_MSG_DOING) {
		ap->mf_Status.flagUpdateSearch = EINA_TRUE;
		ap->mf_Status.flagNoContent = EINA_FALSE;
		mf_search_bar_content_create(result, ap);
		MF_TRACE_END;
	} else {
		if (ap->mf_Status.view_type != mf_view_root_category) {
			elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_TRUE);
		}
		if (ap->mf_Status.more == MORE_SEARCH && ap->mf_Status.flagSearchAll == EINA_FALSE) {
			mf_search_bar_content_create(result, ap);
		} else {
			if (ap->mf_Status.flagUpdateSearch == EINA_FALSE) {
				if (g_list_length(result->dir_list) + g_list_length(result->file_list) == 0) {
					//Evas_Object *pSearchBarLabel = NULL;

					Evas_Object *parent = NULL;
					ap->mf_Status.flagNoContent = EINA_TRUE;
					parent = ap->mf_MainWindow.pNaviLayout;
					//pSearchBarLabel = mf_object_create_no_content(parent);
					//mf_object_disable_virtualkeypad();
					Evas_Object *no_content = elm_layout_add(parent);
					elm_layout_theme_set(no_content, "layout", "nocontents", "search");
					elm_object_focus_set(no_content, EINA_FALSE);
					evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
					evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
					mf_object_text_set(no_content, MF_LABEL_NO_RESULT_FOUND, "elm.text");
					evas_object_show(no_content);
					elm_box_pack_end(ap->mf_MainWindow.pNaviBox, no_content);

					//mf_navi_bar_layout_content_set(parent, no_content);

				} else {
					mf_search_bar_content_create(result, ap);
				}
			} else {
				mf_search_bar_content_create(result, ap);
			}
		}
		ap->mf_Status.flagUpdateSearch = EINA_FALSE;
		Evas_Object *unUsed = elm_object_item_part_content_unset(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN);
		SAFE_FREE_OBJ(unUsed);
	}
	MF_TRACE_END;
}

#else
static void __mf_search_bar_result_update(mf_search_result_t *result, void *user_data, int msg_type)
{
	MF_TRACE_BEGIN;
	mf_retm_if(result == NULL, "result is NULL");

	struct appdata *ap = (struct appdata *)user_data;
	mf_retm_if(ap == NULL, "ap is NULL");
	//mf_object_enable_virtualkeypad();

	if (ap->mf_FileOperation.search_result_folder_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.search_result_folder_list), MYFILE_TYPE_ITEM_DATA);
	}
	if (ap->mf_FileOperation.search_result_file_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.search_result_file_list), MYFILE_TYPE_ITEM_DATA);
	}

	if (ap->mf_Status.view_type != mf_view_root && search_all_flag == EINA_FALSE) {
		mf_search_bar_content_create(result, ap);
		//mf_search_bar_search_all_item_append(ap);
		if (g_list_length(result->dir_list) + g_list_length(result->file_list) == 0) {
			//Evas_Object *pSearchBarLabel = NULL;
			//elm_layout_theme_set(lay, "layout", "nocontents", "search");
			Evas_Object *parent = NULL;
			ap->mf_Status.flagNoContent = EINA_TRUE;
			parent = ap->mf_MainWindow.pNaviLayout;
			//pSearchBarLabel = mf_object_create_no_content(parent);
			Evas_Object *no_content = elm_layout_add(parent);
			elm_layout_theme_set(no_content, "layout", "nocontents", "search");
			evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_object_focus_set(no_content, EINA_FALSE);
			//mf_object_disable_virtualkeypad();
			mf_object_text_set(no_content, MF_LABEL_NO_RESULT_FOUND, "elm.text");
			evas_object_show(no_content);
			mf_search_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, no_content, true);
			//mf_navi_bar_layout_content_set(parent, pSearchBarLabel);

		}
	} else {
		if (g_list_length(result->dir_list) + g_list_length(result->file_list) == 0) {
			//Evas_Object *pSearchBarLabel = NULL;
			//elm_layout_theme_set(lay, "layout", "nocontents", "search");
			Evas_Object *parent = NULL;
			ap->mf_Status.flagNoContent = EINA_TRUE;
			parent = ap->mf_MainWindow.pNaviLayout;
			//pSearchBarLabel = mf_object_create_no_content(parent);
			Evas_Object *no_content = elm_layout_add(parent);
			elm_layout_theme_set(no_content, "layout", "nocontents", "search");
			evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_object_focus_set(no_content, EINA_FALSE);
			//mf_object_disable_virtualkeypad();
			mf_object_text_set(no_content, MF_LABEL_NO_RESULT_FOUND, "elm.text");
			evas_object_show(no_content);
			mf_search_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, no_content, true);
			//mf_navi_bar_layout_content_set(parent, pSearchBarLabel);

		} else {
			mf_search_bar_content_create(result, ap);
		}
		if (ap->mf_MainWindow.pNaviGenlist && elm_genlist_items_count(ap->mf_MainWindow.pNaviGenlist) == 0) {
			//Evas_Object *no_content = NULL;
			Evas_Object *parent = NULL;
			parent = ap->mf_MainWindow.pNaviLayout;
			//no_content = mf_object_create_no_content(parent);
			Evas_Object *no_content = elm_layout_add(parent);
			elm_layout_theme_set(no_content, "layout", "nocontents", "search");
			evas_object_size_hint_weight_set(no_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(no_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_object_focus_set(no_content, EINA_FALSE);
			//mf_object_disable_virtualkeypad();
			mf_object_text_set(no_content, MF_LABEL_NO_RESULT_FOUND, "elm.text");
			evas_object_show(no_content);
			mf_search_bar_set_content(ap, ap->mf_MainWindow.pNaviBox, no_content, true);

			ap->mf_MainWindow.pNaviGenlist = NULL;
		}
	}
	mf_ecore_idler_del(entry_focus_allow_idler);
	entry_focus_allow_idler = ecore_idler_add((Ecore_Task_Cb)__mf_search_bar_entry_focus_allow, (void*)ap->mf_MainWindow.pSearchEntry);
	MF_TRACE_END;
}
#endif
static void __mf_search_bar_stop_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input data error");

	if (ap->mf_FileOperation.sync_pipe != NULL) {
		ecore_pipe_del(ap->mf_FileOperation.sync_pipe);
		ap->mf_FileOperation.sync_pipe = NULL;
	}

	__mf_search_bar_result_update(((ms_handle_t *) ap->mf_Status.search_handler)->result, ap, MF_SEARCH_PIPE_MSG_FINISHED);

	if (ap->mf_Status.search_handler > 0) {
		mf_search_stop(ap->mf_Status.search_handler);
	}

	if (ap->mf_Status.search_handler > 0) {
		mf_search_finalize(&ap->mf_Status.search_handler);
	}
	ap->mf_Status.flagSearchStart = EINA_FALSE;
	SAFE_FREE_OBJ(ap->mf_MainWindow.pProgressPopup);
}

void mf_search_bar_stop(void *data)
{
	__mf_search_bar_stop_cb(data, NULL, NULL);
}

static int __mf_search_bar_idle_search_msg_cope_finished(void *data)
{
	pthread_mutex_lock(&gLockSearchMsg);
	if (flagSearchMsg == 0) {
		flagSearchMsg = 1;
		pthread_cond_signal(&gCondSearchMsg);
	}
	pthread_mutex_unlock(&gLockSearchMsg);

	struct appdata *ap = (struct appdata *)data;
	if (ap == NULL) {
		mf_debug("input ap is NULL");
	} else {
		ap->mf_Status.search_idler = NULL;
	}

	return ECORE_CALLBACK_CANCEL;
}

static void __mf_search_bar_pipe_cb(void *data, void *buffer, unsigned int nbyte)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	if (ap == NULL) {
		mf_debug("input ap is NULL");
		goto MF_CONTINURE_SEARCH;
	}

	mf_search_pipe_msg *pSearchMsg = (mf_search_pipe_msg *) buffer;
	if (pSearchMsg == NULL) {
		mf_debug("received message is NULL");
		goto MF_CONTINURE_SEARCH;
	}

	if (pSearchMsg->mf_sp_msg_type == MF_SEARCH_PIPE_MSG_RESULT_REPORT) {
		mf_debug("result get");

	}
#ifdef MF_SEARCH_UPDATE_COUNT
	else  if (pSearchMsg->mf_sp_msg_type == MF_SEARCH_PIPE_MSG_DOING) {
		mf_search_result_t *result = (mf_search_result_t *)(pSearchMsg->report_result);
		__mf_search_bar_result_update(result, data, MF_SEARCH_PIPE_MSG_DOING);

		if (result->dir_list) {
			g_list_foreach(result->dir_list, (GFunc) g_free, NULL);
			g_list_free(result->dir_list);
			result->dir_list = NULL;
		}
		if (result->file_list) {
			g_list_foreach(result->file_list, (GFunc) g_free, NULL);
			g_list_free(result->file_list);
			result->file_list = NULL;
		}
		g_free(result);
		pSearchMsg->report_result = NULL;
	}
#endif
	else if (pSearchMsg->mf_sp_msg_type == MF_SEARCH_PIPE_MSG_ROOT_CHANGE) {
		mf_debug("path change	%s", pSearchMsg->current_path);
		SAFE_FREE_CHAR(pSearchMsg->current_path);
	} else if (pSearchMsg->mf_sp_msg_type == MF_SEARCH_PIPE_MSG_FINISHED) {
		ap->mf_Status.flagSearchStart = EINA_FALSE;
		SAFE_FREE_OBJ(ap->mf_MainWindow.pProgressPopup);
		__mf_search_bar_result_update((mf_search_result_t *) pSearchMsg->report_result, data, MF_SEARCH_PIPE_MSG_FINISHED);
	}

MF_CONTINURE_SEARCH:
	if (ap) {
		mf_ecore_idler_del(ap->mf_Status.search_idler);
		ap->mf_Status.search_idler = ecore_idler_add((Ecore_Task_Cb)__mf_search_bar_idle_search_msg_cope_finished, ap);
	}
	MF_TRACE_END;
}


static bool __mf_search_bar_idle_search_start(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	{

		int root_num = 0;
		mf_genlist_create_itc_style(&ap->mf_gl_style.search_itc, mf_item_itc_type_search);

		char *text = NULL;
		const char *SearchRoot[10] = {0};
		if (ap->mf_Status.more == MORE_SEARCH) {
			text = g_strdup(elm_object_text_get(ap->mf_MainWindow.pSearchEntry));
		}

		if (ap->mf_Status.view_type == mf_view_root
		        || ap->mf_Status.view_type == mf_view_root_category
		        || ap->mf_Status.view_type == mf_view_storage
		        || ap->mf_Status.view_type == mf_view_recent) {
			SearchRoot[0] = PHONE_FOLDER;
			root_num++;
			if (ap->mf_Status.iStorageState & MYFILE_MMC) {
				SearchRoot[root_num] = MEMORY_FOLDER;
				root_num++;
			}
		} else if (do_search_all) {
			SearchRoot[0] = PHONE_FOLDER;
			root_num++;
			if (ap->mf_Status.iStorageState & MYFILE_MMC) {
				SearchRoot[root_num] = MEMORY_FOLDER;
				root_num++;
			}
			do_search_all = EINA_FALSE;
		} else {
			SearchRoot[0] = ap->mf_Status.path->str;
			root_num++;
		}
		mf_debug("search root is %s text is [%s]", ap->mf_Status.path->str, text);
		char *new_desc = NULL;
		/*Start Search routine*/
		if (!mf_search_start(ap->mf_Status.search_handler, SearchRoot, root_num, \
		                     ((text) ? text : NULL), MF_SEARCH_OPTION_DEF, (void *)ap, NULL, mf_tray_item_category_none, MF_CATEGOR_SEARCH_ITEM_COUNT)) {
			/*generate the popup used to show search path
			**it's sure that new_desc is not NULL even if original path is NULL*/
			new_desc = mf_fm_svc_wrapper_translate_path(ap->mf_Status.path->str, MF_TRANS_OPTION_POPUP);
			ap->mf_MainWindow.pProgressPopup = mf_popup_center_processing(ap, MF_LABEL_PROCESSING, LABEL_CANCEL, __mf_search_bar_stop_cb, ap, EINA_TRUE);
		} else {
			ap->mf_Status.flagSearchStart = EINA_FALSE;
		}

		if (new_desc != NULL) {
			free(new_desc);
			new_desc = NULL;
		}

		if (text != NULL) {
			free(text);
			text = NULL;
		}

		ap->mf_FileOperation.search_IME_hide_timer = NULL;
	}
	return ECORE_CALLBACK_CANCEL;
}

void mf_search_bar_enter_search_routine(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);

	if (ap->mf_Status.more == MORE_RENAME) {
		mf_callback_rename_save_cb(ap, NULL, NULL);
		return;
	}

	if (ap->mf_Status.more != MORE_DEFAULT) {
		return;
	}

	if (ap->mf_FileOperation.search_IME_hide_timer != NULL) {
		ecore_timer_del(ap->mf_FileOperation.search_IME_hide_timer);
		ap->mf_FileOperation.search_IME_hide_timer = NULL;
	}

	ap->mf_Status.more = MORE_SEARCH;
	search_all_flag = EINA_FALSE;

	mf_search_view_create(ap);
	MF_TRACE_END;
}

void mf_search_bar_search_started_callback(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input data error");

	if (ap->mf_Status.flagSearchStart == EINA_TRUE) {
		return;
	} else {
		ap->mf_Status.flagSearchStart = EINA_TRUE;
	}
	char *text = g_strdup(elm_object_text_get(ap->mf_MainWindow.pSearchEntry));
	if (text == NULL || strlen(text) == 0 || strlen(g_strstrip(text)) == 0) {
		g_free(text);
		text = NULL;
		SAFE_FREE_CHAR(ap->mf_Status.search_filter);
		ap->mf_Status.flagSearchStart = EINA_FALSE;
		elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
		return;
	}

	g_free(text);
	text = NULL;

	SAFE_FREE_CHAR(ap->mf_Status.search_filter);
	ap->mf_Status.search_filter = g_strdup(elm_object_text_get(ap->mf_MainWindow.pSearchEntry));
	if (ap->mf_Status.more == MORE_RENAME) {
		mf_callback_rename_save_cb(ap, NULL, NULL);
		ap->mf_Status.flagSearchStart = EINA_FALSE;
		return;
	}

	if (ap->mf_Status.search_handler > 0) {
		mf_search_finalize(&ap->mf_Status.search_handler);
	}
	int ret = mf_search_init(&ap->mf_Status.search_handler);
	mf_retm_if(ret < 0, "Fail to mf_search_init()");

	/*delete guide text label in the box*/
	elm_box_clear(ap->mf_MainWindow.pNaviBox);
	ap->mf_MainWindow.pNaviGengrid = NULL;
	ap->mf_MainWindow.pNaviGenlist = NULL;

	if (ap->mf_FileOperation.sync_pipe != NULL) {
		ecore_pipe_del(ap->mf_FileOperation.sync_pipe);
		ap->mf_FileOperation.sync_pipe = NULL;
	}
	ap->mf_FileOperation.sync_pipe = ecore_pipe_add(__mf_search_bar_pipe_cb, ap);

	if (ap->mf_FileOperation.sync_pipe == NULL) {
		mf_debug("add pipe failed");
	}
	/*this is to init global variable to ensure the first message can be transmitted correctly*/
	/*flagSearchMsg is to indicate the condition wait to sync data of threads*/
	pthread_mutex_lock(&gLockSearchMsg);
	flagSearchMsg = 1;
	pthread_mutex_unlock(&gLockSearchMsg);

	elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
	elm_object_focus_allow_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
	/*delete guide text label in the box*/
	ap->mf_FileOperation.search_IME_hide_timer = ecore_timer_add(MF_SEARCH_TIMER_INTERVAL, (Ecore_Task_Cb)__mf_search_bar_idle_search_start, ap);
	mf_debug("enter pressed");
	MF_TRACE_END;

	return;
}
