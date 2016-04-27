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

#include <stdio.h>
#include <Elementary.h>

#include "mf-main.h"
#include "mf-launch.h"
#include "mf-util.h"


#include "mf-ug-detail-media.h"
#include "mf-ug-detail-dlog.h"
#include "mf-ug-detail.h"
#include "mf-ug-detail-view.h"
#include "mf-edit-view.h"
#include "mf-file-util.h"

void mf_detail_widget_object_text_set(Evas_Object *obj, const char *ID, const char *part)
{
	const char *domain;

	domain = DETAIL_UG_PKGNAME;

	elm_object_domain_translatable_part_text_set(obj, part, domain, ID);
}


void mf_detail_widget_object_item_text_set(Elm_Object_Item *item, const char *ID, const char *part)
{
	const char *domain;

	domain = DETAIL_UG_PKGNAME;
	elm_object_item_domain_translatable_part_text_set(item, part, domain, ID);
}

void mf_detail_widget_object_item_translate_set(Elm_Object_Item *item, const char *ID)
{
	const char *domain;

	domain = DETAIL_UG_PKGNAME;

	elm_object_item_domain_text_translatable_set(item, domain, EINA_TRUE);
}

char *mf_detail_widget_get_text(const char *ID)
{
	char *str;
	str = gettext(ID);
	return str;
}


static void __mf_ug_detail_view_process_genlist(void *data, Evas_Object *genlist);

