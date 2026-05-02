/**************************************************************************/
/*  epic_callback.h                                                       */
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

#ifndef EPIC_CALLBACK_H
#define EPIC_CALLBACK_H

#include "core/message_queue.h"
#include "core/object.h"
#include "core/ustring.h"

// Heap-allocated context handed to EOS as ClientData. The static EOS C
// callback recovers the EpicServices ObjectID + signal name, hops back to
// the main thread via MessageQueue::push_call("emit_signal", ...), then
// frees itself.
struct EpicCallback {
	ObjectID singleton_id;
	StringName signal_name;

	EpicCallback(ObjectID p_id, const StringName &p_signal) :
			singleton_id(p_id), signal_name(p_signal) {}
};

// Convenience: deferred-emit a signal on the EpicServices singleton.
// Always thread-safe; the dispatch happens on the main thread next tick.
static inline void epic_emit_deferred(ObjectID p_id, const StringName &p_signal, const Dictionary &p_payload) {
	MessageQueue::get_singleton()->push_call(p_id, "emit_signal", p_signal, p_payload);
}

// Match the EOS pattern: every callback's payload Dict carries `result_code`
// (an EOS_EResult int) so GDScript can branch via
// EpicServices.is_operation_complete(ret.result_code).
#define EPIC_PAYLOAD_RESULT_KEY "result_code"

#endif // EPIC_CALLBACK_H
