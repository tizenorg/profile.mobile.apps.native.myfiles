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

#include <math.h>
#include <media_content.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "mf-main.h"
#include "mf-object-conf.h"
#include "mf-util.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-fs-util.h"
#include "mf-dlog.h"
#include "mf-ta.h"
#include "mf-delete.h"
#include "mf-launch.h"
#include "mf-resource.h"
#include "mf-callback.h"
#include "mf-object.h"
#include "mf-gengrid.h"
#include "mf-genlist.h"
#include "mf-navi-bar.h"
#include "mf-view.h"
#include "mf-context-popup.h"
#include "mf-popup.h"
#include "mf-object-item.h"
#include "mf-edit-view.h"
#include "mf-media.h"
#include "mf-thumb-gen.h"
#include "mf-file-util.h"

#define NUM_OF_GENLIST_STYLES 4
#define NUM_OF_ITEMS 50
#define MF_DATE_FORMAT_DD_MM_YYYY "%d-%b-%Y "
#define MF_DATE_FORMAT_MM_DD_YYYY "%b-%d-%Y "
#define MF_DATE_FORMAT_YYYY_MM_DD "%Y-%b-%d "
#define MF_DATE_FORMAT_YYYY_DD_MM "%Y-%d-%b "
#define MF_TIME_FORMAT_12HOUR "%l:%M%p"
#define MF_TIME_FORMAT_24HOUR "%H:%M"

#define DEF_LABEL_BUF_LEN       (512)
#define MF_LIST_THUMBNAIL_SIZE  46

typedef struct {
	void *ap_data;
	Evas_Object *layout;
	Evas_Object *navi_layout;
	char *curr_path;
	int type;
	Evas_Object *cloud_item_genlist;
	Eina_List *file_list;
	Evas_Object *navi_item;
	Eina_Bool is_root;
} cloud_view_data;

/****	Global definition	****/
static bool g_is_refresh_at_glist = false;
extern int g_mf_create_thumbnail_count;

/***	static function declare	***/
static void __mf_genlist_gl_del(void *data, Evas_Object *obj);
static Eina_Bool __mf_genlist_gl_state_get(void *data, Evas_Object *obj, const char *part);
static void __mf_genlist_widget_storage_selected_cb(void *data, Evas_Object *obj, void *event_info);
static Evas_Object *__mf_genlist_widget_cloud_item_content_get(void *data, Evas_Object *obj, const char *part);
static int mf_genlist_get_cloud_thumbnail_path(const char *filepath, char **thumb_path);

static void _entry_edit_mode_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	evas_object_event_callback_del(obj, EVAS_CALLBACK_SHOW, _entry_edit_mode_show_cb);
	elm_object_focus_set(obj, EINA_TRUE);
}

