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

#include <string.h>
#include <assert.h>

#include <Eina.h>
#include "sqlite3.h"

#include "mf-media-error.h"
#include "mf-media-types.h"
#include "mf-media.h"
#include "mf-media-db.h"
#include "mf-fs-util.h"
#include "mf-dlog.h"

#define MF_DB_NAME  "/opt/usr/apps/org.tizen.myfile/data/.myfile_media.db"

#define MF_PRAGMA_FOREIGN_KEYS_ON               "PRAGMA foreign_keys = ON;"
#define MF_SELECT_FROM_SHORTCUT_TABLE           "SELECT * FROM %s WHERE (%s='%q' and %s=%d );"
#define MF_SELECT_DISPLAY_NAME_FROM_SHORTCUT_TABLE 			"SELECT * FROM %s WHERE (%s='%q');"
#define MF_INSERT_INTO_SHORTCUT_TABLE           "INSERT INTO %s (%s, %s, %s) VALUES ('%q', %Q, %d);"
#define MF_DELETE_FROM_SHORTCUT_TABLE           "DELETE FROM %s WHERE %s = '%q';"
#define MF_DELETE_BY_TYPE_FROM_SHORTCUT_TABLE   "DELETE FROM %s WHERE %s = %d;"
#define MF_SELECT_SHORTCUT_TABLE				"SELECT * FROM %s;"
#define MF_SELECT_SHORTCUT_COUNT_TABLE          "SELECT count(*) FROM %s;"
#define MF_SELECT_SHORTCUT_NAME				    "SELECT %s FROM %s where %s = '%q';"
#define MF_UPDATE_SHORTCUT_NAME	 			    "UPDATE %s SET %s = '%q' WHERE (%s = '%q');"
#define MF_INSERT_INTO_RECENT_FILES_TABLE       "INSERT INTO %s (%s, %s, %s, %s) VALUES (?, ?, ?, ?);"
#define MF_DELETE_FROM_RECENT_FILES_TABLE       "DELETE FROM %s WHERE %s = '%q';"
#define MF_UPDATE_SET_RECENT_FILES_TABLE	 	"UPDATE %s SET %s = '%q' WHERE (%s = '%q');"
#define MF_UPDATE_FAVORATE_FILES_TABLE	 		"UPDATE %s SET %s = '%q' WHERE (%s = '%q');"
#define MF_SELECT_RECENT_FILES_TABLE			"SELECT * FROM %s;"
#define MF_SELECT_RECENT_FILES_COUNT_TABLE      "SELECT count(*) FROM %s;"
#define MF_DELETE_ALL_FROM_TABLE 			    "DELETE FROM %s;"
#define MF_DELETE_BY_TYPE_FROM_RECENT_FILES_TABLE   "DELETE FROM %s WHERE %s = %d;"
#define MF_SELECT_FROM_RECENT_FILE_TABLE  		"SELECT * FROM %s WHERE (%s='%q');"
//#define MF_SELECT_FROM_SHORTCUT_TABLE 			"SELECT * FROM %s WHERE (%s='%q' and %s=%Q and %s=%d );"

static sqlite3_callback sqlite3_func = NULL;
static void *func_params = NULL;

static void __mf_media_db_cb_set(sqlite3_callback func)
{
	sqlite3_func = func;
}

static void __mf_media_db_cb_params_set(void *params)
{
	func_params = params;
}

typedef enum {
	MF_TABLE_NONE = -1,
	MF_TABLE_SHORTCUT,
	MF_TABLE_RECENT_FILES,
	MF_TABLE_NUM,
} mf_tbl_name_e;

typedef enum {
	MF_FIELD_SHORTCUT_NONE		= -1,
	MF_FIELD_SHORTCUT_PATH,
	MF_FIELD_SHORTCUT_NAME,
	MF_FIELD_SHORTCUT_STORAGE_TYPE,
	MF_FIELD_SHORTCUT_NUM,
} mf_field_shortcut_e;

typedef enum {
	MF_FIELD_RECENT_FILES_NONE		= -1,
	MF_FIELD_RECENT_FILES_PATH,
	MF_FIELD_RECENT_FILES_NAME,
	MF_FIELD_RECENT_FILES_STORAGE_TYPE,
	MF_FIELD_RECENT_FILES_THUMBNAIL,
	MF_FIELD_RECENT_FILES_NUM,
} mf_field_recent_files_e;

