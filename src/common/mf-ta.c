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

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdarg.h>

#include "mf-ta.h"
#include "mf-dlog.h"
#ifdef MYFILE_USE_TA
static void __mf_ta_free_cps(void);
static int __mf_ta_get_cp_index(char *name);

static void __mf_ta_free_accums(void);
static int __mf_ta_get_accum_index(char *name);

mf_ta_checkpoint **mf_g_cps = NULL;
static int mf_g_cp_index = 0;

mf_ta_accum_item **mf_g_accums = NULL;
static int mf_g_accum_index = 0;
static int mf_g_accum_longest_name = 0;
static unsigned long mf_g_accum_first_time = 0xFFFFFFFF;
static int mf_g_enable = 1;

int mf_ta_init(void)
{
	if (mf_g_accums)
		return 0;

	mf_g_cps = (mf_ta_checkpoint **) malloc(MF_TA_MAX_CHECKPOINT * sizeof(mf_ta_checkpoint *));
	if (!mf_g_cps)
		return -1;

	mf_g_accums = (mf_ta_accum_item **) malloc(MF_TA_MAX_CHECKPOINT * sizeof(mf_ta_accum_item *));
	if (!mf_g_accums)
		return -1;

	mf_g_accum_first_time = 0xFFFFFFFF;

	return 0;
}

int mf_ta_release(void)
{
	if (!mf_g_accums)
		return 0;

	__mf_ta_free_cps();
	__mf_ta_free_accums();

	mf_g_accum_first_time = 0xFFFFFFFF;

	return 0;
}

void mf_ta_set_enable(int enable)
{
	SECURE_DEBUG("MF_TA : setting enable to %d\n", enable);
	mf_g_enable = enable;
}

int mf_ta_get_numof_checkpoints()
{
	return mf_g_cp_index;
}

char *mf_ta_fmt(const char *fmt, ...)
{
	static char ta_buf[512];
	va_list args;

	memset(ta_buf, '\0', 512);

	va_start(args, fmt);
	vsnprintf(ta_buf, 512, fmt, args);
	va_end(args);

	return ta_buf;
}


int mf_ta_add_checkpoint(char *name, int show, char *filename, int line)
{
	mf_ta_checkpoint *cp = NULL;
	struct timeval t;

	if (!mf_g_enable)
		return -1;

	if (!mf_g_accums)
		return 0;

	if (mf_g_cp_index == MF_TA_MAX_CHECKPOINT)
		return -1;

	if (!name)
		return -1;

	if (strlen(name) == 0)
		return -1;

	cp = (mf_ta_checkpoint *) malloc(sizeof(mf_ta_checkpoint));
	if (!cp)
		return -1;

	cp->name = (char *)malloc(strlen(name) + 1);
	if (!cp->name) {
		free(cp);
		return -1;
	}
	strncpy(cp->name, name, strlen(name));

	if (show)
		SECURE_DEBUG("[CHECK-POINT] %s...(%s:%d)\n", name, filename, line);

	gettimeofday(&t, NULL);
	cp->timestamp = t.tv_sec * 1000000L + t.tv_usec;
#ifdef MF_TA_UNIT_MSEC
	cp->timestamp = (cp->timestamp >= 1000) ? cp->timestamp / 1000 : 0;
#endif

	mf_g_cps[mf_g_cp_index] = cp;

	mf_g_cp_index++;

	return 0;
}

void mf_ta_show_checkpoints(void)
{
	int i = 0;
	if (!mf_g_accums)
		return;

	SECURE_DEBUG("BEGIN RESULT ============================\n");
	for (i = 0; i < mf_g_cp_index; i++) {
		SECURE_DEBUG("[%d] %s : %ld us.\n", i, mf_g_cps[i]->name, mf_g_cps[i]->timestamp);
	}
	SECURE_DEBUG("END RESULT   ============================\n");
}

void mf_ta_show_diff(char *name1, char *name2)
{
	if (!mf_g_accums)
		return;


	SECURE_DEBUG("Time takes from [%s] to [%s] : %ld us.\n", name1, name2, mf_ta_get_diff(name1, name2));
}

unsigned long mf_ta_get_diff(char *name1, char *name2)
{
	int cp1, cp2;

	if (!mf_g_accums)
		return 0;


	/* fail if bad param. */
	if (!name1 || !name2)
		return -1;

	/* fail if same. */
	if (strcmp(name1, name2) == 0)
		return -1;

	/*get index */
	if ((cp1 = __mf_ta_get_cp_index(name1)) == -1)
		return -1;

	if ((cp2 = __mf_ta_get_cp_index(name2)) == -1)
		return -1;

	/* NOTE :
	 * return value must be positive value.
	 * bcz the value of higher index of mf_g_cps always higher than lower one.
	 */
	return mf_g_cps[cp2]->timestamp - mf_g_cps[cp1]->timestamp;

}

static int __mf_ta_get_cp_index(char *name)
{
	int i;

	assert(name);

	/* find index */
	for (i = 0; i < mf_g_cp_index; i++) {
		if (strcmp(name, mf_g_cps[i]->name) == 0)
			return i;
	}

	return -1;
}

