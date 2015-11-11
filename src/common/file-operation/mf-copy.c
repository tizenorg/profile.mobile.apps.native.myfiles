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

#include "mf-copy.h"
#include "mf-copy-internal.h"
#include "mf-cancel.h"
#include "mf-fo-common.h"
#include "mf-fo-internal.h"
#include "mf-fo-debug.h"
#include "mf-callback.h"
#include "mf-media-content.h"

extern int flagMsg;
extern pthread_mutex_t gLockMsg;
extern pthread_cond_t gCondMsg;

struct _mf_copy_handle {
	GList *src_items;
	char *dst_dir;
	mf_cancel *cancel;
	void *u_data;
	Ecore_Pipe *pipe;
	gboolean sync;

	GMutex lock;
	GCond cond;

	mf_fo_msg msg;
	mf_fo_request *req;
};

static double __mf_copy_get_time(void)
{
	struct timeval timev;

	gettimeofday(&timev, NULL);
	return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

static gboolean __mf_copy_msg_publish_callback(gpointer data)
{
	FO_TRACE_BEGIN;
	struct _mf_copy_handle *cp_handle = NULL;
	cp_handle = (struct _mf_copy_handle *)data;
	mf_fo_msg msg;
	memset(&msg, 0, sizeof(mf_fo_msg));

	MYFILE_MAGIC_SET(&msg, MYFILE_MAGIC_PIPE_DATA);
	if (!cp_handle) {
		goto EXIT;
	}

	g_mutex_lock(&cp_handle->lock);
	msg.msg_type = cp_handle->msg.msg_type;
	msg.error_code = cp_handle->msg.error_code;
	msg.current = cp_handle->msg.current;
	msg.current_index = cp_handle->msg.current_index;
	msg.total_index = cp_handle->msg.total_index;
	msg.current_size = cp_handle->msg.current_size;
	msg.total_size = cp_handle->msg.total_size;
	msg.current_real = cp_handle->msg.current_real;
	msg.request = NULL;
	msg.pipe = cp_handle->pipe;
	g_mutex_unlock(&cp_handle->lock);

	ecore_pipe_write(cp_handle->pipe, &msg, sizeof(msg));

EXIT:
	FO_TRACE_END;
	return FALSE;
}

static void __mf_copy_msg_cb(mf_msg_type msg_type, const char *real, unsigned long long size, int error_code, void *data)
{
	struct _mf_copy_handle *cp_handle = NULL;
	cp_handle = (struct _mf_copy_handle *)data;

	pthread_mutex_lock(&gLockMsg);
	while (flagMsg == 0) {
		mf_fo_loge("!!!!!!!!!!!! wait");
		pthread_cond_wait(&gCondMsg, &gLockMsg);
	}
	flagMsg = 0;
	pthread_mutex_unlock(&gLockMsg);

	if (cp_handle) {
		g_mutex_lock(&cp_handle->lock);
		cp_handle->msg.msg_type = msg_type;
		if (msg_type == MF_MSG_ERROR) {
			cp_handle->msg.error_code = error_code;
			if (real) {
				if (cp_handle->msg.current_real) {
					free(cp_handle->msg.current_real);
				}
				cp_handle->msg.current_real = strdup(real);
			}
		} else {
			cp_handle->msg.error_code = 0;
			if (msg_type == MF_MSG_DOING) {
				if (real) {
					if (cp_handle->msg.current_real) {
						free(cp_handle->msg.current_real);
					}
					cp_handle->msg.current_real = strdup(real);
				}
				cp_handle->msg.current_size += size;
				cp_handle->msg.error_code = 0;
			} else if (msg_type == MF_MSG_SKIP) {
				cp_handle->msg.total_size -= size;
				cp_handle->msg.error_code = 0;
			}
		}
		g_mutex_unlock(&cp_handle->lock);

		__mf_copy_msg_publish_callback(cp_handle);
	}
	FO_TRACE_END;
	return;
}

static gboolean __mf_copy_req_msg_callback(gpointer data)
{
	FO_TRACE_BEGIN;

	struct _mf_copy_handle *cp_handle = NULL;
	cp_handle = (struct _mf_copy_handle *)data;
	mf_fo_msg msg;
	memset(&msg, 0, sizeof(mf_fo_msg));

	MYFILE_MAGIC_SET(&msg, MYFILE_MAGIC_PIPE_DATA);
	if (!cp_handle) {
		goto EXIT;
	}

	g_mutex_lock(&cp_handle->lock);
	msg.msg_type = MF_MSG_REQUEST;
	msg.error_code = cp_handle->msg.error_code;
	msg.current = cp_handle->msg.current;
	msg.current_index = cp_handle->msg.current_index;
	msg.total_index = cp_handle->msg.total_index;
	msg.current_size = cp_handle->msg.current_size;
	msg.total_size = cp_handle->msg.total_size;
	msg.current_real = cp_handle->msg.current_real;
	msg.request = cp_handle->req;
	g_mutex_unlock(&cp_handle->lock);
	ecore_pipe_write(cp_handle->pipe, &msg, sizeof(msg));

EXIT:

	FO_TRACE_END;
	return FALSE;
}

static void __mf_copy_req_cb(mf_fo_request *req, void *data)
{
	FO_TRACE_BEGIN;
	struct _mf_copy_handle *cp_handle = NULL;
	mf_retm_if(data == NULL, "data is NULL");
	cp_handle = (struct _mf_copy_handle *)data;
	struct appdata *ap = (struct appdata *)cp_handle->u_data;
	if (cp_handle) {

		cp_handle->req = req;
		mf_request_set_cond(req, &cp_handle->cond);
		__mf_copy_req_msg_callback(cp_handle);
		g_mutex_lock(&cp_handle->lock);
		if (ap->mf_Status.check == 1) {
			gint64 end_time;
			end_time = g_get_monotonic_time() + 5 * G_TIME_SPAN_SECOND;
			while (mf_request_flag_get(req)) {
				if (!g_cond_wait_until(&cp_handle->cond, &cp_handle->lock, end_time)) {
					mf_fo_loge("g_cond_wait_until is timeout, time is %lld", end_time);
					mf_request_flag_set(req, 1);
					g_mutex_unlock(&cp_handle->lock);
					FO_TRACE_END;
					return;
				}
			}
		} else {
			g_cond_wait(&cp_handle->cond, &cp_handle->lock);
		}
		mf_request_flag_set(req, 1);
		g_mutex_unlock(&cp_handle->lock);
	}
	FO_TRACE_END;
	return;
}

static void __mf_copy_free_handle(struct _mf_copy_handle *handle)
{
	if (handle) {
		g_mutex_clear(&handle->lock);
		g_cond_clear(&handle->cond);

		if (handle->dst_dir) {
			free(handle->dst_dir);
		}
		if (handle->src_items) {
			g_list_foreach(handle->src_items, (GFunc) free, NULL);
			g_list_free(handle->src_items);
		}
		if (handle->msg.current_real) {
			g_free(handle->msg.current_real);
			handle->msg.current_real = NULL;
		}
		free(handle);
	}

	return;
}

static void __mf_media_scan_folder_completed_cb(media_content_error_e err, void *data)
{
	struct _mf_copy_handle *cp_handle = NULL;
	cp_handle = (struct _mf_copy_handle *)data;

	if (cp_handle) {
		g_mutex_lock(&cp_handle->lock);
		mf_debug("!!!!!!!!!!!! sending signal");
		g_cond_signal(&cp_handle->cond);
		g_mutex_unlock(&cp_handle->lock);
	}
	return;
}

static void *__mf_copy_thread_func(void *data)
{
	FO_TRACE_BEGIN;
	struct _mf_copy_handle *cp_handle = NULL;
	cp_handle = (struct _mf_copy_handle *)data;

	gboolean cancelled = FALSE;
	double s_start = 0.0;
	double s_stop = 0.0;
	double c_start = 0.0;
	double c_stop = 0.0;
	if (cp_handle) {
		GList *tmp_src_list = NULL;
		unsigned long long t_size = 0;
		unsigned long long r_size = 0;
		int errcode = 0;

		s_start = __mf_copy_get_time();
		tmp_src_list = cp_handle->src_items;

		errcode = _mf_fo_get_remain_space(cp_handle->dst_dir, &r_size);
		if (errcode < 0) {

			__mf_copy_msg_cb(MF_MSG_ERROR, cp_handle->dst_dir, 0, (MF_FO_ERR_DST_CLASS | _mf_fo_errno_to_mferr(-errcode)), cp_handle);

			goto ERROR_END_THREAD;
		}

		if (r_size == 0) {
			int err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_SPACE);
			__mf_copy_msg_cb(MF_MSG_ERROR, cp_handle->dst_dir, 0, err, cp_handle);

			goto ERROR_END_THREAD;
		}

		while (tmp_src_list) {
			if (tmp_src_list->data) {
				const char *s_path = NULL;
				unsigned long long size = 0;

				s_path = tmp_src_list->data;
				if (access(s_path, R_OK) == 0) {
					errcode = _mf_fo_get_total_item_size(s_path, &size);
					if (errcode < 0) {
						mf_fo_loge("Fail to get size of %s", s_path);

						__mf_copy_msg_cb(MF_MSG_ERROR, s_path, 0,
						                 (MF_FO_ERR_SRC_CLASS | _mf_fo_errno_to_mferr(-errcode)), cp_handle);

						goto ERROR_END_THREAD;
					} else {
						t_size += size;
					}
				} else {
					mf_fo_loge("Unable to access [%s]. error - %s", s_path, strerror(errno));
					__mf_copy_msg_cb(MF_MSG_ERROR, s_path, 0, (MF_FO_ERR_SRC_CLASS | _mf_fo_errno_to_mferr(errno)), cp_handle);
					goto ERROR_END_THREAD;
				}
			}
			tmp_src_list = g_list_next(tmp_src_list);
		}
		s_stop = __mf_copy_get_time();
		g_mutex_lock(&cp_handle->lock);
		cp_handle->msg.total_size = t_size;
		g_mutex_unlock(&cp_handle->lock);
		/* copy items */
		c_start = __mf_copy_get_time();
		tmp_src_list = cp_handle->src_items;
		while (tmp_src_list) {
			if (tmp_src_list->data) {
				const char *s_path = NULL;
				s_path = tmp_src_list->data;
				int ret = 0;
				g_mutex_lock(&cp_handle->lock);
				cp_handle->msg.current_index++;
				cp_handle->msg.current = s_path;
				g_mutex_unlock(&cp_handle->lock);
				ret = _mf_copy_copy_internal(s_path, cp_handle->dst_dir, cp_handle->cancel,
				                             __mf_copy_msg_cb, __mf_copy_req_cb, cp_handle);

				if (ret > 0) {
					if (cp_handle->cancel) {
						mf_cancel_set_cancelled(cp_handle->cancel);
					}
					cancelled = TRUE;
					break;
				}
				if (ret < 0) {
					mf_fo_loge("Fail to copy [%s] to [%s]", s_path, cp_handle->dst_dir);
					break;
				}
			}
			tmp_src_list = g_list_next(tmp_src_list);
		}
		c_stop = __mf_copy_get_time();
		mf_fo_logi("## Total src size - %lld byte, size time : %lf sec, copy time : %lf sec",
		           cp_handle->msg.total_size, s_stop - s_start, c_stop - c_start);
		if (cancelled) {
			__mf_copy_msg_cb(MF_MSG_CANCELLED, NULL, 0, 0, cp_handle);
		}

ERROR_END_THREAD:
		if (cp_handle->sync) {
			double start = 0.0;
			double stop = 0.0;

			__mf_copy_msg_cb(MF_MSG_SYNC, NULL, 0, 0, cp_handle);

			start = __mf_copy_get_time();
			sync();
			stop = __mf_copy_get_time();
			mf_fo_logi("sync time : %lf sec", stop - start);
		}
#if 0
		mf_media_content_scan_folder(cp_handle->dst_dir);
#else
		g_mutex_lock(&cp_handle->lock);

		int media_content_ret = media_content_scan_folder(cp_handle->dst_dir, true, __mf_media_scan_folder_completed_cb, cp_handle);
		if (media_content_ret != MEDIA_CONTENT_ERROR_NONE) {
			mf_error("media_content_scan_folder() is failed, media_content_ret = [%d]", media_content_ret);
			g_mutex_unlock(&cp_handle->lock);
		} else {
			mf_debug("!!!!!!!!!!!! wait");
			g_cond_wait(&cp_handle->cond, &cp_handle->lock);
			mf_debug("!!!!!!!!!!!! Got signal");
			g_mutex_unlock(&cp_handle->lock);
		}
#endif
		__mf_copy_msg_cb(MF_MSG_END, NULL, 0, 0, cp_handle);
		__mf_copy_free_handle(cp_handle);
		cp_handle = NULL;
	} else {
		mf_fo_loga("handle is NULL");
		abort();
	}

	mf_fo_logd("The end of __mf_copy_thread_func");
	return NULL;
}

