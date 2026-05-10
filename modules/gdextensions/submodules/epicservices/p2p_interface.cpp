/**************************************************************************/
/*  p2p_interface.cpp                                                     */
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

#include "epic_callback.h"
#include "epic_utils.h"

#include "eos_p2p.h"

static EOS_P2P_SocketId _make_socket_id(const String &p_name) {
	EOS_P2P_SocketId sid = {};
	sid.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
	const CharString cs = p_name.utf8();
	int n = MIN(int(EOS_P2P_SOCKETID_SOCKETNAME_SIZE - 1), cs.length());
	for (int i = 0; i < n; ++i) {
		sid.SocketName[i] = cs[i];
	}
	sid.SocketName[n] = '\0';
	return sid;
}

int EpicServices::p2p_interface_send_packet(const Dictionary &p_options) {
	if (!p2p_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const PoolByteArray data = p_options.has("data") ? PoolByteArray(p_options["data"]) : PoolByteArray();
	PoolByteArray::Read read = data.read();
	const String socket_name = dict_get_string(p_options, "socket_name", String("default"));
	EOS_P2P_SocketId socket_id = _make_socket_id(socket_name);
	EOS_P2P_SendPacketOptions opts = {};
	opts.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.RemoteUserId = eos_pui_from_string(dict_get_string(p_options, "remote_user_id"));
	opts.SocketId = &socket_id;
	opts.Channel = uint8_t(dict_get_int(p_options, "channel", 0));
	opts.DataLengthBytes = uint32_t(data.size());
	opts.Data = data.size() ? read.ptr() : nullptr;
	opts.bAllowDelayedDelivery = dict_get_bool(p_options, "allow_delayed_delivery", true) ? EOS_TRUE : EOS_FALSE;
	opts.Reliability = EOS_EPacketReliability(dict_get_int(p_options, "reliability", int(EOS_EPacketReliability::EOS_PR_ReliableOrdered)));
	opts.bDisableAutoAcceptConnection = dict_get_bool(p_options, "disable_auto_accept_connection", false) ? EOS_TRUE : EOS_FALSE;
	return int(EOS_P2P_SendPacket(p2p_handle, &opts));
}

int EpicServices::p2p_interface_get_next_received_packet_size(const Dictionary &p_options) {
	if (!p2p_handle) {
		return 0;
	}
	uint8_t channel_buf = 0;
	bool has_channel = p_options.has("requested_channel");
	if (has_channel) {
		channel_buf = uint8_t(dict_get_int(p_options, "requested_channel", 0));
	}
	EOS_P2P_GetNextReceivedPacketSizeOptions opts = {};
	opts.ApiVersion = EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.RequestedChannel = has_channel ? &channel_buf : nullptr;
	uint32_t size = 0;
	if (EOS_P2P_GetNextReceivedPacketSize(p2p_handle, &opts, &size) != EOS_EResult::EOS_Success) {
		return 0;
	}
	return int(size);
}

Dictionary EpicServices::p2p_interface_receive_packet(const Dictionary &p_options) {
	Dictionary out;
	if (!p2p_handle) {
		out["result_code"] = int(EOS_EResult::EOS_NotConfigured);
		return out;
	}
	uint8_t channel_buf = 0;
	bool has_channel = p_options.has("requested_channel");
	if (has_channel) {
		channel_buf = uint8_t(dict_get_int(p_options, "requested_channel", 0));
	}
	uint32_t max_bytes = uint32_t(dict_get_int(p_options, "max_data_size_bytes", 4096));
	EOS_P2P_ReceivePacketOptions opts = {};
	opts.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.MaxDataSizeBytes = max_bytes;
	opts.RequestedChannel = has_channel ? &channel_buf : nullptr;

	PoolByteArray data;
	data.resize(int(max_bytes));
	PoolByteArray::Write write = data.write();
	EOS_ProductUserId peer_id = nullptr;
	EOS_P2P_SocketId socket_id = {};
	socket_id.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
	uint8_t out_channel = 0;
	uint32_t bytes_written = 0;
	EOS_EResult r = EOS_P2P_ReceivePacket(p2p_handle, &opts, &peer_id, &socket_id, &out_channel, write.ptr(), &bytes_written);
	out["result_code"] = int(r);
	if (r == EOS_EResult::EOS_Success) {
		data.resize(int(bytes_written));
		out["peer_id"] = eos_pui_to_string(peer_id);
		out["socket_name"] = String::utf8(socket_id.SocketName);
		out["channel"] = int(out_channel);
		out["data"] = data;
	}
	return out;
}

int EpicServices::p2p_interface_accept_connection(const Dictionary &p_options) {
	if (!p2p_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const String socket_name = dict_get_string(p_options, "socket_name", String("default"));
	EOS_P2P_SocketId socket_id = _make_socket_id(socket_name);
	EOS_P2P_AcceptConnectionOptions opts = {};
	opts.ApiVersion = EOS_P2P_ACCEPTCONNECTION_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.RemoteUserId = eos_pui_from_string(dict_get_string(p_options, "remote_user_id"));
	opts.SocketId = &socket_id;
	return int(EOS_P2P_AcceptConnection(p2p_handle, &opts));
}

int EpicServices::p2p_interface_close_connection(const Dictionary &p_options) {
	if (!p2p_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const String socket_name = dict_get_string(p_options, "socket_name", String("default"));
	EOS_P2P_SocketId socket_id = _make_socket_id(socket_name);
	EOS_P2P_CloseConnectionOptions opts = {};
	opts.ApiVersion = EOS_P2P_CLOSECONNECTION_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.RemoteUserId = eos_pui_from_string(dict_get_string(p_options, "remote_user_id"));
	opts.SocketId = &socket_id;
	return int(EOS_P2P_CloseConnection(p2p_handle, &opts));
}

int EpicServices::p2p_interface_close_connections(const Dictionary &p_options) {
	if (!p2p_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const String socket_name = dict_get_string(p_options, "socket_name", String("default"));
	EOS_P2P_SocketId socket_id = _make_socket_id(socket_name);
	EOS_P2P_CloseConnectionsOptions opts = {};
	opts.ApiVersion = EOS_P2P_CLOSECONNECTIONS_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.SocketId = &socket_id;
	return int(EOS_P2P_CloseConnections(p2p_handle, &opts));
}

static void EOS_CALL _on_p2p_query_nat(const EOS_P2P_OnQueryNATTypeCompleteInfo *data) {
	EpicCallback *cb = (EpicCallback *)data->ClientData;
	Dictionary payload;
	payload[EPIC_PAYLOAD_RESULT_KEY] = int(data->ResultCode);
	payload["nat_type"] = int(data->NATType);
	epic_emit_deferred(cb->singleton_id, cb->signal_name, payload);
	memdelete(cb);
}

void EpicServices::p2p_interface_query_nat_type() {
	if (!p2p_handle) {
		Dictionary payload;
		payload[EPIC_PAYLOAD_RESULT_KEY] = int(EOS_EResult::EOS_NotConfigured);
		_emit_deferred("p2p_interface_query_nat_type_callback", payload);
		return;
	}
	EOS_P2P_QueryNATTypeOptions opts = {};
	opts.ApiVersion = EOS_P2P_QUERYNATTYPE_API_LATEST;
	EpicCallback *cb = memnew(EpicCallback(get_instance_id(), StringName("p2p_interface_query_nat_type_callback")));
	EOS_P2P_QueryNATType(p2p_handle, &opts, cb, &_on_p2p_query_nat);
}

int EpicServices::p2p_interface_get_nat_type() {
	if (!p2p_handle) {
		return int(EOS_ENATType::EOS_NAT_Unknown);
	}
	EOS_P2P_GetNATTypeOptions opts = {};
	opts.ApiVersion = EOS_P2P_GETNATTYPE_API_LATEST;
	EOS_ENATType type = EOS_ENATType::EOS_NAT_Unknown;
	EOS_P2P_GetNATType(p2p_handle, &opts, &type);
	return int(type);
}

int EpicServices::p2p_interface_set_relay_control(int p_relay_control) {
	if (!p2p_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_P2P_SetRelayControlOptions opts = {};
	opts.ApiVersion = EOS_P2P_SETRELAYCONTROL_API_LATEST;
	opts.RelayControl = EOS_ERelayControl(p_relay_control);
	return int(EOS_P2P_SetRelayControl(p2p_handle, &opts));
}

int EpicServices::p2p_interface_get_relay_control() {
	if (!p2p_handle) {
		return int(EOS_ERelayControl::EOS_RC_NoRelays);
	}
	EOS_P2P_GetRelayControlOptions opts = {};
	opts.ApiVersion = EOS_P2P_GETRELAYCONTROL_API_LATEST;
	EOS_ERelayControl ctrl = EOS_ERelayControl::EOS_RC_NoRelays;
	EOS_P2P_GetRelayControl(p2p_handle, &opts, &ctrl);
	return int(ctrl);
}

int EpicServices::p2p_interface_set_port_range(int p_port, int p_max_additional_ports_to_try) {
	if (!p2p_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_P2P_SetPortRangeOptions opts = {};
	opts.ApiVersion = EOS_P2P_SETPORTRANGE_API_LATEST;
	opts.Port = uint16_t(p_port);
	opts.MaxAdditionalPortsToTry = uint16_t(p_max_additional_ports_to_try);
	return int(EOS_P2P_SetPortRange(p2p_handle, &opts));
}

Dictionary EpicServices::p2p_interface_get_port_range() {
	Dictionary out;
	if (!p2p_handle) {
		return out;
	}
	EOS_P2P_GetPortRangeOptions opts = {};
	opts.ApiVersion = EOS_P2P_GETPORTRANGE_API_LATEST;
	uint16_t port = 0;
	uint16_t additional = 0;
	if (EOS_P2P_GetPortRange(p2p_handle, &opts, &port, &additional) == EOS_EResult::EOS_Success) {
		out["port"] = int(port);
		out["max_additional_ports_to_try"] = int(additional);
	}
	return out;
}

int EpicServices::p2p_interface_set_packet_queue_size(int64_t p_max_in, int64_t p_max_out) {
	if (!p2p_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	EOS_P2P_SetPacketQueueSizeOptions opts = {};
	opts.ApiVersion = EOS_P2P_SETPACKETQUEUESIZE_API_LATEST;
	opts.IncomingPacketQueueMaxSizeBytes = uint64_t(p_max_in);
	opts.OutgoingPacketQueueMaxSizeBytes = uint64_t(p_max_out);
	return int(EOS_P2P_SetPacketQueueSize(p2p_handle, &opts));
}

Dictionary EpicServices::p2p_interface_get_packet_queue_info() {
	Dictionary out;
	if (!p2p_handle) {
		return out;
	}
	EOS_P2P_GetPacketQueueInfoOptions opts = {};
	opts.ApiVersion = EOS_P2P_GETPACKETQUEUEINFO_API_LATEST;
	EOS_P2P_PacketQueueInfo info = {};
	if (EOS_P2P_GetPacketQueueInfo(p2p_handle, &opts, &info) == EOS_EResult::EOS_Success) {
		out["incoming_packet_queue_max_size_bytes"] = int64_t(info.IncomingPacketQueueMaxSizeBytes);
		out["incoming_packet_queue_current_size_bytes"] = int64_t(info.IncomingPacketQueueCurrentSizeBytes);
		out["incoming_packet_queue_current_packet_count"] = int64_t(info.IncomingPacketQueueCurrentPacketCount);
		out["outgoing_packet_queue_max_size_bytes"] = int64_t(info.OutgoingPacketQueueMaxSizeBytes);
		out["outgoing_packet_queue_current_size_bytes"] = int64_t(info.OutgoingPacketQueueCurrentSizeBytes);
		out["outgoing_packet_queue_current_packet_count"] = int64_t(info.OutgoingPacketQueueCurrentPacketCount);
	}
	return out;
}

int EpicServices::p2p_interface_clear_packet_queue(const Dictionary &p_options) {
	if (!p2p_handle) {
		return int(EOS_EResult::EOS_NotConfigured);
	}
	const String socket_name = dict_get_string(p_options, "socket_name", String("default"));
	EOS_P2P_SocketId socket_id = _make_socket_id(socket_name);
	EOS_P2P_ClearPacketQueueOptions opts = {};
	opts.ApiVersion = EOS_P2P_CLEARPACKETQUEUE_API_LATEST;
	opts.LocalUserId = eos_pui_from_string(dict_get_string(p_options, "local_user_id"));
	opts.RemoteUserId = eos_pui_from_string(dict_get_string(p_options, "remote_user_id"));
	opts.SocketId = &socket_id;
	return int(EOS_P2P_ClearPacketQueue(p2p_handle, &opts));
}

// Notification helper macros: each takes an optional socket name filter.
#define P2P_NOTIFY_HELPER(NAME, OPT_TY, OPT_API, ADD_FN, REMOVE_FN, INFO_TY, SIGNAL, FILL_BLOCK) \
	static void EOS_CALL _on_p2p_##NAME(const INFO_TY *data) {                                   \
		EpicServices *es = EpicServices::get_singleton();                                        \
		if (!es) {                                                                               \
			return;                                                                              \
		}                                                                                        \
		Dictionary payload;                                                                      \
		FILL_BLOCK                                                                               \
		epic_emit_deferred(es->get_instance_id(), StringName(SIGNAL), payload);                  \
	}                                                                                            \
	uint64_t EpicServices::p2p_interface_add_notify_##NAME(const String &p_socket_name) {        \
		if (!p2p_handle) {                                                                       \
			return 0;                                                                            \
		}                                                                                        \
		OPT_TY opts = {};                                                                        \
		opts.ApiVersion = OPT_API;                                                               \
		EOS_P2P_SocketId sid = _make_socket_id(p_socket_name);                                   \
		opts.SocketId = p_socket_name.empty() ? nullptr : &sid;                                  \
		return uint64_t(ADD_FN(p2p_handle, &opts, nullptr, &_on_p2p_##NAME));                    \
	}                                                                                            \
	void EpicServices::p2p_interface_remove_notify_##NAME(uint64_t p_id) {                       \
		if (!p2p_handle) {                                                                       \
			return;                                                                              \
		}                                                                                        \
		REMOVE_FN(p2p_handle, EOS_NotificationId(p_id));                                         \
	}

P2P_NOTIFY_HELPER(
		peer_connection_request,
		EOS_P2P_AddNotifyPeerConnectionRequestOptions,
		EOS_P2P_ADDNOTIFYPEERCONNECTIONREQUEST_API_LATEST,
		EOS_P2P_AddNotifyPeerConnectionRequest,
		EOS_P2P_RemoveNotifyPeerConnectionRequest,
		EOS_P2P_OnIncomingConnectionRequestInfo,
		"p2p_interface_peer_connection_request",
		{
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["remote_user_id"] = eos_pui_to_string(data->RemoteUserId);
			payload["socket_name"] = data->SocketId ? String::utf8(data->SocketId->SocketName) : String();
		})

P2P_NOTIFY_HELPER(
		peer_connection_established,
		EOS_P2P_AddNotifyPeerConnectionEstablishedOptions,
		EOS_P2P_ADDNOTIFYPEERCONNECTIONESTABLISHED_API_LATEST,
		EOS_P2P_AddNotifyPeerConnectionEstablished,
		EOS_P2P_RemoveNotifyPeerConnectionEstablished,
		EOS_P2P_OnPeerConnectionEstablishedInfo,
		"p2p_interface_peer_connection_established",
		{
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["remote_user_id"] = eos_pui_to_string(data->RemoteUserId);
			payload["socket_name"] = data->SocketId ? String::utf8(data->SocketId->SocketName) : String();
			payload["connection_type"] = int(data->ConnectionType);
			payload["network_type"] = int(data->NetworkType);
		})

P2P_NOTIFY_HELPER(
		peer_connection_interrupted,
		EOS_P2P_AddNotifyPeerConnectionInterruptedOptions,
		EOS_P2P_ADDNOTIFYPEERCONNECTIONINTERRUPTED_API_LATEST,
		EOS_P2P_AddNotifyPeerConnectionInterrupted,
		EOS_P2P_RemoveNotifyPeerConnectionInterrupted,
		EOS_P2P_OnPeerConnectionInterruptedInfo,
		"p2p_interface_peer_connection_interrupted",
		{
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["remote_user_id"] = eos_pui_to_string(data->RemoteUserId);
			payload["socket_name"] = data->SocketId ? String::utf8(data->SocketId->SocketName) : String();
		})

P2P_NOTIFY_HELPER(
		peer_connection_closed,
		EOS_P2P_AddNotifyPeerConnectionClosedOptions,
		EOS_P2P_ADDNOTIFYPEERCONNECTIONCLOSED_API_LATEST,
		EOS_P2P_AddNotifyPeerConnectionClosed,
		EOS_P2P_RemoveNotifyPeerConnectionClosed,
		EOS_P2P_OnRemoteConnectionClosedInfo,
		"p2p_interface_peer_connection_closed",
		{
			payload["local_user_id"] = eos_pui_to_string(data->LocalUserId);
			payload["remote_user_id"] = eos_pui_to_string(data->RemoteUserId);
			payload["socket_name"] = data->SocketId ? String::utf8(data->SocketId->SocketName) : String();
			payload["reason"] = int(data->Reason);
		})
