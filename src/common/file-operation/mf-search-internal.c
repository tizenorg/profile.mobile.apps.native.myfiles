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

#include <glib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <Ecore.h>

#include "mf-main.h"
#include "mf-search.h"
#include "mf-search-internal.h"
#include "mf-fs-util.h"
#include "mf-util.h"

#define APPEND_SIZE 2		/* for null and slash */

#define NORMALIZE_OPTION G_NORMALIZE_NFD
#ifdef CHECK_RESTRICTED_PATH
/* TODO
 *  This code should be revised.
 *  How to get restricted path information?
 *  I think this module should not depend on other lib(except glib and stdlib).
*/
#define ROOT_UMS "/opt/usr/media"
#endif /* CHECK_RESTRICTED_PATH */

int flagSearchMsg = 1;
pthread_mutex_t gLockSearchMsg;
pthread_cond_t gCondSearchMsg;
#ifdef MF_SEARCH_UPDATE_COUNT
static int item_count = 0;
#endif
static void __mf_search_result_publish_msg(mf_search_pipe_msg_type type, void *result, void *user_data);

inline static void __mf_search_cmd_lock(ms_handle_t *handle)
{
	if (handle) {
		g_mutex_lock(&handle->cmd_lock);
	}
	return;
}

inline static void __mf_search_cmd_unlock(ms_handle_t *handle)
{
	if (handle) {
		g_mutex_unlock(&handle->cmd_lock);
	}
	return;
}

inline static void __mf_search_thread_lock(ms_handle_t *handle)
{
	if (handle) {
		g_mutex_lock(&handle->thread_mutex);
	}
	return;
}

inline static void __mf_search_thread_unlock(ms_handle_t *handle)
{
	if (handle) {
		g_mutex_unlock(&handle->thread_mutex);
	}
	return;
}

inline static void __mf_search_args_free(ms_args_t *args)
{
	if (args) {
		if (args->root_path) {
			g_list_foreach(args->root_path, (GFunc) g_free, NULL);
			g_list_free(args->root_path);
			args->root_path = NULL;
		}

		if (args->needle) {
			g_free(args->needle);
		}

		g_free(args);
	}
	return;
}

inline static void __mf_search_result_free(mf_search_result_t *result)
{
	if (result) {
		if (result->current_dir) {
			g_free(result->current_dir);
			result->current_dir = NULL;
		}
		if (result->dir_list) {
			g_list_foreach(result->dir_list, (GFunc) g_free, NULL);
			g_list_free(result->dir_list);
			result->dir_list = NULL;
		}
		if (result->file_list) {
			g_list_foreach(result->file_list, (GFunc) g_free, NULL);
			g_list_free(result->file_list);
			result->file_list = NULL;
		}
		g_free(result);
	}
	return;
}

#ifdef CHECK_RESTRICTED_PATH
gboolean __mf_search_check_licet_path(const char *path)
{
	char *mmc_root_path = NULL;
	if(mf_util_get_external_storage_id() != -1) {
		storage_get_root_directory(mf_util_get_external_storage_id(), &mmc_root_path);
	}
	gboolean check = (gboolean)(strstr(path, ROOT_UMS) || strstr(path, mmc_root_path));
	return check;
}
#endif /*CHECK_RESTRICTED_PATH*/


/*This function is for testing and should be revised for performance before applying*/
static inline gboolean __has_nonspacing_mark(const char *nstr)
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

static gboolean __mf_search_NFD_strstr(const char *str, const char *needle)
{
	int s_len = 0;
	int n_len = 0;

	if (!str) {
		return FALSE;
	}
	s_len = strlen(str);

	if (!needle) {
		return FALSE;
	} else {
		n_len = strlen(needle);
		if (n_len == 0) {
			return FALSE;
		}
	}

	if (s_len < n_len) {
		return FALSE;
	}

	if (__has_nonspacing_mark(str)) {
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
					return FALSE;
				}
			}

			return TRUE;
next:
			p_str = g_utf8_next_char(p_str);
		}
	} else {
		return (gboolean)(!(!strstr(str, needle)));
	}
	return FALSE;
}

