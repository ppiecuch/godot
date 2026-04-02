/**************************************************************************/
/*  ofevents.h                                                            */
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

#ifndef OF_EVENTS_H
#define OF_EVENTS_H

#include "common/gd_core.h"
#include "core/os/input_event.h"
#include "core/reference.h"

#include <functional>
#include <memory>

enum ofEventOrder {
	OF_EVENT_ORDER_BEFORE_APP = 0,
	OF_EVENT_ORDER_APP = 100,
	OF_EVENT_ORDER_AFTER_APP = 200,
};

class ofEventArgs {};
class ofKeyEventArgs : public ofEventArgs {};

namespace priv {
class BaseFunctionId {
public:
	virtual ~BaseFunctionId(){};
	virtual bool operator==(const BaseFunctionId &) const = 0;
};

template <typename T>
class Function {
	std::function<bool(const void *, T &)> function;

public:
	Function(int priority, std::function<bool(const void *, T &)> function, std::unique_ptr<BaseFunctionId> &&id) :
			priority(priority), id(std::move(id)), function(function) {}

	bool operator==(const Function<T> &f) const { return f.priority == priority && *id == *f.id; }

	_FORCE_INLINE_ bool notify(const void *s, T &t) {
		if (function) {
			return function(s, t);
		}
		return false;
	}

	_FORCE_INLINE_ void disable() { function = nullptr; }

	int priority;
	std::unique_ptr<BaseFunctionId> id;
};

class Function<void> {
	std::function<bool(const void *)> function;

public:
	Function(int priority, std::function<bool(const void *)> function, std::unique_ptr<BaseFunctionId> &&id) :
			priority(priority), id(std::move(id)), function(function) {}

	bool operator==(const Function<void> &f) const { return f.priority == priority && *id == *f.id; }

	_FORCE_INLINE_ bool notify(const void *s) {
		if (function) {
			return function(s);
		}
		return false;
	}

	_FORCE_INLINE_ void disable() { function = nullptr; }

	int priority;
	std::unique_ptr<BaseFunctionId> id;
};
} //namespace priv

template <typename T, typename Function = priv::Function<T>>
class ofEvent {
protected:
	template <typename TFunction>
	void add(TFunction &&f) {
		auto it = functions.begin();
		for (; it != functions.end(); ++it) {
			if ((*it)->priority > f->priority)
				break;
		}
		functions.emplace(it, f);
	}

	template <typename TFunction>
	void remove(const TFunction &function) {
		auto it = functions.begin();
		for (; it != functions.end(); ++it) {
			auto f = *it;
			if (*f == *function) {
				f->disable();
				functions.erase(it);
				break;
			}
		}
	}

	template <typename TFunction>
	void replace(const TFunction &function) {
		remove(function);
		add(function);
	}

	std::vector<std::shared_ptr<Function>> functions;
	bool enabled;
};

//----------------------------------------------------
// register any method of any class to an event.
// the method must provide one of the following
// signatures:
//     void method(ArgumentsType & args)
//     void method(const void * sender, ArgumentsType &args)
// ie:
//     ofAddListener(addon.newIntEvent, this, &Class::method)

template <class EventType, typename ArgumentsType, class ListenerClass>
void ofAddListener(EventType &event, ListenerClass *listener, void (ListenerClass::*listenerMethod)(const void *, ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listener, listenerMethod, prio); }

template <class EventType, typename ArgumentsType, class ListenerClass>
void ofAddListener(EventType &event, ListenerClass *listener, void (ListenerClass::*listenerMethod)(ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listener, listenerMethod, prio); }

template <class ListenerClass>
void ofAddListener(ofEvent<void> &event, ListenerClass *listener, void (ListenerClass::*listenerMethod)(const void *), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listener, listenerMethod, prio); }

template <class ListenerClass>
void ofAddListener(ofEvent<void> &event, ListenerClass *listener, void (ListenerClass::*listenerMethod)(), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listener, listenerMethod, prio); }

template <class EventType, typename ArgumentsType, class ListenerClass>
void ofAddListener(EventType &event, ListenerClass *listener, bool (ListenerClass::*listenerMethod)(const void *, ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listener, listenerMethod, prio); }

template <class EventType, typename ArgumentsType, class ListenerClass>
void ofAddListener(EventType &event, ListenerClass *listener, bool (ListenerClass::*listenerMethod)(ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listener, listenerMethod, prio); }

template <class ListenerClass>
void ofAddListener(ofEvent<void> &event, ListenerClass *listener, bool (ListenerClass::*listenerMethod)(const void *), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listener, listenerMethod, prio); }

template <class ListenerClass>
void ofAddListener(ofEvent<void> &event, ListenerClass *listener, bool (ListenerClass::*listenerMethod)(), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listener, listenerMethod, prio); }

template <class EventType, typename ArgumentsType>
void ofAddListener(EventType &event, void (*listenerFunction)(const void *, ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listenerFunction, prio); }

template <class EventType, typename ArgumentsType>
void ofAddListener(EventType &event, void (*listenerFunction)(ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listenerFunction, prio); }

_FORCE_INLINE_ void ofAddListener(ofEvent<void> &event, void (*listenerFunction)(const void *), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listenerFunction, prio); }
_FORCE_INLINE_ void ofAddListener(ofEvent<void> &event, void (*listenerFunction)(), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listenerFunction, prio); }

