/*************************************************************************/
/*  PlayFabAPI_Data.h                                                    */
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

/* Auto-generated with SDKGenerator (don't manually edit) */

namespace Data {

int AbortFileUploads(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Abort pending file uploads to an entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/data/file/abortfileuploads

	return _http_cli->request_append(
			"/File/AbortFileUploads",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int DeleteFiles(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Delete files on an entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/data/file/deletefiles

	return _http_cli->request_append(
			"/File/DeleteFiles",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int FinalizeFileUploads(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Finalize file uploads to an entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/data/file/finalizefileuploads

	return _http_cli->request_append(
			"/File/FinalizeFileUploads",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int GetFiles(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves file metadata from an entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/data/file/getfiles

	return _http_cli->request_append(
			"/File/GetFiles",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int GetObjects(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Retrieves objects from an entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/data/object/getobjects

	return _http_cli->request_append(
			"/Object/GetObjects",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int InitiateFileUploads(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Initiates file uploads to an entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/data/file/initiatefileuploads

	return _http_cli->request_append(
			"/File/InitiateFileUploads",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int SetObjects(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Sets objects on an entity's profile.
	// https://docs.microsoft.com/rest/api/playfab/data/object/setobjects

	return _http_cli->request_append(
			"/Object/SetObjects",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

} // namespace Data
