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

#include "mf-ta.h"
#include "mf-object-conf.h"
#include "mf-callback.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-gengrid.h"
#include "mf-util.h"
#include "mf-resource.h"
#include "mf-launch.h"
#include "mf-navi-bar.h"
#include "mf-object.h"
#include "mf-tray-item.h"
#include "mf-genlist.h"
#include "mf-view.h"
#include "mf-media-data.h"
#include "mf-popup.h"
#include "mf-object-item.h"
#include "mf-focus-ui.h"
#include "mf-media-content.h"
#include "mf-edit-view.h"
#include "mf-file-util.h"
#include "mf-search-view.h"


/* Static variables */
static Eina_List *mf_category_file_list = NULL;
static Elm_Gengrid_Item_Class category_gic;
static Eina_Bool g_is_refresh_space_size_flag = EINA_TRUE;
static Ecore_Idler *mf_category_view_idler = NULL;
static Evas_Object *mf_category_view_popup = NULL;


/* Static functions */
static void __mf_category_click_item(void *data);

void mf_category_view_refresh_space_size_set(Eina_Bool flag)
{
	g_is_refresh_space_size_flag = flag;
}

Eina_Bool mf_category_view_refresh_space_size_get()
{
	return g_is_refresh_space_size_flag;
}

static void __mf_category_item_sel(void *data, Evas_Object * obj, void *event_info)
{
	mf_retm_if(event_info == 0, "event_info is NULL");
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	if (item != NULL) {
		mfItemData_s *selected = (mfItemData_s *) elm_object_item_data_get(item);
		struct appdata *ap = (struct appdata *)selected->ap;
		mf_retm_if(ap == 0, "ap is NULL");

		int view_type = mf_view_style_get(ap);
		if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
			elm_gengrid_item_selected_set(item, FALSE);
		} else {
			elm_genlist_item_selected_set(item, FALSE);
		}
		//when longpressed, selected is called too ,so now if longpress popup exists,selected callbackwill return directly
		if (ap->mf_MainWindow.pLongpressPopup != NULL)
			return;
		if (ap->mf_Status.more == MORE_EDIT
			|| ap->mf_Status.more == MORE_SHARE_EDIT
			|| ap->mf_Status.more == MORE_EDIT_COPY
			|| ap->mf_Status.more == MORE_EDIT_MOVE
		    || ap->mf_Status.more == MORE_EDIT_DETAIL
			|| ap->mf_Status.more == MORE_EDIT_ADD_SHORTCUT
			|| ap->mf_Status.more == MORE_EDIT_DELETE
		) {
			MF_TRACE_END;
			return;
		}
		__mf_category_click_item(selected);
	}
}

Ecore_Idler *g_mf_launching_service_by_idler = NULL;//Fix the bug, when clicking the image again and again quickly, there will be problem.

static Eina_Bool __mf_launch_service_idler_cb(void *data)
{
	g_mf_launching_service_by_idler = NULL;
	mf_retvm_if(data == NULL, ECORE_CALLBACK_CANCEL, "data is NULL");
	mf_debug("__mf_launch_service_idler_cb()... ");
	struct appdata *ap = mf_get_appdata();
	char *path = (char*) data;
	int ret = 0;
	ret = mf_launch_service(ap, path);
	mf_debug("ret is %d\n", ret);
	if (ret) {
		ap->mf_MainWindow.pNormalPopup =
			mf_popup_create_popup(ap, POPMODE_TEXT, NULL, MF_LABEL_UNSUPPORT_FILE_TYPE, NULL, NULL, NULL, NULL, NULL);
	}
	return ECORE_CALLBACK_CANCEL;
}

void mf_launch_service_idler_del()
{
	if (g_mf_launching_service_by_idler) {
		ecore_idler_del(g_mf_launching_service_by_idler);
		g_mf_launching_service_by_idler = NULL;
	}
}

int mf_launch_service_by_idler(void *data, char *path)
{
	//Fix the bug P131030-05285
	mf_launch_service_idler_del();
	g_mf_launching_service_by_idler = ecore_idler_add((Ecore_Task_Cb)__mf_launch_service_idler_cb, path);
	return 0;
}

static void __mf_category_click_item(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mfItemData_s *item_data = (mfItemData_s *)data;
	mf_retm_if(item_data->m_ItemName == NULL, "item_data->m_ItemName is NULL");
	mf_retm_if(item_data->m_ItemName->str == NULL, "item_data->m_ItemName->str is NULL");
	struct appdata *ap = item_data->ap;
	mf_retm_if(ap == NULL, "ap is NULL");
	char *path = item_data->m_ItemName->str;

	if (ap->mf_Status.more == MORE_RENAME) {
		mf_callback_rename_save_cb(item_data, NULL, NULL);
		return;
	}
	if (ap->mf_Status.search_handler > 0) {
		mf_search_stop(ap->mf_Status.search_handler);
		mf_search_finalize(&ap->mf_Status.search_handler);
	}

	mf_launch_service_by_idler(ap, path);

	MF_TRACE_END;
}

void mf_category_view_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap->mf_MainWindow.pNaviBar == NULL, "navibar is NULL");

	mf_ecore_idler_del(mf_category_view_idler);
	SAFE_FREE_OBJ(mf_category_view_popup);
	if (ap->mf_Status.more == MORE_DEFAULT) {
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
		SAFE_FREE_OBJ(ap->mf_MainWindow.pSearchBar);
		ap->mf_MainWindow.pSearchEntry = NULL;
		ap->mf_Status.view_type = mf_view_root;
		Evas_Object *view = elm_object_part_content_get(ap->mf_MainWindow.pNaviLayout, "content");
		SAFE_FREE_OBJ(view);
		mf_media_data_list_free(&mf_category_file_list);
		mf_view_update(ap);
	} else {
		mf_callback_cancel_cb(ap, NULL, NULL);
	}
}

