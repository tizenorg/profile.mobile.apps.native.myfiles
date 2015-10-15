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

#ifndef _MF_FOCUS_UI_H_
#define _MF_FOCUS_UI_H_

#include <stdbool.h>
#include <sys/types.h>
#include "mf-fo-common.h"
#include "mf-util.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _mf_focus_dual_order_t mf_focus_dual_order_e;

enum _mf_focus_dual_order_t {
	MF_FOCUS_DUAL_ORDER_NONE,
	MF_FOCUS_DUAL_ORDER_NEXT_PRIV,
	MF_FOCUS_DUAL_ORDER_RIGHT_LEFT,
	MF_FOCUS_DUAL_ORDER_DOWN_UP,
	MF_FOCUS_DUAL_ORDER_MAX,
};

int _mf_focus_ui_set_dual_focus_order(Evas_Object *curr, Evas_Object *next, mf_focus_dual_order_e order);
int _mf_focus_ui_set_focus(Evas_Object *elm_widget);

#ifdef __cplusplus
}
#endif


#endif //_MF_FOCUS_UI_H_

