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

#ifndef _MF_THUMB_GEN_H_
#define _MF_THUMB_GEN_H_

#include <sys/time.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <app.h>
#include <Elementary.h>

#define MYFILE_THUMBNAIL_REFRESH_STORAGETYPE MYFILE_PHONE

typedef	void *mf_thumb_gen_h;
typedef void(* mf_thumb_gen_progress_cb)(bool bSucess, int nIdx, void *pUserData);
typedef void(* mf_thumb_gen_complete_cb)(int nItemCount, void *pUserData);
typedef void(* mf_thumb_gen_cancel_cb)(void *pUserData);

#define MF_STRDUP(destptr,sourceptr) \
		do {\
			if (sourceptr == NULL) \
				destptr = NULL; \
			else \
				destptr = strdup(sourceptr); \
		} while (0);


mf_thumb_gen_h mf_thumb_gen_create(const char *szMediaURL);
void mf_thumb_gen_destroy(mf_thumb_gen_h hThumbGen);

bool mf_thumb_gen_realize(mf_thumb_gen_h hThumbGen);
bool mf_thumb_gen_unrealize(mf_thumb_gen_h hThumbGen);

bool mf_thumb_gen_is_realize(mf_thumb_gen_h hThumbGen, bool *bIsRealize);

bool mf_thumb_gen_set_save_directory(mf_thumb_gen_h hThumbGen, char *szSaveDir);
bool mf_thumb_gen_set_file_list(mf_thumb_gen_h hThumbGen, Eina_List *file_list);


bool mf_thumb_gen_start(mf_thumb_gen_h hThumbGen);
bool mf_thumb_gen_cancel(mf_thumb_gen_h hThumbGen);


bool mf_thumb_gen_set_user_data(mf_thumb_gen_h hThumbGen, void *pUserData);
bool mf_thumb_gen_set_progress_cb(mf_thumb_gen_h hThumbGen, mf_thumb_gen_progress_cb progress_cb);
bool mf_thumb_gen_set_complete_cb(mf_thumb_gen_h hThumbGen, mf_thumb_gen_complete_cb complete_cb);
bool mf_thumb_gen_set_cancel_cb(mf_thumb_gen_h hThumbGen, mf_thumb_gen_cancel_cb cancel_cb);

void mf_view_refresh_thumbnail_destroy();
void mf_view_refresh_thumbnail_for_other_memory(void *data, Eina_List* file_list);

#endif // _MF_THUMB_GEN_H_