typedef struct {
	 char *field_name;
	 char *field_type;
} mf_tbl_field_s;

typedef struct {
	char *table_name;
	mf_tbl_field_s mf_tbl_field[MF_FIELD_RECENT_FILES_NUM+1];
} mf_tbl_s;

mf_tbl_s mf_tbl[MF_TABLE_NUM] = 
{
    {"shortcut", 
        {
            {"path", ""}	/* PK */
            ,
                {"name", ""}	/* PK */
            ,
                {"storage_type", ""}	/* PK */
        }
    },
    {"recent_files", 
        {
            {"path", ""}	/* PK */
            ,
                {"name", ""}
            ,
                {"storage_type", ""}
            ,
                {"thumbnail_path", ""}
        }
    }
};

int __get_shortcut_name_cb(void *data, int column, char **column_value, char **column_name)
{
	if (data == NULL)
		return 0;

	char **result = (char **)data;
	if (*column_value) {
		*result = g_strdup(*column_value);
	}
	mf_error(" column is [%d] column_value is [%s] column_name is [%s]", column, *column_value, *column_name);
	return 0;
}


static int __mf_busy_handler(void *pData, int count)
{
	usleep(50000);

	mf_debug("web_media_busy_handler called : %d", count);

	return 100 - count;
}

int mf_sqlite3_exec(sqlite3 *p_db,                             /* An open database */
		             const char *sql,                           /* SQL to be evaluated */
		             int (*callback)(void*,int,char**,char**),  /* Callback function */
		             void * params,                             /* 1st argument to callback */
		             char **errmsg)                            /* Error msg written here */
{
	mf_debug("mf_sqlite3_exec enter\n");
	sqlite3_stmt* p_statement = NULL;
	int result = sqlite3_prepare_v2(p_db, sql, -1, &p_statement, NULL);
	if (result != SQLITE_OK) {
		mf_error("sqlite3_prepare_v2 error result=%d", result);
		return result;
	}
	result = sqlite3_step(p_statement);
	if (callback) {
		callback(p_statement, 0, NULL, NULL);
	}

	result = sqlite3_finalize(p_statement);
	if (result != SQLITE_OK) {
		mf_error("sqlite3_finalize error result=%d", result);
	}
	mf_debug("mf_sqlite3_exec leave result=%d", result);
	return result;
}

static int __mf_sqlite3_commit_trans(MFDHandle *mfd_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mfd_handle;
	if (handle == NULL) {
		mf_debug("handle is NULL");
		return MFD_ERROR_DB_INTERNAL;
	}

	mf_debug("gm_sqlite3_commit_trans enter\n");
	if (SQLITE_OK != mf_sqlite3_exec(handle, "COMMIT;", sqlite3_func, func_params, &err_msg)) {
		if (err_msg) {
			mf_debug("Error:failed to end transaction: error=%s\n",
				     err_msg);
			sqlite3_free(err_msg);
		}
		return MFD_ERROR_DB_INTERNAL;
	}
	if (err_msg)
		sqlite3_free(err_msg);
	mf_debug("gm_sqlite3_commit_trans leave\n");
	return 0;
}


static int __mf_query_bind_text(sqlite3_stmt *stmt, int pos, const char *str)
{
	assert(NULL != stmt);

	if (str)
		return sqlite3_bind_text(stmt, pos, (const char*)str, strlen(str), SQLITE_STATIC);
	else
		return sqlite3_bind_null(stmt, pos);
}

static int __mf_query_bind_int(sqlite3_stmt *stmt, int pos, int num)
{
	assert(NULL != stmt);
	assert(pos > -1);
	return sqlite3_bind_int(stmt, pos, num);
}

static char *__mf_query_table_column_text(sqlite3_stmt *stmt, int pos)
{
	assert(NULL != stmt);
	assert(pos > -1);
	mf_error("try to get the value");
	return (char *)sqlite3_column_text(stmt, pos);
}

static int __mf_query_table_column_int(sqlite3_stmt *stmt, int pos)
{
	assert(NULL != stmt);
	assert(pos > -1);
	return sqlite3_column_int(stmt, pos);
}

static void __mf_data_to_text(char *textbuf, char **output)
{
	if (textbuf && strlen(textbuf)>0) {
		if (*output) {
			free(*output);
			*output = NULL;
		}
		*output = strdup(textbuf);
	}
}