template <class EventType, typename ArgumentsType>
void ofAddListener(EventType &event, bool (*listenerFunction)(const void *, ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listenerFunction, prio); }

template <class EventType, typename ArgumentsType>
void ofAddListener(EventType &event, bool (*listenerFunction)(ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(*listenerFunction, prio); }

_FORCE_INLINE_ void ofAddListener(ofEvent<void> &event, bool (*listenerFunction)(const void *), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(*listenerFunction, prio); }
_FORCE_INLINE_ void ofAddListener(ofEvent<void> &event, bool (*listenerFunction)(), int prio = OF_EVENT_ORDER_AFTER_APP) { event.replace(listenerFunction, prio); }

//----------------------------------------------------
// unregister any method of any class to an event.
// the method must provide one the following
// signatures:
//     void method(ArgumentsType & args)
//     void method(const void * sender, ArgumentsType &args)
// ie:
//     ofAddListener(addon.newIntEvent, this, &Class::method)

template <class EventType, typename ArgumentsType, class ListenerClass>
void ofRemoveListener(EventType &event, ListenerClass *listener, void (ListenerClass::*listenerMethod)(const void *, ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listener, listenerMethod, prio); }

template <class EventType, typename ArgumentsType, class ListenerClass>
void ofRemoveListener(EventType &event, ListenerClass *listener, void (ListenerClass::*listenerMethod)(ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listener, listenerMethod, prio); }

template <class ListenerClass>
void ofRemoveListener(ofEvent<void> &event, ListenerClass *listener, void (ListenerClass::*listenerMethod)(const void *), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listener, listenerMethod, prio); }

template <class ListenerClass>
void ofRemoveListener(ofEvent<void> &event, ListenerClass *listener, void (ListenerClass::*listenerMethod)(), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listener, listenerMethod, prio); }

template <class EventType, typename ArgumentsType, class ListenerClass>
void ofRemoveListener(EventType &event, ListenerClass *listener, bool (ListenerClass::*listenerMethod)(const void *, ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listener, listenerMethod, prio); }

template <class EventType, typename ArgumentsType, class ListenerClass>
void ofRemoveListener(EventType &event, ListenerClass *listener, bool (ListenerClass::*listenerMethod)(ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listener, listenerMethod, prio); }

template <class ListenerClass>
void ofRemoveListener(ofEvent<void> &event, ListenerClass *listener, bool (ListenerClass::*listenerMethod)(const void *), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listener, listenerMethod, prio); }

template <class ListenerClass>
void ofRemoveListener(ofEvent<void> &event, ListenerClass *listener, bool (ListenerClass::*listenerMethod)(), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listener, listenerMethod, prio); }

template <class EventType, typename ArgumentsType>
void ofRemoveListener(EventType &event, void (*listenerFunction)(const void *, ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listenerFunction, prio); }

template <class EventType, typename ArgumentsType>
void ofRemoveListener(EventType &event, void (*listenerFunction)(ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listenerFunction, prio); }

inline void ofRemoveListener(ofEvent<void> &event, void (*listenerFunction)(const void *), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listenerFunction, prio); }

inline void ofRemoveListener(ofEvent<void> &event, void (*listenerFunction)(), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listenerFunction, prio); }

template <class EventType, typename ArgumentsType>
void ofRemoveListener(EventType &event, bool (*listenerFunction)(const void *, ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listenerFunction, prio); }

template <class EventType, typename ArgumentsType>
void ofRemoveListener(EventType &event, bool (*listenerFunction)(ArgumentsType &), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listenerFunction, prio); }

_FORCE_INLINE_ void ofRemoveListener(ofEvent<void> &event, bool (*listenerFunction)(const void *), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listenerFunction, prio); }

_FORCE_INLINE_ void ofRemoveListener(ofEvent<void> &event, bool (*listenerFunction)(), int prio = OF_EVENT_ORDER_AFTER_APP) { event.remove(listenerFunction, prio); }

//----------------------------------------------------
// notifies an event so all the registered listeners
// get called
// ie:
//   ofNotifyEvent(addon.newIntEvent, intArgument, this)
//
// or in case there's no sender:
//   ofNotifyEvent(addon.newIntEvent, intArgument)

template <class EventType, typename ArgumentsType, typename SenderType>
_FORCE_INLINE_ void ofNotifyEvent(EventType &event, ArgumentsType &args, SenderType *sender) {
	event.notify(sender, args);
}

template <class EventType, typename ArgumentsType>
_FORCE_INLINE_ void ofNotifyEvent(EventType &event, ArgumentsType &args) {
	event.notify(nullptr, args);
}

template <class EventType, typename ArgumentsType, typename SenderType>
_FORCE_INLINE_ void ofNotifyEvent(EventType &event, const ArgumentsType &args, SenderType *sender) {
	event.notify(sender, args);
}

template <class EventType, typename ArgumentsType>
_FORCE_INLINE_ void ofNotifyEvent(EventType &event, const ArgumentsType &args) {
	event.notify(nullptr, args);
}

template <typename SenderType>
_FORCE_INLINE_ void ofNotifyEvent(ofEvent<void> &event, SenderType *sender) {
	event.notify(sender);
}

_FORCE_INLINE_ void ofNotifyEvent(ofEvent<void> &event) {
	event.notify(nullptr);
}

#endif // OF_EVENTS_H
