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
#include <errno.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>	/*__NR_gettid*/
#include <unistd.h>
#include <ftw.h>
#include <sys/statvfs.h>
#include <storage.h>

#include "mf-fo-internal.h"
#include "mf-fo-common.h"
#include "mf-fo-debug.h"
#include <storage.h>

GSList *dir_list = NULL;
#ifndef SAFE_FREE
#define SAFE_FREE(x) do { \
				if ((x) != NULL) {\
					free(x); \
					x = NULL;\
				} \
			} while (0)
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#ifndef FILENAME_MAX
#define FILENAME_MAX 4096
#endif

static inline long __mf_fo_get_name_max(void)
{
	long max = 0;

#ifdef _PC_NAME_MAX
	max = pathconf("/", _PC_NAME_MAX);
#endif

	if (max < 1) {
		max = NAME_MAX + 1;
	}

	return max;
}

static inline long __mf_fo_get_path_max(void)
{
	long max = 0;

#ifdef _PC_PATH_MAX
	max = pathconf("/", _PC_PATH_MAX);
#endif

	if (max < 1) {
		max = FILENAME_MAX;
	}

	return max;
}

static const char *__mf_fo_get_base_name(const char *path)
{
	const char *base = NULL;


	if (path && (path[0] != '\0')) {
		char *tmp = NULL;
		tmp = strrchr(path, '/');
		if (tmp != NULL && tmp[1] == '\0') {
			if (tmp == path) {
				mf_fo_loge("path is ROOT - %s", path);
			} else {
				mf_fo_loge("invaild arg - %s", path);
			}
		} else {
			base = tmp + 1;
		}
	}
	return base;
}

