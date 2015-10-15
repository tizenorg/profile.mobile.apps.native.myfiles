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

#include <libgen.h>
#include <glib.h>
#include <sys/statvfs.h>
#include "mf-ug-detail-fs.h"
#include "mf-file-util.h"

struct _ug_dt_ftype_by_mime {
	const char *mime;
	File_Type ftype;
};

static struct _ug_dt_ftype_by_mime dt_mime_type[] = {
	{"image/png", FILE_TYPE_IMAGE},
	{"image/jpeg", FILE_TYPE_IMAGE},
	{"image/gif", FILE_TYPE_IMAGE},
	{"image/bmp", FILE_TYPE_IMAGE},
	{"image/vnd.wap.wbmp", FILE_TYPE_IMAGE},

	/*FILE_TYPE_VIDEO */
	{"video/x-msvideo", FILE_TYPE_VIDEO},
	{"video/mp4", FILE_TYPE_VIDEO},
	{"video/3gpp", FILE_TYPE_VIDEO},
	{"video/x-ms-asf", FILE_TYPE_VIDEO},
	{"video/x-ms-wmv", FILE_TYPE_VIDEO},
	{"video/x-matroska", FILE_TYPE_VIDEO},
	{"video/vnd.ms-playready.media.pyv", FILE_TYPE_VIDEO},
	{"video/divx", FILE_TYPE_VIDEO},
	{"video/avi", FILE_TYPE_VIDEO},

	/*FILE_TYPE_MUSIC */
	{"audio/mpeg", FILE_TYPE_MUSIC},
	{"audio/x-wav", FILE_TYPE_MUSIC},
	{"application/x-smaf", FILE_TYPE_MUSIC},
	{"audio/mxmf", FILE_TYPE_MUSIC},
	{"audio/midi", FILE_TYPE_MUSIC},
	{"audio/x-xmf", FILE_TYPE_MUSIC},
	{"audio/x-ms-wma", FILE_TYPE_MUSIC},
	{"audio/aac", FILE_TYPE_MUSIC},
	{"audio/ac3", FILE_TYPE_MUSIC},
	{"audio/ogg", FILE_TYPE_MUSIC},
	{"audio/vorbis", FILE_TYPE_MUSIC},
	{"audio/imelody", FILE_TYPE_MUSIC},
	{"audio/iMelody", FILE_TYPE_MUSIC},
	{"audio/x-rmf", FILE_TYPE_MUSIC},
	{"application/vnd.smaf", FILE_TYPE_MUSIC},
	{"audio/mobile-xmf", FILE_TYPE_MUSIC},
	{"audio/mid", FILE_TYPE_MUSIC},
	{"audio/vnd.ms-playready.media.pya", FILE_TYPE_MUSIC},
	{"audio/imy", FILE_TYPE_MUSIC},
	{"audio/m4a", FILE_TYPE_MUSIC},
	{"audio/melody", FILE_TYPE_MUSIC},
	{"audio/mmf", FILE_TYPE_MUSIC},
	{"audio/mp3", FILE_TYPE_MUSIC},
	{"audio/mp4", FILE_TYPE_MUSIC},
	{"audio/MP4A-LATM", FILE_TYPE_MUSIC},
	{"audio/mpeg3", FILE_TYPE_MUSIC},
	{"audio/mpeg4", FILE_TYPE_MUSIC},
	{"audio/mpg", FILE_TYPE_MUSIC},
	{"audio/mpg3", FILE_TYPE_MUSIC},
	{"audio/smaf", FILE_TYPE_MUSIC},
	{"audio/sp-midi", FILE_TYPE_MUSIC},
	{"audio/wav", FILE_TYPE_MUSIC},
	{"audio/wave", FILE_TYPE_MUSIC},
	{"audio/wma", FILE_TYPE_MUSIC},
	{"audio/xmf", FILE_TYPE_MUSIC},
	{"audio/x-mid", FILE_TYPE_MUSIC},
	{"audio/x-midi", FILE_TYPE_MUSIC},
	{"audio/x-mp3", FILE_TYPE_MUSIC},
	{"audio/-mpeg", FILE_TYPE_MUSIC},
	{"audio/x-mpeg", FILE_TYPE_MUSIC},
	{"audio/x-mpegaudio", FILE_TYPE_MUSIC},
	{"audio/x-mpg", FILE_TYPE_MUSIC},
	{"audio/x-ms-asf", FILE_TYPE_MUSIC},
	{"audio/x-wave", FILE_TYPE_MUSIC},

