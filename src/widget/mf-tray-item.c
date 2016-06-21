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
#include <mime_type.h>

#include "mf-tray-item.h"
#include "mf-util.h"
#include "mf-object-conf.h"
#include "mf-search-view.h"
#include "mf-callback.h"
#include "mf-gengrid.h"
#include "mf-genlist.h"
#include "mf-launch.h"
#include "mf-object.h"
#include "mf-navi-bar.h"
#include "mf-view.h"
#include "mf-storage-space.h"
#include "mf-focus-ui.h"


#ifdef MYFILE_TRAY_FEATURE

#define ICON_NORMAL_SIZE_WIDTH		180.0f
#define ICON_NORMAL_SIZE_HEIGHT		180.0f

#define LAYOUT_SWALLOW_BG		"swallow.bg"

#define MF_CATEGORY_LAYOUT_GROUP "category_frame"
typedef struct __mf_category_layout {
	Evas_Object *category_text;
	Evas_Object *category_frame;
	Evas_Object *category_image;
	Evas_Object *category_video;
	Evas_Object *category_sound;
	Evas_Object *category_document;
	Evas_Object *category_all;
	Evas_Object *category_recent;

	Evas_Object *category_image_title;
	Evas_Object *category_text_title;
	Evas_Object *category_video_title;
	Evas_Object *category_sound_title;
	Evas_Object *category_document_title;
	Evas_Object *category_all_title;
	Evas_Object *category_recent_title;

	Evas_Object *category_image_storage_size;
	Evas_Object *category_video_storage_size;
	Evas_Object *category_sound_storage_size;
	Evas_Object *category_document_storage_size;
	Evas_Object *category_all_size;
	Evas_Object *category_recent_size;

	Ecore_Idler *category_idle;
} mf_category_layout;

mf_category_layout g_mf_category_layout;
//static int g_mf_first_enter_count = 0;

Elm_Gengrid_Item_Class category_gic;
void __mf_category_item_add_click_callback(void *data);
void __mf_category_item_del_click_callback(void *data);
static void __mf_category_item_mode_portrait(void);
static void __mf_category_item_mode_landscape(void);


int mf_tray_item_category_type_get_by_file_type(fsFileType type)
{
	MF_TRACE_BEGIN;
	int category = mf_tray_item_category_others;
	switch (type) {
	case FILE_TYPE_IMAGE:
		category = mf_tray_item_category_image;
		break;
	case FILE_TYPE_VIDEO:
		category = mf_tray_item_category_video;
		break;
	case FILE_TYPE_SOUND:
	case FILE_TYPE_MUSIC:
	case FILE_TYPE_VOICE:
		category = mf_tray_item_category_sounds;
		break;
	case FILE_TYPE_DOC:
	case FILE_TYPE_PDF:
	case FILE_TYPE_PPT:
	case FILE_TYPE_EXCEL:
	case FILE_TYPE_TXT:
	case FILE_TYPE_HWP:
	case FILE_TYPE_SPD:
	case FILE_TYPE_SNB:
		category = mf_tray_item_category_document;
		break;
	case FILE_TYPE_DIR:
		category = mf_tray_item_category_none;
		break;
	default:
		category = mf_tray_item_category_others;
		break;
	}

	return category;
}
int mf_tray_item_type(const char *path)
{
	MF_TRACE_BEGIN;
	int type = mf_file_attr_get_file_type_by_mime(path);
	if (type == FILE_TYPE_MUSIC || type == FILE_TYPE_SOUND) {
		fsFileType file_type = FILE_TYPE_NONE;
		mf_file_attr_get_file_category(path, &file_type);
		if (file_type == FILE_TYPE_VOICE) {
			type = FILE_TYPE_VOICE;
		}
	} else if (type == FILE_TYPE_VIDEO) {
		char *mime = NULL;
		int retcode = -1;
		char *ext = NULL;
		int error_code = mf_file_attr_get_file_ext(path, &ext);
		if (error_code != MYFILE_ERR_NONE || ext == NULL) {
			mf_warning("Fail to get file extension");
			return mf_tray_item_category_none;
		}

		retcode = mime_type_get_mime_type(ext, &mime);

		if ((mime != NULL) && (retcode == MIME_TYPE_ERROR_NONE)) {
			if (g_strcmp0(mime, "video/mp4") == 0) {
				if (mf_file_attr_media_has_video(path)) {
					type = FILE_TYPE_VIDEO;
				} else {
					type = FILE_TYPE_SOUND;
				}
			}
		}
		SAFE_FREE_CHAR(mime);
		SAFE_FREE_CHAR(ext);
	} else if (type == FILE_TYPE_GUL) {
		fsFileType file_type = FILE_TYPE_NONE;
		mf_file_attr_get_file_category(path, &file_type);
		if (file_type == FILE_TYPE_TXT) {
			type = FILE_TYPE_TXT;
		}
	}
	int category = mf_tray_item_category_none;
	switch (type) {
	case FILE_TYPE_IMAGE:
		category = mf_tray_item_category_image;
		break;
	case FILE_TYPE_VIDEO:
		category = mf_tray_item_category_video;
		break;
	case FILE_TYPE_SOUND:
	case FILE_TYPE_MUSIC:
	case FILE_TYPE_VOICE:
		category = mf_tray_item_category_sounds;
		break;
	case FILE_TYPE_DOC:
	case FILE_TYPE_PDF:
	case FILE_TYPE_PPT:
	case FILE_TYPE_EXCEL:
	case FILE_TYPE_TXT:
	case FILE_TYPE_HWP:
	case FILE_TYPE_SPD:
	case FILE_TYPE_SNB:
		category = mf_tray_item_category_document;
		break;
	case FILE_TYPE_DIR:
		category = mf_tray_item_category_none;
		break;
	default:
		category = mf_tray_item_category_others;
		break;
	}
	mf_debug("path is [%s] category is [%d]", path, category);
	return category;
}

