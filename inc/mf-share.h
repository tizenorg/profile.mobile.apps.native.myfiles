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

#ifndef __MF_SHARE_H__
#define __MF_SHARE_H__

#define SHARE_FILE_MODE_NORMAL		0x01
#define SHARE_FILE_MODE_IMAGE		0x02
#define SHARE_FILE_MODE_VIDEO		0x04
#define SHARE_FILE_MODE_MULTI_IMAGE	0x10
#define SHARE_FILE_MODE_MULTI_VIDEO	0x20
#define SHARE_FILE_MODE_OTHERS		0x40

typedef enum __share_mode_e share_mode_e;
enum __share_mode_e {
	SHARE_MODE_NORMAL,
	SHARE_MODE_IMAGE,
	SHARE_MODE_IMAGE_VIDEO,
	SHARE_MODE_VIDEO,
	SHARE_MODE_MULTI_IMAGE,
	SHARE_MODE_MULTI_VIDEO,
	SHARE_MODE_MAX
};

void mf_share_launch_multi_file(void *data);
int mf_share_mode_get(Eina_List * selected_list);

#endif //__MF_SHARE_H__
