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

#ifndef __DEF_MF_UG_DETAIL_RESOURCE_H_
#define __DEF_MF_UG_DETAIL_RESOURCE_H_

#include <stdio.h>
#include <assert.h>
#include <glib.h>

char *mf_detail_widget_get_text(const char *ID);
/*******************   local string **********/
#define MF_UG_DETAIL_LABEL_IMPOSSIBLE	mf_detail_widget_get_text("IDS_MF_BODY_IMPOSSIBLE")
#define MF_UG_DETAIL_LABEL_POSSIBLE		mf_detail_widget_get_text("IDS_MF_BODY_POSSIBLE")

#define MF_UG_DETAIL_LABEL_VALIDITY		mf_detail_widget_get_text("IDS_MF_BODY_VALIDITY")
#define MF_UG_DETAIL_LABEL_ANY			mf_detail_widget_get_text("IDS_MF_BODY_ANY")
#define MF_UG_DETAIL_LABEL_EXECUTE		mf_detail_widget_get_text("IDS_MF_BODY_EXECUTE")
#define MF_UG_DETAIL_LABEL_CONTENTS		mf_detail_widget_get_text("IDS_MF_BODY_CONTAINS") //"IDS_MF_BODY_CONTENTS"
#define MF_UG_DETAIL_LABEL_CREATE_DATE		mf_detail_widget_get_text("IDS_MF_BODY_LAST_MODIFIED_M_DATE_ABB") //"IDS_MF_BODY_LAST_MODIFIED_M_DATE_ABB"
#define MF_UG_DETAIL_LABEL_TIME_COUNT		mf_detail_widget_get_text("IDS_MF_BODY_TIMED_COUNT")

#define MF_UG_DETAIL_LABEL_INTERVAL		mf_detail_widget_get_text("IDS_MF_BODY_INTERVAL")
#define MF_UG_DETAIL_LABEL_INDIVIDUAL		mf_detail_widget_get_text("IDS_MF_BODY_INDIVIDUAL")

#define MF_UG_DETAIL_LABEL_VENDOR		"IDS_MF_BODY_VENDOR"
#define MF_UG_DETAIL_LABEL_RIGHT_STATUS		"IDS_MF_BODY_RIGHT_STATUS"
#define MF_UG_DETAIL_LABEL_DETAILS_CHAP 	"IDS_COM_BODY_DETAILS"

/************* system string *************/
#define MF_UG_DETAIL_LABEL_PLAY			mf_detail_widget_get_text("IDS_COM_SK_PLAY")
#define MF_UG_DETAIL_LABEL_DISPLAY		mf_detail_widget_get_text("IDS_COM_BODY_DISPLAY")
#define MF_UG_DETAIL_LABEL_PRINT		mf_detail_widget_get_text("IDS_COM_SK_PRINT")
#define MF_UG_DETAIL_LABEL_COPY			mf_detail_widget_get_text("IDS_COM_BODY_COPY")
#define MF_UG_DETAIL_LABEL_MOVE			mf_detail_widget_get_text("IDS_COM_BODY_MOVE")
#define MF_UG_DETAIL_LABEL_UNLIMITED		mf_detail_widget_get_text("IDS_COM_POP_UNLIMITED")
#define MF_UG_DETAIL_LABEL_SYSTEM		mf_detail_widget_get_text("IDS_COM_BODY_SYSTEM")
#define MF_UG_DETAIL_LABEL_UNKNOWN		mf_detail_widget_get_text("IDS_COM_BODY_UNKNOWN")
#define MF_UG_DETAIL_LABEL_DETAILS		mf_detail_widget_get_text("IDS_COM_BODY_DETAILS")
#define MF_UG_DETAIL_LABEL_NAME			"IDS_MF_BODY_NAME"
#define MF_UG_DETAIL_LABEL_SIZE			"IDS_COM_BODY_SIZE"
#define MF_UG_DETAIL_LABEL_FORMAT		mf_detail_widget_get_text("IDS_MF_MBODY_FORMAT") //"IDS_MF_POP_FORMAT" //"IDS_COM_BODY_FORMAT"
#define MF_UG_DETAIL_LABEL_RESOLUTION		"IDS_IDLE_BODY_RESOLUTION"
#define MF_UG_DETAIL_LABEL_LOCATION		mf_detail_widget_get_text("IDS_LBS_BODY_PATH") //"IDS_COM_BODY_LOCATION"
#define MF_UG_DETAIL_LABEL_TIME			"IDS_COM_BODY_TIME"
#define MF_UG_DETAIL_LABEL_LATITUDE		"IDS_COM_BODY_LATITUDE"
#define MF_UG_DETAIL_LABEL_LONGITUDE		"IDS_COM_BODY_LONGITUDE"
#define MF_UG_DETAIL_LABEL_DESCRIPTION		"IDS_COM_BODY_DESCRIPTION"
#define MF_UG_DETAIL_LABEL_FORWARDING		"IDS_COM_BODY_FORWARDING"
#define MF_UG_DETAIL_LABEL_AVALIABLE_USE	"IDS_COM_POP_AVAILABLE_USES"
#define MF_UG_DETAIL_LABEL_TYPE			"IDS_COM_BODY_DETAILS_TYPE"
#define MF_UG_DETAIL_LABEL_BASIC		"IDS_COM_BODY_BASIC"
#define MF_UG_DETAIL_LABEL_INFORMATION		"IDS_COM_BODY_INFORMATION"
#define MF_UG_DETAIL_LABELL_FILES		mf_detail_widget_get_text("IDS_MF_BODY_FILES")
#define MF_UG_DETAIL_LABELL_FILE		mf_detail_widget_get_text("IDS_MF_BODY_FILE")
#define MF_UG_DETAIL_LABELL_FOLDER		mf_detail_widget_get_text("IDS_COM_POP_FOLDER")
#define MF_UG_DETAIL_LABELL_FOLDERS		mf_detail_widget_get_text("IDS_COM_BODY_FOLDERS")
#define MF_UG_DETAIL_LABEL_DEVICE_MEMORY	mf_detail_widget_get_text("IDS_VIDEO_SBODY_DEVICE_STORAGE") //mf_detail_widget_get_text("IDS_MF_BODY_DEVICE_MEMORY_ABB")
#define MF_UG_DETAIL_LABEL_SD_CARD		mf_detail_widget_get_text("IDS_MF_BODY_SD_CARD_ABB")
#define MF_UG_DETAIL_LABEL_SIZE_B		"IDS_COM_BODY_BYTE"
#define MF_UG_DETAIL_LABEL_SIZE_K		"IDS_COM_BODY_KBYTE"
#define MF_UG_DETAIL_LABEL_SIZE_M		"IDS_COM_BODY_MB"
#define MF_UG_DETAIL_LABEL_SIZE_G		"IDS_COM_BODY_GB"
#define MF_UG_DETAIL_LABEL_TOTAL_SIZE			mf_detail_widget_get_text("IDS_MF_TMBODY_TOTAL_SIZE")
#define MF_UG_DETAIL_LABEL_CONTAINS		mf_detail_widget_get_text("IDS_MF_BODY_CONTAINS")

#endif //__DEF_MF_UG_DETAIL_RESOURCE_H_