static Evas_Object *mf_rename_view_create_rename_bar(void *data, Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	mfItemData_s *params = (mfItemData_s *) data;
	mf_retvm_if(params == NULL, NULL, "param is NULL");
	mf_retvm_if(params->m_ItemName == NULL, NULL, "m_ItemName is NULL");

	struct appdata *ap = (struct appdata *)params->ap;
	mf_retvm_if(ap == NULL, NULL, "input parameter data error");


	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	Evas_Object *entry;
	entry = elm_entry_add(parent);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	limit_filter_data.max_char_count = MYFILE_FILE_NAME_CHAR_COUNT_MAX;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
	ap->mf_MainWindow.pEntry = entry;
	evas_object_smart_callback_add(entry, "changed", mf_callback_genlist_imf_changed_cb, params);
	evas_object_smart_callback_add(entry, "preedit,changed", mf_callback_genlist_imf_preedit_change_cb, params);
	evas_object_smart_callback_add(entry, "activated", mf_callback_rename_save_cb, params);
	evas_object_smart_callback_add(entry, "maxlength,reached", mf_callback_max_len_reached_cb, params->ap);

	// the below is sample code for control entry. It is not mandatory.
	GString *filename = NULL;

	if (params->ap->mf_FileOperation.to_rename != NULL) {
		g_string_free(params->ap->mf_FileOperation.to_rename, TRUE);
		params->ap->mf_FileOperation.to_rename = NULL;
	}
	params->ap->mf_FileOperation.to_rename = g_string_new((char *)params->m_ItemName->str);

	/* the below is sample code for control entry. It is not mandatory.*/
	/* set guide text */
	filename = mf_fm_svc_wrapper_get_file_name(ap->mf_FileOperation.to_rename);
	char *guide_text = NULL;
	SAFE_FREE_CHAR(ap->mf_FileOperation.file_name_suffix);
	if (!mf_file_attr_is_dir(ap->mf_FileOperation.to_rename->str)) {
		mf_debug();
		char *ext = NULL;
		char *name_without_ext = NULL;
		name_without_ext = g_strdup(ap->mf_FileOperation.to_rename->str);
		mf_file_attr_get_file_ext(ap->mf_FileOperation.to_rename->str, &ext);
		mf_debug("ext is %s", ext);
		if (ext && strlen(ext) != 0) {
			mf_debug();
			name_without_ext[strlen(name_without_ext) - strlen(ext) - 1] = '\0';
			ap->mf_FileOperation.file_name_suffix = strdup(ext);
			SECURE_DEBUG("name_without_ext is [%s]\n", name_without_ext);
			if (strlen(name_without_ext)) {
				guide_text = elm_entry_utf8_to_markup(mf_file_get(name_without_ext));
			} else {
				guide_text = elm_entry_utf8_to_markup(filename->str);
			}
		} else {
			guide_text = elm_entry_utf8_to_markup(filename->str);
		}

		SAFE_FREE_CHAR(ext);
		SAFE_FREE_CHAR(name_without_ext);
	} else {
		guide_text = elm_entry_utf8_to_markup(filename->str);
	}

	limit_filter_data.max_char_count = MYFILE_FILE_NAME_CHAR_COUNT_MAX;

	elm_entry_markup_filter_append(ap->mf_MainWindow.pEntry, elm_entry_filter_limit_size, &limit_filter_data);
	elm_entry_input_panel_return_key_type_set(ap->mf_MainWindow.pEntry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	elm_entry_entry_set(ap->mf_MainWindow.pEntry, guide_text);
	/*elm_entry_entry_set(entry, mf_file_get(params->m_ItemName->str));*/
	elm_entry_cursor_end_set(entry);
	SAFE_FREE_CHAR(guide_text);
	SAFE_FREE_GSTRING(filename);
	//evas_object_show(entry);
	//elm_object_focus_set(entry, EINA_TRUE);
	evas_object_event_callback_add(entry, EVAS_CALLBACK_SHOW, _entry_edit_mode_show_cb, NULL);
	return entry;

}

void mf_genlist_get_thumbnail(mfItemData_s *params)
{
	MF_TRACE_BEGIN;
	int error_code = 0;
	if (params->file_type == FILE_TYPE_DIR) {
		mf_error("params->thumb_path is [%s]", params->thumb_path);
		//if (params->storage_type == MYFILE_MMC) {
		params->thumb_path = strdup(MF_ICON_FOLDER);
		//} else  {
		//	params->thumb_path = strdup(MF_ICON_FOLDER);
		//}
		params->real_thumb_flag = TRUE;
		params->thumbnail_type = MF_THUMBNAIL_DEFAULT;
	} else {  /* nor directory */
		if (!params->real_thumb_flag) {
			char *icon_path = NULL;
			if (params->media) {
				if (params->thumbnail_create == EINA_TRUE) {
					media_info_cancel_thumbnail(params->media);
					params->thumbnail_create = EINA_FALSE;
				}
				media_info_destroy(params->media);
				params->media = NULL;
			}

			int thumbnail_type = mf_file_attr_get_file_icon(params->m_ItemName->str, &error_code,
			                     MF_ROTATE_PORTRAIT, &icon_path, &params->media);

			if (icon_path && thumbnail_type == MF_THUMBNAIL_TYPE_THUMBNAIL) {
				if (mf_file_exists(icon_path)) {
					params->thumb_path = icon_path;
					params->real_thumb_flag = TRUE;
					params->thumbnail_type = MF_THUMBNAIL_THUMB;
				} else {    /* must be image/video file */

					if (params->file_type == FILE_TYPE_VIDEO) {
						params->thumb_path = strdup(MF_ICON_VIDEO);
					} else if (params->file_type == FILE_TYPE_IMAGE) {
						params->thumb_path = strdup(MF_ICON_IMAGE);
					} else if (params->file_type == FILE_TYPE_MUSIC) {
						params->thumb_path = strdup(MF_ICON_MUSIC_THUMBNAIL);
					} else if (params->file_type == FILE_TYPE_SOUND) {
						params->thumb_path = strdup(MF_ICON_MUSIC_THUMBNAIL);
					} else {
						mf_debug("no file type..!!");
						params->thumb_path = strdup(DEFAULT_ICON);
					}

					params->real_thumb_flag = FALSE;
					params->thumbnail_type = MF_THUMBNAIL_DEFAULT;
					error_code = MYFILE_ERR_GET_THUMBNAIL_FAILED;
					SAFE_FREE_CHAR(icon_path);
				}
			} else {
				params->thumb_path = icon_path;
				params->real_thumb_flag = TRUE;
				params->thumbnail_type = MF_THUMBNAIL_DEFAULT;
			}
		}
	}
	if (error_code != 0) {
		if (params->thumbnail_create == EINA_FALSE) {
			mf_callback_create_thumbnail(params, mf_callback_thumb_created_cb);
			params->thumbnail_create = EINA_TRUE;
		}
	}
}

/*  Label Related   */
static char *__mf_genlist_gl_label_get_lite(void *data, Evas_Object *obj, const char *part)
{

	mfItemData_s *params = (mfItemData_s *) data;
	mf_retv_if(params == NULL, NULL);
	mf_error("part=%s", part);
	if (strcmp(part, "elm.text.main.left") == 0) {
		/* supporting multi-lang for default folders */
		if (g_strcmp0(params->m_ItemName->str, PHONE_FOLDER) == 0) {
			return g_strdup(mf_util_get_text(MF_LABEL_DEVICE_MEMORY));
		} else if (g_strcmp0(params->m_ItemName->str, MEMORY_FOLDER) == 0) {
			return g_strdup(mf_util_get_text(MF_LABEL_SD_CARD));
		} else {
			if (params->ap->mf_Status.more == MORE_SEARCH && params->ap->mf_Status.search_filter) {
				char *markup_name = NULL;
				bool res = false;
				markup_name = (char *)mf_util_search_markup_keyword(mf_file_get(params->m_ItemName->str), params->ap->mf_Status.search_filter, &res);
				if (res) {
					return g_strdup(markup_name);
				}
			}

			if (params->ap->mf_Status.iExtensionState == MF_EXTENSION_HIDE && !mf_file_attr_is_dir(params->m_ItemName->str)) {
				return mf_fm_svc_get_file_name(params->m_ItemName);

			} else {
				return g_strdup(elm_entry_utf8_to_markup(mf_file_get(params->m_ItemName->str)));
			}
		}
		/*
		   } else if (strcmp(part, "elm.uptitle.text") == 0) {
		   return g_strdup(params->m_ItemName->str);
		   } else if (strcmp(part, "elm.slide_base.text") == 0) {
		   return g_strdup(mf_file_get(params->m_ItemName->str));
		   } else if (strcmp(part, "elm.slide.text.1") == 0) {
		   return g_strdup(mf_file_get(params->m_ItemName->str));
		 */
	} else if (strcmp(part, "elm.text.main.left.top") == 0) {
		if (g_strcmp0(params->m_ItemName->str, PHONE_FOLDER) == 0) {
			return g_strdup(mf_util_get_text(MF_LABEL_DEVICE_MEMORY));
		} else if (g_strcmp0(params->m_ItemName->str, MEMORY_FOLDER) == 0) {
			return g_strdup(mf_util_get_text(MF_LABEL_SD_CARD));
		} else if (params->ap->mf_Status.more == MORE_SEARCH && params->ap->mf_Status.search_filter) {
			char *markup_name = NULL;
			bool res = false;
			markup_name = (char *)mf_util_search_markup_keyword(mf_file_get(params->m_ItemName->str), params->ap->mf_Status.search_filter, &res);
			if (res) {
				return g_strdup(markup_name);
			}
		}
		if (params->ap->mf_Status.iExtensionState == MF_EXTENSION_HIDE && !mf_file_attr_is_dir(params->m_ItemName->str)) {
			return mf_fm_svc_get_file_name(params->m_ItemName);

		} else {
			return g_strdup(mf_file_get(params->m_ItemName->str));
		}
	} else if (strcmp(part, "elm.text.sub.left.bottom") == 0) {

		if ((params->ap->mf_Status.more == MORE_SEARCH && params->ap->mf_Status.search_filter)
		        || params->ap->mf_Status.view_type == mf_view_root) {
			int root_len = 0;
			char *new_path = NULL;
			GString *parent_path = NULL;
			parent_path = mf_fm_svc_wrapper_get_file_parent_path(params->m_ItemName);
			if (parent_path) {
				switch (mf_fm_svc_wrapper_get_location(parent_path->str)) {
				case MYFILE_PHONE:
					root_len = strlen(PHONE_FOLDER);
					new_path = g_strconcat(mf_util_get_text(MF_LABEL_DEVICE_MEMORY), parent_path->str + root_len, NULL);
					break;
				case MYFILE_MMC:
					root_len = strlen(MEMORY_FOLDER);
					new_path = g_strconcat(mf_util_get_text(MF_LABEL_SD_CARD), parent_path->str + root_len, NULL);
					break;
				default:
					break;
				}
				SAFE_FREE_GSTRING(parent_path);
			}

			return new_path;
		} else {
			int view_style = mf_view_style_get(params->ap);
			if (view_style != MF_VIEW_SYTLE_LIST_DETAIL) {
				int iSortTypeValue = 0;
				mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &iSortTypeValue);
				if (iSortTypeValue == MYFILE_SORT_BY_SIZE_S2L || iSortTypeValue == MYFILE_SORT_BY_SIZE_L2S) {
					if (params->size) {
						return g_strdup(params->size);
					} else {
						return NULL;
					}
					// }else if (iSortTypeValue == MYFILE_SORT_BY_DATE_O2R || iSortTypeValue == MYFILE_SORT_BY_DATE_R2O) {
				} else {
					if (params->create_date) {
						if (params->list_type == mf_list_normal
						        || params->list_type == mf_list_recent_files) {
							if (params->modify_time) {
								char *tempdate = mf_util_icu_translate(params->ap, params->modify_time, false);
								SAFE_FREE_CHAR(params->create_date);
								params->create_date = tempdate;
							}
						}
						return g_strdup(params->create_date);
					} else {
						return NULL;
					}
				}
			} else {
				if (mf_file_attr_is_dir(params->m_ItemName->str)) {
					Eina_List *file_list = NULL;
					Eina_List *folder_list = NULL;
					int ret = 0;
					ret = mf_fs_oper_read_dir(params->m_ItemName->str, &folder_list, &file_list);
					if (ret == MYFILE_ERR_NONE) {
						int count = 0;
						count = eina_list_count(file_list) + eina_list_count(folder_list);
						mf_util_free_eina_list_with_data(&file_list, MYFILE_TYPE_FSNODE);
						mf_util_free_eina_list_with_data(&folder_list, MYFILE_TYPE_FSNODE);

						if (count == 0 || count == 1) {
							return g_strdup_printf(mf_util_get_text(MF_LABEL_ITEM), count);
						} else {
							return g_strdup_printf("%d %s", count , mf_util_get_text(MF_LABEL_ITEMS));
						}
					} else {
						return g_strdup(_(""));
					}
				} else {
					mf_debug("params->size=%s", params->size);
					if (params->size && strlen(params->size) > 0) {
						return g_strdup(params->size);
					} else {//Fixed the P131202-03332
						off_t size = 0;
						mf_file_attr_get_file_size(params->m_ItemName->str, &size);
						mf_file_attr_get_file_size_info(&(params->size), size);
						if (params->size) {
							return g_strdup(params->size);
						} else {
							return NULL;
						}
					}
				}
			}

		}
	} else if (strcmp(part, "elm.text.sub.right.bottom") == 0) {
		/*if (params->ap->mf_Status.more != MORE_EDIT
		&& params->ap->mf_Status.more != MORE_SHARE_EDIT
		&& params->ap->mf_Status.more != MORE_EDIT_COPY
		    && params->ap->mf_Status.more != MORE_EDIT_DETAIL
		&& params->ap->mf_Status.more != MORE_EDIT_MOVE
		&& params->ap->mf_Status.more != MORE_EDIT_DELETE
		&& !(params->ap->mf_Status.more == MORE_RENAME && mf_view_get_pre_state(params->ap) == MORE_EDIT))*/
		if (!params->create_date && params->modify_time) {
			char *tempdate = mf_util_icu_translate(params->ap, params->modify_time, false);
			SAFE_FREE_CHAR(params->create_date);
			params->create_date = tempdate;
			return g_strdup(params->create_date);
		} else {
			i18n_udate date = 0;
			mf_file_attr_get_file_mdate(params->m_ItemName->str, &date);
			params->modify_time = date;
			char *tempdate = mf_util_icu_translate(params->ap, params->modify_time, false);
			SAFE_FREE_CHAR(params->create_date);
			params->create_date = tempdate;
			return g_strdup(params->create_date);
		}
	} else {
		return g_strdup(_(""));
	}
}

