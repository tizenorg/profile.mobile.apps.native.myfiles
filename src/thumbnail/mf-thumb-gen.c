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
#include <metadata_extractor.h>
#include <pthread.h>
#include <thumbnail_util.h>
#include "mf-thumb-gen.h"
#include "mf-util.h"
#include "mf-view.h"
#include "mf-callback.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-fs-util.h"
#include "mf-file-util.h"

#define MF_THUMB_FILE_NAME_DIR				"/opt/usr/media/.mf_thumbnail/"
#define MF_THUMB_FILE_NAME_STR				MF_THUMB_FILE_NAME_DIR"%s.png"
#define MF_FILE_NAME_MAX_LENGTH 			2048
#define MF_STORAGE_SIZE_MIN				(1*1024*1024)

static pthread_t 	g_thread_id;
static pthread_mutex_t 	g_mutex = PTHREAD_MUTEX_INITIALIZER;
static mf_thumb_gen_h g_thumb_gen = NULL;

#define MF_THUMBNAIL_DEBUG mf_debug
char *thumb_request_id = NULL;

typedef struct _ThumbGen 
{
    char    *szMediaURL;
    bool    bIsRealize;

    char    *szSaveDir;
    bool    bIsStart;
    bool    bIscancel;
    void    *pUserData;

    mf_thumb_gen_progress_cb	progress_cb;
    mf_thumb_gen_complete_cb	complete_cb;
    mf_thumb_gen_cancel_cb		cancel_cb;

    Eina_List * file_list;
}   ThumbGen;


static void _mf_thumb_gen_destroy_handle(ThumbGen *pThumbGen);
void _mf_thumbnail_completed_cb(thumbnail_util_error_e error, const char *request_id, int raw_width, int raw_height, unsigned char *raw_data, int raw_size, void *user_data);

static void _mf_thumb_gen_lock()
{
	pthread_mutex_lock(&g_mutex);
}

static void _mf_thumb_gen_unlock()
{
	pthread_mutex_unlock(&g_mutex);
}

static int myfile_thumb_update_filelist(ThumbGen *pThumbGen)
{
    if (pThumbGen == NULL) {
        return 0;
    }

    _mf_thumb_gen_lock();
    if (pThumbGen->bIsRealize == FALSE) {
        mf_debug(" == pThumbGen->bIsRealize is Fail ==");
        _mf_thumb_gen_unlock();
        return 0;
    }

    _mf_thumb_gen_unlock();
    mf_retvm_if(pThumbGen->file_list == NULL, MYFILE_ERR_INVALID_ARG, "pThumbGen->file_list is null");

    fsNodeInfo *pNode = NULL;
    Eina_List *l = NULL;
    int i = 0;
    char *path = pThumbGen->szMediaURL;
    Eina_List *file_list = pThumbGen->file_list;
    Eina_List *list_tmp = file_list;
    int file_count = eina_list_count(file_list) ;

    thumbnail_h thumb_handle;
    while (true)
    {
        list_tmp = file_list->prev;
        MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>list_tmp = %p", list_tmp);
        if (list_tmp == NULL)
            break;
        else
            file_list = file_list->prev;
    }

    MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>path = %s", path);
    MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>file_list = %x", pThumbGen->file_list);
    MF_THUMBNAIL_DEBUG("\nfile count is [%d]", file_count);

    EINA_LIST_FOREACH(file_list, l, pNode) {
        if (pNode) {
            //Make thumbnail begining............................
            if (pThumbGen->bIscancel == true) {
                goto EXIT;
            }

            if (pNode != NULL && pNode->name != NULL && pNode->path != NULL) 
            {
                MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>pNode->path = %s, pNode->name=%s", pNode->path, pNode->name);

                switch (pNode->type)
                {
                    case FILE_TYPE_IMAGE:
                    case FILE_TYPE_VIDEO:
                    {
                        if (mf_util_is_low_memory(PHONE_FOLDER, MF_STORAGE_SIZE_MIN)) {
                            goto EXIT;
                        }

                        pNode->thumbnail_path = calloc(1, sizeof(char) * MF_FILE_NAME_MAX_LENGTH);
                        if (pNode->thumbnail_path != NULL) {
                        	MF_THUMBNAIL_DEBUG("\n>>>>>1");
                        	snprintf(pNode->thumbnail_path, MF_FILE_NAME_MAX_LENGTH - 1, MF_THUMB_FILE_NAME_STR, pNode->name);
                        	MF_THUMBNAIL_DEBUG("\n>>>>>2");
                        	char tmp_path[MF_FILE_NAME_MAX_LENGTH + 1] = {0};
                        	MF_THUMBNAIL_DEBUG("\n>>>>>3");
                        	snprintf(tmp_path, MF_FILE_NAME_MAX_LENGTH - 1, "%s/%s", pNode->path, pNode->name);
                        	MF_THUMBNAIL_DEBUG("\n>>>>>4");
                        	MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>pNode->thumbnail_path1 = %s", pNode->thumbnail_path);
                        	thumbnail_util_create(&thumb_handle);
                        	if (thumb_handle) {
								thumbnail_util_set_path(thumb_handle, tmp_path);
								int ret = thumbnail_util_extract(thumb_handle, _mf_thumbnail_completed_cb, pNode, &thumb_request_id);
								//int ret = thumbnail_request_save_to_file(tmp_path, pNode->thumbnail_path);
								if (ret == THUMBNAIL_UTIL_ERROR_NONE) {
									mf_thumb_gen_progress_cb func = pThumbGen->progress_cb;
									if (func && pThumbGen->bIscancel == false) {
										func(true, i, pNode);
									} else {
										goto EXIT;
									}
								}
								thumbnail_util_destroy(thumb_handle);
                        	}
                        }
                        if (pThumbGen->bIscancel == true) {
                            goto EXIT;
                        }
                        MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>pNode->thumbnail_path2 = %s", pNode->thumbnail_path);
                        break;
                    }
                    default:
                        break;
                }
            }
            i++;
            //End........................................................
        }
    }

    pThumbGen->bIsStart = FALSE;
    mf_thumb_gen_complete_cb func = pThumbGen->complete_cb;

    if (func && pThumbGen->bIscancel == false) {
     //  func(file_count, pThumbGen->pUserData);
    }

EXIT:
    pthread_exit(0);
    thumbnail_util_destroy(thumb_handle);
    return MYFILE_ERR_NONE;
}


