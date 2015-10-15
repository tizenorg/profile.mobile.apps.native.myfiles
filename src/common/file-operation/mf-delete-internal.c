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
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <fts.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <ftw.h>

#include "mf-delete-internal.h"
#include "mf-cancel.h"
#include "mf-fo-common.h"
#include "mf-fo-internal.h"
#include "mf-fo-debug.h"
#include "mf-media-content.h"

GSList *delete_list = NULL;

#ifndef SAFE_FREE
#define SAFE_FREE(x) do {\
				if ((x) != NULL) {\
					free(x); \
					x = NULL;\
				} \
			} while (0)
#endif

int _mf_delete_delete_regfile(const char *file, struct stat *file_statp, mf_cancel *cancel, _mf_fo_msg_cb msg_cb, void *msg_data)
{
	mode_t mode = 0;
	off_t size = 0;
	int err = 0;
	char err_buf[MF_ERR_BUF] = {0,};
	if (!file) {
		mf_fo_loge("file is NULL");
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	if (cancel && mf_cancel_check_cancel(cancel)) {
		return 1;
	}

	if (file_statp) {
		mode = file_statp->st_mode;
		size = file_statp->st_size;
	} else {
		struct stat info;
		if (stat(file, &info)) {
			MF_FILE_ERROR_LOG(err_buf, "Fail to stat file ", file);
			err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
			if (msg_cb) {
				msg_cb(MF_MSG_ERROR, file, 0, err, msg_data);
			}
			return err;
		}
		mode = info.st_mode;
		size = info.st_size;
	}

	if (!S_ISREG(mode) && !S_ISSOCK(mode)) {
		mf_fo_loge("[%s] is not regular file", file);

		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_TYPE);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, file, 0, err, msg_data);
		}
		return err;
	}

	if (remove(file)) {
		MF_FILE_ERROR_LOG(err_buf, "Fail to delete ", file);
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, file, 0, err, msg_data);
		}
		return err;
	}

	mf_fo_logd("success to delete FILE : %s", file);
	/*success message*/
	if (msg_cb) {
		msg_cb(MF_MSG_DOING, file, size, 0, msg_data);
	}

	return 0;
}

static int __get_delete_directory_hierarchies(const char *pathname, const struct stat *statptr, int type)
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
			delete_list = g_slist_prepend(delete_list, info);
			SECURE_DEBUG("File pathname is [%s]", pathname);
			break;
		} else {
			break;
		}
	case FTW_SL:
		info = calloc(sizeof(mf_fo_dir_list_info), 1);
		if (info != NULL) {
			info->ftw_path = g_strdup(pathname);
			info->type = type;
			delete_list = g_slist_append(delete_list, info);
			SECURE_DEBUG("Link pathname is [%s]", pathname);
			break;
		} else {
			break;
		}
	case FTW_D:
		info = calloc(sizeof(mf_fo_dir_list_info), 1);
		if (info != NULL) {
			info->ftw_path = g_strdup(pathname);
			info->type = type;
			delete_list = g_slist_prepend(delete_list, info);
			mf_debug("Directory pathname is [%s]", pathname);
			//process file
			break;
		} else {
			break;
		}
	case FTW_DNR:
		mf_debug("=================== FTW_DNR file exists");
		break;
	case FTW_NS:
		mf_debug("=================== FTW_NS file exists");
		break;
	default:
		mf_debug("Default pathname is [%s]", pathname);
	}

	return 0;
}