static int __mf_ta_get_accum_index(char *name)
{
	int i;

	assert(name);

	/* find index */
	for (i = 0; i < mf_g_accum_index; i++) {
		if (strcmp(name, mf_g_accums[i]->name) == 0)
			return i;
	}

	return -1;
}

static void __mf_ta_free_cps(void)
{
	int i = 0;

	if (!mf_g_cps)
		return;

	for (i = 0; i < mf_g_cp_index; i++) {
		if (mf_g_cps[i]) {
			if (mf_g_cps[i]->name)
				free(mf_g_cps[i]->name);

			free(mf_g_cps[i]);

			mf_g_cps[i] = NULL;
		}
	}

	free(mf_g_cps);
	mf_g_cps = NULL;

	mf_g_cp_index = 0;
}

static void __mf_ta_free_accums(void)
{
	int i = 0;

	if (!mf_g_accums)
		return;

	for (i = 0; i < mf_g_accum_index; i++) {
		if (mf_g_accums[i]) {
			if (mf_g_accums[i]->name)
				free(mf_g_accums[i]->name);

			free(mf_g_accums[i]);

			mf_g_accums[i] = NULL;
		}
	}

	mf_g_accum_index = 0;
	mf_g_accum_longest_name = 0;

	free(mf_g_accums);
	mf_g_accums = NULL;
}


int mf_ta_accum_item_begin(char *name, int show, char *filename, int line)
{
	mf_ta_accum_item *accum = NULL;
	int index = 0;
	int name_len = 0;
	struct timeval t;

	if (!mf_g_enable)
		return -1;

	if (!mf_g_accums)
		return 0;



	if (mf_g_accum_index == MF_TA_MAX_ACCUM)
		return -1;

	if (!name)
		return -1;

	name_len = strlen(name);
	if (name_len == 0)
		return -1;

	/* if 'name' is new one. create new item. */
	if ((index = __mf_ta_get_accum_index(name)) == -1) {
		accum = (mf_ta_accum_item *) malloc(sizeof(mf_ta_accum_item));
		if (!accum)
			return -1;

		/*clear first. */
		memset(accum, 0, sizeof(mf_ta_accum_item));
		accum->elapsed_min = 0xFFFFFFFF;

		accum->name = (char *)malloc(name_len + 1);
		if (!accum->name) {
			free(accum);
			return -1;
		}
		memset(accum->name, 0, name_len + 1);
		strncpy(accum->name, name, strlen(name));

		/* add it to list. */
		mf_g_accums[mf_g_accum_index] = accum;
		mf_g_accum_index++;

		if (mf_g_accum_longest_name < name_len)
			mf_g_accum_longest_name = name_len;

	} else {
		accum = mf_g_accums[index];
	}

	/*verify pairs of begin, end. */
	if (accum->on_estimate) {
		SECURE_DEBUG("[%s] is not 'end'ed!\n", accum->name);
		accum->num_unpair++;
		return -1;
	}
	/*get timestamp */
	gettimeofday(&t, NULL);
	accum->timestamp = t.tv_sec * 1000000L + t.tv_usec;
#ifdef MF_TA_UNIT_MSEC
	accum->timestamp = (accum->timestamp >= 1000) ? accum->timestamp / 1000 : 0;
#endif
	accum->on_estimate = 1;

	if (accum->first_start == 0) {	/* assum that timestamp never could be zero. */
		accum->first_start = accum->timestamp;

		if (mf_g_accum_first_time > accum->first_start)
			mf_g_accum_first_time = accum->first_start;
	}

	if (show)
		SECURE_DEBUG("[ACCUM BEGIN] %s : %ld ---(%s:%d)\n", name, accum->timestamp, filename, line);

	accum->num_calls++;

	return 0;
}

int mf_ta_accum_item_end(char *name, int show, char *filename, int line)
{
	mf_ta_accum_item *accum = NULL;
	unsigned int tval = 0;
	int index = 0;
	struct timeval t;

	if (!mf_g_enable)
		return -1;

	if (!mf_g_accums)
		return 0;


	/* get time first for more accuracy. */
	gettimeofday(&t, NULL);

	if (mf_g_accum_index == MF_TA_MAX_ACCUM)
		return -1;

	if (!name)
		return -1;

	if (strlen(name) == 0)
		return -1;

	/* varify the 'name' is already exist. */
	if ((index = __mf_ta_get_accum_index(name)) == -1) {
		SECURE_DEBUG("[%s] is not added before!\n", name);
		return -1;
	}

	accum = mf_g_accums[index];

	/* verify pairs of begin, end. */
	if (!accum->on_estimate) {
		SECURE_DEBUG("[%s] is not 'begin' yet!\n", accum->name);
		accum->num_unpair++;
		return -1;
	}
	/* get current timestamp. */
	tval = t.tv_sec * 1000000L + t.tv_usec;
#ifdef MF_TA_UNIT_MSEC
	tval = (tval >= 1000) ? tval / 1000 : 0;
#endif

	/* update last_end */
	accum->last_end = tval;

	/* make get elapsed time. */
	tval = tval - accum->timestamp;

	/* update min/max */
	accum->elapsed_max = tval > accum->elapsed_max ? tval : accum->elapsed_max;
	accum->elapsed_min = tval < accum->elapsed_min ? tval : accum->elapsed_min;

	if (show)
		SECURE_DEBUG("[ACCUM END] %s : %ld + %u ---(%s:%d)\n", name, accum->elapsed_accum, tval, filename, line);

	/* add elapsed time */
	accum->elapsed_accum += tval;
	accum->on_estimate = 0;

	return 0;
}

