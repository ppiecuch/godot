#ifndef NAKAMA1_API_H
#define NAKAMA1_API_H

#include "core/class_db.h"
#include "core/object.h"
#include "core/reference.h"
#include "core/variant.h"
#include "api_generated.h"

typedef int32_t NkErrorCode;

#define LOGI(string, ...) print_line(vformat(String("(Nakama Info) ")+string, ##__VA_ARGS__));
#define LOGD(string, ...) print_line(vformat(String("(Nakama Debug) ")+string, ##__VA_ARGS__));

struct Utils {

	_FORCE_INLINE_ static PoolByteArray create_payload(uint8_t *buf_ptr, int buf_size) {
		PoolByteArray data;
		data.resize(buf_size);
		memcpy(data.write().ptr(), buf_ptr, buf_size);
		return data;
	}

	_FORCE_INLINE_ static String bytearray_to_string(const PoolByteArray &p_data) {
		String s;
		if (p_data.size() >= 0) {
			PoolByteArray::Read r = p_data.read();
			CharString cs;
			cs.resize(p_data.size() + 1);
			memcpy(cs.ptrw(), r.ptr(), p_data.size());
			cs[p_data.size()] = 0;
			s = cs.get_data();
		}
		return s;
	}

	_FORCE_INLINE_ static const char *string_c_str(const String &p_str) {
		return p_str.utf8().get_data();
	}

	_FORCE_INLINE_ static PoolByteArray data_to_bytearray(const uint8_t *p_data, int p_len) {
		PoolByteArray data;
		data.resize(p_len);
		memcpy(data.write().ptr(), p_data, p_len);
		return data;
	}

	template<typename T, typename... Args>
	static std::unique_ptr<T> make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

    template <typename T>
    struct _fbloop {
		struct iterator {
			iterator(const flatbuffers::Vector<flatbuffers::Offset<T>> *array, int pointer): array_(array), index_(pointer) { }
			iterator operator++() { ++index_; return *this; }
			bool operator!=(const iterator &other) const { return (array_ != other.array_) || (index_ != other.index_); }
			const T *operator*() const { return array_->Get(index_); }
			const flatbuffers::Vector<flatbuffers::Offset<T>> *array_;
			int index_;
		};
		_fbloop(const flatbuffers::Vector<flatbuffers::Offset<T>> *array) : array_(array) { }
		iterator begin() const noexcept { return iterator(array_, 0); }
		iterator end() const noexcept { return iterator(array_, array_->size()); }
		const flatbuffers::Vector<flatbuffers::Offset<T>> *array_;
	};

	template <typename T>
	static _fbloop<T> mkfbloop(const flatbuffers::Vector<flatbuffers::Offset<T>> *array) noexcept { return { array }; }
};

// A message which returns a response from the server.
class NkCollatedMessage : public Reference {
	GDCLASS(NkCollatedMessage, Reference);

public:
	// The serialized format of the message.
	// p_collation_id : The collation ID to assign to the serialized message instance.
	virtual PoolByteArray as_bytes(String p_collation_id) const = 0;
};

// A message which requires no acknowledgement by the server.
class NkUncollatedMessage : public Reference {
	GDCLASS(NkUncollatedMessage, Reference);

public:
	// The serialized format of the message.
	virtual PoolByteArray as_bytes() const = 0;
};

enum {
  RUNTIME_EXCEPTION = 0, // An unexpected error that is unrecoverable.
  UNRECOGNIZED_PAYLOAD = 1, // Server received a message that is not recognized.
  MISSING_PAYLOAD = 2, // Server received an Envelop message but the internal message is unrecognised. Most likely a protocol mismatch.
  BAD_INPUT = 3, // The message did not include the required data in the correct format.
  AUTH_ERROR = 4, // Authentication failure.
  USER_NOT_FOUND = 5, // Login failed because ID/device/email did not exist.
  USER_REGISTER_INUSE = 6, // Registration failed because ID/device/email exists.
  USER_LINK_INUSE = 7, // Linking operation failed because link exists.
  USER_LINK_PROVIDER_UNAVAILABLE = 8, // Linking operation failed because third-party service was unreachable.
  USER_UNLINK_DISALLOWED = 9, // Unlinking operation failed because you cannot unlink last ID.
  USER_HANDLE_INUSE = 10, // Handle is in-use by another user.
  GROUP_NAME_INUSE = 11, // Group names must be unique and it's already in use.
  GROUP_LAST_ADMIN = 12, // Group leave operation not allowed because the user is the last admin.
  STORAGE_REJECTED = 13, // Storage write operation failed.
  MATCH_NOT_FOUND = 14, // Match with given ID was not found in the system.
  RUNTIME_FUNCTION_NOT_FOUND = 15, // Runtime function name was not found in system registry.
  RUNTIME_FUNCTION_EXCEPTION = 16, // Runtime function caused an internal server error and did not complete.
};

