#include "swfparser.h"

using namespace SWF;

#include <cstdio>
#ifndef LIBSHOCKWAVE_DISABLE_ZLIB
# include <zlib.h>
#endif
#ifndef LIBSHOCKWAVE_DISABLE_LZMA
# define _LZMA_PROB32
# include "lzma/LzmaLib.h"
#endif

template<class T> void _ignore_( const T& ) { }

SWF::Error Parser::parse_swf_data(const uint8_t *data, uint32_t bytes, const char *password)
{
	if(!data)
		return Error::SWF_NULL_DATA;

	size_t datalength = (
		data[Header::FILESIZE_32] |
		data[Header::FILESIZE_32+1]<<8 |
		data[Header::FILESIZE_32+2]<<16 |
		data[Header::FILESIZE_32+3]<<24
	) - Header::LENGTH;

	if(data[Header::SIGNATURE+1] != 'W' || data[Header::SIGNATURE+2] != 'S')
		return Error::SWF_DATA_INVALID;

	if (swfstream) delete swfstream;

	movieprops = new Properties();
	movieprops->version = (data[Header::VERSION]);
	switch(data[Header::SIGNATURE]) {
	case 'C':	// zlib compression
	{
		#ifndef LIBSHOCKWAVE_DISABLE_ZLIB
		//if(data[Header::VERSION] < 6)	return Error::SWF_COMPRESSION_VERSION_MISMATCH;	// invalid if below SWF6
		uLong zliblen = (uLong)(bytes-Header::LENGTH);
		uint8_t *swfdecompressed = (uint8_t*)malloc(datalength);
		int zliberror = uncompress2(swfdecompressed, (uLong*)&datalength, &data[Header::LENGTH], &zliblen);
		switch(zliberror) {
			case Z_ERRNO:			return Error::ZLIB_ERRNO;
			case Z_STREAM_ERROR:	return Error::ZLIB_STREAM_ERROR;
			case Z_DATA_ERROR:		return Error::ZLIB_DATA_ERROR;
			case Z_MEM_ERROR:		return Error::ZLIB_MEMORY_ERROR;
			case Z_BUF_ERROR:		return Error::ZLIB_BUFFER_ERROR;
			case Z_VERSION_ERROR:	return Error::ZLIB_VERSION_ERROR;
		}
		swfstream = new Stream(swfdecompressed, datalength);
		break;
		#else
		return Error::ZLIB_NOT_COMPILED;
		#endif
	}
	case 'Z':	// lzma compression
	{
		#ifndef LIBSHOCKWAVE_DISABLE_LZMA
		//if(data[Header::VERSION] < 13)	return Error::SWF_COMPRESSION_VERSION_MISMATCH;	// invalid if below SWF13
		size_t lzmalen =
			data[Header::LENGTH] |
			data[Header::LENGTH+1]<<8 |
			data[Header::LENGTH+2]<<16 |
			data[Header::LENGTH+3]<<24;
		uint8_t *swfdecompressed = (uint8_t*)malloc(datalength);
		SRes lzmaerror = LzmaUncompress(swfdecompressed, &datalength, &data[Header::LZMA_LENGTH+LZMA_PROPS_SIZE], &lzmalen, &data[Header::LZMA_LENGTH], LZMA_PROPS_SIZE);
		switch(lzmaerror) {
			case SZ_ERROR_DATA:			return Error::LZMA_DATA_ERROR;
			case SZ_ERROR_MEM:			return Error::LZMA_MEM_ALLOC_ERROR;
			case SZ_ERROR_UNSUPPORTED:	return Error::LZMA_INVALID_PROPS;
			case SZ_ERROR_INPUT_EOF:	return Error::LZMA_UNEXPECTED_EOF;
		}
		swfstream = new Stream(swfdecompressed, datalength);
		break;
		#else
		return Error::LZMA_NOT_COMPILED;
		#endif
	}
	default:	// no compression
		swfstream = new Stream(&data[Header::LENGTH], datalength);
	}

	movieprops->dimensions = swfstream->readRECT();
	movieprops->framerate = swfstream->readFIXED8();
	movieprops->framecount = swfstream->readUI16();

	swfstream->swfversion = movieprops->version;

	return this->tag_loop(swfstream);
}

