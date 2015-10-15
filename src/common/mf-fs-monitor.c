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
#include "mf-util.h"
#include "mf-callback.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-inotify-handle.h"
#include "mf-view.h"
#include "mf-genlist.h"
#include "mf-edit-view.h"
#include "mf-object.h"
static Ecore_Pipe *fs_monitor_pipe = NULL;
Ecore_Timer *fs_monitor_update_timer = NULL;
static char *fs_monitor_dir_path = NULL;
static char *fs_monitor_update_item_name = NULL;
typedef struct _mf_dir_event_t {
	int event;
	char *name;
} mf_dir_event_t;

static Eina_Bool monitor_refresh_start_flag = EINA_FALSE;

Eina_Bool __mf_fs_monitor_dir_pipe_add_cb(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, EINA_FALSE, "appdata is NULL");
	int pre_more = MORE_DEFAULT;
	Eina_List *file_list = NULL;
	Eina_List *folder_list = NULL;

	if (monitor_refresh_start_flag) {
		fs_monitor_update_timer = NULL;
		return EINA_FALSE;
	}
	monitor_refresh_start_flag = EINA_TRUE;
	if (ap->mf_Status.more == MORE_THUMBNAIL_RENAME) {
		//char *rename_item_name = mf_popup_rename_item_name_get();
		//if (rename_item_name && g_strcmp0(fs_monitor_update_item_name, rename_item_name) == 0) {
		mf_popup_rename_cancel();
		//}
	}
	if (ap->mf_MainWindow.pLongpressPopup) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
	}
	if (ap->mf_MainWindow.pDeleteConfirmPopup) {
		//mfItemData_s *item_data = elm_object_item_data_get(ap->mf_FileOperation.idle_delete_item);
		//char *delete_item_name = item_data->m_ItemName->str;
		//if (delete_item_name && g_strcmp0(fs_monitor_update_item_name, delete_item_name) == 0) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pDeleteConfirmPopup);
		ap->mf_FileOperation.idle_delete_item = NULL;
		//}
	}

	int view_style = mf_view_style_get(ap);
	if (view_style != MF_VIEW_STYLE_THUMBNAIL && ap->mf_MainWindow.pNaviGenlist) {
		SAFE_FREE_CHAR(ap->mf_Status.EnterFrom);
		ap->mf_Status.EnterFrom = mf_genlist_first_item_name_get(ap->mf_MainWindow.pNaviGenlist);
	}
	if (ap->mf_Status.more == MORE_EDIT 
		|| ap->mf_Status.more == MORE_SHARE_EDIT
		|| ap->mf_Status.more == MORE_EDIT_COPY
		|| ap->mf_Status.more == MORE_EDIT_MOVE
		|| ap->mf_Status.more == MORE_EDIT_DELETE
	    || ap->mf_Status.more == MORE_EDIT_DETAIL
		) {
		pre_more = ap->mf_Status.more;
		ap->mf_Status.more = MORE_DEFAULT;
		folder_list = mf_edit_get_selected_folder_list();
		file_list = mf_edit_get_selected_file_list();
	}
	mf_object_box_clear(ap->mf_MainWindow.pNaviBox);
	mf_view_update(ap);
	if (pre_more == MORE_EDIT 
		|| pre_more == MORE_SHARE_EDIT
		|| pre_more == MORE_EDIT_COPY
		|| pre_more == MORE_EDIT_MOVE
		|| pre_more == MORE_EDIT_DELETE
	    || pre_more == MORE_EDIT_DETAIL
		) {
		ap->mf_Status.more = pre_more;
		mf_edit_view_refresh(ap, &file_list, &folder_list);
	}

	fs_monitor_update_timer = NULL;
	monitor_refresh_start_flag = EINA_FALSE;
	return EINA_FALSE;

}

