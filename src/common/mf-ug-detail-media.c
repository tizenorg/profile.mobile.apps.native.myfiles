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


#include <glib.h>
#include <system_settings.h>
#include <metadata_extractor.h>
#include <mime_type.h>
#include <utils_i18n.h>

#include "mf-ug-detail-media.h"
#include "mf-file-util.h"
#include "mf-util.h"

#define UG_DATE_FORMAT_DD_MM_YYYY			"%d-%b-%Y "
#define UG_DATE_FORMAT_MM_DD_YYYY			"%b-%d-%Y "
#define UG_DATE_FORMAT_YYYY_MM_DD			"%Y-%b-%d "
#define UG_DATE_FORMAT_YYYY_DD_MM			"%Y-%d-%b "
#define UG_TIME_FORMAT_12HOUR				"%l:%M%p"
#define UG_TIME_FORMAT_24HOUR				"%H:%M"
#define UG_ICU_ARR_LENGTH				128
#define UG_EXIF_ARR_LENGTH				255
#define UG_DATE_FORMAT_12				"yMMMdhms"
#define UG_DATE_FORMAT_24				"yMMMdHms"

struct _ftype_by_mime {
	const char *mime;
	const char *ext;
};

static struct _ftype_by_mime mime_type[] = {
	{"image/png", "png"},
	{"image/jpeg", "jpg"},
	{"image/gif", "gif"},
	{"image/bmp", "bmp"},
	{"image/vnd.wap.wbmp", "wbmp"},

	/*FILE_TYPE_VIDEO */
	{"video/mp4", "mp4"},
	{"video/3gpp", "3gp"},
	{"video/x-ms-asf", "asf"},
	{"video/x-ms-wmv", "wmv"},

	/*FILE_TYPE_MUSIC */
	{"audio/mpeg", "mpeg"},
	{"audio/x-wav", "wav"},
	{"application/x-smaf", "smaf"},
	{"audio/mxmf", "mxmf"},
	{"audio/midi", "midi"},
	{"audio/x-xmf", "xmf"},
	{"audio/x-ms-wma", "wma"},
	{"audio/aac", "aac"},
	{"audio/ac3", "ac"},
	{"audio/ogg", "ogg"},
	{"audio/mid", "mid"},
	{"audio/m4a", "m4a"},
	{"audio/mmf", "mmf"},
	{"audio/mp3", "mp3"},
	{"audio/mp4", "mp4"},
	{"audio/mpeg3", "mpeg3"},
	{"audio/mpeg4", "mpeg4"},
	{"audio/mpg", "mpg"},
	{"audio/mpg3", "mpg3"},
	{"audio/wav", "wav"},
	{"audio/wave", "wave"},
	{"audio/wma", "wma"},
	{"audio/x-mp3", "mp3"},
	{"audio/-mpeg", "mpeg"},
	{"audio/x-mpeg", "mpeg"},
	{"audio/x-mpegaudio", "mpeg"},
	{"audio/x-mpg", "mpg"},
	{"audio/x-ms-asf", "asf"},
	{"audio/x-wave", "wave"},
	{"audio/x-flac", "flac"},
	{"text/x-iMelody", "iMelody"},
	/*FILE_TYPE_PDF */
	{"application/pdf", "pdf"},

	/*FILE_TYPE_DOC */
	{"application/msword", "doc"},
	{"application/vnd.openxmlformats-officedocument.wordprocessingml.document", "doc"},

	/*FILE_TYPE_PPT */
	{"application/vnd.ms-powerpoint", "ppt"},
	{"application/vnd.openxmlformats-officedocument.presentationml.presentation", "ppt"},

