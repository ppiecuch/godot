/* GRPacket.cpp */
#include "GRPacket.h"
#include "GRInputData.h"
#include "core/os/os.h"

using namespace GRUtils;

Ref<GRPacket> GRPacket::create(const PoolByteArray &bytes) {
	if (bytes.size() == 0) {
		ERR_FAIL_V_MSG(Ref<GRPacket>(), "Can't create GRPacket from empty data!");
	}

	PacketType type = (PacketType)((PoolByteArray)bytes)[0];
	Ref<StreamPeerBuffer> buf(memnew(StreamPeerBuffer));
	buf->set_data_array(bytes);

#define CREATE(type)                    \
	{                                   \
		Ref<type> packet(memnew(type)); \
		if (packet->_create(buf)) {     \
			return packet;              \
		} else {                        \
			return Ref<GRPacket>();     \
		}                               \
	}

	switch (type) {
		case PacketType::NonePacket:
			ERR_FAIL_V_MSG(Ref<GRPacket>(), "Can't create abstract GRPacket!");
		case PacketType::SyncTime:
			CREATE(GRPacketSyncTime);
		case PacketType::ImageData:
			CREATE(GRPacketImageData);
		case PacketType::InputData:
			CREATE(GRPacketInputData);
		case PacketType::ServerSettings:
			CREATE(GRPacketServerSettings);
		case PacketType::MouseModeSync:
			CREATE(GRPacketMouseModeSync);
		case PacketType::CustomInputScene:
			CREATE(GRPacketCustomInputScene);
		case PacketType::ClientStreamOrientation:
			CREATE(GRPacketClientStreamOrientation);
		case PacketType::ClientStreamAspect:
			CREATE(GRPacketClientStreamAspect);
		case PacketType::CustomUserData:
			CREATE(GRPacketCustomUserData);

			// Requests
		case PacketType::Ping:
			CREATE(GRPacketPing);

			// Responses
		case PacketType::Pong:
			CREATE(GRPacketPong);
		default:
			ERR_FAIL_V_MSG(Ref<GRPacket>(), "Can't create unknown GRPacket! Type: " + str((int)type));
	}
#undef CREATE
	return Ref<GRPacket>();
}

//////////////////////////////////////////////////////////////////////////
// SYNC TIME

Ref<StreamPeerBuffer> GRPacketSyncTime::_get_data() {
	auto buf = GRPacket::_get_data();
	buf->put_var(OS::get_singleton()->get_ticks_usec());
	return buf;
}

bool GRPacketSyncTime::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	time = buf->get_var();
	return true;
}

uint64_t GRPacketSyncTime::get_time() {
	return time;
}

//////////////////////////////////////////////////////////////////////////
// IMAGE DATA
Ref<StreamPeerBuffer> GRPacketImageData::_get_data() {
	auto buf = GRPacket::_get_data();
	buf->put_8(is_empty);
	buf->put_32((int)compression);
	buf->put_var(size);
	buf->put_var(format);
	buf->put_var(img_data);
	buf->put_var(start_time);
	buf->put_var(frametime);
	return buf;
}

bool GRPacketImageData::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	is_empty = (bool)buf->get_8();
	compression = buf->get_32();
	size = buf->get_var();
	format = buf->get_var();
	img_data = buf->get_var();
	start_time = buf->get_var();
	frametime = buf->get_var();
	return true;
}

PoolByteArray GRPacketImageData::get_image_data() {
	return img_data;
}

int GRPacketImageData::get_compression_type() {
	return (int)compression;
}

uint64_t GRPacketImageData::get_start_time() {
	return start_time;
}

uint64_t GRPacketImageData::get_frametime() {
	return frametime;
}

bool GRPacketImageData::get_is_empty() {
	return is_empty;
}

Size2 GRPacketImageData::get_size() {
	return size;
}

int GRPacketImageData::get_format() {
	return format;
}

void GRPacketImageData::set_compression_type(int type) {
	compression = type;
}

void GRPacketImageData::set_start_time(uint64_t time) {
	start_time = time;
}

void GRPacketImageData::set_image_data(PoolByteArray &buf) {
	img_data = buf;
}

