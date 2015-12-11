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
#include <sys/time.h>
#include <glib.h>
#include <media_content.h>
#include <app.h>
#include <Elementary.h>
#include <system_settings.h>

#include "mf-download-app.h"
#include "mf-download-apps-view.h"
#include "mf-launch.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((__visibility__("default")))
#endif

#include "mf-tray-item.h"
#include "mf-main.h"
#include "mf-conf.h"
#include "mf-dlog.h"
#include "mf-util.h"
#include "mf-callback.h"
#include "mf-object-conf.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-ta.h"
#include "mf-log.h"
#include "mf-launch.h"
#include "mf-context-popup.h"
#include "mf-view.h"
#include "mf-object.h"
#include "mf-navi-bar.h"
#include "mf-genlist.h"
#include "mf-media.h"
#include "mf-object-item.h"
#include "mf-gengrid.h"
#include "mf-search-view.h"
#include "mf-fs-monitor.h"
#include "mf-edit-view.h"
#include "mf-thumb-gen.h"
#include "mf-media-content.h"
#include "mf-file-util.h"

#define MF_B_KEY_PATH "path"

GString *phone_folder_as_param;

static bool __mf_main_create_app(void *data);
static void __mf_main_terminate_app(void *data);
static void __mf_main_stop_app(void *data);
static void __mf_main_resume_app(void *data);
static void __mf_main_reset_app(app_control_h app_control, void *data);
static void __mf_main_storage_status_get(void *data);

#define RETRY_MAX               10
#define BUS_NAME                "org.tizen.usb.storage"
#define OBJECT_PATH             "/Org/Tizen/Usb/Storage"
#define INTERFACE_NAME          BUS_NAME
#define SIGNAL_NAME_USB_STORAGE "usbstorage"
#define USB_STORAGE_ADDED       "added"
#define USB_STORAGE_REMOVED     "removed"
#define MF_SEARCH_LAUNCH        "search_view_launch"

struct appdata *g_myfile_app_data = NULL;

struct appdata* mf_get_appdata()
{
	return g_myfile_app_data;
}

static Eina_Bool split_on = EINA_FALSE;

Eina_Bool mf_main_is_split_on()
{
	return split_on;
}

void mf_main_set_split_on(Eina_Bool split_state)
{
	split_on = split_state;
}

static void __mf_main_data_init(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;
	g_myfile_app_data = ap;
	/*set state value for create content*/
	ap->mf_Status.more = MORE_DEFAULT;
	ap->mf_Status.path = g_string_new(PHONE_FOLDER);
	ap->mf_SharedGadget.ug = NULL;
	ap->mf_Status.iStorageState = MYFILE_PHONE;
	ap->mf_Status.flagLCDLock = EINA_FALSE;
	ap->mf_Status.rotation_type = MF_ROTATE_PORTRAIT;
	ap->mf_Status.flag_tab = MF_TAB_LEFT;
	ap->mf_Status.EnterFrom = NULL;

	/* region format related */
	ap->mf_Status.flagIcuInit = FALSE;
	ap->mf_Status.generator = NULL;
	ap->mf_Status.formatter = NULL;
	ap->mf_Status.flagIME = EINA_TRUE;
	ap->mf_Status.view_type = mf_view_root;
	ap->mf_Status.preViewType = mf_view_root;


	mf_util_get_pref_value(PREF_TYPE_VIEW_STYLE, &ap->mf_Status.flagViewType);
	mf_util_get_pref_value(PREF_TYPE_EXTENSION_STATE, &ap->mf_Status.iExtensionState);

	MF_TRACE_END;
	t_end;

}

