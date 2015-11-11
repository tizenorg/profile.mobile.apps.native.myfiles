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

#include "mf-move-internal.h"
#include "mf-cancel.h"
#include "mf-fo-common.h"
#include "mf-fo-internal.h"
#include "mf-copy-internal.h"
#include "mf-delete-internal.h"
#include "mf-fo-debug.h"
#include "mf-media-content.h"

GSList *move_list = NULL;

#ifndef SAFE_FREE
#define SAFE_FREE(x) do {\
		if ((x) != NULL) {\
			free(x); \
			x = NULL;\
		} \
	} while (0)
#endif

#define DIR_MODE_BIT (01777)

static gchar *__mf_move_change_root_name(const char *name, const char *old_root, const char *new_root)
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


int _mf_move_move_regfile(const char *src, struct stat *src_statp, const char *dst,
                          unsigned long buf_size, mf_cancel *cancel, _mf_fo_msg_cb msg_cb, void *msg_data)
{
	mode_t src_mode = 0;
	dev_t src_dev = 0;
	off_t src_size = 0;
	struct stat dst_dir_i;
	char *dst_dir = NULL;
	int err = 0;
	char err_buf[MF_ERR_BUF] = {0,};

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
		src_size = src_statp->st_size;
		src_dev = src_statp->st_dev;
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
		src_size = src_info.st_size;
		src_dev = src_info.st_dev;
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


	dst_dir = g_path_get_dirname(dst);
	if (dst_dir) {
		if (stat(dst_dir, &dst_dir_i)) {
			mf_fo_loge("Fail to stat dst dir file : %s", dst_dir);

			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
			}
			free(dst_dir);
			return err;
		}
		free(dst_dir);
	} else {
		mf_fo_loge("fail to get dirname from dst[%s]", dst);
		err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	if (src_dev == dst_dir_i.st_dev) {
		if (rename(src, dst)) {
			MF_FILE_ERROR_LOG(err_buf, "Fail to rename item ", dst);

			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
			}
			return err;
		} else {
			if (err == 0) {
				mf_media_content_scan_file(src);
			}
			mf_fo_logd("success to move file from [%s] to [%s]", src, dst);
			if (msg_cb) {
				msg_cb(MF_MSG_DOING, src, src_size, 0, msg_data);
			}
		}
	} else {
		err = _mf_copy_copy_regfile(src, src_statp, dst, 0, cancel, msg_cb, msg_data);
		if (err == 0) {
			mf_media_content_scan_file(dst);
		}
		if (err > 0) {
			goto CANCEL_RETURN;
		} else if (err < 0) {
			goto ERROR_RETURN;
		}

		err = _mf_delete_delete_regfile(src, src_statp, cancel, NULL, NULL);
		if (err == 0) {
			mf_media_content_scan_file(src);
		}
		if (err > 0) {
			goto CANCEL_RETURN;
		} else if (err < 0) {
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, src, 0, err, msg_data);
			}
			goto ERROR_RETURN;
		}
		mf_fo_logd("[copy/del]success to move file from [%s] to [%s]", src, dst);
	}

	return 0;

ERROR_RETURN:
	return err;


CANCEL_RETURN:
	return 1;
}

