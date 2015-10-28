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
#include "glib.h"
#include "assert.h"
#include "sys/statvfs.h"
#include <storage.h>
#include "pthread.h"

#include <device/power.h>
#include <system_settings.h>
#include <package_info.h>
#include <package_manager.h>
#include <utils_i18n.h>
#include <app_preference.h>

#include "mf-util.h"
#include "mf-object-conf.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-fs-util.h"
#include "mf-copy.h"
#include "mf-resource.h"
#include "mf-callback.h"
#include "mf-view.h"
#include "mf-object.h"
#include "mf-navi-bar.h"
#include "mf-genlist.h"
#include "mf-gengrid.h"
#include "mf-popup.h"
#include "mf-media-types.h"
#include "mf-media.h"
#include "mf-edit-view.h"
#include "mf-dlog.h"
#include "mf-fs-monitor.h"
#include "mf-file-util.h"

/* For preference */
#define MF_LIST_BY          "list_by"
#define MF_VIEW_STYLE	    "view_style"
#define MF_EXTENSION_STATE  "extension_state"
#define MF_HIDDEN_STATE		"hiden_state"
#define MF_RECENT_FILE		"recent_file"

#define MF_VIBRATION_DEVICE     0
#define MF_VIBRATION_DURATION   500
#define DEF_BUF_LEN            (2048)
#define MF_RECENT_FILES_SEP     ";"
#define MF_RECENT_FILES_COUNT_MAX   10
#define MF_TIMER_INTERVAL_VIBRATION 0.5

extern struct appdata *temp_data;
static int __mf_util_externalStorageId = 0;

int mf_util_get_storage_id()
{
	return __mf_util_externalStorageId;
}

bool __mf_util_get_Supported_Storages_Callback(int storageId, storage_type_e type, storage_state_e state, const char *path, void *userData)
{
	if (type == STORAGE_TYPE_EXTERNAL) {
		__mf_util_externalStorageId = storageId;
		return false;
	}

	return true;
}
/******************************
** Prototype    : __mf_util_is_mmc_supported
** Description  :
** Input        : int* supported
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
static int __mf_util_is_mmc_supported(int *supported)
{
	int error_code = -1;

	if (supported == NULL) {
		mf_debug("supported == NULL");
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	*supported = 0;
	error_code = storage_foreach_device_supported(__mf_util_get_Supported_Storages_Callback, NULL);
	if (error_code == STORAGE_ERROR_NONE) {
		storage_state_e state;
		storage_get_state(__mf_util_externalStorageId, &state);
		if (state != STORAGE_STATE_MOUNTED) {
			*supported = 0;
		} else {
			*supported = 1;
		}
	} else {
		return MYFILE_ERR_GET_STORAGE_FAIL;
	}

	return MYFILE_ERR_NONE;
}

/******************************
** Prototype    : mf_util_is_mmc_on
** Description  :
** Input        : int *mmc_card
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
int mf_util_is_mmc_on(int *mmc_card)
{
	int error_code = 0;

	error_code = __mf_util_is_mmc_supported(mmc_card);
	mf_debug("**************mmc_card is [%d]", *mmc_card);
	return error_code;

}


static Eina_Bool __mf_util_storage_exist_check(Evas_Object *genlist, int storage_type)
{
	mf_retvm_if(genlist == NULL, EINA_TRUE, "genlist is NULL");

	Elm_Object_Item *item = NULL;
	mf_list_data_t *item_data = NULL;

	item = elm_genlist_first_item_get(genlist);
	while (item) {
		item_data = elm_object_item_data_get(item);
		if (item_data) {
			if (item_data->storage_type == storage_type) {
				return EINA_TRUE;
			}
		}
		item = elm_genlist_item_next_get(item);
	}
	return EINA_FALSE;
}

void mf_util_action_storage_insert(void *data, char *pItemLabel)
{

	mf_debug();
	mf_retm_if(data == NULL, "passed data is NULL");
	struct appdata *ap = (struct appdata *)data;

	fsNodeInfo *pNode = NULL;

	if ((ap->mf_Status.view_type == mf_view_storage || ap->mf_Status.view_type == mf_view_root)
	    && (ap->mf_Status.more == MORE_DEFAULT || ap->mf_Status.more == MORE_INTERNAL_COPY || ap->mf_Status.more == MORE_INTERNAL_MOVE || ap->mf_Status.more == MORE_INTERNAL_DECOMPRESS)) {
		Evas_Object *parent = NULL;
		parent = ap->mf_MainWindow.pNaviGenlist;
		if ((ap->mf_Status.iStorageState & MYFILE_MMC) && !__mf_util_storage_exist_check(parent, MYFILE_MMC)) {
			pNode = (fsNodeInfo *) malloc(sizeof(fsNodeInfo));
			if (pNode == NULL)
				return;
			memset(pNode, 0, sizeof(fsNodeInfo));
			/*set path */
			pNode->path = g_strdup(STORAGE_PARENT);
			pNode->name = g_strdup(MMC_NAME);
			pNode->type = FILE_TYPE_DIR;
			pNode->storage_type = MYFILE_MMC;
			pNode->list_type = mf_list_normal;
			ap->mf_FileOperation.folder_list = eina_list_append(ap->mf_FileOperation.folder_list, pNode);
		}

		if ((ap->mf_Status.iStorageState & MYFILE_MMC) && !__mf_util_storage_exist_check(parent, MYFILE_MMC)) {
			mf_root_view_append_mmc_item_after_phone(parent, pNode, ap);
		}

	}
}

