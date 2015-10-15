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


#include <media_content.h>
#include "mf-media-data.h"
#include "mf-media-content.h"
#include "mf-tray-item.h"
#include "mf-dlog.h"
#include "mf-util.h"
#include "mf-file-util.h"

static int __mf_media_data_sort_by_priority(const void *d1, const void *d2, int sequence_type);
static int __mf_media_data_sort_by_name_cb_A2Z(const void *d1, const void *d2);
static int __mf_media_data_sort_by_date_cb_O2R(const void *d1, const void *d2);
static int __mf_media_data_sort_by_type_cb_A2Z(const void *d1, const void *d2);
static int __mf_media_data_sort_by_size_cb_S2L(const void *d1, const void *d2);
static int __mf_media_data_sort_by_name_cb_Z2A(const void *d1, const void *d2);
static int __mf_media_data_sort_by_date_cb_R2O(const void *d1, const void *d2);
static int __mf_media_data_sort_by_type_cb_Z2A(const void *d1, const void *d2);
static int __mf_media_data_sort_by_size_cb_L2S(const void *d1, const void *d2);

void mf_media_data_item_free(media_data_s **item_data)
{
	if (*item_data == NULL)
		return;
	
	SAFE_FREE_CHAR((*item_data)->fullpath);
	SAFE_FREE_CHAR((*item_data)->display_name);
	SAFE_FREE_CHAR((*item_data)->thumbnail_path);
	SAFE_FREE_CHAR((*item_data)->ext);
	SAFE_FREE_CHAR(*item_data);
	*item_data = NULL;
}

media_data_s *mf_media_data_get_by_media_handle(media_info_h media)
{
	mf_retvm_if(media == NULL, NULL, "media is NULL");

	media_data_s *item_data = NULL;

	item_data = calloc(1, sizeof(media_data_s));

	if (item_data == NULL) {
		mf_error("item_data is NULL, calloc failed");
		return NULL;
	}

	if (media_info_get_display_name(media, &(item_data->display_name)) != MEDIA_CONTENT_ERROR_NONE) {
		mf_debug("Get media display name failed!");
		goto MF_LOCAL_FAILED;
	}


	int ret = mf_file_attr_get_file_ext(item_data->display_name, &item_data->ext);
	if (ret != MYFILE_ERR_NONE) {
		item_data->ext = NULL;
	}

	if (media_info_get_file_path(media, &(item_data->fullpath)) != MEDIA_CONTENT_ERROR_NONE) {
		mf_debug("Get media file path failed!");
		goto MF_LOCAL_FAILED;
	}

	media_content_type_e content_type = MEDIA_CONTENT_TYPE_OTHERS;

	if (media_info_get_media_type(media, &content_type) != MEDIA_CONTENT_ERROR_NONE) {
		mf_debug("Get media type failed!");
		goto MF_LOCAL_FAILED;
	}


	if (content_type == MEDIA_CONTENT_TYPE_IMAGE) /* The device's internal storage */
		item_data->file_type = FILE_TYPE_IMAGE;
	else if (content_type == MEDIA_CONTENT_TYPE_VIDEO) /* The device's external storage */
		item_data->file_type = FILE_TYPE_VIDEO;
	else if (content_type == MEDIA_CONTENT_TYPE_SOUND)
		item_data->file_type = FILE_TYPE_SOUND;
	else if (content_type == MEDIA_CONTENT_TYPE_MUSIC) /* The cloud storage - 101 */
		item_data->file_type = FILE_TYPE_MUSIC;
	else if (content_type == MEDIA_CONTENT_TYPE_OTHERS) {
		fsFileType ftype = FILE_TYPE_NONE;
		int ret = mf_file_attr_get_file_category(item_data->fullpath, &ftype);
		if (ret != MYFILE_ERR_NONE || ftype == FILE_TYPE_NONE || ftype == FILE_TYPE_ETC) {
			ftype = mf_file_attr_get_file_type_by_mime(item_data->fullpath);
		}

		item_data->file_type = ftype;
	}
	else {
		mf_debug("content_type[%d]!", content_type);
	}



	switch (item_data->file_type) {
		case FILE_TYPE_IMAGE:
		case FILE_TYPE_VIDEO:
		case FILE_TYPE_MUSIC:
		case FILE_TYPE_SOUND:
		case FILE_TYPE_DOC:
		case FILE_TYPE_PDF:
		case FILE_TYPE_PPT:
		case FILE_TYPE_EXCEL:
		case FILE_TYPE_TXT:
		case FILE_TYPE_HWP:
		case FILE_TYPE_SPD:
		case FILE_TYPE_SNB:
			if (mf_file_exists(item_data->fullpath)) {//Fixed the P131126-04292, sometimes, if the file isn't existed
				//if (content_type != MEDIA_CONTENT_TYPE_SOUND && content_type != MEDIA_CONTENT_TYPE_MUSIC) {//For sound file, don't need to get thumbnail.
					int retcode = media_info_get_thumbnail_path(media, &(item_data->thumbnail_path));
					if (retcode != MEDIA_CONTENT_ERROR_NONE) {
						mf_debug("Get media thumbnail path failed!![%d]", retcode);
						goto MF_LOCAL_FAILED;
					}
				//}
				//mf_debug("thumb_url: %s", item_data->thumbnail_path);

				media_content_storage_e storage_type = 0;
				if (media_info_get_storage_type(media, &storage_type) != MEDIA_CONTENT_ERROR_NONE) {
					mf_debug("Get storage type failed!");
					goto MF_LOCAL_FAILED;
				}
				if (storage_type == MEDIA_CONTENT_STORAGE_INTERNAL) /* The device's internal storage */
					item_data->storage_type = MYFILE_PHONE;
				else if (storage_type == MEDIA_CONTENT_STORAGE_EXTERNAL) /* The device's external storage */
					item_data->storage_type = MYFILE_MMC;
				else
					mf_debug("Undefined mode[%d]!", storage_type);
				unsigned long long file_size = 0;
				media_info_get_size(media, &file_size);
				item_data->size = file_size;

				time_t added_time = {0};
				int ret = media_info_get_modified_time(media, &added_time);
				if (ret == MEDIA_CONTENT_ERROR_NONE)//Fix prevent problem
					item_data->create_date = added_time;

				return item_data;
			} else{
				mf_media_data_item_free(&item_data);
			}
			break;
		default:
			mf_media_data_item_free(&item_data);
			break;
       }
       return NULL;

MF_LOCAL_FAILED:
       mf_media_data_item_free(&item_data);
       return NULL;

}

