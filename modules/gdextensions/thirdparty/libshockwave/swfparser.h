#ifndef LIBSHOCKWAVE_SWF_PARSER_H
#define LIBSHOCKWAVE_SWF_PARSER_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>

#include "core/typedefs.h"
#include "swftypedefs.h"

#define FLOAT16_EXPONENT_BASE 15

namespace SWF
{

	enum Header
	{
		SIGNATURE,
		VERSION = 3,
		FILESIZE_32,
		LENGTH = 8,
		LZMA_LENGTH = 12
	};

	enum Error
	{
		OK,
		SWF_NULL_DATA,
		SWF_DATA_INVALID,
		SWF_COMPRESSION_VERSION_MISMATCH,
		SWF_FILE_PROTECTED,
		SWF_FILE_ENCRYPTED,

		ZLIB_NOT_COMPILED,
		ZLIB_ERRNO,
		ZLIB_STREAM_ERROR,
		ZLIB_DATA_ERROR,
		ZLIB_MEMORY_ERROR,
		ZLIB_BUFFER_ERROR,
		ZLIB_VERSION_ERROR,

		LZMA_NOT_COMPILED,
		LZMA_DATA_ERROR,
		LZMA_MEM_ALLOC_ERROR,
		LZMA_INVALID_PROPS,
		LZMA_UNEXPECTED_EOF
	};

	enum TagType
	{
		End								= 0,
		ShowFrame						= 1,
		DefineShape						= 2,
		PlaceObject						= 4,
		RemoveObject					= 5,
		DefineBits						= 6,
		DefineButton					= 7,
		JPEGTables						= 8,
		SetBackgroundColor				= 9,
		DefineFont						= 10,
		DefineText						= 11,
		DoAction						= 12,
		DefineFontInfo					= 13,
		DefineSound						= 14,
		StartSound						= 15,
		DefineButtonSound				= 17,
		SoundStreamHead					= 18,
		SoundStreamBlock				= 19,
		DefineBitsLossless				= 20,
		DefineBitsJPEG2					= 21,
		DefineShape2					= 22,
		DefineButtonCxform				= 23,
		Protect							= 24,
		PlaceObject2					= 26,
		RemoveObject2					= 28,
		DefineShape3					= 32,
		DefineText2						= 33,
		DefineButton2					= 34,
		DefineBitsJPEG3					= 35,
		DefineBitsLossless2				= 36,
		DefineEditText					= 37,
		DefineSprite					= 39,
		FrameLabel						= 43,
		SoundStreamHead2				= 45,
		DefineMorphShape				= 46,
		DefineFont2						= 48,
		ExportAssets					= 56,
		ImportAssets					= 57,
		EnableDebugger					= 58,
		DoInitAction					= 59,
		DefineVideoStream				= 60,
		VideoFrame						= 61,
		DefineFontInfo2					= 62,
		EnableDebugger2					= 64,
		ScriptLimits					= 65,
		SetTabIndex						= 66,
		FileAttributes					= 69,
		PlaceObject3					= 70,
		ImportAssets2					= 71,
		DefineFontAlignZones			= 73,
		CSMTextSettings					= 74,
		DefineFont3						= 75,
		SymbolClass						= 76,
		Metadata						= 77,
		DefineScalingGrid				= 78,
		DoABC							= 82,
		DefineShape4					= 83,
		DefineMorphShape2				= 84,
		DefineSceneAndFrameLabelData	= 86,
		DefineBinaryData				= 87,
		DefineFontName					= 88,
		StartSound2						= 89,
		DefineBitsJPEG4					= 90,
		DefineFont4						= 91,
		EnableTelemetry					= 93
	};

	class Stream
	{
		const uint8_t *data;
		uint32_t datalength;
		uint8_t swfversion;
		uint32_t pos;
		uint8_t bits_pending : 4;
		uint8_t partial_byte;
		const uint8_t bitmasks[9] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

		Dictionary *dict;

