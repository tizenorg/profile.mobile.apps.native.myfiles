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

#ifndef __DEF_MYFILE_UTIL_H_
#define __DEF_MYFILE_UTIL_H_

#include <sys/time.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <app.h>
#include <Elementary.h>
#include <package_manager.h>
#include "mf-fo-common.h"
#include "mf-fs-util.h"
#include "mf-media-types.h"
#include "mf-object-conf.h"

#define MF_ICON_SOFT_SEARCH_BACK "myfile_icon_soft_search_back.png"
#define MF_ICON_SOFT_SEARCH_CANCEL			"T01_2_button_expand_cancel.png"
#define MF_ICON_SOFT_BACK			"myfile_icon_soft_back.png"
#define MF_SCALE_FACTORY    elm_config_scale_get()
#define SAFE_FREE_CHAR(x) do {\
					if ((x) != NULL) {\
						free(x); \
						x = NULL;\
					} \
			     } while (0)

#define SAFE_FREE_GSTRING(x) do {\
					if ((x) != NULL) {\
						g_string_free(x, TRUE); \
						x = NULL;\
					} \
				} while (0)

#define CHAR_CHECK_NULL_GOTO(arg, dest) do {\
						if ((arg) == NULL) {\
							goto dest;\
						} \
					   } while (0)

#define GSTRING_CHECK_NULL_GOTO(arg, dest) do {\
						if ((arg) == NULL || (arg->str) == NULL) {\
							goto dest;\
						} \
					   } while (0)

#define SAFE_TIMER_DEL(x) do {\
				if ((x) != NULL) {\
					ecore_timer_del(x); \
					x = NULL;\
				} \
			  } while (0)

#define SAFE_DEL_NAVI_ITEM(x) do {\
					if ((*x) != NULL) {\
						elm_object_item_del(*x); \
						(*x) = NULL;\
					} \
				  } while (0)

#define SAFE_DEL_ECORE_TIMER(timer) do { \
						if (timer) { \
							ecore_timer_del(timer);\
							timer = NULL; \
						} \
					} while (0)

#define SAFE_FREE_OBJ(x) do {\
					if ((x) != NULL) {\
						evas_object_del(x); \
						x = NULL;\
					} \
			     } while (0)

#define SAFE_STRCPY(dest, src) \
			do { if (!dest||!src)break;\
				strncpy (dest , src, sizeof(dest)-1);\
				dest[sizeof(dest)-1] = 0;	}while(0)

#define mf_ecore_idler_del(idler) do { \
				if (idler) { \
					ecore_idler_del(idler);\
					idler = NULL; \
				} \
			} while (0)

#define SAFE_FREE_ECORE_EVENT(event) do { \
						if (event) { \
							ecore_event_handler_del(event);\
							event = NULL; \
						} \
					} while (0)

#define mf_genlist_item_class_free(x)	 do { \
						if (x) { \
							elm_genlist_item_class_free(x);\
							x = NULL; \
						} \
					} while (0)
typedef enum _MF_INTERNAL_NAME_ERR MF_INTERNAL_NAME_ERR;
enum _MF_INTERNAL_NAME_ERR {
	MF_INTERNAL_FILE_NAME_NULL = 1,
	MF_INTERNAL_FILE_NAME_EMPTY,
	MF_INTERNAL_FILE_NAME_IGNORE,
	MF_INTERNAL_FILE_NAME_CHUG,
	MF_INTERNAL_FILE_NAME_MAX_LENGTH,
	MF_INTERNAL_FILE_NAME_INVALID_CHAR,
};

typedef enum __mf_thumbnail_type mf_thumbnail_type;
enum __mf_thumbnail_type {
	MF_THUMBNAIL_TYPE_DEFAULT,
	MF_THUMBNAIL_TYPE_THUMBNAIL,
	MF_THUMBNAIL_TYPE_MAX
};

#define MYFILE_MAGIC_MAIN_CONTEXT           (0x1983cdaf)
#define MYFILE_MAGIC_DETAIL_LIST_ITEM       (0x1977abcd)
#define MYFILE_MAGIC_PIPE_DATA				 (0x0716ffcc)

#define MYFILE_FINISH_MMC_INIT_DATA      "db/Apps/FileManager/FinishMmcInitData"

#define MYFILE_MAGIC                 unsigned int  __magic
#define MYFILE_MAGIC_SET(d, m)       (d)->__magic = (m)
#define MYFILE_MAGIC_CHECK(d, m)     ((d) && ((d)->__magic == (m)))

