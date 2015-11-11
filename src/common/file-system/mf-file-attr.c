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

#include <regex.h>
#include <sys/types.h>
#include <media_content.h>
#include <assert.h>
#include <app.h>
#include <metadata_extractor.h>
#include <mime_type.h>


#include "mf-fs-util.h"
#include "mf-util.h"
#include "mf-ta.h"
#include "mf-conf.h"
#include "mf-media-content.h"
#include "mf-file-util.h"

#define MF_PHONE_DEFAULT_LEVEL		3   /*the phone path is /opt/usr/media, it consists of opt and media two parts*/
#define MF_MMC_DEFAULT_LEVEL 		3   /*the mmc path is /opt/storage/sdcard, it consists of opt and storage and sdcard three parts*/
typedef struct __mf_filter_s mf_filter_s;
struct __mf_filter_s {
	char *cond;                              /*set media type or favorite type, or other query statement*/
	media_content_collation_e collate_type;  /*collate type*/
	media_content_order_e sort_type;         /*sort type*/
	char *sort_keyword;                      /*sort keyword*/
	int offset;                              /*offset*/
	int count;                               /*count*/
	bool with_meta;                          /*whether get image or video info*/
};
#define CONDITION_LENGTH 200
#define MF_CONDITION_IMAGE_VIDEO "(MEDIA_TYPE=0 OR MEDIA_TYPE=1 OR MEDIA_TYPE=2 OR MEDIA_TYPE=3)"//"(MEDIA_TYPE=0 OR MEDIA_TYPE=1)"

struct _ftype_by_mime {
	const char *mime;
	fsFileType ftype;
};

static struct _ftype_by_mime mime_type[] = {
	{"image/png", FILE_TYPE_IMAGE},
	{"image/jpeg", FILE_TYPE_IMAGE},
	{"image/gif", FILE_TYPE_IMAGE},
	{"image/bmp", FILE_TYPE_IMAGE},
	{"image/vnd.wap.wbmp", FILE_TYPE_IMAGE},
	{"image/jp2", FILE_TYPE_IMAGE},
	{"image/tif", FILE_TYPE_IMAGE},
	{"image/wmf", FILE_TYPE_IMAGE},

	/*FILE_TYPE_VIDEO */
	{"video/x-msvideo", FILE_TYPE_VIDEO},
	{"video/mp4", FILE_TYPE_VIDEO},
	{"video/3gpp", FILE_TYPE_VIDEO},
	{"video/x-ms-asf", FILE_TYPE_VIDEO},
	{"video/x-ms-wmv", FILE_TYPE_VIDEO},
	{"video/x-matroska", FILE_TYPE_VIDEO},

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
	{"audio/ra", FILE_TYPE_MUSIC},
	{"audio/x-vorbis+ogg", FILE_TYPE_MUSIC},
	{"audio/vorbis", FILE_TYPE_MUSIC},
	{"audio/imelody", FILE_TYPE_MUSIC},
	{"audio/iMelody", FILE_TYPE_MUSIC},
	{"audio/x-rmf", FILE_TYPE_MUSIC},
	{"application/vnd.smaf", FILE_TYPE_MUSIC},
	{"audio/mobile-xmf", FILE_TYPE_MUSIC},
	{"audio/mid", FILE_TYPE_MUSIC},
	{"audio/vnd.ms-playready.media.pya", FILE_TYPE_MUSIC},
	{"audio/imy", FILE_TYPE_MUSIC},
	{"audio/m4a", FILE_TYPE_VOICE},
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
	{"audio/x-flac", FILE_TYPE_MUSIC},
	{"text/x-iMelody", FILE_TYPE_SOUND},
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
	{"text/xml", FILE_TYPE_HTML},//Fixed P131114-04144

	/*FILE_TYPE_FLASH */
	{"application/x-shockwave-flash", FILE_TYPE_FLASH},
	{"video/x-flv", FILE_TYPE_FLASH},

	/*FILE_TYPE_TXT */
	{"text/plain", FILE_TYPE_GUL},
	{"text/calendar", FILE_TYPE_VCALENDAR},

	{"application/vnd.tizen.package", FILE_TYPE_TPK},
	/*FILE_TYPE_RSS */
	{"text/x-opml+xml", FILE_TYPE_RSS},

	/*FILE_TYPE_JAVA */
	{"text/vnd.sun.j2me.app-descriptor", FILE_TYPE_JAVA},
	{"application/x-java-archive", FILE_TYPE_JAVA},

	/* FILE_TYPE_VCONTACT */
	{"text/directory", FILE_TYPE_VCONTACT},
	{"text/x-vcard", FILE_TYPE_VCONTACT},
	{"application/snb", FILE_TYPE_SNB},
	{"application/x-hwp", FILE_TYPE_HWP},

	/*FILE_TYPE_ETC */
	{NULL, FILE_TYPE_ETC},
};