/******************************
** Prototype    : __mf_ug_detail_view_init_data
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_detail_view_init_data(void *data)
{
	UG_DETAIL_TRACE_BEGIN;
	struct detailData *detail = (struct detailData *)data;
	ug_detail_retm_if(detail == NULL, "detail is NULL");

	detail->mf_Info.contains = NULL;
	detail->mf_Info.filename = NULL;
	detail->mf_Info.filepath = NULL;
	detail->mf_Info.filesize = NULL;
	detail->mf_Info.file_ext = NULL;
	detail->mf_Info.file_location = NULL;
	detail->mf_Info.size = 0;
	detail->mf_Info.create_date = NULL;
	detail->mf_Info.category = NULL;
	detail->mf_Info.resolution = NULL;
	detail->mf_Info.latitude = NULL;
	detail->mf_Info.longitude = NULL;
	detail->mf_Status.path = NULL;
	UG_DETAIL_TRACE_END;
}

void mf_ug_detail_view_init_data(void *data)
{
	__mf_ug_detail_view_init_data(data);
}

/******************************
** Prototype    : __mf_ug_detail_view_destroy_data
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_detail_view_destroy_data(void *data)
{
	UG_DETAIL_TRACE_BEGIN;

	struct detailData *detail = (struct detailData *)data;
	ug_detail_retm_if(detail == NULL, "detail is NULL");

	if (detail->mf_Info.contains) {
		free(detail->mf_Info.contains);
		detail->mf_Info.contains = NULL;
	}
	if (detail->mf_Info.filename) {
		free(detail->mf_Info.filename);
		detail->mf_Info.filename = NULL;
	}
	if (detail->mf_Info.filepath) {
		free(detail->mf_Info.filepath);
		detail->mf_Info.filepath = NULL;
	}
	if (detail->mf_Info.filesize) {
		free(detail->mf_Info.filesize);
		detail->mf_Info.filesize = NULL;
	}
	if (detail->mf_Info.file_ext) {
		free(detail->mf_Info.file_ext);
		detail->mf_Info.file_ext = NULL;
	}
	if (detail->mf_Info.file_location) {
		free(detail->mf_Info.file_location);
		detail->mf_Info.file_location = NULL;
	}
	if (detail->mf_Info.create_date) {
		free(detail->mf_Info.create_date);
		detail->mf_Info.create_date = NULL;
	}
	if (detail->mf_Info.category) {
		g_string_free(detail->mf_Info.category, TRUE);
		detail->mf_Info.category = NULL;
	}
	if (detail->mf_Info.resolution) {
		free(detail->mf_Info.resolution);
		detail->mf_Info.resolution = NULL;
	}
	if (detail->mf_Info.latitude) {
		free(detail->mf_Info.latitude);
		detail->mf_Info.latitude = NULL;
	}
	if (detail->mf_Info.longitude) {
		free(detail->mf_Info.longitude);
		detail->mf_Info.longitude = NULL;
	}
	if (detail->mf_Status.path) {
		g_string_free(detail->mf_Status.path, TRUE);
		detail->mf_Status.path = NULL;
	}

	UG_DETAIL_TRACE_END;
}

void mf_ug_detail_view_destroy_data(void *data)
{
	__mf_ug_detail_view_destroy_data(data);
}

void mf_detail_data_destroy(void *data)
{
	MF_TRACE_BEGIN;
	if (data == NULL) {
		return;
	}

	struct appdata *ap = (struct appdata *)data;
	if (ap->mf_Status.detail) {
		mf_ug_detail_view_destroy_data(ap->mf_Status.detail);
		free(ap->mf_Status.detail);
		ap->mf_Status.detail = NULL;
	}
	MF_TRACE_END;
}

static void _mf_update_size_string(struct detailData *detail, char *filesize)
{
	ug_detail_retm_if(filesize == NULL, "filesize is NULL");
	ug_detail_retm_if(detail == NULL, "detail is NULL");
	char *unit = NULL;
	double size = 0.0;

	if (detail->mf_Info.unit_num == UG_SIZE_BYTE) {
		unit = strdup(mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_SIZE_B));
		if (unit != NULL && strstr(unit, mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_SIZE_B))) {//Sometimes, the string isn't translated.
			size = detail->mf_Info.dsize;
			strncpy(unit , "B", sizeof("B"));
		}

	} else if (detail->mf_Info.unit_num == UG_SIZE_KB) {
		unit = strdup(mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_SIZE_K));
		if (unit != NULL && strstr(unit, mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_SIZE_K))) {//Sometimes, the string isn't translated.
			size = 1024 * detail->mf_Info.dsize;
			strncpy(unit , "KB", sizeof("KB"));
		}
	} else if (detail->mf_Info.unit_num == UG_SIZE_MB) {
		unit = strdup(mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_SIZE_M));
		if (unit != NULL && strstr(unit, mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_SIZE_M))) {//Sometimes, the string isn't translated.
			size = 1024 * 1024 * detail->mf_Info.dsize;
			strncpy(unit , "MB", sizeof("MB"));
		}
	} else if (detail->mf_Info.unit_num == UG_SIZE_GB) {
		unit = strdup(mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_SIZE_G));
		if (unit != NULL && strstr(unit, mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_SIZE_G))) { //Sometimes, the string isn't translated.
			size = 1024 * 1024 * 1024 * detail->mf_Info.dsize;
			strncpy(unit , "GB", sizeof("GB"));
		}
	}

	snprintf(filesize, UG_FILE_SIZE_LEN_MAX, "%.1f %s (%.0f bytes)", detail->mf_Info.dsize, unit, size);
	SAFE_FREE_CHAR(unit);
//	snprintf(filesize, UG_FILE_SIZE_LEN_MAX, "%llu %s", detail->mf_Info.dsize, unit);
}


/******************************
** Prototype    : __mf_ug_detail_view_get_gl_label
** Description  : Samsung
** Input        : const void *data
**                Evas_Object *obj
**                const char *part
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static char *__mf_ug_detail_view_get_gl_label(void *data, Evas_Object *obj, const char *part)
{
	struct MF_LIST_Item_S *params = (struct MF_LIST_Item_S *) data;
	ug_detail_retvm_if(params == NULL, NULL, "params is NULL");

	struct detailData *detail = (struct detailData *)params->detail;
	ug_detail_retvm_if(detail == NULL, NULL, "detail is NULL");

	if (strcmp(part, "elm.text") == 0) {
		if (params->m_ItemName && params->m_ItemName->str) {
			return strdup(mf_detail_widget_get_text(params->m_ItemName->str));
		} else {
			return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
		}
	} else if (strcmp(part, "elm.text.sub") == 0) {
		if (params->m_ItemName == NULL) {
			return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
		}

		if (g_strcmp0(MF_UG_DETAIL_LABEL_NAME, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.filename) {
				return elm_entry_utf8_to_markup(detail->mf_Info.filename);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_SIZE, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.filesize && strlen(detail->mf_Info.filesize) > 0) {
				return strdup(detail->mf_Info.filesize);
			} else if (detail->mf_Info.dsize >= 0) {
				char filesize[256] = {0};
				_mf_update_size_string(detail, filesize);
				return strdup(filesize);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_TOTAL_SIZE, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.dsize >= 0) {
				char filesize[256] = {0};
				_mf_update_size_string(detail, filesize);
				return strdup(filesize);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_FORMAT, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.file_ext) {
				return strdup(detail->mf_Info.file_ext);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_RESOLUTION, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.resolution == NULL) {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			} else {
				return strdup(detail->mf_Info.resolution);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_CONTENTS, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.contains) {
				return strdup(detail->mf_Info.contains);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_CONTAINS, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.contains) {
				return strdup(detail->mf_Info.contains);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_CREATE_DATE, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.create_date) {
				return strdup(detail->mf_Info.create_date);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_LOCATION, params->m_ItemName->str) == 0) {
			//update the file location for multi language
			if (detail != NULL && detail->mf_Status.path != NULL) {//Fixed P131021-04045
				mf_ug_detail_media_get_file_location(detail, detail->mf_Status.path->str);
			}
			if (detail->mf_Info.file_location) {
				return elm_entry_utf8_to_markup(detail->mf_Info.file_location);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_LATITUDE, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.latitude) {
				return strdup(detail->mf_Info.latitude);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else if (g_strcmp0(MF_UG_DETAIL_LABEL_LONGITUDE, params->m_ItemName->str) == 0) {
			if (detail->mf_Info.longitude) {
				return strdup(detail->mf_Info.longitude);
			} else {
				return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		} else {
			return strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
		}
	} else {
		return strdup("");
	}
}

/******************************
** Prototype    : __mf_ug_detail_view_del_gl
** Description  : Samsung
** Input        : const void *data
**                Evas_Object *obj
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_detail_view_del_gl(void *data, Evas_Object * obj)
{

	struct MF_LIST_Item_S *params = (struct MF_LIST_Item_S *) data;
	ug_detail_retm_if(params == NULL, "params is NULL");

	if (params->m_ItemName) {
		g_string_free(params->m_ItemName, TRUE);
		params->m_ItemName = NULL;
	}
	return;
}

/******************************
** Prototype    : __mf_ug_detail_view_get_gl_label_title
** Description  : Samsung
** Input        : const void *data
**                Evas_Object *obj
**                const char *part
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static char *__mf_ug_detail_view_get_gl_label_title(void *data, Evas_Object *obj, const char *part)
{
	UG_DETAIL_TRACE_BEGIN;
	if (!strcmp(part, "elm.text.main")) {
		return strdup(data);
	}

	return NULL;
}

/******************************
** Prototype    : __mf_ug_detail_view_get_gl_state
** Description  : Samsung
** Input        : const void *data
**                Evas_Object *obj
**                const char *part
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static Eina_Bool __mf_ug_detail_view_get_gl_state(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void __mf_ug_genlist_lang_changed(void *data, Evas_Object *obj, void *ei)
{
	UG_DETAIL_TRACE_BEGIN;
	ug_detail_retm_if(data == NULL, "data is NULL");
	ug_detail_retm_if(obj == NULL, "obj is NULL");
	//elm_genlist_realized_items_update(obj);
	elm_genlist_clear(obj);
	__mf_ug_detail_view_process_genlist(data, obj);
}


/******************************
** Prototype    : mf_ug_detail_view_create_genlist
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
Evas_Object *
mf_ug_detail_view_create_genlist(Evas_Object *parent, void *data)
{

	UG_DETAIL_TRACE_BEGIN;
	ug_detail_retvm_if(parent == NULL, NULL, "parent is NULL");
	ug_detail_retvm_if(data == NULL, NULL,  "data is NULL");
	struct detailData *detail = (struct detailData *) data;

	Evas_Object *genlist = NULL;

	genlist = elm_genlist_add(parent);
	ug_detail_retvm_if(genlist == NULL, NULL, "genlist is NULL");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	evas_object_smart_callback_add(genlist, "language,changed",
	                               __mf_ug_genlist_lang_changed, detail);

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	detail->mf_Status.itc.item_style = "type1";
	detail->mf_Status.itc.func.text_get = __mf_ug_detail_view_get_gl_label;
	detail->mf_Status.itc.func.content_get = NULL;
	detail->mf_Status.itc.func.state_get = __mf_ug_detail_view_get_gl_state;
	detail->mf_Status.itc.func.del = __mf_ug_detail_view_del_gl;

	/* Set item class for dialogue group seperator*/
	detail->mf_Status.seperator_itc.item_style = "group_index";
	detail->mf_Status.seperator_itc.func.text_get = NULL;
	detail->mf_Status.seperator_itc.func.content_get = NULL;
	detail->mf_Status.seperator_itc.func.state_get = NULL;
	detail->mf_Status.seperator_itc.func.del = NULL;

	/* Set item class for dialogue group title*/
	detail->mf_Status.title_itc.item_style = "group_index";
	detail->mf_Status.title_itc.func.text_get = __mf_ug_detail_view_get_gl_label_title;
	detail->mf_Status.title_itc.func.content_get = NULL;
	detail->mf_Status.title_itc.func.state_get = NULL;
	detail->mf_Status.title_itc.func.del = NULL;

	detail->mf_Status.multiline_itc.item_style = "type1";
	detail->mf_Status.multiline_itc.func.text_get = __mf_ug_detail_view_get_gl_label;
	detail->mf_Status.multiline_itc.func.content_get = NULL;
	detail->mf_Status.multiline_itc.func.state_get = NULL;
	detail->mf_Status.multiline_itc.func.del = NULL;

	//detail->mf_MainWindow.pContent = genlist;
	UG_DETAIL_TRACE_END;
	return genlist;
}

