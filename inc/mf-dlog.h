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

#ifndef __MYFILE_DEBUG_H_
#define __MYFILE_DEBUG_H_

#include <stdio.h>
#include <string.h>
#include "mf-log.h"
#include "mf-conf.h"
#include <sys/types.h>
#include <linux/unistd.h>
#include <dlog.h>

#ifdef MYFILE_USE_LOG

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG		"MYFILES"
#define LOG_COLOR_RED      "\033[31m"
#define LOG_COLOR_RESET    "\033[0m"

#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#define SECURE_DEBUG(fmt, args...)	SECURE_LOGD(FONT_COLOR_BLUE fmt FONT_COLOR_RESET, ##args)
#define SECURE_INFO(fmt, args...)	SECURE_LOGI(FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##args)
#define SECURE_ERROR(fmt, args...)	SECURE_LOGW(FONT_COLOR_RED fmt FONT_COLOR_RESET, ##args)//SECURE_LOGE

#define MYFILE_TRACE_DEBUG(fmt, arg...) do { LOGD("[%s][%d] "fmt "\n", __func__, __LINE__, ##arg); } while (0)
#define MYFILE_TRACE_ERROR(fmt, arg...) do { LOGW("[%s][%d] "fmt"\n", __func__, __LINE__, ##arg); } while (0)//LOGE

#define mf_debug(fmt, args...)	do { LOGD("[%s][%d] "fmt"\n", __func__, __LINE__, ##args); } while (0)

#define mf_error(fmt, arg...)	LOGW(LOG_COLOR_RED"[ %s : %d]   "fmt""LOG_COLOR_RESET, __FUNCTION__, __LINE__,##arg)//LOGE
#define mf_warning(fmt, args...)	do { LOGW("[%s][%d] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define mf_info(fmt, args...)		do { LOGI("[%s][%d] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define mf_assert(fmt, args...)	do { LOGW("[ASSERT][%s][%d] "fmt"\n", __func__, __LINE__, ##args); } while (0)//LOGE

#define t_start 	LOG(LOG_DEBUG, "LAUNCH", "[myfile:Application:%s:IN]",__func__)


#define t_end		LOG(LOG_DEBUG, "LAUNCH", "[myfile:Application:%s:OUT]",__func__)

#define MF_TRACE_BEGIN  while (0) {\
					{\
						LOGD("\n\033[0;35mENTER FUNCTION: %s. \033[0m\t%s:%d\n", \
						__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
					} \
			};

#define MF_TRACE_END  while (0) {\
					{\
						LOGD("\n\033[0;35mEXIT FUNCTION: %s. \033[0m\t%s:%d\n", \
						__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
					} \
			};
#else

#define MYFILE_TRACE_DEBUG(fmt, arg...)	do { printf("%s:%d: " fmt "\n", __FILE__, __LINE__, ##arg); } while (0)
#define MYFILE_TRACE_ERROR(fmt, arg...)	do { printf("[%s][%d] "fmt"\n", strrchr(__FILE__, '/')+1, __LINE__, ##arg); } while (0)

#define mf_debug(fmt, args...)	do { printf("[MYFILE][D][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define mf_error(fmt, args...)	do { \
					printf("[MYFILE][E][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args); \
					MF_LOG_RECORD(fmt,##args); \
				   } while (0)
#define mf_warning(fmt, args...)	do { printf("[MYFILE][W][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define mf_info(fmt, args...)	do { printf("[MYFILE][I][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define mf_assert(fmt, args...)	do { printf("[MYFILE][A][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args); } while (0)

#define MF_TRACE_BEGIN		do { printf("[MYFILE][W][%40s:%4d] \n", __func__, __LINE__); } while (0);
#define MF_TRACE_END		do { printf("[MYFILE][W][%40s:%4d] \n", __func__, __LINE__); } while (0);
#define t_start 		do { printf("[MYFILE][W][%40s:%4d] \n", __func__, __LINE__); } while (0);


#define t_end			do { printf("[MYFILE][W][%40s:%4d] \n", __func__, __LINE__); } while (0);
#define SECURE_DEBUG(fmt, args...)	do { printf("[MYFILE][W][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define SECURE_INFO(fmt, args...)	do { printf("[MYFILE][W][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#define SECURE_ERROR(fmt, args...)	do { printf("[MYFILE][W][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args); } while (0)
#endif
#define mf_retvm_if(expr, val, fmt, arg...) do { \
			if (expr) { \
				MYFILE_TRACE_ERROR(fmt, ##arg); \
				MF_LOG_RECORD(fmt,##arg); \
				return (val); \
			} \
		} while (0)

#define mf_retv_if(expr, val) do { \
			if (expr) { \
				return (val); \
			} \
		} while (0)


#define mf_retm_if(expr, fmt, arg...) do { \
			if (expr) { \
				MYFILE_TRACE_ERROR(fmt, ##arg); \
				MF_LOG_RECORD(fmt,##arg); \
				return; \
			} \
		} while (0)

#define mf_ret_if(expr, fmt, arg...) do { \
			if (expr) { \
				MF_LOG_RECORD(fmt,##arg); \
				return; \
			} \
		} while (0)


#define MF_CHECK(expr) 				mf_retm_if(!(expr),"INVALID PARAM RETURN")
#define MF_CHECK_FALSE(expr) 			mf_retvm_if(!(expr),FALSE,"INVALID PARM RETURN FALSE")
#define MF_CHECK_VAL(expr, val) 		mf_retvm_if(!(expr),val,"INVALID PARM RETURN NULL")
#define MF_CHECK_NULL(expr) 			mf_retvm_if(!(expr),NULL,"INVALID PARM RETURN NULL")

#endif //__MYFILE_DEBUG_H_
