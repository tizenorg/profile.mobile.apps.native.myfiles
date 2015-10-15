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

#include <errno.h>
#include "mf-error.h"
#include "mf-dlog.h"

int mf_error_erron_to_mferror(int err_no)
{
	int err = MYFILE_ERR_NONE;
	char buf[256] = {0,};
	char const *str = strerror_r(err_no, buf, 256);
	
	mf_error("err_no is [%d] error - %s", err_no, str);
	switch (err_no) {
#ifdef EINVAL
	case EINVAL:
		err = MYFILE_ERR_SRC_ARG_INVALID;
		break;
#endif

#ifdef EACCES			/*The requested access to the file is not allowed*/
	case EACCES:		/*report*/
		err = MYFILE_ERR_PERMISSION_DENY;
		break;
#endif

#ifdef EPERM
	case EPERM:
		err = MYFILE_ERR_PERMISSION_DENY;
		break;
#endif

#ifdef EFAULT			/* pathname points outside your accessible address space*/
	case EFAULT:
		err = MYFILE_ERR_FAULT;
		break;
#endif
#ifdef ENOSPC			/*pathname was to be created but the device containing pathname has no room for the new file*/
	case ENOSPC:		/*report*/
		err = MYFILE_ERR_NO_FREE_SPACE;
		break;
#endif

#ifdef EROFS			/*pathname refers to a file on a read-only filesystem and write access was requested*/
	case EROFS:		/*report*/
		err = MYFILE_ERR_READ_ONLY;
		break;
#endif

	}
	return err;
}