static void __genlist_rename_eraser_clicked_cb(void *data, Evas_Object *obj, void *ei)
{
	mfItemData_s *params = (mfItemData_s *) data;
	Evas_Object *entry = elm_object_item_part_content_get(params->item, "elm.flip.content");
	elm_object_focus_set(entry, EINA_TRUE);
	elm_entry_entry_set(entry, "");
}

static void __mf_genlist_gl_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mfItemData_s *params = (mfItemData_s *) data;

	mf_edit_list_item_sel_by_list_data((mf_list_data_t *)params, NULL, false);
}

/*-----     Icon Related        -----*/
static Evas_Object *__mf_genlist_gl_default_icon_get_lite(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *layout = NULL;
	mfItemData_s *params = (mfItemData_s *) data;
	struct appdata *ap = (struct appdata *)params->ap;
	Evas_Object *thumb = NULL;
	mf_retvm_if(params == NULL, NULL, "param is NULL");
	mf_retvm_if(params->m_ItemName == NULL, NULL, "m_ItemName is NULL");
	mf_retvm_if(part == NULL, NULL, "part is NULL");
	mf_retvm_if(obj == NULL, NULL, "obj is NULL");
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");

	if (!strcmp(part, "elm.icon.folder")) {

		if (ap->mf_Status.view_type == mf_view_root || ap->mf_Status.view_type == mf_view_storage) {
			Evas_Object *check = NULL;
			check = elm_image_add(obj);
			elm_image_prescale_set(check, MF_GENLIST_THUMBNAIL_SIZE);
			elm_image_fill_outside_set(check, EINA_FALSE);
			elm_image_smooth_set(check, EINA_FALSE);
			elm_image_file_set(check, EDJ_IMAGE, params->thumb_path);
			evas_object_repeat_events_set(check, EINA_TRUE);
			evas_object_propagate_events_set(check, EINA_FALSE);
			//elm_layout_content_set(content, "elm.swallow.content", check);
			return check;
		} else {
			return NULL;
		}
	} else if (!strcmp(part, "elm.icon.2")) {
		if (ap->mf_Status.more == MORE_SHARE_EDIT
		        || ap->mf_Status.more == MORE_EDIT_DELETE
		        || ap->mf_Status.more == MORE_EDIT_COPY
		        || ap->mf_Status.more == MORE_EDIT_MOVE
		        || ap->mf_Status.more == MORE_EDIT_DELETE_RECENT
		        || ap->mf_Status.more == MORE_EDIT_UNINSTALL
		        || ap->mf_Status.more == MORE_EDIT_DETAIL) {
			if (ap->mf_Status.extra != MORE_SEARCH) {
				Evas_Object *content = elm_layout_add(obj);
				elm_layout_theme_set(content, "layout", "list/C/type.2", "default");

				Evas_Object *check = NULL;
				check = elm_check_add(obj);
				params->pCheckBox = check;
				elm_object_style_set(check, "default");
				elm_object_focus_set(check, EINA_FALSE);
				if (!params->m_checked) {
					elm_object_signal_emit(check, "elm,activate,check,off", "elm");
				} else {
					elm_object_signal_emit(check, "elm,activate,check,on", "elm");
				}
				elm_check_state_pointer_set(check, &params->m_checked);
				evas_object_repeat_events_set(check, EINA_FALSE);
				evas_object_propagate_events_set(check, EINA_FALSE);
				evas_object_smart_callback_add(check, "changed", __mf_genlist_gl_check_clicked_cb, params);
				elm_layout_content_set(content, "elm.swallow.content", check);
				return content;
			}
		}
		return NULL;

	} else if (!strcmp(part, "elm.icon.1")) {

		mf_debug("file type : %d / storage type : %d", params->file_type, params->storage_type);

		/* Make thumbnail for performance issue */
		if (!(params->real_thumb_flag && params->thumb_path)) {
			mf_genlist_get_thumbnail(params);
		}

		if (params->file_type == FILE_TYPE_MUSIC || params->file_type == FILE_TYPE_SOUND) {
			if (params->thumb_path && mf_file_exists(params->thumb_path) &&
			        strcmp(params->thumb_path, MF_MUSIC_DEFAULT_THUMBNAIL_FROM_DB) == 0) {
				SAFE_FREE_CHAR(params->thumb_path);
				if (params->file_type == FILE_TYPE_MUSIC) {
					params->thumb_path = g_strdup(MF_ICON_MUSIC_THUMBNAIL);
				} else {
					params->thumb_path = g_strdup(MF_ICON_SOUND);
				}

				params->thumbnail_type = MF_THUMBNAIL_TYPE_DEFAULT;
			} else {
				if (params->file_type == FILE_TYPE_MUSIC) {
					params->thumb_path = g_strdup(MF_ICON_MUSIC_THUMBNAIL);
				} else {
					params->thumb_path = g_strdup(MF_ICON_SOUND);
				}

				params->thumbnail_type = MF_THUMBNAIL_TYPE_DEFAULT;
			}
			mf_debug("thumb path : %s, thumb type : %d", params->thumb_path, params->thumbnail_type);
		}

		//layout = mf_object_create_layout(obj, EDJ_GENLIST_NAME, "genlist_content");
		//elm_layout_theme_set(layout, "layout", "list/B/type.2", "default");
		//evas_object_repeat_events_set(layout, EINA_TRUE);

		if (params->thumb_path == NULL) {
			mf_debug("thumb path is NULL");
			const char *temp_thumb = mf_file_attr_get_default_icon_by_type(params->file_type);
			mf_debug("temp thumb path : %s", temp_thumb);

			params->thumb_path = g_strconcat(temp_thumb, NULL);
			params->thumbnail_type = MF_THUMBNAIL_DEFAULT;
		}

		bool is_phone_or_mmc = (params->storage_type == MYFILE_PHONE || params->storage_type == MYFILE_MMC);
		bool is_using_original_image_at_phone_or_mmc =
		    (params->thumbnail_type == MF_THUMBNAIL_DEFAULT && params->file_type == FILE_TYPE_IMAGE &&
		     is_phone_or_mmc && params->m_ItemName->str);

		if (params->thumbnail_type == MF_THUMBNAIL_DEFAULT && (is_using_original_image_at_phone_or_mmc == false)) {
			mf_debug("1");

			thumb = elm_image_add(obj);
			elm_image_prescale_set(thumb, MF_GENLIST_THUMBNAIL_SIZE);
			elm_image_fill_outside_set(thumb, EINA_FALSE);
			elm_image_smooth_set(thumb, EINA_FALSE);

			off_t size = 0;
			int isOriginalImage = (params->m_ItemName->str && params->thumb_path &&
			                       strcmp(params->thumb_path, params->m_ItemName->str) == 0);

			if (isOriginalImage == 0) {
				mf_file_attr_get_file_size(params->thumb_path, &size);
				if (size < 4 * 1024 * 1024) {
					elm_image_file_set(thumb, EDJ_IMAGE, params->thumb_path);
					//elm_object_part_content_set(layout, "default_thumbnail", thumb);
					//elm_layout_content_set(layout, "elm.swallow.content", thumb);
				}
			} else {
				if (g_is_refresh_at_glist == false) {
					mf_debug("1");
					mf_view_refresh_thumbnail_for_other_memory(ap, ap->mf_FileOperation.file_list);
				} else {
					mf_debug("2");
					if (params->pNode && params->pNode->thumbnail_path) { //For supporting the otg thumbnail
						mf_debug("21 params->pNode->thumbnail_path=%s", params->pNode->thumbnail_path);
						//elm_image_file_set(thumb, EDJ_IMAGE, params->pNode->thumbnail_path);
						elm_image_file_set(thumb, params->pNode->thumbnail_path, NULL);
						//elm_object_part_content_set(layout, "default_thumbnail", thumb);
						//elm_layout_content_set(layout, "elm.swallow.content", thumb);
					} else {
						mf_debug("22");
						thumb = elm_image_add(obj);
						elm_image_fill_outside_set(thumb, EINA_TRUE);
						if (params->file_type == FILE_TYPE_IMAGE) {
							elm_image_file_set(thumb, EDJ_IMAGE, MF_ICON_IMAGE);
						} else if (params->file_type == FILE_TYPE_VIDEO) {
							elm_image_file_set(thumb, EDJ_IMAGE, MF_ICON_VIDEO);
						} else if (params->thumbnail_type == MF_THUMBNAIL_DEFAULT) {
							elm_image_file_set(thumb, EDJ_IMAGE, params->thumb_path);
						}
						elm_image_smooth_set(thumb, EINA_FALSE);
						elm_image_preload_disabled_set(thumb, EINA_FALSE);
						//elm_object_part_content_set(layout, "default_thumbnail", thumb);
						//elm_layout_content_set(layout, "elm.swallow.content", thumb);
					}
				}
				g_is_refresh_at_glist = true;
			}
		} else {
			mf_debug("5");
			thumb = elm_image_add(obj);
			elm_image_prescale_set(thumb, MF_GENLIST_THUMBNAIL_SIZE);
			elm_image_fill_outside_set(thumb, EINA_FALSE);
			elm_image_smooth_set(thumb, EINA_FALSE);
			if (is_using_original_image_at_phone_or_mmc) {//Fixed the P131112-03632.
				mf_debug("51");
				//Checking file size, if more than 4M, don't display it, it will be very slow.
				/*off_t size = 0;//Comment it, for P140606-04570, some times at efl, it will crash at png file.
				      mf_file_attr_get_file_size(params->m_ItemName->str, &size);
				      if (size < 4*1024*1024)
				      elm_image_file_set(thumb, params->m_ItemName->str, NULL);*/
				if (g_is_refresh_at_glist == false) {
					mf_view_refresh_thumbnail_for_other_memory(ap, ap->mf_FileOperation.file_list);
				} else {
					mf_debug("52");
					if (params->pNode && params->pNode->thumbnail_path) {//For supporting the otg thumbnail
						mf_debug("53");
						//elm_image_file_set(thumb, EDJ_IMAGE, params->pNode->thumbnail_path);
						elm_image_file_set(thumb, params->pNode->thumbnail_path, NULL);
						//	elm_object_part_content_set(layout, "default_thumbnail", thumb);
						//	elm_layout_content_set(layout, "elm.swallow.content", thumb);
					} else {
						mf_debug("54");
						thumb = elm_image_add(obj);
						elm_image_fill_outside_set(thumb, EINA_TRUE);
						if (params->file_type == FILE_TYPE_IMAGE) {
							elm_image_file_set(thumb, EDJ_IMAGE, MF_ICON_IMAGE);
						} else if (params->file_type == FILE_TYPE_VIDEO) {
							elm_image_file_set(thumb, EDJ_IMAGE, MF_ICON_VIDEO);
						} else if (params->thumbnail_type == MF_THUMBNAIL_DEFAULT) {
							elm_image_file_set(thumb, EDJ_IMAGE, params->thumb_path);
						}
						elm_image_smooth_set(thumb, EINA_FALSE);
						elm_image_preload_disabled_set(thumb, EINA_FALSE);
						//elm_layout_content_set(layout, "elm.swallow.content", thumb);
					}
				}
				g_is_refresh_at_glist = true;
			} else {
				elm_image_file_set(thumb, params->thumb_path, NULL);
			}
			/* elm_object_part_content_set(layout, "thumbnail", thumb); */
			//elm_layout_content_set(layout, "elm.swallow.content", thumb);
		}

		if (params->file_type != FILE_TYPE_DIR) {
			if (params->storage_type == MYFILE_MMC) {
				mf_debug("Do nothing for MMC ??");
				Evas_Object *thumb = elm_image_add(obj);
				elm_image_fill_outside_set(thumb, EINA_TRUE);
				elm_image_smooth_set(thumb, EINA_FALSE);
				if (params->file_type == FILE_TYPE_MUSIC || params->file_type == FILE_TYPE_SOUND) {
					elm_image_file_set(thumb, EDJ_IMAGE, params->thumb_path);
				} else if (params->thumbnail_type == MF_THUMBNAIL_DEFAULT) {
					elm_image_file_set(thumb, EDJ_IMAGE, params->thumb_path);
				} else {
					elm_image_file_set(thumb, params->thumb_path, NULL);
				}
				evas_object_show(thumb);
				return thumb;
			}
		}
		if (params->file_type == FILE_TYPE_VIDEO) {
			elm_object_signal_emit(layout, "elm.video.show", "elm");
		}

		if (params->thumb_path && strstr(params->thumb_path, "thumb_default.png")) {
			mf_debug("Applying default thumbnail!!");
			Evas_Object *layout = elm_layout_add(obj);
			elm_layout_file_set(layout, EDJ_NAME, "default_thumb_bg");
			elm_object_part_content_set(layout, "thumb_swallow", thumb);
			evas_object_show(layout);
			return layout;
		}

		if (strstr(params->m_ItemName->str, "/.") != NULL) {
			if (params->file_type == FILE_TYPE_IMAGE) {
				elm_image_file_set(thumb, EDJ_IMAGE, MF_ICON_IMAGE);
			} else if (params->file_type == FILE_TYPE_VIDEO) {
				elm_image_file_set(thumb, EDJ_IMAGE, MF_ICON_VIDEO);
			} else if (params->thumbnail_type == MF_THUMBNAIL_DEFAULT) {
				elm_image_file_set(thumb, EDJ_IMAGE, params->thumb_path);
			}
			elm_image_smooth_set(thumb, EINA_FALSE);
		}

		evas_object_show(thumb);
	} else if (!strcmp(part, "elm.flip.content")) {	/* this is used when the rename mode is enabled.*/
		Evas_Object *edit_field = NULL;
		edit_field = mf_rename_view_create_rename_bar(params, obj);
		evas_object_propagate_events_set(edit_field, EINA_FALSE);

		return edit_field;
	} else if (!strcmp(part, "elm.flip.eraser")) {
		layout = mf_object_create_button(obj, "editfield_clear", NULL, NULL,
		                                 (Evas_Smart_Cb)__genlist_rename_eraser_clicked_cb, params, EINA_FALSE);

		return layout;
	} else if (!strcmp(part, "elm.flip.icon")) {
		Evas_Object *cancel_btn = mf_object_create_button(obj, NULL, LABEL_CANCEL, NULL,
		                          (Evas_Smart_Cb)mf_callback_cancel_cb, params->ap, EINA_FALSE);
		evas_object_size_hint_min_set(cancel_btn, ELM_SCALE_SIZE(140), 0);
		evas_object_show(cancel_btn);
		evas_object_propagate_events_set(cancel_btn, EINA_FALSE);

		return cancel_btn;
	}
	return thumb;
}