SWF::Error Parser::tag_loop(Stream *swfstream, Sprite *sprite)
{
	DisplayList currentdisplaystack;
	dictionary = swfstream->get_dict();

	RecordHeader rh = swfstream->readRECORDHEADER();
	uint32_t framecounter = 0, tagcounter = 0;
	while(rh.tag != TagType::End) {
		switch(rh.tag) {
			case TagType::DefineSprite:
            {
				uint16_t spriteid = swfstream->readUI16();
				uint16_t framecount = swfstream->readUI16();
                Sprite sp;
                sp.id = spriteid;
                sp.frame_count = framecount;
                if (Error err = tag_loop(swfstream, &sp)) {
                    return err;
                }
                break;
            }
			case TagType::DefineShape:
			case TagType::DefineShape2:
			case TagType::DefineShape3:
			{
				uint16_t characterid = swfstream->readUI16();
				Rect shapebounds = swfstream->readRECT();
				swfstream->readSHAPEWITHSTYLE(characterid, shapebounds, rh.tag);
				break;
			}
			case TagType::DefineShape4:
			{
				uint16_t characterid = swfstream->readUI16();
				Rect shapebounds = swfstream->readRECT();
				/* Rect edgebounds = */ swfstream->readRECT();
				swfstream->readUB(5);	// Reserved
				/* bool usesfillwindingrule = */ swfstream->readUB(1);
				/* bool usesnonscalingstrokes = */ swfstream->readUB(1);
				/* bool usesscalingstrokes = */ swfstream->readUB(1);
				swfstream->readSHAPEWITHSTYLE(characterid, shapebounds, rh.tag);
				break;
			}
			case TagType::PlaceObject:
			{
				int readlength = swfstream->get_pos();
				uint16_t characterid = swfstream->readUI16();
				uint16_t depth = swfstream->readUI16();
				Matrix matrix = swfstream->readMATRIX();
				readlength = (swfstream->get_pos()-readlength);
				CXForm colourxform;
				if((rh.length-readlength)>0)
					colourxform = swfstream->readCXFORM();

				if (currentdisplaystack[depth].id != characterid) {
					DisplayChar character;
					character.id = characterid;
					currentdisplaystack[depth] = character;
				}
				currentdisplaystack[depth].transform = matrix;
				currentdisplaystack[depth].colourtransform = colourxform;

				break;
			}
			case TagType::PlaceObject2:
			case TagType::PlaceObject3:
			{
				int readlength = swfstream->get_pos();

				bool placeflaghasclipactions = swfstream->readUB(1);
				bool placeflaghasclipdepth = swfstream->readUB(1);
				bool placeflaghasname = swfstream->readUB(1);
				bool placeflaghasratio = swfstream->readUB(1);
				bool placeflaghascolourtransform = swfstream->readUB(1);
				bool placeflaghasmatrix = swfstream->readUB(1);
				bool placeflaghascharacter = swfstream->readUB(1);
				/* bool placeflagmove = */ swfstream->readUB(1);
				bool placeflagopaquebackground = false, placeflaghasvisible = false, placeflaghasimage = false,
					placeflaghasclassname = false, placeflaghascacheasbitmap = false, placeflaghasblendmode = false,
					placeflaghasfilterlist = false;
				if (rh.tag==TagType::PlaceObject3) {
					placeflagopaquebackground = swfstream->readUB(1);
					placeflaghasvisible = swfstream->readUB(1);
					placeflaghasimage = swfstream->readUB(1);
					placeflaghasclassname = swfstream->readUB(1);
					placeflaghascacheasbitmap = swfstream->readUB(1);
					placeflaghasblendmode = swfstream->readUB(1);
					placeflaghasfilterlist = swfstream->readUB(1);
				}
				_ignore_(placeflagopaquebackground);

				uint16_t depth = swfstream->readUI16();
				const char *name = nullptr;
				if (rh.tag==TagType::PlaceObject3 && (placeflaghasclassname || (placeflaghasimage && placeflaghascharacter)))
					name = swfstream->readSTRING();
				_ignore_(name);
				uint16_t characterid = 0;
				Matrix matrix;
				CXForm colourxform;
				if (placeflaghascharacter) characterid = swfstream->readUI16();
				if (placeflaghasmatrix) matrix = swfstream->readMATRIX();
				if (placeflaghascolourtransform) colourxform = swfstream->readCXFORMWITHALPHA();
				if (placeflaghasratio) swfstream->readUI16();
				if (placeflaghasname) swfstream->readSTRING();
				if (placeflaghasclipdepth) swfstream->readUI16();
				if (rh.tag==TagType::PlaceObject3) {
					if (placeflaghasfilterlist) swfstream->readFILTERLIST();
					if (placeflaghasblendmode) swfstream->readUI8();
					if (placeflaghascacheasbitmap) swfstream->readUI8();
					if (placeflaghasvisible) swfstream->readUI8();
					if (placeflaghasvisible) swfstream->readRGBA();
				}
				_ignore_(characterid);

				if (placeflaghascharacter) {
					DisplayChar character;
					character.id = characterid;
					currentdisplaystack[depth] = character;
				}
				if (placeflaghasmatrix) currentdisplaystack[depth].transform = matrix;
				if (placeflaghascolourtransform) currentdisplaystack[depth].colourtransform = colourxform;
				if (placeflaghasname) currentdisplaystack[depth].name = name;
				if (placeflaghasclipactions) {
					ClipActions *ca = new ClipActions(swfstream->readCLIPACTIONS());
					currentdisplaystack[depth].clip_actions = ca;
				}

				readlength = (swfstream->get_pos()-readlength);
				if ((rh.length-readlength)>0) swfstream->skipBytes(rh.length-readlength);
				break;
			}
			case TagType::RemoveObject:
			case TagType::RemoveObject2:
			{
				uint16_t characterid = 0;
				if (rh.tag==TagType::RemoveObject)	characterid = swfstream->readUI16();
				uint16_t depth = swfstream->readUI16();
				currentdisplaystack.erase(depth);
				_ignore_(characterid);
				break;
			}
			case TagType::DefineSceneAndFrameLabelData:
			{
				uint32_t scenecount = swfstream->readEncodedU32();
				for (uint32_t i = 0; i < scenecount; i++) {
					/* uint32_t offset = */ swfstream->readEncodedU32();
					/* const char *name = */ swfstream->readSTRING();
				}
				uint32_t framelabelcount = swfstream->readEncodedU32();
				for (uint32_t i = 0; i < framelabelcount; i++) {
					/* uint32_t framenum = */ swfstream->readEncodedU32();
					/* const char *name = */ swfstream->readSTRING();
				}
				break;
			}
			case TagType::SetBackgroundColor:
			{
				movieprops->bgcolour = swfstream->readRGB();
				break;
			}
			case TagType::FrameLabel:
			{
				int readlength = swfstream->get_pos();
				const char *label = swfstream->readSTRING();
				readlength = (swfstream->get_pos()-readlength);
				if ((rh.length-readlength)>0)	swfstream->readUI8();	// Named Anchor Flag
				dictionary->FrameLabels[label] = framecounter;
				break;
			}
			// Phase 1: ActionScript tags
			case TagType::DoAction:
			{
				swfstream->readACTIONBLOCK(0, framecounter, false, rh.length);
				break;
			}
			case TagType::DoInitAction:
			{
				uint16_t spriteid = swfstream->readUI16();
				swfstream->readACTIONBLOCK(spriteid, framecounter, true, rh.length - 2);
				break;
			}
			// Phase 2: Buttons
			case TagType::DefineButton:
			{
				int btn_start = swfstream->get_pos();
				Button btn;
				btn.id = swfstream->readUI16();
				while (true) {
					ButtonRecord br = swfstream->readBUTTONRECORD(rh.tag);
					if (br.state_flags == 0) break;
					btn.records.push_back(br);
				}
				dictionary->Buttons[btn.id] = btn;
				// Skip remaining action bytes
				int btn_consumed = swfstream->get_pos() - btn_start;
				if ((int)rh.length - btn_consumed > 0)
					swfstream->skipBytes(rh.length - btn_consumed);
				break;
			}
			case TagType::DefineButton2:
			{
				int tag_start = swfstream->get_pos();
				uint16_t btnid = swfstream->readUI16();
				Button btn;
				btn.id = btnid;
				btn.track_as_menu = (swfstream->readUI8() != 0);
				uint16_t action_offset = swfstream->readUI16();
				while (true) {
					ButtonRecord br = swfstream->readBUTTONRECORD(rh.tag);
					if (br.state_flags == 0) break;
					btn.records.push_back(br);
				}
				dictionary->Buttons[btnid] = btn;
				// Skip button condition actions
				int consumed = swfstream->get_pos() - tag_start;
				if ((int)rh.length - consumed > 0)
					swfstream->skipBytes(rh.length - consumed);
				break;
			}
			// Phase 2: Text
			case TagType::DefineText:
			case TagType::DefineText2:
			{
				TextDef td;
				td.id = swfstream->readUI16();
				td.bounds = swfstream->readRECT();
				td.matrix = swfstream->readMATRIX();
				uint8_t glyph_bits = swfstream->readUI8();
				uint8_t advance_bits = swfstream->readUI8();
				while (true) {
					TextRecord tr = swfstream->readTEXTRECORD(rh.tag, glyph_bits, advance_bits);
					if (tr.glyphs.empty() && !tr.has_font && !tr.has_color && !tr.has_x_offset && !tr.has_y_offset)
						break;
					td.records.push_back(tr);
				}
				dictionary->Texts[td.id] = td;
				break;
			}
			case TagType::DefineEditText:
			{
				int tag_start = swfstream->get_pos();
				EditTextDef etd;
				etd.id = swfstream->readUI16();
				etd.bounds = swfstream->readRECT();
				uint16_t flags = swfstream->readUI16();
				etd.has_text = (flags & 0x0080) != 0;
				etd.wordwrap = (flags & 0x0040) != 0;
				etd.multiline = (flags & 0x0020) != 0;
				etd.password = (flags & 0x0010) != 0;
				etd.readonly = (flags & 0x0008) != 0;
				etd.has_text_color = (flags & 0x0004) != 0;
				etd.has_max_length = (flags & 0x0002) != 0;
				etd.has_font = (flags & 0x0001) != 0;
				etd.has_font_class = (flags & 0x8000) != 0;
				etd.auto_size = (flags & 0x4000) != 0;
				etd.has_layout = (flags & 0x2000) != 0;
				etd.no_select = (flags & 0x1000) != 0;
				etd.border = (flags & 0x0800) != 0;
				etd.was_static = (flags & 0x0400) != 0;
				etd.html = (flags & 0x0200) != 0;
				etd.use_outlines = (flags & 0x0100) != 0;
				if (etd.has_font) etd.font_id = swfstream->readUI16();
				if (etd.has_font_class) etd.font_class = swfstream->readSTRING();
				if (etd.has_font) etd.font_height = swfstream->readUI16();
				if (etd.has_text_color) etd.text_color = swfstream->readRGBA();
				if (etd.has_max_length) etd.max_length = swfstream->readUI16();
				if (etd.has_layout) {
					etd.align = swfstream->readUI8();
					etd.left_margin = swfstream->readUI16();
					etd.right_margin = swfstream->readUI16();
					etd.indent = swfstream->readUI16();
					etd.leading = swfstream->readSI16();
				}
				etd.variable_name = swfstream->readSTRING();
				if (etd.has_text) etd.initial_text = swfstream->readSTRING();
				dictionary->EditTexts[etd.id] = etd;
				break;
			}
			// Phase 2: Bitmaps
			case TagType::DefineBits:
			{
				BitmapDef bd;
				bd.id = swfstream->readUI16();
				bd.tag_type = rh.tag;
				bd.data = &swfstream->data[swfstream->pos];
				bd.data_length = rh.length - 2;
				swfstream->skipBytes(bd.data_length);
				dictionary->Bitmaps[bd.id] = bd;
				break;
			}
			case TagType::JPEGTables:
			{
				dictionary->jpeg_tables = &swfstream->data[swfstream->pos];
				dictionary->jpeg_tables_length = rh.length;
				swfstream->skipBytes(rh.length);
				break;
			}
			case TagType::DefineBitsJPEG2:
			{
				BitmapDef bd;
				bd.id = swfstream->readUI16();
				bd.tag_type = rh.tag;
				bd.data = &swfstream->data[swfstream->pos];
				bd.data_length = rh.length - 2;
				swfstream->skipBytes(bd.data_length);
				dictionary->Bitmaps[bd.id] = bd;
				break;
			}
			case TagType::DefineBitsJPEG3:
			case TagType::DefineBitsJPEG4:
			{
				int jpeg_start = swfstream->get_pos();
				BitmapDef bd;
				bd.id = swfstream->readUI16();
				bd.tag_type = rh.tag;
				uint32_t alpha_offset = swfstream->readUI32();
				if (rh.tag == TagType::DefineBitsJPEG4)
					swfstream->readUI16(); // deblock param
				bd.data = &swfstream->data[swfstream->pos];
				bd.data_length = alpha_offset;
				bd.alpha_data = &swfstream->data[swfstream->pos + alpha_offset];
				bd.alpha_data_length = rh.length - (swfstream->get_pos() - jpeg_start) - alpha_offset;
				swfstream->skipBytes(rh.length - (swfstream->get_pos() - jpeg_start));
				dictionary->Bitmaps[bd.id] = bd;
				break;
			}
			case TagType::DefineBitsLossless:
			case TagType::DefineBitsLossless2:
			{
				BitmapDef bd;
				bd.id = swfstream->readUI16();
				bd.tag_type = rh.tag;
				bd.format = swfstream->readUI8();
				bd.width = swfstream->readUI16();
				bd.height = swfstream->readUI16();
				bd.data = &swfstream->data[swfstream->pos];
				bd.data_length = rh.length - 7;
				swfstream->skipBytes(bd.data_length);
				dictionary->Bitmaps[bd.id] = bd;
				break;
			}
			// Phase 2: Fonts
			case TagType::DefineFont:
			{
				FontDef fd;
				fd.id = swfstream->readUI16();
				// Skip glyph shape data
				swfstream->skipBytes(rh.length - 2);
				dictionary->Fonts[fd.id] = fd;
				break;
			}
			case TagType::DefineFont2:
			case TagType::DefineFont3:
			{
				int tag_start = swfstream->get_pos();
				FontDef fd;
				fd.id = swfstream->readUI16();
				fd.flags = swfstream->readUI8();
				swfstream->readUI8(); // language code
				uint8_t name_len = swfstream->readUI8();
				if (name_len > 0) {
					// Read font name bytes
					char *name = new char[name_len + 1];
					for (uint8_t i = 0; i < name_len; i++)
						name[i] = swfstream->readUI8();
					name[name_len] = 0;
					fd.name = name;
				}
				fd.num_glyphs = swfstream->readUI16();
				// Skip offset table and glyph shapes, read code table
				bool wide_offsets = (fd.flags & 0x08) != 0;
				bool wide_codes = (fd.flags & 0x04) != 0;
				if (fd.num_glyphs > 0) {
					// Skip offset table
					if (wide_offsets)
						swfstream->skipBytes(fd.num_glyphs * 4 + 4); // offsets + code offset
					else
						swfstream->skipBytes(fd.num_glyphs * 2 + 2);
				}
				// Skip shape table - we need code table offset
				// Just skip to end of tag for now and read remaining
				int consumed = swfstream->get_pos() - tag_start;
				if ((int)rh.length - consumed > 0)
					swfstream->skipBytes(rh.length - consumed);
				dictionary->Fonts[fd.id] = fd;
				break;
			}
			// Phase 2: Sound
			case TagType::DefineSound:
			{
				SoundDef sd;
				sd.id = swfstream->readUI16();
				uint8_t flags = swfstream->readUI8();
				sd.format = (flags >> 4) & 0x0F;
				sd.rate = (flags >> 2) & 0x03;
				sd.is_16bit = (flags & 0x02) != 0;
				sd.is_stereo = (flags & 0x01) != 0;
				sd.sample_count = swfstream->readUI32();
				sd.data = &swfstream->data[swfstream->pos];
				sd.data_length = rh.length - 7;
				swfstream->skipBytes(sd.data_length);
				dictionary->Sounds[sd.id] = sd;
				break;
			}
			case TagType::StartSound:
			{
				swfstream->readUI16(); // sound id
				swfstream->skipBytes(rh.length - 2); // SOUNDINFO
				break;
			}
			// Phase 2: MorphShapes
			case TagType::DefineMorphShape:
			case TagType::DefineMorphShape2:
			{
				int morph_start = swfstream->get_pos();
				MorphShapeDef msd;
				msd.id = swfstream->readUI16();
				msd.start_bounds = swfstream->readRECT();
				msd.end_bounds = swfstream->readRECT();
				dictionary->MorphShapes[msd.id] = msd;
				// Skip remaining shape data
				int morph_consumed = swfstream->get_pos() - morph_start;
				if ((int)rh.length - morph_consumed > 0)
					swfstream->skipBytes(rh.length - morph_consumed);
				break;
			}
			// Phase 2: Export/Import Assets
			case TagType::ExportAssets:
			{
				uint16_t count = swfstream->readUI16();
				for (uint16_t i = 0; i < count; i++) {
					AssetEntry ae;
					ae.id = swfstream->readUI16();
					ae.name = swfstream->readSTRING();
					dictionary->ExportedAssets.push_back(ae);
				}
				break;
			}
			case TagType::ImportAssets:
			{
				swfstream->readSTRING(); // URL
				uint16_t count = swfstream->readUI16();
				for (uint16_t i = 0; i < count; i++) {
					AssetEntry ae;
					ae.id = swfstream->readUI16();
					ae.name = swfstream->readSTRING();
					dictionary->ImportedAssets.push_back(ae);
				}
				break;
			}
			case TagType::FileAttributes:
			{
				uint8_t getbits = swfstream->readBits(1);	// Reserved
				getbits = swfstream->readBits(1);			// UseDirectBlit
				getbits = swfstream->readBits(1);			// UseGPU
				getbits = swfstream->readBits(1);			// HasMetadata
				getbits = swfstream->readBits(1);			// SWFFlagsAS3
				getbits = swfstream->readBits(1);			// SWFFlagsNoCrossDomainCache
				getbits = swfstream->readBits(1);			// Reserved
				getbits = swfstream->readBits(1);			// UseNetwork
				for(int i=1; i<rh.length; i++)				// Reserved bytes
					getbits = swfstream->readUI8();
				_ignore_(getbits);
				break;
			}
			case TagType::Protect:
			{
				if(rh.length>0)
					return Error::SWF_FILE_ENCRYPTED;
				//else
				//	return Error::SWF_FILE_PROTECTED;
				break;
			}
			case TagType::Metadata:
			{
				/* const char *xmldata = */ swfstream->readSTRING();
				break;
			}
			case TagType::ShowFrame:
                if (sprite)
                    sprite->Frames.push_back(currentdisplaystack);
                else
                    dictionary->Frames.push_back(currentdisplaystack);
				framecounter++;
				break;
			default:
				if (rh.length)
                    swfstream->skipBytes(rh.length);
				break;
		}
		tagcounter++;
		rh = swfstream->readRECORDHEADER();
	}
	return Error::OK;
}



