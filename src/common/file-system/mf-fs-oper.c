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


#include <libgen.h>
#include <glib.h>
#include <errno.h>
#include <sys/statvfs.h>

#include "mf-fs-util.h"
#include "mf-util.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-file-util.h"

#define MF_FS_MEMORY_RESERVE_VALUE (100*1048576)
#define MF_FS_MEMORY_USR_PATH "/opt/usr"

//static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
static int __mf_fs_oper_sort_by_date_cb_O2R(const void *d1, const void *d2);
static int __mf_fs_oper_sort_by_type_cb_A2Z(const void *d1, const void *d2);
static int __mf_fs_oper_sort_by_size_cb_S2L(const void *d1, const void *d2);
static int __mf_fs_oper_sort_by_name_cb_Z2A(const void *d1, const void *d2);
static int __mf_fs_oper_sort_by_date_cb_R2O(const void *d1, const void *d2);
static int __mf_fs_oper_sort_by_type_cb_Z2A(const void *d1, const void *d2);
static int __mf_fs_oper_sort_by_size_cb_L2S(const void *d1, const void *d2);

/*********************
**Function name:	mf_fs_oper_print_node
**Parameter:
**	fsNodeInfo *pNode:	the file system node information need to print
**
**Return value:
**	void
**
**Action:
**	printf the file system node information for debug
**
*********************/
void mf_fs_oper_print_node(fsNodeInfo *pNode)
{
	if (pNode) {
		/*mf_debug("path is [%s]\nname is [%s]\ndate is [%s]\ntype is [%d]\nsize is [%u]\nextension is [%s]\n\n",pNode->path,
		   pNode->name,asctime(gmtime(&(pNode->date))),pNode->type,pNode->size, pNode->ext);
		 */
	}
}

/*********************
**Function name:	mf_fs_oper_get_file
**Parameter:
**	const char *path:	full path to get file name
**
**Return value:
**	const char*:	file name
**
**Action:
**	get file name from full path
**
*********************/
static const char *mf_fs_oper_get_file(const char *path)
{
	char *result = NULL;

	if (!path) {
		return NULL;
	}
	if ((result = strrchr(path, '/'))) {
		result++;
	} else {
		result = (char *)path;
	}
	return result;
}

/*********************
**Function name:	mf_fs_oper_error
**Parameter:
**	const char *src:	source path
**	const char *dst:	destination path
**	int check_option:	check option
**
**Return value:
**	error code
**
**Action:
**	input parameter checking
**
*********************/
int mf_fs_oper_error(const char *src, const char *dst, int check_option)
{
	if ((check_option & MF_ERROR_CHECK_SRC_ARG_VALID) && (src == NULL)) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}
	if ((check_option & MF_ERROR_CHECK_DST_ARG_VALID) && (dst == NULL)) {
		return MYFILE_ERR_DST_ARG_INVALID;
	}

	if ((check_option & MF_ERROR_CHECK_SRC_EXIST) && (!mf_file_exists(src))) {
		return MYFILE_ERR_SRC_NOT_EXIST;
	}
	if ((check_option & MF_ERROR_CHECK_DST_EXIST) && (!mf_file_exists(dst))) {
		return MYFILE_ERR_DST_NOT_EXIST;
	}

	if (check_option & MF_ERROR_CHECK_SRC_PATH_VALID) {
		if (!mf_is_dir(src)) {
			if (mf_file_attr_is_right_file_path(src)) {
				return MYFILE_ERR_INVALID_FILE_PATH;
			}
		} else {
			if (mf_file_attr_is_right_dir_path(src)) {
				return MYFILE_ERR_INVALID_DIR_PATH;
			}
		}
	}
	if (check_option & MF_ERROR_CHECK_DST_PATH_VALID) {
		if (!mf_is_dir(dst)) {
			int ret = mf_file_attr_is_right_file_path(dst);
			if (ret != 0) {
				return ret;
			}
		} else {
			int ret = mf_file_attr_is_right_dir_path(dst);
			if (ret != 0) {
				return ret;
			}
		}
	}

	if (check_option & MF_ERROR_CHECK_SRC_PARENT_DIR_EXIST) {
		char *parent_path = NULL;
		if (mf_file_attr_get_parent_path(src, &parent_path)) {
			if (!mf_file_exists(parent_path)) {
				SAFE_FREE_CHAR(parent_path);
				return MYFILE_ERR_DIR_NOT_FOUND;
			}
		}
		SAFE_FREE_CHAR(parent_path);
	}

	if (check_option & MF_ERROR_CHECK_DST_PARENT_DIR_EXIST) {
		char *parent_path = NULL;
		if (mf_file_attr_get_parent_path(dst, &parent_path)) {
			if (!mf_file_exists(parent_path)) {
				SAFE_FREE_CHAR(parent_path);
				return MYFILE_ERR_DIR_NOT_FOUND;
			}
		}
		SAFE_FREE_CHAR(parent_path);
	}

	if (check_option & MF_ERROR_CHECK_DUPLICATED) {
		char *parent_path = NULL;

		if (!mf_file_attr_get_parent_path(dst, &parent_path)) {
			if (mf_file_attr_is_duplicated_name(parent_path, mf_fs_oper_get_file(dst))) {
				SAFE_FREE_CHAR(parent_path);
				return MYFILE_ERR_DUPLICATED_NAME;
			}
			SAFE_FREE_CHAR(parent_path);
		} else {
			SAFE_FREE_CHAR(parent_path);
			return MYFILE_ERR_GET_PARENT_PATH_FAIL;
		}
	}

	return MYFILE_ERR_NONE;
}

