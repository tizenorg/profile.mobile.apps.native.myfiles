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

#include "mf-focus-ui.h"
#include "mf-dlog.h"

int _mf_focus_ui_set_dual_focus_order(Evas_Object *curr, Evas_Object *next, mf_focus_dual_order_e order)
{
	mf_retvm_if(curr == NULL, 0, "curr is NULL");
	mf_retvm_if(next == NULL, 0, "next is NULL");

	if (elm_object_focus_allow_get(curr) != EINA_TRUE ||
		elm_object_focus_allow_get(next) != EINA_TRUE) {
		mf_debug("Input elm object is not focusable!");
		return -1;
	}

	if (order == MF_FOCUS_DUAL_ORDER_NEXT_PRIV) {
		elm_object_focus_next_object_set(curr, next, ELM_FOCUS_NEXT);		/* tab */
		elm_object_focus_next_object_set(next, curr, ELM_FOCUS_PREVIOUS);	/* shift + tab */
	} else if (order == MF_FOCUS_DUAL_ORDER_RIGHT_LEFT) {
		elm_object_focus_next_object_set(curr, next, ELM_FOCUS_RIGHT);		/* right */
		elm_object_focus_next_object_set(next, curr, ELM_FOCUS_LEFT);		/* left */
	} else if (order == MF_FOCUS_DUAL_ORDER_DOWN_UP) {
		elm_object_focus_next_object_set(curr, next, ELM_FOCUS_DOWN);		/* down */
		elm_object_focus_next_object_set(next, curr, ELM_FOCUS_UP);			/* up */
	}
	return 0;
}

/* elm widgets only */
int _mf_focus_ui_set_focus(Evas_Object *elm_widget)
{
	mf_retvm_if(elm_widget == NULL, 0, "elm_widget is NULL");

	elm_object_focus_allow_set(elm_widget, EINA_TRUE);
	if (elm_object_focus_get(elm_widget) == EINA_TRUE) {
		elm_object_focus_set(elm_widget, EINA_FALSE);
	}
	elm_object_focus_set(elm_widget, EINA_TRUE);
	return 0;
}