void GRPacketImageData::set_frametime(uint64_t _frametime) {
	frametime = _frametime;
}

void GRPacketImageData::set_is_empty(bool _empty) {
	is_empty = _empty;
}

void GRPacketImageData::set_size(Size2 _size) {
	size = _size;
}

void GRPacketImageData::set_format(int _format) {
	format = _format;
}

//////////////////////////////////////////////////////////////////////////
// INPUT DATA
Ref<StreamPeerBuffer> GRPacketInputData::_get_data() {
	auto buf = GRPacket::_get_data();
	int count = 0;

	for (int i = 0; i < (int)inputs.size(); i++) {
		Ref<GRInputData> inp = inputs[i];
		if (inp.is_valid()) {
			count++;
		} else {
			inputs.erase(inputs.begin() + i);
			i--;
		}
	}
	buf->put_32(count);

	for (unsigned i = 0; i < inputs.size(); i++) {
		buf->put_var(((Ref<GRInputData>)inputs[i])->get_data());
	}
	return buf;
}

bool GRPacketInputData::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	int size = buf->get_32(); // get size
	for (int i = 0; i < size; i++) {
		Ref<GRInputData> id = GRInputData::create(buf->get_var());
		if (id.is_null())
			return false;
		inputs.push_back(id);
	}
	return true;
}

int GRPacketInputData::get_inputs_count() {
	return inputs.size();
}

Ref<GRInputData> GRPacketInputData::get_input_data(int idx) {
	ERR_FAIL_INDEX_V(idx, inputs.size(), Ref<GRInputData>());
	return inputs[idx];
}

void GRPacketInputData::remove_input_data(int idx) {
	ERR_FAIL_INDEX(idx, (int)inputs.size());

	inputs.erase(inputs.begin() + idx);
}

void GRPacketInputData::add_input_data(Ref<GRInputData> &input) {
	inputs.push_back(input);
}

void GRPacketInputData::set_input_data(std::vector<Ref<GRInputData>> &_inputs) {
	inputs = _inputs;
}

//////////////////////////////////////////////////////////////////////////
// SERVER SETTINGS
Ref<StreamPeerBuffer> GRPacketServerSettings::_get_data() {
	auto buf = GRPacket::_get_data();
	buf->put_var(map_to_dict(settings));
	return buf;
}

bool GRPacketServerSettings::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	settings = dict_to_map<int, Variant>(buf->get_var());
	return true;
}

std::map<int, Variant> GRPacketServerSettings::get_settings() {
	return settings;
}

void GRPacketServerSettings::set_settings(std::map<int, Variant> &_settings) {
	settings = _settings;
}

void GRPacketServerSettings::add_setting(int _setting, Variant value) {
	settings[_setting] = value;
}

//////////////////////////////////////////////////////////////////////////
// MOUSE MODE SYNC

Ref<StreamPeerBuffer> GRPacketMouseModeSync::_get_data() {
	auto buf = GRPacket::_get_data();
	buf->put_8(mouse_mode);
	return buf;
}

bool GRPacketMouseModeSync::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	mouse_mode = (Input::MouseMode)buf->get_8();
	return true;
}
Input::MouseMode GRPacketMouseModeSync::get_mouse_mode() {
	return mouse_mode;
}

void GRPacketMouseModeSync::set_mouse_mode(Input::MouseMode _mode) {
	mouse_mode = _mode;
}

//////////////////////////////////////////////////////////////////////////
// CUSTOM INPUT SCENE

Ref<StreamPeerBuffer> GRPacketCustomInputScene::_get_data() {
	auto buf = GRPacket::_get_data();
	buf->put_string(scene_path);
	buf->put_8(compressed);
	buf->put_8(compression_type);
	buf->put_32(original_data_size);
	buf->put_var(scene_data);
	return buf;
}

bool GRPacketCustomInputScene::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	scene_path = buf->get_string();
	compressed = buf->get_8();
	compression_type = buf->get_8();
	original_data_size = buf->get_32();
	scene_data = buf->get_var();
	return true;
}

String GRPacketCustomInputScene::get_scene_path() {
	return scene_path;
}

void GRPacketCustomInputScene::set_scene_path(String _path) {
	scene_path = _path;
}