/*********************
**Function name:	mf_fs_oper_read_dir
**Parameter:
**	char *path:				path which we need to read
**	Eina_List** dir_list:	output parameter of dir list under specified path
**	Eina_List** file_list:	output parameter of file list under specified path
**
**Return value:
**	error code
**
**Action:
**	read element under the specified path
**
*********************/
int mf_fs_oper_read_dir(const char *path, Eina_List ** dir_list, Eina_List ** file_list)
{
	DIR *pDir = NULL;
	struct dirent ent_struct;
	struct dirent *ent;

	mf_retvm_if(path == NULL, MYFILE_ERR_INVALID_ARG, "path is null");
	mf_retvm_if(dir_list == NULL, MYFILE_ERR_INVALID_ARG, "dir_list is null");
	mf_retvm_if(file_list == NULL, MYFILE_ERR_INVALID_ARG, "file_list is null");
	int option = MF_ERROR_CHECK_SRC_ARG_VALID | MF_ERROR_CHECK_SRC_EXIST | MF_ERROR_CHECK_SRC_PATH_VALID;
	int storage_type = mf_fm_svc_wrapper_get_location(path);
	int ret = mf_fs_oper_error(path, NULL, option);
	if (ret != MYFILE_ERR_NONE) {
		return ret;
	}

	pDir = opendir(path);

	if (pDir == NULL) {
		return MYFILE_ERR_DIR_OPEN_FAIL;
	}

	while ((readdir_r(pDir, &ent_struct, &ent) == 0) && ent) {
		GString *childpath = NULL;
		fsNodeInfo *pNode = NULL;

		int len = strlen(ent->d_name);
		if ((len == 1 && strncmp(ent->d_name, ".", strlen(".")) == 0) || (len == 2 && strncmp(ent->d_name, "..", strlen("..")) == 0)) {
			continue;
		}
		int hiden_state = 0;
		mf_util_get_pref_value(PREF_TYPE_HIDEN_STATE, &hiden_state);
		if (hiden_state == MF_HIDEN_HIDE) {
			if (strncmp(ent->d_name, ".", strlen(".")) == 0 || strncmp(ent->d_name, "..", strlen("..")) == 0) {
				continue;
			}
		}

		if ((ent->d_type & DT_DIR) == 0 && (ent->d_type & DT_REG) == 0) {
			continue;
		}
		if ((ent->d_type & DT_DIR) != 0) {
			if ((strlen(path) == strlen(PHONE_FOLDER)) && (strcmp(path, PHONE_FOLDER) == 0)
			        && (strlen(ent->d_name) == strlen(DEBUG_FOLDER)) && (strcmp(ent->d_name, DEBUG_FOLDER) == 0)) {
				continue;
			}
		}

		pNode = (fsNodeInfo *) malloc(sizeof(fsNodeInfo));

		if (pNode == NULL) {
			continue;
		}
		memset(pNode, 0, sizeof(fsNodeInfo));
		/*set path */
		pNode->path = g_strdup(path);
		/*set name */
		pNode->name = g_strdup(ent->d_name);
		/* set storage type */
		pNode->storage_type = storage_type;
		pNode->list_type = mf_list_normal;
		/*set type */
		if (ent->d_type & DT_DIR) {
			pNode->type = FILE_TYPE_DIR;
		} else if (ent->d_type & DT_REG) {
			mf_file_attr_get_file_category(ent->d_name, &(pNode->type));
		}
		/*set date & size */
		childpath = g_string_new(path);
		if (childpath == NULL) {
			SAFE_FREE_CHAR(pNode->path);
			SAFE_FREE_CHAR(pNode->name);
			SAFE_FREE_CHAR(pNode);
			continue;
		}
		g_string_append_printf(childpath, "/%s", ent->d_name);
		mf_file_attr_get_file_stat(childpath->str, &pNode);
		if (pNode->type == FILE_TYPE_DIR) {
			*dir_list = eina_list_append(*dir_list, pNode);
		} else {
			ret = mf_file_attr_get_file_ext(childpath->str, &pNode->ext);
			if (ret != MYFILE_ERR_NONE) {
				pNode->ext = NULL;
				pNode->type = mf_file_attr_get_file_type_by_mime(childpath->str);
			}
			*file_list = eina_list_append(*file_list, pNode);
		}
		g_string_free(childpath, TRUE);
	}
	closedir(pDir);

	return MYFILE_ERR_NONE;
}