/******************************
** Prototype    : __mf_ug_detail_view_create_genlist_folder
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_detail_view_create_genlist_folder(void *data, Evas_Object *genlist)
{

	UG_DETAIL_TRACE_BEGIN;
	struct detailData *detail = (struct detailData *)data;
	ug_detail_retm_if(detail == NULL, "detail is NULL");

	int index = 0;
	//Elm_Object_Item *git = NULL;
	Elm_Object_Item *it = NULL;
	struct MF_LIST_Item_S *m_TempItem = NULL;

	ug_detail_retm_if(genlist == NULL, "genlist is NULL");

	char *m_ItemName[5] = {NULL,};

	m_ItemName[0] = MF_UG_DETAIL_LABEL_NAME;
	m_ItemName[1] = MF_UG_DETAIL_LABEL_SIZE;
	m_ItemName[2] = MF_UG_DETAIL_LABEL_CREATE_DATE;
	m_ItemName[3] = MF_UG_DETAIL_LABEL_CONTENTS;
	m_ItemName[4] = MF_UG_DETAIL_LABEL_LOCATION;

	for (index = 0; index < 5; index++) {
		m_TempItem = (struct MF_LIST_Item_S *) malloc(sizeof(struct MF_LIST_Item_S));
		if (m_TempItem == NULL) {
			continue;
		}

		memset(m_TempItem, 0, sizeof(struct MF_LIST_Item_S));

		m_TempItem->m_ItemName = g_string_new(m_ItemName[index]);
		m_TempItem->detail = detail;
		if (index == 0 || index == 4) {
			it = elm_genlist_item_append(genlist, &detail->mf_Status.multiline_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			it = elm_genlist_item_append(genlist, &detail->mf_Status.itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}
	}
	UG_DETAIL_TRACE_END;
}


static void __mf_ug_detail_view_create_genlist_multi(void *data, Evas_Object *genlist)
{

	UG_DETAIL_TRACE_BEGIN;
	struct detailData *detail = (struct detailData *)data;
	ug_detail_retm_if(detail == NULL, "detail is NULL");

	int index = 0;
	//Elm_Object_Item *git = NULL;
	Elm_Object_Item *it = NULL;
	struct MF_LIST_Item_S *m_TempItem = NULL;

	ug_detail_retm_if(genlist == NULL, "genlist is NULL");

	char *m_ItemName[2] = {NULL,};

	m_ItemName[0] = MF_UG_DETAIL_LABEL_TOTAL_SIZE;
	m_ItemName[1] = MF_UG_DETAIL_LABEL_CONTAINS;

	for (index = 0; index < 2; index++) {
		m_TempItem = (struct MF_LIST_Item_S *) malloc(sizeof(struct MF_LIST_Item_S));
		if (m_TempItem == NULL) {
			continue;
		}

		memset(m_TempItem, 0, sizeof(struct MF_LIST_Item_S));

		m_TempItem->m_ItemName = g_string_new(m_ItemName[index]);
		m_TempItem->detail = detail;
		it = elm_genlist_item_append(genlist, &detail->mf_Status.itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}
	UG_DETAIL_TRACE_END;
}

/******************************
** Prototype    : __mf_ug_detail_view_create_genlist_gps
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_detail_view_create_genlist_gps(void *data)
{
	/*
		UG_DETAIL_TRACE_BEGIN;
		struct detailData *detail = (struct detailData *)data;
		ug_detail_retm_if(detail == NULL, "detail is NULL");

		Evas_Object *genlist = NULL;
		int index = 0;
		Elm_Object_Item *git = NULL;
		Elm_Object_Item *it = NULL;
		struct MF_LIST_Item_S *m_TempItem = NULL;
		char *m_ItemName[FILE_EXIF_INFO_NUM] = { '\0' };

		ug_detail_retm_if(genlist == NULL, "genlist is NULL");

		m_ItemName[0] = MF_UG_DETAIL_LABEL_LATITUDE;
		m_ItemName[1] = MF_UG_DETAIL_LABEL_LONGITUDE;

		git = elm_genlist_item_append(genlist, &detail->mf_Status.title_itc, strdup(mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_INFORMATION)),
					      NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		for (index = 0; index < FILE_EXIF_INFO_NUM; index++) {
			m_TempItem = (struct MF_LIST_Item_S *) malloc(sizeof(struct MF_LIST_Item_S));
			if (m_TempItem == NULL)
				continue;

			memset(m_TempItem, 0, sizeof(struct MF_LIST_Item_S));
			m_TempItem->m_ItemName = g_string_new(m_ItemName[index]);
			m_TempItem->detail = detail;
			it = elm_genlist_item_append(genlist, &detail->mf_Status.itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		}
		UG_DETAIL_TRACE_END;
		*/
}

