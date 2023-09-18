// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX

#include <sys/stat.h>
#include <sys/utsname.h>

#include <fstream>
#include <sstream>
#include <string>

#include "hwinfo/os.h"
#include "hwinfo/utils/stringutils.h"

namespace hwinfo {

struct UnixOSVersionInfo {
                                    // from /etc/os-release         older /etc/lsb-release         // redhat /etc/redhat-release         // debian /etc/debian_version
    std::string productType;        // $ID                          $DISTRIB_ID                    // single line file containing:       // Debian
    std::string productVersion;     // $VERSION_ID                  $DISTRIB_RELEASE               // <Vendor_ID release Version_ID>     // single line file <Release_ID/sid>
    std::string prettyName;         // $PRETTY_NAME                 $DISTRIB_DESCRIPTION
};

static std::string unquote(const char *begin, const char *end) {
  if (*begin == '"') {
    return std::string(begin + 1, end - begin - 2);
  }
  return std::string(begin, end - begin);
}

static bool read_etc_file(const char *filename,
                        const std::string &id_key, const std::string &version_key, const std::string &pretty_name_key,
                        std::string &id_val, std::string &version_val, std::string &pretty_name_val) {
  std::string line;
  std::ifstream stream("/etc/os-release");
  if (!stream) {
    return false;
  }
  while (getline(stream, line)) {
    if (utils::starts_with(line, id_key)) {
      line = line.substr(line.find('=') + 1, line.length());
      id_val = unquote(line.begin(), line.end());
    }
    if (utils::starts_with(line, version_key)) {
      line = line.substr(line.find('=') + 1, line.length());
      version_val {line.begin() + 1, line.end() - 1};
    }
    if (utils::starts_with(line, pretty_name_key)) {
      line = line.substr(line.find('=') + 1, line.length());
      pretty_name_val {line.begin() + 1, line.end() - 1};
    }
  }
  return true;
}

// _____________________________________________________________________________________________________________________

static bool readOsRelease(QUnixOSVersion &v) {
  static const std::string ID = "ID=";
  static const std::string VERSIONID = "VERSION_ID=";
  static const std::string PRETTYNAME = "PRETTY_NAME=";

  // man os-release(5) says:
  // The file /etc/os-release takes precedence over /usr/lib/os-release.
  // Applications should check for the former, and exclusively use its data
  // if it exists, and only fall back to /usr/lib/os-release if it is missing.
  return read_etc_file(v, "/etc/os-release", id, versionId, prettyName) || readEtcFile(v, "/usr/lib/os-release", id, versionId, prettyName);
}

static std::string get_full_name() {
  std::string line;
  std::ifstream stream("/etc/os-release");
  if (!stream) {
    return "Linux <unknown version>";
  }
  while (getline(stream, line)) {
    if (utils::starts_with(line, "PRETTY_NAME")) {
      line = line.substr(line.find('=') + 1, line.length());
      return {line.begin() + 1, line.end() - 1}; // remove \" at begin and end of the substring result
    }
  }
  stream.close();
  return "Linux <unknown version>";
}

static std::string get_name() {
  std::string line;
  std::ifstream stream("/etc/os-release");
  if (!stream) {
    return "Linux";
  }
  while (getline(stream, line)) {
    if (utils::starts_with(line, "NAME")) {
      line = line.substr(line.find('=') + 1, line.length());
      // remove \" at begin and end of the substring result
      return {line.begin() + 1, line.end() - 1};
    }
  }
  stream.close();
  return "Linux";
}

static std::string get_version(const char *etc_file) {
  std::string line;
  std::ifstream stream("/etc/os-release");
  if (!stream) {
    return "<unknown version>";
  }
  while (getline(stream, line)) {
    if (utils::starts_with(line, "VERSION_ID")) {
      line = line.substr(line.find('=') + 1, line.length());
      // remove \" at begin and end of the substring result
      return {line.begin() + 1, line.end() - 1};
    }
  }
  stream.close();
  return "<unknown version>";
}

static std::string get_kernel() {
  static utsname info;
  if (uname(&info) == 0) {
    return info.release;
  }
  return "<unknown kernel>";
}

static bool get_is64_bit() {
  struct stat buffer {};
  return (stat("/lib64/ld-linux-x86-64.so.2", &buffer) == 0);
}

// _____________________________________________________________________________________________________________________
OS::OS() {
  _64bit = get_is64_bit();
  _32bit = !_64bit;

  _name = get_name();
  _fullName = get_full_name();

  _version = get_version();
  _kernel = get_kernel();
  _details = "";
}

}  // namespace hwinfo

#endif  // HWINFO_UNIX
