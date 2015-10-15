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

#ifndef __DEF_MYFILE_H_
#define __DEF_MYFILE_H_

#include <Ethumb.h>
#include <glib.h>
#include <Ecore.h>
#include <Evas.h>
#include <stdbool.h>
#include <media_content.h>
#include <Elementary.h>
//#include <media-svc-types.h>
#include <efl_extension.h>

/* for SG */
#include <app_control.h>
#include <app.h>

/* for dlog */
#include "mf-dlog.h"
#include "mf-util.h"
#include "mf-search.h"
#include "mf-copy.h"
#include "mf-popup.h"

#include "mf-media-types.h"
#include "mf-ug-detail.h"
#include <utils_i18n.h>

#define CTRL_DISABLE_NONE					0x0000
#define CTRL_DISABLE_MOVE					0x0001
#define CTRL_DISABLE_COPY					0x0002
#define CTRL_DISABLE_DELETE					0x0004
#define CTRL_DISABLE_SEND					0x0008
#define CTRL_DISABLE_LIST_BY					0x0010
#define CTRL_DISABLE_CREATE					0x0020
#define CTRL_DISABLE_EDIT					0x0040
#define CTRL_DISABLE_COPY_MOVE					0x0080
#define CTRL_DISABLE_SEARCH					0x0100
#define CTRL_DISABLE_MOVE_HERE				0x0200
#define CTRL_DISABLE_COPY_HERE				0x0400
#define CTRL_DISABLE_CANCEL				0x0800
#define CTRL_DISABLE_MORE				0x1000
#define CTRL_DISABLE_REFRESH				0x2000
#define CTRL_DISABLE_DOWNLOAD				0x3000
#define CTRL_DISABLE_DETAIL				0x4000


#define CTRL_DISABLE_EDIT_ALL				(CTRL_DISABLE_DELETE | CTRL_DISABLE_SEND | CTRL_DISABLE_MORE | CTRL_DISABLE_MOVE | CTRL_DISABLE_COPY)	//fix P131205-00761 by ray
#define CTRL_DISABLE_DEFAULT_SEL			(CTRL_DISABLE_COPY)
#define CTRL_DISABLE_NOCONTENT_VIEW			(CTRL_DISABLE_EDIT | CTRL_DISABLE_SEND)
#define CTRL_DISABLE_USER_FOLDER_SEL			(CTRL_DISABLE_DELETE | CTRL_DISABLE_MORE)
#define CTRL_DISABLE_DEFAULT_ALL			(CTRL_DISABLE_SEND | CTRL_DISABLE_MORE | CTRL_DISABLE_EDIT | CTRL_DISABLE_SEARCH)
#define CTRL_DISABLE_SYSFOLDER_SELECT			(CTRL_DISABLE_MOVE_HERE)
#define CTRL_ROOT_VIEW					(CTRL_DISABLE_SEARCH|CTRL_DISABLE_MORE)