/******************************
** Prototype    : mf_util_operation_alloc_failed
** Description  :
** Input        : void *data
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
void mf_util_operation_alloc_failed(void *data)
{
	struct appdata *ap = (struct appdata *)data;

	ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT, NULL, MF_MSG_MEMORY_NOT_ENOUGH,
							       NULL, NULL, NULL, (Evas_Smart_Cb) elm_exit, NULL);
}

/******************************
** Prototype    : mf_util_refresh_screen
** Description  :
** Input        : void *data
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
void mf_util_refresh_screen(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "passed data is NULL");
	struct appdata *ap = (struct appdata *)data;

	char *message = NULL;
	if (ap->mf_Status.view_type == mf_view_root_category) {
		mf_util_check_pnode_list_items_exists(&ap->mf_FileOperation.category_list);
	}

	if (ap->mf_Status.more == MORE_IDLE_DELETE) {
		mf_view_state_reset_state_with_pre(ap);
		if (ap->mf_FileOperation.iOperationSuccessFlag) {
			elm_object_item_del(ap->mf_FileOperation.idle_delete_item);
			ap->mf_FileOperation.idle_delete_item = NULL;
			int view_type = mf_view_style_get(ap);
			if (view_type == MF_VIEW_STYLE_THUMBNAIL) {
				int count = elm_gengrid_items_count(ap->mf_MainWindow.pNaviGengrid);
				if (count == 0) {
					ap->mf_Status.flagNoContent = EINA_TRUE;
					Evas_Object *pContent = mf_object_create_no_content(ap->mf_MainWindow.pNaviBar);
					mf_object_text_set(pContent, MF_LABEL_NO_FILES, "elm.text");
					mf_navi_bar_set_content(ap, ap->mf_MainWindow.pNaviLayout, pContent);
				} else {
					mf_gengrid_align_set(ap->mf_MainWindow.pNaviGengrid, count);
				}
			}else{//Fixed P140416-05429  by jian12.li
				int count = elm_genlist_items_count(ap->mf_MainWindow.pNaviGenlist);
				if (count == 0) {
					ap->mf_Status.flagNoContent = EINA_TRUE;
					if (ap->mf_MainWindow.pNaviGenlist) {
						SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviGenlist);
						ap->mf_MainWindow.pNaviGenlist = NULL;
					}
					Evas_Object *pContent = mf_object_create_no_content(ap->mf_MainWindow.pNaviBar);
					mf_object_text_set(pContent, MF_LABEL_NO_FILES, "elm.text");
					mf_navi_bar_set_content(ap, ap->mf_MainWindow.pNaviLayout, pContent);
				}
			}
		} else {
			message = ap->mf_FileOperation.pOperationMsg;
		}

	} else if (ap->mf_Status.more == MORE_DELETE && ap->mf_FileOperation.iOperationSuccessFlag) {
		if (ap->mf_MainWindow.pFinishPopup) {
			evas_object_del(ap->mf_MainWindow.pFinishPopup);
			ap->mf_MainWindow.pFinishPopup = NULL;
		}
		if (ap->mf_MainWindow.pMmcRemovedPopup) {
			evas_object_del(ap->mf_MainWindow.pMmcRemovedPopup);
			ap->mf_MainWindow.pMmcRemovedPopup = NULL;
		}

		int view_type = mf_view_style_get(ap);
		Eina_List *folder_list = mf_edit_folder_list_get();
		Eina_List *file_list = mf_edit_file_list_get();
		Eina_List *l = NULL;
		Elm_Object_Item *it = NULL;

		EINA_LIST_FOREACH(folder_list, l, it) {
			if (it) {
				elm_object_item_del(it);
			}
		}
		EINA_LIST_FOREACH(file_list, l, it) {
			if (it) {
				elm_object_item_del(it);
			}
		}
		mf_view_state_reset_state_with_pre(ap);
		if (ap->mf_Status.more == MORE_EDIT || ap->mf_Status.more == MORE_EDIT_DELETE) {
			ap->mf_Status.more = MORE_DEFAULT;
		}
		SAFE_FREE_CHAR(ap->mf_Status.entry_path);
		ap->mf_Status.entry_more = MORE_DEFAULT;
			/*3.    refresh the content of the view */
		if (ap->mf_Status.view_type != mf_view_root_category && ap->mf_Status.more != MORE_SEARCH) {
			mf_navi_bar_recover_info_box(ap);
		}
		Evas_Object *btn = NULL;
		btn = elm_object_item_part_content_unset(ap->mf_MainWindow.pNaviItem, TITLE_RIGHT_BTN);
		SAFE_FREE_OBJ(btn);
		btn = elm_object_item_part_content_unset(ap->mf_MainWindow.pNaviItem, TITLE_LEFT_BTN);
		SAFE_FREE_OBJ(btn);

		if (view_type != MF_VIEW_STYLE_THUMBNAIL) {
			int count = 0;
			if (ap->mf_MainWindow.pNaviGenlist) {
				count = elm_genlist_items_count(ap->mf_MainWindow.pNaviGenlist);
			}
			if (count == 0) {
				ap->mf_Status.flagNoContent = EINA_TRUE;
				if (ap->mf_MainWindow.pNaviGenlist) {
					SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviGenlist);
					ap->mf_MainWindow.pNaviGenlist = NULL;
				}
				Evas_Object *pContent = mf_object_create_no_content(ap->mf_MainWindow.pNaviBar);
				mf_object_text_set(pContent, MF_LABEL_NO_FILES, "elm.text");
				evas_object_show(pContent);
				mf_navi_bar_set_content(ap, ap->mf_MainWindow.pNaviLayout, pContent);
			} else {
				Elm_Object_Item *it = NULL;
				it = elm_genlist_first_item_get(ap->mf_MainWindow.pNaviGenlist);
				while (it) {
					elm_genlist_item_update(it);
					it = elm_genlist_item_next_get(it);
				}
				mf_edit_view_select_all_layout_remove(ap);
			}
		} else {
			mf_error("more is [%d]", ap->mf_Status.more);
			int count = elm_gengrid_items_count(ap->mf_MainWindow.pNaviGengrid);
			if (count == 0) {
				ap->mf_Status.flagNoContent = EINA_TRUE;
				Evas_Object *pContent = mf_object_create_no_content(ap->mf_MainWindow.pNaviBar);
				mf_object_text_set(pContent, MF_LABEL_NO_FILES, "elm.text");
				mf_navi_bar_set_content(ap, ap->mf_MainWindow.pNaviLayout, pContent);
			} else {
				mf_gengrid_align_set(ap->mf_MainWindow.pNaviGengrid, count);
				Eina_List *realize_its;
				Elm_Object_Item *it;
				realize_its = elm_gengrid_realized_items_get(ap->mf_MainWindow.pNaviGengrid);
				EINA_LIST_FREE(realize_its, it) {
					if (it) {
						elm_object_item_signal_emit(it, "check,state,hide", "");
					}
				}
			}

		}
		mf_navi_bar_reset_ctrlbar(ap);
		mf_navi_add_back_button(ap, mf_callback_navi_backbutton_clicked_cb);
		/*4.	set tab enable */
		//mf_navi_bar_title_set(ap);

//		Evas_Object *pImage = elm_image_add(ap->mf_MainWindow.pNaviLayout);
//		elm_image_file_set(pImage, EDJ_IMAGE, MF_ICON_SOFT_BACK);
//		elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
//		evas_object_show(pImage);
//
//		Evas_Object *btn1 = elm_button_add(ap->mf_MainWindow.pNaviLayout);
//		elm_object_content_set(btn1,pImage);
//		evas_object_smart_callback_add(btn1, "clicked", mf_callback_backbutton_clicked_cb, ap);
//		//elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "back_key", btn);
//		elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, "title_left_btn", btn1);

		if (ap->mf_Status.more == MORE_DEFAULT) {
			mf_view_refresh(ap);
			if (ap->mf_Status.view_type == mf_view_root_category) {
				mf_navi_bar_title_content_set(ap, ap->mf_Status.categorytitle);
			} else if (ap->mf_Status.view_type == mf_view_recent) {
				mf_navi_bar_title_content_set(ap, MF_LABEL_RECTENT_FILES);
			} else {
				mf_navi_bar_title_content_set(ap,ap->mf_MainWindow.naviframe_title);
				elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_TRUE, EINA_TRUE);
			}
		} else if (ap->mf_Status.more != MORE_EDIT_RENAME) {
			elm_naviframe_item_title_enabled_set(ap->mf_MainWindow.pNaviItem, EINA_FALSE, EINA_FALSE);
		}
		if (ap->mf_Status.flagNoContent == 1) {
			if (ap->mf_MainWindow.pNaviCtrlBar) {
				mf_navi_bar_set_ctrlbar_item_disable(ap->mf_MainWindow.pNaviItem, CTRL_DISABLE_NOCONTENT_VIEW, TRUE);
			}
		}
		mf_fs_monitor_add_dir_watch(ap->mf_Status.path->str, ap);

	} else {
		if (ap->mf_Status.view_type == mf_view_root_category
		    && (ap->mf_Status.more == MORE_COMPRESS || ap->mf_Status.more == MORE_DECOMPRESS
		        || ap->mf_Status.more == MORE_DECOMPRESS_HERE)) {
			ap->mf_Status.more = MORE_DEFAULT;
			message =  MF_LABEL_SUCCESS;
			mf_category_view_create(ap, true);
		} else if ((ap->mf_Status.more == MORE_DECOMPRESS || ap->mf_Status.more == MORE_DECOMPRESS_HERE) && mf_view_get_pre_state(ap) == MORE_SEARCH) {
			mf_view_state_reset_state_with_pre(ap);

			if (ap->mf_FileOperation.iOperationSuccessFlag) {
				message =  MF_LABEL_SUCCESS;
			} else {
				message = ap->mf_FileOperation.pOperationMsg;
			}
			goto EXIT_WITH_POPUP;
		} else {
			/*0     set state to be Default */
			int current_more = ap->mf_Status.more;
			ap->mf_Status.more = MORE_DEFAULT;
			/*1     recover all the navigation bar */
			mf_view_update(ap);
			if (ap->mf_FileOperation.iOperationSuccessFlag) {
				switch (current_more) {
				case MORE_DATA_COPYING:
					message =  MF_MSG_COPY_SCCESS;
					break;
				case MORE_DATA_MOVING:
					message =  MF_MSG_MOVE_SUCCESS;
					break;
				case MORE_IDLE_DELETE:
				case MORE_DELETE:
					//label =  mf_util_get_text(MF_MSG_DELETE_SUCCESS);
					break;
				case MORE_COMPRESS:
					message =  MF_LABEL_COMPRESSED;
					break;
				case MORE_DECOMPRESS:
				case MORE_DECOMPRESS_HERE:
					message =  MF_LABEL_DECOMPRESSED;
					break;
				default:
					break;
				}
			} else {
				message =  ap->mf_FileOperation.pOperationMsg;
			}
		}
	}
	if (ap->mf_Status.flagNoContent == 1) {
		if (ap->mf_MainWindow.pNaviCtrlBar) {
			mf_navi_bar_set_ctrlbar_item_disable(ap->mf_MainWindow.pNaviItem, CTRL_DISABLE_NOCONTENT_VIEW, TRUE);
		}
		if (ap->mf_MainWindow.pNaviGenlist) {
			SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviGenlist);
			ap->mf_MainWindow.pNaviGenlist = NULL;
		}
	}
	if (ap->mf_MainWindow.pFinishPopup) {
		evas_object_del(ap->mf_MainWindow.pFinishPopup);
		ap->mf_MainWindow.pFinishPopup = NULL;
	}
	if (ap->mf_MainWindow.pMmcRemovedPopup) {
		evas_object_del(ap->mf_MainWindow.pMmcRemovedPopup);
		ap->mf_MainWindow.pMmcRemovedPopup = NULL;
	}