Eina_Bool mf_category_view_navi_back_cb(void *data, Elm_Object_Item *it)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap->mf_MainWindow.pNaviBar == NULL, EINA_FALSE, "navibar is NULL");

	mf_category_view_back_cb(data, NULL, NULL);

	return EINA_FALSE;
}

void mf_category_view_set_ctrl_button(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Elm_Object_Item *navi_it = ap->mf_MainWindow.pNaviItem;
	Evas_Object *more_bt = NULL;
	switch (ap->mf_Status.more) {
	case MORE_DEFAULT:
		more_bt = mf_object_create_button(ap->mf_MainWindow.pNaviBar, NAVI_BUTTON_EDIT, MF_LABEL_MORE, NULL, (Evas_Smart_Cb)mf_callback_more_button_cb, ap, EINA_FALSE);
		break;
	default:
		break;
	}
	if (more_bt) {
		Evas_Object *unset = elm_object_item_part_content_unset(navi_it, NAVI_MORE_BUTTON_PART);
		SAFE_FREE_OBJ(unset);
		elm_object_item_part_content_set(navi_it, NAVI_MORE_BUTTON_PART, more_bt);
		evas_object_event_callback_add(more_bt, EVAS_CALLBACK_KEY_DOWN, mf_callback_more_keydown_cb, ap);
	}
	t_end;
	MF_TRACE_END;

}


#if 1
mfItemData_s *mf_category_media_data_generate(media_data_s *media_data)
{
	mfItemData_s *item_data = NULL;
	item_data = calloc(1, sizeof(mfItemData_s));
	if (item_data) {
		item_data->m_ItemName = g_string_new(media_data->fullpath);
		{
			//Fix P131126-02722,add thumbnail for sound and music in normal view and category view
			if (media_data->file_type == FILE_TYPE_MUSIC || media_data->file_type == FILE_TYPE_SOUND) {
				fsFileType category = FILE_TYPE_NONE;
				mf_file_attr_get_file_category(media_data->fullpath, &category);
				if (category == FILE_TYPE_VOICE) {
					item_data->thumb_path = g_strdup(MF_ICON_SOUND);
					item_data->thumbnail_type = MF_THUMBNAIL_DEFAULT;
					item_data->real_thumb_flag = TRUE;
				} else if (media_data->thumbnail_path && mf_file_exists(media_data->thumbnail_path) && strcmp(media_data->thumbnail_path,MF_MUSIC_DEFAULT_THUMBNAIL_FROM_DB)) {
					item_data->thumb_path = g_strdup(media_data->thumbnail_path);
					item_data->thumbnail_type = MF_THUMBNAIL_THUMB;
					item_data->real_thumb_flag = TRUE;
				} else {
					item_data->thumb_path = g_strdup(MF_ICON_MUSIC_THUMBNAIL);
					item_data->thumbnail_type = MF_THUMBNAIL_DEFAULT;
					item_data->real_thumb_flag = TRUE;
				}
				item_data->file_type = media_data->file_type;
			} else if (media_data->thumbnail_path) {
				if (media_data->thumbnail_path && mf_file_exists(media_data->thumbnail_path) && g_strcmp0(media_data->thumbnail_path, MF_MUSIC_DEFAULT_THUMBNAIL_FROM_DB) == 0) {
					fsFileType type = FILE_TYPE_NONE;
					mf_file_attr_get_file_category(media_data->fullpath, &type);
					const char *default_thumb = mf_file_attr_get_default_icon_by_type(type);
					item_data->thumb_path = g_strdup(default_thumb);
					item_data->thumbnail_type = MF_THUMBNAIL_DEFAULT;
					item_data->file_type = type;
				} else {
					item_data->thumb_path = g_strdup(media_data->thumbnail_path);
					if (item_data->thumb_path && strncmp(item_data->thumb_path, MF_IMAGE_HEAD, strlen(MF_IMAGE_HEAD))) {
						item_data->thumbnail_type = MF_THUMBNAIL_THUMB;
					} else {
						item_data->thumbnail_type = MF_THUMBNAIL_DEFAULT;
					}
					item_data->file_type = media_data->file_type;
				}
				item_data->real_thumb_flag = TRUE;
			} else {
				item_data->real_thumb_flag = FALSE;
				item_data->file_type = media_data->file_type;
			}
		}
		item_data->storage_type = mf_fm_svc_wrapper_get_location(media_data->fullpath);
		item_data->list_type = mf_list_normal;
	}
	return item_data;
}

void mf_category_list_item_remove_by_fullname(Eina_List **list, const char *fullpath)
{
	Eina_List *l = NULL;

	media_data_s *item_data = NULL;

	mf_debug("=============== list count is [%d]", eina_list_count(*list));

	EINA_LIST_FOREACH(*list, l, item_data) {
		if (item_data) {
			if (g_strcmp0(item_data->fullpath, fullpath) == 0) {
				*list = eina_list_remove(*list, item_data);
				mf_media_data_item_free(&item_data);
				break;
			}
		}
	}
	if (eina_list_count(*list) == 0) {
		eina_list_free(*list);
		*list = NULL;
	}
}