static int __mf_query_sql(MFDHandle *mfd_handle, char *query_str)
{
	int err = -1;
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mfd_handle;
	if (handle == NULL) {
		mf_debug("handle is NULL");
		return MFD_ERROR_DB_INTERNAL;
	}

	mf_debug("SQL = %s\n", query_str);

	err = mf_sqlite3_exec(handle, query_str, sqlite3_func, func_params, &err_msg);
	if (SQLITE_OK != err) {
		if (err_msg) {
			mf_debug("failed to query[%s]", err_msg);
			sqlite3_free(err_msg);
		}
		mf_debug("Query fails : query_string[%s]", query_str);
		return MFD_ERROR_DB_INTERNAL;
	}

	if (err_msg)
		sqlite3_free(err_msg);
	mf_debug("query success\n");

	return err;
}

static int __mf_sqlite3_begin_trans(MFDHandle *mfd_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mfd_handle;
	if (handle == NULL) {
		mf_debug("handle is NULL");
		return MFD_ERROR_DB_INTERNAL;
	}

	mf_debug("gm_sqlite3_begin_trans enter\n");
	if (SQLITE_OK !=
	    mf_sqlite3_exec(handle, "BEGIN IMMEDIATE;", NULL, NULL, &err_msg)) {
	    if (err_msg) {
			mf_debug("Error:failed to begin transaction: error=%s\n",
				     err_msg);
			sqlite3_free(err_msg);
	    }
		return MFD_ERROR_DB_INTERNAL;
	}
	if (err_msg)
		sqlite3_free(err_msg);
	mf_debug("gm_sqlite3_begin_trans leave\n");
	return 0;
}

static int __mf_sqlite3_rollback_trans(MFDHandle *mfd_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mfd_handle;
	if (handle == NULL) {
		mf_debug("handle is NULL");
		return MFD_ERROR_DB_INTERNAL;
	}

	mf_debug("gm_sqlite3_rollback_trans enter\n");
	if (SQLITE_OK !=
	    mf_sqlite3_exec(handle, "ROLLBACK;", NULL, NULL, &err_msg)) {
	    if (err_msg) {
			mf_debug("Error:failed to rollback transaction: error=%s\n",
				     err_msg);
			sqlite3_free(err_msg);
	    }
		return MFD_ERROR_DB_INTERNAL;
	}
	if (err_msg)
		sqlite3_free(err_msg);
	mf_debug("gm_sqlite3_rollback_trans leave\n");
	return 0;
}

static void __mf_convert_shortcut_column_to_sitem(sqlite3_stmt *stmt, MFSitem *sitem)
{
	char *textbuf = NULL;
	int storage_type = 0;

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_SHORTCUT_PATH);
	__mf_data_to_text(textbuf, &(sitem->path));


	storage_type = __mf_query_table_column_int(stmt, MF_FIELD_SHORTCUT_STORAGE_TYPE);
	sitem->storage_type = storage_type;
}

static void __mf_convert_shortcut_display_name(void *data, int column, char **column_value, char **column_name)
{
	char *display_name = NULL;

	display_name = __mf_query_table_column_text(data, 0);
	mf_error("========== display name is [%s]", display_name);
	if (display_name && func_params) {
		char **result = (char **)func_params;
		*result = g_strdup(display_name);
	}
}

static void __mf_foreach_shortcut_sitem_cb(mf_shortcut_item_cb callback, void *data, void *user_data)
{
	Eina_List *list = (Eina_List *)data;
	Eina_List *iter = NULL;

	for (iter = list; iter != NULL; iter = eina_list_next(iter)) {
		MFSitem *sitem = NULL;
		sitem = (MFSitem *)iter->data;

		if (callback(sitem, user_data) == FALSE)
			break;
	}
}

static void __mf_free_shortcut_list(void *data)
{
	mf_destroy_shortcut_item(data);
}

static void __mf_convert_recent_files_column_to_citem(sqlite3_stmt *stmt, MFRitem *ritem)
{
	char *textbuf = NULL;

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_RECENT_FILES_PATH);
	__mf_data_to_text(textbuf, &(ritem->path));

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_RECENT_FILES_NAME);
	__mf_data_to_text(textbuf, &(ritem->name));

	ritem->storyage_type = __mf_query_table_column_int(stmt, MF_FIELD_RECENT_FILES_STORAGE_TYPE);

	textbuf = __mf_query_table_column_text(stmt, MF_FIELD_RECENT_FILES_THUMBNAIL);
	__mf_data_to_text(textbuf, &(ritem->thumbnail));

}

