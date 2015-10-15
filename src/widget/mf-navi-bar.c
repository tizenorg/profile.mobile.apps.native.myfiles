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
#include "mf-gengrid.h"
#include "mf-util.h"
#include "mf-ta.h"
#include "mf-resource.h"
#include "mf-launch.h"
#include "mf-tray-item.h"
#include "mf-callback.h"
#include "mf-search-view.h"
#include "mf-genlist.h"
#include "mf-object.h"
#include "mf-view.h"
#include "mf-object-item.h"
#include "mf-thumb-gen.h"
#include "mf-focus-ui.h"
#include "mf-fs-monitor.h"
#include "mf-edit-view.h"


#define MF_PATH_INFO_W  480
#define MF_PATH_INFO_H  76

void mf_navi_bar_reset_navi_obj(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	if (ap->mf_MainWindow.pNaviGenlist) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviGenlist);
		ap->mf_MainWindow.pNaviGenlist = NULL;
	}
	if (ap->mf_MainWindow.pNaviGengrid) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviGengrid);
		ap->mf_MainWindow.pNaviGengrid = NULL;
	}
	if (ap->mf_MainWindow.pNaviLayout) {
		SAFE_FREE_OBJ(ap->mf_MainWindow.pNaviLayout);
		ap->mf_MainWindow.pNaviLayout = NULL;
	}
	t_end;
	MF_TRACE_END;
}

Evas_Object *mf_navi_bar_create(Evas_Object * parent)
{
	MF_TRACE_BEGIN;
	Evas_Object *navi_bar;
	assert(parent);
	navi_bar = elm_naviframe_add(parent);
	elm_naviframe_prev_btn_auto_pushed_set(navi_bar, EINA_FALSE);

	evas_object_show(navi_bar);
	MF_TRACE_END;
	return navi_bar;
}

char *mf_navi_bar_path_info_get(void *data, mf_navi_pathinfo_type type)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	char *info = NULL;

	switch (type) {
	case mf_navi_pathinfo_root:
		info = g_strdup(MF_LABE_HEADER_STORAGE);
		break;
	case mf_navi_pathinfo_normal:
		info = mf_fm_svc_path_info_get(ap->mf_Status.path->str);
		break;
	case mf_navi_pathinfo_recent:
		info = g_strdup(MF_LABEL_RECTENT_FILES);
		break;
	case mf_navi_pathinfo_category:
		info = g_strdup(ap->mf_Status.categorytitle);
		break;
	default:
		return NULL;
	}
	return info;
}

Evas_Object *mf_navi_bar_create_normal_pathinfo(void *data)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	Evas_Object *pathinfo_obj = NULL;
	pathinfo_obj = mf_genlist_create_path_tab(ap->mf_MainWindow.pNaviBar, ap->mf_Status.path->str, ap);

	//char *info = NULL;
	//info = mf_navi_bar_path_info_get(ap, type);

	//mf_genlist_create_path_info(ap->mf_MainWindow.pNaviBar, info, pathinfo, EINA_TRUE);
	//SAFE_FREE_CHAR(info);
	//elm_object_scroll_freeze_push(genlist);

	MF_TRACE_END;

	return pathinfo_obj;
}

void mf_navi_bar_pathinfo_refresh(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Evas_Object *path_tab = elm_object_part_content_get(ap->mf_MainWindow.pNaviLayout, "pathinfo");
	if (path_tab) {
		elm_object_part_content_unset(ap->mf_MainWindow.pNaviLayout, "pathinfo");
		evas_object_del(path_tab);
		path_tab = NULL;
		path_tab = mf_navi_bar_create_normal_pathinfo(ap);
		elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "pathinfo", path_tab);
	}
}


