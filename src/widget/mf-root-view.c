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

#include <storage.h>
#include <media_content.h>

#include "mf-object-conf.h"
#include "mf-callback.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-gengrid.h"
#include "mf-genlist.h"
#include "mf-util.h"
#include "mf-ta.h"
#include "mf-resource.h"
#include "mf-launch.h"
#include "mf-tray-item.h"
#include "mf-navi-bar.h"
#include "mf-object.h"
#include "mf-object-item.h"
#include "mf-tray-item.h"
#include "mf-view.h"
#include "mf-search-view.h"
#include "mf-edit-view.h"
#include "mf-focus-ui.h"
#include "mf-media.h"

//static
Elm_Genlist_Item_Class root_category_itc;
static Elm_Object_Item *category_item = NULL;
static Elm_Object_Item *local_index_item = NULL;
static Elm_Object_Item *cloud_index_item = NULL;
static Elm_Object_Item *first_index_item = NULL;
static Evas_Object *root_category = NULL;
static Evas_Object *box_root_category = NULL;

static Elm_Genlist_Item_Class index_itc;
static Evas_Object *__mf_mw_category_content_get(void *data, Evas_Object * obj, const char *part);
Elm_Object_Item *__mf_root_view_group_index_create(void *data, Evas_Object *genlist, const char *text);

void mf_mw_root_view_category_style_set(bool landscape)
{
	MF_TRACE_BEGIN;
	if (landscape) {
		root_category_itc.item_style = "myfile/1icon/no_padding_line_landscape";
		root_category_itc.decorate_all_item_style = NULL;
		root_category_itc.decorate_item_style = NULL;
		root_category_itc.func.text_get = NULL;
		root_category_itc.func.content_get = __mf_mw_category_content_get;
		root_category_itc.func.del	  = NULL;
	} else {
		root_category_itc.item_style = "myfile/1icon/no_padding_line_portraint";
		root_category_itc.decorate_all_item_style = NULL;
		root_category_itc.decorate_item_style = NULL;
		root_category_itc.func.text_get = NULL;
		root_category_itc.func.content_get = __mf_mw_category_content_get;
		root_category_itc.func.del	  = NULL;
	}
	MF_TRACE_END;

}

Elm_Object_Item *mf_mw_root_category_item_prepend(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	Evas_Object *genlist = ap->mf_MainWindow.pNaviGenlist;

	int changed_angle = elm_win_rotation_get(ap->mf_MainWindow.pWindow);
	mf_debug("changed_angle is [%d]", changed_angle);

	if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
		mf_mw_root_view_category_style_set(true);
		elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "float_land", "search_icon");

	} else {
		mf_mw_root_view_category_style_set(false);
		elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "float_port", "search_icon");
	}

	Elm_Object_Item *it = NULL;
	/*if (first_index_item) {
		it = elm_genlist_item_insert_before(genlist, &root_category_itc, ap, NULL, first_index_item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	} else {*/
	it = elm_genlist_item_prepend(genlist, &root_category_itc, ap, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	//}
	elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	MF_TRACE_END;
	return it;
}

void mf_mw_root_category_item_update(void *data)
{
	MF_TRACE_BEGIN;
	if (category_item) {
		mf_retm_if(data == NULL, "data is NULL");
		struct appdata *ap = (struct appdata *)data;
		elm_object_scroll_freeze_push(ap->mf_MainWindow.pNaviGenlist);
		elm_object_item_del(category_item);
		category_item = NULL;
		category_item = mf_mw_root_category_item_prepend(data);
		elm_object_scroll_freeze_pop(ap->mf_MainWindow.pNaviGenlist);
	}
	MF_TRACE_END;
}