static int __get_move_directory_hierarchies(const char *pathname, const struct stat *statptr, int type)
{
	MF_TRACE_BEGIN;
	mf_fo_dir_list_info *info = NULL;
	mf_debug("pathname is [%s]\t type is [%d]\t",
	         pathname, type);
	switch (type) {

	case FTW_F:
		info = calloc(sizeof(mf_fo_dir_list_info), 1);
		if (info != NULL) {
			info->ftw_path = g_strdup(pathname);
			info->type = type;
			move_list = g_slist_append(move_list, info);
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
			move_list = g_slist_append(move_list, info);
			SECURE_DEBUG("File pathname is [%s]", pathname);
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


int _mf_move_move_directory(const char *src, struct stat *src_statp, const char *dst, mf_cancel *cancel, _mf_fo_msg_cb msg_cb, void *msg_data)
{
	mode_t src_mode = 0;
	dev_t src_dev = 0;
	int ret = -1;
	int err = 0;
	gboolean is_same_dev = FALSE;
	char err_buf[MF_ERR_BUF] = {0,};

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
		src_dev = src_statp->st_dev;
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
		src_dev = src_info.st_dev;
		src_mode = src_info.st_mode;
	}

	if (access(dst, F_OK)) {
		if (mkdir(dst, (src_mode & DIR_MODE_BIT))) {
			MF_FILE_ERROR_LOG(err_buf, "Fail to make directory ", dst);
			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
			}
			return err;
		}
	} else {
		mf_fo_logi("directory[%s] is already existed", dst);
		struct stat dst_info;
		if (stat(dst, &dst_info)) {
			mf_fo_loge("Fail to stat dst dir : %s", dst);
			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, dst, 0, err, msg_data);
			}
			return err;
		}

		if (src_dev == dst_info.st_dev) {
			mf_fo_logd("src and dst is same dev");
			is_same_dev = TRUE;
		}
	}
	ret = ftw(src, __get_move_directory_hierarchies, 16);
	if (ret == 0) {
		mf_fo_dir_list_info *ent = NULL;
		GSList *list = move_list;
		list = move_list;
		while (list) {
			if (cancel && mf_cancel_check_cancel(cancel)) {
				goto DO_CANCEL;
			}
			ent = (mf_fo_dir_list_info *)list->data;
			if (ent->type == FTW_D) {
				if (g_strcmp0(ent->ftw_path, src) == 0) {
					list = g_slist_next(list);
					continue;
				}
				char *new_dir = __mf_move_change_root_name(ent->ftw_path, src, dst);
				mf_fo_logd("move dir %s to %s", ent->ftw_path, new_dir);
				if (new_dir) {
					if (is_same_dev) {
						if (!_mf_fo_check_exist(new_dir)) {
							unsigned long long size = 0;
							int err_code = 0;
							err_code = _mf_fo_get_total_item_size(ent->ftw_path, &size);
							if (err_code < 0) {
								err = (_mf_fo_errno_to_mferr(-err_code) | MF_FO_ERR_SRC_CLASS);
								if (msg_cb) {
									msg_cb(MF_MSG_ERROR, ent->ftw_path, 0, err, msg_data);
								}
								free(new_dir);
								goto ERROR_CLOSE_FD;
							} else {
								if (msg_cb) {
									msg_cb(MF_MSG_DOING, ent->ftw_path, 0, 0, msg_data);
								}
							}
						} else {
							mf_fo_logi("directory[%s] is already existed", new_dir);
						}
					} else {
						if (!_mf_fo_check_exist(new_dir)) {
							struct stat info;
							if (stat(ent->ftw_path, &info) == 0) {
								if (mkdir(new_dir, (info.st_mode & DIR_MODE_BIT))) {
									mf_fo_loge("Fail to make directory [%s]", new_dir);
									/*set FTS_SKIP to skip children of current*/
									/*fts_set(fts, ent, FTS_SKIP);*/
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
								/*fts_set(fts, ent, FTS_SKIP);*/
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
									err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_ARGUMENT);
									if (msg_cb) {
										msg_cb(MF_MSG_ERROR, new_dir, 0, err, msg_data);
									}
									free(new_dir);
									goto ERROR_CLOSE_FD;
								}
							} else {
								MF_FILE_ERROR_LOG(err_buf, "Fail to stat ", new_dir);
								err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
								if (msg_cb) {
									msg_cb(MF_MSG_ERROR, new_dir, 0, err, msg_data);
								}
								free(new_dir);
								goto ERROR_CLOSE_FD;
							}
						}
						free(new_dir);
					}
				} else {

					err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
					if (msg_cb) {
						msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
					}
					goto ERROR_CLOSE_FD;
				}
			} else if (ent->type == FTW_F) {
				char *new_file = __mf_move_change_root_name(ent->ftw_path, src, dst);
				if (new_file) {
					err = _mf_move_move_regfile(ent->ftw_path, NULL, new_file, 0, cancel, msg_cb, msg_data);
					if (err == 0) {
						mf_media_content_scan_file(new_file);
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
		_mf_fo_free_directory_hierarchies(&move_list);
		return err;
	}
	_mf_fo_free_directory_hierarchies(&move_list);
	return 0;

ERROR_CLOSE_FD:
	_mf_fo_free_directory_hierarchies(&move_list);
	return err;

DO_CANCEL:
	_mf_fo_free_directory_hierarchies(&move_list);
	return 1;
}

int _mf_move_move_internal(const char *src, const char *dst_dir,
                           mf_cancel *cancel, mf_req_callback request_callback, _mf_fo_msg_cb msg_callback, void *msg_data)
{
	char *src_basename = NULL;
	char *new_dst = NULL;
	char *next_name = NULL;
	int base_size = 0;
	int root_size = 0;
	int with_slash = 1;
	int alloc_size = 1;	/*for null*/
	int err = 0;
	struct stat src_info;
	struct stat dst_dir_i;
	char err_buf[MF_ERR_BUF] = {0,};

	if (!src || strlen(src) <= 1) {
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	if (!dst_dir) {
		err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	if (access(dst_dir, R_OK | W_OK)) {

		err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_PERMISSION);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, dst_dir, 0, err, msg_data);
		}
		return err;
	}

	if (stat(src, &src_info)) {
		MF_FILE_ERROR_LOG(err_buf, "Fail to stat src", src);
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, src, 0, err, msg_data);
		}
		return err;
	}

	if (stat(dst_dir, &dst_dir_i)) {
		MF_FILE_ERROR_LOG(err_buf, "Fail to stat dst_dir", src);

		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, dst_dir, 0, err, msg_data);
		}
		return err;
	}

	if (S_ISDIR(src_info.st_mode)) {
		if (g_strcmp0(dst_dir, src) == 0) {
			mf_fo_loge("dst is child of src - src : %s, dst : %s", src, dst_dir);

			err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_ARGUMENT);
			if (msg_callback) {
				msg_callback(MF_MSG_ERROR, dst_dir, 0, err, msg_data);
			}
			return err;
		}
	}
	src_basename = g_path_get_basename(src);
	if (!src_basename) {
		mf_fo_loge("fail to get basename from src[%s]", src);
		err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
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
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
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

		if (src_info.st_dev == dst_dir_i.st_dev) {
			unsigned long long size = 0;
			int ret = _mf_fo_get_total_item_size(src, &size);
			if (ret < 0) {
				MF_FILE_ERROR_LOG(err_buf, "Fail to get item size", new_dst);

				err = (_mf_fo_errno_to_mferr(-ret) | MF_FO_ERR_SRC_CLASS);
				if (msg_callback) {
					msg_callback(MF_MSG_ERROR, src, 0, err, msg_data);
				}
				goto ERROR_FREE_MEM;
			} else {
				if (rename(src, new_dst)) {
					MF_FILE_ERROR_LOG(err_buf, "Fail to rename item", new_dst);

					err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_DST_CLASS);
					if (msg_callback) {
						msg_callback(MF_MSG_ERROR, new_dst, 0, err, msg_data);
					}
					goto ERROR_FREE_MEM;
				} else {
					if (err == 0) {
						if (S_ISDIR(src_info.st_mode)) {
							mf_media_content_scan_folder(src);
							mf_media_content_scan_folder(new_dst);

						} else {
							mf_media_content_scan_file(src);
							mf_media_content_scan_file(new_dst);
						}
					}
					if (msg_callback) {
						msg_callback(MF_MSG_DOING, src, size, 0, msg_data);
					}
				}
			}
		} else {
			if (S_ISDIR(src_info.st_mode)) {
				err = _mf_move_move_directory(src, &src_info, new_dst, cancel, msg_callback, msg_data);
				if (err == 0) {
					err = _mf_delete_delete_directory(src, cancel, NULL, NULL);
					if (err > 0) {
						goto CANCEL_FREE_MEM;
					} else if (err < 0) {
						if (msg_callback) {
							msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
						}
						goto ERROR_FREE_MEM;
					}
					mf_media_content_scan_folder(src);
					mf_media_content_scan_folder(new_dst);
				}
			} else if (S_ISREG(src_info.st_mode)) {
				err = _mf_move_move_regfile(src, &src_info, new_dst, 0, cancel, msg_callback, msg_data);
				if (err == 0) {
					mf_media_content_scan_file(new_dst);
				}
			} else {
				mf_fo_loge("item[%s] is not file or directory", src);
				err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_TYPE | MF_FO_ERR_REPORT_CLASS);
				if (msg_callback) {
					msg_callback(MF_MSG_ERROR, src, 0, err, msg_data);
				}
				goto ERROR_FREE_MEM;
			}
		}
	} else {
		mf_request_type result = MF_REQ_NONE;
		if (request_callback) {
			mf_fo_request *req = mf_request_new();
			if (req) {
				mf_request_set_path(req, new_dst);
				mf_fo_logi("~~~~~~ waiting for request");
				request_callback(req, msg_data);
				result = mf_request_get_result(req);
				mf_fo_logi("~~~~~~ get request : %d", result);

				if (result == MF_REQ_RENAME) {
					next_name = mf_request_get_new_name(req);
				}
				mf_request_free(req);
			} else {
				mf_fo_loge("Fail to alloc request");
				err = MF_FO_ERR_SET(MF_FO_ERR_COMMON_CLASS | MF_FO_ERR_MEM);
				if (msg_callback) {
					msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
				}
				goto ERROR_FREE_MEM;
			}
		}

		switch (result) {
		case MF_REQ_NONE:
		case MF_REQ_MERGE: {
			struct stat dst_info;
			if (stat(new_dst, &dst_info)) {
				MF_FILE_ERROR_LOG(err_buf, "Fail to stat new_dst", new_dst);
				if (msg_callback) {
					msg_callback(MF_MSG_ERROR, NULL, 0, errno, msg_data);
				}
				goto ERROR_FREE_MEM;
			}

			if (S_ISDIR(src_info.st_mode)) {
				if (!S_ISDIR(dst_info.st_mode)) {
					mf_fo_loge("src[%s] is directory, but dst[%s] is already existed and not a directory", src, new_dst);

					err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_TYPE);
					if (msg_callback) {
						msg_callback(MF_MSG_ERROR, new_dst, 0, err, msg_data);
					}
					goto ERROR_FREE_MEM;
				}
				err = _mf_move_move_directory(src, &src_info, new_dst, cancel, msg_callback, msg_data);
				if (err == 0) {
					err = _mf_delete_delete_directory(src, cancel, NULL, NULL);
					if (err > 0) {
						goto CANCEL_FREE_MEM;
					} else if (err < 0) {
						if (msg_callback) {
							msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
						}
						goto ERROR_FREE_MEM;
					}
					mf_media_content_scan_folder(new_dst);
				}
			} else if (S_ISREG(src_info.st_mode)) {
				if (!S_ISREG(dst_info.st_mode)) {
					mf_fo_loge("src[%s] is file, but dst[%s] is already existed and not a file", src, new_dst);
					err = MF_FO_ERR_SET(MF_FO_ERR_DST_CLASS | MF_FO_ERR_TYPE);
					if (msg_callback) {
						msg_callback(MF_MSG_ERROR, new_dst, 0, err, msg_data);
					}
					goto ERROR_FREE_MEM;
				}
				err = _mf_move_move_regfile(src, &src_info, new_dst, dst_info.st_blksize, cancel, msg_callback, msg_data);
				if (err == 0) {
					mf_media_content_scan_file(new_dst);
				}
			}

		}
		break;
		case MF_REQ_RENAME: {
			if (next_name) {
				if (S_ISDIR(src_info.st_mode)) {
					err = _mf_move_move_directory(src, &src_info, next_name, cancel, msg_callback, msg_data);
					if (err == 0) {
						mf_media_content_scan_folder(next_name);
					}
				} else if (S_ISREG(src_info.st_mode)) {
					err = _mf_move_move_regfile(src, &src_info, next_name, 0, cancel, msg_callback, msg_data);
					if (err == 0) {
						mf_media_content_scan_file(next_name);
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
						if (msg_callback) {
							msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
						}
						goto ERROR_FREE_MEM;
					}
					err = _mf_move_move_directory(src, &src_info, next_name, cancel, msg_callback, msg_data);
					if (err == 0) {
						err = _mf_delete_delete_directory(src, cancel, NULL, NULL);
						if (err > 0) {
							goto CANCEL_FREE_MEM;
						} else if (err < 0) {
							if (msg_callback) {
								msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
							}
							goto ERROR_FREE_MEM;
						}
						mf_media_content_scan_folder(next_name);
					}
				} else if (S_ISREG(src_info.st_mode)) {
					int errcode = 0;
					next_name = _mf_fo_get_next_unique_filename(new_dst, &errcode);
					if (!next_name) {
						mf_fo_loge("Fail to get next file name [%s]", new_dst);
						err = (_mf_fo_errno_to_mferr(errcode) | MF_FO_ERR_DST_CLASS);
						if (msg_callback) {
							msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
						}
						goto ERROR_FREE_MEM;
					}
					err = _mf_move_move_regfile(src, &src_info, next_name, 0, cancel, msg_callback, msg_data);
					if (err == 0) {
						mf_media_content_scan_file(next_name);
					}
				}
				SAFE_FREE(next_name);
			}
		}
		break;
		case MF_REQ_SKIP: {
			if (msg_callback) {
				unsigned long long size = 0;
				_mf_fo_get_total_item_size(src, &size);
				msg_callback(MF_MSG_SKIP, NULL, size, 0, msg_data);
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
	SAFE_FREE(src_basename);
	SAFE_FREE(new_dst);

	return err;

CANCEL_FREE_MEM:

	mf_fo_logi("move cancelled");
	SAFE_FREE(new_dst);

	return 1;
}