/******************************
** Prototype    : mfNaviBarSetSegment
** Description  :
** Input        : struct appdata* data
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
static void _mf_navi_bar_create_content_set_focus(void *data, Evas_Object *pContent)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
#ifdef MYFILE_ENABLE_FOCUS
	//Focus UI
	_mf_focus_ui_set_focus(pContent);

	//_mf_focus_ui_set_dual_focus_order(pContent, ap->mf_MainWindow.pNaviCtrlBar, MF_FOCUS_DUAL_ORDER_NEXT_PRIV);
	//_mf_focus_ui_set_dual_focus_order(pContent, ap->mf_MainWindow.pNaviCtrlBar, MF_FOCUS_DUAL_ORDER_DOWN_UP);
#endif
	MF_TRACE_END;
}

Evas_Object *mf_navi_bar_create_view_content(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retvm_if(data == NULL, NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	ap->mf_MainWindow.pNaviGenlist = NULL;
	ap->mf_MainWindow.pNaviGengrid = NULL;
	Evas_Object *pContent = NULL;
	int view_style = mf_view_style_get(ap);
	if (ap->mf_Status.flagNoContent) {
		pContent = mf_object_create_no_content(ap->mf_MainWindow.pNaviBar);
		mf_object_text_set(pContent, MF_LABEL_NO_FILES, "elm.text");
	} else {
		if (view_style == MF_VIEW_STYLE_THUMBNAIL) {
			pContent = mf_gengrid_create_list(ap, ap->mf_MainWindow.pNaviBar);
			ap->mf_MainWindow.pNaviGengrid = pContent;
		} else {
			MF_TA_ACUM_ITEM_BEGIN("123456 mf_genlist_create_list", 0);
			pContent = mf_genlist_create_list(ap, ap->mf_MainWindow.pNaviBar);
			MF_TA_ACUM_ITEM_END("123456 mf_genlist_create_list", 0);
			ap->mf_MainWindow.pNaviGenlist = pContent;
		}
	}

	_mf_navi_bar_create_content_set_focus(ap, pContent);

	if (ap->mf_Status.more == MORE_DEFAULT
	    || ap->mf_Status.more == MORE_INTERNAL_COPY
	    || ap->mf_Status.more == MORE_INTERNAL_MOVE) {
		mf_fs_monitor_add_dir_watch(ap->mf_Status.path->str, ap);
	}
	t_end;

	MF_TRACE_END;
	return pContent;
}

/******************************
** Prototype    : mf_navi_bar_set_content
** Description  :
** Input        : Evas_Object *pLayout
**                Evas_Object *NaviContent
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
void mf_navi_bar_layout_content_set(Evas_Object *layout, Evas_Object *content)
{
	elm_object_part_content_set(layout, "elm.swallow.content", content);
}

void mf_navi_bar_clean_content(void *data, Evas_Object *pLayout)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(pLayout == NULL, "pConform is NULL");
	//struct appdata *ap = (struct appdata *) data;

	/*Evas_Object *unUsed = mf_object_unset_part_content(pLayout, "content");
	SAFE_FREE_OBJ(unUsed);*/
	Evas_Object *unUsed = mf_object_unset_part_content(pLayout, "elm.swallow.content");
	SAFE_FREE_OBJ(unUsed);
//	ap->mf_MainWindow.pNaviGenlist = NULL;//Fixed P140804-07325, don't need to set it as NULL.
//	ap->mf_MainWindow.pNaviGengrid = NULL;
	MF_TRACE_END;
}

void mf_navi_bar_set_content(void *data, Evas_Object *pLayout, Evas_Object *NaviContent)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	mf_retm_if(pLayout == NULL, "pConform is NULL");
	mf_retm_if(NaviContent == NULL, "NaviContent is NULL");
	struct appdata *ap = (struct appdata *)data;

	mf_navi_bar_clean_content(data, pLayout);
	
	ap->mf_MainWindow.pNaviBox = mf_object_create_box(ap->mf_MainWindow.pNaviLayout);
	mf_navi_bar_layout_content_set(ap->mf_MainWindow.pNaviLayout, ap->mf_MainWindow.pNaviBox);

	elm_box_pack_end(ap->mf_MainWindow.pNaviBox, NaviContent);
	/*if bs, use idler here*/
	MF_TRACE_END;
}

void mf_navi_bar_title_content_set(void *data, const char *title)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;

	Elm_Object_Item *navi_it = ap->mf_MainWindow.pNaviItem;

	mf_object_item_text_set(navi_it, title, "elm.text.title");
	MF_TRACE_END;

}

Evas_Object *mf_navi_bar_home_button_create(Evas_Object *parent, Evas_Smart_Cb func, void *user_data)
{
	Evas_Object *home_ic = elm_image_add(parent);
	elm_image_file_set(home_ic, EDJ_IMAGE, MF_TITLE_ICON_HOME);
	elm_image_resizable_set(home_ic, EINA_TRUE, EINA_TRUE);
	evas_object_show(home_ic);

	Evas_Object *home_btn = mf_object_create_button(parent,
					      "naviframe/title_icon",
					      NULL,
					      home_ic,
					      func,
					      user_data,
					      EINA_FALSE);
	return home_btn;
}

