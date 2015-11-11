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

#ifdef MYFILE_CRITICAL_LOG
#include <stdio.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "mf-log.h"
#include "mf-dlog.h"
#include "mf-util.h"

static FILE *g_fp = NULL;

int mf_log_init()
{
	MF_TRACE_BEGIN;
	pid_t process_id = 0;
	process_id = getpid();

	char *result_file = g_strdup_printf("%s-%d", MF_LOG_RESULT_FILE, (int)process_id);
	if (result_file == NULL) {
		MF_TRACE_END;
		return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
	}

	g_fp = fopen(result_file, "at+");

	if (g_fp == NULL) {
		free(result_file);
		MF_TRACE_END;
		return MYFILE_ERR_FILE_OPEN_FAIL;
	}

	free(result_file);
	MF_TRACE_END;
	return MYFILE_ERR_NONE;
}
void mf_log_finalize()
{
	if (g_fp != NULL) {
		fclose(g_fp);
		g_fp = NULL;
	}

}

int mf_log_record(char *filename, const char *function, int line, char *fmt, ...)
{
	if (g_fp == NULL) {
		return MYFILE_ERR_INVALID_ARG;
	}

	char *message = NULL;
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	message = g_strdup_vprintf(fmt, arg_ptr);
	va_end(arg_ptr);

	fprintf(g_fp, MF_LOG_FORMAT, filename, function, line, message);
	if (message != NULL) {
		free(message);
		message = NULL;
	}

	return MYFILE_ERR_NONE;
}
#endif