EXIT_WITH_POPUP:
	mf_error("ap->mf_FileOperation.message_type is [%d] message is [%s]",
                ap->mf_FileOperation.message_type, message);
    if (message == NULL)
    {
        mf_error("Message NULL..!!");
        return;
    }

	if (ap->mf_FileOperation.message_type == message_type_notification) {
		mf_popup_indicator_popup(ap, mf_util_get_text(message));
		ap->mf_FileOperation.pOperationMsg = NULL;
	} else {
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL,
								       message, MF_BUTTON_LABEL_OK,
								       NULL, NULL, mf_callback_warning_popup_cb, ap);
		ap->mf_FileOperation.pOperationMsg = NULL;
	}

	ap->mf_FileOperation.message_type = message_type_notification;
	return;
}



/******************************
** Prototype    : mf_util_check_forbidden_operation
** Description  :
** Input        : void *data
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
bool mf_util_check_forbidden_operation(void *data)
{
	/*
	 **    forbidden operations includes:
	 **    1.  Recursion move/copy
	 **    2.  move with same file in the same folder
	 */
		MF_TRACE_BEGIN

	mf_retvm_if(data == NULL, false, "passed data is NULL");
	struct appdata *ap = (struct appdata *)data;

	Eina_List *l = NULL;
	GString *pNode = NULL;
	GString *from = NULL;
	GString *to = ap->mf_FileOperation.destination;
	Eina_List *pSourceList = ap->mf_FileRecordList.value_saver;
	const char *message = NULL;

	EINA_LIST_FOREACH(pSourceList, l, pNode) {
		if (pNode) {
			from = pNode;
			if (mf_fm_svc_wrapper_detect_recursion(from, to) == MYFILE_REPORT_RECURSION_DETECT) {
				if (ap->mf_Status.more == MORE_INTERNAL_MOVE) {
					message = MF_MSG_MOVE_FAILED1;
					ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TITLE_TEXT_BTN, MF_POP_MOVE_WARNING_TITLE, message, MF_BUTTON_LABEL_OK, NULL, NULL, mf_popup_show_vk_cb, ap);

				} else if (ap->mf_Status.more == MORE_INTERNAL_COPY) {
					message = MF_MSG_COPY_FAILED1;
					ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TITLE_TEXT_BTN, MF_POP_COPY_WARNING_TITLE, message, MF_BUTTON_LABEL_OK, NULL, NULL, mf_popup_show_vk_cb, ap);
				}
				return false;
			} else if (mf_fm_svc_wrapper_detect_recursion(from, to) == MYFILE_REPORT_BOTH_ARE_SAME_FILE) {
				if (ap->mf_Status.more == MORE_INTERNAL_MOVE) {
					message = MF_MSG_MOVE_FAILED2;
					ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TITLE_TEXT_BTN, MF_POP_MOVE_WARNING_TITLE, message, MF_BUTTON_LABEL_OK, NULL, NULL, mf_popup_show_vk_cb, ap);
				} else if (ap->mf_Status.more == MORE_INTERNAL_COPY) {
					message = MF_MSG_COPY_FAILED1;
					ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TITLE_TEXT_BTN, MF_POP_COPY_WARNING_TITLE, message, MF_BUTTON_LABEL_OK, NULL, NULL, mf_popup_show_vk_cb, ap);
				}
				return false;

			}
			if (ap->mf_Status.more == MORE_INTERNAL_MOVE) {
				char *parent = mf_dir_get(pNode->str);
				if (parent && !strcmp(parent, to->str)) {
					message = MF_MSG_MOVE_FAILED2;
					ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TITLE_TEXT_BTN, MF_POP_MOVE_WARNING_TITLE, message, MF_BUTTON_LABEL_OK, NULL, NULL, mf_popup_show_vk_cb, ap);
					free(parent);
					return false;
				}
				if (parent) {
					free(parent);
				}
			}
		}
	}

	MF_TRACE_END
	return true;
}

/******************************
** Prototype    : mf_util_check_disk_space
** Description  :
** Input        : void *data
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
int mf_util_check_disk_space(void *data)
{
	mf_debug();
	mf_retvm_if(data == NULL, -1, "passed data is NULL");
	struct appdata *ap = (struct appdata *)data;

	MF_STORAGE state = MYFILE_NONE;
	unsigned long free_space = 0;

	state = mf_fm_svc_wrapper_get_location(ap->mf_Status.path->str);
	free_space = mf_fm_svc_wrapper_get_free_space(state);
	mf_debug("free size is %lu\n", free_space);
	/*
	 **     in vfat fs type, sector size is 16K.
	 **     it is to say that the limited size of the free space should be 16K
	 **     or it will report space used up.
	 **     check free_space == 0 can make sure at least 16K is free on the disk
	 **     while every dir takes 4K
	 */
	if (free_space == 0) {
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT_BTN, NULL,
								       MF_LABE_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS_AND_TRY_AGAIN, MF_BUTTON_LABEL_OK,
								       NULL, NULL, mf_callback_warning_popup_cb, ap);
		mf_debug("Not Enough free size\n");
		return MYFILE_ERR_NO_FREE_SPACE;
	}

	return MYFILE_ERR_NONE;
}

bool mf_util_is_low_memory(const char *path, unsigned long long size)
{
	mf_debug();
	mf_retvm_if(path == NULL, true, "path is NULL");

	unsigned long long free_space = 0;
	struct statvfs dst_fs;

	if (strncmp(path, PHONE_FOLDER, strlen(PHONE_FOLDER)) == 0) {
		if (storage_get_internal_memory_size(&dst_fs) < 0) {
			free_space = 0;
		}
		free_space = ((unsigned long long)(dst_fs.f_bsize) * (unsigned long long)(dst_fs.f_bavail));
	} else if (statvfs(path, &dst_fs) == 0) {
		free_space = ((unsigned long long)(dst_fs.f_bsize) * (unsigned long long)(dst_fs.f_bavail));
	} else {
		free_space = 0;
	}
	mf_error("=============== available device storage size is [%llu] input size is [%llu]", free_space, size);

	if (free_space >= size) {
		return false;
	}
	return true;
}

/******************************
** Prototype    : mf_util_get_eina_list_len
** Description  :
** Input        : const Eina_List *list
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
int mf_util_get_eina_list_len(const Eina_List *list)
{
	return eina_list_count(list);
}

void mf_util_normal_item_data_free(mfItemData_s **item_data)
{
	mf_retm_if(*item_data == NULL, "*item_data is NULL");

	SAFE_FREE_GSTRING((*item_data)->m_ItemName);
	SAFE_FREE_CHAR((*item_data)->thumb_path);
	SAFE_FREE_CHAR((*item_data)->create_date);
	SAFE_FREE_CHAR((*item_data)->size);

	if ((*item_data)->flagExpand) {
		(*item_data)->flagExpand = false;
	}

	if ((*item_data)->item) {
		(*item_data)->item = NULL;
	}

	if ((*item_data)->media) {
		int ret = MEDIA_CONTENT_ERROR_NONE;
		if ((*item_data)->thumbnail_create == EINA_TRUE) {
			ret = media_info_cancel_thumbnail((*item_data)->media);
		}
		if (ret == MEDIA_CONTENT_ERROR_NONE) {
		     media_info_destroy((*item_data)->media);
		    (*item_data)->media = NULL;
		}
	}
	free((*item_data));
	*item_data = NULL;
}

void do_list_pointer_protect(Eina_List *list)
{
	struct appdata *ap =  mf_get_appdata();

	/*mf_warning("list = %p", list);
	mf_warning("ap->search_result_folder_list = %p", ap->mf_FileOperation.search_result_folder_list);
	mf_warning("ap->search_result_file_list = %p", ap->mf_FileOperation.search_result_file_list);
	mf_warning("ap->folder_list = %p", ap->mf_FileOperation.folder_list);
	mf_warning("ap->file_list = %p", ap->mf_FileOperation.file_list);
	mf_warning("ap->category_list = %p", ap->mf_FileOperation.category_list);
	mf_warning("ap->recent_list = %p", ap->mf_FileOperation.recent_list);*/

	if (list==ap->mf_FileOperation.search_result_folder_list)
		ap->mf_FileOperation.search_result_folder_list = NULL;
	if (list==ap->mf_FileOperation.search_result_file_list)
		ap->mf_FileOperation.search_result_file_list = NULL;
	if (list==ap->mf_FileOperation.folder_list)
		ap->mf_FileOperation.folder_list = NULL;
	if (list==ap->mf_FileOperation.file_list)
		ap->mf_FileOperation.file_list = NULL;
	if (list==ap->mf_FileOperation.category_list)
		ap->mf_FileOperation.category_list = NULL;
	if (list==ap->mf_FileOperation.recent_list)
		ap->mf_FileOperation.recent_list = NULL;

}

