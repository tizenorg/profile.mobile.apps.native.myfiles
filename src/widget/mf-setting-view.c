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


#include "mf-object-conf.h"
#include "mf-callback.h"
#include "mf-fm-svc-wrapper.h"
#include "mf-util.h"
#include "mf-launch.h"
#include "mf-ta.h"
#include "mf-resource.h"
#include "mf-object.h"
#include "mf-navi-bar.h"
#include <system_settings.h>

static Eina_Bool g_check_extension_status = EINA_FALSE;
#ifdef MYFILE_HIDEN_FILES_SHOW
static Eina_Bool g_check_hiden_status = EINA_FALSE;
#endif

static void __mf_setting_view_extension_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	Eina_Bool state = elm_check_state_get(obj);

	if (state == EINA_TRUE) {
		mf_util_set_extension_state(MF_EXTENSION_SHOW);
		ap->mf_Status.iExtensionState = MF_EXTENSION_SHOW;
	} else {
		mf_util_set_extension_state(MF_EXTENSION_HIDE);
		ap->mf_Status.iExtensionState = MF_EXTENSION_HIDE;
	}
}

#ifdef MYFILE_HIDEN_FILES_SHOW
static void __mf_setting_view_hiden_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;

	Eina_Bool state = elm_check_state_get(obj);

	if (state == EINA_TRUE) {
		mf_util_set_hiden_state(MF_HIDEN_SHOW);
	} else {
		mf_util_set_hiden_state(MF_HIDEN_HIDE);
	}
}
#endif

static void __mf_setting_view_item_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	/*when move event is called user intends to scroll hence unhighlight*/
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;
	Evas_Object *layout = (Evas_Object *) data;
	if (ev && ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) {
		elm_object_signal_emit(layout, "elm,state,unselected", "elm");
		return;
	}
	MF_TRACE_END;
}

static void __mf_setting_view_item_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	Evas_Object *layout = (Evas_Object *) data;
	elm_object_signal_emit(layout, "elm,state,selected", "elm");
	elm_object_signal_emit(layout, "elm,state,clicked", "elm");
	MF_TRACE_END;
}
Evas_Object *check_hiden = NULL;
Evas_Object *check_extension = NULL;

static void __mf_setting_view_extension_item_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
//	Evas_Object *check = elm_object_part_content_get(obj, "elm.icon.2");
	Evas_Object *check = check_extension;
	if (check) {
		Eina_Bool state = elm_check_state_get(check);
		mf_error("state = %d, g_check_extension_status=%d", state, g_check_extension_status);

		g_check_extension_status = !g_check_extension_status;
		elm_check_state_set(check, g_check_extension_status);
		//elm_object_signal_emit(obj, "elm,state,unselected", "elm");
		evas_object_smart_callback_call(check, "changed", NULL);
	}
	elm_object_signal_emit(obj, "elm,state,unselected", "elm");
	MF_TRACE_END;
}

#ifdef MYFILE_HIDEN_FILES_SHOW
static void __mf_setting_view_hiden_item_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	Evas_Object *check = check_hiden;
	if (check) {
		Eina_Bool state = elm_check_state_get(check);
		mf_error("state = %d, g_check_extension_status=%d", state, g_check_extension_status);

		g_check_hiden_status = !g_check_hiden_status;
		elm_check_state_set(check, g_check_hiden_status);
		//elm_object_signal_emit(obj, "elm,state,unselected", "elm");
		evas_object_smart_callback_call(check, "changed", NULL);
	}
	elm_object_signal_emit(obj, "elm,state,unselected", "elm");
	MF_TRACE_END;
}
#endif

static void __mf_setting_view_item_clicked(void *data, Evas_Object *obj, void *event_info)
{
	MF_TRACE_BEGIN;
	Evas_Object *layout = (Evas_Object *) data;
	elm_object_signal_emit(layout, "elm,state,unselected", "elm");
	MF_TRACE_END;
}

