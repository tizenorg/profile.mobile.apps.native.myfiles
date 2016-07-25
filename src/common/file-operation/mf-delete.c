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

#include <stdio.h>
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/syscall.h>

#include "mf-delete.h"
#include "mf-delete-internal.h"
#include "mf-cancel.h"
#include "mf-fo-common.h"
#include "mf-fo-internal.h"
#include "mf-fo-debug.h"
#include "mf-callback.h"
#include "mf-media-content.h"

extern int flagMsg;
extern pthread_mutex_t gLockMsg;
extern pthread_cond_t gCondMsg;

struct _mf_del_handle {
	GList *src_items;
	mf_cancel *cancel;
	void *u_data;
	gboolean sync;

	GMutex lock;
	GCond cond;
	guint msg_idle_cb;

	mf_fo_msg msg;
	Ecore_Pipe *pipe;
};

static double get_time(void)
{
	struct timeval timev;

	gettimeofday(&timev, NULL);
	return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

static void __mf_delete_free_handle(struct _mf_del_handle *handle)
{
	if (handle) {
		/*free cp_handle*/
		g_mutex_clear(&handle->lock);

		g_cond_clear(&handle->cond);

		if (handle->src_items) {
			g_list_foreach(handle->src_items, (GFunc)free, NULL);
			g_list_free(handle->src_items);
		}
		if (handle->msg.current_real) {
			g_free(handle->msg.current_real);
			handle->msg.current_real = NULL;
		}
		free(handle);
		handle = NULL;
	}

	return;
}

static gboolean _del_msg_publish(gpointer data)
{
	struct _mf_del_handle *handle = NULL;
	handle = (struct _mf_del_handle *)data;
	mf_fo_msg msg;

	if (!handle) {
		goto EXIT;
	}
	MYFILE_MAGIC_SET(&msg, MYFILE_MAGIC_PIPE_DATA);

	g_mutex_lock(&handle->lock);
	msg.msg_type = handle->msg.msg_type;
	msg.error_code = handle->msg.error_code;
	msg.current = handle->msg.current;
	msg.current_index = handle->msg.current_index;
	msg.total_index = handle->msg.total_index;
	msg.current_size = handle->msg.current_size;
	msg.total_size = handle->msg.total_size;
	msg.current_real = handle->msg.current_real;
	msg.pipe = handle->pipe;
	g_mutex_unlock(&handle->lock);

	/*pulish message*/
	ecore_pipe_write(handle->pipe, &msg, sizeof(msg));

EXIT:
	return FALSE;
}

static void _del_msg_cb(mf_msg_type msg_type, const char *real, unsigned long long size, int error_code, void *data)
{
	struct _mf_del_handle *handle = NULL;
	handle = (struct _mf_del_handle *)data;

	pthread_mutex_lock(&gLockMsg);
	while (flagMsg == 0) {
		mf_fo_loge("!!!!!!!!!!!! wait");
		pthread_cond_wait(&gCondMsg, &gLockMsg);
	}
	flagMsg = 0;
	pthread_mutex_unlock(&gLockMsg);

	if (handle) {
		g_mutex_lock(&handle->lock);
		handle->msg.msg_type = msg_type;
		if (msg_type == MF_MSG_ERROR) {
			handle->msg.error_code = error_code;
			if (real) {
				if (handle->msg.current_real) {
					free(handle->msg.current_real);
				}
				handle->msg.current_real = strdup(real);
			}
		} else {
			handle->msg.error_code = 0;
			if (msg_type == MF_MSG_DOING) {
				if (real) {
					if (handle->msg.current_real) {
						free(handle->msg.current_real);
					}
					handle->msg.current_real = strdup(real);
				}
				handle->msg.current_size += size;
				handle->msg.error_code = 0;
			} else if (msg_type == MF_MSG_SKIP) {
				handle->msg.total_size -= size;
				handle->msg.error_code = 0;
			}
		}
		g_mutex_unlock(&handle->lock);

		_del_msg_publish(handle);
	}
	return;
}


static void *delete_thread(void *data)
{
	struct _mf_del_handle *handle = NULL;
	handle = (struct _mf_del_handle *)data;

	_mf_fo_msg_cb msg_cb = NULL;
	gboolean cancelled = FALSE;
	double s_start = 0.0;
	double s_stop = 0.0;
	double c_start = 0.0;
	double c_stop = 0.0;
	char err_buf[MF_ERR_BUF] = { 0, };

	if (handle) {
		GList *tmp_src_list = NULL;
		unsigned long long t_size = 0;

		msg_cb = _del_msg_cb;

		s_start = get_time();
		tmp_src_list = handle->src_items;
		while (tmp_src_list) {
			if (tmp_src_list->data) {
				const char *s_path = NULL;
				unsigned long long size = 0;

				s_path = tmp_src_list->data;
				if (access(s_path, R_OK) == 0) {
					int err = _mf_fo_get_total_item_size(s_path, &size);
					if (err < 0) {
						mf_fo_loge("Fail to get size of %s", s_path);
						/*handle->src_items = g_list_remove(handle->src_items, s_path);*/

						_del_msg_cb(MF_MSG_ERROR, s_path, 0, (MF_FO_ERR_SRC_CLASS | _mf_fo_errno_to_mferr(-err)), handle);

						goto ERROR_END_THREAD;
					} else {
						t_size += size;
					}
				} else {
					MF_FILE_ERROR_LOG(err_buf, "Unable to access ", s_path);
					/*handle->src_items = g_list_remove(handle->src_items, s_path);*/

					_del_msg_cb(MF_MSG_ERROR, s_path, 0, (MF_FO_ERR_SRC_CLASS | _mf_fo_errno_to_mferr(errno)), handle);

					goto ERROR_END_THREAD;
				}
			}
			tmp_src_list = g_list_next(tmp_src_list);
		}
		s_stop = get_time();
		g_mutex_lock(&handle->lock);
		handle->msg.total_size = t_size;
		g_mutex_unlock(&handle->lock);

		/*delete items*/
		c_start = get_time();
		tmp_src_list = handle->src_items;
		while (tmp_src_list) {
			if (tmp_src_list->data) {
				const char *s_path = NULL;
				s_path = tmp_src_list->data;
				int ret = 0;
				g_mutex_lock(&handle->lock);
				handle->msg.current_index++;
				handle->msg.current = s_path;
				g_mutex_unlock(&handle->lock);
				int dir_flag = 0;

				struct stat info;
				if (stat(s_path, &info)) {
					mf_fo_loge("Fail to delete [%s]", s_path);
				} else {
					if (S_ISDIR(info.st_mode)) {
						dir_flag = 1;
					}
				}
				ret = _mf_delete_del_internal(s_path, handle->cancel, msg_cb, handle);
				if (ret == 0) {
					if (dir_flag) {
						mf_media_content_scan_folder(s_path);
					} else {
						mf_media_content_scan_file(s_path);
					}
				} else if (ret > 0) {
					if (handle->cancel) {
						mf_cancel_set_cancelled(handle->cancel);
					}
					cancelled = TRUE;
					break;
				} else {
					mf_fo_loge("Fail to delete [%s]", s_path);
				}
			}
			tmp_src_list = g_list_next(tmp_src_list);
		}
		c_stop = get_time();
		mf_fo_logi("## Total src size - %lld byte, size time : %lf sec, delete time : %lf sec",
		           handle->msg.total_size, s_stop - s_start, c_stop - c_start);

		if (cancelled) {
			/*cancel message*/
			_del_msg_cb(MF_MSG_CANCELLED, NULL, 0, 0, handle);
		}

ERROR_END_THREAD:
		if (handle->sync) {
			double start = 0.0;
			double stop = 0.0;
			_del_msg_cb(MF_MSG_SYNC, NULL, 0, 0, handle);
			start = get_time();
			sync();
			stop = get_time();
			mf_fo_logi("sync time : %lf sec", stop - start);
		}

		_del_msg_cb(MF_MSG_END, NULL, 0, 0, handle);

		__mf_delete_free_handle(handle);
		handle = NULL;
	} else {
		mf_fo_loga("handle is NULL");
		abort();
	}

	mf_fo_logd("The end of del_thread");
	return NULL;
}

int mf_delete_items(GList *item_list, mf_cancel *cancel, gboolean sync, void *u_data)
{
	struct _mf_del_handle *handle = NULL;
	GList *tmp_list = NULL;
	int err = 0;

	if (!item_list) {
		mf_fo_loge("item_list is NULL");
		return -(MF_FO_ERR_ARGUMENT);
	}

#if 0//Deprecated API
	if (!g_thread_supported()) {
		g_thread_init(NULL);
	}
#endif

	handle = malloc(sizeof(struct _mf_del_handle));
	if (!handle) {
		mf_fo_loge("Fail to allocate handle");
		return -(MF_FO_ERR_MEM);
	}
	memset(handle, 0x00, sizeof(struct _mf_del_handle));
#if 0
	handle->lock = g_mutex_new();
	if (!handle->lock) {
		mf_fo_loge("Fail to allocate mutex");
		err = MF_FO_ERR_MEM;
		goto ERROR_FREE_MEM;
	}
	handle->cond = g_cond_new();
	if (!handle->cond) {
		mf_fo_loge("Fail to allocate cond");
		err = MF_FO_ERR_MEM;
		goto ERROR_FREE_MEM;
	}
#endif
	g_mutex_init(&handle->lock);
	g_cond_init(&handle->cond);
	handle->cancel = cancel;
	handle->u_data = u_data;
	handle->sync = sync;
	pthread_mutex_lock(&gLockMsg);
	flagMsg = 1;
	pthread_mutex_unlock(&gLockMsg);

	tmp_list = item_list;
	while (tmp_list) {
		if (tmp_list->data) {
			char *src_item = NULL;
			src_item = strdup((char *)tmp_list->data);
			if (src_item) {
				if (_mf_fo_check_exist(src_item)) {
					handle->src_items = g_list_append(handle->src_items, src_item);
				} else {
					mf_fo_loge("src_item[%s] is not existed", src_item);
					err = MF_FO_ERR_ARGUMENT;
					free(src_item);
					src_item = NULL;
				}
			} else {
				mf_fo_loge("Fail to allocate memory");
				err = MF_FO_ERR_MEM;
				goto ERROR_FREE_MEM;
			}
		}
		tmp_list = g_list_next(tmp_list);
	}

	if (!handle->src_items) {
		mf_fo_loge("Fail to create src list");
		err = MF_FO_ERR_ARGUMENT;
		goto ERROR_FREE_MEM;
	}

	handle->pipe = ecore_pipe_add(mf_callback_thread_pipe_cb, u_data);

	if (!g_thread_new(NULL, (GThreadFunc) delete_thread, (void*)handle)) {
		mf_fo_loge("Fail to create delete thread");
		err = MF_FO_ERR_MEM;
		goto ERROR_FREE_MEM;
	}

	return 0;

ERROR_FREE_MEM:
	__mf_delete_free_handle(handle);
	return -(err);
}