/******************************
** Prototype    : mf_main_load_edj
** Description  :
** Input        : Evas_Object *parent
**                const char *file
**                const char *group
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
Evas_Object *mf_main_load_edj(Evas_Object * parent, const char *file, const char *group)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_debug();
	Evas_Object *eo;
	int r;

	eo = elm_layout_add(parent);
	elm_object_focus_set(eo, EINA_FALSE);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			evas_object_del(eo);
			MF_TRACE_END;
			return NULL;
		}

		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	}
	MF_TRACE_END;
	t_end;
	return eo;
}



/******************************
** Prototype    : __mf_main_del_win
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event
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
static void __mf_main_del_win(void *data, Evas_Object * obj, void *event)
{
	MF_TRACE_BEGIN;
	t_start;
	elm_exit();
	t_end;
	MF_TRACE_END;
}

/******************************
** Prototype    : __mf_main_create_win
** Description  :
** Input        : const char *name
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
static Evas_Object *__mf_main_create_win(const char *name)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_debug();
	Evas_Object *eo;
	int w, h;
	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	elm_object_focus_set(eo, EINA_FALSE);
	elm_win_autodel_set(eo, 1);
	if (eo) {
		elm_win_title_set(eo, name);
		evas_object_smart_callback_add(eo, "delete,request", __mf_main_del_win, NULL);
		elm_win_screen_size_get(eo, NULL, NULL, &w, &h);
		evas_object_resize(eo, w, h);
	}
	if (eo) {
		MF_TRACE_END;
		t_end;
		return eo;
	} else {
		MF_TRACE_END;
		t_end;
		return NULL;
	}
}

static Evas_Object *__mf_main_create_bg(Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retv_if(parent == NULL, NULL);
	Evas_Object *bg = NULL;
	bg = elm_bg_add(parent);
	elm_object_focus_set(bg, EINA_FALSE);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_win_resize_object_add(parent, bg);

	evas_object_show(bg);
	MF_TRACE_END;
	t_end;

	return bg;
}

/******************************
** Prototype    : __mf_main_capture_idle_img
** Description  :
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
static void __mf_main_view_create(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	assert(data);
	struct appdata *ap = (struct appdata *)data;

	//create phone navibar struct

	mf_view_update(ap);
	MF_TRACE_END;
	t_end;

}


static Eina_Bool
__mf_main_app_init_idler_cb(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	assert(data);
	struct appdata *ap = (struct appdata *)data;
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", EDJ_NAME);
	elm_theme_extension_add(NULL, edj_path);

	SAFE_FREE_ECORE_EVENT(ap->mf_MainWindow.font_event);

	SAFE_FREE_ECORE_EVENT(ap->mf_MainWindow.event);
	ap->mf_MainWindow.event = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN,
	                          (Ecore_Event_Handler_Cb)mf_context_popup_mousedown_cb, ap);

	mf_callback_set_mmc_state_cb(ap);
	ap->mf_Status.app_init_idler = NULL;

	//mf_callback_imf_state_callback_register(ap);
	/*** Add the media-db update callback ***********/
	media_content_set_db_updated_cb(mf_category_list_update_cb, ap);
	mf_file_recursive_rm(TEMP_FOLDER_FOR_COPY_PHONE);
	mf_file_recursive_rm(TEMP_FOLDER_FOR_COPY_MMC);
	ap->mf_Status.app_init_idler = NULL;

	MF_TRACE_END;
	t_end;

	return ECORE_CALLBACK_CANCEL;
}

static int __mf_main_remake_app(app_control_h app_control, void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	assert(data);
	struct appdata *ap = (struct appdata *)data;

	/*if myfile state is not normal,  it is not handled argument*/
	if (ap->mf_Status.more != MORE_DEFAULT) {
		mf_warning("Fail to handle budle, current myfiles stat is %d", ap->mf_Status.more);
		goto RAISE_WIN;
	}

	__mf_main_view_create(ap);

RAISE_WIN:

	/**************pre-condition test to launch myfile app******************/

	MF_TRACE_END;
	t_end;
	return 0;

}

