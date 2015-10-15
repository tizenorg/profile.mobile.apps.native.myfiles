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

#ifndef __MF_ERROR_H_DEF__
#define __MF_ERROR_H_DEF__

#define MF_ERROR_MASKL16       0xFFFF

#define MF_ERROR_SET(X)        (X & MF_ERROR_MASKL16)

#define MID_CONTENTS_MGR_ERROR  0

#define MYFILE_ERR_NONE   (MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x00))    /**< No error */

#define MYFILE_ERR_SRC_ARG_INVALID		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x01))	/**< invalid src argument */
#define MYFILE_ERR_DST_ARG_INVALID		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x02))	/**< invalid dst argument */
#define MYFILE_ERR_FILE_DELETE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x03))	/**< exception of delete file */
#define MYFILE_ERR_FILE_MOVE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x04))	/**< exception of move file */
#define MYFILE_ERR_FILE_COPY_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x05))	/**< exception of copy file */
#define MYFILE_ERR_FILE_WRITE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x06))	/**< exception of read file */
#define MYFILE_ERR_RENAME_FAIL			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x07))    /**< exception of rename file */
#define MYFILE_ERR_FILE_NOT_FOUND		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x08))	/**< exception of file doesn't exist*/
#define MYFILE_ERR_DIR_OPEN_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x09))	/**< exception of dir open*/
#define MYFILE_ERR_DIR_CREATE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0a))	/**< exception of create dir */

/*11-20*/
#define MYFILE_ERR_DIR_DELETE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0b))	/**< exception of delete dir */
#define MYFILE_ERR_FILE_OPEN_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0c))    /**< exception of rename dir */
#define MYFILE_ERR_DIR_COPY_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0d))	/**< exception of copy dir */
#define MYFILE_ERR_DIR_MOVE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0e))	/**< exception of move dir */
#define MYFILE_ERR_DIR_FULL			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x0f))	/**< exception of dir full */
#define MYFILE_ERR_DIR_TOO_DEEP			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x10))	/**< exception of too deep dir */
#define MYFILE_ERR_DIR_NOT_FOUND		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x11))	/**< exception of dir doesn't exist*/
#define MYFILE_ERR_INVALID_DIR_NAME		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x12))	/**< exception of invalid dir name */
#define MYFILE_ERR_INVALID_DIR_PATH		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x13))	/**< exception of invalid dir path */
#define MYFILE_ERR_INVALID_FILE_NAME		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x14))	/**< exception of invalid file name */

/*21-30*/
#define MYFILE_ERR_INVALID_FILE_PATH		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x15))	/**< exception of invalid file path */
#define MYFILE_ERR_GET_MEMORY_STATUS_FAIL	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x16))	/**< exception of statfs */
#define MYFILE_ERR_DUPLICATED_NAME		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x17))    /**< exception of duplicated dir name*/
#define MYFILE_ERR_SYSTEM_DIR			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x18))	/**< exception of operating on system dir.*/
#define MYFILE_ERR_DIR_RECUR			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x19))	/**< exception of copy/move a dir to its child */
#define MYFILE_ERR_ALLOCATE_MEMORY_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x1a))	/**< exception of memory allocation */
#define MYFILE_ERR_OUT_OF_RANGE			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x1b))
#define MYFILE_ERR_INVALID_PATH			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x1c))    /**< invalid path string */
#define MYFILE_ERR_ROOT_PATH			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x1d))    /**< root path */
#define MYFILE_ERR_DCM_ENGINE_APPEND		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x1e))    /**< fail to append dcm data */

/*31-40*/
#define MYFILE_ERR_NOT_MMF_FILE			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x1f))	/**< isn't mmf file */
#define MYFILE_ERR_SMAF_PERMISSION_DENY		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x20))	/**< smaf lock prohibits copy/move from mmc to phone */
#define MYFILE_ERR_COPY_TO_SRC_DIR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x22))	    /**< can't copy dir to source place*/
#define MYFILE_ERR_COPY_ROOT_DIR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x23))	    /**< can't copy root dir*/
#define MYFILE_ERR_COPY_TO_SRC_FILE		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x24))	    /**< can't copy file to source place*/
#define MYFILE_ERR_MOVE_TO_SRC_DIR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x25))	    /**< can't move dir to source place */
#define MYFILE_ERR_MOVE_ROOT_DIR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x26))	    /**< can't move root dir */
#define MYFILE_ERR_MOVE_TO_SRC_FILE		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x27))	    /**< can't move file to source place*/
#define MYFILE_ERR_MOVE_FILE_USING		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x28))	    /**< can't move the file is being used*/