void mf_category_list_item_remove_by_storage_type(Eina_List **list, int storage_type)
{
	if (!*list) {
		return;
	}
	Eina_List *l = NULL;

	media_data_s *item_data = NULL;

	EINA_LIST_FOREACH(*list, l, item_data) {
		if (item_data) {
			if (item_data->storage_type == storage_type) {
				*list = eina_list_remove(*list, item_data);
				mf_media_data_item_free(&item_data);
			}
		}
	}
	if (eina_list_count(*list) == 0) {
		eina_list_free(*list);
		*list = 0;
	}
}

void mf_category_list_item_add(const char *fullpath, int media_content_type)
{
	mf_debug("========== entry");

	if (media_content_type == MEDIA_CONTENT_TYPE_OTHERS) {
		int type = mf_file_attr_get_file_type_by_mime(fullpath);
		switch (type) {
		case FILE_TYPE_IMAGE:
		case FILE_TYPE_VIDEO:
		case FILE_TYPE_SOUND:
		case FILE_TYPE_DOC:
		case FILE_TYPE_PDF:
		case FILE_TYPE_PPT:
		case FILE_TYPE_EXCEL:
		case FILE_TYPE_TXT:
		case FILE_TYPE_HWP:
			break;
		default:
			return;
		}
	}
	Eina_List *list = mf_category_file_list;
	mf_media_data_printf(list);
	if (list) {
		mf_media_category_item_get(fullpath, media_content_type, &list);
		mf_media_data_printf(list);
	}
}

void mf_category_list_item_remove(const char *fullpath, int media_content_type)
{
	MF_TRACE_BEGIN;
	if (media_content_type == MEDIA_CONTENT_TYPE_OTHERS) {
		int type = mf_file_attr_get_file_type_by_mime(fullpath);
		switch (type) {
		case FILE_TYPE_IMAGE:
		case FILE_TYPE_VIDEO:
		case FILE_TYPE_SOUND:
		case FILE_TYPE_DOC:
		case FILE_TYPE_PDF:
		case FILE_TYPE_PPT:
		case FILE_TYPE_EXCEL:
		case FILE_TYPE_TXT:
		case FILE_TYPE_HWP:
			break;
		default:
			return;
		}
	}
	mf_media_data_printf(mf_category_file_list);
	if (mf_category_file_list) {
		mf_category_list_item_remove_by_fullname(&mf_category_file_list, fullpath);
		//mf_media_data_printf(mf_category_file_list);
	}
}

void mf_view_refresh_edit_status_for_category_list(Eina_List *category_list)
{
	mf_debug("========== entry");
	mf_retm_if(category_list == NULL, "category_list is NULL");
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	int view_type = mf_view_style_get(ap);

	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;

	if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
		it = elm_gengrid_first_item_get(ap->mf_MainWindow.pNaviGengrid);
	} else {
		it = elm_genlist_first_item_get(ap->mf_MainWindow.pNaviGenlist);
	}
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
			continue;
		}
		//mf_debug("itemData->m_ItemName->str is [%s]", itemData->m_ItemName->str);

		mfItemData_s *item_data_tmp = NULL;
		Eina_List *l = NULL;
		EINA_LIST_FOREACH(category_list, l, item_data_tmp) {
				if (item_data_tmp) {
					if (g_strcmp0(itemData->m_ItemName->str, item_data_tmp->m_ItemName->str) == 0) {
						if (itemData->m_checked == true)
							item_data_tmp->m_checked = true;
						//mf_debug("itemData->m_ItemName->str1111 is [%s]", itemData->m_ItemName->str);
						//mf_debug("item_data_tmp->m_checked is [%d]", item_data_tmp->m_checked);
						break;
					}
				}
			}
		if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
			it = elm_gengrid_item_next_get(it);
		} else {
			it = elm_genlist_item_next_get(it);
		}
	}
	mf_debug("========== end");
}

void mf_category_view_restore_the_edit_item()
{
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	int view_type = mf_view_style_get(ap);

	mfItemData_s *itemData = NULL;
	Elm_Object_Item *it = NULL;

	if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
		it = elm_gengrid_first_item_get(ap->mf_MainWindow.pNaviGengrid);
	} else {
		it = elm_genlist_first_item_get(ap->mf_MainWindow.pNaviGenlist);
	}
	while (it) {
		itemData = elm_object_item_data_get(it);
		if (itemData->m_ItemName == NULL || itemData->m_ItemName->str == NULL) {
			continue;
		}
		if (itemData->m_checked == true) {
			mf_edit_file_list_append(itemData->item);
		}

		if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
			it = elm_gengrid_item_next_get(it);
		} else {
			it = elm_genlist_item_next_get(it);
		}
	}
	int count = mf_edit_file_count_get();
	mf_edit_view_select_all_check(count);
	mf_edit_view_ctrlbar_state_set(ap);
}

