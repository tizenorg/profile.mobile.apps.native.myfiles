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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <ftw.h>

#include "mf-copy-internal.h"
#include "mf-cancel.h"
#include "mf-fo-common.h"
#include "mf-fo-internal.h"
#include "mf-media-content.h"
#include "mf-fo-debug.h"
#include "mf-file-util.h"

struct _mf_copy_handle_intenal {
	GList *src_items;
	char *dst_dir;
	mf_cancel *cancel;
	void *u_data;
	Ecore_Pipe *pipe;
	gboolean sync;

	GMutex *lock;
	GCond *cond;

	mf_fo_msg msg;
	mf_fo_request *req;
};

GSList *copy_list = NULL;

#ifndef SAFE_FREE
#define SAFE_FREE(x) do {\
		if ((x) != NULL) {\
			free(x); x = NULL;\
		} \
	} while (0)
#endif

#define MSG_REPORT_PERIOD (1)
#define DEF_ALLLOC_SIZE (1024*4*2)	/*((4)*(1024))*/
#define DIR_MODE_BIT (01777)
#define FILE_MODE_BIT (S_IRWXU | S_IRWXG | S_IRWXO)
#define PER_HANDLE_MAX_SIZE (10*1024*1024)
#define PER_HANDLE_MIN_SIZE (1024*1024)


static gchar *__mf_copy_change_root_name(const char *name, const char *old_root, const char *new_root)
{
	gchar *new_name = NULL;

	if (name && old_root && new_root) {
		int old_len = strlen(old_root);
		int new_len = strlen(new_root);
		int name_len = strlen(name);
		const char *base = NULL;
		GString *n = NULL;

		if ((strstr(name, old_root) == NULL)
		        || (name_len <= old_len)
		        || ((name[old_len] == '/' && name[old_len + 1] == '\0'))
		        || FALSE) {
			mf_fo_loge("invaild args - name : [%s], old_root : [%s]", name, old_root);
			return NULL;
		}

		base = name + old_len;
		if (name[old_len] == '/') {
			base += 1;
		}

		n = g_string_new(new_root);
		if (n) {
			if (n->str[new_len - 1] == '/') {
				g_string_append_printf(n, "%s", base);
			} else {
				g_string_append_printf(n, "/%s", base);
			}
			new_name = g_string_free(n, FALSE);
		}
	}
	return new_name;
}


int _mf_copy_copy_regfile(const char *src, struct stat *src_statp, const char *dst_file, unsigned long buf_size, mf_cancel *cancel, _mf_fo_msg_cb msg_cb,
                          void *msg_data)
{
	mf_retvm_if(msg_cb == NULL, 0, "msg_cb is NULL");
	FO_TRACE_BEGIN;
	FILE *src_f = NULL;
	FILE *dst_f = NULL;
	void *buf = NULL;
	unsigned long alloc_size = DEF_ALLLOC_SIZE;
	ssize_t r_size = 0;
	unsigned long long remain_size = 0;
	mode_t src_mode = 0;
	int err = 0;
	char err_buf[MF_ERR_BUF] = { 0, };
	struct stat src_info;
	char *dst = (char*) dst_file;
	if (!src) {
		mf_fo_loge("check argument src");
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}
	if (!dst) {
		mf_fo_loge("check argument dst");
		err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}
	int stat_ret = stat(src, &src_info);

	if (src_statp) {
		src_mode = src_statp->st_mode;
	} else {
		if (stat_ret) {
			mf_fo_loge("Fail to stat src file : %s", src);
			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, src, 0, err, msg_data);
			}
			return err;
		}
		src_mode = src_info.st_mode;
	}


	if (!S_ISREG(src_mode)) {
		mf_fo_loge("src[%s] is not regular file", src);
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_TYPE);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, src, 0, err, msg_data);
		}
		return err;
	}
	src_f = fopen(src, "rb");
	if (!src_f) {
		mf_fo_loge("Fail to fopen %s file", src);
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, src, 0, err, msg_data);
		}
		return err;
	}

	int location = _mf_fo_get_file_location(dst);
	char *copy_dst = dst;
	char *copy_folder = NULL;
	char *others_path = NULL;
	if (location == MYFILE_PHONE) {
		copy_dst = (char *)dst;
		const gchar *name = g_path_get_basename(dst);
		others_path = TEMP_FOLDER_FOR_COPY_PHONE;
		copy_folder = g_strconcat(others_path, "/", ".operation_temp", NULL);
		dst = g_strconcat(copy_folder, "/", name, NULL);
	} else if (location == MYFILE_MMC) {
		copy_dst = (char *)dst;
		const gchar *name = g_path_get_basename(dst);
		others_path = TEMP_FOLDER_FOR_COPY_MMC;
		copy_folder = g_strconcat(others_path, "/", ".operation_temp", NULL);
		dst = g_strconcat(TEMP_FOLDER_FOR_COPY_MMC, "/", name, NULL);
	}
	mf_error("===================== copy_dst is [%s], dst is [%s]", copy_dst, dst);
	if (copy_folder && !mf_file_exists(copy_folder)) {
		mf_mkpath(copy_folder);
	}
	dst_f = fopen(dst, "wb");
	if (!dst_f) {
		mf_fo_loge("Fail to fopen %s file", dst);
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
		}
		goto ERROR_CLOSE_FD;
	}
	if (buf_size == 0) {
		struct stat dst_info;
		if (stat(dst, &dst_info)) {
			MF_FILE_ERROR_LOG(err_buf, "Fail to stat dst file", dst);
			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
			}
			goto ERROR_CLOSE_FD;
		}
	}
