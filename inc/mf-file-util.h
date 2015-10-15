/*
* Copyright   2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef __MF_FILE_UTIL_H__
#define __MF_FILE_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <Elementary.h>

const char* mf_file_get(const char path[]);
char* mf_dir_get(const char path[]);
int mf_file_exists(const char *path);
Eina_Bool mf_is_dir(const char *path);
int mf_is_dir_empty(const char *path);
int mf_mkdir(const char *dir);
int mf_mkpath(const char *path);
char *mf_strip_ext(const char *path);
int mf_file_unlink (const char *filename);
int mf_file_size(const char *filename);
int mf_file_rmdir(const char *filename);
Eina_List *mf_file_ls(const char *dir);
int mf_file_recursive_rm(const char *dir);
int mf_file_cp(const char *src,const char *dst);
int mf_file_mv(const char *src, const char *dst);
int mf_remove(const char *filename);

#endif //__mf_FILE_UTIL_H__
