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

#ifndef __MF_MEDIA_CONTENT_DATA_H_DEF__
#define __MF_MEDIA_CONTENT_DATA_H_DEF__

#include <media_content_type.h>
#include <media_filter.h>
#include <media_image.h>
#include <media_video.h>
#include <media_tag.h>
#include <media_folder.h>
#include <media_info.h>
#include <eina_list.h>
#include <Elementary.h>

/*MEDIA_TYPE 0-image, 1-video, 2-sound, 3-music*/
#define MF_CONDITION_LOCAL_IMAGE		"((MEDIA_TYPE=0) AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1))"
#define MF_CONDITION_LOCAL_VIDEO		"((MEDIA_TYPE=1) AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1))"
#define MF_CONDITION_LOCAL_SOUND		"((MEDIA_TYPE=2 OR MEDIA_TYPE=3) AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1))"
#define MF_CONDITION_LOCAL_DOCUMENT		"((MEDIA_TYPE=4) AND (MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1))"


typedef struct __media_data_s media_data_s;
struct __media_data_s {
	char *fullpath;
	char *display_name;
	char *thumbnail_path;
	char *ext;
	long long size;
	time_t create_date;
	int storage_type;
	int file_type;
};
void mf_media_data_item_free(media_data_s **item_data);
void mf_media_category_list_get(int category_type, Eina_List **category_list);
void mf_media_category_item_get(const char *fullpath, int type, Eina_List **category_list);
void mf_media_data_printf(Eina_List *list);
void mf_media_data_sort_list(Eina_List **list, int sort_opt);
void mf_media_data_list_free(Eina_List **list);
media_data_s *mf_media_data_get_by_media_handle(media_info_h media);

#endif //__MF_MEDIA_CONTENT_DATA_H_DEF__