//Prevent issue fix
	/*
			if (dst_info.st_blksize > 0) {
				alloc_size = dst_info.st_blksize;
			}
		} else {
			alloc_size = buf_size;
		}*/
	alloc_size = DEF_ALLLOC_SIZE;

	buf = malloc(alloc_size);
	if (!buf) {
		mf_fo_loge("fail to alloc buf, alloc_size : %lu", alloc_size);
		err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_MEM);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
		}
		goto ERROR_CLOSE_FD;
	}

	int count = 0;
	ssize_t msg_size = 0;

	struct _mf_copy_handle_intenal *cp_handle = NULL;
	cp_handle = (struct _mf_copy_handle_intenal *)msg_data;
	mf_debug("cp_handle->msg.total_size=%lld", cp_handle->msg.total_size);
	int dynamic_size = 0;
	if (cp_handle->msg.total_size > PER_HANDLE_MAX_SIZE) {
		dynamic_size = 64;
	} else if (cp_handle->msg.total_size <= PER_HANDLE_MAX_SIZE &&
	           cp_handle->msg.total_size > PER_HANDLE_MIN_SIZE) {
		dynamic_size = 8;
	} else {
		dynamic_size = 1;
	}

	int dst_fd = fileno(dst_f);
	err = _mf_fo_get_remain_space(dst, &remain_size);
	if (err < 0) {
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
		msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
		goto ERROR_CLOSE_FD;
	}

	mf_error(">>>>>>>>>>>>>>>>>>>remain_size = %lld", remain_size);
	mf_error(">>>>>>>>>>>>>>>>>>>src_info.st_size = %lld",
	         (long long)src_info.st_size);
	if (remain_size == 0 || remain_size < src_info.st_size) {
		err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_SPACE);
		msg_cb(MF_MSG_ERROR, cp_handle->dst_dir, 0, err, msg_data);
		goto ERROR_CLOSE_FD;
	}
	int sync_value = 0;
	ssize_t sync_msg_size = 0;
	while ((r_size = fread(buf, 1, alloc_size, src_f)) > 0) {
		ssize_t total = r_size;
		void *buf_p = buf;
		sync_value++;
		if (sync_value == 1024) {//Fixed P131129-01238, P131206-05287 P131207-01294, Don't need call again, it is checked at the begining of coping, it will cause the lockup.
			err = _mf_fo_get_remain_space(dst, &remain_size);
			if (err < 0) {
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
				remove(dst);

				goto ERROR_CLOSE_FD;
			}

			if (remain_size == 0 && msg_cb) {
				int err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_SPACE);
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
				remove(dst);
				goto ERROR_CLOSE_FD;
			}
		}

		count++;
		while (total > 0) {
			ssize_t w_size = 0;
			w_size = fwrite(buf_p, 1, total, dst_f);
			if (ferror(dst_f) != 0 || (r_size != w_size)) {
				MF_FILE_ERROR_LOG(err_buf, "fail to write", dst_f);
				err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
				if (msg_cb) {
					msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
				}
				remove(dst);
				goto ERROR_CLOSE_FD;
			}


			buf_p += w_size;
			total -= w_size;
		}
		if (cancel && mf_cancel_check_cancel(cancel)) {
			remove(dst);
			goto CANCEL_CLOSE_FD;
		}

		if (count == dynamic_size && msg_cb) {
			msg_size += r_size;
			msg_cb(MF_MSG_DOING, src, msg_size, 0, msg_data);
			msg_size = 0;
		} else {
			msg_size += r_size;
		}
		sync_msg_size += r_size;
		if (sync_value == 1024) {//Fixed P131129-01238, P131206-05287 P131207-01294, Don't need call again, it is checked at the begining of coping, it will cause the lockup.
			int ret = posix_fadvise(dst_fd, 0, sync_msg_size, POSIX_FADV_DONTNEED);//Fixed
			if (ret != 0) {
				mf_debug(" >>>>posix_fadvise ret=%d", ret);
			}
			sync_value = 0;//Reset sync_value
			sync_msg_size = 0;
		}
		count = count % dynamic_size;
	}
	int ret = posix_fadvise(dst_fd, 0, src_info.st_size, POSIX_FADV_DONTNEED);//Fixed
	if (ret != 0) {
		mf_debug(" >>>>posix_fadvise ret=%d", ret);
	}

	if (ferror(src_f) != 0 && errno == 5) {
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
		}
		if (remove(dst) == -1) {
			mf_error("remove file %s failed.", dst);
		}
		goto ERROR_CLOSE_FD;
	}

	if (msg_size > 0 && msg_cb) {
		msg_size += r_size;
		msg_cb(MF_MSG_DOING, src, msg_size, 0, msg_data);
		msg_size = 0;
	}
	free(buf);

	fclose(src_f);
	fclose(dst_f);
	if (copy_dst) {
		if (rename(dst, copy_dst)) {
			MF_FILE_ERROR_LOG(err_buf, "Fail to rename item ", dst);

			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
			}
			SAFE_FREE_CHAR(dst);
			copy_dst = NULL;
			return err;
		} else {
			/* Fix bug P140619-05013.
			 * mf_media_content_scan_file will be called outside imediately
			 * after this function returns, no need to scan file twice.
			 * if (err == 0) {
			 *         mf_media_content_scan_file(copy_dst);
			 * }
			 */
			SAFE_FREE_CHAR(dst);
			copy_dst = NULL;
		}
	}
	mf_file_rmdir(copy_folder);
	return 0;