static int __mf_fs_oper_sort_by_priority(const void *d1, const void *d2, int sequence_type)
{
	int ret = 0;
	switch (sequence_type) {
	case MF_SORT_BY_PRIORITY_TYPE_A2Z:
		ret = __mf_fs_oper_sort_by_date_cb_O2R(d1, d2);
		if (ret == 0) {
			ret = __mf_fs_oper_sort_by_size_cb_S2L(d1, d2);
			if (ret == 0) {
				ret = __mf_fs_oper_sort_by_name_cb_A2Z(d1, d2);
			}
		}
		break;
	case MF_SORT_BY_PRIORITY_TYPE_Z2A:
		ret = __mf_fs_oper_sort_by_date_cb_R2O(d1, d2);
		if (ret == 0) {
			ret = __mf_fs_oper_sort_by_size_cb_L2S(d1, d2);
			if (ret == 0) {
				ret = __mf_fs_oper_sort_by_name_cb_Z2A(d1, d2);
			}
		}
		break;
	case MF_SORT_BY_PRIORITY_DATE_O2R:
		ret = __mf_fs_oper_sort_by_name_cb_A2Z(d1, d2);
		break;
	case MF_SORT_BY_PRIORITY_DATE_R2O:
		ret = __mf_fs_oper_sort_by_name_cb_Z2A(d1, d2);
		break;
	case MF_SORT_BY_PRIORITY_SIZE_S2L:
		ret = __mf_fs_oper_sort_by_name_cb_A2Z(d1, d2);
		break;
	case MF_SORT_BY_PRIORITY_SIZE_L2S:
		ret = __mf_fs_oper_sort_by_name_cb_Z2A(d1, d2);
		break;
	default:
		break;
	}
	return ret;
}
/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the Assic table

**
*********************/
int __mf_fs_oper_sort_by_name_cb_A2Z(const void *d1, const void *d2)
{
	fsNodeInfo *txt1 = (fsNodeInfo *) d1;
	fsNodeInfo *txt2 = (fsNodeInfo *) d2;
	gchar *name1 = NULL;
	gchar *name2 = NULL;
	int result = 0;

	if (!txt1 || !txt1->name) {
		return (1);
	}
	if (!txt2 || !txt2->name) {
		return (-1);
	}

	name1 = g_ascii_strdown(txt1->name, strlen(txt1->name));
	if (name1 == NULL) {
		return (-1);
	}
	name2 = g_ascii_strdown(txt2->name, strlen(txt2->name));
	if (name2 == NULL) {
		g_free(name1);
		name1 = NULL;
		return (-1);
	}
	result = g_strcmp0(name1, name2);

	g_free(name1);
	name1 = NULL;
	g_free(name2);
	name2 = NULL;
	return result;

}

/*********************
**Function name:	__sort_by_date_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the later created the later shown
*********************/
static int __mf_fs_oper_sort_by_date_cb_O2R(const void *d1, const void *d2)
{
	int ret = 0;
	fsNodeInfo *time1 = (fsNodeInfo *) d1;
	fsNodeInfo *time2 = (fsNodeInfo *) d2;

	if (!d1) {
		return 1;
	}
	if (!d2) {
		return -1;
	}

	if (time1->date > time2->date) {
		ret = 1;
	} else if (time1->date < time2->date) {
		ret = -1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_fs_oper_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_DATE_O2R);
	}
	return ret;
}