	/*FILE_TYPE_PDF */
	{"application/pdf", FILE_TYPE_PDF},

	/*FILE_TYPE_DOC */
	{"application/msword", FILE_TYPE_DOC},
	{"application/vnd.openxmlformats-officedocument.wordprocessingml.document", FILE_TYPE_DOC},

	/*FILE_TYPE_PPT */
	{"application/vnd.ms-powerpoint", FILE_TYPE_PPT},
	{"application/vnd.openxmlformats-officedocument.presentationml.presentation", FILE_TYPE_PPT},

	/*FILE_TYPE_EXCEL */
	{"application/vnd.ms-excel", FILE_TYPE_EXCEL},
	{"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", FILE_TYPE_EXCEL},

	/*FILE_TYPE_VOICE */
	{"audio/AMR", FILE_TYPE_VOICE},
	{"audio/AMR-WB", FILE_TYPE_VOICE},
	{"audio/amr", FILE_TYPE_VOICE},
	{"audio/amr-wb", FILE_TYPE_VOICE},
	{"audio/x-amr", FILE_TYPE_VOICE},

	/*FILE_TYPE_HTML */
	{"text/html", FILE_TYPE_HTML},

	/*FILE_TYPE_FLASH */
	{"application/x-shockwave-flash", FILE_TYPE_FLASH},
	{"video/x-flv", FILE_TYPE_FLASH},

	/*FILE_TYPE_TXT */
	{"text/plain", FILE_TYPE_TXT},

	/*FILE_TYPE_RSS */
	{"text/x-opml+xml", FILE_TYPE_RSS},

	/*FILE_TYPE_JAVA */
	{"text/vnd.sun.j2me.app-descriptor", FILE_TYPE_JAVA},
	{"application/x-java-archive", FILE_TYPE_JAVA},

	/*FILE_TYPE_ETC */
	{NULL, FILE_TYPE_ETC},
};


File_Type mf_ug_detail_fs_get_category_by_mime(const char *mime)
{

	ug_detail_retvm_if(mime == NULL, FILE_TYPE_NONE, "mime is NULL");

	int index = 0;
	File_Type ftype = FILE_TYPE_NONE;
	for (index = 0; dt_mime_type[index].mime; index++) {
		if (strncmp(mime, dt_mime_type[index].mime, strlen(mime)) == 0) {
			ftype = dt_mime_type[index].ftype;
			break;
		}
	}
	return ftype;
}

int mf_ug_detaill_fs_get_file_stat(const char *filename, Node_Info **node)
{
	struct stat statbuf = {0};
	struct tm tmtime;

	if (!filename || !node)
		return UG_MYFILE_ERR_GET_STAT_FAIL;

	if (stat(filename, &statbuf) == -1)
		return UG_MYFILE_ERR_GET_STAT_FAIL;

	if (*node == NULL)
		return UG_MYFILE_ERR_GET_STAT_FAIL;

 	(*node)->size = (LONG_LONG_UNSIGNED_INT)statbuf.st_size;
 	//(*node)->size = (LONG_LONG_UNSIGNED_INT)12*1024*1024*1024;//for testing for 12G
 	time_t temptime = statbuf.st_mtime;

	if (temptime == 0 && !g_strcmp0(filename, UG_MEMORY_FOLDER))
	{
		if(stat(UG_MEMORY_DEV_FOLDER, &statbuf) == -1)
		{
			temptime = time(NULL);
			localtime_r(&temptime, &tmtime);
		}
		else
		{
			temptime = statbuf.st_mtime;
		}
	}

	(*node)->date = (i18n_udate) temptime * 1000;

	return UG_MYFILE_ERR_NONE;
}

int mf_ug_detail_fs_is_dir(const char *filepath)
{
	if (filepath == NULL)
		return EINA_FALSE;

	return mf_is_dir(filepath);
}