	/*FILE_TYPE_EXCEL */
	{"application/vnd.ms-excel", "xlsx"},
	{"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "xlsx"},

	/*FILE_TYPE_VOICE */
	{"audio/AMR", "amr"},
	{"audio/AMR-WB", "amr"},
	{"audio/amr", "amr"},
	{"audio/amr-wb", "amr"},
	{"audio/x-amr", "amr"},

	/*FILE_TYPE_HTML */
	{"text/html", "html"},
	{"text/xml", "xml"},//Fixed P131114-04144

	/*FILE_TYPE_FLASH */
	{"application/x-shockwave-flash", "flash"},
	{"video/x-flv", "flv"},

	/*FILE_TYPE_TXT */
	{"text/plain", "txt"},

	{"application/vnd.tizen.package", "tpk"},
	/*FILE_TYPE_RSS */
	{"text/x-opml+xml", "rss"},

	/*FILE_TYPE_JAVA */

	{"text/directory", "vcs"},
	{"text/x-vcard", "vcs"},
	{"application/snb", "snb"},
	{"application/x-hwp", "hwp"},

	/*FILE_TYPE_ETC */
	{NULL, NULL},
};

#define SAFE_FREE(src)      { if (src) { free(src); src = NULL; } }

static void __mf_ug_detail_media_set_default_timezone_id()
{
        i18n_uchar utimezone_id [UG_ICU_ARR_LENGTH] = {0};
        char timezone_buffer[UG_ICU_ARR_LENGTH] = {0};
        char timezone_id[UG_ICU_ARR_LENGTH] = {0};
        char *buffer = NULL;
        int timezone_str_size;
        int retcode = -1;

        retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, &buffer);
        if (retcode != SYSTEM_SETTINGS_ERROR_NONE)
        {
                mf_error("[ERR] failed to get the timezone");
        }
        if (buffer)
                strncpy(timezone_id, buffer, sizeof(timezone_id)-1);
        timezone_str_size = readlink("/opt/etc/localtime", timezone_buffer, sizeof(timezone_buffer)-1);
        free(buffer);

        if (timezone_str_size > 0) {
                char *ptr, *sp, *zone= NULL, *city= NULL;
                ptr = strtok_r(timezone_buffer, "/", &sp);

                while ((ptr = strtok_r(NULL, "/", &sp)))
                {
                        zone = city;
                        city = ptr;
                }

                if (zone != NULL && city != NULL) {
                        if (strcmp("zoneinfo", zone) == 0)
                                snprintf(timezone_id, UG_ICU_ARR_LENGTH, "%s", city);
                        else
                                snprintf(timezone_id, UG_ICU_ARR_LENGTH, "%s/%s", zone, city);
                }
        }

        if (*timezone_id) {
                i18n_ustring_copy_ua_n(utimezone_id, timezone_id, sizeof(timezone_buffer)/2);
                retcode = i18n_ucalendar_set_default_timezone(utimezone_id);
                if (retcode != I18N_ERROR_NONE) {
                    myfile_dlog("Failed to set default time zone[%d]!", retcode);
                }
        }
}

static char *__mf_ug_detail_media_get_best_pattern(const char *locale, i18n_uchar *customSkeleton, i18n_udate date)
{
	int status = -1;
	i18n_udatepg_h generator = NULL;
	i18n_udate_format_h formatter = NULL;
	i18n_uchar bestPattern[UG_ICU_ARR_LENGTH] = {0,};
	i18n_uchar formatted[UG_ICU_ARR_LENGTH] = {0,};
	char formattedString[UG_ICU_ARR_LENGTH] = {0,};
	int32_t bestPatternLength, formattedLength;

        __mf_ug_detail_media_set_default_timezone_id();

	status = i18n_udatepg_create(locale, &generator);
	if ((status != I18N_ERROR_NONE) || (generator == NULL))
		return NULL;

	status = i18n_udatepg_get_best_pattern(generator, customSkeleton, i18n_ustring_get_length(customSkeleton), bestPattern, UG_ICU_ARR_LENGTH, &bestPatternLength);
	if (bestPatternLength <= 0) {
		i18n_udatepg_destroy(generator);
		return NULL;
	}

	status= i18n_udate_create(I18N_UDATE_MEDIUM, I18N_UDATE_MEDIUM, locale, NULL, -1, bestPattern, -1, &formatter);
	if ((status != I18N_ERROR_NONE) || (formatter == NULL)) {
		i18n_udatepg_destroy(generator);
		return NULL;
	}

	status = i18n_udate_format_date(formatter, date, formatted, UG_ICU_ARR_LENGTH, NULL, &formattedLength);
	if ((status != I18N_ERROR_NONE) || (formattedLength <= 0)) {
		i18n_udatepg_destroy(generator);
		i18n_udate_destroy(formatter);
		return NULL;
	}

	i18n_ustring_copy_au(formattedString, formatted);
	i18n_udatepg_destroy(generator);
	i18n_udate_destroy(formatter);

	if (strlen(formattedString) == 0)
		return NULL;

	return g_strdup(formattedString);
}