int mf_copy_copy_items(GList *item_list, const char *dst_dir, mf_cancel *cancel, gboolean sync, void *u_data)
{
	FO_TRACE_BEGIN;
	struct _mf_copy_handle *cp_handle = NULL;
	GList *tmp_list = NULL;
	int err = 0;

	if (!item_list) {
		mf_fo_loge("item_list is NULL");
		return -(MF_FO_ERR_ARGUMENT);
	}
	if (!dst_dir) {
		mf_fo_loge("dst_dir is NULL");
		return -(MF_FO_ERR_ARGUMENT);
	}

	if (!_mf_fo_check_exist(dst_dir)) {
		mf_fo_loge("dst_dir[%s] is not existed", dst_dir);
		return -(MF_FO_ERR_ARGUMENT);
	}
#if 0//Deprecated API
	if (!g_thread_supported()) {
		g_thread_init(NULL);
	}
#endif
	cp_handle = malloc(sizeof(struct _mf_copy_handle));
	if (!cp_handle) {
		mf_fo_loge("Fail to allocate handle");
		return -(MF_FO_ERR_MEM);
	}
	memset(cp_handle, 0x00, sizeof(struct _mf_copy_handle));

#if 0
	cp_handle->lock = g_mutex_new();
	if (!cp_handle->lock) {
		mf_fo_loge("Fail to allocate mutex");
		err = MF_FO_ERR_MEM;
		goto ERROR_FREE_MEM;
	}
	cp_handle->cond = g_cond_new();
	if (!cp_handle->cond) {
		mf_fo_loge("Fail to allocate cond");
		err = MF_FO_ERR_MEM;
		goto ERROR_FREE_MEM;
	}
#endif
	g_mutex_init(&cp_handle->lock);
	g_cond_init(&cp_handle->cond);

	cp_handle->dst_dir = strdup(dst_dir);
	if (!cp_handle->dst_dir) {
		mf_fo_loge("Fail to allocate memory");
		err = MF_FO_ERR_MEM;
		goto ERROR_FREE_MEM;
	}

	cp_handle->cancel = cancel;
	cp_handle->u_data = u_data;
	cp_handle->sync = sync;

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
					cp_handle->src_items = g_list_append(cp_handle->src_items, src_item);
				} else {
					mf_fo_loge("src_item[%s] is not existed", src_item);
					err = MF_FO_ERR_ARGUMENT;
					free(src_item);
					src_item = NULL;
					/* goto ERROR_FREE_MEM; */
				}
			} else {
				mf_fo_loge("Fail to allocate memory");
				err = MF_FO_ERR_MEM;
				goto ERROR_FREE_MEM;
			}
		}
		tmp_list = g_list_next(tmp_list);
	}

	if (!cp_handle->src_items) {
		mf_fo_loge("Fail to create src list");
		err = MF_FO_ERR_ARGUMENT;
		goto ERROR_FREE_MEM;
	}

	cp_handle->pipe = ecore_pipe_add(mf_callback_thread_pipe_cb, u_data);
	if (!g_thread_new(NULL, (GThreadFunc) __mf_copy_thread_func, (void*)cp_handle)) {
		mf_fo_loge("Fail to create copy thread");
		err = MF_FO_ERR_MEM;
		goto ERROR_FREE_MEM;
	}

	return 0;

ERROR_FREE_MEM:
	__mf_copy_free_handle(cp_handle);
	return -(err);
}
