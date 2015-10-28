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

#include "mf-main.h"
#include "mf-callback.h"
#include "mf-search.h"
#include "mf-object-conf.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-resource.h"
#include "mf-ta.h"
#include "mf-genlist.h"
#include "mf-gengrid.h"
#include "mf-object.h"
#include "mf-navi-bar.h"
#include "mf-view.h"
#include "mf-edit-view.h"
#include "mf-tray-item.h"
#include "mf-setting-view.h"
#include "mf-thumb-gen.h"
#include "mf-file-util.h"
extern Elm_Gengrid_Item_Class gic;

static MORE_TYPE pre_rename = MORE_DEFAULT;
static MORE_TYPE pre_internal_decompress = MORE_DEFAULT;
static MORE_TYPE pre_copy_move = MORE_DEFAULT;
static MORE_TYPE pre_decompress_here = MORE_DEFAULT;
static MORE_TYPE pre_decompress = MORE_DEFAULT;
static MORE_TYPE pre_compress = MORE_DEFAULT;
static MORE_TYPE pre_delete = MORE_DEFAULT;
// static MORE_TYPE pre_launch = MORE_DEFAULT;

void mf_view_state_set_with_pre(void *data, MORE_TYPE state)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	switch (state) {
	case MORE_RENAME:
	case MORE_THUMBNAIL_RENAME:
		pre_rename = ap->mf_Status.more;
		break;
	case MORE_INTERNAL_COPY:
	case MORE_INTERNAL_MOVE:
		pre_copy_move = ap->mf_Status.more;
		break;
	case MORE_INTERNAL_DECOMPRESS:
		pre_internal_decompress = ap->mf_Status.more;
		break;
	case MORE_DECOMPRESS:
		pre_decompress = ap->mf_Status.more;
		break;
	case MORE_DECOMPRESS_HERE:
		pre_decompress_here = ap->mf_Status.more;
		break;
	case MORE_COMPRESS:
		pre_compress = ap->mf_Status.more;
		break;
	case MORE_DELETE:
	case MORE_IDLE_DELETE:
		pre_delete = ap->mf_Status.more;
		break;
	default:
		break;
	}
	ap->mf_Status.more = state;
}

void mf_view_state_reset_state_with_pre(void *data)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	switch (ap->mf_Status.more) {
	case MORE_RENAME:
	case MORE_THUMBNAIL_RENAME:
		ap->mf_Status.more = pre_rename;
		pre_rename = MORE_DEFAULT;
		break;
	case MORE_INTERNAL_COPY:
	case MORE_INTERNAL_MOVE:
		ap->mf_Status.more = pre_copy_move;
		pre_copy_move = MORE_DEFAULT;
		break;
	case MORE_INTERNAL_DECOMPRESS:
		ap->mf_Status.more = pre_internal_decompress;
		pre_internal_decompress = MORE_DEFAULT;
		break;
	case MORE_DECOMPRESS:
		ap->mf_Status.more = pre_decompress;
		pre_decompress = MORE_DEFAULT;
		break;
	case MORE_DECOMPRESS_HERE:
		ap->mf_Status.more = pre_decompress_here;
		pre_decompress_here = MORE_DEFAULT;
		break;
	case MORE_COMPRESS:
		ap->mf_Status.more = pre_compress;
		pre_compress = MORE_DEFAULT;
		break;
	case MORE_DELETE:
	case MORE_IDLE_DELETE:
		ap->mf_Status.more = pre_delete;
		pre_delete = MORE_DEFAULT;
		break;
	default:
		ap->mf_Status.more = MORE_DEFAULT;
		break;
	}
}