bool mf_media_data_get_data_cb(media_info_h media, void *data)
{
	mf_retvm_if(data == NULL, -1, "filter is NULL");
	Eina_List **list = (Eina_List **)data;

	media_data_s *item_data = mf_media_data_get_by_media_handle(media);
	if (item_data) {
		*list = eina_list_append(*list, item_data);
	}
	return true;
}

void mf_media_data_list_free(Eina_List **list)
{
	if (*list == NULL) {
		return;
	}
	Eina_List *l = NULL;

	media_data_s *item_data = NULL;

	EINA_LIST_FOREACH(*list, l, item_data) {
		mf_media_data_item_free(&item_data);
	}
	eina_list_free(*list);
	*list = NULL;
}

void mf_media_category_item_get(const char *fullpath, int type, Eina_List **category_list)
{
	MF_TRACE_BEGIN;
	char *path_condition = NULL;
	char *condition = NULL;

	switch (type) {
	case MEDIA_CONTENT_TYPE_IMAGE:
		condition = g_strdup(MF_CONDITION_LOCAL_IMAGE);
		break;
	case MEDIA_CONTENT_TYPE_VIDEO:
		condition = g_strdup(MF_CONDITION_LOCAL_VIDEO);
		break;
	case MEDIA_CONTENT_TYPE_SOUND:
	case MEDIA_CONTENT_TYPE_MUSIC:
		condition = g_strdup(MF_CONDITION_LOCAL_SOUND);
		break;
	case MEDIA_CONTENT_TYPE_OTHERS:
		condition = g_strdup(MF_CONDITION_LOCAL_DOCUMENT);
		break;
	default:
		return;
	}
	path_condition = g_strdup_printf("%s and MEDIA_PATH=\"%s\"", condition, fullpath);
	mf_media_content_data_get(category_list, path_condition, mf_media_data_get_data_cb);

	//SAFE_FREE_CHAR(condition);
	//SAFE_FREE_CHAR(path_condition);
	MF_TRACE_END;
}

