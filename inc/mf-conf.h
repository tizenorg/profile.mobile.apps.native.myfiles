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

#ifndef __DEF_MYFILE_CONF_H_
#define __DEF_MYFILE_CONF_H_


#define MYFILE_STRING_PACKAGE				"myfile"
#define MYFILE_PACKAGE			"My Files"
#define PKGNAME_MYFILE			"org.tizen.myfile"
#define PKGNAME_SYSTEM			"sys_string"
#define LOCALEDIR			"/usr/apps/org.tizen.myfile/res/locale"
#define MF_IMAGE_HEAD			"myfile_"
#define EDJ_PATH			"/usr/apps/org.tizen.myfile/res/edje"
#define ICON_PATH			"/usr/apps/org.tizen.myfile/res/images"

#define EDJ_NAVIGATIONBAR		EDJ_PATH"/myfileNavigationBar.edj"
#define EDJ_IMAGE			EDJ_PATH"/edc_image_macro.edj"
#define GRP_NAVI_VIEW			"navigation_view"

#define EDJ_SEARCHBAR			EDJ_PATH"/myfileSearchBar.edj"
#define GRP_SEARCH_BAR			"search_bar"

#define EDJ_NAME			EDJ_PATH"/myfile.edj"
#define EDJ_GENLIST_NAME			EDJ_PATH"/myfile_genlist.edj"
#define EDJ_GENGRID_NAME			EDJ_PATH"/myfile_gengrid.edj"
#define GRP_THUMBNAIL_ONLY		"thumbnail_only"
#define GRP_GENGRID_COTENT		"gengrid_content"
#define GRP_PROGRESS_LABEL		"label"
#define GRP_TRAY_ITEM			"apptray.item"

#define MYFILE_DATEFORMAT_12		"yMdhm"//"MMddyyyyhm"
#define MYFILE_DATEFORMAT_24		"yMdhm"


#define MYFILE_LABEL_STRING_LENGTH	128

#define MYFILE_ICU_ARR_LENGTH		128
#define BUFF_SIZE			256
#define MYFILE_BASIC_SIZE		1024
#define MAX_FOLDER_COUNT		300
#define MF_UDATE_NUM	1000

#define MF_ROUND_D(x, dig) (floor((x) * pow(10, dig+1) + 0.5) / pow(10, dig+1))

#endif //__DEF_MYFILE_CONF_H_