Stream::Stream(const uint8_t *d, uint32_t len) : data(d), datalength(len)
{
	assert(d!=NULL);

	rewind();
	reset_bits_pending();

	dict = new Dictionary();
}

RecordHeader inline Stream::readRECORDHEADER()
{
	RecordHeader rh;
	uint16_t fulltag = readUI16();
	rh.length = (fulltag & 0x003F);
	if(rh.length==0x3F)	rh.length = readUI32();
	rh.tag = fulltag>>6;
	return rh;
}

FillStyle inline Stream::readFILLSTYLE(uint16_t tag)
{
	FillStyle fs;
	fs.StyleType = static_cast<FillStyle::Type>(readUI8());
	switch(fs.StyleType) {
	case FillStyle::Type::SOLID:
		if(tag>=TagType::DefineShape3)	fs.Color = readRGBA();
		else							fs.Color = readRGB();
		break;
	case FillStyle::Type::LINEARGRADIENT:
	case FillStyle::Type::RADIALGRADIENT:
		readMATRIX();
		readGRADIENT(tag);
		break;
	case FillStyle::Type::FOCALRADIALGRADIENT:
		readMATRIX();
		readFOCALGRADIENT(tag);
		break;
	case FillStyle::Type::REPEATINGBITMAP:
	case FillStyle::Type::CLIPPEDBITMAP:
	case FillStyle::Type::NONSMOOTHEDREPEATINGBITMAP:
	case FillStyle::Type::NONSMOOTHEDCLIPPEDBITMAP:
		readUI16();
		readMATRIX();
		break;
	}
	return fs;
}

uint16_t inline Stream::readFILLSTYLEARRAY(uint16_t characterid, uint16_t tag)
{
	uint16_t stylecount = readUI8();	// FillStyleCount
	if((stylecount==0xFF) && (tag>=TagType::DefineShape2))
		stylecount = readUI16();
	for(int i=0; i<stylecount; i++)
		dict->FillStyles[characterid].push_back(readFILLSTYLE(tag));
	return stylecount;
}

LineStyle inline Stream::readLINESTYLE(uint16_t tag)
{
	LineStyle ls;
	ls.Width = readUI16() / 20.0f;
	if(tag>=TagType::DefineShape3)	ls.Color = readRGBA();
	else							ls.Color = readRGB();
	ls.StartCapStyle = ls.EndCapStyle = LineStyle::Cap::ROUND;
	ls.JoinStyle = LineStyle::Join::ROUND;
	ls.HasFillFlag = ls.NoHScaleFlag = ls.NoVScaleFlag = ls.PixelHintingFlag = ls.NoClose = false;
	ls.MiterLimitFactor = 1.0f;
	return ls;
}

LineStyle inline Stream::readLINESTYLE2(uint16_t tag)
{
	LineStyle ls;
	ls.Width = readUI16() / 20.0f;
	ls.StartCapStyle = static_cast<LineStyle::Cap>(readUB(2));
	ls.JoinStyle = static_cast<LineStyle::Join>(readUB(2));
	ls.HasFillFlag = readUB(1);
	ls.NoHScaleFlag = readUB(1);
	ls.NoVScaleFlag = readUB(1);
	ls.PixelHintingFlag = readUB(1);
	readUB(5);	// Reserved
	ls.NoClose = readUB(1);
	ls.EndCapStyle = static_cast<LineStyle::Cap>(readUB(2));
	if(ls.JoinStyle==LineStyle::Join::MITER)
		ls.MiterLimitFactor = (readUI16()/256.0f);
	if(ls.HasFillFlag)
		ls.FillType = readFILLSTYLE(tag);
	else
		ls.Color = readRGBA();
	return ls;
}

uint16_t inline Stream::readLINESTYLEARRAY(uint16_t characterid, uint16_t tag)
{
	uint16_t stylecount = readUI8();	// LineStyleCount
	if(stylecount==0xFF)
		stylecount = readUI16();
	for(int i=0; i<stylecount; i++) {
		if(tag>=TagType::DefineShape4)	dict->LineStyles[characterid].push_back(readLINESTYLE2(tag));
		else							dict->LineStyles[characterid].push_back(readLINESTYLE(tag));
	}
	return stylecount;
}