char *mf_ug_detail_media_get_icu_date(i18n_udate date)
{
	char *datestr = NULL;
	char *skeleton = NULL;
	i18n_uchar customSkeleton[UG_ICU_ARR_LENGTH] = { 0, };
	int skeletonLength = 0;
	bool timeformat = false;
        int retcode = -1;

        retcode = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, &timeformat);
        if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
                myfile_dlog("Failed to get timeformat info[%d]!", retcode);
        }

	if (timeformat == false) {
		skeleton = g_strdup(UG_DATE_FORMAT_12);
	} else {
		skeleton = g_strdup(UG_DATE_FORMAT_24);
	}

	if (skeleton == NULL) {
		return NULL;
	} else {
		skeletonLength = strlen(skeleton);
	}

	if (i18n_ustring_copy_ua_n(customSkeleton, skeleton, skeletonLength) == NULL) {
		if (skeleton) {
			free(skeleton);
			skeleton = NULL;
		}
		return NULL;
	}

	char *region = NULL;
	retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY, &region);
	if ((retcode != SYSTEM_SETTINGS_ERROR_NONE) || (region == NULL)) {
		myfile_dlog("Cannot get region format.");
		region = strdup("en_US");		// Default value.
	} else {
		char *find = strstr(region, "UTF-8");
		if (find) {
			int diff = find - region;
			if (diff > 0) {
				region[diff-1] = '\0';
			}
		}
	}

	datestr = __mf_ug_detail_media_get_best_pattern(region, customSkeleton, date);
	if (skeleton) {
		free(skeleton);
		skeleton = NULL;
	}

	return datestr;
}

static void __mf_ug_detail_media_get_file_path(void *data, char *path)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);
	struct detailData *ap = (struct detailData *)data;

	if (ap->mf_Info.filepath) {
		free(ap->mf_Info.filepath);
		ap->mf_Info.filepath = NULL;
	}
	if (path == NULL) {
		ap->mf_Info.filepath = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
	} else {
		ap->mf_Info.filepath = g_strdup(path);
	}
}


static void __mf_ug_detail_media_get_file_name(void *data, char *path)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);
	struct detailData *ap = (struct detailData *)data;

	if (ap->mf_Info.filename) {
		free(ap->mf_Info.filename);
		ap->mf_Info.filename = NULL;
	}

	if (path == NULL) {
		ap->mf_Info.filename = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
	} else {
		if (g_strcmp0(path, UG_PHONE_FOLDER) == 0) {
			ap->mf_Info.filename = g_strdup(MF_UG_DETAIL_LABEL_DEVICE_MEMORY);
		} else if (g_strcmp0(path, UG_MEMORY_FOLDER) == 0) {
			ap->mf_Info.filename = g_strdup(MF_UG_DETAIL_LABEL_SD_CARD);
		} else {
			ap->mf_Info.filename = g_strdup(mf_file_get(path));
		}
	}
}

static void __mf_ug_detail_media_get_file_type(void *data)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);
	struct detailData *ap = (struct detailData *)data;

	if (ap->mf_Info.category) {
		g_string_free(ap->mf_Info.category, true);
		ap->mf_Info.category = NULL;
	}

	if (ap->mf_Status.path == NULL) {
		ap->mf_Info.category = g_string_new(MF_UG_DETAIL_LABEL_UNKNOWN);
	} else {
		ap->mf_Info.category = mf_ug_detail_fs_parse_file_type(ap->mf_Status.path);
		if (ap->mf_Info.category == NULL) {
			ap->mf_Info.category = g_string_new(MF_UG_DETAIL_LABEL_UNKNOWN);
		}
	}
}