static void __mf_foreach_recent_files_ritem_cb(mf_recent_files_item_cb callback, void *data, void *user_data)
{
	Eina_List *list = (Eina_List *)data;
	Eina_List *iter = NULL;

	for (iter = list; iter != NULL; iter = eina_list_next(iter)) {
		MFRitem *ritem = NULL;
		ritem = (MFRitem *)iter->data;

		if (callback(ritem, user_data) == FALSE)
			break;
	}
}

static void __mf_free_recent_files_list(void *data)
{
	mf_destroy_recent_files_item(data);
}

static void __mf_media_db_eina_list_free_full(Eina_List **list, void (*func)(void *data)) {
	mf_retm_if(*list == NULL, "list is NULL");

	void *pNode = NULL;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(*list, l, pNode) {
		func(pNode);
	}
	eina_list_free(*list);
	*list = NULL;
}

int mf_connect_db_with_handle(sqlite3 **db_handle)
{
	int ret = MFD_ERROR_NONE;

	if (db_handle == NULL) {
		mf_debug("error invalid arguments");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	/*Connect DB*/
	ret = sqlite3_open(MF_DB_NAME, db_handle);
	if (SQLITE_OK != ret || *db_handle == NULL) {
		mf_debug("error when db open");
		*db_handle = NULL;
		return MFD_ERROR_DB_CONNECT;
	}
	/*Register busy handler*/
	ret = sqlite3_busy_handler(*db_handle, __mf_busy_handler, NULL);
	if (SQLITE_OK != ret) {

		if (*db_handle) {
			mf_debug("[error when register busy handler] %s\n", sqlite3_errmsg(*db_handle));
		}

		ret = sqlite3_close(*db_handle);
		*db_handle = NULL;

		return MFD_ERROR_DB_CONNECT;
	}

	/* set foreign_keys */
	char *query_string = NULL;
	query_string =
	    sqlite3_mprintf(MF_PRAGMA_FOREIGN_KEYS_ON);

	mf_debug("Query : %s", query_string);

	ret = __mf_query_sql(*db_handle, query_string);

	sqlite3_free(query_string);

	return MFD_ERROR_NONE;
}

int mf_disconnect_db_with_handle(sqlite3 *db_handle)
{
	int ret = MFD_ERROR_NONE;

	ret = sqlite3_close(db_handle);
	if (SQLITE_OK != ret) {
		mf_debug("error when db close");
		mf_debug("Error : %s", sqlite3_errmsg(db_handle));
		db_handle = NULL;
		return MFD_ERROR_DB_DISCONNECT;
	}

	return MFD_ERROR_NONE;
}

//1 Shortcut

int mf_update_shortcut(MFDHandle *mfd_handle,const char *new_name, char *old_name)
{
	if (new_name == NULL) {
		mf_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	mf_error("mf_update_shortcut");
	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_UPDATE_FAVORATE_FILES_TABLE,
			    mf_tbl[field_seq].table_name,
			    //mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    new_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    old_name);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("Inserting device table failed\n");
		mf_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}

/* No shortcut : 0, find Shortcut : 1 */
int mf_find_shortcut(MFDHandle *mfd_handle, const char *shortcut_path, const char *shortcut_name,
                     int storage_type)
{
	mf_debug("");

	if (shortcut_path == NULL) {
		mf_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;
	int find = 0;
	query_string = sqlite3_mprintf(MF_SELECT_FROM_SHORTCUT_TABLE,
				mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    shortcut_path,
			   // mf_tbl_field[MF_FIELD_SHORTCUT_NAME].field_name,
			   // shortcut_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_STORAGE_TYPE].field_name,
			    storage_type);

	mf_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		mf_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		mf_debug("Query fails : query_string[%s]", query_string);
		return find;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		mf_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return find;
	}

	while (SQLITE_ROW == rc) {
		mf_debug("Find a same shorcut");
		find = 1;
		rc = sqlite3_step(stmt);
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}
	return find;
}

int mf_find_shortcut_display_name(MFDHandle *mfd_handle,const char *shortcut_name)
{
	mf_debug("");

	if (shortcut_name == NULL) {
		mf_debug("shortcut_name is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;
	int find = 0;
	query_string = sqlite3_mprintf(MF_SELECT_DISPLAY_NAME_FROM_SHORTCUT_TABLE,
				mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_NAME].field_name,
			    shortcut_name);

	mf_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		mf_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		mf_debug("Query fails : query_string[%s]", query_string);
		return find;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		mf_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return find;
	}

	while (SQLITE_ROW == rc) {
		mf_debug("Find a same shorcut");
		find = 1;
		rc = sqlite3_step(stmt);
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}
	return find;
}

int mf_insert_shortcut(MFDHandle *mfd_handle, const char *shortcut_path, const char *shortcut_name, int storage_type)
{
	mf_debug("");

	if (shortcut_path == NULL) {
		mf_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_INSERT_INTO_SHORTCUT_TABLE,
			    mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_NAME].field_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_STORAGE_TYPE].field_name,
			    shortcut_path,
			    shortcut_name,
			    storage_type);

	mf_debug("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("Inserting device table failed\n");
		mf_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}

int mf_delete_shortcut(MFDHandle *mfd_handle, const char *shortcut_path)
{
	mf_debug("");

	if (shortcut_path == NULL) {
		mf_debug("shortcut_path is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_FROM_SHORTCUT_TABLE,
			    mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    shortcut_path);

	mf_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_delete_shortcut_by_type(MFDHandle *mfd_handle, int storage_type)
{
	mf_debug("");

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_BY_TYPE_FROM_SHORTCUT_TABLE,
			    mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_STORAGE_TYPE].field_name,
			    storage_type);

	mf_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_foreach_shortcut_list(MFDHandle *mfd_handle, mf_shortcut_item_cb callback, void *user_data)
{
	mf_debug("Enter");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string =
	    sqlite3_mprintf(MF_SELECT_SHORTCUT_TABLE,
			    mf_tbl[field_seq].table_name);

	mf_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		mf_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		mf_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		mf_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return MFD_ERROR_DB_NO_RECORD;
	}

	Eina_List *shortcut_list = NULL;
	MFSitem *sitem= NULL;

	while (SQLITE_ROW == rc) {
		sitem = (MFSitem *)calloc(1, sizeof(MFSitem));
		if (sitem) {
			__mf_convert_shortcut_column_to_sitem(stmt, sitem);
			shortcut_list = eina_list_append(shortcut_list, sitem);
		}
		rc = sqlite3_step(stmt);
		mf_debug("");
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	__mf_foreach_shortcut_sitem_cb(callback, shortcut_list, user_data);

	if (shortcut_list) {
		__mf_media_db_eina_list_free_full(&shortcut_list, __mf_free_shortcut_list);
	}

	return MFD_ERROR_NONE;
}

int mf_get_short_count(MFDHandle *mfd_handle, int *count)
{
	mf_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string =
	    sqlite3_mprintf(MF_SELECT_SHORTCUT_COUNT_TABLE,
			    mf_tbl[field_seq].table_name);

	mf_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		mf_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		mf_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		mf_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		*count = 0;
		return MFD_ERROR_DB_NO_RECORD;
	}

	*count = sqlite3_column_int(stmt, 0);
	mf_debug("count : %d", *count);

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	return MFD_ERROR_NONE;
}

int mf_get_shortcut_display_name(MFDHandle *mfd_handle, const char *fullpath, char **name)
{
	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	__mf_media_db_cb_set((sqlite3_callback)__mf_convert_shortcut_display_name);
	__mf_media_db_cb_params_set((void *)name);

	query_string =
	    sqlite3_mprintf(MF_SELECT_SHORTCUT_NAME,
	    		    mf_tbl_field[MF_FIELD_SHORTCUT_NAME].field_name,
			    mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    fullpath);

	mf_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_begin_trans failed");
		__mf_media_db_cb_set(NULL);
		__mf_media_db_cb_params_set(NULL);
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("get shortcut display name failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		__mf_media_db_cb_set(NULL);
		__mf_media_db_cb_params_set(NULL);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		__mf_media_db_cb_set(NULL);
		__mf_media_db_cb_params_set(NULL);
		return err;
	}
	__mf_media_db_cb_set(NULL);
	__mf_media_db_cb_params_set(NULL);

	return MFD_ERROR_NONE;
}

int mf_update_shortcut_display_name(MFDHandle *mfd_handle,const char *new_name, const char *old_name)
{
	if (new_name == NULL) {
		mf_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	mf_error("mf_update_shortcut");
	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_SHORTCUT;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_UPDATE_FAVORATE_FILES_TABLE,
			    mf_tbl[field_seq].table_name,
			    //mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_NAME].field_name,
			    new_name,
			    mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    old_name);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("Inserting device table failed\n");
		mf_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}

int mf_insert_recent_file(MFDHandle *mfd_handle, const char *path, const char *name, int storage_type, 
                          const char *thumbnail_path)
{
	mf_debug("");
	mf_retvm_if (path == NULL, MFD_ERROR_INVALID_PARAMETER, "path is NULL");
	//mf_retvm_if (thumbnail_path == NULL, MFD_ERROR_INVALID_PARAMETER, "path is NULL");

	sqlite3_stmt *stmt = NULL;
	int err = -1;

	char query_string[255] = {0,};
	memset(query_string, 0, sizeof(query_string));
	mf_tbl_field_s *mf_tbl_field;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	snprintf(query_string, sizeof(query_string), MF_INSERT_INTO_RECENT_FILES_TABLE,
			    mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_PATH].field_name,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_NAME].field_name,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_STORAGE_TYPE].field_name,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_THUMBNAIL].field_name);

	err = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	if (err != SQLITE_OK) {
		mf_debug("sqlite3_prepare_v2");
		goto INSERT_FAIL;
	}

	__mf_query_bind_text(stmt, 1, path);
	__mf_query_bind_text(stmt, 2, name);
	__mf_query_bind_int(stmt, 3, storage_type);
	__mf_query_bind_text(stmt, 4, thumbnail_path);

