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

#include "mf-storage-space.h"
#include "mf-media-data.h"
#include "mf-media.h"
#include "mf-file-util.h"
mfStorageStatus g_mf_storage_status;
Update_Info g_recent_info;
static double recent_total_size = 0;

static bool __mf_storage_main_media_item_cb(media_info_h media, void *data)
{
	mf_retvm_if(media == NULL, true, "media is NULL");
	mf_retvm_if(data == NULL, true, "Data parameter is NULL");
	mfStorageStatus *status = (mfStorageStatus *) data;
	unsigned long long size = 0;

	if (g_mf_storage_status.exit_flag == 1) {
		mf_debug("exit_flag is 1");
		return false;
	}

	if (g_mf_storage_status.type == MF_STORAGE_DOCUMENT) {
		fsFileType file_type = 0;
		char *file_path = NULL;
		media_info_get_file_path(media, &file_path);
		if (!file_path) {
			mf_debug("file_path is NULL");
			return true;
		}

		mf_file_attr_get_file_category(file_path, &file_type);
		//mf_error(">>>>file_path = %s, file_type=%d, FILE_TYPE_DOC=%d,FILE_TYPE_TXT=%d", file_path, file_type, FILE_TYPE_DOC,FILE_TYPE_TXT);
		switch (file_type) {
		case FILE_TYPE_DOC:
		case FILE_TYPE_PDF:
		case FILE_TYPE_PPT:
		case FILE_TYPE_EXCEL:
		case FILE_TYPE_TXT:
		case FILE_TYPE_HWP:
		case FILE_TYPE_SPD:
		case FILE_TYPE_SNB: {
			//mf_error(">>>>file_type = %d", file_type);
			if (!mf_file_exists(file_path)) {
				free(file_path);
				file_path = NULL;
				return true;
			}
			if (file_path != NULL) {
				free(file_path);
				file_path = NULL;
			}

			media_info_get_size(media, &size);
			//mf_error(">>>>size = %d", size);
			status->total_size += size;
			break;
		}
		default:
			if (file_path != NULL) {
				free(file_path);
				file_path = NULL;
			}
			break;
		}
	} else {
		char *file_path = NULL;
		media_info_get_file_path(media, &file_path);
		if (!file_path) {
			mf_debug("file_path is NULL");
			return true;
		}

		if (!mf_file_exists(file_path)) {
			free(file_path);
			return true;
		}
		if (file_path != NULL) {
			free(file_path);
		}

		media_info_get_size(media, &size);
		status->total_size += size;
	}
	return true;
}

static double __mf_storage_main_picture_status_get()
{
	MF_TRACE_BEGIN;
	double total_size = 0.0;

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;

	pthread_mutex_lock(&g_mf_storage_status.update_mutex);
	g_mf_storage_status.type = MF_STORAGE_IMAGE;
	g_mf_storage_status.total_size = 0;
	pthread_mutex_unlock(&g_mf_storage_status.update_mutex);

	/*Set Filter*/
	char *condition = MF_CONDITION_LOCAL_IMAGE;//" and (MEDIA_PATH LIKE \'/opt/usr/%%\' OR MEDIA_PATH LIKE  \'/opt/storage/sdcard/%%\')";	/*0-image, 1-video, 2-sound, 3-music, 4-other*/

	ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_error("Fail to create filter");
		return total_size;
	}

	ret = media_filter_set_condition(filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_error("Fail to set condition");
		return total_size;
	}

	ret = media_info_foreach_media_from_db(filter, __mf_storage_main_media_item_cb, &g_mf_storage_status);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_error("Fail to get media");
		return total_size;
	}

	ret = media_filter_destroy(filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_error("destroy filter failed\n\n");
		return total_size;
	}

	return g_mf_storage_status.total_size;
}