void mf_ug_detail_media_get_file_location(void *data, char *path)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);
	struct detailData *ap = (struct detailData *)data;

	char *logic_path = NULL;
	Mf_Storage is_in_mmc = 0;
	GString *parent_path = NULL;

	if (path == NULL) {
		ap->mf_Info.file_location = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
		return ;
	}

	if (ap->mf_Info.file_location) {
		free(ap->mf_Info.file_location);
		ap->mf_Info.file_location = NULL;
	}

	logic_path = (char *)malloc(UG_MYFILE_DIR_PATH_LEN_MAX + 1);
	if (logic_path == NULL)
		return;

	memset(logic_path, 0, UG_MYFILE_DIR_PATH_LEN_MAX + 1);

	mf_ug_detail_fs_get_store_type(path, &is_in_mmc);

	parent_path = mf_ug_detail_fs_get_parent(path);
	if (parent_path != NULL && parent_path->str != NULL) {
		mf_ug_detail_fs_get_logi_path(parent_path->str, logic_path);

		if (is_in_mmc == D_MYFILE_MMC) {
			ap->mf_Info.file_location = g_strconcat(MF_UG_DETAIL_LABEL_SD_CARD, logic_path, NULL);
		} else if (is_in_mmc == D_MYFILE_PHONE) {
			ap->mf_Info.file_location = g_strconcat(MF_UG_DETAIL_LABEL_DEVICE_MEMORY, logic_path, NULL);
		} else {
			ap->mf_Info.file_location = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
		}
	} else {
		ap->mf_Info.file_location = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
	}

	if (parent_path) {
		g_string_free(parent_path, true);
		parent_path = NULL;
	}
	if (logic_path) {
		free(logic_path);
		logic_path = NULL;
	}
}

static void __mf_ug_detail_media_get_file_contains(void *data, char *path)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);
	struct detailData *ap = (struct detailData *)data;
	char * buf = NULL;

	if (ap->mf_Info.contains) {
		free(ap->mf_Info.contains);
		ap->mf_Info.contains = NULL;
	}
	if (path == NULL || ap->mf_Status.path == NULL) {
		ap->mf_Info.contains = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
		return;
	}
	if (mf_ug_detail_fs_is_dir(path)) {
		int error_code = 0;
		Eina_List *file_list = NULL;
		Eina_List *dir_list = NULL;

		error_code = mf_ug_detail_fs_get_file_list(ap->mf_Status.path, &dir_list, &file_list);
		if (error_code == 0) {
			int dir_list_len =  mf_ug_detail_fs_get_list_len(dir_list);
			int file_list_len = mf_ug_detail_fs_get_list_len(file_list);

			ug_detail_debug("=================== file count [%d] folder count [%d]", file_list_len, dir_list_len);

			Eina_List *l = NULL;
			Node_Info *filenode = NULL;
			int count = 1;
			EINA_LIST_FOREACH(file_list, l, filenode) {
				if (filenode) {
					ug_detail_debug("[file %d [%s]]",count, filenode->name);
					count++;
				}
			}
			count = 1;
			EINA_LIST_FOREACH(dir_list, l, filenode) {
				ug_detail_debug("[folder %d [%s]]",count, filenode->name);
				count++;
			}
			buf = g_strdup_printf("%d %s %s %d %s", file_list_len, MF_UG_DETAIL_LABELL_FILES, ",", dir_list_len, MF_UG_DETAIL_LABELL_FOLDERS);

			ap->mf_Info.contains = buf;
		} else {
			ap->mf_Info.contains = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
		}
	} else {
		ap->mf_Info.contains = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
	}
}

static void __mf_ug_detail_media_get_file_size(void *data, Node_Info *pNode, char *path)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	ug_detail_retm_if(pNode == NULL, "pNode is NULL");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);
	struct detailData *ap = (struct detailData *)data;
	unsigned long long original_size = 0;
	double size = 0;
