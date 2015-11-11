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

#ifdef MYFILE_DOWNLOAD_APP_FEATURE
#include "mf-download-app.h"
#include <package_info.h>
#include <package_manager.h>
#include "mf-file-util.h"

static GList *download_app_list = NULL;
typedef struct {
	pthread_t tid;
	int alive;

	async_fn fn; //'fn' Must be MT-safe
	int fn_ret;
	callback_fn cb;
	downloadapp_data *ad;

	Ecore_Idler *worker_idler;
} download_app_worker;

int count;//wangyan for test

static inline char *mf_download_app_strdup(const char *str)
{
	if (str != NULL) {
		return strdup(str);
	} else {
		return NULL;
	}
}

static inline char *mf_download_app_check_icon(const char *icon_path)
{
	if (EINA_TRUE == mf_file_exists(icon_path)) {
		return strdup(icon_path);
	} else {
		return NULL;
	}
}

static void mf_download_app_get_listinfo(package_info_h handle,
        downloadapp_listinfo *info)
{
	MF_TRACE_BEGIN;
	int ret = 0;
	char *value;

	mf_ret_if(NULL == handle, -1);
	mf_ret_if(NULL == info, -1);

	value = NULL;
	ret = pkgmgrinfo_pkginfo_get_pkgid(handle, &value);
	if (ret < 0) {
		mf_error("pkgmgrinfo_pkginfo_get_pkgid() Fail(%d)", ret);
	}

	info->pkgid = mf_download_app_strdup(value);

	mf_error("------------------get package pkgid is [%s]--------------------------", info->pkgid);

	value = NULL;
	ret = package_info_get_label(handle, &value);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		mf_error("package_info_get_label() Fail(%d)", ret);
	}

	info->pkg_label = mf_download_app_strdup(value);

	value = NULL;
	ret = package_info_get_package(handle, &value);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		mf_error("package_info_get_package() Fail(%d)", ret);
	}
	info->mainappid = mf_download_app_strdup(value);

	value = NULL;
	ret = package_info_get_icon(handle, &value);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		mf_error("package_info_get_icon() Fail(%d)", ret);
	}
	info->icon_path = mf_download_app_check_icon(value);

	ret = package_info_is_preload_package(handle, &info->is_preload);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		mf_error("package_info_is_preload_package() Fail(%d)", ret);
	}

	//ret = package_info_is_update(handle, &info->is_update);
	//if (ret != PACKAGE_MANAGER_ERROR_NONE)
	//	mf_error("package_info_is_update() Fail(%d)", ret);

	value = NULL;
	ret = package_info_get_type(handle, &value);
	if (ret != PACKAGE_MANAGER_ERROR_NONE) {
		mf_error("package_info_get_type() Fail(%d)", ret);
	}
	info->pkg_type = mf_download_app_strdup(value);
	MF_TRACE_END;
}

static int mf_download_app_list_iter(package_info_h handle, void *data)
{
	MF_TRACE_BEGIN;
	GList **pkg_list = data;
	downloadapp_listinfo *info = NULL;
	int invalid = 0;
	count++ ;//wangyan for test
	mf_retv_if(NULL == handle, 0);
	mf_retv_if(NULL == data, -1);

	info = calloc(1, sizeof(downloadapp_listinfo));
	if (NULL == info) {
		mf_error("downloadapp_listinfo calloc() Fail");
		return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
	}

	mf_download_app_get_listinfo(handle, info);

	if (info->is_preload && !info->is_update) {
		invalid = 1; //not download app
		mf_error("------------------[%s] app is not donwload app---------------------", info->pkgid);
	}
	if (invalid == 0) {
		mf_error("------------------[%s] app is donwload app, count is [%d]---------------------", info->pkgid, count);
		if (count <= 15) { //wangyan for test
			*pkg_list = g_list_append(*pkg_list, info);
		}
	}

	MF_TRACE_END;
	return MYFILE_ERR_NONE;
}