void inline Stream::readSHAPEWITHSTYLE(uint16_t characterid, Rect bounds, uint16_t tag)
{
	uint16_t fillbase = 0;	// Offsets for when new fill styles are found mid-shape
	uint16_t linebase = 0;

	readFILLSTYLEARRAY(characterid, tag);
	readLINESTYLEARRAY(characterid, tag);
	dict->NumFillBits = readUB(4);
	dict->NumLineBits = readUB(4);

	uint8_t typeflag = readUB(1);
	uint8_t stateflags = readUB(5);
	Character character;
	Shape shape;
	Point penlocation;
	while(!(typeflag==0x00 && stateflags==0x00)) {
		if(typeflag) {
			Vertex v = readSHAPERECORDedge((stateflags&0x10) ? ShapeRecordType::STRAIGHTEDGE : ShapeRecordType::CURVEDEDGE, (stateflags&0x0F)+2);
			v.anchor.x += penlocation.x;
			v.anchor.y += penlocation.y;
			v.control.x += penlocation.x;
			v.control.y += penlocation.y;
			shape.vertices.push_back(v);
			penlocation.x = v.anchor.x;
			penlocation.y = v.anchor.y;
		} else {
			StyleChangeRecord change = readSHAPERECORDstylechange(characterid, tag, stateflags);
			if(shape.vertices.size()>1) {
				shape.closed = (
					int32_t(round(shape.vertices.front().anchor.x*20.0)) == int32_t(round(shape.vertices.back().anchor.x*20.0)) &&
					int32_t(round(shape.vertices.front().anchor.y*20.0)) == int32_t(round(shape.vertices.back().anchor.y*20.0))
					);
				character.shapes.push_back(shape);
			}
			if(change.NewStylesFlag) {
				fillbase = this->dict->FillStyles[characterid].size()-change.NumNewFillStyles;
				linebase = this->dict->LineStyles[characterid].size()-change.NumNewLineStyles;
				shape.layer++;
			}
			if(change.MoveDeltaFlag) {
				penlocation.x = change.MoveDeltaX;
				penlocation.y = change.MoveDeltaY;
			}
			shape.vertices.clear();
			Vertex v;
			v.anchor = penlocation;
			if(change.FillStyle0Flag)
				shape.fill0 = (change.FillStyle0 + fillbase);
			if(change.FillStyle1Flag)
				shape.fill1 = (change.FillStyle1 + fillbase);
			if(change.LineStyleFlag)
				shape.stroke = (change.LineStyle + linebase);
			shape.vertices.push_back(v);
		}
		typeflag = readUB(1);
		stateflags = readUB(5);
	}
	if(shape.vertices.size()>1) {
		shape.closed = (
			int32_t(round(shape.vertices.front().anchor.x*20.0)) == int32_t(round(shape.vertices.back().anchor.x*20.0)) &&
			int32_t(round(shape.vertices.front().anchor.y*20.0)) == int32_t(round(shape.vertices.back().anchor.y*20.0))
			);
		character.shapes.push_back(shape);
	}
	if(!character.is_empty()) {
		character.bounds = bounds;
		dict->CharacterList[characterid] = character;
	}
}

Gradient inline Stream::readGRADIENT(uint16_t tag)
{
	reset_bits_pending();
	Gradient g;
	g.SpreadMode = readUB(2);
	g.InterpolationMode = readUB(2);
	uint8_t numgrads = readUB(4);
	for(int i=0; i<numgrads; i++)
		g.GradientRecords.push_back(readGRADRECORD(tag));
	return g;
}

FocalGradient inline Stream::readFOCALGRADIENT(uint16_t tag)
{
	reset_bits_pending();
	FocalGradient fg;
	fg.SpreadMode = readUB(2);
	fg.InterpolationMode = readUB(2);
	uint8_t numgrads = readUB(4);
	for(int i=0; i<numgrads; i++)
		fg.GradientRecords.push_back(readGRADRECORD(tag));
	fg.FocalPoint = readFIXED8();
	return fg;
}

GradRecord inline Stream::readGRADRECORD(uint16_t tag)
{
	GradRecord gr;
	gr.Ratio = readUI8();
	if(tag>TagType::DefineShape2)	gr.Color = readRGBA();
	else							gr.Color = readRGB();
	return gr;
}

StyleChangeRecord inline Stream::readSHAPERECORDstylechange(uint16_t characterid, uint16_t tag, uint8_t stateflags)
{
	StyleChangeRecord r;

	if(stateflags&0x01) {				// StateMoveTo
		uint8_t movebits = readUB(5);
		r.MoveDeltaX = (readSB(movebits)/20.0f);
		r.MoveDeltaY = (readSB(movebits)/20.0f);
		r.MoveDeltaFlag = true;
	}

	if(stateflags&0x02) {				// StateFillStyle0
		r.FillStyle0 = readUB(dict->NumFillBits);
		r.FillStyle0Flag = true;
	}
	if(stateflags&0x04) {				// StateFillStyle1
		r.FillStyle1 = readUB(dict->NumFillBits);
		r.FillStyle1Flag = true;
	}

	if(stateflags&0x08) {				// StateLineStyle
		r.LineStyle = readUB(dict->NumLineBits);
		r.LineStyleFlag = true;
	}

	if(stateflags&0x10) {				// StateNewStyles
		r.NumNewFillStyles = readFILLSTYLEARRAY(characterid, tag);
		r.NumNewLineStyles = readLINESTYLEARRAY(characterid, tag);
		dict->NumFillBits = readUB(4);	// NumFillBits
		dict->NumLineBits = readUB(4);	// NumLineBits
		r.NewStylesFlag = true;
	}

	return r;
}

Vertex inline Stream::readSHAPERECORDedge(ShapeRecordType type, uint8_t numbits)
{
	switch (type) {
		case ShapeRecordType::STRAIGHTEDGE:
		{
			Vertex delta;
			if(readUB(1)) {		// GeneralLineFlag
				delta.anchor.x = delta.control.x = (readSB(numbits)/20.0f);
				delta.anchor.y = delta.control.y = (readSB(numbits)/20.0f);
			} else {
				if(readUB(1))	// VertLineFlag
					delta.anchor.y = delta.control.y = (readSB(numbits)/20.0f);
				else
					delta.anchor.x = delta.control.x = (readSB(numbits)/20.0f);
			}
			return delta;
		}
		case ShapeRecordType::CURVEDEDGE:
		{
			Vertex delta;
			delta.control.x = (readSB(numbits)/20.0f);
			delta.control.y = (readSB(numbits)/20.0f);
			delta.anchor.x = delta.control.x + (readSB(numbits)/20.0f);
			delta.anchor.y = delta.control.y + (readSB(numbits)/20.0f);
			return delta;
		}
		case ShapeRecordType::ENDSHAPE:
		case ShapeRecordType::STYLECHANGE:
		{
		}
	}
	return Vertex();
}

void inline Stream::readFILTERLIST()
{
	uint8_t count = readUI8();
	for(uint8_t i=0; i<count; i++) {
		uint8_t filterid = readUI8();
		switch(filterid) {
			case 0:	// DROPSHADOWFILTER
			{
				readRGBA();
				readFIXED();
				readFIXED();
				readFIXED();
				readFIXED();
				readFIXED8();
				readUB(1);
				readUB(1);
				readUB(1);
				readUB(5);
				break;
			}
			case 1:	// BLURFILTER
			{
				readFIXED();
				readFIXED();
				readUB(5);
				readUB(3);	// Reserved
				break;
			}
			case 2:	// GLOWFILTER
			{
				readRGBA();
				readFIXED();
				readFIXED();
				readFIXED8();
				readUB(1);
				readUB(1);
				readUB(1);
				readUB(5);
				break;
			}
			case 3:	// BEVELFILTER
			{
				readRGBA();
				readRGBA();
				readFIXED();
				readFIXED();
				readFIXED();
				readFIXED8();
				readUB(1);
				readUB(1);
				readUB(1);
				readUB(1);
				readUB(4);
				break;
			}
			case 4:	// GRADIENTGLOWFILTER
			{
				uint8_t numcolors = readUI8();
				for(int j=0; j<numcolors; j++) {
					readRGBA();
					readUI8();
				}
				readFIXED();
				readFIXED();
				readFIXED();
				readFIXED();
				readFIXED8();
				readUB(1);
				readUB(1);
				readUB(1);
				readUB(1);
				readUB(4);
				numcolors = readUI8();
				for(int j=0; j<numcolors; j++) {
					readRGBA();
					readUI8();
				}
				readFIXED();
				readFIXED();
				readFIXED();
				readFIXED();
				readFIXED8();
				readUB(1);
				readUB(1);
				break;
			}
			case 5:	// CONVOLUTIONFILTER
			{
				uint8_t matrixx = readUI8();
				uint8_t matrixy = readUI8();
				readFLOAT();
				readFLOAT();
				for(int j=0; j<( matrixx*matrixy ); j++)
					readFLOAT();
				readRGBA();
				readUB(6);	// Reserved
				readUB(1);
				readUB(1);
				break;
			}
			case 6:	// COLORMATRIXFILTER
			{
				for(int j=0; j<20; j++)
					readFLOAT();
				break;
			}
			case 7:	// GRADIENTBEVELFILTER
			{
				readUB(1);
				readUB(1);
				readUB(4);
				break;
			}
		}
	}
}



