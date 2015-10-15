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

#ifndef __MF_MEDIA_ERROR_H_DEF__
#define __MF_MEDIA_ERROR_H_DEF__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//Error types definition
#define MFD_ERROR_NONE					0				/**< base */
#define MFD_ERROR_INVALID_PARAMETER 		-1  			/**< invalid parameter(s) */
#define MFD_ERROR_INVALID_MEDIA  	 	-2				/**< invalid or unknown media */
#define MFD_ERROR_FILE_NOT_EXSITED 		-3				/**< file doesn't exist */
#define MFD_ERROR_DIR_NOT_EXSITED 		-4				/**< folder doesn't exist */

#define MFD_ERROR_DB_CONNECT 			-201			/**< connect DB error */
#define MFD_ERROR_DB_DISCONNECT 			-202			/**< disconnect DB error  */
#define MFD_ERROR_DB_CREATE_TABLE 		-203			/**< create table error */
#define MFD_ERROR_DB_NO_RECORD 			-204			/**< No record */
#define MFD_ERROR_DB_INTERNAL	 		-206			/**< internal db error  */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__MF_MEDIA_ERROR_H_DEF__*/



