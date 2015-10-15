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

#ifndef __DEF_MYFILE_GENGRID_H
#define __DEF_MYFILE_GENGRID_H

#include "mf-util.h"


#define MF_LANDSCAPE_GENGRID_ITEM_WIDTH		118
#define MF_LANDSCAPE_GENGRID_ITEM_HEIGTH	161

#define MF_ICON_SIZE				(177*MF_SCALE_FACTORY)


#define MF_HD_GENGRID_ITEM_WIDTH		176*MF_SCALE_FACTORY //(int)((8*MF_SCALE_FACTORY)+(164*MF_SCALE_FACTORY)+(8*MF_SCALE_FACTORY))

#define MF_HD_GENGRID_ITEM_HEIGTH		176*MF_SCALE_FACTORY//(int)((18*MF_SCALE_FACTORY)+(164*MF_SCALE_FACTORY)+(72*MF_SCALE_FACTORY))

void mf_gengrid_create_grid_items(void *data, Evas_Object *grid, Eina_List *file_list);

Evas_Object *mf_gengrid_create_grid(Evas_Object *parent);
Evas_Object *mf_gengrid_create(Evas_Object *parent, void *data);

void mf_gengrid_refresh(void *data);
Evas_Object *mf_gengrid_create_list(void *data, Evas_Object *parent);
void mf_gengrid_create_list_default_style(Evas_Object *pGengrid, void *data, Eina_List *dir_list,
				Eina_List *file_list);
void mf_gengrid_get_grid_selected_items(Evas_Object *gengrid, Eina_List **list);
void mf_gengrid_gl_lang_changed(void *data, Evas_Object *obj, void *event_info);
void mf_gengrid_thumbs_longpressed(void *data, Evas_Object *obj, void *event_info);
void mf_gengrid_gen_style_set();
void mf_gengrid_item_del(void *data, Evas_Object * obj);
Evas_Object *mf_gengrid_item_icon_get(void *data, Evas_Object *obj, const char *part);
void mf_gengrid_align_set(Evas_Object *gengrid, int count);
char *mf_gengrid_item_label_get(void *data, Evas_Object * obj, const char *part);
void mf_gengrid_realized(void *data, Evas_Object *obj, void *event_info);

#endif //__DEF_MYFILE_GENGRID_H
