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

#ifndef _MF_SEARCH_VIEW_H_DEF__
#define _MF_SEARCH_VIEW_H_DEF__

#include <Elementary.h>
#include "mf-object-conf.h"
#include "mf-search.h"

void mf_search_bar_enter_search_routine(void *data, Evas_Object *obj, void *event_info);
void mf_search_bar_search_started_callback(void *data, Evas_Object *obj, void *event_info);
void mf_search_bar_category_search(void *data, int category);
void mf_search_view_create(void *data);
void mf_search_bar_stop(void *data);
void mf_search_bar_view_update(void *data);
void mf_search_bar_set_content(void *data, Evas_Object *pLayout, Evas_Object *NaviContent, bool is_no_content);
void mf_entry_focus_allow_idler_destory();
Eina_Bool mf_search_view_back_cb(void *data, Elm_Object_Item *it);
void mf_search_bar_content_create(mf_search_result_t *result, void *user_data);
void mf_search_view_orientation_get(void *data, Evas_Object *obj, void *event_info);

#endif //_MF_SEARCH_VIEW_H_DEF__