/**	delete related	**/
static void __mf_genlist_gl_del(void *data, Evas_Object *obj)
{
	MF_TRACE_BEGIN;
	mfItemData_s *params = (mfItemData_s *) data;
	assert(params);
	mf_util_normal_item_data_free(&params);
	return;
}

/**	state related	**/
static Eina_Bool __mf_genlist_gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

/**	select related	**/
/**	this function will be splited into several functions	**/
void mf_genlist_gl_selected(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	if (item) {
		elm_genlist_item_selected_set(item, FALSE);
	}

	Evas_Object *popup = evas_object_data_get(obj, "popup");
	if (popup) {
		return;
	}

	if (item != NULL) {
		mfItemData_s *selected = (mfItemData_s *) elm_object_item_data_get(item);

		elm_genlist_item_selected_set(item, FALSE);

		if (ap->mf_Status.more == MORE_EDIT_RENAME) {
			if (ap->mf_Status.view_type == mf_view_root) {
				return;
			}
			ap->mf_FileOperation.rename_item = item;
			mf_callback_idle_rename(selected);
		} else if (ap->mf_Status.more == MORE_RENAME) {
			mf_callback_rename_save_cb(ap, NULL, NULL);
			return;
		} else if (ap->mf_Status.more != MORE_EDIT
		           && ap->mf_Status.more != MORE_SHARE_EDIT
		           && ap->mf_Status.more != MORE_EDIT_COPY
		           && ap->mf_Status.more != MORE_EDIT_MOVE
		           && ap->mf_Status.more != MORE_EDIT_DETAIL
		           && ap->mf_Status.more != MORE_EDIT_DELETE
		           && ap->mf_Status.more != MORE_EDIT_DELETE_RECENT
		          ) {
			if (selected->storage_type == MYFILE_PHONE
			        || selected->storage_type == MYFILE_MMC
			   ) {
				if (!mf_file_exists(selected->m_ItemName->str)) {
					mf_popup_indicator_popup(NULL, mf_util_get_text(MF_LABEL_FILE_NOT_EXIST));
					elm_object_item_del(selected->item);
					return;
				}
			}
			mf_callback_click_cb(data, MFACTION_CLICK, selected->m_ItemName);
		}
	}
	MF_TRACE_END;
}