MORE_TYPE mf_view_get_pre_state(void *data)
{
	mf_retvm_if(data == NULL, MORE_DEFAULT, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	switch (ap->mf_Status.more) {
	case MORE_RENAME:
	case MORE_THUMBNAIL_RENAME:
		return pre_rename;
	case MORE_INTERNAL_COPY:
	case MORE_INTERNAL_MOVE:
		return pre_copy_move;
	case MORE_INTERNAL_DECOMPRESS:
		return pre_internal_decompress;
	case MORE_DECOMPRESS_HERE:
		return pre_decompress_here;
	case MORE_DECOMPRESS:
		return pre_decompress;
	case MORE_COMPRESS:
		return pre_compress;
	case MORE_DELETE:
	case MORE_IDLE_DELETE:
		return pre_delete;
	default:
		return MORE_DEFAULT;
	}
}

Eina_Bool mf_view_is_root_view(void *data)
{
	mf_retvm_if(data == NULL, EINA_FALSE, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.view_type == mf_view_root) {
		if (ap->mf_Status.more == MORE_DEFAULT || (ap->mf_Status.more == MORE_THUMBNAIL_RENAME && mf_view_get_pre_state(ap) == MORE_DEFAULT)) {
			return EINA_TRUE;
		}
	}
	return EINA_FALSE;
}

Eina_Bool mf_view_is_operating(void *data)
{
	mf_retvm_if(data == NULL, EINA_FALSE, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	int more = ap->mf_Status.more;
	if (more == MORE_DATA_COPYING ||
	    more == MORE_DATA_MOVING ||
	    more == MORE_DATA_DECOMPRESSING ||
	    more == MORE_DELETE ||
	    more ==MORE_COMPRESS) 
    {
		return EINA_TRUE;
	}
	return EINA_FALSE;
}

void mf_view_phone_storage_init(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata*)data;

	Evas_Object *parent = ap->mf_MainWindow.pWindow;

	ap->mf_MainWindow.pNaviBar = mf_navi_bar_create(parent);
	eext_object_event_callback_add(ap->mf_MainWindow.pNaviBar, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ap->mf_MainWindow.pNaviBar, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	evas_object_smart_callback_add(ap->mf_MainWindow.pNaviBar, "title,clicked", mf_callback_naviframe_title_clicked_cb, data);

	if (ap->mf_MainWindow.pNaviBar == NULL) {
		MF_TRACE_END;
		t_end;
		return;
	}
	ap->mf_MainWindow.location = MYFILE_PHONE;
	ap->mf_MainWindow.naviframe_title = g_strdup(mf_util_get_text(MF_LABEL_DEVICE_MEMORY));
	mf_view_reset_record(&ap->mf_MainWindow.record);
	/*insert phone navi into the navi_list */
	t_end;
	MF_TRACE_END;
}

Eina_Bool mf_view_is_item_exists_by_name(void *data, char *name)
{
	mf_error("name is [%s]", name);
	struct appdata *ap = (struct appdata *)data;
//	int view_style = mf_view_style_get(ap);

	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;

	/*get current content*/
	if (ap->mf_MainWindow.pNaviGenlist) {
		it = elm_genlist_first_item_get(ap->mf_MainWindow.pNaviGenlist);
		while (it) {
                        mf_list_data_t *it_data = elm_object_item_data_get(it);
                        if (it_data->list_type != mf_list_normal &&  it_data->list_type != mf_list_recent_files) {
                                it = elm_genlist_item_next_get(it);
                                continue;
                        }
			itemData = elm_object_item_data_get(it);
			if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
                                it = elm_genlist_item_next_get(it);
				continue;
			}
			SECURE_DEBUG("itemData->m_ItemName->str is [%s]", itemData->m_ItemName->str);
			if (g_strcmp0(itemData->m_ItemName->str, name) == 0) {
				return EINA_TRUE;
			}

			it = elm_genlist_item_next_get(it);
		}
	}
	return EINA_FALSE;
}

void mf_view_refresh(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata*)data;
	mf_error(">>>>>>>>>>>> path is [%s]", ap->mf_Status.path->str);
	mf_error(">>>>>>>>>>>> ap->mf_Status.more =[%d], ap->mf_Status.view_type =[%d]", ap->mf_Status.more, ap->mf_Status.view_type);

	mf_view_refresh_thumbnail_destroy();
	mf_error();
	if (ap->mf_Status.more == MORE_DEFAULT || ap->mf_Status.more == MORE_INTERNAL_COPY_MOVE ||
		ap->mf_Status.more == MORE_INTERNAL_COPY || ap->mf_Status.more == MORE_INTERNAL_MOVE ||
		ap->mf_Status.more == MORE_DATA_COPYING || ap->mf_Status.more == MORE_DATA_MOVING ||
		ap->mf_Status.more == MORE_INTERNAL_DECOMPRESS
		|| ap->mf_Status.more == MORE_EDIT_RENAME) {
		mf_error();
		if (ap->mf_Status.view_type == mf_view_root) {
			mf_root_view_create(ap);
		} else if (ap->mf_Status.view_type == mf_view_root_category) {
			mf_category_view_create(ap, true);
		} else if (ap->mf_Status.view_type == mf_view_storage) {
			mf_storage_view_create(ap);
		} else if (ap->mf_Status.view_type == mf_view_recent) {
			mf_recent_view_create(ap);
		} else if (ap->mf_Status.view_type == mf_view_detail) {
			mf_detail_view_create(ap);
		} else {
			mf_normal_view_create(ap);
		}
	} else if (ap->mf_Status.more == MORE_EDIT) {
		mf_callback_edit_cb(ap, NULL, NULL);
	} else if (ap->mf_Status.more == MORE_SHARE_EDIT) {
		mf_callback_share_button_cb(ap, NULL, NULL);
	} else if (ap->mf_Status.more == MORE_SETTING) {
		mf_setting_view_create(ap);
	}
	mf_error();
	ap->mf_Status.flagViewAsRefreshView = EINA_FALSE;
	SAFE_DEL_NAVI_ITEM(&ap->mf_Status.pPreNaviItem);
	t_end;
	MF_TRACE_END;
}