void mf_tray_item_search(void *data, int type)
{
	MF_TRACE_BEGIN;
	mf_retm_if(NULL == data, "NULL == data");
	struct appdata *ap = (struct appdata *)data;

	if (type != mf_tray_item_category_none) {
		char *name = NULL;
		ap->mf_Status.view_type = mf_view_root_category;
		switch (type) {
		case mf_tray_item_category_image:
			name = MF_LABEL_IMAGES;
			break;
		case mf_tray_item_category_video:
			name = MF_LABEL_VIDEOS;
			break;
		case mf_tray_item_category_sounds:
			name = MF_LABEL_AUDIOS;
			break;
		case mf_tray_item_category_document:
			name = MF_TRAY_CATEGORY_DOC;
			break;
		case mf_tray_item_category_others:
			name = MF_LABEL_OTHERS;
			break;
		case mf_tray_item_category_voice:
			name = MF_LABEL_VOICE;
			break;
		default:
			break;
		}

		ap->mf_Status.categorytitle = name ;
		if (ap->mf_MainWindow.pNaviGengrid == NULL) {
			ap->mf_MainWindow.pNaviGengrid = mf_gengrid_create(ap->mf_MainWindow.pNaviBar, ap);
		}
		ap->mf_Status.category_type = type;
		mf_category_view_create(ap, false);

		mf_navi_bar_reset_ctrlbar(ap);

		//mf_navi_bar_title_content_set(ap, name);
	}
}

void mf_category_refresh(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	Evas_Object *category = elm_object_part_content_get(ap->mf_MainWindow.pNaviLayout, "category");

	int changed_angle = elm_win_rotation_get(ap->mf_MainWindow.pWindow);
	mf_debug("category=%p, changed_angle is [%d]", category, changed_angle);

	if (category) {
		if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
			edje_object_signal_emit(_EDJ(category), "landscape", "category_frame");
			__mf_category_item_mode_landscape();
		} else {
			edje_object_signal_emit(_EDJ(category), "portrait", "category_frame");
			__mf_category_item_mode_portrait();
		}
	}
}