static GList *__mf_search_do_find(const char *root,
                                  const char *needle,
                                  mf_search_option option,
                                  ms_handle_t *handle)
{
	DIR *directory = NULL;
	GList *candidate = NULL;

	char *up_needle = NULL;
	char *up_name = NULL;
	char err_buf[MF_ERR_BUF] = {0,};

	if (!handle) {
		ms_error("handle is NULL");
		return NULL;
	}

	if (!handle->result) {
		ms_error("handle->result is NULL");
		return NULL;
	}

	if (!root) {
		ms_error("invaild args");
		return NULL;
	}

	if (!needle && !handle->args->func) {
		return NULL;
	}

	if (!g_file_test(root, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
		ms_error("invaild root_path : %s", root);
		return NULL;
	}

	directory = opendir(root);
	if (directory) {
		mf_search_result_t *result = NULL;
		struct dirent ent_struct;
		struct dirent *entry = NULL;

		result = handle->result;
		__mf_search_thread_lock(handle);
		if (result->current_dir) {
			g_free(result->current_dir);
		}
		result->current_dir = g_strdup(root);
		__mf_search_thread_unlock(handle);
		while ((readdir_r(directory, &ent_struct, &entry) == 0) && entry) {
			if (!(option & MF_SEARCH_OPT_HIDDEN) && (0 == strncmp(entry->d_name, ".", 1))) {
				SECURE_DEBUG("[%s] is hidden file. Skip it", entry->d_name);
				continue;
			}

			if (handle->is_stop == TRUE) {
				mf_debug("break from do find");
				break;
			}

			if (entry->d_type & DT_REG) {
				if (option & MF_SEARCH_OPT_FILE) {
					__mf_search_thread_lock(handle);
					result->total_count++;
					__mf_search_thread_unlock(handle);
					up_name = elm_entry_utf8_to_markup(entry->d_name);//g_utf8_strup(entry->d_name, strlen(entry->d_name));
					gchar *nor_str = g_utf8_strup(up_name, strlen(up_name));
					if (handle->args->func) {
						gchar *path = NULL;
						gssize len = strlen(root) + strlen(entry->d_name) + APPEND_SIZE;	/* for null and slash*/
						path = g_malloc0(sizeof(gchar) * len);
						if (path) {
							g_snprintf(path, len, "%s/%s", root, entry->d_name);
							int category = handle->args->func(path);
							if (category == handle->args->category) {
								__mf_search_thread_lock(handle);
								result->file_list = g_list_append(result->file_list, (gpointer) path);
								result->is_end = FALSE;
								__mf_search_thread_unlock(handle);

							} else {
								free(path);
								path = NULL;
							}
							/*1  TODO: how can i handle else case?*/
						}
					} else if (needle) {
						up_needle = g_utf8_strup(needle, strlen(needle));
						/*	should we check the return value for further use? */
						if (__mf_search_NFD_strstr(nor_str, up_needle)) {
							gchar *path = NULL;
							gssize len = strlen(root) + strlen(entry->d_name) + APPEND_SIZE;	/* for null and slash*/
							path = g_malloc0(sizeof(gchar) * len);
							if (path) {
								g_snprintf(path, len, "%s/%s", root, entry->d_name);

								__mf_search_thread_lock(handle);
								result->file_list = g_list_append(result->file_list, (gpointer) path);
								result->is_end = FALSE;
								__mf_search_thread_unlock(handle);

							}
							/*1  TODO: how can i handle else case?*/
						}
					}

					/*Todo:*/
					g_free(nor_str);
					free(up_needle);
					up_needle = NULL;
					free(up_name);
					up_name = NULL;
				}
			} else if (entry->d_type & DT_DIR) {
				gchar *path = NULL;
				gssize len = 0;

				len = strlen(entry->d_name);
				/*skip current and upper directory*/
				if (0 == strncmp(entry->d_name, ".", strlen(".")) || 0 == strncmp(entry->d_name, "..", strlen(".."))) {
					continue;
				}
				/* we are not going to search /opt/media/SLP_Debug folder */
				if ((strlen(result->current_dir) == strlen(PHONE_FOLDER)) && (strcmp(result->current_dir, PHONE_FOLDER) == 0)
				        && (strlen(entry->d_name) == strlen(DEBUG_FOLDER)) && (strcmp(entry->d_name, DEBUG_FOLDER) == 0)) {
					SECURE_DEBUG("[%s] is hidden folder. Skip it", entry->d_name);
					continue;
				}

				len = strlen(root) + strlen(entry->d_name) + APPEND_SIZE;	/* for null and slash */
				path = g_malloc0(sizeof(gchar) * len);
				if (path) {
					g_snprintf(path, len, "%s/%s", root, entry->d_name);
					candidate = g_list_append(candidate, (gpointer) path);
				}
				/*1  TODO: how can i handle else case?*/
				if (option & MF_SEARCH_OPT_DIR) {
					__mf_search_thread_lock(handle);
					result->total_count++;
					__mf_search_thread_unlock(handle);

					up_name = elm_entry_utf8_to_markup(entry->d_name);//g_utf8_strup(entry->d_name, strlen(entry->d_name));
					if (up_name && needle) {
						up_needle = g_utf8_strup(needle, strlen(needle));
						gchar *nor_str = g_utf8_strup(up_name, strlen(up_name));
						if (__mf_search_NFD_strstr(nor_str, up_needle)) {
							__mf_search_thread_lock(handle);
							result->dir_list = g_list_append(result->dir_list, (gpointer) g_strdup(path));
							result->is_end = FALSE;
							__mf_search_thread_unlock(handle);
						}
						g_free(nor_str);

						free(up_needle);
						up_needle = NULL;
					}
					if (up_name) {
						free(up_name);
						up_name = NULL;
					}
				}
			}
		}
		closedir(directory);
		directory = NULL;
	} else {
		MF_FILE_ERROR_LOG(err_buf, "open failed", root);
	}

	return candidate;
}

static gpointer __mf_search_find_thread(gpointer data)
{
	ms_handle_t *handle = (ms_handle_t *) data;
	if (handle) {
		ms_args_t *args = NULL;
		mf_search_result_t *result = NULL;

		result = handle->result;
		args = handle->args;

		if (args && result) {
			GList *root = NULL;
			GList *candidate = NULL;	/*use this list as stack*/
			root = args->root_path;
#ifdef MF_SEARCH_UPDATE_COUNT
			item_count = 0;
#endif
			while (root) {
				char *path = (char *)root->data;
				if (path) {
					/*push root paths to stack*/
					candidate = g_list_append(candidate, (gpointer) g_strdup(path));
				}
				root = g_list_next(root);
			}

			while (candidate) {
				GList *new_list = NULL;
				GList *list = NULL;
				gchar *item = NULL;

				__mf_search_thread_lock(handle);
				if (handle->is_stop) {
					__mf_search_thread_unlock(handle);
					result->is_end = TRUE;
					goto MF_FIND_THREAD_EXIT;
				}
				__mf_search_thread_unlock(handle);

				list = g_list_first(candidate);
				/*pop one path from stack*/
				candidate = g_list_remove_link(candidate, list);
				item = (gchar *) list->data;
				if (item) {
					ms_debug("current : %s", item);
					/*publish root change message here*/
					if (handle->is_stop) {
						result->is_end = TRUE;
						goto MF_FIND_THREAD_EXIT;
					}
					new_list = __mf_search_do_find(item, args->needle, args->option, handle);
					g_free(item);
					item = NULL;
					g_list_free(list);
					list = NULL;
				}
				/*push new paths to stack*/
				candidate = g_list_concat(new_list, candidate);
			}

			__mf_search_thread_lock(handle);
			result->is_end = TRUE;
			__mf_search_thread_unlock(handle);
			__mf_search_result_publish_msg(MF_SEARCH_PIPE_MSG_FINISHED, handle->result, args->user_data);
MF_FIND_THREAD_EXIT:
#ifdef MF_SEARCH_UPDATE_COUNT
			item_count = 0;
#endif
			if (candidate) {
				g_list_foreach(candidate, (GFunc) g_free, NULL);
				g_list_free(candidate);
				candidate = NULL;
			}
		} else {
			ms_error("args : %p or result : %p is not allocated yet!!", handle->args, handle->result);
		}
	}
	/*g_thread_exit(NULL);*/
	return NULL;
}

int _mf_search_init(ms_handle_t **handle)
{
	//GMutex *lock = NULL;
	ms_handle_t *ms_handle = NULL;

	ms_debug("");

	if (!handle) {
		return MF_SEARCH_ERROR_INVAL_P;
	}

	ms_handle = g_malloc0(sizeof(ms_handle_t));
	if (ms_handle == NULL) {
		ms_error("Fail to allocate memory for handle ");
		*handle = NULL;
		return MF_SEARCH_ERROR_ALLOC;
	}

	ms_handle->state = MF_SEARCH_STATE_INIT;
	ms_handle->is_stop = FALSE;
#if 0
	lock = g_mutex_new();
	if (!lock) {
		ms_error("Fail to create cmd_lock");
		g_free(ms_handle);
		return MF_SEARCH_ERROR_ALLOC;
	}
	ms_handle->cmd_lock = lock;
#endif
	g_mutex_init(&ms_handle->cmd_lock);

	*handle = ms_handle;

	ms_info("Success to make search handle : %p", ms_handle);
	return MF_SEARCH_ERROR_NONE;
}

int _mf_search_start(ms_handle_t *handle,
                     const char **root_path,
                     unsigned int path_num,
                     const char *needle,
                     mf_search_option option,
                     void *user_data,
                     mf_search_filter_cb func,
                     int category,
                     int item_count)
{
	ms_args_t *args = NULL;
	mf_search_result_t *result = NULL;
	mf_search_option l_opt = MF_SEARCH_OPT_NONE;
	int ret = MF_SEARCH_ERROR_NONE;
	int i = 0;

	if (!handle) {
		ms_error("handle is NULL");
		return MF_SEARCH_ERROR_INVAL_P;
	}

	if (handle->state != MF_SEARCH_STATE_INIT) {
		ms_error("invaild state : %d", handle->state);
		return MF_SEARCH_ERROR_INVAL_S;
	}

	if (!root_path || path_num < 1) {
		ms_error("invaild arguments - root[%p], path_num[%d], needle[%p]", root_path, path_num, needle);
		return MF_SEARCH_ERROR_INVAL_P;
	}
	if (!needle && !func) {
		return MF_SEARCH_ERROR_INVAL_P;
	}

	__mf_search_cmd_lock(handle);

	if (handle->args) {
		__mf_search_args_free(handle->args);
		handle->args = NULL;
	}
	handle->args = args = g_malloc0(sizeof(ms_args_t));
	if (!args) {
		ms_error("fail to alloc args");
		ret = MF_SEARCH_ERROR_ALLOC;
		goto FAIL_FREE_MEM;
	}

	if (option == MF_SEARCH_OPT_NONE) {
		ms_warn("option is MF_SEARCH_OPT_NONE, set all option automatically ");
		l_opt = MF_SEARCH_OPT_HIDDEN | MF_SEARCH_OPT_DIR | MF_SEARCH_OPT_FILE;
	} else {
		l_opt = option;
	}

	for (i = 0; i < path_num; i++) {
		const char *path = root_path[i];

		if (!strcmp(path, "(null)")) {
			mf_debug("path is null");
			continue;
		}

		mf_debug("%d th root path is %s", i, path);
#ifdef CHECK_RESTRICTED_PATH
		if (!__mf_search_check_licet_path(path)) {
			ms_error("%dth root path[%s] is invaild", i, path);
			ret = MF_SEARCH_ERROR_INVAL_P;
			goto FAIL_FREE_MEM;
		}
#endif /*CHECK_RESTRICTED_PATH*/
		if (g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)
		        && ((l_opt & MF_SEARCH_OPT_HIDDEN) || strncmp(path, ".", 1))
		        && TRUE) {
			gchar *new_path = NULL;
			gssize len = strlen(path);

			if (path[len - 1] == '/') {
				new_path = g_strndup(path, len - 1);
			} else {
				new_path = g_strndup(path, len);
			}
			args->root_path = g_list_append(args->root_path, (gpointer) new_path);
		} else {
			ms_error("Fail to test %dth root path[%s]", i, path);
			ret = MF_SEARCH_ERROR_INVAL_P;
			goto FAIL_FREE_MEM;
		}
	}
	args->user_data = user_data;
	if (needle) {
		args->needle = g_strdup(needle);//g_utf8_normalize(needle, -1, NORMALIZE_OPTION);
		if (!args->needle) {
			ms_error("fail to alloc args->needle");
			goto FAIL_FREE_MEM;
		}
	}
	args->option = l_opt;
	args->func = func;
	args->category = category;
	args->item_count = item_count;
	if (handle->result) {
		__mf_search_result_free(handle->result);
		handle->result = NULL;
	}
	handle->result = result = g_malloc0(sizeof(mf_search_result_t));
	if (!result) {
		ms_error("fail to alloc result");
		ret = MF_SEARCH_ERROR_ALLOC;
		goto FAIL_FREE_MEM;
	}

#if 0
	handle->thread_mutex = g_mutex_new();
	if (!handle->thread_mutex) {
		ms_error("fail to alloc handle->thread_mutex");
		ret = MF_SEARCH_ERROR_ALLOC;
		goto FAIL_FREE_MEM;
	}
#endif
	g_mutex_init(&handle->thread_mutex);

	handle->is_stop = FALSE;
	handle->result->is_end = FALSE;

	/*create thread for find item.*/
	handle->thread_h = g_thread_new(NULL, (GThreadFunc) __mf_search_find_thread, handle);
	if (!handle->thread_h) {
		ms_error("fail to create __mf_search_find_thread");
		ret = MF_SEARCH_ERROR_INTERNAL;
		goto FAIL_FREE_MEM;
	}
	/*create idler for reporting find result.*/
	handle->state = MF_SEARCH_STATE_SEARCH;
	__mf_search_cmd_unlock(handle);
	return MF_SEARCH_ERROR_NONE;

FAIL_FREE_MEM:
	if (args) {
		__mf_search_args_free(args);
		handle->args = NULL;
	}

	if (result) {
		__mf_search_result_free(result);
		handle->result = NULL;
	}

	g_mutex_clear(&handle->thread_mutex);

	if (handle->thread_h) {
		__mf_search_thread_lock(handle);
		handle->is_stop = TRUE;
		__mf_search_thread_unlock(handle);
		g_thread_join(handle->thread_h);
		handle->thread_h = NULL;
	}
	__mf_search_cmd_unlock(handle);

	return ret;
}

