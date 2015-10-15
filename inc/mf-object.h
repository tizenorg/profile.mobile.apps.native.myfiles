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

#ifndef __MF_OBJECT_H_DEF__
#define __MF_OBJECT_H_DEF__

#include <Elementary.h>
#include <Evas.h>

Evas_Object *mf_object_create_select_all_btn(Evas_Object *parent,void *data);
Evas_Object *mf_object_create_no_content(Evas_Object *parent);
Evas_Object *mf_object_create_conform(Evas_Object * parent);
Evas_Object *mf_object_create_layout(Evas_Object *parent, const char *edj, const char *grp_name);
Evas_Object *mf_object_create_genlist(Evas_Object *parent);
Evas_Object *mf_object_create_button(Evas_Object *parent, const char *style,
				   const char *caption, Evas_Object *icon,
				   Evas_Smart_Cb func,
				   void *data,
				   Eina_Bool flag_propagate);

void mf_object_panes_right_set(Evas_Object *panes, Evas_Object *content);
Evas_Object *mf_object_create_editfield(Evas_Object *parent, Evas_Object **pEntry);
Evas_Object *mf_object_unset_part_content(Evas_Object *parent, const char *part);
Evas_Object *mf_object_create_multi_no_content(Evas_Object *parent);
void mf_object_box_clear(Evas_Object *box);

/*************** Widget *********************/



void mf_object_create_entryfield(void *data,
			   Evas_Object *parent,
			   void (*changed_cb)(void *data, Evas_Object *obj, void *event_info),
			   void (*click_cb)(void *data, Evas_Object *obj, void *event_info));
Evas_Object *mf_object_create_layout_main(Evas_Object * parent);
Evas_Object *mf_object_create_panes(Evas_Object *parent);
Evas_Object *mf_object_get_part_content(Evas_Object *parent, const char *part);
void mf_object_text_set(Evas_Object *obj, const char *ID, const char* part);
Evas_Object *mf_object_tabbar_create(Evas_Object *parent);
Evas_Object *mf_object_toolbar_create(Evas_Object *parent);
Evas_Object *mf_object_create_select_all_checkbox(Evas_Object *parent);
void mf_object_entry_unfocus(Evas_Object *entry);
Evas_Object *mf_object_path_widget_create(Evas_Object *parent);
Evas_Object *mf_object_create_box(Evas_Object *parent);
void mf_object_create_select_all_layout(Evas_Object *pParent, Evas_Smart_Cb pChangeFunc,
	Evas_Object_Event_Cb pMouseDownFunc, void *pUserData, Evas_Object **pCheckBox, Evas_Object **pSelectLayout);
void mf_object_box_clear(Evas_Object *box);

#endif //__MF_OBJECT_H_DEF__
