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

#include <media_content.h>

#include "mf-dlog.h"
#include "mf-util.h"
#include "mf-media-content.h"
#include "mf-media-data.h"

pthread_mutex_t gLockMediaContent;

mf_condition_s *mf_media_content_condition_create(char *condition)
{
	mf_condition_s *filter = NULL;

	filter = (mf_condition_s *)calloc(1, sizeof(mf_condition_s));

	if (filter == NULL) {
		return NULL;
	}

	memset(filter, 0, sizeof(mf_condition_s));

	SECURE_DEBUG("condition [%s]", condition);
	filter->cond = condition;
	filter->collate_type = MEDIA_CONTENT_COLLATE_DEFAULT;
	filter->sort_type = MEDIA_CONTENT_ORDER_DESC;
	filter->sort_keyword = MEDIA_MODIFIED_TIME;
	filter->with_meta = true;

	return filter;
}

void mf_media_conte_free_condition(mf_condition_s **condition)
{
	mf_retm_if(condition == NULL, "condition is NULL");
	if (*condition) {
		SAFE_FREE_CHAR((*condition)->cond);
		SAFE_FREE_CHAR(*condition);
	}
}

int mf_media_content_create_filter(filter_h *filter, mf_condition_s *condition)
{
	mf_retvm_if(filter == NULL, -1, "filter is NULL");
	mf_retvm_if(condition == NULL, -1, "condition is NULL");

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_h tmp_filter = NULL;
	ret = media_filter_create(&tmp_filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_debug("Fail to create filter");
		return ret;
	}
	if (condition->cond) {
		ret = media_filter_set_condition(tmp_filter, condition->cond,
		                                 condition->collate_type);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			mf_debug("Fail to set condition");
			goto ERROR;
		}
	}
	/*
	if (condition->sort_keyword) {
		ret = media_filter_set_order(tmp_filter, condition->sort_type,
					     condition->sort_keyword,
					     condition->collate_type);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			mf_debug("Fail to set order");
			goto ERROR;
		}
	}
	mf_debug("offset is %d, count is %d", condition->offset, condition->count);
	if (condition->offset != -1 && condition->count != -1 &&
	    condition->count > condition->offset) {
		ret = media_filter_set_offset(tmp_filter, condition->offset,
					      condition->count);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			mf_debug("Fail to set offset");
			goto ERROR;
		}
	}
	*/
	*filter = tmp_filter;
	return ret;
ERROR:
	if (tmp_filter) {
		media_filter_destroy(tmp_filter);
		tmp_filter = NULL;
	}
	return ret;
}

int mf_media_content_destroy_filter(filter_h filter)
{
	mf_retvm_if(filter == NULL, -1, "filter is NULL");
	int ret = MEDIA_CONTENT_ERROR_NONE;
	ret = media_filter_destroy(filter);

	return ret;
}

int mf_media_content_data_count_get(const char *condition)
{
	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;
	ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_error("Fail to create filter");
		return 0;
	}

	ret = media_filter_set_condition(filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_error("Fail to set condition");
		return 0;
	}
	int count = 0;
	ret = media_info_get_media_count_from_db(filter, &count);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_error("Fail to get media");
		return 0;
	}

	ret = media_filter_destroy(filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_error("destroy filter failed\n\n");
		return 0;
	}

	return count;
}

int mf_media_content_data_get(void *data, char *condition, bool (*func)(media_info_h media, void *data))
{
	filter_h filter = NULL;
	int ret = -1;

	ret = media_filter_create(&filter);
	if (ret != 0) {
		mf_debug("Create filter failed");
		return ret;
	}

	ret = media_filter_set_condition(filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_debug("Fail to set condition");
		return ret;
	}

	ret = media_info_foreach_media_from_db(filter,
	                                       (media_info_cb)func,
	                                       data);
	if (ret != 0) {
		mf_debug("Fail to parse folders in db: %d", ret);
	}

	mf_media_content_destroy_filter(filter);

	return ret;
}

void mf_media_content_scan_file(const char *path)
{
	mf_retm_if(path == NULL, "path is NULL");
	pthread_mutex_lock(&gLockMediaContent);
	media_content_scan_file(path);
	pthread_mutex_unlock(&gLockMediaContent);
}

void mf_media_content_scan_folder(const char *path)
{
	mf_retm_if(path == NULL, "path is NULL");
	pthread_mutex_lock(&gLockMediaContent);

	media_content_scan_folder(path, true, NULL, NULL);
	pthread_mutex_unlock(&gLockMediaContent);
}

void mf_media_content_disconnect()
{
	pthread_mutex_lock(&gLockMediaContent);
	if (media_content_disconnect() < 0) {
		mf_error("media content disconnect failed.");
	}
	pthread_mutex_unlock(&gLockMediaContent);
}