/*********************
**Function name:	__sort_by_type_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 < d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the category type value
*********************/
static int __mf_fs_oper_sort_by_type_cb_A2Z(const void *d1, const void *d2)
{
	fsNodeInfo *type1 = (fsNodeInfo *) d1;
	fsNodeInfo *type2 = (fsNodeInfo *) d2;
	gchar *ext1 = NULL;
	gchar *ext2 = NULL;
	int result = 0;

	if (type1 == NULL || type1->ext == NULL) {
		return 1;
	}

	if (type2 == NULL || type2->ext == NULL) {
		return -1;
	}
	ext1 = g_ascii_strdown(type1->ext, strlen(type1->ext));
	if (ext1 == NULL) {
		return (-1);
	}
	ext2 = g_ascii_strdown(type2->ext, strlen(type2->ext));
	if (ext2 == NULL) {
		g_free(ext1);
		ext1 = NULL;
		return (-1);
	}
	result = g_strcmp0(ext1, ext2);

	g_free(ext1);
	ext1 = NULL;
	g_free(ext2);
	ext2 = NULL;

	if (result == 0) {
		result = __mf_fs_oper_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_TYPE_A2Z);
	}

	return result;
}

/*order:	the one with smaller size will be shown earlier*/
/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by size, rule is the smaller the later shown
*********************/
static int __mf_fs_oper_sort_by_size_cb_S2L(const void *d1, const void *d2)
{
	int ret = 0;
	fsNodeInfo *size1 = (fsNodeInfo *) d1;
	fsNodeInfo *size2 = (fsNodeInfo *) d2;

	if (!d1) {
		return 1;
	}

	if (!d2) {
		return -1;
	}

	if (size1->size > size2->size) {
		ret = 1;
	} else if (size1->size < size2->size) {
		ret = -1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_fs_oper_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_SIZE_S2L);
	}
	return ret;
}

/*********************
**Function name:	__mf_fs_oper_sort_by_name_cb_Z2A
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	1	if d1 > d2
**	-1	if d1 <= d2
**
**Action:
**	sort the list order by the Assic table

**
*********************/
static int __mf_fs_oper_sort_by_name_cb_Z2A(const void *d1, const void *d2)
{
	fsNodeInfo *txt1 = (fsNodeInfo *) d1;
	fsNodeInfo *txt2 = (fsNodeInfo *) d2;

	int result = 0;

	if (!txt1 || !txt1->name) {
		return (1);
	}
	if (!txt2 || !txt2->name) {
		return (-1);
	}
	result = strcasecmp(txt1->name, txt2->name);

	if (result < 0) {
		return (1);
	} else {
		return (-1);
	}
}

/*********************
**Function name:	__sort_by_date_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by the later created the later shown
*********************/
static int __mf_fs_oper_sort_by_date_cb_R2O(const void *d1, const void *d2)
{
	int ret = 0;
	fsNodeInfo *time1 = (fsNodeInfo *) d1;
	fsNodeInfo *time2 = (fsNodeInfo *) d2;

	if (!d1) {
		return -1;
	}
	if (!d2) {
		return 1;
	}
	if (time1->date > time2->date) {
		ret = -1;
	} else if (time1->date < time2->date) {
		ret = 1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_fs_oper_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_DATE_R2O);
	}
	return ret;
}

/*********************
**Function name:	__sort_by_type_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by the category type value
*********************/
static int __mf_fs_oper_sort_by_type_cb_Z2A(const void *d1, const void *d2)
{
	fsNodeInfo *type1 = (fsNodeInfo *) d1;
	fsNodeInfo *type2 = (fsNodeInfo *) d2;
	gchar *ext1 = NULL;
	gchar *ext2 = NULL;
	int result = 0;

	if (type1 == NULL || type1->ext == NULL) {
		return -1;
	}

	if (type2 == NULL || type2->ext == NULL) {
		return 1;
	}

	ext1 = g_ascii_strdown(type1->ext, strlen(type1->ext));
	if (ext1 == NULL) {
		return (1);
	}
	ext2 = g_ascii_strdown(type2->ext, strlen(type2->ext));
	if (ext2 == NULL) {
		g_free(ext1);
		ext1 = NULL;
		return (-1);
	}
	result = g_strcmp0(ext1, ext2);
	g_free(ext1);
	ext1 = NULL;
	g_free(ext2);
	ext2 = NULL;
	if (result == 0) {
		result = __mf_fs_oper_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_TYPE_Z2A);
	}

	return -result;
}

/*order:	the one with smaller size will be shown earlier*/
/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by size, rule is the smaller the later shown
*********************/
static int __mf_fs_oper_sort_by_size_cb_L2S(const void *d1, const void *d2)
{
	int ret = 0;
	fsNodeInfo *size1 = (fsNodeInfo *) d1;
	fsNodeInfo *size2 = (fsNodeInfo *) d2;

	if (!d1) {
		return -1;
	}

	if (!d2) {
		return 1;
	}

	if (size1->size > size2->size) {
		ret = -1;
	} else if (size1->size < size2->size) {
		ret = 1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_fs_oper_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_SIZE_L2S);
	}
	return ret;
}

