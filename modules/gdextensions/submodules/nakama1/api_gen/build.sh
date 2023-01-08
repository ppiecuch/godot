#!/bin/bash

set -e

echo "*** Flatb generation"


FLATB=~/Private/Projekty/0.shared/common-dev-tools/network/flatbuffers
FLATC=$FLATB/build-osx/flatc

if [ ! -e api.fbs ]; then
  $FLATC --proto api.proto
fi

$FLATC --cpp \
  --force-empty-vectors \
  --scoped-enums \
  --oneof-union \
  --gen-all \
  --gen-object-api \
  --gen-name-strings \
  --cpp-str-type CharString \
  --cpp-str-flex-ctor \
  --cpp-dictionary-api \
  api.fbs

# patch:
gsed -i 's/Anonymous0/AuthenticateMethod/g' api_generated.h
gsed -i 's/Anonymous1/AuthenticateResult/g' api_generated.h
gsed -i 's/Anonymous2/EnvelopeContent/g' api_generated.h
gsed -i 's/Anonymous9/TopicType/g' api_generated.h
gsed -i 's/AuthenticateResult5/ScoreOperator/g' api_generated.h

gsed -i 's/namespace server {/#undef _timezone\n\nnamespace server {/g' api_generated.h
gsed -i 's/assert/assertion/g' api_generated.h

echo "#include \"flatbuffers/flatbuffers.h\"" > api_generated.cpp
echo "#include <string>" >> api_generated.cpp
echo "#include <map>" >> api_generated.cpp
echo "struct Variant {" >> api_generated.cpp
echo "	Variant() { }" >> api_generated.cpp
echo "	template <typename T> Variant(const T &dummy) { }" >> api_generated.cpp
echo "};" >> api_generated.cpp
echo "using CharString = std::string;" >> api_generated.cpp
echo "using Array = std::vector<Variant>;" >> api_generated.cpp
echo "using Dictionary = std::map<std::string, Variant>;" >> api_generated.cpp
echo "#include \"api_generated.h\"" >> api_generated.cpp

g++ --std=c++11 -c -I $FLATB/include api_generated.cpp


# ****

echo "*** Protobuf generation"

PROTOB=~/Private/Projekty/0.shared/common-dev-tools/network/protobuf/build-osx/dist
PROTOC=$PROTOB/bin/protoc

rm -rf v1_proto && mkdir -p v1_proto

$PROTOC --cpp_out=v1_proto api.proto

# patch:
gsed -i '/@@protoc_insertion_point(includes)/a #pragma push_macro("assert")' v1_proto/api.pb.h
gsed -i '/@@protoc_insertion_point(includes)/a #undef assert' v1_proto/api.pb.h

g++ -c --std=c++11 -I $PROTOB/include v1_proto/api.pb.cc
