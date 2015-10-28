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
#include "mf-genlist.h"
#include "mf-navi-bar.h"
#include "mf-object-item.h"
#include "mf-object.h"
#include "mf-navi-bar.h"
#include "mf-view.h"
#include "mf-edit-view.h"
#include "mf-download-apps-view.h"
#include "mf-file-util.h"


static Eina_List *edit_folder_list = NULL;
static Eina_List *edit_file_list = NULL;
static Eina_Bool selected_all = EINA_FALSE;
static int edit_count = 0;

static Evas_Smart_Cb edit_select_all_cb = NULL;
static mf_edit_select_info select_info_func = NULL;
static Evas_Object *pSelectAllLayout = NULL;
static Evas_Object *pSelectAllCheckBox = NULL;

void mf_edit_select_all_callback_set(Evas_Smart_Cb func)
{
	edit_select_all_cb = func;
}

void mf_edit_select_info_func_set(mf_edit_select_info func)
{
	select_info_func = func;
}

void mf_edit_select_all_check_set(Eina_Bool state)
{
	elm_check_state_set(pSelectAllCheckBox, state);
}

void mf_edit_folder_list_append(void *data)
{
	if (edit_folder_list) {
		if (eina_list_data_find(edit_folder_list, data) == NULL)
			edit_folder_list = eina_list_append(edit_folder_list, data);
	} else {
		edit_folder_list = eina_list_append(edit_folder_list, data);
	}
}

void mf_edit_folder_list_clear()
{
	eina_list_free(edit_folder_list);
	edit_folder_list = NULL;
}

int mf_edit_folder_list_get_length()
{
	return eina_list_count(edit_folder_list);
}

void mf_edit_folder_list_item_remove(void *data)
{
	edit_folder_list = eina_list_remove(edit_folder_list, data);
}

bool mf_edit_folder_list_item_exists(void *data)
{
	if (edit_folder_list) {
		if (eina_list_data_find(edit_folder_list, data) != NULL) {
			return true;
		}
	}
	return false;
}
Eina_List * mf_edit_folder_list_get()
{
	return edit_folder_list;
}

void mf_edit_file_list_append(void *data)
{
	if (edit_file_list) {
		if (eina_list_data_find(edit_file_list, data) == NULL)
			edit_file_list = eina_list_append(edit_file_list, data);
	} else {
		edit_file_list = eina_list_append(edit_file_list, data);
	}
}

void mf_edit_file_list_clear()
{
	eina_list_free(edit_file_list);
	edit_file_list = NULL;
}

int mf_edit_file_list_get_length()
{
	return eina_list_count(edit_file_list);
}

void mf_edit_file_list_item_remove(void *data)
{
	edit_file_list = eina_list_remove(edit_file_list, data);
}

bool mf_edit_file_list_item_exists(void *data)
{
	if (edit_file_list) {
		if (eina_list_data_find(edit_file_list, data) != NULL) {
			return true;
		}
	}
	return false;
}
Eina_List * mf_edit_file_list_get()
{
	return edit_file_list;
}

int mf_edit_file_count_get()
{
	int count = mf_edit_folder_list_get_length() + mf_edit_file_list_get_length();
	return count;
}

void mf_edit_count_set(int count)
{
	edit_count = count;
}

int mf_edit_count_get()
{
	return edit_count;
}

void mf_edit_select_all_set(Eina_Bool select_all_state)
{
	selected_all = select_all_state;
}

Eina_Bool mf_edit_select_all_get()
{
	return selected_all;
}

Eina_List *mf_edit_get_all_selected_files()
{
	MF_TRACE_BEGIN;

	Eina_List *select_list = NULL;

	Eina_List *l = NULL;

	mfItemData_s *item_data = NULL;
	Elm_Object_Item *it = NULL;
	EINA_LIST_FOREACH(edit_folder_list, l, it) {
		if (it) {
			item_data = elm_object_item_data_get(it);
			if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
				GString *pTemp = g_string_new(item_data->m_ItemName->str);
				select_list = eina_list_append(select_list, pTemp);
			}
		}
	}
	EINA_LIST_FOREACH(edit_file_list, l, it) {
		if (it) {
			item_data = elm_object_item_data_get(it);
			if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
				GString *pTemp = g_string_new(item_data->m_ItemName->str);
				select_list = eina_list_append(select_list, pTemp);
			}
		}
	}
	MF_TRACE_END;
	return select_list;

}