void mf_media_category_list_get(int category_type, Eina_List **category_list)
{
	char *condition = NULL;
	switch (category_type) {
	case mf_tray_item_category_image:
		condition = g_strdup(MF_CONDITION_LOCAL_IMAGE);
		break;
	case mf_tray_item_category_video:
		condition = g_strdup(MF_CONDITION_LOCAL_VIDEO);
		break;
	case mf_tray_item_category_sounds:
		condition = g_strdup(MF_CONDITION_LOCAL_SOUND);
		break;
	case mf_tray_item_category_document:
		condition = g_strdup(MF_CONDITION_LOCAL_DOCUMENT);
		break;
	}

	mf_media_content_data_get(category_list, condition, mf_media_data_get_data_cb);

}

void mf_media_data_printf(Eina_List *list)
{
	return;
#if 0
	if (list == NULL) {
		mf_debug("=============== list is NULL ");

	}
	Eina_List *l = NULL;

	media_data_s *item_data = NULL;

	EINA_LIST_FOREACH(list, l, item_data) {
		if (item_data) {
			mf_debug("=============== item is [%s]", item_data->fullpath);
		}
	}
#endif
}

static int __mf_media_data_sort_by_priority(const void *d1, const void *d2, int sequence_type)
{
	int ret = 0;
	switch (sequence_type) {
	case MF_SORT_BY_PRIORITY_TYPE_A2Z:
		ret = __mf_media_data_sort_by_date_cb_O2R(d1, d2);
		if (ret == 0) {
			ret = __mf_media_data_sort_by_size_cb_S2L(d1, d2);
			if (ret == 0) {
				ret = __mf_media_data_sort_by_name_cb_A2Z(d1, d2);
			}
		}
		break;
	case MF_SORT_BY_PRIORITY_TYPE_Z2A:
		ret = __mf_media_data_sort_by_date_cb_R2O(d1, d2);
		if (ret == 0) {
			ret = __mf_media_data_sort_by_size_cb_L2S(d1, d2);
			if (ret == 0) {
				ret = __mf_media_data_sort_by_name_cb_Z2A(d1, d2);
			}
		}
		break;
	case MF_SORT_BY_PRIORITY_DATE_O2R:
		ret = __mf_media_data_sort_by_size_cb_S2L(d1, d2);
		if (ret == 0) {
			ret = __mf_media_data_sort_by_name_cb_A2Z(d1, d2);
		}
		break;
	case MF_SORT_BY_PRIORITY_DATE_R2O:
		ret = __mf_media_data_sort_by_size_cb_L2S(d1, d2);
		if (ret == 0) {
			ret = __mf_media_data_sort_by_name_cb_Z2A(d1, d2);
		}
		break;
	case MF_SORT_BY_PRIORITY_SIZE_S2L:
		ret = __mf_media_data_sort_by_name_cb_A2Z(d1, d2);
		break;
	case MF_SORT_BY_PRIORITY_SIZE_L2S:
		ret = __mf_media_data_sort_by_name_cb_Z2A(d1, d2);
		break;
	default:
		break;
	}
	return ret;
}

/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the Assic table

**
*********************/
static int __mf_media_data_sort_by_name_cb_A2Z(const void *d1, const void *d2)
{
	media_data_s *txt1 = (media_data_s *) d1;
	media_data_s *txt2 = (media_data_s *) d2;
	gchar *name1 = NULL;
	gchar *name2 = NULL;
	int result = 0;

	if (!txt1 || !txt1->display_name) {
		return (1);
	}
	if (!txt2 || !txt2->display_name) {
		return (-1);
	}

	name1 = g_ascii_strdown(txt1->display_name, strlen(txt1->display_name));
	if (name1 == NULL) {
		return (-1);
	}
	name2 = g_ascii_strdown(txt2->display_name, strlen(txt2->display_name));
	if (name2 == NULL) {
		g_free(name1);
		name1 = NULL;
		return (-1);
	}
	result = g_strcmp0(name1, name2);

	g_free(name1);
	name1 = NULL;
	g_free(name2);
	name2 = NULL;
	return result;

}

