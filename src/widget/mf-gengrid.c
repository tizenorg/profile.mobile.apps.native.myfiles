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
#include "mf-util.h"
#include "mf-callback.h"
#include "mf-launch.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-tray-item.h"
#include "mf-gengrid.h"
#include "mf-callback.h"
#include "mf-object.h"
#include "mf-genlist.h"
#include "mf-popup.h"
#include "mf-context-popup.h"
#include "mf-view.h"
#include "mf-edit-view.h"
#include "mf-thumb-gen.h"
#include "mf-file-util.h"
Elm_Gengrid_Item_Class gic;
#define GENGRID_ITEM_WIDTH	72
static bool g_is_refresh_at_grid = false;

static void __mf_gengrid_icon_clicked(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(event_info == NULL, "event_info is NULL");
	struct appdata *ap = (struct appdata *)data;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	if (item != NULL) {
		elm_gengrid_item_selected_set(item, EINA_FALSE);
	}

	if (ap->mf_MainWindow.pLongpressPopup != NULL) {
		return;
	}

	if (item != NULL) {
		mfItemData_s *selected = (mfItemData_s *) elm_object_item_data_get(item);
		mf_retm_if(selected == NULL, "selected is NULL");
		mf_retm_if(selected->m_ItemName == NULL, "selected->m_ItemName is NULL");
		mf_retm_if(selected->m_ItemName->str == NULL, "selected->m_ItemName->str is NULL");

		if (ap->mf_Status.more == MORE_EDIT
		        || ap->mf_Status.more == MORE_SHARE_EDIT
		        || ap->mf_Status.more == MORE_EDIT_COPY
		        || ap->mf_Status.more == MORE_EDIT_MOVE
		        || ap->mf_Status.more == MORE_EDIT_DELETE
		        || ap->mf_Status.more == MORE_EDIT_UNINSTALL
		        || ap->mf_Status.more == MORE_EDIT_DETAIL
		   ) {
			return;
		}

		if (ap->mf_Status.EnterFrom) {
			SAFE_FREE_CHAR(ap->mf_Status.EnterFrom);
		}
		ap->mf_Status.EnterFrom = g_strdup(selected->m_ItemName->str);
		mf_error("ap->mf_Status.EnterFrom = %s", ap->mf_Status.EnterFrom);
		mf_callback_click_cb(data, MFACTION_CLICK, selected->m_ItemName);
	}
	MF_TRACE_END;
}

void mf_gengrid_realized(void *data, Evas_Object *obj, void *event_info)
{

	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata*)data;

	if (ap->mf_Status.more == MORE_EDIT
	        || ap->mf_Status.more == MORE_SHARE_EDIT
	        || ap->mf_Status.more == MORE_EDIT_COPY
	        || ap->mf_Status.more == MORE_EDIT_MOVE
	        || ap->mf_Status.more == MORE_EDIT_DELETE
	        || ap->mf_Status.more == MORE_EDIT_UNINSTALL
	        || ap->mf_Status.more == MORE_EDIT_DETAIL
	   ) {
		mf_list_data_t *list_data = (mf_list_data_t *)elm_object_item_data_get(event_info);
		elm_object_item_signal_emit(event_info, "check,state,show", "");
		if (list_data->m_checked) {
			elm_object_item_signal_emit(event_info, "check,state,on", "");
		} else {
			elm_object_item_signal_emit(event_info, "check,state,off", "");
		}
		MF_TRACE_END;
		return;
	} else {
		elm_object_item_signal_emit(event_info, "check,state,hide", "");
	}
}

