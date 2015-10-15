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

#ifndef __MF_MEDIA_CONTENT_H_DEF__
#define __MF_MEDIA_CONTENT_H_DEF__

#include <media_content.h>

typedef struct __mf_condition_s mf_condition_s;
struct __mf_condition_s {
	char *cond;                              /*set media type or favorite type, or other query statement*/
	media_content_collation_e collate_type;  /*collate type*/
	media_content_order_e sort_type;         /*sort type*/
	char *sort_keyword;                      /*sort keyword*/
	int offset;                              /*offset*/
	int count;                               /*count*/
	bool with_meta;                          /*whether get image or video info*/
};

void mf_media_content_scan_file(const char *path);
void mf_media_content_scan_folder(const char *path);
int mf_media_content_data_get(void *data, char *condition, bool (*func) (media_info_h media, void *data));
int mf_media_content_data_count_get(const char *condition);
void mf_media_content_disconnect();
void mf_media_content_scan_file(const char *path);


#endif //__MF_MEDIA_CONTENT_H_DEF__