/*********************
**Function name:	__sort_by_date_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the later created the later shown
*********************/
static int __mf_media_data_sort_by_date_cb_O2R(const void *d1, const void *d2)
{
	int ret = 0;
	media_data_s *time1 = (media_data_s *) d1;
	media_data_s *time2 = (media_data_s *) d2;

	if (!d1) {
		return 1;
	}
	if (!d2) {
		return -1;
	}

	if (time1->create_date > time2->create_date) {
		ret = 1;
	} else if (time1->create_date < time2->create_date) {
		ret = -1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_media_data_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_DATE_O2R);
	}
	return ret;
}

/*********************
**Function name:	__sort_by_type_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 < d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by the category type value
*********************/
static int __mf_media_data_sort_by_type_cb_A2Z(const void *d1, const void *d2)
{
	media_data_s *type1 = (media_data_s *) d1;
	media_data_s *type2 = (media_data_s *) d2;
	gchar *ext1 = NULL;
	gchar *ext2 = NULL;
	int result = 0;

	if (type1 == NULL || type1->ext == NULL) {
		return 1;
	}

	if (type2 == NULL || type2->ext == NULL) {
		return -1;
	}
	ext1 = g_ascii_strdown(type1->ext, strlen(type1->ext));
	if (ext1 == NULL) {
		return (-1);
	}
	ext2 = g_ascii_strdown(type2->ext, strlen(type2->ext));
	if (ext2 == NULL) {
		g_free(ext1);
		ext1 = NULL;
		return (-1);
	}
	result = g_strcmp0(ext1, ext2);

	g_free(ext1);
	ext1 = NULL;
	g_free(ext2);
	ext2 = NULL;

	if (result == 0) {
		result = __mf_media_data_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_TYPE_A2Z);
	}

	return result;
}

/*order:	the one with smaller size will be shown earlier*/
/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 > d2
**
**Action:
**	sort the list order by size, rule is the smaller the later shown
*********************/
static int __mf_media_data_sort_by_size_cb_S2L(const void *d1, const void *d2)
{
	int ret = 0;
	media_data_s *size1 = (media_data_s *) d1;
	media_data_s *size2 = (media_data_s *) d2;

	if (!d1) {
		return 1;
	}

	if (!d2) {
		return -1;
	}

	if (size1->size > size2->size) {
		ret = 1;
	} else if (size1->size < size2->size) {
		ret = -1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_media_data_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_SIZE_S2L);
	}
	return ret;
}

/*********************
**Function name:	__mf_fs_oper_sort_by_name_cb_Z2A
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	1	if d1 > d2
**	-1	if d1 <= d2
**
**Action:
**	sort the list order by the Assic table

**
*********************/
static int __mf_media_data_sort_by_name_cb_Z2A(const void *d1, const void *d2)
{
	media_data_s *txt1 = (media_data_s *) d1;
	media_data_s *txt2 = (media_data_s *) d2;

	int result = 0;

	if (!txt1 || !txt1->display_name) {
		return (1);
	}
	if (!txt2 || !txt2->display_name) {
		return (-1);
	}
	result = strcasecmp(txt1->display_name, txt2->display_name);

	if (result < 0) {
		return (1);
	} else {
		return (-1);
	}
}

/*********************
**Function name:	__sort_by_date_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by the later created the later shown
*********************/
static int __mf_media_data_sort_by_date_cb_R2O(const void *d1, const void *d2)
{
	int ret = 0;
	media_data_s *time1 = (media_data_s *) d1;
	media_data_s *time2 = (media_data_s *) d2;

	if (!d1) {
		return -1;
	}
	if (!d2) {
		return 1;
	}
	if (time1->create_date > time2->create_date) {
		ret = -1;
	} else if (time1->create_date < time2->create_date) {
		ret = 1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_media_data_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_DATE_R2O);
	}
	return ret;
}

