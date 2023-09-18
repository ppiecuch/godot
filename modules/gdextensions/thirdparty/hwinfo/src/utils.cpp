// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/utils/stringutils.h"

#include <map>

namespace hwinfo {
namespace utils {

// remove all white spaces (' ', '\t', '\n') from start and end of input
void strip(std::string& input) {
  if (input.empty()) {
    return;
  }
  // optimization for input size == 1
  if (input.size() == 1) {
    if (input[0] == ' ' || input[0] == '\t' || input[0] == '\n') {
      input = "";
      return;
    } else {
      return;
    }
  }
  size_t start_index = 0;
  while (true) {
    char c = input[start_index];
    if (c != ' ' && c != '\t' && c != '\n') {
      break;
    }
    start_index++;
  }
  size_t end_index = input.size() - 1;
  while (true) {
    char c = input[end_index];
    if (c != ' ' && c != '\t' && c != '\n') {
      break;
    }
    end_index--;
  }
  if (end_index < start_index) {
    input.assign("");
    return;
  }
  input.assign(input.begin() + start_index, input.begin() + end_index + 1);
}

// count occurrences of a substring in input
unsigned count_substring(const std::string& input, const std::string& substring) {
  unsigned occurrences = 0;
  std::string::size_type shift = 0;
  while ((shift = input.find(substring, shift)) != std::string::npos) {
    occurrences++;
    shift += substring.size();
  }
  return occurrences;
}

// split input string at delimiter and return result
std::vector<std::string> split(const std::string& input, const std::string& delimiter) {
  std::vector<std::string> result;
  size_t shift = 0;
  while (true) {
    size_t match = input.find(delimiter, shift);
    result.emplace_back(input.substr(shift, match - shift));
    if (match == std::string::npos) {
      break;
    }
    shift = match + delimiter.size();
  }
  return result;
}

// split input string at delimiter (char) and return result
std::vector<std::string> split(const std::string& input, const char delimiter) {
  std::vector<std::string> result;
  size_t shift = 0;
  while (true) {
    size_t match = input.find(delimiter, shift);
    if (match == std::string::npos) {
      break;
    }
    result.emplace_back(input.substr(shift, match - shift));
    shift = match + 1;
  }
  return result;
}

// split input at delimiter and return substring at position index.
// index can be negative, where -1 is the last occurrence.
std::string split_get_index(const std::string& input, const std::string& delimiter, size_t index) {
  size_t occ = count_substring(input, delimiter) + 1;
  index = index < 0 ? static_cast<int>(occ + index) : index;
  if (occ <= index) {
    return "";
  }

  std::string::size_type start_index = 0;
  while (true) {
    if (index == 0) {
      break;
    }
    start_index = input.find(delimiter, start_index) + delimiter.size();
    index--;
  }
  std::string::size_type end_index = input.find(delimiter, start_index);
  if (end_index == std::string::npos) {
    return {input.begin() + static_cast<int64_t>(start_index), input.end()};
  }
  return {input.begin() + static_cast<int64_t>(start_index), input.begin() + static_cast<int64_t>(end_index)};
}

// pretty formatting
std::string storage_size_string(double value) {
  static const std::string TokenArray[] = { "bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
  int multiply_factor = 0;
  while (value > 1024) {
    value /= 1024, multiply_factor++;
  }
  std::ostringstream out;
  out << std::setw(4) << std::setprecision(1) << value << " " << TokenArray[multiply_factor];
  return out.str();
} // storage_size_string

std::string time_duration_string(double value) {
  static const struct typelen_t {
    const char *type;
    uint64_t length;
  } TokenArray[] = {
    { "ages", 1000*365*24*60*60L },
    { "centuries", 100*365*24*60*60L },
    { "years", 365*24*60*60L },
    { "weeks", 7*24*60*60L },
    { "days", 24*60*60L },
    { "hours", 60*60L },
    { "minutes", 60L },
    { "seconds", 1L },
  };
  if (value < 1) {
    std::ostringstream out;
    out << std::setprecision(1) << value << " seconds";
    return out.str();
  } else {
    int factor = 0;
    while (value < TokenArray[factor].length) {
      factor++;
    }
    std::ostringstream out;
    out << std::setprecision(1) << value / TokenArray[factor].length << " " << TokenArray[factor].type;
    return out.str();
  }
} // time_duration_string

// https://github.com/wwaassded/fake_pstree/blob/master/fake_pstree.cpp

struct node_t {
  uint64_t parent_node_id, node_id;
  int index;
  node_t *parent = nullptr;
  node_t *last_child = nullptr, *first_child = nullptr;
  node_t *next_sibling = nullptr;
};

struct procid_t {
  uint64_t pid, ppid;
};

struct treeinfo_t {
  int index;
  char label[32];
};

static void _get_tree_descr(std::vector<std::pair<node_t*, std::string>> &descr, const std::string &prefix, node_t *node, bool flag, size_t &max_len) {
  std::ostringstream out;
  out << prefix;
  if (flag) {
    out << "|-";
  } else if (node->parent) {
    out << "|_";
  } else {
    out << "o-";
  }
  descr.push_back({ node, out.str() });
  max_len = std::max(max_len, descr.back().second.size());
  for (node_t *n = node->first_child; n != nullptr; n = n->next_sibling) {
    if (n->next_sibling == nullptr) {
      _get_tree_descr(descr, prefix + (flag ? "| " : "  "), n, false, max_len);
    } else {
      _get_tree_descr(descr, prefix + (flag ? "| " : "  "), n, true, max_len);
    }
  }
} // get_tree_descr

extern "C" bool get_tree_descr_from_data(const procid_t *data, treeinfo_t *treeinfo, size_t num) {
  std::vector<std::pair<node_t*, std::string>> r;
  std::vector<node_t> nodes; nodes.resize(num);
  std::map<uint64_t, node_t*> index;
  node_t *root = nullptr;
  for (size_t i = 0; i < num; i++) {
    node_t *node = &nodes[i];
    node->index = i;
    node->node_id = data[i].pid;
    node->parent_node_id = data[i].ppid;
    if (node->node_id == node->parent_node_id) {
      root = node; // quite likely
    }
    treeinfo[i].index = i, treeinfo[i].label[0] = 0; // default
  }
  for (size_t i = 0; i < num; i++) {
    node_t *node = &nodes[i];
    if (node->node_id == node->parent_node_id) {
      continue;
    }
    node_t *parent = index[node->parent_node_id];
    if (!parent) {
      for (size_t f = 0; f < num; f++) { // update index
        if (nodes[f].node_id == node->parent_node_id) {
          index[node->parent_node_id] = parent = &nodes[f];
          break;
        }
      }
    }
    if (!parent) {
      continue;
    }
    node->parent = parent;
    if (parent->first_child) {
      parent->last_child->next_sibling = node;
      parent->last_child = node;
    } else {
      parent->first_child = parent->last_child = node;
    }
  }
  if (!root) { // last chance
    for (size_t i = 0; i < num; i++) {
      if(!nodes[i].parent) {
        root = &nodes[i];
        break;
      }
    }
  }
  size_t max_len = 0;
  if (root) {
    _get_tree_descr(r, "", root, false, max_len);
  }
  if (r.size() == num) { // make sure we return info. for all entries
    for (size_t i = 0; i < num; i++) {
      treeinfo[i].index = r[i].first->index;
      strncpy(treeinfo[i].label, r[i].second.c_str(), 32);
    }
    return true;
  }
  return false;
} // get_tree_descr_from_data

}  // namespace utils
}  // namespace hwinfo
