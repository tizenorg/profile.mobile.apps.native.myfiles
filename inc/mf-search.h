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

#ifndef _MF_SEARCH_H_
#define _MF_SEARCH_H_

/*+++++++++++++++++++++++ Definitions and Types +++++++++++++++++++++++*/

/**
 * Handle type for mf_search
 **/
typedef unsigned int mf_search_handle;

/**
 * Handle type for search result
 **/
typedef unsigned int mf_search_result;

typedef int (*mf_search_filter_cb) (const char *);


/**
 * Enumerations of search option
 **/

typedef enum _mf_search_option mf_search_option;

enum _mf_search_option {
	MF_SEARCH_OPT_NONE = (1 << 0),
	MF_SEARCH_OPT_HIDDEN = (1 << 1),
	MF_SEARCH_OPT_DIR = (1 << 2),
	MF_SEARCH_OPT_FILE = (1 << 3),
};

typedef enum _mf_search_pipe_msg_type mf_search_pipe_msg_type;
enum _mf_search_pipe_msg_type {
	MF_SEARCH_PIPE_MSG_NONE = 0,
	MF_SEARCH_PIPE_MSG_ROOT_CHANGE,
	MF_SEARCH_PIPE_MSG_DOING,
	MF_SEARCH_PIPE_MSG_RESULT_REPORT,
	MF_SEARCH_PIPE_MSG_FINISHED,
	MF_SEARCH_PIPE_MSG_MAX,
};

typedef enum _mf_search_state mf_search_state;
enum _mf_search_state {
	MF_SEARCH_STATE_NONE = 0,
	MF_SEARCH_STATE_INIT,
	MF_SEARCH_STATE_SEARCH,
	MF_SEARCH_STATE_MAX,
};

typedef struct _mf_search_result_t mf_search_result_t;
struct _mf_search_result_t {
	GList *dir_list;
	GList *file_list;
	gchar *current_dir;
	guint total_count;
	gboolean is_end;
};

typedef struct _ms_args_t ms_args_t;
struct _ms_args_t {
	GList *root_path;
	gchar *needle;
	mf_search_option option;
	void *user_data;
	mf_search_filter_cb func;
	int category;
	int item_count;
} ;

typedef struct _ms_handle_t ms_handle_t;
struct _ms_handle_t {
	mf_search_state state;
	GMutex cmd_lock;
	ms_args_t *args;

	GThread *thread_h;
	GMutex thread_mutex;
	/* critical section */
	gboolean is_stop;
	mf_search_result_t *result;
	/* critical section */
};

typedef struct _mf_search_pipe_msg mf_search_pipe_msg;
struct _mf_search_pipe_msg {
	mf_search_pipe_msg_type mf_sp_msg_type;
	void *report_result;
	gchar *current_path;
};

/**
 * mf_Search_Cb:
 * @result: the handle of result, use util APIs to get detail result with this handle.
 * @user_data: user data specified when installing the function, in mf_search_start()
 **/
typedef void (*mf_Search_Cb) (mf_search_pipe_msg_type type, mf_search_result result, void *user_data);

/**
 * Definition of error code
 **/
#define MF_SEARCH_ERROR_NONE		(0)
#define MF_SEARCH_ERROR_INTERNAL	(-(1))	/* Internal error */
#define MF_SEARCH_ERROR_INVAL_P		(-(2))	/* Invalid params */
#define MF_SEARCH_ERROR_INVAL_S		(-(3))	/* Invalid status */
#define MF_SEARCH_ERROR_ALLOC		(-(4))	/* Memory allocation failed */
#define MF_SEARCH_ERROR_FS		(-(5))	/* File system error */

/*+++++++++++++++++++++++ APIs +++++++++++++++++++++++*/

/**
 * mf_search_init:
 * @handle: the handle of mf_search
 * Creates a new @handle for search. If success,
 * #mf_search state is changed from MF_SEARCH_STATE_NONE to MF_SEARCH_STATE_INIT
 * Return value: This function returns zero on success, or negative value.
 **/
int mf_search_init(mf_search_handle *handle);

/**
 * mf_search_start:
 * @handle: the handle of mf_search
 * @root_path: array of the root path for search
 * @path_num: the number of the root path for search
 * @needle: the key string for search
 * @option :  bitfield of mf_search_option flags
 * @user_data: user data
 * Start searching in given @root_path with @needle,
 * every each idle time, @callback will be called with #mf_search_result_t and @user_data.
 * If success, #mf_search state is changed from MF_SEARCH_STATE_INIT to MF_SEARCH_STATE_SEARCH
 * Return value: This function returns zero on success, or negative value.
 **/
int mf_search_start(mf_search_handle handle,
		    const char **root_path,
		    unsigned int path_num,
		    const char *needle,
		    mf_search_option option,
		    void *user_data,
		    mf_search_filter_cb func,
		    int category,
		    int item_count);
/**
 * mf_search_stop:
 * @handle: the handle of mf_search
 * Stops search
 * If success, #mf_search state is changed from MF_SEARCH_STATE_SEARCH to MF_SEARCH_STATE_INIT
 * Return value: This function returns zero on success, or negative value.
 **/
int mf_search_stop(mf_search_handle handle);

/**
 * mf_search_stop:
 * @handle: the handle of mf_search
 * Finalizes search @handle
 * #mf_search state is changed from MF_SEARCH_STATE_INIT to MF_SEARCH_STATE_NONE
 **/
void mf_search_finalize(mf_search_handle *handle);


/*+++++++++++++++++++++++ UTIL APIs +++++++++++++++++++++++*/

/**
 * mf_search_result_dir_get:
 * @result: the handle of search result
 * Gets one of directory name in given search @result
 * Return value: a directory name which is a newly-allocated string that must be freed after use
 * or NULL if no more result for directory.
 **/
char *mf_search_result_dir_get(mf_search_result_t *result);

/**
 * mf_search_result_file_get:
 * @result: the handle of search result
 * Gets one of file name given search @result
 * Return value: a file name which is a newly-allocated string that must be freed after use
 * or NULL if no more result for directory.
 **/
char *mf_search_result_file_get(mf_search_result_t *result);

/**
 * mf_search_result_current_dir_get:
 * @result: the handle of search result
 * Gets current searching directory name in given search @result
 * Return value: current searching directory name which is a newly-allocated string that must be freed after use
 * or NULL if fail to get current searching directory name.
 **/
char *mf_search_result_current_dir_get(mf_search_result_t *result);

/**
 * mf_search_result_is_end:
 * @result: the handle of search result
 * @is_end : If @result is last result handle, set it to a non-zero value, if not set it to zero.
 * Tests if given search @result is the last one or not
 * Return value: This function returns zero on success, or negative value.
 **/
int mf_search_result_is_end(mf_search_result_t *result, int *is_end);

/**
 * mf_search_result_total_count_get:
 * @result: the handle of search result
 * @count: the items(which is explored directories and files) count.
 * Gets current explored items(this is not result count)
 * Return value: This function returns zero on success, or negative value.
 **/
int mf_search_result_total_count_get(mf_search_result_t *result, unsigned int *count);

void mf_search_bar_item_append(void *data, void *user_data);

#endif //_MF_SEARCH_H_