Evas_Object *mf_navi_bar_search_button_create(Evas_Object *parent, Evas_Smart_Cb func, void *user_data)
{

	Evas_Object *image = elm_image_add(parent);
	elm_image_file_set(image, EDJ_IMAGE, MF_TITLE_ICON_SEARCH);
	elm_image_resizable_set(image, EINA_TRUE, EINA_TRUE);
	evas_object_show(image);
	
	Evas_Object *up_btn = mf_object_create_button(parent,
					      "naviframe/title_icon",
					      NULL,
					      image,
					      func,
					      user_data,
					      EINA_FALSE);
	return up_btn;
}


Evas_Object *mf_navi_bar_upper_button_create(Evas_Object *parent, Evas_Smart_Cb func, void *user_data)
{
	Evas_Object *up_ic = elm_image_add(parent);
	elm_image_file_set(up_ic, EDJ_IMAGE, MF_TITLE_ICON_UPPER);
	elm_image_resizable_set(up_ic, EINA_TRUE, EINA_TRUE);
	evas_object_show(up_ic);

	Evas_Object *up_btn = mf_object_create_button(parent,
					      "naviframe/title_icon",
					      NULL,
					      up_ic,
					      func,
					      user_data,
					      EINA_FALSE);
	return up_btn;
}

Evas_Object *mf_navi_bar_select_all_button_create(Evas_Object *parent, Evas_Smart_Cb func, void *user_data)
{
	Evas_Object *select_ic = elm_image_add(parent);
	elm_image_file_set(select_ic, EDJ_IMAGE, MF_TITLE_ICON_SELECT_ALL);
	elm_image_resizable_set(select_ic, EINA_TRUE, EINA_TRUE);
	evas_object_show(select_ic);

	Evas_Object *select_btn = mf_object_create_button(parent,
					      "naviframe/title_icon",
					      NULL,
					      select_ic,
					      func,
					      user_data,
					      EINA_FALSE);

	return select_btn;
}

/*static void _mf_navi_bar_set_focus(void *data, Evas_Object *search_btn)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

#ifdef MYFILE_ENABLE_FOCUS
	//Focus UI
	_mf_focus_ui_set_focus(search_btn);

	//_mf_focus_ui_set_dual_focus_order(search_btn, ap->mf_MainWindow.category_image, MF_FOCUS_DUAL_ORDER_NEXT_PRIV);
	//_mf_focus_ui_set_dual_focus_order(search_btn, ap->mf_MainWindow.category_video, MF_FOCUS_DUAL_ORDER_DOWN_UP);
#endif
	MF_TRACE_END;
}*/

