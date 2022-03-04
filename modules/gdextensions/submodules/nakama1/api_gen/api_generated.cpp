#include "flatbuffers/flatbuffers.h"
#include <string>
#include <map>
struct Variant {
	Variant() { }
	template <typename T> Variant(const T &dummy) { }
};
using CharString = std::string;
using Array = std::vector<Variant>;
using Dictionary = std::map<std::string, Variant>;
#include "api_generated.h"