ERROR_CLOSE_FD:


	if (src_f) {
		fclose(src_f);
		src_f = NULL;
	}
	if (dst_f) {
		fclose(dst_f);
		dst_f = NULL;
		int ret = remove(dst);
		if (ret != 0) {
			mf_debug(" >>>>remove ret=%d", ret);
		}
	}
	if (buf) {
		free(buf);
	}
	mf_file_rmdir(copy_folder);
	return err;


CANCEL_CLOSE_FD:
	if (buf) {
		free(buf);
	}
	if (src_f) {
		fclose(src_f);
		src_f = NULL;
	}
	if (dst_f) {
		fclose(dst_f);
		dst_f = NULL;
		int ret = remove(dst);
		if (ret != 0) {
			mf_debug(" >>>>remove ret=%d", ret);
		}
	}
	mf_file_rmdir(copy_folder);
	return 1;
}

static int __get_copy_directory_hierarchies(const char *pathname, const struct stat *statptr, int type)
{
	mf_fo_dir_list_info *info = NULL;
	mf_debug("pathname is [%s]\t type is [%d]\t",
	         pathname, type);

	switch (type) {

	case FTW_F:
		info = calloc(sizeof(mf_fo_dir_list_info), 1);
		if (info != NULL) {
			info->ftw_path = g_strdup(pathname);
			info->type = type;
			copy_list = g_slist_append(copy_list, info);
			SECURE_DEBUG("File pathname is [%s]", pathname);
			break;
		} else {
			break;
		}
	case FTW_D:
		info = calloc(sizeof(mf_fo_dir_list_info), 1);
		if (info != NULL) {
			info->ftw_path = g_strdup(pathname);
			info->type = type;
			copy_list = g_slist_append(copy_list, info);
			mf_debug("Directory pathname is [%s]", pathname);
			//process file
			break;
		} else {
			break;
		}
	default:
		mf_debug("Default pathname is [%s]", pathname);
	}

	return 0;
}

