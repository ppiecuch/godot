// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <sys/sysctl.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>

#include <vector>
#include <string>
#include <cstring>

#include <IOKit/IOKitLib.h>
#include <MacTypes.h>

struct Utils {
  //Takes a string and returns the given string as a FourCharCode.
  static FourCharCode stringToFourCharCode(const std::string &givenString) {
    const char *bytes = givenString.c_str();
    UInt32 byte0 = (unsigned)bytes[0] << (unsigned)24;
    UInt32 byte1 = (unsigned)bytes[1] << (unsigned)16;
    UInt32 byte2 = (unsigned)bytes[2] << (unsigned)8;
    UInt32 byte3 = (unsigned)bytes[3];
    return byte0 | byte1 | byte2 | byte3;
  }

  // Takes a FourCharCode and returns the given code as a string.
  static std::string fourCharCodeToString(FourCharCode givenCode) {
    char byte0 = givenCode >> (unsigned)24;
    char byte1 = givenCode >> (unsigned)16;
    char byte2 = givenCode >> (unsigned)8;
    char byte3 = givenCode;
    return std::string({byte0, byte1, byte2, byte3});
  }

  // Converts a given value from the fpe2 data type to int.
  static int fpe2ToInt(UInt8 value[2]) { return (value[0] << (unsigned)6) + (value[1] >> (unsigned)2); }

  // Converts a given value from the flt data type to int.
  static int fltToInt(UInt8 value[4]) {
    float resultValue = 0;
    std::memcpy(&resultValue, value, 4);
    return (int)resultValue;
  }
};

class SmcSystemInfo {

  // These enum and structs are defined in the Apple PowerManagement project:
  // https://opensource.apple.com/source/PowerManagement/PowerManagement-211/pmconfigd/PrivateLib.c.auto.html
  enum SMCResult {
    kSMCSuccess = 0,
    kSMCError = 1,
    kSMCKeyNotFound = 132,
  };

  enum SMCSelector {
    kSMCHandleYPCEvent = 2,
    kSMCReadKey = 5,
    kSMCWriteKey = 6,
    kSMCGetKeyFromIndex = 8,
    kSMCGetKeyInfo = 9
  };

  typedef UInt8 SMCBytes[32];

  typedef struct DataType {
    FourCharCode type;
    UInt32 size;

    DataType(const std::string &givenString, UInt32 givenSize) {
      type = Utils::stringToFourCharCode(givenString);
      size = givenSize;
    }
    DataType(FourCharCode givenType, UInt32 givenSize) : type(givenType), size(givenSize) { }
  } data_type_t;

  typedef struct DataTypes {
    DataType FDS = DataType("{fds", 16);
    DataType Flag = DataType("flag", 1);
    DataType FPE2 = DataType("fpe2", 2);
    DataType FLT = DataType("flt", 4);
    DataType SP78 = DataType("sp78", 2);
    DataType UInt8 = DataType("ui8", 1);
    DataType UInt32 = DataType("ui32", 4);
  } data_types_t;

  typedef struct SMCKey {
    FourCharCode code;
    DataType info;
    SMCKey(const std::string &givenString, data_type_t &typeInfo) : info(typeInfo) { code = Utils::stringToFourCharCode(givenString); }
  } smc_key_t;

  typedef struct SMCVersion {
    unsigned char major = 0;
    unsigned char minor = 0;
    unsigned char build = 0;
    unsigned char reserved = 0;
    unsigned short release = 0;
  } smc_version_t;

  typedef struct SMCLimitData {
    UInt16 version = 0;
    UInt16 length = 0;
    UInt32 cpuPLimit = 0;
    UInt32 gpuPLimit = 0;
    UInt32 memPLimit = 0;
  } smc_limit_data_t;

  typedef struct SMCKeyInfoData {
    IOByteCount dataSize = 0;
    UInt32 dataType = 0;
    UInt8 dataAttributes = 0;
  } smc_key_info_data_t;

  struct SMCParamStruct {
    UInt32 key = 0;
    SMCVersion vers = SMCVersion();
    SMCLimitData pLimitData = SMCLimitData();
    SMCKeyInfoData keyInfo = SMCKeyInfoData();
    UInt8 result = 0;
    UInt8 status = 0;
    UInt8 data8 = 0;
    UInt32 data32 = 0;
    SMCBytes bytes = {0};
  };

  typedef struct CpuTickStruct {
    int userTicks;
    int systemTicks;
    int idleTicks;
    int niceTicks;
  } cpu_tick_t;

  io_connect_t connectionHandle = 0;
  cpu_tick_t prevCpuTicks;

public:
  DataTypes types = DataTypes();

  void open(); // Opens a connection to the SMC driver.
  void close(); // Closes the connection to the SMC driver.

  DataType getKeyInfo(std::string keyString); // Returns the data type of the given key.
  SMCParamStruct callSMC(SMCParamStruct givenStruct, SMCSelector smcSelector = kSMCHandleYPCEvent); // Makes a call to the SMC.
  void readKey(smc_key_t smcKey, SMCBytes &result); // Reads the data of a SMC-Key
  void readKey(const std::string &keyCode, DataType typeInfo, SMCBytes &resultArray); // Reads the data of the key code from the SMC.
  std::vector<float> getMemoryUsage(); // Reads the current ram usage. Returns 5 values: [free, active, inactive, wired, compressed] in GB
  float getTotalMemory(); // Returns the total amount of ram memory of this machine in gigabyte.
  int getCpuTemp(); // Reads the cpu temperature of the CPU_0_PROXIMITY sensor.
  std::vector<float> getCpuUsage(); // Returns the cpu usage of the user, system, idle and nice in the given float array in this order.

  // Returns the cpu ticks of the user, system idle and nice in a struct.
  cpu_tick_t getCpuLoadInfo() {
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    mach_port_t host = mach_host_self();
    host_cpu_load_info_data_t load_info;

    kern_return_t result = host_statistics(host, HOST_CPU_LOAD_INFO, (host_info_t)&load_info, &count);

    if (result != KERN_SUCCESS)
    {
        throw std::runtime_error("An error occured while getting the cpu usage.");
    }

    cpu_tick_t cpuTickStruct;

    cpuTickStruct.userTicks = load_info.cpu_ticks[CPU_STATE_USER];
    cpuTickStruct.systemTicks = load_info.cpu_ticks[CPU_STATE_SYSTEM];
    cpuTickStruct.idleTicks = load_info.cpu_ticks[CPU_STATE_IDLE];
    cpuTickStruct.niceTicks = load_info.cpu_ticks[CPU_STATE_NICE];

    return cpuTickStruct;
  }

  int getFanCount(); // Returns the number of fans of the machine.
  int getMinFanSpeed(int fanID); // Returns the minimum rounds per minute (rpm) of the fan with the given id.
  int getMaxFanSpeed(int fanID); // Returns the maximum rounds per minute (rpm) of the fan with the given id.
  int getCurrentFanSpeed(int fanID); // Returns the current rounds per minute (rpm) of the fan with the given id.
  int getBatteryCount(); // Returns the number of batteries of the machine.
  bool isChargingBattery(); // Returns true if the battery is currently charged. Return false otherwise.
  float getBatteryHealth(); // Returns the health of the battery in percentage.
  int getBatteryCycles(); // Returns the number of already used battery cycles.

  SmcSystemInfo() { prevCpuTicks = getCpuLoadInfo(); }
  ~SmcSystemInfo() { close(); }
};