static int __get_directory_hierarchies(const char *pathname, const struct stat *statptr, int type)
{
	mf_fo_dir_list_info *info = NULL;
	mf_debug("pathname is [%s]\t type is [%d]\t size is [%ld]",
		pathname, type, statptr->st_size);

	switch (type) {

	case FTW_F:
		info = calloc(sizeof(mf_fo_dir_list_info), 1);
		if (info != NULL) {
			info->ftw_path = g_strdup(pathname);
			info->size = statptr->st_size;
			info->type = type;
			dir_list = g_slist_append(dir_list, info);
			SECURE_DEBUG("File pathname is [%s]", pathname);
			break;
		} else {
			break;
		}
	case FTW_D:
		info = calloc(sizeof(mf_fo_dir_list_info), 1);
		if (info != NULL) {
			info->ftw_path = g_strdup(pathname);
			info->size = statptr->st_size;
			info->type = type;
			dir_list = g_slist_append(dir_list, info);
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

static int __mf_fo_get_total_dir_size(const char *dir, unsigned long long *size)
{
	int ret = -1;
	if (!dir) {
		return -EINVAL;
	}
	ret = ftw(dir, __get_directory_hierarchies, 16);
	if (ret == 0) {
		mf_debug();
		mf_fo_dir_list_info *ent = NULL;
		GSList *list = NULL;
		list = dir_list;
		while (list) {
			ent = (mf_fo_dir_list_info *)list->data;
			if (ent->type == FTW_D) {
				*size += MF_VISUAL_FOLDER_SIZE;
			} else if (ent->type == FTW_F) {
				*size += ent->size;
			}
			list = g_slist_next(list);
		}
	} else {
		_mf_fo_free_directory_hierarchies(&dir_list);
		return -(errno);
	}
	_mf_fo_free_directory_hierarchies(&dir_list);

	return 0;
}

char *_mf_fo_get_next_unique_dirname(const char *name, int *errcode)
{
	char *new_name = NULL;
	char *tmp_name = NULL;
	int name_len = 0;
	long p_max = 0;

	if (!name) {
		*errcode = EINVAL;
		return NULL;
	}

	name_len = strlen(name);
	if (name_len <= 1) {
		*errcode = EINVAL;
		return NULL;
	}

	p_max = __mf_fo_get_path_max();

	if (name[name_len - 1] == '/') {
		tmp_name = strndup(name, name_len - 1);
		if (!tmp_name) {
			*errcode = ENOMEM;
			return NULL;
		}
	}

	new_name = malloc(sizeof(char) * p_max);
	

	if (new_name) {
		int next_num = 0;
		long n_max = 0;
              memset(new_name, 0, sizeof(char) * p_max);
		n_max = __mf_fo_get_name_max();

		do {
			int write_len = 0;
			next_num++;
			write_len = snprintf(new_name, p_max, "%s(%d)", tmp_name ? tmp_name : name, next_num);
			if (write_len < 0) {
				/** snprintf may failed. */
				mf_fo_loge("%s", "generate new dir name (snprintf) failed.");
				*errcode = errno;
				goto ERROR_FREE_RETURN;
			} else if (write_len <= name_len) {
				/** new_name must be longger then the old one. */
				mf_fo_loge("wrong length of new_name: [%d], old_name [%d]", write_len, name_len);
				*errcode = EINVAL;
				goto ERROR_FREE_RETURN;
			} else if (write_len >= p_max) {
				mf_fo_loge("write_len[%u] is greater than max[%ld]", write_len, p_max);
				*errcode = ENAMETOOLONG;
				goto ERROR_FREE_RETURN;
			} else {
				const char *b_name = 0;
				b_name = __mf_fo_get_base_name(new_name);
				if (b_name && (mf_util_character_count_get(b_name) > n_max)) {
					mf_fo_loge("b_name length[%u] is greater than name max[%ld]", strlen(b_name), n_max);
					*errcode = ENAMETOOLONG;
					goto ERROR_FREE_RETURN;
				}
			}
		} while (_mf_fo_check_exist(new_name));
	} else {
		*errcode = ENOMEM;
	}

	SAFE_FREE(tmp_name);
	return new_name;

ERROR_FREE_RETURN:
	SAFE_FREE(tmp_name);
	SAFE_FREE(new_name);

	return NULL;
}


char *_mf_fo_get_next_unique_filename(const char *name, int *errcode)
{
	char *new_name = NULL;
	int name_len = 0;
	long p_max = 0;
	const char *base = NULL;

	if (!name) {
		*errcode = EINVAL;
		return NULL;
	}

	name_len = strlen(name);
	if (name_len <= 1) {
		*errcode = EINVAL;
		return NULL;
	}

	base = __mf_fo_get_base_name(name);
	if (!base) {
		*errcode = EINVAL;
		return NULL;
	}

	p_max = __mf_fo_get_path_max();

	new_name = malloc(sizeof(char) * p_max);

	if (new_name) {
		int next_num = 0;
		long n_max = 0;
		int dir_len = 0;
		int base_len = 0;
		const char *ext = NULL;

		n_max = __mf_fo_get_name_max();
		dir_len = (int)(base - name);

		if ((ext = strrchr(name, '.')) != NULL) {
			base_len = (int)(ext - base);
		} else {
			base_len = strlen(base);
		}

		do {
			int write_len = 0;
			next_num++;
			if (ext) {
				write_len = snprintf(new_name, p_max, "%.*s%.*s(%d)%s", dir_len, name, base_len, base, next_num, ext);
			} else {
				write_len = snprintf(new_name, p_max, "%.*s%.*s(%d)", dir_len, name, base_len, base, next_num);
			}

			if (write_len < 0) {
				/** snprintf may failed. */
				mf_fo_loge("%s", "generate new file name (snprintf) failed.");
				*errcode = errno;
				goto ERROR_FREE_RETURN;
			} else if (write_len <= name_len) {
				/** new_name must be longger then the old one. */
				mf_fo_loge("wrong length of new_name: [%d], old_name: [%d]", write_len, name_len);
				*errcode = EINVAL;
				goto ERROR_FREE_RETURN;
			} else if (write_len >= p_max) {
				mf_fo_loge("write_len[%u] is greater than max[%ld]", write_len, p_max);
				*errcode = ENAMETOOLONG;
				goto ERROR_FREE_RETURN;
			} else {
				const char *b_name = NULL;
				b_name = __mf_fo_get_base_name(new_name);
				if (b_name && (strlen(b_name) > n_max)) {
					mf_fo_loge("b_name length[%u] is greater than name max[%ld]", strlen(b_name), n_max);
					*errcode = ENAMETOOLONG;
					goto ERROR_FREE_RETURN;
				}
			}
		} while (_mf_fo_check_exist(new_name));
	} else {
		*errcode = ENOMEM;
	}

	return new_name;

ERROR_FREE_RETURN:
	SAFE_FREE(new_name);

	return NULL;
}

int _mf_fo_get_total_item_size(const char *item, unsigned long long *size)
{
	struct stat info;
	if (!item || !size) {
		return -EINVAL;
	}
	if (stat(item, &info)) {
		mf_fo_loge("Fail to stat item : %s", item);
		return -(errno);
	}

	if (S_ISREG(info.st_mode)) {
		*size = (unsigned long long)info.st_size;
	} else if (S_ISDIR(info.st_mode)) {
		int ret = __mf_fo_get_total_dir_size(item, size);
		if (ret < 0) {
			mf_fo_loge("Fail to get size of directory(%s)", item);
			*size = 0;
			return ret;
		}
	} else {
		mf_fo_loge("item(%s) is not file or directory", item);
		*size = (unsigned long long)info.st_size;
		return -EINVAL;
	}
	return 0;
}

int _mf_fo_get_remain_space(const char *path, unsigned long long *size)
{
	FO_TRACE_BEGIN;
	struct statvfs dst_fs;

	if (!path || !size) {
		FO_TRACE_END;
		mf_error("===============");
		return -EINVAL;
	}
	if (strncmp(path, PHONE_FOLDER, strlen(PHONE_FOLDER)) == 0) {
		if (storage_get_internal_memory_size(&dst_fs) < 0) {
			*size = 0;
			mf_error("===============");
			return 0;
		}
		*size = ((unsigned long long)(dst_fs.f_bsize) * (unsigned long long)(dst_fs.f_bavail));
		mf_fo_loge("available device storage: %llu", *size);
	} else if (statvfs(path, &dst_fs) == 0) {
		*size = ((unsigned long long)(dst_fs.f_bsize) * (unsigned long long)(dst_fs.f_bavail));
	} else {
		mf_error("=============== errno is [%d]", errno);
		FO_TRACE_END;
		return -errno;
	}
	mf_error("=============== available device storage size is [%llu]", *size);
	FO_TRACE_END;

	return 0;
}

inline bool _mf_fo_check_exist(const char *path)
{
	if (path && (access(path, F_OK) == 0)) {
		return true;
	}
	return false;
}

int _mf_fo_errno_to_mferr(int err_no)
{
	mf_error("err_no is [%d] error - %s", err_no, strerror(err_no));

	int err = MF_FO_ERR_SET(MF_FO_ERR_UNKNOWN);
	switch (err_no) {
#ifdef EINVAL
	case EINVAL:
		err = MF_FO_ERR_SET(MF_FO_ERR_ARGUMENT);
		break;
#endif
#ifdef EACCES			/*The requested access to the file is not allowed*/
	case EACCES:		/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_PERMISSION);
		break;
#endif
#ifdef EFAULT			/* pathname points outside your accessible address space*/
	case EFAULT:
		err = MF_FO_ERR_SET(MF_FO_ERR_FAULT);
		break;
#endif
#ifdef EISDIR			/*pathname refers to a directory and the access requested involved writing*/
	case EISDIR:		/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_TYPE);
		break;
#endif
#ifdef EMFILE			/*The process already has the maximum number of files open.*/
	case EMFILE:
		err = MF_FO_ERR_SET(MF_FO_ERR_MAX_OPEN);
		break;
#endif
#ifdef ENOSPC			/*pathname was to be created but the device containing pathname has no room for the new file*/
	case ENOSPC:		/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_SPACE);
		break;
#endif
#ifdef ENOTDIR			/* A component used as a directory in pathname is not*/
	case ENOTDIR:		/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_TYPE);
		break;
#endif
#ifdef EROFS			/*pathname refers to a file on a read-only filesystem and write access was requested*/
	case EROFS:		/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_RO);
		break;
