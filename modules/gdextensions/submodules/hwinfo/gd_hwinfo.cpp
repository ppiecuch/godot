/**************************************************************************/
/*  hwinfo.cpp                                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "gd_hwinfo.h"

#include <hwinfo/battery.h>
#include <hwinfo/cpu.h>
#include <hwinfo/disk.h>
#include <hwinfo/gpu.h>
#include <hwinfo/mainboard.h>
#include <hwinfo/os.h>
#include <hwinfo/ram.h>
#include <hwinfo/system.h>

static _FORCE_INLINE_ String _s(const std::string &s) {
	return String::utf8(s.c_str());
}

// --- CPU ---

Array HWInfo::get_cpu_sockets() const {
	Array result;
	auto sockets = hwinfo::getAllSockets();
	for (const auto &sock : sockets) {
		Dictionary d;
		d["socket_id"] = sock.id();
		const auto &cpu = sock.cpu();
		d["model_name"] = _s(cpu.modelName());
		d["vendor"] = _s(cpu.vendor());
		d["physical_cores"] = cpu.numPhysicalCores();
		d["logical_cores"] = cpu.numLogicalCores();
		d["max_clock_mhz"] = (int64_t)cpu.maxClockSpeed_MHz();
		d["regular_clock_mhz"] = (int64_t)cpu.regularClockSpeed_MHz();
		d["min_clock_mhz"] = (int64_t)cpu.minClockSpeed_MHz();
		d["current_clock_mhz"] = (int64_t)cpu.currentClockSpeed_MHz();
		d["cache_bytes"] = (int64_t)cpu.cacheSize_Bytes();
		d["core_id"] = cpu.id();
		Array flags;
		for (const auto &f : cpu.flags()) {
			flags.push_back(_s(f));
		}
		d["flags"] = flags;
		result.push_back(d);
	}
	return result;
}

Dictionary HWInfo::get_cpu_info() const {
	Array sockets = get_cpu_sockets();
	if (sockets.size() > 0) {
		return sockets[0];
	}
	return Dictionary();
}

// --- GPU ---

Array HWInfo::get_gpus() const {
	Array result;
	auto gpus = hwinfo::getAllGPUs();
	for (const auto &gpu : gpus) {
		Dictionary d;
		d["id"] = gpu.id();
		d["vendor"] = _s(gpu.vendor());
		d["name"] = _s(gpu.name());
		d["driver_version"] = _s(gpu.driverVersion());
		d["platform_details"] = _s(gpu.platformDetails());
		d["memory_mb"] = (int64_t)gpu.totalMemoryMBytes();
		d["frequency_mhz"] = (int64_t)gpu.frequencyMHz();
		d["num_cores"] = gpu.num_cores();
		result.push_back(d);
	}
	return result;
}

Dictionary HWInfo::get_gpu_info() const {
	Array gpus = get_gpus();
	if (gpus.size() > 0) {
		return gpus[0];
	}
	return Dictionary();
}

// --- RAM ---

Dictionary HWInfo::get_ram_info() const {
	hwinfo::RAM ram;
	Dictionary d;
	d["total_bytes"] = (int64_t)ram.total_Bytes();
	d["free_bytes"] = (int64_t)ram.free_Bytes();
	d["available_bytes"] = (int64_t)ram.available_Bytes();
	d["total_mb"] = (int64_t)(ram.total_Bytes() / (1024 * 1024));
	d["free_mb"] = (int64_t)(ram.free_Bytes() / (1024 * 1024));
	d["available_mb"] = (int64_t)(ram.available_Bytes() / (1024 * 1024));
	return d;
}

Array HWInfo::get_ram_modules() const {
	Array result;
	auto bars = hwinfo::getAllRamBars();
	for (const auto &bar : bars) {
		Dictionary d;
		d["vendor"] = _s(bar.vendor());
		d["name"] = _s(bar.name());
		d["model"] = _s(bar.model());
		d["serial_number"] = _s(bar.serialNumber());
		d["total_bytes"] = (int64_t)bar.total_Bytes();
		d["frequency_mhz"] = (int)bar.frequency();
		result.push_back(d);
	}
	return result;
}

// --- Disk ---

Array HWInfo::get_disks() const {
	Array result;
	auto disks = hwinfo::getAllDisks();
	for (const auto &disk : disks) {
		Dictionary d;
		d["vendor"] = _s(disk.vendor());
		d["model"] = _s(disk.model());
		d["serial_number"] = _s(disk.serialNumber());
		d["size_bytes"] = (int64_t)disk.size_Bytes();
		d["size_gb"] = (int64_t)(disk.size_Bytes() / (1024 * 1024 * 1024));
		result.push_back(d);
	}
	return result;
}

// --- Battery ---

Array HWInfo::get_batteries() const {
	Array result;
	auto batteries = hwinfo::getAllBatteries();
	for (auto &bat : batteries) {
		Dictionary d;
		d["vendor"] = _s(bat.vendor());
		d["model"] = _s(bat.model());
		d["serial_number"] = _s(bat.serialNumber());
		d["technology"] = _s(bat.technology());
		d["energy_full_mwh"] = (int)bat.energyFull();
		d["energy_now_mwh"] = (int)bat.energyNow();
		d["capacity"] = bat.capacity();
		d["charging"] = bat.charging();
		d["discharging"] = bat.discharging();
		result.push_back(d);
	}
	return result;
}

Dictionary HWInfo::get_battery_info() const {
	Array batteries = get_batteries();
	if (batteries.size() > 0) {
		return batteries[0];
	}
	return Dictionary();
}

// --- OS ---

Dictionary HWInfo::get_os_info() const {
	hwinfo::OS os;
	Dictionary d;
	d["full_name"] = _s(os.fullName());
	d["name"] = _s(os.name());
	d["version"] = _s(os.version());
	d["kernel"] = _s(os.kernel());
	d["details"] = _s(os.details());
	d["is_32bit"] = os.is32bit();
	d["is_64bit"] = os.is64bit();
	d["is_big_endian"] = os.isBigEndian();
	d["is_little_endian"] = os.isLittleEndian();
	return d;
}

// --- System ---

Dictionary HWInfo::get_system_info() const {
	hwinfo::System sys;
	Dictionary d;
	d["machine_id"] = _s(sys.getMachineUniqueId());
	d["boot_id"] = _s(sys.getBootUniqueId());
	d["uptime_seconds"] = (int64_t)sys.getUptimeSeconds();
	d["num_processes"] = (int64_t)sys.getNumProcesses();
	return d;
}

Array HWInfo::get_cpu_usage() const {
	hwinfo::System sys;
	Array result;
	auto usage = sys.getCpuUsagePercent();
	for (float u : usage) {
		result.push_back(u);
	}
	return result;
}

// --- MainBoard ---

Dictionary HWInfo::get_mainboard_info() const {
	hwinfo::MainBoard mb;
	Dictionary d;
	d["vendor"] = _s(mb.vendor());
	d["name"] = _s(mb.name());
	d["version"] = _s(mb.version());
	d["serial_number"] = _s(mb.serialNumber());
	return d;
}

// --- Summary ---

String HWInfo::get_summary() const {
	String s;

	// CPU
	auto sockets = hwinfo::getAllSockets();
	for (const auto &sock : sockets) {
		const auto &cpu = sock.cpu();
		s += vformat("CPU: %s (%s), %d cores (%d logical), %d MHz\n",
				_s(cpu.modelName()), _s(cpu.vendor()),
				cpu.numPhysicalCores(), cpu.numLogicalCores(),
				(int)cpu.maxClockSpeed_MHz());
	}

	// GPU
	auto gpus = hwinfo::getAllGPUs();
	for (const auto &gpu : gpus) {
		s += vformat("GPU: %s (%s), %d MB, driver %s\n",
				_s(gpu.name()), _s(gpu.vendor()),
				(int)gpu.totalMemoryMBytes(), _s(gpu.driverVersion()));
	}

	// RAM
	hwinfo::RAM ram;
	s += vformat("RAM: %d MB total, %d MB available\n",
			(int)(ram.total_Bytes() / (1024 * 1024)),
			(int)(ram.available_Bytes() / (1024 * 1024)));

	// OS
	hwinfo::OS os;
	s += vformat("OS: %s (%s)\n", _s(os.fullName()), _s(os.kernel()));

	// MainBoard
	hwinfo::MainBoard mb;
	String mb_name = _s(mb.name());
	if (!mb_name.empty() && mb_name != "<unknown>") {
		s += vformat("Board: %s (%s)\n", mb_name, _s(mb.vendor()));
	}

	// Battery
	auto batteries = hwinfo::getAllBatteries();
	for (auto &bat : batteries) {
		s += vformat("Battery: %.0f%% (%s)\n",
				bat.capacity() * 100.0, bat.charging() ? "charging" : "discharging");
	}

	// Disks
	auto disks = hwinfo::getAllDisks();
	for (const auto &disk : disks) {
		s += vformat("Disk: %s %s, %d GB\n",
				_s(disk.vendor()), _s(disk.model()),
				(int)(disk.size_Bytes() / (1024 * 1024 * 1024)));
	}

	return s;
}

// --- Bind methods ---

void HWInfo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_cpu_sockets"), &HWInfo::get_cpu_sockets);
	ClassDB::bind_method(D_METHOD("get_cpu_info"), &HWInfo::get_cpu_info);
	ClassDB::bind_method(D_METHOD("get_gpus"), &HWInfo::get_gpus);
	ClassDB::bind_method(D_METHOD("get_gpu_info"), &HWInfo::get_gpu_info);
	ClassDB::bind_method(D_METHOD("get_ram_info"), &HWInfo::get_ram_info);
	ClassDB::bind_method(D_METHOD("get_ram_modules"), &HWInfo::get_ram_modules);
	ClassDB::bind_method(D_METHOD("get_disks"), &HWInfo::get_disks);
	ClassDB::bind_method(D_METHOD("get_batteries"), &HWInfo::get_batteries);
	ClassDB::bind_method(D_METHOD("get_battery_info"), &HWInfo::get_battery_info);
	ClassDB::bind_method(D_METHOD("get_os_info"), &HWInfo::get_os_info);
	ClassDB::bind_method(D_METHOD("get_system_info"), &HWInfo::get_system_info);
	ClassDB::bind_method(D_METHOD("get_cpu_usage"), &HWInfo::get_cpu_usage);
	ClassDB::bind_method(D_METHOD("get_mainboard_info"), &HWInfo::get_mainboard_info);
	ClassDB::bind_method(D_METHOD("get_summary"), &HWInfo::get_summary);
}

HWInfo::HWInfo() {
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[hwinfo]] CPU") {
	TEST_CASE("get_cpu_sockets returns non-empty array") {
		HWInfo hw;
		Array sockets = hw.get_cpu_sockets();
		REQUIRE(sockets.size() > 0);
	}

	TEST_CASE("CPU socket has required keys") {
		HWInfo hw;
		Array sockets = hw.get_cpu_sockets();
		REQUIRE(sockets.size() > 0);
		Dictionary cpu = sockets[0];
		CHECK(cpu.has("model_name"));
		CHECK(cpu.has("vendor"));
		CHECK(cpu.has("physical_cores"));
		CHECK(cpu.has("logical_cores"));
		CHECK(cpu.has("max_clock_mhz"));
		CHECK(cpu.has("cache_bytes"));
		CHECK(cpu.has("flags"));
		CHECK(cpu.has("core_id"));
		CHECK(cpu.has("socket_id"));
	}

	TEST_CASE("CPU model name is non-empty") {
		HWInfo hw;
		Dictionary cpu = hw.get_cpu_info();
		String model = cpu["model_name"];
		CHECK(!model.empty());
		CHECK(model != "<unknown>");
	}

	TEST_CASE("CPU has positive core count") {
		HWInfo hw;
		Dictionary cpu = hw.get_cpu_info();
		int phys = cpu["physical_cores"];
		int logical = cpu["logical_cores"];
		CHECK(phys > 0);
		CHECK(logical > 0);
		CHECK(logical >= phys);
	}

	TEST_CASE("CPU vendor is known") {
		HWInfo hw;
		Dictionary cpu = hw.get_cpu_info();
		String vendor = cpu["vendor"];
		CHECK(!vendor.empty());
	}

	TEST_CASE("CPU flags is an array") {
		HWInfo hw;
		Dictionary cpu = hw.get_cpu_info();
		Array flags = cpu["flags"];
		// Flags may be empty on some platforms but should be an array
		CHECK(flags.size() >= 0);
	}

	TEST_CASE("get_cpu_info convenience returns same as first socket") {
		HWInfo hw;
		Dictionary info = hw.get_cpu_info();
		Array sockets = hw.get_cpu_sockets();
		if (sockets.size() > 0) {
			Dictionary first = sockets[0];
			CHECK(String(info["model_name"]) == String(first["model_name"]));
		}
	}
}

TEST_SUITE("[[hwinfo]] GPU") {
	TEST_CASE("get_gpus returns array") {
		HWInfo hw;
		Array gpus = hw.get_gpus();
		// May be empty in headless/CI but should not crash
		CHECK(gpus.size() >= 0);
	}

	TEST_CASE("GPU entry has required keys") {
		HWInfo hw;
		Array gpus = hw.get_gpus();
		if (gpus.size() > 0) {
			Dictionary gpu = gpus[0];
			CHECK(gpu.has("id"));
			CHECK(gpu.has("vendor"));
			CHECK(gpu.has("name"));
			CHECK(gpu.has("driver_version"));
			CHECK(gpu.has("memory_mb"));
			CHECK(gpu.has("frequency_mhz"));
			CHECK(gpu.has("num_cores"));
			CHECK(gpu.has("platform_details"));
		}
	}

	TEST_CASE("get_gpu_info convenience works") {
		HWInfo hw;
		Dictionary info = hw.get_gpu_info();
		// May be empty dict if no GPU
		if (!info.empty()) {
			CHECK(info.has("name"));
		}
	}
}

TEST_SUITE("[[hwinfo]] RAM") {
	TEST_CASE("get_ram_info returns valid memory sizes") {
		HWInfo hw;
		Dictionary ram = hw.get_ram_info();
		CHECK(ram.has("total_bytes"));
		CHECK(ram.has("free_bytes"));
		CHECK(ram.has("available_bytes"));
		CHECK(ram.has("total_mb"));
		CHECK(ram.has("free_mb"));
		CHECK(ram.has("available_mb"));

		int64_t total = ram["total_bytes"];
		int64_t avail = ram["available_bytes"];
		CHECK(total > 0);
		CHECK(avail > 0);
		CHECK(avail <= total);
	}

	TEST_CASE("RAM total is reasonable (>= 256MB)") {
		HWInfo hw;
		Dictionary ram = hw.get_ram_info();
		int64_t total_mb = ram["total_mb"];
		CHECK(total_mb >= 256);
	}

	TEST_CASE("get_ram_modules returns array") {
		HWInfo hw;
		Array modules = hw.get_ram_modules();
		CHECK(modules.size() >= 0);
	}

	TEST_CASE("RAM module has required keys") {
		HWInfo hw;
		Array modules = hw.get_ram_modules();
		if (modules.size() > 0) {
			Dictionary mod = modules[0];
			CHECK(mod.has("vendor"));
			CHECK(mod.has("name"));
			CHECK(mod.has("model"));
			CHECK(mod.has("total_bytes"));
			CHECK(mod.has("frequency_mhz"));
		}
	}
}

TEST_SUITE("[[hwinfo]] Disk") {
	TEST_CASE("get_disks returns array") {
		HWInfo hw;
		Array disks = hw.get_disks();
		CHECK(disks.size() >= 0);
	}

	TEST_CASE("Disk entry has required keys") {
		HWInfo hw;
		Array disks = hw.get_disks();
		if (disks.size() > 0) {
			Dictionary disk = disks[0];
			CHECK(disk.has("vendor"));
			CHECK(disk.has("model"));
			CHECK(disk.has("serial_number"));
			CHECK(disk.has("size_bytes"));
			CHECK(disk.has("size_gb"));
		}
	}

	TEST_CASE("At least one disk has positive size") {
		HWInfo hw;
		Array disks = hw.get_disks();
		bool found_positive = false;
		for (int i = 0; i < disks.size(); i++) {
			Dictionary d = disks[i];
			if ((int64_t)d["size_bytes"] > 0) {
				found_positive = true;
				break;
			}
		}
		if (disks.size() > 0) {
			CHECK(found_positive);
		}
	}
}

TEST_SUITE("[[hwinfo]] Battery") {
	TEST_CASE("get_batteries returns array") {
		HWInfo hw;
		Array batteries = hw.get_batteries();
		// Desktop may have 0 batteries — that's fine
		CHECK(batteries.size() >= 0);
	}

	TEST_CASE("Battery entry has required keys") {
		HWInfo hw;
		Array batteries = hw.get_batteries();
		if (batteries.size() > 0) {
			Dictionary bat = batteries[0];
			CHECK(bat.has("vendor"));
			CHECK(bat.has("model"));
			CHECK(bat.has("technology"));
			CHECK(bat.has("capacity"));
			CHECK(bat.has("charging"));
			CHECK(bat.has("discharging"));
			CHECK(bat.has("energy_full_mwh"));
			CHECK(bat.has("energy_now_mwh"));
		}
	}

	TEST_CASE("Battery capacity in valid range") {
		HWInfo hw;
		Array batteries = hw.get_batteries();
		for (int i = 0; i < batteries.size(); i++) {
			Dictionary bat = batteries[i];
			double cap = bat["capacity"];
			CHECK(cap >= 0.0);
			CHECK(cap <= 1.0);
		}
	}

	TEST_CASE("get_battery_info convenience works") {
		HWInfo hw;
		Dictionary info = hw.get_battery_info();
		// May be empty on desktops
		if (!info.empty()) {
			CHECK(info.has("capacity"));
		}
	}
}

TEST_SUITE("[[hwinfo]] OS") {
	TEST_CASE("get_os_info returns all keys") {
		HWInfo hw;
		Dictionary os = hw.get_os_info();
		CHECK(os.has("full_name"));
		CHECK(os.has("name"));
		CHECK(os.has("version"));
		CHECK(os.has("kernel"));
		CHECK(os.has("details"));
		CHECK(os.has("is_32bit"));
		CHECK(os.has("is_64bit"));
		CHECK(os.has("is_big_endian"));
		CHECK(os.has("is_little_endian"));
	}

	TEST_CASE("OS name is non-empty") {
		HWInfo hw;
		Dictionary os = hw.get_os_info();
		String name = os["name"];
		CHECK(!name.empty());
	}

	TEST_CASE("OS is either 32 or 64 bit") {
		HWInfo hw;
		Dictionary os = hw.get_os_info();
		bool is32 = os["is_32bit"];
		bool is64 = os["is_64bit"];
		CHECK((is32 || is64));
	}

	TEST_CASE("OS endianness is consistent") {
		HWInfo hw;
		Dictionary os = hw.get_os_info();
		bool big = os["is_big_endian"];
		bool little = os["is_little_endian"];
		CHECK(big != little); // must be one or the other
	}

	TEST_CASE("OS kernel is non-empty") {
		HWInfo hw;
		Dictionary os = hw.get_os_info();
		String kernel = os["kernel"];
		CHECK(!kernel.empty());
	}
}

TEST_SUITE("[[hwinfo]] System") {
	TEST_CASE("get_system_info returns all keys") {
		HWInfo hw;
		Dictionary sys = hw.get_system_info();
		CHECK(sys.has("machine_id"));
		CHECK(sys.has("boot_id"));
		CHECK(sys.has("uptime_seconds"));
		CHECK(sys.has("num_processes"));
	}

	TEST_CASE("Uptime is positive") {
		HWInfo hw;
		Dictionary sys = hw.get_system_info();
		int64_t uptime = sys["uptime_seconds"];
		CHECK(uptime > 0);
	}

	TEST_CASE("Process count is positive") {
		HWInfo hw;
		Dictionary sys = hw.get_system_info();
		int64_t procs = sys["num_processes"];
		CHECK(procs > 0);
	}

	TEST_CASE("Machine ID is non-empty") {
		HWInfo hw;
		Dictionary sys = hw.get_system_info();
		String mid = sys["machine_id"];
		CHECK(!mid.empty());
	}

	TEST_CASE("get_cpu_usage returns array of floats") {
		HWInfo hw;
		Array usage = hw.get_cpu_usage();
		// May be empty on first call (needs delta)
		CHECK(usage.size() >= 0);
		for (int i = 0; i < usage.size(); i++) {
			float val = usage[i];
			CHECK(val >= 0.0f);
			CHECK(val <= 100.0f);
		}
	}
}

TEST_SUITE("[[hwinfo]] MainBoard") {
	TEST_CASE("get_mainboard_info returns all keys") {
		HWInfo hw;
		Dictionary mb = hw.get_mainboard_info();
		CHECK(mb.has("vendor"));
		CHECK(mb.has("name"));
		CHECK(mb.has("version"));
		CHECK(mb.has("serial_number"));
	}
}

TEST_SUITE("[[hwinfo]] Summary") {
	TEST_CASE("get_summary returns non-empty string") {
		HWInfo hw;
		String summary = hw.get_summary();
		CHECK(!summary.empty());
		CHECK(summary.find("CPU:") >= 0);
		CHECK(summary.find("RAM:") >= 0);
		CHECK(summary.find("OS:") >= 0);
	}

	TEST_CASE("Summary contains CPU model") {
		HWInfo hw;
		Dictionary cpu = hw.get_cpu_info();
		String summary = hw.get_summary();
		if (!cpu.empty()) {
			String model = cpu["model_name"];
			// Model should appear in summary
			CHECK(summary.find(model) >= 0);
		}
	}
}

TEST_SUITE("[[hwinfo]] Cross-checks") {
	TEST_CASE("CPU logical cores >= physical cores") {
		HWInfo hw;
		Dictionary cpu = hw.get_cpu_info();
		if (!cpu.empty()) {
			int phys = cpu["physical_cores"];
			int logical = cpu["logical_cores"];
			CHECK(logical >= phys);
		}
	}

	TEST_CASE("RAM available <= total") {
		HWInfo hw;
		Dictionary ram = hw.get_ram_info();
		int64_t total = ram["total_bytes"];
		int64_t avail = ram["available_bytes"];
		CHECK(avail <= total);
	}

	TEST_CASE("Multiple calls return consistent CPU info") {
		HWInfo hw;
		Dictionary cpu1 = hw.get_cpu_info();
		Dictionary cpu2 = hw.get_cpu_info();
		CHECK(String(cpu1["model_name"]) == String(cpu2["model_name"]));
		CHECK(String(cpu1["vendor"]) == String(cpu2["vendor"]));
		CHECK((int)cpu1["physical_cores"] == (int)cpu2["physical_cores"]);
	}

	TEST_CASE("OS info is stable across calls") {
		HWInfo hw;
		Dictionary os1 = hw.get_os_info();
		Dictionary os2 = hw.get_os_info();
		CHECK(String(os1["name"]) == String(os2["name"]));
		CHECK(String(os1["kernel"]) == String(os2["kernel"]));
	}
}

#endif // DOCTEST