void mf_view_update(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata*)data;

	mf_view_refresh(ap);

	t_end;
	MF_TRACE_END;
}

Elm_Object_Item *mf_view_item_append(Evas_Object *parent, fsNodeInfo *pNode, void *data)
{
	mf_retvm_if(parent == NULL, NULL, "pGenlist is NULL");
	mf_retvm_if(pNode == NULL, NULL, "pNode is NULL");
	mf_retvm_if(pNode->path == NULL, NULL, "pNode->path is NULL");
	mf_retvm_if(pNode->name == NULL, NULL, "pNode->name is NULL");
	mf_retvm_if(data == NULL, NULL, "data is NULL");

	char *real_name = NULL;
	mfItemData_s *m_TempItem = NULL;
	struct appdata *ap = (struct appdata *)data;
	int view_style = mf_view_style_get(ap);

	real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);
	if (real_name == NULL) {
		return NULL;
	}

	mf_genlist_create_data(&m_TempItem, real_name, data);

	if (m_TempItem == NULL) {
		free(real_name);
		real_name = NULL;
		return NULL;
	}

	if (ap->mf_Status.view_type == mf_view_root) {
		if (g_strcmp0(real_name, PHONE_FOLDER) == 0) {
			m_TempItem->thumb_path = strdup(MF_ICON_ITEM_ROOT_PHONE);
			m_TempItem->real_thumb_flag = true;
		} else if (g_strcmp0(real_name, MEMORY_FOLDER) == 0) {
			m_TempItem->thumb_path = strdup(MF_ICON_ITEM_ROOT_MMC);
			m_TempItem->real_thumb_flag = true;
		}
	} else if (ap->mf_Status.view_type == mf_view_storage) {
		if (g_strcmp0(real_name, PHONE_FOLDER) == 0) {
			m_TempItem->thumb_path = strdup(MF_ICON_ITEM_ROOT_PHONE);
			m_TempItem->real_thumb_flag = true;
		} else if (g_strcmp0(real_name, MEMORY_FOLDER) == 0) {
			m_TempItem->thumb_path = strdup(MF_ICON_ITEM_ROOT_MMC);
			m_TempItem->real_thumb_flag = true;
		}
	}

	m_TempItem->file_type = pNode->type;
	m_TempItem->storage_type = pNode->storage_type;
	m_TempItem->list_type = pNode->list_type;
	m_TempItem->thumbnail_type = MF_THUMBNAIL_DEFAULT;
	m_TempItem->pNode = pNode;
	Elm_Object_Item *it = NULL;

	if (view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
		m_TempItem->modify_time = pNode->date;
	}
	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		if (ap->mf_Status.view_type == mf_view_root_category) {
			it = elm_genlist_item_append(parent, ap->mf_gl_style.categoryitc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, mf_genlist_gl_selected, ap);
		} else {
			if (mf_file_attr_is_dir(m_TempItem->m_ItemName->str)) {
				it = elm_genlist_item_append(parent, ap->mf_gl_style.userfolderitc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, mf_genlist_gl_selected, ap);
				if (ap->mf_Status.EnterFrom) {
					if (!g_strcmp0(m_TempItem->m_ItemName->str, ap->mf_Status.EnterFrom)) {
						ap->mf_Status.ToTop = true ;
						SAFE_FREE_CHAR(ap->mf_Status.EnterFrom);
					}
				}
			} else {
				it = elm_genlist_item_append(parent, ap->mf_gl_style.itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, mf_genlist_gl_selected, ap);
				if (ap->mf_Status.EnterFrom) {
					if (!g_strcmp0(m_TempItem->m_ItemName->str, ap->mf_Status.EnterFrom)) {
							ap->mf_Status.ToTop = true ;
							SAFE_FREE_CHAR(ap->mf_Status.EnterFrom);
					}
				}
			}
		}
	} else {
		it = elm_gengrid_item_append(parent, &gic, m_TempItem, mf_edit_gengrid_item_sel_cb, m_TempItem);
	}

	//Fixed P131024-02158  and P131011-01834
	pNode->item = it;
	//End

	m_TempItem->item = it;
	free(real_name);
	return it;
}