void mf_util_free_eina_list_with_data(Eina_List **list, MYFILE_CONTENT_TYPE type)
{
	MF_TRACE_BEGIN;
	if (list == NULL || *list == NULL) {
		return;
	}

	void *pNode = NULL;
	Eina_List *l = NULL;

	switch (type) {
	case MYFILE_TYPE_GSTRING:
		EINA_LIST_FOREACH(*list, l, pNode) {
			GString *node = (GString *)pNode;
			SAFE_FREE_GSTRING(node);
		}
		break;
	case MYFILE_TYPE_CHAR:
		EINA_LIST_FOREACH(*list, l, pNode) {
			char *node = (char *)pNode;
			SAFE_FREE_CHAR(node);
		}
		break;
	case MYFILE_TYPE_FSNODE:
		EINA_LIST_FOREACH(*list, l, pNode) {
			fsNodeInfo *Node = (fsNodeInfo *)pNode;
			if (Node != NULL) {
				SAFE_FREE_CHAR(Node->path);
				SAFE_FREE_CHAR(Node->name);
				SAFE_FREE_CHAR(Node->ext);

				free(Node);
				Node = NULL;
			}
		}
		break;
	case MYFILE_TYPE_ITEM_DATA:
		EINA_LIST_FOREACH(*list, l, pNode) {
			mfItemData_s *Node = (mfItemData_s *)pNode;
			mf_util_normal_item_data_free(&Node);
		}
		break;
	default:
		break;
	}

	eina_list_free(*list);
	do_list_pointer_protect(*list);//Now, there is the pointer which is assigned to twice. there is the wild pointer, one pointer is free, but other one still isn't NULL.
	*list = NULL;
	MF_TRACE_END;
	return;
}

static void __mf_util_icu_set_default_timezone_id()
{
        i18n_uchar utimezone_id [MYFILE_ICU_ARR_LENGTH] = {0};
        char timezone_buffer[MYFILE_ICU_ARR_LENGTH] = {0};
        char timezone_id[MYFILE_ICU_ARR_LENGTH] = {0};
        char *buffer = NULL;
        int timezone_str_size;
        int retcode = -1;

        retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, &buffer);
		if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
                mf_error("[ERR] failed to get the timezone");
		}
        if (buffer)
                strncpy(timezone_id, buffer, sizeof(timezone_id)-1);
        timezone_str_size = readlink("/opt/etc/localtime", timezone_buffer, sizeof(timezone_buffer)-1);
        SAFE_FREE_CHAR(buffer);

        if (timezone_str_size > 0) {
                char *ptr, *sp, *zone= NULL, *city= NULL;
                ptr = strtok_r(timezone_buffer, "/", &sp);

                while ((ptr = strtok_r(NULL, "/", &sp))) {
                        zone = city;
                        city = ptr;
                }

                if (zone != NULL && city != NULL) {
                        if (strcmp("zoneinfo", zone) == 0)
                                snprintf(timezone_id, MYFILE_ICU_ARR_LENGTH, "%s", city);
                        else
                                snprintf(timezone_id, MYFILE_ICU_ARR_LENGTH, "%s/%s", zone, city);
                }
        }

        if (*timezone_id) {
                i18n_ustring_copy_ua_n(utimezone_id, timezone_id, sizeof(timezone_buffer)/2);
                retcode = i18n_ucalendar_set_default_timezone(utimezone_id);
        }
}

int mf_util_icu_init(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, MYFILE_ERR_INVALID_ARG, "passwd data is NULL");
	struct appdata *ap = (struct appdata *)data;
	char *skeleton = NULL;
	i18n_uchar customSkeleton[MYFILE_ICU_ARR_LENGTH] = {'\0'};
	int skeletonLength = 0;

	i18n_udatepg_h generator = NULL;
	i18n_udate_format_h formatter;
	i18n_uchar bestPattern[MYFILE_ICU_ARR_LENGTH] = {0,};
	int32_t bestPatternLength = 0;
	bool timeformat = false;

	__mf_util_icu_set_default_timezone_id();

	int ret = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, &timeformat);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		mf_debug();
		return MYFILE_ERR_SETTING_RESET_FAIL;
	}

	if (!timeformat) {
		skeleton = g_strdup(MYFILE_DATEFORMAT_12);
	} else {
		skeleton = g_strdup(MYFILE_DATEFORMAT_24);
	}

	skeletonLength = strlen(skeleton);
	if (i18n_ustring_copy_ua_n(customSkeleton, skeleton, skeletonLength) == NULL) {
		return MYFILE_ERR_INVALID_ARG;
	}

	char *region = NULL;
	int retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY, &region);
	if ((retcode != SYSTEM_SETTINGS_ERROR_NONE) || (region == NULL)) {
		//return MYFILE_ERR_INVALID_ARG;
		mf_info("Cannot get region format.");
		region = strdup("en_US");		// Default value.
	} else {
		char *find = strstr(region, "UTF-8");
		if (find) {
			int diff = find - region;
			if (diff > 0) {
				region[diff-1] = '\0';
			}
		}
	}

	i18n_ulocale_set_default(getenv("LC_TIME"));

	retcode = i18n_udatepg_create(region, &generator);
	if (generator == NULL) {
		return MYFILE_ERR_INVALID_ARG;
	}

	retcode = i18n_udatepg_get_best_pattern(generator, customSkeleton, i18n_ustring_get_length(customSkeleton), bestPattern, MYFILE_ICU_ARR_LENGTH, &bestPatternLength);
	if (bestPatternLength <= 0) {
		if (ap->mf_Status.generator != NULL) {
			i18n_udatepg_destroy(ap->mf_Status.generator);
			ap->mf_Status.generator = NULL;
		}
		return MYFILE_ERR_INVALID_ARG;
	}

	retcode = i18n_udate_create(I18N_UDATE_PATTERN, I18N_UDATE_PATTERN, region, NULL, -1, bestPattern, -1, &formatter);
	if (formatter == NULL) {
		if (ap->mf_Status.formatter) {
			i18n_udate_destroy(ap->mf_Status.formatter);
			ap->mf_Status.formatter = NULL;
		}
		return MYFILE_ERR_INVALID_ARG;
	}
	ap->mf_Status.generator = generator;
	ap->mf_Status.formatter = formatter;
	ap->mf_Status.flagIcuInit = TRUE;
	SAFE_FREE_CHAR(region);
	MF_TRACE_END;
	return MYFILE_ERR_NONE;
	}

void mf_util_icu_finalize(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "passwd data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.generator != NULL) {
		i18n_udatepg_destroy(ap->mf_Status.generator);
		ap->mf_Status.generator = NULL;
	}

	if (ap->mf_Status.formatter) {
		i18n_udate_destroy(ap->mf_Status.formatter);
		ap->mf_Status.formatter = NULL;
	}

	ap->mf_Status.flagIcuInit = FALSE;
	MF_TRACE_END;
}

char *mf_util_icu_translate(void *data, i18n_udate date, bool is_init_checking)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "passed data is NULL");
	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_Status.flagIcuInit == FALSE || is_init_checking) {
		//Fixed P131205-05353, don't need to init many times, it is tooooo slow.
		if (MYFILE_ERR_NONE != mf_util_icu_init(ap)) {
			mf_debug("invalid icu init check int");
			return NULL;
		}
	}

	int status = -1;
	i18n_uchar formatted[MYFILE_ICU_ARR_LENGTH] = { 0, };
	char formattedString[MYFILE_ICU_ARR_LENGTH] = { 0, };
	int32_t formattedLength;

	status = i18n_udate_format_date(ap->mf_Status.formatter, date, formatted, MYFILE_ICU_ARR_LENGTH,
                                  NULL, &formattedLength);

	if ((status != I18N_ERROR_NONE) || (formattedLength <= 0)) {
		mf_debug("firmatterdLength < 0");
		return NULL;
	}

	i18n_ustring_copy_au(formattedString, formatted);
	if (strlen(formattedString) == 0) {
		return NULL;
	}

	MF_TRACE_END;
	return g_strdup(formattedString);
}