void mf_genlist_disable_items(Evas_Object *genlist, Eina_Bool disable)
{
	MF_TRACE_BEGIN;
	mf_retm_if(genlist == NULL, "genlist is NULL");

	Elm_Object_Item *it = NULL;

	it = elm_genlist_first_item_get(genlist);
	while (it) {
		elm_object_item_disabled_set(it, disable);
		it = elm_genlist_item_next_get(it);
	}

}

static Evas_Object *__mf_genlist_widget_storage_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *layout = mf_object_create_layout(obj, EDJ_GENLIST_NAME, "genlist_content");
		mf_retvm_if(layout == NULL, NULL, "Failed to create layout");

		Evas_Object *cloud_icon = elm_icon_add(layout);
		mf_retvm_if(cloud_icon == NULL, NULL, "Failed to add cloud icon");

		elm_image_file_set(cloud_icon, EDJ_IMAGE, MF_ICON_FOLDER);
		elm_image_resizable_set(cloud_icon, EINA_TRUE, EINA_TRUE);
		elm_layout_content_set(layout, "elm.swallow.content", cloud_icon);

		return layout;
	}
	return NULL;
}

static char *__mf_genlist_widget_storage_label_get(void *data, Evas_Object *obj, const char *part)
{
	storage_info *pStorage = (storage_info *)data;
	mf_retv_if(pStorage == NULL, NULL);

	if (!strcmp(part, "elm.text.main.left")) {
		mf_debug("%s ", pStorage->root_name);
		return elm_entry_utf8_to_markup(pStorage->root_name);
	} else if (!strcmp(part, "elm.text.sub")) {
		char storage_size[256];
		snprintf(storage_size, 256, "%5.2f / %5.2f GB", pStorage->occupied, pStorage->total);
		return elm_entry_utf8_to_markup(storage_size);
	} else {
		return NULL;
	}
}

void mf_genlist_cloud_content_set(void *data, Evas_Object *genlist, Eina_List *file_list)
{
	const Eina_List *list = NULL;
	void *item = NULL;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	mf_retm_if(genlist == NULL, "genlist is NULL");
	mf_retm_if(file_list == NULL, "file_list is NULL");

	EINA_LIST_FOREACH(file_list, list, item) {
		Elm_Object_Item *node_item = NULL;
		Elm_Genlist_Item_Class *itc_cloud = NULL;
		itc_cloud = elm_genlist_item_class_new();
		if (itc_cloud != NULL) {
			itc_cloud->item_style = "1line";
			itc_cloud->func.text_get = __mf_genlist_widget_storage_label_get;
			itc_cloud->func.content_get = __mf_genlist_widget_storage_content_get;
			node_item = elm_genlist_item_append(genlist, itc_cloud, item, NULL, ELM_GENLIST_ITEM_NONE, __mf_genlist_widget_storage_selected_cb, ap);
			elm_object_item_data_set(node_item, item);
		}
	}
}

/**	button related	**/
void mf_genlist_create_itc_style(Elm_Genlist_Item_Class **itc, int itc_type)
{

	struct appdata *ap = mf_get_appdata();
	if (*itc == NULL) {
		mf_error("new item class ==================== itc_type is [%d]", itc_type);
		*itc = elm_genlist_item_class_new();
		if (*itc == NULL) {
			return;
		}
	}
	int view_style = mf_view_style_get(ap);
	mf_debug("itc type : %d, view style : %d", itc_type, view_style);
	switch (itc_type) {
	case mf_item_itc_type_search:
		(*itc)->item_style = "myfile_search_2line.top.3";
		(*itc)->func.text_get = __mf_genlist_gl_label_get_lite;
		(*itc)->func.content_get = __mf_genlist_gl_default_icon_get_lite;
		(*itc)->func.del	    = NULL;
		(*itc)->func.state_get = __mf_genlist_gl_state_get;
		break;
	case mf_item_itc_type_recent:
		if (view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
			(*itc)->item_style = "myfile_2line.top.3";
		} else {
			(*itc)->item_style =  "myfile_1line";
		}
		(*itc)->func.text_get = __mf_genlist_gl_label_get_lite;
		(*itc)->func.content_get = __mf_genlist_gl_default_icon_get_lite;
		(*itc)->func.state_get = __mf_genlist_gl_state_get;
		(*itc)->func.del = __mf_genlist_gl_del;
		break;
	case mf_item_itc_type_normal_list:
		if (ap->mf_Status.view_type == mf_view_root || ap->mf_Status.view_type == mf_view_storage) {
			(*itc)->item_style =  "myfile_1line_root";
		} else {
			(*itc)->item_style =  "myfile_1line";
		}
		(*itc)->func.text_get = __mf_genlist_gl_label_get_lite;
		(*itc)->func.content_get = __mf_genlist_gl_default_icon_get_lite;
		(*itc)->func.state_get = __mf_genlist_gl_state_get;
		(*itc)->func.del = __mf_genlist_gl_del;
		break;
	case mf_item_itc_type_normal_list_details:
		(*itc)->item_style = "myfile_2line.top.3";
		(*itc)->func.text_get = __mf_genlist_gl_label_get_lite;
		(*itc)->func.content_get = __mf_genlist_gl_default_icon_get_lite;
		(*itc)->func.state_get = __mf_genlist_gl_state_get;
		(*itc)->func.del = __mf_genlist_gl_del;
		break;
	case mf_item_itc_type_category:
		if (view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
			(*itc)->item_style = "myfile_2line.top.3";
		} else {
			(*itc)->item_style =  "myfile_1line";
		}
		(*itc)->func.text_get = __mf_genlist_gl_label_get_lite;
		(*itc)->func.content_get = __mf_genlist_gl_default_icon_get_lite;
		(*itc)->func.del = __mf_genlist_gl_del;
		(*itc)->func.state_get = __mf_genlist_gl_state_get;
		break;
	}

	MF_TRACE_BEGIN;
}