void mf_navi_bar_title_set(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	int more = ap->mf_Status.more;
	Elm_Object_Item *navi_it = ap->mf_MainWindow.pNaviItem;
	Evas_Object *pNavi = ap->mf_MainWindow.pNaviBar;
	Evas_Object *unset = NULL;
	char *title = NULL;

	if (ap->mf_Status.more == MORE_SEARCH
	    || ap->mf_Status.more == MORE_EDIT
	    || ap->mf_Status.more == MORE_SHARE_EDIT
	    || ap->mf_Status.more == MORE_EDIT_DETAIL
		|| ap->mf_Status.more == MORE_EDIT_COPY
		|| ap->mf_Status.more == MORE_EDIT_MOVE
		|| ap->mf_Status.more == MORE_EDIT_DELETE
		|| ap->mf_Status.more == MORE_EDIT_DELETE_SHORTCUT
		|| ap->mf_Status.more == MORE_EDIT_DELETE_RECENT
	   ) {
		if (more == MORE_EDIT) {
			title = MF_LABEL_SELECT_ITEMS;
		} else if (more == MORE_SHARE_EDIT) {
			title = MF_LABEL_SELECT_ITEMS;
		} else if ((ap->mf_Status.view_type == mf_view_root_category) && (ap->mf_Status.categorytitle != NULL)) {
			title = ap->mf_Status.categorytitle ;
		} else {
			title = ap->mf_MainWindow.naviframe_title;
		}
		mf_navi_bar_title_content_set(ap, title);
	} else if (more != MORE_EDIT && more != MORE_SEARCH) {
		if (ap->mf_Status.view_type == mf_view_normal) {
			if (more == MORE_INTERNAL_COPY || more == MORE_INTERNAL_MOVE || more == MORE_INTERNAL_DECOMPRESS) {
				Evas_Object *home_btn = mf_navi_bar_home_button_create(pNavi, mf_callback_home_button_cb, ap);
				unset = elm_object_item_part_content_unset(navi_it, TITLE_LEFT_BTN);
				SAFE_FREE_OBJ(unset);
				elm_object_item_part_content_set(navi_it, TITLE_LEFT_BTN, home_btn);

				Evas_Object *upper_btn = mf_navi_bar_upper_button_create(pNavi, mf_callback_upper_click_cb, ap);
				unset = elm_object_item_part_content_unset(navi_it, TITLE_RIGHT_BTN);
				SAFE_FREE_OBJ(unset);
				elm_object_item_part_content_set(navi_it, TITLE_RIGHT_BTN, upper_btn);
			} else {
				Evas_Object *home_btn = mf_navi_bar_home_button_create(pNavi, mf_callback_home_button_cb, ap);
				unset = elm_object_item_part_content_unset(navi_it, TITLE_LEFT_BTN);
				SAFE_FREE_OBJ(unset);
				elm_object_item_part_content_set(navi_it, TITLE_LEFT_BTN, home_btn);

				Evas_Object *search_btn = mf_navi_bar_search_button_create(pNavi, mf_search_bar_enter_search_routine, ap);
				unset = elm_object_item_part_content_unset(navi_it, TITLE_RIGHT_BTN);
				SAFE_FREE_OBJ(unset);
				elm_object_item_part_content_set(navi_it, TITLE_RIGHT_BTN, search_btn);
			}
		} else if (ap->mf_Status.view_type == mf_view_root && more == MORE_DEFAULT) {
		Evas_Object *search_btn = mf_navi_bar_search_button_create(pNavi, mf_search_bar_enter_search_routine, ap);
		unset = elm_object_item_part_content_unset(navi_it, TITLE_RIGHT_BTN);
		SAFE_FREE_OBJ(unset);
		elm_object_item_part_content_set(navi_it, TITLE_RIGHT_BTN, search_btn);
		}
		if ((ap->mf_Status.view_type == mf_view_root_category) && (ap->mf_Status.categorytitle != NULL)) {
			mf_error("ap->mf_Status.categorytitle = %s",ap->mf_Status.categorytitle);
			title = ap->mf_Status.categorytitle ;
			Evas_Object *search_btn = mf_navi_bar_search_button_create(pNavi, mf_search_bar_enter_search_routine, ap);
			unset = elm_object_item_part_content_unset(navi_it, TITLE_RIGHT_BTN);
			SAFE_FREE_OBJ(unset);
			elm_object_item_part_content_set(navi_it, TITLE_RIGHT_BTN, search_btn);
		} else {
			title = ap->mf_MainWindow.naviframe_title;
		}
		mf_navi_bar_title_content_set(ap, title);
	}
	t_end;

	MF_TRACE_END;
}