/*********************
**Function name:	mf_fs_oper_sort_list
**Parameter:
**	Eina_List **list:	the list we need to sort
**	int sort_opt:		sort option
**
**Return value:
**	void
**
**Action:
**	sort the list order by sort option with the call back
*********************/
void mf_fs_oper_sort_list(Eina_List **list, int sort_opt)
{
	Eina_Compare_Cb sort_func = NULL;
	Eina_List *l = NULL;
	fsNodeInfo *data = NULL;
	if (!(*list)) {
		return;
	}
	switch (sort_opt) {
	case MYFILE_SORT_BY_NAME_A2Z:
		sort_func = __mf_fs_oper_sort_by_name_cb_A2Z;
		break;
	case MYFILE_SORT_BY_TYPE_A2Z:
		sort_func = __mf_fs_oper_sort_by_type_cb_A2Z;
		break;
	case MYFILE_SORT_BY_SIZE_S2L:
		sort_func = __mf_fs_oper_sort_by_size_cb_S2L;
		break;
	case MYFILE_SORT_BY_DATE_O2R:
		sort_func = __mf_fs_oper_sort_by_date_cb_O2R;
		break;
	case MYFILE_SORT_BY_NAME_Z2A:
		sort_func = __mf_fs_oper_sort_by_name_cb_Z2A;
		break;
	case MYFILE_SORT_BY_TYPE_Z2A:
		sort_func = __mf_fs_oper_sort_by_type_cb_Z2A;
		break;
	case MYFILE_SORT_BY_SIZE_L2S:
		sort_func = __mf_fs_oper_sort_by_size_cb_L2S;
		break;
	case MYFILE_SORT_BY_DATE_R2O:
		sort_func = __mf_fs_oper_sort_by_date_cb_R2O;
		break;
	default:
		sort_func = __mf_fs_oper_sort_by_type_cb_A2Z;
		break;
	}
	EINA_LIST_FOREACH(*list, l, data) {
		mf_fs_oper_print_node(data);
	}
	*list = eina_list_sort(*list, eina_list_count(*list), sort_func);
	EINA_LIST_FOREACH(*list, l, data) {
		mf_fs_oper_print_node(data);
	}
}


/*********************
**Function name:	mf_fs_oper_create_dir
**Parameter:
**	const char *file:	dir need to be operation
**
**Return value:
**	error code
**
**Action:
**	create dir
*********************/
int mf_fs_oper_create_dir(const char *dir)
{
	int option = MF_ERROR_CHECK_SRC_ARG_VALID | MF_ERROR_CHECK_DUPLICATED;
	int ret = mf_fs_oper_error(dir, dir, option);

	if (ret != 0) {
		return ret;
	}

	ret = mf_file_attr_is_right_dir_path(dir);

	if (ret != 0) {
		return ret;
	}

	errno = 0;
	if (!mf_mkpath(dir)) {
		ret = mf_error_erron_to_mferror(errno);
		if (ret == MYFILE_ERR_NONE) {
			return MYFILE_ERR_DIR_CREATE_FAIL;
		} else {
			return ret;
		}
	}
	sync();
	return MYFILE_ERR_NONE;
}


/*********************
**Function name:	mf_fs_oper_rename_file
**Parameter:
**	const char *src:	source file need to rename
**	const char *dst:	destination file which is to be renamed

**
**Return value:
**	error code
**
**Action:
**	rename a file
*********************/
int mf_fs_oper_rename_file(const char *src, const char *dst)
{
	mf_debug();
	int option = MF_ERROR_CHECK_SRC_ARG_VALID | MF_ERROR_CHECK_DST_ARG_VALID |
	             MF_ERROR_CHECK_SRC_EXIST | MF_ERROR_CHECK_DST_PATH_VALID |
	             MF_ERROR_CHECK_SRC_PATH_VALID | MF_ERROR_CHECK_SRC_PATH_VALID | MF_ERROR_CHECK_DST_PARENT_DIR_EXIST | MF_ERROR_CHECK_DUPLICATED;
	int ret = mf_fs_oper_error(src, dst, option);

	if (ret != 0) {
		return ret;
	}

	mf_debug("src is %s\ndst is %s\n", src, dst);
	if (rename(src, dst)) {
		return MYFILE_ERR_RENAME_FAIL;
	} else {
		return MYFILE_ERR_NONE;
	}
}