static char *icon_array[FILE_TYPE_MAX] = {
	[FILE_TYPE_DIR] = MF_ICON_FOLDER,
	[FILE_TYPE_IMAGE] = MF_ICON_IMAGE,
	[FILE_TYPE_VIDEO] = MF_ICON_VIDEO,
	[FILE_TYPE_MUSIC] = MF_ICON_MUSIC_THUMBNAIL, /*MF_ICON_MUSIC,*/
	[FILE_TYPE_SOUND] = MF_ICON_MUSIC_THUMBNAIL, /*MF_ICON_SOUND,*/
	[FILE_TYPE_PDF] = MF_ICON_PDF,
	[FILE_TYPE_DOC] = MF_ICON_DOC,
	[FILE_TYPE_PPT] = MF_ICON_PPT,
	[FILE_TYPE_EXCEL] = MF_ICON_EXCEL,
	[FILE_TYPE_VOICE] = MF_ICON_SOUND,
	[FILE_TYPE_HTML] = MF_ICON_HTML,
	[FILE_TYPE_FLASH] = MF_ICON_FLASH,
	[FILE_TYPE_TXT] = MF_ICON_TXT,
	[FILE_TYPE_TPK] = MF_ICON_TPK,
	[FILE_TYPE_VCONTACT] = MF_ICON_VCONTACT,
	[FILE_TYPE_VCALENDAR] = MF_ICON_VCALENDAR,
	[FILE_TYPE_VNOTE] = MF_ICON_VNOTE,
	[FILE_TYPE_RSS] = MF_ICON_RSS,
	[FILE_TYPE_JAVA] = MF_ICON_JAVA,
	[FILE_TYPE_HWP] = MF_ICON_HWP,
	[FILE_TYPE_SNB] = MF_ICON_SNB,
	[FILE_TYPE_GUL] = MF_ICON_GUL,
	[FILE_TYPE_TASK] = MF_ICON_TASK,
	[FILE_TYPE_EML] = MF_ICON_EMAIL,
	[FILE_TYPE_CSV] = MF_ICON_EXCEL,
	[FILE_TYPE_SVG] = MF_ICON_SVG,
	[FILE_TYPE_WGT] = MF_ICON_TPK,
};

