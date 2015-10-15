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

#ifndef __DEF_MYFILE_FILEMANAGERSERVICE_WRAPPER_H_
#define __DEF_MYFILE_FILEMANAGERSERVICE_WRAPPER_H_

#include <stdio.h>
#include <glib.h>
#include "mf-main.h"
#include "mf-conf.h"

#define MF_TRANSLATE_LENGTH	40
#define MF_TRANSLATE_OMIT_PART	"/.../"
#define MF_PATH_INFO_MAX_LENGTH_PORTRAIT	35
#define MF_PATH_INFO_MAX_LENGTH_LANDSCAPE	55

typedef enum _TRANS_OPTION	MF_TRANS_OPTION;

enum _TRANS_OPTION {
	MF_TRANS_OPTION_NONE = 0,
	MF_TRANS_OPTION_POPUP,
	MF_TRANS_OPTION_LABEL,
	MF_TRANS_OPTION_MAX,
} ;

/* basis file managing. create/rename/move/copy/rename */
int mf_fm_svc_wrapper_create_service(void *data, char *fullpath);
int mf_fm_svc_wrapper_rename_service(void *data, GString *from_fullpath, GString *to_name);


int mf_fm_svc_wrapper_get_file_list(const char *folder_name, Eina_List **dir_list, Eina_List **file_list);
int mf_fm_svc_wrapper_classify_dir_list(Eina_List *dir_list, Eina_List **default_dir_list, Eina_List **user_dir_list);

char *mf_fm_svc_wrapper_get_root_path_by_tab_label(const char *label);
/* file information get/set */
GString *mf_fm_svc_wrapper_get_file_name(GString *path);
int mf_fm_svc_wrapper_get_location(const char *fullpath);
unsigned long mf_fm_svc_wrapper_get_free_space(int state);
gint mf_fm_svc_wrapper_get_folder_foldersystem(GString *path, bool *result);
gboolean mf_fm_svc_wrapper_is_dir(GString *path);
bool mf_fm_svc_wrapper_detect_duplication(GString *to);
GString *mf_fm_svc_wrapper_get_file_parent_path(GString *fullpath);
int mf_fm_svc_wrapper_file_auto_rename(void *data, GString *fullpath, int file_name_type, GString **filename);
int mf_fm_svc_wrapper_detect_recursion(GString *from, GString *to);
int mf_fm_svc_wrapper_is_root_path(const char *fullpath);

char *mf_fm_svc_wrapper_translate_path(const char *original_path, MF_TRANS_OPTION option);
gboolean mf_fm_svc_is_mass_storage_on();
char *mf_fm_svc_path_info_get(const char *original_path);
char *mf_fm_svc_path_info_translate(char *path_info, int path_info_max_len);
char *mf_fm_svc_get_file_name(GString *path);
char *mf_fm_svc_get_file_name_without_ext(const char *name);
int mf_fm_svc_wrapper_create_p(const char *fullpath);
char *mf_fm_svc_wrapper_get_root_path_by_location(int location);
Eina_List *mf_fm_svc_wrapper_level_path_get(const char *original_path, int view_type);

#endif //__DEF_MYFILE_FILEMANAGERSERVICE_WRAPPER_H_
