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
#include "mf-conf.h"
#include "mf-fs-util.h"
#include "mf-launch.h"
#include "mf-resource.h"
#include "mf-object-conf.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-popup.h"
#include "mf-navi-bar.h"
#include "mf-callback.h"
#include "mf-media-types.h"
#include "mf-media.h"
#include "mf-view.h"
#include "mf-edit-view.h"
#include "mf-file-util.h"
#include <mime_type.h>

#define DETAIL_UG_NAME			"myfile-detail-efl-lite"
#define MYFILE_UG_NAME			"ug-myfile-efl"
#define STORAGE_UG_NAME			"setting-storage-efl"


#define RECENT_FILES_COUNT_MAX			15

typedef struct _mf_share_as_video_operation {
	Ecore_Thread *sound_image_thread; /*For encode sound image*/
	Ecore_Pipe *sync_pipe;	   /* Pipe for processing bar showing*/
	char *videofile; /*generated video file*/
} mf_share_as_video_operation;
mf_share_as_video_operation share_operation;

/*for the edit mode to save image list path*/
typedef struct _mf_edit_sound_image_t mf_edit_sound_image_s;
struct _mf_edit_sound_image_t {
	char **files;
	int count;
};

/*
static void __mf_launch_ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	MF_TRACE_BEGIN;
	mf_retm_if(priv == NULL, "priv is NULL");

	Evas_Object *base = NULL;

	base = ug_get_layout(ug);
	if (!base) {
		ug_destroy(ug);
		return;
	}

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(base);
		break;
	default:
		break;
	}
}

static void __mf_launch_ug_destory_cb(ui_gadget_h ug, void *priv)
{
	MF_TRACE_BEGIN;
	mf_retm_if(priv == NULL, "priv is NULL");

	ug_destroy(ug);
	struct appdata *ap = (struct appdata *)priv;
	ap->mf_SharedGadget.ug = NULL;
	ap->mf_Status.flagIME = EINA_TRUE;
	ap->mf_SharedGadget.location = MYFILE_PHONE;

	MF_TRACE_END;
}
*/

static int __mf_launch_get_share_files(mf_launch_share_u *selected_files, Eina_List *selected_list, int flag)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(selected_list == NULL, 0, "selected_list is NULL");
	mf_retvm_if(selected_files == NULL, -1, "selected_files is NULL");

	int file_count = 0;
	Eina_List *l = NULL;
	char *first_file = NULL;
	GString *fullpath = NULL;
		EINA_LIST_FOREACH(selected_list, l, fullpath) {
			if (fullpath != NULL) {
				if (first_file == NULL && selected_files->multi_files == NULL) {
					first_file = g_strdup(fullpath->str);
				} else {
					if (selected_files->multi_files == NULL) {
						selected_files->multi_files = calloc(eina_list_count(selected_list), sizeof(char *));
						if (selected_files->multi_files) {
							selected_files->multi_files[0] = g_strdup(first_file);
							SAFE_FREE_CHAR(first_file);
							selected_files->multi_files[file_count] = g_strdup(fullpath->str);
						}
					} else {
						selected_files->multi_files[file_count] = g_strdup(fullpath->str);
					}
				}
				file_count++;
			}
		}

	if (first_file) {
		selected_files->single_file = first_file;
	}
	MF_TRACE_END;
	return file_count;
}

