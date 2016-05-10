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

#ifndef __DEF_MF_UG_DETAIL_DLOG_H_
#define __DEF_MF_UG_DETAIL_DLOG_H_
#include <stdio.h>
#include <string.h>
#include "mf-dlog.h"

#define DLOG_ON		1

#define ug_myfile_detail_log(fmt, args...) \
	do { \
		myfile_dlog("\x1b[%dm\x1b[%dm[myfile_details] [%s:%d] : "fmt, \
		40, 32, __func__, __LINE__, ##args); \
		myfile_dlog("\x1b[0m\n"); \
	} while (0)

#if DLOG_ON

#define ug_detail_debug(fmt , args...)    dlog_print(DLOG_WARN, LOG_TAG,"[%s][%d]debug message from ug-myfile-efl is : "fmt"\n", __func__, __LINE__, ##args)
#define myfile_dlog(fmt , args...)        dlog_print(DLOG_WARN, LOG_TAG,"[%s][%d]: "fmt"\n", __func__, __LINE__, ##args)
#define myfile_dlog_assert(fmt , args...) dlog_print(DLOG_ERROR, LOG_TAG,"[%s]: "fmt"\n", __func__, ##args)
#define myfile_dlog_func_line()           dlog_print(DLOG_WARN, LOG_TAG,("[%s][%d]\n", __func__, __LINE__)
#define myfile_dlog_func_start()          dlog_print(DLOG_WARN, LOG_TAG,"[%s][%d][  START  ]\n", __func__, __LINE__)
#define myfile_dlog_func_end()            dlog_print(DLOG_WARN, LOG_TAG,"[%s][%d][  END  ]\n", __func__, __LINE__)
#else
#define myfile_dlog(fmt , args...)        printf("[MYFILE][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args)
#define myfile_dlog_assert(fmt , args...) printf("[MYFILE][%40s:%4d][###assert###] "fmt"\n", __func__, __LINE__, ##args)
#define myfile_dlog_func_line()           printf("[MYFILE][%40s:%4d]\n", __func__, __LINE__)
#endif

#define startfunc   		myfile_dlog("+-  START -------------------------");
#define endfunc     		myfile_dlog("+-  END  --------------------------");


#define ug_detail_trace_error(fmt, arg...) \
	do { \
		dlog_print(DLOG_ERROR, LOG_TAG,"[%s][%d] "fmt"\n", strrchr(__FILE__, '/')+1, __LINE__, ##arg); \
	} while (0)

#define ug_detail_retvm_if(expr, val, fmt, arg...) \
	do { \
		if (expr) { \
			ug_detail_trace_error(fmt, ##arg); \
			return (val); \
		} \
	} while (0)

#define ug_detail_retv_if(expr, val) \
	do { \
		if (expr) { \
			return (val); \
		} \
	} while (0)

#define ug_detail_retm_if(expr, fmt, arg...) \
	do { \
		if (expr) { \
			ug_detail_trace_error(fmt, ##arg); \
			return; \
		} \
	} while (0)

#define UG_DETAIL_TRACE_BEGIN \
	do {\
		{\
			dlog_print(DLOG_DEBUG, LOG_TAG, "\n\033[0;35mENTER FUNCTION: %s. \033[0m\t%s:%d\n", \
			__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
		} \
	} while (0);

#define UG_DETAIL_TRACE_END  \
	do {\
		{\
			dlog_print(DLOG_DEBUG, LOG_TAG, "\n\033[0;35mEXIT FUNCTION: %s. \033[0m\t%s:%d\n", \
			__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
		} \
	} while (0);

#endif				/* end of __DEF_MF_UG_DETAIL_DLOG_H_*/