void mf_gengrid_create_grid_items(void *data, Evas_Object *grid, Eina_List *file_list)
{
	MF_TRACE_BEGIN;
	mf_retm_if(grid == NULL, "grid is NULL");
	mf_retm_if(file_list == NULL, "file_list is NULL");
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata*)data;

	Eina_List *l = NULL;
	void  *pData = NULL;
	mfItemData_s *m_TempItem = NULL;

	elm_gengrid_clear(grid);

	mf_debug("****************** count is [%d]", eina_list_count(file_list));
	EINA_LIST_FOREACH(file_list, l, pData) {
		char *real_name = NULL;

		fsNodeInfo *pNode = (fsNodeInfo *)pData;
		if (pNode == NULL) {
			continue;
		}
		real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);

		if (real_name == NULL) {
			continue;
		}
		SECURE_DEBUG("************** gengrid item is [%s]", real_name);
		m_TempItem = (mfItemData_s *) malloc(sizeof(mfItemData_s));
		if (m_TempItem == NULL) {
			free(real_name);
			real_name = NULL;
			continue;
		}
		Elm_Object_Item *it = NULL;

		m_TempItem->m_ItemName = g_string_new(real_name);
		m_TempItem->size = NULL;
		m_TempItem->create_date = NULL;
		m_TempItem->m_checked = FALSE;
		m_TempItem->pCheckBox = NULL;
		m_TempItem->thumb_path = NULL;
		m_TempItem->real_thumb_flag = FALSE;
		m_TempItem->media = NULL;
		m_TempItem->file_type = pNode->type;
		m_TempItem->storage_type = pNode->storage_type;
		m_TempItem->list_type = pNode->list_type;
		m_TempItem->ap = ap;
		if (ap->mf_Status.more == MORE_INTERNAL_COPY_MOVE || ap->mf_Status.more == MORE_INTERNAL_COPY  || ap->mf_Status.more == MORE_INTERNAL_DECOMPRESS
		        || ap->mf_Status.more == MORE_INTERNAL_MOVE || ap->mf_Status.more == MORE_DATA_COPYING || ap->mf_Status.more == MORE_DATA_MOVING) {
			mf_debug();
			it = elm_gengrid_item_append(grid, &gic, m_TempItem, NULL, NULL);
		} else {
			mf_debug();
			it = elm_gengrid_item_append(grid, &gic, m_TempItem, __mf_gengrid_icon_clicked, ap);
		}
		m_TempItem->item = it;
		free(real_name);
		mf_debug();
	}
}

void mf_gengrid_refresh(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	mf_gengrid_create_grid_items(ap, ap->mf_MainWindow.pNaviGengrid, ap->mf_FileOperation.file_list);
}

void mf_gengrid_get_grid_selected_items(Evas_Object *gengrid, Eina_List **list)
{
	MF_TRACE_BEGIN;
	mf_retm_if(gengrid == NULL, "gengrid is NULL");

	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;

	it = elm_gengrid_first_item_get(gengrid);
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->m_checked) {
			*list = eina_list_append(*list, itemData);
		}
		it = elm_gengrid_item_next_get(it);
	}
	MF_TRACE_END;

}

