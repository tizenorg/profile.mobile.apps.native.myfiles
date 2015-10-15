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
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <glib.h>

#include "mf-dlog.h"
#include "mf-search.h"
#include "mf-search-internal.h"


/*+++++++++++++++++++++++ APIs +++++++++++++++++++++++*/

int mf_search_init(mf_search_handle *handle)
{
	int ret = 0;
	ms_handle_t *ms_handle = NULL;

	if (!handle) {
		return -1;
	}
#if 0//Deprecated API
	if (!g_thread_supported()) {
		g_thread_init(NULL);
	}
#endif
	ret = _mf_search_init(&ms_handle);
	if (ret < 0) {
		ms_error("Fail to init search handle ");
		*handle = (mf_search_handle) 0;
		return ret;
	}

	*handle = (mf_search_handle) ms_handle;

	return MF_SEARCH_ERROR_NONE;
}

int mf_search_start(mf_search_handle handle,
		    const char **root_path,
		    unsigned int path_num,
		    const char *needle,
		    mf_search_option option,
		    void *user_data,
		    mf_search_filter_cb func,
		    int category,
		    int item_count)
{
	int ret = 0;
	if (!handle) {
		return MF_SEARCH_ERROR_INVAL_P;
	}

	if (!root_path || path_num < 1) {
		return MF_SEARCH_ERROR_INVAL_P;
	}

	if (!needle && !func) {
		return MF_SEARCH_ERROR_INVAL_P;
	}
	ret = _mf_search_start((ms_handle_t *) handle, root_path, path_num, needle, option, user_data, func, category, item_count);

	if (ret < 0) {
		ms_error("Fail to start search ");
	}
	return ret;
}

int mf_search_stop(mf_search_handle handle)
{
	int ret = 0;

	ret = _mf_search_stop((ms_handle_t *) handle);
	if (ret < 0) {
		ms_error("Fail to stop search ");
	}
	return ret;
}

void mf_search_finalize(mf_search_handle *handle)
{
	_mf_search_finalize((ms_handle_t **) handle);
	return;
}

/*+++++++++++++++++++++++ UTIL APIs +++++++++++++++++++++++*/

char *mf_search_result_dir_get(mf_search_result_t *result)
{
	return _mf_search_result_dir_get(result);
}

char *mf_search_result_file_get(mf_search_result_t *result)
{
	return _mf_search_result_file_get(result);
}

int mf_search_result_is_end(mf_search_result_t *result, int *is_end)
{
	if (result) {
		*is_end = _mf_search_result_is_end(result);
	} else {
		return MF_SEARCH_ERROR_INVAL_P;
	}
	return MF_SEARCH_ERROR_NONE;
}

int mf_search_result_total_count_get(mf_search_result_t *result, unsigned int *count)
{
	if (result) {
		*count = _mf_search_result_total_count_get(result);
	} else {
		return MF_SEARCH_ERROR_INVAL_P;
	}
	return MF_SEARCH_ERROR_NONE;
}

char *mf_search_result_current_dir_get(mf_search_result_t *result)
{
	return _mf_search_result_current_dir_get(result);
}