PoolByteArray GRPacketCustomInputScene::get_scene_data() {
	return scene_data;
}

void GRPacketCustomInputScene::set_scene_data(PoolByteArray _data) {
	scene_data = _data;
}

bool GRPacketCustomInputScene::is_compressed() {
	return compressed;
}

void GRPacketCustomInputScene::set_compressed(bool val) {
	compressed = val;
}

int GRPacketCustomInputScene::get_original_size() {
	return original_data_size;
}

void GRPacketCustomInputScene::set_original_size(int val) {
	original_data_size = val;
}

int GRPacketCustomInputScene::get_compression_type() {
	return compression_type;
}

void GRPacketCustomInputScene::set_compression_type(int val) {
	compression_type = val;
}

//////////////////////////////////////////////////////////////////////////
// CLIENT DEVICE ROTATION

Ref<StreamPeerBuffer> GRPacketClientStreamOrientation::_get_data() {
	auto buf = GRPacket::_get_data();
	buf->put_8(vertical);
	return buf;
}

bool GRPacketClientStreamOrientation::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	vertical = buf->get_8();
	return true;
}

bool GRPacketClientStreamOrientation::is_vertical() {
	return vertical;
}

void GRPacketClientStreamOrientation::set_vertical(bool val) {
	vertical = val;
}

//////////////////////////////////////////////////////////////////////////
// CLIENT SCREEN ASCPECT

Ref<StreamPeerBuffer> GRPacketClientStreamAspect::_get_data() {
	auto buf = GRPacket::_get_data();
	buf->put_float(stream_aspect);
	return buf;
}

bool GRPacketClientStreamAspect::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	stream_aspect = buf->get_float();
	return true;
}

float GRPacketClientStreamAspect::get_aspect() {
	return stream_aspect;
}

void GRPacketClientStreamAspect::set_aspect(float val) {
	stream_aspect = val;
}

//////////////////////////////////////////////////////////////////////////
// CUSTOM USER DATA

Ref<StreamPeerBuffer> GRPacketCustomUserData::_get_data() {
	auto buf = GRPacket::_get_data();
	buf->put_string(packet_id);
	buf->put_8(full_objects);
	buf->put_var(user_data, full_objects);
	return buf;
}

bool GRPacketCustomUserData::_create(Ref<StreamPeerBuffer> buf) {
	GRPacket::_create(buf);
	packet_id = buf->get_string();
	full_objects = buf->get_8();
	user_data = buf->get_var(full_objects);
	return true;
}

Variant GRPacketCustomUserData::get_packet_id() {
	return packet_id;
}

void GRPacketCustomUserData::set_packet_id(Variant val) {
	packet_id = val;
}

bool GRPacketCustomUserData::get_send_full_objects() {
	return full_objects;
}

void GRPacketCustomUserData::set_send_full_objects(bool val) {
	full_objects = val;
}

Variant GRPacketCustomUserData::get_user_data() {
	return user_data;
}