//	int index = 0;
	int count = 0;

	original_size = pNode->size;
	size = (double)original_size;

	if (ap->mf_Info.filesize) {
		free(ap->mf_Info.filesize);
		ap->mf_Info.filesize = NULL;
	}
	/** malloc for File_size */

	if (pNode != NULL)
		original_size = pNode->size;

	if (mf_ug_detail_fs_is_dir(path))
		original_size = mf_ug_detail_fs_get_folder_size(path);

	size = (double)original_size;

	while (size >= BASIC_SIZE) {
		size /= BASIC_SIZE;
		count++;
	}

	ap->mf_Info.dsize = size;
	ap->mf_Info.unit_num = count;

	myfile_dlog("\n dsize=%d", size);
	myfile_dlog("\n ap->mf_Info.unit_num=%d", ap->mf_Info.unit_num);
}

static void __mf_ug_detail_media_get_file_date(void *data, Node_Info *pNode)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);
	struct detailData *ap = (struct detailData *)data;

	if (ap->mf_Info.create_date) {
		free(ap->mf_Info.create_date);
		ap->mf_Info.create_date = NULL;
	}
	ap->mf_Info.create_date = (char *)malloc(UG_FILE_CREATE_DATE_MAX + 1);

	if (ap->mf_Info.create_date == NULL)
		return;

	memset(ap->mf_Info.create_date, 0, UG_FILE_CREATE_DATE_MAX + 1);

	if (pNode == NULL) {
		snprintf(ap->mf_Info.create_date, UG_FILE_CREATE_DATE_MAX, "%s", MF_UG_DETAIL_LABEL_UNKNOWN);
		return ;
	}

	ap->mf_Info.date = pNode->date;

	char *datestr = mf_ug_detail_media_get_icu_date(pNode->date);

	if (datestr == NULL) {
		snprintf(ap->mf_Info.create_date, UG_FILE_CREATE_DATE_MAX, "%s", MF_UG_DETAIL_LABEL_UNKNOWN);
	} else {
		snprintf(ap->mf_Info.create_date, UG_FILE_CREATE_DATE_MAX, "%s", datestr);
		free(datestr);
		datestr = NULL;
	}
}

char *mf_ug_detail_media_get_file_ext_by_mime(const char *file_path)
{
	int index;
	char *ext = NULL;
	char *extension = NULL;
	char *mime = NULL;
	int retcode = -1;
	int error_code = mf_file_attr_get_file_ext(file_path, &extension);
	if (error_code != MYFILE_ERR_NONE || extension == NULL) {
		mf_warning("Fail to get file extension");
		return ext;
	}
	retcode = mime_type_get_mime_type(extension, &mime);
	if ((mime == NULL) || (retcode != MIME_TYPE_ERROR_NONE)) {
		ug_detail_debug("Fail to mime type, set etc icon");
		SAFE_FREE_CHAR(extension);
		return ext;
	}

	for (index = 0; mime_type[index].mime; index++) {
		if (strncmp(mime, mime_type[index].mime, strlen(mime)) == 0) {
			if (mime_type[index].ext) {
				ext = g_strdup(mime_type[index].ext);
				SAFE_FREE_CHAR(extension);
				SAFE_FREE_CHAR(mime);
				return ext;
			}
		}
	}
	SAFE_FREE_CHAR(mime);
	SAFE_FREE_CHAR(extension);
	return ext;
}

void mf_ug_detail_media_get_file_ext(void *data, char *path)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	ug_detail_retm_if(path == NULL, "path is NULL");
	ug_detail_retm_if(strlen(path) == 0, "path lenth is 0");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);
	struct detailData *ap = (struct detailData *)data;

	if (ap->mf_Info.file_ext) {
		free(ap->mf_Info.file_ext);
		ap->mf_Info.file_ext = NULL;
	}
	//prevent issue fix
	/*if (path == NULL)
		ap->mf_Info.file_ext = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);*/

	if (!mf_ug_detail_fs_is_dir(path)) {

		ap->mf_Info.file_ext = (char *)malloc(UG_FILE_EXT_LEN_MAX);

		if (ap->mf_Info.file_ext == NULL)
			return;
		memset(ap->mf_Info.file_ext, 0, UG_FILE_EXT_LEN_MAX);

		int ret = mf_ug_detail_fs_get_file_ext(path, ap->mf_Info.file_ext);

		if (ret != UG_MYFILE_ERR_NONE) {
			free(ap->mf_Info.file_ext);
			ap->mf_Info.file_ext = NULL;
			ap->mf_Info.file_ext = mf_ug_detail_media_get_file_ext_by_mime(path);
			if (ap->mf_Info.file_ext == NULL) {
				ap->mf_Info.file_ext = g_strdup(MF_UG_DETAIL_LABEL_UNKNOWN);
			}
		}
	}
}