void mf_launch_item_share_file(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mfItemData_s *item_data = (mfItemData_s *)data;
	mf_retm_if(item_data == NULL, "input data is NULL");

	struct appdata *ap = (struct appdata *)item_data->ap;
	mf_retm_if(ap == NULL, "input ap is NULL");

	int ret = 0;

	app_control_h handle = NULL;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_error(" app_control_create failed");
		return ;
	}
	mf_error("item_data->m_ItemName->str is %s",item_data->m_ItemName->str);

	char prefix_file[BUFF_SIZE] = {0,};
	snprintf(prefix_file, BUFF_SIZE, "%s%s", MF_SHARE_FILE_PREFIX, item_data->m_ItemName->str);
	ret = app_control_set_uri(handle, prefix_file);
	if (ret != APP_CONTROL_ERROR_NONE) {
		app_control_destroy(handle);
		mf_debug("app_control_set_uri()... [0x%x]", ret);
		return;
	}
	ret = app_control_set_operation(handle, MF_SHARE_OPERATION_SINGLE);
	if (ret != APP_CONTROL_ERROR_NONE) {
		app_control_destroy(handle);
		mf_debug("app_control_set_operation()... [0x%x]", ret);
		return;
	}
	//app_control_add_extra_data_array(handle, MF_SHARE_SVC_FILE_PATH, &item_data->m_ItemName->str, 1);
	app_control_add_extra_data(handle, MF_SHARE_SVC_FILE_PATH, item_data->m_ItemName->str);
/*
	ret = app_control_set_window(handle, elm_win_xwindow_get(ap->mf_MainWindow.pWindow));
	if (ret != APP_CONTROL_ERROR_NONE) {
		app_control_destroy(handle);
		mf_debug("app_control_set_window()... [0x%x]", ret);
		return;
	}
*/
	ret = app_control_send_launch_request(handle, NULL, NULL);
	if (ret == APP_CONTROL_ERROR_APP_NOT_FOUND) {
		mf_popup_create_popup(ap, POPMODE_TEXT, NULL, MF_LABEL_NO_APP, NULL, NULL, NULL, NULL, NULL);
	} else if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("app_control_send_launch_request()... [0x%x]", ret);
	}

	if (handle) {
		app_control_destroy(handle);
	}
}

/*bool _gl_db_update_lock_always(void *data, bool status)
{
	gl_dbg("");
	GL_CHECK_FALSE(data);
	gl_appdata *ad = (gl_appdata *)data;
	GL_CHECK_FALSE(ad->db_noti_d);
	gl_db_noti_s *db_noti = ad->db_noti_d;
	if (status)
		db_noti->lock_state = GL_DU_LOCK_ALWAYS;
	else
		db_noti->lock_state = GL_DU_LOCK_NONE;
	return true;
}*/


#ifdef _USE_SHARE_PANEL
void mf_launch_item_share(void *data, Evas_Object * obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mfItemData_s *item_data = (mfItemData_s *)data;
	mf_retm_if(item_data == NULL, "input data is NULL");

	struct appdata *ap = (struct appdata *)item_data->ap;
	mf_retm_if(ap == NULL, "input ap is NULL");
	mf_info("pLongpressPopup is deleted");
	SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	//other files will share directly
	mf_launch_item_share_file(data,obj,event_info);
	MF_TRACE_END;
}

static void __mf_launch_share_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(user_data == NULL, "user_data is NULL");
	struct appdata *ap = (struct appdata *)user_data;

	switch (result) {
	case APP_CONTROL_RESULT_APP_STARTED:
		if (ap->mf_Status.share) {
			ap->mf_Status.share = 0;
			if (ap->mf_Status.more == MORE_SHARE_EDIT) {
				mf_callback_cancel_cb(ap, NULL, NULL);
			}
		} else {
			ap->mf_Status.share = 1;
		}
		mf_debug("APP_CONTROL_RESULT_APP_STARTED");
		break;
	case APP_CONTROL_RESULT_SUCCEEDED:
		if (ap->mf_Status.more == MORE_SHARE_EDIT) {
			mf_callback_cancel_cb(ap, NULL, NULL);
		}
		mf_debug("APP_CONTROL_RESULT_SUCCEEDED");
		break;
	case APP_CONTROL_RESULT_FAILED:
		if (ap->mf_Status.extra == MORE_SEARCH) {
			mf_edit_file_list_clear();
		}
		ap->mf_Status.share = 0;
		mf_debug("APP_CONTROL_RESULT_FAILED");
		break;
	case APP_CONTROL_RESULT_CANCELED:
		if (ap->mf_Status.extra == MORE_SEARCH) {
			mf_edit_file_list_clear();
		}
		ap->mf_Status.share = 0;
		mf_debug("APP_CONTROL_RESULT_CANCELED");
		break;
	default:
		mf_debug("Unhandled value: %d!", result);
		break;
	}
	MF_TRACE_END;
}