int _mf_search_stop(ms_handle_t *handle)
{
	ms_debug("");

	if (!handle) {
		ms_error("handle is NULL");
		return MF_SEARCH_ERROR_INVAL_P;
	}

	if (handle->state != MF_SEARCH_STATE_SEARCH) {
		ms_error("invaild state : %d", handle->state);
		return MF_SEARCH_ERROR_INVAL_S;
	}

	__mf_search_cmd_lock(handle);

	__mf_search_thread_lock(handle);
	handle->is_stop = TRUE;
	__mf_search_thread_unlock(handle);

	pthread_mutex_lock(&gLockSearchMsg);
	if (flagSearchMsg == 0) {
		flagSearchMsg = 1;
		pthread_cond_signal(&gCondSearchMsg);
	}
	pthread_mutex_unlock(&gLockSearchMsg);

	if (handle->thread_h) {
		g_thread_join(handle->thread_h);
		handle->thread_h = NULL;
	}

	g_mutex_clear(&handle->thread_mutex);

	if (handle->args) {
		__mf_search_args_free(handle->args);
		handle->args = NULL;
	}
	if (handle->result) {
		__mf_search_result_free(handle->result);
		handle->result = NULL;
	}

	handle->state = MF_SEARCH_STATE_INIT;
	handle->is_stop = FALSE;

	__mf_search_cmd_unlock(handle);

	return MF_SEARCH_ERROR_NONE;
}

