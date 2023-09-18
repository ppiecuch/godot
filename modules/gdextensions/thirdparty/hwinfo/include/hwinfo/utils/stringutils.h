// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <codecvt>
#include <cstdint>
#include <cstring>
#include <locale>
#include <limits>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#ifdef HWINFO_APPLE
# include <MacTypes.h>
# include <CoreFoundation/CoreFoundation.h>
#endif

namespace hwinfo {
namespace utils {

void strip(std::string& input); // remove all white spaces (' ', '\t', '\n') from start and end of input
unsigned count_substring(const std::string& input, const std::string& substring); // count occurrences of a substring in input
std::vector<std::string> split(const std::string& input, const std::string& delimiter); // split input string at delimiter and return result
std::vector<std::string> split(const std::string& input, const char delimiter); // split input string at delimiter (char) and return result
std::string split_get_index(const std::string& input, const std::string& delimiter, int index); // split input at delimiter and return substring at pos. index (index can be negative, where -1 is the last occurrence).

// convert windows wstring to string
inline std::string wstring_to_string() { return ""; }

// convert wstring to string
inline std::string wstring_to_std_string(const std::wstring& ws) {
  std::string str_locale = setlocale(LC_ALL, "");
  const wchar_t* wch_src = ws.c_str();
  size_t n_dest_size = wcstombs(NULL, wch_src, 0) + 1;
  char* ch_dest = new char[n_dest_size];
  memset(ch_dest, 0, n_dest_size);
  wcstombs(ch_dest, wch_src, n_dest_size);
  std::string result_text = ch_dest;
  delete[] ch_dest;
  setlocale(LC_ALL, str_locale.c_str());
  return result_text;
}

// replace the std::string::starts_with function only available in C++20 and above.
template <typename string_type, typename prefix_type>
inline bool starts_with(const string_type& str, const prefix_type& prefix) {
#ifdef __cpp_lib_starts_ends_with
  return str.starts_with(prefix);
#else
  return str.rfind(prefix, 0) == 0;
#endif
}

// convert anything to string
template <typename T> std::string to_string(const T& var) {
  std::basic_ostringstream<typename std::string::value_type> temp;
  temp << var;
  return temp.str();
}

// convert string to uint/long/bool
inline uint32_t stou(const std::string& str, std::size_t* pos = nullptr, int base = 10) {
  const auto ret = std::stoull(str, pos, base);
  if (ret > 0xFFFFFFFFull) {
    return std::numeric_limits<uint32_t>::max();
  }
  return (uint32_t)ret;
}

inline std::size_t stosize(const std::string& str, std::size_t* pos = nullptr, int base = 10) {
  return (std::size_t)std::stoull(str, pos, base);
}

inline bool stob(const std::string& str) {
  return (str == "1" || str == "true" || str == "TRUE" || str == "YES");
}

std::string storage_size_string(double value);
std::string time_duration_string(double value);

void dfs(int k, int t);

}  // namespace utils
}  // namespace hwinfo