bool mf_launch_share(void *data)
{
	mf_retvm_if(data == NULL, false, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Eina_List *select_list = NULL;
	app_control_h handle = NULL;
	mf_launch_share_u share_file;
	int count = 0;
	int flag = false;
	int ret = 0;
	ap->mf_Status.share = 0;
	if (ap->mf_FileRecordList.selected_files) {
		mf_util_free_eina_list_with_data(&ap->mf_FileRecordList.selected_files, MYFILE_TYPE_GSTRING);
	}
	ap->mf_FileRecordList.selected_files = mf_edit_get_all_selected_files();

	select_list = ap->mf_FileRecordList.selected_files;

	memset(&share_file, 0x00, sizeof(mf_launch_share_u));


	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_error("app_control_create failed");
		return false;
	}

	count = __mf_launch_get_share_files(&share_file, select_list, flag);
	mf_error(" count is [%d]", count);

	char prefix_file[BUFF_SIZE] = {0,};
	if (count == 1) {
		snprintf(prefix_file, BUFF_SIZE, "%s%s", MF_SHARE_FILE_PREFIX, share_file.single_file);
		ret = app_control_set_uri(handle, prefix_file);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_set_uri()... [0x%x]", ret);
			goto END;
		}

		ret = app_control_set_operation(handle, MF_SHARE_OPERATION_SINGLE);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_set_operation()... [0x%x]", ret);
			goto END;
		}
		ret = app_control_enable_app_started_result_event(handle);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_enable_app_started_result_event()... [0x%x]", ret);
			goto END;
		}
		//app_control_add_extra_data_array(handle, MF_SHARE_SVC_FILE_PATH, &share_file.single_file, 1);
		app_control_add_extra_data(handle, MF_SHARE_SVC_FILE_PATH, share_file.single_file);
	} else if (count > 1) {
		snprintf(prefix_file, BUFF_SIZE, "%s%s", MF_SHARE_FILE_PREFIX, share_file.multi_files[0]);
		ret = app_control_set_uri(handle, prefix_file);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_set_uri()... [0x%x]", ret);
			goto END;
		}

		ret = app_control_set_operation(handle, MF_SHARE_OPERATION_MULTIPLE);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_set_operation()... [0x%x]", ret);
			goto END;
		}
		ret = app_control_enable_app_started_result_event(handle);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_enable_app_started_result_event()... [0x%x]", ret);
			goto END;
		}
		ret = app_control_add_extra_data_array(handle, MF_SHARE_SVC_FILE_PATH, (const char **)&share_file.multi_files[0], count);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_add_extra_data_array()... [0x%x]", ret);
			goto END;
		}
	} else {
		goto END;
	}
/*
	ret = app_control_set_window(handle, elm_win_xwindow_get(ap->mf_MainWindow.pWindow));
	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("app_control_set_window()... [0x%x]", ret);
		goto END;
	}
*/

	ret = app_control_set_launch_mode(handle, APP_CONTROL_LAUNCH_MODE_GROUP);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("app_control_set_launch_mode()... [0x%x]", ret);
		goto END;
	}

	ret = app_control_send_launch_request(handle, __mf_launch_share_reply_cb, ap);
	if (ret == APP_CONTROL_ERROR_APP_NOT_FOUND) {
		mf_popup_create_popup(data, POPMODE_TEXT, NULL, MF_LABEL_NO_APP, NULL, NULL, NULL, NULL, NULL);
	} else if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("app_control_send_launch_request()... [0x%x]", ret);
	}

	if (handle) {
		app_control_destroy(handle);
	}
	if (count == 1) {
		SAFE_FREE_CHAR(share_file.single_file);

	} else if (count > 1) {
		int i = 0;
		for (i = 0; i < count; i++) {
			SAFE_FREE_CHAR(share_file.multi_files[i]);
		}
		SAFE_FREE_CHAR(share_file.multi_files);
	}
	return true;