void GRPacketCustomUserData::set_user_data(Variant val) {
	user_data = val;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

TEST_SUITE("[[gd_godot_remote]] GRPacket serialization") {
	TEST_CASE("[gr] Ping packet roundtrip") {
		Ref<GRPacketPing> ping(memnew(GRPacketPing));
		CHECK(ping->get_type() == GRPacket::Ping);

		PoolByteArray data = ping->get_data();
		CHECK(data.size() > 0);
		CHECK(data[0] == (uint8_t)GRPacket::Ping);

		Ref<GRPacket> restored = GRPacket::create(data);
		REQUIRE(restored.is_valid());
		CHECK(restored->get_type() == GRPacket::Ping);
	}

	TEST_CASE("[gr] Pong packet roundtrip") {
		Ref<GRPacketPong> pong(memnew(GRPacketPong));
		CHECK(pong->get_type() == GRPacket::Pong);

		PoolByteArray data = pong->get_data();
		Ref<GRPacket> restored = GRPacket::create(data);
		REQUIRE(restored.is_valid());
		CHECK(restored->get_type() == GRPacket::Pong);
	}

	TEST_CASE("[gr] SyncTime packet roundtrip") {
		Ref<GRPacketSyncTime> pkt(memnew(GRPacketSyncTime));
		CHECK(pkt->get_type() == GRPacket::SyncTime);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacket> restored = GRPacket::create(data);
		REQUIRE(restored.is_valid());

		Ref<GRPacketSyncTime> st = restored;
		REQUIRE(st.is_valid());
		CHECK(st->get_time() > 0); // Should have a timestamp
	}

	TEST_CASE("[gr] ImageData packet roundtrip") {
		Ref<GRPacketImageData> pkt(memnew(GRPacketImageData));
		pkt->set_compression_type(__COMPRESSION_JPG);
		pkt->set_size(Size2(320, 240));
		pkt->set_format(Image::FORMAT_RGB8);
		pkt->set_start_time(12345);
		pkt->set_frametime(16666);
		pkt->set_is_empty(false);

		PoolByteArray img;
		img.resize(4);
		{
			PoolByteArray::Write w = img.write();
			w[0] = 0xAA;
			w[1] = 0xBB;
			w[2] = 0xCC;
			w[3] = 0xDD;
		}
		pkt->set_image_data(img);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacket> restored = GRPacket::create(data);
		REQUIRE(restored.is_valid());

		Ref<GRPacketImageData> id = restored;
		REQUIRE(id.is_valid());
		CHECK(id->get_compression_type() == __COMPRESSION_JPG);
		CHECK(id->get_size() == Size2(320, 240));
		CHECK(id->get_format() == Image::FORMAT_RGB8);
		CHECK(id->get_start_time() == 12345);
		CHECK(id->get_frametime() == 16666);
		CHECK_FALSE(id->get_is_empty());
		CHECK(id->get_image_data().size() == 4);
	}

	TEST_CASE("[gr] ImageData empty packet") {
		Ref<GRPacketImageData> pkt(memnew(GRPacketImageData));
		pkt->set_is_empty(true);
		pkt->set_size(Size2(0, 0));

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketImageData> id = GRPacket::create(data);
		REQUIRE(id.is_valid());
		CHECK(id->get_is_empty());
	}

	TEST_CASE("[gr] MouseModeSync roundtrip") {
		Ref<GRPacketMouseModeSync> pkt(memnew(GRPacketMouseModeSync));
		pkt->set_mouse_mode(Input::MOUSE_MODE_HIDDEN);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketMouseModeSync> ms = GRPacket::create(data);
		REQUIRE(ms.is_valid());
		CHECK(ms->get_mouse_mode() == Input::MOUSE_MODE_HIDDEN);
	}

	TEST_CASE("[gr] ClientStreamOrientation roundtrip") {
		Ref<GRPacketClientStreamOrientation> pkt(memnew(GRPacketClientStreamOrientation));
		pkt->set_vertical(true);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketClientStreamOrientation> co = GRPacket::create(data);
		REQUIRE(co.is_valid());
		CHECK(co->is_vertical());
	}

	TEST_CASE("[gr] ClientStreamAspect roundtrip") {
		Ref<GRPacketClientStreamAspect> pkt(memnew(GRPacketClientStreamAspect));
		pkt->set_aspect(1.778f);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketClientStreamAspect> ca = GRPacket::create(data);
		REQUIRE(ca.is_valid());
		CHECK(ca->get_aspect() == doctest::Approx(1.778f));
	}

	TEST_CASE("[gr] CustomUserData roundtrip") {
		Ref<GRPacketCustomUserData> pkt(memnew(GRPacketCustomUserData));
		pkt->set_packet_id("test_msg");
		pkt->set_user_data(42);
		pkt->set_send_full_objects(false);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketCustomUserData> ud = GRPacket::create(data);
		REQUIRE(ud.is_valid());
		CHECK(String(ud->get_packet_id()) == "test_msg");
		CHECK((int)ud->get_user_data() == 42);
		CHECK_FALSE(ud->get_send_full_objects());
	}

	TEST_CASE("[gr] CustomUserData with string value") {
		Ref<GRPacketCustomUserData> pkt(memnew(GRPacketCustomUserData));
		pkt->set_packet_id("chat");
		pkt->set_user_data("Hello GodotRemote");

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketCustomUserData> ud = GRPacket::create(data);
		REQUIRE(ud.is_valid());
		CHECK(String(ud->get_user_data()) == "Hello GodotRemote");
	}

	TEST_CASE("[gr] ServerSettings roundtrip") {
		Ref<GRPacketServerSettings> pkt(memnew(GRPacketServerSettings));
		pkt->add_setting(1, 75); // quality
		pkt->add_setting(2, 0.5f); // scale

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketServerSettings> ss = GRPacket::create(data);
		REQUIRE(ss.is_valid());

		auto settings = ss->get_settings();
		CHECK(settings.size() == 2);
		CHECK((int)settings[1] == 75);
		CHECK((float)settings[2] == doctest::Approx(0.5f));
	}

	TEST_CASE("[gr] CustomInputScene roundtrip") {
		Ref<GRPacketCustomInputScene> pkt(memnew(GRPacketCustomInputScene));
		pkt->set_scene_path("res://ui/joystick.tscn");
		pkt->set_compressed(false);
		pkt->set_compression_type(0);
		pkt->set_original_size(1024);

		PoolByteArray scene;
		scene.resize(8);
		pkt->set_scene_data(scene);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketCustomInputScene> cs = GRPacket::create(data);
		REQUIRE(cs.is_valid());
		CHECK(cs->get_scene_path() == "res://ui/joystick.tscn");
		CHECK_FALSE(cs->is_compressed());
		CHECK(cs->get_original_size() == 1024);
		CHECK(cs->get_scene_data().size() == 8);
	}

	TEST_CASE("[gr] create from empty data fails") {
		PoolByteArray empty;
		EXPECT_ERROR({
			Ref<GRPacket> pkt = GRPacket::create(empty);
			CHECK(pkt.is_null());
		});
	}

	TEST_CASE("[gr] create from unknown type fails") {
		PoolByteArray bad;
		bad.resize(1);
		{
			PoolByteArray::Write w = bad.write();
			w[0] = 255; // invalid type
		}
		EXPECT_ERROR({
			Ref<GRPacket> pkt = GRPacket::create(bad);
			CHECK(pkt.is_null());
		});
	}

	TEST_CASE("[gr] all packet types have correct type ID") {
		CHECK(Ref<GRPacketPing>(memnew(GRPacketPing))->get_type() == GRPacket::Ping);
		CHECK(Ref<GRPacketPong>(memnew(GRPacketPong))->get_type() == GRPacket::Pong);
		CHECK(Ref<GRPacketSyncTime>(memnew(GRPacketSyncTime))->get_type() == GRPacket::SyncTime);
		CHECK(Ref<GRPacketImageData>(memnew(GRPacketImageData))->get_type() == GRPacket::ImageData);
		CHECK(Ref<GRPacketInputData>(memnew(GRPacketInputData))->get_type() == GRPacket::InputData);
		CHECK(Ref<GRPacketServerSettings>(memnew(GRPacketServerSettings))->get_type() == GRPacket::ServerSettings);
		CHECK(Ref<GRPacketMouseModeSync>(memnew(GRPacketMouseModeSync))->get_type() == GRPacket::MouseModeSync);
		CHECK(Ref<GRPacketCustomInputScene>(memnew(GRPacketCustomInputScene))->get_type() == GRPacket::CustomInputScene);
		CHECK(Ref<GRPacketClientStreamOrientation>(memnew(GRPacketClientStreamOrientation))->get_type() == GRPacket::ClientStreamOrientation);
		CHECK(Ref<GRPacketClientStreamAspect>(memnew(GRPacketClientStreamAspect))->get_type() == GRPacket::ClientStreamAspect);
		CHECK(Ref<GRPacketCustomUserData>(memnew(GRPacketCustomUserData))->get_type() == GRPacket::CustomUserData);
	}
}

TEST_SUITE("[[gd_godot_remote]] GRPacketInputData") {
	TEST_CASE("[gr] empty input packet roundtrip") {
		Ref<GRPacketInputData> pkt(memnew(GRPacketInputData));
		CHECK(pkt->get_inputs_count() == 0);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketInputData> id = GRPacket::create(data);
		REQUIRE(id.is_valid());
		CHECK(id->get_inputs_count() == 0);
	}

	TEST_CASE("[gr] sensor input data roundtrip") {
		Ref<GRInputDeviceSensorsData> sensor(memnew(GRInputDeviceSensorsData));
		PoolVector3Array sensors;
		sensors.push_back(Vector3(0, -9.8f, 0)); // accelerometer
		sensors.push_back(Vector3(0.1f, 0, 0)); // gyroscope
		sensors.push_back(Vector3(25, 0, -40)); // magnetometer
		sensors.push_back(Vector3(0, -9.8f, 0)); // gravity
		sensor->set_sensors(sensors);

		Ref<GRPacketInputData> pkt(memnew(GRPacketInputData));
		Ref<GRInputData> input = sensor;
		pkt->add_input_data(input);
		CHECK(pkt->get_inputs_count() == 1);

		PoolByteArray data = pkt->get_data();
		Ref<GRPacketInputData> id = GRPacket::create(data);
		REQUIRE(id.is_valid());
		CHECK(id->get_inputs_count() == 1);

		Ref<GRInputDeviceSensorsData> restored = id->get_input_data(0);
		REQUIRE(restored.is_valid());
		PoolVector3Array restored_sensors = restored->get_sensors();
		CHECK(restored_sensors.size() == 4);
		CHECK(restored_sensors[0].is_equal_approx(Vector3(0, -9.8f, 0)));
	}
}

TEST_SUITE("[[gd_godot_remote]] GRUtils") {
	TEST_CASE("[gr] version is valid") {
		// GRUtils is initialized by GodotRemote singleton at startup
		REQUIRE(GRUtils::_grutils_data != nullptr);
		CHECK(GRUtils::_grutils_data->internal_VERSION.size() == 3);
	}

	TEST_CASE("[gr] packet header is valid") {
		REQUIRE(GRUtils::_grutils_data != nullptr);
		CHECK(GRUtils::_grutils_data->internal_PACKET_HEADER.size() == 4);
		{
			PoolByteArray::Read r = GRUtils::_grutils_data->internal_PACKET_HEADER.read();
			CHECK(r[0] == 'G');
			CHECK(r[1] == 'R');
			CHECK(r[2] == 'H');
			CHECK(r[3] == 'D');
		}
	}

	TEST_CASE("[gr] validate_packet with valid header") {
		uint8_t valid[] = { 'G', 'R', 'H', 'D' };
		CHECK(GRUtils::validate_packet(valid));
	}

	TEST_CASE("[gr] validate_packet with invalid header") {
		uint8_t invalid[] = { 'B', 'A', 'D', '!' };
		CHECK_FALSE(GRUtils::validate_packet(invalid));
	}

	TEST_CASE("[gr] str converts variants") {
		CHECK(GRUtils::str(42) == "42");
		CHECK(GRUtils::str("hello") == "hello");
		CHECK(GRUtils::str(3.14f).begins_with("3.14"));
	}

	TEST_CASE("[gr] compare_pool_byte_arrays identical") {
		PoolByteArray a, b;
		a.resize(4);
		b.resize(4);
		{
			PoolByteArray::Write wa = a.write();
			PoolByteArray::Write wb = b.write();
			for (int i = 0; i < 4; i++) {
				wa[i] = i;
				wb[i] = i;
			}
		}
		CHECK(GRUtils::compare_pool_byte_arrays(a, b));
	}

	TEST_CASE("[gr] compare_pool_byte_arrays different") {
		PoolByteArray a, b;
		a.resize(4);
		b.resize(4);
		{
			PoolByteArray::Write wa = a.write();
			PoolByteArray::Write wb = b.write();
			wa[0] = 1;
			wb[0] = 2;
		}
		CHECK_FALSE(GRUtils::compare_pool_byte_arrays(a, b));
	}

	TEST_CASE("[gr] compare_pool_byte_arrays different sizes") {
		PoolByteArray a, b;
		a.resize(4);
		b.resize(8);
		CHECK_FALSE(GRUtils::compare_pool_byte_arrays(a, b));
	}
}

#endif // DOCTEST