void mf_category_item_title_refresh(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	if (g_mf_category_layout.category_image) {
		elm_object_part_text_set((g_mf_category_layout.category_image), "category_image_title", mf_util_get_text(MF_LABEL_IMAGES));
	}
	if (g_mf_category_layout.category_video) {
		elm_object_part_text_set((g_mf_category_layout.category_video), "category_video_title", mf_util_get_text(MF_LABEL_VIDEOS));
	}
	if (g_mf_category_layout.category_sound) {
		elm_object_part_text_set((g_mf_category_layout.category_sound), "category_sound_title", mf_util_get_text(MF_LABEL_TITLE_AUDIOS));
	}
	if (g_mf_category_layout.category_document) {
		elm_object_part_text_set((g_mf_category_layout.category_document), "category_document_title", mf_util_get_text(MF_LABLE_SHORTCUT_DOCUMENTS));
	}
	if (g_mf_category_layout.category_text) {
		elm_object_part_text_set((g_mf_category_layout.category_text), "category_text_title", mf_util_get_text(MF_LABEL_CATEGORY));
	}
	if (g_mf_category_layout.category_recent) {
		elm_object_part_text_set((g_mf_category_layout.category_recent), "category_recent_title", mf_util_get_text(MF_LABEL_RECTENT_FILES));
		elm_object_part_text_set((g_mf_category_layout.category_recent), "category_recent_size", "0.0 Byte");
	}
}

void mf_category_widgets_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_category_item_title_refresh(obj);
}

void mf_category_storage_size_reset(mf_tray_item_category category) //fix  P131121-00236 by ray
{
	MF_TRACE_BEGIN;
	switch (category) {
	case mf_tray_item_category_none: {
		if (g_mf_category_layout.category_image) {
			elm_object_part_text_set((g_mf_category_layout.category_image), "category_image_size", "");
		}
		if (g_mf_category_layout.category_video) {
			elm_object_part_text_set((g_mf_category_layout.category_video), "category_video_size", "");
		}
		if (g_mf_category_layout.category_sound) {
			elm_object_part_text_set((g_mf_category_layout.category_sound), "category_sound_size", "");
		}
		if (g_mf_category_layout.category_document) {
			elm_object_part_text_set((g_mf_category_layout.category_document), "category_document_size", "");
		}
		if (g_mf_category_layout.category_recent) {
			elm_object_part_text_set((g_mf_category_layout.category_recent), "category_recent_size", "");
		}
		break;
	}
	case mf_tray_item_category_image: {
		elm_object_part_text_set((g_mf_category_layout.category_image), "category_image_size", "");
		break;
	}
	case mf_tray_item_category_video: {
		elm_object_part_text_set((g_mf_category_layout.category_video), "category_video_size", "");
		break;
	}
	case mf_tray_item_category_sounds: {
		elm_object_part_text_set((g_mf_category_layout.category_sound), "category_sound_size", "");
		break;
	}
	case mf_tray_item_category_document: {
		elm_object_part_text_set((g_mf_category_layout.category_document), "category_document_size", "");
		break;
	}
	case mf_tray_item_category_recent: {
		elm_object_part_text_set((g_mf_category_layout.category_recent), "category_recent_size", "");
		break;
	}
	default:
		break;
	}
}

