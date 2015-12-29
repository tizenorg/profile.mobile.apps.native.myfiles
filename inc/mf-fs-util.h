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

#ifndef _MYFILE_FILESYSTEM_UTIL_H_
#define _MYFILE_FILESYSTEM_UTIL_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <utils_i18n.h>
#include <Eina.h>
#include <Elementary.h>
#include <storage.h>
#include "media_content.h"
#include "mf-dlog.h"
#include "mf-error.h"

int mf_file_attr_get_parent_path(const char *path, char **parent_path);

static inline char *Get_Root_Path(int storage_id)
{
	char *path = NULL;
	storage_get_root_directory (storage_id,&path) ;
	return path;
}

static inline char *Get_Parent_Path(int storage_id)
{
	char *path = NULL;
	char *storage_path = NULL;
	storage_get_root_directory(storage_id, &path);
	if (path) {
		mf_file_attr_get_parent_path(path, &storage_path);
		free(path);
	}
	return storage_path;
}

#define DEBUG_FOLDER    "SLP_debug"

/*	File system related value definition	*/
#define MYFILE_FILE_NAME_CHAR_COUNT_MAX 50
#define MYFILE_FILE_NAME_LEN_MAX        255     /* use g_utf8_strlen to check length*/
#define MYFILE_FILE_PATH_LEN_MAX        4096	/* use g_utf8_strlen to check length*/
#define MYFILE_REPORT_SIZE		        16384

#define MYFILE_FOLDER_SIZE			0;

/*	File system related String definition	*/
#define PHONE_FOLDER    Get_Root_Path(STORAGE_TYPE_INTERNAL)
#define MEMORY_FOLDER   Get_Root_Path(STORAGE_TYPE_EXTERNAL)
#define PHONE_PARENT    Get_Parent_Path(STORAGE_TYPE_INTERNAL)
#define STORAGE_PARENT 	Get_Parent_Path(STORAGE_TYPE_EXTERNAL)
#define PHONE_NAME	    "content"
#define MMC_NAME	    "sdcard"

#define MYFILE_NAME_PATTERN	"[\\<>:;*\"|?/]"

/*	File system define default folder	*/
#define DEFAULT_FOLDER_CAMERA_SHOTS "Camera shots"
#define DEFAULT_FOLDER_DOWNLOADS    "Downloads"
#define DEFAULT_FOLDER_IMAGE        "Images"
#define DEFAULT_FOLDER_VIDEO        "Videos"
#define DEFAULT_FOLDER_MUSIC        "Music"
#define SUB_FODER_WALLPAPER	        "Wallpapers"
#define DEFAULT_FOLDER_BOOKMARK	    "Bookmark"
#define DEFAULT_FOLDER_RSS	        "RSS"
#define DEFAULT_FOLDER_ALERTS_AND_RINGTONES	"Alerts and ringtones"

#define SUB_FODER_FM		    "FM radio"
#define SUB_FODER_ALERTS		"Alerts"
#define SUB_FODER_RINGTONES		"Ringtones"
#define SUB_FODER_VOICE_RECORD  "Voice recorder"


#ifndef ICON_PATH
    #define ICON_PATH "/usr/apps/org.tizen.myfile/res/images"
#endif

#ifndef DEFAULT_ICON
#define DEFAULT_ICON "myfile_icon_etc.png"
#endif

/***************     Title Icon      ******************/
#define MF_TITLE_ICON_HOME          "myfile_controlbar_cion_home.png"
#define MF_TITLE_ICON_UPPER         "myfile_controlbar_cion_up_folder.png"
#define MF_TITLE_ICON_SEARCH        "myfile_title_icon_search.png"
#define MF_TITLE_ICON_SELECT_ALL    "myfile_icon_select_all.png"