Elm_Object_Item *mf_view_item_append_with_data(Evas_Object *parent, mfItemData_s *item_data, void *data, void *itc, Evas_Smart_Cb func,void *user_data)
{
	mf_retvm_if(parent == NULL, NULL, "pGenlist is NULL");
	mf_retvm_if(data == NULL, NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;
	int view_style = mf_view_style_get(ap);
	int hiden_state = 0;
	mf_util_get_pref_value(PREF_TYPE_HIDEN_STATE, &hiden_state);
	if (hiden_state == MF_HIDEN_HIDE) {
		const char *name = mf_file_get(item_data->m_ItemName->str);
		if ((strncmp(name, ".", strlen(".")) == 0)) {
			return NULL;
		}
	}
	Elm_Object_Item *it = NULL;

	item_data->ap = ap;
	if (!(item_data->real_thumb_flag && item_data->thumb_path)) {
		mf_genlist_get_thumbnail(item_data);
	}

	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		it = elm_genlist_item_append(parent, itc, item_data, NULL, ELM_GENLIST_ITEM_NONE, mf_genlist_gl_selected, ap);
	} else {
		it = elm_gengrid_item_append(parent, itc, item_data, func, user_data);
	}

	item_data->item = it;
	return it;
}

void mf_view_item_remove(Evas_Object *parent, const char *path, int type)
{
	mf_debug("path is [%s]", path);
	mf_retm_if(parent == NULL, "parent is NULL");
	mf_retm_if(path == NULL, "path is NULL");

	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;

	if (type == MF_VIEW_STYLE_THUMBNAIL) {
		it = elm_gengrid_first_item_get(parent);
	} else {
		it = elm_genlist_first_item_get(parent);
	}
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
			continue;
		}
		mf_debug("itemData->m_ItemName->str is [%s]", itemData->m_ItemName->str);
		if (g_strcmp0(path, itemData->m_ItemName->str) == 0) {
			elm_object_item_del(it);
			break;
		}

		if (type == MF_VIEW_STYLE_THUMBNAIL) {
			it = elm_gengrid_item_next_get(it);
		} else {
			it = elm_genlist_item_next_get(it);
		}
	}

}

void mf_view_item_remove_by_type(Evas_Object *parent, int storage_type, int view_type)
{
	mf_retm_if(parent == NULL, "parent is NULL");

	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;

	if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
		it = elm_gengrid_first_item_get(parent);
	} else {
		it = elm_genlist_first_item_get(parent);
	}
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->storage_type == storage_type) {
			elm_object_item_del(it);
			mf_view_item_remove_by_type(parent, storage_type, view_type);
			return;
		}
		if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
			it = elm_gengrid_item_next_get(it);
		} else {
			it = elm_genlist_item_next_get(it);
		}
	}

}

void mf_view_items_remove(Evas_Object *parent, int storage, int type)
{
	mf_retm_if(parent == NULL, "parent is NULL");

	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;
	Elm_Object_Item *next_it = NULL;

	if (type == MF_VIEW_STYLE_THUMBNAIL) {
		it = elm_gengrid_first_item_get(parent);
	} else {
		it = elm_genlist_first_item_get(parent);
	}
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
			continue;
		}
		if (mf_fm_svc_wrapper_get_location(itemData->m_ItemName->str) == storage) {
			if (type == MF_VIEW_STYLE_THUMBNAIL) {
				next_it = elm_gengrid_item_next_get(it);
			} else {
				next_it = elm_genlist_item_next_get(it);
			}
			elm_object_item_del(it);
			it = next_it;
		} else {
			if (type == MF_VIEW_STYLE_THUMBNAIL) {
				it = elm_gengrid_item_next_get(it);
			} else {
				it = elm_genlist_item_next_get(it);
			}
		}
	}
}

