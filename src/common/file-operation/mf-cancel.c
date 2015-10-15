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

#include <glib.h>
#include "mf-cancel.h"

mf_cancel *mf_cancel_new(void)
{
	mf_cancel *cancel = NULL;

	cancel = g_new0(mf_cancel, 1);
	if (cancel) {
		cancel->is_cancel = FALSE;
		cancel->do_cancel = FALSE;
		g_mutex_init(&cancel->s_mutex);
	}
	return cancel;
}

void mf_cancel_free(mf_cancel *cancel)
{
	if (cancel) {
		cancel->is_cancel = FALSE;
		cancel->do_cancel = FALSE;
		g_mutex_clear(&cancel->s_mutex);
		g_free(cancel);
	}
	return;
}

void mf_cancel_do_cancel(mf_cancel *cancel)
{
	if (cancel) {
		g_mutex_lock(&cancel->s_mutex);
		cancel->do_cancel = TRUE;
		g_mutex_unlock(&cancel->s_mutex);
	}
	return;
}

gboolean mf_cancel_check_cancel(mf_cancel *cancel)
{
	gboolean do_cancel = FALSE;
	if (cancel) {
		g_mutex_lock(&cancel->s_mutex);
		if (cancel->do_cancel)
			do_cancel = TRUE;
		g_mutex_unlock(&cancel->s_mutex);
	}
	return do_cancel;
}

void mf_cancel_set_cancelled(mf_cancel *cancel)
{
	if (cancel) {
		g_mutex_lock(&cancel->s_mutex);
		cancel->is_cancel = TRUE;
		g_mutex_unlock(&cancel->s_mutex);
	}
	return;
}

gboolean mf_cancel_is_cancelled(mf_cancel *cancel)
{
	gboolean is_cancelled = FALSE;
	if (cancel) {
		g_mutex_lock(&cancel->s_mutex);
		if (cancel->is_cancel)
			is_cancelled = TRUE;
		g_mutex_unlock(&cancel->s_mutex);
	}
	return is_cancelled;
}