/******************************
** Prototype    : __mf_ug_detail_view_create_genlist_file
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_detail_view_create_genlist_file(void *data, Evas_Object *genlist)
{

	UG_DETAIL_TRACE_BEGIN;
	struct detailData *detail = (struct detailData *)data;
	ug_detail_retm_if(detail == NULL, "detail is NULL");

	int index = 0;

	Elm_Object_Item *git = NULL;
	Elm_Object_Item *it = NULL;
	struct MF_LIST_Item_S *m_TempItem = NULL;

	ug_detail_retm_if(genlist == NULL, "genlist is NULL");

	if (detail->mf_Status.view_type == VIEW_FILE_NORMAL
	   ) {

		char *m_ItemName[FOLDER_NORMDETILES_NUM] = { '\0' };
		m_ItemName[0] = MF_UG_DETAIL_LABEL_NAME;
		m_ItemName[1] = MF_UG_DETAIL_LABEL_SIZE;
		//m_ItemName[2] = MF_UG_DETAIL_LABEL_FORMAT;
		m_ItemName[2] = MF_UG_DETAIL_LABEL_CREATE_DATE;
		m_ItemName[3] = MF_UG_DETAIL_LABEL_LOCATION;
		for (index = 0; index < FOLDER_NORMDETILES_NUM; index++) {
			m_TempItem = (struct MF_LIST_Item_S *) malloc(sizeof(struct MF_LIST_Item_S));
			if (m_TempItem == NULL) {
				continue;
			}

			memset(m_TempItem, 0, sizeof(struct MF_LIST_Item_S));

			m_TempItem->m_ItemName = g_string_new(m_ItemName[index]);
			m_TempItem->detail = detail;
			if (index == 0 || index == 4) {
				it = elm_genlist_item_append(genlist, &detail->mf_Status.multiline_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE,
				                             NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				it = elm_genlist_item_append(genlist, &detail->mf_Status.itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE,
				                             NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			}
		}

	} else if (
	    detail->mf_Status.view_type == VIEW_FILE_IMAGE
	    || detail->mf_Status.view_type == VIEW_FILE_VIDEO) {

		char *m_ItemName[FILE_DETILES_NUM] = { '\0' };
		m_ItemName[0] = MF_UG_DETAIL_LABEL_NAME;
		m_ItemName[1] = MF_UG_DETAIL_LABEL_SIZE;
		//m_ItemName[2] = MF_UG_DETAIL_LABEL_FORMAT;
		//m_ItemName[3] = MF_UG_DETAIL_LABEL_RESOLUTION;
		m_ItemName[2] = MF_UG_DETAIL_LABEL_CREATE_DATE;
		m_ItemName[3] = MF_UG_DETAIL_LABEL_LOCATION;
		for (index = 0; index < FILE_DETILES_NUM; index++) {
			m_TempItem = (struct MF_LIST_Item_S *) malloc(sizeof(struct MF_LIST_Item_S));
			if (m_TempItem == NULL) {
				continue;
			}

			memset(m_TempItem, 0, sizeof(struct MF_LIST_Item_S));

			m_TempItem->m_ItemName = g_string_new(m_ItemName[index]);
			m_TempItem->detail = detail;
			if (index == 0 || index == FILE_DETILES_NUM - 1) {
				it = elm_genlist_item_append(genlist, &detail->mf_Status.multiline_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE,
				                             NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				it = elm_genlist_item_append(genlist, &detail->mf_Status.itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE,
				                             NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			}
		}

	} else {

		char *m_ItemName[FILE_DETILES_NUM] = {NULL,};
		m_ItemName[0] = MF_UG_DETAIL_LABEL_NAME;
		m_ItemName[1] = MF_UG_DETAIL_LABEL_SIZE;
		//m_ItemName[2] = MF_UG_DETAIL_LABEL_FORMAT;
		//m_ItemName[3] = MF_UG_DETAIL_LABEL_RESOLUTION;
		m_ItemName[2] = MF_UG_DETAIL_LABEL_CREATE_DATE;
		m_ItemName[3] = MF_UG_DETAIL_LABEL_LOCATION;

		git = elm_genlist_item_append(genlist, &detail->mf_Status.title_itc, strdup(mf_detail_widget_get_text(MF_UG_DETAIL_LABEL_BASIC)),
		                              NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		for (index = 0; index < FILE_DETILES_NUM; index++) {
			m_TempItem = (struct MF_LIST_Item_S *) malloc(sizeof(struct MF_LIST_Item_S));
			if (m_TempItem == NULL) {
				continue;
			}

			memset(m_TempItem, 0, sizeof(struct MF_LIST_Item_S));

			m_TempItem->m_ItemName = g_string_new(m_ItemName[index]);
			m_TempItem->detail = detail;

			if (index == 0 || index == FILE_DETILES_NUM - 1) {
				it = elm_genlist_item_append(genlist, &detail->mf_Status.multiline_itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE,
				                             NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				it = elm_genlist_item_append(genlist, &detail->mf_Status.itc, m_TempItem, NULL, ELM_GENLIST_ITEM_NONE,
				                             NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			}
		}
		__mf_ug_detail_view_create_genlist_gps(detail);
	}
	UG_DETAIL_TRACE_END;
}

/******************************
** Prototype    : __mf_ug_detail_view_process_genlist
** Description  : Samsung
** Input        : void *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static void __mf_ug_detail_view_process_genlist(void *data, Evas_Object *genlist)
{

	UG_DETAIL_TRACE_BEGIN;
	struct detailData *detail = (struct detailData *)data;
	ug_detail_retm_if(detail == NULL, "detail is NULL");

	switch (detail->mf_Status.view_type) {
	case VIEW_DIR:
		__mf_ug_detail_view_create_genlist_folder(detail, genlist);
		break;
	case VIEW_FILE_NORMAL:
	case VIEW_FILE_IMAGE:
	case VIEW_FILE_VIDEO:
		__mf_ug_detail_view_create_genlist_file(detail, genlist);
		break;
	case VIEW_FILE_MULTI:
		__mf_ug_detail_view_create_genlist_multi(detail, genlist);
		break;
	default:
		break;
	}
	UG_DETAIL_TRACE_END;
}

void
mf_ug_detail_view_process_genlist(void *data, Evas_Object *genlist)
{
	__mf_ug_detail_view_process_genlist(data, genlist);
}
/******************************
** Prototype    : __mf_ug_detail_view_create_navibar
** Description  : Samsung
** Input        : struct detailData *data
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static int __mf_ug_detail_get_file_view_type(const char *path)
{
	UG_DETAIL_TRACE_BEGIN;
	ug_detail_retvm_if(path == NULL, VIEW_NONE, "path is NULL");

	int view_type = VIEW_NONE;
	{
		File_Type category = FILE_TYPE_NONE;
		mf_ug_detail_fs_get_file_type(path, &category);
		if (category == FILE_TYPE_VIDEO) {
			view_type = VIEW_FILE_VIDEO;
		} else if (category == FILE_TYPE_IMAGE) {
			view_type = VIEW_FILE_IMAGE;
		} else {
			view_type = VIEW_FILE_NORMAL;
		}
	}
	return view_type;
}

static void __senior_info_get(void *data)
{
	UG_DETAIL_TRACE_BEGIN;
	struct detailData *detail = (struct detailData *)data;
	ug_detail_retm_if(detail == NULL, "detail is NULL");
	ug_detail_retm_if(detail->mf_Status.path == NULL, "detail->mf_Status.path is NULL");
	ug_detail_retm_if(detail->mf_Status.path->str == NULL, "detail->mf_Status.path->str is NULL");

	switch (detail->mf_Status.view_type) {
	case VIEW_FILE_IMAGE:
	case VIEW_FILE_VIDEO:
		mf_ug_detail_media_get_exif_info(detail);
		break;
	default:
		break;
	}
	UG_DETAIL_TRACE_END;
}

void mf_ug_detail_get_params_path(Eina_List **dest_list, const char *path_list)
{
	ug_detail_retm_if(dest_list == NULL, "dest_list is NULL");

	gchar **result = NULL;
	gchar **params = NULL;
	result = g_strsplit(path_list, ";", 0);

	for (params = result; *params; params++) {
		*dest_list = eina_list_append(*dest_list, strdup(*params));
	}

	g_strfreev(result);
}

void mf_ug_detail_get_multi_information(Eina_List *list, int *total_file_count,
                                        int *total_folder_count,
                                        unsigned long long int *total_size)
{
	Eina_List *l = NULL;
	char *file_path = NULL;
	int file_count = 0;
	int folder_count = 0;
	unsigned long long int size = 0;
	EINA_LIST_FOREACH(list, l, file_path) {
		if (file_path) {
			if (mf_is_dir(file_path)) {
				folder_count++;
				size += mf_ug_detail_fs_get_folder_size(file_path);
			} else {
				file_count++;
				Node_Info *pNode = (Node_Info *) malloc(sizeof(Node_Info));
				if (pNode == NULL) {
					return;
				}

				memset(pNode, 0, sizeof(Node_Info));
				mf_ug_detaill_fs_get_file_stat(file_path, &pNode);
				size += (unsigned long long int)pNode->size;
				free(pNode);
				pNode = NULL;
			}
			free(file_path);
			file_path = NULL;
		}
	}
	eina_list_free(list);
	list = NULL;

	*total_size = size;
	*total_file_count = file_count;
	*total_folder_count = folder_count;
}

struct detailData *
mf_ug_detail_multi_info_extract(const char *path)
{
	UG_DETAIL_TRACE_BEGIN;
	ug_detail_retvm_if(path == NULL, false, "path is null");

	Eina_List *path_list = NULL;
	int file_count = 0;
	int folder_count = 0;
	unsigned long long int original_size = 0;
	double size = 0;
	int count = 0;

	struct detailData *data = malloc(sizeof(struct detailData));
	ug_detail_retvm_if(data == NULL, NULL, "malloc detail data failed");

	memset(data, 0, sizeof(struct detailData));
	mf_ug_detail_view_init_data(data);

	mf_ug_detail_get_params_path(&path_list, path);
	mf_ug_detail_get_multi_information(path_list, &file_count,
	                                   &folder_count, &original_size);
	size = (double)original_size;

	while (size >= BASIC_SIZE) {
		size /= BASIC_SIZE;
		count++;
	}

	data->mf_Info.dsize = size;
	data->mf_Info.unit_num = count;

	char * buf = NULL;
	if (file_count == 0) {
		buf = g_strdup_printf("%d %s", folder_count, MF_UG_DETAIL_LABELL_FOLDERS);
	} else if (folder_count == 0) {
		buf = g_strdup_printf("%d %s", file_count, MF_UG_DETAIL_LABELL_FILES);
	} else {
		buf = g_strdup_printf("%d %s %d %s", file_count, MF_UG_DETAIL_LABELL_FILES, folder_count, MF_UG_DETAIL_LABELL_FOLDERS);
	}

	data->mf_Info.contains = buf;

	data->mf_Status.view_type = VIEW_FILE_MULTI;

	return data;
}

struct detailData *
mf_ug_detail_common_info_extract(const char *path)
{
	UG_DETAIL_TRACE_BEGIN;
	ug_detail_retvm_if(path == NULL, NULL, "path is NULL");

	struct detailData *data = malloc(sizeof(struct detailData));
	ug_detail_retvm_if(data == NULL, NULL, "malloc detail data failed");

	memset(data, 0, sizeof(struct detailData));
	mf_ug_detail_view_init_data(data);

	data->mf_Status.path = g_string_new(path);

	View_Style view_type = VIEW_NONE;

	if (mf_ug_detail_fs_is_dir(path)) {
		view_type = VIEW_DIR;
	} else {
		view_type = __mf_ug_detail_get_file_view_type(path);
	}

	/*get view type*/
	data->mf_Status.view_type = view_type;
	/*get file bacis information*/
	mf_ug_detail_media_get_common_info(data, (char*)path);
	/*get senior information*/
	__senior_info_get(data);

	UG_DETAIL_TRACE_END;

	return data;
}
