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

#ifndef __DEF_MF_UG_DETAIL_MEDIA_H_
#define __DEF_MF_UG_DETAIL_MEDIA_H_

#include "mf-ug-detail.h"
#include "mf-ug-detail-view.h"
#include <libexif/exif-data.h>

#include "mf-ug-detail-fs.h"


void mf_ug_detail_media_get_common_info(void *data, char *path);

void mf_ug_detail_media_get_exif_info(void *data);

void mf_ug_detail_media_get_file_resolution(void *data, char *path);

void mf_ug_detail_media_get_file_location(void *data, char* path);
char *mf_ug_detail_media_get_icu_date(i18n_udate date);
void mf_ug_detail_media_get_file_ext(void *data, char *path);

#endif //__DEF_MF_UG_DETAIL_MEDIA_H_