int mf_util_get_pref_value(MYFILE_PREF_TYPE type, int *value)
{
    int ret = MYFILE_ERR_NONE;
    switch (type) {
        case PREF_TYPE_SORT_TYPE:
        {
            ret = preference_get_int(MF_LIST_BY, value);
            if (ret < 0) {
                mf_warning("fail to get list_by value, set it default");
                ret = preference_set_int(MF_LIST_BY, MYFILE_SORT_BY_DATE_R2O);
                *value = MYFILE_SORT_BY_DATE_R2O;
                mf_debug("set int : %d", ret);
            } else {
                if (*value <= MYFILE_SORT_BY_NONE || *value >= MYFILE_SORT_BY_MAX) 
                {
                    mf_warning("invaild list by type[%d], set it default", *value);
                    ret = preference_set_int(MF_LIST_BY, MYFILE_SORT_BY_DATE_R2O);
                    mf_debug("set int : %d", ret);
                    *value = MYFILE_SORT_BY_DATE_R2O;
                }
            }
            return MYFILE_ERR_NONE;
        }
            break;

        case PREF_TYPE_VIEW_STYLE:
            ret = preference_get_int(MF_VIEW_STYLE, value);
            if (ret <  0) {
                mf_warning("fail to get list_by value, set it default");
                ret = preference_set_int(MF_VIEW_STYLE, MF_VIEW_STYLE_LIST);
                *value = MF_VIEW_STYLE_LIST;
                mf_debug("ret %d", ret);
            } else {
                if (*value < MF_VIEW_STYLE_LIST || *value > MF_VIEW_STYLE_THUMBNAIL) {
                    mf_warning("invaild list by type[%d], set it default", *value);
                    ret = preference_set_int(MF_VIEW_STYLE, MF_VIEW_STYLE_LIST);
                    *value = MF_VIEW_STYLE_LIST;
                    mf_debug("ret %d", ret);
                }
            }
            break;

        case PREF_TYPE_EXTENSION_STATE:
            ret = preference_get_int(MF_EXTENSION_STATE, value);
            if (ret < 0) {
                mf_warning("fail to get list_by value, set it default");
                ret = preference_set_int(MF_EXTENSION_STATE, MF_EXTENSION_SHOW);
                *value = MF_EXTENSION_SHOW;
                mf_debug("ret : %d", ret);
            } else {
                if (*value < MF_EXTENSION_SHOW || *value > MF_EXTENSION_HIDE) 
                {
                    mf_warning("invaild list by type[%d], set it default", *value);
                    ret = preference_set_int(MF_EXTENSION_STATE, MF_EXTENSION_SHOW);
                    *value = MF_EXTENSION_SHOW;
                    mf_debug("ret : %d", ret);
                }
            }
            break;
        case PREF_TYPE_HIDEN_STATE:
            ret = preference_get_int(MF_HIDDEN_STATE, value);
            if (ret < 0) {
                mf_warning("fail to get list_by value, set it default");
                ret = preference_set_int(MF_HIDDEN_STATE, MF_HIDEN_HIDE);
                *value = MF_HIDEN_HIDE;
                mf_debug("ret : %d", ret);
            } else {
                if (*value < MF_HIDEN_SHOW|| *value > MF_HIDEN_HIDE) {
                    mf_warning("invaild list by type[%d], set it default", *value);
                    ret = preference_set_int(MF_HIDDEN_STATE, MF_HIDEN_HIDE);
                    *value = MF_HIDEN_HIDE;
                    mf_debug("ret : %d", ret);
                }
            }
            break;

        default:
            mf_debug("No case");
            break;
    }

    if (ret != MYFILE_ERR_NONE) {
        ret = MYFILE_ERR_STORAGE_GET_FAILED;
        mf_warning("MYFILE_ERR_STORAGE_GET_FAILED");
    }

    return ret;
}

void mf_util_set_sort_type(int value)
{
    int ret = -1;

    if (value <= MYFILE_SORT_BY_NONE || value >= MYFILE_SORT_BY_MAX)
    {
        ret = preference_set_int(MF_LIST_BY, MYFILE_SORT_BY_DATE_R2O);
        mf_warning("invaild list by type[%d], set it default, %d", value, ret);
    } else {
        ret = preference_set_int(MF_LIST_BY, value);
        mf_debug("value is [%d], [%d]", value, ret);
    }

    return;
}

void mf_util_set_extension_state(int value)
{
    int ret = -1;

    if (value <= MF_EXTENSION_NONE || value >= MF_EXTENSION_MAX) {
        ret = preference_set_int(MF_EXTENSION_STATE, MF_EXTENSION_SHOW);
        mf_warning("invaild list by type[%d], set it default : %d", value, ret);
    } else {
        ret = preference_set_int(MF_EXTENSION_STATE, value);
        mf_debug("value is [%d], [%d]", value, ret);
    }

    return;
}

void mf_util_set_hiden_state(int value)
{
    int ret = -1;

    if (value <= MF_HIDEN_NONE || value >= MF_HIDEN_MAX) {
        ret = preference_set_int(MF_HIDDEN_STATE, MF_HIDEN_HIDE);
        mf_warning("invaild hiden by type[%d], set it default : %d", value, ret);
    } else {
        ret = preference_set_int(MF_HIDDEN_STATE, value);
        mf_debug("value is [%d] [%d]", value, ret);
    }

    return;
}

void mf_util_set_view_style(int value)
{
    int ret = -1;

    if (value < MF_VIEW_STYLE_LIST || value > MF_VIEW_STYLE_THUMBNAIL) {
        ret = preference_set_int(MF_VIEW_STYLE, MF_VIEW_STYLE_LIST);
        mf_warning("invaild list by type[%d], set it default : %d", value, ret);
    } else {
        ret = preference_set_int(MF_VIEW_STYLE, value);
        mf_debug("value is [%d], [%d]", value, ret);
    }

    return;
}

void mf_util_set_recent_file(char *path)
{
    if (path == NULL) {
        mf_debug("path NULL");
        return;
    }

    int ret = -1;
    ret = preference_set_string(MF_RECENT_FILE, path);
    mf_debug("ret is [%d]", ret);

    return;
}

bool mf_util_db_get_recent_files_cb(MFRitem *Ritem, void *user_data)
{
	struct appdata *ap = (struct appdata *)user_data;
	if (Ritem && Ritem->path) {
		SECURE_ERROR("Ritem->path is [%s]mf_file_exists is [%d] access(dst_dir, R_OK | W_OK) is [%d] ",Ritem->path, mf_file_exists(Ritem->path),  access(Ritem->path, R_OK | W_OK));
		if (mf_file_exists(Ritem->path)) {
			mf_util_generate_list_prepend(&ap->mf_FileOperation.recent_list, g_strdup(Ritem->path), FILE_TYPE_ETC, mf_list_recent_files);
		} else {
			mf_media_delete_recent_files(ap->mf_MainWindow.mfd_handle, Ritem->path);
		}
	}
	return true;
}

void mf_util_db_get_recent_files(MFDHandle *handle, void *data)
{
	mf_media_foreach_recent_files_list(handle, mf_util_db_get_recent_files_cb, data);
}

void mf_util_db_add_recent_files(MFDHandle *handle, const char *path, const char *name, int storage, const char *thumbnail)
{
	mf_media_add_recent_files(handle, path, name, storage, thumbnail);
}

void mf_util_db_remove_recent_files(MFDHandle *handle, char *recent_file)
{
	mf_media_delete_recent_files(handle, recent_file);
}

void mf_util_generate_saved_files_list(void *data, int type)
{
	struct appdata *ap = (struct appdata *)data;
	switch (type) {
	case mf_list_recent_files:
		mf_util_db_get_recent_files(ap->mf_MainWindow.mfd_handle, data);
		break;
	}
}

void mf_util_merge_eina_list_to_glist(const Eina_List *eSource, GList **gSource)
{
	mf_retm_if(eSource == NULL, "eSource is NULL");
	mf_retm_if(gSource == NULL, "gSource is NULL");

	const Eina_List *l = NULL;
	GString *pNode = NULL;

	EINA_LIST_FOREACH(eSource, l, pNode) {
		if (pNode) {
			if (mf_file_exists(pNode->str)) {
				*gSource = g_list_append(*gSource, pNode->str);
			}
		}
	}
}

void mf_util_exception_func(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	switch (ap->mf_Status.more) {
	case MORE_DATA_COPYING:
		ap->mf_Status.more = MORE_INTERNAL_COPY;
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT, NULL,
								       MF_MSG_COPY_FAILED2, NULL, NULL, NULL, (Evas_Smart_Cb)mf_callback_exception_popup_cb, ap);
		break;
	case MORE_DATA_MOVING:
		ap->mf_Status.more = MORE_INTERNAL_MOVE;
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT, NULL,
								       MF_MSG_MOVE_FAILED3, NULL, NULL, NULL, (Evas_Smart_Cb)mf_callback_exception_popup_cb, ap);
		break;
	case MORE_DELETE:
		ap->mf_Status.more = MORE_DEFAULT;
		ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(ap, POPMODE_TEXT, NULL, MF_MSG_DEL_FAILED1, NULL, NULL, NULL, NULL, NULL);
		break;
	default:
		break;
	}
	MF_TRACE_END;
}

int mf_util_is_valid_name_check(const char *name)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(name == NULL, MF_INTERNAL_FILE_NAME_NULL, "name is NULL");
	gchar *temp_name = NULL;
	int ret = MYFILE_ERR_NONE;

	int length = strlen(name);
	if (length) {
		if (strncmp(name, ".", length) == 0 || strncmp(name, "..", length) == 0) {
			ret = MF_INTERNAL_FILE_NAME_IGNORE;
			goto EXIT;
		}

		temp_name = g_strconcat(name, NULL);
		if (strlen(g_strstrip(temp_name)) == 0) {
			ret = MF_INTERNAL_FILE_NAME_CHUG;
			goto EXIT;
		}
		if (mf_file_attr_is_valid_name(name) != MYFILE_ERR_NONE) {
			ret = MF_INTERNAL_FILE_NAME_INVALID_CHAR;
		}
	} else {
		ret = MF_INTERNAL_FILE_NAME_EMPTY;
	}