#endif
#ifdef ELOOP			/* Too many symbolic links were encountered in resolving pathname */
	case ELOOP:
		err = MF_FO_ERR_SET(MF_FO_ERR_LOOP);
		break;
#endif
#ifdef ENOMEM			/* Insufficient kernel memory was available */
	case ENOMEM:		/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_MEM);
		break;
#endif
#ifdef ENOENT			/* O_CREAT is not set and the named file does not exist*/
	case ENOENT:		/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_NOT_EXIST);
		break;
#endif
#ifdef ENAMETOOLONG		/*pathname was too long.*/
	case ENAMETOOLONG:	/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_LONG_NAME);
		break;
#endif
#ifdef EFBIG			/* An attempt was made to write a file that exceeds the implementation-defined maximum
				file size or the process file size limit*/
	case EFBIG:		/*report*/
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_BIG_SIZE);
		break;
#endif
#ifdef EIO			/* I/O error */
	case EIO:
		err = MF_FO_ERR_SET(MF_FO_ERR_REPORT_CLASS | MF_FO_ERR_IO);
		break;
#endif
	default:
		break;
	}

	return err;
}

void _mf_fo_free_directory_hierarchies(GSList **glist)
{
	if (*glist == NULL)
		return;
	GSList *list = *glist;
	while (list) {
		mf_fo_dir_list_info *info = NULL;
		info = (mf_fo_dir_list_info *)list->data;
		g_free(info->ftw_path);
		g_free(info);
		list = g_slist_next(list);
	}
	g_slist_free(*glist);
	*glist = NULL;
}

int _mf_fo_get_file_location(const char *path)
{
	int len_phone = strlen(PHONE_FOLDER);
	int len_memory = strlen(MEMORY_FOLDER);
	if (strncmp(path, PHONE_FOLDER, len_phone) == 0) {
		return MYFILE_PHONE;
	} else if (strncmp(path, MEMORY_FOLDER, len_memory) == 0) {
		return MYFILE_MMC;
	} else {
		return MYFILE_NONE;
	}

}