static void* _mf_thumb_gen_thread_loop(void *pUserData)
{
	if (pUserData == NULL) {
		return NULL;
	}
	ThumbGen *pThumbGen = (ThumbGen *)pUserData;
	myfile_thumb_update_filelist(pThumbGen);
	mf_debug(" == Thread End ==");
	return NULL;
}

static void _mf_thumb_gen_destroy_handle(ThumbGen *pThumbGen)
{
	if (pThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return;
	}

	_mf_thumb_gen_lock();

	pThumbGen->progress_cb = NULL;
	pThumbGen->complete_cb = NULL;
	pThumbGen->cancel_cb = NULL;

	SAFE_FREE_CHAR(pThumbGen->szMediaURL);
	SAFE_FREE_CHAR(pThumbGen->szSaveDir);

	SAFE_FREE_CHAR(pThumbGen);

	_mf_thumb_gen_unlock();

}

/* external functions */
mf_thumb_gen_h mf_thumb_gen_create(const char *szMediaURL)
{
	if (szMediaURL == NULL) {
		mf_error("szMediaURL is NULL");
		return NULL;
	}

	ThumbGen *pThumbGen = calloc(1, sizeof(ThumbGen));
	if (pThumbGen == NULL) {
		mf_error("ThumbGen alloc is fail");
		return NULL;
	}


	MF_STRDUP(pThumbGen->szMediaURL, szMediaURL);

	pthread_mutex_init(&g_mutex, NULL);

	return (mf_thumb_gen_h)pThumbGen;
}

void mf_thumb_gen_destroy(mf_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return;
	}
	mf_debug("");

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	if (mf_file_exists(pThumbGen->szSaveDir)) {
		if (!mf_remove(pThumbGen->szSaveDir)) {
			mf_error("Make directory Fail : %s", pThumbGen->szSaveDir);
		}
	}

	mf_thumb_gen_unrealize((mf_thumb_gen_h)pThumbGen);
	_mf_thumb_gen_destroy_handle(pThumbGen);

	pthread_mutex_destroy(&g_mutex);

}

bool mf_thumb_gen_realize(mf_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	pThumbGen->bIsRealize = TRUE;

	if (pthread_create(&g_thread_id, NULL, _mf_thumb_gen_thread_loop, (void *)pThumbGen) != 0) {
		mf_error("pthread_create fail");
		pThumbGen->bIsRealize = FALSE;
		return FALSE;
	}
	return TRUE;
}