EXIT:
	if (temp_name) {
		g_free(temp_name);
		temp_name = NULL;
	}
	MF_TRACE_END;
	return ret;
}

void mf_util_set_pm_lock(void *data, Eina_Bool isLock)
{
    MF_TRACE_BEGIN;
    mf_retm_if(data == NULL, "data is NULL");

    int ret = -1;
    struct appdata *ap = (struct appdata *)data;

    if (ap->mf_Status.flagLCDLock != isLock) {
        if (ap->mf_Status.flagLCDLock == EINA_TRUE) {
            ap->mf_Status.flagLCDLock = EINA_FALSE;
        } else {
            ap->mf_Status.flagLCDLock = EINA_TRUE;
        }

        if (ap->mf_Status.flagLCDLock) {
            mf_debug("lock the LCD_OFF");
            ret = device_power_request_lock(POWER_LOCK_CPU, 0);
        } else {
            mf_debug("unlock the LCD_OFF");
            ret = device_power_release_lock(POWER_LOCK_CPU);
        }

        if (ret != 0)
            mf_debug("fail to lock(unlock)");
    }
    MF_TRACE_END;
}

long mf_util_character_count_get(const char *original)
{
	mf_retvm_if(original == NULL, 0, "input string is NULL");
	long count = 0;
	char *utf8_form = g_locale_to_utf8(original, -1, NULL, NULL, NULL);
	if (utf8_form == NULL)
		return count;
	else {
		  count = g_utf8_strlen(utf8_form, -1);
		  free(utf8_form);
		  mf_debug("utf8 count is %ld", count);
		  return count;
	}
}

gboolean mf_util_is_file_selected(Eina_List **source, GString *path)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(*source == NULL, FALSE, "source is NULL");
	mf_retvm_if(path == NULL, FALSE, "path is NULL");

	bool flag = FALSE;
	Eina_List *l = NULL;
	void *data = NULL;

	EINA_LIST_FOREACH(*source, l, data) {
		GString *source_path = (GString *)data;
		if (source_path && source_path->str) {
			mf_debug("source_path is [%s] path is [%s]", source_path->str, path->str);
			if (g_strcmp0(source_path->str, path->str) == 0) {
				flag = TRUE;
				mf_debug("flag is [%d]source_path is [%s] path is [%s]",flag, source_path->str, path->str);
				*source = eina_list_remove(*source, source_path);
				g_string_free(source_path, TRUE);
				source_path = NULL;
				int ret = eina_list_count(*source);
				if (ret == 0) {
					*source = NULL;
				}
				break;
			} else {
				continue;
			}
		}
	}
	MF_TRACE_END;
	return flag;
}

int mf_util_generate_list_data(const char *path, Eina_List **dir_list ,Eina_List ** file_list)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(path == NULL, 0, "input path is NULL");
	mf_retvm_if(dir_list == NULL, 0, "input dir_list is NULL");
	mf_retvm_if(file_list == NULL, 0, "input file_list is NULL");
	int error_code = 0;
	Eina_List *temp_dir_list = NULL;
	error_code = mf_fm_svc_wrapper_get_file_list(path, &temp_dir_list, file_list);

	if (error_code != MYFILE_ERR_NONE) {
		return error_code;
	}
	*dir_list = temp_dir_list;
	/*      classify the dir list to default and user defined       */
	MF_TRACE_END;
	return error_code;
}

int mf_util_generate_file_list(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retvm_if(data == NULL, MYFILE_ERR_INVALID_ARG, "data is null");
	struct appdata *ap = (struct appdata *)data;

	Eina_List *file_list = NULL;
	Eina_List *dir_list = NULL;
	int error_code = 0;

	mf_retvm_if (ap->mf_Status.path == NULL, MYFILE_ERR_INVALID_ARG, "ap->mf_Status.path is NULL");
	mf_retvm_if (ap->mf_Status.path->str == NULL, MYFILE_ERR_INVALID_ARG, "ap->mf_Status.path->str is NULL");

	error_code = mf_util_generate_list_data(ap->mf_Status.path->str, &dir_list, &file_list);
	if (error_code != MYFILE_ERR_NONE) {
		/*Todo: we need to free all the Eina_List*/
		t_end;
		return error_code;
	}


	if (ap->mf_FileOperation.folder_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.folder_list), MYFILE_TYPE_FSNODE);
	}
	if (ap->mf_FileOperation.file_list) {
		mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.file_list), MYFILE_TYPE_FSNODE);
	}

	ap->mf_FileOperation.file_list = file_list;
	ap->mf_FileOperation.folder_list = dir_list;
	t_end;
	MF_TRACE_END;

	return error_code;
}

int mf_util_generate_root_view_file_list(void *data, Eina_List **list, int storage_state)
{
	MF_TRACE_BEGIN;
	fsNodeInfo *pNode = NULL;
	mf_retvm_if(data == NULL, 0, "data is null");

	pNode = (fsNodeInfo *) malloc(sizeof(fsNodeInfo));
	if (pNode == NULL)
		return 0;

	/*set path */
	memset(pNode, 0, sizeof(fsNodeInfo));
	pNode->path = g_strdup(PHONE_PARENT);
	pNode->name = g_strdup(PHONE_NAME);
	mf_file_attr_get_file_stat(PHONE_FOLDER, &pNode);
	pNode->type = FILE_TYPE_DIR;
	pNode->storage_type = MYFILE_PHONE;
	pNode->list_type = mf_list_normal;
	*list = eina_list_append(*list, pNode);

	if (storage_state & MYFILE_MMC) {
		pNode = (fsNodeInfo *) malloc(sizeof(fsNodeInfo));
		if (pNode == NULL) {
			mf_error("pNode is NULL");
			return 0;
		}
		memset(pNode, 0, sizeof(fsNodeInfo));
		/*set path */
		pNode->path = g_strdup(STORAGE_PARENT);
		pNode->name = g_strdup(MMC_NAME);
		mf_file_attr_get_file_stat(MEMORY_FOLDER, &pNode);
		pNode->type = FILE_TYPE_DIR;
		pNode->storage_type = MYFILE_MMC;
		pNode->list_type = mf_list_normal;
		*list = eina_list_append(*list, pNode);
	}

	return 0;
}

void mf_util_sort_the_file_list(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is null");
	struct appdata *ap = (struct appdata *)data;

	int iSortTypeValue = 0;

	mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &iSortTypeValue);

	if (ap->mf_Status.view_type == mf_view_root_category) {
		mf_fs_oper_sort_list(&ap->mf_FileOperation.category_list, iSortTypeValue);
	}
	mf_fs_oper_sort_list(&ap->mf_FileOperation.file_list, iSortTypeValue);

	/*need to sort folder items only By Name and Date*/
	if (iSortTypeValue == MYFILE_SORT_BY_NAME_A2Z || iSortTypeValue == MYFILE_SORT_BY_NAME_Z2A || iSortTypeValue == MYFILE_SORT_BY_DATE_R2O
	    || iSortTypeValue == MYFILE_SORT_BY_DATE_O2R) {
		mf_fs_oper_sort_list(&ap->mf_FileOperation.folder_list, iSortTypeValue);
	} else {
		mf_fs_oper_sort_list(&ap->mf_FileOperation.folder_list, MYFILE_SORT_BY_NAME_A2Z);
	}
	t_end;
	MF_TRACE_END;
}

int mf_util_get_rotate_state_by_angle(int angle)
{
	if (angle == 90 || angle == 270) {
		return MF_ROTATE_LANDSCAPE;
	} else {
		return MF_ROTATE_PORTRAIT;
	}
}

int mf_util_rotate_state_get(Evas_Object *win)
{
	int type = MF_ROTATE_PORTRAIT;
	int angle = elm_win_rotation_get(win);
	mf_error("angle [%d]", angle);
	type = mf_util_get_rotate_state_by_angle(angle);
	return type;

}