void mf_category_storage_size_refresh(mf_tray_item_category category, Update_Info* update_info, void *pUserData)
{
	MF_TRACE_BEGIN;
	switch (category) {
	case mf_tray_item_category_none: {
		mfStorageStatus*  storage_status = (mfStorageStatus*) mf_storage_get_status(pUserData);
		if (g_mf_category_layout.category_image) {
			if (storage_status->image_size_info.total_size >= 0.0) {
				char *size = mf_launch_item_size_calculate((double)storage_status->image_size_info.total_size);
				elm_object_part_text_set((g_mf_category_layout.category_image), "category_image_size", size);
				SAFE_FREE_CHAR(size);
			}
		}
		if (g_mf_category_layout.category_video) {
			if (storage_status->video_size_info.total_size >= 0.0) {
				char *size = mf_launch_item_size_calculate((double)storage_status->video_size_info.total_size);
				elm_object_part_text_set((g_mf_category_layout.category_video), "category_video_size", size);
				SAFE_FREE_CHAR(size);
			}
		}
		if (g_mf_category_layout.category_sound) {
			if (storage_status->sound_size_info.total_size >= 0.0) {
				char *size = mf_launch_item_size_calculate((double)storage_status->sound_size_info.total_size);
				elm_object_part_text_set((g_mf_category_layout.category_sound), "category_sound_size", size);
				SAFE_FREE_CHAR(size);
			}
		}
		if (g_mf_category_layout.category_document) {
			if (storage_status->document_size_info.total_size >= 0.0) {
				char *size = mf_launch_item_size_calculate((double)storage_status->document_size_info.total_size);
				elm_object_part_text_set((g_mf_category_layout.category_document), "category_document_size", size);
				SAFE_FREE_CHAR(size);
			}
		}
		if (g_mf_category_layout.category_recent) {
			if (storage_status->recent_size_info.total_size >= 0.0) {
				char *size = mf_launch_item_size_calculate((double)storage_status->recent_size_info.total_size);
				elm_object_part_text_set((g_mf_category_layout.category_recent), "category_recent_size", size);
				SAFE_FREE_CHAR(size);
			}
		}
		break;
	}
	case mf_tray_item_category_image: {
		if (update_info) {
			char *size = mf_launch_item_size_calculate((double)update_info->total_size);
			elm_object_part_text_set((g_mf_category_layout.category_image), "category_image_size", size);
			SAFE_FREE_CHAR(size);
		}
		break;
	}
	case mf_tray_item_category_video: {
		if (update_info) {
			char *size = mf_launch_item_size_calculate((double)update_info->total_size);
			elm_object_part_text_set((g_mf_category_layout.category_video), "category_video_size", size);
			SAFE_FREE_CHAR(size);
		}
		break;
	}
	case mf_tray_item_category_sounds: {
		if (update_info) {
			char *size = mf_launch_item_size_calculate((double)update_info->total_size);
			elm_object_part_text_set((g_mf_category_layout.category_sound), "category_sound_size", size);
			SAFE_FREE_CHAR(size);
		}
		break;
	}
	case mf_tray_item_category_document: {
		if (update_info) {
			char *size = mf_launch_item_size_calculate((double)update_info->total_size);
			elm_object_part_text_set((g_mf_category_layout.category_document), "category_document_size", size);
			SAFE_FREE_CHAR(size);
		}
		break;
	}
	case mf_tray_item_category_recent: {
		if (update_info) {
			char *size = mf_launch_item_size_calculate((double)update_info->total_size);
			elm_object_part_text_set((g_mf_category_layout.category_recent), "category_recent_size", size);
			SAFE_FREE_CHAR(size);
		}
		break;
	}
	default:
		break;
	}
	MF_TRACE_END;
}

#ifdef MYFILE_ENABLE_FOCUS
static void __mf_category_item_key_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	int category = (int) data;
	mf_debug("the category = %d", category);

	Evas_Event_Key_Down *ev = event_info;
	mf_retm_if(ev == NULL, "ev is NULL");
	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) {
		return;
	}

	if ((!strcmp(ev->keyname, "Return")) || (!strcmp(ev->keyname, "KP_Enter")) || (!strcmp(ev->keyname, "space"))) {
		if (category != mf_tray_item_category_none) {
			mf_tray_item_search(ap, category);
		}
	}

	MF_TRACE_END;
}
#endif