bool mf_view_same_item_exist(Evas_Object *obj, const char *check_path, mf_obj_type_e type)
{
	Elm_Object_Item *it = NULL;
	mfItemData_s *itemData = NULL;
	mf_retvm_if(obj == NULL, false, "Obj is NULL");
	mf_retvm_if(check_path == NULL, false, "check_path is NULL");
	if (type == mf_obj_genlist) {
		it = elm_genlist_first_item_get(obj);
		while (it) {
			itemData = elm_object_item_data_get(it);
			if (itemData && itemData->m_ItemName && itemData->m_ItemName->str) {
				if (g_strcmp0(itemData->m_ItemName->str, check_path) == 0) {
					return true;
				}
			}
			it = elm_genlist_item_next_get(it);
		}

	} else if (type == mf_obj_gengrid) {
		it = elm_gengrid_first_item_get(obj);
		while (it) {
			itemData = elm_object_item_data_get(it);
			if (itemData && itemData->m_ItemName && itemData->m_ItemName->str) {
				if (g_strcmp0(itemData->m_ItemName->str, check_path) == 0) {
					return true;
				}
			}
			it = elm_gengrid_item_next_get(it);
		}
	}
	return false;
}

char *mf_view_item_data_get(void *data, int data_type)
{
	mf_list_data_t *item_data = (mf_list_data_t *)data;
	int list_type = item_data->list_type;
	if (data_type == mf_list_data_fullpath) {
		char *fullpath = NULL;
		switch (list_type) {
		case mf_list_normal:
		case mf_list_recent_files:
			fullpath = ((mfItemData_s *)item_data)->m_ItemName->str;
			break;

		default:
			fullpath = NULL;
			break;
		}
		return fullpath;
	}
	return NULL;
}

void mf_view_reset_record(oper_record *record)
{
	mf_retm_if(record == NULL, "record is NULL");
	record->location = MYFILE_PHONE;
	record->view_type = mf_view_root;
	record->more = MORE_DEFAULT;
	SAFE_FREE_CHAR(record->path);
	record->path = g_strdup(PHONE_FOLDER);
}

Eina_Bool mf_view_is_search_view(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	if (ap->mf_Status.more == MORE_SEARCH || ap->mf_Status.more == MORE_ADVANCED_SEARCH ||
		mf_view_get_pre_state(ap) == MORE_SEARCH || mf_view_get_pre_state(ap) == MORE_ADVANCED_SEARCH) {
		return EINA_TRUE;
	}

	return EINA_FALSE;
}
int mf_view_style_get(void *data)
{
	mf_retvm_if(data == NULL, MF_VIEW_STYLE_LIST, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	int view_style = MF_VIEW_STYLE_LIST;

	if (ap->mf_Status.view_type == mf_view_root) {
		view_style = MF_VIEW_STYLE_LIST;
	} else if (ap->mf_Status.view_type == mf_view_root_category) {
		/*add preMode!= MORE_SEARCH condition to fix P131122-04484 by wangyan , myfile->images->search->rename searched item->item name is not changed*/
		if (!mf_view_is_search_view(ap)) {
			view_style = ap->mf_Status.flagViewType;//Fix the P131121-01931
		} else {
			view_style = MF_VIEW_STYLE_LIST;
		}
	} else if (mf_view_is_search_view(ap)) {
		view_style = MF_VIEW_STYLE_LIST;
	} else {
		view_style = ap->mf_Status.flagViewType;
	}
	return view_style;
}

Eina_Bool mf_view_item_popup_check(void *data, char *path)
{
	mf_retvm_if(data == NULL, EINA_FALSE, "data is NULL");
	mf_retvm_if(path == NULL, EINA_FALSE, "path is NULL");
	struct appdata *ap = (struct appdata *)data;

	mfItemData_s *item_data = NULL;
	Evas_Object *popup = NULL;
	if (ap->mf_MainWindow.pNewFolderPopup) {
		popup = ap->mf_MainWindow.pNewFolderPopup;
	}
	if (ap->mf_MainWindow.pLongpressPopup) {
		popup = ap->mf_MainWindow.pLongpressPopup;
	}
	if (ap->mf_MainWindow.pDeleteConfirmPopup) {
		popup = ap->mf_MainWindow.pDeleteConfirmPopup;
	}
	if (popup) {
		item_data = (mfItemData_s *)evas_object_data_get(popup, "item_data");
		if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
			if (g_strcmp0(item_data->m_ItemName->str, path) == 0) {
				if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME || ap->mf_Status.more == MORE_EDIT_RENAME) {
					mf_popup_rename_cancel();
				}
				SAFE_FREE_OBJ(popup);
				return EINA_TRUE;
			}
		}
	}
	return EINA_FALSE;
}