const char *mf_util_search_markup_keyword(const char *string, char *searchword, bool *result)
{
	//SEARCH_FUNC_START;
	MF_TRACE_BEGIN;
	char pstr[DEF_BUF_LEN + 1] = {0,};
	static char return_string[DEF_BUF_LEN + 1] = { 0, };
	int word_len = 0;
	int search_len = 0;
	int translate_search_len = 0;
	int i = 0;
	bool found = false;
	gchar *markup_text_start = NULL;
	gchar *markup_text_end= NULL;
	gchar *markup_text= NULL;

	int r = 222;
	int g = 111;
	int b = 31;
	int a = 255;

	mf_retvm_if(string == NULL, NULL, "string is NULL");
	mf_retvm_if(searchword == NULL, NULL, "searchword is NULL");
	mf_retvm_if(result == NULL, NULL, "result is NULL");

	char *translate_keyword = elm_entry_markup_to_utf8(searchword);
	if (g_utf8_validate(string,-1,NULL)) {
		strncpy(pstr, string, DEF_BUF_LEN);

		word_len = strlen(pstr);
		if (translate_keyword) {
			translate_search_len = strlen(translate_keyword);
			if (translate_search_len) {
				search_len = translate_search_len;
				for (i = 0; i < word_len; i++) {
					if (!strncasecmp(translate_keyword, &pstr[i], search_len)) {
						found = true;
						break;
					}
				}
			} else {
				search_len = strlen(searchword);

				for (i = 0; i < word_len; i++) {
					if (!strncasecmp(searchword, &pstr[i], search_len)) {
						found = true;
						break;
					}
				}
			}
			free(translate_keyword);
			translate_keyword = NULL;
		} else {
			search_len = strlen(searchword);
			for (i = 0; i < word_len; i++) {
				if (!strncasecmp(searchword, &pstr[i], search_len)) {
					found = true;
					break;
				}
			}
		}

		*result = found;
		memset(return_string, 0x00, DEF_BUF_LEN+1);
		bool is_valid_length = ((i + search_len) <= word_len);//Fixed P131112-02074
		if (found && is_valid_length) {
			if (i == 0) {
				markup_text = g_markup_escape_text(&pstr[0], search_len);
				markup_text_end = g_markup_escape_text(&pstr[search_len], word_len-search_len);
				mf_retvm_if(markup_text == NULL, NULL, "markup_text is NULL");
				mf_retvm_if(markup_text_end == NULL, NULL, "markup_text_end is NULL");

				snprintf(return_string, DEF_BUF_LEN, "<color=#%02x%02x%02x%02x>%s</color>%s", r, g, b, a, markup_text, (char*)markup_text_end);
				SAFE_FREE_CHAR(markup_text);
				SAFE_FREE_CHAR(markup_text_end);
			} else {
				markup_text_start = g_markup_escape_text(&pstr[0], i);
				markup_text = g_markup_escape_text(&pstr[i], search_len);

				markup_text_end =  g_markup_escape_text(&pstr[i+search_len], word_len-(i+search_len));
				mf_retvm_if(markup_text_start == NULL, NULL, "markup_text_start is NULL");
				mf_retvm_if(markup_text == NULL, NULL, "markup_text is NULL");
				mf_retvm_if(markup_text_end == NULL, NULL, "markup_text_end is NULL");

				snprintf(return_string, DEF_BUF_LEN, "%s<color=#%02x%02x%02x%02x>%s</color>%s", (char*)markup_text_start, r, g, b, a, markup_text, (char*)markup_text_end);
				SAFE_FREE_CHAR(markup_text);
				SAFE_FREE_CHAR(markup_text_start);
				SAFE_FREE_CHAR(markup_text_end);
			}
		} else {
			snprintf(return_string, DEF_BUF_LEN, "%s", pstr);
		}
	}
	if (translate_keyword) {
		free(translate_keyword);
		translate_keyword = NULL;
	}
	MF_TRACE_END;

	return return_string;
}

char *mf_util_get_text(const char *ID)
{
	MF_CHECK_NULL(ID);
	char *str;

	if (strstr(ID, "IDS_COM"))
		str = dgettext("sys_string", ID);
	else
		str = gettext(ID);

	return str;
}

fsNodeInfo *mf_util_generate_pnode(const char *path, int file_type)
{
	mf_retvm_if(path == NULL, NULL, "path is NULL");
	fsNodeInfo *pNode = NULL;
	int error = MYFILE_ERR_NONE;

	int storage_type = mf_fm_svc_wrapper_get_location(path);
	pNode = (fsNodeInfo *) calloc(1, sizeof(fsNodeInfo));
	if (pNode == NULL) {
		return NULL;
	}
	memset(pNode, 0, sizeof(fsNodeInfo));
	char *parent = NULL;
	error = mf_file_attr_get_parent_path(path, &parent);
	if (error == MYFILE_ERR_NONE) {
		pNode->path = parent;
		pNode->name = g_strdup(mf_file_get(path));
		mf_file_attr_get_file_stat(path, &pNode);
		pNode->storage_type = storage_type;
		if (file_type == FILE_TYPE_DIR) {
			pNode->type = file_type;
		} else {
			mf_file_attr_get_file_category(path, &(pNode->type));
		}
		pNode->ext = NULL;
	} else {
		SAFE_FREE_CHAR(pNode);
	}
	return pNode;
}

void mf_util_generate_list(Eina_List **list, const char *path, int file_type, int list_type)
{
	mf_retm_if(list == NULL, "list is NULL");
	mf_retm_if(path == NULL, "path is NULL");

	fsNodeInfo *pNode = mf_util_generate_pnode(path, file_type);

	if (pNode) {
		pNode->list_type = list_type;
		*list = eina_list_append(*list, pNode);
	}
}

void mf_util_generate_list_prepend(Eina_List **list, const char *path, int file_type, int list_type)
{
	mf_retm_if(list == NULL, "list is NULL");
	mf_retm_if(path == NULL, "path is NULL");

	fsNodeInfo *pNode = mf_util_generate_pnode(path, file_type);
	char *ext = NULL;
	mf_file_attr_get_file_ext(path, &ext);
	if (pNode) {
		pNode->list_type = list_type;
		pNode->ext = ext;
		*list = eina_list_prepend(*list, pNode);
	}
}

void mf_util_remove_item_from_list_by_location(Eina_List **list, int location)
{
	mf_retm_if(list == NULL, "list is NULL");

	Eina_List *l = NULL;
	fsNodeInfo *node = NULL;

	EINA_LIST_FOREACH(*list, l, node) {
		if ((fsNodeInfo *)node != NULL && ((fsNodeInfo *)node)->path != NULL) {
			if (mf_fm_svc_wrapper_get_location(node->path) == location) {
				SAFE_FREE_CHAR(node->path);
				SAFE_FREE_CHAR(node->name);
				SAFE_FREE_CHAR(node->ext);
				SAFE_FREE_CHAR(node);
				*list = eina_list_remove_list(*list, l);
			}
		}
	}
}

void mf_util_update_item_from_list_by_name(Eina_List **list, const char *path, char *new_name)
{
	mf_retm_if(list == NULL, "list is NULL");

	Eina_List *l = NULL;
	fsNodeInfo *node = NULL;

	EINA_LIST_FOREACH(*list, l, node) {
		if ((fsNodeInfo *)node != NULL && ((fsNodeInfo *)node)->name != NULL && ((fsNodeInfo *)node)->path != NULL) {
			char *real_name = g_strconcat(((fsNodeInfo *)node)->path, "/", ((fsNodeInfo *)node)->name, NULL);
			SECURE_DEBUG("real is [%s] path is [%s]", real_name, path);
			if (g_strcmp0(real_name, path) == 0) {
				SAFE_FREE_CHAR(node->name);
				node->name = g_strdup(mf_file_get(new_name));
			}
			SAFE_FREE_CHAR(real_name);
		}
	}
}

void mf_util_remove_item_from_list_by_name(Eina_List **list, const char *path)
{
	mf_retm_if(list == NULL, "list is NULL");

	Eina_List *l = NULL;
	fsNodeInfo *node = NULL;

	EINA_LIST_FOREACH(*list, l, node) {
		if ((fsNodeInfo *)node != NULL && ((fsNodeInfo *)node)->name != NULL && ((fsNodeInfo *)node)->path != NULL) {
			char *real_name = g_strconcat(((fsNodeInfo *)node)->path, "/", ((fsNodeInfo *)node)->name, NULL);
			SECURE_DEBUG("real is [%s] path is [%s]", real_name, path);
			if (g_strcmp0(real_name, path) == 0) {
				SAFE_FREE_CHAR(node->path);
				SAFE_FREE_CHAR(node->name);
				SAFE_FREE_CHAR(node->ext);
				SAFE_FREE_CHAR(node);
				*list = eina_list_remove_list(*list, l);
				break;
			}
			SAFE_FREE_CHAR(real_name);
		}
	}
}

