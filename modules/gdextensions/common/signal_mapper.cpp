#include "signal_mapper.h"

void SignalMapper::_forward(Array p_args) {
	ERR_FAIL_COND(p_args.size() < 2);

	String signal = p_args[p_args.size() - 1];
	Object *target = p_args[p_args.size() - 2];

	target->emit_signal(signal, p_args);
}

void SignalMapper::forward(Object *p_obj_from, const String &p_sig_from, Object *p_obj_to, const String &p_sig_to) {
	p_obj_from->connect(p_sig_from, this, "_forward", varray(p_obj_to, p_sig_to));
}

void SignalMapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_forward"), &SignalMapper::_forward);
}