#define MYFILE_DBUS_SIGNAL_INTERFACE	"app.fexplorer.dbus.Signal"
#define MYFILE_DBUS_SIGNAL_PATH		"/app/fexplorer/dbus"
#define MAX_LEN_VIB_DURATION 		0.5

typedef enum _MORE_TYPE MORE_TYPE;
enum _MORE_TYPE {			/* softkey / contextual popup */
	MORE_DEFAULT = 0,
	MORE_EDIT,		// 1
	MORE_SHARE_EDIT,	// 2
	MORE_DELETE,		// 3
	MORE_IDLE_DELETE,	// 4

	MORE_DATA_COPYING,	// 5
	MORE_DATA_MOVING,	// 6
	MORE_DATA_DECOMPRESSING,	// 7 apply the new UI feature of decompress
	MORE_COMPRESS,		// 8
	MORE_DECOMPRESS,	// 9

	MORE_INTERNAL_COPY_MOVE,// 10
	MORE_INTERNAL_COPY,	// 11
	MORE_INTERNAL_MOVE,	// 12
	MORE_THUMBNAIL_RENAME,	// 13
	MORE_RENAME,		// 14

	MORE_SEARCH,		// 15
	MORE_DECOMPRESS_HERE,	// 16
	MORE_INTERNAL_DECOMPRESS,// 17 apply the new UI feature of decompress
	MORE_SETTING,
	MORE_ADVANCED_SEARCH,

	MORE_ADVANCED_SEARCH_RESULT,
	MORE_EDIT_COPY, 	
	MORE_EDIT_MOVE, 	
	MORE_EDIT_RENAME,		
	MORE_EDIT_ADD_SHORTCUT,

	MORE_EDIT_DELETE_SHORTCUT, 	
	MORE_EDIT_DELETE_RECENT,
	MORE_EDIT_UNINSTALL,	
	MORE_EDIT_DELETE,
	MORE_EDIT_DETAIL,

	MORE_DOING_UNINSTALL,
	MORE_TYPE_MAX
};

typedef enum __mf_view_type_e mf_view_type_e;
enum __mf_view_type_e {
	mf_view_none = 0,
	mf_view_root,
	mf_view_root_category,
	mf_view_storage,
	mf_view_recent,
	mf_view_normal,
	mf_view_detail,
};

typedef enum _LAUNCH_TYPE LAUNCH_TYPE;
enum _LAUNCH_TYPE {
	LAUNCH_TYPE_FORK = 0,
	LAUNCH_TYPE_FAKE,
	LAUNCH_TYPE_FAIL,
	LAUNCH_TYPE_DIR,
	LAUNCH_TYPE_UNSUPPORT,
	LAUNCH_TYPE_MAX
};

typedef enum __mf_list_type mf_list_type;
enum __mf_list_type {
	mf_list_recent_files = 0,
	mf_list_shortcut,
	mf_list_normal,
};

typedef enum _REPORT_TYPE REPORT_TYPE;
enum _REPORT_TYPE {
	MYFILE_REPORT_NONE = 1,
	MYFILE_REPORT_RECURSION_DETECT,
	MYFILE_REPORT_BOTH_ARE_SAME_FILE,
	MYFILE_REPORT_MAX,
};

typedef enum _MYFILE_FILE_NAME_TYPE MYFILE_FILE_NAME_TYPE;
enum _MYFILE_FILE_NAME_TYPE {
	FILE_NAME_WITH_BRACKETS,
	FILE_NAME_WITH_UNDERLINE,
	FILE_NAME_NONE,
};

typedef enum _MYFILE_CONTENT_TYPE MYFILE_CONTENT_TYPE;
enum _MYFILE_CONTENT_TYPE {
	MYFILE_TYPE_MIN,
	MYFILE_TYPE_GSTRING,
	MYFILE_TYPE_CHAR,
	MYFILE_TYPE_FSNODE,
	MYFILE_TYPE_NAVILIST,
	MYFILE_TYPE_ITEM_DATA,
	MYFILE_TYPE_MAX
};

typedef enum __mf_network_type_e mf_network_type_e;
enum __mf_network_type_e {
	mf_network_none = 0,
	mf_network_wifi = 0x01,
	mf_network_cellular = 0x02
};

typedef enum 
{
    PREF_TYPE_SORT_TYPE = 0,
    PREF_TYPE_TIME_FORMAT,
    PREF_TYPE_MASS_STORAGE,
    PREF_TYPE_VIEW_STYLE,
    PREF_TYPE_EXTENSION_STATE,
    PREF_TYPE_RECENT_FILES,
    PREF_TYPE_HIDEN_STATE,
    PREF_TYPE_MAX,
} MYFILE_PREF_TYPE;