/******************************
** Prototype    : __mf_ug_detail_media_get_exif_gps_info
** Description  :
** Input          : const char *file_full_path
**                   double *gps_value
**                   int ifdtype
**                   long tagtype
** Output       : int
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2011/6/8
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static int __mf_ug_detail_media_get_exif_gps_info(const char *file_full_path, double *gps_value, int ifdtype, long tagtype)
{
	ExifData *ed = NULL;
	ExifEntry *entry = NULL;
	//ExifIfd ifd;
	ExifTag tag;
	char buf[UG_EXIF_ARR_LENGTH + 1] = { 0, };
	if (file_full_path == NULL)
		return -1;

	if (gps_value == NULL)
		return -1;

	/** get exifdata*/
	ed = exif_data_new_from_file(file_full_path);
	if (!ed)
		return -1;

	//ifd = ifdtype;
	tag = tagtype;
	/** get exifentry*/
	entry = exif_data_get_entry(ed, tag);
	if (entry) {
		if (tag == EXIF_TAG_GPS_LATITUDE || tag == EXIF_TAG_GPS_LONGITUDE) {
			/** get value of the entry*/
			if (exif_entry_get_value(entry, buf, UG_EXIF_ARR_LENGTH) == NULL) {
				exif_data_unref(ed);
				return -1;
			}

			buf[strlen(buf)] = '\0';
			double tmp_arr[3] = { 0.0, 0.0, 0.0 };
			int count = 0;
			gchar **split_result = NULL;
			gchar **split_temp = NULL;

			/** split the buf by , */
			split_result = g_strsplit(buf, ", ", 0);
			if (split_result == NULL) {
				exif_data_unref(ed);
				return -1;
			}

			for (split_temp = split_result; *split_temp; split_temp++) {
				if (count == 3) {
					exif_data_unref(ed);
					g_strfreev(split_result);
					split_result = NULL;
					return -1;
				}
				tmp_arr[count] = g_ascii_strtod(*split_temp, NULL);
				if (errno == ERANGE) {
					exif_data_unref(ed);
					g_strfreev(split_result);
					split_result = NULL;
					return -1;
				}
				count++;
			}
			g_strfreev(split_result);
			split_result = NULL;

			if (count != 3) {
				exif_data_unref(ed);
				g_strfreev(split_result);
				split_result = NULL;
				return -1;
			}
			/** exchange the value*/
			*gps_value = tmp_arr[0] + tmp_arr[1] / 60 + tmp_arr[2] / 3600;
		} else {
			/** free exifdata newed before*/
			exif_data_unref(ed);
			return -1;
		}
	} else {
		exif_data_unref(ed);
		return -1;
	}
	exif_data_unref(ed);
	return 0;
}

/******************************
** Prototype    : __mf_ug_detail_media_extract_image_meta_latitude
** Description  :
** Input        : char *file_full_path
**                double *lat
** Output       : bool
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2011/6/8
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static bool __mf_ug_detail_media_extract_image_meta_latitude(char *file_full_path, double *lat)
{
	double value = 0.0;
	if (file_full_path == NULL || lat == NULL)
		return false;

	/** check if the file exists*/
	int ret = access(file_full_path, 0);
	if (ret == -1)
		return false;

	/** get laitude value by path*/
	if (__mf_ug_detail_media_get_exif_gps_info(file_full_path, &value, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE) == 0) {
		*lat = value;
	} else {
		return false;
	}
	return true;
}