void mf_category_search_item_update(void *data, char *path, media_content_db_update_item_type_e update_item, media_content_db_update_type_e update_type, char *uuid)
{
	mf_error("path is [%s]", path);
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	if (update_type == MEDIA_CONTENT_INSERT) {
		return;
	}
	if (update_type == MEDIA_CONTENT_UPDATE) {
		if (update_item == MEDIA_ITEM_FILE) {
			if (ap->mf_Status.search_filter) {
				if (mf_util_NFD_strstr(mf_file_get(path), ap->mf_Status.search_filter)) {
					media_info_h handle = NULL;
					int ret = media_info_get_media_from_db(uuid, &handle);
					if (ret == MEDIA_CONTENT_ERROR_NONE) {
						media_data_s *media_data = mf_media_data_get_by_media_handle(handle);
						mf_view_item_popup_check(ap, path);
						if (media_data) {
							if (!mf_util_NFD_strstr(media_data->display_name, ap->mf_Status.search_filter)) {
								mf_view_item_delete_by_name(ap, path);
							} else {
								mf_view_search_item_update(ap, path, media_data->fullpath);
							}
							mf_media_data_item_free(&media_data);
						}
					}
					if (handle) {
						media_info_destroy(handle);
						handle = NULL;
					}
				}
			} else {
				media_info_h handle = NULL;
				int ret = media_info_get_media_from_db(uuid, &handle);
				if (ret == MEDIA_CONTENT_ERROR_NONE) {
					media_data_s *media_data = mf_media_data_get_by_media_handle(handle);
					mf_view_item_popup_check(ap, path);
					if (media_data) {
						if (!mf_util_NFD_strstr(media_data->display_name, ap->mf_Status.search_filter)) {
							mf_view_item_delete_by_name(ap, path);
						} else {
							mf_view_search_item_update(ap, path, media_data->fullpath);
						}
						mf_media_data_item_free(&media_data);
					}
				}
				if (handle) {
					media_info_destroy(handle);
					handle = NULL;
				}
			}
		} else {
			mf_view_item_delete_by_exists(ap);
		}
		return;
	}
	if (update_type == MEDIA_CONTENT_DELETE) {
		if (update_item == MEDIA_ITEM_FILE) {
			if (ap->mf_Status.search_filter) {
				if (mf_util_NFD_strstr(mf_file_get(path), ap->mf_Status.search_filter)) {
					mf_view_item_popup_check(ap, path);
					mf_view_item_delete_by_name(ap, path);
				}
			} else {
				mf_view_item_popup_check(ap, path);
				mf_view_item_delete_by_name(ap, path);
			}
		} else {
			mf_view_item_delete_by_exists(ap);
		}
	}
}