/*41-50*/
#define MYFILE_ERR_DELETE_ROOT_DIR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x29))	    /**< can't delete root dir */
#define MYFILE_ERR_DELETE_SYSTEM_DIR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x2a))	    /**< can't delete system dir */
#define MYFILE_ERR_RENAME_ROOT_DIR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x2b))	    /**< can't rename root dir */
#define MYFILE_ERR_RENAME_SYSTEM_DIR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x2c))	    /**< can't rename system dir */
#define MYFILE_ERR_EXCEED_MAX_LENGTH		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x2d))	    /**< length of file/dir path exceeds maximum length*/
#define MYFILE_ERR_LOW_MEMORY			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x2e))	    /**< low memory*/
#define MYFILE_ERR_UNKNOWN_ERROR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x2f))	    /**< unknow error*/
#define MYFILE_ERR_WRONG_FILE_TYPE		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x30))	    /**< wrong file type */
#define MYFILE_ERR_FILE_IS_BEING_USED		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x31))	    /**< file is being used */
#define MYFILE_ERR_SRC_NOT_EXIST		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x32))	/**< source not found */

/*51-60*/
#define MYFILE_ERR_DST_NOT_EXIST		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x33))	/**< destination not found */
#define MYFILE_ERR_CREATE_TEMP_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x34))	/**< create temp file failed */
#define MYFILE_ERR_GET_LOGIC_PATH_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x35))	/**< get logical path failed */
#define MYFILE_ERR_STORAGE_TYPE_ERROR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x36))	/**< storage type error */
#define MYFILE_ERR_EXT_GET_ERROR		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x37))	/**< get ext type failed */
#define MYFILE_ERR_GET_PARENT_PATH_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x38))	/**< get parent path failed */
#define MYFILE_ERR_GET_STAT_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x39))	/**< get stat failed */
#define MYFILE_ERR_GENERATE_NAME_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x3a))	/**< generate name failed */
#define MYFILE_ERR_GET_CATEGORY_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x3b))	/**< get file category failed */
#define MYFILE_ERR_GET_STORAGE_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x3c))	/**< get storage value failed */

/*61-70*/
#define MYFILE_ERR_SETTING_RESET_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x3d))	/**< setting item reset  failed */
#define MYFILE_ERR_DIR_CLEAR_FAILED		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x3e))	/**< dir not clearly deleted */
#define MYFILE_ERR_SETTING_DELETE_FAILED	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x3f))	/**< delete setting item failed */
#define MYFILE_ERR_GET_THUMBNAIL_FAILED		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x40))	/**< get file thumbnail failed */
#define MYFILE_ERR_CANCEL_PRESSED		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x41))	/**< cancel pressed while copy/move */
#define MYFILE_ERR_ACCESS_MODE			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x42))	/**< access mode not satisfied */
#define MYFILE_ERR_FILE_READ_FAIL		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x43))	/**< exception of read file */
#define MYFILE_ERR_INVALID_ARG			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x44))	       /**< argument of function is not valid */
#define MYFILE_ERR_NO_FREE_SPACE		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x45))	       /**< get free space failed */
#define MYFILE_ERR_GET_NAVI_FAILED		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x46))	       /**< get navigation bar failed */

/*71-80*/
#define MYFILE_ERR_STORAGE_INUSE_REMOVED	(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x47))	       /**< get navigation bar failed */
#define MYFILE_ERR_STORAGE_GET_FAILED		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x48))	       /**< get STORAGE value failed */
#define MYFILE_ERR_PERMISSION_DENY		(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x49))
#define MYFILE_ERR_FAULT			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x4a))
#define MYFILE_ERR_READ_ONLY			(MID_CONTENTS_MGR_ERROR - MF_ERROR_SET(0x4b))

int mf_error_erron_to_mferror(int err_no);

#endif //__MF_ERROR_H_DEF__