/******************************
** Prototype    : __mf_main_create_app
** Description  :
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
static void __mf_main_storage_status_get(void *data)
{
	MF_TRACE_BEGIN
	t_start;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata*)data;
	int mmc_card = 0;
	int error_code = 0;

	error_code = mf_util_is_mmc_on(&mmc_card);
	if (error_code == 0 && mmc_card == 1) {
		ap->mf_Status.iStorageState |= MYFILE_MMC;
	}
	t_end;
	MF_TRACE_END;
}

void __mf_main_rotation_list_register(Evas_Object *win)
{
	MF_TRACE_BEGIN;
	mf_retm_if(win == NULL, "win is NULL");
	if (elm_win_wm_rotation_supported_get(win)) {
		const int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(win, rots, 4);
	}
	MF_TRACE_END;

}

void __mf_main_rotation_callback_register(void *data, Evas_Object *win)
{
	MF_TRACE_BEGIN;
	mf_retm_if(win == NULL, "win is NULL");
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	evas_object_smart_callback_add(win, "wm,rotation,changed", mf_callback_app_rotate_cb, ap);
}

/*variable for control, if thread initialization_extra performed it's work*/
static int initialization_done_extra = 0;
static pthread_t thread_init;
static int initialization_done = 0;

void *initialization_extra(void *data)
{
	//struct appdata *ap = (struct appdata *)data;
	/*switching on at once to prevent double invocation in any case*/



#ifdef MYFILE_DOWNLOAD_APP_FEATURE
	mf_download_app_main(ap);
	mf_download_app_pkgmgr_subscribe(ap);
#endif

	return NULL;
}

static void run_extra(void *data)
{
	pthread_t thread_extra;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (initialization_done_extra != 1) {
		pthread_create(&thread_extra, &attr, initialization_extra, data);
		initialization_done_extra = 1;
	}
}

void *initialization(void *data)
{
	struct appdata *ap = (struct appdata *)data;
	int ret = 0;

	__mf_main_data_init(ap);
	ret = mf_media_connect(&ap->mf_MainWindow.mfd_handle);
	if (ret == MFD_ERROR_NONE) {
		mf_error("db open success");
	} else {
		mf_error("db open failed");
	}
	__mf_main_storage_status_get(ap);


	/*Create content frame of Main Layout*/
	ret = media_content_connect();
	mf_retvm_if(ret < 0, NULL, "Fail to media_content_connect()");

	ret = mf_fs_monitor_create(ap);
	mf_retvm_if(ret < 0, NULL, "Fail to mf_fs_monitor_create()");

	ap->mf_Status.app_init_idler = ecore_idler_add((Ecore_Task_Cb)__mf_main_app_init_idler_cb, ap);

	initialization_done = 1;
	return NULL;
}

