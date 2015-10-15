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

#ifndef __MF_REQUEST_H_DEF__
#define __MF_REQUEST_H_DEF__

#include <glib.h>

/**
 * mf_request_type:
 * @MF_REQ_NONE: initial state of #mf_fo_request, if set this one by mf_request_set_result(),
 *                           undefined action may occurs.
 * @MF_REQ_MERGE: indicates merge type operation
 * @MF_REQ_RENAME: indicates rename type operation.
 *                               to specify new name, use mf_request_set_result_rename()
 * @MF_REQ_SKIP : indicates skip this item
 * @MF_REQ_CANCEL : indicates cancel whole operation
 *
 * If the #mf_req_callback is called by operator, conductor should decide request type by
 * mf_request_set_result() or mf_request_set_result_rename().
 *
 * Note that the #mf_request_type enumeration may be extended at a later
 *
 */

typedef enum _request_type mf_request_type;

enum _request_type {
	MF_REQ_NONE,
	MF_REQ_MERGE,
	MF_REQ_RENAME,
	MF_REQ_SKIP,
	MF_REQ_CANCEL,
};

/**
 * mf_fo_request:
 * <structname>mf_fo_request</structname> is an opaque structure whose members
 * cannot be accessed directly.
 */
typedef struct _mf_fo_req mf_fo_request;

/**
 * mf_req_callback:
 * @req: the handle of request, use mf_request_set_result() or  mf_request_set_result_rename()
 * to set detail decision with this handle.
 * @data: user data.
 **/
typedef void (*mf_req_callback) (mf_fo_request *req, void *data);

/**
 * mf_request_new:
 * Creates a new #mf_fo_request.
 * Return value: This function returns a new #mf_fo_request on success, or %NULL.
 **/
mf_fo_request *mf_request_new(void);

/**
 * mf_request_free:
 * @req : a #mf_fo_request
 * Release all allocated memory for @req.
 **/
void mf_request_free(mf_fo_request *req);

/**
 * mf_request_set_result:
 * @req : a #mf_fo_request
 * @result : a user's decision type #mf_request_type value
 * Set @result to @req
 **/
void mf_request_set_result(mf_fo_request *req, mf_request_type result);

/**
 * mf_request_set_result_rename:
 * @req : a #mf_fo_request
 * @new_name : a user specified new name, string for item name
  * Set @new_name to @req, and #mf_request_type is selected #MF_REQ_RENAME
 **/
void mf_request_set_result_rename(mf_fo_request *req, const char *new_name);

/**
 * mf_request_set_cond:
 * @req : a #mf_fo_request
 * @cond : a #GCond for @req
 * Set @cond to @req
 **/
void mf_request_set_cond(mf_fo_request *req, GCond *cond);

/**
 * mf_request_set_path:
 * @req : a #mf_fo_request
 * @path : a string for duplicated item name.
 * Set @path to @req
 **/
void mf_request_set_path(mf_fo_request *req, const char *path);


/**
 * mf_request_get_result:
 * @req : a #mf_fo_request
 * Return value: user selected #mf_request_type.
 **/
mf_request_type mf_request_get_result(mf_fo_request *req);

/**
 * mf_request_get_path:
 * @req : a #mf_fo_request
 * Return value: a string for duplicated item name.
 **/
const char *mf_request_get_path(mf_fo_request *req);

/**
 * mf_request_get_path:
 * @req : a #mf_fo_request
 * To get item's new name , if user specify new name for duplicated item by mf_request_set_result_rename.
 * Return value: a user specified new name, string for item name, or NULL
 **/
char *mf_request_get_new_name(mf_fo_request *req);
int mf_request_flag_get(mf_fo_request *req);
void mf_request_flag_set(mf_fo_request *req, int value);
void mf_msg_request_handled_send();

#endif //__MF_REQUEST_H_DEF__
