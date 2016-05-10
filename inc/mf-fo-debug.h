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

#ifndef __MF_FO_DEBUG_H_
#define __MF_FO_DEBUG_H_

#ifdef MYFILE_USE_LOG
#include <stdio.h>
#include <string.h>
#include <mf-dlog.h>
#include <mf-log.h>

#define mf_fo_logd(fmt , arg...)		do { dlog_print(DLOG_DEBUG, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg); } while (0)
#define mf_fo_loge(fmt , args...)		do {\
							dlog_print(DLOG_WARN, LOG_TAG, "[%s][%d] "fmt"\n", __func__, __LINE__, ##args); \
							MF_LOG_RECORD(fmt,##args); \
						   } while (0)//LOGE
#define mf_fo_logw(fmt , arg...)		do { dlog_print(DLOG_WARN, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg); } while (0)
#define mf_fo_logi(fmt , arg...)		do { dlog_print(DLOG_INFO, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg); } while (0)
#define mf_fo_loga(fmt , arg...)		do { dlog_print(DLOG_WARN, LOG_TAG, "[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg); } while (0)//LOGE
#define FO_TRACE_BEGIN 		do {\
					{\
						dlog_print(DLOG_WARN, LOG_TAG, "\n\033[0;35mENTER FUNCTION: %s. \033[0m\t%s:%d\n", \
						__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
					} \
				} while (0) ;

#define FO_TRACE_END  		do {\
					{\
						dlog_print(DLOG_WARN, LOG_TAG, "\n\033[0;35mEXIT FUNCTION: %s. \033[0m\t%s:%d\n", \
						__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
					} \
				} while (0) ;


#else
#include <sys/syscall.h>
#include <unistd.h>
#define mf_fo_logd(fmt , args...)		do { printf("[%ld][D][%20s:%4d] "fmt"\n", syscall(__NR_gettid), __func__, __LINE__, ##args); } while (0)
#define mf_fo_loge(fmt , args...)		do {\
							printf("[%ld][E][%20s:%4d] "fmt"\n", syscall(__NR_gettid), __func__, __LINE__, ##args);\
							MF_LOG_RECORD(fmt,##args); \
						   } while (0)
#define mf_fo_logw(fmt , args...)		do { printf("[%ld][W][%20s:%4d] "fmt"\n", syscall(__NR_gettid), __func__, __LINE__, ##args); } while (0)
#define mf_fo_logi(fmt , args...)		do { printf("[%ld][I][%20s:%4d] "fmt"\n", syscall(__NR_gettid), __func__, __LINE__, ##args); } while (0)
#define mf_fo_loga(fmt , args...)		do { printf("[%ld][A][%20s:%4d] "fmt"\n", syscall(__NR_gettid), __func__, __LINE__, ##args); } while (0)
#define FO_TRACE_BEGIN 		do { printf("[MYFILE][W][%40s:%4d] \n", __func__, __LINE__); } while (0);
#define FO_TRACE_END  		do { printf("[MYFILE][W][%40s:%4d] \n", __func__, __LINE__); } while (0);

#endif



#endif /* end of __MF_FO_DEBUG_H_ */
