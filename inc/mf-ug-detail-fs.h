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

#ifndef __DEF_MF_UG_DETAIL_FS_H_
#define __DEF_MF_UG_DETAIL_FS_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <Eina.h>
#include <glib.h>
#include <tzplatform_config.h>

#include <Elementary.h>
#include <utils_i18n.h>
#include "mf-ug-detail-dlog.h"
#include "mf-ug-detail-view.h"
#include "mf-fs-util.h"
#define UG_MYFILE_DIR_PATH_LEN_MAX		4096
#define UG_MYFILE_FILE_NAME_LEN_MAX	256
#define UG_FILE_EXT_LEN_MAX		32
#define UG_FILE_SIZE_LEN_MAX		64
#define UG_FILE_CREATE_DATE_MAX		128
#define UG_NOMAL_BUF			128
#define UG_MYFILE_CHILDPATH_LEN		512
#define UG_PHONE_FOLDER			get_path_external(STORAGE_TYPE_INTERNAL)
#define UG_MEMORY_FOLDER		get_path_external(STORAGE_TYPE_EXTERNAL)
#define UG_MEMORY_DEV_FOLDER		"/dev/mmcblk1p1"

#define UG_MF_ERROR_MASKL16		0xFFFF
#define UG_MID_CONTENTS_MGR_ERROR		0	/*MID_CONTENTS_MGR_SERVICE*/
#define UG_MF_ERROR_CHECK_SRC_ARG_VALID	0x0001
#define UG_MF_ERROR_CHECK_SRC_EXIST	0x0004
#define UG_MF_ERROR_CHECK_SRC_PATH_VALID	0x0010

#define UG_MF_ERROR_SET(X)			(X & MF_ERROR_MASKL16)
/**< No error */
#define UG_MYFILE_ERR_NONE   \
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x00))
/**< invalid src argument */
#define UG_MYFILE_ERR_SRC_ARG_INVALID \
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x01))
/**< get stat failed */
#define UG_MYFILE_ERR_GET_STAT_FAIL		\
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x02))
/**< storage type error */
#define UG_MYFILE_ERR_STORAGE_TYPE_ERROR		\
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x03))
/**< get file category failed */
#define UG_MYFILE_ERR_GET_CATEGORY_FAIL		 \
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x04))

/**< exception of invalid file name */
#define UG_MYFILE_ERR_INVALID_FILE_NAME \
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x06))
/**< get ext type failed */
#define UG_MYFILE_ERR_EXT_GET_ERROR		\
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x07))
/**< exception of dir open*/
#define UG_MYFILE_ERR_DIR_OPEN_FAIL     \
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x08))
/**< exception of dir full */
#define UG_MYFILE_ERR_DIR_FULL          \
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x09))

#define UG_MYFILE_ERR_RETURN_VAL        \
	(UG_MID_CONTENTS_MGR_ERROR - UG_MF_ERROR_SET(0x0a))

#include "mf-fs-util.h"
#define File_Type fsFileType

typedef enum __Mf_Storage Mf_Storage;
enum __Mf_Storage {
	D_MYFILE_NONE,
	D_MYFILE_PHONE,
	D_MYFILE_MMC,
	D_MYFILE_MAX
};

typedef struct __Node_Info Node_Info;
struct __Node_Info {
	char path[UG_MYFILE_DIR_PATH_LEN_MAX];
	char name[UG_MYFILE_FILE_NAME_LEN_MAX];
	i18n_udate date;
	File_Type type;
	char ext[UG_FILE_EXT_LEN_MAX];
	LONG_LONG_UNSIGNED_INT size;
};

int mf_ug_detail_fs_get_store_type(const char *filepath, Mf_Storage *store_type);

int mf_ug_detail_fs_is_dir(const char *filepath);

int mf_ug_detaill_fs_get_file_stat(const char *filename, Node_Info **node);

int mf_ug_detail_fs_get_file_type(const char *filepath, File_Type *category);

int mf_ug_detail_fs_get_logi_path(const char *full_path, char *path);

GString *mf_ug_detail_fs_parse_file_type(GString *path);

int mf_ug_detail_fs_get_list_len(const Eina_List *list);

View_Style mf_ug_detail_fs_get_view_type(char *path, GString *type);

GString *mf_ug_detail_fs_get_parent(char *fullpath);

int mf_ug_detail_fs_get_file_list(GString *folder_name, Eina_List **dir_list, Eina_List **file_list);

LONG_LONG_UNSIGNED_INT mf_ug_detail_fs_get_folder_size(char *path);

int mf_ug_detail_fs_check_path(void *data, char *path);
File_Type mf_ug_detail_fs_get_category_by_mime(const char *mime);
int  mf_ug_detail_fs_get_file_ext(const char *filepath, char *file_ext);

#endif