END:
	if (count == 1) {
		SAFE_FREE_CHAR(share_file.single_file);

	} else if (count > 1) {
		int i = 0;
		for (i = 0; i < count; i++) {
			SAFE_FREE_CHAR(share_file.multi_files[i]);
		}
		SAFE_FREE_CHAR(share_file.multi_files);
	}
	if (handle) {
		app_control_destroy(handle);
	}
	return false;

}
#endif

/******************************
** Prototype    : mf_launch_service
** Description  :
** Input        : void *data
**                char *path
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
void result_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	MF_TRACE_BEGIN;
	mf_error("===================== result is [%d]", result);

	struct appdata *ap = (struct appdata *)user_data;
	if (result == APP_CONTROL_RESULT_APP_STARTED) {
		mf_launch_add_recent_files(ap, ap->mf_Status.launch_path);
		mf_util_set_recent_file(ap->mf_Status.launch_path);
	}
	SAFE_FREE_CHAR(ap->mf_Status.launch_path);
	ap->mf_SharedGadget.ug = NULL;
	ap->mf_Status.flagIME = EINA_TRUE;
	ap->mf_SharedGadget.location = MYFILE_PHONE;
	MF_TRACE_END;
}

void mf_launch_load_ug(void *data, char *path, MF_LOAD_UG_TYPE type, Eina_Bool multi_flag)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(path == NULL, "path is NULL");

	struct appdata *ap = (struct appdata *)data;
	//struct ug_cbs cbs = { 0, };

	app_control_h app_control;
	int ret = 0;
	ret = app_control_create(&app_control);

	mf_retm_if(ret != APP_CONTROL_ERROR_NONE, "app_control create failed");

	//cbs.layout_cb = __mf_launch_ug_layout_cb;
	//cbs.result_cb = __mf_launch_ug_result_cb;
	//cbs.destroy_cb = __mf_launch_ug_destory_cb;
	//cbs.priv = ap;

	app_control_add_extra_data(app_control, "Path", path);
	if (multi_flag) {
		app_control_add_extra_data(app_control, "multi", "true");
	}
	ap->mf_SharedGadget.location = mf_fm_svc_wrapper_get_location(path);
	ret = app_control_send_launch_request(app_control, result_cb, ap);

	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("Fail to Create UG : %d", type);
		app_control_destroy(app_control);
		MF_TRACE_END;
		return;
	} else {
		ap->mf_SharedGadget.ug = app_control;
		ap->mf_Status.flagIME = EINA_FALSE;
		app_control_destroy(app_control);
		MF_TRACE_END;
		return;
	}
}

void mf_launch_load_storage(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	//struct appdata *ap = (struct appdata *)data;

	app_control_h app_control;
	int ret = 0;
	ret = app_control_create(&app_control);

	mf_retm_if(ret != APP_CONTROL_ERROR_NONE, "app_control create failed");

	ret = app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("app_control_set_operation()... [0x%x]", ret);
		goto END;
	}
/*
	ret = app_control_set_window(app_control, elm_win_xwindow_get(ap->mf_MainWindow.pWindow));
	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("app_control_set_uri()... [0x%x]", ret);
		goto END;
	}
*/
	ret = app_control_set_app_id(app_control, STORAGE_UG_NAME);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("app_control_set_uri()... [0x%x]", ret);
		goto END;
	}


	ret = app_control_send_launch_request(app_control, NULL, NULL);
END:
	if (app_control) {
		app_control_destroy(app_control);
	}
	return;
}

char * mf_launch_item_size_calculate(double size)
{
	MF_TRACE_BEGIN;
	double dsize = 0.0;
	int count = 0;
	char *p_size = NULL;
	dsize = size;
	while (dsize >= MYFILE_BASIC_SIZE) {
		dsize /= MYFILE_BASIC_SIZE;
		count++;
	}
	char *unit = NULL;
	if (count == SIZE_BYTE) {
		unit = strdup(mf_util_get_text(MF_LABEL_SIZE_B));  //fix P131121-00236 by ray
	} else if (count == SIZE_KB) {
		unit = strdup(mf_util_get_text(MF_LABEL_SIZE_K));
	} else if (count == SIZE_MB) {
		unit = strdup(mf_util_get_text(MF_LABEL_SIZE_M));
	} else if (count == SIZE_GB) {
		unit = strdup(mf_util_get_text(MF_LABEL_SIZE_G));
	}
	p_size = g_strdup_printf("%.2f %s", dsize, unit);
	SAFE_FREE_CHAR(unit);
	MF_TRACE_END;

	return p_size;
}