static double __mf_storage_main_video_status_get()
{
	MF_TRACE_BEGIN;
	double total_size = 0.0;

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;

	pthread_mutex_lock(&g_mf_storage_status.update_mutex);
	g_mf_storage_status.type = MF_STORAGE_VIDEO;
	g_mf_storage_status.total_size = 0;
	pthread_mutex_unlock(&g_mf_storage_status.update_mutex);

	/*Set Filter*/
	char *condition = MF_CONDITION_LOCAL_VIDEO;//" and MEDIA_PATH LIKE \'/opt/usr/%%\'";	/*0-image, 1-video, 2-sound, 3-music, 4-other*/

	ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_error("Fail to create filter");
		return total_size;
	}

	ret = media_filter_set_condition(filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_error("Fail to set condition");
		return total_size;
	}

	ret = media_info_foreach_media_from_db(filter, __mf_storage_main_media_item_cb, &g_mf_storage_status);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_error("Fail to get media");
		return total_size;
	}

	ret = media_filter_destroy(filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_error("destroy filter failed\n\n");
		return total_size;
	}
	return g_mf_storage_status.total_size;
}


static double __mf_storage_main_sound_status_get()
{
	MF_TRACE_BEGIN;

	double total_size = 0.0;

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;
	pthread_mutex_lock(&g_mf_storage_status.update_mutex);
	g_mf_storage_status.type = MF_STORAGE_SOUND;
	g_mf_storage_status.total_size = 0;
	pthread_mutex_unlock(&g_mf_storage_status.update_mutex);

	/*Set Filter*/
	char *condition = MF_CONDITION_LOCAL_SOUND;//" and MEDIA_PATH LIKE \'/opt/usr/%%\'";	/*0-image, 1-video, 2-sound, 3-music, 4-other*/

	ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_debug("Fail to create filter");
		return total_size;
	}

	ret = media_filter_set_condition(filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_debug("Fail to set condition");
		return total_size;
	}

	ret = media_info_foreach_media_from_db(filter, __mf_storage_main_media_item_cb, &g_mf_storage_status);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_debug("Fail to get media");
		return total_size;
	}

	ret = media_filter_destroy(filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_error("destroy filter failed\n\n");
		return total_size;
	}

	return g_mf_storage_status.total_size;
}

static double __mf_storage_main_document_status_get()
{
	MF_TRACE_BEGIN;

	double total_size = 0.0;

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;
	pthread_mutex_lock(&g_mf_storage_status.update_mutex);
	g_mf_storage_status.type = MF_STORAGE_DOCUMENT;
	g_mf_storage_status.total_size = 0;
	pthread_mutex_unlock(&g_mf_storage_status.update_mutex);

	/*Set Filter*/
	char *condition = MF_CONDITION_LOCAL_DOCUMENT;//" and MEDIA_PATH LIKE \'/opt/usr/%%\'";	/*0-image, 1-video, 2-sound, 3-music, 4-other*/

	ret = media_filter_create(&filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_debug("Fail to create filter");
		return total_size;
	}

	ret = media_filter_set_condition(filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_debug("Fail to set condition");
		return total_size;
	}

	ret = media_info_foreach_media_from_db(filter, __mf_storage_main_media_item_cb, &g_mf_storage_status);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		mf_debug("Fail to get media");
		return total_size;
	}

	ret = media_filter_destroy(filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mf_error("destroy filter failed\n\n");
		return total_size;
	}

	return g_mf_storage_status.total_size;
}

/*static double __mf_storage_main_recent_status_get()
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)mf_get_appdata();

	pthread_mutex_lock(&g_mf_storage_status.update_mutex);
	g_mf_storage_status.type = MF_STORAGE_RECENT;
	g_mf_storage_status.total_size = 0;
	pthread_mutex_unlock(&g_mf_storage_status.update_mutex);

	mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.recent_list), MYFILE_TYPE_FSNODE);
	mf_util_generate_saved_files_list(ap, mf_list_recent_files);
	void *pNode = NULL;
	Eina_List *l = NULL;

	EINA_LIST_FOREACH(ap->mf_FileOperation.recent_list, l, pNode) {
		fsNodeInfo *Node = (fsNodeInfo *)pNode;
		if (Node != NULL) {
			char *fullpath = (char*) calloc(1, (strlen(Node->path) + strlen (Node->path))*sizeof(char) + 5);
			sprintf(fullpath, "%s/%s", Node->path, Node->path);
			mf_debug("fullpath=%s", fullpath);
			off_t size = 0;
			mf_file_attr_get_file_size(fullpath, &size);
			mf_debug("size=%lld", size);
			g_mf_storage_status.total_size = g_mf_storage_status.total_size + size;
			free(fullpath);
		}
	}

	return g_mf_storage_status.total_size;
}*/