INSERT_FAIL:
	err = sqlite3_step(stmt);
	if (err != SQLITE_DONE)	{
		SECURE_DEBUG("Inserting content table failed. %s", sqlite3_errmsg(mfd_handle));
		if (SQLITE_OK != sqlite3_finalize(stmt)) {
			mf_error("sqlite3_finalize() failed.");
		}
		return MFD_ERROR_DB_INTERNAL;
	}

	if (SQLITE_OK != sqlite3_finalize(stmt)) {
		mf_error("sqlite3_finalize() failed.");
	}
	mf_debug("Query : %s", query_string);

	return MFD_ERROR_NONE;
}

int mf_delete_recent_files(MFDHandle *mfd_handle, const char *path)
{
	mf_debug("");

	if (path == NULL) {
		mf_debug("shortcut_path is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_FROM_RECENT_FILES_TABLE,
			    mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_PATH].field_name,
			    path);

	mf_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_delete_recent_files_by_type(MFDHandle *mfd_handle, int storage_type)
{
	mf_debug("");

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;

	query_string =
	    sqlite3_mprintf(MF_DELETE_BY_TYPE_FROM_RECENT_FILES_TABLE,
			    mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_STORAGE_TYPE].field_name,
			    storage_type);

	mf_debug("Query : %s", query_string);

	err = __mf_sqlite3_begin_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_begin_trans failed");
		return err;
	}

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("delete content by content_id failed.. Now start to rollback");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	err = __mf_sqlite3_commit_trans(mfd_handle);
	if (err < 0) {
		mf_debug("gm_sqlite3_commit_trans failed.. Now start to rollback\n");
		__mf_sqlite3_rollback_trans(mfd_handle);
		return err;
	}

	return MFD_ERROR_NONE;
}

