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

#ifndef __DEF_MF_UG_DETAIL_H_
#define __DEF_MF_UG_DETAIL_H_

#define __ARM__

#include <Elementary.h>
#include <Ethumb.h>
#include <glib.h>
#include <Ecore.h>
#include "mf-ug-detail-view.h"
/* for SG */
#include <glib-object.h>
#include <utils_i18n.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>

/* for dlog */
#include "mf-ug-detail-dlog.h"
#include "mf-ug-detail-view.h"
#include "mf-ug-detail-fs.h"
#include "mf-ug-detail-media.h"

/***********	Global Definitions		***********/

struct myfileStatus {
	View_Style view_type;
	GString *path;
	Elm_Genlist_Item_Class itc;
	Elm_Genlist_Item_Class title_itc;
	Elm_Genlist_Item_Class seperator_itc;
	Elm_Genlist_Item_Class multiline_itc;
};

struct myfileInfo {
	/** Common info*/
	i18n_udate date;
	int validFlag;
	int category_type;
	int method;
	int permission;
	int file_type;
	LONG_LONG_UNSIGNED_INT size;
	char *filesize;
	char *filepath;
	char *filename;
	char *file_ext;
	char *file_location;
	char *contains;
	char *create_date;
	GString *category;
	/**resolution */
	char *resolution;
	/**Gps info */
	char *latitude;
	char *longitude;
	double dsize;
	int unit_num;
} ;


typedef struct detailData mfDetailData;
struct detailData {
	struct myfileStatus mf_Status;
	struct myfileInfo mf_Info;
};


#define IF_FREE(ptr) if (ptr) {free(ptr); ptr = NULL;}

Evas_Object *mf_ug_detail_view_create_genlist(Evas_Object *parent, void *data);

struct detailData *mf_ug_detail_multi_info_extract(const char *path);
struct detailData *mf_ug_detail_common_info_extract(const char *path);
void mf_ug_detail_view_init_data(void *data);
void mf_ug_detail_view_destroy_data(void *data);
void mf_ug_detail_view_process_genlist(void *data, Evas_Object *genlist);


#endif	/* __DEF_MF_UG_DETAIL_H_ */