void mf_genlist_create_data(mfItemData_s **m_TempItem, const char *name, void *data)
{
	mf_retm_if(m_TempItem == NULL, "m_TempItem is NULL");
	mf_retm_if(name == NULL, "name is NULL");

	*m_TempItem = (mfItemData_s *) calloc(1, sizeof(mfItemData_s));
	if (*m_TempItem == NULL) {
		mf_error();
		return;
	}

	(*m_TempItem)->m_ItemName = g_string_new(name);
	(*m_TempItem)->size = NULL;
	(*m_TempItem)->create_date = NULL;
	(*m_TempItem)->m_checked = FALSE;
	(*m_TempItem)->pCheckBox = NULL;
	(*m_TempItem)->thumb_path = NULL;
	(*m_TempItem)->real_thumb_flag = FALSE;
	(*m_TempItem)->media = NULL;
	(*m_TempItem)->ap = (struct appdata *)data;

}

void mf_genlist_set_folder_edit_style(void *data)
{
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_gl_style.userfolderitc) {
		ap->mf_gl_style.userfolderitc->decorate_all_item_style = NULL;
	}
}

void mf_genlist_create_list_default_style(Evas_Object *pGenlist, void *data, Eina_List *dir_list,
        Eina_List *file_list)
{
	MF_TRACE_BEGIN;
	/*0.    variable definition and parameter check*/
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input parameter data error");

	mf_retm_if(pGenlist == NULL, "input parameter pGenlist error");
	/*0.5.  data list varaible set*/
	/*1.    item style set */
	/*3.    check if we need give no content view*/
	/*4.    append items to the genlist*/
	fsNodeInfo *pNode = NULL;
	Eina_List *l = NULL;
	Elm_Object_Item *it = NULL;

	mf_debug("dir_list count is [%d] file is [%d]", eina_list_count(dir_list), eina_list_count(file_list));
	/*      add default folder items into the genlist       */
	EINA_LIST_FOREACH(dir_list, l, pNode) {
		if (pNode) {
			it = mf_view_item_append(pGenlist, pNode, ap);
			if (ap->mf_Status.ToTop) {
				elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
				ap->mf_Status.ToTop = false ;
			}
		}
	}
	/*      add file items into the genlist */
	mf_debug("view_type is [%d]", ap->mf_Status.view_type);
	EINA_LIST_FOREACH(file_list, l, pNode) {
		if (pNode) {
			it = mf_view_item_append(pGenlist, pNode, ap);
			if (ap->mf_Status.ToTop) {
				elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
				ap->mf_Status.ToTop = false ;
			}
		}
	}
	MF_TRACE_BEGIN;
}

void mf_genlist_gl_longpress(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input parameter data error");

	if (event_info) {
		Elm_Object_Item *it = (Elm_Object_Item *)event_info;
		//elm_genlist_item_selected_set(it, EINA_FALSE);//Fixed P131126-05278 ,pressed, don't need to set as false, only need set as false at the sel callback.

		if (ap->mf_Status.more == MORE_DEFAULT || ap->mf_Status.more == MORE_SEARCH) {
			elm_genlist_item_selected_set(it, FALSE);
			void *selected =  elm_object_item_data_get(it);
			SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
			ap->mf_MainWindow.pLongpressPopup = mf_popup_create_operation_item_pop(selected);
		}
	}
	MF_TRACE_END;
}

void mf_genlist_gl_drag_stop(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input parameter data error");
	MF_TRACE_END;
}


void mf_genlist_gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_realized_items_update(obj);
}

Evas_Object *mf_genlist_create_list(void *data, Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	struct appdata *ap = (struct appdata *)data;
	Evas_Object *genlist = NULL;
	Eina_List *file_list = NULL;
	Eina_List *dir_list = NULL;
	int view_style = mf_view_style_get(ap);

	/*	generate raw data list*/

	if (ap->mf_Status.view_type == mf_view_root_category) {
		file_list = ap->mf_FileOperation.category_list;
	} else {
		file_list = ap->mf_FileOperation.file_list;
		dir_list = ap->mf_FileOperation.folder_list;
	}

	/*      create Genlist*/
	MF_TA_ACUM_ITEM_BEGIN("123456 mf_object_create_genlist", 0);
	genlist = mf_object_create_genlist(parent);
	MF_TA_ACUM_ITEM_END("123456 mf_object_create_genlist", 0);
	MF_TA_ACUM_ITEM_BEGIN("123456 register genlist callback functions", 0);
	evas_object_smart_callback_add(genlist, "language,changed", mf_genlist_gl_lang_changed, data);
	//evas_object_smart_callback_add(genlist, "longpressed", mf_genlist_gl_longpress, ap);

	if (genlist == NULL) {
		MF_TRACE_END;
		t_end;
		return NULL;
	}
	mf_debug("More is [%d]", ap->mf_Status.more);
	switch (ap->mf_Status.more) {
	case MORE_DEFAULT://Go through.

	case MORE_INTERNAL_COPY:
	case MORE_INTERNAL_MOVE:
	case MORE_INTERNAL_COPY_MOVE:
	case MORE_DATA_COPYING:
	case MORE_DATA_MOVING:
	case MORE_COMPRESS:
	case MORE_DECOMPRESS:
	case MORE_DECOMPRESS_HERE:
	case MORE_INTERNAL_DECOMPRESS:
		evas_object_smart_callback_add(genlist, "selected", mf_edit_list_item_sel_cb, ap);
		if (view_style == MF_VIEW_SYTLE_LIST_DETAIL) {
			mf_genlist_create_itc_style(&ap->mf_gl_style.itc, mf_item_itc_type_normal_list_details);
			mf_genlist_create_itc_style(&ap->mf_gl_style.userfolderitc, mf_item_itc_type_normal_list_details);
		} else {
			mf_genlist_create_itc_style(&ap->mf_gl_style.itc, mf_item_itc_type_normal_list);
			mf_genlist_create_itc_style(&ap->mf_gl_style.userfolderitc, mf_item_itc_type_normal_list);
		}
		MF_TA_ACUM_ITEM_END("123456 register genlist callback functions", 0);
		MF_TA_ACUM_ITEM_BEGIN("123456 append genlist items", 0);
		mf_genlist_create_list_default_style(genlist, ap, dir_list, file_list);
		MF_TA_ACUM_ITEM_END("123456 append genlist items", 0);
		break;
	default:
		break;
	}
	/*4.    add watcher:*/
	/*5.    clear temporary data*/
	MF_TRACE_END;
	t_end;
	g_is_refresh_at_glist = false;
	g_mf_create_thumbnail_count = 0;//Fixed P140827-07370
	return genlist;
}

void mf_genlist_get_list_selected_items(Evas_Object * pGenlist, Eina_List **list)
{
	MF_TRACE_BEGIN;
	mf_retm_if(pGenlist == NULL, "pGenlist is NULL");
	Elm_Object_Item *item = NULL;
	mfItemData_s *itemData = NULL;

	item = elm_genlist_first_item_get(pGenlist);
	while (item) {
		itemData = elm_object_item_data_get(item);
		if (itemData->m_checked) {
			*list = eina_list_append(*list, itemData);
		}
		item = elm_genlist_item_next_get(item);
	}
	MF_TRACE_END;
}

void mf_genlist_clear(Evas_Object *genlist)
{
	mf_retm_if(genlist == NULL, "genlist is NULL");

	Elm_Object_Item *it;
	it = elm_genlist_first_item_get(genlist);
	while (it) {
		elm_object_item_del(it);
		it = elm_genlist_first_item_get(genlist);
	}

}

static void _index_clicked(void *data, Evas_Object *obj, const char *em, const char *src)
{
	if (!obj) {
		return;
	}
	mf_error(" >>>>>>>>>>>>>> obj is [%p]", obj);
	elm_object_signal_emit(obj, "elm,state,slide,start", "");
}