void mf_category_list_update_cb (media_content_error_e error, int pid, 
                                 media_content_db_update_item_type_e update_item,
                                 media_content_db_update_type_e update_type, media_content_type_e media_type,
                                 char *uuid, char *path, char *mime_type, void *user_data)
{
    int pre_more = MORE_DEFAULT;
    Evas_Object *newContent = NULL;
    struct appdata *ap = (struct appdata *)mf_get_appdata();

    mf_debug("update_item : [%d] update_type : [%d] media_type : [%d]",
              update_item, update_type, media_type);

    if (ap->mf_Status.view_type == mf_view_recent) {
    	if(mf_view_is_item_exists_by_name(ap, path)) {
    		mf_recent_view_create(ap);
    		return;
    	}
    }
    if (ap->mf_Status.more == MORE_SEARCH || mf_view_get_pre_state(ap) == MORE_SEARCH)
    {
        mf_category_search_item_update(user_data, path, update_item, update_type, uuid);
        return;
    }

    switch (update_type)
    {
        case MEDIA_CONTENT_INSERT://go through.
        {
            mf_debug("MEDIA_CONTENT_INSERT");
            /* TODO : need to add lists, not to update whole lists */
            //break;//go through.
        }
        case MEDIA_CONTENT_UPDATE:
        {
            mf_debug("MEDIA_CONTENT_UPDATE");

            if ((ap->mf_Status.more == MORE_SEARCH || mf_view_get_pre_state(ap) == MORE_SEARCH) &&
                (ap->mf_Status.search_filter!=NULL))
            {
                if (!ap->mf_Status.b_run_background) {
                    mf_category_view_refresh_space_size_set(true);
                    mf_category_size_update(ap);
                    return;
                }

                if (mf_is_dir(path)) {
                    mf_category_view_refresh_space_size_set(true);
                    mf_category_size_update(ap);
                    return;
                }

                if (mf_util_NFD_strstr(mf_file_get(path), ap->mf_Status.search_filter))
                {
                    media_info_h handle = NULL;
                    int ret = media_info_get_media_from_db(uuid, &handle);

                    if (ret == MEDIA_CONTENT_ERROR_NONE)
                    {
                        media_data_s *media_data = mf_media_data_get_by_media_handle(handle);
                        if (media_data) {
                            mf_view_item_popup_check(ap, path);
                            if (!mf_util_NFD_strstr(media_data->display_name, ap->mf_Status.search_filter)) {
                                mf_view_item_delete_by_name(ap, path);
                            } else {
                                mf_view_search_item_update(ap, path, media_data->fullpath);
                            }
                            mf_media_data_item_free(&media_data);
                        }
                    }

                    if (handle) {
                        media_info_destroy(handle);
                        handle = NULL;
                    }
                }
            }
            else
            {
                if (ap->mf_Status.view_type == mf_view_root_category &&
                    mf_view_get_pre_state(ap) != MORE_SEARCH &&
                    (ap->mf_Status.more == MORE_DEFAULT || ap->mf_Status.more == MORE_EDIT ||
                     ap->mf_Status.more == MORE_SHARE_EDIT || ap->mf_Status.more == MORE_EDIT_COPY ||
                     ap->mf_Status.more == MORE_EDIT_MOVE || ap->mf_Status.more == MORE_EDIT_ADD_SHORTCUT ||
                     ap->mf_Status.more == MORE_EDIT_DELETE || ap->mf_Status.more == MORE_EDIT_DETAIL))
                {   /* if it is rename state, don't refresh list */
                    if (mf_callback_monitor_media_db_update_flag_get()) {
                        mf_callback_monitor_media_db_update_flag_set(EINA_FALSE);
                        mf_category_view_refresh_space_size_set(true);
                        return;
                    }

                    Eina_List *file_list = NULL;
                    int pre_more = ap->mf_Status.more;

                    if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME) {
                        mf_view_state_reset_state_with_pre(ap);
                    }

                    if (ap->mf_MainWindow.pNewFolderPopup) {
                        SAFE_FREE_OBJ(ap->mf_MainWindow.pNewFolderPopup);
                    }

                    if (pre_more == MORE_EDIT ||
                        pre_more == MORE_SHARE_EDIT ||
                        pre_more == MORE_EDIT_COPY ||
                        pre_more == MORE_EDIT_MOVE ||
                        pre_more == MORE_EDIT_ADD_SHORTCUT ||
                        pre_more == MORE_EDIT_DELETE ||
                        pre_more == MORE_EDIT_DETAIL)
                    {
                        ap->mf_Status.more = MORE_DEFAULT;
                        file_list = mf_edit_get_selected_file_list();
                    }

                    elm_box_clear(ap->mf_MainWindow.pNaviBox);
                    newContent = mf_category_get_from_media_db(ap, ap->mf_Status.category_type, true);
                    elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);
                    evas_object_show(newContent);
                    //mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, newContent);

                    if (pre_more == MORE_EDIT ||
                        pre_more == MORE_EDIT_COPY ||
                        pre_more == MORE_EDIT_MOVE ||
                        pre_more == MORE_EDIT_ADD_SHORTCUT ||
                        pre_more == MORE_EDIT_DELETE ||
                        pre_more == MORE_EDIT_DETAIL)
                    {
                        if (!ap->mf_Status.flagNoContent) {
                            ap->mf_Status.more = pre_more;
                            mf_edit_view_refresh(ap, &file_list, NULL);
                        } else {
                            mf_naviframe_title_button_delete(ap->mf_MainWindow.pNaviItem);
                            mf_navi_bar_title_content_set(ap, ap->mf_Status.categorytitle);
                        }
                    } else if (pre_more == MORE_SHARE_EDIT) {
                        if (!ap->mf_Status.flagNoContent) {
                            ap->mf_Status.more = pre_more;
                            mf_edit_view_refresh(ap, &file_list, NULL);
                        } else {
                            mf_naviframe_title_button_delete(ap->mf_MainWindow.pNaviItem);
                            mf_navi_bar_title_content_set(ap, ap->mf_Status.categorytitle);
                        }
                    }
                }
            }

            mf_category_view_refresh_space_size_set(true);
            mf_category_size_update(ap);
            //mf_storage_refresh(ap);
        }
            break;

        case MEDIA_CONTENT_DELETE:
        {
            if (ap->mf_MainWindow.pDeleteConfirmPopup) {
                SAFE_FREE_OBJ(ap->mf_MainWindow.pDeleteConfirmPopup);
                ap->mf_FileOperation.idle_delete_item = NULL;
            }
            mf_category_list_item_remove(path, media_type);
            mf_category_view_refresh_space_size_set(true);
            if (ap->mf_Status.more == MORE_IDLE_DELETE) {
                return;
            }
            if (ap->mf_Status.view_type == mf_view_root_category && mf_view_get_pre_state(ap) != MORE_SEARCH
                && (ap->mf_Status.more == MORE_DEFAULT ||
                    ap->mf_Status.more == MORE_EDIT ||
                    ap->mf_Status.more == MORE_SHARE_EDIT ||
                    ap->mf_Status.more == MORE_THUMBNAIL_RENAME ||
                    ap->mf_Status.more == MORE_EDIT_COPY ||
                    ap->mf_Status.more == MORE_EDIT_MOVE ||
                    ap->mf_Status.more == MORE_EDIT_DETAIL ||
                    ap->mf_Status.more == MORE_EDIT_ADD_SHORTCUT ||
                    ap->mf_Status.more == MORE_EDIT_DELETE))
            {
                Eina_List *file_list = NULL;
                if (mf_callback_monitor_media_db_update_flag_get()) {
                    return;
                }

                if (ap->mf_Status.more == MORE_EDIT ||
                    ap->mf_Status.more == MORE_SHARE_EDIT ||
                    ap->mf_Status.more == MORE_EDIT_COPY ||
                    ap->mf_Status.more == MORE_EDIT_MOVE ||
                    ap->mf_Status.more == MORE_EDIT_DELETE ||
                    ap->mf_Status.more == MORE_EDIT_DETAIL
                  )
                {
                    pre_more = ap->mf_Status.more;
                    ap->mf_Status.more = MORE_DEFAULT;
                    file_list = mf_edit_get_selected_file_list();
                }

                mf_object_box_clear(ap->mf_MainWindow.pNaviBox);
                newContent = mf_category_get_from_media_db(ap, ap->mf_Status.category_type, true);
                elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);
                //mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, newContent);

                if (pre_more == MORE_EDIT || pre_more == MORE_SHARE_EDIT
                        || pre_more == MORE_EDIT_COPY
                        || pre_more == MORE_EDIT_MOVE
                        || pre_more == MORE_EDIT_DELETE
                        || pre_more == MORE_EDIT_DETAIL
                  ) {
                    if (!ap->mf_Status.flagNoContent) {
                        ap->mf_Status.more = pre_more;
                        mf_edit_view_refresh(ap, &file_list, NULL);
                    } else {
                        mf_naviframe_title_button_delete(ap->mf_MainWindow.pNaviItem);
                        mf_navi_bar_title_content_set(ap, ap->mf_Status.categorytitle);
                    }
                }
            }
            //mf_storage_refresh(ap);
        }
            break;

        default:
            mf_debug("Invalid Case : %d", update_type);
            break;
    }
}

