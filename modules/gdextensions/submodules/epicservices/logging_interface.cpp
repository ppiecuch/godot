/**************************************************************************/
/*  logging_interface.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "gd_epic_services.h"

#include "core/message_queue.h"
#include "core/print_string.h"

#include "eos_logging.h"

// EOS_Logging_SetCallback registers a single global C function. We hop the
// message back to the Godot main thread and emit it as a signal on the
// EpicServices singleton so multiple GDScript listeners can subscribe.
static void EOS_CALL _epic_log_callback(const EOS_LogMessage *message) {
	EpicServices *es = EpicServices::get_singleton();
	if (!es || !message) {
		return;
	}
	Dictionary payload;
	payload["category"] = int(0); // EOS_ELogCategory is opaque enum — pass through as-is would need numeric cast
	payload["category_name"] = String::utf8(message->Category ? message->Category : "");
	payload["level"] = int(message->Level);
	payload["message"] = String::utf8(message->Message ? message->Message : "");
	MessageQueue::get_singleton()->push_call(es->get_instance_id(), "emit_signal",
			StringName("log_message_received"),
			payload["category"], payload["level"], payload["category_name"], payload["message"]);
}

int EpicServices::logging_interface_set_callback() {
	return int(EOS_Logging_SetCallback(&_epic_log_callback));
}

int EpicServices::logging_interface_set_log_level(int p_category, int p_level) {
	return int(EOS_Logging_SetLogLevel(EOS_ELogCategory(p_category), EOS_ELogLevel(p_level)));
}
