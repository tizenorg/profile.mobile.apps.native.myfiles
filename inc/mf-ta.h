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

#ifndef _MF_TA_H_
#define _MF_TA_H_

#ifdef MYFILE_USE_TA

/* defs. */
#define MF_TA_MAX_CHECKPOINT	500
#define MF_TA_MAX_ACCUM		500
#define MF_TA_BUFF_SIZE		256

typedef struct _mf_ta_checkpoint mf_ta_checkpoint;
struct _mf_ta_checkpoint {
	unsigned long timestamp;
	char *name;
};

typedef struct _mf_ta_accum_item mf_ta_accum_item;
struct _mf_ta_accum_item {
	unsigned long elapsed_accum;
	unsigned long num_calls;
	unsigned long elapsed_min;
	unsigned long elapsed_max;
	unsigned long first_start;
	unsigned long last_end;

	char *name;

	unsigned long timestamp;
	int on_estimate;
	int num_unpair;
};

#define MF_TA_SHOW_STDOUT	0
#define MF_TA_SHOW_STDERR	1
#define MF_TA_SHOW_FILE	2
#define MF_TA_RESULT_FILE "/tmp/myfile-ta.log"


/* COMMON */
int mf_ta_init(void);
int mf_ta_release(void);
void mf_ta_set_enable(int enable);
char *mf_ta_fmt(const char *fmt, ...);


/* CHECK POINT */
int mf_ta_add_checkpoint(char *name, int show, char *filename, int line);
void mf_ta_show_checkpoints(void);
void mf_ta_show_diff(char *name1, char *name2);

int mf_ta_get_numof_checkpoints();
unsigned long mf_ta_get_diff(char *name1, char *name2);

/* ACCUM ITEM */
int mf_ta_accum_item_begin(char *name, int show, char *filename, int line);
int mf_ta_accum_item_end(char *name, int show, char *filename, int line);
void mf_ta_show_accum_result(int direction);

/* macro. */
#define MF_TA_INIT()				(mf_ta_init())
#define MF_TA_RELEASE()				(mf_ta_release())
#define MF_TA_SET_ENABLE(enable)		(mf_ta_set_enable(enable))

/* checkpoint handling */
#define MF_TA_ADD_CHECKPOINT(name, show)	(mf_ta_add_checkpoint(name, show, __FILE__, __LINE__))
#define MF_TA_SHOW_CHECKPOINTS()		(mf_ta_show_checkpoints())
#define MF_TA_SHOW_DIFF(name1, name2)		(mf_ta_show_diff(name1, name2))
#define MF_TA_GET_NUMOF_CHECKPOINTS()		(mf_ta_get_numof_checkpoints())
#define MF_TA_GET_DIFF(name1, name2)		(mf_ta_get_diff(name1, name2))

/* accum item handling */
#define MF_TA_ACUM_ITEM_BEGIN(name, show)	(mf_ta_accum_item_begin(name, show, __FILE__, __LINE__))
#define MF_TA_ACUM_ITEM_END(name, show)		(mf_ta_accum_item_end(name, show, __FILE__, __LINE__))
#define MF_TA_ACUM_ITEM_SHOW_RESULT()		(mf_ta_show_accum_result(MF_TA_SHOW_STDOUT))
#define MF_TA_ACUM_ITEM_SHOW_RESULT_TO(x)	(mf_ta_show_accum_result(x))
/*
#define __mf_ta__(name, x) \
MF_TA_ACUM_ITEM_BEGIN(name, 0); \
x \
MF_TA_ACUM_ITEM_END(name, 0);

*/

#else /*#ifdef MYFILE_USE_TA*/

#define MF_TA_INIT()
#define MF_TA_RELEASE()
#define MF_TA_SET_ENABLE(enable)

/* checkpoint handling */
#define MF_TA_ADD_CHECKPOINT(name, show)
#define MF_TA_SHOW_CHECKPOINTS()
#define MF_TA_SHOW_DIFF(name1, name2)
#define MF_TA_GET_NUMOF_CHECKPOINTS()
#define MF_TA_GET_DIFF(name1, name2)
/* #define MF_TA_GET_NAME(idx) */

/* accum item handling */
#define MF_TA_ACUM_ITEM_BEGIN(name, show)
#define MF_TA_ACUM_ITEM_END(name, show)
#define MF_TA_ACUM_ITEM_SHOW_RESULT()
#define MF_TA_ACUM_ITEM_SHOW_RESULT_TO(x)
/*
#define __mf_ta__(name, x)
*/
#endif /*#ifdef MYFILE_USE_TA */

#endif //_MF_TA_H_