/******************************
** Prototype    : __mf_ug_detail_media_extract_image_meta_lontitude
** Description  :
** Input        : char *file_full_path
**                double *lon
** Output       : bool
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2011/6/8
**    Author       : Samsung
**    Modification : Created function
**
******************************/
static bool __mf_ug_detail_media_extract_image_meta_lontitude(char *file_full_path, double *lon)
{
	double value = 0.0;
	if (file_full_path == NULL || lon == NULL)
		return false;

	/** check if the file exists*/
	int ret = access(file_full_path, 0);
	if (ret == -1)
		return false;

	/** get lontitude by path*/
	if (__mf_ug_detail_media_get_exif_gps_info(file_full_path, &value, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE) == 0) {
		*lon = value;
	} else {
		return false;
	}
	return true;
}

/******************************
** Prototype    : mf_ug_detail_media_get_common_info
** Description  :
** Input        : void *data
**                char *path
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
void mf_ug_detail_media_get_common_info(void *data, char *path)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	myfile_dlog("*******************%s %d\n", __func__, __LINE__);

	Node_Info *pNode = (Node_Info *) malloc(sizeof(Node_Info));
	if (pNode == NULL)
		return;

	memset(pNode, 0, sizeof(Node_Info));
	mf_ug_detaill_fs_get_file_stat(path, &pNode);

	__mf_ug_detail_media_get_file_path(data, path);
	__mf_ug_detail_media_get_file_name(data, path);
	__mf_ug_detail_media_get_file_type(data);
	__mf_ug_detail_media_get_file_size(data, pNode, path);
	__mf_ug_detail_media_get_file_date(data, pNode);
	mf_ug_detail_media_get_file_ext(data, path);
	__mf_ug_detail_media_get_file_contains(data, path);
	mf_ug_detail_media_get_file_location(data, path);

	if (pNode) {
		free(pNode);
		pNode = NULL;
	}
}

/******************************
** Prototype    : mf_ug_detail_media_get_exif_info
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
void mf_ug_detail_media_get_exif_info(void *data)
{
	ug_detail_retm_if(data == NULL, "data is NULL");
	struct detailData *ap = (struct detailData *)data;
	/** preprocess the possible dirty data*/
	if (ap->mf_Info.longitude) {
		free(ap->mf_Info.longitude);
		ap->mf_Info.longitude = NULL;
	}
	/** preprocess the possible dirty data*/
	if (ap->mf_Info.latitude) {
		free(ap->mf_Info.latitude);
		ap->mf_Info.latitude = NULL;
	}

	ap->mf_Info.longitude = malloc(UG_EXIF_ARR_LENGTH);
	if (ap->mf_Info.longitude == NULL) {
		return;
	}

	memset(ap->mf_Info.longitude, 0, UG_EXIF_ARR_LENGTH);
	ap->mf_Info.latitude = malloc(UG_EXIF_ARR_LENGTH);
	if (ap->mf_Info.latitude == NULL) {
		if (ap->mf_Info.longitude) {
			free(ap->mf_Info.longitude);
			ap->mf_Info.longitude = NULL;
		}
		return;
	}

	memset(ap->mf_Info.latitude, 0, UG_EXIF_ARR_LENGTH);
	double longtitude = 0.0;
	double latitude = 0.0;
	bool latiret = false;
	bool lonret = false;

	if (ap->mf_Status.path != NULL && ap->mf_Status.path->str != NULL) {
		/** get latitude of the file*/
		latiret = __mf_ug_detail_media_extract_image_meta_latitude(ap->mf_Status.path->str, &latitude);
		lonret = __mf_ug_detail_media_extract_image_meta_lontitude(ap->mf_Status.path->str, &longtitude);
	}

	if (latiret == false) {
		snprintf(ap->mf_Info.latitude, UG_EXIF_ARR_LENGTH, "%s", MF_UG_DETAIL_LABEL_UNKNOWN);
	} else {
		snprintf(ap->mf_Info.latitude, UG_EXIF_ARR_LENGTH, "%.5f", latitude);
	}
	/** get lontitude of the file*/

	if (lonret == false) {
		snprintf(ap->mf_Info.longitude, UG_EXIF_ARR_LENGTH, "%s", MF_UG_DETAIL_LABEL_UNKNOWN);
	} else {
		snprintf(ap->mf_Info.longitude, UG_EXIF_ARR_LENGTH, "%.5f", longtitude);
	}

}

