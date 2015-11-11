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
#include <glib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include "mf-dlog.h"
#include "mf-inotify-handle.h"

#define MF_WATCH_FLAGS \
	IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE

#define MF_EVENT_SIZE  (sizeof(struct inotify_event))
/** reasonable guess as to size of 1024 events */
#define MF_EVENT_BUF_LEN (1024 * (MF_EVENT_SIZE + 16))
#define MF_U32_MAX	0xFFFFFFFF

int flagInotifyMsg = 1;
pthread_cond_t g_cond_inotify_msg;

typedef struct _mf_inotify_t {
	int fd;
	int wd;
	gchar *path;
	unsigned int prev_event;
	pthread_t monitor;
	mf_inotify_cb callback;
	void *u_data;
} mf_inotify_t;

static pthread_mutex_t mf_noti_lock;
static mf_inotify_t *g_handle = NULL;

static void __mf_inotify_handle_free(void)
{
	pthread_mutex_destroy(&mf_noti_lock);

	if (g_handle) {
		if (g_handle->fd >= 0) {
			close(g_handle->fd);
			g_handle->fd = -1;
		}
		if (g_handle->path) {
			g_free(g_handle->path);
			g_handle->path = NULL;
		}
		g_free(g_handle);
		g_handle = NULL;
	}

	return;
}

static mf_inotify_t *__mf_inotify_handle_init(void)
{
	__mf_inotify_handle_free();
	g_handle = g_new0(mf_inotify_t, 1);

	if (g_handle) {
		g_handle->fd = -1;
		g_handle->wd = -1;
		pthread_mutex_init(&mf_noti_lock, NULL);
	}

	return g_handle;
}

static void __mf_inotify_handle_thread_clean_up(void *data)
{
	pthread_mutex_t *lock = (pthread_mutex_t *) data;
	mf_debug("Thread cancel Clean_up function");
	if (lock) {
		pthread_mutex_unlock(lock);
	}
	return;
}


static gpointer __mf_inotify_handle_watch_thread(gpointer user_data)
{
	mf_inotify_t *handle = (mf_inotify_t *) user_data;
	int oldtype = 0;

	mf_retvm_if(handle == NULL, NULL, "handle is NULL");
	mf_debug("Create __mf_inotify_handle_watch_thread!!! ");

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);

	while (1) {
		ssize_t len = 0;
		uint32_t i = 0;
		//Prevent issue fix
		char event_buff[32752] = { 0, };

		if (handle->fd < 0) {
			mf_error("fd is not a vaild one");
			pthread_exit(NULL);
		}

		len = read(handle->fd, event_buff, sizeof(event_buff) - 1);
		if (len <= 0 || len > sizeof(event_buff) - 1) {
			mf_error("Fail to read() -fd : %d,  len : %d", handle->fd, len);
			continue;
		}

		flagInotifyMsg = 1;

		while (i < len) {
			struct inotify_event *pevent = (struct inotify_event *)&event_buff[i];

			mf_inotify_event s_event = MF_INOTI_NONE;
			SECURE_DEBUG("mask=%x dir=%s len=%d name=%s",
			             pevent->mask, (pevent->mask & IN_ISDIR) ? "yes" : "no", pevent->len, (pevent->len) ? pevent->name : NULL);

			if (pevent->len && strncmp(pevent->name, ".", 1) == 0) {
				s_event = MF_INOTI_NONE;
			} else if (pevent->mask & IN_ISDIR) {
				if (pevent->mask & IN_DELETE_SELF) {
					s_event = MF_INOTI_DELETE_SELF;
				}

				if (pevent->mask & IN_MOVE_SELF) {
					s_event = MF_INOTI_MOVE_SELF;
				}

				if (pevent->mask & IN_CREATE) {
					s_event = MF_INOTI_CREATE;
				}

				if (pevent->mask & IN_DELETE) {
					s_event = MF_INOTI_DELETE;
				}

				if (pevent->mask & IN_MOVED_FROM) {
					s_event = MF_INOTI_MOVE_OUT;
				}

				if (pevent->mask & IN_MOVED_TO) {
					s_event = MF_INOTI_MOVE_IN;
				}
			} else {
				if (pevent->mask & IN_CREATE) {
					s_event = MF_INOTI_NONE;
					handle->prev_event = IN_CREATE;
				}

				if (pevent->mask & IN_CLOSE_WRITE) {
					if (handle->prev_event == IN_CREATE) {
						s_event = MF_INOTI_CREATE;
					} else {
						s_event = MF_INOTI_MODIFY;
					}
					handle->prev_event = 0;
				}

				if (pevent->mask & IN_DELETE) {
					s_event = MF_INOTI_DELETE;
				}

				if (pevent->mask & IN_MOVED_FROM) {
					s_event = MF_INOTI_MOVE_OUT;
				}

				if (pevent->mask & IN_MOVED_TO) {
					s_event = MF_INOTI_MOVE_IN;
				}
			}

			mf_debug("s_event : %d, prev_event: %x, callback : %p", s_event, handle->prev_event, handle->callback);
			if (s_event != MF_INOTI_NONE) {
				pthread_cleanup_push(__mf_inotify_handle_thread_clean_up, (void *)&mf_noti_lock);
				pthread_mutex_lock(&mf_noti_lock);

				while (flagInotifyMsg == 0) {
					mf_debug("!!!!!!!!!!!! wait");
					pthread_cond_wait(&g_cond_inotify_msg, &mf_noti_lock);
				}
				flagInotifyMsg = 0;

				if (handle->callback) {
					handle->callback(s_event, (pevent->len) ? pevent->name : NULL, handle->u_data);
				}
				pthread_mutex_unlock(&mf_noti_lock);
				pthread_cleanup_pop(0);
			}

#if 0
			if ((MF_U32_MAX - pevent->len) >=  MF_EVENT_SIZE) {
				i += sizeof(struct inotify_event) + pevent->len;
			} else {
				break;
			}
#else
			i += sizeof(struct inotify_event) + pevent->len;
			if (i > sizeof(event_buff) - 1) {
				mf_error("out of bounds access: %d(max: %d)", i, sizeof(event_buff));
				break;
			}
#endif
		}
	}

	mf_debug("end __mf_inotify_handle_watch_thread!!! ");

	return NULL;
}