Ecore_Timer *g_mf_avoid_multi_tap = NULL;//Fix the bug, when clicking the image again and again quickly, there will be problem.

static Eina_Bool __mf_ext_avoid_multi_click_timer_cb(void *data)
{
	mf_retvm_if(data == NULL, FALSE, "data is NULL");
	mf_debug("__mf_ext_avoid_multi_click_timer_cb()... ");
	mf_launch_service_timer_del();
	return ECORE_CALLBACK_CANCEL;
}

void mf_launch_service_timer_del()
{
	if (g_mf_avoid_multi_tap) {
		ecore_timer_del(g_mf_avoid_multi_tap);
		g_mf_avoid_multi_tap = NULL;
	}
}

static bool __mf_launch_get_recent_files_cb(MFRitem *Ritem, void *user_data)
{
	Eina_List **list = (Eina_List **)user_data;
	if (Ritem && Ritem->path) {
		*list = eina_list_append(*list, g_strdup(Ritem->path));
	}
	return true;
}

void mf_launch_add_recent_files(void *data, const char *path)
{
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(path == NULL, "path is NULL");
	struct appdata *ap = (struct appdata *)data;

	int count = 0;
	mf_media_delete_recent_files(ap->mf_MainWindow.mfd_handle, path);
	mf_media_get_recent_files_count(ap->mf_MainWindow.mfd_handle, &count);
	if (count >= RECENT_FILES_COUNT_MAX) {
		Eina_List *list = NULL;
		mf_media_foreach_recent_files_list(ap->mf_MainWindow.mfd_handle, __mf_launch_get_recent_files_cb, &list);
		if (list) {
			int index = 0;
			int list_len = eina_list_count(list);
			for (; (list_len-index) >= RECENT_FILES_COUNT_MAX; index++) {
				char *cross_path = eina_list_nth(list, index);
				mf_media_delete_recent_files(ap->mf_MainWindow.mfd_handle, cross_path);
			}
			list_len = eina_list_count(list);
			mf_util_free_eina_list_with_data(&list, MYFILE_TYPE_CHAR);
		}
	}
	int storage_type = mf_fm_svc_wrapper_get_location(path);
	mf_util_db_add_recent_files(ap->mf_MainWindow.mfd_handle, path, NULL, storage_type, NULL);
}

void mf_launch_load_ug_myfile(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;
	if (ap->mf_SharedGadget.ug) {
		mf_debug("Already exits some Gallery UG called by me, destroy it first!");
		//ug_destroy(ap->mf_SharedGadget.ug);
		ap->mf_SharedGadget.ug = NULL;
	}

	app_control_h app_control;
	int ret = 0;
	ret = app_control_create(&app_control);

	mf_retm_if(ret != APP_CONTROL_ERROR_NONE, "service create failed");

	//cbs.layout_cb = __mf_launch_ug_layout_cb;
	//cbs.result_cb = __mf_launch_ug_myfile_result_cb;
	//cbs.destroy_cb = __mf_launch_ug_myfile_ug_destory_cb;
	//cbs.priv = ap;

	//UG_INIT_EFL(ap->mf_MainWindow.pWindow, UG_OPT_INDICATOR_ENABLE);

	app_control_add_extra_data(app_control, "path", ap->mf_Bundle.path);
	app_control_add_extra_data(app_control, "select_type", ap->mf_Bundle.select_type);
	app_control_add_extra_data(app_control, "file_type", ap->mf_Bundle.file_type);
	app_control_add_extra_data(app_control, "marked_mode", ap->mf_Bundle.marked_mode);
	//ug = ug_create(NULL, MYFILE_UG_NAME, UG_MODE_FULLVIEW, service, &cbs);
/*
	ret = app_control_set_window(app_control, elm_win_xwindow_get(ap->mf_MainWindow.pWindow));
	mf_retm_if(ret != APP_CONTROL_ERROR_NONE, "service create failed");
*/
	ret = app_control_set_app_id(app_control, MYFILE_UG_NAME);
	if (ret != APP_CONTROL_ERROR_NONE) {
		app_control_destroy(app_control);
		mf_error("service create failed");
		return;
	}
	ret = app_control_send_launch_request(app_control, result_cb, ap);
	//mf_retm_if(ret != APP_CONTROL_ERROR_NONE, NULL, "service create failed");

	if (ret != APP_CONTROL_ERROR_NONE) {
		mf_debug("Fail to Create UG");
	}
	app_control_destroy(app_control);
	MF_TRACE_END;
}