void mf_mw_root_category_refresh(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	Evas_Object *category = root_category;

	int changed_angle = elm_win_rotation_get(ap->mf_MainWindow.pWindow);
	mf_debug("category=%p, changed_angle is [%d]", category, changed_angle);

	if (category) {
		if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
			if (box_root_category) {
				evas_object_size_hint_min_set(box_root_category, ELM_SCALE_SIZE(1280), -1);
			}
			edje_object_signal_emit(_EDJ(category), "landscape", "category_frame");
		} else {
			if (box_root_category) {
				evas_object_size_hint_min_set(box_root_category, ELM_SCALE_SIZE(720), -1);
			}
			edje_object_signal_emit(_EDJ(category), "portrait", "category_frame");
		}
	}
	MF_TRACE_END;
}

static Evas_Object *__mf_mw_category_content_get(void *data, Evas_Object * obj, const char *part)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	/*create detail layout*/
	Evas_Object *category = mf_category_create(ap);

	/*
	Evas_Object *bx = elm_box_add(ap->mf_MainWindow.pNaviLayout);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0);
	evas_object_size_hint_min_set(bx, 720, -1);
	elm_box_pack_end(bx, category);

	Evas_Object *scroller = elm_scroller_add(ap->mf_MainWindow.pNaviLayout);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_AUTO,ELM_SCROLLER_POLICY_AUTO);
	elm_object_content_set(scroller, bx);
	elm_object_scroll_lock_x_set(scroller,EINA_TRUE);
	evas_object_show(scroller);

	root_category = category;
	box_root_category = bx;
	mf_mw_root_category_refresh(ap);
	//return category;
	*/
	MF_TRACE_END;
	return category;
}

Elm_Object_Item *mf_mw_root_category_item_append(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	Evas_Object *genlist = ap->mf_MainWindow.pNaviGenlist;
	//category_index_item = __mf_root_view_group_index_create(ap, genlist, MF_LABEL_CATEGORY);

	int changed_angle = elm_win_rotation_get(ap->mf_MainWindow.pWindow);
	mf_debug("changed_angle is [%d]", changed_angle);
	if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
		mf_mw_root_view_category_style_set(true);
		elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "float_land", "search_icon");
	} else {
		mf_mw_root_view_category_style_set(false);
		elm_object_signal_emit(ap->mf_MainWindow.pNaviLayout, "float_port", "search_icon");
	}

	Elm_Object_Item *it = NULL;

	it = elm_genlist_item_append(genlist, &root_category_itc, ap, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	MF_TRACE_END;
	return it;
}

void __mf_root_group_index_style_set()
{
	index_itc.item_style = "group_index";//"custom_groupindex";
	index_itc.func.text_get = mf_genlist_group_index_label_get;
	index_itc.func.content_get = NULL;
	index_itc.func.state_get = NULL;
	index_itc.func.del = mf_genlist_group_index_del;
}

Elm_Object_Item *__mf_root_view_group_index_create(void *data, Evas_Object *genlist, const char *text)
{
	mfItemData_s *m_TempItem = NULL;
	//struct appdata *ap = (struct appdata *)data;

	mf_genlist_create_data(&m_TempItem, text, data);
	m_TempItem->item = elm_genlist_item_append(genlist, &index_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if (first_index_item == NULL) {
		first_index_item = m_TempItem->item;
	}
	elm_genlist_item_select_mode_set(m_TempItem->item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	return m_TempItem->item;
}

void __mf_root_view_local_storage_items_append(void *data)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *genlist = ap->mf_MainWindow.pNaviGenlist;
	local_index_item = __mf_root_view_group_index_create(ap, genlist, MF_LABEL_LOCAL_STORAGE);
	int view_style = mf_view_style_get(ap);

	if (ap->mf_FileOperation.folder_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.folder_list), MYFILE_TYPE_FSNODE);
	}
	if (ap->mf_FileOperation.file_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.file_list), MYFILE_TYPE_FSNODE);
	}
	if (view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
		mf_genlist_create_itc_style(&ap->mf_gl_style.itc, mf_item_itc_type_normal_list_details);
		mf_genlist_create_itc_style(&ap->mf_gl_style.userfolderitc, mf_item_itc_type_normal_list_details);
	} else {
		mf_genlist_create_itc_style(&ap->mf_gl_style.itc, mf_item_itc_type_normal_list);
		mf_genlist_create_itc_style(&ap->mf_gl_style.userfolderitc, mf_item_itc_type_normal_list);
	}
	mf_util_generate_root_view_file_list(ap, &(ap->mf_FileOperation.folder_list), ap->mf_Status.iStorageState);

	ap->mf_Status.flagNoContent = EINA_FALSE;

	mf_genlist_create_list_default_style(genlist, ap, ap->mf_FileOperation.folder_list, NULL);
}