static void _mf_category_item_set_focus(void *data, Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

#ifdef MYFILE_ENABLE_FOCUS
	//Focus UI
	_mf_focus_ui_set_focus(g_mf_category_layout.category_image);
	_mf_focus_ui_set_focus(g_mf_category_layout.category_video);
	_mf_focus_ui_set_focus(g_mf_category_layout.category_sound);
	_mf_focus_ui_set_focus(g_mf_category_layout.category_document);

	_mf_focus_ui_set_dual_focus_order(g_mf_category_layout.category_image, g_mf_category_layout.category_video, MF_FOCUS_DUAL_ORDER_RIGHT_LEFT);
	_mf_focus_ui_set_dual_focus_order(g_mf_category_layout.category_sound, g_mf_category_layout.category_document, MF_FOCUS_DUAL_ORDER_RIGHT_LEFT);

	_mf_focus_ui_set_dual_focus_order(g_mf_category_layout.category_image, g_mf_category_layout.category_sound, MF_FOCUS_DUAL_ORDER_DOWN_UP);
	_mf_focus_ui_set_dual_focus_order(g_mf_category_layout.category_video, g_mf_category_layout.category_document, MF_FOCUS_DUAL_ORDER_DOWN_UP);

	_mf_focus_ui_set_dual_focus_order(g_mf_category_layout.category_image, g_mf_category_layout.category_video, MF_FOCUS_DUAL_ORDER_NEXT_PRIV);
	_mf_focus_ui_set_dual_focus_order(g_mf_category_layout.category_video, g_mf_category_layout.category_sound, MF_FOCUS_DUAL_ORDER_NEXT_PRIV);
	_mf_focus_ui_set_dual_focus_order(g_mf_category_layout.category_sound, g_mf_category_layout.category_document, MF_FOCUS_DUAL_ORDER_NEXT_PRIV);

	evas_object_event_callback_add(g_mf_category_layout.category_image, EVAS_CALLBACK_KEY_DOWN, __mf_category_item_key_down, (void *)mf_tray_item_category_image);
	evas_object_event_callback_add(g_mf_category_layout.category_video, EVAS_CALLBACK_KEY_DOWN, __mf_category_item_key_down, (void *)mf_tray_item_category_video);
	evas_object_event_callback_add(g_mf_category_layout.category_sound, EVAS_CALLBACK_KEY_DOWN, __mf_category_item_key_down, (void *)mf_tray_item_category_sounds);
	evas_object_event_callback_add(g_mf_category_layout.category_document, EVAS_CALLBACK_KEY_DOWN, __mf_category_item_key_down, (void *)mf_tray_item_category_document);
#endif
	MF_TRACE_END;
}

Eina_Bool mf_category_widgets_create_idle_cb(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, EINA_FALSE, "data is NULL");

	if (g_mf_category_layout.category_idle) {
		mf_ecore_idler_del(g_mf_category_layout.category_idle);
		g_mf_category_layout.category_idle = NULL;
	}
	if (mf_category_view_refresh_space_size_get() == EINA_TRUE) {
		mf_storage_refresh(data);
		mf_category_view_refresh_space_size_set(EINA_FALSE);
	} else {
		mf_storage_create(data);
	}
	MF_TRACE_END;
	return EINA_FALSE;
}

void mf_category_size_update(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	mf_debug("In category_size_update type: [%d] and more: [%d]", ap->mf_Status.view_type, ap->mf_Status.more);
	if (ap->mf_Status.view_type == mf_view_root && ap->mf_Status.more == MORE_DEFAULT) {
		if (g_mf_category_layout.category_idle) {
			mf_ecore_idler_del(g_mf_category_layout.category_idle);
			g_mf_category_layout.category_idle = NULL;
		}
		g_mf_category_layout.category_idle = ecore_idler_add((Ecore_Task_Cb)mf_category_widgets_create_idle_cb, data);//improve the preload time
	}

}

