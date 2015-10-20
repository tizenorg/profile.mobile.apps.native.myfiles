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

#ifndef __DEF_MYFILE_LAUNCH_H_
#define __DEF_MYFILE_LAUNCH_H_

#include <app_control.h>
#include <app.h>


#ifdef _USE_SHARE_PANEL
#define MF_SHARE_FILE_PREFIX "file://"
#define MF_SHARE_OPERATION_SINGLE "http://tizen.org/appcontrol/operation/share"
#define MF_SHARE_OPERATION_MULTIPLE "http://tizen.org/appcontrol/operation/multi_share"
#define MF_SHARE_OPERATION_TEXT "http://tizen.org/appcontrol/operation/share_text"
#define MF_SHARE_SVC_FILE_PATH "http://tizen.org/appcontrol/data/path"
#define MF_SHARE_SVC_TEXT_PATH "http://tizen.org/appcontrol/data/text"
#endif

typedef union __mf_launch_share_u mf_launch_share_u;
union __mf_launch_share_u {
	char *single_file;
	char **multi_files;
};

typedef enum _LOAD_UG_TYPE MF_LOAD_UG_TYPE;

enum _LOAD_UG_TYPE {
	MF_LOAD_UG_MESSAGE,
	MF_LOAD_UG_EMAIL,
	MF_LOAD_UG_BLUETOOTH,
	MF_LOAD_UG_DETAIL,

	MF_LOAD_UG_MAX
};

void mf_launch_load_ug(void *data, char *path, MF_LOAD_UG_TYPE type, Eina_Bool multi_flag);

void mf_launch_service_idler_del();

int mf_launch_service(void *data, char *path);
void mf_launch_load_ug_myfile(void *data);

#ifdef _USE_SHARE_PANEL

bool mf_launch_share(void *data);
void mf_launch_item_share(void *data, Evas_Object * obj, void *event_info);
#endif
char * mf_launch_item_size_calculate(double size);
void mf_launch_service_timer_del();
void mf_launch_load_storage(void *data);
void mf_launch_add_recent_files(void *data, const char *path);

#endif //__DEF_MYFILE_LAUNCH_H_
