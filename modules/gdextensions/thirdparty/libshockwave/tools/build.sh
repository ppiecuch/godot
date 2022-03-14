#!/bin/bash

g++ -o swtool -D_7ZIP_ST --std=c++11 -Wno-deprecated -I.. \
	main.cpp \
	../swfparser.cpp \
	../lzma/Alloc.c ../lzma/LzFind.c ../lzma/LzmaDec.c ../lzma/LzmaEnc.c ../lzma/LzmaLib.c \
	-lz
