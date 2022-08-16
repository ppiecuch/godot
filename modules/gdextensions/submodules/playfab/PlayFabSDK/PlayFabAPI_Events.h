/*************************************************************************/
/*  PlayFabAPI_Events.h                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/* Auto-generated at Tue Aug 16 2022 20:53:56 GMT+0000 (Coordinated Universal Time) */

namespace Events {


int DeleteEventSamplingRatio(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes the sampling ratio for an event.
	// https://docs.microsoft.com/rest/api/playfab/events/playstream/deleteeventsamplingratio
	

	return _http_cli->request_append(
		"/Event/DeleteEventSamplingRatio",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetEventSamplingRatio(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the sampling ratio for an event.
	// https://docs.microsoft.com/rest/api/playfab/events/playstream/geteventsamplingratio
	

	return _http_cli->request_append(
		"/Event/GetEventSamplingRatio",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetEventSamplingRatios(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the event sampling ratios for a title.
	// https://docs.microsoft.com/rest/api/playfab/events/playstream/geteventsamplingratios
	

	return _http_cli->request_append(
		"/Event/GetEventSamplingRatios",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetEventSamplingRatio(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the sampling ratio for an event.
	// https://docs.microsoft.com/rest/api/playfab/events/playstream/seteventsamplingratio
	

	return _http_cli->request_append(
		"/Event/SetEventSamplingRatio",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int WriteEvents(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Write batches of entity based events to PlayStream. The namespace of the Event must be 'custom' or start with 'custom.'.
	// https://docs.microsoft.com/rest/api/playfab/events/playstream-events/writeevents
	

	return _http_cli->request_append(
		"/Event/WriteEvents",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int WriteTelemetryEvents(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Write batches of entity based events to as Telemetry events (bypass PlayStream). The namespace must be 'custom' or start
	// with 'custom.'
	// https://docs.microsoft.com/rest/api/playfab/events/playstream-events/writetelemetryevents
	

	return _http_cli->request_append(
		"/Event/WriteTelemetryEvents",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

} // namespace Events