bool mf_util_check_forbidden_operation(void *data);
int mf_util_check_disk_space(void *data);
void mf_util_refresh_screen(void *data);
int mf_util_is_mmc_on(int *mmc_card);
int mf_util_get_eina_list_len(const Eina_List *list);
void mf_util_free_eina_list_with_data(Eina_List **list, MYFILE_CONTENT_TYPE type);
void mf_util_operation_alloc_failed(void *data);
void mf_util_action_storage_insert(void *data, char *pItemLabel);
int mf_util_get_pref_value(MYFILE_PREF_TYPE type, int *value);
void mf_util_set_sort_type(int value);
void mf_util_merge_eina_list_to_glist(const Eina_List *eSource, GList **gSource);
void mf_util_exception_func(void *data);
int mf_util_is_valid_name_check(const char *name);
void mf_util_ex_disk_list_update(void *data);
long mf_util_character_count_get(const char *original);
/**icu related**/
char *mf_util_get_icu_date(fsNodeInfo *pNode);
int mf_util_icu_init(void *data);
void mf_util_icu_finalize(void *data);
char *mf_util_icu_translate(void *data, i18n_udate date, bool is_init_checking);
void mf_util_set_pm_lock(void *data, Eina_Bool isLock);
gboolean mf_util_is_file_selected(Eina_List **source, GString *path);
void mf_util_sort_the_file_list(void *data);
int mf_util_generate_file_list(void *data);
char *mf_util_get_text(const char *ID);
void mf_util_set_view_style(int value);
int mf_util_generate_root_view_file_list(void *data, Eina_List **list, int storage_state);
void mf_util_set_extension_state(int value);
void mf_util_set_hiden_state(int value);

const char *mf_util_search_markup_keyword(const char *string, char *searchword, bool *result);
void mf_util_set_recent_file(char *path);
void mf_util_remove_item_from_list_by_location(Eina_List **list, int location);
void mf_util_generate_list(Eina_List **list, const char *path, int file_type, int list_type);
bool mf_util_is_rotation_lock(void);
void mf_util_remove_item_from_list_by_name(Eina_List **list, const char *path);
fsNodeInfo *mf_util_generate_pnode(const char *path, int file_type);
void mf_util_update_item_from_list_by_name(Eina_List **list, const char *path, char *new_name);
int mf_util_rotate_state_get(Evas_Object *win);
int mf_util_get_rotate_state_by_angle(int angle);
void mf_util_generate_saved_files_list(void *data, int type);
void mf_util_free_data(void **data, int type);

char *mf_util_get_shortcut();
void mf_util_add_shortcut(char *path);
void mf_util_item_remove_invalid_category_items(Eina_List **list);
void mf_util_db_add_recent_files(MFDHandle *handle, const char *path, const char *name, int storage, const char *thumbnail);
void mf_util_db_get_recent_files(MFDHandle *handle, void *data);
void mf_util_db_remove_recent_files(MFDHandle *handle, char *recent_file);
void mf_util_rotation_flag_set(bool rotation);
void mf_util_rotation_flag_get(int *rotation);

void mf_util_check_pnode_list_items_exists(Eina_List **list);
void mf_util_db_remove_shortcut(MFDHandle *handle, char *shortcut);
bool mf_util_db_find_shortcut(MFDHandle *handle, const char *path, const char *name, int storage);
bool mf_util_db_find_shortcut_display_name(MFDHandle *handle, const char *name);
void mf_util_db_add_shortcut(MFDHandle *handle, const char *path, const char *name, int storage);
void mf_util_db_remove_recent_files(MFDHandle *handle, char *recent_file);
void mf_util_db_add_recent_files(MFDHandle *handle, const char *path, const char *name, int storage, const char *thumbnail);
void mf_util_generate_list_prepend(Eina_List **list, const char *path, int file_type, int list_type);
void mf_util_normal_item_data_free(mfItemData_s **item_data);
bool mf_util_is_low_memory(const char *path, unsigned long long size);
gboolean mf_util_NFD_strstr(const char *nor_str, const char *nor_needle);
void mf_util_first_item_push(void *data);
void mf_util_path_stack_free();
char *mf_util_first_item_get(char *path);
int mf_util_get_storage_id(void);

#endif /* __DEF_MYFILE_UTIL_H_ */
