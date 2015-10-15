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

#ifndef __MF_OBJECT_CONF_DEF__
#define __MF_OBJECT_CONF_DEF__

#include <glib.h>
#include <media_content.h>
#include <Elementary.h>
#include "mf-resource.h"
#include "mf-fs-util.h"

#define NAVI_CTRL_PART			"toolbar"
#define NAVI_CTRL_TEXT_PART		"elm.text"
#define NAVI_MORE_BUTTON_PART		"toolbar_more_btn"
#define NAVI_BOTTOM_BUTTON_1_PART	"toolbar_button1"
#define NAVI_BOTTOM_BUTTON_2_PART	"toolbar_button2"
#define NAVI_BUTTON_EDIT		"naviframe/more/default"

#define NAVI_BUTTON_STYLE		"naviframe/toolbar/default"
#define NAVI_BUTTON_LEFT		"naviframe/toolbar/left"
#define NAVI_BUTTON_RIGHT		"naviframe/toolbar/right"

#define CTRL_STYLE_TYPE		"controlbar"
#define TITLE_LEFT_BTN		"title_left_btn"
#define TITLE_RIGHT_BTN		"title_right_btn"

#define MF_NAVI_STYLE_ENABLE	"basic"

typedef enum __mf_navi_layout_state mf_navi_layout_state;
enum __mf_navi_layout_state {
	mf_navi_layout_all = 0,
	mf_navi_layout_normal,
	mf_navi_layout_no_recent,
	mf_navi_layout_category_recent,
	mf_navi_layout_only_category,
	mf_navi_layout_content_only,
	mf_navi_layout_hide_bottom,
	mf_navi_layout_show_bottom,
	mf_navi_layout_show_bottom_recent,
	mf_navi_layout_root_all,
	mf_navi_layout_root_all_horizon,
	mf_navi_layout_root_content,
	mf_navi_layout_none,
};

typedef enum __mf_navi_pathinfo_type mf_navi_pathinfo_type;
enum __mf_navi_pathinfo_type {
	mf_navi_pathinfo_root = 0,
	mf_navi_pathinfo_normal,
	mf_navi_pathinfo_recent,
	mf_navi_pathinfo_category,
	mf_navi_pathinfo_none,
};

typedef enum __mf_thumbnail_type_e mf_thumbnail_type_e;
enum __mf_thumbnail_type_e {
	MF_THUMBNAIL_DEFAULT,
	MF_THUMBNAIL_THUMB,
};

#define INHERIT_MF_LIST \
	char *create_date;\
	char *thumb_path;\
	bool real_thumb_flag;\
	int flagExpand;\
	fsFileType file_type;\
	int storage_type;\
	int list_type;\
	int thumbnail_type;\
	Elm_Object_Item *item;\
	struct appdata *ap;\
	Evas_Object *pCheckBox;\
	Eina_Bool m_checked;


typedef struct __mf_list_data_t{
	INHERIT_MF_LIST
}mf_list_data_t;

typedef struct {
	INHERIT_MF_LIST
	GString *m_ItemName;
	char *display_name;
	char *size;
	Eina_Bool thumbnail_create;
	media_info_h media;

	i18n_udate modify_time;
	fsNodeInfo *pNode;
} mfItemData_s;

typedef enum __eMfViewStyle {
	MF_VIEW_STYLE_LIST,
	MF_VIEW_SYTLE_LIST_DETAIL,
	MF_VIEW_STYLE_THUMBNAIL,
}eMfViewStyle;

typedef enum {
	POPMODE_MIN = 0,
	POPMODE_TEXT,
	POPMODE_TITLE_TEXT,
	POPMODE_TEXT_NOT_DISABLED,
	POPMODE_TEXT_TWO_BTN,
	POPMODE_TITLE_TEXT_TWO_BTN,
	POPMODE_TEXT_BTN,
	POPMODE_TITLE_TEXT_BTN,
	POPMODE_TITLE_TEXT_THREE_BTN,
	POPMODE_PROGRESSBAR,
	POPMODE_TITLE_LIST_BTN,
	POPMODE_VIEW_AS_LIST,
	POPMPDE_MAX
} ePopMode;

typedef enum __MF_ROTATE_TYPE {
	MF_ROTATE_PORTRAIT,
	MF_ROTATE_LANDSCAPE
}MF_ROTATE_TYPE;

typedef enum __mf_obj_type_e mf_obj_type_e;
enum __mf_obj_type_e {
	mf_obj_genlist = 0,
	mf_obj_gengrid,
	mf_obj_none,
};

#endif //__MF_OBJECT_CONF_DEF__