/******************************
** Prototype    : __mf_navi_bar_select_count_label_timeout_cb
** Description  :
** Input        : void *data
**                Evas_Object *obj
**                void *event_info
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
Evas_Object *__mf_navi_bar_backbutton_create(Evas_Object *parent, Evas_Object *win)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(parent == NULL, NULL, "parent is NULL");
	mf_retvm_if(win == NULL, NULL, "win is NULL");

	Evas_Object *btn = NULL;
	btn = elm_button_add(parent);
	elm_object_style_set(btn, "naviframe/end_btn/default");
	evas_object_show(btn);
	MF_TRACE_END;
	return btn;
}

void mf_navi_add_back_button(void *data, Eina_Bool (*func)(void *data, Elm_Object_Item *it))
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;
	if (ap->mf_Status.view_type != mf_view_root) {
		Evas_Object *pBackButton = NULL;
		pBackButton = __mf_navi_bar_backbutton_create(ap->mf_MainWindow.pNaviBar, ap->mf_MainWindow.pWindow);
		evas_object_smart_callback_add(pBackButton, "clicked", func, ap);

		if (pBackButton) {
			Evas_Object *unset = elm_object_item_part_content_unset(ap->mf_MainWindow.pNaviItem, "prev_btn");
			SAFE_FREE_OBJ(unset);
			elm_object_item_part_content_set(ap->mf_MainWindow.pNaviItem, "prev_btn", pBackButton);
			evas_object_event_callback_add(pBackButton, EVAS_CALLBACK_MOUSE_UP, mf_callback_mouseup_cb, ap);
			evas_object_event_callback_add(pBackButton, EVAS_CALLBACK_KEY_DOWN, mf_callback_keydown_cb, ap);
		}
	}
	elm_naviframe_item_pop_cb_set(ap->mf_MainWindow.pNaviItem, func, ap);
}

void mf_navi_bar_set_ctrlbar(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Elm_Object_Item *navi_it = ap->mf_MainWindow.pNaviItem;
	Evas_Object *more_bt = NULL;
//Prevent issue fix
//	Evas_Object *toolbar = NULL;

	switch (ap->mf_Status.more) {
	case MORE_DEFAULT:
		more_bt = mf_object_create_button(ap->mf_MainWindow.pNaviBar, NAVI_BUTTON_EDIT, MF_LABEL_MORE, NULL, (Evas_Smart_Cb)mf_callback_more_button_cb, ap, EINA_FALSE);
		break;
	case MORE_EDIT:
		more_bt = mf_object_create_button(ap->mf_MainWindow.pNaviBar, NAVI_BUTTON_EDIT, MF_LABEL_MORE, NULL, (Evas_Smart_Cb)mf_callback_more_button_cb, ap, EINA_FALSE);
		//toolbar = mf_object_toolbar_create(ap->mf_MainWindow.pNaviBar);
		//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_DELETE, mf_callback_delete_cb, ap);
		//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_MOVE, mf_callback_copy_move_cb, ap);
		//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_COPY, mf_callback_copy_move_cb, ap);
		break;
	case MORE_INTERNAL_COPY:
	case MORE_DATA_COPYING:
		if (ap->mf_Status.view_type != mf_view_root) {
			more_bt = mf_object_create_button(ap->mf_MainWindow.pNaviBar, NAVI_BUTTON_EDIT, MF_LABEL_MORE, NULL, (Evas_Smart_Cb)mf_callback_more_button_cb, ap, EINA_FALSE);
			//toolbar = mf_object_toolbar_create(ap->mf_MainWindow.pNaviBar);
			//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_CANCEL, mf_callback_cancel_cb, ap);
			//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_COPY_HERE, mf_callback_paste_here_cb, ap);
		}
		break;
	case MORE_INTERNAL_MOVE:
	case MORE_DATA_MOVING:
		if (ap->mf_Status.view_type != mf_view_root) {
			more_bt = mf_object_create_button(ap->mf_MainWindow.pNaviBar, NAVI_BUTTON_EDIT, MF_LABEL_MORE, NULL, (Evas_Smart_Cb)mf_callback_more_button_cb, ap, EINA_FALSE);
			//toolbar = mf_object_toolbar_create(ap->mf_MainWindow.pNaviBar);
			//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_CANCEL, mf_callback_cancel_cb, ap);
			//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_MOVE_HERE, mf_callback_move_here_cb, ap);
		}
		break;
	case MORE_SHARE_EDIT:
		//toolbar = mf_object_toolbar_create(ap->mf_MainWindow.pNaviBar);
		//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_CANCEL, mf_callback_cancel_cb, ap);
		//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_SHARE, mf_callback_share_cb, ap);
                break;

	case MORE_INTERNAL_DECOMPRESS:
		if (ap->mf_Status.view_type != mf_view_root) {
			more_bt = mf_object_create_button(ap->mf_MainWindow.pNaviBar, NAVI_BUTTON_EDIT, MF_LABEL_MORE, NULL, (Evas_Smart_Cb)mf_callback_more_button_cb, ap, EINA_FALSE);
			//toolbar = mf_object_toolbar_create(ap->mf_MainWindow.pNaviBar);
			//mf_object_item_tabbar_item_append(toolbar, NULL, LABEL_CANCEL, mf_callback_cancel_cb, ap);
			//mf_object_item_tabbar_item_append(toolbar, NULL, MF_LABEL_DECOMPRESS_HERE, mf_callback_decompress_here_cb, ap);
		}
          break;
	default:
		break;
	}
	if (more_bt) {
		elm_object_item_part_content_set(navi_it, NAVI_MORE_BUTTON_PART, more_bt);
		evas_object_event_callback_add(more_bt, EVAS_CALLBACK_KEY_DOWN, mf_callback_more_keydown_cb, ap);
	}
	//Prevent issue fix
/*	if (toolbar) {
		ap->mf_MainWindow.pNaviCtrlBar = toolbar;
		elm_object_item_part_content_set(navi_it, NAVI_CTRL_PART, toolbar);
		
		elm_object_focus_allow_set(ap->mf_MainWindow.pNaviCtrlBar, EINA_TRUE);
	}*/
	elm_object_item_signal_callback_add(navi_it, "elm,action,more_event", "", mf_callback_hardkey_more_cb, ap);
	t_end;
	MF_TRACE_END;

}