int mf_launch_service(void *data, char *path)
{
	mf_retvm_if(data == NULL, LAUNCH_TYPE_FAIL, "data is NULL");
	mf_retvm_if(path == NULL, LAUNCH_TYPE_FAIL, "path is NULL");

	mf_debug("path is [%s]", path);
	//mf_launch_get_match(path);
	struct appdata *ap = (struct appdata *)data;
	if (ap->mf_Status.more == MORE_DEFAULT || ap->mf_Status.more == MORE_SEARCH) {
		fsFileType category = FILE_TYPE_NONE;
		int ret = 0;
		/* defence code */
		if (mf_file_get(path) == NULL) {
			return LAUNCH_TYPE_FAIL;
		}
		if (mf_file_attr_is_dir(path)) {
			return LAUNCH_TYPE_DIR;
		}
		if (g_mf_avoid_multi_tap) {
			return LAUNCH_TYPE_FORK;
		}
		Ecore_Timer *timer = NULL;
		timer = ecore_timer_add(1.0, __mf_ext_avoid_multi_click_timer_cb, data);
		g_mf_avoid_multi_tap = timer;

		app_control_h app_control;

		ret = app_control_create(&app_control);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_create()... [0x%x]", ret);
			goto END;
		}
		mf_file_attr_get_file_category(path, &category);
		if (category == FILE_TYPE_HTML || category == FILE_TYPE_TPK) {
			char *html_path = g_strconcat("file://", path, NULL);
			ret = app_control_set_uri(app_control, html_path);
			mf_error("uri is [%s]", html_path);
			SAFE_FREE_CHAR(html_path);
		} else if (category == FILE_TYPE_IMAGE) {
			mf_debug("category  is image");
			if (ap->mf_Status.view_type == mf_view_root_category) {
				app_control_add_extra_data(app_control, "View By", "All");
				app_control_add_extra_data(app_control, "Media type", "Image");
			} else {
				app_control_add_extra_data(app_control, "View By", "By Folder");
			}
			ret = app_control_set_uri(app_control, path);
			app_control_set_mime(app_control, "image/*");
			app_control_add_extra_data(app_control, "Path", path);
			app_control_set_launch_mode(app_control, APP_CONTROL_LAUNCH_MODE_GROUP);
		} else if (category == FILE_TYPE_SOUND
				|| category == FILE_TYPE_MUSIC
				|| category == FILE_TYPE_VOICE) {

			mf_debug("category  is sound");
			app_control_add_extra_data(app_control, "View By", "By Folder");
			int sort_type = 0;
			mf_util_get_pref_value(PREF_TYPE_SORT_TYPE, &sort_type);

			if (sort_type ==   MYFILE_SORT_BY_NAME_A2Z) {
				app_control_add_extra_data(app_control, "sort_type", "MYFILE_SORT_BY_NAME_A2Z");
			} else if(sort_type ==	 MYFILE_SORT_BY_SIZE_S2L) {
				app_control_add_extra_data(app_control, "sort_type", "MYFILE_SORT_BY_SIZE_S2L");
			} else if(sort_type ==	 MYFILE_SORT_BY_DATE_O2R) {
				app_control_add_extra_data(app_control, "sort_type", "MYFILE_SORT_BY_DATE_O2R");
			} else if(sort_type ==	 MYFILE_SORT_BY_TYPE_A2Z) {
				app_control_add_extra_data(app_control, "sort_type", "MYFILE_SORT_BY_TYPE_A2Z");
			} else if(sort_type ==	 MYFILE_SORT_BY_NAME_Z2A) {
				app_control_add_extra_data(app_control, "sort_type", "MYFILE_SORT_BY_NAME_Z2A");
			} else if(sort_type ==	 MYFILE_SORT_BY_SIZE_L2S) {
				app_control_add_extra_data(app_control, "sort_type", "MYFILE_SORT_BY_SIZE_L2S");
			} else if(sort_type ==	 MYFILE_SORT_BY_DATE_R2O) {
				app_control_add_extra_data(app_control, "sort_type", "MYFILE_SORT_BY_DATE_R2O");
			} else if(sort_type ==	 MYFILE_SORT_BY_TYPE_Z2A) {
				app_control_add_extra_data(app_control, "sort_type", "MYFILE_SORT_BY_TYPE_Z2A");
			}
			if (ap->mf_Status.view_type != mf_view_root_category) {
				app_control_add_extra_data(app_control, "launching_application", "myfile");
			}
			app_control_add_extra_data(app_control, "enableChangePlayer", "true");
			ret = app_control_set_uri(app_control, path);
			app_control_set_mime(app_control, "audio/*");
		} else {
			if (ap->mf_Status.view_type != mf_view_root_category) {
				app_control_add_extra_data(app_control, "launching_application", "myfile");
			}
			ret = app_control_set_uri(app_control, path);
		}
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_set_uri()... [0x%x]", ret);
			goto END;
		}
		ret = app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_set_operation()... [0x%x]", ret);
			goto END;
		}
		ret = app_control_enable_app_started_result_event(app_control);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mf_debug("app_control_enable_app_started_result_event()... [0x%x]", ret);
			goto END;
		}
		ret = app_control_send_launch_request(app_control, result_cb, ap);
		if (ret == APP_CONTROL_ERROR_APP_NOT_FOUND) {
			ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(data, POPMODE_TITLE_TEXT_BTN, MF_LABEL_UNABLE_TO_OPEN_FILE, MF_LABEL_NO_APP, MF_BUTTON_LABEL_OK, NULL, NULL, mf_callback_unsupported_app_cb, ap);
		} else if (ret != APP_CONTROL_ERROR_NONE) {
			ap->mf_MainWindow.pNormalPopup = mf_popup_create_popup(data, POPMODE_TITLE_TEXT_BTN, MF_LABEL_UNABLE_TO_OPEN_FILE, MF_LABEL_FAILED, MF_BUTTON_LABEL_OK, NULL, NULL, mf_callback_unsupported_app_cb, ap);
			mf_debug("app_control_send_launch_request()... [0x%x]", ret);
		} else {
			SAFE_FREE_CHAR(ap->mf_Status.launch_path);
			ap->mf_Status.launch_path = g_strdup(path);
			mf_debug("app_control_send_launch_request()... [0x%x]", ret);
			/*reflesh recent files list in root view */
			mf_info("ap->mf_Status.view_type is %d, ap->mf_Status.flag_tab is %d",ap->mf_Status.view_type,ap->mf_Status.flag_tab);
			if (ap->mf_Status.view_type == mf_view_root && ap->mf_Status.flag_tab == MF_TAB_RIGHT && ap->mf_Status.more == MORE_DEFAULT) {
				//__mf_root_view_recent_files_content_create(ap);
			}
		}
END:
		if (app_control) {
			app_control_destroy(app_control);
		}
		//mf_util_free_eina_list_with_data(&sound_list, MYFILE_TYPE_GSTRING);
		return LAUNCH_TYPE_FORK;
	}

	return LAUNCH_TYPE_UNSUPPORT;
}
