/**************************************************************************/
/*  keychain_apple.mm                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "keychain.h"

#if defined(OSX_ENABLED) || defined(IPHONE_ENABLED)

#import <Foundation/Foundation.h>
#import <Security/Security.h>

static Keychain::Error _status_to_error(OSStatus status, String &r_message) {
	switch (status) {
		case errSecSuccess:
			r_message = "";
			return Keychain::NO_ERROR;
		case errSecItemNotFound:
			r_message = "Item not found in keychain.";
			return Keychain::ENTRY_NOT_FOUND;
		case errSecUserCanceled:
			r_message = "User canceled the operation.";
			return Keychain::ACCESS_DENIED_BY_USER;
		case errSecInteractionNotAllowed:
			r_message = "Interaction not allowed.";
			return Keychain::ACCESS_DENIED;
		case errSecNotAvailable:
			r_message = "Keychain not available.";
			return Keychain::NO_BACKEND_AVAILABLE;
		case errSecAuthFailed:
			r_message = "Authentication failed.";
			return Keychain::ACCESS_DENIED;
		case errSecDuplicateItem:
			r_message = "Duplicate item in keychain.";
			return Keychain::OTHER_ERROR;
		default:
			r_message = vformat("Keychain error: OSStatus %d", (int)status);
			return Keychain::OTHER_ERROR;
	}
}

static NSString *to_ns(const String &p_str) {
	CharString utf8 = p_str.utf8();
	return [[NSString alloc] initWithBytes:utf8.get_data()
									length:utf8.length()
								  encoding:NSUTF8StringEncoding];
}

Keychain::Error Keychain::_platform_read(const String &p_key, PoolByteArray &r_data) {
	@autoreleasepool {
		NSString *ns_service = to_ns(_service);
		NSString *ns_key = to_ns(p_key);

		NSDictionary *query = @{
			(__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
			(__bridge id)kSecAttrService : ns_service,
			(__bridge id)kSecAttrAccount : ns_key,
			(__bridge id)kSecReturnData : @YES,
		};

		CFTypeRef data_ref = nil;
		OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)query, &data_ref);

		if (status == errSecSuccess && data_ref) {
			NSData *ns_data = (__bridge NSData *)data_ref;
			r_data.resize([ns_data length]);
			{
				PoolByteArray::Write w = r_data.write();
				memcpy(w.ptr(), [ns_data bytes], [ns_data length]);
			}
			CFRelease(data_ref);
			_last_error_string = "";
			return NO_ERROR;
		}

		if (data_ref) {
			CFRelease(data_ref);
		}

		return _status_to_error(status, _last_error_string);
	}
}

Keychain::Error Keychain::_platform_write(const String &p_key, const PoolByteArray &p_data) {
	@autoreleasepool {
		NSString *ns_service = to_ns(_service);
		NSString *ns_key = to_ns(p_key);

		PoolByteArray::Read r = p_data.read();
		NSData *ns_data = [NSData dataWithBytes:r.ptr() length:p_data.size()];

		// Check if entry exists
		NSDictionary *query = @{
			(__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
			(__bridge id)kSecAttrService : ns_service,
			(__bridge id)kSecAttrAccount : ns_key,
		};

		OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)query, nil);

		if (status == errSecSuccess) {
			// Update existing
			NSDictionary *update = @{
				(__bridge id)kSecValueData : ns_data,
			};
			status = SecItemUpdate((__bridge CFDictionaryRef)query, (__bridge CFDictionaryRef)update);
		} else {
			// Insert new
			NSDictionary *insert = @{
				(__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
				(__bridge id)kSecAttrService : ns_service,
				(__bridge id)kSecAttrAccount : ns_key,
				(__bridge id)kSecValueData : ns_data,
			};
			status = SecItemAdd((__bridge CFDictionaryRef)insert, nil);
		}

		if (status == errSecSuccess) {
			_last_error_string = "";
			return NO_ERROR;
		}
		return _status_to_error(status, _last_error_string);
	}
}

Keychain::Error Keychain::_platform_delete(const String &p_key) {
	@autoreleasepool {
		NSString *ns_service = to_ns(_service);
		NSString *ns_key = to_ns(p_key);

		NSDictionary *query = @{
			(__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
			(__bridge id)kSecAttrService : ns_service,
			(__bridge id)kSecAttrAccount : ns_key,
		};

		OSStatus status = SecItemDelete((__bridge CFDictionaryRef)query);

		if (status == errSecSuccess) {
			_last_error_string = "";
			return NO_ERROR;
		}
		return _status_to_error(status, _last_error_string);
	}
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[keychains]] Apple Security.framework backend") {
	TEST_CASE("Error mapping: errSecSuccess") {
		String msg;
		Keychain::Error err = _status_to_error(errSecSuccess, msg);
		CHECK(err == Keychain::NO_ERROR);
		CHECK(msg.empty());
	}

	TEST_CASE("Error mapping: errSecItemNotFound") {
		String msg;
		Keychain::Error err = _status_to_error(errSecItemNotFound, msg);
		CHECK(err == Keychain::ENTRY_NOT_FOUND);
		CHECK(!msg.empty());
	}

	TEST_CASE("Error mapping: errSecUserCanceled") {
		String msg;
		Keychain::Error err = _status_to_error(errSecUserCanceled, msg);
		CHECK(err == Keychain::ACCESS_DENIED_BY_USER);
	}

	TEST_CASE("Error mapping: unknown status") {
		String msg;
		Keychain::Error err = _status_to_error(-99999, msg);
		CHECK(err == Keychain::OTHER_ERROR);
		CHECK(!msg.empty());
	}

	TEST_CASE("NSString conversion") {
		NSString *ns = to_ns("Hello");
		CHECK([ns isEqualToString:@"Hello"]);
	}

	TEST_CASE("Write and read via Security.framework") {
		Keychain kc;
		kc.set_service("doctest_apple_keychain");

		// Write a test password
		Keychain::Error err = kc.write_password("apple_test_key", "apple_secret_123");
		CHECK(err == Keychain::NO_ERROR);

		// Read it back
		String result = kc.read_password("apple_test_key");
		CHECK(result == "apple_secret_123");
		CHECK(kc.get_last_error() == Keychain::NO_ERROR);

		// Delete
		err = kc.delete_entry("apple_test_key");
		CHECK(err == Keychain::NO_ERROR);

		// Verify deleted
		result = kc.read_password("apple_test_key");
		CHECK(kc.get_last_error() == Keychain::ENTRY_NOT_FOUND);
	}

	TEST_CASE("Read nonexistent key") {
		Keychain kc;
		kc.set_service("doctest_apple_keychain");
		String result = kc.read_password("nonexistent_apple_key_xyz");
		CHECK(kc.get_last_error() == Keychain::ENTRY_NOT_FOUND);
	}

	TEST_CASE("Overwrite key") {
		Keychain kc;
		kc.set_service("doctest_apple_keychain");

		kc.write_password("apple_ow_key", "first");
		CHECK(kc.read_password("apple_ow_key") == "first");

		kc.write_password("apple_ow_key", "second");
		CHECK(kc.read_password("apple_ow_key") == "second");

		kc.delete_entry("apple_ow_key");
	}
}

#endif // DOCTEST

#endif // OSX_ENABLED || IPHONE_ENABLED
