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

#ifndef _MF_SEARCH_INTERNAL_H_
#define _MF_SEARCH_INTERNAL_H_

#include "mf-search.h"

#ifdef MS_USE_DEF_LOG

#include "mf-dlog.h"

#define ms_debug(fmt , args...)  mf_debug
#define ms_info(fmt , args...)  mf_info
#define ms_warn(fmt , args...)  mf_warning
#define ms_error(fmt , args...)  mf_error
#define ms_assert(fmt , args...)  mf_assert

#else

#ifdef DEBUG_ON
#define ms_debug(fmt , args...)  do { printf("[%10s:%4d][D] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define ms_info(fmt , args...)  do { printf("[%10s:%4d][I] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define ms_warn(fmt , args...)  do { printf("[%10s:%4d][W] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define ms_error(fmt , args...)  do { printf("[%10s:%4d][E] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define ms_assert(fmt , args...)  do { printf("[%10s:%4d][A] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#else
#define ms_debug(fmt , args...)  do { (void)0; } while (0)
#define ms_info(fmt , args...)  do { (void)0; } while (0)
#define ms_warn(fmt , args...)  do { (void)0; } while (0)
#define ms_error(fmt , args...)  do { (void)0; } while (0)
#define ms_assert(fmt , args...)  do { (void)0; } while (0)
#endif
#endif

/**
 * Enumerations of search state
 **/

int _mf_search_init(ms_handle_t **handle);
int _mf_search_start(ms_handle_t *handle,
	const char **root_path,
	unsigned int path_num,
	const char *needle,
	mf_search_option option,
	void *user_data,
	mf_search_filter_cb func,
	int category,
	int item_count);
int _mf_search_stop(ms_handle_t *handle);
void _mf_search_finalize(ms_handle_t **handle);

/*+++++++++++++++++++++++++ UTIL APIs ++++++++++++++++++++++++++++++ */

gchar *_mf_search_result_dir_get(mf_search_result_t *result);
gchar *_mf_search_result_file_get(mf_search_result_t *result);
gboolean _mf_search_result_is_end(mf_search_result_t *result);
gchar *_mf_search_result_current_dir_get(mf_search_result_t *result);
guint _mf_search_result_total_count_get(mf_search_result_t *result);

#endif //_MF_SEARCH_INTERNAL_H_
