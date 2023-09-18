// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include <hwinfo/hwinfo.h>

#include "common/gd_core.h"

#include <map>

extern "C" void hwinfo_error(const char *p_format, ...) {
  va_list list;

  va_start(list, p_format);
  String s = string_format(p_format, list);
  va_end(list);

  WARN_PRINT(s);
}

extern "C" void hwinfo_info(const char *p_format, ...) {
  va_list list;

  va_start(list, p_format);
  String s = string_format(p_format, list);
  va_end(list);

  print_verbose(s);
}

namespace hwinfo {
  std::map<CustomProperty, PropertyHandler> props;

  void _get_prop(CustomProperty prop, PropertyArg arg, bool *missing, int64_t *i, float *f, std::string *s) {
    if (props.count(prop) > 0) {
      props[prop](arg, i, f, s);
      if (missing) {
        *missing = false;
      }
    } else if (missing) {
      *missing = true;
    }
  }

  int64_t get_int_property(CustomProperty prop, PropertyArg arg, bool *missing) {
    int64_t v = 0;
    _get_prop(prop, arg, missing, &v, nullptr, nullptr);
    return v;
  }

  float get_float_property(CustomProperty prop, PropertyArg arg, bool *missing) {
    float v = 0;
    _get_prop(prop, arg, missing, nullptr, &v, nullptr);
    return v;
  }

  std::string get_string_property(CustomProperty prop, PropertyArg arg, bool *missing) {
    std::string v;
    _get_prop(prop, arg, missing, nullptr, nullptr, &v);
    return v;
  }

  int64_t get_int_property(CustomProperty prop, bool *missing) { return get_int_property(prop, PropertyArg(), missing); }
  float get_float_property(CustomProperty prop, bool *missing) { return get_float_property(prop, PropertyArg(), missing); }
  std::string get_string_property(CustomProperty prop, bool *missing) { return get_string_property(prop, PropertyArg(), missing); }

  void register_property(CustomProperty prop, PropertyHandler func) {
    props[prop] = func;
  }
}