void mf_inotify_handle_request_handled_send()
{
	pthread_mutex_lock(&mf_noti_lock);
	if (flagInotifyMsg == 0) {
		flagInotifyMsg = 1;
		pthread_cond_signal(&g_cond_inotify_msg);
	}
	pthread_mutex_unlock(&mf_noti_lock);
}

int mf_inotify_handle_init_inotify(void)
{
	mf_inotify_t *handle = NULL;
	handle = __mf_inotify_handle_init();
	mf_retvm_if(handle == NULL, -1, "fail to __mf_inotify_handle_init()");

	handle->fd = inotify_init();

	if (handle->fd < 0) {
		switch (errno) {
		case EMFILE:
			mf_error("The user limit on the total number of inotify instances has been reached.\n");
			break;
		case ENFILE:
			mf_error("The system limit on the total number of file descriptors has been reached.\n");
			break;
		case ENOMEM:
			mf_error("Insufficient kernel memory is available.\n");
			break;
		default:
			mf_error("Fail to inotify_init(), Unknown error.\n");
			break;
		}
		return -1;
	}
	pthread_create(&handle->monitor, NULL, __mf_inotify_handle_watch_thread, handle);
	return 0;
}

int mf_inotify_handle_add_watch(const char *path, mf_inotify_cb callback, void *user_data)
{
	mf_inotify_t *handle = NULL;
	handle = g_handle;
	mf_retvm_if(handle == NULL, -1, "handle is NULL");

	if (handle->wd >= 0) {
		mf_warning("The mf_notify module supports single instance, the watch descript [%d] is removed automatically\n", handle->wd);
		mf_inotify_handle_rm_watch();
	}

	pthread_mutex_lock(&mf_noti_lock);
	handle->wd = inotify_add_watch(handle->fd, path, MF_WATCH_FLAGS);

	if (handle->wd < 0) {
		switch (errno) {
		case EACCES:
			mf_error("Read access to the given file is not permitted.\n");
			break;
		case EBADF:
			mf_error("The given file descriptor is not valid.\n");
			handle->fd = -1;
			break;
		case EFAULT:
			mf_error("pathname points outside of the process's accessible address space.\n");
			break;
		case EINVAL:
			mf_error("The given event mask contains no legal events; or fd is not an inotify file descriptor.\n");
			break;
		case ENOMEM:
			mf_error("Insufficient kernel memory is available.\n");
			break;
		case ENOSPC:
			mf_error("User limit on the total number of inotify watches was reached or the kernel failed to alloc a needed resource.\n");
			break;
		default:
			mf_error("Fail to mf_inotify_handle_add_watch(), Unknown error.\n");
			break;
		}
		pthread_mutex_unlock(&mf_noti_lock);
		return -1;
	}

	mf_debug("start watching [%s] directory", path);
	if (handle->path) {
		g_free(handle->path);
		handle->path = NULL;
	}
	handle->path = g_strdup(path);
	handle->callback = callback;
	handle->u_data = user_data;
	pthread_mutex_unlock(&mf_noti_lock);

	return 0;
}



int mf_inotify_handle_rm_watch(void)
{
	int ret = -1;
	mf_inotify_t *handle = NULL;

	handle = g_handle;
	mf_retvm_if(handle == NULL, -1, "handle is NULL");

	if (handle->fd < 0 || handle->wd < 0) {
		mf_warning("inotify is not initialized or has no watching dir - fd [%d] wd [%d]", handle->fd, handle->wd);
		return 0;
	}

	pthread_mutex_lock(&mf_noti_lock);

	ret = inotify_rm_watch(handle->fd, handle->wd);
	if (ret < 0) {
		switch (errno) {
		case EBADF:
			mf_error("fd is not a valid file descriptor\n");
			handle->fd = -1;
			break;
		case EINVAL:
			mf_error("The watch descriptor wd is not valid; or fd is not an inotify file descriptor.\n");
			handle->wd = -1;
			break;
		default:
			mf_error("Fail to mf_inotify_handle_add_watch(), Unknown error.\n");
			break;
		}
		pthread_mutex_unlock(&mf_noti_lock);
		return -1;
	}
	mf_debug("stop watching [%s] directory", handle->path);
	if (handle->path) {
		g_free(handle->path);
		handle->path = NULL;
	}
	handle->callback = NULL;
	handle->u_data = NULL;
	handle->wd = -1;
	pthread_mutex_unlock(&mf_noti_lock);

	return 0;
}

void mf_inotify_handle_finalize_inotify(void)
{
	mf_inotify_t *handle = NULL;
	handle = g_handle;

	mf_retm_if(handle == NULL, "handle is NULL");

	if (handle->fd >= 0 && handle->wd >= 0) {
		mf_inotify_handle_rm_watch();
	}

	pthread_cancel(handle->monitor);
	pthread_join(handle->monitor, NULL);

	__mf_inotify_handle_free();

	return;
}