int _mf_copy_copy_directory(const char *src, struct stat *src_statp, const char *dst, mf_cancel *cancel, _mf_fo_msg_cb msg_cb, void *msg_data)
{
	mf_retvm_if(msg_cb == NULL, 0, "msg_cb is NULL");
	int ret = -1;
	mode_t src_mode = 0;
	int err = 0;
	unsigned long long remain_size = 0;
	char err_buf[MF_ERR_BUF] = { 0, };
	if (!src) {
		mf_fo_loge("check argument src");
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}
	if (!dst) {
		mf_fo_loge("check argument dst");
		err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	if (src_statp) {
		src_mode = src_statp->st_mode;
	} else {
		struct stat src_info;
		if (stat(src, &src_info)) {
			mf_fo_loge("Fail to stat src file : %s", src);
			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, src, 0, err, msg_data);
			}
			return err;
		}
		src_mode = src_info.st_mode;
	}

	char *parent_dir = mf_dir_get(dst);
	if (parent_dir) {
		err = _mf_fo_get_remain_space(parent_dir, &remain_size);
		SAFE_FREE(parent_dir);
		if (err < 0) {
			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
			msg_cb(MF_MSG_ERROR, parent_dir, 0, err, msg_data);
			return err;
		}

		if (remain_size == 0) {
			err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_SPACE);
			msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
			return err;
		}
	}
	if (access(dst, F_OK)) {
		if (mkdir(dst, (src_mode & DIR_MODE_BIT))) {
			MF_FILE_ERROR_LOG(err_buf, "Fail to make directory", dst);
			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
			}
			return err;
		} else {
			if (msg_cb) {
				msg_cb(MF_MSG_DOING, dst, MF_VISUAL_FOLDER_SIZE, 0, msg_data);
			}
		}
	} else {
		mf_fo_logd("directory[%s] is already existed", dst);
	}

	ret = ftw(src, __get_copy_directory_hierarchies, 16);
	if (ret == 0) {
		mf_debug();
		mf_fo_dir_list_info *ent = NULL;
		GSList *list = NULL;
		list = copy_list;
		while (list) {
			if (cancel && mf_cancel_check_cancel(cancel)) {
				goto DO_CANCEL;
			}
			ent = (mf_fo_dir_list_info *)list->data;
			SECURE_DEBUG("name is [%s] type is [%d]", ent->ftw_path, ent->type);
			if (ent->type == FTW_D) {
				if (ent->ftw_path == NULL || strlen(ent->ftw_path) == 0) {
					list = g_slist_next(list);
					continue;
				}
				if (g_strcmp0(ent->ftw_path, src) == 0) {
					list = g_slist_next(list);
					continue;
				}
				char *new_dir = __mf_copy_change_root_name(ent->ftw_path, src, dst);
				mf_fo_logi("copy dir %s to %s", ent->ftw_path, new_dir);
				if (new_dir) {
					if (!_mf_fo_check_exist(new_dir)) {
						struct stat info;
						if (stat(ent->ftw_path, &info) == 0) {
							if (mkdir(new_dir, (info.st_mode & DIR_MODE_BIT))) {
								/* fts_set(fts, ent, FTS_SKIP); */
								mf_fo_loge("Fail to make directory [%s]", new_dir);
								err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
								if (msg_cb) {
									msg_cb(MF_MSG_ERROR, new_dir, 0, err, msg_data);
								}
								free(new_dir);
								goto ERROR_CLOSE_FD;
							} else {
								if (msg_cb) {
									msg_cb(MF_MSG_DOING, ent->ftw_path, 0, 0, msg_data);
								}
							}
						} else {
							MF_FILE_ERROR_LOG(err_buf, "Fail to stat ", ent->ftw_path);
							/* fts_set(fts, ent, FTS_SKIP); */
							err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
							if (msg_cb) {
								msg_cb(MF_MSG_ERROR, ent->ftw_path, 0, err, msg_data);
							}
							free(new_dir);
							goto ERROR_CLOSE_FD;
						}
					} else {
						struct stat new_dst_info;
						if (stat(new_dir, &new_dst_info) == 0) {
							if (S_ISDIR(new_dst_info.st_mode)) {
								if (msg_cb) {
									msg_cb(MF_MSG_DOING, ent->ftw_path, 0, 0, msg_data);
								}
							} else {
								mf_fo_loge("[%s] is already existed, and this one is not directory", new_dir);
								/*set FTS_SKIP to skip children of current*/
								/*fts_set(fts, ent, FTS_SKIP);*/
								err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_ARGUMENT);
								if (msg_cb) {
									msg_cb(MF_MSG_ERROR, new_dir, 0, err, msg_data);
								}
								free(new_dir);
								goto ERROR_CLOSE_FD;
							}
						} else {
							MF_FILE_ERROR_LOG(err_buf, "Fail to stat ", new_dir);
							/*fts_set(fts, ent, FTS_SKIP);*/
							err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
							if (msg_cb) {
								msg_cb(MF_MSG_ERROR, new_dir, 0, err, msg_data);
							}
							free(new_dir);
							goto ERROR_CLOSE_FD;
						}
					}
					free(new_dir);
				} else {
					err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
					if (msg_cb) {
						msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
					}
					goto ERROR_CLOSE_FD;
				}
			} else if (ent->type == FTW_F) {
				if (ent->ftw_path == NULL || strlen(ent->ftw_path) == 0) {
					list = g_slist_next(list);
					continue;
				}
				char *new_file = __mf_copy_change_root_name(ent->ftw_path, src, dst);
				if (new_file && msg_cb != NULL) {
					err = _mf_copy_copy_regfile(ent->ftw_path, NULL, new_file, 0, cancel, msg_cb, msg_data);
					if (err == 0) {
						//mf_media_content_scan_file(new_file);
					}
					free(new_file);
					if (err > 0) {
						goto DO_CANCEL;
					} else if (err < 0) {
						goto ERROR_CLOSE_FD;
					}
				} else {
					err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
					if (msg_cb) {
						msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
					}
					goto ERROR_CLOSE_FD;
				}
			}
			mf_debug("ent->path is [%s]", ent->ftw_path);
			list = g_slist_next(list);
		}

	} else {
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, src, 0, err, msg_data);
		}
		_mf_fo_free_directory_hierarchies(&copy_list);
		return err;

	}
	_mf_fo_free_directory_hierarchies(&copy_list);
	return 0;