Evas_Object *mf_gengrid_item_icon_get(void *data, Evas_Object *obj, const char *part)
{
	MF_TRACE_BEGIN;
	mfItemData_s *params = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)params->ap;
	mf_retvm_if(params == NULL, NULL, "param is NULL");
	mf_retvm_if(params->m_ItemName == NULL, NULL, "m_ItemName is NULL");
	mf_retvm_if(part == NULL, NULL, "part is NULL");
	mf_retvm_if(obj == NULL, NULL, "obj is NULL");
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", EDJ_IMAGE);
	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *thumb = NULL;

		if (!(params->real_thumb_flag && params->thumb_path)) {
			mf_genlist_get_thumbnail(params);
		}
		if (params->file_type == FILE_TYPE_MUSIC || params->file_type == FILE_TYPE_SOUND) {
			if (params->thumb_path && mf_file_exists(params->thumb_path) && strcmp(params->thumb_path, MF_MUSIC_DEFAULT_THUMBNAIL_FROM_DB) == 0) {
				SAFE_FREE_CHAR(params->thumb_path);
				params->thumb_path = g_strdup(MF_ICON_MUSIC_THUMBNAIL);
				params->thumbnail_type = MF_THUMBNAIL_TYPE_DEFAULT;
			}
		}
		bool is_phone_or_mmc = (params->storage_type == MYFILE_PHONE || params->storage_type == MYFILE_MMC);//Fixed the P131112-03632.
		//Fixed the P131112-03632. If it is at phone and mmc card, it should enter the default thumbnail. so we will use the original image as the default icon.
		bool is_using_original_image_at_phone_or_mmc = (params->thumbnail_type == MF_THUMBNAIL_DEFAULT && params->file_type == FILE_TYPE_IMAGE && is_phone_or_mmc && params->m_ItemName->str);//Fixed the P131112-03632.
		if (params->file_type == FILE_TYPE_DIR) {
			return NULL;
		} else if (params->thumbnail_type == MF_THUMBNAIL_DEFAULT && (is_using_original_image_at_phone_or_mmc == false)) {
			thumb = elm_image_add(obj);
			elm_image_prescale_set(thumb, MF_ICON_SIZE);
			elm_image_fill_outside_set(thumb, EINA_TRUE);
			elm_image_smooth_set(thumb, EINA_FALSE);
			//Checking file size, if more than 4M, don't display it, it will be very slow.
			off_t size = 0;//Comment it, for P140606-04570, some times at efl, it will crash at png file.
			int isOriginalImage = (params->m_ItemName->str && params->thumb_path && strcmp(params->thumb_path, params->m_ItemName->str) == 0);
			if (isOriginalImage == 0) {
				mf_file_attr_get_file_size(params->thumb_path, &size);
				if (size < 4 * 1024 * 1024) {
					elm_image_file_set(thumb, params->thumb_path, NULL);
				}
			} else {
				if (g_is_refresh_at_grid == false) {
					mf_view_refresh_thumbnail_for_other_memory(ap, ap->mf_FileOperation.file_list);
				} else {
					if (params->pNode && params->pNode->thumbnail_path) {//For supporting the otg thumbnail
						mf_debug("params->pNode->thumbnail_path=%s", params->pNode->thumbnail_path);
						thumb = elm_image_add(obj);
						elm_image_prescale_set(thumb, MF_ICON_SIZE);
						elm_image_fill_outside_set(thumb, EINA_TRUE);
						elm_image_smooth_set(thumb, EINA_FALSE);
						elm_image_file_set(thumb, params->pNode->thumbnail_path, NULL);
					} else if (params->thumb_path) {
						thumb = elm_image_add(obj);
						elm_image_prescale_set(thumb, MF_ICON_SIZE);
						elm_image_fill_outside_set(thumb, EINA_TRUE);
						elm_image_smooth_set(thumb, EINA_FALSE);

						if (params->file_type == FILE_TYPE_IMAGE) {
							elm_image_file_set(thumb, edj_path, MF_ICON_IMAGE);
						} else if (params->file_type == FILE_TYPE_VIDEO) {
							elm_image_file_set(thumb, edj_path, MF_ICON_VIDEO);
						} else {
							elm_image_file_set(thumb, params->thumb_path, NULL);
						}
					} else {
						return NULL;
					}
				}
				g_is_refresh_at_grid = true;
			}
		} else {
			thumb = elm_image_add(obj);
			elm_image_prescale_set(thumb, MF_ICON_SIZE);
			elm_image_fill_outside_set(thumb, EINA_TRUE);
			elm_image_smooth_set(thumb, EINA_FALSE);
			//Checking file size, if more than 4M, don't display it, it will be very slow.
			off_t size = 0;//Comment it, for P140606-04570, some times at efl, it will crash at png file.
			int isOriginalImage = (params->m_ItemName->str && params->thumb_path && strcmp(params->thumb_path, params->m_ItemName->str) == 0);
			if (isOriginalImage == 0) {
				mf_file_attr_get_file_size(params->thumb_path, &size);
				if (size < 4 * 1024 * 1024) {
					elm_image_file_set(thumb, params->thumb_path, NULL);
				}
			} else {
				if (g_is_refresh_at_grid == false) {
					mf_view_refresh_thumbnail_for_other_memory(ap, ap->mf_FileOperation.file_list);
				} else {
					if (params->pNode && params->pNode->thumbnail_path) {//For supporting the otg thumbnail
						mf_debug("params->pNode->thumbnail_path=%s", params->pNode->thumbnail_path);
						thumb = elm_image_add(obj);
						elm_image_prescale_set(thumb, MF_ICON_SIZE);
						elm_image_fill_outside_set(thumb, EINA_TRUE);
						elm_image_smooth_set(thumb, EINA_FALSE);
						elm_image_file_set(thumb, params->pNode->thumbnail_path, NULL);
					} else if (params->thumb_path) {
						thumb = elm_image_add(obj);
						elm_image_prescale_set(thumb, MF_ICON_SIZE);
						elm_image_fill_outside_set(thumb, EINA_TRUE);
						elm_image_smooth_set(thumb, EINA_FALSE);

						if (params->file_type == FILE_TYPE_IMAGE) {
							elm_image_file_set(thumb, edj_path, MF_ICON_IMAGE);
						} else if (params->file_type == FILE_TYPE_VIDEO) {
							elm_image_file_set(thumb, edj_path, MF_ICON_VIDEO);
						} else {
							elm_image_file_set(thumb, params->thumb_path, NULL);
						}
					} else {
						return NULL;
					}
				}
				g_is_refresh_at_grid = true;
			}

		}

		if (params->file_type == FILE_TYPE_VIDEO) {
			elm_object_item_signal_emit(params->item, "elm.video.show", "elm");
		} else {
			elm_object_item_signal_emit(params->item, "elm.video.hide", "elm");
		}
		evas_object_show(thumb);
		return thumb;
	} else if (!strcmp(part, "elm.swallow.inner_icon")) {
		Evas_Object *thumb = NULL;

		if (!(params->real_thumb_flag && params->thumb_path)) {
			mf_genlist_get_thumbnail(params);
		}
		bool is_phone_or_mmc = (params->storage_type == MYFILE_PHONE || params->storage_type == MYFILE_MMC);//Fixed the P131112-03632.
		//Fixed the P131112-03632. If it is at phone and mmc card, it should enter the default thumbnail. so we will use the original image as the default icon.
		bool is_using_original_image_at_phone_or_mmc = (params->thumbnail_type == MF_THUMBNAIL_DEFAULT && params->file_type == FILE_TYPE_IMAGE && is_phone_or_mmc && params->m_ItemName->str);//Fixed the P131112-03632.
		if (params->file_type == FILE_TYPE_DIR) {
			thumb = elm_image_add(obj);
			elm_image_file_set(thumb, edj_path, params->thumb_path);
			elm_image_fill_outside_set(thumb, EINA_TRUE);
			elm_image_smooth_set(thumb, EINA_FALSE);
			elm_image_preload_disabled_set(thumb, EINA_FALSE);
		} else if (params->thumbnail_type == MF_THUMBNAIL_DEFAULT && (is_using_original_image_at_phone_or_mmc == false)) {
			thumb = elm_image_add(obj);
			elm_image_file_set(thumb, edj_path, params->thumb_path);
			elm_image_fill_outside_set(thumb, EINA_TRUE);
			elm_image_smooth_set(thumb, EINA_FALSE);
			elm_image_preload_disabled_set(thumb, EINA_FALSE);
		} else {
			return NULL;
		}

		if (params->file_type != FILE_TYPE_DIR) {
			if (params->file_type == FILE_TYPE_VIDEO) {
				elm_object_item_signal_emit(params->item, "elm.video.show", "elm");
			} else {
				elm_object_item_signal_emit(params->item, "elm.video.hide", "elm");
			}
		} else {
			elm_object_item_signal_emit(params->item, "elm.video.hide", "elm");
		}
		evas_object_show(thumb);
		return thumb;
	}
	return NULL;
}