Evas_Object *mf_category_widgets_create(void *data, Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");

	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", "myfile_category.edj");
	free(path);

	g_mf_category_layout.category_frame = mf_object_create_layout(parent, edj_path, MF_CATEGORY_LAYOUT_GROUP);
	mf_retvm_if(g_mf_category_layout.category_frame == NULL, NULL, "category_frame is NULL");
	g_mf_category_layout.category_text = mf_object_create_layout(parent, edj_path, "group_category_text");
	mf_retvm_if(g_mf_category_layout.category_text == NULL, NULL, "category_text is NULL");
	g_mf_category_layout.category_recent = mf_object_create_layout(parent, edj_path, "group_category_recent");
	mf_retvm_if(g_mf_category_layout.category_recent == NULL, NULL, "category_recent_files is NULL");
	//g_mf_category_layout.category_all = mf_object_create_layout(parent, edj_path, "group_category_all_files");
	//mf_retvm_if(g_mf_category_layout.category_all == NULL, NULL, "category_all_files is NULL");
	g_mf_category_layout.category_image = mf_object_create_layout(parent, edj_path, "group_category_image");
	mf_retvm_if(g_mf_category_layout.category_image == NULL, NULL, "category_image is NULL");
	g_mf_category_layout.category_video = mf_object_create_layout(parent, edj_path, "group_category_video");
	mf_retvm_if(g_mf_category_layout.category_video == NULL, NULL, "category_video is NULL");
	g_mf_category_layout.category_sound = mf_object_create_layout(parent, edj_path, "group_category_sound");
	mf_retvm_if(g_mf_category_layout.category_sound == NULL, NULL, "category_sound is NULL");
	g_mf_category_layout.category_document = mf_object_create_layout(parent, edj_path, "group_category_document");
	mf_retvm_if(g_mf_category_layout.category_document == NULL, NULL, "category_document is NULL");


	evas_object_repeat_events_set(g_mf_category_layout.category_text, EINA_TRUE);
	evas_object_repeat_events_set(g_mf_category_layout.category_recent, EINA_TRUE);
	evas_object_repeat_events_set(g_mf_category_layout.category_image, EINA_TRUE);
	evas_object_repeat_events_set(g_mf_category_layout.category_video, EINA_TRUE);
	evas_object_repeat_events_set(g_mf_category_layout.category_sound, EINA_TRUE);
	evas_object_repeat_events_set(g_mf_category_layout.category_document, EINA_TRUE);

	elm_object_part_content_set(g_mf_category_layout.category_frame, "category_text", g_mf_category_layout.category_text);
	evas_object_show(g_mf_category_layout.category_text);
	elm_object_part_content_set(g_mf_category_layout.category_frame, "1", g_mf_category_layout.category_recent);
	evas_object_show(g_mf_category_layout.category_recent);
	elm_object_part_content_set(g_mf_category_layout.category_frame, "2", g_mf_category_layout.category_image);
	evas_object_show(g_mf_category_layout.category_image);
	elm_object_part_content_set(g_mf_category_layout.category_frame, "3", g_mf_category_layout.category_video);
	evas_object_show(g_mf_category_layout.category_video);
	elm_object_part_content_set(g_mf_category_layout.category_frame, "4", g_mf_category_layout.category_sound);
	evas_object_show(g_mf_category_layout.category_sound);
	elm_object_part_content_set(g_mf_category_layout.category_frame, "5", g_mf_category_layout.category_document);
	evas_object_show(g_mf_category_layout.category_document);

	_mf_category_item_set_focus(data, g_mf_category_layout.category_frame);

	//evas_object_move(frame, g_select_layout_rect->x, g_select_layout_rect->y);
	//evas_object_resize(frame, g_select_layout_rect->width, g_select_layout_rect->height);
	evas_object_show(g_mf_category_layout.category_frame);
	evas_object_smart_callback_add(g_mf_category_layout.category_frame, "language,changed", mf_category_widgets_lang_changed, data);
	g_mf_category_layout.category_idle = ecore_idler_add((Ecore_Task_Cb)mf_category_widgets_create_idle_cb, data);//improve the preload time

	mf_storage_set_update_cb(data, mf_category_storage_size_refresh);
	return g_mf_category_layout.category_frame;
}

static void __mf_category_item_clicked(void *data, Evas_Object *o, const char *emission, const char *source)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	int category = (int) data;
	mf_debug("the category = %d", category);
	if (ap->mf_Status.more == MORE_EDIT_RENAME) {
		return;
	}

	if (category != mf_tray_item_category_none) {
		mf_tray_item_search(ap, category);
	}
	MF_TRACE_END;
}

static void __mf_category_item_released(void *data, Evas_Object *o, const char *emission, const char *source)
{
	MF_TRACE_BEGIN;
	//mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)mf_get_appdata();
	int changed_angle = elm_win_rotation_get(ap->mf_MainWindow.pWindow);

	if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
		__mf_category_item_mode_landscape();
	} else {
		__mf_category_item_mode_portrait();
	}
	MF_TRACE_END;
}

static void __mf_category_recent_item_clicked(void *data, Evas_Object *o, const char *emission, const char *source)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)mf_get_appdata();
	if (ap->mf_Status.more == MORE_EDIT_RENAME) {
		return;
	}

	ap->mf_Status.view_type = mf_view_recent;
	mf_view_update(ap);
	MF_TRACE_END;
}

