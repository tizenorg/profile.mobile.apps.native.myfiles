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

#include <pthread.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <storage.h>

#include "mf-util.h"
#include "mf-callback.h"
#include "mf-object-conf.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-inotify-handle.h"
#include "mf-media-content.h"
#include "mf-resource.h"
#include "mf-fs-monitor.h"
#include "mf-file-util.h"

#define MF_PATH_INFO_RETRENCH			128
#define MF_PATH_INFO_HEAD_LEN(x)		strlen(x)
#define MF_PATH_INFO_TRANS_OMIT			elm_entry_utf8_to_markup("..")
#define	MF_PATH_INFO_LEVEL_BOUNDARY		3
#define MF_PATH_INFO_LEN_THRESHOLD		4
#define MF_PATH_INFO_SEP			elm_entry_utf8_to_markup("/")
typedef struct {
	int len_orig;
	int len_trans;
	char *original;
	char *transfer;
	bool flag_trans;
} pNode;

static int __mf_fm_svc_wrapper_get_unique_name(const char *default_dir_full_path, char *original_file_name,
        char **unique_file_name, int file_name_type, void *data);
/*********************
**Function name:	__mf_fm_svc_wrapper_COMESFROM
**Parameter:
**	GString* fullpath:	fullpath to check the location
**
**Return value:
**	location of the path
**
**Action:
**	get storage type by fullpath
*********************/
static int __mf_fm_svc_wrapper_COMESFROM(const char *fullpath)
{

	int len_phone = strlen(PHONE_FOLDER);
	int len_memory = strlen(MEMORY_FOLDER);

	if (strncmp(fullpath, PHONE_FOLDER, len_phone) == 0) {
		return MYFILE_PHONE;
	} else if (strncmp(fullpath, MEMORY_FOLDER, len_memory) == 0) {
		return MYFILE_MMC;
	} else {
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
}

/*********************
**Function name:	mf_fm_svc_wrapper_get_location
**Parameter:
**	GString* fullpath:	fullpath to check the location
**
**Return value:
**	location of the path
**
**Action:
**	get storage type by fullpath
*********************/

int mf_fm_svc_wrapper_get_location(const char *fullpath)
{
	return __mf_fm_svc_wrapper_COMESFROM(fullpath);
}

/*********************
**Function name:	mf_fm_svc_wrapper_detect_duplication
**Parameter:
**	GString* fullpath:	fullpath to check the duplication
**
**Return value:
**	TRUE	if duplication detected
**	FALSE	if duplication not detected
**
**Action:
**	check if the destination is duplicated
*********************/
bool mf_fm_svc_wrapper_detect_duplication(GString *to)
{
	int existing = MYFILE_ERR_NONE;
	if (to == NULL) {
		return false;
	}
	GString *parent_path = mf_fm_svc_wrapper_get_file_parent_path(to);
	GString *file_name = mf_fm_svc_wrapper_get_file_name(to);

	SECURE_DEBUG("full path and file name %s", to->str);
	if (file_name == NULL || parent_path == NULL || file_name->len == 0) {
		return false;
	}

	if (parent_path->str != NULL) {
		mf_debug("parent_path->str is %s", parent_path->str);
	}
	if (file_name->str != NULL) {
		SECURE_DEBUG("file_name->str is %s", file_name->str);
	}

	existing = mf_file_attr_is_duplicated_name(parent_path->str, file_name->str);

	mf_debug("EXIST result is %d", existing);

	if (parent_path != NULL) {
		g_string_free(parent_path, TRUE);
	}
	parent_path = NULL;

	if (file_name != NULL) {
		g_string_free(file_name, TRUE);
	}
	file_name = NULL;

	if (existing == MYFILE_ERR_NONE) {
		return false;
	} else {
		return true;
	}
}

/*********************
**Function name:	mf_fm_svc_wrapper_detect_recursion
**Parameter:
**	GString* from:	the dir to check
**	GString* to:	the dir to operate
**
**Return value:
**	error code
**
**Action:
**	check if the to path is recursive with the from path
*********************/
int mf_fm_svc_wrapper_detect_recursion(GString *from, GString *to)
{
	/* recurion detection */
	int ret = 0;
	mf_debug();
	int lensrc = from->len;
	int lendst = to->len;

	if (lensrc == 0 || lendst == 0 || lensrc > lendst) {
		return MYFILE_REPORT_NONE;
	}
	MF_STORAGE from_store_type = MYFILE_NONE;
	MF_STORAGE to_store_type = MYFILE_NONE;

	if ((ret = mf_file_attr_get_store_type_by_full(from->str, &from_store_type)) != 0) {
		return ret;
	}
	if ((ret = mf_file_attr_get_store_type_by_full(to->str, &to_store_type)) != 0) {
		return ret;
	}

	if (from_store_type != to_store_type) {
		return MYFILE_REPORT_NONE;
	}

	if (g_string_equal(from, to)) {
		return MYFILE_REPORT_BOTH_ARE_SAME_FILE;
	}

	GString *from_parent = mf_fm_svc_wrapper_get_file_parent_path(from);
	if (from_parent == NULL) {
		return ret;
	}

	GString *to_parent = mf_fm_svc_wrapper_get_file_parent_path(to);
	if (to_parent == NULL) {
		SAFE_FREE_GSTRING(from_parent);
		return ret;
	}
	if (strncmp(from->str, to_parent->str, lensrc) == 0) {
		SAFE_FREE_GSTRING(from_parent);
		SAFE_FREE_GSTRING(to_parent);
		return MYFILE_REPORT_RECURSION_DETECT;
	}

	SAFE_FREE_GSTRING(from_parent);
	SAFE_FREE_GSTRING(to_parent);
	return MYFILE_REPORT_NONE;
}

char *mf_fm_svc_wrapper_get_root_path_by_location(int location)
{
	if (location == MYFILE_PHONE) {
		return g_strdup(PHONE_FOLDER);
	} else if (location == MYFILE_MMC) {
		return g_strdup(MEMORY_FOLDER);
	} else {
		return NULL;
	}
}

/*********************
**Function name:	mf_fm_svc_wrapper_file_auto_rename
**Parameter:
**	GString* from:	the dir to check
**	GString* to:	the dir to operate
**
**Return value:
**	error code
**
**Action:
**	check if the to path is recursive with the from path
*********************/
int mf_fm_svc_wrapper_file_auto_rename(void *data, GString *fullpath, int file_name_type, GString **filename)
{
	struct appdata *ap = (struct appdata *)data;
	if (ap == NULL) {
		mf_debug("appdata is NULL");
	}
	assert(ap);

	GString *parent_path = mf_fm_svc_wrapper_get_file_parent_path(fullpath);
	GString *file_name = mf_fm_svc_wrapper_get_file_name(fullpath);

	if (parent_path == NULL || file_name == NULL) {
		return MYFILE_ERR_GENERATE_NAME_FAIL;
	}
	if (parent_path->str == NULL || file_name->str == NULL) {
		g_string_free(parent_path, TRUE);
		parent_path = NULL;
		g_string_free(file_name, TRUE);
		file_name = NULL;
		return MYFILE_ERR_GENERATE_NAME_FAIL;
	}

	char *name = NULL;
	int error_code = 0;

	if (parent_path->str != NULL) {
		//mf_debug("parent_full_path is [%s]", parent_path->str);
	}

	if (file_name->str != NULL) {
		//mf_debug("original_file_name is [%s]", file_name->str);
	}
	error_code = __mf_fm_svc_wrapper_get_unique_name(parent_path->str, file_name->str, &name, file_name_type, ap);
	if (error_code) {
		SAFE_FREE_CHAR(name);
		return MYFILE_ERR_GENERATE_NAME_FAIL;
	}
	g_string_append_printf(parent_path, "/%s", name);
	mf_debug("After gstring append, PATH ::: [%s]", parent_path->str);

	if (file_name != NULL) {
		g_string_free(file_name, TRUE);
	}

	file_name = NULL;
	if (name != NULL) {
		free(name);
		name = NULL;
	}

	*filename = parent_path;
	return MYFILE_ERR_NONE;
}

/*********************
**Function name:	mf_fm_svc_wrapper_get_folder_foldersystem
**Parameter:
**	GString* path:
**		path need to check
**	bool *result:
**		output result
**
**Return value:
**	error code
**
**Action:
**	check if the to path is system folder
*********************/
gint mf_fm_svc_wrapper_get_folder_foldersystem(GString *path, bool *result)
{

	int error_code = 0;
	mf_debug("Start");
	/*ToDo: How to tell if the folder is System folder? */
	error_code = mf_file_attr_is_system_dir(path->str, result);
	mf_debug("nerror_code is %d\nresult is %d", error_code, *result);
	return error_code;

}

/*********************
**Function name:	mf_fm_svc_wrapper_get_file_list
**Parameter:
**	GString* folder_name:
**		path to read
**	Eina_List** dir_list:
**		output value of dir element
**	Eina_List** file_list
**		output value of file element
**
**Return value:
**	error code
**
**Action:
**	read elements under the specified dir
*********************/
int mf_fm_svc_wrapper_get_file_list(const char *folder_name, Eina_List **dir_list, Eina_List **file_list)
{
	mf_debug("Start");
	int error_code = 0;

	mf_retvm_if(folder_name == NULL, MYFILE_ERR_INVALID_PATH, "folder_name is NULL");
	error_code = mf_fs_oper_read_dir(folder_name, dir_list, file_list);
	if (error_code != 0) {
		mf_error("error_code is [%d]\n", error_code);
	} else {
		mf_error("success get the file list\n");
	}

	return error_code;
}

/******************************
** Prototype    : mf_fm_svc_wrapper_classify_dir_list
** Description  : classify the dir list into default and user defined
** Input        : Eina_List *dir_list
**                Eina_List **default_dir_list
**                Eina_List **user_dir_list
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
int mf_fm_svc_wrapper_classify_dir_list(Eina_List *dir_list, Eina_List **default_dir_list, Eina_List **user_dir_list)
{
	int error_code = MYFILE_ERR_NONE;
	fsNodeInfo *pNode = NULL;
	Eina_List *l = NULL;

	EINA_LIST_FOREACH(dir_list, l, pNode) {
		if (pNode) {
			char *real_name = NULL;
			bool result = false;
			GString *foldername = NULL;
			real_name = g_strconcat(pNode->path, "/", pNode->name, NULL);

			if (real_name) {
				foldername = g_string_new(real_name);
				SAFE_FREE_CHAR(real_name);
			} else {
				continue;
			}
			int error_code = mf_fm_svc_wrapper_get_folder_foldersystem(foldername, &result);

			if (error_code == 0 && result == true) {
				mf_debug("in default");
				*default_dir_list = eina_list_append(*default_dir_list, pNode);
			} else {
				mf_debug("in else");
				*user_dir_list = eina_list_append(*user_dir_list, pNode);
			}
		}
	}
	return error_code;
}


/*********************
**Function name:	mf_fm_svc_wrapper_is_root_path
**Parameter:
**	void *data:
**		global variable to keep status
**
**Return value:
**	Non-Zero if it's the root path
**	MYFILE_NONE if not
**
**Action:
**	check if current path is root path
*********************/
int mf_fm_svc_wrapper_is_root_path(const char *fullpath)
{
	/* assert(fullpath); */
	if (fullpath == NULL) {
		mf_debug("fullpath is NULL");
		return MYFILE_NONE;
	}

	mf_debug("path is [%s]", fullpath);

	if (!g_strcmp0(fullpath, PHONE_FOLDER)) {
		mf_debug("PHONE_FOLDER");
		return MYFILE_PHONE;
	} else if (!g_strcmp0(fullpath, MEMORY_FOLDER)) {
		mf_debug("MEMORY_FOLDER");
		return MYFILE_MMC;
	} else {
		mf_debug("Not root path");
		return MYFILE_NONE;
	}
}

/*********************
**Function name:	mf_fm_svc_wrapper_is_dir
**Parameter:
**	GString* path:
**		full path of the file
**
**Return value:
**	TRUE	if path is a directory
**	FALSE	if not
**
**Action:
**	check if the path is a directory
*********************/
gboolean mf_fm_svc_wrapper_is_dir(GString *path)
{
	if (!path || !path->str) {
		return FALSE;
	}

	return mf_file_attr_is_dir(path->str);
}


/*********************
**Function name:	mf_fm_svc_wrapper_get_file_name
**Parameter:
**	GString* path:
**		full path of the file
**
**Return value:
**	file name
**
**Action:
**	get file name from full path
*********************/
GString *mf_fm_svc_wrapper_get_file_name(GString *path)
{
	if (!path || !path->str) {
		return NULL;
	}

	GString *ret = g_string_new(mf_file_get(path->str));
	return ret;
}


/*********************
**Function name:	mf_fm_svc_wrapper_get_file_parent_path
**Parameter:
**	GString* path:
**		full path of the file
**
**Return value:
**	parent path of the current path
**
**Action:
**	get parent path from the full path
*********************/
GString *mf_fm_svc_wrapper_get_file_parent_path(GString *fullpath)
{
	GString *ret = NULL;
	char *path = NULL;
	int error_code = 0;

	if (fullpath == NULL || fullpath->str == NULL) {
		return NULL;
	}
	error_code = mf_file_attr_get_parent_path(fullpath->str, &path);
	if (error_code != 0) {
		return NULL;
	}

	ret = g_string_new(path);
	SAFE_FREE_CHAR(path);
	return ret;
}

/*********************
**Function name:	mf_fm_svc_wrapper_get_free_space
**Parameter:
**	int state:
**		storage to check
**
**Return value:
**	free space on the storage
**
**Action:
**	get free space on the storage
*********************/
unsigned long mf_fm_svc_wrapper_get_free_space(int state)
{
	struct statvfs info;
	char *path = NULL;

	if (state == MYFILE_PHONE) {
		if (storage_get_internal_memory_size(&info) < 0) {
			return 0;
		}
	} else if (state == MYFILE_MMC) {
		path = MEMORY_FOLDER;
		if (-1 == statvfs(path, &info)) {
			return 0;
		}
	} else {
		return 0;
	}

	mf_error("free space = [%ld]", info.f_bavail);
	return (info.f_bsize) * info.f_bavail;
}

/*********************
**Function name:	mf_fm_svc_wrapper_create_service
**Parameter:
**	void *data:
**		global variable to store data
**	GString* fullpath
**		the path to create
**
**Return value:
**	error code
**
**Action:
**	create the specified path
*********************/
int mf_fm_svc_wrapper_create_service(void *data, char *fullpath)
{
	int error_code;
	if (!fullpath) {
		return MYFILE_ERR_INVALID_FILE_PATH;
	}

	mf_fs_monitor_remove_dir_watch();
	error_code = mf_fs_oper_create_dir(fullpath);

	if (error_code != 0) {
		mf_debug("Make DIR error\n");
	} else {
		mf_media_content_scan_folder(fullpath);
	}

	return error_code;
}


/*********************
**Function name:	mf_fm_svc_wrapper_rename_service
**Parameter:
**	void *data:
**		global variable to store data
**	GString* from:
**		source file
**	GString* to:
**		destination file
**
**Return value:
**	error code
**
**Action:
**	rename the specified file to the destination
*********************/
int mf_fm_svc_wrapper_rename_service(void *data, GString *from_fullpath, GString *to)
{
	struct appdata *ap = (struct appdata *)data;
	assert(ap);
	int error_code = 0;

	SECURE_ERROR("from_fullpath is [%s] to is [%s]", from_fullpath->str, to->str);

	error_code = mf_fs_oper_rename_file(from_fullpath->str, to->str);

	if (error_code != 0) {
		mf_debug("rename failed	%d\n", error_code);
	} else {
		sync();
		if (mf_fm_svc_wrapper_is_dir(to)) {
			mf_media_content_scan_folder(from_fullpath->str);
			mf_media_content_scan_folder(to->str);
		}
	}

	return error_code;
}

/*********************
**Function name:	__mf_fm_svc_wrapper_get_next_number
**Parameter:
**	char *file_name_without_ext:
**		file name
**	int file_name_type:
**		file name type
**
**Return value:
**	error code
**
**Action:
**	get the next file number
*********************/
static int __mf_fm_svc_wrapper_get_next_number(char *file_name_without_ext, int file_name_type)
{
	int nCount = 0;
	int nLength = 0;
	int nUnderline = 0;
	bool bAllDigits = true;
	int i;

	/* check _02d format */
	nLength = strlen(file_name_without_ext);

	if (file_name_type == FILE_NAME_WITH_UNDERLINE) {
		if (nLength < 3) {	/*4 means the # of minimum characters (*_n) */
			return 1;	/*doesn't match */
		} else {	/* input is more than 3 bytes */
			/* find '_' */
			for (nUnderline = nLength - 1; nUnderline >= 0; nUnderline--) {
				if (file_name_without_ext[nUnderline] == '_') {
					break;
				}
			}

			if (nUnderline == 0 && file_name_without_ext[0] != '_') {
				return 1;	/* doesn't match */
			}
			/* check the right characters are all digits */
			for (i = nUnderline + 1; i < nLength; i++) {
				if (file_name_without_ext[i] < '0' || file_name_without_ext[i] > '9') {
					bAllDigits = false;
					break;
				}
			}

			if (bAllDigits) {
				for (i = nUnderline + 1; i < nLength; i++) {
					nCount *= 10;
					nCount += file_name_without_ext[i] - '0';
				}

				file_name_without_ext[nUnderline] = '\0';	/* truncate the last  '_dd' */
			}
		}
	} else {

		if (nLength < 5) {	/* 5 means the # of minimum characters (*_(n)) */
			return 1;	/*doesn't match */
		} else {	/* input is more than 3 bytes */
			/* find '_' */
			for (nUnderline = nLength - 1; nUnderline >= 0; nUnderline--) {
				if (file_name_without_ext[nUnderline] == '(') {
					break;
				}
			}

			if (nUnderline == 0 && file_name_without_ext[0] != '(') {
				return 1;	/* doesn't match */
			}
			/* check the right characters are all digits */
			for (i = nUnderline + 1; i < nLength - 1; i++) {
				if (file_name_without_ext[i] < '0' || file_name_without_ext[i] > '9') {
					bAllDigits = false;
					break;
				}
			}

			/* and more than 2 columns. */
			if (bAllDigits) {
				for (i = nUnderline + 1; i < nLength - 1; i++) {
					nCount *= 10;
					nCount += file_name_without_ext[i] - '0';
				}

				file_name_without_ext[nUnderline] = '\0';	/* truncate the last  '_dd' */
			}
		}
	}

	/* increase nCount by 1 */
	nCount++;

	return nCount;
}


char *mf_fm_svc_path_info_retrench(const char *string)
{
	mf_retvm_if(string == NULL, g_strdup(MF_PATH_INFO_TRANS_OMIT), "input path is NULL");
	char *retrench = NULL;
	char *utf8_string = elm_entry_utf8_to_markup(string);
	if (utf8_string && strlen(string) > MF_PATH_INFO_LEN_THRESHOLD) {
		if (g_utf8_strlen(utf8_string, -1) > 2) {
			retrench = calloc(1, MF_PATH_INFO_RETRENCH);
			if (retrench) {
				char *omit = MF_PATH_INFO_TRANS_OMIT;
				char *temp = g_utf8_strncpy(retrench, utf8_string, 2);
				retrench = g_strconcat(temp, omit, NULL);
				SAFE_FREE_CHAR(omit);
				SAFE_FREE_CHAR(temp);
			}
			SAFE_FREE_CHAR(utf8_string);

		} else {
			retrench = utf8_string;
		}
		return retrench;
	} else {
		return utf8_string;
	}
}

static void __mf_fm_svc_wrapper_path_info_node_free(Eina_List *list)
{
	mf_retm_if(list == NULL, "list is NULL");
	const Eina_List *l = NULL;
	void *data = NULL;
	EINA_LIST_FOREACH(list, l, data) {
		pNode *node = (pNode *)data;
		if (node != NULL) {
			SAFE_FREE_CHAR(node->original);
			SAFE_FREE_CHAR(node->transfer);
			SAFE_FREE_CHAR(node);
		}
	}
	eina_list_free(list);
}

char *mf_fm_svc_path_info_translate(char *path_info, int path_info_max_len)
{

	mf_retvm_if(path_info == NULL, g_strdup(dgettext(MYFILE_STRING_PACKAGE, "IDS_COM_BODY_UNKNOWN")), "input path is NULL");

	int top = 0;
	bool flag = TRUE;
	Eina_List *temp_list = NULL;
	const Eina_List *l = NULL;
	gchar **result = NULL;
	gchar **params = NULL;
	int count = 0;
	int max_len = 0;
	int total_len = 0;
	int i = 0;
	char *output = NULL;
	void *pnode = NULL;
	char *omit = MF_PATH_INFO_TRANS_OMIT;

	if (strlen(path_info) < path_info_max_len) {
		SAFE_FREE_CHAR(omit);
		return path_info;
	}

	result = g_strsplit(path_info, "/", 0);
	if (result == NULL) {
		SAFE_FREE_CHAR(path_info);
		SAFE_FREE_CHAR(omit);
		return g_strdup(dgettext(MYFILE_STRING_PACKAGE, "IDS_COM_BODY_UNKNOWN"));
	}

	params = result;
	count = g_strv_length(result);

	if (count > MF_PATH_INFO_LEVEL_BOUNDARY) {
		top = MF_PATH_INFO_LEVEL_BOUNDARY;
		flag = FALSE;
		max_len = path_info_max_len - MF_PATH_INFO_LEVEL_BOUNDARY - MF_PATH_INFO_HEAD_LEN(omit);//(2 is length of ..) ../aa../bb../***
	} else {
		top = count;
		flag = TRUE;
		max_len = path_info_max_len - (count - 1);
	}

	for (i = top; i > 1; i--) {
		pNode *nodeB = calloc(sizeof(pNode), 1);
		if (nodeB) {
			nodeB->original = elm_entry_utf8_to_markup(params[count - i]);
			nodeB->len_orig = strlen(params[count - i]);
			nodeB->transfer = mf_fm_svc_path_info_retrench(params[count - i]);
			if (nodeB->transfer) {
				nodeB->len_trans = strlen(nodeB->transfer);
			} else {
				mf_error("nodeB->transfer is NULL");
			}
			nodeB->flag_trans = FALSE;
			total_len += nodeB->len_orig;

			temp_list = eina_list_append(temp_list, nodeB);
		}
	}

	total_len += strlen(params[count - 1]);

	for (i = 0 ; i < eina_list_count(temp_list); i++) {
		if (total_len > max_len) {
			pNode *data = NULL;
			data = eina_list_nth(temp_list, i);
			if (data != NULL) {
				total_len -= (data->len_orig - data->len_trans);
				data->flag_trans = TRUE;
			}
		}

		if (total_len <= max_len) {
			break;
		}
	}


	if (flag == FALSE) {
		output = elm_entry_utf8_to_markup("..");
	}
	char *temp = NULL;
	char *sep = MF_PATH_INFO_SEP;
	EINA_LIST_FOREACH(temp_list, l, pnode) {
		if (pnode) {
			pNode *node = (pNode *)pnode;
			temp = output;
			if (node->flag_trans == TRUE) {
				if (output != NULL) {
					output = g_strconcat(output, sep, node->transfer, NULL);
				} else {
					output = g_strdup(node->transfer);
				}
			} else {
				if (output != NULL) {
					output = g_strconcat(output, sep , node->original, NULL);
				} else {
					output = g_strdup(node->original);
				}
			}
			SAFE_FREE_CHAR(temp);
		}
	}
	temp = output;
	char *last_string = params[count - 1];
	char *utf8_last = elm_entry_utf8_to_markup(last_string);

	if (output != NULL) {
		int last_len = strlen(last_string);
		int output_len = strlen(output);
		int d_value = path_info_max_len - output_len;
		if ((last_len + output_len) > path_info_max_len) {
			mf_debug();

			const char *end = NULL;
			gboolean ret = FALSE;
			if (utf8_last != NULL) {
				ret = g_utf8_validate(utf8_last, d_value, &end);
			} else {
				mf_error("utf8_last is NULL");
			}
			if (ret == TRUE) {
				d_value = last_len - strlen(end);
				utf8_last[d_value] = '\0';
				output = g_strconcat(output, sep, utf8_last, omit, NULL);
				SAFE_FREE_CHAR(temp);
			}
		} else {
			output = g_strconcat(output, sep, utf8_last, NULL);
			SAFE_FREE_CHAR(temp);
		}
	} else {
		output = g_strdup(utf8_last);
		SAFE_FREE_CHAR(temp);
	}
	SAFE_FREE_CHAR(utf8_last);

	SAFE_FREE_CHAR(sep);
	SAFE_FREE_CHAR(omit);
	SAFE_FREE_CHAR(path_info);
	__mf_fm_svc_wrapper_path_info_node_free(temp_list);
	temp_list = NULL;
	g_strfreev(result);
	result = NULL;
	return output;
}

/*********************
**Function name:	__mf_fm_svc_wrapper_get_unique_name
**Parameter:
**	const char *default_dir_full_path
**	char *original_file_name,
**	char *unique_file_name,
**	int file_name_type,
**	void* data
**
**Return value:
**	error code
**
**Action:
**	get the unique name of the file name
*********************/
static int __mf_fm_svc_wrapper_get_unique_name(const char *default_dir_full_path, char *original_file_name, char **unique_file_name,
        int file_name_type, void *data)
{
	//mf_debug("%s %d\n", __func__, __LINE__);
	assert(unique_file_name);
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	//Eina_List *l = NULL;
	//GString *content = NULL;
	//const char *file_name = NULL;

	char *file_name_without_ext = NULL;
	char *file_ext = NULL;
	char *new_file_name = NULL;
	bool result = false;
	char *dir_rel_path = NULL;
	int nCount = 0;
	bool bExt = false;
	int error_code = 0;

	if (default_dir_full_path == NULL || original_file_name == NULL) {
		MYFILE_TRACE_DEBUG("default_dir_full_path == NULL || \
						original_file_name == NULL ||   \
						unique_file_name == NULL || \
						error_code == NULL ");
		error_code =  MYFILE_ERR_SRC_ARG_INVALID;
		goto Exception;
	}
	result = mf_file_attr_get_logical_path_by_full(default_dir_full_path, &dir_rel_path);

	if (result) {
		error_code = MYFILE_ERR_GET_LOGIC_PATH_FAIL;
		goto Exception;
	}

#ifdef MYFILE_CHECK_DIR_FILE_PATH_MAXIMUM_LENGTH
	int slash = 1;
	if (strncmp(dir_rel_path, "/", strlen(dir_rel_path)) == 0) {
		slash = 0;
	}
	if (mf_util_character_count_get(dir_rel_path) + mf_util_charactor_count_get(original_file_name) + slash > MYFILE_FILE_PATH_LEN_MAX) {
		SECURE_DEBUG("......(%s/%s) exceeds maximum length: %d...", dir_rel_path, original_file_name, MYFILE_FILE_PATH_LEN_MAX);
		error_code = MYFILE_ERR_EXCEED_MAX_LENGTH;
		goto Exception;
	}
#endif
	error_code = mf_file_attr_is_duplicated_name(default_dir_full_path, original_file_name);
	if (error_code == 0) {
		SECURE_DEBUG("unique_file_name [%s]", *unique_file_name);
		SECURE_DEBUG("original_file_name [%s]", new_file_name);
		*unique_file_name = g_strdup(original_file_name);
		SECURE_DEBUG("unique_file_name [%s]", *unique_file_name);
	}

	while (error_code < 0) {
		error_code = 0;
		bExt = mf_file_attr_get_file_ext(original_file_name, &file_ext);
		file_name_without_ext = g_strdup(original_file_name);

		if (file_name_without_ext == NULL) {
			error_code = MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
			goto Exception;
		}

		/* add a condition, whether extention is or not. */
		if (bExt == 0) {
			file_name_without_ext[strlen(file_name_without_ext) - strlen(file_ext) - 1] = '\0';
		}

		nCount = __mf_fm_svc_wrapper_get_next_number(file_name_without_ext, file_name_type);
		if (nCount == 1 && file_name_type == FILE_NAME_WITH_BRACKETS) {
			char *file_name_with_space = g_strconcat(file_name_without_ext, " ", NULL);
			if (file_name_with_space) {
				SAFE_FREE_CHAR(file_name_without_ext);
				file_name_without_ext = file_name_with_space;
				file_name_with_space = NULL;
			}
		}

		if (bExt == 0) {
			if (file_name_type == FILE_NAME_WITH_BRACKETS) {
				new_file_name = g_strdup_printf("%s(%d).%s", file_name_without_ext, nCount, file_ext);
			} else {
				new_file_name = g_strdup_printf("%s_%d.%s", file_name_without_ext, nCount, file_ext);
			}
		} else {

			if (file_name_type == FILE_NAME_WITH_BRACKETS) {
				new_file_name = g_strdup_printf("%s(%d)", file_name_without_ext, nCount);
			} else {
				new_file_name = g_strdup_printf("%s_%d", file_name_without_ext, nCount);
			}
		}
		//mf_debug("new_file_name [%s]", new_file_name);
		//mf_debug("original_file_name [%s]", new_file_name);
		SAFE_FREE_CHAR(file_name_without_ext);

#ifdef MYFILE_CHECK_DIR_FILE_PATH_MAXIMUM_LENGTH
		if (mf_util_character_count_get(new_file_name) > MYFILE_FILE_NAME_LEN_MAX ||
		        mf_util_character_count_get(dir_rel_path) + slash + mf_util_charactor_count_get(new_file_name) > MYFILE_FILE_PATH_LEN_MAX) {
			SECURE_DEBUG("......(%s/%s) exceeds maximum length: %d...", dir_rel_path, new_file_name, MYFILE_FILE_PATH_LEN_MAX);
			error_code = MYFILE_ERR_EXCEED_MAX_LENGTH;
			goto Exception;
		}
#endif
		SECURE_DEBUG("new name is %s\n", new_file_name);
//prevent issue fix
		/*	if (error_code != 0) {
				original_file_name = g_strdup(new_file_name);
				error_code = MYFILE_ERR_DUPLICATED_NAME;
				SAFE_FREE_CHAR(new_file_name);
				SAFE_FREE_CHAR(file_ext);
				continue;
			} else*/
		{
			error_code = mf_file_attr_is_duplicated_name(default_dir_full_path, new_file_name);
			if (error_code == 0) {
				*unique_file_name = g_strdup(new_file_name);
				//mf_debug("rename finished\n");
				error_code =  MYFILE_ERR_NONE;
				goto Exception;
			} else {
				//mf_debug("rename continue\n");
				original_file_name = g_strdup(new_file_name);
				SAFE_FREE_CHAR(new_file_name);
			}
		}
		SAFE_FREE_CHAR(file_ext);
	}

	return MYFILE_ERR_NONE;

Exception:
	SAFE_FREE_CHAR(dir_rel_path);
	SAFE_FREE_CHAR(file_ext);
	SAFE_FREE_CHAR(new_file_name);
	return error_code;
}

char *mf_fm_svc_path_info_get(const char *original_path)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(original_path == NULL, g_strdup(dgettext(MYFILE_STRING_PACKAGE, "IDS_COM_BODY_UNKNOWN")), "input path is NULL");
	char *path_info = NULL;
	int len = 0;

	path_info = mf_fm_svc_wrapper_translate_path(original_path, MF_TRANS_OPTION_LABEL);
	if (path_info) {
		len = strlen(path_info);
		if (len > 0 && path_info[len - 1] == '/') {
			path_info[len - 1] = '\0';
		}
	}
	MF_TRACE_END;
	return path_info;

}