		_FORCE_INLINE_ FillStyle readFILLSTYLE(uint16_t);
		_FORCE_INLINE_ uint16_t readFILLSTYLEARRAY(uint16_t, uint16_t);
		_FORCE_INLINE_ LineStyle readLINESTYLE(uint16_t);
		_FORCE_INLINE_ LineStyle readLINESTYLE2(uint16_t);
		_FORCE_INLINE_ uint16_t readLINESTYLEARRAY(uint16_t, uint16_t);
		_FORCE_INLINE_ Gradient readGRADIENT(uint16_t);
		_FORCE_INLINE_ FocalGradient readFOCALGRADIENT(uint16_t);
		_FORCE_INLINE_ GradRecord readGRADRECORD(uint16_t);
		_FORCE_INLINE_ Vertex readSHAPERECORDedge(ShapeRecordType, uint8_t);
		_FORCE_INLINE_ StyleChangeRecord readSHAPERECORDstylechange(uint16_t, uint16_t, uint8_t);

	public:
		Stream(const uint8_t*,uint32_t);
		_FORCE_INLINE_ void seek(uint32_t s) {pos=s;}
		_FORCE_INLINE_ uint32_t get_pos() {return pos;};
		_FORCE_INLINE_ void rewind(){pos=0;}
		_FORCE_INLINE_ void reset_bits_pending() {bits_pending=0;}

		Dictionary *get_dict() {return dict;}

		_FORCE_INLINE_ RecordHeader readRECORDHEADER();
		_FORCE_INLINE_ void readSHAPEWITHSTYLE(uint16_t, Rect, uint16_t);
		_FORCE_INLINE_ void readFILTERLIST();

		_FORCE_INLINE_ Rect readRECT();
		_FORCE_INLINE_ RGBA readRGB();
		_FORCE_INLINE_ RGBA readRGBA();
		_FORCE_INLINE_ RGBA readARGB();
		_FORCE_INLINE_ Matrix readMATRIX();
		//_FORCE_INLINE_ ClipActions readCLIPACTIONS();
		CXForm readCXFORM(bool alpha = false);
		CXForm readCXFORMWITHALPHA() { return readCXFORM(true); }

		_FORCE_INLINE_ int8_t readSI8();
		_FORCE_INLINE_ int16_t readSI16();
		_FORCE_INLINE_ int32_t readSI32();
		_FORCE_INLINE_ int64_t readSI64();
		_FORCE_INLINE_ uint8_t readUI8();
		_FORCE_INLINE_ uint16_t readUI16();
		_FORCE_INLINE_ uint32_t readUI32();
		_FORCE_INLINE_ uint64_t readUI64();
		_FORCE_INLINE_ float readFLOAT();
		_FORCE_INLINE_ float readFLOAT16();
		_FORCE_INLINE_ double readDOUBLE();
		_FORCE_INLINE_ float readFIXED();
		_FORCE_INLINE_ float readFIXED8();
		_FORCE_INLINE_ const char *readSTRING();

		_FORCE_INLINE_ int32_t readSB(uint8_t);
		_FORCE_INLINE_ uint32_t readUB(uint8_t);
		_FORCE_INLINE_ float readFB(uint8_t);

        _FORCE_INLINE_ void skipBytes(uint32_t bytes);
		_FORCE_INLINE_ uint8_t readByte();

		_FORCE_INLINE_ uint64_t readBytesAligned(uint8_t);
		_FORCE_INLINE_ uint64_t readBytesAlignedBigEndian(uint8_t);
		_FORCE_INLINE_ uint32_t readBits(uint8_t);
		_FORCE_INLINE_ uint32_t readEncodedU32();

		friend class Parser;
	};

	class Parser
	{
		Stream *swfstream;
		Dictionary *dictionary;
		Properties *movieprops;

		Error tag_loop(Stream*, Sprite* = 0);

	public:
		Parser() { swfstream = NULL; dictionary = NULL; movieprops=NULL; }
		~Parser() { if (dictionary) delete dictionary; }
		Error parse_swf_data(const uint8_t*, uint32_t, const char *password="");
		Dictionary *get_dict() { return dictionary; }
		Properties *get_properties() { return movieprops; }
	};
	
}

#endif //LIBSHOCKWAVE_SWF_PARSER_H