Rect inline Stream::readRECT()
{
	reset_bits_pending();
	uint8_t bits = readUB(5);
	Rect rect;
	rect.xmin = readSB(bits) / 20.0f;
	rect.xmax = readSB(bits) / 20.0f;
	rect.ymin = readSB(bits) / 20.0f;
	rect.ymax = readSB(bits) / 20.0f;
	return rect;
}

RGBA inline Stream::readRGB()
{
	RGBA c;
	c.r = readUI8();
	c.g = readUI8();
	c.b = readUI8();
	c.a = 0xFF;
	return c;
}

RGBA inline Stream::readRGBA()
{
	RGBA c;
	c.r = readUI8();
	c.g = readUI8();
	c.b = readUI8();
	c.a = readUI8();
	return c;
}

RGBA inline Stream::readARGB()
{
	RGBA c;
	c.a = readUI8();
	c.r = readUI8();
	c.g = readUI8();
	c.b = readUI8();
	return c;
}

Matrix inline Stream::readMATRIX()
{
	reset_bits_pending();
	Matrix m;
	uint8_t bits = readUB(1);			// HasScale
	if (bits) {
		bits = readUB(5);				// NScaleBits
		m.ScaleX = readFB(bits);		// ScaleX
		m.ScaleY = readFB(bits);		// ScaleY
	}
	bits = readUB(1);					// HasRotate
	if (bits) {
		bits = readUB(5);				// NRotateBits
		m.RotateSkew0 = readFB(bits);	// RotateSkew0
		m.RotateSkew1 = readFB(bits);	// RotateSkew1
	}
	bits = readUB(5);					// NTranslateBits
	m.TranslateX = readSB(bits)/20.0f;	// TranslateX
	m.TranslateY = readSB(bits)/20.0f;	// TranslateY
	return m;
}

CXForm inline Stream::readCXFORM(bool alpha)
{
	reset_bits_pending();
	CXForm cx;
	bool hasaddterms = readUB(1);
	bool hasmultterms = readUB(1);
	uint8_t nbits = readUB(4);
	if (hasmultterms) {
		cx.RedMultTerm = (readSB(nbits)/256.0f);
		cx.GreenMultTerm = (readSB(nbits)/256.0f);
		cx.BlueMultTerm = (readSB(nbits)/256.0f);
		if(alpha)
			cx.AlphaMultTerm = (readSB(nbits)/256.0f);
	}
	if (hasaddterms) {
		cx.RedAddTerm = readSB(nbits);
		cx.GreenAddTerm = readSB(nbits);
		cx.BlueAddTerm = readSB(nbits);
		if(alpha)
			cx.AlphaAddTerm = readSB(nbits);
	}
	return cx;
}

// Phase 1: ActionScript bytecode parsing

ActionRecord inline Stream::readACTIONRECORD()
{
	ActionRecord ar;
	ar.offset = pos;
	ar.opcode = readUI8();

	if (ar.opcode == 0) return ar; // End of actions

	if (ar.opcode >= 0x80) {
		ar.length = readUI16();
	}

	uint32_t start_pos = pos;

	switch (ar.opcode) {
		case ActionPush:
		{
			uint32_t end_pos = start_pos + ar.length;
			while (pos < end_pos) {
				PushValue pv;
				pv.type = static_cast<PushType>(readUI8());
				switch (pv.type) {
					case PUSH_STRING:
						pv.string_value = readSTRING();
						break;
					case PUSH_FLOAT:
					{
						uint32_t bits = readUI32();
						memcpy(&pv.float_value, &bits, 4);
						break;
					}
					case PUSH_NULL:
					case PUSH_UNDEFINED:
						break;
					case PUSH_REGISTER:
						pv.register_index = readUI8();
						break;
					case PUSH_BOOLEAN:
						pv.boolean_value = readUI8() != 0;
						break;
					case PUSH_DOUBLE:
					{
						// SWF stores doubles with swapped 32-bit halves
						uint32_t lo = readUI32();
						uint32_t hi = readUI32();
						uint64_t full = ((uint64_t)hi) | ((uint64_t)lo << 32);
						memcpy(&pv.double_value, &full, 8);
						break;
					}
					case PUSH_INTEGER:
						pv.integer_value = readSI32();
						break;
					case PUSH_CONSTANT8:
						pv.constant8 = readUI8();
						break;
					case PUSH_CONSTANT16:
						pv.constant16 = readUI16();
						break;
				}
				ar.push_values.push_back(pv);
			}
			break;
		}
		case ActionJump:
		case ActionIf:
			ar.branch_offset = readSI16();
			break;
		case ActionGotoFrame:
			ar.frame = readUI16();
			break;
		case ActionGetURL:
			ar.string1 = readSTRING();
			ar.string2 = readSTRING();
			break;
		case ActionConstantPool:
		{
			uint16_t count = readUI16();
			for (uint16_t i = 0; i < count; i++)
				ar.constant_pool.push_back(readSTRING());
			break;
		}
		case ActionStoreRegister:
			ar.register_index = readUI8();
			break;
		case ActionDefineFunction:
		{
			ar.function_def.name = readSTRING();
			ar.function_def.num_params = readUI16();
			for (uint16_t i = 0; i < ar.function_def.num_params; i++) {
				FunctionParam fp;
				fp.name = readSTRING();
				ar.function_def.params.push_back(fp);
			}
			ar.function_def.code_size = readUI16();
			// Parse nested action body
			uint32_t body_end = pos + ar.function_def.code_size;
			while (pos < body_end) {
				ActionRecord nested = readACTIONRECORD();
				ar.function_def.body.push_back(nested);
				if (nested.opcode == 0) break;
			}
			pos = body_end; // ensure alignment
			break;
		}
		case ActionDefineFunction2:
		{
			ar.function_def.name = readSTRING();
			ar.function_def.num_params = readUI16();
			ar.function_def.register_count = readUI8();
			ar.function_def.preload_flags = readUI16();
			for (uint16_t i = 0; i < ar.function_def.num_params; i++) {
				FunctionParam fp;
				fp.register_index = readUI8();
				fp.name = readSTRING();
				ar.function_def.params.push_back(fp);
			}
			ar.function_def.code_size = readUI16();
			// Parse nested action body
			uint32_t body_end = pos + ar.function_def.code_size;
			while (pos < body_end) {
				ActionRecord nested = readACTIONRECORD();
				ar.function_def.body.push_back(nested);
				if (nested.opcode == 0) break;
			}
			pos = body_end;
			break;
		}
		case ActionTry:
		{
			uint8_t flags = readUI8();
			ar.try_def.has_catch = (flags & 0x01) != 0;
			ar.try_def.has_finally = (flags & 0x02) != 0;
			ar.try_def.catch_in_register = (flags & 0x04) != 0;
			ar.try_def.try_size = readUI16();
			ar.try_def.catch_size = readUI16();
			ar.try_def.finally_size = readUI16();
			if (ar.try_def.catch_in_register)
				ar.try_def.catch_register = readUI8();
			else
				ar.try_def.catch_name = readSTRING();
			break;
		}
		case ActionWith:
			ar.with_size = readUI16();
			break;
		case ActionGetURL2:
			ar.url2_flags = readUI8();
			break;
		case ActionGotoFrame2:
		{
			uint8_t flags = readUI8();
			ar.play_flag = (flags & 0x01) != 0;
			bool has_bias = (flags & 0x02) != 0;
			if (has_bias)
				ar.scene_bias = readUI16();
			break;
		}
		case ActionSetTarget:
		case ActionGoToLabel:
			ar.string1 = readSTRING();
			break;
		case ActionWaitForFrame:
			ar.frame = readUI16();
			ar.skip_count = readUI8();
			break;
		case ActionWaitForFrame2:
			ar.skip_count = readUI8();
			break;
		default:
			// Unknown action with data - skip remaining bytes
			if (ar.opcode >= 0x80 && ar.length > 0) {
				uint32_t consumed = pos - start_pos;
				if (consumed < ar.length)
					skipBytes(ar.length - consumed);
			}
			break;
	}

	return ar;
}

void inline Stream::readACTIONBLOCK(uint16_t sprite_id, uint16_t frame, bool is_init, uint32_t tag_length)
{
	ActionBlock block;
	block.sprite_id = sprite_id;
	block.frame = frame;
	block.is_init = is_init;

	uint32_t end_pos = pos + tag_length;
	while (pos < end_pos) {
		ActionRecord ar = readACTIONRECORD();
		block.actions.push_back(ar);
		if (ar.opcode == 0) break;
	}
	pos = end_pos; // ensure alignment

	dict->Actions.push_back(block);
}

uint32_t inline Stream::readCLIPEVENTFLAGS()
{
	if (swfversion >= 6)
		return readUI32();
	else
		return readUI16();
}

