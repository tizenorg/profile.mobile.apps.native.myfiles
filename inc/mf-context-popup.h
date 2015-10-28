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

#ifndef __MF_CONTEXT_POPUP_H_DEF__
#define __MF_CONTEXT_POPUP_H_DEF__

#include <Elementary.h>
#include "mf-object-conf.h"

typedef enum __mf_context_popup_item_type_e mf_context_popup_item_type_e;
enum __mf_context_popup_item_type_e {
	mf_context_popup_item_setting,
	mf_context_popup_item_storage_usage,
	mf_context_popup_item_refresh,
	mf_context_popup_item_download,
	mf_context_popup_item_view_by,
	mf_context_popup_item_sort_by,
	mf_context_popup_item_search,
	mf_context_popup_item_edit,
	mf_context_popup_item_share,
	mf_context_popup_item_new_folder,
	mf_context_popup_item_copy,
	mf_context_popup_item_move,
	mf_context_popup_item_delete,
	mf_context_popup_item_rename,
	mf_context_popup_item_compress,
	mf_context_popup_item_decompress,
	mf_context_popup_item_details,
	mf_context_popup_item_show_hide_hidden,
	mf_context_popup_item_remove_recent,
	mf_context_popup_item_uninstall,
};

void mf_context_popup_create_more(void *data, Evas_Object *parent);
int mf_context_popup_mousedown_cb( void *data, int type, void *event );
void mf_context_popup_get_more_position(Evas_Object *obj, int *x, int *y);
void mf_context_popup_create_share(void *data, int share_mode, Evas_Object *parent);
void mf_context_popup_create_gengrid(void *data);
void mf_context_popup_position_get(int *x, int *y);
Evas_Object *mf_context_popup_search_filter(Evas_Object *parent, void *user_data, Evas_Object *obj);

#endif //__MF_CONTEXT_POPUP_H_DEF__
