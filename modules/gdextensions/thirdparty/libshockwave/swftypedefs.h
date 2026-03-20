#ifndef LIBSHOCKWAVE_SWF_TYPEDEFS_H
#define LIBSHOCKWAVE_SWF_TYPEDEFS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <map>

#include "swfaction.h"

namespace SWF
{
	enum ShapeRecordType
	{
		ENDSHAPE,
		STYLECHANGE,
		STRAIGHTEDGE,
		CURVEDEDGE
	};

	enum GradSpreadMode
	{
		PAD,
		REFLECT,
		REPEAT
	};

	enum GradInterpMode
	{
		RGB,
		LINEAR_RGB
	};

	struct RecordHeader
	{
		uint16_t tag : 10;
		uint32_t length;
	};

	struct Rect
	{
		float xmin = 0;
		float xmax = 0;
		float ymin = 0;
		float ymax = 0;
	};

	struct RGBA
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		uint8_t a = 0;
	};

	struct Matrix
	{
		float ScaleX = 1;
		float ScaleY = 1;
		float RotateSkew0 = 0;
		float RotateSkew1 = 0;
		float TranslateX = 0;
		float TranslateY = 0;
	};

	struct CXForm
	{
		float RedMultTerm = 1;
		float GreenMultTerm = 1;
		float BlueMultTerm = 1;
		float AlphaMultTerm = 1;
		int16_t RedAddTerm = 0;
		int16_t GreenAddTerm = 0;
		int16_t BlueAddTerm = 0;
		int16_t AlphaAddTerm = 0;
		bool IsModified() { return !(
			(this->RedAddTerm==0 && this->GreenAddTerm==0 && this->BlueAddTerm==0 && this->AlphaAddTerm==0) &&
			(this->RedMultTerm==1 && this->GreenMultTerm==1 && this->BlueMultTerm==1.0f && this->AlphaMultTerm==1)
			); }
	};

	struct GradRecord
	{
		uint8_t Ratio = 0;
		RGBA Color;
	};
	typedef std::list<GradRecord> GradRecordArray;
	struct Gradient
	{
		uint8_t SpreadMode : 2;
		uint8_t InterpolationMode : 2;
		GradRecordArray GradientRecords;
	};
	struct FocalGradient : public Gradient
	{
		float FocalPoint = 0;
	};



	struct FillStyle
	{
		enum class Type { SOLID, LINEARGRADIENT=0x10, RADIALGRADIENT=0x12, FOCALRADIALGRADIENT=0x13, REPEATINGBITMAP=0x40, CLIPPEDBITMAP=0x41, NONSMOOTHEDREPEATINGBITMAP=0x42, NONSMOOTHEDCLIPPEDBITMAP=0x43 } StyleType;
		RGBA Color;
		Matrix GradientMatrix;
		Gradient Gradient;
		uint16_t BitmapId = 0;
		Matrix BitmapMatrix;
	};
	typedef std::vector<FillStyle> FillStyleArray;
	typedef std::map<uint16_t,FillStyleArray> FillStyleMap;

	struct LineStyle
	{
		enum class Type { LINESTYLE, LINESTYLE2 } StyleType;
		enum class Cap  { ROUND, NONE, SQUARE } StartCapStyle, EndCapStyle;
		enum class Join { ROUND, BEVEL, MITER } JoinStyle;
		float Width = 1;
		RGBA Color;
		bool HasFillFlag : 1;
		bool NoHScaleFlag : 1;
		bool NoVScaleFlag : 1;
		bool PixelHintingFlag : 1;
		bool NoClose : 1;
		float MiterLimitFactor = 1;
		FillStyle FillType;
	};
	typedef std::vector<LineStyle> LineStyleArray;
	typedef std::map<uint16_t,LineStyleArray> LineStyleMap;

	struct StyleChangeRecord
	{
		bool MoveDeltaFlag = false;
		bool FillStyle0Flag = false;
		bool FillStyle1Flag = false;
		bool LineStyleFlag = false;
		bool NewStylesFlag = false;
		float MoveDeltaX = 0.0f;
		float MoveDeltaY = 0.0f;
		uint16_t FillStyle0 = 0;
		uint16_t FillStyle1 = 0;
		uint16_t LineStyle = 0;
		uint8_t NumFillBits : 4;
		uint8_t NumLineBits : 4;
		uint16_t NumNewFillStyles = 0;
		uint16_t NumNewLineStyles = 0;
	};

	struct Point
	{
		float x = 0;
		float y = 0;
		void transform(Matrix m)
		{
			x = ( x*m.ScaleX ) + ( y*m.RotateSkew1 ) + m.TranslateX;
			y = ( x*m.RotateSkew0 ) + ( y*m.ScaleY ) + m.TranslateY;
		}
	};
	struct Vertex
	{
		Point anchor;
		Point control;
	};
	struct Shape
	{
		uint8_t layer = 0;
		uint16_t fill0 = 0;
		uint16_t fill1 = 0;
		uint16_t stroke = 0;
		bool closed = false;
		std::vector<Vertex> vertices;
	};
	typedef std::vector<Shape> ShapeList;
	struct Character
	{
		Rect bounds;
		ShapeList shapes;
		bool is_empty() { return !shapes.size(); }
	};
	struct DisplayChar
	{
		uint16_t id = 0;
		Matrix transform;
		CXForm colourtransform;
		const char *name = nullptr;
		void *clip_actions = nullptr; // ClipActions* (forward ref avoidance)
	};
	typedef std::map<uint16_t,DisplayChar> DisplayList;
    typedef std::list<DisplayList> FrameList;
    struct Sprite {
		uint16_t id;
        uint16_t frame_count;
		FrameList Frames;
	};
	typedef std::map<uint16_t,Character> CharacterDict;
	typedef std::map<uint16_t,Sprite> SpriteDict;

	// Buttons (tags 7, 34)
	struct ButtonRecord
	{
		uint8_t state_flags = 0;
		uint16_t character_id = 0;
		uint16_t depth = 0;
		Matrix matrix;
		CXForm color_transform;
		uint8_t blend_mode = 0;
		bool has_blend_mode = false;
		bool has_filter_list = false;
	};

	struct ButtonCondAction;
	typedef std::vector<struct ActionRecord> ActionList_fwd;

	struct Button
	{
		uint16_t id = 0;
		bool track_as_menu = false;
		std::vector<ButtonRecord> records;
	};
	typedef std::map<uint16_t, Button> ButtonDict;

	// Text (tags 11, 33, 37)
	struct GlyphEntry
	{
		uint32_t index = 0;
		int32_t advance = 0;
	};

	struct TextRecord
	{
		bool has_font = false;
		bool has_color = false;
		bool has_x_offset = false;
		bool has_y_offset = false;
		uint16_t font_id = 0;
		RGBA color;
		int16_t x_offset = 0;
		int16_t y_offset = 0;
		uint16_t text_height = 0;
		std::vector<GlyphEntry> glyphs;
	};

	struct TextDef
	{
		uint16_t id = 0;
		Rect bounds;
		Matrix matrix;
		std::vector<TextRecord> records;
	};
	typedef std::map<uint16_t, TextDef> TextDict;

	struct EditTextDef
	{
		uint16_t id = 0;
		Rect bounds;
		bool has_font = false;
		bool has_max_length = false;
		bool has_text_color = false;
		bool readonly = false;
		bool password = false;
		bool multiline = false;
		bool wordwrap = false;
		bool has_text = false;
		bool use_outlines = false;
		bool html = false;
		bool was_static = false;
		bool border = false;
		bool no_select = false;
		bool has_layout = false;
		bool auto_size = false;
		bool has_font_class = false;
		uint16_t font_id = 0;
		const char *font_class = nullptr;
		uint16_t font_height = 0;
		RGBA text_color;
		uint16_t max_length = 0;
		uint8_t align = 0;
		uint16_t left_margin = 0;
		uint16_t right_margin = 0;
		uint16_t indent = 0;
		int16_t leading = 0;
		const char *variable_name = nullptr;
		const char *initial_text = nullptr;
	};
	typedef std::map<uint16_t, EditTextDef> EditTextDict;

	// Bitmaps (tags 6, 8, 20, 21, 35, 36, 90)
	struct BitmapDef
	{
		uint16_t id = 0;
		uint16_t tag_type = 0;
		uint16_t width = 0;
		uint16_t height = 0;
		uint8_t format = 0;
		const uint8_t *data = nullptr;
		uint32_t data_length = 0;
		const uint8_t *alpha_data = nullptr;
		uint32_t alpha_data_length = 0;
	};
	typedef std::map<uint16_t, BitmapDef> BitmapDict;

	// Fonts (tags 10, 48, 75)
	struct FontDef
	{
		uint16_t id = 0;
		const char *name = nullptr;
		uint8_t flags = 0;
		int16_t ascent = 0;
		int16_t descent = 0;
		int16_t leading = 0;
		uint16_t num_glyphs = 0;
		std::vector<uint16_t> code_table;
	};
	typedef std::map<uint16_t, FontDef> FontDict;

	// Sounds (tags 14, 15)
	struct SoundDef
	{
		uint16_t id = 0;
		uint8_t format = 0;
		uint8_t rate = 0;
		bool is_16bit = false;
		bool is_stereo = false;
		uint32_t sample_count = 0;
		const uint8_t *data = nullptr;
		uint32_t data_length = 0;
	};
	typedef std::map<uint16_t, SoundDef> SoundDict;

	// MorphShapes (tags 46, 84)
	struct MorphShapeDef
	{
		uint16_t id = 0;
		Rect start_bounds;
		Rect end_bounds;
	};
	typedef std::map<uint16_t, MorphShapeDef> MorphShapeDict;

	// Export/Import assets (tags 56, 57)
	struct AssetEntry
	{
		uint16_t id = 0;
		const char *name = nullptr;
	};
	typedef std::vector<AssetEntry> AssetList;

	// Frame labels
	typedef std::map<std::string, uint16_t> FrameLabelMap;

	struct Dictionary
	{
		FillStyleMap FillStyles;
		LineStyleMap LineStyles;
        SpriteDict SpriteList;
		CharacterDict CharacterList;
		FrameList Frames;

		uint8_t NumFillBits : 4;
		uint8_t NumLineBits : 4;
		uint16_t NewCharOffset = 0;

		// Phase 1: ActionScript
		ActionBlockList Actions;

		// Phase 2 additions
		ButtonDict Buttons;
		TextDict Texts;
		EditTextDict EditTexts;
		BitmapDict Bitmaps;
		FontDict Fonts;
		SoundDict Sounds;
		MorphShapeDict MorphShapes;
		AssetList ExportedAssets;
		AssetList ImportedAssets;
		FrameLabelMap FrameLabels;
		const uint8_t *jpeg_tables = nullptr;
		uint32_t jpeg_tables_length = 0;
	};

	struct Properties
	{
		uint8_t version;
		Rect dimensions;
		RGBA bgcolour;
		float framerate;
		uint16_t framecount;
	};

}

#endif	// LIBSHOCKWAVE_SWF_TYPEDEFS_H
