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

#include <Evas.h>
#include "mf-object.h"
#include "mf-dlog.h"
#include "mf-object-conf.h"
#include "mf-callback.h"

Evas_Object *mf_object_create_genlist(Evas_Object *parent)
{
	Evas_Object *genlist = NULL;
	//mf_object_enable_virtualkeypad();
	genlist = elm_genlist_add(parent);
	//elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	//elm_object_focus_set(genlist, EINA_FALSE);
	//elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);//Fix the P130928-00732.
	elm_object_focus_allow_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(genlist);
	return genlist;
}

Evas_Object *mf_object_create_layout(Evas_Object *parent, const char *edj, const char *grp_name)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	mf_retvm_if(edj == NULL, NULL, "edj is NULL");
	mf_retvm_if(grp_name == NULL, NULL, "grp_name is NULL");

	Evas_Object *layout = NULL;
	layout = elm_layout_add(parent);
	mf_retvm_if(layout == NULL, NULL, "layout is NULL");
	elm_layout_file_set(layout, edj, grp_name);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);
	t_end;
	MF_TRACE_END;
	return layout;

}

Evas_Object *mf_object_create_conform(Evas_Object * parent)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retv_if(parent == NULL, NULL);

	Evas_Object *conform;

	conform = elm_conformant_add(parent);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(conform);
	MF_TRACE_END;
	t_end;

	return conform;
}

Evas_Object *mf_object_create_check_box(Evas_Object *parent)
{
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas_Object *checkbox = NULL;
	checkbox = elm_check_add(parent);
	elm_object_focus_set(checkbox, EINA_TRUE);
	evas_object_propagate_events_set(checkbox, EINA_FALSE);
	evas_object_show(checkbox);
	return checkbox;
}

