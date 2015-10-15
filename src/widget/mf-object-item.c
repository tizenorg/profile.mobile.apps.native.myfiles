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

#include "mf-object-item.h"
#include "mf-object-conf.h"
#include "mf-dlog.h"
#include "mf-util.h"
#include "mf-main.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-view.h"
#include "mf-file-util.h"

extern Elm_Gengrid_Item_Class gic;

void mf_object_item_text_set(Elm_Object_Item *item, const char *ID, const char *part)
{
	MF_TRACE_BEGIN;
	mf_retm_if(ID == NULL, "ID is NULL");
	mf_retm_if(item == NULL, "item is NULL");
	const char *domain;

	if (strstr(ID, "IDS_COM"))
		domain = PKGNAME_SYSTEM;
	else
		domain = NULL;//PKGNAME_MYFILE;
	elm_object_item_domain_translatable_part_text_set(item, part, domain, ID);
	MF_TRACE_END;
}

void mf_object_item_translate_set(Elm_Object_Item *item, const char *ID)
{
	mf_retm_if(ID == NULL, "ID is NULL");
	mf_retm_if(item == NULL, "item is NULL");
	const char *domain;

	if (strstr(ID, "IDS_COM"))
		domain = PKGNAME_SYSTEM;
	else
		domain = NULL;

	SECURE_DEBUG(">>>>>>>>>>>>>>> ID is [%s] domain is [%s]", ID, domain);
	elm_object_item_domain_text_translatable_set(item, domain, EINA_TRUE);
}

Elm_Object_Item *mf_object_item_tabbar_item_append(Evas_Object *obj,
		                        const char *icon,
		                        const char *label,
		                        Evas_Smart_Cb func,
		                        const void *data)
{
	Elm_Object_Item *item = elm_toolbar_item_append(obj, icon, label, func, data);

	mf_object_item_translate_set(item, label);
	return item;
}

void mf_object_item_tabbar_item_set_disable(Evas_Object *obj, const char *label, Eina_Bool disable)
{
	mf_retm_if(obj == NULL, "obj is NULL");
	mf_retm_if(label == NULL, "label is NULL");

	Elm_Object_Item *item = elm_toolbar_first_item_get(obj);
	const char *button_label = NULL;

	while (item) {
		button_label = elm_object_item_part_text_get(item, NAVI_CTRL_TEXT_PART);
		if (g_strcmp0(button_label, mf_util_get_text(label)) == 0
		    || g_strcmp0(button_label, (label)) == 0)
			elm_object_item_disabled_set(item, disable);
		button_label = NULL;
		item = elm_toolbar_item_next_get(item);
	}
}

void mf_object_item_part_content_remove(Elm_Object_Item *item, const char *part)
{
	MF_TRACE_BEGIN;
	mf_retm_if(item == NULL, "item is NULL");
	mf_retm_if(part == NULL, "part is NULL");

	Evas_Object *removed = NULL;
	removed = elm_object_item_part_content_unset(item, part);
	SAFE_FREE_OBJ(removed);

	MF_TRACE_END;
}

mfItemData_s *mf_object_item_normal_data_get(const char *fullpath, void *user_data, int list_type)
{
	mf_retvm_if(fullpath == NULL, NULL, "fullpath error");
	mf_retvm_if(user_data == NULL, NULL, "user_data error");
	struct appdata *ap = (struct appdata *)user_data;
	mfItemData_s *item_data = NULL;
	i18n_udate date = 0;
	off_t size = 0;

	item_data = (mfItemData_s *) calloc(sizeof(mfItemData_s), sizeof(char));
	if (item_data == NULL) {
		return NULL;
	}
	item_data->m_ItemName = g_string_new(fullpath);
	if (!mf_is_dir(fullpath)) {
		mf_file_attr_get_file_size(fullpath, &size);
		mf_file_attr_get_file_size_info(&(item_data->size), size);
	}

	mf_file_attr_get_file_mdate(fullpath, &date);
	item_data->modify_time = date;
	item_data->m_checked = FALSE;
	item_data->pCheckBox = NULL;
	item_data->thumb_path = NULL;
	item_data->real_thumb_flag = FALSE;
	item_data->media = NULL;
	item_data->ap = ap;
	if (mf_is_dir(fullpath)) {
		item_data->file_type = FILE_TYPE_DIR;
	} else {
		mf_file_attr_get_file_category(fullpath, &item_data->file_type);
	}
	item_data->storage_type = mf_fm_svc_wrapper_get_location(fullpath);
	item_data->list_type = list_type;
	return item_data;

}

Elm_Object_Item *mf_object_list_item_append(Evas_Object *parent,
				   void *obj_data,
				   void (*func)(void *data, Evas_Object * obj, void *event_info),
				   void *func_data,
				   void *user_data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(parent == NULL, NULL, "parent error");
	mf_retvm_if(obj_data == NULL, NULL, "obj_data error");
	struct appdata *ap = (struct appdata *)user_data;
	Elm_Object_Item *it = NULL;
	int view_style = mf_view_style_get(ap);

	if (view_style != MF_VIEW_STYLE_THUMBNAIL) {
		Elm_Genlist_Item_Class *itc = NULL;

		if (ap->mf_Status.view_type == mf_view_root_category) {
			if (ap->mf_Status.more != MORE_SEARCH) {
				itc = ap->mf_gl_style.categoryitc;
			} else {
				itc = ap->mf_gl_style.search_itc;
			}
		} else {
			mfItemData_s *item_data = (mfItemData_s *)obj_data;
			if (item_data->file_type != FILE_TYPE_DIR) {
				itc = ap->mf_gl_style.itc;
			} else {
				itc = ap->mf_gl_style.userfolderitc;
			}
		}
		it = elm_genlist_item_append(parent, itc, obj_data, NULL, ELM_GENLIST_ITEM_NONE, func, func_data);
	} else {
		Elm_Gengrid_Item_Class *gitc = NULL;
//Prevent issue fix
              gitc = &gic;
	/*	if (ap->mf_Status.view_type == mf_view_root_category) {
			gitc = &gic;
		} else {
			gitc = &gic;
		}*/
		it = elm_genlist_item_append(parent, gitc, obj_data, NULL, ELM_GENLIST_ITEM_NONE, func, func_data);

	}
	return it;
}

Elm_Object_Item *mf_object_item_genlist_x_y_item_get(Evas_Object *genlist, int x, int y)
{
	Elm_Object_Item *it = NULL;

	if (genlist) {
		int posret = 0;
		it =elm_genlist_at_xy_item_get(genlist, x, y, &posret);
	}
	return it;
}

void mf_object_item_gengrid_current_page_get(Evas_Object *gengrid, int *x, int *y)
{

	if (gengrid) {
		//elm_gengrid_current_page_get(gengrid, x, y);
	}
}