/***************    Default Icon     ***********************/
#define MF_ICON_FOLDER		"myfile_icon_folder.png"
#define MF_ICON_IMAGE		"myfile_icon_images.png"
#define MF_ICON_VIDEO		"myfile_icon_video.png"
#define MF_ICON_MUSIC		"myfile_icon_music.png"
#define MF_ICON_SOUND		"myfile_icon_amr.png"
#define MF_ICON_PDF			"myfile_icon_pdf.png"
#define MF_ICON_DOC			"myfile_icon_word.png"
#define MF_ICON_PPT			"myfile_icon_ppt.png"
#define MF_ICON_EXCEL		"myfile_icon_excel.png"
//#define MF_ICON_VOICE		"myfile_icon_amr.png"
#define MF_ICON_HTML		"myfile_icon_html.png"
#define MF_ICON_FLASH		"myfile_icon_swf.png"
#define MF_ICON_TXT			"myfile_icon_text.png"
#define MF_ICON_VCONTACT	"myfile_icon_vcard.png"
#define MF_ICON_VCALENDAR	"myfile_icon_vcalender.png"
#define MF_ICON_VNOTE		"myfile_icon_vText.png"
#define MF_ICON_RSS			"myfile_icon_rss.png"
#define MF_ICON_JAVA		"myfile_icon_java.png"
#define MF_ICON_MUSIC_PAUSE	"myfile_icon_control_pause.png"
#define MF_ICON_VIDEO_PLAY	"myfile_icon_video_play.png"
#define MF_ICON_TPK			"myfile_icon_tpk.png"
#define MF_ICON_SNB			"myfile_icon_snb.png"
#define MF_ICON_HWP			"myfile_icon_hwp.png"
#define MF_ICON_GUL			"myfile_icon_etc.png"
#define MF_ICON_EMAIL		"myfile_icon_email.png"
#define MF_ICON_TASK		"myfile_icon_task.png"
#define MF_ICON_ZIP			"myfile_icon_zip.png"
#define MF_ICON_SVG			"myfile_icon_svg.png"

#define MF_ICON_ITEM_MMC	"myfile_icon_folder_sdcard.png"

#define MF_ICON_MUSIC_THUMBNAIL "myfile_icon_music.png"

/******* Root Folder Icon *******/
#define MF_ICON_ITEM_ROOT_PHONE		"myfile_icon_root_folder_device_memory.png"
#define MF_ICON_ITEM_ROOT_MMC		"my_files_sd_card.png"

/************ Search Category Icon ***********/
#define IMG_ICON_SEARCH_CATEGORY_ALL    "myfile_search_category_all.png"
#define IMG_ICON_SEARCH_CATEGORY_IMG    "myfile_search_category_img.png"
#define IMG_ICON_SEARCH_CATEGORY_SND    "myfile_search_category_snd.png"
#define IMG_ICON_SEARCH_CATEGORY_VIDEO  "myfile_search_category_video.png"
#define IMG_ICON_SEARCH_CATEGORY_DOC    "myfile_search_category_doc.png"

/******* No Contents icon **********************/
#define IMG_ICON_IMG_NO_CONTENTS	"00_nocontents_picture.png"
#define IMG_ICON_MULTI_NO_CONTENTS	"00_nocontents_multimedia.png"
#define IMG_ICON_TEXT_NO_CONTENTS	"00_nocontents_text.png"
#define IMG_ICON_DEV_NO_CONTENTS	"00_nocontents_devices.png"

typedef enum _SORT_OPTION fsSortOption;
enum _SORT_OPTION {
	MYFILE_SORT_BY_NONE = 0,    /**< Sort by default */
	MYFILE_SORT_BY_NAME_A2Z,    /**< Sort by file name ascending */
	MYFILE_SORT_BY_SIZE_S2L,    /**< Sort by file size ascending */
	MYFILE_SORT_BY_DATE_O2R,    /**< Sort by file date ascending */
	MYFILE_SORT_BY_TYPE_A2Z,    /**< Sort by file type ascending */
	MYFILE_SORT_BY_NAME_Z2A,    /**< Sort by file name descending */
	MYFILE_SORT_BY_SIZE_L2S,    /**< Sort by file size descending */
	MYFILE_SORT_BY_DATE_R2O,    /**< Sort by file date descending */
	MYFILE_SORT_BY_TYPE_Z2A,    /**< Sort by file type descending */
	MYFILE_SORT_BY_MAX
};

typedef enum _mf_extension_state_e mf_extension_state_e;
enum _mf_extension_state_e {
	MF_EXTENSION_NONE = 0,
	MF_EXTENSION_SHOW,
	MF_EXTENSION_HIDE,
	MF_EXTENSION_MAX
};

typedef enum _mf_hiden_state_e mf_hiden_state_e;
enum _mf_hiden_state_e {
	MF_HIDEN_NONE = 0,
	MF_HIDEN_SHOW,
	MF_HIDEN_HIDE,
	MF_HIDEN_MAX
};

typedef enum __mf_split_state_e mf_split_state_e;
enum __mf_split_state_e {
	mf_split_off = 0,
	mf_split_on
};