void mf_navi_bar_reset_ctrlbar(void *data)
{
	MF_TRACE_BEGIN;
	mf_retm_if(data == NULL, "data is NULL");
	struct appdata *ap = (struct appdata *)data;
	Elm_Object_Item *navi_it = ap->mf_MainWindow.pNaviItem;
	mf_retm_if(navi_it == NULL, "navi_it is NULL");

	Evas_Object *more_bt = NULL;
	Evas_Object *ctrlbar = NULL;
	more_bt = elm_object_item_part_content_unset(navi_it, NAVI_MORE_BUTTON_PART);
	SAFE_FREE_OBJ(more_bt);
	ctrlbar = elm_object_item_part_content_unset(navi_it, NAVI_CTRL_PART);
	SAFE_FREE_OBJ(ctrlbar);
	mf_navi_bar_set_ctrlbar(ap);
	MF_TRACE_END;
}

void mf_navi_bar_layout_state_set(Evas_Object *layout, int type)
{
	mf_error(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  type is [%d]", type);
	mf_retm_if(layout == NULL, "layout is NULL");
	switch (type) {
	case mf_navi_layout_normal:
		elm_object_signal_emit(layout, "elm.pathinfo.show", "elm");
		elm_object_signal_emit(layout, "elm.content.show", "elm");
		break;
	case mf_navi_layout_content_only:
		elm_object_signal_emit(layout, "elm.pathinfo.hide", "elm");
		elm_object_signal_emit(layout, "elm.content.only", "elm");
		break;
	case mf_navi_layout_root_all:
		elm_object_signal_emit(layout, "elm.category.show", "elm");
		elm_object_signal_emit(layout, "elm.tabbar.hide", "elm");
		elm_object_signal_emit(layout, "elm.content.show", "elm");
		break;
	case mf_navi_layout_root_all_horizon:
		elm_object_signal_emit(layout, "elm.category.horizon", "elm");
		elm_object_signal_emit(layout, "elm.tabbar.hide", "elm");
		elm_object_signal_emit(layout, "elm.content.show", "elm");
		break;
	}
	return;
}

/******************************
** Prototype    : mf_edit_view_create
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

void mf_navi_bar_remove_info_box(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	if (ap->mf_Status.view_type != mf_view_root_category && ap->mf_Status.more != MORE_SEARCH) {
		mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_content_only);
	}
	MF_TRACE_END;
}

void mf_navi_bar_recover_info_box(void *data)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retm_if(ap == NULL, "ap is NULL");

	if (ap->mf_Status.view_type != mf_view_root_category) {
		mf_navi_bar_layout_state_set(ap->mf_MainWindow.pNaviLayout, mf_navi_layout_normal);
	}
	MF_TRACE_END;
	return;
}

/******************************
** Prototype    : mfNaviBarListItemRemove
** Description  : remove the navi bar item from the list by label
** Input        : void *data
**                const char *pNaviLabel
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
bool mf_navi_bar_reset_item_by_location(void *data, int location)
{
	MF_TRACE_BEGIN;
	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap == NULL, false, "ap is NULL");

	bool bEditStartFlag = FALSE;
	bool bInUseFlag = FALSE;
	if (ap->mf_MainWindow.record.location == location) {
		bEditStartFlag = TRUE;
	}
	if (ap->mf_MainWindow.location == location) {
		bInUseFlag = TRUE;
	}

	if (bInUseFlag == TRUE) {
		SAFE_FREE_GSTRING(ap->mf_Status.path);
		ap->mf_Status.path = g_string_new(ap->mf_MainWindow.record.path);
		ap->mf_MainWindow.location = ap->mf_MainWindow.record.location;
		ap->mf_Status.view_type = ap->mf_MainWindow.record.view_type;
	}
	MF_TRACE_END;
	return bEditStartFlag;
}

void mf_navi_bar_recover_state(void *data)
{
	MF_TRACE_BEGIN;
	mf_debug();
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	/*1     recover all the navigation bar*/
	SAFE_FREE_GSTRING(ap->mf_Status.path);
	ap->mf_Status.path = g_string_new(ap->mf_MainWindow.record.path);
	ap->mf_Status.more = ap->mf_MainWindow.record.more;
	ap->mf_Status.view_type = ap->mf_MainWindow.record.view_type;
	ap->mf_MainWindow.location = ap->mf_MainWindow.record.location;
	mf_view_reset_record(&ap->mf_MainWindow.record);
	MF_TRACE_END;
}

