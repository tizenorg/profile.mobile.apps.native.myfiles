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

#include <sqlite3.h>
#include <string.h>
#include "mf-media.h"
#include "mf-media-db.h"
#include "mf-media-error.h"
#include "mf-dlog.h"
#include "mf-media-types.h"

int mf_media_connect(MFDHandle **handle)
{
	int ret = MFD_ERROR_NONE;
	sqlite3 * db_handle = NULL;

	ret = mf_connect_db_with_handle(&db_handle);
	if (ret != MFD_ERROR_NONE) {
		return ret;
	}

	*handle = db_handle;
	return MFD_ERROR_NONE;

}

int mf_media_disconnect(MFDHandle *handle)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if (handle == NULL) {
		return MFD_ERROR_INVALID_PARAMETER;
	}

	return mf_disconnect_db_with_handle(db_handle);
}

int mf_media_add_recent_files(MFDHandle *mfd_handle, const char *path, const char *name, int storage_type, const char *thumbnail_path)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		mf_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_insert_recent_file(mfd_handle, path, name, storage_type, thumbnail_path);
	if (ret != MFD_ERROR_NONE) {
		mf_debug("insert content info into folder table failed");
		return ret;
	}

	return ret;
}

int mf_media_delete_recent_files(MFDHandle *mfd_handle, const char *path)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		mf_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_delete_recent_files(mfd_handle, path);
	if (ret != MFD_ERROR_NONE) {
		mf_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_media_delete_recent_files_by_type(MFDHandle *mfd_handle, int storage_type)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		mf_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_delete_recent_files_by_type(mfd_handle, storage_type);
	if (ret != MFD_ERROR_NONE) {
		mf_debug("delete device info into devices table failed");
		return ret;
	}

	return ret;
}

int mf_media_update_recent_files_thumbnail(MFDHandle *mfd_handle, const char *thumbnail, const char *new_thumbnail)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		mf_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_update_recent_files_thumbnail(mfd_handle, thumbnail, new_thumbnail);
	if (ret != MFD_ERROR_NONE) {
		mf_debug
		("update device icon failed");
		return ret;
	}

	return ret;

}

int mf_media_foreach_recent_files_list(MFDHandle *mfd_handle, mf_recent_files_item_cb callback, void *user_data)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		mf_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_foreach_recent_files_list(mfd_handle, callback, user_data);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		mf_debug("foreach content list fail");
		return ret;
	}

	return ret;
}

int mf_media_get_recent_files_count(MFDHandle *mfd_handle, int *count)
{
	int ret = MFD_ERROR_NONE;

	if (mfd_handle == NULL) {
		mf_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	ret = mf_get_recent_files_count(mfd_handle, count);
	if (ret != MFD_ERROR_NONE && ret != MFD_ERROR_DB_NO_RECORD) {
		mf_debug
		("foreach content list fail");
		return ret;
	}

	return ret;
}

int mf_destroy_recent_files_item(MFRitem *ritem)
{
	if (ritem == NULL) {
		mf_debug("citem is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	if (ritem->path) {
		free(ritem->path);
		ritem->path = NULL;
	}
	if (ritem->name) {
		free(ritem->name);
		ritem->name = NULL;
	}
	if (ritem->thumbnail) {
		free(ritem->thumbnail);
		ritem->thumbnail = NULL;
	}

	return MFD_ERROR_NONE;
}

int mf_media_find_recent_file(MFDHandle *mfd_handle, const char *path)
{
	int find = 0;

	if (mfd_handle == NULL) {
		mf_debug("media service handle is NULL");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	find = mf_find_recent_file(mfd_handle, path);

	return find;
}