bool __mf_main_create_app(void *data)
{
	MF_TA_ACUM_ITEM_BEGIN("123 __mf_main_create_app", 0);
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;
	int ret_value = 0;

	bindtextdomain(MYFILE_STRING_PACKAGE, LOCALEDIR);

	textdomain(MYFILE_STRING_PACKAGE);

//	ret_value = pthread_create(&thread_init, NULL, initialization, data);
//	if (ret_value != 0) {
//		initialization(data);
//		mf_retvm_if(!initialization_done, -1, "Initialization failed");
//	}
	initialization(data);
	//elm_config_preferred_engine_set("opengl_x11");
#if 0//Deprecated API
	if (!g_thread_supported()) {
		g_thread_init(NULL);
	}
#endif
	MF_TA_ACUM_ITEM_BEGIN("1234 __mf_main_create_win", 0);

	ap->mf_MainWindow.pWindow = __mf_main_create_win(MYFILE_STRING_PACKAGE);
	mf_retvm_if(ap->mf_MainWindow.pWindow == NULL, -1, "Fail to __mf_main_create_win()");

	evas_object_geometry_get(ap->mf_MainWindow.pWindow, NULL, NULL, &ap->mf_MainWindow.root_w, &ap->mf_MainWindow.root_h);

	MF_TA_ACUM_ITEM_END("1234 __mf_main_create_win", 0);
	MF_TA_ACUM_ITEM_BEGIN("1234 __mf_main_rotation_callback_register", 0);

	__mf_main_rotation_callback_register(ap, ap->mf_MainWindow.pWindow);

	if (ret_value == 0 && !initialization_done) {
		pthread_join(thread_init, NULL);
		mf_retvm_if(!initialization_done, -1, "Initialization failed");
	}

	ap->mf_Status.rotation_type = MF_ROTATE_PORTRAIT;
	__mf_main_rotation_list_register(ap->mf_MainWindow.pWindow);
	elm_win_wm_rotation_preferred_rotation_set(ap->mf_MainWindow.pWindow, -1);

	MF_TA_ACUM_ITEM_END("1234 __mf_main_rotation_callback_register", 0);

	/**************start to launch myfile app******************/
	MF_TA_ACUM_ITEM_BEGIN("1234 __mf_main_create_bg", 0);


	ap->mf_MainWindow.pBackGround = __mf_main_create_bg(ap->mf_MainWindow.pWindow);
	MF_TA_ACUM_ITEM_END("1234 __mf_main_create_bg", 0);

	MF_TA_ACUM_ITEM_BEGIN("1234 mf_object_create_conform", 0);
	ap->mf_MainWindow.pConformant = mf_object_create_conform(ap->mf_MainWindow.pWindow);
	elm_win_resize_object_add(ap->mf_MainWindow.pWindow, ap->mf_MainWindow.pConformant);
	MF_TA_ACUM_ITEM_END("1234 mf_object_create_conform", 0);

	Evas_Object *conformant_bg = elm_bg_add(ap->mf_MainWindow.pConformant);
	elm_object_style_set(conformant_bg, "indicator/headerbg");
	elm_object_part_content_set(ap->mf_MainWindow.pConformant, "elm.swallow.indicator_bg", conformant_bg);
	evas_object_show(conformant_bg);

	MF_TA_ACUM_ITEM_BEGIN("1234 mf_object_create_layout_main", 0);
	ap->mf_MainWindow.pMainLayout = mf_object_create_layout_main(ap->mf_MainWindow.pConformant);

	elm_object_content_set(ap->mf_MainWindow.pConformant, ap->mf_MainWindow.pMainLayout);
	elm_win_conformant_set(ap->mf_MainWindow.pWindow, EINA_TRUE);
	//recover indicator mode and type when your view disappeared.
	elm_win_indicator_mode_set(ap->mf_MainWindow.pWindow, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ap->mf_MainWindow.pWindow, ELM_WIN_INDICATOR_TRANSPARENT);
	elm_object_signal_emit(ap->mf_MainWindow.pConformant, "elm,state,indicator,overlap", "");
	evas_object_data_set(ap->mf_MainWindow.pConformant, "overlap", (void *)EINA_TRUE);

	MF_TA_ACUM_ITEM_END("1234 mf_object_create_layout_main", 0);
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", EDJ_GENLIST_NAME);
	elm_theme_extension_add(NULL, edj_path);
	memset(edj_path,0,1024);
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", EDJ_GENGRID_NAME);
	elm_theme_extension_add(NULL, edj_path);
	//free(edj_path);

	MF_TA_ACUM_ITEM_BEGIN("1234 create naviframe", 0);
	mf_view_phone_storage_init(ap);
	MF_TA_ACUM_ITEM_END("1234 create naviframe", 0);

	//create landscape/portrait view

	elm_object_part_content_set(ap->mf_MainWindow.pMainLayout, "elm.swallow.content", ap->mf_MainWindow.pNaviBar);

	//__mf_main_app_init_idler_cb(ap);

	evas_object_smart_callback_add(ap->mf_MainWindow.pWindow, "profile,changed", mf_callback_profile_changed_cb, ap);

	ap->mf_Status.b_run_background = false;

	MF_TRACE_END;
	t_end;
	MF_TA_ACUM_ITEM_END("123 __mf_main_create_app", 0);

	return true;
}