void mf_genlist_path_item_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ap = mf_get_appdata();
	if (!(ap->mf_Status.more == MORE_DEFAULT
	        || ap->mf_Status.more == MORE_INTERNAL_COPY
	        || ap->mf_Status.more == MORE_INTERNAL_MOVE
	     )) {
		return;
	}
	char *fullpath = (char *)data;
	mf_error("~~~~~~~~~~~~~~~~~~ fullpath is [%s], current path is [%s]", fullpath, ap->mf_Status.path->str);
	mf_util_path_stack_free();
	if (fullpath == NULL) {
		SAFE_FREE_GSTRING(ap->mf_Status.path);
		ap->mf_Status.path = g_string_new(PHONE_FOLDER);
		ap->mf_Status.view_type = mf_view_root;
		mf_view_update(ap);
	} else {
		if (g_strcmp0(ap->mf_Status.path->str, fullpath) == 0) {
			mf_error("The same folder selected");
			return;
		} else {
			if (!mf_file_exists(fullpath)) {
				SAFE_FREE_GSTRING(ap->mf_Status.path);
				ap->mf_Status.path = g_string_new(PHONE_FOLDER);
				ap->mf_Status.view_type = mf_view_root;
				mf_view_update(ap);
				return;
			}
			SAFE_FREE_GSTRING(ap->mf_Status.path);
			ap->mf_Status.path = g_string_new(fullpath);
			mf_view_update(ap);
		}
	}
}

Evas_Object *mf_genlist_create_path_tab(Evas_Object *parent, char *info, void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	Evas_Object *tab = NULL;
	struct appdata *ap = mf_get_appdata();
	int location = mf_fm_svc_wrapper_get_location(info);
	Eina_List *path_list = mf_fm_svc_wrapper_level_path_get(info, ap->mf_Status.view_type);
	{
		if (path_list) {
			tab = mf_object_path_widget_create(parent);
			Eina_List *l = NULL;
			char *path = NULL;
			const char *label = NULL;
			int count = 1;
			EINA_LIST_FOREACH(path_list, l, path) {
				if (path) {
					mf_error("path is [%s]", path);
					if (count == 1) {
						if (ap->mf_Status.view_type == mf_view_storage) {
							label = MF_LABEL_LOCAL_STORAGE;
						} else if (ap->mf_Status.view_type == mf_view_recent && ap->mf_Status.more == MORE_DEFAULT) {
							label = MF_LABEL_RECTENT_FILES;
						} else if (ap->mf_Status.view_type == mf_view_root_category && ap->mf_Status.categorytitle && ap->mf_Status.more == MORE_DEFAULT) {
							label = ap->mf_Status.categorytitle;
						} else {
							switch (location) {
							case MYFILE_PHONE:
								label = MF_LABEL_DEVICE_MEMORY;
								break;
							case MYFILE_MMC:
								label = MF_LABEL_SD_CARD;
								break;
							default:
								return NULL;
							}
						}
						Elm_Object_Item *item = mf_object_item_tabbar_item_append(tab, NULL, mf_util_get_text(elm_entry_utf8_to_markup(label)), mf_genlist_path_item_cb, g_strdup(path));
						mf_object_item_translate_set(item, label);
						count++;
					} else {
						if (ap->mf_Status.view_type == mf_view_storage) {
							continue;
						}
						label = mf_file_get(path);
						mf_object_item_tabbar_item_append(tab, NULL, mf_util_get_text(elm_entry_utf8_to_markup(label)), mf_genlist_path_item_cb, g_strdup(path));
						count++;
					}
				}
			}
			Elm_Object_Item *last_item = elm_toolbar_last_item_get(tab);
			elm_object_item_disabled_set(last_item, EINA_FALSE);
			elm_toolbar_item_show(last_item, ELM_TOOLBAR_ITEM_SCROLLTO_LAST);
		}
	}
	mf_util_free_eina_list_with_data(&path_list, MYFILE_TYPE_CHAR);
	return tab;
}

Evas_Object *mf_genlist_create_path_info(Evas_Object *parent, char *info, Eina_Bool slide_flag)
{
	char *strings = NULL;
	Evas_Object *bx = elm_box_add(parent);
	Evas_Object *ly = elm_layout_add(parent);
	elm_layout_theme_set(ly, "genlist/item", "groupindex", "default");
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(ly, -1, -1);

	if (info) {
		strings = elm_entry_utf8_to_markup(info);
	}

	if (strings) {
		mf_object_text_set(ly, strings, "elm.text.main");
		free(strings);
		strings = NULL;
	} else if (info) {
		mf_object_text_set(ly, info, "elm.text.main");
	}

	evas_object_show(ly);
	if (slide_flag) {
		elm_object_signal_emit(ly, "elm,state,slide,start", "");
	}
	elm_layout_signal_callback_add(ly, "mouse,clicked,1", "*", _index_clicked, NULL);
	elm_box_pack_end(bx, ly);
	return bx;
	MF_TRACE_END;
}

char *mf_genlist_first_item_name_get(Evas_Object *genlist)
{
	mf_retvm_if(genlist == NULL, NULL, "genlist is NULL");
	int x = 300;
	int y = 220;
	char *fullname = NULL;
	Elm_Object_Item *it = mf_object_item_genlist_x_y_item_get(genlist, x, y);
	if (it) {
		mf_list_data_t *list_data = elm_object_item_data_get(it);
		if (list_data && list_data->list_type == mf_list_normal) {
			mfItemData_s *item_data = elm_object_item_data_get(it);
			if (item_data && item_data->m_ItemName && item_data->m_ItemName->str) {
				fullname = g_strdup(item_data->m_ItemName->str);
			}
		}
	}
	return fullname;
}

char *mf_genlist_group_index_label_get(void *data, Evas_Object * obj, const char *part)
{
	mfItemData_s *params = (mfItemData_s *) data;
	mf_retvm_if(params == NULL, NULL, "params is NULL");
	mf_error("part=%s", part);
	if (strcmp(part, "elm.text.main") == 0) {
		mf_error("params->m_ItemName->str, mf_util_get_text(params->m_ItemName->str) is [%s] [%s]", params->m_ItemName->str, mf_util_get_text(params->m_ItemName->str));
		return g_strdup(mf_util_get_text(params->m_ItemName->str));
	}
	return g_strdup(_(""));
}

void mf_genlist_group_index_del(void *data, Evas_Object * obj)
{
	MF_TRACE_BEGIN;
	mfItemData_s *params = (mfItemData_s *) data;
	SAFE_FREE_CHAR(params);
	return;
}

static int __mf_model_utils_read_dir(const char *dir_path, Eina_List **dir_list, Eina_List **file_list)
{
	mf_retvm_if(dir_path == NULL, -1, "dir_path is NULL");
	mf_retvm_if(dir_list == NULL, -1, "dir_list is NULL");
	mf_retvm_if(file_list == NULL, -1, "file_list is NULL");
	DIR *const pDir = opendir(dir_path);
	mf_retvm_if(pDir == NULL, -1, "Failed to open dir %s", dir_path);

	struct dirent ent_struct;
	struct dirent *ent = NULL;
	while ((readdir_r(pDir, &ent_struct, &ent) == 0) && ent) {
		int skip = ((strncmp(ent->d_name, ".", 1) == 0) ||
		            (strncmp(ent->d_name, "..", 2) == 0));

		skip = skip || ((ent->d_type != DT_DIR) && (ent->d_type != DT_REG));

		skip = skip || ((ent->d_type == DT_DIR) &&
		                (strcmp(dir_path, PHONE_FOLDER) == 0) &&
		                (strcmp(ent->d_name, DEBUG_FOLDER) == 0));

		node_info *const pNode = skip ? NULL : calloc(1, sizeof(node_info));
		if (pNode) {
			pNode->parent_path = strdup(dir_path);
			pNode->name = strdup(ent->d_name);
			pNode->is_selected = EINA_FALSE;
			char *fullpath = g_strconcat(dir_path, "/", ent->d_name, NULL);

			if (ent->d_type == DT_DIR) {
				pNode->type = FILE_TYPE_DIR;
			} else {
				mf_file_attr_get_file_category(ent->d_name, &(pNode->type));
				mf_genlist_get_cloud_thumbnail_path(fullpath, &(pNode->thumb_path));
			}

			if (pNode->type == FILE_TYPE_DIR) {
				*dir_list = eina_list_append(*dir_list, pNode);
			} else {
				*file_list = eina_list_append(*file_list, pNode);
			}
			free(fullpath);
		}
	}

	closedir(pDir);

	return MYFILE_ERR_NONE;
}