void __mf_storage_main_pipe_cb(void *data, void *buffer, unsigned int nbyte)
{
	mf_retm_if(data == NULL, "Data parameter is NULL");
	mf_retm_if(buffer == NULL, "buffer parameter is NULL");
	Update_Info *update_info = (Update_Info *) buffer;
	mfStorageStatus *storage_status = (mfStorageStatus *)data;

	mf_debug("update_info : %d", update_info->type);

	switch (update_info->type) {
	case MF_STORAGE_IMAGE: {
		if (storage_status->update_cb) {
			storage_status->update_cb(mf_tray_item_category_image, update_info, storage_status->pUserData);
		}

		storage_status->image_size_info.total_size = update_info->total_size;
		storage_status->image_size_info.type = update_info->type;
		break;
	}
	case MF_STORAGE_VIDEO: {
		if (storage_status->update_cb) {
			storage_status->update_cb(mf_tray_item_category_video, update_info, storage_status->pUserData);
		}
		storage_status->video_size_info.total_size = update_info->total_size;
		storage_status->video_size_info.type = update_info->type;
		break;
	}
	case MF_STORAGE_SOUND: {
		if (storage_status->update_cb) {
			storage_status->update_cb(mf_tray_item_category_sounds, update_info, storage_status->pUserData);
		}
		storage_status->sound_size_info.total_size = update_info->total_size;
		storage_status->sound_size_info.type = update_info->type;
		break;
	}
	case MF_STORAGE_DOCUMENT: {
		if (storage_status->update_cb) {
			storage_status->update_cb(mf_tray_item_category_document, update_info, storage_status->pUserData);
		}
		storage_status->document_size_info.total_size = update_info->total_size;
		storage_status->document_size_info.type = update_info->type;
		break;
	}
	case MF_STORAGE_RECENT: {
		/*if (storage_status->update_cb)
			storage_status->update_cb(mf_tray_item_category_recent, update_info, storage_status->pUserData);
		storage_status->recent_size_info.total_size = update_info->total_size;
		storage_status->recent_size_info.type= update_info->type;*/
		break;
	}
	default:
		mf_error("wrong update type");
		break;
	}
}