void mf_util_item_remove_invalid_category_items(Eina_List **list)
{
	MF_TRACE_BEGIN;
	mf_retm_if(list == NULL, "list is NULL");

	Eina_List *l = NULL;
	fsNodeInfo *node = NULL;

	EINA_LIST_FOREACH(*list, l, node) {
		if ((fsNodeInfo *)node != NULL && ((fsNodeInfo *)node)->name != NULL && ((fsNodeInfo *)node)->path != NULL) {
			char *real_name = g_strconcat(((fsNodeInfo *)node)->path, "/", ((fsNodeInfo *)node)->name, NULL);
			if (real_name != NULL && mf_file_exists(real_name)) {
				SAFE_FREE_CHAR(real_name);
				continue;
			} else {
				SAFE_FREE_CHAR(node->path);
				SAFE_FREE_CHAR(node->name);
				SAFE_FREE_CHAR(node->ext);
				SAFE_FREE_CHAR(node);
				*list = eina_list_remove_list(*list, l);
				SAFE_FREE_CHAR(real_name);
				mf_util_item_remove_invalid_category_items(list);
				MF_TRACE_END;
				return;
			}
		}
	}
	MF_TRACE_END;
}

bool mf_util_is_rotation_lock(void)
{
	MF_TRACE_BEGIN;
	bool lock = false;
	int retcode = -1;

	retcode = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_DISPLAY_SCREEN_ROTATION_AUTO, &lock);
	if (retcode == SYSTEM_SETTINGS_ERROR_NONE) {
		mf_debug("Rotation locked state[%d].", lock);
		MF_TRACE_END;
		return lock;
	} else {
		mf_debug("Get rotation lock state failed!");
		MF_TRACE_END;
		return false;
	}
}

void mf_util_free_data(void **data, int type)
{
	switch (type) {
	case MYFILE_TYPE_FSNODE:
	{
		if (*data != NULL) {
			SAFE_FREE_CHAR(((fsNodeInfo *)(*data))->path);
			SAFE_FREE_CHAR(((fsNodeInfo *)(*data))->name);
			SAFE_FREE_CHAR(((fsNodeInfo *)(*data))->ext);

			free((fsNodeInfo *)(*data));
			*data = NULL;
		}
	}
		break;
	case MYFILE_TYPE_ITEM_DATA:
	{
		if (*data != NULL) {
			SAFE_FREE_GSTRING(((mfItemData_s *)(*data))->m_ItemName);
			SAFE_FREE_CHAR(((mfItemData_s *)(*data))->size);
			SAFE_FREE_CHAR(((mfItemData_s *)(*data))->create_date);
			SAFE_FREE_CHAR(((mfItemData_s *)(*data))->thumb_path);
			if (((mfItemData_s *)(*data))->media) {
				media_info_cancel_thumbnail(((mfItemData_s *)(*data))->media);
				media_info_destroy(((mfItemData_s *)(*data))->media);
				((mfItemData_s *)(*data))->media = NULL;
			}

			free((mfItemData_s *)(*data));
			*data = NULL;
		}
	}
		break;
	default:
		break;
	}
}

void mf_util_check_pnode_list_items_exists(Eina_List **list)
{
	mf_retm_if(list == NULL || *list == NULL, "list is NULL");
	Eina_List *l = NULL;
	fsNodeInfo *pNode = NULL;
	EINA_LIST_FOREACH(*list, l, pNode) {
		if (pNode && pNode->name && pNode->path) {
			char *real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);
			SECURE_DEBUG("real is [%s]", real_name);
			if (real_name) {
				if (mf_file_exists(real_name)) {
					SAFE_FREE_CHAR(real_name);
				} else {
					SAFE_FREE_CHAR(pNode->path);
					SAFE_FREE_CHAR(pNode->name);
					SAFE_FREE_CHAR(pNode->ext);
					SAFE_FREE_CHAR(pNode);
					*list = eina_list_remove_list(*list, l);
					SAFE_FREE_CHAR(real_name);
				}
			} else {
				*list = eina_list_remove_list(*list, l);
				SAFE_FREE_CHAR(real_name);
			}
		}
	}
}

static inline gboolean __mf_util_has_nonspacing_mark(const char *nstr)
{
	if (nstr) {
		const char *p_str = nstr;
		while (p_str && *p_str) {
			gunichar uc;
			uc = g_utf8_get_char(p_str);
			if (g_unichar_type(uc) == G_UNICODE_NON_SPACING_MARK) {
				return TRUE;
			} else {
				p_str = g_utf8_next_char(p_str);
			}
		}
	}
	return FALSE;
}

gboolean mf_util_NFD_strstr(const char *nor_str, const char *nor_needle)
{
	if (nor_str == NULL || strlen(nor_str) == 0) {
		return FALSE;
	}
	if (nor_needle == NULL || strlen(nor_needle) == 0) {
		return FALSE;
	}

	const char *name = mf_file_get(nor_str);
	if (name == NULL || strlen(name) == 0) {
		return FALSE;
	}
	char *up_name = elm_entry_utf8_to_markup(nor_str);//g_utf8_strup(entry->d_name, strlen(entry->d_name));
	gchar *str = g_utf8_strup(up_name, strlen(up_name));
	char *needle = g_utf8_strup(nor_needle, strlen(nor_needle));
	int s_len = 0;
	int n_len = 0;

	if (!str) {
		goto EXIT;
	}
	s_len = strlen(str);

	if (!needle) {
		goto EXIT;
	} else {
		n_len = strlen(needle);
		if (n_len == 0) {
			mf_error();
			goto EXIT;
		}
	}

	if (s_len < n_len) {
		goto EXIT;
	}

	if (__mf_util_has_nonspacing_mark(str)) {
		const char *p_str = str;
		const char *end = p_str + s_len - n_len;

		while (p_str && p_str <= end && *p_str) {
			const char *s = p_str;
			const char *n = needle;
			while (n && *n) {
				if (s && *s) {
					gunichar sc, nc;
					sc = g_utf8_get_char(s);
					nc = g_utf8_get_char(n);
					if (g_unichar_type(sc) == G_UNICODE_NON_SPACING_MARK) {
						if (g_unichar_type(nc) == G_UNICODE_NON_SPACING_MARK) {
							if (sc != nc) {
								goto next;
							} else {
								s = g_utf8_next_char(s);
								n = g_utf8_next_char(n);
							}
						} else {
							s = g_utf8_next_char(s);
						}
					} else if (sc != nc) {
						goto next;
					} else {
						s = g_utf8_next_char(s);
						n = g_utf8_next_char(n);
					}
				} else {
					goto EXIT;
				}
			}
			goto EXIT;
next:
			p_str = g_utf8_next_char(p_str);
		}
	} else {
		gboolean result = !(!strstr(str, needle));
		SAFE_FREE_CHAR(needle);
		SAFE_FREE_CHAR(str);
		SAFE_FREE_CHAR(up_name);
		return result;
	}
EXIT:
	SAFE_FREE_CHAR(needle);
	SAFE_FREE_CHAR(str);
	SAFE_FREE_CHAR(up_name);
	return FALSE;
}

static Eina_List *entry_path_stack = NULL;

void mf_util_path_push(char *path)
{
	mf_retm_if(path == NULL, "path is NULL");
	char *fullpath = NULL;
	fullpath = g_strdup(path);
	if (fullpath) {
		entry_path_stack = eina_list_prepend(entry_path_stack, fullpath);
	}
}

char *mf_util_path_pop()
{

	char *fullpath = NULL;
	fullpath = eina_list_nth(entry_path_stack, 0);
	if (fullpath) {
		entry_path_stack = eina_list_remove(entry_path_stack, fullpath);
	}
	return fullpath;
}

char *mf_util_path_top_get()
{
	char *fullpath = eina_list_nth(entry_path_stack, 0);
	if (fullpath) {
		mf_error("================= top path is [%s]", fullpath);
	} else {
		mf_error("Failed to get the top");
	}
	return fullpath;
}

void mf_util_path_stack_free()
{
	if (entry_path_stack) {
		char *fullpath = NULL;
		Eina_List *l = NULL;

		EINA_LIST_FOREACH(entry_path_stack, l, fullpath) {
			SAFE_FREE_CHAR(fullpath);
		}
		eina_list_free(entry_path_stack);
		entry_path_stack = NULL;
	}
}

void mf_util_first_item_push(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");
	if (!ap->mf_Status.flagNoContent) {
		int view_style = mf_view_style_get(ap);
		char *first_item_fullpath = NULL;
		if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
			first_item_fullpath = mf_genlist_first_item_name_get(ap->mf_MainWindow.pNaviGenlist);
		}
		mf_util_path_push(first_item_fullpath);
	}
}

char *mf_util_first_item_get(char *path)
{
	mf_retvm_if(path == NULL, NULL, "path is NULL");
	char *top_fullpath = NULL;
	top_fullpath = mf_util_path_top_get();
	if (top_fullpath) {
		char *parent = mf_dir_get(top_fullpath);
		mf_error("path is [%s] top is [%s]", path, parent);
		if (g_strcmp0(path, parent) == 0) {
			SAFE_FREE_CHAR(parent);
			return mf_util_path_pop();
		}
		SAFE_FREE_CHAR(parent);
	}
	return NULL;
}