ClipActionRecord inline Stream::readCLIPACTIONRECORD()
{
	ClipActionRecord car;
	car.event_flags = readCLIPEVENTFLAGS();
	if (car.event_flags == 0) return car; // End sentinel

	uint32_t action_record_size = readUI32();
	if (car.event_flags & ClipEventKeyPress)
		car.key_code = readUI8();

	uint32_t end_pos = pos + action_record_size - ((car.event_flags & ClipEventKeyPress) ? 1 : 0);
	while (pos < end_pos) {
		ActionRecord ar = readACTIONRECORD();
		car.actions.push_back(ar);
		if (ar.opcode == 0) break;
	}
	pos = end_pos;

	return car;
}

ClipActions inline Stream::readCLIPACTIONS()
{
	ClipActions ca;
	readUI16(); // Reserved
	ca.all_event_flags = readCLIPEVENTFLAGS();

	while (true) {
		ClipActionRecord car = readCLIPACTIONRECORD();
		if (car.event_flags == 0) break; // End sentinel
		ca.records.push_back(car);
	}

	return ca;
}

// Phase 2: Additional tag parsing helpers

ButtonRecord inline Stream::readBUTTONRECORD(uint16_t tag)
{
	ButtonRecord br;
	uint8_t flags = readUI8();
	if (flags == 0) {
		br.state_flags = 0;
		return br;
	}
	br.state_flags = flags;
	br.has_blend_mode = (tag == TagType::DefineButton2) && (flags & 0x20);
	br.has_filter_list = (tag == TagType::DefineButton2) && (flags & 0x10);
	br.character_id = readUI16();
	br.depth = readUI16();
	br.matrix = readMATRIX();
	if (tag == TagType::DefineButton2)
		br.color_transform = readCXFORMWITHALPHA();
	if (br.has_filter_list)
		readFILTERLIST();
	if (br.has_blend_mode)
		br.blend_mode = readUI8();
	return br;
}

TextRecord inline Stream::readTEXTRECORD(uint16_t tag, uint8_t glyph_bits, uint8_t advance_bits)
{
	TextRecord tr;
	uint8_t flags = readUI8();
	if (flags == 0) return tr; // End sentinel

	tr.has_font = (flags & 0x08) != 0;
	tr.has_color = (flags & 0x04) != 0;
	tr.has_y_offset = (flags & 0x02) != 0;
	tr.has_x_offset = (flags & 0x01) != 0;

	if (tr.has_font) tr.font_id = readUI16();
	if (tr.has_color) {
		if (tag == TagType::DefineText2)
			tr.color = readRGBA();
		else
			tr.color = readRGB();
	}
	if (tr.has_x_offset) tr.x_offset = readSI16();
	if (tr.has_y_offset) tr.y_offset = readSI16();
	if (tr.has_font) tr.text_height = readUI16();

	uint8_t glyph_count = readUI8();
	for (uint8_t i = 0; i < glyph_count; i++) {
		GlyphEntry ge;
		ge.index = readUB(glyph_bits);
		ge.advance = readSB(advance_bits);
		tr.glyphs.push_back(ge);
	}

	return tr;
}



int8_t inline Stream::readSI8()
{
	return readByte();
}

int16_t inline Stream::readSI16()
{
	return (int16_t)readBytesAligned(sizeof(int16_t));
}

int32_t inline Stream::readSI32()
{
	return (int32_t)readBytesAligned(sizeof(int32_t));
}

int64_t inline Stream::readSI64()
{
	return (int64_t)readBytesAligned(sizeof(int64_t));
}

uint8_t inline Stream::readUI8()
{
	return readByte();
}

uint16_t inline Stream::readUI16()
{
	return (uint16_t)readBytesAligned(sizeof(uint16_t));
}

uint32_t inline Stream::readUI32()
{
	return (uint32_t)readBytesAligned(sizeof(uint32_t));
}

uint64_t inline Stream::readUI64()
{
	return (uint64_t)readBytesAligned(sizeof(uint64_t));
}

float inline Stream::readFLOAT()
{
	return (float)readBytesAligned(sizeof(float));
}

float inline Stream::readFLOAT16()
{
	uint16_t float16 = readSI16();
	int8_t sign = 1;
	if ((float16&0x8000) != 0)	sign = -1;
	uint8_t exponent = ( float16>>10 ) & 0x1F;
	uint16_t significand = float16 & 0x3FF;
	if (exponent==0) {
		if (significand==0)	return 0.0f;
		return (float)(sign * pow(2, 1-FLOAT16_EXPONENT_BASE) * (significand / 1024.0f));
	} else if (exponent==31) {
		if(significand==0)	return 0.0f;
		return 0.0f;
	}
	return (float)(sign * pow(2, exponent-FLOAT16_EXPONENT_BASE) * (1 + significand / 1024.0f));
}

double inline Stream::readDOUBLE()
{
	return (double)readBytesAligned(sizeof(double));
}

float inline Stream::readFIXED()
{
	return readSI32() / 65536.0f;
}

float inline Stream::readFIXED8()
{
	return readSI16() / 256.0f;
}

const char inline *Stream::readSTRING()
{
	char *s = new char[256];
	char newchar;
	unsigned int i=0;
	for(; i == 0 || newchar != 0x00; i++) {
		if(i%256 == 0) {
			char *snew = new char[i+256];
			memcpy(snew, s, i);
			delete [] s;
			s = snew;
		}
		newchar = readUI8();
		s[i] = newchar;
	}
	char *snew = new char[i];
	memcpy(snew, s, i);
	delete [] s;
	return snew;
}



int32_t inline Stream::readSB(uint8_t bits)
{
	uint32_t readbits = readBits(bits);
	if (readbits&(1<<(bits-1)))	readbits |= (0xFFFFFFFF<<bits);
	return (int32_t)readbits;
}

uint32_t inline Stream::readUB(uint8_t bits)
{
	return readBits(bits);
}

float inline Stream::readFB(uint8_t bits)
{
	return readSB(bits) / 65536.0f;
}



void inline Stream::skipBytes(uint32_t bytes)
{
	reset_bits_pending();
    pos += bytes;
	assert(pos<datalength);
}

uint8_t inline Stream::readByte()
{
	reset_bits_pending();
	assert(pos!=datalength);
	return data[pos++];
}


uint64_t inline Stream::readBytesAligned(uint8_t bytes)
{
	uint64_t returnval = 0;
	for (uint8_t i = 0; i < bytes; i++)
		returnval |= (uint64_t)readByte()<<(i*8);
	return returnval;
}

uint64_t inline Stream::readBytesAlignedBigEndian(uint8_t bytes)
{
	uint64_t returnval = 0;
	for (int8_t i = bytes; i > 0; i--)
		returnval |= (uint64_t)readByte()<<((i-1)*8);
	return returnval;
}

uint32_t inline Stream::readBits(uint8_t bits)
{
	if (bits==0)	return 0;
	if ((bits%8)==0 && bits_pending==0)	return readBytesAlignedBigEndian(bits/8);
	
	uint32_t returnval = 0;
	while (bits>0) {
		if (bits_pending > 0) {
			uint8_t take = (uint8_t)fmin(bits_pending, bits);
			if (bits_pending==take) {
				returnval = (returnval<<take) | partial_byte;
				partial_byte = 0;
			} else {
				uint8_t mask = bitmasks[take];
				uint8_t remainmask = bitmasks[bits_pending-take];
				uint8_t taken = ( ( partial_byte >> (bits_pending - take) ) & mask );
				returnval = (returnval<<take) | taken;
				partial_byte = partial_byte & remainmask;
			}
			if (take==bits_pending)	partial_byte = 0;
			bits_pending -= take;
			bits -= take;
			continue;
		}
		partial_byte = readByte();
		bits_pending = 8;
	}

	return returnval;
}

uint32_t inline Stream::readEncodedU32()
{
	uint32_t result = readByte();
	if ((result&0x80) != 0) {
		result = (result&0x7F) | ( readByte()<<7);
		if ((result&0x4000) != 0) {
			result = (result&0x3FFF) | ( readByte()<<14);
			if ((result&0x200000) != 0) {
				result = (result&0x1FFFFF) | ( readByte()<<21);
				if ((result&0x10000000) != 0) {
					result = (result&0xFFFFFFF) | ( readByte()<<28);
				}
			}
		}
	}
	return result;
}

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "swfexport.h"