static bool __mf_add_storage_info_to_list(media_storage_h storage, void *data)
{
	struct appdata *ap = (struct appdata *)data;
	char *base_dir = NULL;
	//char *icon_path = NULL;
	char *storage_name = NULL;
	char *storage_uuid = NULL;
//	long long int total = 0;
//	long long int occupied = 0;
//	double totalG = 0;
//	double occupiedG = 0;
	media_content_storage_e storage_type;

	media_storage_get_type(storage, &storage_type);

	if (storage_type == MEDIA_CONTENT_STORAGE_CLOUD) {
		media_storage_get_id(storage, &storage_uuid);
		media_storage_get_name(storage, &storage_name);
		media_storage_get_path(storage, &base_dir);
//		totalG = total / GIGABYTE;
//		occupiedG = occupied / GIGABYTE;

		//mf_error("storage_loaded - %s, %s, %s, %s, %5.2f/%5.2f GB, %d", storage_name, storage_uuid, base_dir, icon_path, occupiedG, totalG, storage_type);
		storage_info *const pNode_cloud_storage = calloc(1, sizeof(storage_info));
		if (!pNode_cloud_storage) {
			mf_error("no memory allocated");
			SAFE_FREE_CHAR(storage_uuid);
			SAFE_FREE_CHAR(storage_name);
			SAFE_FREE_CHAR(base_dir);
			return TRUE;
		}

		pNode_cloud_storage->root_name = storage_name;
		pNode_cloud_storage->type = STORAGE_TYPE_STORAGE;

		pNode_cloud_storage->uuid = storage_uuid;
		pNode_cloud_storage->root_path = base_dir;

//		pNode_cloud_storage->total = totalG;
//		pNode_cloud_storage->occupied = occupiedG;
//		pNode_cloud_storage->icon_path = icon_path;

		ap->storage_list = eina_list_append(ap->storage_list, pNode_cloud_storage);
	}
	SAFE_FREE_CHAR(storage_uuid);
	SAFE_FREE_CHAR(storage_name);
	SAFE_FREE_CHAR(base_dir);
	return TRUE;
}

static int __mf_root_view_cloud_storage(void *data)
{
	mf_retvm_if(data == NULL, -1, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *genlist = ap->mf_MainWindow.pNaviGenlist;

	if (!ap->storage_list) {
		int ret = media_storage_foreach_storage_from_db(NULL, __mf_add_storage_info_to_list, data);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			mf_error("Cannot get clouds, errorcode: %d", ret);
		}
	}
	/*Run once storage list is created. */
	if (ap->storage_list) {
		cloud_index_item = __mf_root_view_group_index_create(ap, genlist, MF_LABEL_CLOUD);
		mf_genlist_cloud_content_set(ap, ap->mf_MainWindow.pNaviGenlist, ap->storage_list);
	}
	return 0;
}

static Evas_Object *__mf_root_view_content_create(void *data)
{
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *content = NULL;

	//int view_style = mf_view_style_get(ap);

	content = ap->mf_MainWindow.pNaviGenlist;
	__mf_root_view_local_storage_items_append(ap);
	__mf_root_view_cloud_storage(ap);

	return content;
}