static void __mf_main_bundle_parse(app_control_h service, void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	assert(data);

	struct appdata *ap = (struct appdata *)data;

	app_control_get_extra_data(service, "path", &ap->mf_Bundle.path);
	app_control_get_extra_data(service, "select_type", &ap->mf_Bundle.select_type);
	app_control_get_extra_data(service, "file_type", &ap->mf_Bundle.file_type);
	app_control_get_extra_data(service, "marked_mode", &ap->mf_Bundle.marked_mode);
	MF_TRACE_END;
	t_end;
}

static void __mf_main_reset_app(app_control_h app_control, void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	MF_TA_ACUM_ITEM_BEGIN("123 __mf_main_reset_app", 0);

	struct appdata *ap = (struct appdata *)data;

	if (ap->mf_MainWindow.pWindow) {
		evas_object_show(ap->mf_MainWindow.pWindow);
		elm_win_activate(ap->mf_MainWindow.pWindow);
		//ecore_main_loop_iterate();
		MF_TA_ACUM_ITEM_BEGIN("1234 create main view", 0);
		__mf_main_remake_app(NULL, ap);
		MF_TA_ACUM_ITEM_END("1234 create main view", 0);
		ap->mf_Status.rotation_type = mf_util_rotate_state_get(ap->mf_MainWindow.pWindow);
	}

	mf_error("~~~~~~~~~~~~~~~");

	char *uri = NULL;
	char *tmp = NULL;
	mf_error();
	char *operation = NULL;

	app_control_get_operation(app_control, &operation);
	mf_error("operation is [%s]", operation);
	if (g_strcmp0(operation, APP_CONTROL_OPERATION_PICK) == 0) {
		__mf_main_bundle_parse(app_control, ap);
		mf_launch_load_ug_myfile(ap);
		SAFE_FREE_CHAR(operation);
	} else {
		app_control_get_extra_data(app_control, "path", &tmp);
		mf_error("tmp is [%s]", tmp);
		if (tmp && mf_file_exists(tmp)) {
			mf_file_attr_get_parent_path(tmp, &uri);
		}
		mf_error();
		mf_error("uri is [%s] tmp is [%s]", uri, tmp);
		if (uri && mf_file_exists(uri)) {
			int locate = mf_fm_svc_wrapper_get_location(uri);
			if (locate != MYFILE_ERR_STORAGE_TYPE_ERROR) {
				ap->mf_Status.view_type = mf_view_normal;


				ap->mf_MainWindow.location = locate;

				/*2.2	set related status value. */
				if (ap->mf_Status.path != NULL) {
					g_string_free(ap->mf_Status.path, TRUE);
					ap->mf_Status.path = NULL;
				}
				ap->mf_Status.path = g_string_new(uri);
				ap->mf_Status.EnterFrom = g_strdup(tmp);
				/*2.3	update the content to catch update */
				/*Todo: How to ensure insert only once */

				ap->mf_Status.more = MORE_DEFAULT;
				__mf_main_remake_app(app_control, ap);
				SAFE_DEL_NAVI_ITEM(&ap->mf_Status.pPreNaviItem);
				//run_extra(data);
			} else {
				if (ap->mf_MainWindow.pWindow != NULL && ap->mf_Status.b_run_background) {
					if (mf_util_is_rotation_lock() == 0) {
						mf_debug("rotation is locked");
						if (ap->mf_Status.rotation_type == MF_ROTATE_PORTRAIT) {
							elm_win_activate(ap->mf_MainWindow.pWindow);
							ap->mf_Status.b_run_background = false;
							SAFE_FREE_CHAR(tmp);
							return;
						} else {
							ap->mf_Status.rotation_type = MF_ROTATE_PORTRAIT;
							elm_win_activate(ap->mf_MainWindow.pWindow);
							ap->mf_Status.b_run_background = false;
							__mf_main_remake_app(app_control, ap);
						}
					} else {
						int type = mf_util_rotate_state_get(ap->mf_MainWindow.pWindow);
						if (type == ap->mf_Status.rotation_type) {
							elm_win_activate(ap->mf_MainWindow.pWindow);
							ap->mf_Status.b_run_background = false;
							SAFE_FREE_CHAR(tmp);
							return;
						}
					}
					elm_win_activate(ap->mf_MainWindow.pWindow);
					ap->mf_Status.b_run_background = false;
					MF_TRACE_END;
					t_end;
					SAFE_FREE_CHAR(tmp);
					return;

				}
			}
			run_extra(data);
			SAFE_FREE_CHAR(uri);
		} else {
			if (ap->mf_MainWindow.pWindow != NULL && ap->mf_Status.b_run_background) {
				if (mf_util_is_rotation_lock() == 0) {
					mf_debug("rotation is locked");
					if (ap->mf_Status.rotation_type == MF_ROTATE_PORTRAIT) {
						elm_win_activate(ap->mf_MainWindow.pWindow);
						ap->mf_Status.b_run_background = false;
						SAFE_FREE_CHAR(tmp);
						return;
					} else {
						ap->mf_Status.rotation_type = MF_ROTATE_PORTRAIT;
						elm_win_activate(ap->mf_MainWindow.pWindow);
						ap->mf_Status.b_run_background = false;
						__mf_main_remake_app(app_control, ap);
					}
				} else {
					int type = mf_util_rotate_state_get(ap->mf_MainWindow.pWindow);
					if (type == ap->mf_Status.rotation_type) {
						elm_win_activate(ap->mf_MainWindow.pWindow);
						ap->mf_Status.b_run_background = false;
						SAFE_FREE_CHAR(tmp);
						return;
					}
				}
				elm_win_activate(ap->mf_MainWindow.pWindow);
				ap->mf_Status.b_run_background = false;
				MF_TRACE_END;
				t_end;
				SAFE_FREE_CHAR(tmp);
				return;
			} else {
				run_extra(data);
			}
		}
		SAFE_FREE_CHAR(tmp);
	}
	if (ap->mf_MainWindow.pWindow) {
		evas_object_show(ap->mf_MainWindow.pWindow);
		elm_win_activate(ap->mf_MainWindow.pWindow);
	}
	MF_TA_ACUM_ITEM_END("123 __mf_main_reset_app", 0);
	MF_TA_ACUM_ITEM_END("1 Launch myfile", 0);
	t_end;
	MF_TRACE_END;
}