/*********************
**Function name:	 mf_ug_detail_fs_get_file_ext
**Parameter:
**	const char *filepath:	file full path
**	char *file_ext:			output parameter of file extension
**
**Return value:
**	error code
**
**Action:
**	get file extension by file full path
**
*********************/
int  mf_ug_detail_fs_get_file_ext(const char *filepath, char *file_ext)
{

	if (filepath == NULL || file_ext == NULL)
		return UG_MYFILE_ERR_SRC_ARG_INVALID;

	const char *filename = mf_file_get(filepath);
	if (filename == NULL)
		return UG_MYFILE_ERR_INVALID_FILE_NAME;

	char *pdot = strrchr(filename, '.');
	if (!pdot) {
		return UG_MYFILE_ERR_EXT_GET_ERROR;
	} else if (pdot != filepath) {
		memcpy(file_ext, pdot + 1, strlen(pdot) - 1);
		return UG_MYFILE_ERR_NONE;
	} else {
		return UG_MYFILE_ERR_EXT_GET_ERROR;
	}
}

/*********************
**Function name:	__ ug_mf_dtl_assist_fs_get_category_by_file_ext
**Parameter:		const char *file_ext
**Return value:		fsFileType
**
**Action:
**	Get file category by extention
**
*********************/
static File_Type __mf_ug_detail_fs_get_category_by_file_ext(const char *file_ext, const char *fullpath)
{
	int i = 0;
	if (file_ext == NULL)
		return FILE_TYPE_ETC;

	if (file_ext[0] == '.')
		i = 1;

	switch (file_ext[i]) {
	case 'a':
	case 'A':
		if (strcasecmp("ASF", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;	/*2010.3.12. dsp.shin appended*/

		if (strcasecmp("AMR", &file_ext[i]) == 0)
			return FILE_TYPE_VOICE;

		if (strcasecmp("AWB", &file_ext[i]) == 0)
			return FILE_TYPE_VOICE;	/*2009.4.8 han. open wideband amr*/

		if (strcasecmp("AAC", &file_ext[i]) == 0)
			return FILE_TYPE_MUSIC;

		if (strcasecmp("AVI", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		if (strcasecmp("AAC", &file_ext[i]) == 0)
			return FILE_TYPE_MUSIC;	/*2010.4.12 shyeon.kim appended*/

		break;
	case 'b':
	case 'B':
		if (strcasecmp("BMP", &file_ext[i]) == 0)
			return FILE_TYPE_IMAGE;

		break;
	case 'd':
	case 'D':
		if (strcasecmp("DOC", &file_ext[i]) == 0)
			return FILE_TYPE_DOC;

		if (strcasecmp("DOCX", &file_ext[i]) == 0)
			return FILE_TYPE_DOC;

		if (strcasecmp("DIVX", &file_ext[i]) == 0) {
			{
				return FILE_TYPE_VIDEO;	/* 2009.4.8 : han. open*/
			}
		}
		break;
	case 'g':
	case 'G':
		if (strcasecmp("GIF", &file_ext[i]) == 0)
			return FILE_TYPE_IMAGE;

		if (strcasecmp("G72", &file_ext[i]) == 0)
			return FILE_TYPE_MUSIC;

		break;
	case 'h':
	case 'H':
		if (strcasecmp("H263", &file_ext[i]) == 0)
			return FILE_TYPE_MUSIC;

		if (strcasecmp("HTML", &file_ext[i]) == 0)
			return FILE_TYPE_HTML;

		if (strcasecmp("HTM", &file_ext[i]) == 0)
			return FILE_TYPE_HTML;

		break;
	case 'i':
	case 'I':
		if (strcasecmp("IMY", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("IPK", &file_ext[i]) == 0)
			return FILE_TYPE_APP;

		break;
	case 'j':
	case 'J':
		/*2009.5.7 han. added for java*/
		if (strcasecmp("JAD", &file_ext[i]) == 0)
			return FILE_TYPE_JAVA;

		if (strcasecmp("JAR", &file_ext[i]) == 0)
			return FILE_TYPE_JAVA;

		if (strcasecmp("JPG", &file_ext[i]) == 0)
			return FILE_TYPE_IMAGE;

		if (strcasecmp("JPEG", &file_ext[i]) == 0)
			return FILE_TYPE_IMAGE;

		/*2008.5.1 han. MTA issue.*/
		if (strcasecmp("JPE", &file_ext[i]) == 0)
			return FILE_TYPE_IMAGE;

		break;
	case 'm':
	case 'M':
		if (strcasecmp("MMF", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("MP3", &file_ext[i]) == 0)
			return FILE_TYPE_MUSIC;

		if (strcasecmp("MID", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("MIDI", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("MP4", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		/*"mp4" may be music*/
		if (strcasecmp("MPG", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		if (strcasecmp("MPEG", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		if (strcasecmp("M4A", &file_ext[i]) == 0)
			return FILE_TYPE_MUSIC;

		if (strcasecmp("M3G", &file_ext[i]) == 0)
			return FILE_TYPE_FLASH;

		if (strcasecmp("MXMF", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("MKV", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;	/*2010.4.12 shyeon.kim appended*/

		if (strcasecmp("MKA", &file_ext[i]) == 0)
			return FILE_TYPE_MUSIC;	/*2010.4.12 shyeon.kim appended*/

		break;
	case 'o':
	case 'O':
		if (strcasecmp("opml", &file_ext[i]) == 0)
			return FILE_TYPE_RSS;

		break;
	case 'p':
	case 'P':
		if (strcasecmp("PNG", &file_ext[i]) == 0)
			return FILE_TYPE_IMAGE;

		if (strcasecmp("PJPEG", &file_ext[i]) == 0)
			return FILE_TYPE_IMAGE;

		if (strcasecmp("PDF", &file_ext[i]) == 0)
			return FILE_TYPE_PDF;

		if (strcasecmp("PPT", &file_ext[i]) == 0)
			return FILE_TYPE_PPT;

		if (strcasecmp("PPTX", &file_ext[i]) == 0)
			return FILE_TYPE_PPT;

		if (strcasecmp("PEM", &file_ext[i]) == 0)
			return FILE_TYPE_CERTIFICATION;

		break;
		/* 2008.3.5 han.*/
	case 'r':
	case 'R':
		break;
	case 's':
	case 'S':
		if (strcasecmp("SDP", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		if (strcasecmp("SPM", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("SMP", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("SPF", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("SWF", &file_ext[i]) == 0)
			return FILE_TYPE_FLASH;

		if (strcasecmp("SCN", &file_ext[i]) == 0)
			return FILE_TYPE_MOVIE_MAKER;

		if (strcasecmp("SVG", &file_ext[i]) == 0)
			return FILE_TYPE_SVG;

		if (strcasecmp("SVGZ", &file_ext[i]) == 0)
			return FILE_TYPE_SVG;	/* 2009.4.17 han.*/

		break;
	case 't':
	case 'T':
		if (strcasecmp("TXT", &file_ext[i]) == 0)
			return FILE_TYPE_TXT;

		if (strcasecmp("THM", &file_ext[i]) == 0)
			return FILE_TYPE_THEME;

		break;
	case 'v':
	case 'V':
		if (strcasecmp("VCF", &file_ext[i]) == 0)
			return FILE_TYPE_VCONTACT;

		if (strcasecmp("VCS", &file_ext[i]) == 0)
			return FILE_TYPE_VCALENDAR;

		if (strcasecmp("VNT", &file_ext[i]) == 0)
			return FILE_TYPE_VNOTE;

		if (strcasecmp("VBM", &file_ext[i]) == 0)
			return FILE_TYPE_VBOOKMARK;

		break;
	case 'w':
	case 'W':
		if (strcasecmp("WAV", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;	/*modified by dsp.shin, 2010.3.13. to support music player*/

		/*2008.10.17 han.*/
		if (strcasecmp("WBMP", &file_ext[i]) == 0)
			return FILE_TYPE_IMAGE;

		if (strcasecmp("WGT", &file_ext[i]) == 0)
			return FILE_TYPE_WGT;

		if (strcasecmp("WMA", &file_ext[i]) == 0)
			return FILE_TYPE_MUSIC;

		if (strcasecmp("WMV", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		break;
	case 'x':
	case 'X':
		if (strcasecmp("XLS", &file_ext[i]) == 0)
			return FILE_TYPE_EXCEL;

		if (strcasecmp("XLSX", &file_ext[i]) == 0)
			return FILE_TYPE_EXCEL;

		if (strcasecmp("XMF", &file_ext[i]) == 0)
			return FILE_TYPE_SOUND;

		if (strcasecmp("XHTML", &file_ext[i]) == 0)
			return FILE_TYPE_HTML;

		break;
	case '3':
		if (strcasecmp("3GP", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		if (strcasecmp("3GPP", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		if (strcasecmp("3G2", &file_ext[i]) == 0)
			return FILE_TYPE_VIDEO;

		/*"3GP", "3GPP" may be music*/
		break;
	}

	return FILE_TYPE_ETC;
}

int mf_ug_detail_fs_get_file_type(const char *filepath, File_Type * category)
{

	if (filepath == NULL || category == NULL)
		return UG_MYFILE_ERR_SRC_ARG_INVALID;

	int i = 0;
	int flag = 0;

	if (mf_ug_detail_fs_is_dir(filepath)) {
		*category = FILE_TYPE_DIR;
		return UG_MYFILE_ERR_NONE;
	}

	const char *filename = NULL;
	filename = mf_file_get(filepath);
	/*return value ceck*/
	if (filename == NULL) {
		*category = FILE_TYPE_NONE;
		return UG_MYFILE_ERR_SRC_ARG_INVALID;
	}
	char file_ext[UG_FILE_EXT_LEN_MAX + 1] = { 0 };
	/*ToDo: error file name like the last letter is "."*/
	for (i = strlen(filename); i >= 0; i--) {
		if (filename[i] == '.') {
			//strncpy(file_ext, &filename[i + 1], UG_FILE_EXT_LEN_MAX + 1);
			strncpy(file_ext, &filename[i + 1], UG_FILE_EXT_LEN_MAX);
			flag = 1;
			break;
		}
		/*meet the dir. no ext*/
		if (filename[i] == '/')	{
			flag = 0;
			break;
		}
	}

	if (flag == 1) {
		*category = __mf_ug_detail_fs_get_category_by_file_ext(file_ext, filepath);
		return UG_MYFILE_ERR_NONE;
	} else {
		*category = FILE_TYPE_NONE;
		return UG_MYFILE_ERR_GET_CATEGORY_FAIL;
	}
}

/*********************
**Function name:	mf_ug_detail_fs_get_store_type
**Parameter:
**	const char *filepath:	file full path
**	Mf_Storage *store_type:		output parameter of storage type
**Return value:
**	error code
**
**Action:
**	Get file storage type by file path
**
*********************/
int mf_ug_detail_fs_get_store_type(const char *filepath, Mf_Storage *store_type)
{
	if (filepath == NULL || store_type == NULL)
		return UG_MYFILE_ERR_SRC_ARG_INVALID;

	if (g_str_has_prefix(filepath, UG_PHONE_FOLDER)) {
		*store_type = D_MYFILE_PHONE;
		return UG_MYFILE_ERR_NONE;	/*store in phone*/
	} else if (g_str_has_prefix(filepath, UG_MEMORY_FOLDER)) {
		*store_type = D_MYFILE_MMC;
		return UG_MYFILE_ERR_NONE;	/*store in MMC*/
	} else {
		*store_type = D_MYFILE_NONE;
		return UG_MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
}

/*********************
**Function name:	mf_ug_detail_fs_get_logi_path
**Parameter:
**	const char *full_path:	the full path
**	char *path:				logic path of output parameter
**
**Return value:
**	error code
**
**Action:
**	get logic path by full path
**
*********************/
int mf_ug_detail_fs_get_logi_path(const char *full_path, char *path)
{
	if (full_path == NULL || path == NULL)
		return UG_MYFILE_ERR_STORAGE_TYPE_ERROR;

	Mf_Storage store_type = 0;
	int root_len = 0;
	//int error_code = 0;

	mf_ug_detail_fs_get_store_type(full_path, &store_type);

	switch (store_type) {
	case D_MYFILE_PHONE:
		root_len = strlen(UG_PHONE_FOLDER);
		break;
	case D_MYFILE_MMC:
		root_len = strlen(UG_MEMORY_FOLDER);
		break;
	default:
		return UG_MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
	/*size of path is UG_MYFILE_DIR_PATH_LEN_MAX+1*/
	g_strlcpy(path, full_path + root_len, UG_MYFILE_DIR_PATH_LEN_MAX);

	if (strlen(path) == 0)
		g_strlcpy(path, "/", UG_MYFILE_DIR_PATH_LEN_MAX);


	return UG_MYFILE_ERR_NONE;
}

GString *mf_ug_detail_fs_parse_file_type(GString * path)
{
	GString *catetory = NULL;
	File_Type category_t = FILE_TYPE_NONE;

	if (path != NULL)
		mf_ug_detail_fs_get_file_type(path->str, &category_t);

	switch (category_t) {
	case FILE_TYPE_NONE:						   /**< Default */
		catetory = g_string_new("None");
		break;
	case FILE_TYPE_IMAGE:						   /**< Image category */
		catetory = g_string_new("Image");
		break;

	case FILE_TYPE_VIDEO:						   /**< Video category */
		catetory = g_string_new("Video");
		break;

	case FILE_TYPE_MUSIC:						   /**< Music category */
		catetory = g_string_new("Music");
		break;

	case FILE_TYPE_SOUND:						   /**< Sound category */
		catetory = g_string_new("Sound");
		break;

	case FILE_TYPE_PDF:						   /**< Pdf category */
		catetory = g_string_new("PDF");
		break;

	case FILE_TYPE_DOC:						   /**< Word category */
		catetory = g_string_new("Doc");
		break;

	case FILE_TYPE_PPT:						   /**< Powerpoint category */
		catetory = g_string_new("PPT");
		break;

	case FILE_TYPE_EXCEL:						   /**< Excel category */
		catetory = g_string_new("Excel");
		break;

	case FILE_TYPE_VOICE:						   /**< Voice category */
		catetory = g_string_new("Sound");
		break;

	case FILE_TYPE_HTML:						   /**< Html category */
		catetory = g_string_new("HTML");
		break;

	case FILE_TYPE_FLASH:						   /**< Flash category */
		catetory = g_string_new("Flash");
		break;

	case FILE_TYPE_GAME:						   /**< Game category */
		catetory = g_string_new("Game");
		break;

	case FILE_TYPE_APP:						   /**< Application category */
		catetory = g_string_new("App");
		break;

	case FILE_TYPE_THEME:						   /**< Theme category */
		catetory = g_string_new("Theme");
		break;

	case FILE_TYPE_TXT:						   /**< Txt category */
		catetory = g_string_new("Text");
		break;

	case FILE_TYPE_VCONTACT:				   /**< Vcontact category */
		catetory = g_string_new("Contact");
		break;

	case FILE_TYPE_VCALENDAR:				   /**< Vcalendar category */
		catetory = g_string_new("Calendar");
		break;

	case FILE_TYPE_VNOTE:						   /**< Vnote category */
		catetory = g_string_new("Note");
		break;

	case FILE_TYPE_VBOOKMARK:				   /**< Vbookmark category */
		catetory = g_string_new("Bookmark");
		break;

	case FILE_TYPE_VIDEO_PROJECT:				   /**< Video editor project category */
		catetory = g_string_new("Video Project");
		break;

	case FILE_TYPE_RADIO_RECORDED:				   /**< radio recorded clips category */
		catetory = g_string_new("Radio Recorded");
		break;

	case FILE_TYPE_MOVIE_MAKER:				   /**< Movie maker project category */
		catetory = g_string_new("Movie Maker");
		break;

	case FILE_TYPE_SVG:						   /**< Svg category */
		catetory = g_string_new("SVG");
		break;

	case FILE_TYPE_RSS:						   /**< Rss reader file, *.opml */
		catetory = g_string_new("RSS");
		break;

	case FILE_TYPE_CERTIFICATION:				   /**< certification file, *.pem */
		catetory = g_string_new("Certification");
		break;

	case FILE_TYPE_JAVA:						   /**< java file, *.jad, *.jar */
		catetory = g_string_new("JAVA");
		break;

	case FILE_TYPE_WGT:						   /**< wrt , *.wgt, *.wgt */
		catetory = g_string_new("WGT");
		break;

	case FILE_TYPE_ETC:						   /**< Other files category */
	default:
		catetory = g_string_new("ETC");
		break;

	}
	myfile_dlog("%s %d\n", __func__, __LINE__);
	return catetory;
}

/******************************
** Prototype    : mf_ug_detail_fs_get_list_len
** Description  :
** Input        : const Eina_List *list
** Output       : int
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
int mf_ug_detail_fs_get_list_len(const Eina_List *list)
{
	if (list == NULL)
		return 0;

	return eina_list_count(list);
}

View_Style mf_ug_detail_fs_get_view_type(char *path, GString * category)
{
	myfile_dlog("%s %d\n", __func__, __LINE__);

	if (path == NULL)
		return VIEW_NONE;

	if (mf_ug_detail_fs_is_dir(path)) {
		return VIEW_DIR;
	} else {
		{
			if (category == NULL || category->str == NULL) {
				return VIEW_NONE;
			} else {
				if (strcmp(category->str, "Image") == 0 || strcmp(category->str, "Video") == 0) {
					return VIEW_FILE_WITH_GPS;
				} else {
					return VIEW_FILE_NORMAL;
				}
			}
		}
	}
}

static int __mf_ug_detail_fs_get_parent_path(const char *path, char *parent_path)
{

	if (path == NULL || parent_path == NULL)
		return UG_MYFILE_ERR_SRC_ARG_INVALID;
	/*parent_path size is UG_MYFILE_DIR_PATH_LEN_MAX+1*/
	g_strlcpy(parent_path, path, UG_MYFILE_DIR_PATH_LEN_MAX);

	const char *name = NULL;
	name = mf_file_get(path);
	if (name == NULL)
		return UG_MYFILE_ERR_SRC_ARG_INVALID;

	parent_path[strlen(parent_path) - strlen(name) - 1] = '\0';

	if (strlen(parent_path) == 0)
		g_strlcpy(parent_path, "/", UG_MYFILE_DIR_PATH_LEN_MAX);

	myfile_dlog("%s %d\n", __func__, __LINE__);

	return UG_MYFILE_ERR_NONE;
}

GString *mf_ug_detail_fs_get_parent(char *fullpath)
{
	myfile_dlog("%s %d\n", __func__, __LINE__);
	GString *ret = NULL;
	char path[UG_MYFILE_DIR_PATH_LEN_MAX + 1] = { '\0', };
	int error_code = 0;

	if (fullpath == NULL)
		return NULL;

	error_code = __mf_ug_detail_fs_get_parent_path(fullpath, path);
	if (error_code != 0)
		return NULL;

	ret = g_string_new(path);
	myfile_dlog("%s %d\n", __func__, __LINE__);
	return ret;
}

/*********************
**Function name:	mf_ug_detail_fs_read_dir
**Parameter:
**	char *path:				path which we need to read
**	Eina_List** dir_list:	output parameter of dir list under specified path
**	Eina_List** file_list:	output parameter of file list under specified path
**
**Return value:
**	error code
**
**Action:
**	read element under the specified path
**
*********************/
int mf_ug_detail_fs_read_dir(char *path, Eina_List **dir_list, Eina_List **file_list)
{
	if (path == NULL)
		return UG_MYFILE_ERR_DIR_OPEN_FAIL;

	DIR *pDir = NULL;
	struct dirent ent_struct;
	struct dirent *ent = NULL;
	int count = 0;
	char childpath[UG_MYFILE_CHILDPATH_LEN] = { 0, };


	int ret = 0;
	pDir = opendir(path);

	if (pDir == NULL)
		return UG_MYFILE_ERR_DIR_OPEN_FAIL;

	while ((readdir_r(pDir, &ent_struct, &ent) == 0) && ent) {
		if (strncmp(ent->d_name, ".", 1) == 0 || strcmp(ent->d_name, "..") == 0) {
			continue;
		}
		/*only deal with dirs and regular files*/
		if ((ent->d_type & DT_DIR) == 0 && (ent->d_type & DT_REG) == 0)
			continue;

#ifdef	DEBUG_FOLDER_OPTION
		if ((ent->d_type & DT_DIR) != 0) {
			if ((strlen(path) == strlen(UG_PHONE_FOLDER)) && (g_strcmp0(path, UG_PHONE_FOLDER) == 0) &&
			    (strlen(ent->d_name) == strlen(DEBUG_FOLDER)) && (g_strcmp0(ent->d_name, DEBUG_FOLDER) == 0)) {
				continue;
			}
		}
#endif
		Node_Info *pNode = (Node_Info *) malloc(sizeof(Node_Info));

		if (pNode == NULL)
			continue;

		memset(pNode, 0, sizeof(Node_Info));
		/*get path*/

		g_strlcpy(pNode->path, path, (gsize) sizeof(pNode->path));

		/*get name*/

		g_strlcpy(pNode->name, ent->d_name, (gsize) sizeof(pNode->name));

		/*get type*/
		if (ent->d_type & DT_DIR) {
			pNode->type = FILE_TYPE_DIR;
		} else if (ent->d_type & DT_REG) {
			mf_ug_detail_fs_get_file_type(ent->d_name, &(pNode->type));
		}

		/*get date & size*/
		int copiednum = snprintf(childpath, sizeof(childpath) - 1, "%s/%s", path, ent->d_name);
		if (copiednum < 0) {
			free(pNode);
			pNode = NULL;
			continue;
		}
		if (mf_ug_detaill_fs_get_file_stat(childpath, &pNode) == UG_MYFILE_ERR_GET_STAT_FAIL) {
			free(pNode);
			pNode = NULL;
			continue;
		}

		if (pNode->type == FILE_TYPE_DIR) {
			*dir_list = eina_list_append(*dir_list, pNode);
		} else {
			ret =  mf_ug_detail_fs_get_file_ext(childpath, pNode->ext);
			if (ret != UG_MYFILE_ERR_NONE) {
				memset(pNode->ext, 0, UG_FILE_EXT_LEN_MAX);
			}
			*file_list = eina_list_append(*file_list, pNode);
		}
		count++;
	}
	closedir(pDir);

	return UG_MYFILE_ERR_NONE;
}

/******************************
** Prototype    : mf_ug_detail_fs_get_file_list
** Description  :
** Input        : GString* folder_name
**                Eina_List** dir_list
**                Eina_List** file_list
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
int mf_ug_detail_fs_get_file_list(GString *folder_name, Eina_List **dir_list, Eina_List **file_list)
{

	myfile_dlog("%s %d\n", __func__, __LINE__);

	if (folder_name == NULL || folder_name->str == NULL || folder_name->len == 0)
		return 0;

	int error_code = 0;
	if (folder_name != NULL) {
		error_code = mf_ug_detail_fs_read_dir(folder_name->str, dir_list, file_list);
	}
	myfile_dlog("%s %d\n", __func__, __LINE__);

	return error_code;
}

LONG_LONG_UNSIGNED_INT mf_ug_detail_fs_get_storage_size(char *path)
{
	LONG_LONG_UNSIGNED_INT use_space = 0;

	struct statvfs statbuf;
	if (g_strcmp0(path, UG_PHONE_FOLDER) == 0 ||  g_strcmp0(path, UG_MEMORY_FOLDER) == 0) {
		if (statvfs(path, &statbuf) == -1) {
			return use_space;
		} else {
			use_space = (statbuf.f_blocks - statbuf.f_bfree)*statbuf.f_bsize;
		}
	}
	return use_space;
}

LONG_LONG_UNSIGNED_INT mf_ug_detail_fs_get_folder_size(char *path)
{
	myfile_dlog("%s %d\n", __func__, __LINE__);

	LONG_LONG_UNSIGNED_INT size = 0;
	GString *fullpath = g_string_new(path);
	Eina_List *file_list = NULL;
	Eina_List *dir_list = NULL;

	if (mf_ug_detail_fs_is_dir(path)) {
		size = mf_ug_detail_fs_get_storage_size(path);
		if (size != 0)
			return size;

		int error_code = 0;
		error_code = mf_ug_detail_fs_get_file_list(fullpath, &dir_list, &file_list);

		if (error_code == 0) {
			int i = 0;
			int dir_list_len =  mf_ug_detail_fs_get_list_len(dir_list);
			int file_list_len = mf_ug_detail_fs_get_list_len(file_list);
			myfile_dlog("dir_list_len is [%d]\nfile_list_len is [%d]\n", dir_list_len, file_list_len);
			for (i = 0; i < file_list_len; i++) {
				Node_Info *pNode = NULL;
				pNode = (Node_Info *) eina_list_nth(file_list, i);
				if (pNode == NULL)
					continue;
				size += pNode->size;
			}
			i = 0;
			for (i = 0; i < dir_list_len; i++) {
				Node_Info *pNode = NULL;
				char *full_path = NULL;
				pNode = (Node_Info *) eina_list_nth(dir_list, i);
				if (pNode == NULL)
					continue;
				full_path =  g_strconcat(pNode->path, "/", pNode->name, NULL);
				myfile_dlog("full_path is [%s]\n", full_path);
				size += (mf_ug_detail_fs_get_folder_size(full_path));
				free(full_path);
				full_path = NULL;
			}
		}
	}

	if (fullpath) {
		g_string_free(fullpath, TRUE);
		fullpath = NULL;
	}

	if (file_list) {
		eina_list_free(file_list);
		file_list = NULL;
	}

	if (dir_list) {
		eina_list_free(dir_list);
		dir_list = NULL;
	}

	myfile_dlog("%s %d\n", __func__, __LINE__);
	return size;
}

int mf_ug_detail_fs_check_path(void *data, char *path)
{
	if (path == NULL || !mf_file_exists(path)) {
		return 0;
	} else {
		return 1;
	}
}
