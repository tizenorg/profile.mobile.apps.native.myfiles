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

#ifndef _MF_INOTIFY_HANDLE_H_
#define _MF_INOTIFY_HANDLE_H_

typedef enum _mf_inotify_event mf_inotify_event;
enum _mf_inotify_event {
	MF_INOTI_NONE = 0,
	MF_INOTI_CREATE,
	MF_INOTI_DELETE,
	MF_INOTI_MODIFY,
	MF_INOTI_MOVE_OUT,
	MF_INOTI_MOVE_IN,
	MF_INOTI_DELETE_SELF,
	MF_INOTI_MOVE_SELF,
	MF_INOTI_MAX,
};

typedef void (*mf_inotify_cb) (mf_inotify_event event, char *name, void *data);

int mf_inotify_handle_init_inotify(void);
int mf_inotify_handle_add_watch(const char *path, mf_inotify_cb callback, void *user_data);
int mf_inotify_handle_rm_watch(void);
void mf_inotify_handle_finalize_inotify(void);
void mf_inotify_handle_request_handled_send();

#endif //_MF_INOTIFY_HANDLE_H_