int _mf_delete_delete_directory(const char *dir, mf_cancel *cancel, _mf_fo_msg_cb msg_cb, void *msg_data)
{
	int err = 0;
	int ret = -1;
	char err_buf[MF_ERR_BUF] = {0,};
	if (!dir) {
		mf_fo_loge("check argument dir");
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	ret = ftw(dir, __get_delete_directory_hierarchies, 16);
	if (ret == 0) {
		mf_debug();
		mf_fo_dir_list_info *ent = NULL;
		GSList *list = NULL;
		list = delete_list;
		while (list) {
			if (cancel && mf_cancel_check_cancel(cancel)) {
				goto DO_CANCEL;
			}
			ent = (mf_fo_dir_list_info *)list->data;
			if (ent->type == FTW_F) {
				if (ent->ftw_path == NULL || strlen(ent->ftw_path) == 0) {
					list = g_slist_next(list);
					continue;
				}
				err = _mf_delete_delete_regfile(ent->ftw_path, NULL, cancel, msg_cb, msg_data);
				if (err > 0) {
					goto DO_CANCEL;
				} else if (err < 0) {
					goto ERROR_CLOSE_FD;
				}

			} else if (ent->type == FTW_SL) {
				if (ent->ftw_path == NULL || strlen(ent->ftw_path) == 0) {
					list = g_slist_next(list);
					continue;
				}
				ret = remove(ent->ftw_path);
				if (ret) {
					MF_FILE_ERROR_LOG(err_buf, "Fail to delete ", ent->ftw_path);
					err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
					if (msg_cb) {
						msg_cb(MF_MSG_ERROR, ent->ftw_path, 0, err, msg_data);
					}
				}
				if (err > 0) {
					goto DO_CANCEL;
				} else if (err < 0) {
					goto ERROR_CLOSE_FD;
				}

			} else if (ent->type == FTW_D) {
				if (cancel && mf_cancel_check_cancel(cancel)) {
					goto DO_CANCEL;
				}
				if (ent->ftw_path == NULL || strlen(ent->ftw_path) == 0) {
					list = g_slist_next(list);
					continue;
				}

				if (remove(ent->ftw_path)) {
					MF_FILE_ERROR_LOG(err_buf, "Fail to delete ", ent->ftw_path);
					err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
					if (msg_cb) {
						msg_cb(MF_MSG_ERROR, ent->ftw_path, 0, err, msg_data);
					}
					goto ERROR_CLOSE_FD;
				} else {
					mf_fo_logd("success to delete DIR : %s", ent->ftw_path);
					if (msg_cb) {
						msg_cb(MF_MSG_DOING, ent->ftw_path, 0, 0, msg_data);
					}
				}
			}
			mf_debug("ent->path is [%s]", ent->ftw_path);
			list = g_slist_next(list);
		}
	} else {
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
		if (msg_cb) {
			msg_cb(MF_MSG_ERROR, dir, 0, err, msg_data);
		}
		_mf_fo_free_directory_hierarchies(&delete_list);
		return err;
	}

	_mf_fo_free_directory_hierarchies(&delete_list);

	return 0;

ERROR_CLOSE_FD:
	_mf_fo_free_directory_hierarchies(&delete_list);
	return err;

DO_CANCEL:
	_mf_fo_free_directory_hierarchies(&delete_list);
	return 1;
}

int _mf_delete_del_internal(const char *item, mf_cancel *cancel, _mf_fo_msg_cb msg_callback, void *msg_data)
{
	struct stat info;
	int err = 0;
	char err_buf[MF_ERR_BUF] = {0,};
	if (!item || strlen(item) <= 1) {
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_ARGUMENT);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, NULL, 0, err, msg_data);
		}
		return err;
	}

	if (stat(item, &info)) {
		MF_FILE_ERROR_LOG(err_buf, "Fail to stat item ", item);
		err = (_mf_fo_errno_to_mferr(errno) | MF_FO_ERR_SRC_CLASS);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, item, 0, err, msg_data);
		}
		return err;
	}

	if (cancel && mf_cancel_check_cancel(cancel)) {
		goto DO_CANCEL;
	}

	if (S_ISDIR(info.st_mode)) {
		err = _mf_delete_delete_directory(item, cancel, msg_callback, msg_data);
	} else if (S_ISSOCK(info.st_mode)) {
		err = _mf_delete_delete_regfile(item, &info, cancel, msg_callback, msg_data);
	} else if (S_ISREG(info.st_mode)) {
		err = _mf_delete_delete_regfile(item, &info, cancel, msg_callback, msg_data);
	} else {

		mf_fo_loge("item[%s] is not file or directory", item);
		err = MF_FO_ERR_SET(MF_FO_ERR_SRC_CLASS | MF_FO_ERR_TYPE | MF_FO_ERR_REPORT_CLASS);
		if (msg_callback) {
			msg_callback(MF_MSG_ERROR, item, 0, err, msg_data);
		}
		return err;
	}

	if (err > 0) {
		goto DO_CANCEL;
	} else if (err < 0) {
		goto ERROR_RETURN;
	}

	return 0;

ERROR_RETURN:
	return err;

DO_CANCEL:
	return 1;
}