static void __mf_callback_dir_pipe_cb(void *data, void *buffer, unsigned int nbyte)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "appdata is NULL");
	if (!(ap->mf_Status.more == MORE_DEFAULT
	    || ap->mf_Status.more == MORE_INTERNAL_COPY
	    || ap->mf_Status.more == MORE_INTERNAL_MOVE
	    || ap->mf_Status.more == MORE_EDIT
	    || ap->mf_Status.more == MORE_EDIT_COPY
	    || ap->mf_Status.more == MORE_EDIT_MOVE
	    || ap->mf_Status.more == MORE_EDIT_ADD_SHORTCUT
	    || ap->mf_Status.more == MORE_EDIT_DELETE
	    || ap->mf_Status.more == MORE_THUMBNAIL_RENAME
	      || ap->mf_Status.more == MORE_EDIT_DETAIL)) {
		return;
	}
	if (ap->mf_Status.view_type == mf_view_root || ap->mf_Status.view_type == mf_view_root_category || ap->mf_Status.view_type == mf_view_storage) {
		return;
	}
	if (g_strcmp0(fs_monitor_dir_path, ap->mf_Status.path->str)) {
		return;
	}
	if (mf_callback_monitor_internal_update_flag_get()) {
		return;
	}
	if (buffer) {
		mf_dir_event_t *msg = (mf_dir_event_t *)buffer;
		SECURE_DEBUG("event : %d, name : %s", msg->event, msg->name);
		if (msg->name) {
			fs_monitor_update_item_name = g_strconcat(ap->mf_Status.path->str, "/", msg->name, NULL);
		}

		if (fs_monitor_update_timer == NULL) {
			fs_monitor_update_timer = ecore_timer_add(1, (Ecore_Task_Cb)__mf_fs_monitor_dir_pipe_add_cb, ap);
			SAFE_FREE_CHAR(fs_monitor_update_item_name);
		}
		if (msg->name)
			free(msg->name);
		mf_inotify_handle_request_handled_send();
	}
}

static void __mf_fs_monitor_dir_update_cb(mf_inotify_event event, char *name, void *data)
{
	SECURE_DEBUG("event : %d, name : %s", event, name);

	mf_dir_event_t buffer;

	buffer.event = event;
	buffer.name = strdup(name);

	ecore_pipe_write(fs_monitor_pipe, &buffer, sizeof(buffer));

	return;
}

int mf_fs_monitor_add_dir_watch(const char *path, void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retvm_if(data == NULL, 0, "data is NULL");

	SAFE_FREE_CHAR(fs_monitor_dir_path);
	SAFE_FREE_CHAR(fs_monitor_update_item_name);
	
	SAFE_DEL_ECORE_TIMER(fs_monitor_update_timer);
	monitor_refresh_start_flag = EINA_FALSE;
	fs_monitor_dir_path = g_strdup(path);
	MF_TRACE_END;
	t_end;
	return mf_inotify_handle_add_watch(path, __mf_fs_monitor_dir_update_cb, data);
}

int mf_fs_monitor_remove_dir_watch(void)
{
	SAFE_FREE_CHAR(fs_monitor_dir_path);
	SAFE_FREE_CHAR(fs_monitor_update_item_name);
	SAFE_DEL_ECORE_TIMER(fs_monitor_update_timer);
	monitor_refresh_start_flag = EINA_FALSE;
	return mf_inotify_handle_rm_watch();
}

int mf_fs_monitor_create(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, -1, "appdata is NULL");

	if (fs_monitor_pipe) {
		ecore_pipe_del(fs_monitor_pipe);
	}
	
	SAFE_DEL_ECORE_TIMER(fs_monitor_update_timer);
	monitor_refresh_start_flag = EINA_FALSE;

	fs_monitor_pipe = ecore_pipe_add(__mf_callback_dir_pipe_cb, (const void *)ap);
	MF_TRACE_END;
	t_end;

	return mf_inotify_handle_init_inotify();
}

void mf_fs_monitor_destory()
{
	if (fs_monitor_pipe) {
		ecore_pipe_del(fs_monitor_pipe);
		fs_monitor_pipe = NULL;
	}
	SAFE_FREE_CHAR(fs_monitor_dir_path);
	SAFE_FREE_CHAR(fs_monitor_update_item_name);
	SAFE_DEL_ECORE_TIMER(fs_monitor_update_timer);
	monitor_refresh_start_flag = EINA_FALSE;

	mf_inotify_handle_finalize_inotify();

	return;
}