void *mf_edit_file_list_item_get(int index)
{
	Elm_Object_Item *it = NULL;
	void *data = NULL;
	if (edit_file_list) {
		it = eina_list_nth(edit_file_list, index);
		if (it) {
			data = elm_object_item_data_get(it);
		}
	}
	return data;
}

void *mf_edit_folder_list_item_get(int index)
{
	Elm_Object_Item *it = NULL;
	void *data = NULL;
	if (edit_folder_list) {
		it = eina_list_nth(edit_folder_list, index);
		if (it) {
			data = elm_object_item_data_get(it);
		}
	}
	return data;
}

void mf_edit_list_item_reset(void *data)
{
	MF_TRACE_BEGIN;

	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	
	Evas_Object *genlist = NULL;
	Elm_Object_Item *it = NULL;
	mf_list_data_t *item_data = NULL;

	genlist = ap->mf_MainWindow.pNaviGenlist;
	mf_edit_folder_list_clear();
	mf_edit_file_list_clear();
	mf_edit_select_all_set(EINA_FALSE);
	mf_edit_count_set(0);

	
	edit_count = elm_genlist_items_count(genlist);
	selected_all = EINA_FALSE;
	it = elm_genlist_first_item_get(genlist);
	while (it) {
		item_data = (mf_list_data_t *)elm_object_item_data_get(it);
		if (item_data->m_checked) {
			if (item_data->file_type == FILE_TYPE_DIR) {
				mf_edit_folder_list_append(it);
			} else {
				mf_edit_file_list_append(it);
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	if (edit_count == mf_edit_file_count_get()) {
		selected_all = EINA_TRUE;
	}
	if (select_info_func != NULL) {//Fixed P140818-02260
		select_info_func(ap);
	}
	mf_edit_select_all_check_set(selected_all);
}

void mf_edit_view_select_info_create(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;
	int count = mf_edit_file_count_get();
	mf_error("============ count is [%d]", count);
	char *label = NULL;

	if (count > 0) {
		char *tmp = mf_util_get_text(MF_LABEL_SELECTED);
		label = g_strdup_printf(tmp, count);
	} else {
		label = g_strdup(MF_LABEL_SELECT_ITEMS);
	}
	mf_object_item_text_set(ap->mf_MainWindow.pNaviItem, label, "elm.text.title");
        SAFE_FREE_CHAR(label);

}

static void mf_edit_item_sel_all_press_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (pSelectAllCheckBox) {
		Eina_Bool state = elm_check_state_get(pSelectAllCheckBox);
		if (state) {
			elm_object_signal_emit(pSelectAllCheckBox, "elm,activate,check,off", "elm");
		}
		else {
			elm_object_signal_emit(pSelectAllCheckBox, "elm,activate,check,on", "elm");
		}

		mf_edit_select_all_check_set(!state);
		edit_select_all_cb(data, NULL, NULL);
	}
}

void mf_edit_item_sel_all_cb(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input parameter data error");
	int view_style = mf_view_style_get(ap);

	mf_list_data_t *it_data = NULL;
	Elm_Object_Item *it;

	if (selected_all) {
		selected_all = EINA_FALSE;
	} else {
		selected_all = EINA_TRUE;
	}
	mf_edit_folder_list_clear();
	mf_edit_file_list_clear();

	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		it = elm_genlist_first_item_get(ap->mf_MainWindow.pNaviGenlist);
		while (it) {
			it_data = elm_object_item_data_get(it);
			if (selected_all) {
				if (it_data->file_type == FILE_TYPE_DIR) {
					mf_edit_folder_list_append(it);
				} else {
					mf_edit_file_list_append(it);
				}
			}
			it_data->m_checked = selected_all;
			if (it_data->pCheckBox) {
				if (it_data->m_checked) {
					elm_object_signal_emit(it_data->pCheckBox, "elm,activate,check,on", "elm");
				} else {
					elm_object_signal_emit(it_data->pCheckBox, "elm,activate,check,off", "elm");
				}
				elm_check_state_set(it_data->pCheckBox, it_data->m_checked);
			}
		//	elm_genlist_item_update(it);
			it = elm_genlist_item_next_get(it);
		}
		//elm_genlist_realized_items_update(ap->mf_MainWindow.pNaviGenlist);
	} else {
		it = elm_gengrid_first_item_get(ap->mf_MainWindow.pNaviGengrid);
		while (it) {
			it_data = elm_object_item_data_get(it);
			if (selected_all) {
				if (it_data->file_type == FILE_TYPE_DIR) {
					mf_edit_folder_list_append(it);
				} else {
					mf_edit_file_list_append(it);
				}
				elm_genlist_item_update(it);
			}
			it_data->m_checked = selected_all;
			if (it_data->m_checked) {
				elm_object_item_signal_emit(it, "check,state,on", "");
			} else {
				elm_object_item_signal_emit(it, "check,state,off", "");
			}
			it = elm_gengrid_item_next_get(it);
		}
	}
	if (select_info_func != NULL) {
		select_info_func(ap);
	}
	//mf_edit_view_select_info_create(ap);
	mf_navi_bar_set_ctrlbar_item_disable(ap->mf_MainWindow.pNaviItem, CTRL_DISABLE_SEND, FALSE);
	MF_TRACE_END;
}

void mf_edit_view_ctrlbar_state_set(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input parameter data error");

	//int count = mf_edit_file_count_get();
	mf_navi_bar_set_ctrlbar_item_disable(ap->mf_MainWindow.pNaviItem, CTRL_DISABLE_SEND, FALSE);
}

void mf_edit_view_select_all_check(int count)
{
	if (count == edit_count) {
		selected_all = EINA_TRUE;
	} else {
		selected_all = EINA_FALSE;
	}

}

void mf_edit_gengrid_item_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;

	mf_list_data_t *selected = (mf_list_data_t *)data;
	mf_retm_if(selected == NULL, "selected is NULL");

	Elm_Object_Item *item = (Elm_Object_Item *) selected->item;

	struct appdata *ap = (struct appdata *)selected->ap;
	mf_retm_if(ap == NULL, "input parameter data error");
	int view_style = mf_view_style_get(ap);
        if (ap->mf_Status.more != MORE_EDIT
                && ap->mf_Status.more != MORE_SHARE_EDIT
	&& ap->mf_Status.more != MORE_EDIT_COPY
	&& ap->mf_Status.more != MORE_EDIT_MOVE
	&& ap->mf_Status.more != MORE_EDIT_DELETE
	&& ap->mf_Status.more != MORE_EDIT_DELETE_RECENT
	&& ap->mf_Status.more != MORE_EDIT_UNINSTALL
	    && ap->mf_Status.more != MORE_EDIT_DETAIL
       ) {
		MF_TRACE_END;
                return;
        }
	if (selected->file_type == FILE_TYPE_DIR) {
		if (mf_edit_folder_list_item_exists(item)) {
			selected->m_checked = false;
			mf_edit_folder_list_item_remove(item);
		} else {
			selected->m_checked = true;
			mf_edit_folder_list_append(item);
		}

	} else {
		if (mf_edit_file_list_item_exists(item)) {
			selected->m_checked = false;
			mf_edit_file_list_item_remove(item);
		} else {
			selected->m_checked = true;
			mf_edit_file_list_append(item);
		}
	}
	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		elm_genlist_item_fields_update(item, "elm.icon.2", ELM_GENLIST_ITEM_FIELD_CONTENT);
	} else {
		elm_gengrid_item_selected_set(item, selected->m_checked);
		if (selected->m_checked) {
			elm_object_item_signal_emit(item, "check,state,on", "");
		} else {
			elm_object_item_signal_emit(item, "check,state,off", "");
		}
	}

	int count = mf_edit_file_count_get();

	mf_error("================== edit_list count is [%d]", count);
	if (count == edit_count) {
		selected_all = EINA_TRUE;
	} else {
		selected_all = EINA_FALSE;
	}

	if (select_info_func != NULL) {
		select_info_func(ap);
	}
	mf_edit_view_ctrlbar_state_set(ap);
	//mf_edit_view_select_info_create(ap);

	MF_TRACE_END;
}
void mf_edit_list_item_sel_by_list_data(mf_list_data_t *selected, Evas_Object * obj,  Eina_Bool is_update_checkbox)
{
	mf_retm_if(selected == NULL, "selected is NULL");
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	mf_retm_if(ap == NULL, "input parameter data error");
	int view_style = mf_view_style_get(ap);

	if (ap->mf_Status.more != MORE_EDIT
		&& ap->mf_Status.more != MORE_SHARE_EDIT
		&& ap->mf_Status.more != MORE_EDIT_COPY
		&& ap->mf_Status.more != MORE_EDIT_MOVE
		&& ap->mf_Status.more != MORE_EDIT_DELETE
		&& ap->mf_Status.more != MORE_EDIT_DELETE_RECENT
		&& ap->mf_Status.more != MORE_EDIT_UNINSTALL
		&& ap->mf_Status.more != MORE_EDIT_DETAIL
	) {
		return;
	}

	if (obj) {
		Evas_Object *popup = evas_object_data_get(obj, "popup"); // Get popup
		if (popup) return;  // If popup exists, do nothing
	}

	if (selected->file_type == FILE_TYPE_DIR) {
		mf_error("select type is DIR");
		if (mf_edit_folder_list_item_exists(selected->item)) {
			selected->m_checked = false;
			mf_edit_folder_list_item_remove(selected->item);
		} else {
			selected->m_checked = true;
			mf_edit_folder_list_append(selected->item);
		}

	} else {
		mf_error("select type is FILE");
		if (mf_edit_file_list_item_exists(selected->item)) {
			mf_error("select file exists, remove from list");
			selected->m_checked = false;
			mf_edit_file_list_item_remove(selected->item);
		} else {
			mf_error("select file not exists, add to list");
			selected->m_checked = true;
			mf_edit_file_list_append(selected->item);
		}
	}

	if (is_update_checkbox) {
		if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
			elm_genlist_item_fields_update(selected->item, "elm.icon.2", ELM_GENLIST_ITEM_FIELD_CONTENT);
		} else {
			if (selected->pCheckBox) {
				selected->m_checked = !(elm_check_state_get(selected->pCheckBox));
				elm_check_state_set(selected->pCheckBox, selected->m_checked);
			}
			elm_gengrid_item_selected_set(selected->item, selected->m_checked);
		}
	}

	int count = mf_edit_file_count_get();

	mf_error("================== edit_list count is [%d], g_mf_edit_count is [%d]", count, edit_count);
	if (count == edit_count) {
		selected_all = EINA_TRUE;
	} else {
		selected_all = EINA_FALSE;
	}
	mf_edit_select_all_check_set(selected_all);
	//mf_edit_view_select_info_create(ap);
	if (select_info_func != NULL) {
		select_info_func(ap);
	}
	mf_edit_view_ctrlbar_state_set(ap);
}