static void *__mf_storage_main_thread_start_routine(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "Data parameter is NULL");
	mfStorageStatus *storage_status = (mfStorageStatus *)data;
	mf_update_type type = MF_STORAGE_IMAGE;
	while (1) {
		pthread_mutex_lock(&storage_status->update_mutex);
		/*while (storage_status->type == MF_STORAGE_SLEEP) {
			pthread_cond_wait(&storage_status->wait_cond, &storage_status->update_mutex);
		}*/
		type = storage_status->type;
		pthread_mutex_unlock(&storage_status->update_mutex);

		int exit_flag = 0;

		pthread_mutex_lock(&storage_status->exit_mutex);
		exit_flag = storage_status->exit_flag;
		pthread_mutex_unlock(&storage_status->exit_mutex);

		if (exit_flag == 1) {
			mf_debug("exit_flag is 1");
			break;
		}

		if (type >= MF_STORAGE_MAX) {
			mf_debug("work is done, thread will sleep");
			pthread_mutex_lock(&storage_status->update_mutex);
			storage_status->type = MF_STORAGE_SLEEP;
			pthread_mutex_unlock(&storage_status->update_mutex);
			break;
		}

		switch (type) {
		case MF_STORAGE_IMAGE: {
			double images_size = __mf_storage_main_picture_status_get();
			mf_debug("images_size = %f", images_size);

			Update_Info images_info;
			memset(&images_info, 0, sizeof(Update_Info));
			images_info.type = MF_STORAGE_IMAGE;
			images_info.total_size = images_size;
			if (storage_status->pipe) {
				ecore_pipe_write(storage_status->pipe, &images_info, sizeof(images_info));
			}
		}
		break;
		case MF_STORAGE_VIDEO: {
			double video_size = __mf_storage_main_video_status_get();
			mf_debug("video_size = %f", video_size);

			Update_Info video_info;
			memset(&video_info, 0, sizeof(Update_Info));
			video_info.type = MF_STORAGE_VIDEO;
			video_info.total_size = video_size;
			if (storage_status->pipe) {
				ecore_pipe_write(storage_status->pipe, &video_info, sizeof(video_info));
			}
		}
		break;
		case MF_STORAGE_SOUND: {
			double sound_size = __mf_storage_main_sound_status_get();
			mf_debug("sound_size = %f", sound_size);

			Update_Info sound_info;
			memset(&sound_info, 0, sizeof(Update_Info));
			sound_info.type = MF_STORAGE_SOUND;
			sound_info.total_size = sound_size;
			if (storage_status->pipe) {
				ecore_pipe_write(storage_status->pipe, &sound_info, sizeof(sound_info));
			}
		}
		break;
		case MF_STORAGE_DOCUMENT: {
			double document_size = __mf_storage_main_document_status_get();
			mf_debug("document_size = %f", document_size);

			Update_Info document_info;
			memset(&document_info, 0, sizeof(Update_Info));
			document_info.type = MF_STORAGE_DOCUMENT;
			document_info.total_size = document_size;
			if (storage_status->pipe) {
				ecore_pipe_write(storage_status->pipe, &document_info, sizeof(document_info));
			}
		}
		break;
		case MF_STORAGE_RECENT: {
			/*double recent_size = __mf_storage_main_recent_status_get();
				mf_debug("recent_size = %f", recent_size);

				Update_Info recent_info;
				memset(&recent_info, 0, sizeof(Update_Info));
				recent_info.type = MF_STORAGE_RECENT;
				recent_info.total_size = recent_size;
				if (storage_status->pipe)
					ecore_pipe_write(storage_status->pipe, &recent_info, sizeof(Update_Info));*/
		}
		break;
		default: {
			mf_debug("type = %d", type);
			break;
		}
		}
		pthread_mutex_lock(&storage_status->update_mutex);
		storage_status->type++;
		pthread_mutex_unlock(&storage_status->update_mutex);
	}
	pthread_mutex_lock(&storage_status->update_data_mutex);
	storage_status->is_update_data = true;
	pthread_mutex_unlock(&storage_status->update_data_mutex);

	pthread_exit((void *) 0);
	//storage_status->tid = 0;
	MF_TRACE_END;
}

static void __mf_storage_main_init(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "Data parameter is NULL");
	mfStorageStatus *storage_status = (mfStorageStatus *)data;

	pthread_mutex_init(&storage_status->exit_mutex, NULL);
	pthread_mutex_init(&storage_status->update_mutex, NULL);
	//pthread_cond_init(&storage_status->wait_cond, NULL);

	pthread_mutex_lock(&storage_status->exit_mutex);
	storage_status->exit_flag = EINA_FALSE;
	pthread_mutex_unlock(&storage_status->exit_mutex);

	pthread_mutex_lock(&storage_status->update_mutex);
	storage_status->type = MF_STORAGE_IMAGE;
	pthread_mutex_unlock(&storage_status->update_mutex);

	pthread_mutex_lock(&storage_status->update_data_mutex);
	storage_status->is_update_data = false;
	pthread_mutex_unlock(&storage_status->update_data_mutex);
	MF_TRACE_END;
}

int mf_storage_create(void* app_data)
{
	MF_TRACE_BEGIN;
	mfStorageStatus* storage_status = &g_mf_storage_status;
	if (storage_status->is_update_data == true) {
		return 1;
	}
	__mf_storage_main_init(storage_status);
	if (storage_status->pipe) {
		ecore_pipe_del(storage_status->pipe);
		storage_status->pipe = NULL;
	}
	storage_status->pipe = ecore_pipe_add(__mf_storage_main_pipe_cb, storage_status);
	if (storage_status->pipe) {
		int ret = pthread_create(&storage_status->tid, NULL, __mf_storage_main_thread_start_routine, storage_status);
		if (ret != 0) {
			mf_error("fail to pthread_create");
		}
		mf_debug("thread id = %ld", storage_status->tid);
	} else {
		mf_error("fail to ecore_pipe_add");
	}
	MF_TRACE_END;
	return 1;
}