Eina_Bool mf_root_view_back_cb(void *data, Elm_Object_Item *it)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	mf_retv_if(ap->mf_MainWindow.pNaviBar == NULL, EINA_FALSE);
	if (ap->mf_Status.more == MORE_DEFAULT) {
		if (ap->mf_Status.view_type == mf_view_root) {
			Evas_Object *win = ap->mf_MainWindow.pWindow;
			elm_win_lower(win);
		} else if (ap->mf_Status.view_type == mf_view_root_category) {
			if (ap->mf_MainWindow.pProgressPopup) {
				mf_search_bar_stop(ap);
				SAFE_FREE_OBJ(ap->mf_MainWindow.pProgressPopup);
				return EINA_FALSE;
			}
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
			elm_object_focus_set(ap->mf_MainWindow.pSearchEntry, EINA_FALSE);
			SAFE_FREE_OBJ(ap->mf_MainWindow.pSearchBar);
			ap->mf_MainWindow.pSearchEntry = NULL;
#ifdef MF_SEARCH_UPDATE_COUNT
			ap->mf_Status.flagUpdateSearch = EINA_FALSE;
#endif
			ap->mf_Status.view_type = mf_view_root;
			Evas_Object *view = elm_object_part_content_get(ap->mf_MainWindow.pNaviLayout, "content");
			SAFE_FREE_OBJ(view);
			mf_view_update(ap);
		}
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}

	return EINA_FALSE;

}

Evas_Object *mf_mw_root_genlist_create(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *genlist = NULL;
	genlist = mf_object_create_genlist(ap->mf_MainWindow.pNaviBar);
	evas_object_smart_callback_add(genlist, "language,changed", mf_genlist_gl_lang_changed, data);
	//evas_object_smart_callback_add(genlist, "longpressed", mf_genlist_gl_longpress, ap);

	evas_object_smart_callback_add(genlist, "selected", mf_edit_list_item_sel_cb, ap);
	mf_genlist_create_itc_style(&ap->mf_gl_style.itc, mf_item_itc_type_normal_list);
	mf_genlist_create_itc_style(&ap->mf_gl_style.userfolderitc, mf_item_itc_type_normal_list);
	MF_TRACE_END;
	return genlist;

}

void mf_root_view_append_mmc_item_after_phone(Evas_Object *parent, fsNodeInfo *pNode, void *data)
{
	mf_retm_if(parent == NULL, "pGenlist is NULL");
	mf_retm_if(pNode == NULL, "pNode is NULL");
	mf_retm_if(pNode->path == NULL, "pNode->path is NULL");
	mf_retm_if(pNode->name == NULL, "pNode->name is NULL");
	mf_retm_if(data == NULL, "data is NULL");

	char *real_name = NULL;
	mfItemData_s *m_TempItem = NULL;
	struct appdata *ap = (struct appdata *)data;
	//int view_style = mf_view_style_get(ap);

	real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);
	if (real_name == NULL) {
		return ;
	}

	mf_genlist_create_data(&m_TempItem, real_name, data);

	if (m_TempItem == NULL) {
		free(real_name);
		real_name = NULL;
		return ;
	}

	m_TempItem->thumb_path = strdup(MF_ICON_ITEM_ROOT_MMC);
	m_TempItem->real_thumb_flag = true;

	m_TempItem->file_type = pNode->type;
	m_TempItem->storage_type = pNode->storage_type;
	m_TempItem->list_type = pNode->list_type;
	m_TempItem->thumbnail_type = MF_THUMBNAIL_DEFAULT;
	m_TempItem->pNode = pNode;
	Elm_Object_Item *it = NULL;


	it = elm_genlist_item_append(parent, ap->mf_gl_style.userfolderitc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, mf_genlist_gl_selected, ap);

	//Fixed P131024-02158  and P131011-01834
	pNode->item = it;
	//End

	m_TempItem->item = it;
	free(real_name);
	return;
}

static Eina_Bool __mf_root_view_launch_search(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	mf_ecore_idler_del(ap->mf_Status.float_button_idler);

	mf_search_bar_enter_search_routine(ap, NULL, NULL);

	return ECORE_CALLBACK_CANCEL;
}