char *mf_fm_svc_wrapper_translate_path(const char *original_path, MF_TRANS_OPTION option)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(original_path == NULL, g_strdup(dgettext(MYFILE_STRING_PACKAGE, "IDS_COM_BODY_UNKNOWN")), "input path is NULL");

	char *new_path = NULL;
	int root_len = 0;

	if (mf_fm_svc_wrapper_get_location(original_path) == MYFILE_PHONE) {
		switch (option) {
		case MF_TRANS_OPTION_POPUP:
			if (strlen(original_path) > MF_TRANSLATE_LENGTH) {
				new_path =
				    g_strconcat(mf_util_get_text(MF_LABEL_DEVICE_MEMORY), MF_TRANSLATE_OMIT_PART, mf_file_get(original_path), "/",
				                NULL);
			} else {
				root_len = strlen(PHONE_FOLDER);
				new_path = g_strconcat(mf_util_get_text(MF_LABEL_DEVICE_MEMORY), original_path + root_len, "/", NULL);
			}
			break;
		case MF_TRANS_OPTION_LABEL:
			root_len = strlen(PHONE_FOLDER);
			new_path = g_strconcat(mf_util_get_text(MF_LABEL_DEVICE_MEMORY), original_path + root_len, "/", NULL);
			break;
		default:
			new_path = g_strdup(original_path);
			break;
		}


	} else if (mf_fm_svc_wrapper_get_location(original_path) == MYFILE_MMC) {
		switch (option) {
		case MF_TRANS_OPTION_POPUP:
			if (strlen(original_path) > MF_TRANSLATE_LENGTH) {
				new_path =
				    g_strconcat(mf_util_get_text(MF_LABEL_SD_CARD), MF_TRANSLATE_OMIT_PART, mf_file_get(original_path),
				                NULL);
			} else {
				root_len = strlen(MEMORY_FOLDER);
				new_path = g_strconcat(mf_util_get_text(MF_LABEL_SD_CARD), original_path + root_len, "/", NULL);
			}
			break;
		case MF_TRANS_OPTION_LABEL:
			root_len = strlen(MEMORY_FOLDER);
			new_path = g_strconcat(mf_util_get_text(MF_LABEL_SD_CARD), original_path + root_len, "/", NULL);
			break;
		default:
			new_path = g_strdup(original_path);
			break;
		}
	} else {
		new_path = g_strdup(original_path);
	}

	mf_debug("new path is %s", new_path);
	MF_TRACE_END;
	return new_path;
}

