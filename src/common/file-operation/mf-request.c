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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mf-request.h"

int flagMsg = 1;
pthread_mutex_t gLockMsg;
pthread_cond_t gCondMsg;

struct _mf_fo_req {
	GCond *cond;
	GMutex *lock;
	const char *path;
	char *new_name;
	int flagCond;
	mf_request_type request;
};

mf_fo_request *mf_request_new(void)
{
	mf_fo_request *result = NULL;

	result = malloc(sizeof(mf_fo_request));
	if (result) {
		result->cond = NULL;
		result->path = NULL;
		result->new_name = NULL;
		result->flagCond = 1;
		result->request = MF_REQ_NONE;
	}
	return result;
}

void mf_request_free(mf_fo_request *req)
{
	if (req) {
		if (req->new_name) {
			free(req->new_name);
		}
		free(req);
	}
	return;
}

void mf_request_set_result(mf_fo_request *req, mf_request_type result)
{
	if (req) {
		req->request = result;

		if (req->cond) {
			if (req->flagCond == 1) {
				req->flagCond = 0;
				g_cond_broadcast(req->cond);
			}
		}
	}

	return;
}

void mf_request_set_result_rename(mf_fo_request *req, const char *new_name)
{
	if (req) {
		req->request = MF_REQ_RENAME;
		if (new_name) {
			req->new_name = strdup(new_name);
		}

		if (req->cond) {
			g_cond_broadcast(req->cond);
		}
	}

	return;
}

void mf_request_set_cond(mf_fo_request *req, GCond * cond)
{
	if (req) {
		req->cond = cond;
	}
	return;
}

void mf_request_set_path(mf_fo_request *req, const char *path)
{
	if (req) {
		req->path = path;
	}
	return;
}

char *mf_request_get_new_name(mf_fo_request *req)
{
	char *new_name = NULL;
	if (req) {
		new_name = req->new_name;
		req->new_name = NULL;
	}
	return new_name;
}

const char *mf_request_get_path(mf_fo_request *req)
{
	if (req) {
		return req->path;
	}
	return NULL;
}

mf_request_type mf_request_get_result(mf_fo_request *req)
{
	mf_request_type request = MF_REQ_NONE;
	if (req) {
		request = req->request;
	}
	return request;
}


void mf_msg_request_handled_send()
{
	pthread_mutex_lock(&gLockMsg);
	if (flagMsg == 0) {
		flagMsg = 1;
		pthread_cond_signal(&gCondMsg);
	}
	pthread_mutex_unlock(&gLockMsg);
}

int mf_request_flag_get(mf_fo_request *req)
{
	if (req) {
		return req->flagCond;
	}
	return -1;
}

void mf_request_flag_set(mf_fo_request *req, int value)
{
	if (req) {
		req->flagCond = value;
	}
}