void mf_view_search_item_update(void *data, const char *path, char *new_path)
{
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(path == NULL, "path is NULL");
	struct appdata *ap = (struct appdata *)data;
	int view_style = mf_view_style_get(ap);
	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;

	/*get current content*/
	if (view_style == MF_VIEW_STYLE_THUMBNAIL) {
		/*owner should make the followed routine as common function*/
		if (ap->mf_MainWindow.pNaviGengrid) {
			it = elm_gengrid_first_item_get(ap->mf_MainWindow.pNaviGengrid);
            while (it) {
                mf_list_data_t *it_data = elm_object_item_data_get(it);
                if (it_data->list_type != mf_list_normal) {
                    it = elm_gengrid_item_next_get(it);
                    continue;
                }
				itemData = elm_object_item_data_get(it);
				if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
					continue;
				}
				if (g_strcmp0(itemData->m_ItemName->str, path) == 0) {
					g_string_free(itemData->m_ItemName, TRUE);
					itemData->m_ItemName = g_string_new(new_path);
					i18n_udate date = 0;
					int ret = 0;
					ret = mf_file_attr_get_file_mdate(itemData->m_ItemName->str, &date);
					if (ret == MYFILE_ERR_NONE) {
						itemData->modify_time = date;
					}
					if (!mf_is_dir(itemData->m_ItemName->str)) {
						SAFE_FREE_CHAR(itemData->thumb_path);
						itemData->real_thumb_flag = EINA_FALSE;
						mf_genlist_get_thumbnail(itemData);
					}
					elm_object_item_data_set(itemData->item, itemData);
					elm_gengrid_item_update(itemData->item);
					break;
				}
				it = elm_gengrid_item_next_get(it);
			}
		}
	} else {
		/*owner should make the followed routine as common function*/
		if (ap->mf_MainWindow.pNaviGenlist) {
			it = elm_genlist_first_item_get(ap->mf_MainWindow.pNaviGenlist);
            while (it) {
                mf_list_data_t *it_data = elm_object_item_data_get(it);
                if (it_data->list_type != mf_list_normal) {
                    it = elm_genlist_item_next_get(it);
                    continue;
                }
				itemData = elm_object_item_data_get(it);
				if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
					mf_debug();
					continue;
				}
				if (g_strcmp0(itemData->m_ItemName->str, path) == 0) {
					g_string_free(itemData->m_ItemName, TRUE);
					itemData->m_ItemName = g_string_new(new_path);
					i18n_udate date = 0;
					int ret = 0;
					ret = mf_file_attr_get_file_mdate(itemData->m_ItemName->str, &date);
					if (ret == MYFILE_ERR_NONE) {
						itemData->modify_time = date;
					}
					if (!mf_is_dir(itemData->m_ItemName->str)) {
						SAFE_FREE_CHAR(itemData->thumb_path);
						itemData->real_thumb_flag = EINA_FALSE;
						mf_genlist_get_thumbnail(itemData);
					}
					elm_object_item_data_set(itemData->item, itemData);
					elm_genlist_item_update(itemData->item);
					break;
				}
				it = elm_genlist_item_next_get(it);
			}
		}
	}
	/*maintain category_list*/
	if (ap->mf_Status.view_type == mf_view_root_category) {
		mf_util_check_pnode_list_items_exists(&ap->mf_FileOperation.category_list);
	}

}