void mf_edit_list_item_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(event_info == NULL, "event_info is NULL");
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(item, FALSE);
	mf_list_data_t *selected = (mf_list_data_t *)elm_object_item_data_get(event_info);
	mf_retm_if(selected == NULL, "selected is NULL");
	mf_edit_list_item_sel_by_list_data(selected, obj, EINA_TRUE);
	MF_TRACE_END;
}

void mf_edit_view_list_update(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;
	int view_style = mf_view_style_get(ap);
	Evas_Object *pGenlist = NULL;
	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		if (ap->mf_MainWindow.pNaviGenlist) {
			if (ap->mf_Status.more == MORE_SHARE_EDIT) {
				pGenlist = ap->mf_MainWindow.pNaviGenlist;
				mf_genlist_set_folder_edit_style(data);
				mfItemData_s *itemData = NULL;
				Elm_Object_Item *it = NULL;
				it = elm_genlist_first_item_get(pGenlist);
				while (it) {
					itemData = elm_object_item_data_get(it);
					mf_error();
					if (itemData) {
						if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
							it = elm_genlist_item_next_get(it);
							continue;
						}
						if (itemData->file_type == FILE_TYPE_DIR) {
							it = elm_genlist_item_next_get(it);
							elm_object_item_del(itemData->item);
						} else {
							it = elm_genlist_item_next_get(it);
						}
					} else {
						it = elm_genlist_item_next_get(it);
					}
				}
			}
#if 0
			//sluggish performance

			pGenlist = ap->mf_MainWindow.pNaviGenlist;
			Elm_Object_Item *it = NULL;
			it = elm_genlist_first_item_get(pGenlist);
			while (it) {
				//mfItemData_s *itemData = NULL;
				//itemData = elm_object_item_data_get(it);
				elm_genlist_item_update(it);
				it = elm_genlist_item_next_get(it);
			}
#else
			//Better performance

			elm_genlist_realized_items_update(ap->mf_MainWindow.pNaviGenlist);
#endif
			edit_count = elm_genlist_items_count(ap->mf_MainWindow.pNaviGenlist);
			selected_all = EINA_FALSE;
		}
	} else {
		if (ap->mf_Status.more == MORE_SHARE_EDIT) {
			Evas_Object *gengrid = ap->mf_MainWindow.pNaviGengrid;;
			mfItemData_s *itemData = NULL;
			Elm_Object_Item *it = NULL;
			it = elm_gengrid_first_item_get(gengrid);
			while (it) {
				itemData = elm_object_item_data_get(it);
				if (itemData) {
					if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
						it = elm_gengrid_item_next_get(it);
						continue;
					}
					if (itemData->file_type == FILE_TYPE_DIR) {
						it = elm_gengrid_item_next_get(it);
						elm_object_item_del(itemData->item);
					}else {
						it = elm_gengrid_item_next_get(it);
					}
				}else {
					it = elm_gengrid_item_next_get(it);
				}
			}
		}
		if (ap->mf_MainWindow.pNaviGengrid) {
			Eina_List *realize_its;
			Elm_Object_Item *it;
			realize_its = elm_gengrid_realized_items_get(ap->mf_MainWindow.pNaviGengrid);
			EINA_LIST_FREE(realize_its, it) {
				if (it) {
					elm_object_item_signal_emit(it, "check,state,show", "");
				}
			}
			edit_count = elm_gengrid_items_count(ap->mf_MainWindow.pNaviGengrid);
			mf_gengrid_align_set(ap->mf_MainWindow.pNaviGengrid, edit_count);	//fix P131202-03352 by ray
			selected_all = EINA_FALSE;
		}
	}
	MF_TRACE_END;
}