/*********************
**Function name:	__sort_by_type_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by the category type value
*********************/
static int __mf_media_data_sort_by_type_cb_Z2A(const void *d1, const void *d2)
{
	media_data_s *type1 = (media_data_s *) d1;
	media_data_s *type2 = (media_data_s *) d2;
	gchar *ext1 = NULL;
	gchar *ext2 = NULL;
	int result = 0;

	if (type1 == NULL || type1->ext == NULL) {
		return -1;
	}

	if (type2 == NULL || type2->ext == NULL) {
		return 1;
	}

	ext1 = g_ascii_strdown(type1->ext, strlen(type1->ext));
	if (ext1 == NULL) {
		return (1);
	}
	ext2 = g_ascii_strdown(type2->ext, strlen(type2->ext));
	if (ext2 == NULL) {
		g_free(ext1);
		ext1 = NULL;
		return (-1);
	}
	result = g_strcmp0(ext1, ext2);
	g_free(ext1);
	ext1 = NULL;
	g_free(ext2);
	ext2 = NULL;
	if (result == 0) {
		result = __mf_media_data_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_TYPE_Z2A);
	}

	return -result;
}

/*order:	the one with smaller size will be shown earlier*/
/*********************
**Function name:	__sort_by_name_cb
**Parameter:
**	const void *d1:	node1 to compare
**	const void *d2:	node2 to compare
**
**Return value:
**	-1	if d1 > d2
**	0	if d1 = d2
**	1	if d1 < d2
**
**Action:
**	sort the list order by size, rule is the smaller the later shown
*********************/
static int __mf_media_data_sort_by_size_cb_L2S(const void *d1, const void *d2)
{
	int ret = 0;
	media_data_s *size1 = (media_data_s *) d1;
	media_data_s *size2 = (media_data_s *) d2;

	if (!d1) {
		return -1;
	}

	if (!d2) {
		return 1;
	}

	if (size1->size > size2->size) {
		ret = -1;
	} else if (size1->size < size2->size) {
		ret = 1;
	} else {
		ret = 0;
	}

	if (ret == 0) {
		ret = __mf_media_data_sort_by_priority(d1, d2, MF_SORT_BY_PRIORITY_SIZE_L2S);
	}
	return ret;
}

/*********************
**Function name:	mf_fs_oper_sort_list
**Parameter:
**	Eina_List **list:	the list we need to sort
**	int sort_opt:		sort option
**
**Return value:
**	void
**
**Action:
**	sort the list order by sort option with the call back
*********************/
void mf_media_data_sort_list(Eina_List **list, int sort_opt)
{
	Eina_Compare_Cb sort_func = NULL;
	if (!(*list)) {
		return;
	}
	switch (sort_opt) {
	case MYFILE_SORT_BY_NAME_A2Z:
		sort_func = __mf_media_data_sort_by_name_cb_A2Z;
		break;
	case MYFILE_SORT_BY_TYPE_A2Z:
		sort_func = __mf_media_data_sort_by_type_cb_A2Z;
		break;
	case MYFILE_SORT_BY_SIZE_S2L:
		sort_func = __mf_media_data_sort_by_size_cb_S2L;
		break;
	case MYFILE_SORT_BY_DATE_O2R:
		sort_func = __mf_media_data_sort_by_date_cb_O2R;
		break;
	case MYFILE_SORT_BY_NAME_Z2A:
		sort_func = __mf_media_data_sort_by_name_cb_Z2A;
		break;
	case MYFILE_SORT_BY_TYPE_Z2A:
		sort_func = __mf_media_data_sort_by_type_cb_Z2A;
		break;
	case MYFILE_SORT_BY_SIZE_L2S:
		sort_func = __mf_media_data_sort_by_size_cb_L2S;
		break;
	case MYFILE_SORT_BY_DATE_R2O:
		sort_func = __mf_media_data_sort_by_date_cb_R2O;
		break;
	default:
		sort_func = __mf_media_data_sort_by_type_cb_A2Z;
		break;
	}
	*list = eina_list_sort(*list, eina_list_count(*list), sort_func);
}