TEST_SUITE("[[libshockwave]] SWF Stream Reading") {

	TEST_CASE("[swf] readUI8") {
		uint8_t data[] = { 0x00, 0x42, 0xFF };
		Stream s(data, sizeof(data));
		CHECK(s.readUI8() == 0x00);
		CHECK(s.readUI8() == 0x42);
		CHECK(s.readUI8() == 0xFF);
	}

	TEST_CASE("[swf] readUI16") {
		uint8_t data[] = { 0x34, 0x12, 0xFF, 0xFF, 0x00, 0x00 };
		Stream s(data, sizeof(data));
		CHECK(s.readUI16() == 0x1234);
		CHECK(s.readUI16() == 0xFFFF);
		CHECK(s.readUI16() == 0x0000);
	}

	TEST_CASE("[swf] readUI32") {
		uint8_t data[] = { 0x78, 0x56, 0x34, 0x12 };
		Stream s(data, sizeof(data));
		CHECK(s.readUI32() == 0x12345678);
	}

	TEST_CASE("[swf] readSI8") {
		uint8_t data[] = { 0x00, 0x7F, 0x80, 0xFF };
		Stream s(data, sizeof(data));
		CHECK(s.readSI8() == 0);
		CHECK(s.readSI8() == 127);
		CHECK(s.readSI8() == -128);
		CHECK(s.readSI8() == -1);
	}

	TEST_CASE("[swf] readSI16") {
		uint8_t data[] = { 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x80, 0xFF, 0xFF };
		Stream s(data, sizeof(data));
		CHECK(s.readSI16() == 0);
		CHECK(s.readSI16() == 32767);
		CHECK(s.readSI16() == -32768);
		CHECK(s.readSI16() == -1);
	}

	TEST_CASE("[swf] readFIXED") {
		uint8_t data[] = { 0x00, 0x00, 0x01, 0x00 };
		Stream s(data, sizeof(data));
		CHECK(s.readFIXED() == doctest::Approx(1.0f));
	}

	TEST_CASE("[swf] readFIXED8") {
		uint8_t data[] = { 0x00, 0x01 };
		Stream s(data, sizeof(data));
		CHECK(s.readFIXED8() == doctest::Approx(1.0f));
	}

	TEST_CASE("[swf] readBits") {
		uint8_t data[] = { 0xA5, 0xFF };
		Stream s(data, sizeof(data));
		CHECK(s.readBits(1) == 1);
		CHECK(s.readBits(1) == 0);
		CHECK(s.readBits(2) == 2);
		CHECK(s.readBits(4) == 5);
		CHECK(s.readBits(8) == 0xFF);
	}

	TEST_CASE("[swf] readSB") {
		uint8_t data[] = { 0xE0 }; // 11100000
		Stream s(data, sizeof(data));
		CHECK(s.readSB(3) == -1);
	}

	TEST_CASE("[swf] readSTRING") {
		uint8_t data[] = { 'H', 'e', 'l', 'l', 'o', 0x00 };
		Stream s(data, sizeof(data));
		const char *str = s.readSTRING();
		CHECK(strcmp(str, "Hello") == 0);
		delete[] str;
	}

	TEST_CASE("[swf] readSTRING empty") {
		uint8_t data[] = { 0x00 };
		Stream s(data, sizeof(data));
		const char *str = s.readSTRING();
		CHECK(strcmp(str, "") == 0);
		delete[] str;
	}

	TEST_CASE("[swf] readEncodedU32") {
		SUBCASE("1-byte") {
			uint8_t data[] = { 0x3F };
			Stream s(data, sizeof(data));
			CHECK(s.readEncodedU32() == 63);
		}
		SUBCASE("2-byte") {
			uint8_t data[] = { 0x80, 0x01 };
			Stream s(data, sizeof(data));
			CHECK(s.readEncodedU32() == 128);
		}
	}

	TEST_CASE("[swf] seek and get_pos") {
		uint8_t data[] = { 0xAA, 0xBB, 0xCC, 0xDD };
		Stream s(data, sizeof(data));
		CHECK(s.get_pos() == 0);
		s.seek(2);
		CHECK(s.get_pos() == 2);
		CHECK(s.readUI8() == 0xCC);
	}
}

TEST_SUITE("[[libshockwave]] SWF Record Header") {

	TEST_CASE("[swf] short header") {
		uint16_t header = (1 << 6) | 0;
		uint8_t data[] = { (uint8_t)(header & 0xFF), (uint8_t)(header >> 8), 0, 0 };
		Stream s(data, sizeof(data));
		RecordHeader rh = s.readRECORDHEADER();
		CHECK(rh.tag == 1);
		CHECK(rh.length == 0);
	}

	TEST_CASE("[swf] short header with length") {
		uint16_t header = (9 << 6) | 3;
		uint8_t data[] = { (uint8_t)(header & 0xFF), (uint8_t)(header >> 8), 0, 0 };
		Stream s(data, sizeof(data));
		RecordHeader rh = s.readRECORDHEADER();
		CHECK(rh.tag == 9);
		CHECK(rh.length == 3);
	}

	TEST_CASE("[swf] long header") {
		uint16_t header = (2 << 6) | 0x3F;
		uint8_t data[6];
		data[0] = header & 0xFF;
		data[1] = header >> 8;
		data[2] = 100; data[3] = 0; data[4] = 0; data[5] = 0;
		Stream s(data, sizeof(data));
		RecordHeader rh = s.readRECORDHEADER();
		CHECK(rh.tag == 2);
		CHECK(rh.length == 100);
	}
}

TEST_SUITE("[[libshockwave]] SWF Data Structures") {

	TEST_CASE("[swf] readRGB") {
		uint8_t data[] = { 0xFF, 0x80, 0x40 };
		Stream s(data, sizeof(data));
		RGBA c = s.readRGB();
		CHECK(c.r == 0xFF);
		CHECK(c.g == 0x80);
		CHECK(c.b == 0x40);
		CHECK(c.a == 0xFF);
	}

	TEST_CASE("[swf] readRGBA") {
		uint8_t data[] = { 0xFF, 0x80, 0x40, 0xC0 };
		Stream s(data, sizeof(data));
		RGBA c = s.readRGBA();
		CHECK(c.r == 0xFF);
		CHECK(c.g == 0x80);
		CHECK(c.b == 0x40);
		CHECK(c.a == 0xC0);
	}

	TEST_CASE("[swf] readMATRIX identity") {
		uint8_t data[] = { 0x00, 0x00 };
		Stream s(data, sizeof(data));
		Matrix m = s.readMATRIX();
		CHECK(m.ScaleX == doctest::Approx(1.0f));
		CHECK(m.ScaleY == doctest::Approx(1.0f));
		CHECK(m.TranslateX == doctest::Approx(0.0f));
		CHECK(m.TranslateY == doctest::Approx(0.0f));
	}

	TEST_CASE("[swf] CXForm IsModified") {
		CXForm cx;
		CHECK_FALSE(cx.IsModified());
		cx.RedAddTerm = 10;
		CHECK(cx.IsModified());
	}
}

static void swf_append_ui16_le(std::vector<uint8_t> &buf, uint16_t val) {
	buf.push_back(val & 0xFF);
	buf.push_back((val >> 8) & 0xFF);
}
static void swf_append_si16_le(std::vector<uint8_t> &buf, int16_t val) {
	swf_append_ui16_le(buf, (uint16_t)val);
}
static void swf_append_string(std::vector<uint8_t> &buf, const char *s) {
	while (*s) buf.push_back(*s++);
	buf.push_back(0);
}