void mf_edit_view_select_all_layout_prepend(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	if (ap->mf_MainWindow.pNaviBox) {
		pSelectAllLayout = NULL;
		pSelectAllCheckBox = NULL;
		mf_object_create_select_all_layout(ap->mf_MainWindow.pNaviBar, edit_select_all_cb, (Evas_Object_Event_Cb)mf_edit_item_sel_all_press_cb , ap, &pSelectAllCheckBox, &pSelectAllLayout);
		if (pSelectAllLayout && pSelectAllCheckBox) {
			elm_box_pack_start(ap->mf_MainWindow.pNaviBox, pSelectAllLayout);
		}
	}
}

void mf_edit_view_select_all_layout_remove(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	if (ap->mf_MainWindow.pNaviBox) {
		if (pSelectAllLayout && pSelectAllCheckBox) {
			elm_box_unpack(ap->mf_MainWindow.pNaviBox, pSelectAllLayout);
			SAFE_FREE_OBJ(pSelectAllCheckBox);
			SAFE_FREE_OBJ(pSelectAllLayout);
		}
	}
}

void mf_edit_view_title_btn_add(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	if (ap->mf_Status.view_type != mf_view_root_category) {
		mf_navi_bar_remove_info_box(ap);
	}
	/*create conformant*/

	Evas_Object *allbtn = NULL;

	mf_object_item_part_content_remove(ap->mf_MainWindow.pNaviItem, TITLE_LEFT_BTN);
	mf_object_item_part_content_remove(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN);

	allbtn = mf_navi_bar_select_all_button_create(ap->mf_MainWindow.pNaviBar, edit_select_all_cb, ap);
	elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN, allbtn);

	if (edit_count == 0) {
		elm_object_disabled_set(allbtn, EINA_TRUE);
	}
	mf_navi_bar_title_content_set(ap, MF_LABEL_SELECT_ITEMS);

	mf_navi_bar_reset_ctrlbar(ap);
}