void mf_navi_bar_reset(void *data)
{
	MF_TRACE_BEGIN;
	mf_debug();
	struct appdata *ap = (struct appdata *)data;
	assert(ap);

	/*1     recover all the navigation bar*/
	SAFE_FREE_GSTRING(ap->mf_Status.path);
	ap->mf_Status.path = g_string_new(PHONE_FOLDER);
	ap->mf_Status.more = MORE_DEFAULT;
	ap->mf_Status.view_type = mf_view_root;
	ap->mf_MainWindow.location = MYFILE_PHONE;
	mf_view_reset_record(&ap->mf_MainWindow.record);
	MF_TRACE_END;
}


void
my_elm_object_item_disabled_set(Evas_Object *ctrlbar, Elm_Object_Item *it, Eina_Bool disabled)
{//Fixed P131025-04748 by jian12.li.
	MF_TRACE_BEGIN;
	const char *button_label = elm_object_item_part_text_get(it, NAVI_CTRL_TEXT_PART);
	mf_error("my_elm_object_item_disabled_set button_label is [%s]", button_label);
	elm_object_item_disabled_set(it, disabled);
	if (disabled) {
		elm_object_focus_allow_set(ctrlbar, false);
		//elm_object_focus_allow_set(it, false);
	}
 	MF_TRACE_END;
}

void mf_navi_bar_set_ctrlbar_item_disable(Elm_Object_Item *navi_it, int disable_item, bool disable)
{
	MF_TRACE_BEGIN;
	mf_retm_if(navi_it == NULL, "navi_it is NULL");
	Evas_Object *btn = elm_object_item_part_content_get(navi_it, "title_right_btn");
	if (btn) {
		if (mf_edit_file_count_get() <= 0) {
			elm_object_disabled_set(btn, EINA_TRUE);
		} else {
			elm_object_disabled_set(btn, EINA_FALSE);
		}
	}

	MF_TRACE_END;
}

int mf_navi_bar_get_disable_ctrlbar_item(Elm_Object_Item *navi_it)
{
	MF_TRACE_BEGIN;
	mf_retvm_if(navi_it == NULL, 0, "navi_it is NULL");
	int disable_item = 0;

	Evas_Object *ctrlbar = elm_object_item_part_content_get(navi_it, NAVI_CTRL_PART);
	Elm_Object_Item *item = NULL;
	const char *button_label = NULL;
	item = elm_toolbar_first_item_get(ctrlbar);

	while (item) {
		button_label = elm_object_item_part_text_get(item, NAVI_CTRL_TEXT_PART);
		if (elm_object_item_disabled_get(item)) {
			if (g_strcmp0(button_label, mf_util_get_text(LABEL_DELETE)) == 0) {
				disable_item |= CTRL_DISABLE_DELETE;
			}
			if (g_strcmp0(button_label, mf_util_get_text(LABEL_SHARE)) == 0) {
				disable_item |= CTRL_DISABLE_SEND;
			}

		}
		item = elm_toolbar_item_next_get(item);
	}

	MF_TRACE_END;

	return disable_item;
}

