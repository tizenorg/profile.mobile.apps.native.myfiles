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

#ifndef __MF_OBJECT_ITEM_H_DEF__
#define __MF_OBJECT_ITEM_H_DEF__
#include <Elementary.h>

#include "mf-object-conf.h"

void mf_object_item_text_set(Elm_Object_Item *item, const char *ID, const char* part);
void mf_object_item_translate_set(Elm_Object_Item *item, const char *ID);
Elm_Object_Item *mf_object_item_tabbar_item_append(Evas_Object *obj,
		                        const char *icon,
		                        const char *label,
		                        Evas_Smart_Cb func,
		                        const void *data);
void mf_object_item_part_content_remove(Elm_Object_Item *item, const char *part);
void mf_object_item_tabbar_item_set_disable(Evas_Object *obj, const char *label, Eina_Bool disable);
mfItemData_s *mf_object_item_normal_data_get(const char *fullpath, void *user_data, int list_type);
Elm_Object_Item *mf_object_item_genlist_x_y_item_get(Evas_Object *genlist, int x, int y);
void mf_object_item_gengrid_current_page_get(Evas_Object *gengrid, int *x, int *y);

#endif //__MF_OBJECT_ITEM_H_DEF__