TEST_SUITE("[[libshockwave]] SWF ActionScript Parsing") {

	TEST_CASE("[swf] simple actions") {
		uint8_t data[] = { ActionPlay, ActionStop, ActionNextFrame, 0x00 };
		Stream s(data, sizeof(data));
		CHECK(s.readACTIONRECORD().opcode == ActionPlay);
		CHECK(s.readACTIONRECORD().opcode == ActionStop);
		CHECK(s.readACTIONRECORD().opcode == ActionNextFrame);
	}

	TEST_CASE("[swf] ActionPush string") {
		std::vector<uint8_t> data;
		data.push_back(ActionPush);
		std::vector<uint8_t> payload;
		payload.push_back(PUSH_STRING);
		swf_append_string(payload, "hello");
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		REQUIRE(ar.push_values.size() == 1);
		CHECK(ar.push_values[0].type == PUSH_STRING);
		CHECK(strcmp(ar.push_values[0].string_value, "hello") == 0);
	}

	TEST_CASE("[swf] ActionPush integer") {
		std::vector<uint8_t> data;
		data.push_back(ActionPush);
		std::vector<uint8_t> payload;
		payload.push_back(PUSH_INTEGER);
		payload.push_back(42); payload.push_back(0); payload.push_back(0); payload.push_back(0);
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		REQUIRE(ar.push_values.size() == 1);
		CHECK(ar.push_values[0].integer_value == 42);
	}

	TEST_CASE("[swf] ActionPush multiple values") {
		std::vector<uint8_t> data;
		data.push_back(ActionPush);
		std::vector<uint8_t> payload;
		payload.push_back(PUSH_NULL);
		payload.push_back(PUSH_BOOLEAN);
		payload.push_back(1);
		payload.push_back(PUSH_UNDEFINED);
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		REQUIRE(ar.push_values.size() == 3);
		CHECK(ar.push_values[0].type == PUSH_NULL);
		CHECK(ar.push_values[1].type == PUSH_BOOLEAN);
		CHECK(ar.push_values[1].boolean_value == true);
		CHECK(ar.push_values[2].type == PUSH_UNDEFINED);
	}

	TEST_CASE("[swf] ActionPush float") {
		std::vector<uint8_t> data;
		data.push_back(ActionPush);
		std::vector<uint8_t> payload;
		payload.push_back(PUSH_FLOAT);
		float val = 1.5f;
		uint32_t bits;
		memcpy(&bits, &val, 4);
		payload.push_back(bits & 0xFF);
		payload.push_back((bits >> 8) & 0xFF);
		payload.push_back((bits >> 16) & 0xFF);
		payload.push_back((bits >> 24) & 0xFF);
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		REQUIRE(ar.push_values.size() == 1);
		CHECK(ar.push_values[0].float_value == doctest::Approx(1.5f));
	}

	TEST_CASE("[swf] ActionPush register") {
		std::vector<uint8_t> data;
		data.push_back(ActionPush);
		std::vector<uint8_t> payload;
		payload.push_back(PUSH_REGISTER);
		payload.push_back(3);
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		REQUIRE(ar.push_values.size() == 1);
		CHECK(ar.push_values[0].register_index == 3);
	}

	TEST_CASE("[swf] ActionPush constant8 and constant16") {
		std::vector<uint8_t> data;
		data.push_back(ActionPush);
		std::vector<uint8_t> payload;
		payload.push_back(PUSH_CONSTANT8);
		payload.push_back(5);
		payload.push_back(PUSH_CONSTANT16);
		payload.push_back(0x00); payload.push_back(0x01);
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		REQUIRE(ar.push_values.size() == 2);
		CHECK(ar.push_values[0].constant8 == 5);
		CHECK(ar.push_values[1].constant16 == 256);
	}

	TEST_CASE("[swf] ActionConstantPool") {
		std::vector<uint8_t> data;
		data.push_back(ActionConstantPool);
		std::vector<uint8_t> payload;
		swf_append_ui16_le(payload, 2);
		swf_append_string(payload, "foo");
		swf_append_string(payload, "bar");
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		REQUIRE(ar.constant_pool.size() == 2);
		CHECK(strcmp(ar.constant_pool[0], "foo") == 0);
		CHECK(strcmp(ar.constant_pool[1], "bar") == 0);
	}

	TEST_CASE("[swf] ActionJump") {
		std::vector<uint8_t> data;
		data.push_back(ActionJump);
		swf_append_ui16_le(data, 2);
		swf_append_si16_le(data, 10);
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(ar.opcode == ActionJump);
		CHECK(ar.branch_offset == 10);
	}

	TEST_CASE("[swf] ActionIf negative offset") {
		std::vector<uint8_t> data;
		data.push_back(ActionIf);
		swf_append_ui16_le(data, 2);
		swf_append_si16_le(data, -5);
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(ar.branch_offset == -5);
	}

	TEST_CASE("[swf] ActionGotoFrame") {
		std::vector<uint8_t> data;
		data.push_back(ActionGotoFrame);
		swf_append_ui16_le(data, 2);
		swf_append_ui16_le(data, 42);
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(ar.frame == 42);
	}

	TEST_CASE("[swf] ActionGetURL") {
		std::vector<uint8_t> data;
		data.push_back(ActionGetURL);
		std::vector<uint8_t> payload;
		swf_append_string(payload, "http://example.com");
		swf_append_string(payload, "_blank");
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(strcmp(ar.string1, "http://example.com") == 0);
		CHECK(strcmp(ar.string2, "_blank") == 0);
	}

	TEST_CASE("[swf] ActionStoreRegister") {
		std::vector<uint8_t> data;
		data.push_back(ActionStoreRegister);
		swf_append_ui16_le(data, 1);
		data.push_back(2);
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(ar.register_index == 2);
	}

	TEST_CASE("[swf] ActionDefineFunction") {
		std::vector<uint8_t> data;
		data.push_back(ActionDefineFunction);
		std::vector<uint8_t> payload;
		swf_append_string(payload, "myFunc");
		swf_append_ui16_le(payload, 1);
		swf_append_string(payload, "x");
		swf_append_ui16_le(payload, 1);
		payload.push_back(ActionPlay);
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(strcmp(ar.function_def.name, "myFunc") == 0);
		CHECK(ar.function_def.num_params == 1);
		REQUIRE(ar.function_def.params.size() == 1);
		CHECK(strcmp(ar.function_def.params[0].name, "x") == 0);
	}

	TEST_CASE("[swf] ActionTry") {
		std::vector<uint8_t> data;
		data.push_back(ActionTry);
		std::vector<uint8_t> payload;
		payload.push_back(0x07);
		swf_append_ui16_le(payload, 10);
		swf_append_ui16_le(payload, 5);
		swf_append_ui16_le(payload, 3);
		payload.push_back(2);
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(ar.try_def.has_catch == true);
		CHECK(ar.try_def.has_finally == true);
		CHECK(ar.try_def.catch_in_register == true);
		CHECK(ar.try_def.try_size == 10);
		CHECK(ar.try_def.catch_register == 2);
	}

	TEST_CASE("[swf] ActionGotoFrame2 with play flag") {
		std::vector<uint8_t> data;
		data.push_back(ActionGotoFrame2);
		swf_append_ui16_le(data, 1);
		data.push_back(0x01);
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(ar.play_flag == true);
	}

	TEST_CASE("[swf] ActionSetTarget") {
		std::vector<uint8_t> data;
		data.push_back(ActionSetTarget);
		std::vector<uint8_t> payload;
		swf_append_string(payload, "/mc1");
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(strcmp(ar.string1, "/mc1") == 0);
	}

	TEST_CASE("[swf] ActionGoToLabel") {
		std::vector<uint8_t> data;
		data.push_back(ActionGoToLabel);
		std::vector<uint8_t> payload;
		swf_append_string(payload, "start");
		swf_append_ui16_le(data, (uint16_t)payload.size());
		data.insert(data.end(), payload.begin(), payload.end());
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(strcmp(ar.string1, "start") == 0);
	}

	TEST_CASE("[swf] ActionWaitForFrame") {
		std::vector<uint8_t> data;
		data.push_back(ActionWaitForFrame);
		swf_append_ui16_le(data, 3);
		swf_append_ui16_le(data, 10);
		data.push_back(2);
		data.push_back(0);

		Stream s(data.data(), data.size());
		ActionRecord ar = s.readACTIONRECORD();
		CHECK(ar.frame == 10);
		CHECK(ar.skip_count == 2);
	}
}

TEST_SUITE("[[libshockwave]] SWF Shape Export") {

	static Dictionary make_test_dict() {
		Dictionary dict;
		Character ch;
		ch.bounds.xmin = 0; ch.bounds.ymin = 0;
		ch.bounds.xmax = 100; ch.bounds.ymax = 100;

		Shape shape;
		shape.fill1 = 1; shape.stroke = 1; shape.closed = true;

		Vertex v1; v1.anchor.x = 0; v1.anchor.y = 0; v1.control.x = 0; v1.control.y = 0;
		Vertex v2; v2.anchor.x = 100; v2.anchor.y = 0; v2.control.x = 100; v2.control.y = 0;
		Vertex v3; v3.anchor.x = 100; v3.anchor.y = 100; v3.control.x = 100; v3.control.y = 100;
		Vertex v4; v4.anchor.x = 0; v4.anchor.y = 0; v4.control.x = 0; v4.control.y = 0;
		shape.vertices.push_back(v1);
		shape.vertices.push_back(v2);
		shape.vertices.push_back(v3);
		shape.vertices.push_back(v4);

		ch.shapes.push_back(shape);
		dict.CharacterList[1] = ch;

		FillStyle fs;
		fs.StyleType = FillStyle::Type::SOLID;
		fs.Color.r = 255; fs.Color.g = 0; fs.Color.b = 0; fs.Color.a = 255;
		dict.FillStyles[1].push_back(fs);

		LineStyle ls;
		ls.Width = 1.0f;
		ls.Color.r = 0; ls.Color.g = 0; ls.Color.b = 0; ls.Color.a = 255;
		dict.LineStyles[1].push_back(ls);
		return dict;
	}

	TEST_CASE("[swf] SVG export produces valid SVG") {
		Dictionary dict = make_test_dict();
		std::string svg = SVGExporter::export_shape(dict, 1);
		CHECK(svg.find("<svg") != std::string::npos);
		CHECK(svg.find("<path") != std::string::npos);
		CHECK(svg.find("</svg>") != std::string::npos);
	}

	TEST_CASE("[swf] SVG fill solid color") {
		Dictionary dict = make_test_dict();
		std::string svg = SVGExporter::export_shape(dict, 1);
		CHECK(svg.find("fill=\"rgb(255,0,0)\"") != std::string::npos);
	}

	TEST_CASE("[swf] SVG stroke") {
		Dictionary dict = make_test_dict();
		std::string svg = SVGExporter::export_shape(dict, 1);
		CHECK(svg.find("stroke=\"rgb(0,0,0)\"") != std::string::npos);
	}

	TEST_CASE("[swf] JSON export produces valid JSON") {
		Dictionary dict = make_test_dict();
		std::string json = JSONExporter::export_shape(dict, 1);
		CHECK(json.find("\"bounds\"") != std::string::npos);
		CHECK(json.find("\"shapes\"") != std::string::npos);
		CHECK(json.find("\"commands\"") != std::string::npos);
	}

	TEST_CASE("[swf] JSON structure has expected keys") {
		Dictionary dict = make_test_dict();
		std::string json = JSONExporter::export_shape(dict, 1);
		CHECK(json.find("\"type\": \"M\"") != std::string::npos);
		CHECK(json.find("\"type\": \"L\"") != std::string::npos);
		CHECK(json.find("\"closed\": true") != std::string::npos);
	}

	TEST_CASE("[swf] JSON non-existent character") {
		Dictionary dict = make_test_dict();
		CHECK(JSONExporter::export_shape(dict, 999) == "{}");
	}
}

#endif // DOCTEST
