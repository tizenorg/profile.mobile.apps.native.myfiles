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

#ifndef _MF_CANCEL_DEF_H_
#define _MF_CANCEL_DEF_H_

#include <glib.h>

/**
 * mf_cancel:
 * <structname>mf_cancel</structname> is an opaque structure whose members
 * cannot be accessed directly.
 */
typedef struct _mf_cancel mf_cancel;

struct _mf_cancel {
	gboolean is_cancel;
	gboolean do_cancel;
	GMutex s_mutex;
};

/**
 * mf_cancel_new:
 * Creates a new #mf_cancel.
 * Return value: This function returns a new #mf_cancel on success, or %NULL.
 **/
mf_cancel *mf_cancel_new(void);

/**
 * mf_cancel_free:
 * @cancel : a #mf_cancel
 * Release all allocated memory for @cancel.
 **/
void mf_cancel_free(mf_cancel *cancel);

/**
 * mf_cancel_do_cancel:
 * @cancel : a #mf_cancel
 * Request to cancel operation related on @cancel.
 * This function should be called by conductor at most case
 **/
void mf_cancel_do_cancel(mf_cancel *cancel);

/**
 * mf_cancel_check_cancel:
 * Check request of @cancel
 * Return value: This function returns TRUE, if cancel is requested, or FALSE.
 **/
gboolean mf_cancel_check_cancel(mf_cancel *cancel);

/**
 * mf_cancel_is_cancelled:
 * Check status of @cancel
 * Return value: This function returns TRUE, if cancel is done, or FALSE.
 **/
gboolean mf_cancel_is_cancelled(mf_cancel *cancel);


/**
 * mf_cancel_set_cancelled:
 * set status of @cancel after cancel is done
 * This function should be called by operator
 **/
void mf_cancel_set_cancelled(mf_cancel *cancel);

#endif //_MF_CANCEL_DEF_H_
