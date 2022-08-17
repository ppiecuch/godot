/*************************************************************************/
/*  PlayFabAPI_Groups.h                                                  */
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

namespace Groups {

int AcceptGroupApplication(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Accepts an outstanding invitation to to join a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/acceptgroupapplication

	return _http_cli->request_append(
			"/Group/AcceptGroupApplication",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int AcceptGroupInvitation(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Accepts an invitation to join a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/acceptgroupinvitation

	return _http_cli->request_append(
			"/Group/AcceptGroupInvitation",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int AddMembers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Adds members to a group or role.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/addmembers

	return _http_cli->request_append(
			"/Group/AddMembers",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ApplyToGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Applies to join a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/applytogroup

	return _http_cli->request_append(
			"/Group/ApplyToGroup",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int BlockEntity(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Blocks a list of entities from joining a group.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/blockentity

	return _http_cli->request_append(
			"/Group/BlockEntity",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ChangeMemberRole(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Changes the role membership of a list of entities from one role to another.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/changememberrole

	return _http_cli->request_append(
			"/Group/ChangeMemberRole",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int CreateGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Creates a new group.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/creategroup

	return _http_cli->request_append(
			"/Group/CreateGroup",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int CreateRole(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Creates a new group role.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/createrole

	return _http_cli->request_append(
			"/Group/CreateRole",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int DeleteGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Deletes a group and all roles, invitations, join requests, and blocks associated with it.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/deletegroup

	return _http_cli->request_append(
			"/Group/DeleteGroup",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int DeleteRole(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Deletes an existing role in a group.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/deleterole

	return _http_cli->request_append(
			"/Group/DeleteRole",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int GetGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Gets information about a group and its roles
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/getgroup

	return _http_cli->request_append(
			"/Group/GetGroup",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int InviteToGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Invites a player to join a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/invitetogroup

	return _http_cli->request_append(
			"/Group/InviteToGroup",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int IsMember(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Checks to see if an entity is a member of a group or role within the group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/ismember

	return _http_cli->request_append(
			"/Group/IsMember",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ListGroupApplications(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Lists all outstanding requests to join a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/listgroupapplications

	return _http_cli->request_append(
			"/Group/ListGroupApplications",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ListGroupBlocks(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Lists all entities blocked from joining a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/listgroupblocks

	return _http_cli->request_append(
			"/Group/ListGroupBlocks",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ListGroupInvitations(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Lists all outstanding invitations for a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/listgroupinvitations

	return _http_cli->request_append(
			"/Group/ListGroupInvitations",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ListGroupMembers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Lists all members for a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/listgroupmembers

	return _http_cli->request_append(
			"/Group/ListGroupMembers",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ListMembership(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Lists all groups and roles for an entity
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/listmembership

	return _http_cli->request_append(
			"/Group/ListMembership",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int ListMembershipOpportunities(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Lists all outstanding invitations and group applications for an entity
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/listmembershipopportunities

	return _http_cli->request_append(
			"/Group/ListMembershipOpportunities",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int RemoveGroupApplication(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Removes an application to join a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/removegroupapplication

	return _http_cli->request_append(
			"/Group/RemoveGroupApplication",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int RemoveGroupInvitation(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Removes an invitation join a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/removegroupinvitation

	return _http_cli->request_append(
			"/Group/RemoveGroupInvitation",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int RemoveMembers(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Removes members from a group.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/removemembers

	return _http_cli->request_append(
			"/Group/RemoveMembers",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int UnblockEntity(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Unblocks a list of entities from joining a group
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/unblockentity

	return _http_cli->request_append(
			"/Group/UnblockEntity",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int UpdateGroup(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Updates non-membership data about a group.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/updategroup

	return _http_cli->request_append(
			"/Group/UpdateGroup",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

int UpdateRole(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	// Updates metadata about a role.
	// https://docs.microsoft.com/rest/api/playfab/groups/groups/updaterole

	return _http_cli->request_append(
			"/Group/UpdateRole",
			dict_request,
			user_callback,
			dict_header_extra,
			array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
			Array());
}

} // namespace Groups