bool mf_thumb_gen_unrealize(mf_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	_mf_thumb_gen_lock();
	pThumbGen->bIsStart = FALSE;
	_mf_thumb_gen_unlock();

	if (pThumbGen->bIsRealize == TRUE) {
		_mf_thumb_gen_lock();
		pThumbGen->bIsRealize = FALSE;
		_mf_thumb_gen_unlock();

		int status = 0;
		pthread_join(g_thread_id, (void **)&status);
	}

	return TRUE;
}

bool mf_thumb_gen_is_realize(mf_thumb_gen_h hThumbGen, bool *bIsRealize)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	*bIsRealize = pThumbGen->bIsRealize;

	return TRUE;
}

bool mf_thumb_gen_set_save_directory(mf_thumb_gen_h hThumbGen, char *szSaveDir)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	_mf_thumb_gen_lock();

	if (!mf_file_exists(szSaveDir)) {
		if (!mf_mkdir(szSaveDir)) {
			MF_THUMBNAIL_DEBUG("Make directory Fail : %s", szSaveDir);
		}
	} else {
		if (!mf_file_recursive_rm(szSaveDir)) {
			MF_THUMBNAIL_DEBUG("mf_rmdir Fail : %s", szSaveDir);
		} else {
			MF_THUMBNAIL_DEBUG("mf_rmdir success");
		}
		if (!mf_mkdir(szSaveDir)) {
			MF_THUMBNAIL_DEBUG("Make directory Fail : %s", szSaveDir);
		}
	}

	MF_STRDUP(pThumbGen->szSaveDir, szSaveDir);

	_mf_thumb_gen_unlock();

	return TRUE;
}

bool mf_thumb_gen_start(mf_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	if (pThumbGen->bIsRealize == FALSE) {
		mf_error("Not yet realize state");
		return FALSE;
	}

	if (pThumbGen->bIsStart == TRUE) {
		mf_error("Already thumb gen start");
		return FALSE;
	}

	unsigned long size = mf_fm_svc_wrapper_get_free_space(MYFILE_PHONE);

	if (size <= 0) {
		_mf_thumb_gen_lock();

		pThumbGen->bIscancel = TRUE;

		_mf_thumb_gen_unlock();
	} else {
		mf_thumb_gen_set_save_directory(hThumbGen, MF_THUMB_FILE_NAME_DIR);

		_mf_thumb_gen_lock();

		pThumbGen->bIsStart = TRUE;

		_mf_thumb_gen_unlock();
	}
	return TRUE;
}

bool mf_thumb_gen_cancel(mf_thumb_gen_h hThumbGen)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	if (pThumbGen->bIsRealize == FALSE) {
		mf_error("Not yet realize state");
		return FALSE;
	}

	if (pThumbGen->bIscancel == TRUE) {
		mf_error("Already thumb gen cancels");
		return FALSE;
	}

	_mf_thumb_gen_lock();

	pThumbGen->bIscancel = TRUE;

	_mf_thumb_gen_unlock();

	return TRUE;
}

bool mf_thumb_gen_set_user_data(mf_thumb_gen_h hThumbGen, void *pUserData)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	_mf_thumb_gen_lock();

	pThumbGen->pUserData = pUserData;

	_mf_thumb_gen_unlock();

	return TRUE;
}

bool mf_thumb_gen_set_progress_cb(mf_thumb_gen_h hThumbGen, mf_thumb_gen_progress_cb progress_cb)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	_mf_thumb_gen_lock();

	pThumbGen->progress_cb = progress_cb;

	_mf_thumb_gen_unlock();

	return TRUE;
}

bool mf_thumb_gen_set_complete_cb(mf_thumb_gen_h hThumbGen, mf_thumb_gen_complete_cb complete_cb)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	_mf_thumb_gen_lock();

	pThumbGen->complete_cb = complete_cb;

	_mf_thumb_gen_unlock();

	return TRUE;
}

bool mf_thumb_gen_set_file_list(mf_thumb_gen_h hThumbGen, Eina_List *file_list)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	_mf_thumb_gen_lock();

	pThumbGen->file_list = file_list;
	int file_count = eina_list_count(file_list) ;

	MF_THUMBNAIL_DEBUG("\nfile count is [%d]", file_count);

	_mf_thumb_gen_unlock();

	return TRUE;
}

