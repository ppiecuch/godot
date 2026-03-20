TARGET = tester
CONFIG += debug console
CONFIG -= app_bundle

OBJECTS_DIR = build-$$TARGET
MOC_DIR = build-$$TARGET
UI_DIR = build-$$TARGET
RCC_DIR = build-$$TARGET

SOURCES += \
    tester.cpp \
    \
    ../libldr/bfc.cpp \
    ../libldr/color.cpp \
    ../libldr/elements.cpp \
    ../libldr/lmath.cpp \
    ../libldr/metrics.cpp \
    ../libldr/model.cpp \
    ../libldr/normal_extension.cpp \
    ../libldr/part_library.cpp \
    ../libldr/part_library_posix.cpp \
    ../libldr/reader.cpp \
    ../libldr/geometry_exporter.cpp \
    ../libldr/lutils.cpp \
    ../libldr/writer.cpp \
    ../libldr/bit_buffer.cpp \
    ../libldr/gzmemlib.c

DEFINES += LDR_ARCHIVE_SUPPORT LDR_TESTER
LIBS += $$PWD/../libldr/ldrawlib/ldraw_lib.o
INCLUDEPATH += ../ ../../../../contrib/zip/src
