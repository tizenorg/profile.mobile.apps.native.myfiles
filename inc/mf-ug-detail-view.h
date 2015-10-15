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

#ifndef __DEF_MF_UG_DETAIL_VIEW_H_
#define __DEF_MF_UG_DETAIL_VIEW_H_

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <app.h>
#include "mf-ug-detail-resource.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <Elementary.h>
#include <libintl.h>

#ifndef PREFIX
#define PREFIX "/usr"
#endif

#define UG_LOCALEDIR "/usr/ug/res/locale"
#define DETAIL_UG_PKGNAME "ug-myfile-detail-efl"

#define _UG_EDJ(o)				elm_layout_edje_get(o)

#define FOLDER_DETILES_NUM		5
#define FOLDER_NORMDETILES_NUM		4
#define FILE_DETILES_NUM		4
#define FILE_EXIF_INFO_NUM		2
#define BUF_SIZE			4096
#define BASIC_SIZE			1024	/*used for file size check*/

#define LONG_LONG_UNSIGNED_INT off_t

struct MF_LIST_Item_S {
	GString *m_ItemName;
	struct detailData *detail;
};

enum UG_SIZE_TYPE {
	UG_SIZE_BYTE = 0,
	UG_SIZE_KB,
	UG_SIZE_MB,
	UG_SIZE_GB
};

typedef enum __View_Style View_Style;
enum __View_Style {
	VIEW_NONE = 0,
	VIEW_DIR,
	VIEW_FILE_NORMAL,
	VIEW_FILE_IMAGE,
	VIEW_FILE_VIDEO,
	VIEW_FILE_WITH_GPS,
	VIEW_FILE_MULTI,
	VIEW_MAX
};


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* end of __DEF_MF_UG_DETAIL_VIEW_H_ */