void mf_navi_bar_title_label_update(char **naviframe_title, const char *title_label)
{
	MF_TRACE_BEGIN;
	mf_retm_if(naviframe_title == NULL, "pNavi_s is NULL");
	mf_retm_if(title_label == NULL, "title_label is NULL");
	GString *title = NULL;

	title = g_string_new(title_label);
	if (title != NULL) {
		SAFE_FREE_CHAR(*naviframe_title);
		*naviframe_title = g_strdup(title->str);
		g_string_free(title, TRUE);
		title = NULL;
	}
}
Evas_Object *mf_navi_bar_content_create(void *data)
{
	MF_TRACE_BEGIN;
	t_start;
	mf_retvm_if(data == NULL, NULL, "data is NULL");

	struct appdata *ap = (struct appdata *)data;
	mf_retvm_if(ap->mf_Status.path == NULL, NULL, "ap->mf_Status.path is NULL");
	mf_retvm_if(ap->mf_Status.path->str == NULL, NULL, "ap->mf_Status.path->str is NULL");

	Evas_Object *pContent = NULL;
	Eina_List *file_list = NULL;
	Eina_List *dir_list = NULL;
	int dir_list_len = 0;
	int file_list_len = 0;
	int error_code = 0;

	if (ap->mf_MainWindow.pButton) {
		elm_object_part_content_set(ap->mf_MainWindow.pNaviLayout, "search_icon", ap->mf_MainWindow.pButton);
		evas_object_show(ap->mf_MainWindow.pButton);
	}

	if (ap->mf_Status.view_type == mf_view_root) {
		if (ap->mf_FileOperation.folder_list) {
			mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.folder_list), MYFILE_TYPE_FSNODE);
		}
		if (ap->mf_FileOperation.file_list) {
			mf_util_free_eina_list_with_data(&(ap->mf_FileOperation.file_list), MYFILE_TYPE_FSNODE);
		}

		mf_util_generate_root_view_file_list(ap, &(ap->mf_FileOperation.folder_list), ap->mf_Status.iStorageState);
		ap->mf_Status.flagNoContent = EINA_FALSE;
		mf_debug("count is [%d]", eina_list_count(ap->mf_FileOperation.folder_list));
	} else if (ap->mf_Status.view_type == mf_view_root_category) {
		pContent = mf_category_get_from_media_db(ap, ap->mf_Status.category_type, false);
		return pContent;
	} else {
		error_code = mf_util_generate_file_list(ap);
		if (error_code != MYFILE_ERR_NONE) {
			/*Todo: we need to free all the Eina_List*/
			MF_TRACE_END;
			return NULL;
		}

		/*	sort the list by sort_type*/
		mf_util_sort_the_file_list(ap);

		file_list = ap->mf_FileOperation.file_list;
		dir_list = ap->mf_FileOperation.folder_list;

		dir_list_len = eina_list_count(dir_list);
		file_list_len = eina_list_count(file_list);

		if ((dir_list_len + file_list_len) > 0) {
			ap->mf_Status.flagNoContent = EINA_FALSE;
		} else {
			ap->mf_Status.flagNoContent = EINA_TRUE;
		}
	}
	ap->mf_MainWindow.pNaviGenlist = NULL;
	ap->mf_MainWindow.pNaviGengrid = NULL;

	if (ap->mf_Status.flagNoContent) {
		pContent = mf_object_create_no_content(ap->mf_MainWindow.pNaviBar);
		mf_object_text_set(pContent, MF_LABEL_NO_FILES, "elm.text");
	} else {
		int view_style = mf_view_style_get(ap);
		if (view_style == MF_VIEW_STYLE_THUMBNAIL) {
			pContent = mf_gengrid_create_list(ap, ap->mf_MainWindow.pNaviBar);
			ap->mf_MainWindow.pNaviGengrid = pContent;
		} else {
			pContent = mf_genlist_create_list(ap, ap->mf_MainWindow.pNaviBar);
			ap->mf_MainWindow.pNaviGenlist = pContent;
		}
	}
	if (ap->mf_Status.more == MORE_DEFAULT
	    || ap->mf_Status.more == MORE_INTERNAL_COPY
	    || ap->mf_Status.more == MORE_INTERNAL_MOVE) {
		mf_fs_monitor_add_dir_watch(ap->mf_Status.path->str, ap);
	}
	t_end;

	MF_TRACE_END;
	return pContent;
}

Evas_Object *mf_naviframe_left_cancel_button_create(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
	Evas_Smart_Cb pFunc, void *pUserData)
{
	Evas_Object *btn = elm_button_add(pParent);
	elm_object_style_set(btn, "naviframe/title_left");
	mf_object_text_set(btn, MF_LABEL_CANCEL, NULL);
	evas_object_smart_callback_add(btn, "clicked", pFunc, pUserData);
	elm_object_item_part_content_set(pNaviItem, "title_left_btn", btn);

	evas_object_show(btn);

	return btn;
}

Evas_Object *mf_naviframe_right_save_button_create(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
	Evas_Smart_Cb pFunc, void *pUserData)
{
	struct appdata *ap = (struct appdata *)pUserData;
	Evas_Object *btn = elm_button_add(pParent);
	elm_object_style_set(btn, "naviframe/title_right");
	if (ap->mf_Status.more == MORE_EDIT_DELETE) {
		mf_object_text_set(btn, MF_LABEL_DELETE, NULL);
	} else {
		mf_object_text_set(btn, MF_LABEL_DONE, NULL);
	}
	evas_object_smart_callback_add(btn, "clicked", pFunc, pUserData);
	elm_object_item_part_content_set(pNaviItem, "title_right_btn", btn);

	evas_object_show(btn);

	return btn;
}

void mf_naviframe_title_button_delete(Elm_Object_Item *navi_it)
{
	Evas_Object *btn = NULL;
	btn = elm_object_item_part_content_get(navi_it, "title_left_btn");
	SAFE_FREE_OBJ(btn);
	btn = elm_object_item_part_content_get(navi_it, "title_right_btn");
	SAFE_FREE_OBJ(btn);
}