int mf_download_app_list_get(downloadapp_data *da)
{
	MF_TRACE_BEGIN;
	int ret;
	mf_retv_if(NULL == da, MYFILE_ERR_INVALID_ARG);
	//init app list
	//count = 0 ;//wangyan for test

	if (download_app_list) {
		g_list_foreach(download_app_list, (GFunc) free, NULL);
		g_list_free(download_app_list);
		download_app_list = NULL;
		mf_info("successfully deleteddownload_app_list previous list items!");
	}

	ret = package_info_foreach_package_info(mf_download_app_list_iter, &download_app_list);
	if (ret < 0) {
		mf_error("pkgmgrinfo_pkginfo_get_list() Fail(%d)", ret);
		return MYFILE_ERR_UNKNOWN_ERROR;
	}
	da->app_list = download_app_list;

	MF_TRACE_END;
	return MYFILE_ERR_NONE;
}

void mf_download_app_list_get_cb(int fn_result, downloadapp_data *da)
{
	MF_TRACE_BEGIN;
	mf_ret_if(NULL == da, -1);

	if (MYFILE_ERR_NONE != fn_result) {
		mf_error("appmgrUg_get_listinfos() Fail(%d)", fn_result);
	} else {
		//add codes to update view,da->app_list is the got download app list,the data in this list is downloadapp_listinfo
		mf_error("appmgrUg_get_listinfos() success(%d)", fn_result);

		/*
		GList *list = da->app_list;
		while (list) {
			downloadapp_listinfo *info = list->data;
			mf_error("get app list item is %s",info->pkgid);
			list = list->next;
		} */

	}
	da->list_worker = NULL;
	MF_TRACE_END;
}

void mf_download_app_destroy_data(downloadapp_data *da)
{
	if (da->list_worker) {
		da->list_worker = NULL;
	}
	//destroy data
}

static Eina_Bool _async_worker_idler(void *data)
{
	download_app_worker *worker = data;

	mf_retv_if(NULL == data, ECORE_CALLBACK_CANCEL);

	pthread_join(worker->tid, NULL);
	worker->alive = FALSE;

	if (worker->cb) {
		worker->cb(worker->fn_ret, worker->ad);
	}

	//g_hash_table_remove(async_worker_hashT, worker);

	return ECORE_CALLBACK_CANCEL;
}

static void* _async_worker_thread(void *data)
{
	MF_TRACE_BEGIN;
	int ret;
	download_app_worker *worker = data;

	mf_retv_if(NULL == data, NULL);

	ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	if (0 != ret) {
		mf_error("pthread_setcancelstate() Fail(%d)", ret);
		pthread_exit(NULL);
	}

	worker->fn_ret = worker->fn(worker->ad);

	worker->worker_idler = ecore_idler_add(_async_worker_idler, worker);

	pthread_exit(NULL);
	MF_TRACE_END;
}

void* mf_download_app_start_async_worker(async_fn fn, callback_fn cb,
        downloadapp_data *ad)
{
	MF_TRACE_BEGIN;
	int ret;
	download_app_worker *worker;

	mf_retv_if(NULL == fn, NULL);

	/*if (NULL == async_worker_hashT)
	{
		async_worker_hashT = g_hash_table_new_full(NULL, NULL,
				_async_worker_hash_free_key, NULL);
	}*/

	worker = calloc(1, sizeof(download_app_worker));
	if (NULL == worker) {
		mf_error("calloc() Fail");
		return NULL;
	}
	worker->fn = fn;
	worker->cb = cb;
	worker->ad = ad;

	//g_hash_table_add(async_worker_hashT, worker);

	ret = pthread_create(&worker->tid, NULL, _async_worker_thread, worker);
	if (ret) {
		mf_error("phread_create() Fail(%d)", ret);
	}

	worker->alive = TRUE;

	MF_TRACE_END;
	return worker;
}

GList *mf_download_app_data_list_get()
{
	mf_info("download_app_list count in data list get: [%d]", g_list_length(download_app_list));
	return download_app_list;
}

void mf_download_app_update_list_info(void *ad)
{
	//mf_download_app_start_async_worker
}

void mf_download_app_main(void *data)
{
	MF_TRACE_BEGIN;
	downloadapp_data *da = NULL;
	da = calloc(1, sizeof(downloadapp_data));
	if (NULL == da) {
		mf_error("downloadapp_data calloc() Fail");
		return;
	}
	da->data = (struct appdata *)data;
	da->list_worker = NULL;
	da->list_worker = mf_download_app_start_async_worker(mf_download_app_list_get,
	                  mf_download_app_list_get_cb, da);
	MF_TRACE_END;
}
#endif