Eina_List *mf_edit_get_selected_folder_list()
{
	Eina_List *select_list = NULL;

	Eina_List *l = NULL;

	mfItemData_s *item_data = NULL;
	Elm_Object_Item *it = NULL;
	EINA_LIST_FOREACH(edit_folder_list, l, it) {
		if (it) {
			item_data = elm_object_item_data_get(it);
			if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
				if (mf_file_exists(item_data->m_ItemName->str)) {
					char *pTemp = g_strdup(item_data->m_ItemName->str);
					select_list = eina_list_append(select_list, pTemp);
				} else {
					mf_edit_folder_list_item_remove(it);
				}
			}
		}
	}
	MF_TRACE_END;
	return select_list;
}

Eina_List *mf_edit_get_selected_file_list()
{
	Eina_List *select_list = NULL;

	Eina_List *l = NULL;

	mfItemData_s *item_data = NULL;
	Elm_Object_Item *it = NULL;
	EINA_LIST_FOREACH(edit_file_list, l, it) {
		if (it) {
			item_data = elm_object_item_data_get(it);
			if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
				if (mf_file_exists(item_data->m_ItemName->str)) {
					char *pTemp = g_strdup(item_data->m_ItemName->str);
					select_list = eina_list_append(select_list, pTemp);
				} else {
					mf_edit_file_list_item_remove(it);
				}
			}
		}
	}
	MF_TRACE_END;
	return select_list;
}