/*********************
**Function name:	__mf_file_attr_get_category_by_file_ext
**Parameter:		const char *file_ext
**Return value:		fsFileType
**
**Action:
**	Get file category by extention
**
*********************/
static fsFileType __mf_file_attr_get_category_by_file_ext(const char *file_ext, const char *fullpath)
{
	int i = 0;

	if (file_ext == NULL) {
		return FILE_TYPE_ETC;
	}

	if (file_ext[0] == '.') {
		i = 1;
	}

	switch (file_ext[i]) {
	case 'a':
	case 'A':
		if (strcasecmp("ASF", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("AMR", &file_ext[i]) == 0) {
			return FILE_TYPE_VOICE;
		}
		if (strcasecmp("AWB", &file_ext[i]) == 0) {
			return FILE_TYPE_VOICE;
		}
		if (strcasecmp("AAC", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		if (strcasecmp("AVI", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("AAC", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}

		break;
	case 'b':
	case 'B':
		if (strcasecmp("BMP", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		break;
	case 'c':
	case 'C':
		if (strcasecmp("csv", &file_ext[i]) == 0) {
			return FILE_TYPE_CSV;
		}
		break;
	case 'd':
	case 'D':
		if (strcasecmp("DOC", &file_ext[i]) == 0) {
			return FILE_TYPE_DOC;
		}
		if (strcasecmp("DOCX", &file_ext[i]) == 0) {
			return FILE_TYPE_DOC;
		}
		if (strcasecmp("DIVX", &file_ext[i]) == 0) {
			{
				return FILE_TYPE_VIDEO;
			}
		}
		break;
	case 'E':
	case 'e':
		if (strcasecmp("EML", &file_ext[i]) == 0) {
			return FILE_TYPE_EML;
		}
		break;
	case 'f':
	case 'F':
		if (strcasecmp("FLAC", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		break;
	case 'g':
	case 'G':
		if (strcasecmp("GIF", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		if (strcasecmp("G72", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		if (strcasecmp("GUL", &file_ext[i]) == 0) {
			return FILE_TYPE_GUL;
		}
		break;
	case 'h':
	case 'H':
		if (strcasecmp("HTML", &file_ext[i]) == 0) {
			return FILE_TYPE_HTML;
		}
		if (strcasecmp("HTM", &file_ext[i]) == 0) {
			return FILE_TYPE_HTML;
		}
		if (strcasecmp("HWP", &file_ext[i]) == 0) {
			return FILE_TYPE_HWP;
		}
		break;
	case 'i':
	case 'I':
		if (strcasecmp("IMY", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("IPK", &file_ext[i]) == 0) {
			return FILE_TYPE_APP;
		}
		if (strcasecmp("ICS", &file_ext[i]) == 0) {
			return FILE_TYPE_VCALENDAR;
		}
		break;
	case 'j':
	case 'J':
		if (strcasecmp("JAD", &file_ext[i]) == 0) {
			return FILE_TYPE_JAVA;
		}
		if (strcasecmp("JAR", &file_ext[i]) == 0) {
			return FILE_TYPE_JAVA;
		}

		if (strcasecmp("JPG", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		if (strcasecmp("JPEG", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		if (strcasecmp("JPE", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		if (strcasecmp("JP2", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		break;
	case 'm':
	case 'M':
		if (strcasecmp("MMF", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("MP3", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		if (strcasecmp("MID", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("MIDI", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("MP4", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("MPG", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("MPEG", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("M4A", &file_ext[i]) == 0) {
			//Fix P131207-02509,keep the type same with the one media_info_get_media_type() gets
			return FILE_TYPE_MUSIC;
		}
		if (strcasecmp("M3G", &file_ext[i]) == 0) {
			return FILE_TYPE_FLASH;
		}
		if (strcasecmp("MXMF", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("MKV", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("MKA", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		break;
	case 'o':
	case 'O':
		if (strcasecmp("opml", &file_ext[i]) == 0) {
			return FILE_TYPE_RSS;
		}
		if (strcasecmp("ogg", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		break;
	case 'p':
	case 'P':
		if (strcasecmp("PNG", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		if (strcasecmp("PJPEG", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		if (strcasecmp("PDF", &file_ext[i]) == 0) {
			return FILE_TYPE_PDF;
		}
		if (strcasecmp("PPT", &file_ext[i]) == 0) {
			return FILE_TYPE_PPT;
		}
		if (strcasecmp("PPTX", &file_ext[i]) == 0) {
			return FILE_TYPE_PPT;
		}
		if (strcasecmp("PEM", &file_ext[i]) == 0) {
			return FILE_TYPE_CERTIFICATION;
		}
		break;
	case 'r':
	case 'R':
		if (strcasecmp("RA", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		break;
	case 's':
	case 'S':
		if (strcasecmp("SDP", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("SPM", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("SMP", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("SPF", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("SWF", &file_ext[i]) == 0) {
			return FILE_TYPE_FLASH;
		}
		if (strcasecmp("SCN", &file_ext[i]) == 0) {
			return FILE_TYPE_MOVIE_MAKER;
		}
		if (strcasecmp("SVG", &file_ext[i]) == 0) {
			return FILE_TYPE_SVG;
		}
		if (strcasecmp("SVGZ", &file_ext[i]) == 0) {
			return FILE_TYPE_SVG;
		}
		if (strcasecmp("SNB", &file_ext[i]) == 0) {
			return FILE_TYPE_SNB;
		}
		if (strcasecmp("SPD", &file_ext[i]) == 0) {
			return FILE_TYPE_SPD;
		}

		break;
	case 't':
	case 'T':
		if (strcasecmp("TXT", &file_ext[i]) == 0) {
			return FILE_TYPE_TXT;
		}
		if (strcasecmp("THM", &file_ext[i]) == 0) {
			return FILE_TYPE_THEME;
		}
		if (strcasecmp("TPK", &file_ext[i]) == 0) {
			return FILE_TYPE_TPK;
		}
		if (strcasecmp("TIF", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		break;
	case 'v':
	case 'V':
		if (strcasecmp("VCF", &file_ext[i]) == 0) {
			return FILE_TYPE_VCONTACT;
		}
		if (strcasecmp("VCS", &file_ext[i]) == 0) {
			return FILE_TYPE_VCALENDAR;
		}
		if (strcasecmp("VNT", &file_ext[i]) == 0) {
			return FILE_TYPE_VNOTE;
		}
		if (strcasecmp("VBM", &file_ext[i]) == 0) {
			return FILE_TYPE_VBOOKMARK;
		}
		if (strcasecmp("VTS", &file_ext[i]) == 0) {
			return FILE_TYPE_TASK;
		}
		break;
	case 'w':
	case 'W':
		if (strcasecmp("WAV", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("WBMP", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		if (strcasecmp("WGT", &file_ext[i]) == 0) {
			return FILE_TYPE_WGT;
		}
		if (strcasecmp("WMA", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		if (strcasecmp("WMV", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("WML", &file_ext[i]) == 0) {
			return FILE_TYPE_HTML;
		}
		if (strcasecmp("WAVE", &file_ext[i]) == 0) {
			return FILE_TYPE_MUSIC;
		}
		if (strcasecmp("WMF", &file_ext[i]) == 0) {
			return FILE_TYPE_IMAGE;
		}
		break;
	case 'x':
	case 'X':
		if (strcasecmp("XLS", &file_ext[i]) == 0) {
			return FILE_TYPE_EXCEL;
		}
		if (strcasecmp("XLSX", &file_ext[i]) == 0) {
			return FILE_TYPE_EXCEL;
		}
		if (strcasecmp("XMF", &file_ext[i]) == 0) {
			return FILE_TYPE_SOUND;
		}
		if (strcasecmp("XHTML", &file_ext[i]) == 0) {
			return FILE_TYPE_HTML;
		}
		if (strcasecmp("XML", &file_ext[i]) == 0) {
			return FILE_TYPE_HTML;
		}
		break;
	case '3':
		if (strcasecmp("3GP", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("3GPP", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		if (strcasecmp("3G2", &file_ext[i]) == 0) {
			return FILE_TYPE_VIDEO;
		}
		break;
	}

	return FILE_TYPE_ETC;
}

/*********************
**Function name:	mf_get_category
**Parameter:
**	const char *filepath:	file fullpath
**	fsFileType *category:	output parameter of category
**Return value:
**	error code
**
**Action:
**	Get file category by file full path
**
*********************/
int mf_file_attr_get_file_category(const char *filepath, fsFileType * category)
{
	int i = 0;
	int flag = 0;

	if (mf_file_attr_is_dir(filepath)) {
		*category = FILE_TYPE_DIR;
		return MYFILE_ERR_NONE;
	}

	const char *filename = NULL;
	filename = mf_file_get(filepath);
	if (filename == NULL) {
		*category = FILE_TYPE_NONE;
		return MYFILE_ERR_SRC_ARG_INVALID;
	}
	char *file_ext = NULL;
	/*ToDo: error file name like the last letter is "." */
	for (i = strlen(filename); i >= 0; i--) {
		if (filename[i] == '.') {
			file_ext = g_strdup(&filename[i + 1]);
			flag = 1;
			break;
		}

		if (filename[i] == '/') {
			flag = 0;
			break;
		}
	}

	if (flag == 1) {
		*category = __mf_file_attr_get_category_by_file_ext(file_ext, filepath);
		SAFE_FREE_CHAR(file_ext);
		return MYFILE_ERR_NONE;
	} else {
		*category = FILE_TYPE_NONE;
		SAFE_FREE_CHAR(file_ext);
		return MYFILE_ERR_GET_CATEGORY_FAIL;
	}
}

/*********************
**Function name:	mf_file_attr_get_file_stat
**Parameter:
**	const char *filename:	file name
**	fsNodeInfo **node:		output parameter of what we need to refine
**Return value:
**	error code
**
**Action:
**	Get file size and last modified date by file path
**
*********************/
int mf_file_attr_get_file_mdate(const char *filename, i18n_udate *date)
{

	mf_retvm_if(filename == NULL, MYFILE_ERR_INVALID_ARG, "filename is null");
	struct stat statbuf;
	if (stat(filename, &statbuf) == -1) {
		return MYFILE_ERR_GET_STAT_FAIL;
	}
	time_t tempdate = statbuf.st_mtime;
	*date = (i18n_udate) tempdate * MF_UDATE_NUM;
	return MYFILE_ERR_NONE;

}

int mf_file_attr_get_file_size(const char *filename, off_t *size)
{

	mf_retvm_if(filename == NULL, MYFILE_ERR_INVALID_ARG, "filename is null");
	struct stat statbuf;
	if (stat(filename, &statbuf) == -1) {
		return MYFILE_ERR_GET_STAT_FAIL;
	}
	*size = statbuf.st_size;
	return MYFILE_ERR_NONE;

}

int mf_file_attr_get_file_stat(const char *filename, fsNodeInfo **node)
{
	struct stat statbuf;

	mf_retvm_if(filename == NULL, MYFILE_ERR_INVALID_ARG, "filename is null");
	mf_retvm_if(node == NULL, MYFILE_ERR_INVALID_ARG, "node is null");

	if (stat(filename, &statbuf) == -1) {
		return MYFILE_ERR_GET_STAT_FAIL;
	}
	time_t tempdate = statbuf.st_mtime;
	(*node)->size = statbuf.st_size;
	(*node)->date = (i18n_udate) tempdate * MF_UDATE_NUM;

	return MYFILE_ERR_NONE;
}

/*********************
**Function name:	mf_file_attr_is_dir
**Parameter:
**	const char *filename:	file fullpath
**Return value:
**	if path is a directory, return 1
**	else, return 0
**
**Action:
**	check if the file path is Directory
**
*********************/
int mf_file_attr_is_dir(const char *filepath)
{
	mf_retvm_if(filepath == NULL, 0, "filepath is null");

	return mf_is_dir(filepath);
}

/*********************
**Function name:	mf_file_attr_get_store_type_by_full
**Parameter:
**	const char *filepath:	file full path
**	MF_STORAGE *store_type:		output parameter of storage type
**Return value:
**	error code
**
**Action:
**	Get file storage type by file path
**
*********************/
int mf_file_attr_get_store_type_by_full(const char *filepath, MF_STORAGE * store_type)
{
	if (filepath == NULL || store_type == NULL) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	if (strncmp(filepath, PHONE_FOLDER, strlen(PHONE_FOLDER)) == 0) {
		*store_type = MYFILE_PHONE;
		return MYFILE_ERR_NONE;
	} else if (strncmp(filepath, MEMORY_FOLDER, strlen(MEMORY_FOLDER)) == 0) {
		*store_type = MYFILE_MMC;
		return MYFILE_ERR_NONE;
	} else {
		*store_type = MYFILE_NONE;
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
}


/*********************
**Function name:	mf_file_attr_get_file_ext
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
int mf_file_attr_get_file_ext(const char *filepath, char **file_ext)
{
	assert(filepath);
	assert(file_ext);
	const char *filename = NULL;
	filename = mf_file_get(filepath);

	if (filename == NULL) {
		return MYFILE_ERR_INVALID_FILE_NAME;
	}

	char *pdot = strrchr(filename, '.');

	if (!pdot) {
		return MYFILE_ERR_EXT_GET_ERROR;
	} else if (pdot != filepath) {
		*file_ext = g_strdup(pdot + 1);
		return MYFILE_ERR_NONE;
	} else {
		return MYFILE_ERR_EXT_GET_ERROR;
	}
}

/*********************
**Function name:	mf_file_attr_is_duplicated_name
**Parameter:
**	const char *dir:	dir which we need to check
**	const char *name:	the file/dir name we need to check
**
**Return value:
**	-23 if the name is already existed
**	0	if the name is not existed
**
**Action:
**	check if the name is existed in the specified dir
**
*********************/
int mf_file_attr_is_duplicated_name(const char *dir, const char *name)
{

	char *file_path = g_strconcat(dir, "/", name, NULL);
	if (file_path != NULL && mf_file_exists(file_path)) {
		SAFE_FREE_CHAR(file_path);
		return MYFILE_ERR_DUPLICATED_NAME;
	} else {
		SAFE_FREE_CHAR(file_path);
		return MYFILE_ERR_NONE;
	}
}

/*********************
**Function name:	mf_file_attr_is_valid_name
**Parameter:
**	const char *filename:	the file/dir name we need to check
**
**Return value:
**	-0x14	if the name is invalid
**	0		if the name is valid
**
**Action:
**	check if the name is valid by file name
**
*********************/
int mf_file_attr_is_valid_name(const char *filename)
{
	char *pattern;
	int ret, z, cflags = 0;
	char ebuf[128];
	regex_t reg;
	regmatch_t pm[1];
	const size_t nmatch = 1;
	/*ToDo: ignore the file star with . */
#ifndef MYFILE_HIDEN_FILES_SHOW
	if (strncmp(filename, ".", 1) == 0) {
		return MYFILE_ERR_INVALID_FILE_NAME;
	}
#endif

	pattern = MYFILE_NAME_PATTERN;
	z = regcomp(&reg, pattern, cflags);

	if (z != 0) {
		regerror(z, &reg, ebuf, sizeof(ebuf));
		fprintf(stderr, "%s: pattern '%s' \n", ebuf, pattern);
		return MYFILE_ERR_INVALID_FILE_NAME;
	}

	z = regexec(&reg, filename, nmatch, pm, 0);
	if (z == REG_NOMATCH) {
		ret = MYFILE_ERR_NONE;
	} else {
		ret = MYFILE_ERR_INVALID_FILE_NAME;
	}
	regfree(&reg);
	return ret;
}


/*********************
**Function name:	mf_file_attr_is_right_dir_path
**Parameter:
**	const char *filename:	the file/dir name we need to check
**
**Return value:
**	error code
**
**Action:
**	check if the dir path is correct
**
*********************/
int mf_file_attr_is_right_dir_path(const char *dir_path)
{
	int result = MYFILE_ERR_NONE;
	int length = 0;

	length = strlen(dir_path);
	if (length == 0) {
		return MYFILE_ERR_INVALID_DIR_PATH;
	}

	if (dir_path[length - 1] == '/' && length > 1) {
		return MYFILE_ERR_INVALID_DIR_PATH;
	}

	if (dir_path[0] != '/') {
		return MYFILE_ERR_INVALID_DIR_PATH;
	}

	const char *file_name = NULL;
	file_name = mf_file_get(dir_path);
	result = mf_file_attr_is_valid_name(file_name);

	if (result != MYFILE_ERR_NONE) {
		mf_error("Is NOT Valid dir path name");
	}

	return result;
}


/*********************
**Function name:	mf_file_attr_is_right_file_path
**Parameter:
**	const char *filename:	the file/dir name we need to check
**
**Return value:
**	error code
**
**Action:
**	check if the file path is correct
**
*********************/
int mf_file_attr_is_right_file_path(const char *file_path)
{
	int result = MYFILE_ERR_NONE;

	if (strlen(file_path) == 0) {
		return MYFILE_ERR_INVALID_FILE_PATH;
	}

	if (file_path[0] != '/') {
		return MYFILE_ERR_INVALID_DIR_PATH;
	}

	const char *file_name = NULL;
	file_name = mf_file_get(file_path);
	result = mf_file_attr_is_valid_name(file_name);
	if (result != MYFILE_ERR_NONE) {
		mf_error("Is NOT Valid dir path name");
	}

	return result;
}

/*********************
**Function name:	mf_file_attr_is_right_dir_path
**Parameter:
**	const char *filename:	the file/dir name we need to check
**
**Return value:
**	error code
**
**Action:
**	check if the dir path is correct
**
*********************/
int mf_file_attr_get_parent_path(const char *path, char **parent_path)
{
	assert(path);
	assert(parent_path);
	SECURE_DEBUG("Path :::: [%s]", path);

	*parent_path = g_strdup(path);
	if (*parent_path == NULL) {
		return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
	}

	const char *name = NULL;
	name = mf_file_get(path);
	/*
	**	input path and parent_path are check in the caller.
	**	parent_path is full path must be like /opt/media/file.ext
	**	name is file.ext
	**	strlen(parent_path) should large than strlen(name) normally.
	**	to take exception like input path is "", we add a if condition
	*/
	if (strlen(*parent_path) > strlen(name)) {
		(*parent_path)[strlen(*parent_path) - strlen(name) - 1] = '\0';
	}

	if (strlen(*parent_path) == 0) {
		free(*parent_path);
		*parent_path = g_strdup("/");
	}

	return MYFILE_ERR_NONE;
}

/*********************
**Function name:	mf_file_attr_get_logical_path_by_full
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
int mf_file_attr_get_logical_path_by_full(const char *full_path, char **path)
{
	assert(full_path);
	assert(path);
	MF_STORAGE store_type = 0;
	int root_len = 0;

	int err = mf_file_attr_get_store_type_by_full(full_path, &store_type);
	if (err != MYFILE_ERR_NONE) { // something is wrong.
		mf_error("get store type by path %s failed.", full_path);
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}

	*path = g_strdup(full_path);
	if (*path == NULL) {
		return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
	}

	memset(*path, 0, strlen(*path));
	switch (store_type) {
	case MYFILE_PHONE:
		root_len = strlen(PHONE_FOLDER);
		break;
	case MYFILE_MMC:
		root_len = strlen(MEMORY_FOLDER);
		break;
	default:
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}

	/*
	**	*path has the same length with full_path
	**	strlen(*path) is 0 since the memset called
	**	we use length of full_path to reprecent the *path's
	*/
	g_strlcpy(*path, full_path + root_len, strlen(full_path));
	if (strlen(*path) == 0) {
		SAFE_FREE_CHAR(*path);
		*path = g_strdup("/");
	}

	return MYFILE_ERR_NONE;
}


typedef struct __mf_transfer_data_s mf_transfer_data_s;

struct __mf_transfer_data_s {
	const char *file_path;
	char *thumb_path;
	media_info_h *media;
};

static bool __mf_local_data_get_media_thumbnail_cb(media_info_h media, void *data)
{
	mf_retvm_if(data == NULL, -1, "filter is NULL");
	mf_transfer_data_s *tmp_data = (mf_transfer_data_s *)data;


	media_info_clone(tmp_data->media, media);
	int retcode = media_info_get_thumbnail_path(media, &(tmp_data->thumb_path));
	if (retcode != MEDIA_CONTENT_ERROR_NONE) {
		mf_debug("Get media thumbnail path failed!![%d]", retcode);
	}

	return false;
}

int mf_file_attr_get_file_icon(const char *file_path, int *error_code, int view_type, char **thumbnail, media_info_h *media_info)
{
	int index = 0;
	char *icon_path = g_strdup(DEFAULT_ICON);
	fsFileType ftype = FILE_TYPE_NONE;
	fsFileType ftype_by_mime = FILE_TYPE_NONE;
	char *mime = NULL;
	int thumbnail_type = MF_THUMBNAIL_TYPE_DEFAULT;
	int retcode = -1;
	char *ext = NULL;
	mf_retvm_if(file_path == NULL, thumbnail_type, "file_path is NULL");

	int ret = mf_file_attr_get_file_category(file_path, &ftype);
	ftype_by_mime = ftype;
	if (ret != MYFILE_ERR_NONE || ftype == FILE_TYPE_NONE || ftype == FILE_TYPE_ETC) {
		int err_code = mf_file_attr_get_file_ext(file_path, &ext);
		if (err_code != MYFILE_ERR_NONE || ext == NULL) {
			mf_warning("Fail to get file extension");
			return thumbnail_type;
		}
		retcode = mime_type_get_mime_type(ext, &mime);
		if ((mime == NULL) || (retcode != MIME_TYPE_ERROR_NONE)) {
			mf_warning("Fail to get mime type, set etc icon");
			SAFE_FREE_CHAR(ext);
			return thumbnail_type;
		}

		mf_error("mime is [%s]", mime);
		for (index = 0; mime_type[index].mime; index++) {
			if (strncmp(mime, mime_type[index].mime, strlen(mime)) == 0) {
				ftype_by_mime = mime_type[index].ftype;
				break;
			}
		}
	}

	ftype = ftype_by_mime;

	SAFE_FREE_CHAR(icon_path);
	SAFE_FREE_CHAR(ext);

	switch (ftype) {
	case FILE_TYPE_IMAGE:
	case FILE_TYPE_VIDEO:
	case FILE_TYPE_MUSIC:
	case FILE_TYPE_SOUND: {

		int err = 0;
		mf_transfer_data_s tmp_data;
		memset(&tmp_data, 0x00, sizeof(mf_transfer_data_s));
		tmp_data.file_path = file_path;
		tmp_data.media = media_info;
		//err = mf_file_attr_get_thumbnail(&tmp_data);
		char *condition = NULL;
		condition = g_strdup_printf("%s and MEDIA_PATH=\"%s\"", MF_CONDITION_IMAGE_VIDEO, tmp_data.file_path);
		err = mf_media_content_data_get(&tmp_data, condition, __mf_local_data_get_media_thumbnail_cb);
		if (err == 0) {
			if (tmp_data.thumb_path && mf_file_exists(tmp_data.thumb_path)) {
				SAFE_FREE_CHAR(icon_path);
				icon_path = g_strdup(tmp_data.thumb_path);
				thumbnail_type = MF_THUMBNAIL_TYPE_THUMBNAIL;
			} else {
				icon_path = g_strdup(mf_file_attr_get_default_icon_by_type(ftype));
				thumbnail_type = MF_THUMBNAIL_TYPE_DEFAULT;
				*error_code = 1;
			}
		} else {
			if (error_code) {
				*error_code = err;
			}

			icon_path = g_strdup(mf_file_attr_get_default_icon_by_type(ftype));
			thumbnail_type = MF_THUMBNAIL_TYPE_DEFAULT;
		}
		SAFE_FREE_CHAR(tmp_data.thumb_path);
	}
	break;
	default:
		icon_path = g_strdup(mf_file_attr_get_default_icon_by_type(ftype));
		thumbnail_type = MF_THUMBNAIL_TYPE_DEFAULT;
		break;
	}

	*thumbnail = icon_path;
	SAFE_FREE_CHAR(mime);
	return thumbnail_type;
}

fsFileType mf_file_attr_get_file_type(const char *mime)
{
	int index;
	fsFileType ftype = FILE_TYPE_NONE;
	for (index = 0; mime_type[index].mime; index++) {
		if (strncmp(mime, mime_type[index].mime, strlen(mime)) == 0) {
			ftype = mime_type[index].ftype;
		}
	}
	return ftype;

}

fsFileType mf_file_attr_get_file_type_by_mime(const char *file_path)
{
	int index;
	fsFileType ftype = FILE_TYPE_NONE;
	char *mime = NULL;
	int retcode = -1;

	if (!file_path) {
		return ftype;
	}
	char *ext = NULL;
	int error_code = mf_file_attr_get_file_ext(file_path, &ext);
	if (error_code != MYFILE_ERR_NONE || ext == NULL) {
		mf_warning("Fail to get file extension");
		return ftype;
	}

	retcode = mime_type_get_mime_type(ext, &mime);
	if ((mime == NULL) || (retcode != MIME_TYPE_ERROR_NONE)) {
		mf_warning("Fail to get mime type, set etc icon");
		SAFE_FREE_CHAR(ext);
		return ftype;
	}

	for (index = 0; mime_type[index].mime; index++) {
		if (strncmp(mime, mime_type[index].mime, strlen(mime)) == 0) {
			ftype = mime_type[index].ftype;
			SAFE_FREE_CHAR(mime);
			SAFE_FREE_CHAR(ext);
			return ftype;
		}
	}

	mf_warning("No matched mimetype(%s) for file: %s", mime, file_path);
	SAFE_FREE_CHAR(mime);
	SAFE_FREE_CHAR(ext);
	return ftype;
}

const char *mf_file_attr_get_default_icon_by_type(fsFileType ftype)
{
	const char *icon_path = DEFAULT_ICON;

	if (icon_array[ftype]) {
		icon_path = icon_array[ftype];
	} else {
		icon_path = DEFAULT_ICON;
	}

	return icon_path;
}

/*********************
**Function name:	mf_file_attr_get_path_level
**Parameter:
**	const char *file_fullpath:	the full path
**	int* level:					level which the specified path under(this is output parameter)
**
**Return value:
**	error code
**
**Action:
**	get level of specified path under
**
*********************/
int mf_file_attr_get_path_level(const char *fullpath, int *level)
{
	if (fullpath == NULL) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	if (mf_file_attr_is_right_dir_path(fullpath) != 0) {
		return MYFILE_ERR_INVALID_PATH;
	}

	MF_STORAGE storage_t = 0;
	int start_level = 0;
	int error_code = mf_file_attr_get_store_type_by_full(fullpath, &storage_t);
	if (error_code != 0) {
		return error_code;
	}

	if (storage_t == MYFILE_PHONE) {
		start_level = MF_PHONE_DEFAULT_LEVEL;
	} else if (storage_t == MYFILE_MMC) {
		start_level = MF_MMC_DEFAULT_LEVEL;
	}

	char *temp = strdup(fullpath);
	if (temp == NULL) {
		return MYFILE_ERR_UNKNOWN_ERROR;
	}
	int count = 0;

	gchar **result = NULL;
	gchar **params = NULL;
	result = g_strsplit(temp, "/", 0);
	if (result == NULL) {
		free(temp);
		temp = NULL;
		return MYFILE_ERR_UNKNOWN_ERROR;
	}
	for (params = result; *params; params++) {
		mf_debug("*params is [%s]", *params);
		count++;
	}

	g_strfreev(result);
	*level = count - start_level - 1;
	mf_debug("cout is [%d] start level is [%d]", count, start_level);
	free(temp);
	return MYFILE_ERR_NONE;

}

/*********************
**Function name:	mf_file_attr_is_in_system_folder
**Parameter:
**	const char *file_fullpath:	the full path
**	int* level:					level which the specified path under
**	bool* result:				output parameter for the result
**
**Return value:
**	error code
**
**Action:
**	check if the specified path is under system folder
**
*********************/
int mf_file_attr_is_in_system_folder(char *fullpath, int level, bool * result)
{
	if (fullpath == NULL) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	MF_STORAGE storage_t = 0;
	int error_code = mf_file_attr_get_store_type_by_full(fullpath, &storage_t);
	if (error_code != 0) {
		return error_code;
	}

	const char *name = NULL;
	name = mf_file_get(fullpath);
	char *parent_path = NULL;
	error_code = mf_file_attr_get_parent_path(fullpath, &parent_path);

	if (error_code != 0) {
		return error_code;
	}

	if (storage_t == MYFILE_PHONE || storage_t == MYFILE_MMC) {
		if (level == 1) {
			if ((strlen(name) == strlen(DEFAULT_FOLDER_CAMERA_SHOTS)) && strcmp(name, DEFAULT_FOLDER_CAMERA_SHOTS) == 0) {
				*result = true;
			} else if ((strlen(name) == strlen(DEFAULT_FOLDER_IMAGE)) && strcmp(name, DEFAULT_FOLDER_IMAGE) == 0) {
				*result = true;
			} else if ((strlen(name) == strlen(DEFAULT_FOLDER_VIDEO)) && strcmp(name, DEFAULT_FOLDER_VIDEO) == 0) {
				*result = true;
			} else if ((strlen(name) == strlen(DEFAULT_FOLDER_MUSIC)) && strcmp(name, DEFAULT_FOLDER_MUSIC) == 0) {
				*result = true;
			} else if ((strlen(name) == strlen(DEFAULT_FOLDER_DOWNLOADS)) && strcmp(name, DEFAULT_FOLDER_DOWNLOADS) == 0) {
				*result = true;
			} else {
				if (storage_t == MYFILE_PHONE) {
					if ((strlen(name) == strlen(DEFAULT_FOLDER_ALERTS_AND_RINGTONES)) && strcmp(name, DEFAULT_FOLDER_ALERTS_AND_RINGTONES) == 0) {
						*result = true;
					} else if ((strlen(name) == strlen(DEFAULT_FOLDER_BOOKMARK)) && strcmp(name, DEFAULT_FOLDER_BOOKMARK) == 0) {
						*result = true;
					} else if ((strlen(name) == strlen(DEFAULT_FOLDER_RSS)) && strcmp(name, DEFAULT_FOLDER_RSS) == 0) {
						*result = true;
					} else {
						*result = false;
					}
				} else {
					*result = false;
				}
			}

		} else if (level == 2) {
			const char *parent_name = NULL;
			parent_name = mf_file_get(parent_path);
			if (storage_t == MYFILE_PHONE) {
				if (!g_strcmp0(parent_name, DEFAULT_FOLDER_IMAGE) && !g_strcmp0(name, SUB_FODER_WALLPAPER)) {
					*result = true;
				} else if (!g_strcmp0(parent_name, DEFAULT_FOLDER_ALERTS_AND_RINGTONES)
				           && (!g_strcmp0(name, SUB_FODER_ALERTS) || !g_strcmp0(name, SUB_FODER_RINGTONES))) {
					*result = true;
				} else if (!g_strcmp0(parent_name, DEFAULT_FOLDER_MUSIC)
				           && (!g_strcmp0(name, SUB_FODER_FM) || !g_strcmp0(name, SUB_FODER_VOICE_RECORD))) {
					*result = true;
				} else {
					*result = false;
				}
			} else {
				*result = false;
			}
		} else {
			if (parent_path) {
				free(parent_path);
				parent_path = NULL;
			}
			return MYFILE_ERR_STORAGE_TYPE_ERROR;
		}
	}

	else {
		if (parent_path) {
			free(parent_path);
			parent_path = NULL;
		}
		*result = false;
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
	if (parent_path) {
		free(parent_path);
		parent_path = NULL;
	}
	return MYFILE_ERR_NONE;
}

/*********************
**Function name:	mf_file_attr_is_system_dir
**Parameter:
**	const char *file_fullpath:	the full path
**	bool* result:				output parameter for the result
**
**Return value:
**	error code
**
**Action:
**	check if the specified path is system folder
**
*********************/
int mf_file_attr_is_system_dir(char *fullpath, bool * result)
{
	if (fullpath == NULL) {
		mf_error("source argument invalid");
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	if (mf_file_attr_is_dir(fullpath) == 0) {
		mf_error("source is not exist");
		return MYFILE_ERR_SRC_NOT_EXIST;
	}

	int level = 0;
	int error_code = 0;

	error_code = mf_file_attr_get_path_level(fullpath, &level);
	if (error_code != 0) {
		mf_error("Fail to get path level");
		return error_code;
	}

	if (level >= 3 || level <= 0) {
		*result = false;
		mf_error("Path Level is wrong");
		return MYFILE_ERR_NONE;
	}
	error_code = mf_file_attr_is_in_system_folder(fullpath, level, result);

	if (error_code != 0) {
		mf_error("Fail .. is in system folder err :: %d", error_code);
		return error_code;
	}

	return MYFILE_ERR_NONE;
}

void mf_file_attr_get_file_size_info(char **file_size, off_t src_size)
{
	MF_TRACE_BEGIN;
	mf_retm_if(file_size == NULL, "file_size is NULL");
	unsigned long long original_size = 0;
	double size = 0;
	int index = 0;
	int len = 0;

	original_size = src_size;
	size = (double)original_size;

	while (size >= MYFILE_BASIC_SIZE) {
		size /= MYFILE_BASIC_SIZE;
		index++;
	}

	if (index == SIZE_BYTE) {
		snprintf(NULL, 0, "%llu B%n", original_size, &len);
	} else {
		size = MF_ROUND_D(size, 1);
		snprintf(NULL, 0, "%0.1lf XB%n", size, &len);
	}

	if (len <= 0) {
		*file_size = NULL;
		return;
	}
	len += 1;
	*file_size = (char *)calloc(len, sizeof(char));
	if (*file_size == NULL) {
		return;

	}

	if (index == SIZE_BYTE) {
		snprintf(*file_size, len, "%llu B", original_size);
	} else {
		if (index == SIZE_KB) {
			snprintf(*file_size, len, "%0.1lf KB", size);
		} else if (index == SIZE_MB) {
			snprintf(*file_size, len, "%0.1lf MB", size);
		} else if (index == SIZE_GB) {
			snprintf(*file_size, len, "%0.1lf GB", size);
		} else {
			free(*file_size);
			*file_size = NULL;
		}
	}
	MF_TRACE_END;
	return;
}


int mf_file_attr_media_has_video(const char *filename)
{
	MF_TRACE_BEGIN;
	int ret = 0;
	metadata_extractor_h handle = NULL;
	char *value = NULL;

	if (!filename) {
		goto CATCH_ERROR;
	}
	SECURE_DEBUG("filename is [%s]", filename);
	ret = metadata_extractor_create(&handle);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		mf_error("metadata_extractor_create().. %d", ret);
		goto CATCH_ERROR;
	}

	ret = metadata_extractor_set_path(handle, filename);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		mf_error("metadata_extractor_set_path().. %d", ret);
		goto CATCH_ERROR;
	}

	ret = metadata_extractor_get_metadata(handle, METADATA_HAS_VIDEO, &value);
	if (ret == METADATA_EXTRACTOR_ERROR_NONE && value) {
		if (g_strcmp0(value, "1") == 0) {
			mf_error("ret is [%d] value is [%s]", ret, "1");
			if (handle) {
				metadata_extractor_destroy(handle);
			}
			MF_TRACE_END;
			SAFE_FREE_CHAR(value);
			return 1;
		}
		SAFE_FREE_CHAR(value);
	}
	mf_error("ret is [%d] value is [%s]", ret, value);
	SAFE_FREE_CHAR(value);

	if (handle) {
		metadata_extractor_destroy(handle);
	}

	MF_TRACE_END;
	return 0;
CATCH_ERROR:
	if (handle) {
		metadata_extractor_destroy(handle);
	}

	MF_TRACE_END;
	return 0;
}