void _mf_search_finalize(ms_handle_t **handle)
{
	ms_handle_t *ms_handle = *handle;

	ms_debug("");

	if (!ms_handle) {
		ms_warn("invaild handle");
		return;
	}

	if (ms_handle->state == MF_SEARCH_STATE_SEARCH) {
		mf_search_stop((mf_search_handle)ms_handle);
	}
	/*      __mf_search_cmd_lock(ms_handle); */
	/*      __mf_search_cmd_unlock(ms_handle); */

	g_mutex_clear(&ms_handle->cmd_lock);

	g_free(ms_handle);
	*handle = NULL;

	return;
}

/*+++++++++++++++++++++++++ UTIL APIs ++++++++++++++++++++++++++++++ */
static void __mf_search_result_publish_msg(mf_search_pipe_msg_type type, void *result, void *user_data)
{
	struct appdata *ap = (struct appdata *)user_data;
	/*generate message block*/
	if (ap->mf_FileOperation.sync_pipe) {
		mf_search_pipe_msg msg;
		memset(&msg, 0, sizeof(mf_search_pipe_msg));

		msg.mf_sp_msg_type = type;
		if (msg.mf_sp_msg_type == MF_SEARCH_PIPE_MSG_RESULT_REPORT) {
			msg.report_result = g_strdup((gchar *) result);
			msg.current_path = NULL;
		} else if (msg.mf_sp_msg_type == MF_SEARCH_PIPE_MSG_ROOT_CHANGE) {
			msg.report_result = NULL;
			msg.current_path = g_strdup((gchar *) result);
			mf_debug("current path is %s", msg.current_path);
		} else if (msg.mf_sp_msg_type == MF_SEARCH_PIPE_MSG_FINISHED) {
			msg.report_result = result;
			msg.current_path = NULL;
		}
#ifdef MF_SEARCH_UPDATE_COUNT
		else if (msg.mf_sp_msg_type == MF_SEARCH_PIPE_MSG_DOING) {
			mf_search_result_t *report_result = (mf_search_result_t *)g_malloc0(sizeof(mf_search_result_t));
			report_result->dir_list = g_list_copy(((mf_search_result_t *)result)->dir_list);
			report_result->file_list = g_list_copy(((mf_search_result_t *)result)->file_list);
			msg.report_result = report_result;
			msg.current_path = NULL;
		}
#endif
		else {
			msg.report_result = NULL;
			msg.current_path = NULL;
		}

		/*write message to pipe*/
		ecore_pipe_write(ap->mf_FileOperation.sync_pipe, &msg, sizeof(msg));
	}
}