void mf_view_item_delete_by_name(void *data, const char *name)
{
	struct appdata *ap = (struct appdata *)data;
	int view_style = mf_view_style_get(ap);
	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;

	/*get current content*/
	if (view_style == MF_VIEW_STYLE_THUMBNAIL) {
		/*owner should make the followed routine as common function*/
		if (ap->mf_MainWindow.pNaviGengrid) {
			it = elm_gengrid_first_item_get(ap->mf_MainWindow.pNaviGengrid);
            while (it) {
                mf_list_data_t *it_data = elm_object_item_data_get(it);
                if (it_data->list_type != mf_list_normal) {
                    it = elm_gengrid_item_next_get(it);
                    continue;
                }
				itemData = elm_object_item_data_get(it);
				if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
					continue;
				}
				if (g_strcmp0(itemData->m_ItemName->str, name) == 0) {
					elm_object_item_del(itemData->item);
					break;
				}

				it = elm_gengrid_item_next_get(it);
			}
		}
	} else {
		/*owner should make the followed routine as common function*/
		if (ap->mf_MainWindow.pNaviGenlist) {
			it = elm_genlist_first_item_get(ap->mf_MainWindow.pNaviGenlist);
			while (it) {
                                mf_list_data_t *it_data = elm_object_item_data_get(it);
                                if (it_data->list_type != mf_list_normal) {
                                        it = elm_genlist_item_next_get(it);
                                        continue;
                                }
				itemData = elm_object_item_data_get(it);
				if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
					mf_debug();
					continue;
				}
				if (g_strcmp0(itemData->m_ItemName->str, name) == 0) {
					elm_object_item_del(itemData->item);
					break;
				}

				it = elm_genlist_item_next_get(it);
			}
		}
	}
}

void mf_view_item_delete_by_exists(void *data)
{
	mf_error();
	struct appdata *ap = (struct appdata *)data;
	if (ap->mf_Status.more == MORE_SEARCH || mf_view_get_pre_state(ap) == MORE_SEARCH) {
		mf_error();
		int view_style = mf_view_style_get(ap);
		mfItemData_s *itemData = NULL;
		Elm_Object_Item *it = NULL;

		/*get current content*/
		if (view_style == MF_VIEW_STYLE_THUMBNAIL) {
			/*owner should make the followed routine as common function*/
			if (ap->mf_MainWindow.pNaviGengrid) {
				it = elm_gengrid_first_item_get(ap->mf_MainWindow.pNaviGengrid);
				while (it) {
                                        mf_list_data_t *it_data = elm_object_item_data_get(it);
                                        if (it_data->list_type != mf_list_normal) {
                                                it = elm_gengrid_item_next_get(it);
                                                continue;
                                        }
					itemData = elm_object_item_data_get(it);
					if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
						continue;
					}
					if (mf_file_exists(itemData->m_ItemName->str) == 0) {
						it = elm_gengrid_item_next_get(it);
						elm_object_item_del(itemData->item);
						continue;
					}

					it = elm_gengrid_item_next_get(it);
				}
			}
		} else {
			/*owner should make the followed routine as common function*/
			if (ap->mf_MainWindow.pNaviGenlist) {
				it = elm_genlist_first_item_get(ap->mf_MainWindow.pNaviGenlist);
                while (it) {
                    mf_list_data_t *it_data = elm_object_item_data_get(it);
                    if (it_data->list_type != mf_list_normal) {
                        it = elm_genlist_item_next_get(it);
                        continue;
                    }

					itemData = elm_object_item_data_get(it);
					if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
						mf_debug();
						continue;
					}
					if (mf_file_exists(itemData->m_ItemName->str) == 0) {
						it = elm_genlist_item_next_get(it);
						elm_object_item_del(itemData->item);
						continue;
					}
					it = elm_genlist_item_next_get(it);
				}
			}
		}

	}
}

void mf_view_resume(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	if (ap->mf_Status.view_type == mf_view_normal) {
		if (!mf_file_exists(ap->mf_Status.path->str)) {
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNormalPopup);
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
			SAFE_FREE_OBJ(ap->mf_MainWindow.pProgressPopup);
			SAFE_FREE_OBJ(ap->mf_MainWindow.pDeleteConfirmPopup);
			SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
			ap->mf_Status.more = MORE_DEFAULT;
			mf_view_update(ap);
		}
	}
}