void mf_category_list_destory()
{
	mf_media_data_list_free(&mf_category_file_list);
}

void mf_category_gen_style_set()
{
	category_gic.item_style = "custom/myfile";
	category_gic.func.text_get = mf_gengrid_item_label_get;
	category_gic.func.content_get = mf_gengrid_item_icon_get;
	category_gic.func.state_get = NULL;
	category_gic.func.del = mf_gengrid_item_del;
}

static Evas_Object *__mf_category_media_content_create(Eina_List *category_list, void *user_data)
{
	MF_TRACE_BEGIN;

	struct appdata *ap = (struct appdata *)user_data;
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");
	Evas_Object *content = NULL;
	int view_type = mf_view_style_get(ap);
	Evas_Object *parent = NULL;
	if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
		ap->mf_MainWindow.pNaviGengrid = mf_gengrid_create_grid(ap->mf_MainWindow.pNaviBar);
		mf_category_gen_style_set();
		parent = ap->mf_MainWindow.pNaviGengrid;
                evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGengrid, "language,changed", mf_gengrid_gl_lang_changed, ap);
                //evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGengrid, "longpressed", mf_gengrid_thumbs_longpressed, ap);
		evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGengrid, "selected", __mf_category_item_sel, ap);
		evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGengrid, "realized", mf_gengrid_realized, ap);

		mf_gengrid_align_set(ap->mf_MainWindow.pNaviGengrid, eina_list_count(category_list));

		elm_gengrid_item_size_set(ap->mf_MainWindow.pNaviGengrid, MF_HD_GENGRID_ITEM_WIDTH, MF_HD_GENGRID_ITEM_HEIGTH);
		evas_object_show(ap->mf_MainWindow.pNaviGengrid);
		content = ap->mf_MainWindow.pNaviGengrid;
		Eina_List *l = NULL;
		mfItemData_s *item_data = NULL;

		EINA_LIST_FOREACH(category_list, l, item_data) {
			if (item_data) {
				mf_view_item_append_with_data(parent, item_data, ap, &category_gic, mf_edit_gengrid_item_sel_cb, item_data);
			}
		}

	} else {
		mf_genlist_create_itc_style(&ap->mf_gl_style.categoryitc, mf_item_itc_type_category);
		ap->mf_MainWindow.pNaviGenlist = elm_genlist_add(ap->mf_MainWindow.pNaviBar);
		parent = ap->mf_MainWindow.pNaviGenlist;
		elm_genlist_mode_set(ap->mf_MainWindow.pNaviGenlist, ELM_LIST_COMPRESS);

		elm_genlist_homogeneous_set(ap->mf_MainWindow.pNaviGenlist, EINA_TRUE);//Fix the P140416-01947 by jian12.li
		elm_object_focus_allow_set(ap->mf_MainWindow.pNaviGenlist, EINA_TRUE);

		evas_object_size_hint_weight_set(ap->mf_MainWindow.pNaviGenlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(ap->mf_MainWindow.pNaviGenlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		//elm_genlist_mode_set(ap->mf_MainWindow.pNaviGenlist, ELM_LIST_COMPRESS);
		evas_object_show(ap->mf_MainWindow.pNaviGenlist);
		//evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGenlist, "longpressed", mf_genlist_gl_longpress, ap);
		evas_object_smart_callback_add(ap->mf_MainWindow.pNaviGenlist, "selected", mf_edit_list_item_sel_cb, ap);
		content = ap->mf_MainWindow.pNaviGenlist;
		Eina_List *l = NULL;
		mfItemData_s *item_data = NULL;

		EINA_LIST_FOREACH(category_list, l, item_data) {
			if (item_data) {
				mf_view_item_append_with_data(parent, item_data, ap, ap->mf_gl_style.categoryitc, __mf_category_item_sel, ap);
			}
		}
	}

	return content;

}

static Evas_Object *__mf_category_media_result_update(void *user_data, Eina_List *category_list)
{
	MF_TRACE_BEGIN;

	struct appdata *ap = (struct appdata *)user_data;
	mf_retvm_if(ap == NULL, NULL, "ap is NULL");
	Evas_Object *content = NULL;

	if (eina_list_count(category_list) == 0) {
		Evas_Object *no_content = NULL;

		Evas_Object *parent = NULL;
		ap->mf_Status.flagNoContent = EINA_TRUE;
		parent = ap->mf_MainWindow.pNaviLayout;
		no_content = mf_object_create_no_content(parent);
		mf_object_text_set(no_content, MF_LABEL_NO_FILES, "elm.text");
		evas_object_show(no_content);
		return no_content;
	} else {
		ap->mf_Status.flagNoContent = EINA_FALSE;
		content = __mf_category_media_content_create(category_list, ap);
		return content;
	}
	MF_TRACE_END;
}


Eina_List *mf_category_list_generate(Eina_List *file_list)
{
	Eina_List *l = NULL;
	media_data_s *media_data = NULL;
	Eina_List *category_list = NULL;

	EINA_LIST_FOREACH(file_list, l, media_data) {
                if (media_data) {
                        mfItemData_s *item_data = NULL;
                        item_data = mf_category_media_data_generate(media_data);
                        if (item_data) {
                                mf_debug("============= name is [%s]", item_data->m_ItemName->str);
                                category_list = eina_list_append(category_list, item_data);
                        }
                }
	}
	return category_list;
}

Evas_Object *mf_category_get_from_media_db(void *data, int category, bool is_use_previous_state)
{
	MF_TRACE_BEGIN;
	Evas_Object *content = NULL;
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, NULL, "input data error");
	Eina_List *media_list = NULL;
	Eina_List *category_list = NULL;
	
	mf_media_data_list_free(&mf_category_file_list);
	mf_media_category_list_get(category, &media_list);
	
	int sort_type = 0;
	mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &sort_type);
	mf_media_data_sort_list(&media_list, sort_type);
	mf_category_file_list = media_list;
	category_list = mf_category_list_generate(media_list);   

	if (is_use_previous_state) {
		mf_view_refresh_edit_status_for_category_list(category_list);
	}
	content = __mf_category_media_result_update(ap, category_list);

	MF_TRACE_END;
	return content;
}
Eina_List * g_mf_category_list = NULL;

