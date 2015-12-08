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

#ifndef __MF_LOG__
#define __MF_LOG__

#define MF_LOG_RESULT_FILE "/usr/apps/org.tizen.myfile/data/log"
#define MF_LOG_FORMAT	"[%s] [%s] {%d} -- %s"

int mf_log_init();
void mf_log_finalize();
int mf_log_record(char *filename, const char *function, int line, char *fmt, ...);


#ifdef MYFILE_CRITICAL_LOG
#define MF_LOG_RECORD(fmt, arg...)	do { mf_log_record(__FILE__, __func__, __LINE__, fmt, ##arg); } while (0)
#else
#define MF_LOG_RECORD(fmt, arg...)	do { (void)0; }while (0)
#endif

#endif //__MF_LOG__