char *mf_gengrid_item_label_get(void *data, Evas_Object * obj, const char *part)
{
	MF_TRACE_BEGIN;
	mfItemData_s *params = (mfItemData_s *) data;
	mf_retv_if(params == NULL, NULL);
	struct appdata *ap = (struct appdata *)params->ap;
	bool is_apply_filename = true;

	mf_error("===================== label get entry");
	if (ap->mf_Status.view_type == mf_view_root_category) {
		if (ap->mf_Status.more != MORE_SEARCH && mf_view_get_pre_state(ap) != MORE_SEARCH) {
			if (ap->mf_Status.category_type == mf_tray_item_category_image
			        || ap->mf_Status.category_type == mf_tray_item_category_video) {
				if (params->thumbnail_type != MF_THUMBNAIL_DEFAULT) {
					is_apply_filename = false;
				}
			}
		}
	}
	if (is_apply_filename && strcmp(part, "elm.text") == 0) {
		/* supporting multi-lang for default folders */
		if (g_strcmp0(params->m_ItemName->str, PHONE_FOLDER) == 0) {
			return g_strdup(mf_util_get_text(MF_LABEL_DEVICE_MEMORY));
		} else if (g_strcmp0(params->m_ItemName->str, MEMORY_FOLDER) == 0) {
			return g_strdup(mf_util_get_text(MF_LABEL_SD_CARD));
		} else {
			char *file_name = NULL;
			if (params->ap->mf_Status.more == MORE_SEARCH && params->ap->mf_Status.search_filter) {
				char *markup_name = NULL;
				bool res = false;
				markup_name = (char *)mf_util_search_markup_keyword(mf_file_get(params->m_ItemName->str), params->ap->mf_Status.search_filter, &res);
				if (res) {
					MF_TRACE_END;
					file_name = g_strdup(markup_name);
				}
			} else if (params->ap->mf_Status.iExtensionState == MF_EXTENSION_HIDE && !mf_file_attr_is_dir(params->m_ItemName->str)) {
				file_name = mf_fm_svc_get_file_name(params->m_ItemName);
			} else {
				file_name = g_strdup(mf_file_get(params->m_ItemName->str));
			}
			if (file_name != NULL) {//Fixed P131107-03138
				int length = strlen(file_name);
				if (length > 10) { //fix P131125-01330 by ray
					char *file_name_tmp = (char*) malloc(length + 5);
					if (file_name_tmp == NULL) {
						SAFE_FREE_CHAR(file_name);
						return strdup(_(""));
					}
					memset(file_name_tmp, 0, sizeof(*file_name_tmp));
					snprintf(file_name_tmp, length + 5 - 1, " %s ", file_name);
					SAFE_FREE_CHAR(file_name);
					if (file_name_tmp) {
						char *translate_label = elm_entry_utf8_to_markup(file_name_tmp);
						if (translate_label) {
							SAFE_FREE_CHAR(file_name_tmp);
							mf_error("translate_label1=%s", translate_label);
							return translate_label;
						}
					}
					return file_name_tmp;
				} else {
					if (file_name) {
						char *translate_label = elm_entry_utf8_to_markup(file_name);
						if (translate_label) {
							SAFE_FREE_CHAR(file_name);
							mf_error("translate_label2=%s", translate_label);
							return translate_label;
						}
					}
					return file_name;
				}
			} else {
				return strdup(_(""));
			}
		}
	}  else {
		return strdup(_(""));
	}
	MF_TRACE_END;
}