gchar *_mf_search_result_dir_get(mf_search_result_t * result)
{
	gchar *name = NULL;
	if (result) {
		GList *list = NULL;
		list = result->dir_list;
		if (list && list->data) {
			gchar *item = (gchar *) list->data;
			result->dir_list = g_list_remove(list, item);
			name = item;
		}
	}
	return name;
}

gchar *_mf_search_result_file_get(mf_search_result_t * result)
{
	gchar *name = NULL;
	if (result) {
		GList *list = NULL;
		list = result->file_list;
		if (list && list->data) {
			gchar *item = (gchar *) list->data;
			result->file_list = g_list_remove(list, item);
			name = item;
		}
	}
	return name;
}

gboolean _mf_search_result_is_end(mf_search_result_t *result)
{
	gboolean end = FALSE;
	if (result) {
		end = result->is_end;
	}
	return end;
}

guint _mf_search_result_total_count_get(mf_search_result_t *result)
{
	guint count = 0;
	if (result) {
		count = result->total_count;
	}
	return count;
}

gchar *_mf_search_result_current_dir_get(mf_search_result_t * result)
{
	gchar *c_dir = NULL;
	if (result) {
		if (result->current_dir) {
			c_dir = result->current_dir;
			result->current_dir = NULL;
		}
	}
	return c_dir;
}