void mf_edit_view_title_button_set(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	if (ap->mf_Status.extra != MORE_SEARCH) {
		mf_naviframe_left_cancel_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_cancel_cb, ap);
		if (ap->mf_Status.more == MORE_EDIT_COPY || ap->mf_Status.more == MORE_EDIT_MOVE) {
			mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_copy_move_cb, ap);
		} else if (ap->mf_Status.more == MORE_EDIT_DELETE) {
			mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_delete_cb, ap);
		} else if (ap->mf_Status.more == MORE_SHARE_EDIT) {
			mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_share_cb, ap);
		} else if (ap->mf_Status.more == MORE_EDIT_DELETE_RECENT) {
			mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_delete_recent_files_confirm_cb, ap);
		} else if (ap->mf_Status.more == MORE_EDIT_UNINSTALL) {
	#ifdef MYFILE_DOWNLOAD_APP_FEATURE
			mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_download_app_uninstall_cb, ap);
	#endif
		} else if (ap->mf_Status.more == MORE_EDIT_DETAIL) {
			mf_debug("detail callback");
			mf_naviframe_right_save_button_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pNaviItem, mf_callback_details_cb, ap);
		}
	}
}

void mf_edit_view_refresh(void *data, Eina_List **file_list, Eina_List **folder_list)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	mf_edit_folder_list_clear();
	mf_edit_file_list_clear();
	mf_edit_select_all_set(EINA_FALSE);
	mf_edit_count_set(0);
	elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_FALSE);
	int view_style = mf_view_style_get(ap);
	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		Evas_Object *genlist = ap->mf_MainWindow.pNaviGenlist;;
		mfItemData_s *itemData = NULL;
		Elm_Object_Item *it = NULL;
		it = elm_genlist_first_item_get(genlist);
		while (it) {
			itemData = elm_object_item_data_get(it);
			if (itemData) {
				if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
					it = elm_genlist_item_next_get(it);
					continue;
				}
				if (itemData->file_type == FILE_TYPE_DIR) {
					if (*folder_list) {
						char *folder_name = NULL;
						Eina_List *l = NULL;
						EINA_LIST_FOREACH(*folder_list, l, folder_name) {
							if (folder_name) {
								if (g_strcmp0(folder_name, itemData->m_ItemName->str) == 0) {
									itemData->m_checked = true;
									mf_edit_folder_list_append(itemData->item);
									elm_genlist_item_update(it);
									*folder_list = eina_list_remove(*folder_list, folder_name);
									SAFE_FREE_CHAR(folder_name);
									break;
								}
							}
						}
					}
					it = elm_genlist_item_next_get(it);
				} else {
					if (*file_list) {
						char *file_name = NULL;
						Eina_List *l = NULL;
						EINA_LIST_FOREACH(*file_list, l, file_name) {
							if (file_name) {
								if (g_strcmp0(file_name, itemData->m_ItemName->str) == 0) {
									itemData->m_checked = true;
									mf_edit_file_list_append(itemData->item);
									elm_genlist_item_update(it);
									*file_list = eina_list_remove(*file_list, file_name);
									SAFE_FREE_CHAR(file_name);
									break;
								}
							}
						}
					}
					it = elm_genlist_item_next_get(it);
				}
			} else {
				it = elm_genlist_item_next_get(it);
			}
		}
		edit_count = elm_genlist_items_count(ap->mf_MainWindow.pNaviGenlist);
	} else {
		Evas_Object *gengrid = ap->mf_MainWindow.pNaviGengrid;;
		mfItemData_s *itemData = NULL;
		Elm_Object_Item *it = NULL;
		it = elm_gengrid_first_item_get(gengrid);
		while (it) {
			itemData = elm_object_item_data_get(it);
			if (itemData) {
				if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
					it = elm_gengrid_item_next_get(it);
					continue;
				}
				if (itemData->file_type == FILE_TYPE_DIR) {
					if (*folder_list) {
						char *folder_name = NULL;
						Eina_List *l = NULL;
						EINA_LIST_FOREACH(*folder_list, l, folder_name) {
							if (folder_name) {
								if (g_strcmp0(folder_name, itemData->m_ItemName->str) == 0) {
									itemData->m_checked = true;
									mf_edit_folder_list_append(itemData->item);
									*folder_list = eina_list_remove(*folder_list, folder_name);
									SAFE_FREE_CHAR(folder_name);
									break;
								}
							}
						}
					}
					it = elm_gengrid_item_next_get(it);
				} else {
					if (*file_list) {
						char *file_name = NULL;
						Eina_List *l = NULL;
						EINA_LIST_FOREACH(*file_list, l, file_name) {
							if (file_name) {
								if (g_strcmp0(file_name, itemData->m_ItemName->str) == 0) {
									itemData->m_checked = true;
									mf_edit_file_list_append(itemData->item);
									*file_list = eina_list_remove(*file_list, file_name);
									SAFE_FREE_CHAR(file_name);
									break;
								}
							}
						}
					}
					it = elm_gengrid_item_next_get(it);
				}
			} else {
				it = elm_gengrid_item_next_get(it);
			}
		}
		edit_count = elm_gengrid_items_count(ap->mf_MainWindow.pNaviGengrid);
	}
	
	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		elm_genlist_realized_items_update(ap->mf_MainWindow.pNaviGenlist);
	} else {
		elm_gengrid_realized_items_update(ap->mf_MainWindow.pNaviGengrid);
	}

	int count = mf_edit_file_count_get();

	mf_error("================== edit_list count is [%d]", count);
	if (count == edit_count) {
		selected_all = EINA_TRUE;
	} else {
		selected_all = EINA_FALSE;
	}
	//mf_edit_view_select_info_create(ap);
	//mf_edit_view_title_btn_add(ap);
	if (select_info_func != NULL) {
		select_info_func(ap);
	}

	mf_edit_view_select_all_layout_prepend(ap);
	if (count == edit_count)
		mf_edit_select_all_check_set(EINA_TRUE);
	else
		mf_edit_select_all_check_set(EINA_FALSE);

	mf_edit_view_title_button_set(ap);

	mf_edit_view_ctrlbar_state_set(ap);
	MF_TRACE_END;
}

void mf_edit_view_create(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	mf_edit_folder_list_clear();
	mf_edit_file_list_clear();
	mf_edit_select_all_set(EINA_FALSE);
	mf_edit_count_set(0);
	char *title = NULL;
	
	elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_FALSE);
	mf_edit_select_all_callback_set(mf_edit_item_sel_all_cb);
	mf_edit_select_info_func_set(mf_edit_view_select_info_create);
	if(ap->mf_Status.extra != MORE_SEARCH) {
		mf_edit_view_select_all_layout_prepend(ap);
		title = g_strdup_printf(mf_util_get_text(MF_LABEL_SELECTED), 0);
	} else {
		title =mf_util_get_text(MF_LABEL_SELECT_ITEMS);
	}
	mf_edit_view_list_update(ap);
	mf_navi_bar_title_content_set(ap, title);
	//mf_edit_view_title_btn_add(ap);
	mf_edit_view_title_button_set(ap);


	mf_navi_bar_set_ctrlbar_item_disable(ap->mf_MainWindow.pNaviItem, CTRL_DISABLE_EDIT_ALL, TRUE);
	MF_TRACE_END;
}