void __mf_category_item_add_click_callback(void *data)
{
	MF_TRACE_BEGIN;
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_image),
	                                "category_image,clicked", "background_signal", __mf_category_item_clicked, (void *)mf_tray_item_category_image);
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_video),
	                                "category_video,clicked", "background_signal", __mf_category_item_clicked, (void *)mf_tray_item_category_video);
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_sound),
	                                "category_sound,clicked", "background_signal", __mf_category_item_clicked, (void *)mf_tray_item_category_sounds);
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_document),
	                                "category_document,clicked", "background_signal", __mf_category_item_clicked, (void *)mf_tray_item_category_document);
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_recent),
	                                "category_recent,clicked", "background_signal", __mf_category_recent_item_clicked, NULL);
}

void __mf_category_item_del_click_callback(void *data)
{
	MF_TRACE_BEGIN;
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_image),
	                                "category_image,clicked", "background_signal", __mf_category_item_clicked);
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_video),
	                                "category_video,clicked", "background_signal", __mf_category_item_clicked);
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_sound),
	                                "category_sound,clicked", "background_signal", __mf_category_item_clicked);
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_document),
	                                "category_document,clicked", "background_signal", __mf_category_item_clicked);
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_recent),
	                                "category_recent,clicked", "background_signal", __mf_category_recent_item_clicked);
}

void __mf_category_item_add_release_callback(void *data)
{
	MF_TRACE_BEGIN;
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_image),
	                                "category_image,released", "released_signal", __mf_category_item_released, NULL);
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_video),
	                                "category_video,released", "released_signal", __mf_category_item_released, NULL);
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_sound),
	                                "category_sound,released", "released_signal", __mf_category_item_released, NULL);
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_document),
	                                "category_document,released", "released_signal", __mf_category_item_released, NULL);
	edje_object_signal_callback_add(_EDJ(g_mf_category_layout.category_recent),
	                                "category_recent,released", "released_signal", __mf_category_item_released, NULL);
}

void __mf_category_item_del_release_callback(void *data)
{
	MF_TRACE_BEGIN;
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_image),
	                                "category_image,released", "released_signal", __mf_category_item_released);
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_video),
	                                "category_video,released", "released_signal", __mf_category_item_released);
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_sound),
	                                "category_sound,released", "released_signal", __mf_category_item_released);
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_document),
	                                "category_document,released", "released_signal", __mf_category_item_released);
	edje_object_signal_callback_del(_EDJ(g_mf_category_layout.category_recent),
	                                "category_recent,released", "released_signal", __mf_category_item_released);
}


void __mf_category_item_mode_landscape(void)
{
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_image), "landscape", "category_image");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_recent), "landscape", "category_recent");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_video), "landscape", "category_video");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_sound), "landscape", "category_sound");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_document), "landscape", "category_document");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_text), "landscape", "category_text");
}

void __mf_category_item_mode_portrait(void)
{
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_image), "portrait", "category_image");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_recent), "portrait", "category_recent");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_video), "portrait", "category_video");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_sound), "portrait", "category_sound");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_document), "portrait", "category_document");
	edje_object_signal_emit(_EDJ(g_mf_category_layout.category_text), "portrait", "category_text");
}

Evas_Object *mf_category_create(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *category_frame = NULL;
	category_frame = mf_category_widgets_create(data, ap->mf_MainWindow.pNaviLayout);
	int changed_angle = elm_win_rotation_get(ap->mf_MainWindow.pWindow);
	mf_debug("category=%p, changed_angle is [%d]", category_frame, changed_angle);

	if (category_frame) {
		if (changed_angle == APP_DEVICE_ORIENTATION_90 || changed_angle == APP_DEVICE_ORIENTATION_270) {
			edje_object_signal_emit(_EDJ(category_frame), "landscape", "category_frame");
			__mf_category_item_mode_landscape();
		} else {
			edje_object_signal_emit(_EDJ(category_frame), "portrait", "category_frame");
			__mf_category_item_mode_portrait();
		}
	}

	__mf_category_item_add_release_callback(data);
	__mf_category_item_add_click_callback(data);
	mf_category_item_title_refresh(ap->mf_MainWindow.pNaviLayout);
	mf_storage_get_recent_files_size();
	mf_category_storage_size_refresh(mf_tray_item_category_none, NULL, NULL);

	MF_TRACE_END;
	return category_frame;
}

#endif

