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

#ifndef __MF_FO_COMMON_H_DEF__
#define __MF_FO_COMMON_H_DEF__

#include <Ecore.h>


#include "mf-request.h"
#include "mf-fo-debug.h"

#define MYFILE_MAGIC_MAIN_CONTEXT           (0x1983cdaf)
#define MYFILE_MAGIC_DETAIL_LIST_ITEM       (0x1977abcd)
#define MYFILE_MAGIC_PIPE_DATA				 (0x0716ffcc)

#define MYFILE_MAGIC                 unsigned int  __magic
#define MYFILE_MAGIC_SET(d, m)       (d)->__magic = (m)
#define MYFILE_MAGIC_CHECK(d, m)     ((d) && ((d)->__magic == (m)))
#define FO_ERROR_CHECK(x)	((x)&0x000000FF)

#define MF_VISUAL_FOLDER_SIZE 16	/*used only for the progress bar,
					if the file system is different, the folder size is diff, fat is 4k and fat32 is 16k*/
#define MF_ERR_BUF		256

#define TEMP_FOLDER_FOR_COPY_PHONE		"/opt/usr/media/.operation_temp"
#define TEMP_FOLDER_FOR_COPY_MMC		"/opt/storage/sdcard/.operation_temp"

#define MF_FILE_ERROR_LOG(buf, fmt, arg)    do { \
							{\
								if (strerror_r(errno, buf, MF_ERR_BUF) == 0 ) { \
									mf_fo_logd(fmt": [%s]  error is [%s] \n", (arg), (buf));	\
								} else {					\
									mf_fo_logd(fmt": [%s] \n", (arg));		\
								}\
							}\
						} while (0)
typedef enum _msg_type mf_msg_type;

enum _msg_type {
	MF_MSG_NONE,
	MF_MSG_DOING,
	MF_MSG_SKIP,
	MF_MSG_SYNC,
	MF_MSG_REQUEST,
	MF_MSG_ERROR,
	MF_MSG_END,
	MF_MSG_CANCELLED,
} ;


typedef struct __mf_fo_dir_list_info mf_fo_dir_list_info;

struct __mf_fo_dir_list_info {
	char *ftw_path;
	unsigned long long size;
	int type;
};


typedef struct _mf_fo_msg mf_fo_msg;

struct _mf_fo_msg {
	MYFILE_MAGIC;
	mf_msg_type msg_type;
	int error_code;
	unsigned long long total_size;
	unsigned long long current_size;
	unsigned int total_index;
	unsigned int current_index;
	const char *current;
	char *current_real;
	mf_fo_request *request;
	Ecore_Pipe *pipe;
};

typedef void (*mf_msg_callback) (mf_fo_msg *msg, void *data);

enum {
	MF_FO_ERR_NONE = 0,
	MF_FO_ERR_PERMISSION = 1,
	MF_FO_ERR_ARGUMENT = 2,
	MF_FO_ERR_FAULT = 3,
	MF_FO_ERR_TYPE = 4,
	MF_FO_ERR_MAX_OPEN = 5,
	MF_FO_ERR_SPACE = 6,
	MF_FO_ERR_RO = 7,
	MF_FO_ERR_LOOP = 8,
	MF_FO_ERR_MEM = 9,
	MF_FO_ERR_NOT_EXIST = 10,
	MF_FO_ERR_LONG_NAME = 11,
	MF_FO_ERR_BIG_SIZE = 12,
	MF_FO_ERR_IO = 13,
	MF_FO_ERR_UNKNOWN = 254,
	MF_FO_ERR_MAX = 255,
};


#define MF_FO_ERR_COMMON_CLASS (0x0800)
#define MF_FO_ERR_SRC_CLASS (0x0400)
#define MF_FO_ERR_DST_CLASS (0x0200)
#define MF_FO_ERR_REPORT_CLASS (0x0100)

#define E_IS_SRC(err) ((err)&MF_FO_ERR_SRC_CLASS)
#define E_IS_DST(err) ((err)&MF_FO_ERR_DST_CLASS)
#define E_IS_COMMON(err) ((err)&MF_FO_ERR_COMMON_CLASS)
#define E_IS_REPORTABLE(err) ((err)&MF_FO_ERR_REPORT_CLASS)

#define E_GET_CLASS(err) ((err)&0x0F00)
#define E_GET_DETAIL(err) ((err)&0x00FF)

#define MF_FO_ERR_SET(x) (((x) & 0x0FFF) | 0xFFFFF000)

typedef void (*mf_common_pipe_cb) (void *data, void *buffer, unsigned int nbyte);

#endif //__MF_FO_COMMON_H_DEF__
