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

#ifndef __MF_PS_GENLIST_H_DEF__
#define __MF_PS_GENLIST_H_DEF__

#include <Elementary.h>
Evas_Object *mf_ps_create_genlist(Evas_Object *parent);
void mf_ps_genlist_item_selected(void *data, Evas_Object * obj, void *event_info);
void mf_ps_genlist_items_append(Evas_Object *parent, Elm_Genlist_Item_Class *itc, Eina_List *file_list, Evas_Smart_Cb select_cb, void *user_data);

#endif //__MF_PS_GENLIST_H_DEF__
