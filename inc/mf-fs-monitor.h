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


#ifndef __MF_FS_MONITOR_H_DEF__
#define __MF_FS_MONITOR_H_DEF__

int mf_fs_monitor_add_dir_watch(const char *path, void *data);
int mf_fs_monitor_create(void *data);
int mf_fs_monitor_remove_dir_watch(void);
void mf_fs_monitor_destory();

#endif //__MF_FS_MONITOR_H_DEF__