typedef enum __mf_view_content_type_e mf_view_content_type_e;
enum __mf_view_content_type_e {
	mf_view_content_type_navi = 0,
	mf_view_content_type_panes
};

typedef enum _FILE_TYPE fsFileType;
enum _FILE_TYPE {
	FILE_TYPE_NONE = 0,
	FILE_TYPE_DIR,		    /**< Folder category */
	FILE_TYPE_FILE,		    /**< File category */
	FILE_TYPE_IMAGE,	   /**< Image category */
	FILE_TYPE_VIDEO,	   /**< Video category */
	FILE_TYPE_MUSIC,	   /**< Music category */
	FILE_TYPE_SOUND,	   /**< Sound category */
	FILE_TYPE_PDF,		   /**< Pdf category */
	FILE_TYPE_DOC,		   /**< Word category */
	FILE_TYPE_PPT,		   /**< Powerpoint category */
	FILE_TYPE_EXCEL,	   /**< Excel category */
	FILE_TYPE_VOICE,	   /**< Voice category */
	FILE_TYPE_HTML,		   /**< Html category */
	FILE_TYPE_FLASH,	   /**< Flash category */
	FILE_TYPE_GAME,		   /**< Game category */
	FILE_TYPE_APP,		   /**< Application category */
	FILE_TYPE_THEME,	   /**< Theme category */
	FILE_TYPE_TXT,		   /**< Txt category */
	FILE_TYPE_VCONTACT,	   /**< Vcontact category */
	FILE_TYPE_VCALENDAR,   /**< Vcalendar category */
	FILE_TYPE_VNOTE,	   /**< Vnote category */
	FILE_TYPE_VBOOKMARK,	   /**< Vbookmark category */
	FILE_TYPE_VIDEO_PROJECT,   /**< Video editor project category */
	FILE_TYPE_RADIO_RECORDED,  /**< radio recorded clips category */
	FILE_TYPE_MOVIE_MAKER,	   /**< Movie maker project category */
	FILE_TYPE_SVG,		   /**< Svg category */
	FILE_TYPE_RSS,		   /**< Rss reader file, *.opml */
	FILE_TYPE_CERTIFICATION,   /**< certification file, *.pem */
	FILE_TYPE_JAVA,		   /**< java file, *.jad, *.jar */
	FILE_TYPE_WGT,		   /**< wrt , *.wgt, *.wgt */
	FILE_TYPE_TPK,          /**< *.tpk>*/
	FILE_TYPE_SNB,          /**<*.snb> */
	FILE_TYPE_GUL,          /**<*.gul> */
	FILE_TYPE_HWP,          /**<*.hwp> */
	FILE_TYPE_ETC,          /**< Other files category */
	FILE_TYPE_TASK,
	FILE_TYPE_EML,
	FILE_TYPE_CSV,
	FILE_TYPE_SPD,
	FILE_TYPE_MAX
};

typedef enum _STORAGE MF_STORAGE;

enum _STORAGE {
	MYFILE_NONE = 0x00,
	MYFILE_PHONE = 0x01,
	MYFILE_MMC = 0x02,
	MYFILE_MAX = 0xFF
};


typedef enum _SIZE_TYPE MF_SIZE_TYPE;
enum _SIZE_TYPE {
	SIZE_BYTE = 0,
	SIZE_KB,
	SIZE_MB,
	SIZE_GB
};

typedef enum __MF_SORT_BY_PRIORITY_SEQUENCE MF_SORT_BY_PRIORITY_SEQUENCE;
enum __MF_SORT_BY_PRIORITY_SEQUENCE {
	MF_SORT_BY_PRIORITY_TYPE_A2Z,
	MF_SORT_BY_PRIORITY_TYPE_Z2A,
	MF_SORT_BY_PRIORITY_DATE_O2R,
	MF_SORT_BY_PRIORITY_DATE_R2O,
	MF_SORT_BY_PRIORITY_SIZE_S2L,
	MF_SORT_BY_PRIORITY_SIZE_L2S,
};

/* File operation error check options definition */
#define	MF_ERROR_CHECK_SRC_ARG_VALID        0x0001
#define	MF_ERROR_CHECK_DST_ARG_VALID		0x0002
#define	MF_ERROR_CHECK_SRC_EXIST			0x0004
#define	MF_ERROR_CHECK_DST_EXIST			0x0008
#define MF_ERROR_CHECK_SRC_PATH_VALID		0x0010
#define MF_ERROR_CHECK_DST_PATH_VALID		0x0020
#define	MF_ERROR_CHECK_SRC_PARENT_DIR_EXIST 0x0040
#define	MF_ERROR_CHECK_DST_PARENT_DIR_EXIST 0x0080
#define	MF_ERROR_CHECK_DUPLICATED			0x0100