ERROR_CLOSE_FD:
	_mf_fo_free_directory_hierarchies(&copy_list);
	return err;

DO_CANCEL:
	_mf_fo_free_directory_hierarchies(&copy_list);
	return 1;
}

int _mf_copy_copy_internal(const char *src, const char *dst_dir, mf_cancel *cancel, _mf_fo_msg_cb msg_func, mf_req_callback req_func, void *msg_data)
{
	int err = 0;
	char *src_basename = NULL;
	char *new_dst = NULL;
	char *next_name = NULL;
	int base_size = 0;
	int root_size = 0;
	int with_slash = 1;
	int alloc_size = 1;	/*for null*/
	struct stat src_info;
	char err_buf[MF_ERR_BUF] = { 0, };

	if (!src || strlen(src) <= 1) {
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_func) {
			msg_func(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	if (!dst_dir) {
		err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_func) {
			msg_func(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	if (access(dst_dir, R_OK | W_OK)) {
		err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_PERMISSION);
		if (msg_func) {
			msg_func(MF_MSG_ERROR, dst_dir, 0, err, msg_data);
		}
		return err;
	}

	if (stat(src, &src_info)) {
		MF_FILE_ERROR_LOG(err_buf, "Fail to stat src ", src);
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
		if (msg_func) {
			msg_func(MF_MSG_ERROR, src, 0, err, msg_data);
		}
		return err;
	}

	if (S_ISDIR(src_info.st_mode)) {
		if (g_strcmp0(dst_dir, src) == 0) {
			mf_fo_loge("dst is child of src - src : %s, dst : %s", src, dst_dir);
			err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_ARGUMENT);
			if (msg_func) {
				msg_func(MF_MSG_ERROR, dst_dir, 0, err, msg_data);
			}
			return err;
		}
	}
	src_basename = g_path_get_basename(src);
	if (!src_basename) {
		mf_fo_loge("fail to get basename from src[%s]", src);
		err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
		if (msg_func) {
			msg_func(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	base_size = strlen(src_basename);
	root_size = strlen(dst_dir);

	if (dst_dir[root_size - 1] != '/') {
		alloc_size += 1;
		with_slash = 0;
	}

	alloc_size += (base_size + root_size);

	new_dst = malloc(sizeof(char) * (alloc_size));
	if (!new_dst) {
		mf_fo_loge("fail to alloc new dst");
		err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
		if (msg_func) {
			msg_func(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		goto ERROR_FREE_MEM;
	}

	if (with_slash) {
		snprintf(new_dst, alloc_size, "%s%s", dst_dir, src_basename);
	} else {
		snprintf(new_dst, alloc_size, "%s/%s", dst_dir, src_basename);
	}
	SAFE_FREE(src_basename);

	if (cancel && mf_cancel_check_cancel(cancel)) {
		goto CANCEL_FREE_MEM;
	}

	if (access(new_dst, F_OK)) {
		if (S_ISDIR(src_info.st_mode)) {
			err = _mf_copy_copy_directory(src, &src_info, new_dst, cancel, msg_func, msg_data);
			if (err == 0) {
				//mf_media_content_scan_folder(new_dst);
			}
		} else if (S_ISREG(src_info.st_mode)) {
			/*just copy*/
			err = _mf_copy_copy_regfile(src, &src_info, new_dst, 0, cancel, msg_func, msg_data);
			if (err == 0) {
				//mf_media_content_scan_file(new_dst);
			}
		} else {
			mf_fo_loge("item[%s] is not file or directory", src);
			err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_TYPE | MF_FO_ERR_REPORT_CLASS);
			if (msg_func) {
				msg_func(MF_MSG_ERROR, src, 0, err, msg_data);
			}
			goto ERROR_FREE_MEM;
		}
	} else {
		mf_request_type result = MF_REQ_NONE;
		if (req_func) {
			mf_fo_request *req = mf_request_new();
			if (req) {
				mf_request_set_path(req, new_dst);
				mf_fo_logd("~~~~~~ waiting for request");
				req_func(req, msg_data);
				result = mf_request_get_result(req);
				mf_fo_logd("~~~~~~ get request : %d", result);
				if (result == MF_REQ_RENAME) {
					next_name = mf_request_get_new_name(req);
				}
				mf_request_free(req);
			} else {
				mf_fo_loge("Fail to alloc request");
				err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
				if (msg_func) {
					msg_func(MF_MSG_ERROR, NULL, 0, err, msg_data);
				}
				goto ERROR_FREE_MEM;
			}
		}

		switch (result) {
		case MF_REQ_NONE:
		case MF_REQ_MERGE: {
			struct stat dst_info;
			if (stat(new_dst, &dst_info)) {
				MF_FILE_ERROR_LOG(err_buf, "Fail to stat new_dst ", new_dst);
				if (msg_func) {
					msg_func(MF_MSG_ERROR, NULL, 0, errno, msg_data);
				}
				goto ERROR_FREE_MEM;
			}

			if (S_ISDIR(src_info.st_mode)) {
				if (!S_ISDIR(dst_info.st_mode)) {
					mf_fo_loge("src[%s] is directory, but dst[%s] is already existed and not a directory", src, new_dst);

					err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_TYPE);
					if (msg_func) {
						msg_func(MF_MSG_ERROR, new_dst, 0, err, msg_data);
					}
					goto ERROR_FREE_MEM;
				}
				/*just copy*/
				err = _mf_copy_copy_directory(src, &src_info, new_dst, cancel, msg_func, msg_data);
				if (err == 0) {
					//mf_media_content_scan_folder(new_dst);
				}
			} else if (S_ISREG(src_info.st_mode)) {
				if (!S_ISREG(dst_info.st_mode)) {
					mf_fo_loge("src[%s] is file, but dst[%s] is already existed and not a file", src, new_dst);
					err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_TYPE);
					if (msg_func) {
						msg_func(MF_MSG_ERROR, new_dst, 0, err, msg_data);
					}
					goto ERROR_FREE_MEM;
				}
				/*just copy*/
				err = _mf_copy_copy_regfile(src, &src_info, new_dst, dst_info.st_blksize, cancel, msg_func, msg_data);
				if (err == 0) {
					//mf_media_content_scan_file(new_dst);
				}
			}

		}
		break;
		case MF_REQ_RENAME: {
			if (next_name) {
				if (S_ISDIR(src_info.st_mode)) {
					err = _mf_copy_copy_directory(src, &src_info, next_name, cancel, msg_func, msg_data);
					if (err == 0) {
						//mf_media_content_scan_folder(next_name);
					}
				} else if (S_ISREG(src_info.st_mode)) {
					err = _mf_copy_copy_regfile(src, &src_info, next_name, 0, cancel, msg_func, msg_data);
					if (err == 0) {
						//mf_media_content_scan_file(next_name);
					}
				}
				SAFE_FREE(next_name);
			} else {
				if (S_ISDIR(src_info.st_mode)) {
					int errcode = 0;
					next_name = _mf_fo_get_next_unique_dirname(new_dst, &errcode);
					if (!next_name) {
						mf_fo_loge("Fail to get next directory name [%s]", new_dst);
						err = (_mf_fo_errno_to_mferr(errcode) | MF_FO_ERR_DST_CLASS);
						if (msg_func) {
							msg_func(MF_MSG_ERROR, NULL, 0, err, msg_data);
						}
						goto ERROR_FREE_MEM;
					}
					err = _mf_copy_copy_directory(src, &src_info, next_name, cancel, msg_func, msg_data);
					if (err == 0) {
						//mf_media_content_scan_folder(next_name);
					}
				} else if (S_ISREG(src_info.st_mode)) {
					int errcode = 0;
					next_name = _mf_fo_get_next_unique_filename(new_dst, &errcode);
					if (!next_name) {
						mf_fo_loge("Fail to get next file name [%s]", new_dst);
						err = (_mf_fo_errno_to_mferr(errcode) | MF_FO_ERR_DST_CLASS);
						if (msg_func) {
							msg_func(MF_MSG_ERROR, NULL, 0, err, msg_data);
						}
						goto ERROR_FREE_MEM;
					}
					err = _mf_copy_copy_regfile(src, &src_info, next_name, 0, cancel, msg_func, msg_data);
					if (err == 0) {
						//mf_media_content_scan_file(next_name);
					}
				}
				SAFE_FREE(next_name);
			}
		}
		break;
		case MF_REQ_SKIP: {
			if (msg_func) {
				unsigned long long size = 0;
				/*1 TODO : Do i need to report error, if i fail to get size ?*/
				_mf_fo_get_total_item_size(src, &size);
				msg_func(MF_MSG_SKIP, NULL, size, 0, msg_data);
			}
		}
		break;
		case MF_REQ_CANCEL: {
			if (cancel) {
				mf_cancel_do_cancel(cancel);
			}
			goto CANCEL_FREE_MEM;
		}
		break;
		default:
			abort();
			break;

		}
	}
	SAFE_FREE(new_dst);

	if (err > 0) {
		goto CANCEL_FREE_MEM;
	} else if (err < 0) {
		goto ERROR_FREE_MEM;
	}

	return 0;

ERROR_FREE_MEM:
	mf_fo_logi("Copy error");
	mf_file_recursive_rm(TEMP_FOLDER_FOR_COPY_PHONE);
	mf_file_recursive_rm(TEMP_FOLDER_FOR_COPY_MMC);

	SAFE_FREE(src_basename);
	SAFE_FREE(new_dst);

	return err;

CANCEL_FREE_MEM:

	mf_fo_logi("Copy cancelled");
	mf_file_recursive_rm(TEMP_FOLDER_FOR_COPY_PHONE);
	mf_file_recursive_rm(TEMP_FOLDER_FOR_COPY_MMC);

	SAFE_FREE(new_dst);
	SAFE_FREE(next_name);

	return 1;
}