class DefaultAuthenticateRequest: public NkCollatedMessage {

	server::AuthenticateRequestT *message = nullptr;

public:
	class Builder {
		public:
			static Ref<DefaultAuthenticateRequest> custom(String p_custom);
			static Ref<DefaultAuthenticateRequest> device(String p_device);
			static Ref<DefaultAuthenticateRequest> email(String p_email, String p_password);
			static Ref<DefaultAuthenticateRequest> facebook(String p_oauth_token);
			static Ref<DefaultAuthenticateRequest> game_center(String p_player_id, String p_bundle_id, long p_timestamp, String p_salt, String p_signature, String p_public_key_url);
			static Ref<DefaultAuthenticateRequest> google(String p_oauth_token);
			static Ref<DefaultAuthenticateRequest> steam(String p_session_token);
	};

	PoolByteArray as_bytes(String p_collation_id) const;

	DefaultAuthenticateRequest(server::AuthenticateRequestT *p_message);
	~DefaultAuthenticateRequest();
};

class AuthenticateResponse {

	const PoolByteArray payload;
	const server::AuthenticateResponse *message;

public:
	const AuthenticateResponse *get() const;

	String get_collation_id() const;
	bool is_valid() const;
	bool is_error() const;
	NkErrorCode get_error_code() const;
	String get_error_message() const;
	bool is_session() const;
	String get_session_token() const;

	AuthenticateResponse(PoolByteArray p_payload);
};

class NkMessage : public Object {
	GDCLASS(NkMessage, Object);

	PoolByteArray payload;
	const server::Envelope *message;

protected:
	static void _bind_methods();

public:
	enum PayloadCase {
		PAYLOAD_NOT_SET,
		// events:
		PAYLOAD_HEARTBEAT,
		PAYLOAD_TOPIC_MESSAGE,
		PAYLOAD_TOPIC_PRESENCE,
		PAYLOAD_MATCH_DATA,
		PAYLOAD_MATCH_PRESENCE,
		PAYLOAD_MATCHMAKE_MATCHED,
		PAYLOAD_LIVE_NOTIFICATIONS,
		// responses:
		PAYLOAD_ERROR,
		PAYLOAD_SELF,
		PAYLOAD_USERS,
		PAYLOAD_FRIENDS,
		PAYLOAD_GROUPS,
		PAYLOAD_GROUPS_SELF,
		PAYLOAD_GROUP_USERS,
		PAYLOAD_STORAGE_DATA,
		PAYLOAD_STORAGE_KEYS,
		PAYLOAD_RPC,
		PAYLOAD_TOPICS,
		PAYLOAD_TOPIC_MESSAGE_ACK,
		PAYLOAD_TOPIC_MESSAGES,
		PAYLOAD_MATCH,
		PAYLOAD_MATCHES,
		PAYLOAD_MATCHMAKE_TICKET,
		PAYLOAD_LEADERBOARDS,
		PAYLOAD_LEADERBOARD_RECORDS,
		PAYLOAD_NOTIFICATIONS,
	};
	enum TopicType {
		TOPIC_DIRECTMESSAGE,
		TOPIC_ROOM,
		TOPIC_GROUP,
	};
	enum ScoreOperator {
		SUBMITOP_INCR ,
		SUBMITOP_DECR,
		SUBMITOP_SET,
		SUBMITOP_BEST,
	};
	const NkMessage *get() const;
	String get_collation_id() const;
	PayloadCase get_payload_case() const;
	const server::Envelope_::EnvelopeContent *get_envelope_payload() const;

	NkMessage(PoolByteArray p_payload);
};

VARIANT_ENUM_CAST(NkMessage::PayloadCase);
VARIANT_ENUM_CAST(NkMessage::TopicType);
VARIANT_ENUM_CAST(NkMessage::ScoreOperator);

template <typename T> class DeferredMessage : public Reference {
	GDCLASS(DeferredMessage, Reference);

protected:
	static void _bind_methods() {

		ClassDB::bind_method(D_METHOD("get_content"), &DeferredMessage::get_content);
	}

public:
	const T *message;
	const PoolByteArray payload;

	Dictionary get_content() const { return message->UnPackToDict(); }

	DeferredMessage(const PoolByteArray &p_payload, const void *p_content)
		: message(flatbuffers::GetRoot<T>(p_content))
		, payload(p_payload) { }
};

template <typename T> class RequestMessage: public NkCollatedMessage {

	PoolByteArray payload;

protected:
	void _nk_message();

	static void _bind_methods() {

		ClassDB::bind_method(D_METHOD("_nk_message"), &RequestMessage::_nk_message);
	}

public:
	PoolByteArray as_bytes(String p_collation_id) const;

	RequestMessage();
};

class LogoutMessage: public NkUncollatedMessage {

	PoolByteArray payload;

public:
	PoolByteArray as_bytes() const { return payload; }

	LogoutMessage();
};

#endif // NAKAMA1_API_H