Eina_List *mf_fm_svc_wrapper_level_path_get(const char *original_path, int view_type)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(original_path == NULL, NULL, "input path is NULL");

	char *current_path = g_strdup(original_path);
	Eina_List *path_list = NULL;
	const char *root_path = NULL;
	gchar *path = current_path;

	mf_error("original_path is [%s]", original_path);
	int location = mf_fm_svc_wrapper_is_root_path(current_path);
	{
		mf_error();
		if (location == MYFILE_NONE) {
			location = mf_fm_svc_wrapper_get_location(current_path);
			mf_error("locations:%d", location);
			switch (location) {
			case MYFILE_PHONE:
				root_path = PHONE_FOLDER;
				break;
			case MYFILE_MMC:
				root_path = MEMORY_FOLDER;
				break;
			default:
				g_free(path);
				return NULL;
			}
			current_path = current_path + strlen(root_path) + 1;
			path_list = eina_list_append(path_list, g_strdup(root_path));
			gchar **result = NULL;
			gchar **params = NULL;
			result = g_strsplit(current_path, "/", 0);
			char *level_path = NULL;
			for (params = result; *params; params++) {
				if (level_path == NULL) {
					level_path = g_strconcat(root_path, "/", *params, NULL);
				} else {
					level_path = g_strconcat(level_path, "/", *params, NULL);
				}
				path_list = eina_list_append(path_list, level_path);
			}

			g_strfreev(result);

		} else {
			path_list = eina_list_append(path_list, g_strdup(original_path));
		}
	}
	MF_TRACE_END;
	g_free(path);
	return path_list;
}