int mf_storage_destroy(void* app_data)
{
	MF_TRACE_BEGIN;
	mfStorageStatus* storage_status = &g_mf_storage_status;
	int ret = 0;
	struct appdata *ap = (struct appdata *)mf_get_appdata();

	pthread_mutex_lock(&storage_status->update_data_mutex);
	storage_status->is_update_data = false;
	if (ap->mf_Status.view_type != mf_view_root || ap->mf_Status.more != MORE_DEFAULT) {
		storage_status->update_cb = NULL;
	}
	pthread_mutex_unlock(&storage_status->update_data_mutex);

	pthread_mutex_lock(&storage_status->exit_mutex);
	storage_status->exit_flag = 1;
	pthread_mutex_unlock(&storage_status->exit_mutex);

	pthread_mutex_lock(&storage_status->update_mutex);
	storage_status->type = MF_STORAGE_SLEEP;
	//pthread_cond_signal(&storage_status->wait_cond);
	pthread_mutex_unlock(&storage_status->update_mutex);
	if (storage_status->tid > 0) {
		void *thread_ret = NULL;
		ret = pthread_join(storage_status->tid, &thread_ret);
		if (ret != 0) {
			mf_error("fail to join with thread");
		}
		storage_status->tid = 0;
		mf_debug("thread exit code %d", (int)thread_ret);
	}

	pthread_mutex_destroy(&storage_status->exit_mutex);
	pthread_mutex_destroy(&storage_status->update_mutex);
	//pthread_cond_destroy(&storage_status->wait_cond);

	if (storage_status->pipe) {
		ecore_pipe_del(storage_status->pipe);
		storage_status->pipe = NULL;
	}
	MF_TRACE_END;
	return 0;
}

int mf_storage_set_update_cb(void* app_data, mf_storage_update_cb update_cb)
{
	mfStorageStatus* storage_status = &g_mf_storage_status;
	storage_status->update_cb = update_cb;
	return 0;
}

mfStorageStatus* mf_storage_get_status(void* app_data)
{
	return &g_mf_storage_status;
}

int mf_storage_set_update_data(void* app_data, bool is_update)
{
	mfStorageStatus* storage_status = &g_mf_storage_status;
	pthread_mutex_lock(&storage_status->update_data_mutex);
	storage_status->is_update_data = is_update;
	pthread_mutex_unlock(&storage_status->update_data_mutex);
	return 0;
}

int mf_storage_refresh(void* app_data)
{
	MF_TRACE_BEGIN;
	mf_storage_destroy(app_data);
	mf_storage_create(app_data);
	MF_TRACE_END;
	return 1;
}

bool mf_storage_get_recent_files_size_cb(MFRitem *Ritem, void *user_data)
{
	if (Ritem && Ritem->path) {
		SECURE_ERROR("Ritem->path is [%s]mf_file_exists is [%d] access(dst_dir, R_OK | W_OK) is [%d] ", Ritem->path, mf_file_exists(Ritem->path),  access(Ritem->path, R_OK | W_OK));
		if (mf_file_exists(Ritem->path)) {
			off_t size = 0;
			mf_file_attr_get_file_size(Ritem->path, &size);
			if (size > 0) {
				recent_total_size += size;
			}
		}
	}
	return true;
}

void mf_storage_get_recent_files_size()
{
	struct appdata *ap = mf_get_appdata();
	recent_total_size = 0;
	g_recent_info.type = MF_STORAGE_RECENT;
	mf_media_foreach_recent_files_list(ap->mf_MainWindow.mfd_handle, mf_storage_get_recent_files_size_cb, NULL);
	g_recent_info.total_size = recent_total_size;
	g_mf_storage_status.recent_size_info.total_size = g_recent_info.total_size;
	g_mf_storage_status.recent_size_info.type = g_recent_info.type;
	//mf_category_storage_size_refresh(mf_tray_item_category_recent, &g_recent_info, NULL);
}