static int mf_category_view_content_create_by_thread(void *data)
{
	MF_TRACE_BEGIN;
	Evas_Object *content = NULL;
	//Eina_List *category_list = NULL;
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	SAFE_FREE_OBJ(mf_category_view_popup);
	//ecore_main_loop_iterate();

	content = __mf_category_media_result_update(ap, g_mf_category_list);
	elm_box_pack_end(ap->mf_MainWindow.pNaviBox, content);
	MF_TRACE_END;
	mf_category_view_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void *__mf_get_media_data_func(void *data)
{
	FO_TRACE_BEGIN;
	Eina_List *media_list = NULL;
	//Eina_List *category_list = NULL;
	int category = (int)data;

	mf_media_data_list_free(&mf_category_file_list);
	mf_media_category_list_get(category, &media_list);

	int sort_type = 0;
	mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &sort_type);
	mf_media_data_sort_list(&media_list, sort_type);
	mf_category_file_list = media_list;
	g_mf_category_list = mf_category_list_generate(mf_category_file_list);
	//refresh UI by pipe
	mf_category_view_idler = ecore_idler_add((Ecore_Task_Cb)mf_category_view_content_create_by_thread, NULL);
	return NULL;
}

void mf_category_get_from_media_db_by_thread(void *data, int category, bool is_use_previous_state)
{
	MF_TRACE_BEGIN;
//	Evas_Object *content = NULL;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "input data error");

#if 0//Deprecated API
	if (!g_thread_supported()) {
		g_thread_init(NULL);
	}
#endif
	if (!g_thread_new(NULL, (GThreadFunc) __mf_get_media_data_func, (void*)category)) {
		mf_fo_loge("Fail to create __mf_get_media_data_func thread");
	}
}
#endif

int mf_category_view_item_count_get(int type)
{
	char *condition = MF_CONDITION_LOCAL_IMAGE;
	switch (type) {
	case mf_tray_item_category_image:
		condition = g_strdup(MF_CONDITION_LOCAL_IMAGE);
		break;
	case mf_tray_item_category_video:
		condition = g_strdup(MF_CONDITION_LOCAL_VIDEO);
		break;
	case mf_tray_item_category_sounds:
		condition = g_strdup(MF_CONDITION_LOCAL_SOUND);
		break;
	case mf_tray_item_category_document:
		condition = g_strdup(MF_CONDITION_LOCAL_DOCUMENT);
		break;
	default:
		return 0;
	}
	int count = 0;
	count = mf_media_content_data_count_get(condition);
	return count;
}

static int mf_category_view_content_create(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *newContent = mf_category_get_from_media_db(ap, ap->mf_Status.category_type, false);
	elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);
	//mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, newContent);   
	SAFE_FREE_OBJ(mf_category_view_popup);   
	mf_category_view_idler = NULL; 

	return ECORE_CALLBACK_CANCEL;
}

void mf_category_view_create_vew_as(void *data, bool flag_show)
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;

	mf_retm_if (ap->mf_MainWindow.pNaviBar == NULL, "ap->mf_MainWindow.pNaviBar is NULL");
	Evas_Object *newContent = NULL;
	//Evas_Object *pathinfo = NULL;
	ap->mf_Status.view_type = mf_view_root_category;

	elm_box_clear(ap->mf_MainWindow.pNaviBox);

	if (ap->mf_MainWindow.pNaviGenlist) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviGenlist);
		ap->mf_MainWindow.pNaviGenlist = NULL;
	}
	if (ap->mf_MainWindow.pNaviGengrid) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviGengrid);
		ap->mf_MainWindow.pNaviGengrid = NULL;
	}

	mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.recent_list), MYFILE_TYPE_FSNODE);
	if (ap->mf_FileOperation.folder_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.folder_list), MYFILE_TYPE_FSNODE);
	}
	if (ap->mf_FileOperation.file_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.file_list), MYFILE_TYPE_FSNODE);
	}
	mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.shortcut_list), MYFILE_TYPE_FSNODE);
	newContent = mf_category_get_from_media_db(ap, ap->mf_Status.category_type, false);
	elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);
	//mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, newContent);

	evas_object_show(newContent);

	t_end;
	/*temp data free*/
	MF_TRACE_END;
}