int mf_update_recent_files_thumbnail(MFDHandle *mfd_handle, const char *thumbnail, const char *new_thumbnail)
{
	mf_debug("");

	if (thumbnail == NULL) {
		mf_debug("thumbnail is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}
	if (new_thumbnail == NULL) {
		mf_debug("new_thumbnail is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;

	mf_tbl_field_s *mf_tbl_field;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;
	char *query_string = NULL;

	query_string =
	    sqlite3_mprintf(MF_UPDATE_SET_RECENT_FILES_TABLE,
			    mf_tbl[field_seq].table_name,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_THUMBNAIL].field_name,
			    new_thumbnail,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_THUMBNAIL].field_name,
			    thumbnail);

	mf_debug("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("Updating content table failed");
		mf_debug("query string is %s", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}


int mf_update_recent_files_name(MFDHandle *mfd_handle,const char *new_name, char *old_name)
{
	if (new_name == NULL) {
		mf_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;


	query_string =
	    sqlite3_mprintf(MF_UPDATE_SET_RECENT_FILES_TABLE,
			    mf_tbl[field_seq].table_name,
			    //mf_tbl_field[MF_FIELD_SHORTCUT_PATH].field_name,
			    mf_tbl_field[MF_FIELD_RECENT_FILES_PATH].field_name,
			    new_name,
				mf_tbl_field[MF_FIELD_RECENT_FILES_PATH].field_name,
				old_name);

	mf_error("Query : %s", query_string);

	err = __mf_query_sql(mfd_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mf_debug("Inserting device table failed\n");
		mf_debug("query string is %s\n", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	return MFD_ERROR_NONE;
}

int mf_foreach_recent_files_list(MFDHandle *mfd_handle, mf_recent_files_item_cb callback, void *user_data)
{
	mf_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string = sqlite3_mprintf(MF_SELECT_RECENT_FILES_TABLE, mf_tbl[field_seq].table_name);

	mf_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		mf_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		mf_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		mf_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return MFD_ERROR_DB_NO_RECORD;
	}

	Eina_List *recent_files_list = NULL;
	MFRitem *ritem= NULL;

	while (SQLITE_ROW == rc) {
		ritem = (MFRitem *)calloc(1, sizeof(MFRitem));
		if (ritem) {
			__mf_convert_recent_files_column_to_citem(stmt, ritem);
			recent_files_list = eina_list_append(recent_files_list, ritem);
		}
		rc = sqlite3_step(stmt);
		mf_debug("");
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	__mf_foreach_recent_files_ritem_cb(callback, recent_files_list, user_data);

	if (recent_files_list) {
		__mf_media_db_eina_list_free_full(&recent_files_list, __mf_free_recent_files_list);
	}

	return MFD_ERROR_NONE;
}


int mf_get_recent_files_count(MFDHandle *mfd_handle, int *count)
{
	mf_debug("");

	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;

	query_string = sqlite3_mprintf(MF_SELECT_RECENT_FILES_COUNT_TABLE, mf_tbl[field_seq].table_name);

	mf_debug("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		mf_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		mf_debug("Query fails : query_string[%s]", query_string);
		return MFD_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		mf_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		*count = 0;
		return MFD_ERROR_DB_NO_RECORD;
	}

	*count = sqlite3_column_int(stmt, 0);
	mf_debug("count : %d", *count);

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}

	return MFD_ERROR_NONE;
}

int mf_find_recent_file(MFDHandle *mfd_handle, const char *path)
{
	mf_debug("");

	if (path == NULL) {
		mf_debug("device_id is null");
		return MFD_ERROR_INVALID_PARAMETER;
	}

	mf_tbl_field_s *mf_tbl_field;
	char *query_string = NULL;
	mf_tbl_name_e field_seq = MF_TABLE_RECENT_FILES;
	mf_tbl_field = mf_tbl[field_seq].mf_tbl_field;
	sqlite3_stmt *stmt = NULL;
	int rc = 0;
	int find = 0;
	query_string = sqlite3_mprintf(MF_SELECT_FROM_RECENT_FILE_TABLE,
		    mf_tbl[field_seq].table_name,
		    mf_tbl_field[MF_FIELD_RECENT_FILES_PATH].field_name,
		    path);

	mf_error("Query : %s", query_string);

	rc = sqlite3_prepare_v2(mfd_handle, query_string, strlen(query_string), &stmt, NULL);
	sqlite3_free(query_string);
	if (SQLITE_OK != rc) {
		mf_debug("failed to query[%s]", sqlite3_errmsg(mfd_handle));
		mf_debug("Query fails : query_string[%s]", query_string);
		return find;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_ROW != rc) {
		mf_debug("No result");
		rc = sqlite3_finalize(stmt);
		if (SQLITE_OK != rc) {
			mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
		}
		return find;
	}

	while (SQLITE_ROW == rc) {
		mf_debug("Find a same recent_file");
		find = 1;
		rc = sqlite3_step(stmt);
	}

	rc = sqlite3_finalize(stmt);
	if (SQLITE_OK != rc) {
		mf_debug("sqlite3_finalize fail, rc : %d, db_error : %s", rc, sqlite3_errmsg(mfd_handle));
	}
	return find;
}