bool mf_thumb_gen_set_cancel_cb(mf_thumb_gen_h hThumbGen, mf_thumb_gen_cancel_cb cancel_cb)
{
	if (hThumbGen == NULL) {
		mf_error("hThumbGen is NULL");
		return FALSE;
	}

	ThumbGen *pThumbGen = (ThumbGen *)hThumbGen;

	_mf_thumb_gen_lock();

	pThumbGen->cancel_cb = cancel_cb;

	_mf_thumb_gen_unlock();

	return TRUE;
}

static Eina_Bool __mf_make_thumbnail_done(void *data)
{
	mf_retvm_if(data == NULL, ECORE_CALLBACK_CANCEL, "data is NULL");
	mf_debug("__mf_make_thumbnail_done()... ");
	struct appdata *ap = mf_get_appdata();
	int view_style = mf_view_style_get(ap);
	mf_debug("view_style=%d... ", view_style);
	mf_debug("ap->mf_MainWindow.pNaviGenlist=%p... ", ap->mf_MainWindow.pNaviGenlist);
	mf_debug("ap->mf_MainWindow.pNaviGengrid=%p... ", ap->mf_MainWindow.pNaviGengrid);

	if (ap->mf_MainWindow.pNaviGenlist != NULL && ap->mf_MainWindow.pNaviGengrid != NULL) {//FixedP131206-01928
		mf_view_refresh_thumbnail_destroy();
	}

	if (view_style != MF_VIEW_STYLE_THUMBNAIL && ap->mf_MainWindow.pNaviGenlist != NULL) {
		elm_genlist_realized_items_update(ap->mf_MainWindow.pNaviGenlist);
	}
	else if (view_style == MF_VIEW_STYLE_THUMBNAIL && ap->mf_MainWindow.pNaviGengrid != NULL) {
		elm_gengrid_realized_items_update(ap->mf_MainWindow.pNaviGengrid);
	}

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __mf_make_thumbnail_progress(void *data)
{
	mf_retvm_if(data == NULL, ECORE_CALLBACK_CANCEL, "data is NULL");
	mf_debug("__mf_make_thumbnail_done()... ");
	struct appdata *ap = mf_get_appdata();
	int view_style = mf_view_style_get(ap);
	mf_debug("view_style=%d... ", view_style);

	mf_debug("ap->mf_MainWindow.pNaviGenlist=%p... ", ap->mf_MainWindow.pNaviGenlist);
	mf_debug("ap->mf_MainWindow.pNaviGengrid=%p... ", ap->mf_MainWindow.pNaviGengrid);
	if (ap->mf_MainWindow.pNaviGenlist != NULL && ap->mf_MainWindow.pNaviGengrid != NULL) {//FixedP131206-01928
		mf_view_refresh_thumbnail_destroy();
	}

	if (view_style != MF_VIEW_STYLE_THUMBNAIL && ap->mf_MainWindow.pNaviGenlist != NULL) {
		//elm_genlist_realized_items_update(ap->mf_MainWindow.pNaviGenlist);
		fsNodeInfo *pNode = data;
		elm_genlist_item_fields_update(pNode->item, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
	}else if (view_style == MF_VIEW_STYLE_THUMBNAIL && ap->mf_MainWindow.pNaviGengrid != NULL) {
		//elm_genlist_realized_items_update(ap->mf_MainWindow.pNaviGenlist);
		fsNodeInfo *pNode = data;
		elm_gengrid_item_update(pNode->item);
	}
	return ECORE_CALLBACK_CANCEL;
}

Ecore_Pipe *g_mf_thumbnail_progress_pipe = NULL;//Fix the bug, when clicking the image again and again quickly, there will be problem.

typedef enum{
	THUMBNAIL_UPDATE_PROGRESS,
	THUMBNAIL_UPDATE_COMPLETE,
}thumbnail_progress_state_e;

typedef struct {
	int download_id;
	thumbnail_progress_state_e state;
	void *user_data;
} thumbnail_progress_pipe_data_s;

static void
__mf_thumbnail_progress_pipe_handler(void *data, void *buffer, unsigned int nbyte)
{
	thumbnail_progress_pipe_data_s *pipe_data = buffer;
	MF_CHECK(pipe_data);

	MF_THUMBNAIL_DEBUG("\npipe_data->state.. %d", pipe_data->state);
	switch (pipe_data->state) {
	case THUMBNAIL_UPDATE_PROGRESS:
		__mf_make_thumbnail_progress(pipe_data->user_data);
		break;
	case THUMBNAIL_UPDATE_COMPLETE:
		__mf_make_thumbnail_done(pipe_data->user_data);
		break;
	default:
		MF_THUMBNAIL_DEBUG("\nNot defined.. %d", pipe_data->state);
	}
}

void mf_util_thumb_gen_progress_cb(bool bSucess, int nIdx, void *pUserData)
{
	MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>mf_util_thumb_gen_progress_cb enter");
	mf_retm_if(pUserData == NULL, "pUserData is NULL");
	//struct appdata *ap = mf_get_appdata();

	thumbnail_progress_pipe_data_s buffer;
	buffer.state = THUMBNAIL_UPDATE_PROGRESS;
	buffer.user_data = pUserData;

	ecore_pipe_write(g_mf_thumbnail_progress_pipe, &buffer, sizeof(thumbnail_progress_pipe_data_s));
	MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>mf_util_thumb_gen_progress_cb leave");
}

void mf_util_thumb_gen_complete_cb(int nItemCount, void *pUserData)
{
	MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>mf_util_thumb_gen_complete_cb enter");
	mf_retm_if(pUserData == NULL, "pUserData is NULL");
	thumbnail_progress_pipe_data_s buffer;
	buffer.state = THUMBNAIL_UPDATE_COMPLETE;
	buffer.user_data = pUserData;
	ecore_pipe_write(g_mf_thumbnail_progress_pipe, &buffer, sizeof(thumbnail_progress_pipe_data_s));
	MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>mf_util_thumb_gen_progress_cb leave");
}

void mf_view_refresh_thumbnail_destroy()
{
	MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>mf_view_refresh_thumbnail_destroy enter");
	if (g_mf_thumbnail_progress_pipe) {
		ecore_pipe_del(g_mf_thumbnail_progress_pipe);
		g_mf_thumbnail_progress_pipe = NULL;
	}
	if (g_thumb_gen != NULL) {
		mf_thumb_gen_cancel(g_thumb_gen);
		mf_thumb_gen_destroy(g_thumb_gen);
		g_thumb_gen = NULL;
	}
	MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>>mf_view_refresh_thumbnail_destroy leave");
}

void mf_view_refresh_thumbnail_for_other_memory(void *data, Eina_List *file_list)
{
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(file_list == NULL, "file_list is NULL");

	mf_view_refresh_thumbnail_destroy();
	struct appdata *ap = (struct appdata*)data;
	bool is_at_otg = false;
	if (ap->mf_Status.path) {
		is_at_otg = true;
	}
	MF_THUMBNAIL_DEBUG("\n>>>>>>>>>>>> is_at_otg is [%d]", is_at_otg);
	if (is_at_otg) {//Only for otg, we will update the thumbnail
		MF_THUMBNAIL_DEBUG(">>>>>>>>>>>> path is [%s]", ap->mf_Status.path->str);
		if (g_mf_thumbnail_progress_pipe == NULL)
			g_mf_thumbnail_progress_pipe = ecore_pipe_add(__mf_thumbnail_progress_pipe_handler, data);

		mf_thumb_gen_h thumb_gen = mf_thumb_gen_create(ap->mf_Status.path->str);
		g_thumb_gen = thumb_gen;
		mf_thumb_gen_realize(thumb_gen);
		mf_thumb_gen_set_file_list(thumb_gen, file_list);
		mf_thumb_gen_set_complete_cb(thumb_gen, mf_util_thumb_gen_complete_cb);
		mf_thumb_gen_set_progress_cb(thumb_gen, mf_util_thumb_gen_progress_cb);
		mf_thumb_gen_set_user_data(thumb_gen, ap);
		mf_thumb_gen_start(thumb_gen);
	}
}

void _mf_thumbnail_completed_cb(thumbnail_util_error_e error, const char *request_id, int raw_width, int raw_height, unsigned char *raw_data, int raw_size, void *user_data)
{
#if 0 //test code
	mf_retm_if(user_data == NULL, "user_data is NULL");
	FILE *fp = NULL;
	fsNodeInfo *pNode = (fsNodeInfo *)user_data;
	if (pNode->thumbnail_path != NULL) {
		fp = fopen(pNode->thumbnail_path, "w");
		if (fp) {
			fwrite(raw_data, 1, raw_size, fp);
			fclose(fp);
		}
	}
#endif
}