Evas_Object *mf_object_create_no_content(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	t_start;
	Evas_Object *nocontent = elm_layout_add(parent);
	elm_layout_theme_set(nocontent, "layout", "nocontents", "text");
	elm_object_focus_set(nocontent, EINA_FALSE);

	Evas_Object *icon = elm_image_add(nocontent);
	elm_image_file_set(icon, EDJ_IMAGE, IMG_ICON_MULTI_NO_CONTENTS);
	elm_object_part_content_set(nocontent, "nocontents.image", icon);
	evas_object_size_hint_weight_set(nocontent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(nocontent, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//mf_object_text_set(nocontent, MF_LABEL_NO_FILES, "elm.text");  //fix P131223-01999 by ray
	evas_object_show(nocontent);
	MF_TRACE_END;
	t_end;
	return nocontent;
}

Evas_Object *mf_object_create_text_no_content(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	t_start;
	Evas_Object *nocontent = elm_layout_add(parent);
	elm_layout_theme_set(nocontent, "layout", "nocontents", "text");
	elm_object_focus_set(nocontent, EINA_FALSE);

	Evas_Object *icon = elm_image_add(nocontent);
	elm_image_file_set(icon, EDJ_IMAGE, IMG_ICON_TEXT_NO_CONTENTS);
	elm_object_part_content_set(nocontent, "nocontents.image", icon);
	evas_object_size_hint_weight_set(nocontent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(nocontent, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//mf_object_text_set(nocontent, MF_LABEL_NO_FILES, "elm.text");  //fix P131223-01999 by ray

	MF_TRACE_END;
	t_end;
	return nocontent;
}

Evas_Object *mf_object_create_pic_no_content(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	t_start;
	Evas_Object *nocontent = elm_layout_add(parent);
	elm_layout_theme_set(nocontent, "layout", "nocontents", "text");
	elm_object_focus_set(nocontent, EINA_FALSE);

	Evas_Object *icon = elm_image_add(nocontent);
	elm_image_file_set(icon, EDJ_IMAGE, IMG_ICON_IMG_NO_CONTENTS);
	elm_object_part_content_set(nocontent, "nocontents.image", icon);
	evas_object_size_hint_weight_set(nocontent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(nocontent, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//mf_object_text_set(nocontent, MF_LABEL_NO_FILES, "elm.text");  //fix P131223-01999 by ray

	MF_TRACE_END;
	t_end;
	return nocontent;
}

Evas_Object *mf_object_create_multi_no_content(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	t_start;
	Evas_Object *nocontent = elm_layout_add(parent);
	elm_layout_theme_set(nocontent, "layout", "nocontents", "text");
	elm_object_focus_set(nocontent, EINA_FALSE);

	Evas_Object *icon = elm_image_add(nocontent);
	elm_image_file_set(icon, EDJ_IMAGE, IMG_ICON_MULTI_NO_CONTENTS);
	elm_object_part_content_set(nocontent, "nocontents.image", icon);
	evas_object_size_hint_weight_set(nocontent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(nocontent, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//mf_object_text_set(nocontent, MF_LABEL_NO_FILES, "elm.text");  //fix P131223-01999 by ray

	MF_TRACE_END;
	t_end;
	return nocontent;
}

Evas_Object *mf_object_create_dev_no_content(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	t_start;
	Evas_Object *nocontent = elm_layout_add(parent);
	elm_layout_theme_set(nocontent, "layout", "nocontents", "text");
	elm_object_focus_set(nocontent, EINA_FALSE);

	Evas_Object *icon = elm_image_add(nocontent);
	elm_image_file_set(icon, EDJ_IMAGE, IMG_ICON_DEV_NO_CONTENTS);
	elm_object_part_content_set(nocontent, "nocontents.image", icon);
	evas_object_size_hint_weight_set(nocontent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(nocontent, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//mf_object_text_set(nocontent, MF_LABEL_NO_FILES, "elm.text");  //fix P131223-01999 by ray

	MF_TRACE_END;
	t_end;
	return nocontent;
}

void mf_object_disable_virtualkeypad()
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *) mf_get_appdata();
	elm_object_signal_emit(ap->mf_MainWindow.pConformant, "elm,state,virtualkeypad,disable", "");
	elm_object_signal_emit(ap->mf_MainWindow.pConformant, "elm,state,clipboard,disable", "");
	//elm_object_signal_emit(ap->mf_MainWindow.pConformant, "elm,state,virtualkeypad,enable", "");
	//elm_object_signal_emit(ap->mf_MainWindow.pConformant, "elm,state,clipboard,enable", "");
	MF_TRACE_END;
	t_end;
}

void mf_object_enable_virtualkeypad()
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *) mf_get_appdata();
	elm_object_signal_emit(ap->mf_MainWindow.pConformant, "elm,state,virtualkeypad,enable", "");
	elm_object_signal_emit(ap->mf_MainWindow.pConformant, "elm,state,clipboard,enable", "");
	MF_TRACE_END;
	t_end;
}

Evas_Object *mf_object_create_editfield(Evas_Object *parent, Evas_Object **pEntry)
{
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "item/editfield", "default");
	Evas_Object *entry = elm_entry_add(parent);
	elm_object_style_set(entry, "default");
	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
	elm_object_part_content_set(layout, "elm.icon.entry", entry);
	*pEntry = entry;
	return layout;
}

/*static void __mf_object_eraser_clicked_cb(void *data, Evas_Object *obj, void *ei)
{
	mf_retm_if(data == NULL, "data is NULL");
	Evas_Object *entry = data;
	elm_object_focus_set(entry, EINA_TRUE);
	elm_entry_entry_set(entry, "");
}*/

Evas_Object *mf_object_create_panes(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas_Object *panes = NULL;
	panes = elm_panes_add(parent);
	mf_retvm_if(panes == NULL, NULL, "panes is NULL");
	elm_object_focus_set(panes, EINA_FALSE);
	evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_panes_fixed_set(panes, EINA_FALSE);
	evas_object_show(panes);
	MF_TRACE_END;
	return panes;
}

Evas_Object *mf_object_create_layout_main(Evas_Object * parent)
{
	MF_TRACE_BEGIN;
	t_start;
	Evas_Object *layout;

	mf_retv_if(parent == NULL, NULL);

	layout = elm_layout_add(parent);
	mf_retvm_if(layout == NULL, NULL, "Failed elm_layout_add.\n");
	elm_object_focus_set(layout, EINA_FALSE);

	elm_layout_theme_set(layout, "layout", "application", "default");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	//elm_win_resize_object_add(parent, layout);

	evas_object_show(layout);
	MF_TRACE_END;
	t_end;

	return layout;
}

Evas_Object *mf_object_get_part_content(Evas_Object *parent, const char *part)
{
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	mf_retvm_if(part == NULL, NULL, "part is NULL");

	Evas_Object *content = NULL;
	content = elm_object_part_content_get(parent, part);
	return content;
}

Evas_Object *mf_object_unset_part_content(Evas_Object *parent, const char *part)
{
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	mf_retvm_if(part == NULL, NULL, "part is NULL");

	Evas_Object *content = NULL;
	content = elm_object_part_content_unset(parent, part);
	return content;
}

Evas_Object *mf_object_create_button(Evas_Object *parent, const char *style,
                                     const char *caption, Evas_Object *icon,
                                     Evas_Smart_Cb func,
                                     void *data,
                                     Eina_Bool flag_propagate)
{
	if (!parent) {
		return NULL;
	}

	Evas_Object *btn;

	btn = elm_button_add(parent);


	if (style) {
		elm_object_style_set(btn, style);
	}
	if (caption) {
		mf_object_text_set(btn, caption, NULL);
	}

	if (icon) {
		elm_object_content_set(btn, icon);
	}

	evas_object_propagate_events_set(btn, flag_propagate);

	evas_object_smart_callback_add(btn, "clicked", func, (void *)data);
	evas_object_show(btn);

	return btn;
}

void mf_object_panes_right_set(Evas_Object *panes, Evas_Object *content)
{
	mf_retm_if(panes == NULL, "panes is NULL");
	mf_retm_if(content == NULL, "content is NULL");
	elm_object_part_content_set(panes, "right", content);
}

Evas_Object *mf_object_tabbar_create(Evas_Object *parent)
{
	mf_debug("");
	Evas_Object *tabbar = NULL;

	/* create controlbar */
	tabbar = elm_toolbar_add(parent);
	mf_retvm_if(tabbar == NULL, NULL, "Failed to add toolbar");

	elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(tabbar, EINA_FALSE);
	elm_toolbar_transverse_expanded_set(tabbar, EINA_TRUE);
	elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_object_style_set(tabbar, "root_tabbar/item_with_title");

	return tabbar;
}

Evas_Object *mf_object_path_widget_create(Evas_Object *parent)
{
	mf_debug("");
	Evas_Object *obj = NULL;

	obj = elm_toolbar_add(parent);
	mf_retvm_if(obj == NULL, NULL, "Failed to add toolbar");

	elm_object_style_set(obj, "navigationbar");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_SCROLL);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_align_set(obj, 0.0);
	elm_toolbar_homogeneous_set(obj, EINA_FALSE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	return obj;
}

Evas_Object *mf_object_toolbar_create(Evas_Object *parent)
{
	mf_debug("");
	Evas_Object *toolbar = NULL;

	toolbar = elm_toolbar_add(parent);
	mf_retvm_if(toolbar == NULL, NULL, "Failed to add toolbar");

	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);

	elm_object_focus_allow_set(toolbar, EINA_TRUE);
	return toolbar;
}

void mf_object_text_set(Evas_Object *obj, const char *ID, const char *part)
{
	mf_retm_if(ID == NULL, "ID is NULL");
	mf_retm_if(obj == NULL, "obj is NULL");
	const char *domain;

	if (strstr(ID, "IDS_COM")) {
		domain = PKGNAME_SYSTEM;
	} else {
		domain = NULL;    //PKGNAME_MYFILE;
	}

	elm_object_domain_translatable_part_text_set(obj, part, domain, ID);
	//elm_object_domain_part_text_translatable_set(obj, part, domain, EINA_TRUE);
}

Evas_Object *mf_object_create_select_all_btn(Evas_Object *parent, void *data)
{
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");

	Evas_Object *btn = elm_button_add(parent);

	elm_object_style_set(btn, "naviframe/title_icon");
	Evas_Object *icon = elm_icon_add(btn);
	elm_image_file_set(icon, EDJ_NAME, "icon.select_all");
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1 , 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_content_set(btn, icon);
	elm_object_focus_allow_set(btn, EINA_FALSE);
	evas_object_propagate_events_set(btn, EINA_FALSE);

	evas_object_show(btn);

	return btn;
}

Evas_Object *mf_object_create_select_all_checkbox(Evas_Object *parent)
{
	Evas_Object *check = NULL;
	check = elm_check_add(parent);
	elm_object_focus_set(check, EINA_FALSE);
	evas_object_propagate_events_set(check, EINA_FALSE);
	elm_check_state_pointer_set(check, NULL);
	evas_object_show(check);
	return check;
}

void mf_object_entry_unfocus(Evas_Object *entry)
{
	if (entry) {
		elm_object_focus_set(entry, EINA_FALSE);
	}
}

Evas_Object *mf_object_create_box(Evas_Object *parent)
{
	Evas_Object *box = NULL;
	box = elm_box_add(parent);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_horizontal_set(box, EINA_FALSE);
	return box;
}

void mf_object_create_select_all_layout(Evas_Object *pParent, Evas_Smart_Cb pChangeFunc,
                                        Evas_Object_Event_Cb pMouseDownFunc, void *pUserData, Evas_Object **pCheckBox, Evas_Object **pSelectLayout)
{
	Evas_Object *pSelectAllLayout = elm_layout_add(pParent);
	//elm_layout_theme_set(pSelectAllLayout, "genlist", "item", "select_all/default");
	elm_layout_file_set(pSelectAllLayout, EDJ_NAME, "select.all.layout");
	evas_object_size_hint_weight_set(pSelectAllLayout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(pSelectAllLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(pSelectAllLayout, EVAS_CALLBACK_MOUSE_DOWN, pMouseDownFunc, pUserData);
	*pSelectLayout = pSelectAllLayout;

	Evas_Object *pSelectAllCheckbox = elm_check_add(pSelectAllLayout);
	//elm_check_state_pointer_set(pSelectAllCheckbox, (void *)FALSE);
	evas_object_smart_callback_add(pSelectAllCheckbox, "changed", pChangeFunc, pUserData);
	evas_object_propagate_events_set(pSelectAllCheckbox, EINA_FALSE);
	elm_object_part_content_set(pSelectAllLayout, "elm.icon", pSelectAllCheckbox);
	mf_object_text_set(pSelectAllLayout, MF_LABEL_SELECT_ALL, "elm.text");
	evas_object_show(pSelectAllLayout);
	*pCheckBox = pSelectAllCheckbox;
}

void mf_object_box_clear(Evas_Object *box)
{
	mf_retm_if(box == NULL, "box is NULL");
	Eina_List *children = elm_box_children_get(box);
	Eina_List *l = NULL;
	void *child = NULL;
	if (children) {
		EINA_LIST_FOREACH(children, l, child) {
			Evas_Object *obj = (Evas_Object *)child;
			SAFE_FREE_OBJ(obj);
		}
	}
	elm_box_clear(box);
}