static void __mf_main_resume_app(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	/*check if update search result*/

	if (ap->mf_SharedGadget.ug) {
		mf_error("========================= resume ug");
	}
	mf_view_resume(ap);
	t_end;
	MF_TRACE_END;
}

/******************************
** Prototype    : __mf_main_terminate_app
** Description  :
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
void __mf_main_terminate_app(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	MF_TA_ACUM_ITEM_SHOW_RESULT_TO(MF_TA_SHOW_FILE);
	MF_TA_RELEASE();
	if (mf_view_is_operating(ap)) {
		mf_callback_progress_bar_cancel_cb(ap, NULL, NULL);
	}

	mf_media_content_disconnect();

	SAFE_FREE_OBJ(ap->mf_MainWindow.pProgressPopup);
	media_content_unset_db_updated_cb();
	mf_category_list_destory();
	mf_callback_unregister_mmc_state_cb();
	if (ap->mf_Status.search_handler) {
		mf_search_finalize(&ap->mf_Status.search_handler);
	}

	mf_edit_folder_list_clear();
	mf_edit_file_list_clear();

	mf_fs_monitor_destory();

	//mf_callback_imf_state_callback_del(ap);
	mf_util_free_eina_list_with_data(&ap->mf_FileOperation.folder_list, MYFILE_TYPE_FSNODE);
	mf_util_free_eina_list_with_data(&ap->mf_FileOperation.file_list, MYFILE_TYPE_FSNODE);

	mf_ecore_idler_del(ap->mf_Status.app_init_idler);
	SAFE_DEL_ECORE_TIMER(ap->mf_Status.rename_timer);
	mf_ecore_idler_del(ap->mf_Status.operation_refresh_idler);
	mf_ecore_idler_del(ap->mf_Status.search_idler);
	mf_entry_focus_allow_idler_destory();
	mf_download_update_idler_del();
	mf_popup_timer_del();
	mf_launch_service_timer_del();
	mf_launch_service_idler_del();
	mf_view_refresh_thumbnail_destroy();

	SAFE_DEL_ECORE_TIMER(ap->mf_MainWindow.pPopupTimer);
	SAFE_FREE_ECORE_EVENT(ap->mf_MainWindow.event);
	SAFE_FREE_ECORE_EVENT(ap->mf_MainWindow.font_event);
	SAFE_FREE_CHAR(ap->mf_MainWindow.naviframe_title);
	SAFE_FREE_CHAR(ap->mf_MainWindow.record.path);

	mf_media_disconnect(ap->mf_MainWindow.mfd_handle);
	SAFE_FREE_CHAR(ap->mf_Status.search_filter);
	SAFE_FREE_CHAR(ap->mf_Status.entry_path);
	SAFE_FREE_CHAR(ap->mf_Status.EnterFrom);

#ifdef MYFILE_CRITICAL_LOG
	mf_log_finalize();
#endif

	if (ap->mf_Status.flagIcuInit == TRUE) {
		mf_util_icu_finalize(ap);
	}

	if (ap->mf_FileOperation.sync_pipe) {
		ecore_pipe_del(ap->mf_FileOperation.sync_pipe);
		ap->mf_FileOperation.sync_pipe = NULL;
	}
	mf_genlist_item_class_free(ap->mf_gl_style.itc);
	mf_genlist_item_class_free(ap->mf_gl_style.userfolderitc);
	mf_genlist_item_class_free(ap->mf_gl_style.search_itc);
	mf_genlist_item_class_free(ap->mf_gl_style.left_itc);
	mf_genlist_item_class_free(ap->mf_gl_style.categoryitc);
	mf_genlist_item_class_free(ap->mf_gl_style.popup_itc);
	mf_genlist_item_class_free(ap->mf_gl_style.listby_itc);
	mf_genlist_item_class_free(ap->mf_gl_style.order_itc);

	mf_util_set_pm_lock(ap, EINA_FALSE);
	char edj_path[1024] = {0};
	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path , "edje", EDJ_NAME);
	elm_theme_extension_del(NULL, edj_path);

	t_end;
	MF_TRACE_END;
}

/******************************
** Prototype    : __mf_main_stop_app
** Description  :
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
void __mf_main_stop_app(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	//Fix P140818-01985, when the app is paused, don't update the list.
	mf_download_update_idler_del();
	ap->mf_Status.b_run_background = true;
	if (ap->mf_Status.more == MORE_SEARCH) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pContextPopup);
	}
	if (ap->mf_SharedGadget.ug) {
		mf_error("================ pause ug");
	}
	t_end;
	MF_TRACE_END;

}

/******************************
** Prototype    : main
** Description  :
** Input        : int argc
**                char *argv[]
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
static void __mf_language_changed_cb(app_event_info_h event_info, void *user_data)
{
	MF_TRACE_BEGIN;
	char *locale = NULL;
	int retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);

	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		mf_error("[ERR] failed to get the language");
	}

	if (locale) {
		struct appdata *ap = (struct appdata *)user_data;
		mf_error("locale is [%s]", locale);
		elm_language_set(locale);

		if (ap->mf_MainWindow.pLongpressPopup) {
			SAFE_FREE_OBJ(ap->mf_MainWindow.pLongpressPopup);
			ap->mf_MainWindow.pLongpressPopup = NULL;
		}
		if (ap->mf_Status.more == MORE_EDIT
		        || ap->mf_Status.more == MORE_SHARE_EDIT
		        || ap->mf_Status.more == MORE_COMPRESS
		        || ap->mf_Status.more == MORE_EDIT_COPY
		        || ap->mf_Status.more == MORE_EDIT_MOVE
		        || ap->mf_Status.more == MORE_EDIT_DELETE
		        || ap->mf_Status.more == MORE_EDIT_DELETE_RECENT
		        || ap->mf_Status.more == MORE_EDIT_UNINSTALL

		        || (mf_view_get_pre_state(ap) == MORE_EDIT && ap->mf_Status.more == MORE_RENAME)) {
			char *label = NULL;
			int count = mf_edit_file_count_get();

			if (count > 0) {
				char *tmp = mf_util_get_text(MF_LABEL_SELECTED);
				label = g_strdup_printf(tmp, count);
			} else {
				label = g_strdup(MF_LABEL_SELECT_ITEMS);
			}

			mf_object_item_text_set(ap->mf_MainWindow.pNaviItem, label, "elm.text.title");
			SAFE_FREE_CHAR(label);
		}
		GString *title = NULL;
		if (mf_fm_svc_wrapper_is_root_path(ap->mf_Status.path->str)) {
			mf_navi_bar_pathinfo_refresh(ap);
			title = g_string_new(LABEL_MYFILE_CHAP);
		} else {
			mf_navi_bar_pathinfo_refresh(ap);
			title = mf_fm_svc_wrapper_get_file_name(ap->mf_Status.path);
		}
		if (ap->mf_Status.view_type == mf_view_root && (ap->mf_Status.more != MORE_SEARCH || mf_view_get_pre_state(ap) != MORE_SEARCH)) {
			mf_category_storage_size_reset(mf_tray_item_category_none); 				//fix  P131121-00236 by ray
			mf_category_storage_size_refresh(mf_tray_item_category_none, NULL, NULL);	//
		}
		if (title != NULL) {
			g_string_free(title, TRUE);
			title = NULL;
		}
	}
	MF_TRACE_END;
}

EXPORT_API int main(int argc, char *argv[])
{
	MF_TRACE_BEGIN;
	t_start;
	MF_TA_INIT();
	MF_TA_ACUM_ITEM_BEGIN("1 Launch myfile", 0);
	MF_TA_ACUM_ITEM_BEGIN("12 Main", 0);
	ui_app_lifecycle_callback_s ops;
	int ret = APP_ERROR_NONE;
	struct appdata ad;
	app_event_handler_h hLanguageChangedHandle;
	app_event_handler_h hRegionFormatChangedHandle;

#ifdef MYFILE_CRITICAL_LOG
	ret = mf_log_init();
	if (ret != MYFILE_ERR_NONE) {
		mf_debug("initialize critical log failed");
	}
#endif
	memset(&ops, 0x0, sizeof(ui_app_lifecycle_callback_s));
	memset(&ad, 0x0, sizeof(struct appdata));

	ops.create = __mf_main_create_app;
	ops.terminate = __mf_main_terminate_app;
	ops.pause = __mf_main_stop_app;
	ops.resume = __mf_main_resume_app;
	ops.app_control = __mf_main_reset_app;

	ret = ui_app_add_event_handler(&hRegionFormatChangedHandle, APP_EVENT_REGION_FORMAT_CHANGED, __mf_language_changed_cb, (void*)&ad);
	if (ret != APP_ERROR_NONE) {
		mf_error("APP_EVENT_REGION_FORMAT_CHANGED ui_app_add_event_handler failed : [%d]!!!", ret);
		return -1;
	}

	ret = ui_app_add_event_handler(&hLanguageChangedHandle, APP_EVENT_LANGUAGE_CHANGED, __mf_language_changed_cb, (void*)&ad);
	if (ret != APP_ERROR_NONE) {
		mf_error("APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", ret);
		return -1;
	}
	MF_TA_ACUM_ITEM_END("12 Main", 0);

	MF_TRACE_END;
	t_end;
	return ui_app_main(argc, argv, &ops, &ad);
}