void __mf_ta_print_some_info(FILE *fp)
{
	if (!fp)
		return;

	/* comment */
	{
		fprintf(fp, "\nb~ b~ b~\n\n");
	}

	/* General infomation */
	{
		time_t t_val;
		char hostname[MF_TA_BUFF_SIZE] = { '\0', };
		char buf[MF_TA_BUFF_SIZE] = {'\0', };
		struct utsname uts;
		struct rusage r_usage;

		fprintf(fp, "\n[[ General info ]]\n");

		/* time and date */
		time(&t_val);
		ctime_r(&t_val, buf);
		fprintf(fp, "Date : %s", buf);

		/* system */
		if (gethostname(hostname, 255) == 0 && uname(&uts) >= 0) {
			fprintf(fp, "Hostname : %s\n", hostname);
			fprintf(fp, "System : %s\n", uts.sysname);
			fprintf(fp, "Machine : %s\n", uts.machine);
			fprintf(fp, "Nodename : %s\n", uts.nodename);
			fprintf(fp, "Release : %s \n", uts.release);
			fprintf(fp, "Version : %s \n", uts.version);
		}
		/* process info. */
		fprintf(fp, "Process priority : %d\n", getpriority(PRIO_PROCESS, getpid()));
		getrusage(RUSAGE_SELF, &r_usage);
		fprintf(fp, "CPU usage : User = %ld.%06ld, System = %ld.%06ld\n",
			r_usage.ru_utime.tv_sec, r_usage.ru_utime.tv_usec, r_usage.ru_stime.tv_sec, r_usage.ru_stime.tv_usec);


	}

	/* host environment variables */
	{
		extern char **environ;
		char **env = environ;

		fprintf(fp, "\n[[ Host environment variables ]]\n");
		while (*env) {
			fprintf(fp, "%s\n", *env);
			env++;
		}
	}
}

void mf_ta_show_accum_result(int direction)
{
	int i = 0;
	char format[256];
	FILE *fp = stderr;

	if (!mf_g_accums)
		return;

	switch (direction) {
	case MF_TA_SHOW_STDOUT:
		fp = stdout;
		break;
	case MF_TA_SHOW_STDERR:
		fp = stderr;
		break;
	case MF_TA_SHOW_FILE:
		{
			fp = fopen(MF_TA_RESULT_FILE, "wt");
			if (!fp)
				return;
		}
	}
	__mf_ta_print_some_info(fp);

#ifdef MF_TA_UNIT_MSEC
	snprintf(format, sizeof(format),
		"[%%3d] %%-%ds | \ttotal : %%4ld\tcalls : %%3ld\tavg : %%4ld\tmin : %%4ld\tmax : %%4ld\tstart : %%4lu\tend : %%4lu\tunpair : %%3ld\n",
		mf_g_accum_longest_name);
	fprintf(fp, "BEGIN RESULT ACCUM============================ : NumOfItems : %d, unit(msec)\n", mf_g_accum_index);
#else
	snprintf(format, sizeof(format),
		"[%%3d] %%-%ds : \ttotal : %%ld\t:calls : %%ld\t:avg : %%ld\tmin : %%ld\tmax : %%ld\tstart : %%lu\tend : %%lu\tunpair : %%ld\n",
		mf_g_accum_longest_name);
	fprintf(fp, "BEGIN RESULT ACCUM============================ : NumOfItems : %d, unit(usec)\n", mf_g_accum_index);
#endif

	for (i = 0; i < mf_g_accum_index; i++) {
		/*prevent 'devide by zero' error */
		if (mf_g_accums[i]->num_calls == 0)
			mf_g_accums[i]->num_calls = 1;

		fprintf(fp, format, i, mf_g_accums[i]->name, mf_g_accums[i]->elapsed_accum, mf_g_accums[i]->num_calls,
			(mf_g_accums[i]->elapsed_accum == 0) ? 0 : (int)(mf_g_accums[i]->elapsed_accum / mf_g_accums[i]->num_calls),
			mf_g_accums[i]->elapsed_min,
			mf_g_accums[i]->elapsed_max,
			mf_g_accums[i]->first_start - mf_g_accum_first_time,
			mf_g_accums[i]->last_end - mf_g_accum_first_time, mf_g_accums[i]->num_unpair);
	}
	fprintf(fp, "END RESULT ACCUM  ============================\n");

	if (direction == MF_TA_SHOW_FILE)
		fclose(fp);
}
#endif