#define myfile_ret_if(expr) do { \
	if (expr) { \
		mf_debug("!!! CHECK ERROR !!! (%s) -> %s() return!!!", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)
#define myfile_retv_if(expr, val) do { \
	if (expr) { \
		mf_debug("!!! CHECK ERROR !!! (%s) -> %s() return !!!", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)
#define myfile_retm_if(expr, fmt, arg...) do { \
	if (expr) { \
		mf_debug(fmt, ##arg); \
		mf_debug("!!! CHECK ERROR !!! (%s) -> %s() return !!!", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)
#define myfile_retvm_if(expr, val, fmt, arg...) do { \
	if (expr) { \
		mf_debug(fmt, ##arg); \
		mf_debug("!!! CHECK ERROR !!! (%s) -> %s() return !!!", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

/***********	Global Definitions		***********/
typedef void (*mfCallBack) (void *, Evas_Object *, void *);

typedef struct _oper_record oper_record;

struct _oper_record {
	char *path;
	int location;
	int more;
	int view_type;
};

typedef struct _myfileMainWindow myfileMainWindow;
struct _myfileMainWindow {
	/* root window size */
	int root_w;
	int root_h;
	int root_x;
	int root_y;

	/* graphic data */
	Evas_Object *pWindow;
	Evas_Object *pBackGround;
	Evas_Object *pConformant;
	Evas_Object *pMainLayout;
	Evas_Object *pTabBar;

	Evas_Object *pNormalPopup;
	Evas_Object *pDeleteConfirmPopup;
	Evas_Object *pContextPopup;
	Evas_Object *pEntry;
	Evas_Object *pBox;
	Ecore_Event_Handler *event;
	Ecore_Event_Handler *font_event;
	Evas_Object *pProgressPopup;
	Evas_Object *pFinishPopup;
	Evas_Object *pMmcRemovedPopup;
	Evas_Object *pProgressLayout;
	Evas_Object *pOperationNotify;
	Evas_Object *pSearchEntry;
	Evas_Object *pSearchBar;
	Evas_Object *pSearchCategoryBtn;
	Evas_Object *pNewFolderPopup;
	Evas_Object *pButton;
	Evas_Object *pPopupBox;

	Evas_Object *pNaviBar;
	Evas_Object *pNaviLayout;
	Evas_Object *pNaviBox;
	Evas_Object *pNaviCtrlBar;
	Evas_Object *pNaviSearchBar;
	Evas_Object *pNaviGenlist;
	Evas_Object *pNaviGengrid;
	Evas_Object *pSearchLayout;
	Evas_Object *pSearchEraser;
	Evas_Object *pLongpressPopup;

	Ecore_Timer *pPopupTimer;

	Elm_Object_Item *pNaviItem;

	MFDHandle *mfd_handle;
	char *naviframe_title;
	int location;
	oper_record record;

};

enum _MF_TAB_FLAG {
	MF_TAB_INIT = 0,
	MF_TAB_LEFT,
	MF_TAB_RIGHT
};



typedef struct _myfileStatus myfileStatus;

struct _myfileStatus {

	int more;		/** current mode **/
	int pre_create_more;		/** the mode before operation **/
	int rotation_type;	/** current rotation type **/
	int folder_count;	/** current new created folders count **/
	int iRadioValue;	/** current the radio box selected item value **/
	int flagViewType;	/** current view type, List view or Thumbnail view **/
	int iStorageState;	/** current storage state **/
	int iSelectedSortType;
	int iExtensionState;
	int view_type;
	int preViewType;
	int search_category;
	int category_type;
	int flag_tab;
	int tab_mode;
	int count;
	int extra;
	int share;
	int check;
	mf_request_type req;

	bool is_from_shortcut;
	char* shortcut_from_path;
	bool ToTop;

	Eina_Bool flagNoContent;
	Eina_Bool flagIcuInit;
	Eina_Bool flagLCDLock;		/*lcd lock status*/
	Eina_Bool flagSearchStart;

	Eina_Bool flagViewAsRefreshView;

#ifdef MF_SEARCH_UPDATE_COUNT
	Eina_Bool flagUpdateSearch;
#endif
	Eina_Bool flagIME;
	char *entry_path;
	int entry_more;
	Evas_Object *pRadioGroup;
	char *launch_path;
	int is_DBcontent;
	GString *path;		/* current path */
	char *search_filter;
	Elm_Object_Item *pPreNaviItem;
	/* icu related */
	i18n_udatepg_h generator;
	i18n_udate_format_h formatter;
	char *categorytitle;
	char *EnterFrom;
	mf_search_handle search_handler;
	Ecore_Idler *app_init_idler;
	Ecore_Idler *operation_refresh_idler;
	Ecore_Idler *search_idler;
	Ecore_Idler *float_button_idler;
	Ecore_Timer *rename_timer;

	bool b_run_background;
	mfDetailData *detail;
};


typedef struct _myfileFileOperation myfileFileOperation;
struct _myfileFileOperation {
	/* to rename */
	GString *to_rename;
	char *file_name_suffix;

	/* pipe for inotify */
	Ecore_Pipe *sync_pipe;

	/*      refresh type */
	GString *source;
	GString *destination;
	gboolean refresh_type;


	/*  progress bar data record    */
	Evas_Object *progress_bar;
	unsigned int current_count;
	unsigned long total_file_size;
	unsigned long finished_size;
	int progress_cancel;

	Eina_List *search_result_folder_list;
	Eina_List *search_result_file_list;
	Eina_List *folder_list;
	Eina_List *file_list;
	Eina_List *category_list;
	Eina_List *recent_list;
	Eina_List *shortcut_list;

	mf_cancel *pCancel;
	mf_fo_request *pRequest;
	mf_fo_msg *pMessage;
	GList *pSourceList;

	int iTotalCount;
	int iRequestType;
	Ecore_Timer *search_IME_hide_timer;
	Elm_Object_Item *rename_item;

	/**job handler**/
	bool iOperationSuccessFlag;	/*used for confirm whether the operation fininsh successfully, not canceled and failed*/
	char *pOperationMsg;
	message_type_e message_type;
	Elm_Object_Item *idle_delete_item;
};

typedef struct _myfileFileRecord myfileFileRecord;

struct _myfileFileRecord {
	/* value saver for bluetooth / protection */
	Eina_List *value_saver;
	Eina_List *selected_files;
};

typedef struct _myfileSharedGadget myfileSharedGadget;
struct _myfileSharedGadget {
	app_control_h ug;
	int location;
	/*SGController  *sg_controller;for privacy lock*/
};

struct _myfileBundle {	
    app_control_h recv_service;
	char *path;
	char *select_type;
	char *file_type;
	char *marked_mode;
};

typedef struct _myfileBundle myfileBundle;

typedef struct __mf_genlist_item_class_s mf_genlist_item_class_s;
struct __mf_genlist_item_class_s {
	Elm_Genlist_Item_Class *listby_itc;
	Elm_Genlist_Item_Class *popup_itc;
	Elm_Genlist_Item_Class *categoryitc;
	Elm_Genlist_Item_Class *left_itc;
	Elm_Genlist_Item_Class *search_itc;
	Elm_Genlist_Item_Class *itc;
	Elm_Genlist_Item_Class *userfolderitc;
	Elm_Genlist_Item_Class *order_itc;
};


struct appdata {
	myfileMainWindow mf_MainWindow;
	myfileStatus mf_Status;
	myfileFileOperation mf_FileOperation;
	myfileFileRecord mf_FileRecordList;
	myfileSharedGadget mf_SharedGadget;
	myfileBundle mf_Bundle;
	mf_genlist_item_class_s mf_gl_style;
	Eina_List *storage_list;
	Eina_List *file_list;
	char *nTemp_entry;
	Evas_Object *label;
	char *file_name;
};


/*handler list node struct*/
typedef struct _myfileEcoreHandleNode	myfileEcoreHandleNode;
struct _myfileEcoreHandleNode {
	Ecore_Job *handler;
	char *navi_label;
};


typedef enum _MF_ACTION mfAction;

enum _MF_ACTION {
	MFACTION_CLICK,
	MFACTION_FLICK
};


#define _EDJ(o) elm_layout_edje_get(o)
Evas_Object *mf_main_load_edj(Evas_Object * parent, const char *file, const char *group);
struct appdata * mf_get_appdata();

Eina_Bool mf_main_is_split_on();
bool mf_main_is_sync_on();

#endif /* __DEF_MYFILE_H_ */