static void mf_root_view_enter_search_routine(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	mf_ecore_idler_del(ap->mf_Status.float_button_idler);
	ap->mf_Status.float_button_idler = ecore_idler_add((Ecore_Task_Cb)__mf_root_view_launch_search, ap);
}

void mf_root_view_create(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	MF_TA_ACUM_ITEM_BEGIN("12345 mf_root_view_create", 0);
	MF_TA_ACUM_ITEM_BEGIN("123456 create root view layout", 0);
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	mf_retm_if(ap->mf_MainWindow.pNaviBar == NULL, "ap->mf_MainWindow.pNaviBar is NULL");
	first_index_item = NULL;
	local_index_item = NULL;
	mf_navi_bar_reset_navi_obj(ap);
	ap->mf_Status.pPreNaviItem = ap->mf_MainWindow.pNaviItem;
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", EDJ_NAME);

	ap->mf_MainWindow.pNaviLayout = mf_object_create_layout(ap->mf_MainWindow.pNaviBar, edj_path, "view_layout");
	mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_content_only);

	MF_TA_ACUM_ITEM_END("123456 create root view layout", 0);
	Evas_Object *newContent = mf_mw_root_genlist_create(ap);
	ap->mf_MainWindow.pNaviGenlist = newContent;
	category_item = mf_mw_root_category_item_append(ap);

	//Evas_Object *category = mf_category_create(ap);

	//elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "category", category);
	//mf_category_refresh(ap);


	__mf_root_group_index_style_set();

	__mf_root_view_content_create(ap);
	evas_object_show(newContent);


	mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, newContent);
	MF_TA_ACUM_ITEM_BEGIN("123456 push naviframe item", 0);
	if (ap->mf_Status.pPreNaviItem) {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar, ap->mf_Status.pPreNaviItem, "", NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	} else {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar, NULL, NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	}

	Evas_Object *float_button = eext_floatingbutton_add(ap->mf_MainWindow.pNaviLayout);
	Evas_Object *btn = elm_button_add(float_button);
	if (float_button && btn) {
		memset(edj_path, 0, 1024);
		snprintf(edj_path, 1024, "%s%s/%s", path, "edje", EDJ_IMAGE);

		elm_object_part_content_set(float_button, "button1", btn);
		Evas_Object *search_image = elm_image_add(float_button);
		elm_image_file_set(search_image, edj_path, MF_TITLE_ICON_SEARCH);
		elm_image_resizable_set(search_image, EINA_TRUE, EINA_TRUE);
		evas_object_show(search_image);
		elm_object_part_content_set(btn, "icon", search_image);
		evas_object_smart_callback_add(btn, "clicked", mf_root_view_enter_search_routine, ap);
		elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "search_icon", float_button);
	}
	free(path);

	MF_TA_ACUM_ITEM_END("123456 push naviframe item", 0);

	GString *title = g_string_new(LABEL_MYFILE_CHAP);

	if (title != NULL) {
		SAFE_FREE_CHAR(ap->mf_MainWindow.naviframe_title);
		ap->mf_MainWindow.naviframe_title = g_strdup(title->str);
		g_string_free(title, TRUE);
		title = NULL;
	}
	MF_TA_ACUM_ITEM_BEGIN("123456 mf_navi_add_back_button", 0);

	mf_navi_add_back_button(ap, mf_root_view_back_cb);
	MF_TA_ACUM_ITEM_END("123456 mf_navi_add_back_button", 0);

	/*add control bar for navigation bar*/
	MF_TA_ACUM_ITEM_BEGIN("123456 mf_navi_bar_set_ctrlbar", 0);
	mf_navi_bar_set_ctrlbar(data);
	MF_TA_ACUM_ITEM_END("123456 mf_navi_bar_set_ctrlbar", 0);
	mf_navi_bar_title_content_set(ap, ap->mf_MainWindow.naviframe_title);
	elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_TRUE);

	MF_TA_ACUM_ITEM_END("12345 mf_root_view_create", 0);
	t_end;
	/*temp data free*/
	MF_TRACE_END;

}