void mf_gengrid_item_del(void *data, Evas_Object * obj)
{
	mfItemData_s *params = (mfItemData_s *) data;
	assert(params);
	if (params->m_ItemName) {
		g_string_free(params->m_ItemName, TRUE);
		params->m_ItemName = NULL;
	}
	if (params->thumb_path) {
		free(params->thumb_path);
		params->thumb_path = NULL;
	}
	if (params->create_date) {
		free(params->create_date);
		params->create_date = NULL;
	}
	if (params->size) {
		free(params->size);
		params->size = NULL;
	}
	if (params->item) {
		params->item = NULL;
	}
	if (params->media) {
		media_info_cancel_thumbnail(params->media);
		media_info_destroy(params->media);
		params->media = NULL;
	}
	free(params);
	return;
}



void mf_gengrid_gen_style_set()
{
	gic.item_style = "custom/myfile";
	gic.func.text_get = mf_gengrid_item_label_get;
	gic.func.content_get = mf_gengrid_item_icon_get;
	gic.func.state_get = NULL;
	gic.func.del = mf_gengrid_item_del;
}

Evas_Object *mf_gengrid_create(Evas_Object *parent, void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	mf_retvm_if(data == NULL, NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;

	Evas_Object *grid = NULL;
	grid = mf_gengrid_create_grid(parent);
	int gengrid_len = eina_list_count(ap->mf_FileOperation.file_list) + eina_list_count(ap->mf_FileOperation.folder_list);
	mf_error("gengrid length = %d", gengrid_len);
	mf_gengrid_align_set(grid, gengrid_len);
	mf_gengrid_gen_style_set();
	return grid;
}

Evas_Object *mf_gengrid_create_grid(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas_Object *grid = NULL;
	grid = elm_gengrid_add(parent);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	//elm_object_focus_set(grid, EINA_FALSE);
	elm_object_focus_allow_set(grid, EINA_TRUE);

	elm_gengrid_horizontal_set(grid, EINA_FALSE);
	elm_scroller_bounce_set(grid, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(grid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_gengrid_multi_select_set(grid, EINA_TRUE);
	evas_object_show(grid);
	g_is_refresh_at_grid = false;
	return grid;
}

void mf_gengrid_create_list_default_style(Evas_Object *pGengrid, void *data, Eina_List *dir_list,
        Eina_List *file_list)
{

	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(pGengrid == NULL, "pGengrid is NULL");
	/*0.    variable definition and parameter check*/
	struct appdata *ap = (struct appdata *)data;
	Elm_Object_Item *it = NULL;
	fsNodeInfo *pNode = NULL;
	Eina_List *l = NULL;

	EINA_LIST_FOREACH(dir_list, l, pNode) {
		if (pNode) {
			it = mf_view_item_append(pGengrid, pNode, ap);
			mf_error();
			if (ap->mf_Status.ToTop) {
				elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
				ap->mf_Status.ToTop = false ;
			}
		}
	}
	/*	add file items into the genlist */

	EINA_LIST_FOREACH(file_list, l, pNode) {
		if (pNode) {
			mf_view_item_append(pGengrid, pNode, ap);
		}
	}
}

void mf_gengrid_thumbs_longpressed(void *data, Evas_Object *obj,
                                   void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	mf_retm_if(event_info == NULL, "event_info is NULL");

	Elm_Object_Item *selected =  elm_object_item_data_get(event_info);
	mf_retm_if(selected == NULL, "selected is NULL");
	//elm_gengrid_item_selected_set(event_info, EINA_FALSE);//Fixed P131126-05278 ,Longpress, don't need to set as false.
	if (ap->mf_Status.more == MORE_DEFAULT || ap->mf_Status.more == MORE_SEARCH) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
		ap->mf_MainWindow.pLongpressPopup = mf_popup_create_operation_item_pop(selected);
	}
}

void mf_gengrid_gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
	elm_gengrid_realized_items_update(obj);
}


Evas_Object *mf_gengrid_create_list(void *data, Evas_Object *parent)
{

	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	struct appdata *ap = (struct appdata *)data;
	Evas_Object *gengrid = NULL;
	Eina_List *file_list = NULL;
	Eina_List *dir_list = NULL;


	if (ap->mf_Status.view_type == mf_view_root_category) {
		file_list = ap->mf_FileOperation.category_list;
	} else {
		file_list = ap->mf_FileOperation.file_list;
		dir_list = ap->mf_FileOperation.folder_list;
	}

	gengrid = mf_gengrid_create(parent, ap);

	evas_object_smart_callback_add(gengrid, "language,changed", mf_gengrid_gl_lang_changed, ap);

	//evas_object_smart_callback_add(gengrid, "longpressed",
	//			       mf_gengrid_thumbs_longpressed, ap);
	evas_object_smart_callback_add(gengrid, "realized", mf_gengrid_realized, ap);
	elm_gengrid_item_size_set(gengrid, MF_HD_GENGRID_ITEM_WIDTH, MF_HD_GENGRID_ITEM_HEIGTH);

	if (gengrid == NULL) {
		return NULL;
	}
	//elm_object_signal_callback_add(gengrid, "selected", "*", __mf_gengrid_icon_clicked, ap);
	evas_object_smart_callback_add(gengrid, "selected", __mf_gengrid_icon_clicked, ap);
	mf_gengrid_create_list_default_style(gengrid, ap, dir_list, file_list);
	MF_TRACE_END;
	return gengrid;
}

void mf_gengrid_align_set(Evas_Object *gengrid, int count)
{
	app_device_orientation_e  rotate_mode;
	rotate_mode = app_get_device_orientation();

	switch (rotate_mode) {
	case APP_DEVICE_ORIENTATION_270:
	case APP_DEVICE_ORIENTATION_90:
		if (count < 7) {
			elm_gengrid_align_set(gengrid, 0.0, 0.0);
		} else {
			elm_gengrid_align_set(gengrid, 0.5, 0.0);
		}
		break;
	case APP_DEVICE_ORIENTATION_180:
	case APP_DEVICE_ORIENTATION_0 :
	default:
		if (count < 4) {
			elm_gengrid_align_set(gengrid, 0.0, 0.0);
		} else {
			elm_gengrid_align_set(gengrid, 0.5, 0.0);
		}
		break;
	}

}