char *mf_fm_svc_get_file_name_without_ext(const char *name)
{
	mf_retv_if(name == NULL, NULL);
	char *name_without_ext = NULL;
	char *guide_text = NULL;
	char *ext = NULL;

	name_without_ext = g_strdup(name);
	mf_file_attr_get_file_ext(name, &ext);
	mf_debug("ext is %s", ext);
	if (ext && strlen(ext) != 0) {
		name_without_ext[strlen(name_without_ext) - strlen(ext) - 1] = '\0';
		SECURE_DEBUG("name_without_ext is [%s]\n", name_without_ext);
		if (strlen(name_without_ext)) {
			guide_text = elm_entry_utf8_to_markup(mf_file_get(name_without_ext));
		} else {
			guide_text = elm_entry_utf8_to_markup(name);
		}
	} else {
		guide_text = elm_entry_utf8_to_markup(name);
	}
	SAFE_FREE_CHAR(ext);
	SAFE_FREE_CHAR(name_without_ext);
	return guide_text;
}

char *mf_fm_svc_get_file_name(GString *path)
{
	mf_retv_if(path == NULL, NULL);

	GString *filename = NULL;
	char *guide_text = NULL;
	filename = mf_fm_svc_wrapper_get_file_name(path);

	if (filename != NULL) {
		guide_text = mf_fm_svc_get_file_name_without_ext(filename->str);
		SAFE_FREE_GSTRING(filename);
	}
	return guide_text;
}

int mf_fm_svc_wrapper_create_p(const char *fullpath)
{
	MF_TRACE_BEGIN;

	mf_error("path is [%s]", fullpath);
	int error_code = MYFILE_ERR_NONE;

	if (mf_file_exists(fullpath)) {
		return error_code;
	}
	if (!mf_mkpath(fullpath)) {
		error_code = MYFILE_ERR_DIR_CREATE_FAIL;
		goto EXIT;
	}

EXIT:
	mf_error("error_code  = %d", error_code);
	return error_code;
}