static int mf_genlist_get_cloud_file_list(const char *dir_path, Eina_List **file_list)
{
	Eina_List *dirs = NULL;
	Eina_List *files = NULL;
	int ret = __mf_model_utils_read_dir(dir_path, &dirs, &files);
	if (ret != 0) {
		mf_error("Failed to read dir '%s'", dir_path);
		return ret;
	}
	dirs = eina_list_sort(dirs, eina_list_count(dirs), __mf_fs_oper_sort_by_name_cb_A2Z);
	files = eina_list_sort(files, eina_list_count(files), __mf_fs_oper_sort_by_name_cb_A2Z);
	*file_list = eina_list_merge(dirs, files);

	return 0;
}

static Evas_Object *__mf_genlist_cloud_layout_add(Evas_Object *parent, Evas_Object_Event_Cb destroy_cb, void *cb_data)
{
	Evas_Object *layout = elm_layout_add(parent);
	mf_retvm_if(layout == NULL, NULL , "Layout is NULL");
	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_event_callback_add(layout, EVAS_CALLBACK_FREE, destroy_cb, cb_data);

	evas_object_show(layout);

	return layout;
}

static char *__mf_genlist_cloud_title_get(char *curr_path, Eina_List *storage_list)
{
	Eina_List *l = storage_list;
	storage_info* info = eina_list_data_get(l);
	for (; l; l = eina_list_next(l), info = eina_list_data_get(l)) {
		if ((info->type != STORAGE_TYPE_LABEL) && (!strncmp(info->root_path, curr_path, strlen(info->root_path)))) {
			return strdup(info->root_name);
		}
	}
	return strdup("");
}

storage_type __mf_genlist_cloud_is_root_path(const char *fullpath, Eina_List *storage_list)
{
	mf_retvm_if(fullpath == NULL, STORAGE_TYPE_NONE , "Input fullpath is NULL");

	Eina_List *list = NULL;
	void *item = NULL;
	EINA_LIST_FOREACH(storage_list, list, item) {
		storage_info *info = item;
		if ((info->type != STORAGE_TYPE_LABEL) && (!strcmp(info->root_path, fullpath))) {
			return info->type;
		}
	}

	return STORAGE_TYPE_NONE;
}

static void __mf_genlist_widget_storage_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ap = (struct appdata *)data;
	storage_info *pStorage = (storage_info *)elm_object_item_data_get((Elm_Object_Item *)event_info);
	if (!pStorage) {
		mf_error("Fail to get object data");
		return;
	}

	cloud_view_data *cloud_data = calloc(1, sizeof(cloud_view_data));
	if (!cloud_data) {
		mf_error("Fail to allocate cloud data");
		return;
	}

	cloud_data->ap_data = ap;
	cloud_data->layout = obj;
	cloud_data->curr_path = strdup(pStorage->root_path);
	cloud_data->is_root = __mf_genlist_cloud_is_root_path(cloud_data->curr_path, ap->storage_list);
	cloud_data->file_list = NULL;

	cloud_data->navi_layout = __mf_genlist_cloud_layout_add(ap->mf_MainWindow.pNaviBar, NULL, cloud_data);
	if (!cloud_data->layout) {
		mf_error("Fail to create Layout");
		return;
	}
	elm_layout_file_set(cloud_data->navi_layout, EDJ_NAME, "navi_layout");
	int result = mf_genlist_get_cloud_file_list(cloud_data->curr_path, &cloud_data->file_list);

	cloud_data->cloud_item_genlist = elm_genlist_add(cloud_data->navi_layout);
	elm_genlist_mode_set(cloud_data->cloud_item_genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(cloud_data->cloud_item_genlist, EINA_TRUE);
	mf_genlist_cloud_item_content_set(ap, cloud_data->cloud_item_genlist, cloud_data->file_list);
	evas_object_show(cloud_data->cloud_item_genlist);
	elm_object_part_content_set(cloud_data->navi_layout, "content", cloud_data->cloud_item_genlist);
	if (result != 0) {
		mf_error("Fail to get file list");
		evas_object_del(ap->mf_MainWindow.pNaviLayout);
		return;
	}
	char *title = __mf_genlist_cloud_title_get(cloud_data->curr_path, ap->storage_list);
	if (ap->mf_Status.pPreNaviItem) {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar,
		                              ap->mf_Status.pPreNaviItem,
		                              "", NULL, NULL,
		                              cloud_data->navi_layout,
		                              MF_NAVI_STYLE_ENABLE);
	} else {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar,
		                              title, NULL, NULL,
		                              cloud_data->navi_layout,
		                              MF_NAVI_STYLE_ENABLE);
	}
	mf_navi_add_back_button(ap, mf_callback_navi_backbutton_clicked_cb);
	free(title);
}

static char *__mf_genlist_widget_cloud_item_label_get(void *data, Evas_Object *obj, const char *part)
{
	node_info *pNode = (node_info *)data;

	if (!strcmp(part, "elm.text.main.left")) {
		mf_debug("%s ", pNode->name);
		return elm_entry_utf8_to_markup(pNode->name);
	} else {
		return NULL;
	}
}

void mf_genlist_cloud_item_content_set(void *data, Evas_Object *genlist, Eina_List *file_list)
{
	const Eina_List *list = NULL;
	void *item = NULL;

	EINA_LIST_FOREACH(file_list, list, item) {
		Elm_Object_Item *node_item;
		Elm_Genlist_Item_Class *itc_cloud;
		itc_cloud = elm_genlist_item_class_new();
		if (itc_cloud != NULL) {
			itc_cloud->item_style = "1line";
			itc_cloud->func.text_get = __mf_genlist_widget_cloud_item_label_get;
			itc_cloud->func.content_get = __mf_genlist_widget_cloud_item_content_get;
			node_item = elm_genlist_item_append(genlist, itc_cloud, item, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_object_item_data_set(node_item, item);
		}
	}
}

static int mf_genlist_get_cloud_thumbnail_path(const char *filepath, char **thumb_path)
{
	mf_retvm_if(filepath == NULL, -1, "file path is NULL");
	mf_retvm_if(thumb_path == NULL, -1, "thumb_path is NULL");
	char thumbnail_path[256] = {0,};
	if (0 == strncmp(filepath, PHONE_FOLDER, strlen(PHONE_FOLDER))) {
		*thumb_path = NULL;
		return 0;
	}

	if (-1 != getxattr(filepath, "user.thumbnail", thumbnail_path, sizeof(thumbnail_path))) {
		*thumb_path = strdup(thumbnail_path);
		return 0;
	}

	*thumb_path = NULL;
	return -1;
}
static Evas_Object *__mf_genlist_widget_cloud_item_content_get(void *data, Evas_Object *obj, const char *part)
{
	node_info *pNode = (node_info *)data;
	if (!strcmp(part, "elm.icon.1")) {
		mf_retvm_if(pNode == NULL, NULL, "pNode is NULL");
		Evas_Object *layout = mf_object_create_layout(obj, EDJ_GENLIST_NAME, "genlist_content");
		Evas_Object *cloud_icon = elm_icon_add(layout);
		elm_image_file_set(cloud_icon, pNode->thumb_path, NULL);
		elm_image_resizable_set(cloud_icon, EINA_TRUE, EINA_TRUE);
		elm_layout_content_set(layout, "elm.swallow.content", cloud_icon);

		return layout;
	}
	return NULL;
}