void mf_category_view_create(void *data, bool flag_show)
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;
	mf_error(">>>>>>>>> ap->mf_Status.flagViewAsRefreshView = %d", ap->mf_Status.flagViewAsRefreshView);
	if (ap->mf_Status.flagViewAsRefreshView == EINA_TRUE && flag_show == true) {
		mf_category_view_create_vew_as(data, flag_show);
		ap->mf_Status.flagViewAsRefreshView = EINA_FALSE;
		return;
	}

	mf_retm_if (ap->mf_MainWindow.pNaviBar == NULL, "ap->mf_MainWindow.pNaviBar is NULL");
	Evas_Object *newContent = NULL;
	ap->mf_Status.view_type = mf_view_root_category;
	//int view_style = mf_view_style_get(ap);

	mf_navi_bar_reset_navi_obj(ap);
	mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.recent_list), MYFILE_TYPE_FSNODE);
	if (ap->mf_FileOperation.folder_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.folder_list), MYFILE_TYPE_FSNODE);
	}
	if (ap->mf_FileOperation.file_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.file_list), MYFILE_TYPE_FSNODE);
	}
	mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.shortcut_list), MYFILE_TYPE_FSNODE);

	ap->mf_Status.pPreNaviItem = ap->mf_MainWindow.pNaviItem;

	ap->mf_MainWindow.pNaviLayout = mf_object_create_layout(ap->mf_MainWindow.pNaviBar, EDJ_NAME, "view_layout");
	mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_normal);
	ap->mf_MainWindow.pNaviBox = mf_object_create_box(ap->mf_MainWindow.pNaviLayout);
	mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, ap->mf_MainWindow.pNaviBox);
	Evas_Object *pathinfo = mf_navi_bar_create_normal_pathinfo(ap);
	elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "pathinfo", pathinfo);
	if (flag_show) {
		newContent = mf_category_get_from_media_db(ap, ap->mf_Status.category_type, false);
		elm_box_pack_end(ap->mf_MainWindow.pNaviBox, newContent);
		//mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, newContent);
		evas_object_show(newContent);
	}

	if (ap->mf_Status.pPreNaviItem) {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar, ap->mf_Status.pPreNaviItem, NULL, NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	} else {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar, NULL, NULL, NULL, ap->mf_MainWindow.pNaviLayout, MF_NAVI_STYLE_ENABLE);
	}
//	Evas_Object *pImage = elm_image_add(ap->mf_MainWindow.pNaviBar);
//	elm_image_file_set(pImage, EDJ_IMAGE, MF_ICON_SOFT_BACK);
//	elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
//	evas_object_show(pImage);
//
//	Evas_Object *btn = elm_button_add(ap->mf_MainWindow.pNaviBar);
//	elm_object_content_set(btn, pImage);
//	elm_object_style_set(btn, "transparent");
//	evas_object_smart_callback_add(btn, "clicked", mf_category_view_back_cb, ap);
//	elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, "title_left_btn", btn);

//	Evas_Object *search_image = elm_image_add(ap->mf_MainWindow.pNaviLayout);
//	elm_image_file_set(search_image, EDJ_IMAGE, MF_TITLE_ICON_SEARCH);
//	elm_image_resizable_set(search_image, EINA_TRUE, EINA_TRUE);
//	evas_object_show(search_image);
//
//	Evas_Object *btn1 = elm_button_add(ap->mf_MainWindow.pNaviLayout);
//	elm_object_content_set(btn1, search_image);
//	evas_object_smart_callback_add(btn1, "clicked", mf_search_bar_enter_search_routine, ap);
//	elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "search_icon", btn1);
//	ap->mf_MainWindow.pButton = btn1;

	mf_navi_add_back_button(ap, mf_category_view_navi_back_cb);
	mf_category_view_set_ctrl_button(ap);

	/*add control bar for navigation bar*/

	//mf_navi_bar_title_set(ap);
	SAFE_DEL_NAVI_ITEM(&ap->mf_Status.pPreNaviItem);

	if (!flag_show) {
		int count = 0;
		count = mf_category_view_item_count_get(ap->mf_Status.category_type);
		mf_error("=================================== count is [%d]", count);
		mf_ecore_idler_del(mf_category_view_idler);
		if (count >= 500) {
			SAFE_FREE_OBJ(mf_category_view_popup);
			mf_category_view_popup = mf_popup_center_processing(ap, MF_LABEL_PROCESSING, NULL, NULL, NULL, EINA_FALSE);

			//mf_category_view_idler = ecore_idler_add((Ecore_Task_Cb)mf_category_view_content_create, ap);
			mf_category_get_from_media_db_by_thread(ap, ap->mf_Status.category_type, false);
		} else {
			mf_category_view_content_create(ap);
		}

	}
	//Evas_Object *pathinfo = mf_genlist_create_path_info(ap->mf_MainWindow.pNaviLayout, ap->mf_Status.categorytitle, EINA_FALSE);
	
	//elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "pathinfo", pathinfo);
	//elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_FALSE, EINA_FALSE);
	mf_navi_bar_title_content_set(ap, LABEL_MYFILE_CHAP);
	ap->mf_Status.flagViewAsRefreshView = EINA_FALSE;
	if (ap->mf_Status.more == MORE_EDIT_RENAME) {
		elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_FALSE);
		mf_navi_bar_title_content_set(ap, LABEL_RENAME);
	}
	t_end;
	/*temp data free*/
	MF_TRACE_END;
}