/*	File system related callback definition	*/
typedef int (*myfile_operation_cb) (const char *current_path, const char *destination, int copied_size, 
                                    FILE * fSource, FILE * fDestination, void *data);

typedef enum
{
    STORAGE_TYPE_NONE = 0,
    STORAGE_TYPE_LABEL,
    STORAGE_TYPE_LOCAL,
    STORAGE_TYPE_MMC,
    STORAGE_TYPE_MUSIC,
    STORAGE_TYPE_IMAGE,
    STORAGE_TYPE_VIDEO,
    STORAGE_TYPE_DOCUMENT,
    STORAGE_TYPE_STORAGE,
} storage_type;

typedef struct _storage_info
{
    char *root_path;        /**< Storage path */
    char *root_name;        /**< Storage name */
    storage_type type;      /**< Storage type */
    char *icon_path;
    char *uuid;
    double total;
    double occupied;
}storage_info;

typedef struct _node_info
{
    char *parent_path;          /**< Node path */
    char *name;                 /**< Node name */
    fsFileType type;             /**< Node type */
    char *thumb_path;
    Eina_Bool is_selected;      /**< Node selected or not */
    char *storage_name;
    char *storage_uuid;
}node_info;

#define GIGABYTE (1024.0*1024.0*1024.0)

typedef struct _FS_NODE_INFO fsNodeInfo;
struct _FS_NODE_INFO {
	char *path;
	char *name;
	i18n_udate date;
	fsFileType type;
	int storage_type;
	int list_type;
	char *ext;
	off_t size;
	char* thumbnail_path;
	int thumbnail_type;
	Elm_Object_Item *item;
};

/*----------      File Attribute Related        ----------*/
void mf_file_attr_get_file_size_info(char **file_size, off_t src_size);
int mf_file_attr_get_file_stat(const char *filename, fsNodeInfo **node);
int mf_file_attr_get_file_category(const char *filepath, fsFileType *category);
int mf_file_attr_is_dir(const char *filepath);
int mf_file_attr_get_store_type_by_full(const char *filepath, MF_STORAGE *store_type);
int mf_file_attr_get_file_ext(const char *filepath, char **file_ext);
int mf_file_attr_is_duplicated_name(const char *dir, const char *name);
int mf_file_attr_is_valid_name(const char *filename);
int mf_file_attr_is_right_dir_path(const char *dir_path);
int mf_file_attr_is_right_file_path(const char *file_path);
int mf_file_attr_get_logical_path_by_full(const char *full_path, char **path);
int mf_file_attr_get_path_level(const char *fullpath, int *level);
int mf_file_attr_is_system_dir(char *fullpath, bool *result);
int mf_file_attr_is_disk_link(const char *fullpath, bool *result);
int mf_file_attr_get_file_mdate(const char *filename, i18n_udate *date);
int mf_file_attr_media_has_video(const char *filename);
int mf_file_attr_get_file_size(const char *filename, off_t *size);
int mf_file_attr_get_file_icon(const char *file_path, int *error_code, int view_type, char **thumbnail, 
                               media_info_h *media_info);
const char *mf_file_attr_get_recent_default_icon_by_type(fsFileType ftype);
const char *mf_file_attr_get_default_icon_by_type(fsFileType ftype);
fsFileType mf_file_attr_get_file_type_by_mime(const char *file_path);
fsFileType mf_file_attr_get_file_type(const char *mime);

/*----------      File Operation Related        ----------*/
int mf_fs_oper_error(const char *src, const char *dst, int check_option);
int mf_fs_oper_read_dir(const char *path, Eina_List **dir_list, Eina_List **file_list);
int mf_fs_oper_create_dir(const char *dir);
int mf_fs_oper_rename_file(const char *src, const char *dst);
void mf_fs_oper_print_node(fsNodeInfo *pNode);
void mf_fs_oper_sort_list(Eina_List **list, int sort_opt);
int __mf_fs_oper_sort_by_name_cb_A2Z(const void *d1, const void *d2);

#endif //_MYFILE_FILESYSTEM_UTIL_H_
