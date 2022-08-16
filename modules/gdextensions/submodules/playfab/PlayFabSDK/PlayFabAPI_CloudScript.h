/*************************************************************************/
/*  PlayFabAPI_CloudScript.h                                             */
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

namespace CloudScript {


int ExecuteEntityCloudScript(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Cloud Script is one of PlayFab's most versatile features. It allows client code to request execution of any kind of
	// custom server-side functionality you can implement, and it can be used in conjunction with virtually anything.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/executeentitycloudscript
	

	return _http_cli->request_append(
		"/CloudScript/ExecuteEntityCloudScript",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ExecuteFunction(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Cloud Script is one of PlayFab's most versatile features. It allows client code to request execution of any kind of
	// custom server-side functionality you can implement, and it can be used in conjunction with virtually anything.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/executefunction
	

	return _http_cli->request_append(
		"/CloudScript/ExecuteFunction",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetFunction(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets registered Azure Functions for a given title id and function name.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/getfunction
	

	return _http_cli->request_append(
		"/CloudScript/GetFunction",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListFunctions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists all currently registered Azure Functions for a given title.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/listfunctions
	

	return _http_cli->request_append(
		"/CloudScript/ListFunctions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListHttpFunctions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists all currently registered HTTP triggered Azure Functions for a given title.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/listhttpfunctions
	

	return _http_cli->request_append(
		"/CloudScript/ListHttpFunctions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ListQueuedFunctions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Lists all currently registered Queue triggered Azure Functions for a given title.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/listqueuedfunctions
	

	return _http_cli->request_append(
		"/CloudScript/ListQueuedFunctions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int PostFunctionResultForEntityTriggeredAction(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Generate an entity PlayStream event for the provided function result.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/postfunctionresultforentitytriggeredaction
	

	return _http_cli->request_append(
		"/CloudScript/PostFunctionResultForEntityTriggeredAction",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int PostFunctionResultForFunctionExecution(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Generate an entity PlayStream event for the provided function result.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/postfunctionresultforfunctionexecution
	

	return _http_cli->request_append(
		"/CloudScript/PostFunctionResultForFunctionExecution",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int PostFunctionResultForPlayerTriggeredAction(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Generate a player PlayStream event for the provided function result.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/postfunctionresultforplayertriggeredaction
	

	return _http_cli->request_append(
		"/CloudScript/PostFunctionResultForPlayerTriggeredAction",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int PostFunctionResultForScheduledTask(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Generate a PlayStream event for the provided function result.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/postfunctionresultforscheduledtask
	

	return _http_cli->request_append(
		"/CloudScript/PostFunctionResultForScheduledTask",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RegisterHttpFunction(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Registers an HTTP triggered Azure function with a title.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/registerhttpfunction
	

	return _http_cli->request_append(
		"/CloudScript/RegisterHttpFunction",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RegisterQueuedFunction(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Registers a queue triggered Azure Function with a title.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/registerqueuedfunction
	

	return _http_cli->request_append(
		"/CloudScript/RegisterQueuedFunction",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UnregisterFunction(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Unregisters an Azure Function with a title.
	// https://docs.microsoft.com/rest/api/playfab/cloudscript/server-side-cloud-script/unregisterfunction
	

	return _http_cli->request_append(
		"/CloudScript/UnregisterFunction",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

} // namespace CloudScript