Evas_Object *mf_setting_view_content_create(void *data, Evas_Object *parent)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	Evas_Object *box = NULL;
	Evas_Object *separator1 = NULL;
	Evas_Object *extension = NULL;

	int extension_state = ap->mf_Status.iExtensionState;

	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	elm_object_style_set(scroller, "dialogue");

	box = elm_box_add(scroller);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);
	elm_object_content_set(scroller, box);
	evas_object_show(box);

	separator1 = elm_layout_add(box);
	elm_layout_theme_set(separator1, "layout", "dialogue", "separator");
	evas_object_size_hint_weight_set(separator1, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(separator1, EVAS_HINT_FILL, 0.0);
	elm_box_pack_end(box, separator1);
	evas_object_show(separator1);

	extension = elm_layout_add(box);
	elm_layout_theme_set(extension, "genlist/item", "1line", "default");
	evas_object_size_hint_weight_set(extension, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(extension, EVAS_HINT_FILL, 0.0);
	//evas_object_resize(extension, -1, 76);
	mf_object_text_set(extension, MF_LABEL_SHOW_EXTENSION, "elm.text.main.left");
	elm_box_pack_end(box, extension);
	evas_object_show(extension);
	evas_object_event_callback_add(extension, EVAS_CALLBACK_MOUSE_DOWN, __mf_setting_view_item_down, extension);
	evas_object_event_callback_add(extension, EVAS_CALLBACK_MOUSE_MOVE, __mf_setting_view_item_move, extension);
	evas_object_event_callback_add(extension, EVAS_CALLBACK_MOUSE_UP, __mf_setting_view_extension_item_up, ap);
	//elm_object_signal_emit(extension, "elm,state,top", "");


	Evas_Object *content = elm_layout_add(extension);
	elm_layout_theme_set(content, "layout",
				"list/C/type.2", "default");
					
	check_extension = elm_check_add(extension);
	//evas_object_resize(check_extension, -1, 46);
	evas_object_propagate_events_set(check_extension, EINA_FALSE);
	elm_layout_content_set(content, "elm.swallow.content", check_extension);
	elm_object_part_content_set(extension, "elm.icon.2", content);
	evas_object_smart_callback_add(check_extension, "changed", __mf_setting_view_extension_check_clicked_cb, ap);
	//elm_object_signal_emit(extension, "elm,state,center", "");
	if (extension_state == MF_EXTENSION_SHOW) {
		elm_check_state_set(check_extension, EINA_TRUE);
		g_check_extension_status = EINA_TRUE;
	} else {
		elm_check_state_set(check_extension, EINA_FALSE);
		g_check_extension_status = EINA_FALSE;
	}

	Evas_Object *button1;
	button1 = elm_button_add(parent);
	elm_object_style_set(button1, "focus_top");
	evas_object_size_hint_align_set(button1, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(extension, "elm.icon.focus", button1);
	evas_object_smart_callback_add(button1, "clicked", __mf_setting_view_item_clicked, extension);
	//elm_layout_theme_set(ly, "genlist/item", "groupindex", "default");
#ifdef MYFILE_HIDEN_FILES_SHOW
	Evas_Object *hiden = elm_layout_add(box);
	//evas_object_resize(hiden, -1, 76);
	elm_layout_theme_set(hiden, "genlist/item", "1line", "default");
	//elm_layout_theme_set(hiden, "genlist/item", "groupindex", "1text.1icon.2");
	evas_object_size_hint_weight_set(hiden, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(hiden, EVAS_HINT_FILL, 0.0);
	mf_object_text_set(hiden, MF_LABEL_HIDEN_SHOW, "elm.text.main.left");
	elm_box_pack_end(box, hiden);
	evas_object_show(hiden);
	evas_object_event_callback_add(hiden, EVAS_CALLBACK_MOUSE_DOWN, __mf_setting_view_item_down, hiden);
	evas_object_event_callback_add(hiden, EVAS_CALLBACK_MOUSE_MOVE, __mf_setting_view_item_move, hiden);
	evas_object_event_callback_add(hiden, EVAS_CALLBACK_MOUSE_UP, __mf_setting_view_hiden_item_up, ap);
	elm_object_signal_emit(hiden, "elm,state,bottom", "");

	content = elm_layout_add(extension);
	elm_layout_theme_set(content, "layout",
				"list/C/type.2", "default");

	check_hiden = elm_check_add(hiden);
	//evas_object_resize(check_hiden, -1, 46);
	evas_object_propagate_events_set(check_hiden, EINA_FALSE);
	elm_layout_content_set(content, "elm.swallow.content", check_hiden);
	elm_object_part_content_set(hiden, "elm.icon.2", content);
	//elm_object_part_content_set(hiden, "elm.icon.2", check_hiden);
	evas_object_smart_callback_add(check_hiden, "changed", __mf_setting_view_hiden_check_clicked_cb, ap);
	//elm_object_signal_emit(extension, "elm,state,center", "");
	int hiden_state = 0;
	mf_util_get_pref_value(PREF_TYPE_HIDEN_STATE, &hiden_state);
	if (hiden_state == MF_HIDEN_SHOW) {
		mf_error();
		elm_check_state_set(check_hiden, EINA_TRUE);
		g_check_hiden_status = EINA_TRUE;
	} else {
		mf_error();
		elm_check_state_set(check_hiden, EINA_FALSE);
		g_check_hiden_status = EINA_FALSE;
	}

	Evas_Object *button2;
	button2 = elm_button_add(parent);
	elm_object_style_set(button2, "focus_bottom");
	evas_object_size_hint_align_set(button2, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(hiden, "elm.icon.focus", button2);
	evas_object_smart_callback_add(button2, "clicked", __mf_setting_view_item_clicked, hiden);
#endif

	return scroller;

}

Eina_Bool mf_setting_view_back_cb(void *data, Elm_Object_Item *it)
{
	mf_retvm_if(data == NULL, EINA_FALSE, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	mf_callback_cancel_cb(ap, NULL, NULL);
	return EINA_FALSE;
}

void mf_setting_view_create(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;

	mf_navi_bar_reset_navi_obj(ap);


	Evas_Object *content = mf_setting_view_content_create(ap, ap->mf_MainWindow.pNaviBar);

	ap->mf_Status.pPreNaviItem = ap->mf_MainWindow.pNaviItem;
	if (ap->mf_Status.pPreNaviItem) {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_insert_after(ap->mf_MainWindow.pNaviBar, ap->mf_Status.pPreNaviItem, NULL, NULL, NULL, content, MF_NAVI_STYLE_ENABLE);
	} else {
		ap->mf_MainWindow.pNaviItem = elm_naviframe_item_push(ap->mf_MainWindow.pNaviBar, NULL, NULL, NULL, content, MF_NAVI_STYLE_ENABLE);
	}

	mf_navi_add_back_button(ap, mf_setting_view_back_cb);

	/*hide Tab Bar in search view*/

	/*add control bar for navigation bar*/
	/*temp data free*/
	mf_navi_bar_title_content_set(ap, MF_LABEL_SETTINGS);

	MF_TRACE_END;

}

void update_lang(void)
{
    char *lang = NULL;
    char *baselang = NULL;
    char *r = NULL;
	int retcode = -1;
	retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &lang);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
	    mf_error("[ERR] failed to get the language");
	}
    if (lang) {
        setenv("LANG", lang, 1);
        setenv("LC_MESSAGES", lang, 1);
        r = setlocale(LC_ALL, "");
        if (r == NULL) {
            retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &baselang);
            if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
                mf_error("[ERR] failed to get the language");
            }
            if (baselang) {
                setlocale(LC_ALL, baselang);
                free(baselang);
            }
        }
        free(lang);
    }
}

void update_region(void)
{
    char *region = NULL;
    int retcode = -1;
    retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY, &region);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
        mf_error("[ERR] failed to get the region");
	}
    if (region) {
        setenv("LC_CTYPE", region, 1);
        setenv("LC_NUMERIC", region, 1);
        setenv("LC_TIME", region, 1);
        setenv("LC_COLLATE", region, 1);
        setenv("LC_MONETARY", region, 1);
        setenv("LC_PAPER", region, 1);
        setenv("LC_NAME", region, 1);
        setenv("LC_ADDRESS", region, 1);
        setenv("LC_TELEPHONE", region, 1);
        setenv("LC_MEASUREMENT", region, 1);
        setenv("LC_IDENTIFICATION", region, 1);
        free(region);
    }
}

static int __set_i18n(const char *domain, const char *dir)
{
	mf_error("===============================");
	char *r;
	if (domain == NULL) {
		errno = EINVAL;
		return -1;
	}

	r = setlocale(LC_ALL, "");
	/* if locale is not set properly, try again to set as language base */
	if (r == NULL) {
        char *lang = NULL;
        int retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &lang);
        if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
            mf_error("[ERR] failed to get the language");
        }
        if (lang) {
            setlocale(LC_ALL, lang);
            free(lang);
        }
	}
	bindtextdomain(domain, dir);
	textdomain(domain);
	return 0;
}

int mf_setting_set_i18n(char *pkgname, char *localedir)
{
	mf_error("===============================");
    //update_lang();
   // update_region();

    return __set_i18n(pkgname, localedir);
}

