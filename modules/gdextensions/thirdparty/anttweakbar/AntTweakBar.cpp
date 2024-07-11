#include "AntTweakBar.h"

#include "common/gd_core.h"
#include "core/math/math_funcs.h"
#include "core/os/input_event.h"
#include "core/os/keyboard.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "scene/2d/canvas_item.h"

#ifdef __APPLE__
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#define _MACOSX
#define __PLACEMENT_NEW_INLINE
#endif

#ifdef __linux__
#define _UNIX
#define __PLACEMENT_NEW_INLINE
#endif

#if defined(WIN32) || defined(_WIN32)
#define _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#endif // _DEBUG
#else
#define _snprintf snprintf
#define _stricmp strcasecmp
#define _strdup strdup
#endif

#define TW_EXPORTS
#define ANT_CALL TW_CALL
// uncomment to activate benchmarks
// #define BENCH

#define ANT_TEXEL_ALIGNEMENT 0 // 0.5 for D3D
#define ANT_PIXEL_OFFSET 1 // 0 for D3D

//  ---------------------------------------------------------------------------
//  @file       TwPrecomp.h
//  @brief      Precompiled header
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

#if !defined ANT_TW_PRECOMP_INCLUDED
#define ANT_TW_PRECOMP_INCLUDED

#if defined _MSC_VER
#pragma warning(disable : 4514) // unreferenced inline function has been removed
#pragma warning(disable : 4710) // function not inlined
#pragma warning(disable : 4786) // template name truncated
#pragma warning(disable : 4530) // exceptions not handled
#define _CRT_SECURE_NO_DEPRECATE // visual 8 secure crt warning
#endif

#include <memory.h>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(push, 3)
#endif
#include <functional>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(pop)
#endif

#endif // !defined ANT_TW_PRECOMP_INCLUDED

//  ---------------------------------------------------------------------------
//
//  @file       AntPerfTimer.h
//  @brief      A performance (precision) timer for benchs
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//
//  note:       No cpp file is needed, everything is defined in this header
//
//  ---------------------------------------------------------------------------

#if !defined ANT_PERF_TIMER_INCLUDED
#define ANT_PERF_TIMER_INCLUDED

#if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)

#include <tchar.h>
#include <windows.h>

struct PerfTimer {
	_FORCE_INLINE_ PerfTimer() {
		if (!QueryPerformanceFrequency(&Freq))
			MessageBox(NULL, _T("Precision timer not supported"), _T("Problem"), MB_ICONEXCLAMATION);
		Reset();
	}
	_FORCE_INLINE_ void Reset() { QueryPerformanceCounter(&Start); }
	_FORCE_INLINE_ double GetTime() {
		if (QueryPerformanceCounter(&End))
			return ((double)End.QuadPart - (double)Start.QuadPart) / ((double)Freq.QuadPart);
		else
			return 0;
	}

protected:
	LARGE_INTEGER Start, End, Freq;
};

#else // !_WIN (-> LINUX)

#include <sys/time.h>
#include <unistd.h>

struct PerfTimer {
	_FORCE_INLINE_ PerfTimer() { Reset(); }
	_FORCE_INLINE_ void Reset() { gettimeofday(&Start, &TZ); }
	_FORCE_INLINE_ double GetTime() {
		gettimeofday(&End, &TZ);
		double t1 = (double)Start.tv_sec + (double)Start.tv_usec / (1000 * 1000);
		double t2 = (double)End.tv_sec + (double)End.tv_usec / (1000 * 1000);
		return t2 - t1;
	}

protected:
	struct timeval Start, End;
	struct timezone TZ;
};

#endif // _WIN

#endif // ANT_PERF_TIMER_INCLUDED

//  ---------------------------------------------------------------------------
//  @file       TwColors.h
//  @brief      Color conversions
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

#if !defined ANT_TW_COLORS_INCLUDED
#define ANT_TW_COLORS_INCLUDED

//  ---------------------------------------------------------------------------

typedef unsigned int color32;

const color32 COLOR32_BLACK = 0xff000000; // Black
const color32 COLOR32_WHITE = 0xffffffff; // White
const color32 COLOR32_ZERO = 0x00000000; // Zero
const color32 COLOR32_RED = 0xffff0000; // Red
const color32 COLOR32_GREEN = 0xff00ff00; // Green
const color32 COLOR32_BLUE = 0xff0000ff; // Blue

template <typename _T>
_FORCE_INLINE_ const _T &TClamp(const _T &_X, const _T &_Limit1, const _T &_Limit2) {
	if (_Limit1 < _Limit2)
		return (_X <= _Limit1) ? _Limit1 : ((_X >= _Limit2) ? _Limit2 : _X);
	else
		return (_X <= _Limit2) ? _Limit2 : ((_X >= _Limit1) ? _Limit1 : _X);
}

_FORCE_INLINE_ color32 Color32FromARGBi(int _A, int _R, int _G, int _B) {
	return (((color32)TClamp(_A, 0, 255)) << 24) | (((color32)TClamp(_R, 0, 255)) << 16) | (((color32)TClamp(_G, 0, 255)) << 8) | ((color32)TClamp(_B, 0, 255));
}

_FORCE_INLINE_ color32 Color32FromARGBf(float _A, float _R, float _G, float _B) {
	return (((color32)TClamp(_A * 256.0f, 0.0f, 255.0f)) << 24) | (((color32)TClamp(_R * 256.0f, 0.0f, 255.0f)) << 16) | (((color32)TClamp(_G * 256.0f, 0.0f, 255.0f)) << 8) | ((color32)TClamp(_B * 256.0f, 0.0f, 255.0f));
}

_FORCE_INLINE_ void Color32ToARGBi(color32 _Color, int *_A, int *_R, int *_G, int *_B) {
	if (_A)
		*_A = (_Color >> 24) & 0xff;
	if (_R)
		*_R = (_Color >> 16) & 0xff;
	if (_G)
		*_G = (_Color >> 8) & 0xff;
	if (_B)
		*_B = _Color & 0xff;
}

_FORCE_INLINE_ void Color32ToARGBf(color32 _Color, float *_A, float *_R, float *_G, float *_B) {
	if (_A)
		*_A = (1.0 / 255.0) * float((_Color >> 24) & 0xff);
	if (_R)
		*_R = (1.0 / 255.0) * float((_Color >> 16) & 0xff);
	if (_G)
		*_G = (1.0 / 255.0) * float((_Color >> 8) & 0xff);
	if (_B)
		*_B = (1.0 / 255.0) * float(_Color & 0xff);
}

void ColorRGBToHLSf(float _R, float _G, float _B, float *_Hue, float *_Light, float *_Saturation);
void ColorRGBToHLSi(int _R, int _G, int _B, int *_Hue, int *_Light, int *_Saturation);
void ColorHLSToRGBf(float _Hue, float _Light, float _Saturation, float *_R, float *_G, float *_B);
void ColorHLSToRGBi(int _Hue, int _Light, int _Saturation, int *_R, int *_G, int *_B);
color32 ColorBlend(color32 _Color1, color32 _Color2, float _S);

//  ---------------------------------------------------------------------------

#endif // !defined ANT_TW_COLORS_INCLUDED

//  ---------------------------------------------------------------------------
//  @file       TwFonts.h
//  @brief      Bitmaps fonts
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

#if !defined ANT_TW_FONTS_INCLUDED
#define ANT_TW_FONTS_INCLUDED

// A source bitmap includes 224 characters starting from ascii char 32 (i.e. space)
// to ascii char 255 (extended ASCII Latin1/CP1252):
//
// !"#$%&'()*+,-./0123456789:;<=>?
// @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
// `abcdefghijklmnopqrstuvwxyz{|}~
// ÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóòôöõúùûü
// †°¢£§•¶ß®©™´¨≠ÆØ∞±≤≥¥µ∂∑∏π∫ªºΩæø
// ¿¡¬√ƒ≈∆«»… ÀÃÕŒœ–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ
// ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˙˚¸˝˛ˇ
//
// First pixel column of a source bitmap is a delimiter with color=zero at the end of each line of characters.
// Last pixel row of a line of characters is a delimiter with color=zero at the last pixel of each character.

struct CTexFont {
	unsigned char *m_TexBytes;
	int m_TexWidth; // power of 2
	int m_TexHeight; // power of 2
	float m_CharU0[256];
	float m_CharV0[256];
	float m_CharU1[256];
	float m_CharV1[256];
	int m_CharWidth[256];
	int m_CharHeight;
	int m_NbCharRead;

	CTexFont();
	~CTexFont();
};

CTexFont *TwGenerateFont(const unsigned char *_Bitmap, int _BmWidth, int _BmHeight, float _Scaling = 1);

extern CTexFont *g_DefaultSmallFont;
extern CTexFont *g_DefaultNormalFont;
extern CTexFont *g_DefaultLargeFont;
extern CTexFont *g_DefaultFixed1Font;

void TwGenerateDefaultFonts(float _Scaling = 1);
void TwDeleteDefaultFonts();

#endif // !defined ANT_TW_FONTS_INCLUDED

//  ---------------------------------------------------------------------------
//  @file       TwGraph.h
//  @brief      ITwGraph pure interface
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

#if !defined ANT_TW_GRAPH_INCLUDED
#define ANT_TW_GRAPH_INCLUDED

//  ---------------------------------------------------------------------------

class ITwGraph {
public:
	virtual int Init() = 0;
	virtual int Shut() = 0;
	virtual void BeginDraw(int _WndWidth, int _WndHeight) = 0;
	virtual void EndDraw() = 0;
	virtual bool IsDrawing() = 0;
	virtual void Restore() = 0;

	virtual void DrawLine(int _X0, int _Y0, int _X1, int _Y1, color32 _Color0, color32 _Color1, bool _AntiAliased = false) = 0;
	virtual void DrawLine(int _X0, int _Y0, int _X1, int _Y1, color32 _Color, bool _AntiAliased = false) = 0;
	virtual void DrawRect(int _X0, int _Y0, int _X1, int _Y1, color32 _Color00, color32 _Color10, color32 _Color01, color32 _Color11) = 0;
	virtual void DrawRect(int _X0, int _Y0, int _X1, int _Y1, color32 _Color) = 0;
	enum Cull {
		CULL_NONE,
		CULL_CW,
		CULL_CCW
	};
	virtual void DrawTriangles(int _NumTriangles, int *_Vertices, color32 *_Colors, Cull _CullMode) = 0;

	virtual void *NewTextObj() = 0;
	virtual void DeleteTextObj(void *_TextObj) = 0;
	virtual void BuildText(void *_TextObj, const std::string *_TextLines, color32 *_LineColors, color32 *_LineBgColors, int _NbLines, const CTexFont *_Font, int _Sep, int _BgWidth) = 0;
	virtual void DrawText(void *_TextObj, int _X, int _Y, color32 _Color, color32 _BgColor) = 0;

	virtual void ChangeViewport(int _X0, int _Y0, int _Width, int _Height, int _OffsetX, int _OffsetY) = 0;
	virtual void RestoreViewport() = 0;
	virtual void SetScissor(int _X0, int _Y0, int _Width, int _Height) = 0;

	virtual ~ITwGraph() {} // required by gcc
};

ITwGraph *TwCreateRenderer(void *_Device);

//  ---------------------------------------------------------------------------

#endif // ANT_TW_GRAPH_INCLUDED

//  ---------------------------------------------------------------------------
//  @file       TwMgr.h
//  @brief      Tweak bar manager.
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

#if !defined ANT_TW_MGR_INCLUDED
#define ANT_TW_MGR_INCLUDED

#ifdef BENCH
#define PERF(cmd) cmd
#else // BENCH
#define PERF(cmd)
#endif // BENCH

const int NB_ROTO_CURSORS = 12;

//  ---------------------------------------------------------------------------
//  API unexposed by AntTweakBar.h
//  ---------------------------------------------------------------------------

// bar states -> use TwDefine instead
typedef enum ETwState {
	TW_STATE_SHOWN = 1,
	TW_STATE_ICONIFIED = 2,
	TW_STATE_HIDDEN = 3,
	TW_STATE_UNICONIFIED = 4,
	TW_STATE_ERROR = 0
} TwState;

int ANT_CALL TwSetBarState(TwBar *bar, TwState state);

struct CTwVarGroup;
typedef void(ANT_CALL *TwStructExtInitCallback)(void *structExtValue, void *clientData);
typedef void(ANT_CALL *TwCopyVarFromExtCallback)(void *structValue, const void *structExtValue, unsigned int structExtMemberIndex, void *clientData);
typedef void(ANT_CALL *TwCopyVarToExtCallback)(const void *structValue, void *structExtValue, unsigned int structExtMemberIndex, void *clientData);
TwType ANT_CALL TwDefineStructExt(const char *name, const TwStructMember *structExtMembers, unsigned int nbExtMembers, size_t structSize, size_t structExtSize, TwStructExtInitCallback structExtInitCallback, TwCopyVarFromExtCallback copyVarFromExtCallback, TwCopyVarToExtCallback copyVarToExtCallback, TwSummaryCallback summaryCallback, void *clientData, const char *help);
typedef void(ANT_CALL *TwCustomDrawCallback)(int w, int h, void *structExtValue, void *clientData, TwBar *bar, CTwVarGroup *varGrp);
typedef bool(ANT_CALL *TwCustomMouseMotionCallback)(int mouseX, int mouseY, int w, int h, void *structExtValue, void *clientData, TwBar *bar, CTwVarGroup *varGrp);
typedef bool(ANT_CALL *TwCustomMouseButtonCallback)(TwMouseButtonID button, bool pressed, int mouseX, int mouseY, int w, int h, void *structExtValue, void *clientData, TwBar *bar, CTwVarGroup *varGrp);
typedef void(ANT_CALL *TwCustomMouseLeaveCallback)(void *structExtValue, void *clientData, TwBar *bar);

enum ERetType {
	RET_ERROR = 0,
	RET_DOUBLE,
	RET_STRING
};

enum EButtonAlign {
	BUTTON_ALIGN_LEFT,
	BUTTON_ALIGN_CENTER,
	BUTTON_ALIGN_RIGHT
};

//  ---------------------------------------------------------------------------
//  AntTweakBar Manager
//  ---------------------------------------------------------------------------

struct CTwMgr {
	void *m_Device;
	int m_WndID;
	class ITwGraph *m_Graph;
	int m_WndWidth;
	int m_WndHeight;
	const CTexFont *m_CurrentFont;

	std::vector<TwBar *> m_Bars;
	std::vector<int> m_Order;

	std::vector<bool> m_MinOccupied;
	void Minimize(TwBar *_Bar);
	void Maximize(TwBar *_Bar);
	void Hide(TwBar *_Bar);
	void Unhide(TwBar *_Bar);
	void SetFont(const CTexFont *_Font, bool _ResizeBars);
	int m_LastMouseX;
	int m_LastMouseY;
	int m_LastMouseWheelPos;
	int m_IconPos; // 0: bottom-left, 1:bottom-right, 2:top-left, 3:top-right
	int m_IconAlign; // 0: vertical, 1: horizontal
	int m_IconMarginX, m_IconMarginY;
	bool m_FontResizable;
	std::string m_BarAlwaysOnTop;
	std::string m_BarAlwaysOnBottom;
	bool m_UseOldColorScheme;
	bool m_Contained;
	EButtonAlign m_ButtonAlign;
	bool m_OverlapContent;
	bool m_Terminating;

	std::string m_Help;
	TwBar *m_HelpBar;
	float m_LastHelpUpdateTime;
	void UpdateHelpBar();
	bool m_HelpBarNotUpToDate;
	bool m_HelpBarUpdateNow;
	void *m_KeyPressedTextObj;
	bool m_KeyPressedBuildText;
	std::string m_KeyPressedStr;
	float m_KeyPressedTime;
	void *m_InfoTextObj;
	bool m_InfoBuildText;
	int m_BarInitColorHue;
	int FindBar(const char *_Name) const;
	int HasAttrib(const char *_Attrib, bool *_HasValue) const;
	int SetAttrib(int _AttribID, const char *_Value);
	ERetType GetAttrib(int _AttribID, std::vector<double> &outDouble, std::ostringstream &outString) const;
	void SetLastError(const char *_StaticErrorMesssage); // _StaticErrorMesssage must be a static string
	const char *GetLastError(); // returns a static string describing the error, and set LastError to NULL
	const char *CheckLastError() const; // returns the LastError, but does not set it to NULL
	void SetCurrentDbgParams(const char *file, int line);
	TwBar *m_PopupBar;

	CTwMgr(void *_Device, int _WndID);
	~CTwMgr();

	struct CStructMember {
		std::string m_Name;
		std::string m_Label;
		TwType m_Type;
		size_t m_Offset;
		std::string m_DefString;
		size_t m_Size;
		std::string m_Help;
	};
	struct CStruct {
		std::string m_Name;
		std::vector<CStructMember> m_Members;
		size_t m_Size;
		TwSummaryCallback m_SummaryCallback;
		void *m_SummaryClientData;
		std::string m_Help;
		bool m_IsExt;
		size_t m_ClientStructSize;
		TwStructExtInitCallback m_StructExtInitCallback;
		TwCopyVarFromExtCallback m_CopyVarFromExtCallback;
		TwCopyVarToExtCallback m_CopyVarToExtCallback;
		void *m_ExtClientData;
		CStruct() :
				m_IsExt(false), m_StructExtInitCallback(nullptr), m_CopyVarFromExtCallback(nullptr), m_CopyVarToExtCallback(nullptr), m_ExtClientData(nullptr) {}
		static void ANT_CALL DefaultSummary(char *_SummaryString, size_t _SummaryMaxLength, const void *_Value, void *_ClientData);
		static void *s_PassProxyAsClientData;
	};
	std::vector<CStruct> m_Structs;

	// followings are used for TwAddVarCB( ... StructType ... )
	struct CStructProxy {
		TwType m_Type;
		void *m_StructData;
		bool m_DeleteStructData;
		void *m_StructExtData;
		TwSetVarCallback m_StructSetCallback;
		TwGetVarCallback m_StructGetCallback;
		void *m_StructClientData;
		TwCustomDrawCallback m_CustomDrawCallback;
		TwCustomMouseMotionCallback m_CustomMouseMotionCallback;
		TwCustomMouseButtonCallback m_CustomMouseButtonCallback;
		TwCustomMouseLeaveCallback m_CustomMouseLeaveCallback;
		bool m_CustomCaptureFocus;
		int m_CustomIndexFirst;
		int m_CustomIndexLast;
		CStructProxy();
		~CStructProxy();
	};
	struct CMemberProxy {
		CStructProxy *m_StructProxy;
		int m_MemberIndex;
		struct CTwVar *m_Var;
		struct CTwVarGroup *m_VarParent;
		CTwBar *m_Bar;
		CMemberProxy();
		~CMemberProxy();
		static void ANT_CALL SetCB(const void *_Value, void *_ClientData);
		static void ANT_CALL GetCB(void *_Value, void *_ClientData);
	};
	std::list<CStructProxy> m_StructProxies; // elements should not move
	std::list<CMemberProxy> m_MemberProxies; // elements should not move

	struct CEnum {
		std::string m_Name;
		typedef std::map<unsigned int, std::string> CEntries;
		CEntries m_Entries;
	};
	std::vector<CEnum> m_Enums;

	TwType m_TypeColor32;
	TwType m_TypeColor3F;
	TwType m_TypeColor4F;
	TwType m_TypeQuat4F;
	TwType m_TypeQuat4D;
	TwType m_TypeDir3F;
	TwType m_TypeDir3D;

	std::vector<char> m_CSStringBuffer;
	struct CCDStdString {
		std::string *m_ClientStdStringPtr;
		char m_LocalString[sizeof(std::string) + 2 * sizeof(void *)]; //+2*sizeof(void*) because of VC++ std::string extra info in Debug
		TwSetVarCallback m_ClientSetCallback;
		TwGetVarCallback m_ClientGetCallback;
		void *m_ClientData;
		static void ANT_CALL SetCB(const void *_Value, void *_ClientData);
		static void ANT_CALL GetCB(void *_Value, void *_ClientData);
	};
	std::list<CCDStdString> m_CDStdStrings;
	struct CClientStdString // Convertion between VC++ Debug/Release std::string
	{
		CClientStdString();
		void FromLib(const char *libStr);
		std::string &ToClient();

	private:
		char m_Data[sizeof(std::string) + 2 * sizeof(void *)];
		std::string m_LibStr;
	};
	struct CLibStdString // Convertion between VC++ Debug/Release std::string
	{
		CLibStdString();
		void FromClient(const std::string &clientStr);
		std::string &ToLib();

	private:
		char m_Data[sizeof(std::string) + 2 * sizeof(void *)];
	};
	struct CCDStdStringRecord {
		void *m_DataPtr;
		char m_PrevValue[sizeof(std::string) + 2 * sizeof(void *)];
		CClientStdString m_ClientStdString;
	};
	std::vector<CCDStdStringRecord> m_CDStdStringRecords;
	void UnrollCDStdString(std::vector<CCDStdStringRecord> &_Records, TwType _Type, void *_Data);
	void RestoreCDStdString(const std::vector<CCDStdStringRecord> &_Records);
	std::map<void *, std::vector<char>> m_CDStdStringCopyBuffers;

	struct CCustom // custom var type
	{
		virtual ~CCustom() = 0;
	};
	std::vector<CCustom *> m_Customs;

	PerfTimer m_Timer;
	double m_LastMousePressedTime;
	TwMouseButtonID m_LastMousePressedButtonID;
	int m_LastMousePressedPosition[2];
	double m_RepeatMousePressedDelay;
	double m_RepeatMousePressedPeriod;
	bool m_CanRepeatMousePressed;
	bool m_IsRepeatingMousePressed;
	double m_LastDrawTime;

	enum CCursor {
		CursorArrow,
		CursorMove,
		CursorWE,
		CursorNS,
		CursorTopLeft,
		CursorTopRight,
		CursorBottomLeft,
		CursorBottomRight,
		CursorHelp,
		CursorHand,
		CursorCross,
		CursorUpArrow,
		CursorNo,
		CursorIBeam,
		RotoCursors0,
		RotoCursors1,
		RotoCursors2,
		RotoCursors3,
		RotoCursors4,
		RotoCursors5,
		RotoCursors6,
		RotoCursors7,
		RotoCursors8,
		RotoCursors9,
		RotoCursors10,
		RotoCursors11,
		CursorCenter,
		CursorPoint,
	};

	static constexpr CCursor s_RotoCursors[NB_ROTO_CURSORS] = { RotoCursors0, RotoCursors1, RotoCursors2, RotoCursors3, RotoCursors4, RotoCursors5, RotoCursors6, RotoCursors7, RotoCursors8, RotoCursors9, RotoCursors10, RotoCursors11 };

	void SetCursor(CCursor _Cursor);

	TwCopyCDStringToClient m_CopyCDStringToClient;
	TwCopyStdStringToClient m_CopyStdStringToClient;
	size_t m_ClientStdStringStructSize;
	TwType m_ClientStdStringBaseType;

protected:
	int m_NbMinimizedBars;
	const char *m_LastError;
	const char *m_CurrentDbgFile;
	int m_CurrentDbgLine;
};

extern CTwMgr *g_TwMgr;

//  ---------------------------------------------------------------------------
//  Extra functions and TwTypes
//  ---------------------------------------------------------------------------

bool TwGetKeyCode(int *_Code, int *_Modif, const char *_String);
bool TwGetKeyString(std::string *_String, int _Code, int _Modif);

const TwType TW_TYPE_SHORTCUT = TwType(0xfff1);
const TwType TW_TYPE_HELP_GRP = TwType(0xfff2);
const TwType TW_TYPE_HELP_ATOM = TwType(0xfff3);
const TwType TW_TYPE_HELP_HEADER = TwType(0xfff4);
const TwType TW_TYPE_HELP_STRUCT = TwType(0xfff5);
const TwType TW_TYPE_BUTTON = TwType(0xfff6);
const TwType TW_TYPE_CDSTDSTRING = TwType(0xfff7);
const TwType TW_TYPE_STRUCT_BASE = TwType(0x10000000);
const TwType TW_TYPE_ENUM_BASE = TwType(0x20000000);
const TwType TW_TYPE_CSSTRING_BASE = TW_TYPE_CSSTRING(0); // defined as 0x30000000 (see AntTweakBar.h)
const TwType TW_TYPE_CSSTRING_MAX = TW_TYPE_CSSTRING(0xfffffff);
#define TW_CSSTRING_SIZE(type) ((int)((type) & 0xfffffff))
const TwType TW_TYPE_CUSTOM_BASE = TwType(0x40000000);
const TwType TW_TYPE_STDSTRING_VS2008 = TwType(0x2fff0000);
const TwType TW_TYPE_STDSTRING_VS2010 = TwType(0x2ffe0000);

extern "C" int ANT_CALL TwSetLastError(const char *_StaticErrorMessage);

// Clipping helper
struct CRect {
	int X, Y, W, H;
	CRect() :
			X(0), Y(0), W(0), H(0) {}
	CRect(int _X, int _Y, int _W, int _H) :
			X(_X), Y(_Y), W(_W), H(_H) {}
	bool operator==(const CRect &_Rect) { return (Empty() && _Rect.Empty()) || (X == _Rect.X && Y == _Rect.Y && W == _Rect.W && H == _Rect.H); }
	bool Empty(int _Margin = 0) const { return (W <= _Margin || H <= _Margin); }
	bool Subtract(const CRect &_Rect, std::vector<CRect> &_OutRects) const;
	bool Subtract(const std::vector<CRect> &_Rects, std::vector<CRect> &_OutRects) const;
};

//  ---------------------------------------------------------------------------
//  Global bar attribs
//  ---------------------------------------------------------------------------

enum EMgrAttribs {
	MGR_HELP = 1,
	MGR_FONT_SIZE,
	MGR_FONT_STYLE,
	MGR_ICON_POS,
	MGR_ICON_ALIGN,
	MGR_ICON_MARGIN,
	MGR_FONT_RESIZABLE,
	MGR_COLOR_SCHEME,
	MGR_CONTAINED,
	MGR_BUTTON_ALIGN,
	MGR_OVERLAP
};

//  ---------------------------------------------------------------------------
//  Color struct ext
//  ---------------------------------------------------------------------------

struct CColorExt {
	int R, G, B;
	int H, L, S;
	int A;
	bool m_HLS, m_HasAlpha, m_OGL;
	bool m_CanHaveAlpha;
	bool m_IsColorF;
	unsigned int m_PrevConvertedColor;
	CTwMgr::CStructProxy *m_StructProxy;
	void RGB2HLS();
	void HLS2RGB();
	static void ANT_CALL InitColor32CB(void *_ExtValue, void *_ClientData);
	static void ANT_CALL InitColor3FCB(void *_ExtValue, void *_ClientData);
	static void ANT_CALL InitColor4FCB(void *_ExtValue, void *_ClientData);
	static void ANT_CALL CopyVarFromExtCB(void *_VarValue, const void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData);
	static void ANT_CALL CopyVarToExtCB(const void *_VarValue, void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData);
	static void ANT_CALL SummaryCB(char *_SummaryString, size_t _SummaryMaxLength, const void *_ExtValue, void *_ClientData);
	static void CreateTypes();
};

//  ---------------------------------------------------------------------------
//  Quaternion struct ext
//  ---------------------------------------------------------------------------

struct CQuaternionExt {
	double Qx, Qy, Qz, Qs; // Quat value
	double Vx, Vy, Vz, Angle; // Not used
	double Dx, Dy, Dz; // Dir value set when used as a direction
	bool m_AAMode; // Axis & angle mode -> disabled
	bool m_ShowVal; // Display values
	bool m_IsFloat; // Quat/Dir uses floats
	bool m_IsDir; // Mapped to a dir vector instead of a quat
	double m_Dir[3]; // If not zero, display one direction vector
	color32 m_DirColor; // Direction vector color
	float m_Permute[3][3]; // Permute frame axis
	CTwMgr::CStructProxy *m_StructProxy;
	static void ANT_CALL InitQuat4FCB(void *_ExtValue, void *_ClientData);
	static void ANT_CALL InitQuat4DCB(void *_ExtValue, void *_ClientData);
	static void ANT_CALL InitDir3FCB(void *_ExtValue, void *_ClientData);
	static void ANT_CALL InitDir3DCB(void *_ExtValue, void *_ClientData);
	static void ANT_CALL CopyVarFromExtCB(void *_VarValue, const void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData);
	static void ANT_CALL CopyVarToExtCB(const void *_VarValue, void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData);
	static void ANT_CALL SummaryCB(char *_SummaryString, size_t _SummaryMaxLength, const void *_ExtValue, void *_ClientData);
	static void ANT_CALL DrawCB(int _W, int _H, void *_ExtValue, void *_ClientData, TwBar *_Bar, CTwVarGroup *varGrp);
	static bool ANT_CALL MouseMotionCB(int _MouseX, int _MouseY, int _W, int _H, void *_StructExtValue, void *_ClientData, TwBar *_Bar, CTwVarGroup *varGrp);
	static bool ANT_CALL MouseButtonCB(TwMouseButtonID _Button, bool _Pressed, int _MouseX, int _MouseY, int _W, int _H, void *_StructExtValue, void *_ClientData, TwBar *_Bar, CTwVarGroup *varGrp);
	static void ANT_CALL MouseLeaveCB(void *_StructExtValue, void *_ClientData, TwBar *_Bar);
	static void CreateTypes();
	static TwType s_CustomType;
	void ConvertToAxisAngle();
	void ConvertFromAxisAngle();
	void CopyToVar();
	static std::vector<float> s_SphTri;
	static std::vector<color32> s_SphCol;
	static std::vector<int> s_SphTriProj;
	static std::vector<color32> s_SphColLight;
	static std::vector<float> s_ArrowTri[4];
	static std::vector<int> s_ArrowTriProj[4];
	static std::vector<float> s_ArrowNorm[4];
	static std::vector<color32> s_ArrowColLight[4];
	enum EArrowParts { ARROW_CONE,
		ARROW_CONE_CAP,
		ARROW_CYL,
		ARROW_CYL_CAP };
	static void CreateSphere();
	static void CreateArrow();
	static void ApplyQuat(float *outX, float *outY, float *outZ, float x, float y, float z, float qx, float qy, float qz, float qs);
	static void QuatFromDir(double *outQx, double *outQy, double *outQz, double *outQs, double dx, double dy, double dz);
	_FORCE_INLINE_ void Permute(float *outX, float *outY, float *outZ, float x, float y, float z);
	_FORCE_INLINE_ void PermuteInv(float *outX, float *outY, float *outZ, float x, float y, float z);
	_FORCE_INLINE_ void Permute(double *outX, double *outY, double *outZ, double x, double y, double z);
	_FORCE_INLINE_ void PermuteInv(double *outX, double *outY, double *outZ, double x, double y, double z);
	bool m_Highlighted;
	bool m_Rotating;
	double m_OrigQuat[4];
	float m_OrigX, m_OrigY;
	double m_PrevX, m_PrevY;
};

//  ---------------------------------------------------------------------------
//  CTwFPU objects set and restore the fpu precision if needed.
//  (could be useful because DirectX changes it and AntTweakBar requires default double precision)
//  ---------------------------------------------------------------------------

struct CTwFPU {
	CTwFPU() {
#ifdef _WINDOWS
		state0 = _controlfp(0, 0);
		if ((state0 & MCW_PC) == _PC_24) // we need at least _PC_53
			_controlfp(_PC_53, MCW_PC);
#else
		state0 = 0;
#endif
	}
	~CTwFPU() {
#ifdef _WINDOWS
		if ((state0 & MCW_PC) == _PC_24)
			_controlfp(_PC_24, MCW_PC);
#else
		state0 = 0;
#endif
	}

private:
	unsigned int state0;
};

//  ---------------------------------------------------------------------------

#endif // !defined ANT_TW_MGR_INCLUDED

//  ---------------------------------------------------------------------------
//  @file       TwBar.h
//  @brief      Tweak bar and var classes.
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

#if !defined ANT_TW_BAR_INCLUDED
#define ANT_TW_BAR_INCLUDED

//  ---------------------------------------------------------------------------

bool IsCustomType(int _Type);

struct CTwVar {
	std::string m_Name;
	std::string m_Label;
	std::string m_Help;
	bool m_IsRoot;
	bool m_DontClip;
	bool m_Visible;
	signed short m_LeftMargin;
	signed short m_TopMargin;
	const color32 *m_ColorPtr;
	const color32 *m_BgColorPtr;

	virtual bool IsGroup() const = 0;
	virtual bool IsCustom() const { return false; }
	virtual const CTwVar *Find(const char *_Name, struct CTwVarGroup **_Parent, int *_Index) const = 0;
	virtual int HasAttrib(const char *_Attrib, bool *_HasValue) const;
	virtual int SetAttrib(int _AttribID, const char *_Value, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex);
	virtual ERetType GetAttrib(int _AttribID, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex, std::vector<double> &outDouble, std::ostringstream &outString) const;
	virtual void SetReadOnly(bool _ReadOnly) = 0;
	virtual bool IsReadOnly() const = 0;
	CTwVar();
	virtual ~CTwVar() {}

	static size_t GetDataSize(TwType _Type);
};

struct CTwVarAtom : CTwVar {
	ETwType m_Type;
	void *m_Ptr;
	TwSetVarCallback m_SetCallback;
	TwGetVarCallback m_GetCallback;
	void *m_ClientData;
	bool m_ReadOnly;
	bool m_NoSlider;
	int m_KeyIncr[2]; // [0]=key_code [1]=modifiers
	int m_KeyDecr[2]; // [0]=key_code [1]=modifiers

	template <typename _T>
	struct TVal {
		_T m_Min;
		_T m_Max;
		_T m_Step;
		signed char m_Precision;
		bool m_Hexa;
	};
	union UVal {
		TVal<unsigned char> m_Char;
		TVal<signed char> m_Int8;
		TVal<unsigned char> m_UInt8;
		TVal<signed short> m_Int16;
		TVal<unsigned short> m_UInt16;
		TVal<signed int> m_Int32;
		TVal<unsigned int> m_UInt32;
		TVal<float> m_Float32;
		TVal<double> m_Float64;
		struct CBoolVal {
			char *m_TrueString;
			char *m_FalseString;
			bool m_FreeTrueString;
			bool m_FreeFalseString;
		} m_Bool;
		struct CEnumVal // empty -> enum entries are deduced from m_Type
		{
			// typedef std::map<unsigned int, std::string> CEntries;
			// CEntries *    m_Entries;
		} m_Enum;
		struct CShortcutVal {
			int m_Incr[2];
			int m_Decr[2];
		} m_Shortcut;
		struct CHelpStruct {
			int m_StructType;
		} m_HelpStruct;
		struct CButtonVal {
			TwButtonCallback m_Callback;
			int m_Separator;
		} m_Button;
		struct CCustomVal {
			CTwMgr::CMemberProxy *m_MemberProxy;
		} m_Custom;
	};
	UVal m_Val;

	virtual bool IsGroup() const { return false; }
	virtual bool IsCustom() const { return IsCustomType(m_Type); }
	virtual void ValueToString(std::string *_Str) const;
	virtual double ValueToDouble() const;
	virtual void ValueFromDouble(double _Val);
	virtual void MinMaxStepToDouble(double *_Min, double *_Max, double *_Step) const;
	virtual const CTwVar *Find(const char *_Name, struct CTwVarGroup **_Parent, int *_Index) const;
	virtual int HasAttrib(const char *_Attrib, bool *_HasValue) const;
	virtual int SetAttrib(int _AttribID, const char *_Value, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex);
	virtual ERetType GetAttrib(int _AttribID, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex, std::vector<double> &outDouble, std::ostringstream &outString) const;
	virtual void Increment(int _Step);
	virtual void SetDefaults();
	virtual void SetReadOnly(bool _ReadOnly) {
		m_ReadOnly = _ReadOnly;
		if (m_Type != TW_TYPE_BUTTON && m_SetCallback == NULL && m_Ptr == NULL)
			m_ReadOnly = true;
	}
	virtual bool IsReadOnly() const {
		if (m_Type != TW_TYPE_BUTTON && m_SetCallback == NULL && m_Ptr == NULL)
			return true;
		else
			return m_ReadOnly;
	}
	// virtual int DefineEnum(const TwEnumVal *_EnumValues, unsigned int _NbValues);
	CTwVarAtom();
	virtual ~CTwVarAtom();
};

struct CTwVarGroup : CTwVar {
	std::vector<CTwVar *> m_Vars;
	bool m_Open;
	TwSummaryCallback m_SummaryCallback;
	void *m_SummaryClientData;
	void *m_StructValuePtr;
	TwType m_StructType;

	virtual bool IsGroup() const { return true; }
	virtual const CTwVar *Find(const char *_Name, CTwVarGroup **_Parent, int *_Index) const;
	virtual int HasAttrib(const char *_Attrib, bool *_HasValue) const;
	virtual int SetAttrib(int _AttribID, const char *_Value, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex);
	virtual ERetType GetAttrib(int _AttribID, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex, std::vector<double> &outDouble, std::ostringstream &outString) const;
	virtual CTwVarAtom *FindShortcut(int _Key, int _Modifiers, bool *_DoIncr);
	virtual void SetReadOnly(bool _ReadOnly) {
		for (size_t i = 0; i < m_Vars.size(); ++i)
			if (m_Vars[i])
				m_Vars[i]->SetReadOnly(_ReadOnly);
	}
	virtual bool IsReadOnly() const {
		for (size_t i = 0; i < m_Vars.size(); ++i)
			if (m_Vars[i] && !m_Vars[i]->IsReadOnly())
				return false;
		return true;
	}
	CTwVarGroup() {
		m_Open = false;
		m_StructType = TW_TYPE_UNDEF;
		m_SummaryCallback = NULL;
		m_SummaryClientData = NULL;
		m_StructValuePtr = NULL;
	}
	virtual ~CTwVarGroup();
};

//  ---------------------------------------------------------------------------

struct CTwBar {
	std::string m_Name;
	std::string m_Label;
	std::string m_Help;
	bool m_Visible;
	int m_PosX;
	int m_PosY;
	int m_Width;
	int m_Height;
	color32 m_Color;
	bool m_DarkText;
	const CTexFont *m_Font;
	int m_ValuesWidth;
	int m_Sep;
	int m_LineSep;
	int m_FirstLine;
	float m_UpdatePeriod;
	bool m_IsHelpBar;
	int m_MinNumber; // accessed by TwDeleteBar
	bool m_IsPopupList;
	CTwVarAtom *m_VarEnumLinkedToPopupList;
	CTwBar *m_BarLinkedToPopupList;
	bool m_Resizable;
	bool m_Movable;
	bool m_Iconifiable;
	bool m_Contained;

	CTwVarGroup m_VarRoot;

	enum EDrawPart {
		DRAW_BG = (1 << 0),
		DRAW_CONTENT = (1 << 1),
		DRAW_ALL = DRAW_BG | DRAW_CONTENT
	};
	void Draw(int _DrawPart = DRAW_ALL);
	void NotUpToDate();
	const CTwVar *Find(const char *_Name, CTwVarGroup **_Parent = NULL, int *_Index = NULL) const;
	CTwVar *Find(const char *_Name, CTwVarGroup **_Parent = NULL, int *_Index = NULL);
	int HasAttrib(const char *_Attrib, bool *_HasValue) const;
	int SetAttrib(int _AttribID, const char *_Value);
	ERetType GetAttrib(int _AttribID, std::vector<double> &outDouble, std::ostringstream &outString) const;
	bool MouseMotion(int _X, int _Y);
	bool MouseButton(ETwMouseButtonID _Button, bool _Pressed, int _X, int _Y);
	bool MouseWheel(int _Pos, int _PrevPos, int _MouseX, int _MouseY);
	bool KeyPressed(int _Key, int _Modifiers);
	bool KeyTest(int _Key, int _Modifiers);
	bool IsMinimized() const { return m_IsMinimized; }
	bool IsDragging() const { return m_MouseDrag; }
	bool Show(CTwVar *_Var); // display the line associated to _Var
	bool OpenHier(CTwVarGroup *_Root, CTwVar *_Var); // open a hierarchy if it contains _Var
	int LineInHier(CTwVarGroup *_Root, CTwVar *_Var); // returns the number of the line associated to _Var
	void UnHighlightLine() {
		m_HighlightedLine = -1;
		NotUpToDate();
	} // used by PopupCallback
	void HaveFocus(bool _Focus) { m_DrawHandles = _Focus; } // used by PopupCallback
	void StopEditInPlace() {
		if (m_EditInPlace.m_Active)
			EditInPlaceEnd(false);
	}
	CTwBar(const char *_Name);
	~CTwBar();

	color32 m_ColBg, m_ColBg1, m_ColBg2;
	color32 m_ColHighBg0;
	color32 m_ColHighBg1;
	color32 m_ColLabelText;
	color32 m_ColStructText;
	color32 m_ColValBg;
	color32 m_ColValText;
	color32 m_ColValTextRO;
	color32 m_ColValTextNE;
	color32 m_ColValMin;
	color32 m_ColValMax;
	color32 m_ColStructBg;
	color32 m_ColTitleBg;
	color32 m_ColTitleHighBg;
	color32 m_ColTitleUnactiveBg;
	color32 m_ColTitleText;
	color32 m_ColTitleShadow;
	color32 m_ColLine;
	color32 m_ColLineShadow;
	color32 m_ColUnderline;
	color32 m_ColBtn;
	color32 m_ColHighBtn;
	color32 m_ColFold;
	color32 m_ColHighFold;
	color32 m_ColGrpBg;
	color32 m_ColGrpText;
	color32 m_ColHierBg;
	color32 m_ColShortcutText;
	color32 m_ColShortcutBg;
	color32 m_ColInfoText;
	color32 m_ColHelpBg;
	color32 m_ColHelpText;
	color32 m_ColRoto;
	color32 m_ColRotoVal;
	color32 m_ColRotoBound;
	color32 m_ColEditBg;
	color32 m_ColEditText;
	color32 m_ColEditSelBg;
	color32 m_ColEditSelText;
	color32 m_ColSeparator;
	color32 m_ColStaticText;
	void UpdateColors();

protected:
	int m_TitleWidth;
	int m_VarX0;
	int m_VarX1;
	int m_VarX2;
	int m_VarY0;
	int m_VarY1;
	int m_VarY2;
	int m_ScrollYW;
	int m_ScrollYH;
	int m_ScrollY0;
	int m_ScrollY1;
	int m_NbHierLines;
	int m_NbDisplayedLines;
	bool m_UpToDate;
	float m_LastUpdateTime;
	void Update();

	bool m_MouseDrag;
	bool m_MouseDragVar;
	bool m_MouseDragTitle;
	bool m_MouseDragScroll;
	bool m_MouseDragResizeUR;
	bool m_MouseDragResizeUL;
	bool m_MouseDragResizeLR;
	bool m_MouseDragResizeLL;
	bool m_MouseDragValWidth;
	int m_MouseOriginX;
	int m_MouseOriginY;
	double m_ValuesWidthRatio;
	bool m_VarHasBeenIncr;
	int m_FirstLine0;
	int m_HighlightedLine;
	int m_HighlightedLinePrev;
	int m_HighlightedLineLastValid;
	bool m_HighlightIncrBtn;
	bool m_HighlightDecrBtn;
	bool m_HighlightRotoBtn;
	bool m_HighlightListBtn;
	bool m_HighlightBoolBtn;
	bool m_HighlightClickBtn;
	double m_HighlightClickBtnAuto;
	bool m_HighlightTitle;
	bool m_HighlightScroll;
	bool m_HighlightUpScroll;
	bool m_HighlightDnScroll;
	bool m_HighlightMinimize;
	bool m_HighlightFont;
	bool m_HighlightValWidth;
	bool m_HighlightLabelsHeader;
	bool m_HighlightValuesHeader;
	bool m_DrawHandles;

	bool m_IsMinimized;
	int m_MinPosX;
	int m_MinPosY;
	bool m_HighlightMaximize;
	bool m_DrawIncrDecrBtn;
	bool m_DrawRotoBtn;
	bool m_DrawClickBtn;
	bool m_DrawListBtn;
	bool m_DrawBoolBtn;
	EButtonAlign m_ButtonAlign;

	struct CHierTag {
		CTwVar *m_Var;
		int m_Level;
		bool m_Closing;
	};
	std::vector<CHierTag> m_HierTags;
	void BrowseHierarchy(int *_LineNum, int _CurrLevel, const CTwVar *_Var, int _First, int _Last);
	void *m_TitleTextObj;
	void *m_LabelsTextObj;
	void *m_ValuesTextObj;
	void *m_ShortcutTextObj;
	int m_ShortcutLine;
	void *m_HeadersTextObj;
	void ListLabels(std::vector<std::string> &_Labels, std::vector<color32> &_Colors, std::vector<color32> &_BgColors, bool *_HasBgColors, const CTexFont *_Font, int _AtomWidthMax, int _GroupWidthMax);
	void ListValues(std::vector<std::string> &_Values, std::vector<color32> &_Colors, std::vector<color32> &_BgColors, const CTexFont *_Font, int _WidthMax);
	int ComputeLabelsWidth(const CTexFont *_Font);
	int ComputeValuesWidth(const CTexFont *_Font);
	void DrawHierHandle();

	enum EValuesWidthFit { VALUES_WIDTH_FIT = -5555 };

	// RotoSlider
	struct CPoint {
		int x, y;
		CPoint() {}
		CPoint(int _X, int _Y) :
				x(_X), y(_Y) {}
		const CPoint operator+(const CPoint &p) const { return CPoint(x + p.x, y + p.y); }
		const CPoint operator-(const CPoint &p) const { return CPoint(x - p.x, y - p.y); }
	};
	struct CRotoSlider {
		CRotoSlider();
		CTwVarAtom *m_Var;
		double m_PreciseValue;
		double m_CurrentValue;
		double m_Value0;
		double m_ValueAngle0;
		bool m_Active;
		bool m_ActiveMiddle;
		CPoint m_Origin;
		CPoint m_Current;
		bool m_HasPrevious;
		CPoint m_Previous;
		double m_Angle0;
		double m_AngleDT;
		int m_Subdiv;
	};
	CRotoSlider m_Roto;
	int m_RotoMinRadius;
	int m_RotoNbSubdiv; // number of steps for one turn
	void RotoDraw();
	void RotoOnMouseMove(int _X, int _Y);
	void RotoOnLButtonDown(int _X, int _Y);
	void RotoOnLButtonUp(int _X, int _Y);
	void RotoOnMButtonDown(int _X, int _Y);
	void RotoOnMButtonUp(int _X, int _Y);
	double RotoGetValue() const;
	void RotoSetValue(double _Val);
	double RotoGetMin() const;
	double RotoGetMax() const;
	double RotoGetStep() const;
	double RotoGetSteppedValue() const;

	// Edit-in-place
	struct CEditInPlace {
		CEditInPlace();
		~CEditInPlace();
		CTwVarAtom *m_Var;
		bool m_Active;
		std::string m_String;
		void *m_EditTextObj;
		void *m_EditSelTextObj;
		int m_CaretPos;
		int m_SelectionStart;
		int m_X, m_Y;
		int m_Width;
		int m_FirstChar;
		std::string m_Clipboard;
	};
	CEditInPlace m_EditInPlace;
	void EditInPlaceDraw();
	bool EditInPlaceAcceptVar(const CTwVarAtom *_Var);
	bool EditInPlaceIsReadOnly();
	void EditInPlaceStart(CTwVarAtom *_Var, int _X, int _Y, int _Width);
	void EditInPlaceEnd(bool _Commit);
	bool EditInPlaceKeyPressed(int _Key, int _Modifiers);
	bool EditInPlaceEraseSelect();
	bool EditInPlaceMouseMove(int _X, int _Y, bool _Select);
	bool EditInPlaceSetClipboard(const std::string &_String);
	bool EditInPlaceGetClipboard(std::string *_OutString);

	struct CCustomRecord {
		int m_IndexMin;
		int m_IndexMax;
		int m_XMin, m_XMax;
		int m_YMin, m_YMax; // Y visible range
		int m_Y0, m_Y1; // Y widget range
		CTwVarGroup *m_Var;
	};
	typedef std::map<CTwMgr::CStructProxy *, CCustomRecord> CustomMap;
	CustomMap m_CustomRecords;
	CTwMgr::CStructProxy *m_CustomActiveStructProxy;

	friend struct CTwMgr;
};

void DrawArc(int _X, int _Y, int _Radius, float _StartAngleDeg, float _EndAngleDeg, color32 _Color);

//  ---------------------------------------------------------------------------

#endif // !defined ANT_TW_BAR_INCLUDED

//  ---------------------------------------------------------------------------
//  @file       TwFonts.cpp
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

using std::memset; // Fedora patch: memset()

//  ---------------------------------------------------------------------------

CTexFont::CTexFont() {
	for (int i = 0; i < 256; ++i) {
		m_CharU0[i] = 0;
		m_CharU1[i] = 0;
		m_CharV0[i] = 0;
		m_CharV1[i] = 0;
		m_CharWidth[i] = 0;
	}
	m_TexWidth = 0;
	m_TexHeight = 0;
	m_TexBytes = NULL;
	m_NbCharRead = 0;
	m_CharHeight = 0;
}

//  ---------------------------------------------------------------------------

CTexFont::~CTexFont() {
	if (m_TexBytes)
		delete[] m_TexBytes;
	m_TexBytes = nullptr;
	m_TexWidth = 0;
	m_TexHeight = 0;
	m_NbCharRead = 0;
}

//  ---------------------------------------------------------------------------

static int NextPow2(int _n) {
	int r = 1;
	while (r < _n)
		r *= 2;
	return r;
}

//  ---------------------------------------------------------------------------

const char *g_ErrBadFontHeight = "Cannot determine font height while reading font bitmap (check first pixel column)";

CTexFont *TwGenerateFont(const unsigned char *_Bitmap, int _BmWidth, int _BmHeight, float _Scaling) {
	// find height of the font
	int x, y;
	int h = 0, hh = 0;
	int r, NbRow = 0;
	for (y = 0; y < _BmHeight; ++y)
		if (_Bitmap[y * _BmWidth] == 0) {
			if ((hh <= 0 && h <= 0) || (h != hh && h > 0 && hh > 0)) {
				g_TwMgr->SetLastError(g_ErrBadFontHeight);
				return nullptr;
			} else if (h <= 0)
				h = hh;
			else if (hh <= 0)
				break;
			hh = 0;
			++NbRow;
		} else
			++hh;

	// find width and position of each character
	int w = 0;
	int x0[224], y0[224], x1[224], y1[224];
	int ch = 32;
	int start;
	for (r = 0; r < NbRow; ++r) {
		start = 1;
		for (x = 1; x < _BmWidth; ++x)
			if (_Bitmap[(r * (h + 1) + h) * _BmWidth + x] == 0 || x == _BmWidth - 1) {
				if (x == start)
					break; // next row
				if (ch < 256) {
					x0[ch - 32] = start;
					x1[ch - 32] = x;
					y0[ch - 32] = r * (h + 1);
					y1[ch - 32] = r * (h + 1) + h - 1;
					w += x - start + 1;
					start = x + 1;
				}
				++ch;
			}
	}
	for (x = ch - 32; x < 224; ++x) {
		x0[ch] = 0;
		x1[ch] = 0;
		y0[ch] = 0;
		y1[ch] = 0;
	}

	(void)w;

	// Repack: build 14 rows of 16 characters.
	// - First, find the largest row
	int l, lmax = 1;
	for (r = 0; r < 14; ++r) {
		l = 0;
		for (x = 0; x < 16; ++x)
			l += x1[x + r * 16] - x0[x + r * 16] + 1;
		if (l > lmax)
			lmax = l;
	}
	// A little empty margin is added between chars to avoid artefact when antialiasing is on
	const int MARGIN_X = 2;
	const int MARGIN_Y = 2;
	lmax += 16 * MARGIN_X;
	// - Second, build the texture
	CTexFont *TexFont = new CTexFont;
	TexFont->m_NbCharRead = ch - 32;
	TexFont->m_CharHeight = (int)(_Scaling * h + 0.5f);
	TexFont->m_TexWidth = NextPow2(lmax);
	TexFont->m_TexHeight = NextPow2(14 * (h + MARGIN_Y));
	TexFont->m_TexBytes = new unsigned char[TexFont->m_TexWidth * TexFont->m_TexHeight];
	memset(TexFont->m_TexBytes, 0, TexFont->m_TexWidth * TexFont->m_TexHeight);
	int xx;
	float du = 0.4;
	float dv = 0.4;
	DEV_ASSERT(g_TwMgr != NULL);
	if (g_TwMgr) {
		du = ANT_TEXEL_ALIGNEMENT;
		dv = ANT_TEXEL_ALIGNEMENT;
	}
	float alpha;
	for (r = 0; r < 14; ++r)
		for (xx = 0, ch = r * 16; ch < (r + 1) * 16; ++ch)
			if (y1[ch] - y0[ch] == h - 1) {
				for (y = 0; y < h; ++y)
					for (x = x0[ch]; x <= x1[ch]; ++x) {
						alpha = ((float)(_Bitmap[x + (y0[ch] + y) * _BmWidth])) / 256.0;
						// alpha = alpha*sqrtf(alpha); // powf(alpha, 1.5f);   // some gamma correction
						TexFont->m_TexBytes[(xx + x - x0[ch]) + (r * (h + MARGIN_Y) + y) * TexFont->m_TexWidth] = (unsigned char)(alpha * 256.0);
					}
				TexFont->m_CharU0[ch + 32] = (float(xx) + du) / float(TexFont->m_TexWidth);
				xx += x1[ch] - x0[ch] + 1;
				TexFont->m_CharU1[ch + 32] = (float(xx) + du) / float(TexFont->m_TexWidth);
				TexFont->m_CharV0[ch + 32] = (float(r * (h + MARGIN_Y)) + dv) / float(TexFont->m_TexHeight);
				TexFont->m_CharV1[ch + 32] = (float(r * (h + MARGIN_Y) + h) + dv) / float(TexFont->m_TexHeight);
				TexFont->m_CharWidth[ch + 32] = (int)(_Scaling * (x1[ch] - x0[ch] + 1) + 0.5);
				xx += MARGIN_X;
			}

	const unsigned char Undef = 127; // default character used as for undifined ones (having ascii codes from 0 to 31)
	for (ch = 0; ch < 32; ++ch) {
		TexFont->m_CharU0[ch] = TexFont->m_CharU0[Undef];
		TexFont->m_CharU1[ch] = TexFont->m_CharU1[Undef];
		TexFont->m_CharV0[ch] = TexFont->m_CharV0[Undef];
		TexFont->m_CharV1[ch] = TexFont->m_CharV1[Undef];
		TexFont->m_CharWidth[ch] = TexFont->m_CharWidth[Undef] / 2;
	}

	return TexFont;
}

//  ---------------------------------------------------------------------------

CTexFont *g_DefaultSmallFont = nullptr;
CTexFont *g_DefaultNormalFont = nullptr;
CTexFont *g_DefaultLargeFont = nullptr;
CTexFont *g_DefaultFixed1Font = nullptr;

// Small font
const int FONT0_BM_W = 211;
const int FONT0_BM_H = 84;
static const unsigned char s_Font0[] = {
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 255, 255,
	255, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 0, 255, 255, 0, 0, 255,
	255, 255, 255, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 127, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 255, 255,
	0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255,
	255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 255, 0, 0, 0, 255, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 255, 0,
	255, 0, 255, 255, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 255, 0, 255, 255, 255, 0, 0, 255, 255, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 255,
	0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255,
	255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 255, 0,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0,
	0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 255,
	0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	255, 0, 0, 255, 255, 255, 0, 255, 255, 255, 255, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	255, 255, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 0, 127, 0, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 0, 127, 127, 0, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 0, 127, 0, 127,
	127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0,
	255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0,
	0, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 0,
	0, 0, 0, 255, 0, 255, 255, 255, 0, 0, 255, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 255,
	0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255,
	0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 255, 255,
	255, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0,
	255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0,
	255, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 0, 255, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 255, 0, 255, 0, 255,
	0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 0, 255, 255, 255, 0, 255, 255,
	255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 255, 0, 255, 0, 255, 0, 255, 255, 255,
	255, 255, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255,
	0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0,
	255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 127, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255,
	255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 0, 255, 255, 0, 0, 255, 0, 0,
	0, 255, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255,
	255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 255, 255,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255,
	0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 0, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4,
	4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 4, 4, 4, 4, 12, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 0, 255, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 0, 0, 255, 255, 255, 0, 0,
	255, 255, 0, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 0, 255, 0, 255, 255, 0,
	255, 0, 0, 255, 0, 255, 0, 255, 255, 255, 0, 255, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255,
	0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 255, 0, 255, 255, 255, 255, 0, 255, 255, 0, 255,
	0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255,
	0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0,
	255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0,
	0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0,
	255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0,
	255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0,
	255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255,
	0, 0, 255, 0, 0, 255, 4, 4, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0,
	255, 255, 255, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255,
	255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0,
	0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255, 255, 4, 255, 255, 0, 4, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0,
	0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0,
	0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0,
	255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0,
	0, 255, 255, 255, 0, 255, 255, 255, 0, 0, 0, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255,
	0, 255, 0, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255,
	0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255,
	255, 255, 0, 255, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 255, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 0, 127, 127, 0, 127, 127, 127, 127,
	0, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 0, 127, 127, 0,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127,
	0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	255, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255,
	0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255,
	255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 255, 0, 0, 255, 255, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 255, 0, 255,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255,
	0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0,
	255, 255, 255, 0, 255, 0, 0, 0, 255, 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	255, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 255, 0, 255,
	255, 0, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 0,
	0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 255,
	255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255,
	0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255,
	255, 255, 0, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 0, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 0, 127, 0, 127,
	127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	0, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 255, 0, 255, 0, 0, 255, 255, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0,
	0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 0, 255, 0, 0,
	0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255,
	0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 255,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255,
	255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0,
	255, 255, 0, 255, 0, 255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 255, 255, 255, 0, 255, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255,
	0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 255,
	255, 0, 0, 255, 255, 0, 255, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0,
	255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0,
	255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 255, 255, 0, 255, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0,
	0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0,
	0, 0, 0, 255, 0, 0, 255, 255, 255, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255,
	255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 0, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 0, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 127, 0, 0, 255, 255,
	0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255,
	0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255,
	255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 255, 255, 255,
	255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255,
	255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0,
	0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0,
	255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0,
	255, 0, 127, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255,
	255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 255, 0, 127, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 127,
	0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255,
	255, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 255, 255, 0, 0, 255,
	0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255,
	0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 127, 255, 255,
	255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 255,
	255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255,
	0, 0, 255, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 127, 255, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255,
	255, 0, 0, 255, 0, 0, 255, 0, 127, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255,
	255, 0, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255,
	255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0,
	255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 255,
	255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255,
	0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 255, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 255, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0,
	255, 0, 0, 255, 0, 255, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 255, 0, 255, 0, 255, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 127, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255,
	255, 0, 0, 0, 255, 255, 0, 0, 255, 255, 255, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 255, 255, 0,
	0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0,
	0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 255, 255,
	0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 255, 255, 255, 255,
	0, 255, 0, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255,
	0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 0, 255, 0, 0, 0,
	255, 255, 255, 255, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 0,
	0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0,
	255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0,
	255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 127, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0,
	255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255,
	255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 0, 255,
	255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0,
	255, 255, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 0, 0, 255, 0, 0, 255, 0,
	0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255,
	255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 0, 127, 0, 127, 127, 0, 127, 127, 0,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Normal font
const int FONT1_BM_W = 253;
const int FONT1_BM_H = 106;
static const unsigned char s_Font1[] = {
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255,
	255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255,
	255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 127, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 255,
	255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255,
	0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 255, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0,
	255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255,
	255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255,
	0, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255,
	255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 0, 255, 255, 255,
	255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 0,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
	255, 255, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255,
	255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 0, 0, 255,
	255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 255, 0, 255,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0,
	255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255,
	255, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255,
	255, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 255, 0, 0, 0, 0, 255, 255, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255,
	255, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 127, 0, 255, 0, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0,
	255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 255, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	255, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 255, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 255,
	255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 255, 0, 0, 0, 255, 255, 255, 0, 255, 255,
	255, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
	255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 255,
	0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 255, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 127, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0,
	255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 0, 255, 255, 255, 255,
	255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255,
	255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255,
	255, 255, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 255, 255, 255, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 255, 0,
	255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255,
	255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 255, 0, 0,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255,
	0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 255, 0, 255, 255,
	255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0,
	0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 255, 0, 255, 255, 255, 0, 0, 255, 0, 255,
	255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 255, 255, 255, 0, 255, 255, 255, 0, 0, 255, 0, 255,
	255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255,
	255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 0, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255,
	255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 255, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0,
	0, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	255, 255, 255, 255, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 255, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0,
	0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0,
	255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 255, 0, 255, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 255, 255,
	255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 255, 255,
	255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0,
	255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 255, 0, 255, 0, 0, 0,
	255, 255, 255, 0, 0, 0, 0, 255, 255, 0, 0, 255, 255, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0,
	255, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255, 0, 0,
	0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 255, 255, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 127, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 255, 255, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 127, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 255, 0, 255, 0, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 255, 255, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 127,
	255, 255, 255, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 127, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255,
	255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255,
	255, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 127, 255, 255, 255, 255, 255, 255, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 127, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 127, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 127, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0,
	0, 0, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255,
	255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0,
	255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0,
	0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255,
	255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255,
	255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255,
	0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255,
	255, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255,
	0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255,
	255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 255, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0,
	255, 0, 0, 255, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 255, 255,
	255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255,
	0, 255, 255, 255, 0, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0,
	0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255,
	255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 255, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255,
	255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255,
	0, 0, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255,
	255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0,
	255, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 255, 255,
	255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 255,
	255, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255,
	255, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255,
	0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255,
	255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255,
	0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0,
	255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255,
	255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255,
	0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
	255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255,
	255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0,
	0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 255,
	0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	255, 0, 0, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0,
	255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255,
	255, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255,
	0, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 255, 255,
	255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255,
	255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 255,
	255, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0,
	255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127,
	127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 0, 255, 255, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0,
	255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 255, 255, 0, 255,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255,
	0, 0, 255, 0, 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0,
	255, 0, 255, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255,
	0, 255, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0,
	0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255,
	0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 255, 255, 0, 0, 0,
	0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255,
	255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255,
	0, 255, 0, 0, 255, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255,
	0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0,
	0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 255,
	255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255,
	255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255,
	0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 255, 0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 127, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 255, 255, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0,
	0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 255, 255, 255, 0, 0,
	255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255,
	255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255, 0,
	0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255,
	255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255, 255,
	255, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255,
	0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255,
	0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255,
	0, 255, 0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255,
	0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255,
	0, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0,
	0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255,
	0, 0, 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0,
	255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 0,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 255,
	255, 0, 255, 0, 0, 0, 255, 255, 0, 255, 0, 0, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0,
	255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 255, 255, 255, 255, 0, 0, 255,
	255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255,
	0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255,
	0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255,
	255, 255, 255, 0, 0, 0, 255, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255,
	0, 0, 0, 0, 255, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255,
	255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0,
	0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255, 255, 255, 0, 255, 0, 0, 255,
	255, 255, 0, 255, 0, 0, 255, 255, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 255,
	0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
	0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 0, 127, 0, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Normal font anti-aliased
const int FONT1AA_BM_W = 264;
const int FONT1AA_BM_H = 106;
static const unsigned char s_Font1AA[] = {
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0,
	59, 241, 97, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 168, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 251, 89, 0, 0, 89, 255, 125, 89, 255, 125, 0, 0, 0, 0,
	7, 199, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 59, 238, 42,
	206, 125, 0, 0, 0, 0, 7, 199, 34, 89, 166, 0, 0, 0, 0, 168, 34, 0, 0, 0, 175, 255, 255, 166, 0,
	0, 7, 202, 89, 0, 0, 0, 0, 59, 245, 255, 251, 89, 0, 0, 0, 59, 238, 34, 0, 12, 232, 89, 0, 0, 89,
	247, 34, 0, 59, 245, 206, 199, 124, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 12, 235, 255, 247, 34, 0, 0, 0, 0, 12, 232, 89, 0, 0, 12, 235,
	255, 255, 251, 89, 0, 7, 206, 255, 255, 255, 125, 0, 0, 0, 0, 138, 251, 89, 0, 0, 59, 245, 255,
	255, 255, 251, 89, 0, 0, 89, 255, 255, 166, 0, 89, 255, 255, 255, 255, 255, 201, 0, 0, 59, 245,
	255, 255, 125, 0, 0, 12, 235, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 255, 247, 34, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 59, 238, 34, 175, 125, 0, 0, 0, 0, 59, 192, 0, 172,
	89, 0, 0, 59, 245, 255, 255, 251, 89, 89, 247, 34, 12, 228, 34, 0, 138, 166, 0, 0, 0, 0, 12, 235,
	125, 0, 175, 225, 21, 0, 0, 59, 238, 34, 0, 138, 201, 0, 0, 0, 0, 175, 166, 0, 0, 0, 89, 255, 201,
	0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 215, 21, 0, 175,
	166, 0, 138, 201, 0, 0, 7, 206, 255, 251, 89, 0, 0, 59, 192, 0, 0, 138, 247, 34, 59, 192, 0, 0,
	89, 251, 89, 0, 0, 59, 245, 251, 89, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0,
	0, 0, 7, 206, 166, 0, 7, 206, 125, 0, 89, 247, 34, 7, 206, 166, 0, 138, 225, 21, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 0,
	0, 0, 89, 125, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 12, 206,
	21, 175, 125, 0, 0, 89, 255, 255, 255, 255, 255, 255, 166, 59, 241, 89, 168, 34, 138, 125,
	89, 225, 21, 7, 202, 89, 12, 228, 34, 0, 0, 0, 0, 12, 232, 89, 0, 138, 201, 0, 0, 0, 12, 206, 21,
	7, 202, 89, 0, 0, 0, 0, 59, 215, 21, 59, 245, 206, 199, 124, 255, 125, 0, 0, 0, 0, 7, 202, 89,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 12, 232, 89, 0, 59, 238, 34, 0, 0,
	0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 12, 232, 132, 241, 89,
	0, 0, 59, 241, 89, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 12, 232, 89, 0,
	12, 232, 89, 59, 241, 89, 0, 59, 241, 89, 0, 138, 247, 34, 0, 0, 138, 247, 34, 0, 0, 0, 0, 0, 12,
	235, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138,
	225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 172,
	89, 59, 192, 0, 0, 59, 238, 34, 168, 34, 0, 0, 89, 247, 34, 12, 228, 34, 138, 166, 0, 0, 0, 0,
	0, 0, 138, 251, 159, 247, 34, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 7, 202, 89, 0, 0, 7, 199,
	34, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 59,
	241, 89, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 138, 201,
	0, 7, 206, 125, 59, 241, 89, 0, 0, 59, 245, 255, 255, 251, 89, 0, 12, 235, 255, 255, 255, 125,
	0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 175, 251, 89, 138, 201, 0, 59, 241, 89, 0, 12, 235, 125, 0, 138,
	247, 34, 0, 0, 138, 247, 34, 0, 0, 0, 59, 245, 247, 34, 0, 0, 0, 0, 7, 206, 255, 255, 255, 255,
	255, 255, 125, 0, 0, 0, 0, 138, 255, 201, 0, 0, 0, 0, 0, 0, 89, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 206, 21, 138, 125, 0, 0, 7, 206, 255,
	247, 34, 0, 0, 0, 175, 255, 255, 166, 59, 215, 21, 175, 255, 255, 125, 0, 0, 138, 171, 206,
	166, 0, 175, 201, 0, 0, 0, 0, 89, 201, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235,
	255, 255, 255, 255, 255, 255, 125, 0, 0, 0, 0, 138, 255, 255, 251, 89, 0, 0, 0, 0, 0, 59, 215,
	21, 0, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 138,
	255, 255, 125, 0, 175, 201, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 175, 247, 34, 59, 241, 89, 0, 89,
	247, 34, 0, 0, 0, 89, 247, 34, 0, 0, 0, 89, 255, 255, 255, 125, 0, 12, 235, 166, 0, 59, 245, 125,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 89, 251, 89, 0, 0, 7, 206, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 201,
	0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 255, 255, 225, 21, 0, 0, 0, 175, 255, 251, 89, 0,
	0, 0, 0, 0, 175, 125, 89, 225, 21, 59, 238, 34, 89, 225, 21, 12, 235, 166, 175, 166, 0, 0, 0,
	0, 89, 201, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89,
	0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 59, 241, 97, 206, 255, 255, 255, 255, 255, 125, 0,
	0, 0, 0, 0, 59, 241, 89, 59, 238, 34, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 0, 12, 232, 89,
	0, 59, 245, 125, 0, 89, 255, 255, 232, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 201, 0, 0, 0, 0, 7, 206, 125, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
	4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 125, 12, 206, 21,
	0, 0, 0, 0, 0, 168, 34, 175, 166, 0, 0, 0, 0, 59, 215, 21, 138, 201, 0, 12, 228, 34, 138, 225,
	21, 0, 12, 235, 251, 89, 0, 0, 0, 0, 59, 215, 21, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 12, 232, 89, 0,
	59, 238, 34, 0, 0, 0, 59, 241, 89, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0,
	0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 59, 241, 89, 12, 232, 89, 0, 12, 232, 89, 0, 0, 138, 225, 21,
	0, 0, 0, 59, 238, 34, 0, 7, 206, 166, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 12, 235, 247, 34, 0, 0, 7, 206, 255, 255, 255, 255, 255, 255, 125, 0, 0, 138, 255, 166, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0,
	0, 0, 0, 0, 172, 89, 89, 166, 0, 0, 0, 89, 166, 0, 168, 42, 206, 125, 0, 0, 0, 7, 202, 89, 0, 89,
	225, 21, 59, 238, 34, 89, 251, 89, 0, 0, 175, 255, 201, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 59,
	215, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 138, 247, 34, 0, 0, 0, 0, 0, 7, 206,
	201, 0, 12, 228, 34, 0, 0, 0, 175, 166, 0, 138, 201, 0, 0, 0, 0, 59, 241, 89, 0, 0, 12, 235, 166,
	0, 0, 0, 0, 89, 166, 0, 0, 89, 251, 89, 0, 0, 0, 59, 241, 89, 0, 0, 59, 192, 0, 0, 175, 225, 21,
	0, 175, 201, 0, 138, 225, 21, 0, 12, 235, 125, 0, 0, 0, 0, 12, 235, 166, 0, 59, 241, 89, 0, 0,
	0, 7, 206, 166, 0, 0, 138, 247, 34, 0, 0, 59, 245, 125, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 12, 206, 21, 138, 125, 0, 0, 0, 12, 235, 255,
	255, 255, 166, 0, 0, 0, 0, 138, 201, 0, 0, 0, 175, 255, 255, 125, 0, 0, 138, 255, 255, 255, 125,
	12, 235, 247, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 7, 206, 201, 0, 89, 201, 0, 0, 0, 0, 12, 235, 255, 247,
	34, 0, 0, 7, 206, 255, 255, 255, 225, 21, 89, 255, 255, 255, 255, 255, 166, 59, 245, 255, 255,
	251, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 12, 235, 255, 255, 225, 21, 0, 0, 12, 235, 255, 251, 89,
	0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 59, 245, 255, 255, 125, 0, 0, 89, 255, 255, 166, 0, 0, 0, 138,
	247, 34, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 168, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 168, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89,
	255, 125, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 201, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 125, 138, 166, 0, 0, 0, 89, 255, 255, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 89, 255, 255,
	255, 255, 166, 0, 0, 0, 0, 0, 12, 235, 225, 21, 0, 0, 59, 245, 255, 255, 255, 251, 89, 0, 0, 0,
	59, 245, 255, 255, 251, 89, 59, 245, 255, 255, 255, 247, 34, 0, 0, 59, 245, 255, 255, 255,
	255, 127, 81, 245, 255, 255, 255, 255, 127, 0, 0, 59, 245, 255, 255, 255, 166, 0, 59, 241,
	89, 0, 0, 0, 59, 241, 89, 89, 255, 255, 255, 125, 7, 206, 255, 251, 89, 59, 241, 89, 0, 0, 89,
	255, 166, 59, 241, 89, 0, 0, 0, 0, 59, 245, 225, 21, 0, 0, 7, 206, 251, 89, 59, 245, 247, 34,
	0, 0, 59, 241, 89, 0, 0, 138, 255, 255, 255, 166, 0, 0, 59, 245, 255, 255, 255, 225, 21, 0, 0,
	0, 138, 255, 255, 255, 166, 0, 0, 59, 245, 255, 255, 255, 251, 89, 0, 0, 0, 59, 245, 255, 255,
	201, 89, 255, 255, 255, 255, 255, 255, 255, 125, 59, 241, 89, 0, 0, 0, 59, 241, 97, 206, 166,
	0, 0, 0, 0, 175, 201, 175, 201, 0, 0, 7, 206, 201, 0, 0, 0, 175, 171, 206, 225, 21, 0, 0, 59, 245,
	166, 245, 125, 0, 0, 0, 89, 251, 89, 89, 255, 255, 255, 255, 255, 127, 0, 228, 34, 0, 0, 59,
	215, 21, 0, 0, 0, 0, 12, 228, 34, 0, 0, 0, 59, 245, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 175, 225, 21, 0, 0, 0, 175, 225, 21, 0, 0, 0, 89, 232, 241, 89, 0, 0, 59,
	241, 89, 0, 0, 138, 225, 21, 0, 89, 255, 125, 0, 0, 59, 192, 59, 241, 89, 0, 0, 175, 251, 89,
	0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 7, 199, 34, 59, 241,
	89, 0, 0, 0, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0,
	59, 241, 89, 0, 0, 0, 0, 59, 245, 255, 125, 0, 0, 89, 255, 251, 89, 59, 245, 255, 201, 0, 0, 59,
	241, 89, 0, 138, 251, 89, 0, 12, 235, 166, 0, 59, 241, 89, 0, 7, 206, 225, 21, 0, 138, 251, 89,
	0, 12, 235, 166, 0, 59, 241, 89, 0, 0, 138, 247, 34, 0, 12, 235, 125, 0, 7, 176, 21, 0, 0, 59,
	241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 138, 225, 21, 0, 0, 12, 235, 125, 89, 225,
	21, 0, 59, 245, 247, 34, 0, 12, 232, 89, 12, 235, 166, 0, 7, 206, 166, 0, 89, 247, 34, 0, 7, 206,
	125, 0, 0, 0, 0, 0, 7, 206, 166, 12, 228, 34, 0, 0, 7, 202, 89, 0, 0, 0, 0, 12, 228, 34, 0, 0, 12,
	235, 133, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 138, 201, 0, 138,
	255, 255, 255, 125, 138, 166, 0, 0, 7, 206, 166, 175, 166, 0, 0, 59, 241, 89, 0, 0, 89, 247,
	34, 7, 206, 166, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 175, 225, 21, 59, 241, 89, 0, 0, 0, 0, 59,
	241, 89, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 59, 241,
	89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 159,
	225, 21, 0, 175, 166, 241, 89, 59, 241, 132, 241, 89, 0, 59, 241, 89, 12, 235, 166, 0, 0, 0,
	89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 12, 235, 166, 0, 0, 0, 89, 247, 34, 59, 241, 89,
	0, 0, 59, 241, 89, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59,
	241, 89, 59, 241, 89, 0, 0, 89, 225, 21, 59, 241, 89, 0, 89, 206, 202, 89, 0, 59, 238, 34, 0,
	89, 251, 89, 138, 225, 21, 0, 0, 175, 201, 0, 138, 225, 21, 0, 0, 0, 0, 0, 175, 225, 21, 12, 228,
	34, 0, 0, 0, 138, 166, 0, 0, 0, 0, 12, 228, 34, 0, 7, 206, 166, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 7, 202, 89, 89, 225, 21, 7, 206, 125, 12, 206, 21, 0, 59, 238,
	34, 89, 247, 34, 0, 59, 241, 89, 0, 0, 175, 201, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0,
	0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0,
	59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 102, 232, 89,
	0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 102, 232, 89, 59, 215, 81, 241, 89, 59, 241, 89, 138,
	225, 21, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 7, 206, 201, 0, 59, 241,
	89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 175, 201, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 59,
	241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 7, 206, 166, 0, 0, 175, 166, 0, 7, 206, 125,
	0, 175, 125, 175, 166, 0, 138, 201, 0, 0, 0, 175, 255, 251, 89, 0, 0, 0, 59, 245, 166, 241, 89,
	0, 0, 0, 0, 0, 89, 247, 34, 0, 12, 228, 34, 0, 0, 0, 89, 201, 0, 0, 0, 0, 12, 228, 34, 12, 235, 201,
	0, 0, 0, 59, 245, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 59, 215, 21, 175, 125,
	0, 7, 206, 125, 7, 199, 34, 0, 138, 201, 0, 12, 235, 125, 0, 59, 245, 255, 255, 255, 247, 34,
	0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 245, 255, 255, 255, 255,
	127, 59, 245, 255, 255, 255, 255, 127, 59, 241, 89, 0, 0, 0, 0, 0, 0, 59, 245, 255, 255, 255,
	255, 255, 251, 89, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 245, 255, 247, 34, 0, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 59, 241, 89, 138, 201, 175, 166, 59, 241, 89, 59, 241, 89, 12, 235, 125, 59,
	241, 89, 59, 241, 89, 0, 0, 0, 12, 235, 125, 59, 245, 255, 255, 255, 201, 0, 0, 59, 241, 89,
	0, 0, 0, 12, 235, 125, 59, 245, 255, 255, 255, 125, 0, 0, 0, 0, 59, 245, 255, 255, 125, 0, 0,
	0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 138, 225, 21, 59, 241, 89, 0, 0,
	175, 201, 7, 202, 89, 89, 201, 0, 175, 166, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 138, 255, 166,
	0, 0, 0, 0, 0, 59, 245, 125, 0, 0, 12, 228, 34, 0, 0, 0, 12, 228, 34, 0, 0, 0, 12, 228, 34, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 59, 215, 21, 175, 125, 0, 7,
	206, 125, 7, 199, 34, 7, 206, 125, 0, 0, 175, 201, 0, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241,
	89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0,
	0, 0, 59, 241, 89, 0, 59, 245, 255, 251, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 59, 241, 89,
	0, 0, 0, 59, 241, 89, 59, 241, 89, 175, 225, 21, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 12,
	235, 247, 34, 59, 241, 89, 59, 241, 89, 0, 89, 247, 94, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241,
	89, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 12, 235, 166, 0,
	0, 0, 0, 0, 0, 0, 138, 251, 89, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0,
	12, 232, 89, 138, 225, 21, 0, 0, 89, 225, 81, 215, 21, 12, 228, 47, 232, 89, 0, 0, 0, 175, 255,
	251, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 7, 206, 201, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 175,
	125, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	12, 228, 34, 89, 201, 0, 7, 206, 125, 59, 215, 21, 59, 245, 255, 255, 255, 255, 247, 34, 59,
	241, 89, 0, 0, 59, 241, 89, 7, 206, 166, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 138, 225, 21, 59,
	241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0,
	0, 0, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 7, 206, 201, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 59, 241, 89, 0, 175, 166, 0, 59, 241, 89, 59, 241, 89, 0, 7, 206, 200, 241, 89,
	12, 235, 166, 0, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 89, 247,
	34, 59, 241, 89, 0, 59, 245, 125, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 59, 241, 89, 0, 0, 0, 12,
	232, 89, 0, 0, 0, 59, 238, 34, 0, 0, 175, 171, 206, 166, 0, 0, 0, 12, 232, 159, 201, 0, 7, 202,
	132, 215, 21, 0, 0, 89, 247, 34, 175, 225, 21, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 138, 225, 21,
	0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 89, 201, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 138, 201, 7, 206, 255, 251, 226, 255, 255, 166, 0,
	138, 201, 0, 0, 0, 12, 235, 125, 59, 241, 89, 0, 0, 138, 247, 34, 0, 89, 255, 125, 0, 0, 59, 192,
	59, 241, 89, 0, 0, 138, 251, 89, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 89, 255,
	125, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0, 89, 247, 34,
	59, 241, 89, 0, 12, 235, 166, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89,
	59, 241, 89, 0, 0, 59, 245, 251, 89, 0, 138, 251, 89, 0, 59, 245, 166, 0, 59, 241, 89, 0, 0, 0,
	0, 0, 0, 138, 251, 89, 0, 59, 245, 166, 0, 59, 241, 89, 0, 0, 138, 251, 89, 0, 89, 166, 0, 0, 89,
	247, 34, 0, 0, 59, 241, 89, 0, 0, 0, 0, 138, 225, 21, 0, 7, 206, 166, 0, 0, 0, 89, 255, 251, 89,
	0, 0, 0, 7, 206, 255, 125, 0, 0, 138, 255, 201, 0, 0, 12, 235, 125, 0, 12, 235, 166, 0, 0, 0, 59,
	241, 89, 0, 0, 0, 89, 251, 89, 0, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 12, 228, 34, 0, 0, 12, 228, 34,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 7, 206, 225, 21, 0,
	0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 175, 201, 59, 245, 255, 255, 255, 247, 34, 0, 0, 0, 59,
	245, 255, 255, 251, 89, 59, 245, 255, 255, 255, 225, 21, 0, 0, 59, 245, 255, 255, 255, 255,
	127, 81, 241, 89, 0, 0, 0, 0, 0, 0, 59, 245, 255, 255, 255, 201, 0, 59, 241, 89, 0, 0, 0, 59, 241,
	89, 89, 255, 255, 255, 138, 235, 255, 255, 125, 0, 59, 241, 89, 0, 0, 89, 255, 201, 59, 245,
	255, 255, 255, 255, 166, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 175, 251,
	89, 0, 0, 138, 255, 255, 255, 166, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 138, 255, 255, 255,
	166, 0, 0, 59, 241, 89, 0, 0, 0, 175, 251, 89, 12, 235, 255, 255, 251, 89, 0, 0, 0, 59, 241, 89,
	0, 0, 0, 0, 0, 59, 245, 255, 251, 89, 0, 0, 0, 0, 12, 235, 201, 0, 0, 0, 0, 0, 138, 251, 89, 0, 0,
	89, 255, 125, 0, 7, 206, 225, 21, 0, 0, 89, 255, 125, 0, 0, 59, 241, 89, 0, 0, 0, 175, 255, 255,
	255, 255, 255, 127, 0, 228, 34, 0, 0, 0, 0, 0, 175, 125, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 89, 255, 255, 255, 255, 125, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12,
	228, 34, 0, 0, 0, 0, 0, 89, 201, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 125, 0, 0, 0, 12, 228,
	124, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 245, 255, 255, 255, 255, 255, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 166, 0, 0, 0, 0, 0,
	0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 255, 201, 0, 12, 228,
	34, 0, 0, 89, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 116, 116,
	4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 89, 251, 89, 59, 241, 89, 0, 0, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 225,
	21, 0, 0, 12, 228, 34, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4,
	4, 4, 28, 244, 252, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59,
	241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166,
	0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4,
	4, 180, 252, 164, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 255, 255,
	125, 0, 59, 241, 194, 255, 251, 89, 0, 0, 7, 206, 255, 255, 201, 0, 12, 235, 255, 255, 251,
	89, 0, 12, 235, 255, 251, 89, 7, 206, 255, 255, 247, 34, 0, 12, 235, 255, 255, 251, 89, 59,
	241, 194, 255, 255, 125, 0, 59, 241, 89, 89, 255, 251, 89, 59, 241, 89, 0, 138, 251, 89, 59,
	241, 89, 59, 241, 159, 255, 255, 125, 89, 255, 255, 166, 0, 59, 241, 194, 255, 255, 125, 0,
	0, 0, 12, 235, 255, 247, 34, 0, 59, 241, 194, 255, 255, 125, 0, 0, 12, 235, 255, 255, 251, 89,
	59, 241, 159, 255, 201, 0, 138, 255, 255, 247, 34, 206, 255, 255, 255, 166, 59, 241, 89, 0,
	59, 241, 97, 206, 166, 0, 0, 12, 235, 125, 175, 201, 0, 7, 206, 166, 0, 7, 206, 133, 206, 225,
	21, 0, 89, 255, 255, 166, 0, 0, 12, 235, 125, 138, 255, 255, 255, 255, 166, 0, 0, 138, 166,
	0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4,
	76, 252, 244, 20, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34,
	59, 245, 166, 0, 138, 225, 21, 7, 206, 201, 0, 0, 0, 7, 206, 166, 0, 59, 241, 89, 7, 206, 125,
	0, 89, 225, 21, 59, 241, 89, 0, 0, 7, 206, 166, 0, 59, 241, 89, 59, 245, 166, 0, 89, 247, 34,
	59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 138, 225, 21, 0, 59, 241, 89, 59, 245, 201, 0, 89,
	255, 201, 0, 89, 247, 34, 59, 245, 166, 0, 89, 247, 34, 0, 7, 206, 166, 0, 138, 225, 21, 59,
	245, 166, 0, 138, 247, 34, 7, 206, 166, 0, 59, 241, 89, 59, 245, 201, 0, 0, 59, 238, 34, 0, 130,
	34, 59, 241, 89, 0, 0, 59, 241, 89, 0, 59, 241, 89, 89, 247, 34, 0, 89, 247, 34, 138, 225, 21,
	12, 235, 225, 21, 12, 232, 89, 7, 206, 166, 12, 235, 125, 89, 247, 34, 0, 89, 247, 34, 0, 0,
	0, 89, 247, 34, 0, 0, 138, 166, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 89, 225, 21, 0, 0, 7, 206, 247,
	34, 0, 0, 89, 201, 0, 0, 4, 4, 68, 12, 4, 4, 4, 220, 252, 108, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59,
	241, 89, 0, 59, 241, 89, 59, 238, 34, 0, 59, 238, 34, 59, 241, 89, 0, 0, 59, 241, 89, 0, 59, 241,
	89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 159, 201, 0, 0, 0, 59,
	241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241,
	89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0,
	0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 59, 241, 89, 12, 235, 125, 0, 175,
	166, 0, 59, 238, 34, 89, 171, 202, 89, 89, 225, 21, 0, 59, 241, 226, 201, 0, 12, 235, 125, 0,
	175, 166, 0, 0, 0, 12, 235, 125, 0, 0, 59, 238, 34, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 7, 206, 125,
	0, 7, 202, 89, 12, 235, 166, 0, 175, 125, 0, 0, 4, 60, 244, 172, 4, 4, 132, 252, 212, 4, 4, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 255, 251, 89, 59, 241, 89,
	0, 59, 241, 89, 59, 238, 34, 0, 0, 0, 59, 238, 34, 0, 59, 241, 89, 59, 245, 255, 255, 255, 251,
	0, 59, 241, 89, 0, 0, 59, 238, 34, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0,
	59, 241, 89, 59, 245, 255, 225, 21, 0, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241,
	89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 238, 34, 0, 12, 232, 89, 59, 241, 89, 0, 12, 232, 89,
	59, 238, 34, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 175, 255, 255, 201, 0, 59, 241, 89, 0, 0, 59,
	241, 89, 0, 59, 241, 89, 0, 175, 201, 12, 232, 89, 0, 7, 206, 125, 172, 89, 138, 166, 138, 201,
	0, 0, 0, 138, 247, 34, 0, 0, 175, 201, 12, 232, 89, 0, 0, 7, 206, 166, 0, 0, 175, 225, 21, 0, 0,
	0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 175, 225, 34, 206, 21, 0, 0, 175, 255, 166, 0, 0, 0, 4, 52,
	244, 252, 140, 36, 244, 252, 60, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 59,
	241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 238, 34, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 59,
	241, 89, 59, 238, 34, 0, 0, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89,
	0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 97, 206, 201, 0, 0, 59, 241, 89, 59, 241,
	89, 0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89,
	59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 0, 0, 59, 245,
	125, 59, 241, 89, 0, 0, 59, 241, 89, 0, 59, 241, 89, 0, 59, 238, 124, 225, 21, 0, 0, 175, 176,
	206, 21, 59, 215, 187, 125, 0, 0, 59, 245, 255, 201, 0, 0, 89, 247, 124, 225, 21, 0, 0, 138,
	225, 21, 0, 0, 0, 59, 241, 89, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 4, 4, 76, 252, 252, 220, 252, 164, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 89, 251, 89, 59, 241, 89, 0, 175, 201, 0, 7, 206, 201, 0, 0, 0,
	7, 206, 166, 0, 138, 251, 89, 7, 206, 166, 0, 7, 199, 34, 59, 241, 89, 0, 0, 7, 206, 166, 0, 138,
	251, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 12, 235, 166,
	0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 7,
	206, 166, 0, 138, 225, 21, 59, 241, 89, 0, 138, 225, 21, 7, 206, 166, 0, 89, 251, 89, 59, 241,
	89, 0, 0, 89, 125, 0, 12, 232, 89, 12, 232, 89, 0, 12, 12, 235, 125, 0, 175, 251, 89, 0, 7, 206,
	255, 125, 0, 0, 0, 89, 255, 201, 0, 7, 206, 247, 34, 0, 7, 206, 166, 59, 245, 125, 0, 7, 206,
	255, 125, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 89, 225,
	21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 100, 252, 252, 244, 28, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 175, 255, 255, 232, 241, 89, 59, 245, 255, 255, 247, 34,
	0, 0, 12, 235, 255, 255, 201, 0, 59, 245, 255, 200, 241, 89, 0, 12, 235, 255, 251, 89, 0, 59,
	241, 89, 0, 0, 0, 12, 235, 255, 200, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59,
	241, 89, 59, 241, 89, 0, 59, 245, 201, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241,
	89, 59, 241, 89, 0, 59, 241, 89, 0, 0, 12, 235, 255, 247, 34, 0, 59, 245, 166, 255, 247, 34,
	0, 0, 59, 245, 255, 166, 241, 89, 59, 241, 89, 0, 0, 59, 245, 255, 255, 166, 0, 0, 138, 255,
	255, 125, 0, 89, 255, 255, 166, 241, 89, 0, 0, 138, 247, 34, 0, 0, 0, 59, 245, 125, 0, 0, 138,
	225, 21, 7, 206, 225, 21, 0, 138, 251, 0, 0, 138, 247, 34, 0, 0, 175, 255, 255, 255, 255, 166,
	0, 0, 138, 166, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
	4, 4, 4, 132, 252, 108, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0,
	0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138,
	201, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4,
	116, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 166, 255, 255,
	247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 59,
	241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255,
	201, 0, 12, 228, 34, 0, 0, 89, 255, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255,
	225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	12, 235, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 138, 201, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0, 138, 255,
	201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 251, 89, 0, 0, 138, 255, 251, 97, 206, 201, 0, 0, 138,
	251, 102, 235, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255,
	201, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 166, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 166, 12, 235, 127, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 175, 255, 255, 255, 225, 21, 59, 245, 255, 255, 255,
	255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 247, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 7, 199, 34, 0, 0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 138, 225, 21, 175, 166, 0, 0, 175, 255, 255,
	166, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 255, 201, 0, 0, 0, 0, 0, 0, 0, 0,
	59, 245, 255, 255, 255, 255, 255, 255, 255, 255, 125, 0, 59, 245, 255, 255, 255, 255, 255,
	125, 0, 0, 89, 255, 255, 255, 255, 255, 225, 21, 59, 245, 255, 255, 255, 255, 255, 125, 0,
	0, 0, 59, 245, 255, 255, 255, 255, 255, 125, 7, 206, 166, 0, 0, 175, 171, 206, 166, 89, 247,
	34, 0, 175, 201, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	12, 228, 34, 175, 255, 125, 0, 0, 89, 255, 255, 255, 125, 175, 251, 89, 89, 255, 125, 0, 7,
	206, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 255, 255,
	255, 255, 125, 0, 0, 7, 206, 255, 125, 59, 245, 125, 0, 0, 0, 89, 251, 89, 0, 0, 0, 0, 0, 0, 0,
	127, 7, 206, 225, 21, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 0, 175, 201, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 255, 255, 255, 125, 0, 59, 245, 255,
	255, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 12, 228, 34, 0, 138, 166, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 12, 235, 125, 0, 7, 176, 21, 0, 0, 0, 0, 0, 0, 138, 251, 89, 0, 0, 138, 201, 0, 0, 0,
	0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 7, 206, 166, 0, 59, 115, 0, 0, 0, 0, 59, 115,
	0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 89, 201, 0, 12, 232, 89, 89, 201, 7, 202, 89, 12, 232,
	89, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 7, 199, 34, 0, 172, 132, 196, 199, 163, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 7,
	206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 127, 89, 247, 34, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0,
	0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 0,
	0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 225, 21, 7, 202, 89, 12, 228, 34, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 130, 34, 59, 241, 89, 0, 0, 0, 138, 201, 0, 0,
	0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 175, 225, 21, 0, 59, 115, 0, 0, 0, 0, 59,
	115, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 12, 228, 34, 59, 192, 0, 12, 228, 34, 138, 166,
	59, 215, 21, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 0, 172, 89, 175, 166, 138, 125, 0, 138, 255, 255, 247, 34, 12,
	146, 0, 0, 0, 0, 89, 255, 255, 255, 125, 12, 235, 255, 255, 125, 0, 0, 0, 59, 115, 0, 0, 0, 0,
	59, 115, 0, 138, 255, 255, 255, 255, 127, 0, 175, 201, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 245, 255, 255, 255, 255, 255, 125, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 12,
	232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0, 0, 7, 199,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 12, 228, 34, 138, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	12, 235, 166, 0, 0, 0, 0, 0, 0, 175, 225, 21, 138, 225, 21, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0,
	59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0,
	0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 166,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 0, 172,
	89, 0, 0, 138, 125, 59, 238, 34, 0, 130, 34, 7, 206, 201, 0, 0, 59, 241, 89, 0, 12, 235, 255,
	125, 0, 59, 241, 89, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 89, 247, 34, 0, 59, 245, 166,
	241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 138, 225, 21, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115,
	0, 0, 0, 0, 0, 89, 255, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199,
	34, 0, 0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 255, 255, 166, 59, 215, 21, 175,
	255, 255, 125, 0, 89, 255, 255, 201, 0, 0, 0, 59, 245, 255, 255, 125, 0, 12, 235, 166, 0, 0,
	138, 225, 21, 0, 0, 0, 138, 255, 255, 255, 255, 247, 34, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0,
	0, 0, 59, 245, 125, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 251, 102, 0, 255, 255, 255, 255,
	255, 0, 245, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 127, 21, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 138, 247, 34, 138, 201, 0, 0, 0, 175,
	201, 0, 0, 0, 175, 166, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 12, 235, 125, 0, 0, 0, 138,
	255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 245, 255, 255, 255, 255, 225, 21, 0, 59, 115, 0, 0,
	0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 7, 199, 34, 0, 0, 0, 59, 245, 255, 255, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175,
	125, 89, 225, 21, 59, 238, 47, 232, 89, 7, 206, 125, 0, 0, 0, 0, 0, 138, 251, 89, 12, 235, 166,
	0, 0, 138, 225, 21, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 7,
	206, 201, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 255, 255, 201, 0,
	0, 0, 138, 247, 34, 175, 201, 0, 0, 0, 138, 255, 255, 255, 255, 255, 166, 0, 0, 59, 115, 0, 0,
	0, 0, 59, 115, 0, 0, 7, 206, 166, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 89, 251,
	89, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 59, 215, 21, 138, 201, 0, 12, 228, 47, 228, 34, 0, 175, 166, 0, 0, 0, 0, 0, 12, 232,
	89, 0, 0, 175, 225, 21, 59, 241, 89, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59,
	115, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 59, 115, 0, 0, 0, 0,
	59, 115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 166, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59,
	245, 125, 7, 206, 201, 0, 0, 138, 201, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0,
	0, 59, 115, 0, 0, 138, 225, 21, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 7, 206,
	247, 34, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 59, 245, 125, 0, 0, 175, 166, 0, 0, 0, 59,
	245, 125, 175, 225, 29, 206, 166, 0, 89, 247, 34, 7, 206, 166, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0,
	0, 7, 199, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 89, 225, 21, 59, 238, 47, 232, 89,
	7, 206, 125, 0, 89, 166, 0, 0, 89, 247, 34, 0, 0, 0, 130, 34, 0, 138, 255, 125, 0, 0, 138, 201,
	0, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 89, 251, 89, 0, 0, 0, 0, 0, 59, 115, 0, 0, 0,
	0, 59, 115, 0, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 89, 125, 0, 12, 232, 89, 12, 146, 0, 0, 0, 59, 241, 89, 0, 12, 235, 247,
	34, 0, 0, 89, 125, 0, 0, 59, 115, 0, 0, 0, 0, 59, 115, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 175, 255, 255, 255, 225, 21, 59, 245, 255, 255, 255, 255,
	255, 125, 0, 138, 225, 21, 0, 12, 235, 125, 0, 0, 0, 138, 225, 34, 235, 125, 7, 206, 166, 0,
	89, 247, 34, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	138, 201, 0, 0, 0, 175, 255, 255, 125, 0, 89, 255, 255, 201, 0, 0, 12, 235, 255, 255, 251, 89,
	0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 255, 255, 255, 255, 255, 255, 255, 125, 0, 59, 245, 255,
	255, 255, 255, 255, 125, 0, 0, 175, 255, 255, 255, 255, 255, 247, 34, 59, 245, 255, 255, 255,
	255, 255, 125, 0, 0, 0, 59, 245, 255, 255, 255, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255,
	255, 125, 59, 245, 255, 255, 201, 0, 0, 0, 59, 245, 255, 255, 255, 255, 255, 125, 0, 175, 255,
	255, 255, 255, 127, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 175, 166, 0, 255, 255, 201, 0, 0, 0, 0, 175, 166, 59, 238, 34, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 0, 0,
	0, 0, 0, 0, 0, 12, 228, 34, 138, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 245, 255, 255, 255, 255, 255,
	251, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21,
	0, 0, 0, 138, 125, 0, 0, 0, 0, 59, 245, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0,
	175, 166, 0, 12, 228, 34, 0, 0, 59, 245, 255, 255, 247, 34, 0, 89, 225, 29, 206, 166, 0, 0, 0,
	0, 0, 89, 255, 255, 255, 255, 125, 0, 0, 0, 7, 206, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 138, 255, 255, 166, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 12, 235, 255, 125, 0, 0,
	175, 255, 255, 225, 21, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 255, 255,
	255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 0, 0, 89, 255, 255, 225, 21, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 89, 225,
	21, 0, 0, 0, 12, 235, 255, 255, 166, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 138, 125, 0, 0, 0, 12,
	235, 125, 0, 59, 115, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 59, 215, 21, 0, 12, 228, 34, 0, 12,
	235, 125, 0, 0, 168, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 175, 225, 21, 0, 0,
	0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 225,
	21, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 7, 206, 125, 0, 0, 0, 7, 202, 89,
	0, 0, 0, 0, 7, 199, 34, 59, 238, 34, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 89, 255, 255, 255, 125, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 247, 34, 0,
	59, 241, 89, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 166, 0, 0, 12, 232, 89, 0, 0, 0,
	0, 7, 206, 255, 166, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 59, 215, 21, 0, 89, 201, 0, 0, 0, 0,
	0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
	206, 255, 255, 251, 89, 0, 59, 241, 89, 0, 0, 0, 138, 201, 0, 0, 0, 138, 201, 0, 0, 89, 225, 21,
	175, 125, 0, 0, 12, 228, 34, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0,
	89, 255, 255, 247, 34, 89, 201, 0, 0, 89, 255, 255, 255, 166, 0, 0, 0, 0, 168, 34, 7, 151, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 7, 206, 255, 255, 225, 21, 89, 201, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 89, 166, 0, 0, 138, 166, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0,
	7, 206, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 175, 166, 0, 175, 255, 255, 255,
	125, 175, 125, 0, 138, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 0, 89, 201, 0, 0, 89,
	225, 0, 81, 115, 0, 134, 89, 0, 0, 0, 0, 0, 138, 166, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 138, 166,
	0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 59, 245, 247, 34, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 201, 0, 7, 206, 201, 138, 125,
	138, 125, 0, 59, 241, 89, 0, 0, 0, 0, 175, 255, 255, 255, 225, 21, 0, 0, 7, 206, 166, 215, 21,
	0, 0, 12, 228, 34, 0, 0, 138, 255, 255, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 206, 21, 59, 241,
	89, 0, 134, 89, 0, 172, 89, 59, 238, 34, 0, 138, 166, 0, 0, 7, 206, 201, 12, 235, 125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 206, 21, 7, 202, 89, 12, 235, 125, 0, 172, 89, 0, 0, 0, 0,
	0, 0, 0, 0, 59, 238, 34, 7, 206, 125, 12, 235, 255, 255, 255, 255, 255, 255, 125, 0, 0, 0, 12,
	235, 125, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 175, 166, 0, 175, 255,
	255, 255, 125, 175, 125, 0, 138, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 0, 89, 201,
	0, 0, 89, 225, 0, 29, 206, 166, 59, 245, 125, 0, 0, 0, 0, 138, 166, 0, 12, 228, 34, 0, 175, 225,
	21, 0, 0, 0, 138, 166, 0, 12, 228, 42, 206, 255, 255, 166, 0, 0, 0, 0, 12, 228, 34, 138, 166,
	0, 89, 247, 34, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 138, 201, 0, 59, 241, 89, 138, 125, 0, 0, 59, 245, 255, 255, 255, 125, 0, 0, 138, 166,
	0, 89, 201, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 138, 251, 89, 0, 0, 0,
	0, 0, 0, 0, 0, 89, 166, 0, 138, 201, 0, 0, 0, 0, 0, 89, 166, 59, 215, 21, 0, 175, 166, 0, 59, 245,
	125, 89, 251, 89, 0, 0, 12, 235, 255, 255, 255, 255, 255, 255, 125, 138, 255, 255, 251, 89,
	127, 166, 0, 7, 202, 89, 12, 232, 89, 0, 89, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 255, 166,
	0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 59, 238, 34, 0, 0, 175, 166, 0, 89, 255, 255, 255, 125, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 12, 228, 34, 0, 59, 241, 89, 0, 175, 201, 0, 0, 0, 175, 201, 7, 206, 201, 0, 0, 0,
	138, 166, 0, 175, 166, 0, 138, 200, 215, 21, 0, 0, 0, 138, 166, 0, 175, 166, 7, 151, 0, 89, 247,
	34, 0, 0, 0, 59, 238, 47, 228, 34, 59, 219, 209, 34, 0, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 59, 238, 34, 138, 125, 0, 0, 0, 59,
	241, 89, 0, 0, 0, 0, 138, 166, 0, 89, 201, 0, 0, 59, 245, 255, 255, 255, 255, 125, 0, 0, 0, 0,
	0, 59, 238, 34, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 89, 166, 0, 138, 201, 0, 0, 0, 0, 0, 89, 166,
	0, 175, 255, 255, 223, 166, 0, 12, 235, 125, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125,
	0, 0, 0, 0, 0, 138, 125, 0, 7, 206, 255, 255, 125, 0, 0, 59, 157, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 7, 206, 255, 255, 255, 166, 7, 206, 255, 255, 201, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 175, 166, 0, 0, 89, 255, 255, 125, 175, 125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 251, 89, 0, 89, 255, 255, 225, 21, 0, 0, 0, 175, 225,
	29, 206, 166, 0, 0, 0, 138, 166, 59, 215, 21, 59, 215, 81, 215, 21, 0, 0, 0, 138, 166, 59, 215,
	21, 0, 0, 0, 89, 225, 21, 59, 245, 255, 255, 125, 138, 166, 7, 202, 97, 199, 34, 0, 0, 89, 251,
	89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 59, 241,
	89, 138, 125, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 175, 255, 255, 255, 225, 21, 0, 0, 0, 12, 232,
	89, 0, 0, 0, 12, 228, 34, 0, 12, 235, 225, 21, 59, 215, 21, 0, 0, 0, 0, 0, 0, 0, 0, 12, 206, 21,
	59, 241, 89, 0, 134, 89, 0, 172, 89, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 201, 12, 235, 125, 0, 0, 0,
	0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 12, 206, 21, 7, 202, 89, 7, 206, 125, 0, 172, 89, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 175, 166, 0, 0, 0, 0, 175, 125, 175, 125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 166, 59, 245, 125, 0, 0, 0, 0, 0, 0,
	175, 125, 12, 228, 34, 59, 215, 21, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0,
	59, 238, 34, 175, 125, 7, 199, 34, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225, 21, 7, 206, 201, 138, 125, 138, 125, 7, 202, 89, 0, 0, 0,
	0, 138, 201, 0, 0, 0, 138, 166, 0, 0, 0, 12, 232, 89, 0, 0, 0, 12, 228, 34, 0, 0, 0, 175, 255, 255,
	166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 89, 255, 255, 247, 34, 89, 201, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 168, 34, 7, 151, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 0, 138, 166, 7, 202,
	89, 0, 89, 247, 124, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 255,
	255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 125, 0, 7, 206,
	166, 0, 0, 0, 0, 175, 125, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 59, 115, 0, 134, 89, 0, 0, 0, 0, 0, 0, 59, 215, 21, 59, 245, 255, 255, 255, 225, 21, 0,
	0, 0, 59, 215, 21, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 175, 125, 7, 206, 255, 255, 255, 251, 89,
	0, 138, 247, 34, 0, 59, 157, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 225,
	21, 0, 7, 206, 255, 255, 251, 89, 138, 255, 255, 255, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 12, 232, 89, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175,
	225, 21, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 198,
	255, 251, 194, 166, 0, 0, 0, 0, 175, 125, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0,
	7, 206, 125, 0, 0, 12, 235, 255, 255, 255, 166, 0, 0, 0, 89, 225, 21, 0, 0, 0, 12, 228, 34, 0,
	0, 0, 175, 255, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 138, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228,
	34, 0, 7, 176, 21, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 255, 255, 125,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89,
	255, 255, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 175,
	125, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 59, 215, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 138, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	12, 228, 34, 0, 7, 206, 255, 255, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 175, 125,
	0, 0, 0, 0, 0, 0, 59, 245, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 0, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 89,
	255, 125, 0, 0, 0, 0, 0, 0, 12, 235, 201, 0, 0, 0, 0, 12, 235, 251, 89, 0, 0, 0, 0, 175, 255, 125,
	89, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 138, 251, 89, 0, 0, 0, 12, 235, 251,
	89, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 225, 21, 0, 0, 0, 89, 255, 125, 0, 89, 255, 225, 21, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 247, 34, 172, 89, 0, 0, 0, 0, 7, 206, 225, 21, 0, 0,
	0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 89, 255, 225, 21, 0, 0, 0, 0, 12, 235, 247, 34, 172,
	89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89,
	255, 125, 0, 0, 0, 0, 0, 0, 0, 7, 206, 225, 21, 0, 0, 0, 0, 89, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0,
	0, 89, 247, 34, 0, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 7, 206, 125, 59, 241, 89, 0, 0, 89, 201, 12,
	235, 247, 34, 0, 0, 7, 206, 166, 59, 241, 89, 0, 0, 12, 228, 34, 59, 215, 21, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 7,
	206, 125, 59, 241, 89, 0, 7, 206, 166, 59, 238, 34, 0, 0, 175, 166, 0, 0, 59, 241, 89, 0, 89,
	247, 34, 138, 201, 59, 238, 34, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 89, 255, 201,
	0, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 59, 238, 34, 138, 225,
	21, 0, 0, 0, 175, 125, 89, 255, 201, 0, 0, 0, 0, 59, 238, 34, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0,
	0, 59, 238, 34, 138, 225, 21, 0, 0, 0, 59, 238, 34, 138, 201, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 59, 215, 21, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 125, 0,
	0, 0, 0, 127, 0, 0, 12, 235, 225, 21, 0, 0, 0, 0, 12, 235, 225, 21, 0, 0, 0, 0, 12, 235, 225, 21,
	0, 0, 0, 0, 12, 235, 225, 21, 0, 0, 0, 0, 12, 235, 225, 21, 0, 0, 0, 0, 12, 235, 225, 21, 0, 0, 0,
	0, 0, 175, 255, 255, 255, 255, 255, 255, 255, 166, 0, 0, 138, 255, 255, 255, 251, 89, 59, 245,
	255, 255, 255, 255, 127, 81, 245, 255, 255, 255, 255, 225, 21, 59, 245, 255, 255, 255, 255,
	127, 81, 245, 255, 255, 255, 255, 127, 111, 255, 255, 255, 125, 89, 255, 255, 255, 125, 89,
	255, 255, 255, 125, 89, 255, 255, 255, 125, 7, 206, 255, 255, 255, 255, 125, 0, 0, 59, 245,
	247, 34, 0, 0, 59, 241, 89, 0, 0, 0, 138, 255, 255, 255, 166, 0, 0, 0, 0, 138, 255, 255, 255,
	166, 0, 0, 0, 0, 0, 138, 255, 255, 255, 166, 0, 0, 0, 0, 138, 255, 255, 255, 166, 0, 0, 0, 0, 138,
	255, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 255, 255, 210, 235, 166,
	59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59,
	241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 132, 245, 125, 0, 0, 0, 89, 251, 89, 12, 232, 89, 0, 0,
	0, 0, 7, 206, 166, 0, 89, 251, 89, 0, 0, 0, 127, 0, 0, 89, 232, 241, 89, 0, 0, 0, 0, 89, 232, 241,
	89, 0, 0, 0, 0, 89, 232, 241, 89, 0, 0, 0, 0, 89, 232, 241, 89, 0, 0, 0, 0, 89, 232, 241, 89, 0,
	0, 0, 0, 89, 232, 241, 89, 0, 0, 0, 0, 12, 232, 89, 89, 225, 21, 0, 0, 0, 0, 0, 175, 247, 34, 0,
	0, 59, 192, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 7, 206,
	166, 0, 0, 59, 245, 201, 0, 59, 245, 255, 201, 0, 0, 59, 241, 89, 0, 0, 138, 251, 89, 0, 12, 235,
	166, 0, 0, 138, 251, 89, 0, 12, 235, 166, 0, 0, 0, 138, 251, 89, 0, 12, 235, 166, 0, 0, 138, 251,
	89, 0, 12, 235, 166, 0, 0, 138, 251, 89, 0, 12, 235, 166, 0, 0, 0, 138, 166, 0, 0, 0, 12, 228,
	34, 0, 0, 175, 247, 34, 0, 0, 175, 225, 21, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0,
	0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 89, 247,
	34, 0, 7, 206, 125, 0, 12, 232, 89, 0, 0, 0, 0, 59, 241, 89, 0, 12, 232, 89, 0, 0, 0, 127, 0, 7,
	206, 166, 175, 166, 0, 0, 0, 7, 206, 166, 175, 166, 0, 0, 0, 7, 206, 166, 175, 166, 0, 0, 0, 7,
	206, 166, 175, 166, 0, 0, 0, 7, 206, 166, 175, 166, 0, 0, 0, 0, 175, 166, 175, 166, 0, 0, 0, 0,
	138, 225, 21, 89, 225, 21, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 59, 241,
	89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 7, 206, 166, 0, 0, 0, 59, 245, 125, 59, 241, 132,
	241, 89, 0, 59, 241, 89, 0, 12, 235, 166, 0, 0, 0, 89, 247, 34, 12, 235, 166, 0, 0, 0, 89, 247,
	34, 0, 12, 235, 166, 0, 0, 0, 89, 247, 34, 12, 235, 166, 0, 0, 0, 89, 247, 34, 12, 235, 166, 0,
	0, 0, 89, 247, 34, 0, 0, 12, 235, 125, 0, 12, 235, 125, 0, 0, 59, 241, 89, 0, 0, 138, 176, 235,
	166, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0,
	59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 175, 201, 0, 138, 225, 21, 0, 12, 235, 255,
	255, 255, 225, 21, 59, 238, 34, 0, 138, 225, 21, 0, 0, 0, 127, 0, 59, 238, 34, 89, 247, 34, 0,
	0, 59, 238, 34, 89, 247, 34, 0, 0, 59, 238, 34, 89, 247, 34, 0, 0, 59, 238, 34, 89, 247, 34, 0,
	0, 59, 238, 34, 89, 247, 34, 0, 0, 59, 241, 89, 89, 225, 21, 0, 0, 7, 206, 125, 0, 89, 225, 21,
	0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0,
	0, 59, 241, 89, 0, 7, 206, 166, 0, 0, 0, 7, 206, 166, 59, 241, 89, 138, 225, 21, 59, 241, 89,
	0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0,
	59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 12,
	235, 138, 235, 125, 0, 0, 0, 138, 225, 21, 0, 59, 215, 21, 175, 201, 59, 241, 89, 0, 0, 0, 59,
	241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0,
	0, 0, 59, 241, 89, 0, 59, 245, 166, 241, 89, 0, 0, 12, 232, 89, 0, 0, 175, 225, 59, 238, 47, 235,
	225, 21, 0, 0, 0, 0, 127, 0, 138, 201, 0, 12, 235, 125, 0, 0, 138, 201, 0, 12, 235, 125, 0, 0,
	138, 201, 0, 12, 235, 125, 0, 0, 138, 201, 0, 12, 235, 125, 0, 0, 138, 201, 0, 12, 235, 125,
	0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 89, 247, 34, 0, 89, 255, 255, 255, 255, 251, 89, 138,
	225, 21, 0, 0, 0, 0, 0, 59, 245, 255, 255, 255, 255, 127, 59, 245, 255, 255, 255, 255, 166,
	0, 59, 245, 255, 255, 255, 255, 127, 59, 245, 255, 255, 255, 255, 127, 0, 59, 241, 89, 0, 0,
	59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 7, 206, 255, 255, 255, 166, 0, 0, 175, 201,
	59, 241, 89, 12, 235, 125, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0, 12, 235, 125, 59, 241, 89, 0,
	0, 0, 12, 235, 125, 0, 59, 241, 89, 0, 0, 0, 12, 235, 125, 59, 241, 89, 0, 0, 0, 12, 235, 125,
	59, 241, 89, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 138, 225, 21, 7, 199,
	34, 0, 138, 225, 81, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241,
	89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 138, 255, 166, 0, 0, 0, 12, 232,
	89, 0, 0, 89, 247, 59, 238, 34, 0, 59, 245, 125, 0, 0, 0, 127, 7, 206, 125, 0, 0, 175, 201, 0,
	7, 206, 125, 0, 0, 175, 201, 0, 7, 206, 125, 0, 0, 175, 201, 0, 7, 206, 125, 0, 0, 175, 201, 0,
	7, 206, 125, 0, 0, 175, 201, 0, 7, 206, 125, 0, 0, 175, 201, 0, 7, 206, 255, 255, 255, 255, 225,
	21, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59,
	241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89,
	0, 0, 59, 241, 89, 0, 7, 206, 166, 0, 0, 0, 7, 206, 166, 59, 241, 89, 0, 89, 247, 94, 241, 89,
	0, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0,
	59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 0, 0, 0, 12,
	235, 138, 235, 125, 0, 0, 0, 138, 225, 21, 175, 125, 0, 0, 175, 201, 59, 241, 89, 0, 0, 0, 59,
	241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0,
	0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 0, 12, 232, 89, 0, 7, 206, 201, 59, 238, 34, 0, 0, 138,
	201, 0, 0, 0, 127, 59, 245, 255, 255, 255, 255, 247, 34, 59, 245, 255, 255, 255, 255, 247,
	34, 59, 245, 255, 255, 255, 255, 247, 34, 59, 245, 255, 255, 255, 255, 247, 34, 59, 245, 255,
	255, 255, 255, 247, 34, 59, 245, 255, 255, 255, 255, 247, 34, 59, 241, 89, 0, 0, 89, 225, 21,
	0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0,
	0, 59, 241, 89, 0, 7, 206, 166, 0, 0, 0, 59, 241, 89, 59, 241, 89, 0, 7, 206, 200, 241, 89, 0,
	12, 235, 166, 0, 0, 0, 89, 247, 34, 12, 235, 166, 0, 0, 0, 89, 247, 34, 0, 12, 235, 166, 0, 0,
	0, 89, 247, 34, 12, 235, 166, 0, 0, 0, 89, 247, 34, 12, 235, 166, 0, 0, 0, 89, 247, 34, 0, 0, 12,
	235, 125, 0, 12, 235, 125, 0, 0, 59, 241, 159, 166, 0, 0, 12, 235, 166, 12, 232, 89, 0, 0, 0,
	59, 238, 34, 12, 232, 89, 0, 0, 0, 59, 238, 34, 12, 232, 89, 0, 0, 0, 59, 238, 34, 12, 232, 89,
	0, 0, 0, 59, 238, 34, 0, 0, 59, 241, 89, 0, 0, 0, 12, 235, 255, 255, 255, 201, 0, 59, 238, 34,
	0, 0, 138, 201, 0, 0, 0, 127, 138, 201, 0, 0, 0, 12, 235, 125, 138, 201, 0, 0, 0, 12, 235, 125,
	138, 201, 0, 0, 0, 12, 235, 125, 138, 201, 0, 0, 0, 12, 235, 125, 138, 201, 0, 0, 0, 12, 235,
	125, 138, 201, 0, 0, 0, 12, 235, 125, 175, 201, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 175, 247,
	34, 0, 0, 59, 192, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59,
	241, 89, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0, 0, 59, 241, 89, 0,
	7, 206, 166, 0, 0, 59, 245, 201, 0, 59, 241, 89, 0, 0, 59, 245, 251, 89, 0, 0, 138, 251, 89, 0,
	59, 245, 166, 0, 0, 138, 251, 89, 0, 59, 245, 166, 0, 0, 0, 138, 251, 89, 0, 59, 245, 166, 0,
	0, 138, 251, 89, 0, 59, 245, 166, 0, 0, 138, 251, 89, 0, 59, 245, 166, 0, 0, 0, 138, 166, 0, 0,
	0, 12, 228, 34, 0, 0, 175, 247, 34, 0, 7, 206, 225, 21, 0, 138, 225, 21, 0, 7, 206, 166, 0, 0,
	138, 225, 21, 0, 7, 206, 166, 0, 0, 138, 225, 21, 0, 7, 206, 166, 0, 0, 138, 225, 21, 0, 7, 206,
	166, 0, 0, 0, 59, 241, 89, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 59, 238, 34, 0, 12, 235, 125, 0, 0,
	0, 127, 206, 125, 0, 0, 0, 0, 175, 206, 206, 125, 0, 0, 0, 0, 175, 206, 206, 125, 0, 0, 0, 0, 175,
	206, 206, 125, 0, 0, 0, 0, 175, 206, 206, 125, 0, 0, 0, 0, 175, 206, 206, 125, 0, 0, 0, 0, 175,
	232, 245, 125, 0, 0, 0, 89, 255, 255, 255, 255, 255, 166, 0, 0, 138, 255, 255, 255, 251, 89,
	59, 245, 255, 255, 255, 255, 127, 81, 245, 255, 255, 255, 255, 225, 21, 59, 245, 255, 255,
	255, 255, 127, 81, 245, 255, 255, 255, 255, 127, 111, 255, 255, 255, 125, 89, 255, 255, 255,
	125, 89, 255, 255, 255, 125, 89, 255, 255, 255, 125, 7, 206, 255, 255, 255, 255, 125, 0, 0,
	59, 241, 89, 0, 0, 0, 175, 251, 89, 0, 0, 0, 138, 255, 255, 255, 166, 0, 0, 0, 0, 138, 255, 255,
	255, 166, 0, 0, 0, 0, 0, 138, 255, 255, 255, 166, 0, 0, 0, 0, 138, 255, 255, 255, 166, 0, 0, 0,
	0, 138, 255, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 194, 255, 255, 255, 201,
	0, 0, 0, 0, 59, 245, 255, 251, 89, 0, 0, 0, 0, 59, 245, 255, 251, 89, 0, 0, 0, 0, 59, 245, 255,
	251, 89, 0, 0, 0, 0, 59, 245, 255, 251, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 12, 232, 89, 0, 0,
	0, 0, 59, 238, 47, 235, 255, 166, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138,
	166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 166, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 0,
	0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 7, 206,
	225, 21, 0, 0, 0, 0, 0, 12, 235, 201, 0, 0, 0, 138, 255, 201, 0, 0, 0, 59, 245, 225, 29, 202, 89,
	0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 206, 225, 21, 0, 0, 0, 0, 0, 12, 235, 201, 0, 0, 0, 89, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 175,
	247, 34, 0, 12, 235, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 225, 29, 202, 89,
	0, 0, 138, 251, 89, 0, 0, 0, 0, 0, 7, 206, 225, 21, 0, 0, 0, 89, 255, 225, 21, 0, 0, 59, 245, 225,
	29, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 225,
	21, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 89, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138,
	251, 89, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 175,
	166, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 89, 225, 21, 175, 201, 0, 7, 202, 89, 138, 255, 166, 0,
	0, 89, 247, 34, 175, 166, 0, 0, 138, 166, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 175, 166, 0, 0, 0, 89, 247, 34, 138, 201, 0, 0, 89, 247,
	34, 175, 201, 0, 0, 138, 201, 0, 175, 200, 215, 34, 235, 247, 47, 232, 0, 138, 255, 225, 111,
	225, 21, 0, 0, 172, 89, 138, 255, 166, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0,
	89, 247, 34, 138, 201, 0, 0, 172, 89, 138, 255, 166, 0, 0, 59, 238, 34, 138, 201, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 89,
	247, 34, 138, 201, 0, 0, 0, 59, 238, 34, 138, 201, 0, 0, 0, 89, 247, 34, 0, 0, 59, 238, 34, 0,
	0, 0, 0, 0, 138, 225, 29, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 201, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 225, 21, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 0, 0, 89, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 7, 206, 255, 255, 255, 125, 0, 7, 206, 255, 255, 255,
	125, 0, 7, 206, 255, 255, 255, 125, 0, 7, 206, 255, 255, 255, 125, 0, 7, 206, 255, 255, 255,
	125, 0, 12, 235, 255, 255, 251, 89, 0, 12, 235, 255, 255, 251, 89, 59, 245, 255, 166, 0, 0,
	59, 245, 255, 255, 201, 0, 12, 235, 255, 251, 89, 0, 0, 12, 235, 255, 251, 89, 0, 0, 12, 235,
	255, 251, 89, 0, 0, 12, 235, 255, 251, 89, 0, 59, 238, 34, 59, 238, 34, 59, 238, 34, 59, 238,
	34, 0, 59, 241, 89, 175, 225, 21, 0, 59, 241, 194, 255, 255, 125, 0, 0, 12, 235, 255, 247, 34,
	0, 0, 12, 235, 255, 247, 34, 0, 0, 0, 12, 235, 255, 247, 34, 0, 0, 12, 235, 255, 247, 34, 0, 0,
	12, 235, 255, 247, 34, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 7, 206, 255, 255, 225, 21, 0, 59,
	241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241,
	89, 0, 59, 241, 97, 206, 166, 0, 0, 12, 235, 125, 59, 238, 163, 255, 255, 201, 7, 206, 166,
	0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 89, 247,
	34, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 138, 225,
	21, 0, 0, 0, 0, 89, 255, 225, 21, 0, 138, 201, 59, 245, 125, 0, 0, 0, 7, 206, 125, 0, 89, 225,
	21, 7, 206, 125, 0, 89, 225, 21, 7, 206, 125, 0, 89, 225, 21, 7, 206, 125, 0, 89, 225, 21, 59,
	238, 34, 59, 238, 34, 59, 238, 34, 59, 238, 34, 0, 0, 0, 0, 12, 235, 125, 0, 59, 245, 166, 0,
	89, 247, 34, 7, 206, 166, 0, 138, 225, 21, 7, 206, 166, 0, 138, 225, 21, 0, 7, 206, 166, 0, 138,
	225, 21, 7, 206, 166, 0, 138, 225, 21, 7, 206, 166, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 12, 232, 89, 0, 138, 251, 89, 0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89,
	59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 89, 247, 34, 0, 89, 247, 34, 59,
	245, 166, 0, 7, 206, 166, 89, 247, 34, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0,
	59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0,
	59, 241, 89, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 12, 232, 89, 0, 0, 59, 238, 127, 225, 21, 0, 0,
	0, 59, 238, 34, 0, 59, 238, 34, 59, 238, 34, 0, 59, 238, 34, 59, 238, 34, 0, 59, 238, 34, 59,
	238, 34, 0, 59, 238, 34, 59, 238, 34, 59, 238, 34, 59, 238, 34, 59, 238, 34, 0, 138, 255, 255,
	255, 255, 201, 0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59,
	241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241,
	89, 0, 12, 235, 255, 255, 255, 255, 255, 255, 166, 138, 201, 0, 59, 157, 175, 201, 0, 59, 241,
	89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89,
	0, 59, 241, 89, 12, 235, 125, 0, 175, 166, 0, 59, 238, 34, 0, 0, 138, 225, 34, 235, 125, 0, 175,
	166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 89, 255, 255, 255, 251, 89, 0, 89, 255, 255, 255, 251,
	89, 0, 89, 255, 255, 255, 251, 89, 0, 89, 255, 255, 255, 251, 89, 0, 89, 255, 255, 255, 251,
	89, 0, 138, 255, 255, 255, 247, 34, 0, 175, 255, 255, 255, 255, 255, 255, 255, 255, 251, 127,
	201, 0, 0, 0, 0, 59, 245, 255, 255, 255, 251, 89, 59, 245, 255, 255, 255, 251, 89, 59, 245,
	255, 255, 255, 251, 89, 59, 245, 255, 255, 255, 251, 89, 59, 238, 34, 59, 238, 34, 59, 238,
	34, 59, 238, 34, 138, 247, 34, 0, 0, 138, 201, 0, 59, 241, 89, 0, 59, 241, 89, 59, 238, 34, 0,
	12, 232, 89, 59, 238, 34, 0, 12, 232, 89, 0, 59, 238, 34, 0, 12, 232, 89, 59, 238, 34, 0, 12,
	232, 89, 59, 238, 34, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 201, 7, 176, 21, 138, 201,
	0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59,
	241, 89, 0, 59, 241, 89, 0, 175, 201, 12, 232, 89, 0, 59, 238, 34, 0, 0, 138, 225, 21, 175, 201,
	12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59,
	241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89,
	138, 247, 34, 0, 59, 238, 34, 138, 225, 21, 0, 12, 232, 89, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 59,
	238, 34, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 59,
	238, 34, 59, 238, 34, 59, 238, 34, 59, 238, 34, 175, 201, 0, 0, 0, 138, 166, 0, 59, 241, 89,
	0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 0, 59,
	241, 89, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241, 89, 0, 0, 0, 0, 12, 235, 166, 0,
	0, 0, 138, 201, 134, 89, 0, 175, 166, 0, 59, 241, 89, 0, 59, 241, 89, 59, 241, 89, 0, 59, 241,
	89, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 0, 59, 241, 89, 0, 89, 247, 124, 225, 21, 0,
	59, 238, 34, 0, 0, 138, 201, 0, 89, 247, 124, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 89, 247,
	34, 0, 89, 251, 89, 89, 247, 34, 0, 89, 251, 89, 89, 247, 34, 0, 89, 251, 89, 89, 247, 34, 0,
	89, 251, 89, 89, 247, 34, 0, 89, 251, 89, 175, 201, 0, 0, 175, 247, 34, 175, 201, 0, 0, 59, 245,
	225, 21, 0, 7, 199, 94, 245, 125, 0, 0, 0, 7, 206, 166, 0, 7, 199, 34, 7, 206, 166, 0, 7, 199,
	34, 7, 206, 166, 0, 7, 199, 34, 7, 206, 166, 0, 7, 199, 34, 59, 238, 34, 59, 238, 34, 59, 238,
	34, 59, 238, 34, 138, 247, 34, 0, 12, 232, 89, 0, 59, 241, 89, 0, 59, 241, 89, 7, 206, 166, 0,
	138, 225, 21, 7, 206, 166, 0, 138, 225, 21, 0, 7, 206, 166, 0, 138, 225, 21, 7, 206, 166, 0,
	138, 225, 21, 7, 206, 166, 0, 138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 59, 245, 166, 0,
	59, 241, 89, 0, 12, 235, 125, 0, 175, 251, 89, 12, 235, 125, 0, 175, 251, 89, 12, 235, 125,
	0, 175, 251, 89, 0, 12, 235, 125, 0, 175, 251, 89, 0, 7, 206, 255, 125, 0, 0, 59, 238, 34, 0,
	12, 235, 125, 0, 7, 206, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 175, 255, 255, 232,
	241, 89, 0, 175, 255, 255, 232, 241, 89, 0, 175, 255, 255, 232, 241, 89, 0, 175, 255, 255,
	232, 241, 89, 0, 175, 255, 255, 232, 241, 89, 12, 235, 255, 255, 166, 238, 34, 12, 235, 255,
	255, 225, 21, 89, 255, 255, 251, 89, 0, 89, 255, 255, 255, 201, 0, 12, 235, 255, 251, 89, 0,
	0, 12, 235, 255, 251, 89, 0, 0, 12, 235, 255, 251, 89, 0, 0, 12, 235, 255, 251, 89, 0, 59, 238,
	34, 59, 238, 34, 59, 238, 34, 59, 238, 34, 0, 138, 255, 255, 255, 125, 0, 0, 59, 241, 89, 0,
	59, 241, 89, 0, 12, 235, 255, 247, 34, 0, 0, 12, 235, 255, 247, 34, 0, 0, 0, 12, 235, 255, 247,
	34, 0, 0, 12, 235, 255, 247, 34, 0, 0, 12, 235, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
	206, 255, 255, 225, 21, 0, 0, 0, 89, 255, 255, 166, 241, 89, 0, 89, 255, 255, 166, 241, 89,
	0, 89, 255, 255, 166, 241, 89, 0, 0, 89, 255, 255, 166, 241, 89, 0, 0, 138, 247, 34, 0, 0, 59,
	245, 166, 255, 255, 166, 0, 0, 0, 138, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 215, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0,
	12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 12, 235, 255, 125, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 0, 127, 127,
	0, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Large font anti-aliased
const int FONT2AA_BM_W = 276;
const int FONT2AA_BM_H = 120;
static const unsigned char s_Font2AA[] = {
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
	4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 125, 175, 225, 21,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 12, 235, 201, 89, 255, 166, 0, 0, 0, 0, 0, 172, 89, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 247, 34, 0, 12,
	232, 89, 138, 225, 21, 0, 0, 0, 0, 138, 125, 7, 199, 34, 0, 0, 0, 0, 138, 125, 0, 0, 0, 0, 138,
	255, 255, 201, 0, 0, 0, 59, 215, 21, 0, 0, 0, 0, 59, 245, 255, 255, 166, 0, 0, 0, 59, 241, 89,
	0, 7, 206, 201, 0, 0, 89, 251, 89, 0, 59, 215, 21, 172, 89, 59, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 166, 0, 0, 138, 255, 251, 89, 0, 0, 0, 0, 0,
	138, 201, 0, 0, 0, 7, 206, 255, 255, 255, 166, 0, 0, 7, 206, 255, 255, 255, 201, 0, 0, 0, 0, 0,
	0, 138, 251, 89, 0, 0, 175, 255, 255, 255, 255, 225, 21, 0, 0, 12, 235, 255, 255, 125, 89, 255,
	255, 255, 255, 255, 251, 89, 0, 12, 235, 255, 255, 225, 21, 0, 0, 59, 245, 255, 255, 166, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 59, 245, 255, 255, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
	4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247,
	34, 0, 12, 232, 89, 138, 201, 0, 0, 0, 0, 7, 202, 89, 59, 215, 21, 0, 0, 12, 235, 255, 255, 255,
	166, 0, 59, 241, 89, 12, 235, 125, 0, 0, 172, 89, 0, 0, 0, 0, 7, 206, 166, 0, 89, 251, 89, 0, 0,
	12, 228, 34, 0, 89, 247, 34, 0, 0, 0, 175, 201, 0, 0, 89, 251, 191, 194, 247, 34, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 175, 201, 0, 12, 235,
	125, 0, 0, 138, 255, 255, 201, 0, 0, 0, 12, 182, 0, 0, 59, 245, 125, 0, 12, 206, 21, 0, 12, 235,
	166, 0, 0, 0, 0, 89, 255, 251, 89, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0,
	0, 0, 0, 89, 251, 89, 12, 235, 166, 0, 7, 206, 201, 0, 59, 245, 125, 0, 12, 235, 166, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 89, 166, 0, 0, 138, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247, 34, 0, 12,
	228, 34, 89, 201, 0, 0, 0, 0, 12, 206, 21, 89, 166, 0, 0, 12, 235, 125, 138, 125, 59, 192, 0,
	89, 247, 34, 7, 206, 166, 0, 89, 201, 0, 0, 0, 0, 0, 12, 235, 125, 0, 12, 232, 89, 0, 0, 12, 228,
	34, 0, 175, 201, 0, 0, 0, 0, 59, 241, 89, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 225, 21, 59, 241, 89, 0, 0, 138, 225, 21, 0,
	0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 7, 206, 201, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 59, 241, 132,
	241, 89, 0, 0, 175, 201, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 201, 0, 59, 241,
	89, 0, 0, 138, 225, 21, 138, 225, 21, 0, 0, 138, 225, 21, 89, 255, 125, 0, 0, 89, 255, 125, 0,
	0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 0,
	0, 89, 255, 255, 255, 255, 255, 255, 255, 125, 59, 238, 34, 138, 125, 0, 0, 0, 89, 247, 34,
	7, 206, 166, 7, 202, 89, 0, 0, 0, 0, 0, 0, 175, 225, 21, 138, 225, 21, 0, 0, 0, 0, 0, 12, 235, 125,
	0, 0, 0, 0, 7, 206, 125, 0, 89, 251, 191, 194, 247, 34, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 89, 247, 34, 0, 0, 89, 247, 34, 0, 0, 0, 175, 201,
	0, 0, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 59, 245, 125, 0, 0, 12, 235, 125, 59, 241, 89,
	0, 0, 175, 201, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 12, 235, 201,
	0, 0, 175, 201, 0, 138, 225, 21, 0, 0, 89, 247, 34, 89, 255, 125, 0, 0, 89, 255, 125, 0, 0, 0,
	0, 0, 12, 235, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 255, 251, 89, 0, 0, 0, 0,
	0, 0, 0, 0, 138, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 175, 125, 7, 199, 34, 0, 0, 12, 235, 166, 138, 125, 0, 0, 0, 59, 241, 89, 12, 235,
	125, 89, 201, 12, 235, 255, 251, 89, 0, 0, 7, 206, 255, 166, 0, 59, 241, 89, 0, 0, 0, 59, 238,
	34, 0, 0, 0, 0, 0, 175, 166, 59, 215, 21, 172, 89, 59, 192, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 138, 247, 34, 0, 0, 89, 247, 34, 0, 0, 0,
	175, 201, 0, 0, 0, 0, 0, 0, 0, 89, 251, 89, 0, 0, 0, 89, 255, 247, 34, 0, 0, 7, 206, 166, 0, 59,
	241, 89, 0, 0, 175, 255, 255, 255, 225, 21, 0, 89, 251, 226, 255, 255, 247, 34, 0, 0, 0, 7, 206,
	166, 0, 0, 0, 12, 235, 255, 255, 201, 0, 0, 89, 255, 125, 0, 0, 138, 247, 34, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 89, 255, 255, 166, 0, 0, 0, 0, 0, 175, 255, 255, 255, 255, 255, 255, 225, 21, 0,
	0, 0, 0, 59, 245, 255, 201, 0, 0, 0, 0, 0, 175, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 59, 215, 21, 0, 0, 0, 59, 245, 255, 255,
	201, 0, 0, 0, 138, 255, 255, 201, 12, 228, 34, 175, 166, 0, 138, 201, 0, 12, 235, 125, 89, 255,
	125, 59, 241, 89, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 172, 89, 0, 0, 0, 7, 206,
	255, 255, 255, 255, 255, 255, 247, 34, 0, 0, 0, 0, 89, 255, 255, 255, 166, 0, 0, 0, 0, 0, 59,
	238, 34, 0, 138, 247, 34, 0, 0, 89, 247, 34, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 59, 245, 166,
	0, 0, 0, 0, 0, 0, 12, 235, 166, 0, 138, 201, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 12, 235, 201, 0, 138,
	251, 89, 0, 0, 175, 225, 21, 0, 0, 89, 247, 34, 0, 0, 7, 206, 166, 0, 175, 255, 166, 0, 0, 89,
	255, 255, 255, 223, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 225, 21, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0,
	0, 0, 127, 0, 0, 0, 0, 0, 59, 215, 21, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 255, 255, 255,
	166, 0, 0, 0, 0, 138, 125, 175, 225, 21, 0, 0, 0, 0, 0, 138, 166, 7, 206, 125, 0, 89, 247, 34,
	138, 225, 21, 0, 89, 255, 166, 215, 21, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 201, 0,
	0, 89, 247, 34, 0, 0, 89, 247, 34, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 12, 235, 201, 0, 0, 0, 0, 0,
	0, 0, 0, 138, 225, 21, 175, 255, 255, 255, 255, 255, 255, 125, 0, 0, 0, 0, 0, 138, 247, 34, 89,
	247, 34, 0, 0, 59, 241, 89, 0, 7, 206, 166, 0, 0, 0, 138, 247, 34, 0, 0, 138, 247, 34, 0, 0, 0,
	0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 201, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 166, 0, 175, 125, 0, 0, 0, 0, 0, 0, 138,
	125, 89, 247, 34, 0, 0, 0, 0, 12, 228, 34, 7, 206, 125, 0, 89, 247, 34, 138, 247, 34, 0, 0, 89,
	255, 166, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138,
	166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 59, 241, 89, 0, 0, 138, 225,
	21, 0, 0, 0, 175, 201, 0, 0, 0, 0, 12, 235, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0,
	0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 138, 225, 21, 59, 241, 89, 0, 0, 59, 241, 89, 0, 89, 247, 34,
	0, 0, 0, 138, 247, 34, 0, 0, 89, 251, 89, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 12, 235, 255, 225, 21, 0, 0, 175, 255, 255, 255, 255, 255, 255, 225, 21, 0, 0, 175,
	255, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 7, 199, 34, 0, 0, 0, 89, 201, 0, 138, 125, 175, 201, 0,
	0, 0, 0, 0, 138, 166, 0, 0, 175, 166, 0, 138, 201, 0, 89, 255, 166, 0, 0, 89, 255, 255, 125, 0,
	0, 0, 12, 235, 125, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0,
	0, 0, 138, 255, 125, 0, 0, 0, 0, 0, 0, 175, 247, 34, 59, 238, 34, 0, 0, 0, 175, 201, 0, 12, 235,
	125, 0, 0, 0, 0, 175, 201, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 89, 166, 0, 0, 59, 245, 166, 0,
	0, 0, 0, 0, 59, 241, 89, 0, 59, 215, 21, 0, 12, 235, 166, 0, 7, 206, 201, 0, 0, 175, 225, 21, 7,
	206, 166, 0, 0, 0, 0, 59, 245, 166, 0, 7, 206, 225, 21, 0, 0, 0, 0, 175, 225, 21, 0, 89, 255, 125,
	0, 0, 12, 235, 201, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 201,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 59, 215, 21, 0, 0, 0, 12, 235, 255, 255, 255, 201, 0, 0,
	0, 0, 0, 59, 215, 21, 0, 0, 12, 235, 255, 251, 89, 0, 0, 89, 255, 255, 255, 201, 0, 89, 255, 0,
	0, 0, 0, 175, 201, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175,
	201, 0, 0, 0, 0, 0, 0, 0, 175, 247, 34, 138, 201, 0, 0, 0, 0, 0, 138, 255, 251, 89, 0, 0, 0, 138,
	255, 255, 255, 255, 166, 0, 89, 255, 255, 255, 255, 255, 247, 34, 12, 235, 255, 255, 255,
	166, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 12, 235, 255, 255, 255, 166, 0, 0, 0, 7, 206, 255, 255,
	225, 21, 0, 138, 247, 34, 0, 0, 0, 0, 0, 59, 245, 255, 255, 201, 0, 0, 0, 175, 255, 255, 201,
	0, 0, 0, 89, 255, 125, 0, 0, 89, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 52, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 175, 201, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 201, 0, 0, 89, 251, 89, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 201, 0, 0, 201, 201, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 255,
	201, 0, 138, 201, 0, 0, 0, 0, 89, 255, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 127, 0, 0, 0, 0, 175, 255, 255, 255, 247, 34, 0, 0, 0, 0, 0, 7, 206, 251, 89, 0, 0, 12, 0,
	235, 255, 255, 255, 255, 201, 0, 0, 0, 0, 59, 245, 255, 255, 255, 201, 12, 0, 235, 255, 255,
	255, 255, 166, 0, 0, 0, 12, 235, 255, 255, 255, 255, 255, 127, 12, 235, 255, 255, 255, 255,
	251, 89, 0, 0, 12, 235, 255, 255, 255, 251, 89, 12, 235, 166, 0, 0, 0, 12, 235, 125, 89, 255,
	255, 255, 201, 0, 0, 175, 255, 255, 225, 21, 12, 235, 166, 0, 0, 7, 206, 251, 102, 0, 235, 166,
	0, 0, 0, 0, 12, 235, 251, 89, 0, 0, 0, 89, 255, 225, 21, 12, 235, 251, 89, 0, 0, 12, 235, 125,
	0, 0, 0, 138, 255, 255, 166, 0, 0, 0, 12, 235, 255, 255, 255, 251, 89, 0, 0, 0, 0, 175, 255, 255,
	201, 0, 0, 0, 12, 235, 255, 255, 255, 251, 89, 0, 0, 0, 12, 235, 255, 255, 255, 247, 47, 235,
	255, 255, 255, 255, 255, 255, 255, 138, 0, 235, 125, 0, 0, 0, 59, 245, 133, 206, 166, 0, 0,
	0, 0, 59, 245, 255, 133, 201, 0, 0, 0, 138, 251, 89, 0, 0, 12, 235, 133, 206, 247, 34, 0, 0, 0,
	175, 229, 216, 225, 21, 0, 0, 0, 138, 247, 124, 255, 255, 255, 255, 255, 255, 125, 7, 206,
	125, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 175, 247, 34, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 59, 245, 166, 0, 0, 0, 59, 245, 166, 0, 0, 0, 0, 59, 245, 255, 166, 0, 0,
	12, 0, 235, 166, 0, 0, 59, 245, 125, 0, 0, 138, 255, 125, 0, 0, 7, 202, 102, 0, 235, 166, 0, 0,
	59, 245, 225, 21, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 89, 255, 166,
	0, 0, 0, 89, 127, 12, 235, 166, 0, 0, 0, 12, 235, 125, 0, 12, 235, 125, 0, 0, 0, 0, 0, 138, 225,
	21, 12, 235, 166, 0, 7, 206, 225, 21, 12, 0, 235, 166, 0, 0, 0, 0, 12, 235, 255, 166, 0, 0, 7,
	206, 255, 225, 21, 12, 235, 255, 201, 0, 0, 12, 235, 125, 0, 59, 245, 166, 0, 0, 138, 251, 89,
	0, 12, 235, 166, 0, 0, 138, 251, 89, 0, 89, 255, 125, 0, 0, 89, 255, 125, 0, 12, 235, 166, 0,
	0, 138, 251, 89, 0, 12, 235, 166, 0, 0, 7, 202, 89, 0, 0, 0, 138, 225, 21, 0, 0, 12, 0, 235, 125,
	0, 0, 0, 59, 245, 125, 138, 225, 21, 0, 0, 0, 138, 225, 151, 34, 247, 34, 0, 0, 175, 255, 125,
	0, 0, 89, 247, 34, 12, 235, 166, 0, 0, 89, 247, 34, 59, 245, 125, 0, 0, 59, 245, 125, 0, 0, 0,
	0, 0, 138, 247, 34, 7, 206, 125, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 138,
	225, 187, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 12, 232, 89, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0,
	0, 138, 225, 151, 225, 21, 0, 12, 0, 235, 166, 0, 0, 12, 235, 166, 0, 12, 235, 166, 0, 0, 0, 0,
	0, 12, 0, 235, 166, 0, 0, 0, 12, 235, 166, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0,
	0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 12, 235, 125, 0, 12, 235, 125, 0, 0,
	0, 0, 0, 138, 225, 21, 12, 235, 166, 0, 175, 225, 21, 0, 12, 0, 235, 166, 0, 0, 0, 0, 12, 235,
	166, 238, 34, 0, 59, 215, 187, 225, 21, 12, 235, 166, 245, 125, 0, 12, 235, 125, 12, 235, 125,
	0, 0, 0, 0, 138, 247, 34, 12, 235, 166, 0, 0, 12, 235, 166, 12, 235, 125, 0, 0, 0, 0, 138, 247,
	34, 12, 235, 166, 0, 0, 12, 235, 166, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0,
	12, 0, 235, 125, 0, 0, 0, 59, 245, 125, 59, 241, 89, 0, 0, 7, 206, 166, 59, 0, 241, 89, 0, 12,
	232, 194, 201, 0, 0, 138, 225, 21, 0, 89, 251, 89, 12, 235, 166, 0, 0, 138, 247, 34, 7, 206,
	201, 0, 0, 0, 0, 0, 59, 245, 125, 0, 7, 206, 125, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 12, 235, 125,
	0, 0, 59, 241, 89, 12, 235, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 175, 166, 0, 59, 245, 255, 255,
	247, 34, 138, 201, 0, 0, 7, 206, 166, 59, 241, 89, 0, 12, 0, 235, 166, 0, 0, 89, 251, 89, 0, 89,
	247, 34, 0, 0, 0, 0, 0, 12, 0, 235, 166, 0, 0, 0, 0, 138, 225, 21, 12, 235, 166, 0, 0, 0, 0, 0, 12,
	235, 166, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 12, 235, 125, 0, 12,
	235, 125, 0, 0, 0, 0, 0, 138, 225, 21, 12, 235, 166, 175, 247, 34, 0, 0, 12, 0, 235, 166, 0, 0,
	0, 0, 12, 235, 133, 206, 166, 0, 175, 166, 175, 225, 21, 12, 235, 125, 138, 225, 21, 12, 235,
	125, 89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 12, 235, 166, 0, 0, 12, 235, 166, 89, 247, 34, 0,
	0, 0, 0, 59, 245, 125, 12, 235, 166, 0, 0, 12, 235, 125, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0,
	138, 225, 21, 0, 0, 12, 0, 235, 125, 0, 0, 0, 59, 245, 125, 7, 206, 201, 0, 0, 59, 241, 89, 7,
	0, 206, 166, 0, 59, 215, 111, 225, 21, 7, 206, 166, 0, 0, 0, 175, 225, 187, 225, 21, 0, 0, 12,
	235, 166, 89, 247, 34, 0, 0, 0, 0, 7, 206, 201, 0, 0, 7, 206, 125, 0, 0, 0, 0, 89, 225, 21, 0, 0,
	0, 0, 12, 235, 125, 0, 12, 235, 166, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 202, 89,
	12, 235, 125, 0, 12, 228, 34, 59, 215, 0, 0, 59, 241, 89, 7, 206, 166, 0, 12, 0, 235, 255, 255,
	255, 255, 166, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 12, 0, 235, 166, 0, 0, 0, 0, 89, 247, 34, 12,
	235, 255, 255, 255, 255, 247, 34, 12, 235, 255, 255, 255, 255, 247, 0, 163, 225, 21, 0, 0,
	0, 0, 0, 0, 12, 235, 255, 255, 255, 255, 255, 255, 125, 0, 12, 235, 125, 0, 0, 0, 0, 0, 138, 225,
	21, 12, 235, 255, 247, 34, 0, 0, 0, 12, 0, 235, 166, 0, 0, 0, 0, 12, 235, 125, 89, 225, 34, 228,
	34, 175, 225, 21, 12, 235, 125, 12, 235, 125, 12, 235, 125, 138, 225, 21, 0, 0, 0, 0, 12, 235,
	166, 12, 235, 166, 0, 0, 175, 247, 34, 138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 12, 235, 166,
	0, 0, 175, 225, 21, 0, 0, 175, 255, 255, 225, 21, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 12, 0, 235,
	125, 0, 0, 0, 59, 245, 125, 0, 138, 247, 34, 0, 138, 225, 21, 0, 0, 175, 201, 0, 138, 201, 12,
	232, 89, 12, 235, 125, 0, 0, 0, 12, 235, 251, 89, 0, 0, 0, 0, 89, 255, 255, 125, 0, 0, 0, 0, 0,
	138, 247, 34, 0, 0, 7, 206, 125, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 12, 235, 125, 7, 206, 201,
	0, 0, 0, 0, 138, 251, 89, 0, 0, 0, 0, 0, 0, 0, 127, 7, 228, 34, 89, 225, 21, 0, 12, 228, 34, 12,
	228, 0, 0, 138, 225, 21, 0, 138, 225, 21, 12, 0, 235, 166, 0, 0, 12, 235, 201, 0, 138, 225, 21,
	0, 0, 0, 0, 0, 12, 0, 235, 166, 0, 0, 0, 0, 89, 247, 34, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166,
	0, 0, 0, 0, 0, 138, 225, 21, 0, 12, 235, 255, 255, 127, 12, 235, 166, 0, 0, 0, 12, 235, 125, 0,
	12, 235, 125, 0, 0, 0, 0, 0, 138, 225, 21, 12, 235, 229, 216, 225, 21, 0, 0, 12, 0, 235, 166,
	0, 0, 0, 0, 12, 235, 125, 12, 235, 223, 201, 0, 175, 225, 21, 12, 235, 125, 0, 138, 225, 34,
	235, 125, 138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 12, 235, 255, 255, 255, 247, 34, 0, 138, 225,
	21, 0, 0, 0, 0, 12, 235, 166, 12, 235, 255, 255, 255, 166, 0, 0, 0, 0, 0, 0, 89, 255, 255, 247,
	34, 0, 0, 0, 138, 225, 21, 0, 0, 12, 0, 235, 125, 0, 0, 0, 59, 245, 125, 0, 59, 245, 125, 7, 206,
	166, 0, 0, 0, 89, 247, 34, 175, 125, 7, 206, 125, 89, 247, 34, 0, 0, 0, 12, 235, 251, 89, 0, 0,
	0, 0, 7, 206, 225, 21, 0, 0, 0, 0, 59, 245, 125, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 175, 166, 0,
	0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 12, 228, 34, 89, 225,
	21, 0, 12, 228, 34, 59, 215, 0, 7, 206, 255, 255, 255, 255, 251, 89, 12, 0, 235, 166, 0, 0, 0,
	138, 247, 0, 124, 247, 34, 0, 0, 0, 0, 0, 12, 0, 235, 166, 0, 0, 0, 0, 138, 225, 21, 12, 235, 166,
	0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 175, 127, 12, 235, 166, 0, 0,
	0, 12, 235, 125, 0, 12, 235, 125, 0, 0, 0, 0, 0, 138, 225, 21, 12, 235, 166, 59, 245, 201, 0,
	0, 12, 0, 235, 166, 0, 0, 0, 0, 12, 235, 125, 0, 138, 251, 89, 0, 175, 225, 21, 12, 235, 125,
	0, 12, 235, 138, 235, 125, 89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 12, 235, 166, 0, 0, 0, 0, 0,
	89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 12, 235, 166, 0, 175, 247, 34, 0, 0, 0, 0, 0, 0, 0, 59, 245,
	166, 0, 0, 0, 138, 225, 21, 0, 0, 12, 0, 235, 125, 0, 0, 0, 59, 241, 89, 0, 7, 206, 201, 59, 241,
	89, 0, 0, 0, 59, 241, 102, 232, 89, 0, 138, 201, 138, 225, 21, 0, 0, 0, 175, 201, 175, 225, 21,
	0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 7, 206, 201, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 89, 225, 21,
	0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 12, 232, 89, 59, 241,
	89, 0, 89, 247, 34, 89, 201, 0, 59, 241, 89, 0, 0, 7, 206, 166, 12, 0, 235, 166, 0, 0, 0, 138,
	225, 0, 81, 245, 166, 0, 0, 0, 0, 0, 12, 0, 235, 166, 0, 0, 0, 12, 235, 166, 0, 12, 235, 166, 0,
	0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 175, 127, 12, 235, 166, 0, 0,
	0, 12, 235, 125, 0, 12, 235, 125, 0, 0, 0, 0, 0, 138, 225, 21, 12, 235, 166, 0, 89, 255, 166,
	0, 12, 0, 235, 166, 0, 0, 0, 0, 12, 235, 125, 0, 12, 182, 0, 0, 175, 225, 21, 12, 235, 125, 0,
	0, 138, 232, 245, 125, 12, 235, 125, 0, 0, 0, 0, 138, 247, 34, 12, 235, 166, 0, 0, 0, 0, 0, 12,
	235, 125, 0, 0, 0, 0, 138, 247, 34, 12, 235, 166, 0, 7, 206, 225, 21, 0, 0, 0, 0, 0, 0, 12, 235,
	166, 0, 0, 0, 138, 225, 21, 0, 0, 12, 0, 235, 166, 0, 0, 0, 89, 251, 89, 0, 0, 138, 247, 163, 225,
	21, 0, 0, 0, 7, 206, 200, 215, 21, 0, 89, 225, 187, 166, 0, 0, 0, 89, 251, 89, 12, 235, 166, 0,
	0, 0, 0, 175, 225, 21, 0, 0, 0, 138, 247, 34, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 12, 232, 89,
	0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 12, 175, 166, 0, 89,
	255, 255, 210, 235, 255, 255, 125, 0, 138, 225, 21, 0, 0, 0, 138, 247, 47, 0, 235, 166, 0, 0,
	59, 245, 166, 0, 0, 138, 255, 125, 0, 0, 7, 202, 102, 0, 235, 166, 0, 0, 12, 235, 225, 21, 0,
	12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 138, 255, 125, 0, 0, 0, 175, 127, 12,
	235, 166, 0, 0, 0, 12, 235, 125, 0, 12, 235, 125, 0, 0, 0, 0, 7, 206, 201, 0, 12, 235, 166, 0,
	0, 138, 255, 125, 12, 0, 235, 166, 0, 0, 0, 12, 0, 235, 125, 0, 0, 0, 0, 0, 175, 225, 21, 12, 235,
	125, 0, 0, 12, 235, 255, 125, 0, 89, 255, 125, 0, 0, 89, 251, 89, 0, 12, 235, 166, 0, 0, 0, 0,
	0, 0, 89, 255, 125, 0, 0, 89, 255, 125, 0, 12, 235, 166, 0, 0, 12, 235, 201, 0, 138, 166, 0, 0,
	0, 138, 251, 89, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 138, 247, 34, 0, 7, 206, 225, 21, 0, 0, 12,
	235, 255, 166, 0, 0, 0, 0, 0, 175, 255, 201, 0, 0, 12, 235, 255, 125, 0, 0, 12, 235, 166, 0, 0,
	138, 251, 89, 0, 0, 0, 175, 225, 21, 0, 0, 89, 251, 89, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0,
	0, 175, 166, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 59, 241,
	89, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 59, 245, 138, 0, 235, 255, 255, 255, 255, 125,
	0, 0, 0, 0, 59, 245, 255, 255, 255, 201, 12, 0, 235, 255, 255, 255, 255, 166, 0, 0, 0, 12, 235,
	255, 255, 255, 255, 255, 127, 12, 235, 166, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 255, 255, 225,
	21, 12, 235, 166, 0, 0, 0, 12, 235, 125, 89, 255, 255, 255, 210, 127, 235, 255, 255, 225, 21,
	0, 12, 235, 166, 0, 0, 0, 175, 255, 127, 0, 235, 255, 255, 255, 247, 47, 0, 235, 125, 0, 0, 0,
	0, 0, 175, 225, 21, 12, 235, 125, 0, 0, 0, 138, 255, 125, 0, 0, 0, 175, 255, 255, 201, 0, 0, 0,
	12, 235, 166, 0, 0, 0, 0, 0, 0, 0, 0, 175, 255, 255, 201, 0, 0, 0, 12, 235, 166, 0, 0, 0, 89, 255,
	225, 34, 235, 255, 255, 255, 247, 34, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 138, 255, 255,
	255, 201, 0, 0, 0, 0, 0, 175, 251, 89, 0, 0, 0, 0, 0, 89, 255, 166, 0, 0, 7, 206, 247, 34, 0, 7,
	206, 225, 21, 0, 0, 7, 206, 225, 21, 0, 0, 175, 225, 21, 0, 0, 138, 255, 255, 255, 255, 255,
	255, 166, 7, 206, 125, 0, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 7, 206, 255, 255, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 251, 89, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 12, 235, 125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 245, 255, 255, 255, 255, 255, 255, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138,
	255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 255, 201, 0, 0, 0, 0, 0, 0, 89, 89, 255, 255,
	255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 59, 245, 166, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235,
	125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235,
	255, 247, 0, 0, 0, 0, 0, 0, 0, 12, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125,
	0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 251, 89, 0, 7, 206, 125, 0, 89, 255, 251, 89,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 84, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 7, 206,
	125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138,
	225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 12, 235, 125, 0, 0, 0,
	0, 0, 59, 245, 102, 0, 89, 247, 34, 12, 235, 125, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	59, 241, 89, 0, 0, 0, 7, 206, 125, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 4, 4, 4, 4, 4, 4, 4, 100, 252, 252, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12,
	235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 12, 0,
	235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125,
	0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235,
	125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 7, 206, 125, 0, 0, 0, 59, 238,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 20, 236, 252, 164, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 166, 0, 12, 235, 166, 245, 255, 247, 34, 0, 0, 12,
	235, 255, 255, 247, 34, 0, 34, 235, 255, 255, 255, 225, 21, 0, 12, 235, 255, 255, 225, 29,
	0, 206, 255, 255, 255, 127, 0, 12, 235, 255, 255, 255, 225, 21, 12, 235, 138, 235, 255, 247,
	34, 0, 12, 235, 102, 175, 255, 247, 34, 12, 235, 125, 0, 59, 245, 201, 0, 12, 235, 125, 12,
	0, 235, 166, 245, 255, 225, 29, 206, 255, 251, 89, 0, 12, 235, 138, 235, 255, 247, 34, 0, 0,
	12, 235, 255, 255, 201, 0, 0, 12, 235, 166, 245, 255, 251, 89, 0, 0, 12, 235, 255, 255, 255,
	225, 21, 12, 235, 138, 235, 247, 127, 34, 138, 255, 255, 255, 206, 0, 206, 255, 255, 255,
	201, 59, 241, 89, 0, 0, 89, 247, 42, 206, 201, 0, 0, 0, 138, 225, 187, 201, 0, 0, 138, 225, 21,
	0, 59, 241, 187, 226, 247, 34, 0, 7, 206, 206, 206, 201, 0, 0, 0, 138, 225, 151, 255, 255, 255,
	255, 247, 0, 0, 89, 247, 34, 0, 0, 0, 7, 206, 125, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 148, 252, 236, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 12, 206,
	21, 0, 59, 245, 125, 12, 235, 247, 34, 0, 138, 225, 21, 12, 235, 166, 0, 0, 134, 102, 0, 235,
	166, 0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 175, 201, 12, 0, 235, 125, 0, 0, 12, 235, 166, 0,
	0, 138, 225, 21, 12, 235, 247, 34, 0, 175, 201, 0, 12, 235, 102, 0, 89, 247, 34, 12, 235, 125,
	12, 235, 166, 0, 0, 12, 235, 125, 12, 0, 235, 225, 21, 12, 235, 251, 89, 0, 175, 201, 0, 12,
	235, 247, 34, 0, 175, 201, 0, 12, 235, 166, 0, 7, 206, 201, 0, 12, 235, 225, 21, 0, 175, 225,
	21, 12, 235, 166, 0, 0, 138, 225, 21, 12, 235, 247, 34, 0, 0, 89, 247, 34, 0, 12, 206, 34, 0,
	235, 125, 0, 0, 59, 241, 89, 0, 0, 89, 247, 34, 89, 247, 34, 0, 7, 206, 166, 138, 225, 21, 7,
	206, 251, 89, 0, 89, 225, 138, 34, 235, 201, 0, 138, 225, 21, 89, 247, 34, 0, 7, 206, 166, 0,
	0, 0, 7, 206, 166, 0, 0, 89, 225, 21, 0, 0, 0, 7, 206, 125, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 138,
	251, 89, 0, 0, 7, 202, 89, 0, 0, 4, 4, 4, 4, 4, 4, 52, 252, 252, 108, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 102, 12, 235, 125, 0, 0, 59, 241, 89, 138, 225, 21, 0, 0, 0, 34,
	89, 225, 21, 0, 0, 138, 225, 21, 89, 225, 21, 0, 0, 89, 247, 47, 0, 235, 125, 0, 0, 89, 225, 21,
	0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 89, 247, 34, 12, 235, 102, 0, 89, 247, 34, 12, 235, 138,
	235, 166, 0, 0, 0, 12, 235, 125, 12, 0, 235, 125, 0, 7, 206, 166, 0, 0, 138, 225, 21, 12, 235,
	125, 0, 0, 89, 247, 34, 138, 225, 21, 0, 0, 59, 238, 34, 12, 235, 125, 0, 0, 59, 241, 89, 89,
	225, 21, 0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 0, 138, 225, 21, 0, 0, 0, 12, 0, 235, 125, 0,
	0, 59, 241, 89, 0, 0, 89, 247, 34, 12, 235, 125, 0, 59, 241, 89, 59, 238, 34, 12, 228, 198, 166,
	0, 175, 166, 59, 0, 89, 251, 132, 241, 89, 0, 12, 235, 125, 0, 59, 238, 34, 0, 0, 0, 138, 225,
	21, 0, 12, 235, 166, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 175, 201, 0, 0, 0, 138, 166, 12, 235,
	166, 0, 12, 232, 89, 0, 0, 12, 84, 4, 4, 4, 4, 204, 252, 204, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 59, 245, 255, 255, 255, 102, 12, 235, 125, 0, 0, 12, 235, 125, 175, 201, 0, 0, 0, 0,
	0, 175, 201, 0, 0, 0, 138, 225, 21, 175, 255, 255, 255, 255, 255, 247, 47, 0, 235, 125, 0, 0,
	175, 201, 0, 0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 89, 247, 34, 12, 235, 102, 0, 89, 247, 34,
	12, 235, 255, 225, 21, 0, 0, 0, 12, 235, 125, 12, 0, 235, 125, 0, 7, 206, 166, 0, 0, 138, 225,
	21, 12, 235, 125, 0, 0, 89, 247, 34, 175, 201, 0, 0, 0, 12, 232, 89, 12, 235, 125, 0, 0, 12, 235,
	125, 175, 201, 0, 0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 0, 59, 245, 255, 247, 34, 0, 12, 0,
	235, 125, 0, 0, 59, 241, 89, 0, 0, 89, 247, 34, 0, 175, 201, 0, 138, 201, 0, 12, 235, 125, 89,
	201, 89, 225, 29, 206, 125, 12, 0, 0, 175, 255, 166, 0, 0, 0, 175, 201, 0, 138, 201, 0, 0, 0,
	89, 251, 89, 0, 138, 247, 34, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 89, 255, 125, 7, 202, 89,
	0, 89, 251, 89, 89, 201, 0, 0, 0, 172, 252, 84, 4, 4, 100, 252, 252, 60, 4, 4, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 89, 255, 166, 0, 7, 206, 102, 12, 235, 125, 0, 0, 12, 235, 125, 175, 201,
	0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 138, 225, 21, 175, 201, 0, 0, 0, 0, 0, 12, 0, 235, 125, 0, 0, 175,
	201, 0, 0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 89, 247, 34, 12, 235, 102, 0, 89, 247, 34, 12,
	235, 138, 235, 201, 0, 0, 0, 12, 235, 125, 12, 0, 235, 125, 0, 7, 206, 166, 0, 0, 138, 225, 21,
	12, 235, 125, 0, 0, 89, 247, 34, 175, 201, 0, 0, 0, 12, 232, 89, 12, 235, 125, 0, 0, 12, 235,
	125, 175, 201, 0, 0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 0, 0, 0, 138, 255, 255, 201, 12, 0,
	235, 125, 0, 0, 59, 241, 89, 0, 0, 89, 247, 34, 0, 89, 247, 42, 206, 125, 0, 0, 175, 166, 175,
	125, 12, 232, 102, 232, 89, 0, 0, 0, 175, 255, 201, 0, 0, 0, 89, 247, 47, 235, 125, 0, 0, 12,
	235, 166, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 138, 201, 0, 0, 12, 232, 89,
	0, 0, 59, 245, 225, 21, 0, 0, 0, 196, 252, 244, 60, 20, 236, 252, 156, 4, 4, 4, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 7, 206, 102, 12, 235, 125, 0, 0, 59, 241, 89, 138, 225, 21,
	0, 0, 0, 34, 89, 225, 21, 0, 0, 138, 225, 21, 138, 247, 34, 0, 0, 0, 0, 12, 0, 235, 125, 0, 0, 138,
	225, 21, 0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 89, 247, 34, 12, 235, 102, 0, 89, 247, 34, 12,
	235, 125, 59, 245, 125, 0, 0, 12, 235, 125, 12, 0, 235, 125, 0, 7, 206, 166, 0, 0, 138, 225,
	21, 12, 235, 125, 0, 0, 89, 247, 34, 138, 225, 21, 0, 0, 89, 247, 34, 12, 235, 125, 0, 0, 59,
	241, 89, 138, 225, 21, 0, 0, 138, 225, 21, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 89, 247, 47, 0, 235,
	125, 0, 0, 59, 241, 89, 0, 0, 89, 247, 34, 0, 12, 235, 166, 238, 34, 0, 0, 138, 210, 228, 34,
	0, 175, 166, 215, 21, 0, 0, 89, 251, 159, 251, 89, 0, 0, 12, 235, 191, 247, 34, 0, 0, 175, 225,
	21, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 7, 206, 125, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 20, 220, 252, 236, 180, 252, 244, 28, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0,
	138, 225, 21, 0, 138, 255, 102, 12, 235, 125, 0, 7, 206, 201, 0, 12, 235, 166, 0, 0, 134, 132,
	0, 245, 125, 0, 59, 245, 225, 21, 12, 235, 201, 0, 0, 12, 206, 34, 0, 235, 125, 0, 0, 59, 245,
	125, 0, 12, 235, 225, 21, 12, 235, 125, 0, 0, 89, 247, 34, 12, 235, 102, 0, 89, 247, 34, 12,
	235, 125, 0, 138, 251, 89, 0, 12, 235, 125, 12, 0, 235, 125, 0, 7, 206, 166, 0, 0, 138, 225,
	21, 12, 235, 125, 0, 0, 89, 247, 34, 12, 235, 166, 0, 7, 206, 201, 0, 12, 235, 125, 0, 7, 206,
	201, 0, 59, 245, 125, 0, 12, 235, 225, 21, 12, 235, 125, 0, 0, 0, 138, 125, 0, 0, 138, 225, 29,
	0, 206, 166, 0, 0, 7, 206, 166, 0, 59, 245, 247, 34, 0, 0, 175, 255, 201, 0, 0, 0, 59, 245, 225,
	21, 0, 89, 255, 201, 0, 0, 12, 235, 166, 0, 175, 225, 21, 0, 0, 138, 255, 166, 0, 0, 89, 251,
	89, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 7, 206, 125, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 4, 36, 236, 252, 252, 252, 108, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 206, 255, 255, 171, 206, 102, 12, 232, 226, 255, 255, 225, 21, 0, 0, 12, 235, 255, 255,
	247, 34, 0, 89, 255, 255, 247, 163, 225, 21, 0, 7, 206, 255, 255, 247, 34, 12, 0, 235, 125,
	0, 0, 0, 89, 255, 255, 247, 163, 225, 21, 12, 235, 125, 0, 0, 89, 247, 34, 12, 235, 102, 0, 89,
	247, 34, 12, 235, 125, 0, 0, 175, 251, 34, 0, 235, 125, 12, 0, 235, 125, 0, 7, 206, 166, 0, 0,
	138, 225, 21, 12, 235, 125, 0, 0, 89, 247, 34, 0, 12, 235, 255, 255, 201, 0, 0, 12, 235, 255,
	255, 255, 225, 21, 0, 0, 89, 255, 255, 247, 163, 225, 21, 12, 235, 125, 0, 0, 0, 89, 255, 255,
	255, 247, 34, 0, 0, 89, 255, 255, 127, 0, 59, 245, 255, 225, 111, 247, 34, 0, 0, 59, 245, 125,
	0, 0, 0, 12, 235, 166, 0, 0, 59, 245, 125, 7, 0, 206, 225, 21, 0, 12, 235, 201, 0, 0, 59, 241,
	89, 0, 0, 175, 255, 255, 255, 255, 247, 0, 0, 89, 247, 34, 0, 0, 0, 7, 206, 125, 0, 0, 0, 59, 238,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 60, 252, 252, 204, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	89, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 7,
	206, 125, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 76, 252, 60, 4, 4,
	4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 0, 12, 232, 89, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 12, 235, 125, 0, 0, 0, 7, 206, 125, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	12, 235, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235,
	125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89,
	247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 251, 89, 0, 7, 206, 125, 0, 89, 255, 247, 34,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 175, 166, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 206, 125, 0, 175, 166,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 225, 21, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 166, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 166, 0, 138, 201, 0, 7, 206, 166, 12, 235,
	125, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 175, 125,
	0, 0, 0, 0, 0, 175, 171, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 125, 0,
	31, 206, 130, 255, 166, 175, 247, 34, 0, 0, 89, 255, 125, 175, 247, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 247, 34, 138, 166, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 59, 241, 132, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 132, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0,
	59, 245, 255, 255, 255, 125, 0, 12, 235, 255, 255, 255, 255, 255, 225, 21, 0, 0, 0, 0, 0, 0,
	0, 0, 175, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0,
	0, 0, 175, 125, 0, 0, 0, 0, 89, 225, 21, 59, 238, 34, 0, 0, 138, 255, 255, 201, 0, 0, 0, 59, 215,
	21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255,
	255, 255, 255, 255, 255, 255, 255, 251, 89, 0, 12, 235, 255, 255, 255, 255, 255, 225, 21,
	0, 89, 255, 255, 255, 255, 255, 255, 125, 0, 12, 235, 255, 255, 255, 255, 255, 225, 21, 0,
	0, 12, 235, 255, 255, 255, 255, 255, 225, 21, 7, 206, 201, 0, 50, 206, 56, 255, 201, 12, 235,
	125, 0, 0, 138, 225, 29, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 7, 202, 89, 89, 255, 225, 21, 0, 89, 255, 255, 255, 225, 81, 245, 201, 0, 138, 251,
	89, 0, 0, 138, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255,
	255, 255, 255, 225, 21, 0, 0, 0, 138, 255, 166, 7, 206, 225, 21, 0, 0, 0, 138, 247, 34, 0, 0,
	0, 0, 127, 0, 89, 255, 125, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 0, 89,
	247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 175,
	125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 12, 235, 125, 0, 0, 172, 89, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 12, 235, 166, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 89, 255, 201, 0, 0, 12, 235, 125, 0, 0, 0,
	0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 138, 247, 34, 0, 12, 146, 0, 0, 0, 0, 0,
	144, 21, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 89, 225, 21, 71, 157, 22, 191, 225, 21, 175,
	201, 0, 7, 206, 125, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 59, 196, 199, 47, 206, 184, 89, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0,
	0, 0, 0, 59, 245, 125, 0, 0, 59, 245, 125, 0, 0, 0, 0, 0, 127, 12, 235, 166, 0, 0, 0, 0, 0, 0, 12,
	146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 255, 127, 34, 235, 255, 255, 255, 255, 225, 21, 0,
	0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 7, 206, 166, 0, 89, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34,
	0, 0, 0, 0, 0, 0, 0, 0, 59, 115, 12, 235, 166, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 12, 146, 0,
	0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 59, 245, 125, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 12,
	146, 0, 0, 0, 0, 0, 144, 21, 0, 7, 202, 89, 117, 104, 0, 29, 202, 89, 59, 215, 21, 59, 215, 21,
	138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 175, 125, 0, 59, 192, 89, 223, 125, 172, 89, 0, 138, 255, 255, 255, 201, 12, 182,
	0, 0, 0, 0, 0, 175, 255, 255, 125, 0, 89, 255, 255, 247, 34, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144,
	21, 0, 138, 255, 255, 255, 255, 247, 34, 138, 247, 34, 7, 206, 201, 0, 0, 0, 0, 0, 0, 127, 89,
	251, 89, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 7, 206, 166, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255,
	125, 0, 0, 0, 0, 0, 0, 0, 89, 255, 125, 89, 247, 34, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 12,
	146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 7, 206, 201, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0,
	0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245,
	255, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175,
	125, 0, 59, 192, 12, 228, 34, 172, 89, 89, 247, 34, 0, 12, 206, 29, 206, 201, 0, 0, 7, 206, 166,
	0, 7, 206, 255, 225, 21, 0, 89, 247, 34, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 7, 206,
	166, 0, 12, 235, 166, 89, 247, 34, 0, 0, 0, 0, 0, 0, 127, 245, 255, 255, 255, 255, 255, 201,
	0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 59, 245, 255, 255, 255, 127, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 59, 241, 89, 12, 235, 125, 89, 201, 12, 235, 255, 251, 89, 0, 89, 255, 255, 225,
	21, 0, 175, 255, 255, 225, 21, 0, 0, 0, 89, 251, 89, 0, 138, 225, 21, 0, 0, 0, 12, 235, 255, 255,
	255, 255, 225, 21, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 138, 247, 34, 0, 0, 0, 12, 146,
	0, 0, 0, 0, 0, 144, 21, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 59, 192, 0, 0, 0, 172, 89, 138, 225, 21, 0, 0, 0,
	0, 7, 206, 225, 21, 138, 225, 21, 0, 0, 89, 251, 89, 0, 0, 12, 235, 125, 0, 12, 146, 0, 0, 0, 0,
	0, 144, 21, 0, 0, 0, 0, 138, 225, 21, 0, 0, 89, 255, 255, 125, 0, 0, 0, 0, 0, 0, 0, 127, 138, 225,
	21, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 12, 235, 255, 255, 255, 255,
	225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 255, 255, 201, 12, 228, 34, 175, 166, 0, 138, 201, 7,
	206, 125, 7, 206, 166, 0, 0, 0, 89, 255, 255, 247, 34, 59, 241, 89, 0, 0, 138, 225, 21, 0, 0,
	0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 59, 245, 125, 0, 0, 0,
	0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 255, 255, 207, 235, 255, 255, 255, 255, 255, 255,
	207, 235, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 225, 21, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 247, 34, 0, 0, 0, 0, 175, 166, 175, 201, 0, 0, 0,
	59, 245, 255, 255, 255, 255, 255, 125, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 89, 251, 89,
	0, 0, 0, 7, 206, 225, 21, 0, 0, 0, 0, 0, 0, 0, 127, 245, 255, 255, 255, 255, 255, 125, 0, 0, 12,
	146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 138, 166, 7, 206, 125, 0, 89, 247, 94, 241, 89, 0, 138, 201, 0, 0, 0, 0, 0, 59, 245, 166,
	0, 89, 251, 89, 0, 89, 247, 34, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144,
	21, 0, 0, 7, 206, 201, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 12, 146, 0, 0, 0, 0, 0,
	144, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 166, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 138, 255, 255, 201, 0, 7, 206, 225, 21, 175, 201, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0,
	0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0,
	0, 0, 0, 127, 89, 255, 125, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 89,
	247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0,
	175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 228, 34, 7, 206, 125, 0, 89, 247, 94, 241,
	89, 0, 138, 201, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 89, 255, 125, 12, 235, 166, 0, 0, 0, 12, 235,
	125, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 138, 247, 34, 0, 0, 0, 0, 0, 12, 146,
	0, 0, 0, 0, 0, 144, 21, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 42, 206, 201, 0, 0, 89,
	225, 21, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 175, 225, 21,
	0, 0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 127, 0, 175, 251, 89, 0, 0, 0, 0, 0, 12, 146, 0,
	0, 0, 0, 0, 144, 21, 0, 59, 245, 166, 0, 0, 138, 225, 21, 0, 0, 0, 59, 245, 166, 138, 251, 89,
	7, 206, 201, 0, 12, 235, 125, 0, 59, 241, 89, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 175, 125, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0, 175, 166, 0, 138, 201, 7, 206, 125, 7, 206,
	166, 138, 166, 0, 0, 0, 138, 251, 89, 0, 0, 0, 59, 115, 0, 89, 255, 201, 0, 0, 12, 235, 125, 0,
	0, 0, 0, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 89, 251, 89, 0, 0, 0, 0, 0, 0, 12, 146, 0, 0, 0,
	0, 0, 144, 21, 0, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 125, 0, 0, 138, 225, 34, 182, 0, 0, 0, 7, 206, 166, 0,
	7, 206, 255, 247, 34, 0, 0, 175, 125, 0, 12, 146, 0, 0, 0, 0, 0, 144, 21, 0, 89, 251, 89, 0, 0,
	0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 138, 255, 255, 255, 255, 125, 0, 12, 235,
	255, 255, 255, 255, 255, 225, 21, 0, 138, 247, 34, 0, 7, 206, 166, 0, 0, 0, 0, 89, 247, 34, 175,
	201, 0, 7, 206, 201, 0, 12, 235, 125, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 215, 21, 0, 0, 12, 235, 255, 251, 89, 0, 89, 255, 255, 225,
	21, 12, 235, 255, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 255, 255, 255, 255,
	255, 255, 255, 251, 89, 0, 12, 235, 255, 255, 255, 255, 255, 225, 21, 0, 138, 255, 255, 255,
	255, 255, 255, 166, 0, 12, 235, 255, 255, 255, 255, 255, 225, 21, 0, 0, 12, 235, 255, 255,
	255, 255, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 89, 255, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 175, 255, 255, 125, 0, 138, 255, 255,
	255, 125, 0, 0, 12, 235, 255, 255, 255, 255, 255, 225, 21, 0, 175, 255, 255, 255, 255, 247,
	0, 0, 0, 175, 225, 21, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 175, 166, 0, 255, 255, 201, 0, 0, 0, 0, 0, 175, 166, 12, 232, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12,
	228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 59, 215, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 245, 255, 255, 255, 255, 255, 255, 225, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247,
	34, 0, 0, 0, 0, 59, 192, 0, 0, 0, 0, 0, 7, 206, 255, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 138, 247,
	34, 0, 0, 89, 251, 89, 0, 7, 206, 125, 0, 0, 7, 206, 255, 255, 255, 166, 0, 89, 251, 89, 138,
	247, 34, 0, 0, 0, 0, 7, 206, 255, 255, 255, 247, 34, 0, 0, 0, 0, 175, 255, 255, 251, 89, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 255, 255, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12,
	235, 255, 247, 34, 0, 0, 7, 206, 255, 251, 89, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 89, 255, 255, 255, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0,
	59, 245, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 201, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0,
	0, 0, 89, 201, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 0, 59, 245, 255, 201, 0, 0, 0, 59, 241, 89, 0, 0,
	0, 0, 0, 59, 245, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247, 34,
	0, 0, 0, 0, 59, 192, 0, 0, 0, 0, 0, 175, 201, 0, 0, 144, 21, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 166, 0,
	7, 206, 166, 0, 0, 7, 206, 125, 0, 7, 206, 201, 0, 0, 89, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255,
	125, 0, 0, 0, 59, 245, 166, 0, 0, 0, 0, 0, 0, 12, 206, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 59, 245, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59,
	241, 89, 0, 138, 201, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 168, 34, 7, 206, 166, 0, 0, 172, 89,
	0, 175, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 255, 166, 89, 225, 21,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 251, 89, 0, 0, 12, 235, 125, 0, 138, 225, 21, 0, 0,
	0, 0, 0, 0, 0, 0, 7, 206, 255, 201, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 7, 206, 255, 201, 0, 0, 0, 89,
	225, 21, 0, 0, 0, 0, 12, 206, 21, 12, 235, 125, 0, 0, 175, 166, 0, 0, 0, 0, 0, 0, 59, 245, 125,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 255, 255, 255,
	166, 0, 0, 12, 235, 125, 0, 0, 0, 0, 89, 225, 21, 0, 0, 12, 232, 89, 0, 89, 247, 34, 89, 247, 34,
	0, 0, 7, 206, 125, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0,
	0, 7, 206, 125, 0, 0, 7, 206, 255, 255, 247, 34, 0, 0, 0, 85, 89, 0, 85, 89, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 201,
	0, 0, 12, 228, 34, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 7, 206, 255, 166,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 89, 247, 34, 175, 255, 255, 255, 166, 89, 225, 21,
	0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 89, 225, 21, 0, 12, 232, 89, 59,
	115, 0, 59, 115, 0, 0, 0, 0, 0, 89, 201, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 89, 201, 0, 0, 7,
	206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 138, 247, 94, 192, 12, 182,
	0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 175, 255, 255, 255, 255, 166, 0, 0, 7, 206, 171, 206, 166,
	0, 0, 0, 7, 206, 125, 0, 7, 206, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 59, 245,
	255, 255, 201, 0, 12, 228, 34, 12, 235, 166, 0, 12, 228, 34, 0, 0, 138, 251, 89, 138, 247, 34,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 138, 255, 255, 255, 125, 0, 12, 228, 34,
	0, 0, 0, 0, 0, 0, 0, 0, 59, 241, 89, 0, 138, 201, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 175,
	201, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 89, 247, 34, 175, 255,
	255, 255, 166, 89, 225, 21, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 138,
	225, 21, 0, 12, 235, 125, 12, 235, 166, 59, 245, 166, 0, 0, 0, 0, 89, 201, 0, 0, 89, 225, 21,
	0, 0, 0, 0, 0, 0, 0, 89, 201, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 0, 12, 235, 255, 125, 0, 0, 175, 125,
	0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89,
	225, 21, 0, 12, 235, 125, 59, 192, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 59, 215, 21, 59, 238,
	34, 0, 0, 0, 89, 255, 247, 34, 0, 0, 0, 7, 206, 125, 0, 0, 7, 206, 255, 255, 247, 34, 0, 0, 0, 0,
	0, 0, 0, 0, 59, 192, 0, 12, 235, 166, 0, 7, 176, 21, 0, 175, 125, 59, 238, 34, 0, 12, 228, 34,
	0, 138, 247, 34, 138, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 192, 0, 0, 138,
	201, 0, 89, 247, 34, 0, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 255, 225, 21, 0, 7, 206, 255,
	255, 255, 255, 255, 255, 247, 34, 0, 12, 235, 125, 0, 0, 0, 7, 176, 21, 0, 175, 201, 0, 0, 0,
	0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 89, 247, 34, 89, 255, 255, 255, 166, 89, 225, 21, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 89, 225, 21, 0, 12, 232, 89, 0, 12, 235, 166, 12,
	235, 166, 0, 0, 0, 89, 201, 0, 7, 206, 125, 0, 12, 235, 166, 0, 0, 0, 0, 89, 201, 0, 7, 206, 125,
	89, 255, 255, 255, 125, 0, 0, 0, 0, 7, 206, 125, 89, 225, 21, 0, 138, 225, 21, 0, 0, 0, 138, 255,
	125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247, 34, 0, 59, 241, 89, 59,
	192, 0, 0, 0, 12, 235, 255, 255, 255, 225, 21, 0, 0, 138, 166, 0, 7, 202, 89, 0, 0, 0, 7, 206,
	166, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 12, 235, 201, 0, 0, 0, 0, 0, 0, 0, 0, 89, 166, 0, 89,
	247, 34, 0, 0, 0, 0, 0, 89, 166, 12, 232, 89, 0, 138, 247, 34, 89, 247, 34, 59, 238, 34, 0, 0,
	12, 235, 255, 255, 255, 255, 255, 255, 247, 34, 89, 255, 255, 255, 166, 89, 166, 0, 0, 138,
	201, 0, 138, 225, 21, 0, 89, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166,
	0, 0, 0, 0, 7, 206, 255, 255, 255, 247, 34, 0, 59, 245, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 12,
	235, 125, 0, 0, 89, 247, 34, 0, 89, 255, 255, 166, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 89, 255, 255, 255, 166, 0, 12, 235, 125, 0, 138, 225, 21, 0, 0, 12, 235, 125, 12, 235,
	125, 0, 0, 89, 201, 0, 89, 201, 0, 7, 206, 223, 166, 0, 0, 0, 0, 89, 201, 0, 89, 201, 0, 89, 125,
	0, 138, 225, 21, 12, 182, 0, 7, 206, 133, 206, 125, 0, 89, 232, 215, 21, 0, 7, 206, 247, 34,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247, 34, 0, 59, 241, 89, 59,
	192, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 59, 215, 21, 59, 238, 34, 0, 59, 245, 255, 255, 255,
	255, 225, 21, 0, 0, 0, 0, 0, 59, 241, 89, 0, 0, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 89, 166, 0, 89,
	247, 34, 0, 0, 0, 0, 0, 89, 166, 0, 138, 255, 255, 176, 228, 34, 0, 138, 247, 34, 138, 247, 34,
	0, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 89, 166, 0, 0, 138, 255, 255, 225, 21, 0, 0, 89,
	166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 89, 247, 34, 0, 0, 0, 138, 166, 89, 225,
	21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 255, 247, 34, 0, 0, 12, 235,
	166, 12, 235, 166, 0, 0, 0, 0, 0, 12, 232, 89, 0, 175, 166, 138, 166, 0, 0, 0, 0, 0, 0, 12, 232,
	89, 0, 0, 0, 0, 138, 201, 0, 0, 89, 255, 255, 201, 89, 225, 21, 89, 225, 81, 215, 21, 0, 138,
	247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247, 34, 0, 12, 235,
	125, 59, 192, 0, 0, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 175, 255, 255, 255, 255, 166, 0, 0, 0, 7,
	206, 166, 0, 0, 0, 0, 7, 206, 125, 0, 12, 235, 201, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 59, 192,
	0, 12, 235, 166, 0, 7, 176, 21, 0, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 251, 89, 138, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 59, 192, 0, 0, 138, 201, 59, 245, 166, 0, 0,
	175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 89, 247, 34, 0, 0, 0, 138, 166,
	89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 166,
	59, 245, 166, 0, 0, 0, 0, 0, 0, 138, 201, 0, 138, 201, 0, 138, 166, 0, 0, 0, 0, 0, 0, 138, 201,
	0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 7, 206, 125, 59, 238, 34, 59, 215, 21, 0, 175, 225, 21,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 175, 225, 81,
	192, 12, 182, 0, 7, 206, 125, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 12, 232, 89, 0, 0, 7, 206, 166,
	0, 0, 0, 0, 7, 206, 125, 0, 0, 59, 245, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 59,
	245, 255, 255, 166, 0, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 85, 89, 0, 85, 89, 0, 0, 0, 0, 0,
	0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 7, 202, 89, 0, 138, 201, 0, 59, 245, 225, 34, 228, 34, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 255, 255, 255, 255, 255, 247, 34, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 201, 0, 0, 175, 247, 34, 0, 0, 0, 138, 166,
	89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 115, 0,
	59, 115, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 175, 255, 255, 255, 255, 201, 0, 0, 0, 0, 12, 232, 89,
	0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 89, 201, 0, 89, 255, 255, 255, 255, 247, 34, 138, 251,
	89, 0, 7, 176, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 138, 247, 34, 0, 0, 0,
	175, 255, 255, 255, 166, 0, 89, 255, 255, 255, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 206, 166, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 138, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89,
	225, 21, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 12, 235, 191, 255, 255, 166, 238, 34, 0, 0, 0, 138, 166, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0,
	0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 89, 255, 255, 255, 255, 125, 0, 0, 0, 12, 232,
	89, 0, 0, 0, 0, 59, 215, 21, 0, 0, 138, 255, 255, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	89, 255, 125, 0, 0, 0, 59, 245, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 59, 245, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175,
	125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 192, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 125, 0, 7, 199, 34,
	0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 255, 255, 247, 34, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 255,
	255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166,
	89, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 7, 206, 125, 0, 7, 206, 255, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 255, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 138, 225,
	21, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0, 19, 172, 255, 190, 11, 0, 0, 0, 0, 138, 255, 201, 7,
	202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 138, 225, 21, 0, 0, 0, 0, 0, 0, 59, 245, 201, 0, 0, 0, 19, 172,
	255, 190, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 225, 21, 0, 0, 0, 59, 245, 201, 19, 172, 255, 190,
	11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 251, 89, 89, 201, 0, 0, 0, 0, 0, 175,
	201, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 225, 21, 0, 0, 0, 0, 0, 19, 172, 255, 190, 11, 0, 0, 0, 0, 0,
	175, 255, 166, 12, 228, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255, 125, 0, 0, 0, 0, 0, 0, 0, 12, 175, 247, 34, 0, 0, 0, 19, 172, 255,
	190, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 7, 206, 125, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 136, 190,
	45, 196, 145, 0, 0, 0, 59, 215, 21, 175, 255, 166, 0, 0, 0, 175, 225, 29, 206, 166, 0, 0, 7, 202,
	89, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 209, 125,
	0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 136, 190, 45, 196, 145, 0, 0, 0, 175, 225, 29, 206, 166,
	0, 0, 12, 235, 125, 0, 0, 12, 138, 225, 21, 136, 190, 45, 196, 145, 159, 251, 89, 138, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 125, 59, 245, 247, 34, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0,
	0, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 0, 0, 12, 136, 190, 45, 196, 145, 0, 0, 0, 0, 138, 166, 12,
	235, 255, 125, 0, 0, 0, 0, 7, 206, 166, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 232, 89, 0, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 136, 190, 45,
	196, 145, 34, 0, 0, 0, 89, 251, 89, 138, 247, 34, 0, 0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 89, 7, 202, 89, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 255,
	255, 125, 0, 0, 0, 0, 127, 0, 0, 7, 206, 251, 89, 0, 0, 0, 0, 7, 206, 251, 89, 0, 0, 0, 0, 7, 206,
	251, 89, 0, 0, 0, 0, 0, 7, 206, 251, 89, 0, 0, 0, 0, 7, 206, 251, 89, 0, 0, 0, 0, 12, 235, 255, 125,
	0, 0, 0, 0, 0, 89, 255, 255, 255, 255, 255, 255, 255, 255, 125, 0, 0, 0, 59, 245, 255, 255, 255,
	201, 12, 235, 255, 255, 255, 255, 255, 125, 12, 235, 255, 255, 255, 255, 255, 125, 12, 235,
	255, 255, 255, 255, 255, 125, 12, 235, 255, 255, 255, 255, 255, 125, 89, 255, 255, 255, 201,
	89, 255, 255, 255, 201, 89, 255, 255, 255, 201, 89, 255, 255, 255, 201, 0, 175, 255, 255,
	255, 255, 201, 0, 0, 0, 12, 235, 251, 89, 0, 0, 12, 235, 125, 0, 0, 0, 138, 255, 255, 166, 0,
	0, 0, 0, 0, 0, 138, 255, 255, 166, 0, 0, 0, 0, 0, 0, 138, 255, 255, 166, 0, 0, 0, 0, 0, 0, 138, 255,
	255, 166, 0, 0, 0, 0, 0, 0, 138, 255, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138,
	255, 255, 201, 89, 251, 89, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245,
	125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 7, 206, 225, 21,
	0, 0, 0, 138, 247, 0, 235, 166, 0, 0, 0, 0, 0, 0, 138, 225, 21, 7, 206, 166, 0, 0, 0, 127, 0, 0,
	59, 245, 255, 166, 0, 0, 0, 0, 59, 245, 255, 166, 0, 0, 0, 0, 59, 245, 255, 166, 0, 0, 0, 0, 0,
	59, 245, 255, 166, 0, 0, 0, 0, 59, 245, 255, 166, 0, 0, 0, 0, 59, 245, 255, 166, 0, 0, 0, 0, 0,
	175, 201, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 138, 255, 125, 0, 0, 7, 202, 102, 235, 166, 0, 0, 0,
	0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 12,
	235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 175, 201, 0, 0, 7,
	206, 251, 89, 0, 12, 235, 255, 201, 0, 0, 12, 235, 125, 0, 59, 245, 166, 0, 0, 138, 251, 89,
	0, 0, 59, 245, 166, 0, 0, 138, 251, 89, 0, 0, 59, 245, 166, 0, 0, 138, 251, 89, 0, 0, 59, 245,
	166, 0, 0, 138, 251, 89, 0, 0, 59, 245, 166, 0, 0, 138, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 59, 245, 166, 0, 0, 89, 255, 166, 0, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0,
	0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125,
	0, 59, 245, 125, 0, 0, 59, 245, 125, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 175, 201,
	0, 0, 0, 127, 0, 0, 138, 225, 151, 225, 21, 0, 0, 0, 138, 225, 151, 225, 21, 0, 0, 0, 138, 225,
	151, 225, 21, 0, 0, 0, 0, 138, 225, 151, 225, 21, 0, 0, 0, 138, 225, 151, 225, 21, 0, 0, 0, 138,
	225, 151, 225, 21, 0, 0, 0, 59, 241, 89, 7, 206, 166, 0, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0,
	0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235,
	166, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125,
	0, 0, 175, 201, 0, 0, 0, 0, 175, 225, 21, 12, 235, 166, 245, 125, 0, 12, 235, 125, 12, 235, 125,
	0, 0, 0, 0, 138, 247, 34, 12, 235, 125, 0, 0, 0, 0, 138, 247, 34, 12, 235, 125, 0, 0, 0, 0, 138,
	247, 34, 12, 235, 125, 0, 0, 0, 0, 138, 247, 34, 12, 235, 125, 0, 0, 0, 0, 138, 247, 34, 0, 138,
	225, 21, 0, 0, 0, 175, 201, 0, 12, 235, 125, 0, 0, 7, 202, 159, 247, 34, 12, 235, 125, 0, 0, 0,
	59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235,
	125, 0, 0, 0, 59, 245, 125, 0, 0, 138, 247, 34, 7, 206, 201, 0, 12, 235, 255, 255, 255, 251,
	89, 0, 12, 235, 125, 0, 12, 235, 125, 0, 0, 0, 127, 0, 7, 206, 166, 59, 241, 89, 0, 0, 7, 206,
	166, 59, 241, 89, 0, 0, 7, 206, 166, 59, 241, 89, 0, 0, 0, 7, 206, 166, 59, 241, 89, 0, 0, 7, 206,
	166, 59, 241, 89, 0, 0, 7, 206, 166, 59, 241, 89, 0, 0, 0, 138, 225, 21, 7, 206, 166, 0, 0, 0,
	0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12,
	235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0,
	12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 175, 201, 0, 0, 0, 0, 59, 241, 89, 12, 235, 125, 138,
	225, 21, 12, 235, 125, 89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 89, 247, 34, 0, 0, 0, 0, 59, 245,
	125, 89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 89, 247, 34,
	0, 0, 0, 0, 59, 245, 125, 0, 0, 175, 225, 21, 0, 175, 225, 21, 0, 89, 247, 34, 0, 0, 138, 166,
	12, 235, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235,
	125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 0, 0, 12, 235, 166, 89, 247,
	34, 0, 12, 235, 166, 0, 0, 138, 251, 89, 12, 235, 133, 206, 255, 125, 0, 0, 0, 0, 127, 0, 59,
	241, 89, 7, 206, 166, 0, 0, 59, 241, 89, 7, 206, 166, 0, 0, 59, 241, 89, 7, 206, 166, 0, 0, 0,
	59, 241, 89, 7, 206, 166, 0, 0, 59, 241, 89, 7, 206, 166, 0, 0, 59, 241, 89, 7, 206, 166, 0, 0,
	12, 235, 125, 0, 7, 206, 255, 255, 255, 255, 247, 34, 0, 138, 225, 21, 0, 0, 0, 0, 0, 12, 235,
	255, 255, 255, 255, 247, 34, 12, 235, 255, 255, 255, 255, 247, 34, 12, 235, 255, 255, 255,
	255, 247, 34, 12, 235, 255, 255, 255, 255, 247, 34, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0,
	0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 206, 255, 255, 255, 247, 34, 0, 12, 235, 125, 12, 235,
	125, 12, 235, 125, 12, 235, 125, 138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 138, 225, 21, 0, 0,
	0, 0, 12, 235, 166, 138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 138, 225, 21, 0, 0, 0, 0, 12, 235,
	166, 138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 175, 225, 187, 225, 21, 0, 0, 138, 225,
	21, 0, 59, 215, 21, 7, 206, 166, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59,
	245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 0, 0, 0, 89,
	255, 255, 125, 0, 0, 12, 235, 166, 0, 0, 12, 235, 166, 12, 235, 125, 0, 7, 206, 201, 0, 0, 0,
	127, 0, 138, 225, 21, 0, 138, 225, 21, 0, 138, 225, 21, 0, 138, 225, 21, 0, 138, 225, 21, 0,
	138, 225, 21, 0, 0, 138, 225, 21, 0, 138, 225, 21, 0, 138, 225, 21, 0, 138, 225, 21, 0, 138,
	225, 21, 0, 138, 225, 21, 0, 89, 255, 255, 255, 255, 255, 166, 0, 0, 0, 0, 0, 0, 138, 225, 21,
	0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0,
	0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12,
	235, 125, 0, 0, 175, 201, 0, 0, 0, 0, 12, 235, 125, 12, 235, 125, 0, 138, 225, 34, 235, 125,
	138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 138, 225, 21,
	0, 0, 0, 0, 12, 235, 166, 138, 225, 21, 0, 0, 0, 0, 12, 235, 166, 138, 225, 21, 0, 0, 0, 0, 12,
	235, 166, 0, 0, 0, 0, 175, 225, 21, 0, 0, 0, 138, 225, 21, 7, 202, 89, 0, 7, 206, 166, 12, 235,
	125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 12, 235, 125, 0, 0, 0, 59, 245,
	125, 12, 235, 125, 0, 0, 0, 59, 245, 125, 0, 0, 0, 7, 206, 225, 21, 0, 0, 12, 235, 166, 0, 0, 12,
	235, 166, 12, 235, 125, 0, 0, 59, 241, 89, 0, 0, 127, 7, 206, 255, 255, 255, 255, 251, 89, 7,
	206, 255, 255, 255, 255, 251, 89, 7, 206, 255, 255, 255, 255, 251, 89, 0, 7, 206, 255, 255,
	255, 255, 251, 89, 7, 206, 255, 255, 255, 255, 251, 89, 7, 206, 255, 255, 255, 255, 251, 89,
	7, 206, 166, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0,
	0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0,
	12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 175, 201, 0, 0,
	0, 0, 59, 241, 89, 12, 235, 125, 0, 12, 235, 138, 235, 125, 89, 247, 34, 0, 0, 0, 0, 59, 245,
	125, 89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 89, 247, 34,
	0, 0, 0, 0, 59, 245, 125, 89, 247, 34, 0, 0, 0, 0, 59, 245, 125, 0, 0, 0, 175, 225, 187, 225, 21,
	0, 0, 138, 247, 34, 175, 125, 0, 0, 12, 235, 125, 12, 235, 125, 0, 0, 0, 59, 241, 89, 12, 235,
	125, 0, 0, 0, 59, 241, 89, 12, 235, 125, 0, 0, 0, 59, 241, 89, 12, 235, 125, 0, 0, 0, 59, 241,
	89, 0, 0, 0, 0, 175, 225, 21, 0, 0, 12, 235, 166, 0, 0, 175, 247, 34, 12, 235, 125, 0, 0, 12, 235,
	125, 0, 0, 127, 59, 241, 89, 0, 0, 7, 206, 166, 59, 241, 89, 0, 0, 7, 206, 166, 59, 241, 89, 0,
	0, 7, 206, 166, 0, 59, 241, 89, 0, 0, 7, 206, 166, 59, 241, 89, 0, 0, 7, 206, 166, 59, 241, 89,
	0, 0, 7, 206, 166, 59, 241, 89, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 59, 245, 166, 0, 0, 0, 0, 0,
	12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166,
	0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0,
	175, 201, 0, 0, 0, 0, 175, 225, 21, 12, 235, 125, 0, 0, 138, 232, 245, 125, 12, 235, 125, 0,
	0, 0, 0, 138, 247, 34, 12, 235, 125, 0, 0, 0, 0, 138, 247, 34, 12, 235, 125, 0, 0, 0, 0, 138, 247,
	34, 12, 235, 125, 0, 0, 0, 0, 138, 247, 34, 12, 235, 125, 0, 0, 0, 0, 138, 247, 34, 0, 0, 175,
	225, 21, 0, 175, 225, 21, 0, 59, 245, 191, 201, 0, 0, 0, 89, 225, 21, 12, 235, 166, 0, 0, 0, 89,
	251, 89, 12, 235, 166, 0, 0, 0, 89, 251, 89, 12, 235, 166, 0, 0, 0, 89, 251, 89, 12, 235, 166,
	0, 0, 0, 89, 251, 89, 0, 0, 0, 0, 175, 225, 21, 0, 0, 12, 235, 255, 255, 255, 247, 34, 0, 12, 235,
	125, 0, 0, 59, 241, 89, 0, 0, 127, 138, 225, 21, 0, 0, 0, 138, 247, 163, 225, 21, 0, 0, 0, 138,
	247, 163, 225, 21, 0, 0, 0, 138, 247, 34, 138, 225, 21, 0, 0, 0, 138, 247, 163, 225, 21, 0, 0,
	0, 138, 247, 163, 225, 21, 0, 0, 0, 138, 247, 198, 225, 21, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0,
	0, 138, 255, 125, 0, 0, 7, 202, 102, 235, 166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235,
	166, 0, 0, 0, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 12, 235, 125, 0, 0, 12, 235,
	125, 0, 0, 12, 235, 125, 0, 0, 175, 201, 0, 0, 7, 206, 251, 89, 0, 12, 235, 125, 0, 0, 12, 235,
	255, 125, 0, 89, 255, 125, 0, 0, 89, 251, 89, 0, 0, 89, 255, 125, 0, 0, 89, 251, 89, 0, 0, 89,
	255, 125, 0, 0, 89, 251, 89, 0, 0, 89, 255, 125, 0, 0, 89, 251, 89, 0, 0, 89, 255, 125, 0, 0, 89,
	251, 89, 0, 0, 138, 225, 21, 0, 0, 0, 175, 201, 0, 0, 138, 251, 89, 0, 0, 89, 251, 89, 0, 0, 138,
	247, 34, 0, 7, 206, 225, 21, 0, 138, 247, 34, 0, 7, 206, 225, 21, 0, 138, 247, 34, 0, 7, 206,
	225, 21, 0, 138, 247, 34, 0, 7, 206, 225, 21, 0, 0, 0, 0, 175, 225, 21, 0, 0, 12, 235, 166, 0,
	0, 0, 0, 0, 12, 235, 125, 0, 0, 175, 225, 21, 0, 0, 127, 206, 166, 0, 0, 0, 0, 59, 245, 255, 166,
	0, 0, 0, 0, 59, 245, 255, 166, 0, 0, 0, 0, 59, 245, 133, 206, 166, 0, 0, 0, 0, 59, 245, 255, 166,
	0, 0, 0, 0, 59, 245, 255, 166, 0, 0, 0, 0, 59, 245, 255, 125, 0, 0, 0, 7, 206, 255, 255, 255, 255,
	255, 125, 0, 0, 0, 59, 245, 255, 255, 255, 201, 12, 235, 255, 255, 255, 255, 255, 125, 12,
	235, 255, 255, 255, 255, 255, 125, 12, 235, 255, 255, 255, 255, 255, 125, 12, 235, 255, 255,
	255, 255, 255, 125, 89, 255, 255, 255, 201, 89, 255, 255, 255, 201, 89, 255, 255, 255, 201,
	89, 255, 255, 255, 201, 0, 175, 255, 255, 255, 255, 225, 21, 0, 0, 12, 235, 125, 0, 0, 0, 138,
	255, 125, 0, 0, 0, 175, 255, 255, 201, 0, 0, 0, 0, 0, 0, 175, 255, 255, 201, 0, 0, 0, 0, 0, 0, 175,
	255, 255, 201, 0, 0, 0, 0, 0, 0, 175, 255, 255, 201, 0, 0, 0, 0, 0, 0, 175, 255, 255, 201, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 202, 97, 206, 255, 255, 201, 0, 0, 0, 0, 0, 138, 255, 255, 255,
	201, 0, 0, 0, 0, 138, 255, 255, 255, 201, 0, 0, 0, 0, 138, 255, 255, 255, 201, 0, 0, 0, 0, 138,
	255, 255, 255, 201, 0, 0, 0, 0, 0, 0, 175, 225, 21, 0, 0, 12, 235, 166, 0, 0, 0, 0, 0, 12, 235,
	133, 206, 255, 225, 21, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 138, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 225, 21,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 7, 206, 225, 21,
	0, 0, 0, 0, 0, 12, 235, 225, 21, 0, 0, 89, 255, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 7, 206, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 138, 251, 89, 0, 0, 59, 245, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 247,
	34, 0, 0, 175, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 225, 21,
	0, 0, 0, 0, 0, 0, 0, 138, 255, 125, 0, 0, 0, 12, 235, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 251, 89, 0, 0, 0, 0, 0, 0,
	7, 206, 225, 21, 0, 0, 0, 7, 206, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59, 245, 166, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 59, 241, 89, 0, 0, 0, 0,
	0, 89, 247, 34, 0, 0, 7, 206, 138, 235, 125, 0, 0, 89, 255, 225, 21, 175, 125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 138, 201, 0, 138, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12,
	235, 125, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 175, 171, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 206, 166, 0, 59, 245, 255, 166, 238, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 125, 59,
	215, 21, 0, 0, 59, 241, 89, 0, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 138, 201, 175, 201, 0, 0,
	0, 12, 235, 251, 89, 89, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 0, 0, 89, 225, 151, 201, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 201, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 127, 0, 0, 0, 138, 201, 0, 0, 0, 0, 7, 206, 125, 0, 0, 0, 138, 201, 0, 89, 225, 21,
	12, 228, 34, 138, 255, 201, 0, 0, 0, 138, 247, 34, 175, 225, 21, 0, 138, 201, 0, 138, 201, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 0, 0, 89, 225,
	21, 0, 0, 89, 247, 34, 59, 241, 89, 0, 59, 241, 89, 89, 247, 34, 0, 0, 89, 225, 21, 175, 127,
	215, 21, 206, 247, 42, 206, 0, 138, 255, 247, 42, 206, 125, 0, 0, 138, 166, 12, 235, 251, 89,
	0, 0, 0, 0, 138, 201, 0, 0, 0, 0, 0, 0, 89, 225, 21, 0, 0, 0, 59, 241, 89, 12, 235, 125, 0, 0, 175,
	125, 59, 245, 247, 34, 0, 0, 12, 235, 125, 89, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 175, 166, 0, 0, 0, 0, 12, 232, 89, 7, 206, 125, 0,
	0, 12, 235, 166, 59, 245, 125, 0, 0, 0, 59, 238, 34, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 89, 247,
	34, 138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 247, 34, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 251, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 199, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 235, 125, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 12, 235, 255, 255, 255, 166, 0, 12, 235, 255,
	255, 255, 166, 0, 12, 235, 255, 255, 255, 166, 0, 12, 235, 255, 255, 255, 166, 0, 0, 12, 235,
	255, 255, 255, 166, 0, 12, 235, 255, 255, 255, 166, 0, 12, 235, 255, 255, 255, 166, 0, 175,
	255, 255, 125, 0, 0, 12, 235, 255, 255, 125, 0, 0, 12, 235, 255, 255, 225, 21, 0, 0, 12, 235,
	255, 255, 225, 21, 0, 12, 235, 255, 255, 225, 21, 0, 12, 235, 255, 255, 225, 21, 0, 12, 235,
	125, 12, 235, 125, 12, 235, 125, 12, 235, 125, 0, 12, 235, 125, 89, 251, 89, 0, 12, 235, 138,
	235, 255, 247, 34, 0, 0, 12, 235, 255, 255, 201, 0, 0, 0, 12, 235, 255, 255, 201, 0, 0, 0, 12,
	235, 255, 255, 201, 0, 0, 0, 12, 235, 255, 255, 201, 0, 0, 0, 12, 235, 255, 255, 201, 0, 0, 0,
	0, 0, 0, 175, 247, 34, 0, 0, 0, 12, 235, 255, 255, 255, 166, 0, 59, 241, 89, 0, 0, 89, 247, 34,
	59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 42,
	206, 201, 0, 0, 0, 138, 232, 245, 166, 245, 255, 251, 89, 7, 206, 201, 0, 0, 0, 138, 225, 21,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 12, 206, 21, 0, 59, 245, 125, 12, 206, 21, 0, 59, 245, 125, 12, 206,
	21, 0, 59, 245, 125, 12, 206, 21, 0, 59, 245, 125, 0, 12, 206, 21, 0, 59, 245, 125, 12, 206,
	21, 0, 59, 245, 125, 12, 206, 21, 0, 12, 235, 255, 125, 0, 7, 206, 166, 12, 235, 166, 0, 0, 172,
	102, 0, 235, 125, 0, 0, 175, 201, 0, 12, 235, 125, 0, 0, 175, 201, 12, 235, 125, 0, 0, 175, 201,
	12, 235, 125, 0, 0, 175, 201, 0, 12, 235, 125, 12, 235, 125, 12, 235, 125, 12, 235, 125, 0,
	0, 0, 0, 0, 175, 201, 0, 12, 235, 247, 34, 0, 175, 201, 0, 12, 235, 166, 0, 7, 206, 201, 0, 12,
	235, 166, 0, 7, 206, 201, 0, 12, 235, 166, 0, 7, 206, 201, 0, 12, 235, 166, 0, 7, 206, 201, 0,
	12, 235, 166, 0, 7, 206, 201, 0, 0, 0, 0, 0, 175, 247, 34, 0, 0, 12, 235, 166, 0, 12, 235, 201,
	0, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247,
	34, 59, 241, 89, 0, 0, 89, 247, 34, 89, 247, 34, 0, 7, 206, 176, 235, 225, 21, 0, 175, 225, 21,
	89, 247, 34, 0, 7, 206, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 7,
	206, 166, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 0, 0,
	7, 206, 166, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 89, 225, 138, 225, 21, 0, 0, 0, 0, 89, 225, 21, 0,
	0, 89, 247, 34, 89, 225, 21, 0, 0, 89, 247, 124, 225, 21, 0, 0, 89, 247, 124, 225, 21, 0, 0, 89,
	247, 34, 12, 235, 125, 12, 235, 125, 12, 235, 125, 12, 235, 125, 0, 89, 255, 255, 255, 255,
	247, 34, 12, 235, 125, 0, 0, 89, 247, 34, 138, 225, 21, 0, 0, 59, 238, 34, 138, 225, 21, 0, 0,
	59, 238, 34, 138, 225, 21, 0, 0, 59, 238, 34, 138, 225, 21, 0, 0, 59, 238, 34, 138, 225, 21,
	0, 0, 59, 238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 172, 132, 238, 34, 59, 241, 89, 0,
	0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0,
	0, 89, 247, 34, 12, 235, 125, 0, 59, 238, 47, 235, 125, 0, 0, 59, 241, 89, 12, 235, 125, 0, 59,
	238, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 59, 245, 255, 255, 255, 166, 0, 59, 245, 255, 255,
	255, 166, 0, 59, 245, 255, 255, 255, 166, 0, 59, 245, 255, 255, 255, 166, 0, 0, 59, 245, 255,
	255, 255, 166, 0, 59, 245, 255, 255, 255, 166, 0, 89, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 247, 175, 201, 0, 0, 0, 0, 0, 175, 255, 255, 255, 255, 255, 247, 34, 175, 255, 255, 255,
	255, 255, 247, 198, 255, 255, 255, 255, 255, 247, 198, 255, 255, 255, 255, 255, 247, 34,
	12, 235, 125, 12, 235, 125, 12, 235, 125, 12, 235, 125, 89, 251, 89, 0, 0, 59, 241, 89, 12,
	235, 125, 0, 0, 89, 247, 34, 175, 201, 0, 0, 0, 12, 232, 89, 175, 201, 0, 0, 0, 12, 232, 89, 175,
	201, 0, 0, 0, 12, 232, 89, 175, 201, 0, 0, 0, 12, 232, 89, 175, 201, 0, 0, 0, 12, 232, 89, 7, 206,
	255, 255, 255, 255, 255, 255, 251, 226, 201, 0, 89, 166, 12, 232, 89, 59, 241, 89, 0, 0, 89,
	247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89,
	247, 34, 0, 175, 201, 0, 138, 201, 12, 235, 125, 0, 0, 12, 235, 125, 0, 175, 201, 0, 138, 201,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 89, 255, 166, 0, 7, 206, 166, 89, 255, 166, 0, 7, 206, 166, 89,
	255, 166, 0, 7, 206, 166, 89, 255, 166, 0, 7, 206, 166, 0, 89, 255, 166, 0, 7, 206, 166, 89,
	255, 166, 0, 7, 206, 166, 138, 255, 125, 0, 0, 175, 201, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0,
	175, 201, 0, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0, 0, 175, 201, 0, 0, 0, 0,
	0, 0, 12, 235, 125, 12, 235, 125, 12, 235, 125, 12, 235, 125, 175, 201, 0, 0, 0, 59, 241, 89,
	12, 235, 125, 0, 0, 89, 247, 34, 175, 201, 0, 0, 0, 12, 232, 89, 175, 201, 0, 0, 0, 12, 232, 89,
	175, 201, 0, 0, 0, 12, 232, 89, 175, 201, 0, 0, 0, 12, 232, 89, 175, 201, 0, 0, 0, 12, 232, 89,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 175, 201, 7, 176, 21, 12, 232, 89, 59, 241, 89, 0, 0, 89, 247, 34, 59,
	241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 0,
	89, 247, 47, 235, 125, 12, 235, 125, 0, 0, 12, 235, 125, 0, 89, 247, 47, 235, 125, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 127, 175, 201, 0, 0, 7, 206, 166, 175, 201, 0, 0, 7, 206, 166, 175, 201, 0, 0,
	7, 206, 166, 175, 201, 0, 0, 7, 206, 166, 0, 175, 201, 0, 0, 7, 206, 166, 175, 201, 0, 0, 7, 206,
	166, 175, 201, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 138, 225, 21, 0, 0, 0, 0, 138, 247, 34, 0, 0,
	0, 0, 0, 138, 247, 34, 0, 0, 0, 0, 138, 247, 34, 0, 0, 0, 0, 138, 247, 34, 0, 0, 0, 0, 0, 12, 235,
	125, 12, 235, 125, 12, 235, 125, 12, 235, 125, 175, 201, 0, 0, 0, 89, 247, 34, 12, 235, 125,
	0, 0, 89, 247, 34, 138, 225, 21, 0, 0, 89, 247, 34, 138, 225, 21, 0, 0, 89, 247, 34, 138, 225,
	21, 0, 0, 89, 247, 34, 138, 225, 21, 0, 0, 89, 247, 34, 138, 225, 21, 0, 0, 89, 247, 34, 0, 0,
	0, 0, 175, 247, 34, 0, 0, 138, 225, 151, 125, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34,
	59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34, 59, 241, 89, 0, 0, 89, 247, 34,
	0, 12, 235, 191, 247, 34, 12, 235, 125, 0, 0, 59, 241, 89, 0, 12, 235, 191, 247, 34, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 127, 138, 225, 21, 0, 138, 255, 166, 138, 225, 21, 0, 138, 255, 166, 138,
	225, 21, 0, 138, 255, 166, 138, 225, 21, 0, 138, 255, 166, 0, 138, 225, 21, 0, 138, 255, 166,
	138, 225, 21, 0, 138, 255, 166, 89, 247, 34, 0, 89, 255, 255, 166, 0, 0, 12, 206, 12, 235, 166,
	0, 0, 127, 102, 0, 235, 201, 0, 0, 12, 206, 21, 12, 235, 201, 0, 0, 12, 206, 34, 235, 201, 0,
	0, 12, 206, 34, 235, 201, 0, 0, 12, 206, 21, 12, 235, 125, 12, 235, 125, 12, 235, 125, 12, 235,
	125, 89, 255, 125, 0, 7, 206, 166, 0, 12, 235, 125, 0, 0, 89, 247, 34, 12, 235, 166, 0, 7, 206,
	201, 0, 12, 235, 166, 0, 7, 206, 201, 0, 12, 235, 166, 0, 7, 206, 201, 0, 12, 235, 166, 0, 7,
	206, 201, 0, 12, 235, 166, 0, 7, 206, 201, 0, 0, 0, 0, 0, 175, 247, 34, 0, 0, 12, 235, 201, 0,
	7, 206, 201, 0, 7, 206, 166, 0, 59, 245, 247, 34, 7, 206, 166, 0, 59, 245, 247, 34, 7, 206, 166,
	0, 59, 245, 247, 34, 7, 206, 166, 0, 59, 245, 247, 34, 0, 0, 138, 255, 166, 0, 12, 235, 125,
	0, 7, 206, 201, 0, 0, 0, 138, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 7, 206, 255, 255, 171,
	206, 166, 7, 206, 255, 255, 171, 206, 166, 7, 206, 255, 255, 171, 206, 166, 7, 206, 255, 255,
	171, 206, 166, 0, 7, 206, 255, 255, 171, 206, 166, 7, 206, 255, 255, 171, 206, 166, 0, 89,
	255, 255, 201, 0, 0, 175, 255, 255, 247, 34, 0, 12, 235, 255, 255, 166, 0, 0, 7, 206, 255, 255,
	247, 34, 0, 0, 7, 206, 255, 255, 247, 34, 0, 7, 206, 255, 255, 247, 34, 0, 7, 206, 255, 255,
	247, 34, 0, 12, 235, 125, 12, 235, 125, 12, 235, 125, 12, 235, 125, 0, 89, 255, 255, 255, 201,
	0, 0, 12, 235, 125, 0, 0, 89, 247, 34, 0, 12, 235, 255, 255, 201, 0, 0, 0, 12, 235, 255, 255,
	201, 0, 0, 0, 12, 235, 255, 255, 201, 0, 0, 0, 12, 235, 255, 255, 201, 0, 0, 0, 12, 235, 255,
	255, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 255, 255, 255, 201, 0, 0, 0, 59, 245, 255, 225,
	111, 247, 34, 0, 59, 245, 255, 225, 111, 247, 34, 0, 59, 245, 255, 225, 111, 247, 34, 0, 59,
	245, 255, 225, 111, 247, 34, 0, 0, 59, 241, 89, 0, 12, 235, 255, 255, 255, 225, 21, 0, 0, 0,
	59, 241, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 175, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 138, 225, 21, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 0,
	138, 225, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 7, 202, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 206, 166, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 7, 206,
	166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	12, 235, 255, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 247, 34, 0, 0, 12, 235, 125, 0, 0, 0, 0, 0, 0, 89, 247,
	34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 0, 127, 127, 0, 127, 127, 0, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0
};

const int FONTFIXED1_BM_W = 257;
const int FONTFIXED1_BM_H = 112;

static const unsigned char s_FontFixed1[] = {
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 56, 255, 4, 0, 0,
	0, 0, 0, 212, 44, 76, 180, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 109, 231, 218, 72, 0, 0, 0, 0, 0, 96, 227,
	243, 170, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 158, 104, 0, 0, 0, 0, 153, 114, 0, 0, 0, 0, 0, 0, 0,
	52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 172, 128, 0, 0, 12, 164, 241, 234, 133, 1, 0, 0, 22, 179, 237, 255, 4, 0, 0, 0, 141, 220, 246, 236, 164, 22, 0, 0, 94, 216, 242, 243, 194, 56, 0, 0, 0, 0, 0, 186, 255, 4, 0, 0, 52, 255, 244,
	244, 244, 91, 0, 0, 1, 120, 223, 244, 225, 62, 0, 0, 244, 244, 244, 244, 249, 242, 0, 0, 62, 200,
	245, 242, 181, 35, 0, 0, 46, 196, 244, 232, 139, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 104, 216, 246, 215, 62, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 56, 255, 4, 0, 0, 0, 0, 13, 239, 2, 131, 124, 0,
	0, 110, 232, 255, 238, 202, 62, 0, 29, 254, 51, 99, 231, 0, 0, 0, 0, 0, 241, 53, 0, 34, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 0, 45, 225, 4, 0, 0, 0, 0, 30, 237, 15, 0, 0, 0, 0, 99, 95, 52, 255, 11, 127, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 242,
	19, 0, 0, 155, 188, 12, 29, 221, 103, 0, 0, 21, 101, 90, 255, 4, 0, 0, 0, 127, 46, 1, 15, 165, 192, 0,
	0, 34, 24, 0, 5, 127, 233, 0, 0, 0, 0, 98, 197, 255, 4, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 121, 219, 45, 0,
	22, 27, 0, 0, 0, 0, 0, 0, 170, 151, 0, 14, 242, 119, 4, 12, 160, 207, 0, 3, 224, 136, 5, 18, 188, 114,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 114, 38, 2, 133, 225, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255,
	56, 255, 4, 0, 0, 0, 0, 67, 189, 0, 187, 69, 0, 26, 254, 100, 255, 8, 53, 44, 0, 30, 254, 49, 100, 235, 0, 1, 0, 0, 0, 206, 47, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 155, 133, 0, 0, 0, 0, 0, 0, 186,
	110, 0, 0, 0, 0, 3, 103, 195, 255, 177, 75, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 156, 144, 0, 0, 5, 244, 63, 0, 0, 112, 200, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 0, 0, 60, 251, 0, 0, 0, 0, 0, 10, 127, 211, 0, 0, 0, 25, 215, 67, 255, 4, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 1, 235, 77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 250, 49, 0, 44, 255, 15, 0, 0, 64, 251, 0,
	41, 255, 17, 0, 0, 68, 205, 0, 0, 0, 43, 216, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 47, 147, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 197, 97, 11, 0, 0, 0, 0, 0, 0, 0, 0, 69, 248, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 240, 247, 248, 240, 254, 241, 0, 42, 255, 69, 255, 4, 0, 0, 0, 0, 112, 232, 221, 80, 97, 184, 0, 0, 14, 189, 196, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	233, 64, 0, 0, 0, 0, 0, 0, 117, 186, 0, 0, 0, 0, 3, 102, 194, 255, 177, 74, 0, 0, 0, 0, 52, 255, 4, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 244, 30, 0, 0, 37, 255, 37, 175, 0, 66, 244, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 126, 195, 0, 0, 0, 48, 241, 255, 190, 16,
	0, 0, 0, 176, 90, 52, 255, 4, 0, 0, 52, 255, 228, 236, 162, 16, 0, 33, 255, 106, 220, 237, 172, 21, 0, 0, 0, 0, 0, 125, 203, 0, 0, 4, 205, 120, 6, 14, 160, 159, 0, 40, 255, 21, 0, 0, 69, 245, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 13, 102, 203, 225, 132, 0, 0, 240, 240, 240, 240, 240, 240, 0, 0, 84, 180, 237, 152, 52, 0, 0, 0, 0, 0, 11, 204, 150, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 190, 66, 54, 202, 0, 0, 2, 196, 220, 255, 106, 32, 0, 0, 0, 0, 13, 116, 184, 93, 4, 0, 0, 176, 114, 109, 159, 0, 52, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 29, 255, 22, 0, 0, 0, 0, 0, 0, 72, 236, 0, 0, 0, 0, 99, 96, 52, 255, 11, 128, 0, 0, 240, 240, 243, 255, 240, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 140, 161, 0, 0, 0, 49, 255,
	33, 216, 0, 54, 255, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 68, 236, 45, 0, 0, 0, 0, 0, 27, 198, 133, 0, 0, 87, 189, 0, 52, 255, 4, 0, 0, 19, 40, 0, 29, 197, 168, 0, 49, 255, 146, 9, 15, 182, 176, 0, 0, 0, 0, 3, 226, 100, 0, 0, 0, 28, 210, 252, 255, 182, 7, 0, 2, 223, 142, 6, 20, 186, 255, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 43, 216, 3, 0, 0, 0, 0, 158, 233, 162, 67, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 115,
	210, 208, 0, 0, 0, 0, 176, 192, 6, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 7, 239, 9, 118, 138, 0, 0, 0, 5, 109, 255, 153, 234, 112, 0, 0, 88, 179, 76, 110, 231,
	220, 0, 28, 255, 16, 0, 176, 110, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 46, 255, 8, 0, 0, 0, 0, 0, 0, 57,
	253, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 244, 244,
	244, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 241, 43, 0, 0, 0, 37, 255, 17, 0, 0, 66, 244, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 73, 233, 60, 0, 0, 0, 0, 0, 0, 0, 69, 238, 0, 16, 227, 40, 0, 52, 255, 4, 0, 0, 0,
	0, 0, 0, 69, 246, 0, 38, 255, 20, 0, 0, 69, 247, 0, 0, 0, 0, 79, 242, 11, 0, 0, 5, 214, 122, 7, 16, 162,
	175, 0, 0, 46, 193, 239, 210, 125, 241, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 202, 215,
	114, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 67, 162, 236, 0, 0, 0, 34, 255, 31, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 236, 244, 251, 240, 251, 244, 228, 0, 0,
	0, 52, 255, 4, 76, 245, 0, 0, 11, 0, 29, 254, 51, 101, 0, 40, 255, 30, 0, 18, 222, 193, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 29, 255, 23, 0, 0, 0, 0, 0, 0, 72, 237, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 124, 178, 0, 0, 0, 0, 5, 245, 63, 0, 0, 111, 200, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 87, 233, 54, 0, 0, 0, 0, 0, 0, 0, 0, 70, 248, 0,
	52, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 68, 246, 0, 6, 248, 19, 0, 0, 65, 247, 0, 0, 0, 0,
	185, 151, 0, 0, 0, 42, 255, 15, 0, 0, 59, 252, 0, 0, 0, 0, 0, 0, 124, 188, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 46, 147, 234, 180, 86, 0, 0, 244, 244, 244, 244, 244, 244, 0, 0, 39, 133, 226,
	197, 97, 10, 0, 0, 0, 51, 255, 4, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 140, 115, 12, 235, 8, 0, 0, 32, 73, 55, 255, 11, 143, 215, 0, 0, 0, 0, 31, 255, 47, 97, 0, 1, 211, 174, 16, 6, 148, 252, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 233, 65, 0, 0, 0, 0, 0, 0, 116, 187, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	44, 220, 3, 0, 0, 0, 0, 9, 234, 58, 0, 0, 0, 0, 0, 155, 186, 12, 28, 220, 103, 0, 0, 0, 8, 58, 255, 11, 8, 0, 0, 105, 232, 47, 0, 0, 0, 0, 0, 101, 32, 0, 25, 193, 173, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 70, 25, 0,
	25, 193, 166, 0, 0, 165, 147, 7, 13, 176, 176, 0, 0, 0, 36, 254, 49, 0, 0, 0, 15, 245, 120, 4, 12, 157, 210, 0, 0, 37, 12, 0, 68, 239, 74, 0, 0, 0, 44, 220, 3, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 8,
	92, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 235, 142, 41, 0, 0, 0, 0, 0, 0, 4, 24, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 49, 244, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 222, 33, 87, 169, 0, 0, 0, 23, 186, 245, 255, 243, 186, 39, 0, 0, 0, 0, 0, 122, 239, 229, 0, 0, 30, 181, 245, 231, 147, 178, 0, 3, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 155, 133, 0, 0, 0, 0, 0, 0, 185, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 241, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 108, 195, 0, 0, 0, 0, 0, 0, 13,
	165, 241, 235, 134, 1, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 255, 250, 244, 244, 244, 244, 0, 0,
	143, 226, 244, 238, 163, 17, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 134, 236, 245, 236, 155, 13, 0, 0, 16,
	167, 238, 242, 175, 22, 0, 0, 0, 139, 203, 0, 0, 0, 0, 0, 68, 203, 246, 243, 185, 39, 0, 0, 104, 235,
	244, 212, 88, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 62, 241, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 49, 244, 3, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 225, 4, 0, 0, 0, 0, 29, 237, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 120, 157, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 223,
	75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 120, 157, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 158, 104, 0, 0, 0, 0, 153, 115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31, 119, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 28, 160, 229, 236, 163, 14, 0, 0, 0, 20, 252, 200, 0, 0, 0, 52, 255, 244, 243, 238, 177, 32, 0, 0, 0, 96, 213, 244, 234, 137, 0, 52, 255, 244, 239, 190, 68, 0, 0, 52, 255, 244, 244, 244, 244, 68, 0, 0, 52, 255, 244, 244, 244, 244, 0, 0, 0, 104, 217, 244, 222, 100, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 244, 246,
	255, 244, 244, 0, 0, 0, 0, 137, 244, 246, 255, 4, 0, 52, 255, 4, 0, 0, 137, 216, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 163, 0, 0, 212, 255, 0, 52, 255, 166, 0, 0, 52, 255, 0, 0, 15, 168, 241, 236, 141, 2,
	0, 52, 255, 244, 244, 239, 176, 33, 0, 0, 15, 168, 241, 236, 141, 2, 0, 52, 255, 244, 244, 236, 171, 30, 0, 0, 47, 189, 243, 241, 204, 73, 0, 0, 244, 244, 246, 255, 244, 244, 0, 52, 255, 4, 0, 0, 52,
	255, 0, 125, 203, 0, 0, 0, 24, 254, 0, 236, 75, 0, 0, 0, 0, 152, 0, 44, 247, 51, 0, 0, 71, 241, 0, 220,
	112, 0, 0, 0, 161, 179, 0, 0, 244, 244, 244, 244, 249, 254, 0, 0, 0, 52, 255, 244, 125, 0, 0, 35, 244, 23, 0, 0, 0, 0, 0, 0, 0, 171, 246, 255, 4, 0, 0, 0, 0, 41, 242, 200, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127,
	24, 227, 130, 13, 21, 206, 163, 0, 0, 0, 98, 207, 251, 26, 0, 0, 52, 255, 4, 0, 8, 147, 204, 0, 0, 100, 224, 51, 0, 22, 89, 0, 52, 255, 4, 7, 81, 238, 66, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0,
	0, 0, 106, 221, 46, 0, 29, 89, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 52, 255, 4, 0, 133, 218, 26, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 202, 4, 36, 219, 255, 0,
	52, 255, 242, 30, 0, 52, 255, 0, 0, 160, 175, 10, 24, 213, 112, 0, 52, 255, 4, 0, 20, 166, 204, 0, 0,
	160, 175, 10, 24, 213, 112, 0, 52, 255, 4, 0, 17, 166, 201, 0, 8, 232, 119, 7, 3, 56, 63, 0, 0, 0, 0,
	52, 255, 4, 0, 0, 52, 255, 4, 0, 0, 52, 255, 0, 48, 253, 19, 0, 0, 94, 226, 0, 197, 107, 0, 0, 0, 0, 184, 0, 0, 131, 201, 2, 8, 220, 101, 0, 74, 238, 18, 0, 52, 245, 33, 0, 0, 0, 0, 0, 1, 192, 148, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 168, 135, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 16, 218, 106, 178, 159, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 157, 170, 0, 0, 0, 88, 243, 0, 0, 0, 179, 121, 200, 105, 0, 0, 52, 255, 4, 0, 0, 63, 251, 0, 0, 227, 86, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 125, 186, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 1, 229, 84, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0,
	0, 0, 0, 52, 255, 4, 0, 52, 255, 4, 130, 220, 27, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 141, 66, 115, 140, 255, 0, 52, 255, 150, 143, 0, 52, 255, 0, 6, 246, 54, 0, 0, 105, 204, 0, 52, 255, 4, 0, 0, 60,
	253, 0, 6, 246, 54, 0, 0, 105, 204, 0, 52, 255, 4, 0, 0, 59, 252, 0, 46, 255, 15, 0, 0, 0, 0, 0, 0, 0, 0,
	52, 255, 4, 0, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 225, 87, 0, 0, 165, 150, 0, 157, 139, 0, 234, 154, 0, 215, 0, 0, 7, 216, 102, 132, 187, 0, 0, 0, 176, 146, 0, 194, 127, 0, 0, 0, 0, 0, 0, 101, 223, 12, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 48, 239, 14, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 3, 184, 122, 0, 8, 193,
	112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 243, 67, 72, 217, 233, 162, 255, 0, 3, 12, 248, 51, 130, 186, 0, 0, 52, 255, 4, 0, 12, 158, 197, 0, 31, 255, 22, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 69, 240, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 32, 255, 22, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 52, 255, 130, 244, 29, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52,
	255, 61, 146, 188, 66, 255, 0, 52, 255, 37, 239, 16, 52, 255, 0, 38, 255, 15, 0, 0, 64, 244, 0, 52,
	255, 4, 0, 24, 169, 202, 0, 38, 255, 15, 0, 0, 64, 244, 0, 52, 255, 4, 0, 16, 159, 188, 0, 11, 235,
	176, 72, 17, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 148, 157, 0, 1, 234, 73, 0, 118, 170, 31, 225, 207, 0, 246, 0, 0, 0, 62, 237, 237, 32, 0, 0, 0, 31, 241, 124, 221, 8, 0, 0, 0,
	0, 0, 25, 238, 68, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 184, 118, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0,
	44, 101, 0, 0, 0, 14, 121, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 32, 242, 102, 5, 147, 255, 0, 4, 86, 235, 1, 59, 250, 16, 0, 52, 255, 240, 241, 254, 223, 32, 0, 48, 255, 6, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 55, 255, 0, 52, 255, 240, 240, 240, 240, 22, 0, 0, 52, 255, 240, 240, 240, 202, 0, 48, 255, 6, 0, 138,
	241, 248, 0, 52, 255, 240, 240, 240, 243, 255, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0,
	52, 255, 220, 235, 100, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 6, 209, 182, 52, 255, 0, 52, 255,
	4, 169, 120, 52, 255, 0, 49, 255, 5, 0, 0, 54, 255, 0, 52, 255, 240, 240, 231, 169, 31, 0, 49, 255, 5, 0, 0, 54, 255, 0, 52, 255, 240, 244, 255, 173, 11, 0, 0, 38, 160, 229, 253, 188, 37, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 70, 227, 0, 51, 244, 7, 0, 78, 202, 83, 147, 220, 32,
	251, 0, 0, 0, 14, 235, 202, 0, 0, 0, 0, 0, 124, 255, 75, 0, 0, 0, 0, 0, 0, 173, 156, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 64, 230, 7, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 255, 54, 255, 13, 0, 62, 255, 0, 4, 167, 167, 0, 4, 240, 92, 0, 52, 255, 4, 0, 18, 164,
	181, 0, 31, 255, 22, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 69, 240, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 32, 255, 20, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0,
	0, 0, 52, 255, 3, 0, 52, 255, 33, 86, 243, 35, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 4, 127, 89, 52,
	255, 0, 52, 255, 4, 52, 230, 58, 255, 0, 38, 255, 15, 0, 0, 64, 244, 0, 52, 255, 4, 0, 0, 0, 0, 0, 38,
	255, 15, 0, 0, 64, 247, 0, 52, 255, 4, 1, 87, 251, 67, 0, 0, 0, 0, 0, 32, 176, 207, 0, 0, 0, 0, 52, 255,
	4, 0, 0, 51, 255, 4, 0, 0, 52, 255, 0, 0, 6, 242, 42, 121, 174, 0, 0, 39, 234, 136, 91, 168, 112, 218,
	0, 0, 0, 153, 180, 211, 98, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 80, 228, 15, 0, 0, 0, 0, 0, 52, 255, 4,
	0, 0, 0, 0, 0, 0, 200, 101, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 255, 37, 243, 102, 5, 147, 255, 0, 11, 241, 250, 248, 248, 253, 173, 0, 52, 255, 4, 0, 0, 58,
	249, 0, 0, 228, 84, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 126, 186, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 1, 230, 78, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 61, 250, 0, 0, 52, 255, 4, 0, 170, 199, 3, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 192, 149, 255, 0, 6, 247, 54, 0, 0, 104, 204, 0, 52, 255, 4, 0, 0, 0, 0, 0, 6, 247, 54,
	0, 0, 104, 209, 0, 52, 255, 4, 0, 0, 151, 207, 0, 0, 0, 0, 0, 0, 61, 253, 0, 0, 0, 0, 52, 255, 4, 0, 0, 40, 255, 8, 0, 0, 56, 248, 0, 0, 0, 171, 112, 192, 97, 0, 0, 4, 250, 199, 35, 111, 196, 178, 0, 0, 66,
	241, 28, 64, 236, 21, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 14, 228, 75, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0,
	0, 0, 0, 80, 218, 2, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 238, 80, 75, 219, 234, 164, 248, 0, 77, 249, 16, 0, 0, 87, 245, 0, 52, 255, 4, 0, 9, 145, 216, 0, 0, 102,
	222, 50, 0, 22, 89, 0, 52, 255, 4, 7, 82, 239, 67, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 109, 215, 40, 0, 81, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 0, 52, 255, 4, 0, 0, 0, 43, 112, 13, 3, 145, 201, 0, 0, 52, 255, 4, 0, 20, 233, 126, 0, 0, 52, 255, 11, 8, 8, 8, 0, 52, 255, 4, 0, 0, 52, 255,
	0, 52, 255, 4, 0, 75, 246, 255, 0, 0, 163, 173, 9, 23, 212, 114, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 162,
	173, 9, 23, 212, 123, 0, 52, 255, 4, 0, 0, 25, 246, 0, 14, 123, 29, 0, 18, 163, 202, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 5, 232, 107, 3, 9, 151, 190, 0, 0, 0, 93, 195, 248, 21, 0, 0, 0, 216, 233, 0, 54, 254,
	139, 0, 10, 221, 108, 0, 0, 168, 163, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 151, 163, 0, 0, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 0, 1, 214, 84, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 146, 193, 1, 0, 0, 0, 0, 0, 155, 179, 0, 0, 0, 11, 245, 0, 52, 255, 244, 243, 239, 184, 44, 0, 0, 0, 99, 214, 244, 237, 140, 0, 52, 255, 244, 240, 191, 69, 0, 0, 52, 255, 244, 244, 244, 244,
	99, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 109, 219, 243, 223, 116, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 244, 246, 255, 244, 244, 0, 0, 27, 186, 240, 244, 204, 50, 0, 0, 52, 255, 4, 0, 0, 82, 251, 0, 0, 52, 255, 255, 255, 255, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 1, 213, 255, 0, 0, 16, 171, 241,
	237, 143, 3, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 16, 170, 241, 254, 187, 6, 0, 52, 255, 4, 0, 0, 0, 138, 0,
	9, 171, 229, 246, 240, 177, 33, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 51, 197, 244, 241, 177, 28, 0, 0, 0,
	18, 252, 198, 0, 0, 0, 0, 176, 179, 0, 5, 247, 99, 0, 144, 199, 2, 0, 0, 27, 243, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 254, 247, 244, 244, 244, 244, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 96, 203, 0, 0, 0, 0, 0,
	52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 16, 216, 165, 31, 2, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 87, 237, 57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 5, 226, 68, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 19, 144, 222, 242, 159, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 58, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 248, 240, 123, 0, 0, 0, 0, 0, 0, 0, 71, 79, 0, 0, 0, 168,
	241, 248, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 244, 244, 244, 244, 244, 244, 99, 0, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127,
	127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 9, 124, 20, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 140, 0, 127, 0, 0, 112, 174, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 221, 0, 127, 0, 0, 0, 159, 98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 166, 243, 244, 102, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 49, 244, 3, 0, 0, 0, 0, 0, 49, 244, 3, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 11, 244, 246, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 158, 239, 209, 0, 0, 0, 0, 52, 255, 4, 0, 0, 11, 244, 233, 119, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 162, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 46, 255, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 37, 255, 46, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 96, 244, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 149, 81, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 103, 213, 244, 239, 180, 36, 0, 52, 255, 122, 232, 241, 159, 9, 0, 0, 12, 151, 233, 244, 202, 40, 0, 0, 26, 183, 244, 223, 133, 255, 0, 0, 13, 156, 236, 241, 165, 12, 0, 19, 244, 246, 255, 244, 244, 102, 0, 0, 25, 182, 244, 221, 127, 255, 0, 52, 255, 111, 233, 235, 80, 0, 0, 0, 0,
	244, 246, 255, 4, 0, 0, 0, 45, 244, 246, 255, 4, 0, 0, 0, 52, 255, 4, 0, 122, 210, 0, 0, 0, 52, 255, 4,
	0, 0, 0, 255, 173, 246, 137, 195, 245, 114, 0, 52, 255, 111, 233, 235, 80, 0, 0, 0, 29, 182, 242,
	237, 157, 11, 0, 52, 255, 121, 231, 241, 161, 10, 0, 0, 26, 183, 244, 222, 130, 255, 0, 0, 52, 255,
	78, 217, 244, 175, 0, 0, 0, 109, 226, 245, 229, 114, 0, 19, 244, 246, 255, 244, 244, 152, 0, 52,
	255, 4, 0, 52, 255, 4, 0, 51, 242, 9, 0, 0, 73, 228, 0, 226, 63, 0, 0, 0, 0, 140, 0, 13, 219, 93, 0, 0,
	173, 156, 0, 40, 250, 19, 0, 0, 50, 245, 0, 0, 209, 244, 244, 246, 255, 0, 0, 0, 0, 0, 51, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 8, 0, 1, 219, 10, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 93, 52, 2, 9, 162, 204, 0, 52, 255, 163, 9, 19, 198, 144, 0, 0, 174, 188, 24, 0, 53, 43, 0, 0, 193, 157, 7, 20, 200, 255, 0, 0, 173, 186, 16, 11, 179, 152, 0, 0, 0, 52, 255, 4, 0,
	0, 0, 0, 189, 167, 10, 19, 199, 255, 0, 52, 255, 121, 2, 133, 219, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 133, 204, 19, 0, 0, 0, 52, 255, 4, 0, 0, 0, 255, 51, 99, 255, 55,
	96, 227, 0, 52, 255, 121, 2, 133, 219, 0, 0, 0, 198, 158, 8, 19, 198, 149, 0, 52, 255, 163, 9, 19,
	198, 145, 0, 0, 192, 158, 8, 20, 198, 255, 0, 0, 52, 255, 169, 20, 2, 52, 0, 0, 31, 255, 62, 0, 20, 57, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 216, 84, 0, 0, 162, 139, 0, 166, 118, 0,
	0, 0, 0, 195, 0, 0, 49, 235, 36, 104, 211, 9, 0, 0, 200, 106, 0, 0, 142, 163, 0, 0, 0, 0, 0, 149, 171, 0, 0, 0, 0, 0, 54, 255, 2, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 50, 255, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	95, 232, 100, 0, 55, 175, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 252, 0, 52, 255, 33, 0, 0,
	81, 233, 0, 22, 255, 41, 0, 0, 0, 0, 0, 25, 255, 32, 0, 0, 82, 255, 0, 21, 255, 49, 0, 0, 70, 236, 0, 0,
	0, 52, 255, 4, 0, 0, 0, 24, 255, 35, 0, 0, 82, 255, 0, 52, 255, 18, 0, 56, 254, 1, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 146, 227, 15, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 255, 8, 56,
	255, 8, 56, 253, 0, 52, 255, 18, 0, 56, 254, 1, 0, 27, 255, 32, 0, 0, 81, 234, 0, 52, 255, 33, 0, 0, 81, 232, 0, 25, 255, 32, 0, 0, 81, 255, 0, 0, 52, 255, 41, 0, 0, 0, 0, 0, 26, 252, 96, 11, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 125, 173, 0, 8, 242, 48, 0, 106, 173, 0, 192, 118, 4, 246, 0, 0, 0, 106, 212, 234, 40, 0, 0, 0, 103, 199, 0, 3, 230, 67, 0, 0, 0, 0, 90, 216, 13, 0, 0, 0, 0,
	1, 121, 226, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 22, 252, 74, 0, 0, 0, 0, 115, 225, 233, 149, 42,
	25, 0, 15, 54, 177, 0, 136, 94, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 78, 197, 237, 240, 243, 255, 0, 52, 255, 6, 0, 0, 55, 254, 0, 47, 255, 7, 0, 0, 0, 0, 0, 48, 255, 6, 0, 0, 55, 255, 0, 47, 255, 240, 240,
	240, 241, 247, 0, 0, 0, 52, 255, 4, 0, 0, 0, 48, 255, 7, 0, 0, 55, 255, 0, 52, 255, 4, 0, 52, 255, 4, 0,
	0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 187, 238, 80, 0, 0, 0, 0, 52, 255, 4, 0,
	0, 0, 255, 4, 52, 255, 4, 52, 255, 0, 55, 255, 4, 0, 52, 255, 4, 0, 48, 255, 6, 0, 0, 55, 254, 0, 52,
	255, 6, 0, 0, 55, 254, 0, 48, 255, 6, 0, 0, 55, 255, 0, 0, 52, 255, 6, 0, 0, 0, 0, 0, 0, 84, 186, 239,
	208, 75, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 35, 246, 13, 84, 213, 0, 0, 46,
	228, 16, 202, 187, 49, 226, 0, 0, 0, 13, 239, 176, 0, 0, 0, 0, 15, 246, 36, 70, 225, 1, 0, 0, 0, 43,
	233, 43, 0, 0, 0, 0, 33, 247, 247, 73, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 112, 254, 237, 0, 0, 0, 139, 14, 17, 117, 219, 229, 0, 0, 1, 219, 8, 211, 18, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 24, 251, 86, 5, 0, 65, 255, 0, 52, 255, 32, 0, 0, 80, 232, 0, 22, 255, 41, 0, 0, 0, 0, 0, 25, 255, 32, 0, 0, 81, 255, 0, 22, 255, 21, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 25, 255, 34, 0, 0, 81, 255, 0, 52, 255, 4, 0, 52,
	255, 4, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 8, 84, 237, 29, 0, 0, 0, 51,
	255, 5, 0, 0, 0, 255, 4, 52, 255, 4, 52, 255, 0, 56, 255, 4, 0, 52, 255, 4, 0, 27, 255, 32, 0, 0, 81,
	234, 0, 52, 255, 32, 0, 0, 80, 233, 0, 26, 255, 32, 0, 0, 81, 255, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0,
	0, 4, 126, 237, 0, 0, 0, 51, 255, 4, 0, 0, 0, 48, 255, 8, 0, 66, 255, 4, 0, 0, 0, 199, 94, 173, 122, 0, 0, 2, 239, 110, 116, 189, 116, 166, 0, 0, 0, 166, 166, 222, 90, 0, 0, 0, 0, 166, 128, 162, 131, 0, 0, 0, 13, 216, 90, 0, 0, 0, 0, 0, 0, 3, 137, 210, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 14, 244, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 154, 117, 188, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 33, 255, 68, 0, 26, 189, 255, 0, 52, 255, 160, 8, 17, 195, 143, 0, 0, 177, 185, 23, 0, 50, 42, 0, 0, 194, 158, 7, 19, 198,
	255, 0, 0, 176, 162, 17, 0, 32, 76, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 191, 164, 10, 17, 195, 255, 0, 52,
	255, 4, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 156, 194, 0, 0, 0, 24, 254, 67, 0, 0, 0, 255, 4, 52, 255, 4, 52, 255, 0, 56, 255, 4, 0, 52, 255, 4, 0, 0, 200,
	158, 7, 19, 198, 151, 0, 52, 255, 160, 8, 17, 195, 146, 0, 0, 194, 158, 7, 19, 198, 255, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 31, 72, 8, 2, 124, 230, 0, 0, 0, 30, 255, 51, 0, 0, 0, 15, 251, 62, 0, 147, 255, 4, 0, 0, 0, 108, 195, 246, 32, 0, 0, 0, 182, 228, 42, 120, 228, 106, 0, 0, 103, 217, 11, 55, 238, 39, 0, 0, 0, 70, 222, 242, 36, 0, 0, 0, 172, 148, 0, 0, 0, 0, 0, 0, 0, 0, 57, 254, 1, 0, 0, 0, 0, 0, 52, 255, 4,
	0, 0, 0, 0, 49, 255, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 78, 244, 107, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 114, 231, 243, 204, 108, 255, 0, 52, 255, 120, 232, 240, 159, 9, 0, 0, 14, 153, 234, 244,
	206, 41, 0, 0, 28, 184, 244, 223, 132, 255, 0, 0, 14, 155, 234, 244, 218, 111, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 27, 184, 245, 221, 123, 255, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 118, 244, 246, 255, 244,
	244, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 11, 217, 0, 0, 0, 0, 121, 240, 244, 114, 0, 255, 4,
	52, 255, 4, 52, 255, 0, 56, 255, 4, 0, 52, 255, 4, 0, 0, 31, 184, 243, 238, 160, 12, 0, 52, 255, 120,
	232, 241, 160, 10, 0, 0, 28, 185, 244, 223, 130, 255, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 35, 199, 242,
	246, 204, 61, 0, 0, 0, 0, 141, 236, 229, 142, 0, 0, 120, 234, 191, 137, 255, 4, 0, 0, 0, 21, 251, 197, 0, 0, 0, 0, 122, 224, 0, 45, 255, 46, 0, 48, 241, 50, 0, 0, 118, 210, 0, 0, 0, 2, 227, 198, 0, 0, 0, 0,
	255, 246, 244, 244, 244, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 52, 255, 4, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 245, 27, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 72, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 254, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	199, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 255, 5, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 53,
	255, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 53, 55, 2, 15, 182, 162, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 121, 215, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 244, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 255, 56, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 106, 232, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 75, 220, 246, 237, 162, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	194, 246, 227, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 203, 237, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 133, 229, 206,
	0, 0, 0, 0, 52, 255, 4, 0, 0, 11, 240, 221, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 187, 58, 108, 143, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 102, 10, 58, 54, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 97, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 33, 222, 208, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 194, 183, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 48, 240, 52, 240, 3, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 58, 199, 211, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 184, 184, 143, 119, 77, 39, 156, 0,
	0, 0, 73, 164, 22, 196, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 42,
	179, 3, 188, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 4, 134, 230, 245, 210, 44, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 219, 243, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 4, 195, 31, 94, 137, 0, 0, 109, 231, 221, 73, 0, 0, 0, 0, 0, 47, 189, 243, 241, 204, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 175, 241, 247, 255, 244, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 244, 244, 244, 244, 249, 254, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 111,
	129, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 111, 178, 0, 111, 129, 0, 0, 52, 255, 4, 52, 255, 4, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 156, 224, 40, 191, 0, 0, 0, 196, 0,
	196, 159, 147, 211, 0, 0, 0, 0, 125, 231, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 143, 189, 126, 0, 0, 0, 171, 161, 0, 0, 1, 208, 134, 0, 127, 0, 137, 197, 27, 0, 51, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 218, 103, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 51, 101, 235, 0, 0, 0, 0, 8, 233, 119, 7, 3, 56, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 177, 20, 52, 255, 4, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 192, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 206,
	69, 0, 0, 0, 0, 0, 0, 62, 241, 1, 0, 0, 0, 0, 206, 94, 0, 206, 69, 0, 0, 62, 241, 1, 66, 241, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 174, 39, 188, 126, 0, 0, 0, 196, 0, 196, 55, 118, 196, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 125, 8, 0, 0, 0, 29, 241, 39, 0, 85, 240, 17, 0, 127, 8, 244, 57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31, 249, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 118,
	244, 246, 255, 244, 244, 72, 0, 118, 244, 246, 255, 244, 244, 72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 50,
	100, 234, 1, 57, 97, 0, 46, 255, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31, 0, 0, 0, 10, 252, 51, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101, 223, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 35,
	255, 14, 0, 0, 0, 0, 0, 0, 118, 157, 0, 0, 0, 0, 35, 255, 18, 35, 255, 14, 0, 0, 120, 157, 0, 143, 157,
	0, 0, 0, 0, 84, 234, 231, 76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	196, 0, 196, 0, 0, 196, 0, 0, 0, 109, 226, 245, 229, 114, 0, 0, 0, 0, 30, 0, 0, 0, 0, 0, 103, 239, 220,
	122, 231, 236, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 209, 244, 244, 246, 255, 0, 0, 0, 120, 167, 2, 213, 127, 0, 0, 127, 240, 255, 241, 240, 236, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74,
	210, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 111, 234, 255, 210, 148, 92, 14, 0, 10, 232, 176, 72, 17, 0, 0, 0, 0, 0, 0,
	55, 219, 0, 0, 0, 40, 255, 14, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 238, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 178, 63, 0, 0, 0, 0, 52, 255, 4,
	52, 255, 4, 0, 0, 181, 63, 0, 225, 63, 0, 0, 0, 0, 234, 255, 255, 232, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31, 255, 62, 0, 20, 57, 0, 0, 0, 0, 218,
	54, 0, 0, 0, 5, 243, 62, 113, 255, 57, 106, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 149, 171, 0, 0, 0, 6,
	214, 135, 237, 14, 0, 0, 127, 51, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	22, 240, 247, 250, 240, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 149, 89, 13, 0, 0, 0, 0, 0, 33, 152, 222, 253, 188,
	37, 0, 0, 0, 85, 217, 46, 0, 0, 0, 49, 255, 6, 0, 52, 255, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 173,
	156, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 234, 255, 255, 232, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 252, 96, 11, 0, 0, 0, 0, 0, 0, 46, 217, 85,
	0, 0, 39, 255, 11, 62, 255, 6, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90, 216, 13, 0, 0, 0, 0, 68, 255,
	121, 0, 0, 0, 127, 240, 255, 240, 240, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	156, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 113, 231, 221, 73, 73, 223, 225, 0, 0, 0, 0, 0, 27, 170, 207, 0, 0, 0,
	229, 102, 0, 0, 0, 0, 40, 255, 14, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 80, 228, 15, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 84, 235, 233, 79, 0, 0, 244, 244, 244, 244, 244, 244, 244, 0, 244, 244,
	244, 244, 244, 244, 244, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 84, 186, 239, 208, 75, 0, 0, 0, 0, 0, 103, 229, 0, 0, 50, 255, 5, 53, 255, 240, 241, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 233, 43,
	0, 0, 0, 0, 0, 0, 252, 60, 0, 0, 0, 127, 9, 247, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 200, 85, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 116,
	240, 243, 255, 240, 240, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 51, 102, 235, 229, 51, 101, 0, 0, 0, 0, 0,
	0, 61, 253, 0, 0, 0, 29, 206, 115, 0, 0, 0, 10, 252, 51, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14,
	228, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 126, 237, 0, 0, 0, 0, 116, 206, 29, 0, 0, 39, 255, 11, 60, 255, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 216, 90, 0, 0, 0, 0, 0, 0, 0, 252, 60, 0, 0,
	0, 127, 0, 146, 201, 26, 0, 50, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 1, 243, 45,
	0, 0, 0, 0, 52, 255, 4, 52, 255, 4, 0, 44, 220, 3, 44, 220, 3, 44, 0, 3, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 47, 99, 235, 230, 47, 97, 0, 15, 135, 41, 5, 30, 171, 202,
	0, 0, 0, 0, 12, 180, 0, 0, 0, 0, 181, 177, 19, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 151, 163, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31, 72, 8, 2, 124, 230, 0, 0, 0, 0, 179, 12, 0, 0, 0, 6, 244, 62,
	114, 255, 80, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 172, 148, 0, 0, 0, 0, 0, 0, 0, 0, 252, 60, 0, 0, 0, 127, 0, 6, 140, 231, 245, 213, 44, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 241, 1, 0, 0, 0, 0, 0, 33, 253, 7, 0, 0, 0, 0, 62, 241, 1, 66, 241, 1, 0, 52, 255, 4, 52, 255, 4, 52, 0, 4, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 239, 229, 79, 79, 231, 233, 0, 9, 166, 229, 252, 240, 177, 33, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 26, 177, 242, 247, 255, 244, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 247, 244, 244,
	244, 244, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 35, 199, 242, 246, 204, 61, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0,
	105, 240, 221, 111, 222, 244, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 255, 246, 244, 244, 244, 0, 0, 0, 0, 0, 252, 60, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 120, 157, 0, 0, 0, 0, 0, 0, 85, 212,
	0, 0, 0, 0, 0, 120, 157, 0, 143, 157, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 63, 0, 0, 0, 0, 30, 11, 195, 131,
	0, 0, 0, 0, 0, 181, 63, 0, 225, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 227, 245, 177, 12, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127,
	127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 58, 98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
	8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 221, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 103, 185, 255, 4, 0, 0, 0, 0, 106, 186, 255, 4, 0, 0, 0, 0, 0, 144, 235, 229, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 49, 244, 3, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 86, 224, 245, 206, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 215, 116, 0, 0, 0, 164, 172, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 110, 227, 245, 223, 102, 0, 0, 0, 49, 244, 53, 244, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 122, 237, 237, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 244, 244, 244, 175, 0, 0, 0, 0, 116, 239, 227, 77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 235, 233, 108, 0, 0, 0, 0, 165, 239, 235, 132, 0, 0, 0, 0, 0, 173, 88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 184, 245, 255, 246, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101, 185, 255, 4, 0, 0, 0, 0, 77, 219, 247, 201, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 17, 0, 101, 238, 0, 0, 0, 0, 0, 49, 244, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 11, 245, 105, 1,
	43, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 241, 25, 0, 61, 236, 23, 0, 0, 0, 0, 52, 255, 4, 0, 0, 33, 255, 60, 0,
	26, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 53, 181, 236, 230, 161, 31, 0, 0, 0, 33, 1, 102, 238, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 53, 181, 236, 230, 161, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30,
	254, 51, 99, 236, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 42, 3, 96, 248, 0, 0, 0, 0, 15, 2, 107, 236, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 226, 255, 255, 255, 56, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 14, 244, 93, 2, 142, 209, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 222, 242, 72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 161, 243, 255, 236, 0, 0, 0, 47,
	255, 11, 0, 0, 0, 0, 82, 25, 0, 0, 44, 64, 0, 0, 155, 165, 3, 210, 93, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 15, 230, 104, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67, 255, 255, 253, 224, 140, 233, 0, 0, 3, 160, 237,
	243, 255, 3, 0, 0, 0, 0, 31, 0, 0, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67, 255, 255, 251, 251, 255, 233, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 30, 254, 50, 100, 234, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 113, 173, 0, 0,
	0, 0, 0, 244, 239, 67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 52, 255, 4, 0, 45, 255, 255, 255, 255, 56, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 47, 255, 12, 0, 61, 253, 0, 0, 30, 0, 0, 30, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 108, 224,
	0, 0, 0, 0, 0, 44, 255, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 174, 182, 65, 255, 13, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 79, 222, 204, 189, 221, 47, 0, 123,
	185, 244, 166, 235, 176, 90, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 100, 237, 188, 46, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 222, 255, 110, 2, 21, 6, 156, 0, 0, 42, 255, 40, 100, 255, 4, 0, 0, 0, 55, 219, 0, 55, 218, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 222, 161, 255, 4, 86, 247, 157, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 114, 233, 220, 71, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 65, 191, 12, 0, 0, 0, 0, 0, 0, 107, 224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 52, 255, 4, 0, 19, 250, 255, 255, 255, 56, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 12, 243, 94, 4, 144, 206, 0, 0, 218, 54, 0, 219, 54, 0, 0, 81,
	176, 212, 160, 0, 28, 70, 0, 81, 176, 212, 168, 31, 8, 78, 0, 0, 35, 1, 114, 240, 0, 0, 0, 0, 0, 63,
	242, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 22, 255,
	48, 52, 255, 4, 0, 0, 172, 243, 255, 240, 240, 37, 0, 0, 17, 230, 49, 97, 201, 0, 0, 0, 0, 101, 255,
	50, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 21, 249, 20, 108, 240, 103, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 14, 0, 0, 0, 67, 0, 0, 5, 182, 234, 147, 248, 3, 0, 0, 85, 219, 47, 85, 212, 43, 0, 186, 244, 244,
	244, 244, 245, 252, 0, 255, 71, 255, 244, 232, 84, 67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 240, 240, 243, 255, 240, 240, 0, 0, 0, 56, 182, 13, 0, 0, 0, 0, 0, 42, 1, 116, 239, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 105, 248, 255, 255, 56, 255, 0, 0, 0, 44, 220, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 81, 176, 212, 168, 49, 0, 0, 0, 72, 213, 242, 195, 40, 0, 0, 44, 213, 85, 48, 220, 85, 0, 0, 2, 56, 132, 204, 195, 104, 0, 0, 0, 12, 86, 169, 231, 170, 0, 0, 180, 240, 219, 87,
	42, 91, 0, 0, 15, 210, 132, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 48, 255, 11, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 16, 229, 53, 101, 198, 0, 0, 168,
	240, 243, 255, 240, 240, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 254, 67, 0, 93, 244, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 255, 222, 111, 4, 17, 5, 67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 229, 109, 0, 229, 89, 0, 0, 0, 0, 0, 0, 0,
	52, 255, 0, 255, 71, 255, 9, 178, 76, 67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 173, 242, 240, 240, 11, 0, 0, 0, 200, 241, 217, 82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 52, 255, 4, 0, 0, 0, 18, 107, 255, 56, 255, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 229, 0, 109, 229, 0, 150, 216, 167, 92, 18, 0, 0, 0, 92, 177, 230, 162, 79, 8, 0, 0, 21, 98, 143, 151, 148, 110, 60, 0, 0, 186, 168, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 21, 255, 43, 52, 255, 4, 0, 0,
	0, 52, 255, 4, 0, 0, 0, 0, 76, 223, 204, 189, 222, 49, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4,
	0, 0, 0, 117, 241, 119, 83, 210, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 224, 153, 197, 240, 202, 13, 157, 0, 0, 3, 240, 240, 240, 240, 11, 0, 0, 29, 207, 117, 29, 205, 112, 0, 0, 0, 0, 0, 0, 50, 248, 0, 224, 156,
	228, 3, 22, 188, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 11, 0, 62, 255, 4, 0, 0, 0, 0, 52, 255,
	56, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 228, 240, 240, 240,
	172, 0, 0, 113, 205, 29, 118, 207, 29, 0, 49, 4, 0, 0, 146, 255, 4, 0, 130, 71, 5, 87, 225, 232, 129,
	0, 21, 53, 7, 0, 2, 174, 255, 0, 40, 255, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 175, 172, 59, 255, 8, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 85, 25, 0, 0,
	46, 62, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 43, 182, 246, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 240, 97, 7, 14, 129, 232, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 180, 0, 12, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 240, 97, 7, 14, 129, 232, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 99, 3, 157, 255, 18, 0, 0, 0, 0, 52, 255, 56, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 179, 12, 0, 179, 12, 0, 0, 0, 0, 0, 44, 141, 255, 4, 0, 0, 0, 0, 28, 5, 88, 239, 0, 0, 0,
	0, 0, 126, 102, 255, 0, 23, 252, 95, 8, 66, 99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 13, 156, 235, 255, 231, 0, 0, 244, 246, 255, 244, 244, 244, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 127, 213, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 53, 184, 236, 232, 164, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 53, 184, 236, 232, 164, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 244, 244, 244,
	244, 244, 244, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 163, 243,
	148, 208, 191, 0, 0, 0, 0, 52, 255, 56, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 136, 53, 255, 4, 0, 0, 0, 0, 0, 9, 168, 67,
	0, 0, 0, 0, 84, 94, 52, 255, 0, 0, 101, 231, 249, 202, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 1, 72, 10, 3, 120, 233, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 56, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 33, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 240, 243,
	255, 176, 0, 0, 0, 0, 28, 162, 38, 0, 0, 0, 0, 0, 174, 240, 243, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 2, 188, 234, 241, 201, 61, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 23, 116, 25,
	116, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 214, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 143, 245, 244, 244, 0, 0, 0, 0, 0, 0, 52, 255, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 116, 199, 178, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 0, 0, 51, 90, 0, 0, 0,
	0, 0, 0, 0, 6, 119, 17, 0, 0, 0, 0, 4, 117, 84, 0, 0, 0, 0, 2, 196, 190, 20, 191, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 145, 237, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 99, 44, 0, 0, 0, 0, 0, 0, 0, 48, 95, 0, 0, 0, 0, 0, 41, 128, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 97, 35, 0, 0, 0, 0, 0, 0, 0, 57, 85, 0, 0, 0, 0, 0, 46, 127, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 217, 181, 17, 203, 0, 0, 0, 0, 74, 213, 6, 0, 0, 0, 0, 0, 0, 27, 229, 40, 0, 0, 0, 0, 29, 222, 209, 8, 0, 0, 0, 2, 195,
	200, 27, 193, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101, 212, 8,
	0, 0, 0, 0, 0, 0, 29, 234, 61, 0, 0, 0, 0, 42, 226, 218, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 143, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 3, 172, 83, 0, 0, 0, 0, 0, 0, 158, 105, 0, 0, 0, 0, 0, 154, 91, 157, 84, 0, 0, 0, 34, 144, 63, 189, 101, 0, 0, 0, 48, 240, 52, 240, 3, 0, 0, 0, 32, 255, 96, 239, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 39, 201, 20, 0, 0, 0, 0, 0, 25,
	203, 35, 0, 0, 0, 0, 24, 188, 62, 188, 22, 0, 0, 0, 48, 240, 52, 240, 3, 0, 0, 0, 0, 42, 195, 12, 0, 0, 0, 0, 0, 29, 198, 25, 0, 0, 0, 0, 22, 184, 66, 185, 14, 0, 0, 0, 48, 240, 52, 240, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74, 110, 77, 192, 109, 0, 0, 0, 0, 0, 139, 130, 0, 0, 0, 0, 0, 0, 177, 92, 0, 0, 0, 0, 1, 183,
	63, 107, 141, 0, 0, 0, 37, 141, 60, 187, 123, 0, 0, 0, 48, 240, 3, 48, 240, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 151, 143, 0, 0, 0, 0, 0, 1, 189, 104, 0, 0, 0, 0, 8, 200, 53, 93, 169, 0, 0, 0, 48, 240, 3, 48, 240, 3, 0, 0, 0, 42, 192, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 20, 246, 100, 217, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 20, 252, 200,
	0, 0, 0, 0, 0, 20, 252, 200, 0, 0, 0, 0, 0, 20, 252, 200, 0, 0, 0, 0, 0, 20, 252, 200, 0, 0, 0, 0, 0, 80,
	255, 32, 0, 0, 0, 0, 0, 62, 238, 30, 0, 0, 0, 0, 0, 196, 247, 255, 244, 244, 0, 0, 0, 96, 213, 244, 234, 137, 0, 52, 255, 244, 244, 244, 244, 68, 0, 52, 255, 244, 244, 244, 244, 68, 0, 52, 255, 244, 244, 244, 244, 68, 0, 52, 255, 244, 244, 244, 236, 0, 0, 0, 244, 246, 255, 244, 244, 0, 0, 0, 244, 246,
	255, 244, 244, 0, 0, 0, 244, 246, 255, 244, 244, 0, 0, 0, 244, 246, 255, 244, 244, 0, 0, 52, 255,
	244, 239, 190, 68, 0, 0, 52, 255, 166, 0, 0, 52, 255, 0, 0, 15, 168, 241, 236, 141, 2, 0, 0, 15, 168,
	241, 236, 141, 2, 0, 0, 15, 168, 241, 236, 141, 2, 0, 0, 15, 168, 241, 236, 141, 2, 0, 0, 15, 168,
	241, 236, 141, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 166, 240, 239, 148, 152, 0, 52, 255, 4, 0, 0, 52,
	255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 220,
	112, 0, 0, 0, 161, 179, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 100, 226, 247, 212, 58, 0, 0, 127, 0, 0, 98,
	207, 251, 26, 0, 0, 0, 0, 98, 207, 251, 26, 0, 0, 0, 0, 98, 207, 251, 26, 0, 0, 0, 0, 98, 207, 251, 26,
	0, 0, 0, 0, 160, 212, 112, 0, 0, 0, 0, 0, 91, 210, 42, 0, 0, 0, 0, 17, 242, 58, 255, 4, 0, 0, 0, 100, 223, 50, 0, 22, 89, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 52, 255, 4, 7, 81, 238, 66, 0, 52, 255, 242, 30, 0, 52, 255, 0, 0, 160, 175, 10, 24,
	213, 112, 0, 0, 160, 175, 10, 24, 213, 112, 0, 0, 160, 175, 10, 24, 213, 112, 0, 0, 160, 175, 10, 24, 213, 112, 0, 0, 160, 175, 10, 24, 213, 112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 158, 169, 7, 42, 243, 176, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 74, 238, 18, 0, 52, 245, 33, 0, 52, 255, 4, 0, 0, 0, 0, 0, 20, 251, 87, 1, 120, 216, 0,
	0, 127, 0, 0, 179, 121, 200, 105, 0, 0, 0, 0, 179, 121, 200, 105, 0, 0, 0, 0, 179, 121, 200, 105, 0, 0, 0, 0, 179, 121, 200, 105, 0, 0, 0, 3, 236, 97, 192, 0, 0, 0, 0, 0, 185, 107, 136, 0, 0, 0, 0, 86, 189,
	52, 255, 4, 0, 0, 0, 227, 84, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255,
	4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52,
	255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 0, 125, 186, 0, 52, 255, 150, 143, 0, 52, 255, 0, 6, 246, 54, 0, 0, 105, 204, 0, 6, 246, 54, 0, 0, 105, 204, 0, 6, 246, 54, 0, 0, 105, 204, 0, 6, 246, 54, 0, 0, 105, 204, 0, 6, 246, 54, 0, 0, 105, 204, 0, 0, 123, 22, 0, 0, 66, 81, 0, 5, 246, 57, 0, 52,
	245, 208, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0,
	52, 255, 4, 0, 0, 52, 255, 0, 0, 176, 146, 0, 194, 127, 0, 0, 52, 255, 240, 240, 233, 177, 38, 0, 50,
	255, 8, 69, 180, 160, 0, 0, 127, 0, 12, 248, 51, 130, 186, 0, 0, 0, 12, 248, 51, 130, 186, 0, 0, 0, 12, 248, 51, 130, 186, 0, 0, 0, 12, 248, 51, 130, 186, 0, 0, 0, 64, 222, 16, 250, 19, 0, 0, 0, 25, 230,
	26, 227, 2, 0, 0, 0, 159, 125, 52, 255, 4, 0, 0, 31, 255, 21, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 0, 69, 240, 0,
	52, 255, 37, 239, 16, 52, 255, 0, 38, 255, 15, 0, 0, 64, 244, 0, 38, 255, 15, 0, 0, 64, 244, 0, 38,
	255, 15, 0, 0, 64, 244, 0, 38, 255, 15, 0, 0, 64, 244, 0, 38, 255, 15, 0, 0, 64, 244, 0, 0, 130, 214,
	25, 73, 235, 64, 0, 38, 255, 20, 12, 210, 108, 246, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0,
	52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 31, 241, 124, 221, 8, 0, 0,
	52, 255, 4, 0, 18, 164, 208, 0, 52, 255, 30, 251, 36, 0, 0, 0, 127, 0, 86, 235, 1, 59, 250, 16, 0, 0,
	86, 235, 1, 59, 250, 16, 0, 0, 86, 235, 1, 59, 250, 16, 0, 0, 86, 235, 1, 59, 250, 16, 0, 0, 144, 164,
	0, 213, 96, 0, 0, 0, 117, 182, 0, 230, 68, 0, 0, 1, 231, 61, 52, 255, 240, 240, 0, 48, 255, 6, 0, 0, 0,
	0, 0, 52, 255, 240, 240, 240, 240, 22, 0, 52, 255, 240, 240, 240, 240, 22, 0, 52, 255, 240, 240,
	240, 240, 22, 0, 52, 255, 240, 240, 240, 187, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 239, 255, 240, 198, 0, 55, 255, 0, 52, 255, 4,
	169, 120, 52, 255, 0, 49, 255, 5, 0, 0, 54, 255, 0, 49, 255, 5, 0, 0, 54, 255, 0, 49, 255, 5, 0, 0, 54,
	255, 0, 49, 255, 5, 0, 0, 54, 255, 0, 49, 255, 5, 0, 0, 54, 255, 0, 0, 0, 124, 225, 235, 60, 0, 0, 49,
	255, 6, 160, 101, 53, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 52, 255, 4, 0, 0, 52, 255, 0, 0, 0, 124, 255, 75, 0, 0, 0, 52, 255, 4, 0, 0, 59, 253, 0,
	52, 255, 26, 245, 109, 2, 0, 0, 127, 0, 167, 167, 0, 4, 240, 92, 0, 0, 167, 167, 0, 4, 240, 92, 0, 0,
	167, 167, 0, 4, 240, 92, 0, 0, 167, 167, 0, 4, 240, 92, 0, 0, 223, 105, 0, 154, 176, 0, 0, 0, 210, 130, 0, 179, 162, 0, 0, 49, 246, 6, 52, 255, 4, 0, 0, 31, 255, 23, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0,
	52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 0, 69, 240, 0, 52, 255, 4, 52, 230, 58, 255, 0, 38, 255, 15, 0, 0, 64, 244, 0, 38, 255, 15, 0, 0, 64, 244, 0, 38,
	255, 15, 0, 0, 64, 244, 0, 38, 255, 15, 0, 0, 64, 244, 0, 38, 255, 15, 0, 0, 64, 244, 0, 0, 0, 80, 241,
	224, 31, 0, 0, 40, 255, 92, 164, 0, 63, 245, 0, 51, 255, 4, 0, 0, 52, 255, 0, 51, 255, 4, 0, 0, 52, 255, 0, 51, 255, 4, 0, 0, 52, 255, 0, 51, 255, 4, 0, 0, 52, 255, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0,
	17, 161, 208, 0, 52, 255, 4, 47, 187, 209, 46, 0, 127, 7, 241, 250, 248, 248, 253, 173, 0, 7, 241,
	250, 248, 248, 253, 173, 0, 7, 241, 250, 248, 248, 253, 173, 0, 7, 241, 250, 248, 248, 253, 173, 0, 48, 255, 249, 248, 250, 246, 10, 0, 49, 255, 249, 248, 250, 244, 11, 0, 122, 251, 240, 243, 255,
	4, 0, 0, 0, 228, 88, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0,
	0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 0, 126, 186, 0, 52, 255, 4, 0, 192, 149, 255, 0, 6, 247,
	54, 0, 0, 104, 204, 0, 6, 247, 54, 0, 0, 104, 204, 0, 6, 247, 54, 0, 0, 104, 204, 0, 6, 247, 54, 0, 0,
	104, 204, 0, 6, 247, 54, 0, 0, 104, 204, 0, 0, 87, 233, 51, 113, 224, 34, 0, 10, 253, 203, 12, 0, 100, 203, 0, 40, 255, 9, 0, 0, 57, 248, 0, 40, 255, 9, 0, 0, 57, 248, 0, 40, 255, 9, 0, 0, 57, 248, 0, 40,
	255, 8, 0, 0, 56, 248, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 240, 240, 234, 178, 38, 0, 52, 255, 4, 0,
	0, 116, 220, 0, 127, 74, 249, 16, 0, 0, 87, 245, 0, 74, 249, 16, 0, 0, 87, 245, 0, 74, 249, 16, 0, 0,
	87, 245, 0, 74, 249, 16, 0, 0, 87, 245, 0, 128, 211, 0, 0, 15, 248, 80, 0, 143, 187, 0, 0, 5, 234, 94,
	0, 195, 109, 0, 52, 255, 4, 0, 0, 0, 102, 228, 62, 5, 34, 99, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255,
	4, 0, 0, 0, 0, 8, 58, 255, 11, 8, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 7, 82, 239, 67, 0, 52, 255,
	4, 0, 75, 246, 255, 0, 0, 163, 173, 9, 23, 212, 114, 0, 0, 163, 173, 9, 23, 212, 114, 0, 0, 163, 173,
	9, 23, 212, 114, 0, 0, 163, 173, 9, 23, 212, 114, 0, 0, 163, 173, 9, 23, 212, 114, 0, 0, 166, 49, 0, 0, 108, 107, 0, 13, 237, 190, 15, 18, 206, 111, 0, 5, 232, 117, 10, 19, 158, 190, 0, 5, 232, 117, 10,
	19, 158, 190, 0, 5, 232, 117, 10, 19, 158, 190, 0, 5, 232, 107, 3, 9, 151, 190, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 34, 8, 2, 118, 240, 0, 127, 155, 179, 0, 0, 0, 11, 245, 0,
	155, 179, 0, 0, 0, 11, 245, 0, 155, 179, 0, 0, 0, 11, 245, 0, 155, 179, 0, 0, 0, 11, 245, 0, 208, 126,
	0, 0, 0, 176, 160, 0, 232, 94, 0, 0, 0, 144, 188, 0, 251, 31, 0, 52, 255, 244, 244, 0, 0, 0, 99, 214,
	255, 240, 135, 0, 52, 255, 244, 244, 244, 244, 99, 0, 52, 255, 244, 244, 244, 244, 99, 0, 52, 255,
	244, 244, 244, 244, 99, 0, 52, 255, 244, 244, 244, 244, 22, 0, 0, 244, 246, 255, 244, 244, 0, 0, 0,
	244, 246, 255, 244, 244, 0, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 244, 246, 255, 244, 244, 0, 0,
	52, 255, 244, 240, 191, 69, 0, 0, 52, 255, 4, 0, 1, 213, 255, 0, 0, 16, 171, 241, 237, 143, 3, 0, 0,
	16, 171, 241, 237, 143, 3, 0, 0, 16, 171, 241, 237, 143, 3, 0, 0, 16, 171, 241, 237, 143, 3, 0, 0, 16, 171, 241, 237, 143, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 157, 99, 185, 245, 234, 140, 2, 0, 0, 51, 197, 249,
	244, 177, 28, 0, 0, 51, 197, 249, 244, 177, 28, 0, 0, 51, 197, 249, 244, 177, 28, 0, 0, 51, 197, 244, 241, 177, 28, 0, 0, 0, 52, 255, 4, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 116, 240, 246, 211, 74, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 107, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 69, 194, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 233, 241, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 104, 226, 228, 110, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 8, 195, 125, 0, 0, 0, 0, 0, 0, 0, 0, 98, 212, 15, 0, 0, 0, 2, 197, 213, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 241, 45, 96, 248, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 157, 139, 0, 0, 0, 0, 0, 0, 0, 0, 154, 142, 0, 0, 0, 0, 10,
	199, 192, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 111, 190, 2, 0, 0, 0, 0, 0, 0, 0, 50, 225, 29, 0, 0, 0, 0,
	130, 241, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 187, 114, 0, 0, 0, 0, 0, 0, 0, 1, 161, 149, 1, 0, 0, 0, 24, 211, 193, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 217, 9, 0, 0, 0, 0, 0, 0, 0, 44, 223, 23, 0, 0, 0, 0, 88, 244, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 91, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 127, 0, 0, 18, 213, 67, 0, 0, 0, 0, 0, 0, 46, 220, 30, 0, 0, 0, 0, 115, 148, 123, 138, 0, 0, 0,
	14, 196, 237, 203, 189, 27, 0, 0, 0, 240, 3, 49, 244, 3, 0, 0, 0, 105, 226, 228, 110, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 155, 105, 0, 0, 0, 0, 0, 0, 121, 140, 0, 0, 0, 0, 0, 158, 74, 90, 145, 0, 0, 0, 0, 240, 3, 49, 244, 3, 0, 0, 0, 0, 158, 122, 0, 0, 0, 0, 0, 0, 12, 210, 56, 0, 0, 0, 0, 38,
	200, 77, 175, 0, 0, 0, 49, 244, 53, 244, 3, 0, 0, 0, 3, 138, 202, 114, 121, 12, 0, 0, 86, 232, 214,
	194, 37, 0, 0, 0, 0, 7, 172, 97, 0, 0, 0, 0, 0, 0, 144, 133, 0, 0, 0, 0, 6, 185, 54, 92, 153, 0, 0, 0, 25,
	206, 234, 199, 174, 3, 0, 0, 49, 244, 3, 49, 244, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 123, 138, 0, 0, 0, 0, 0, 0, 2, 195, 59, 0, 0, 0, 0, 11, 204, 78, 171, 0, 0, 0, 0, 49, 244, 53, 244, 3, 0, 0, 0, 0, 0, 51, 205, 26, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 49, 244, 53, 244, 3, 0, 0, 127, 0, 0, 0, 32,
	105, 0, 0, 0, 0, 0, 0, 91, 46, 0, 0, 0, 0, 2, 113, 7, 1, 111, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 117, 7, 0, 0, 0, 0, 0, 57, 72, 0, 0, 0, 0, 0, 74, 38, 0, 93, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51, 110, 93, 212, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 1, 109, 6, 0, 0, 0, 0, 0, 35, 80, 0, 0, 0, 0, 0, 49, 64, 0,
	96, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 103, 25, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 103, 213, 244, 239, 180, 36, 0, 0, 103, 213, 244, 240, 180, 36, 0, 0, 103, 213, 244,
	239, 180, 36, 0, 0, 103, 213, 244, 239, 180, 36, 0, 0, 103, 213, 244, 239, 180, 36, 0, 0, 103, 213,
	244, 239, 180, 36, 0, 0, 132, 239, 226, 131, 234, 236, 0, 0, 12, 151, 233, 244, 202, 40, 0, 0, 13,
	156, 236, 241, 165, 12, 0, 0, 13, 156, 236, 241, 165, 12, 0, 0, 13, 156, 236, 241, 165, 12, 0, 0, 13, 156, 235, 241, 165, 12, 0, 0, 0, 244, 246, 255, 4, 0, 0, 0, 0, 244, 246, 255, 4, 0, 0, 0, 0, 244, 246, 255, 4, 0, 0, 0, 148, 244, 255, 4, 0, 0, 0, 0, 25, 178, 243, 251, 214, 9, 0, 52, 255, 111, 233, 235,
	80, 0, 0, 0, 29, 182, 242, 237, 157, 11, 0, 0, 29, 182, 242, 237, 157, 11, 0, 0, 29, 182, 242, 237,
	157, 11, 0, 0, 29, 182, 242, 237, 157, 11, 0, 0, 29, 182, 242, 237, 157, 11, 0, 0, 0, 0, 49, 244, 3, 0, 0, 0, 29, 181, 242, 238, 160, 192, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52,
	255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 40, 250, 19, 0, 0, 50, 245, 0, 52, 255, 121,
	231, 241, 161, 10, 0, 93, 212, 0, 0, 0, 221, 87, 0, 127, 0, 93, 52, 2, 9, 162, 204, 0, 0, 93, 52, 2, 10, 165, 204, 0, 0, 93, 52, 2, 9, 162, 204, 0, 0, 93, 52, 2, 9, 162, 204, 0, 0, 93, 52, 2, 9, 162, 204, 0,
	0, 93, 52, 2, 9, 159, 204, 0, 0, 48, 4, 120, 255, 57, 104, 0, 0, 174, 188, 24, 0, 53, 43, 0, 0, 173,
	187, 16, 11, 178, 153, 0, 0, 173, 187, 16, 11, 178, 153, 0, 0, 173, 186, 16, 11, 179, 152, 0, 0, 173, 177, 14, 11, 179, 152, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 252, 4, 0, 0, 0, 0, 192, 167, 11, 4, 181, 123, 0, 52, 255, 121, 2, 133, 219, 0, 0, 0, 198,
	158, 8, 19, 198, 149, 0, 0, 198, 158, 8, 19, 198, 149, 0, 0, 198, 158, 8, 19, 198, 149, 0, 0, 198,
	158, 8, 19, 198, 149, 0, 0, 198, 158, 8, 19, 198, 149, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 198, 161, 9, 24,
	239, 165, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0,
	52, 255, 4, 0, 52, 255, 4, 0, 0, 200, 106, 0, 0, 142, 163, 0, 52, 255, 163, 9, 19, 198, 145, 0, 11,
	244, 36, 0, 48, 244, 10, 0, 127, 0, 0, 0, 0, 0, 61, 252, 0, 0, 0, 0, 0, 0, 62, 252, 0, 0, 0, 0, 0, 0, 61,
	252, 0, 0, 0, 0, 0, 0, 61, 252, 0, 0, 0, 0, 0, 0, 61, 252, 0, 0, 0, 0, 0, 0, 57, 252, 0, 0, 0, 0, 55, 255, 6, 54, 0, 22, 255, 41, 0, 0, 0, 0, 0, 21, 255, 50, 0, 0, 67, 236, 0, 21, 255, 50, 0, 0, 67, 236, 0, 21,
	255, 49, 0, 0, 70, 236, 0, 21, 255, 37, 0, 0, 70, 236, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4,
	0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 252, 4, 0, 0, 0, 25, 255, 35, 0, 0, 86, 217, 0, 52, 255, 18, 0,
	56, 254, 1, 0, 27, 255, 32, 0, 0, 81, 234, 0, 27, 255, 32, 0, 0, 81, 234, 0, 27, 255, 32, 0, 0, 81, 234, 0, 27, 255, 32, 0, 0, 81, 234, 0, 27, 255, 32, 0, 0, 81, 234, 0, 0, 240, 240, 240, 240, 240, 240, 0,
	27, 255, 31, 2, 168, 165, 235, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255,
	4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 103, 199, 0, 3, 230, 67, 0, 52, 255, 33, 0, 0, 81,
	232, 0, 0, 162, 117, 0, 130, 165, 0, 0, 127, 0, 78, 197, 237, 240, 243, 255, 0, 0, 71, 189, 224, 224, 230, 255, 0, 0, 78, 197, 237, 240, 243, 255, 0, 0, 78, 197, 237, 240, 243, 255, 0, 0, 78, 197, 237, 240, 243, 255, 0, 0, 78, 197, 237, 240, 243, 255, 0, 0, 125, 231, 244, 255, 240, 241, 0, 50, 255,
	7, 0, 0, 0, 0, 0, 47, 255, 225, 224, 224, 227, 239, 0, 47, 255, 225, 224, 224, 227, 239, 0, 47, 255,
	240, 240, 240, 241, 247, 0, 47, 255, 240, 240, 240, 241, 247, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0,
	52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 252, 4, 0, 0, 0, 48, 255, 7, 0, 0, 56, 254, 0, 52,
	255, 4, 0, 52, 255, 4, 0, 48, 255, 6, 0, 0, 55, 254, 0, 48, 255, 6, 0, 0, 55, 254, 0, 48, 255, 6, 0, 0,
	55, 254, 0, 48, 255, 6, 0, 0, 55, 254, 0, 48, 255, 6, 0, 0, 55, 254, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 255, 7, 152, 111, 54, 255, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 52,
	255, 4, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 15, 246, 36, 70, 225, 1, 0, 52, 255, 6, 0, 0, 55, 254, 0, 0,
	68, 197, 0, 212, 76, 0, 0, 127, 24, 251, 95, 6, 0, 62, 255, 0, 23, 250, 82, 2, 0, 65, 255, 0, 24, 251,
	86, 5, 0, 65, 255, 0, 24, 251, 86, 5, 0, 65, 255, 0, 24, 251, 86, 5, 0, 65, 255, 0, 24, 251, 86, 5, 0,
	65, 255, 0, 32, 255, 57, 53, 255, 8, 0, 0, 22, 255, 41, 0, 0, 0, 0, 0, 22, 255, 23, 0, 0, 0, 0, 0, 22,
	255, 23, 0, 0, 0, 0, 0, 22, 255, 21, 0, 0, 0, 0, 0, 22, 255, 21, 0, 0, 0, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 252, 4, 0, 0, 0, 25, 255, 34, 0, 0, 83, 238, 0, 52, 255, 4, 0, 52, 255, 4, 0, 27, 255, 32, 0, 0, 81, 234, 0, 27, 255, 32, 0, 0, 81, 234, 0, 27, 255,
	32, 0, 0, 81, 234, 0, 27, 255, 32, 0, 0, 81, 234, 0, 27, 255, 32, 0, 0, 81, 234, 0, 0, 0, 0, 48, 240, 3,
	0, 0, 27, 255, 165, 129, 0, 81, 234, 0, 48, 255, 8, 0, 66, 255, 4, 0, 48, 255, 8, 0, 66, 255, 4, 0, 48,
	255, 8, 0, 66, 255, 4, 0, 48, 255, 8, 0, 66, 255, 4, 0, 0, 0, 166, 128, 162, 131, 0, 0, 52, 255, 32, 0,
	0, 80, 233, 0, 0, 2, 226, 62, 238, 5, 0, 0, 127, 33, 255, 38, 0, 4, 170, 255, 0, 33, 255, 69, 0, 26,
	189, 255, 0, 33, 255, 68, 0, 26, 189, 255, 0, 33, 255, 68, 0, 26, 189, 255, 0, 33, 255, 68, 0, 26,
	189, 255, 0, 33, 255, 68, 0, 26, 189, 255, 0, 38, 255, 39, 95, 255, 76, 1, 0, 0, 177, 185, 23, 0, 50,
	42, 0, 0, 176, 165, 17, 0, 32, 76, 0, 0, 176, 165, 17, 0, 32, 76, 0, 0, 176, 162, 17, 0, 32, 76, 0, 0,
	176, 162, 17, 0, 29, 79, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 252, 4, 0, 0, 0, 0, 194, 163, 8, 21, 202, 156, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 200, 158, 7, 19, 198, 151, 0, 0, 200, 158, 7, 19, 198, 151, 0, 0, 200, 158, 7, 19, 198, 151, 0, 0, 200, 158, 7,
	19, 198, 151, 0, 0, 200, 158, 7, 19, 198, 151, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 216, 210, 7, 19, 198, 151,
	0, 15, 251, 62, 0, 147, 255, 4, 0, 15, 251, 62, 0, 147, 255, 4, 0, 15, 251, 62, 0, 147, 255, 4, 0, 15,
	251, 62, 0, 147, 255, 4, 0, 0, 0, 70, 222, 242, 36, 0, 0, 52, 255, 160, 8, 17, 195, 146, 0, 0, 0, 137,
	215, 157, 0, 0, 0, 127, 0, 114, 219, 196, 189, 127, 255, 0, 0, 114, 231, 243, 204, 107, 255, 0, 0,
	114, 231, 243, 204, 108, 255, 0, 0, 114, 231, 243, 204, 108, 255, 0, 0, 114, 231, 243, 204, 108,
	255, 0, 0, 113, 230, 243, 205, 106, 255, 0, 0, 154, 245, 215, 95, 224, 242, 0, 0, 14, 153, 237, 255, 207, 42, 0, 0, 14, 155, 234, 244, 218, 111, 0, 0, 14, 155, 234, 244, 218, 111, 0, 0, 14, 155, 234,
	244, 218, 111, 0, 0, 14, 153, 233, 244, 220, 120, 0, 0, 118, 244, 246, 255, 244, 244, 0, 0, 108,
	224, 230, 255, 224, 224, 0, 0, 108, 224, 230, 255, 224, 224, 0, 22, 244, 244, 255, 244, 244, 34, 0, 0, 27, 180, 242, 238, 161, 13, 0, 52, 255, 4, 0, 52, 255, 4, 0, 0, 31, 184, 243, 238, 160, 12, 0, 0,
	31, 184, 243, 238, 160, 12, 0, 0, 31, 184, 243, 238, 160, 12, 0, 0, 31, 184, 243, 238, 160, 12, 0, 0, 31, 184, 243, 238, 160, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 95, 164, 178, 242, 238, 160, 12, 0, 0, 120,
	234, 191, 137, 255, 4, 0, 0, 120, 234, 191, 137, 255, 4, 0, 0, 120, 234, 191, 137, 255, 4, 0, 0, 120, 234, 191, 137, 255, 4, 0, 0, 0, 2, 227, 198, 0, 0, 0, 52, 255, 120, 232, 241, 160, 10, 0, 0, 0, 43,
	255, 72, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 14, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 199, 105, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 18, 232, 4, 0,
	0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 1, 167, 67, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 244, 16, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 0, 0, 132, 148, 0, 0, 0, 0, 127,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 242, 213, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 203, 237, 92, 0, 0, 0, 0, 52, 255, 4, 0, 0, 0, 0, 0, 11, 243, 209, 25, 0, 0, 0, 0, 0, 127,
	127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127,
	0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0,
	127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127,
	127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127,
	127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127, 127, 0, 127, 127, 127, 127, 127, 127,
	127, 0, 127, 127, 127, 127, 127, 127, 127, 0
};

void TwGenerateDefaultFonts(float _Scaling) {
	g_DefaultSmallFont = TwGenerateFont(s_Font0, FONT0_BM_W, FONT0_BM_H, _Scaling);
	DEV_ASSERT(g_DefaultSmallFont && g_DefaultSmallFont->m_NbCharRead == 224);
	g_DefaultNormalFont = TwGenerateFont(s_Font1AA, FONT1AA_BM_W, FONT1AA_BM_H, _Scaling);
	DEV_ASSERT(g_DefaultNormalFont && g_DefaultNormalFont->m_NbCharRead == 224);
	g_DefaultLargeFont = TwGenerateFont(s_Font2AA, FONT2AA_BM_W, FONT2AA_BM_H, _Scaling);
	DEV_ASSERT(g_DefaultLargeFont && g_DefaultLargeFont->m_NbCharRead == 224);
	g_DefaultFixed1Font = TwGenerateFont(s_FontFixed1, FONTFIXED1_BM_W, FONTFIXED1_BM_H, _Scaling);
	DEV_ASSERT(g_DefaultFixed1Font && g_DefaultFixed1Font->m_NbCharRead == 224);
}

//  ---------------------------------------------------------------------------

void TwDeleteDefaultFonts() {
	delete g_DefaultSmallFont;
	g_DefaultSmallFont = NULL;
	delete g_DefaultNormalFont;
	g_DefaultNormalFont = NULL;
	delete g_DefaultLargeFont;
	g_DefaultLargeFont = NULL;
	delete g_DefaultFixed1Font;
	g_DefaultFixed1Font = NULL;
}

//  ---------------------------------------------------------------------------

//  ---------------------------------------------------------------------------
//  @file       TwMgr.cpp
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

CTwMgr *g_TwMgr = nullptr; // current TwMgr
bool g_BreakOnError = false;
TwErrorHandler g_ErrorHandler = nullptr;
int g_TabLength = 4;
CTwBar *const TW_GLOBAL_BAR = (CTwBar *)(-1);
int g_InitWndWidth = -1;
int g_InitWndHeight = -1;
TwCopyCDStringToClient g_InitCopyCDStringToClient = nullptr;
TwCopyStdStringToClient g_InitCopyStdStringToClient = nullptr;
float g_FontScaling = 1;

// multi-windows
const int TW_MASTER_WINDOW_ID = 0;
typedef std::map<int, CTwMgr *> CTwWndMap;
CTwWndMap g_Wnds;
CTwMgr *g_TwMasterMgr = nullptr;

// error messages
extern const char *g_ErrUnknownAttrib;
extern const char *g_ErrNoValue;
extern const char *g_ErrBadValue;
const char *g_ErrInit = "Already initialized";
const char *g_ErrShut = "Already shutdown";
const char *g_ErrNotInit = "Not initialized";
const char *g_ErrUnknownAPI = "Unsupported graph API";
const char *g_ErrBadDevice = "Invalid graph device";
const char *g_ErrBadParam = "Invalid parameter";
const char *g_ErrExist = "Exists already";
const char *g_ErrNotFound = "Not found";
const char *g_ErrNthToDo = "Nothing to do";
const char *g_ErrBadSize = "Bad size";
const char *g_ErrIsDrawing = "Asynchronous drawing detected";
const char *g_ErrIsProcessing = "Asynchronous processing detected";
const char *g_ErrOffset = "Offset larger than StructSize";
const char *g_ErrDelStruct = "Cannot delete a struct member";
const char *g_ErrNoBackQuote = "Name cannot include back-quote";
const char *g_ErrStdString = "Debug/Release std::string mismatch";
const char *g_ErrCStrParam = "Value count for TW_PARAM_CSTRING must be 1";
const char *g_ErrOutOfRange = "Index out of range";
const char *g_ErrHasNoValue = "Has no value";
const char *g_ErrBadType = "Incompatible type";
const char *g_ErrDelHelp = "Cannot delete help bar";
char g_ErrParse[512];

void ANT_CALL TwGlobalError(const char *_ErrorMessage);

//  ---------------------------------------------------------------------------

#if !defined(FLOAT_EPS)
#define FLOAT_EPS 1.0e-7f
#endif // !defined(FLOAT_EPS)
#if !defined(FLOAT_EPS_SQ)
#define FLOAT_EPS_SQ 1.0e-14f
#endif // !defined(FLOAT_EPS_SQ)
#if !defined(FLOAT_PI)
#define FLOAT_PI 3.14159265358979323846f
#endif
#if !defined(DOUBLE_EPS)
#define DOUBLE_EPS 1.0e-14
#endif // !defined(DOUBLE_EPS)
#if !defined(DOUBLE_EPS_SQ)
#define DOUBLE_EPS_SQ 1.0e-28
#endif // !defined(DOUBLE_EPS_SQ)
#if !defined(DOUBLE_PI)
#define DOUBLE_PI 3.14159265358979323846
#endif

inline double DegToRad(double degree) { return degree * (DOUBLE_PI / 180.0); }
inline double RadToDeg(double radian) { return radian * (180.0 / DOUBLE_PI); }

//  ---------------------------------------------------------------------------

//  a static global object to verify that Tweakbar module has been properly terminated (in debug mode only)
#ifdef _DEBUG
static struct CTwVerif {
	~CTwVerif() {
		if (g_TwMgr != NULL)
			g_TwMgr->SetLastError("Tweak bar module has not been terminated properly: call TwTerminate()\n");
	}
} s_Verif;
#endif // _DEBUG

//  ---------------------------------------------------------------------------
//  Color ext type
//  ---------------------------------------------------------------------------

void CColorExt::RGB2HLS() {
	float fH = 0, fL = 0, fS = 0;
	ColorRGBToHLSf((float)R / 255.0, (float)G / 255.0f, (float)B / 255.0f, &fH, &fL, &fS);
	H = (int)fH;
	if (H >= 360)
		H -= 360;
	else if (H < 0)
		H += 360;
	L = (int)(255.0 * fL + 0.5);
	if (L < 0)
		L = 0;
	else if (L > 255)
		L = 255;
	S = (int)(255.0 * fS + 0.5);
	if (S < 0)
		S = 0;
	else if (S > 255)
		S = 255;
}

void CColorExt::HLS2RGB() {
	float fR = 0, fG = 0, fB = 0;
	ColorHLSToRGBf((float)H, (float)L / 255.0, (float)S / 255.0f, &fR, &fG, &fB);
	R = (int)(255.0f * fR + 0.5f);
	if (R < 0)
		R = 0;
	else if (R > 255)
		R = 255;
	G = (int)(255.0 * fG + 0.5);
	if (G < 0)
		G = 0;
	else if (G > 255)
		G = 255;
	B = (int)(255.0 * fB + 0.5);
	if (B < 0)
		B = 0;
	else if (B > 255)
		B = 255;
}

void ANT_CALL CColorExt::InitColor32CB(void *_ExtValue, void *_ClientData) {
	CColorExt *ext = static_cast<CColorExt *>(_ExtValue);
	if (ext) {
		ext->m_IsColorF = false;
		ext->R = 0;
		ext->G = 0;
		ext->B = 0;
		ext->H = 0;
		ext->L = 0;
		ext->S = 0;
		ext->A = 255;
		ext->m_HLS = false;
		ext->m_HasAlpha = false;
		ext->m_CanHaveAlpha = true;
		ext->m_PrevConvertedColor = Color32FromARGBi(ext->A, ext->R, ext->G, ext->B);
		ext->m_StructProxy = (CTwMgr::CStructProxy *)_ClientData;
	}
}

void ANT_CALL CColorExt::InitColor3FCB(void *_ExtValue, void *_ClientData) {
	InitColor32CB(_ExtValue, _ClientData);
	CColorExt *ext = static_cast<CColorExt *>(_ExtValue);
	if (ext) {
		ext->m_IsColorF = true;
		ext->m_HasAlpha = false;
		ext->m_CanHaveAlpha = false;
	}
}

void ANT_CALL CColorExt::InitColor4FCB(void *_ExtValue, void *_ClientData) {
	InitColor32CB(_ExtValue, _ClientData);
	CColorExt *ext = static_cast<CColorExt *>(_ExtValue);
	if (ext) {
		ext->m_IsColorF = true;
		ext->m_HasAlpha = true;
		ext->m_CanHaveAlpha = true;
	}
}

void ANT_CALL CColorExt::CopyVarFromExtCB(void *_VarValue, const void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData) {
	unsigned int *var32 = static_cast<unsigned int *>(_VarValue);
	float *varF = static_cast<float *>(_VarValue);
	CColorExt *ext = (CColorExt *)(_ExtValue);
	CTwMgr::CMemberProxy *mProxy = static_cast<CTwMgr::CMemberProxy *>(_ClientData);
	if (_VarValue && ext) {
		if (ext->m_HasAlpha && mProxy && mProxy->m_StructProxy && mProxy->m_StructProxy->m_Type == g_TwMgr->m_TypeColor3F)
			ext->m_HasAlpha = false;

		// Synchronize HLS and RGB
		if (_ExtMemberIndex >= 0 && _ExtMemberIndex <= 2)
			ext->RGB2HLS();
		else if (_ExtMemberIndex >= 3 && _ExtMemberIndex <= 5)
			ext->HLS2RGB();
		else if (mProxy && _ExtMemberIndex == 7 && mProxy->m_VarParent) {
			assert(mProxy->m_VarParent->m_Vars.size() == 8);
			if (mProxy->m_VarParent->m_Vars[0]->m_Visible != !ext->m_HLS || mProxy->m_VarParent->m_Vars[1]->m_Visible != !ext->m_HLS || mProxy->m_VarParent->m_Vars[2]->m_Visible != !ext->m_HLS || mProxy->m_VarParent->m_Vars[3]->m_Visible != ext->m_HLS || mProxy->m_VarParent->m_Vars[4]->m_Visible != ext->m_HLS || mProxy->m_VarParent->m_Vars[5]->m_Visible != ext->m_HLS) {
				mProxy->m_VarParent->m_Vars[0]->m_Visible = !ext->m_HLS;
				mProxy->m_VarParent->m_Vars[1]->m_Visible = !ext->m_HLS;
				mProxy->m_VarParent->m_Vars[2]->m_Visible = !ext->m_HLS;
				mProxy->m_VarParent->m_Vars[3]->m_Visible = ext->m_HLS;
				mProxy->m_VarParent->m_Vars[4]->m_Visible = ext->m_HLS;
				mProxy->m_VarParent->m_Vars[5]->m_Visible = ext->m_HLS;
				mProxy->m_Bar->NotUpToDate();
			}
			if (mProxy->m_VarParent->m_Vars[6]->m_Visible != ext->m_HasAlpha) {
				mProxy->m_VarParent->m_Vars[6]->m_Visible = ext->m_HasAlpha;
				mProxy->m_Bar->NotUpToDate();
			}
			if (static_cast<CTwVarAtom *>(mProxy->m_VarParent->m_Vars[7])->m_ReadOnly) {
				static_cast<CTwVarAtom *>(mProxy->m_VarParent->m_Vars[7])->m_ReadOnly = false;
				mProxy->m_Bar->NotUpToDate();
			}
		}
		// Convert to color32
		color32 col = Color32FromARGBi((ext->m_HasAlpha ? ext->A : 255), ext->R, ext->G, ext->B);
		if (ext->m_OGL && !ext->m_IsColorF)
			col = (col & 0xff00ff00) | (unsigned char)(col >> 16) | (((unsigned char)(col)) << 16);
		if (ext->m_IsColorF)
			Color32ToARGBf(col, (ext->m_HasAlpha ? varF + 3 : NULL), varF + 0, varF + 1, varF + 2);
		else {
			if (ext->m_HasAlpha)
				*var32 = col;
			else
				*var32 = ((*var32) & 0xff000000) | (col & 0x00ffffff);
		}
		ext->m_PrevConvertedColor = col;
	}
}

void ANT_CALL CColorExt::CopyVarToExtCB(const void *_VarValue, void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData) {
	const unsigned int *var32 = static_cast<const unsigned int *>(_VarValue);
	const float *varF = static_cast<const float *>(_VarValue);
	CColorExt *ext = static_cast<CColorExt *>(_ExtValue);
	CTwMgr::CMemberProxy *mProxy = static_cast<CTwMgr::CMemberProxy *>(_ClientData);
	if (_VarValue && ext) {
		if (ext->m_HasAlpha && mProxy && mProxy->m_StructProxy && mProxy->m_StructProxy->m_Type == g_TwMgr->m_TypeColor3F)
			ext->m_HasAlpha = false;

		if (mProxy && _ExtMemberIndex == 7 && mProxy->m_VarParent) {
			assert(mProxy->m_VarParent->m_Vars.size() == 8);
			if (mProxy->m_VarParent->m_Vars[0]->m_Visible != !ext->m_HLS || mProxy->m_VarParent->m_Vars[1]->m_Visible != !ext->m_HLS || mProxy->m_VarParent->m_Vars[2]->m_Visible != !ext->m_HLS || mProxy->m_VarParent->m_Vars[3]->m_Visible != ext->m_HLS || mProxy->m_VarParent->m_Vars[4]->m_Visible != ext->m_HLS || mProxy->m_VarParent->m_Vars[5]->m_Visible != ext->m_HLS) {
				mProxy->m_VarParent->m_Vars[0]->m_Visible = !ext->m_HLS;
				mProxy->m_VarParent->m_Vars[1]->m_Visible = !ext->m_HLS;
				mProxy->m_VarParent->m_Vars[2]->m_Visible = !ext->m_HLS;
				mProxy->m_VarParent->m_Vars[3]->m_Visible = ext->m_HLS;
				mProxy->m_VarParent->m_Vars[4]->m_Visible = ext->m_HLS;
				mProxy->m_VarParent->m_Vars[5]->m_Visible = ext->m_HLS;
				mProxy->m_Bar->NotUpToDate();
			}
			if (mProxy->m_VarParent->m_Vars[6]->m_Visible != ext->m_HasAlpha) {
				mProxy->m_VarParent->m_Vars[6]->m_Visible = ext->m_HasAlpha;
				mProxy->m_Bar->NotUpToDate();
			}
			if (static_cast<CTwVarAtom *>(mProxy->m_VarParent->m_Vars[7])->m_ReadOnly) {
				static_cast<CTwVarAtom *>(mProxy->m_VarParent->m_Vars[7])->m_ReadOnly = false;
				mProxy->m_Bar->NotUpToDate();
			}
		}
		color32 col;
		if (ext->m_IsColorF)
			col = Color32FromARGBf((ext->m_HasAlpha ? varF[3] : 1), varF[0], varF[1], varF[2]);
		else
			col = *var32;
		if (ext->m_OGL && !ext->m_IsColorF)
			col = (col & 0xff00ff00) | (unsigned char)(col >> 16) | (((unsigned char)(col)) << 16);
		Color32ToARGBi(col, (ext->m_HasAlpha ? &ext->A : NULL), &ext->R, &ext->G, &ext->B);
		if ((col & 0x00ffffff) != (ext->m_PrevConvertedColor & 0x00ffffff))
			ext->RGB2HLS();
		ext->m_PrevConvertedColor = col;
	}
}

void ANT_CALL CColorExt::SummaryCB(char *_SummaryString, size_t /*_SummaryMaxLength*/, const void *_ExtValue, void * /*_ClientData*/) {
	// copy var
	CColorExt *ext = (CColorExt *)(_ExtValue);
	if (ext && ext->m_StructProxy && ext->m_StructProxy->m_StructData) {
		if (ext->m_StructProxy->m_StructGetCallback)
			ext->m_StructProxy->m_StructGetCallback(ext->m_StructProxy->m_StructData, ext->m_StructProxy->m_StructClientData);
		// if (*(unsigned int *)(ext->m_StructProxy->m_StructData)!=ext->m_PrevConvertedColor)
		CopyVarToExtCB(ext->m_StructProxy->m_StructData, ext, 99, NULL);
	}

	_SummaryString[0] = ' '; // required to force background color for this value
	_SummaryString[1] = '\0';
}

void CColorExt::CreateTypes() {
	if (g_TwMgr == NULL)
		return;
	TwStructMember ColorExtMembers[] = { { "Red", TW_TYPE_INT32, offsetof(CColorExt, R), "min=0 max=255" },
		{ "Green", TW_TYPE_INT32, offsetof(CColorExt, G), "min=0 max=255" },
		{ "Blue", TW_TYPE_INT32, offsetof(CColorExt, B), "min=0 max=255" },
		{ "Hue", TW_TYPE_INT32, offsetof(CColorExt, H), "hide min=0 max=359" },
		{ "Lightness", TW_TYPE_INT32, offsetof(CColorExt, L), "hide min=0 max=255" },
		{ "Saturation", TW_TYPE_INT32, offsetof(CColorExt, S), "hide min=0 max=255" },
		{ "Alpha", TW_TYPE_INT32, offsetof(CColorExt, A), "hide min=0 max=255" },
		{ "Mode", TW_TYPE_BOOLCPP, offsetof(CColorExt, m_HLS), "true='HLS' false='RGB' readwrite" } };
	g_TwMgr->m_TypeColor32 = TwDefineStructExt("COLOR32", ColorExtMembers, 8, sizeof(unsigned int), sizeof(CColorExt), CColorExt::InitColor32CB, CColorExt::CopyVarFromExtCB, CColorExt::CopyVarToExtCB, CColorExt::SummaryCB, CTwMgr::CStruct::s_PassProxyAsClientData, "A 32-bit-encoded color.");
	g_TwMgr->m_TypeColor3F = TwDefineStructExt("COLOR3F", ColorExtMembers, 8, 3 * sizeof(float), sizeof(CColorExt), CColorExt::InitColor3FCB, CColorExt::CopyVarFromExtCB, CColorExt::CopyVarToExtCB, CColorExt::SummaryCB, CTwMgr::CStruct::s_PassProxyAsClientData, "A 3-floats-encoded RGB color.");
	g_TwMgr->m_TypeColor4F = TwDefineStructExt("COLOR4F", ColorExtMembers, 8, 4 * sizeof(float), sizeof(CColorExt), CColorExt::InitColor4FCB, CColorExt::CopyVarFromExtCB, CColorExt::CopyVarToExtCB, CColorExt::SummaryCB, CTwMgr::CStruct::s_PassProxyAsClientData, "A 4-floats-encoded RGBA color.");
	// Do not name them "TW_COLOR*" because the name is displayed in the help bar.
}

//  ---------------------------------------------------------------------------
//  Quaternion ext type
//  ---------------------------------------------------------------------------

void ANT_CALL CQuaternionExt::InitQuat4FCB(void *_ExtValue, void *_ClientData) {
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(_ExtValue);
	if (ext) {
		ext->Qx = ext->Qy = ext->Qz = 0;
		ext->Qs = 1;
		ext->Vx = 1;
		ext->Vy = ext->Vz = 0;
		ext->Angle = 0;
		ext->Dx = ext->Dy = ext->Dz = 0;
		ext->m_AAMode = false; // Axis & angle mode hidden
		ext->m_ShowVal = false;
		ext->m_IsFloat = true;
		ext->m_IsDir = false;
		ext->m_Dir[0] = ext->m_Dir[1] = ext->m_Dir[2] = 0;
		ext->m_DirColor = 0xffffff00;
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				ext->m_Permute[i][j] = (i == j) ? 1 : 0;
		ext->m_StructProxy = (CTwMgr::CStructProxy *)_ClientData;
		ext->ConvertToAxisAngle();
		ext->m_Highlighted = false;
		ext->m_Rotating = false;
		if (ext->m_StructProxy != NULL) {
			ext->m_StructProxy->m_CustomDrawCallback = CQuaternionExt::DrawCB;
			ext->m_StructProxy->m_CustomMouseButtonCallback = CQuaternionExt::MouseButtonCB;
			ext->m_StructProxy->m_CustomMouseMotionCallback = CQuaternionExt::MouseMotionCB;
			ext->m_StructProxy->m_CustomMouseLeaveCallback = CQuaternionExt::MouseLeaveCB;
		}
	}
}

void ANT_CALL CQuaternionExt::InitQuat4DCB(void *_ExtValue, void *_ClientData) {
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(_ExtValue);
	if (ext) {
		ext->Qx = ext->Qy = ext->Qz = 0;
		ext->Qs = 1;
		ext->Vx = 1;
		ext->Vy = ext->Vz = 0;
		ext->Angle = 0;
		ext->Dx = ext->Dy = ext->Dz = 0;
		ext->m_AAMode = false; // Axis & angle mode hidden
		ext->m_ShowVal = false;
		ext->m_IsFloat = false;
		ext->m_IsDir = false;
		ext->m_Dir[0] = ext->m_Dir[1] = ext->m_Dir[2] = 0;
		ext->m_DirColor = 0xffffff00;
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				ext->m_Permute[i][j] = (i == j) ? 1 : 0;
		ext->m_StructProxy = (CTwMgr::CStructProxy *)_ClientData;
		ext->ConvertToAxisAngle();
		ext->m_Highlighted = false;
		ext->m_Rotating = false;
		if (ext->m_StructProxy != NULL) {
			ext->m_StructProxy->m_CustomDrawCallback = CQuaternionExt::DrawCB;
			ext->m_StructProxy->m_CustomMouseButtonCallback = CQuaternionExt::MouseButtonCB;
			ext->m_StructProxy->m_CustomMouseMotionCallback = CQuaternionExt::MouseMotionCB;
			ext->m_StructProxy->m_CustomMouseLeaveCallback = CQuaternionExt::MouseLeaveCB;
		}
	}
}

void ANT_CALL CQuaternionExt::InitDir3FCB(void *_ExtValue, void *_ClientData) {
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(_ExtValue);
	if (ext) {
		ext->Qx = ext->Qy = ext->Qz = 0;
		ext->Qs = 1;
		ext->Vx = 1;
		ext->Vy = ext->Vz = 0;
		ext->Angle = 0;
		ext->Dx = 1;
		ext->Dy = ext->Dz = 0;
		ext->m_AAMode = false; // Axis & angle mode hidden
		ext->m_ShowVal = true;
		ext->m_IsFloat = true;
		ext->m_IsDir = true;
		ext->m_Dir[0] = ext->m_Dir[1] = ext->m_Dir[2] = 0;
		ext->m_DirColor = 0xffffff00;
		int i, j;
		for (i = 0; i < 3; ++i)
			for (j = 0; j < 3; ++j)
				ext->m_Permute[i][j] = (i == j) ? 1 : 0;
		ext->m_StructProxy = (CTwMgr::CStructProxy *)_ClientData;
		ext->ConvertToAxisAngle();
		ext->m_Highlighted = false;
		ext->m_Rotating = false;
		if (ext->m_StructProxy != NULL) {
			ext->m_StructProxy->m_CustomDrawCallback = CQuaternionExt::DrawCB;
			ext->m_StructProxy->m_CustomMouseButtonCallback = CQuaternionExt::MouseButtonCB;
			ext->m_StructProxy->m_CustomMouseMotionCallback = CQuaternionExt::MouseMotionCB;
			ext->m_StructProxy->m_CustomMouseLeaveCallback = CQuaternionExt::MouseLeaveCB;
		}
	}
}

void ANT_CALL CQuaternionExt::InitDir3DCB(void *_ExtValue, void *_ClientData) {
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(_ExtValue);
	if (ext) {
		ext->Qx = ext->Qy = ext->Qz = 0;
		ext->Qs = 1;
		ext->Vx = 1;
		ext->Vy = ext->Vz = 0;
		ext->Angle = 0;
		ext->Dx = 1;
		ext->Dy = ext->Dz = 0;
		ext->m_AAMode = false; // Axis & angle mode hidden
		ext->m_ShowVal = true;
		ext->m_IsFloat = false;
		ext->m_IsDir = true;
		ext->m_Dir[0] = ext->m_Dir[1] = ext->m_Dir[2] = 0;
		ext->m_DirColor = 0xffffff00;
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				ext->m_Permute[i][j] = (i == j) ? 1 : 0;
		ext->m_StructProxy = (CTwMgr::CStructProxy *)_ClientData;
		ext->ConvertToAxisAngle();
		ext->m_Highlighted = false;
		ext->m_Rotating = false;
		if (ext->m_StructProxy != NULL) {
			ext->m_StructProxy->m_CustomDrawCallback = CQuaternionExt::DrawCB;
			ext->m_StructProxy->m_CustomMouseButtonCallback = CQuaternionExt::MouseButtonCB;
			ext->m_StructProxy->m_CustomMouseMotionCallback = CQuaternionExt::MouseMotionCB;
			ext->m_StructProxy->m_CustomMouseLeaveCallback = CQuaternionExt::MouseLeaveCB;
		}
	}
}

void ANT_CALL CQuaternionExt::CopyVarFromExtCB(void *_VarValue, const void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData) {
	CQuaternionExt *ext = (CQuaternionExt *)(_ExtValue);
	CTwMgr::CMemberProxy *mProxy = static_cast<CTwMgr::CMemberProxy *>(_ClientData);
	if (_VarValue && ext) {
		if (_ExtMemberIndex >= 4 && _ExtMemberIndex <= 7) { // Synchronize Quat and AxisAngle
			ext->ConvertToAxisAngle();
			if (_ExtMemberIndex == 4 && mProxy && mProxy->m_VarParent) { // show/hide quat values
				DEV_ASSERT(mProxy->m_VarParent->m_Vars.size() == 16);
				bool visible = ext->m_ShowVal;
				if (ext->m_IsDir) {
					if (mProxy->m_VarParent->m_Vars[13]->m_Visible != visible || mProxy->m_VarParent->m_Vars[14]->m_Visible != visible || mProxy->m_VarParent->m_Vars[15]->m_Visible != visible) {
						mProxy->m_VarParent->m_Vars[13]->m_Visible = visible;
						mProxy->m_VarParent->m_Vars[14]->m_Visible = visible;
						mProxy->m_VarParent->m_Vars[15]->m_Visible = visible;
						mProxy->m_Bar->NotUpToDate();
					}
				} else {
					if (mProxy->m_VarParent->m_Vars[4]->m_Visible != visible || mProxy->m_VarParent->m_Vars[5]->m_Visible != visible || mProxy->m_VarParent->m_Vars[6]->m_Visible != visible || mProxy->m_VarParent->m_Vars[7]->m_Visible != visible) {
						mProxy->m_VarParent->m_Vars[4]->m_Visible = visible;
						mProxy->m_VarParent->m_Vars[5]->m_Visible = visible;
						mProxy->m_VarParent->m_Vars[6]->m_Visible = visible;
						mProxy->m_VarParent->m_Vars[7]->m_Visible = visible;
						mProxy->m_Bar->NotUpToDate();
					}
				}
			}
		} else if (_ExtMemberIndex >= 8 && _ExtMemberIndex <= 11)
			ext->ConvertFromAxisAngle();
		else if (mProxy && _ExtMemberIndex == 12 && mProxy->m_VarParent && !ext->m_IsDir) {
			DEV_ASSERT(mProxy->m_VarParent->m_Vars.size() == 16);
			bool aa = ext->m_AAMode;
			if (mProxy->m_VarParent->m_Vars[4]->m_Visible != !aa || mProxy->m_VarParent->m_Vars[5]->m_Visible != !aa || mProxy->m_VarParent->m_Vars[6]->m_Visible != !aa || mProxy->m_VarParent->m_Vars[7]->m_Visible != !aa || mProxy->m_VarParent->m_Vars[8]->m_Visible != aa || mProxy->m_VarParent->m_Vars[9]->m_Visible != aa || mProxy->m_VarParent->m_Vars[10]->m_Visible != aa || mProxy->m_VarParent->m_Vars[11]->m_Visible != aa) {
				mProxy->m_VarParent->m_Vars[4]->m_Visible = !aa;
				mProxy->m_VarParent->m_Vars[5]->m_Visible = !aa;
				mProxy->m_VarParent->m_Vars[6]->m_Visible = !aa;
				mProxy->m_VarParent->m_Vars[7]->m_Visible = !aa;
				mProxy->m_VarParent->m_Vars[8]->m_Visible = aa;
				mProxy->m_VarParent->m_Vars[9]->m_Visible = aa;
				mProxy->m_VarParent->m_Vars[10]->m_Visible = aa;
				mProxy->m_VarParent->m_Vars[11]->m_Visible = aa;
				mProxy->m_Bar->NotUpToDate();
			}
			if (static_cast<CTwVarAtom *>(mProxy->m_VarParent->m_Vars[12])->m_ReadOnly) {
				static_cast<CTwVarAtom *>(mProxy->m_VarParent->m_Vars[12])->m_ReadOnly = false;
				mProxy->m_Bar->NotUpToDate();
			}
		}

		if (ext->m_IsFloat) {
			float *var = static_cast<float *>(_VarValue);
			if (ext->m_IsDir) {
				var[0] = (float)ext->Dx;
				var[1] = (float)ext->Dy;
				var[2] = (float)ext->Dz;
			} else { // quat
				var[0] = (float)ext->Qx;
				var[1] = (float)ext->Qy;
				var[2] = (float)ext->Qz;
				var[3] = (float)ext->Qs;
			}
		} else {
			double *var = static_cast<double *>(_VarValue);
			if (ext->m_IsDir) {
				var[0] = ext->Dx;
				var[1] = ext->Dy;
				var[2] = ext->Dz;
			} else { // quat
				var[0] = ext->Qx;
				var[1] = ext->Qy;
				var[2] = ext->Qz;
				var[3] = ext->Qs;
			}
		}
	}
}

void ANT_CALL CQuaternionExt::CopyVarToExtCB(const void *_VarValue, void *_ExtValue, unsigned int _ExtMemberIndex, void *_ClientData) {
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(_ExtValue);
	CTwMgr::CMemberProxy *mProxy = static_cast<CTwMgr::CMemberProxy *>(_ClientData);
	(void)mProxy;
	if (_VarValue && ext) {
		if (mProxy && _ExtMemberIndex == 12 && mProxy->m_VarParent && !ext->m_IsDir) {
			DEV_ASSERT(mProxy->m_VarParent->m_Vars.size() == 16);
			bool aa = ext->m_AAMode;
			if (mProxy->m_VarParent->m_Vars[4]->m_Visible != !aa || mProxy->m_VarParent->m_Vars[5]->m_Visible != !aa || mProxy->m_VarParent->m_Vars[6]->m_Visible != !aa || mProxy->m_VarParent->m_Vars[7]->m_Visible != !aa || mProxy->m_VarParent->m_Vars[8]->m_Visible != aa || mProxy->m_VarParent->m_Vars[9]->m_Visible != aa || mProxy->m_VarParent->m_Vars[10]->m_Visible != aa || mProxy->m_VarParent->m_Vars[11]->m_Visible != aa) {
				mProxy->m_VarParent->m_Vars[4]->m_Visible = !aa;
				mProxy->m_VarParent->m_Vars[5]->m_Visible = !aa;
				mProxy->m_VarParent->m_Vars[6]->m_Visible = !aa;
				mProxy->m_VarParent->m_Vars[7]->m_Visible = !aa;
				mProxy->m_VarParent->m_Vars[8]->m_Visible = aa;
				mProxy->m_VarParent->m_Vars[9]->m_Visible = aa;
				mProxy->m_VarParent->m_Vars[10]->m_Visible = aa;
				mProxy->m_VarParent->m_Vars[11]->m_Visible = aa;
				mProxy->m_Bar->NotUpToDate();
			}
			if (static_cast<CTwVarAtom *>(mProxy->m_VarParent->m_Vars[12])->m_ReadOnly) {
				static_cast<CTwVarAtom *>(mProxy->m_VarParent->m_Vars[12])->m_ReadOnly = false;
				mProxy->m_Bar->NotUpToDate();
			}
		} else if (mProxy && _ExtMemberIndex == 4 && mProxy->m_VarParent) {
			DEV_ASSERT(mProxy->m_VarParent->m_Vars.size() == 16);
			bool visible = ext->m_ShowVal;
			if (ext->m_IsDir) {
				if (mProxy->m_VarParent->m_Vars[13]->m_Visible != visible || mProxy->m_VarParent->m_Vars[14]->m_Visible != visible || mProxy->m_VarParent->m_Vars[15]->m_Visible != visible) {
					mProxy->m_VarParent->m_Vars[13]->m_Visible = visible;
					mProxy->m_VarParent->m_Vars[14]->m_Visible = visible;
					mProxy->m_VarParent->m_Vars[15]->m_Visible = visible;
					mProxy->m_Bar->NotUpToDate();
				}
			} else {
				if (mProxy->m_VarParent->m_Vars[4]->m_Visible != visible || mProxy->m_VarParent->m_Vars[5]->m_Visible != visible || mProxy->m_VarParent->m_Vars[6]->m_Visible != visible || mProxy->m_VarParent->m_Vars[7]->m_Visible != visible) {
					mProxy->m_VarParent->m_Vars[4]->m_Visible = visible;
					mProxy->m_VarParent->m_Vars[5]->m_Visible = visible;
					mProxy->m_VarParent->m_Vars[6]->m_Visible = visible;
					mProxy->m_VarParent->m_Vars[7]->m_Visible = visible;
					mProxy->m_Bar->NotUpToDate();
				}
			}
		}

		if (ext->m_IsFloat) {
			const float *var = static_cast<const float *>(_VarValue);
			if (ext->m_IsDir) {
				ext->Dx = var[0];
				ext->Dy = var[1];
				ext->Dz = var[2];
				QuatFromDir(&ext->Qx, &ext->Qy, &ext->Qz, &ext->Qs, var[0], var[1], var[2]);
			} else {
				ext->Qx = var[0];
				ext->Qy = var[1];
				ext->Qz = var[2];
				ext->Qs = var[3];
			}
		} else {
			const double *var = static_cast<const double *>(_VarValue);
			if (ext->m_IsDir) {
				ext->Dx = var[0];
				ext->Dy = var[1];
				ext->Dz = var[2];
				QuatFromDir(&ext->Qx, &ext->Qy, &ext->Qz, &ext->Qs, var[0], var[1], var[2]);
			} else {
				ext->Qx = var[0];
				ext->Qy = var[1];
				ext->Qz = var[2];
				ext->Qs = var[3];
			}
		}
		ext->ConvertToAxisAngle();
	}
}

void ANT_CALL CQuaternionExt::SummaryCB(char *_SummaryString, size_t _SummaryMaxLength, const void *_ExtValue, void * /*_ClientData*/) {
	const CQuaternionExt *ext = static_cast<const CQuaternionExt *>(_ExtValue);
	if (ext) {
		if (ext->m_AAMode)
			_snprintf(_SummaryString, _SummaryMaxLength, "V={%.2f,%.2f,%.2f} A=%.0f%c", ext->Vx, ext->Vy, ext->Vz, ext->Angle, 176);
		else if (ext->m_IsDir) {
			_snprintf(_SummaryString, _SummaryMaxLength, "V={%.2f,%.2f,%.2f}", ext->Dx, ext->Dy, ext->Dz);
		} else
			_snprintf(_SummaryString, _SummaryMaxLength, "Q={x:%.2f,y:%.2f,z:%.2f,s:%.2f}", ext->Qx, ext->Qy, ext->Qz, ext->Qs);
	} else {
		_SummaryString[0] = ' '; // required to force background color for this value
		_SummaryString[1] = '\0';
	}
}

TwType CQuaternionExt::s_CustomType = TW_TYPE_UNDEF;
std::vector<float> CQuaternionExt::s_SphTri;
std::vector<color32> CQuaternionExt::s_SphCol;
std::vector<int> CQuaternionExt::s_SphTriProj;
std::vector<color32> CQuaternionExt::s_SphColLight;
std::vector<float> CQuaternionExt::s_ArrowTri[4];
std::vector<float> CQuaternionExt::s_ArrowNorm[4];
std::vector<int> CQuaternionExt::s_ArrowTriProj[4];
std::vector<color32> CQuaternionExt::s_ArrowColLight[4];

void CQuaternionExt::CreateTypes() {
	if (g_TwMgr == nullptr)
		return;
	s_CustomType = (TwType)(TW_TYPE_CUSTOM_BASE + (int)g_TwMgr->m_Customs.size());
	g_TwMgr->m_Customs.push_back(NULL); // increment custom type number

	for (int pass = 0; pass < 2; pass++) { // pass 0: create quat types; pass 1: create dir types
		const char *quatDefPass0 = "step=0.01 hide";
		const char *quatDefPass1 = "step=0.01 hide";
		const char *quatSDefPass0 = "step=0.01 min=-1 max=1 hide";
		const char *quatSDefPass1 = "step=0.01 min=-1 max=1 hide";
		const char *dirDefPass0 = "step=0.01 hide";
		const char *dirDefPass1 = "step=0.01";
		const char *quatDef = (pass == 0) ? quatDefPass0 : quatDefPass1;
		const char *quatSDef = (pass == 0) ? quatSDefPass0 : quatSDefPass1;
		const char *dirDef = (pass == 0) ? dirDefPass0 : dirDefPass1;

		TwStructMember QuatExtMembers[] = { { "0", s_CustomType, 0, "" },
			{ "1", s_CustomType, 0, "" },
			{ "2", s_CustomType, 0, "" },
			{ "3", s_CustomType, 0, "" },
			{ "Quat X", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Qx), quatDef }, // copy of the source quaternion
			{ "Quat Y", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Qy), quatDef },
			{ "Quat Z", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Qz), quatDef },
			{ "Quat S", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Qs), quatSDef },
			{ "Axis X", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Vx), "step=0.01 hide" }, // axis and angle conversion -> Mode hidden because it is not equivalent to a quat (would have required vector renormalization)
			{ "Axis Y", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Vy), "step=0.01 hide" },
			{ "Axis Z", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Vz), "step=0.01 hide" },
			{ "Angle (degree)", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Angle), "step=1 min=-360 max=360 hide" },
			{ "Mode", TW_TYPE_BOOLCPP, offsetof(CQuaternionExt, m_AAMode), "true='Axis Angle' false='Quaternion' readwrite hide" },
			{ "Dir X", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Dx), dirDef }, // copy of the source direction
			{ "Dir Y", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Dy), dirDef },
			{ "Dir Z", TW_TYPE_DOUBLE, offsetof(CQuaternionExt, Dz), dirDef } };
		if (pass == 0) {
			g_TwMgr->m_TypeQuat4F = TwDefineStructExt("QUAT4F", QuatExtMembers, sizeof(QuatExtMembers) / sizeof(QuatExtMembers[0]), 4 * sizeof(float), sizeof(CQuaternionExt), CQuaternionExt::InitQuat4FCB, CQuaternionExt::CopyVarFromExtCB, CQuaternionExt::CopyVarToExtCB, CQuaternionExt::SummaryCB, CTwMgr::CStruct::s_PassProxyAsClientData, "A 4-floats-encoded quaternion");
			g_TwMgr->m_TypeQuat4D = TwDefineStructExt("QUAT4D", QuatExtMembers, sizeof(QuatExtMembers) / sizeof(QuatExtMembers[0]), 4 * sizeof(double), sizeof(CQuaternionExt), CQuaternionExt::InitQuat4DCB, CQuaternionExt::CopyVarFromExtCB, CQuaternionExt::CopyVarToExtCB, CQuaternionExt::SummaryCB, CTwMgr::CStruct::s_PassProxyAsClientData, "A 4-doubles-encoded quaternion");
		} else if (pass == 1) {
			g_TwMgr->m_TypeDir3F = TwDefineStructExt("DIR4F", QuatExtMembers, sizeof(QuatExtMembers) / sizeof(QuatExtMembers[0]), 3 * sizeof(float), sizeof(CQuaternionExt), CQuaternionExt::InitDir3FCB, CQuaternionExt::CopyVarFromExtCB, CQuaternionExt::CopyVarToExtCB, CQuaternionExt::SummaryCB, CTwMgr::CStruct::s_PassProxyAsClientData, "A 3-floats-encoded direction");
			g_TwMgr->m_TypeDir3D = TwDefineStructExt("DIR4D", QuatExtMembers, sizeof(QuatExtMembers) / sizeof(QuatExtMembers[0]), 3 * sizeof(double), sizeof(CQuaternionExt), CQuaternionExt::InitDir3DCB, CQuaternionExt::CopyVarFromExtCB, CQuaternionExt::CopyVarToExtCB, CQuaternionExt::SummaryCB, CTwMgr::CStruct::s_PassProxyAsClientData, "A 3-doubles-encoded direction");
		}
	}

	CreateSphere();
	CreateArrow();
}

void CQuaternionExt::ConvertToAxisAngle() {
	if (fabs(Qs) > (1.0 + FLOAT_EPS)) {
		//Vx = Vy = Vz = 0; // no, keep the previous value
		Angle = 0;
	} else {
		double a;
		if (Qs >= 1)
			a = 0; // and keep V
		else if (Qs <= -1)
			a = DOUBLE_PI; // and keep V
		else if (fabs(Qx * Qx + Qy * Qy + Qz * Qz + Qs * Qs) < FLOAT_EPS_SQ)
			a = 0;
		else {
			a = Math::acos(Qs);
			if (a * Angle < 0) // Preserve the sign of Angle
				a = -a;
			double f = 1 / Math::sin(a);
			Vx = Qx * f;
			Vy = Qy * f;
			Vz = Qz * f;
		}
		Angle = 2.0 * a;
	}

	//  if (Angle>FLOAT_PI)
	//      Angle -= 2.0f*FLOAT_PI;
	//  else if (Angle<-FLOAT_PI)
	//      Angle += 2.0f*FLOAT_PI;
	Angle = RadToDeg(Angle);

	if (Math::abs(Angle) < FLOAT_EPS && Math::abs(Vx * Vx + Vy * Vy + Vz * Vz) < FLOAT_EPS_SQ)
		Vx = 1.0e-7; // all components cannot be null
}

void CQuaternionExt::ConvertFromAxisAngle() {
	double n = Vx * Vx + Vy * Vy + Vz * Vz;
	if (Math::abs(n) > FLOAT_EPS_SQ) {
		double f = 0.5 * DegToRad(Angle);
		Qs = Math::cos(f);
		// do not normalize
		// if (Math::abs(n - 1.0)>FLOAT_EPS_SQ )
		//   f = Math::sin(f) * (1.0/Math::sqrt(n));
		// else
		//   f = Math::sin(f);
		f = Math::sin(f);
		Qx = Vx * f;
		Qy = Vy * f;
		Qz = Vz * f;
	} else {
		Qs = 1;
		Qx = Qy = Qz = 0;
	}
}

void CQuaternionExt::CopyToVar() {
	if (m_StructProxy != NULL) {
		if (m_StructProxy->m_StructSetCallback != NULL) {
			if (m_IsFloat) {
				if (m_IsDir) {
					float d[] = { 1, 0, 0 };
					ApplyQuat(d + 0, d + 1, d + 2, 1, 0, 0, (float)Qx, (float)Qy, (float)Qz, (float)Qs);
					float l = (float)Math::sqrt(Dx * Dx + Dy * Dy + Dz * Dz);
					d[0] *= l;
					d[1] *= l;
					d[2] *= l;
					Dx = d[0];
					Dy = d[1];
					Dz = d[2]; // update also Dx,Dy,Dz
					m_StructProxy->m_StructSetCallback(d, m_StructProxy->m_StructClientData);
				} else {
					float q[] = { (float)Qx, (float)Qy, (float)Qz, (float)Qs };
					m_StructProxy->m_StructSetCallback(q, m_StructProxy->m_StructClientData);
				}
			} else {
				if (m_IsDir) {
					float d[] = { 1, 0, 0 };
					ApplyQuat(d + 0, d + 1, d + 2, 1, 0, 0, (float)Qx, (float)Qy, (float)Qz, (float)Qs);
					double l = Math::sqrt(Dx * Dx + Dy * Dy + Dz * Dz);
					double dd[] = { l * d[0], l * d[1], l * d[2] };
					Dx = dd[0];
					Dy = dd[1];
					Dz = dd[2]; // update also Dx,Dy,Dz
					m_StructProxy->m_StructSetCallback(dd, m_StructProxy->m_StructClientData);
				} else {
					double q[] = { Qx, Qy, Qz, Qs };
					m_StructProxy->m_StructSetCallback(q, m_StructProxy->m_StructClientData);
				}
			}
		} else if (m_StructProxy->m_StructData != NULL) {
			if (m_IsFloat) {
				if (m_IsDir) {
					float *d = static_cast<float *>(m_StructProxy->m_StructData);
					ApplyQuat(d + 0, d + 1, d + 2, 1, 0, 0, (float)Qx, (float)Qy, (float)Qz, (float)Qs);
					float l = (float)Math::sqrt(Dx * Dx + Dy * Dy + Dz * Dz);
					d[0] *= l;
					d[1] *= l;
					d[2] *= l;
					Dx = d[0];
					Dy = d[1];
					Dz = d[2]; // update also Dx,Dy,Dz
				} else {
					float *q = static_cast<float *>(m_StructProxy->m_StructData);
					q[0] = (float)Qx;
					q[1] = (float)Qy;
					q[2] = (float)Qz;
					q[3] = (float)Qs;
				}
			} else {
				if (m_IsDir) {
					double *dd = static_cast<double *>(m_StructProxy->m_StructData);
					float d[] = { 1, 0, 0 };
					ApplyQuat(d + 0, d + 1, d + 2, 1, 0, 0, (float)Qx, (float)Qy, (float)Qz, (float)Qs);
					double l = Math::sqrt(Dx * Dx + Dy * Dy + Dz * Dz);
					dd[0] = l * d[0];
					dd[1] = l * d[1];
					dd[2] = l * d[2];
					Dx = dd[0];
					Dy = dd[1];
					Dz = dd[2]; // update also Dx,Dy,Dz
				} else {
					double *q = static_cast<double *>(m_StructProxy->m_StructData);
					q[0] = Qx;
					q[1] = Qy;
					q[2] = Qz;
					q[3] = Qs;
				}
			}
		}
	}
}

void CQuaternionExt::CreateSphere() {
	const int SUBDIV = 7;
	s_SphTri.clear();
	s_SphCol.clear();

	const float A[8 * 3] = { 1, 0, 0, 0, 0, -1, -1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, -1, -1, 0, 0 };
	const float B[8 * 3] = { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0 };
	const float C[8 * 3] = { 0, 0, 1, 1, 0, 0, 0, 0, -1, -1, 0, 0, 1, 0, 0, 0, 0, -1, -1, 0, 0, 0, 0, 1 };
	// const color32 COL_A[8] = { 0xffff8080, 0xff000080, 0xff800000, 0xff8080ff,  0xff8080ff, 0xffff8080, 0xff000080, 0xff800000 };
	// const color32 COL_B[8] = { 0xff80ff80, 0xff80ff80, 0xff80ff80, 0xff80ff80,  0xff008000, 0xff008000, 0xff008000, 0xff008000 };
	// const color32 COL_C[8] = { 0xff8080ff, 0xffff8080, 0xff000080, 0xff800000,  0xffff8080, 0xff000080, 0xff800000, 0xff8080ff };
	const color32 COL_A[8] = { 0xffffffff, 0xffffff40, 0xff40ff40, 0xff40ffff, 0xffff40ff, 0xffff4040, 0xff404040, 0xff4040ff };
	const color32 COL_B[8] = { 0xffffffff, 0xffffff40, 0xff40ff40, 0xff40ffff, 0xffff40ff, 0xffff4040, 0xff404040, 0xff4040ff };
	const color32 COL_C[8] = { 0xffffffff, 0xffffff40, 0xff40ff40, 0xff40ffff, 0xffff40ff, 0xffff4040, 0xff404040, 0xff4040ff };

	float xa, ya, za, xb, yb, zb, xc, yc, zc, x, y, z, norm, u[3], v[3];
	color32 col;
	for (int i = 0; i < 8; ++i) {
		xa = A[3 * i + 0];
		ya = A[3 * i + 1];
		za = A[3 * i + 2];
		xb = B[3 * i + 0];
		yb = B[3 * i + 1];
		zb = B[3 * i + 2];
		xc = C[3 * i + 0];
		yc = C[3 * i + 1];
		zc = C[3 * i + 2];
		for (int j = 0; j <= SUBDIV; ++j)
			for (int k = 0; k <= 2 * (SUBDIV - j); ++k) {
				if (k % 2 == 0) {
					u[0] = ((float)j) / (SUBDIV + 1);
					v[0] = ((float)(k / 2)) / (SUBDIV + 1);
					u[1] = ((float)(j + 1)) / (SUBDIV + 1);
					v[1] = ((float)(k / 2)) / (SUBDIV + 1);
					u[2] = ((float)j) / (SUBDIV + 1);
					v[2] = ((float)(k / 2 + 1)) / (SUBDIV + 1);
				} else {
					u[0] = ((float)j) / (SUBDIV + 1);
					v[0] = ((float)(k / 2 + 1)) / (SUBDIV + 1);
					u[1] = ((float)(j + 1)) / (SUBDIV + 1);
					v[1] = ((float)(k / 2)) / (SUBDIV + 1);
					u[2] = ((float)(j + 1)) / (SUBDIV + 1);
					v[2] = ((float)(k / 2 + 1)) / (SUBDIV + 1);
				}

				for (int l = 0; l < 3; ++l) {
					x = (1.0f - u[l] - v[l]) * xa + u[l] * xb + v[l] * xc;
					y = (1.0f - u[l] - v[l]) * ya + u[l] * yb + v[l] * yc;
					z = (1.0f - u[l] - v[l]) * za + u[l] * zb + v[l] * zc;
					norm = sqrtf(x * x + y * y + z * z);
					x /= norm;
					y /= norm;
					z /= norm;
					s_SphTri.push_back(x);
					s_SphTri.push_back(y);
					s_SphTri.push_back(z);
					if (u[l] + v[l] > FLOAT_EPS)
						col = ColorBlend(COL_A[i], ColorBlend(COL_B[i], COL_C[i], v[l] / (u[l] + v[l])), u[l] + v[l]);
					else
						col = COL_A[i];
					// if( (j==0 && k==0) || (j==0 && k==2*SUBDIV) || (j==SUBDIV && k==0) )
					//   col = 0xffff0000;
					s_SphCol.push_back(col);
				}
			}
	}
	s_SphTriProj.clear();
	s_SphTriProj.resize(2 * s_SphCol.size(), 0);
	s_SphColLight.clear();
	s_SphColLight.resize(s_SphCol.size(), 0);
}

void CQuaternionExt::CreateArrow() {
	const int SUBDIV = 15;
	const float CYL_RADIUS = 0.08;
	const float CONE_RADIUS = 0.16;
	const float CONE_LENGTH = 0.25;
	const float ARROW_BGN = -1.1;
	const float ARROW_END = 1.15;
	int i;
	for (i = 0; i < 4; ++i) {
		s_ArrowTri[i].clear();
		s_ArrowNorm[i].clear();
	}

	float x0, x1, y0, y1, z0, z1, a0, a1, nx, nn;
	for (i = 0; i < SUBDIV; ++i) {
		a0 = 2.0f * FLOAT_PI * (float(i)) / SUBDIV;
		a1 = 2.0f * FLOAT_PI * (float(i + 1)) / SUBDIV;
		x0 = ARROW_BGN;
		x1 = ARROW_END - CONE_LENGTH;
		y0 = cosf(a0);
		z0 = sinf(a0);
		y1 = cosf(a1);
		z1 = sinf(a1);
		s_ArrowTri[ARROW_CYL].push_back(x1);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * y0);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * z0);
		s_ArrowTri[ARROW_CYL].push_back(x0);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * y0);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * z0);
		s_ArrowTri[ARROW_CYL].push_back(x0);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * y1);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * z1);
		s_ArrowTri[ARROW_CYL].push_back(x1);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * y0);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * z0);
		s_ArrowTri[ARROW_CYL].push_back(x0);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * y1);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * z1);
		s_ArrowTri[ARROW_CYL].push_back(x1);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * y1);
		s_ArrowTri[ARROW_CYL].push_back(CYL_RADIUS * z1);
		s_ArrowNorm[ARROW_CYL].push_back(0);
		s_ArrowNorm[ARROW_CYL].push_back(y0);
		s_ArrowNorm[ARROW_CYL].push_back(z0);
		s_ArrowNorm[ARROW_CYL].push_back(0);
		s_ArrowNorm[ARROW_CYL].push_back(y0);
		s_ArrowNorm[ARROW_CYL].push_back(z0);
		s_ArrowNorm[ARROW_CYL].push_back(0);
		s_ArrowNorm[ARROW_CYL].push_back(y1);
		s_ArrowNorm[ARROW_CYL].push_back(z1);
		s_ArrowNorm[ARROW_CYL].push_back(0);
		s_ArrowNorm[ARROW_CYL].push_back(y0);
		s_ArrowNorm[ARROW_CYL].push_back(z0);
		s_ArrowNorm[ARROW_CYL].push_back(0);
		s_ArrowNorm[ARROW_CYL].push_back(y1);
		s_ArrowNorm[ARROW_CYL].push_back(z1);
		s_ArrowNorm[ARROW_CYL].push_back(0);
		s_ArrowNorm[ARROW_CYL].push_back(y1);
		s_ArrowNorm[ARROW_CYL].push_back(z1);
		s_ArrowTri[ARROW_CYL_CAP].push_back(x0);
		s_ArrowTri[ARROW_CYL_CAP].push_back(0);
		s_ArrowTri[ARROW_CYL_CAP].push_back(0);
		s_ArrowTri[ARROW_CYL_CAP].push_back(x0);
		s_ArrowTri[ARROW_CYL_CAP].push_back(CYL_RADIUS * y1);
		s_ArrowTri[ARROW_CYL_CAP].push_back(CYL_RADIUS * z1);
		s_ArrowTri[ARROW_CYL_CAP].push_back(x0);
		s_ArrowTri[ARROW_CYL_CAP].push_back(CYL_RADIUS * y0);
		s_ArrowTri[ARROW_CYL_CAP].push_back(CYL_RADIUS * z0);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(-1);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(0);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(0);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(-1);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(0);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(0);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(-1);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(0);
		s_ArrowNorm[ARROW_CYL_CAP].push_back(0);
		x0 = ARROW_END - CONE_LENGTH;
		x1 = ARROW_END;
		nx = CONE_RADIUS / (x1 - x0);
		nn = 1 / Math::sqrt(nx * nx + 1);
		s_ArrowTri[ARROW_CONE].push_back(x1);
		s_ArrowTri[ARROW_CONE].push_back(0);
		s_ArrowTri[ARROW_CONE].push_back(0);
		s_ArrowTri[ARROW_CONE].push_back(x0);
		s_ArrowTri[ARROW_CONE].push_back(CONE_RADIUS * y0);
		s_ArrowTri[ARROW_CONE].push_back(CONE_RADIUS * z0);
		s_ArrowTri[ARROW_CONE].push_back(x0);
		s_ArrowTri[ARROW_CONE].push_back(CONE_RADIUS * y1);
		s_ArrowTri[ARROW_CONE].push_back(CONE_RADIUS * z1);
		s_ArrowTri[ARROW_CONE].push_back(x1);
		s_ArrowTri[ARROW_CONE].push_back(0);
		s_ArrowTri[ARROW_CONE].push_back(0);
		s_ArrowTri[ARROW_CONE].push_back(x0);
		s_ArrowTri[ARROW_CONE].push_back(CONE_RADIUS * y1);
		s_ArrowTri[ARROW_CONE].push_back(CONE_RADIUS * z1);
		s_ArrowTri[ARROW_CONE].push_back(x1);
		s_ArrowTri[ARROW_CONE].push_back(0);
		s_ArrowTri[ARROW_CONE].push_back(0);
		s_ArrowNorm[ARROW_CONE].push_back(nn * nx);
		s_ArrowNorm[ARROW_CONE].push_back(nn * y0);
		s_ArrowNorm[ARROW_CONE].push_back(nn * z0);
		s_ArrowNorm[ARROW_CONE].push_back(nn * nx);
		s_ArrowNorm[ARROW_CONE].push_back(nn * y0);
		s_ArrowNorm[ARROW_CONE].push_back(nn * z0);
		s_ArrowNorm[ARROW_CONE].push_back(nn * nx);
		s_ArrowNorm[ARROW_CONE].push_back(nn * y1);
		s_ArrowNorm[ARROW_CONE].push_back(nn * z1);
		s_ArrowNorm[ARROW_CONE].push_back(nn * nx);
		s_ArrowNorm[ARROW_CONE].push_back(nn * y0);
		s_ArrowNorm[ARROW_CONE].push_back(nn * z0);
		s_ArrowNorm[ARROW_CONE].push_back(nn * nx);
		s_ArrowNorm[ARROW_CONE].push_back(nn * y1);
		s_ArrowNorm[ARROW_CONE].push_back(nn * z1);
		s_ArrowNorm[ARROW_CONE].push_back(nn * nx);
		s_ArrowNorm[ARROW_CONE].push_back(nn * y1);
		s_ArrowNorm[ARROW_CONE].push_back(nn * z1);
		s_ArrowTri[ARROW_CONE_CAP].push_back(x0);
		s_ArrowTri[ARROW_CONE_CAP].push_back(0);
		s_ArrowTri[ARROW_CONE_CAP].push_back(0);
		s_ArrowTri[ARROW_CONE_CAP].push_back(x0);
		s_ArrowTri[ARROW_CONE_CAP].push_back(CONE_RADIUS * y1);
		s_ArrowTri[ARROW_CONE_CAP].push_back(CONE_RADIUS * z1);
		s_ArrowTri[ARROW_CONE_CAP].push_back(x0);
		s_ArrowTri[ARROW_CONE_CAP].push_back(CONE_RADIUS * y0);
		s_ArrowTri[ARROW_CONE_CAP].push_back(CONE_RADIUS * z0);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(-1);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(0);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(0);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(-1);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(0);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(0);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(-1);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(0);
		s_ArrowNorm[ARROW_CONE_CAP].push_back(0);
	}

	for (i = 0; i < 4; ++i) {
		s_ArrowTriProj[i].clear();
		s_ArrowTriProj[i].resize(2 * (s_ArrowTri[i].size() / 3), 0);
		s_ArrowColLight[i].clear();
		s_ArrowColLight[i].resize(s_ArrowTri[i].size() / 3, 0);
	}
}

static _FORCE_INLINE_ void QuatMult(double *out, const double *q1, const double *q2) {
	out[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
	out[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
	out[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
	out[3] = q1[3] * q2[3] - (q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2]);
}

static _FORCE_INLINE_ void QuatFromAxisAngle(double *out, const double *axis, double angle) {
	double n = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2];
	if (Math::abs(n) > DOUBLE_EPS) {
		double f = 0.5 * angle;
		out[3] = Math::cos(f);
		f = Math::sin(f) / Math::sqrt(n);
		out[0] = axis[0] * f;
		out[1] = axis[1] * f;
		out[2] = axis[2] * f;
	} else {
		out[3] = 1;
		out[0] = out[1] = out[2] = 0;
	}
}

static _FORCE_INLINE_ void Vec3Cross(double *out, const double *a, const double *b) {
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

static _FORCE_INLINE_ double Vec3Dot(const double *a, const double *b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static _FORCE_INLINE_ void Vec3RotY(float *x, float *y, float *z) {
	(void)y;
	float tmp = *x;
	*x = -*z;
	*z = tmp;
}

static _FORCE_INLINE_ void Vec3RotZ(float *x, float *y, float *z) {
	(void)z;
	float tmp = *x;
	*x = -*y;
	*y = tmp;
}

void CQuaternionExt::ApplyQuat(float *outX, float *outY, float *outZ, float x, float y, float z, float qx, float qy, float qz, float qs) {
	float ps = -qx * x - qy * y - qz * z;
	float px = qs * x + qy * z - qz * y;
	float py = qs * y + qz * x - qx * z;
	float pz = qs * z + qx * y - qy * x;
	*outX = -ps * qx + px * qs - py * qz + pz * qy;
	*outY = -ps * qy + py * qs - pz * qx + px * qz;
	*outZ = -ps * qz + pz * qs - px * qy + py * qx;
}

void CQuaternionExt::QuatFromDir(double *outQx, double *outQy, double *outQz, double *outQs, double dx, double dy, double dz) {
	// compute a quaternion that rotates (1,0,0) to (dx,dy,dz)
	double dn = sqrt(dx * dx + dy * dy + dz * dz);
	if (dn < DOUBLE_EPS_SQ) {
		*outQx = *outQy = *outQz = 0;
		*outQs = 1;
	} else {
		double rotAxis[3] = { 0, -dz, dy };
		if (rotAxis[0] * rotAxis[0] + rotAxis[1] * rotAxis[1] + rotAxis[2] * rotAxis[2] < DOUBLE_EPS_SQ) {
			rotAxis[0] = rotAxis[1] = 0;
			rotAxis[2] = 1;
		}
		double rotAngle = Math::acos(dx / dn);
		double rotQuat[4];
		QuatFromAxisAngle(rotQuat, rotAxis, rotAngle);
		*outQx = rotQuat[0];
		*outQy = rotQuat[1];
		*outQz = rotQuat[2];
		*outQs = rotQuat[3];
	}
}

void CQuaternionExt::Permute(float *outX, float *outY, float *outZ, float x, float y, float z) {
	float px = x, py = y, pz = z;
	*outX = m_Permute[0][0] * px + m_Permute[1][0] * py + m_Permute[2][0] * pz;
	*outY = m_Permute[0][1] * px + m_Permute[1][1] * py + m_Permute[2][1] * pz;
	*outZ = m_Permute[0][2] * px + m_Permute[1][2] * py + m_Permute[2][2] * pz;
}

void CQuaternionExt::PermuteInv(float *outX, float *outY, float *outZ, float x, float y, float z) {
	float px = x, py = y, pz = z;
	*outX = m_Permute[0][0] * px + m_Permute[0][1] * py + m_Permute[0][2] * pz;
	*outY = m_Permute[1][0] * px + m_Permute[1][1] * py + m_Permute[1][2] * pz;
	*outZ = m_Permute[2][0] * px + m_Permute[2][1] * py + m_Permute[2][2] * pz;
}

void CQuaternionExt::Permute(double *outX, double *outY, double *outZ, double x, double y, double z) {
	double px = x, py = y, pz = z;
	*outX = m_Permute[0][0] * px + m_Permute[1][0] * py + m_Permute[2][0] * pz;
	*outY = m_Permute[0][1] * px + m_Permute[1][1] * py + m_Permute[2][1] * pz;
	*outZ = m_Permute[0][2] * px + m_Permute[1][2] * py + m_Permute[2][2] * pz;
}

void CQuaternionExt::PermuteInv(double *outX, double *outY, double *outZ, double x, double y, double z) {
	double px = x, py = y, pz = z;
	*outX = m_Permute[0][0] * px + m_Permute[0][1] * py + m_Permute[0][2] * pz;
	*outY = m_Permute[1][0] * px + m_Permute[1][1] * py + m_Permute[1][2] * pz;
	*outZ = m_Permute[2][0] * px + m_Permute[2][1] * py + m_Permute[2][2] * pz;
}

static _FORCE_INLINE_ float QuatD(int w, int h) {
	return (float)MIN(Math::abs(w), Math::abs(h)) - 4;
}

static _FORCE_INLINE_ int QuatPX(float x, int w, int h) {
	return (int)(x * 0.5 * QuatD(w, h) + (float)w * 0.5 + 0.5);
}

static _FORCE_INLINE_ int QuatPY(float y, int w, int h) {
	return (int)(-y * 0.5 * QuatD(w, h) + (float)h * 0.5 - 0.5);
}

static _FORCE_INLINE_ float QuatIX(int x, int w, int h) {
	return (2 * (float)x - (float)w - 1) / QuatD(w, h);
}

static _FORCE_INLINE_ float QuatIY(int y, int w, int h) {
	return (-2 * (float)y + (float)h - 1) / QuatD(w, h);
}

void CQuaternionExt::DrawCB(int w, int h, void *_ExtValue, void *_ClientData, TwBar *_Bar, CTwVarGroup *varGrp) {
	if (g_TwMgr == nullptr || g_TwMgr->m_Graph == nullptr)
		return;
	DEV_ASSERT(g_TwMgr->m_Graph->IsDrawing());
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(_ExtValue);
	DEV_ASSERT(ext != nullptr);
	(void)_ClientData;
	(void)_Bar;

	// show/hide quat values
	DEV_ASSERT(varGrp->m_Vars.size() == 16);
	bool visible = ext->m_ShowVal;
	if (ext->m_IsDir) {
		if (varGrp->m_Vars[13]->m_Visible != visible || varGrp->m_Vars[14]->m_Visible != visible || varGrp->m_Vars[15]->m_Visible != visible) {
			varGrp->m_Vars[13]->m_Visible = visible;
			varGrp->m_Vars[14]->m_Visible = visible;
			varGrp->m_Vars[15]->m_Visible = visible;
			_Bar->NotUpToDate();
		}
	} else {
		if (varGrp->m_Vars[4]->m_Visible != visible || varGrp->m_Vars[5]->m_Visible != visible || varGrp->m_Vars[6]->m_Visible != visible || varGrp->m_Vars[7]->m_Visible != visible) {
			varGrp->m_Vars[4]->m_Visible = visible;
			varGrp->m_Vars[5]->m_Visible = visible;
			varGrp->m_Vars[6]->m_Visible = visible;
			varGrp->m_Vars[7]->m_Visible = visible;
			_Bar->NotUpToDate();
		}
	}

	// force ext update
	static_cast<CTwVarAtom *>(varGrp->m_Vars[4])->ValueToDouble();

	DEV_ASSERT(s_SphTri.size() > 0);
	DEV_ASSERT(s_SphTri.size() == 3 * s_SphCol.size());
	DEV_ASSERT(s_SphTriProj.size() == 2 * s_SphCol.size());
	DEV_ASSERT(s_SphColLight.size() == s_SphCol.size());

	if (QuatD(w, h) <= 2)
		return;
	float x, y, z, nx, ny, nz, kx, ky, kz, qx, qy, qz, qs;
	// normalize quaternion
	float qn = (float)sqrt(ext->Qs * ext->Qs + ext->Qx * ext->Qx + ext->Qy * ext->Qy + ext->Qz * ext->Qz);
	if (qn > FLOAT_EPS) {
		qx = (float)ext->Qx / qn;
		qy = (float)ext->Qy / qn;
		qz = (float)ext->Qz / qn;
		qs = (float)ext->Qs / qn;
	} else {
		qx = qy = qz = 0;
		qs = 1;
	}

	double normDir = sqrt(ext->m_Dir[0] * ext->m_Dir[0] + ext->m_Dir[1] * ext->m_Dir[1] + ext->m_Dir[2] * ext->m_Dir[2]);
	bool drawDir = ext->m_IsDir || (normDir > DOUBLE_EPS);
	color32 alpha = ext->m_Highlighted ? 0xffffffff : 0xb0ffffff;

	// check if frame is right-handed
	ext->Permute(&kx, &ky, &kz, 1, 0, 0);
	double px[3] = { (double)kx, (double)ky, (double)kz };
	ext->Permute(&kx, &ky, &kz, 0, 1, 0);
	double py[3] = { (double)kx, (double)ky, (double)kz };
	ext->Permute(&kx, &ky, &kz, 0, 0, 1);
	double pz[3] = { (double)kx, (double)ky, (double)kz };
	double ez[3];
	Vec3Cross(ez, px, py);
	bool frameRightHanded = (ez[0] * pz[0] + ez[1] * pz[1] + ez[2] * pz[2] >= 0);
	ITwGraph::Cull cull = frameRightHanded ? ITwGraph::CULL_CW : ITwGraph::CULL_CCW;

	if (drawDir) {
		float dir[] = { (float)ext->m_Dir[0], (float)ext->m_Dir[1], (float)ext->m_Dir[2] };
		if (normDir < DOUBLE_EPS) {
			normDir = 1;
			dir[0] = 1;
		}
		kx = dir[0];
		ky = dir[1];
		kz = dir[2];
		double rotDirAxis[3] = { 0, -kz, ky };
		if (rotDirAxis[0] * rotDirAxis[0] + rotDirAxis[1] * rotDirAxis[1] + rotDirAxis[2] * rotDirAxis[2] < DOUBLE_EPS_SQ) {
			rotDirAxis[0] = rotDirAxis[1] = 0;
			rotDirAxis[2] = 1;
		}
		double rotDirAngle = acos(kx / normDir);
		double rotDirQuat[4];
		QuatFromAxisAngle(rotDirQuat, rotDirAxis, rotDirAngle);

		kx = 1;
		ky = 0;
		kz = 0;
		ApplyQuat(&kx, &ky, &kz, kx, ky, kz, (float)rotDirQuat[0], (float)rotDirQuat[1], (float)rotDirQuat[2], (float)rotDirQuat[3]);
		ApplyQuat(&kx, &ky, &kz, kx, ky, kz, qx, qy, qz, qs);
		for (int k = 0; k < 4; ++k) { // 4 parts of the arrow
			ext->Permute(&x, &y, &z, kx, ky, kz); // draw order
			int j = (z > 0) ? 3 - k : k;

			DEV_ASSERT(s_ArrowTriProj[j].size() == 2 * (s_ArrowTri[j].size() / 3) && s_ArrowColLight[j].size() == s_ArrowTri[j].size() / 3 && s_ArrowNorm[j].size() == s_ArrowTri[j].size());
			const int ntri = (int)s_ArrowTri[j].size() / 3;
			const float *tri = &(s_ArrowTri[j][0]);
			const float *norm = &(s_ArrowNorm[j][0]);
			int *triProj = &(s_ArrowTriProj[j][0]);
			color32 *colLight = &(s_ArrowColLight[j][0]);
			for (int i = 0; i < ntri; ++i) {
				x = tri[3 * i + 0];
				y = tri[3 * i + 1];
				z = tri[3 * i + 2];
				nx = norm[3 * i + 0];
				ny = norm[3 * i + 1];
				nz = norm[3 * i + 2];
				if (x > 0)
					x = 2.5 * x - 2.0;
				else
					x += 0.2;
				y *= 1.5;
				z *= 1.5;
				ApplyQuat(&x, &y, &z, x, y, z, (float)rotDirQuat[0], (float)rotDirQuat[1], (float)rotDirQuat[2], (float)rotDirQuat[3]);
				ApplyQuat(&x, &y, &z, x, y, z, qx, qy, qz, qs);
				ext->Permute(&x, &y, &z, x, y, z);
				ApplyQuat(&nx, &ny, &nz, nx, ny, nz, (float)rotDirQuat[0], (float)rotDirQuat[1], (float)rotDirQuat[2], (float)rotDirQuat[3]);
				ApplyQuat(&nx, &ny, &nz, nx, ny, nz, qx, qy, qz, qs);
				ext->Permute(&nx, &ny, &nz, nx, ny, nz);
				triProj[2 * i + 0] = QuatPX(x, w, h);
				triProj[2 * i + 1] = QuatPY(y, w, h);
				color32 col = (ext->m_DirColor | 0xff000000) & alpha;
				colLight[i] = ColorBlend(0xff000000, col, fabsf(TClamp(nz, -1.0f, 1.0f)));
			}
			if (s_ArrowTri[j].size() >= 9) // 1 tri = 9 floats
				g_TwMgr->m_Graph->DrawTriangles((int)s_ArrowTri[j].size() / 9, triProj, colLight, cull);
		}
	} else {
		// int px0 = QuatPX(0, w, h)-1, py0 = QuatPY(0, w, h), r0 = (int)(0.5*QuatD(w, h)-0.5);
		// color32 col0 = 0x80000000;
		// DrawArc(px0-1, py0, r0, 0, 360, col0);
		// DrawArc(px0+1, py0, r0, 0, 360, col0);
		// DrawArc(px0, py0-1, r0, 0, 360, col0);
		// DrawArc(px0, py0+1, r0, 0, 360, col0);

		// draw arrows & sphere
		const float SPH_RADIUS = 0.75;
		for (int m = 0; m < 2; ++m) // m=0: back, m=1: front
		{
			for (int l = 0; l < 3; ++l) // draw 3 arrows
			{
				kx = 1;
				ky = 0;
				kz = 0;
				if (l == 1)
					Vec3RotZ(&kx, &ky, &kz);
				else if (l == 2)
					Vec3RotY(&kx, &ky, &kz);
				ApplyQuat(&kx, &ky, &kz, kx, ky, kz, qx, qy, qz, qs);
				for (int k = 0; k < 4; ++k) // 4 parts of the arrow
				{
					// draw order
					ext->Permute(&x, &y, &z, kx, ky, kz);
					int j = (z > 0) ? 3 - k : k;

					bool cone = true;
					if ((m == 0 && z > 0) || (m == 1 && z <= 0)) {
						if (j == ARROW_CONE || j == ARROW_CONE_CAP) // do not draw cone
							continue;
						else
							cone = false;
					}
					DEV_ASSERT(s_ArrowTriProj[j].size() == 2 * (s_ArrowTri[j].size() / 3) && s_ArrowColLight[j].size() == s_ArrowTri[j].size() / 3 && s_ArrowNorm[j].size() == s_ArrowTri[j].size());
					const int ntri = (int)s_ArrowTri[j].size() / 3;
					const float *tri = &(s_ArrowTri[j][0]);
					const float *norm = &(s_ArrowNorm[j][0]);
					int *triProj = &(s_ArrowTriProj[j][0]);
					color32 *colLight = &(s_ArrowColLight[j][0]);
					for (int i = 0; i < ntri; ++i) {
						x = tri[3 * i + 0];
						y = tri[3 * i + 1];
						z = tri[3 * i + 2];
						if (cone && x <= 0)
							x = SPH_RADIUS;
						else if (!cone && x > 0)
							x = -SPH_RADIUS;
						nx = norm[3 * i + 0];
						ny = norm[3 * i + 1];
						nz = norm[3 * i + 2];
						if (l == 1) {
							Vec3RotZ(&x, &y, &z);
							Vec3RotZ(&nx, &ny, &nz);
						} else if (l == 2) {
							Vec3RotY(&x, &y, &z);
							Vec3RotY(&nx, &ny, &nz);
						}
						ApplyQuat(&x, &y, &z, x, y, z, qx, qy, qz, qs);
						ext->Permute(&x, &y, &z, x, y, z);
						ApplyQuat(&nx, &ny, &nz, nx, ny, nz, qx, qy, qz, qs);
						ext->Permute(&nx, &ny, &nz, nx, ny, nz);
						triProj[2 * i + 0] = QuatPX(x, w, h);
						triProj[2 * i + 1] = QuatPY(y, w, h);
						float fade = (m == 0 && z < 0) ? TClamp(2.0f * z * z, 0.0f, 1.0f) : 0;
						float alphaFade = 1.0f;
						Color32ToARGBf(alpha, &alphaFade, nullptr, nullptr, nullptr);
						alphaFade *= (1.0f - fade);
						color32 alphaFadeCol = Color32FromARGBf(alphaFade, 1, 1, 1);
						color32 col = (l == 0) ? 0xffff0000 : ((l == 1) ? 0xff00ff00 : 0xff0000ff);
						colLight[i] = ColorBlend(0xff000000, col, fabsf(TClamp(nz, -1.0f, 1.0f))) & alphaFadeCol;
					}
					if (s_ArrowTri[j].size() >= 9) // 1 tri = 9 floats
						g_TwMgr->m_Graph->DrawTriangles((int)s_ArrowTri[j].size() / 9, triProj, colLight, cull);
				}
			}

			if (m == 0) {
				const float *tri = &(s_SphTri[0]);
				int *triProj = &(s_SphTriProj[0]);
				const color32 *col = &(s_SphCol[0]);
				color32 *colLight = &(s_SphColLight[0]);
				const int ntri = (int)s_SphTri.size() / 3;
				for (int i = 0; i < ntri; ++i) // draw sphere
				{
					x = SPH_RADIUS * tri[3 * i + 0];
					y = SPH_RADIUS * tri[3 * i + 1];
					z = SPH_RADIUS * tri[3 * i + 2];
					ApplyQuat(&x, &y, &z, x, y, z, qx, qy, qz, qs);
					ext->Permute(&x, &y, &z, x, y, z);
					triProj[2 * i + 0] = QuatPX(x, w, h);
					triProj[2 * i + 1] = QuatPY(y, w, h);
					colLight[i] = ColorBlend(0xff000000, col[i], Math::abs(TClamp(z / SPH_RADIUS, -1.0f, 1.0f))) & alpha;
				}
				g_TwMgr->m_Graph->DrawTriangles((int)s_SphTri.size() / 9, triProj, colLight, cull);
			}
		}

		// draw x
		g_TwMgr->m_Graph->DrawLine(w - 12, h - 36, w - 12 + 5, h - 36 + 5, 0xffc00000, true);
		g_TwMgr->m_Graph->DrawLine(w - 12 + 5, h - 36, w - 12, h - 36 + 5, 0xffc00000, true);
		// draw y
		g_TwMgr->m_Graph->DrawLine(w - 12, h - 25, w - 12 + 3, h - 25 + 4, 0xff00c000, true);
		g_TwMgr->m_Graph->DrawLine(w - 12 + 5, h - 25, w - 12, h - 25 + 7, 0xff00c000, true);
		// draw z
		g_TwMgr->m_Graph->DrawLine(w - 12, h - 12, w - 12 + 5, h - 12, 0xff0000c0, true);
		g_TwMgr->m_Graph->DrawLine(w - 12, h - 12 + 5, w - 12 + 5, h - 12 + 5, 0xff0000c0, true);
		g_TwMgr->m_Graph->DrawLine(w - 12, h - 12 + 5, w - 12 + 5, h - 12, 0xff0000c0, true);
	}

	// draw borders
	g_TwMgr->m_Graph->DrawLine(1, 0, w - 1, 0, 0x40000000);
	g_TwMgr->m_Graph->DrawLine(w - 1, 0, w - 1, h - 1, 0x40000000);
	g_TwMgr->m_Graph->DrawLine(w - 1, h - 1, 1, h - 1, 0x40000000);
	g_TwMgr->m_Graph->DrawLine(1, h - 1, 1, 0, 0x40000000);
}

bool CQuaternionExt::MouseMotionCB(int mouseX, int mouseY, int w, int h, void *structExtValue, void *clientData, TwBar *bar, CTwVarGroup *varGrp) {
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(structExtValue);
	if (ext == NULL)
		return false;
	(void)clientData, (void)varGrp;

	if (mouseX > 0 && mouseX < w && mouseY > 0 && mouseY < h)
		ext->m_Highlighted = true;

	if (ext->m_Rotating) {
		double x = QuatIX(mouseX, w, h);
		double y = QuatIY(mouseY, w, h);
		double z = 1;
		double px, py, pz, ox, oy, oz;
		ext->PermuteInv(&px, &py, &pz, x, y, z);
		ext->PermuteInv(&ox, &oy, &oz, ext->m_OrigX, ext->m_OrigY, 1);
		double n0 = sqrt(ox * ox + oy * oy + oz * oz);
		double n1 = sqrt(px * px + py * py + pz * pz);
		if (n0 > DOUBLE_EPS && n1 > DOUBLE_EPS) {
			double v0[] = { ox / n0, oy / n0, oz / n0 };
			double v1[] = { px / n1, py / n1, pz / n1 };
			double axis[3];
			Vec3Cross(axis, v0, v1);
			double sa = sqrt(Vec3Dot(axis, axis));
			double ca = Vec3Dot(v0, v1);
			double angle = atan2(sa, ca);
			if (x * x + y * y > 1.0)
				angle *= 1.0 + 0.2 * (Math::sqrt(x * x + y * y) - 1);
			double qrot[4], qres[4], qorig[4];
			QuatFromAxisAngle(qrot, axis, angle);
			double nqorig = sqrt(ext->m_OrigQuat[0] * ext->m_OrigQuat[0] + ext->m_OrigQuat[1] * ext->m_OrigQuat[1] + ext->m_OrigQuat[2] * ext->m_OrigQuat[2] + ext->m_OrigQuat[3] * ext->m_OrigQuat[3]);
			if (fabs(nqorig) > DOUBLE_EPS_SQ) {
				qorig[0] = ext->m_OrigQuat[0] / nqorig;
				qorig[1] = ext->m_OrigQuat[1] / nqorig;
				qorig[2] = ext->m_OrigQuat[2] / nqorig;
				qorig[3] = ext->m_OrigQuat[3] / nqorig;
				QuatMult(qres, qrot, qorig);
				ext->Qx = qres[0];
				ext->Qy = qres[1];
				ext->Qz = qres[2];
				ext->Qs = qres[3];
			} else {
				ext->Qx = qrot[0];
				ext->Qy = qrot[1];
				ext->Qz = qrot[2];
				ext->Qs = qrot[3];
			}
			ext->CopyToVar();
			if (bar != NULL)
				bar->NotUpToDate();

			ext->m_PrevX = x;
			ext->m_PrevY = y;
		}
	}

	return true;
}

bool CQuaternionExt::MouseButtonCB(TwMouseButtonID button, bool pressed, int mouseX, int mouseY, int w, int h, void *structExtValue, void *clientData, TwBar *bar, CTwVarGroup *varGrp) {
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(structExtValue);
	if (ext == nullptr)
		return false;
	(void)clientData;
	(void)bar, (void)varGrp;

	if (button == TW_MOUSE_LEFT) {
		if (pressed) {
			ext->m_OrigQuat[0] = ext->Qx;
			ext->m_OrigQuat[1] = ext->Qy;
			ext->m_OrigQuat[2] = ext->Qz;
			ext->m_OrigQuat[3] = ext->Qs;
			ext->m_OrigX = QuatIX(mouseX, w, h);
			ext->m_OrigY = QuatIY(mouseY, w, h);
			ext->m_PrevX = ext->m_OrigX;
			ext->m_PrevY = ext->m_OrigY;
			ext->m_Rotating = true;
		} else
			ext->m_Rotating = false;
	}

	//printf("Click %x\n", structExtValue);
	return true;
}

void CQuaternionExt::MouseLeaveCB(void *structExtValue, void *clientData, TwBar *bar) {
	CQuaternionExt *ext = static_cast<CQuaternionExt *>(structExtValue);
	if (ext == nullptr)
		return;
	(void)clientData;
	(void)bar;

	ext->m_Highlighted = false;
	ext->m_Rotating = false;
}

//  ---------------------------------------------------------------------------
//  Convertion between VC++ Debug/Release std::string
//  (Needed because VC++ adds some extra info to std::string in Debug mode!)
//  And resolve binary std::string incompatibility between VS2010 and other VS versions
//  ---------------------------------------------------------------------------

#ifdef _MSC_VER
// VS2010 store the string allocator pointer at the end
// VS2008 VS2012 and others store the string allocator pointer at the beginning
static void FixVS2010StdStringLibToClient(void *strPtr) {
	char *ptr = (char *)strPtr;
	const size_t SizeOfUndecoratedString = 16 + 2 * sizeof(size_t) + sizeof(void *); // size of a VS std::string without extra debug iterator and info.
	assert(SizeOfUndecoratedString <= sizeof(std::string));
	TwType LibStdStringBaseType = (TwType)(TW_TYPE_STDSTRING & 0xffff0000);
	void **allocAddress2008 = (void **)(ptr + sizeof(std::string) - SizeOfUndecoratedString);
	void **allocAddress2010 = (void **)(ptr + sizeof(std::string) - sizeof(void *));
	if (LibStdStringBaseType == TW_TYPE_STDSTRING_VS2008 && g_TwMgr->m_ClientStdStringBaseType == TW_TYPE_STDSTRING_VS2010) {
		void *allocator = *allocAddress2008;
		memmove(allocAddress2008, allocAddress2008 + 1, SizeOfUndecoratedString - sizeof(void *));
		*allocAddress2010 = allocator;
	} else if (LibStdStringBaseType == TW_TYPE_STDSTRING_VS2010 && g_TwMgr->m_ClientStdStringBaseType == TW_TYPE_STDSTRING_VS2008) {
		void *allocator = *allocAddress2010;
		memmove(allocAddress2008 + 1, allocAddress2008, SizeOfUndecoratedString - sizeof(void *));
		*allocAddress2008 = allocator;
	}
}

static void FixVS2010StdStringClientToLib(void *strPtr) {
	char *ptr = (char *)strPtr;
	const size_t SizeOfUndecoratedString = 16 + 2 * sizeof(size_t) + sizeof(void *); // size of a VS std::string without extra debug iterator and info.
	assert(SizeOfUndecoratedString <= sizeof(std::string));
	TwType LibStdStringBaseType = (TwType)(TW_TYPE_STDSTRING & 0xffff0000);
	void **allocAddress2008 = (void **)(ptr + sizeof(std::string) - SizeOfUndecoratedString);
	void **allocAddress2010 = (void **)(ptr + sizeof(std::string) - sizeof(void *));
	if (LibStdStringBaseType == TW_TYPE_STDSTRING_VS2008 && g_TwMgr->m_ClientStdStringBaseType == TW_TYPE_STDSTRING_VS2010) {
		void *allocator = *allocAddress2010;
		memmove(allocAddress2008 + 1, allocAddress2008, SizeOfUndecoratedString - sizeof(void *));
		*allocAddress2008 = allocator;
	} else if (LibStdStringBaseType == TW_TYPE_STDSTRING_VS2010 && g_TwMgr->m_ClientStdStringBaseType == TW_TYPE_STDSTRING_VS2008) {
		void *allocator = *allocAddress2008;
		memmove(allocAddress2008, allocAddress2008 + 1, SizeOfUndecoratedString - sizeof(void *));
		*allocAddress2010 = allocator;
	}
}
#endif // _MSC_VER

CTwMgr::CClientStdString::CClientStdString() {
	memset(m_Data, 0, sizeof(m_Data));
}

void CTwMgr::CClientStdString::FromLib(const char *libStr) {
	m_LibStr = libStr; // it is ok to have a local copy here
	memcpy(m_Data + sizeof(void *), &m_LibStr, sizeof(std::string));
#ifdef _MSC_VER
	FixVS2010StdStringLibToClient(m_Data + sizeof(void *));
#endif
}

std::string &CTwMgr::CClientStdString::ToClient() {
	DEV_ASSERT(g_TwMgr != nullptr);
	if (g_TwMgr->m_ClientStdStringStructSize == sizeof(std::string) + sizeof(void *))
		return *(std::string *)(m_Data);
	else if (g_TwMgr->m_ClientStdStringStructSize + sizeof(void *) == sizeof(std::string))
		return *(std::string *)(m_Data + 2 * sizeof(void *));
	else {
		DEV_ASSERT(g_TwMgr->m_ClientStdStringStructSize == sizeof(std::string));
		return *(std::string *)(m_Data + sizeof(void *));
	}
}

CTwMgr::CLibStdString::CLibStdString() {
	memset(m_Data, 0, sizeof(m_Data));
}

void CTwMgr::CLibStdString::FromClient(const std::string &clientStr) {
	DEV_ASSERT(g_TwMgr != nullptr);
	memcpy(m_Data + sizeof(void *), &clientStr, g_TwMgr->m_ClientStdStringStructSize);
#ifdef _MSC_VER
	FixVS2010StdStringClientToLib(m_Data + sizeof(void *));
#endif
}

std::string &CTwMgr::CLibStdString::ToLib() {
	DEV_ASSERT(g_TwMgr != nullptr);
	if (g_TwMgr->m_ClientStdStringStructSize == sizeof(std::string) + sizeof(void *))
		return *(std::string *)(m_Data + 2 * sizeof(void *));
	else if (g_TwMgr->m_ClientStdStringStructSize + sizeof(void *) == sizeof(std::string))
		return *(std::string *)(m_Data);
	else {
		DEV_ASSERT(g_TwMgr->m_ClientStdStringStructSize == sizeof(std::string));
		return *(std::string *)(m_Data + sizeof(void *));
	}
}

//  ---------------------------------------------------------------------------
//  Management functions
//  ---------------------------------------------------------------------------

static int TwCreateGraph(void *_Device) {
	DEV_ASSERT(g_TwMgr != nullptr && g_TwMgr->m_Graph == nullptr);

	g_TwMgr->m_Graph = TwCreateRenderer(_Device);

	if (g_TwMgr->m_Graph == nullptr) {
		g_TwMgr->SetLastError(g_ErrUnknownAPI);
		return 0;
	} else
		return g_TwMgr->m_Graph->Init();
}

//  ---------------------------------------------------------------------------

static _FORCE_INLINE_ int TwFreeAsyncDrawing() {
	if (g_TwMgr && g_TwMgr->m_Graph && g_TwMgr->m_Graph->IsDrawing()) {
		const float SLEEP_MAX = 0.25; // wait at most 1/4 second
		PerfTimer timer;
		while (g_TwMgr->m_Graph->IsDrawing() && timer.GetTime() < SLEEP_MAX) {
#if defined(_WINDOWS)
			Sleep(1); // milliseconds
#elif defined(__linux__) || defined(__APPLE__)
			usleep(1000); // microseconds
#endif
		}
		if (g_TwMgr->m_Graph->IsDrawing()) {
			g_TwMgr->SetLastError(g_ErrIsDrawing);
			return 0;
		}
	}
	return 1;
}

//  ---------------------------------------------------------------------------

static int TwInitMgr() {
	DEV_ASSERT(g_TwMasterMgr != nullptr);
	DEV_ASSERT(g_TwMgr != nullptr);

	g_TwMgr->m_CurrentFont = g_DefaultNormalFont;
	g_TwMgr->m_Graph = g_TwMasterMgr->m_Graph;

	g_TwMgr->m_KeyPressedTextObj = g_TwMgr->m_Graph->NewTextObj();
	g_TwMgr->m_InfoTextObj = g_TwMgr->m_Graph->NewTextObj();

	g_TwMgr->m_HelpBar = TwNewBar("TW_HELP");
	if (g_TwMgr->m_HelpBar) {
		g_TwMgr->m_HelpBar->m_Label = "~ Help & Shortcuts ~";
		g_TwMgr->m_HelpBar->m_PosX = 32;
		g_TwMgr->m_HelpBar->m_PosY = 32;
		g_TwMgr->m_HelpBar->m_Width = 400;
		g_TwMgr->m_HelpBar->m_Height = 200;
		g_TwMgr->m_HelpBar->m_ValuesWidth = 12 * (g_TwMgr->m_HelpBar->m_Font->m_CharHeight / 2);
		g_TwMgr->m_HelpBar->m_Color = 0xa05f5f5f; // 0xd75f5f5f;
		g_TwMgr->m_HelpBar->m_DarkText = false;
		g_TwMgr->m_HelpBar->m_IsHelpBar = true;
		g_TwMgr->Minimize(g_TwMgr->m_HelpBar);
	} else
		return 0;

	CColorExt::CreateTypes();
	CQuaternionExt::CreateTypes();

	return 1;
}

int ANT_CALL TwInit(void *_Device) {
#if defined(_DEBUG) && defined(_WINDOWS)
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF));
#endif

	if (g_TwMasterMgr != nullptr) {
		g_TwMasterMgr->SetLastError(g_ErrInit);
		return 0;
	}
	DEV_ASSERT(g_TwMgr == 0);
	DEV_ASSERT(g_Wnds.empty());

	g_TwMasterMgr = new CTwMgr(_Device, TW_MASTER_WINDOW_ID);
	g_Wnds[TW_MASTER_WINDOW_ID] = g_TwMasterMgr;
	g_TwMgr = g_TwMasterMgr;

	TwGenerateDefaultFonts(g_FontScaling);
	g_TwMgr->m_CurrentFont = g_DefaultNormalFont;

	int Res = TwCreateGraph(_Device);
	if (Res)
		Res = TwInitMgr();

	if (!Res)
		TwTerminate();

	return Res;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwSetLastError(const char *_StaticErrorMessage) {
	if (g_TwMasterMgr != 0) {
		g_TwMasterMgr->SetLastError(_StaticErrorMessage);
		return 1;
	} else
		return 0;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwTerminate() {
	if (g_TwMgr == nullptr) {
		//TwGlobalError(g_ErrShut); -> not an error
		return 0; // already shutdown
	}

	// For multi-thread safety
	if (!TwFreeAsyncDrawing())
		return 0;

	CTwWndMap::iterator it;
	for (it = g_Wnds.begin(); it != g_Wnds.end(); it++) {
		g_TwMgr = it->second;

		g_TwMgr->m_Terminating = true;
		TwDeleteAllBars();

		if (g_TwMgr->m_Graph) {
			if (g_TwMgr->m_KeyPressedTextObj) {
				g_TwMgr->m_Graph->DeleteTextObj(g_TwMgr->m_KeyPressedTextObj);
				g_TwMgr->m_KeyPressedTextObj = nullptr;
			}
			if (g_TwMgr->m_InfoTextObj) {
				g_TwMgr->m_Graph->DeleteTextObj(g_TwMgr->m_InfoTextObj);
				g_TwMgr->m_InfoTextObj = nullptr;
			}
			if (g_TwMgr != g_TwMasterMgr)
				g_TwMgr->m_Graph = nullptr;
		}

		if (g_TwMgr != g_TwMasterMgr) {
			delete g_TwMgr;
			g_TwMgr = nullptr;
		}
	}

	// delete g_TwMasterMgr
	int Res = 1;
	g_TwMgr = g_TwMasterMgr;
	if (g_TwMasterMgr->m_Graph) {
		Res = g_TwMasterMgr->m_Graph->Shut();
		delete g_TwMasterMgr->m_Graph;
		g_TwMasterMgr->m_Graph = nullptr;
	}
	TwDeleteDefaultFonts();
	delete g_TwMasterMgr;
	g_TwMasterMgr = nullptr;
	g_TwMgr = nullptr;
	g_Wnds.clear();

	return Res;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwGetCurrentWindow() {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}

	return g_TwMgr->m_WndID;
}

int ANT_CALL TwSetCurrentWindow(int wndID) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}

	if (wndID != g_TwMgr->m_WndID) {
		CTwWndMap::iterator foundWnd = g_Wnds.find(wndID);
		if (foundWnd == g_Wnds.end()) {
			// create a new CTwMgr
			g_TwMgr = new CTwMgr(g_TwMasterMgr->m_Device, wndID);
			g_Wnds[wndID] = g_TwMgr;
			return TwInitMgr();
		} else {
			g_TwMgr = foundWnd->second;
			return 1;
		}
	} else
		return 1;
}

int ANT_CALL TwWindowExists(int wndID) {
	CTwWndMap::iterator foundWnd = g_Wnds.find(wndID);
	if (foundWnd == g_Wnds.end())
		return 0;
	else
		return 1;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwDraw() {
	PERF(PerfTimer Timer; double DT;)
	// CTwFPU fpu;   // fpu precision only forced in update (do not modif dx draw calls)

	if (g_TwMgr == NULL || g_TwMgr->m_Graph == NULL) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}

	DEV_ASSERT(g_TwMgr->m_Bars.size() == g_TwMgr->m_Order.size());

	// For multi-thread savety
	if (!TwFreeAsyncDrawing())
		return 0;

	// Autorepeat TW_MOUSE_PRESSED
	double CurrTime = g_TwMgr->m_Timer.GetTime();
	double RepeatDT = CurrTime - g_TwMgr->m_LastMousePressedTime;
	double DrawDT = CurrTime - g_TwMgr->m_LastDrawTime;
	if (RepeatDT > 2.0 * g_TwMgr->m_RepeatMousePressedDelay || DrawDT > 2.0 * g_TwMgr->m_RepeatMousePressedDelay || abs(g_TwMgr->m_LastMousePressedPosition[0] - g_TwMgr->m_LastMouseX) > 4 || Math::abs(g_TwMgr->m_LastMousePressedPosition[1] - g_TwMgr->m_LastMouseY) > 4) {
		g_TwMgr->m_CanRepeatMousePressed = false;
		g_TwMgr->m_IsRepeatingMousePressed = false;
	}
	if (g_TwMgr->m_CanRepeatMousePressed) {
		if ((!g_TwMgr->m_IsRepeatingMousePressed && RepeatDT > g_TwMgr->m_RepeatMousePressedDelay) || (g_TwMgr->m_IsRepeatingMousePressed && RepeatDT > g_TwMgr->m_RepeatMousePressedPeriod)) {
			g_TwMgr->m_IsRepeatingMousePressed = true;
			g_TwMgr->m_LastMousePressedTime = g_TwMgr->m_Timer.GetTime();
			TwMouseMotion(g_TwMgr->m_LastMouseX, g_TwMgr->m_LastMouseY);
			TwMouseButton(TW_MOUSE_PRESSED, g_TwMgr->m_LastMousePressedButtonID);
		}
	}
	g_TwMgr->m_LastDrawTime = CurrTime;

	if (g_TwMgr->m_WndWidth < 0 || g_TwMgr->m_WndHeight < 0) {
		g_TwMgr->SetLastError(g_ErrBadSize);
		return 0;
	} else if (g_TwMgr->m_WndWidth == 0 || g_TwMgr->m_WndHeight == 0) // probably iconified
		return 1; // nothing to do

	// count number of bars to draw
	int Nb = 0;
	for (size_t i = 0; i < g_TwMgr->m_Bars.size(); ++i)
		if (g_TwMgr->m_Bars[i] != NULL && g_TwMgr->m_Bars[i]->m_Visible)
			++Nb;

	if (Nb > 0) {
		PERF(Timer.Reset();)
		g_TwMgr->m_Graph->BeginDraw(g_TwMgr->m_WndWidth, g_TwMgr->m_WndHeight);
		PERF(DT = Timer.GetTime(); printf("\nBegin=%.4fms ", 1000.0 * DT);)

		PERF(Timer.Reset();)
		std::vector<CRect> TopBarsRects, ClippedBarRects;
		for (size_t i = 0; i < g_TwMgr->m_Bars.size(); ++i) {
			CTwBar *Bar = g_TwMgr->m_Bars[g_TwMgr->m_Order[i]];
			if (Bar->m_Visible) {
				if (g_TwMgr->m_OverlapContent || Bar->IsMinimized())
					Bar->Draw();
				else {
					// Clip overlapped transparent bars to make them more readable
					const int Margin = 4;
					CRect BarRect(Bar->m_PosX - Margin, Bar->m_PosY - Margin, Bar->m_Width + 2 * Margin, Bar->m_Height + 2 * Margin);
					TopBarsRects.clear();
					for (size_t j = i + 1; j < g_TwMgr->m_Bars.size(); ++j) {
						CTwBar *TopBar = g_TwMgr->m_Bars[g_TwMgr->m_Order[j]];
						if (TopBar->m_Visible && !TopBar->IsMinimized())
							TopBarsRects.push_back(CRect(TopBar->m_PosX, TopBar->m_PosY, TopBar->m_Width, TopBar->m_Height));
					}
					ClippedBarRects.clear();
					BarRect.Subtract(TopBarsRects, ClippedBarRects);

					if (ClippedBarRects.size() == 1 && ClippedBarRects[0] == BarRect)
						// g_TwMgr->m_Graph->DrawRect(Bar->m_PosX, Bar->m_PosY, Bar->m_PosX+Bar->m_Width-1, Bar->m_PosY+Bar->m_Height-1, 0x70ffffff); // Clipping test
						Bar->Draw(); // unclipped
					else {
						Bar->Draw(CTwBar::DRAW_BG); // draw background only

						// draw content for each clipped rectangle
						for (size_t j = 0; j < ClippedBarRects.size(); j++)
							if (ClippedBarRects[j].W > 1 && ClippedBarRects[j].H > 1) {
								g_TwMgr->m_Graph->SetScissor(ClippedBarRects[j].X + 1, ClippedBarRects[j].Y, ClippedBarRects[j].W, ClippedBarRects[j].H - 1);
								// g_TwMgr->m_Graph->DrawRect(0, 0, 1000, 1000, 0x70ffffff); // Clipping test
								Bar->Draw(CTwBar::DRAW_CONTENT);
							}
						g_TwMgr->m_Graph->SetScissor(0, 0, 0, 0);
					}
				}
			}
		}
		PERF(DT = Timer.GetTime(); printf("Draw=%.4fms ", 1000.0 * DT);)

		PERF(Timer.Reset();)
		g_TwMgr->m_Graph->EndDraw();
		PERF(DT = Timer.GetTime(); printf("End=%.4fms\n", 1000.0 * DT);)
	}

	return 1;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwWindowSize(int _Width, int _Height) {
	g_InitWndWidth = _Width;
	g_InitWndHeight = _Height;

	if (g_TwMgr == nullptr || g_TwMgr->m_Graph == nullptr) {
		// TwGlobalError(g_ErrNotInit);  -> not an error here
		return 0; // not initialized
	}

	if (_Width < 0 || _Height < 0) {
		g_TwMgr->SetLastError(g_ErrBadSize);
		return 0;
	}

	// For multi-thread savety
	if (!TwFreeAsyncDrawing())
		return 0;

	// Delete the extra text objects
	if (g_TwMgr->m_KeyPressedTextObj) {
		g_TwMgr->m_Graph->DeleteTextObj(g_TwMgr->m_KeyPressedTextObj);
		g_TwMgr->m_KeyPressedTextObj = NULL;
	}
	if (g_TwMgr->m_InfoTextObj) {
		g_TwMgr->m_Graph->DeleteTextObj(g_TwMgr->m_InfoTextObj);
		g_TwMgr->m_InfoTextObj = NULL;
	}

	g_TwMgr->m_WndWidth = _Width;
	g_TwMgr->m_WndHeight = _Height;
	g_TwMgr->m_Graph->Restore();

	// Recreate extra text objects
	if (g_TwMgr->m_WndWidth != 0 && g_TwMgr->m_WndHeight != 0) {
		if (g_TwMgr->m_KeyPressedTextObj == NULL) {
			g_TwMgr->m_KeyPressedTextObj = g_TwMgr->m_Graph->NewTextObj();
			g_TwMgr->m_KeyPressedBuildText = true;
		}
		if (g_TwMgr->m_InfoTextObj == NULL) {
			g_TwMgr->m_InfoTextObj = g_TwMgr->m_Graph->NewTextObj();
			g_TwMgr->m_InfoBuildText = true;
		}
	}

	for (std::vector<TwBar *>::iterator it = g_TwMgr->m_Bars.begin(); it != g_TwMgr->m_Bars.end(); ++it)
		(*it)->NotUpToDate();

	return 1;
}

//  ---------------------------------------------------------------------------

CTwMgr::CTwMgr(void *_Device, int _WndID) {
	m_Device = _Device;
	m_WndID = _WndID;
	m_LastError = NULL;
	m_CurrentDbgFile = "";
	m_CurrentDbgLine = 0;
	m_Graph = NULL;
	m_WndWidth = g_InitWndWidth;
	m_WndHeight = g_InitWndHeight;
	m_CurrentFont = NULL; // set after by TwIntialize
	m_NbMinimizedBars = 0;
	m_HelpBar = NULL;
	m_HelpBarNotUpToDate = true;
	m_HelpBarUpdateNow = false;
	m_LastHelpUpdateTime = 0;
	m_LastMouseX = -1;
	m_LastMouseY = -1;
	m_LastMouseWheelPos = 0;
	m_IconPos = 0;
	m_IconAlign = 0;
	m_IconMarginX = m_IconMarginY = 8;
	m_FontResizable = true;
	m_KeyPressedTextObj = NULL;
	m_KeyPressedBuildText = false;
	m_KeyPressedTime = 0;
	m_InfoTextObj = NULL;
	m_InfoBuildText = true;
	m_BarInitColorHue = 155;
	m_PopupBar = NULL;
	m_TypeColor32 = TW_TYPE_UNDEF;
	m_TypeColor3F = TW_TYPE_UNDEF;
	m_TypeColor4F = TW_TYPE_UNDEF;
	m_LastMousePressedTime = 0;
	m_LastMousePressedButtonID = TW_MOUSE_MIDDLE;
	m_LastMousePressedPosition[0] = -1000;
	m_LastMousePressedPosition[1] = -1000;
	m_RepeatMousePressedDelay = 0.5;
	m_RepeatMousePressedPeriod = 0.1;
	m_CanRepeatMousePressed = false;
	m_IsRepeatingMousePressed = false;
	m_LastDrawTime = 0;
	m_UseOldColorScheme = false;
	m_Contained = false;
	m_ButtonAlign = BUTTON_ALIGN_RIGHT;
	m_OverlapContent = false;
	m_Terminating = false;

	m_CopyCDStringToClient = g_InitCopyCDStringToClient;
	m_CopyStdStringToClient = g_InitCopyStdStringToClient;
	m_ClientStdStringStructSize = 0;
	m_ClientStdStringBaseType = (TwType)0;
}

//  ---------------------------------------------------------------------------

CTwMgr::~CTwMgr() {
}

//  ---------------------------------------------------------------------------

int CTwMgr::FindBar(const char *_Name) const {
	if (_Name == NULL || strlen(_Name) <= 0)
		return -1;
	int i;
	for (i = 0; i < (int)m_Bars.size(); ++i)
		if (m_Bars[i] != NULL && strcmp(_Name, m_Bars[i]->m_Name.c_str()) == 0)
			return i;
	return -1;
}

//  ---------------------------------------------------------------------------

int CTwMgr::HasAttrib(const char *_Attrib, bool *_HasValue) const {
	*_HasValue = true;
	if (_stricmp(_Attrib, "help") == 0)
		return MGR_HELP;
	else if (_stricmp(_Attrib, "fontsize") == 0)
		return MGR_FONT_SIZE;
	else if (_stricmp(_Attrib, "fontstyle") == 0)
		return MGR_FONT_STYLE;
	else if (_stricmp(_Attrib, "iconpos") == 0)
		return MGR_ICON_POS;
	else if (_stricmp(_Attrib, "iconalign") == 0)
		return MGR_ICON_ALIGN;
	else if (_stricmp(_Attrib, "iconmargin") == 0)
		return MGR_ICON_MARGIN;
	else if (_stricmp(_Attrib, "fontresizable") == 0)
		return MGR_FONT_RESIZABLE;
	else if (_stricmp(_Attrib, "colorscheme") == 0)
		return MGR_COLOR_SCHEME;
	else if (_stricmp(_Attrib, "contained") == 0)
		return MGR_CONTAINED;
	else if (_stricmp(_Attrib, "buttonalign") == 0)
		return MGR_BUTTON_ALIGN;
	else if (_stricmp(_Attrib, "overlap") == 0)
		return MGR_OVERLAP;

	*_HasValue = false;
	return 0; // not found
}

int CTwMgr::SetAttrib(int _AttribID, const char *_Value) {
	switch (_AttribID) {
		case MGR_HELP:
			if (_Value && strlen(_Value) > 0) {
				m_Help = _Value;
				m_HelpBarNotUpToDate = true;
				return 1;
			} else {
				SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_FONT_SIZE:
			if (_Value && strlen(_Value) > 0) {
				int s;
				int n = sscanf(_Value, "%d", &s);
				if (n == 1 && s >= 1 && s <= 3) {
					if (s == 1)
						SetFont(g_DefaultSmallFont, true);
					else if (s == 2)
						SetFont(g_DefaultNormalFont, true);
					else if (s == 3)
						SetFont(g_DefaultLargeFont, true);
					return 1;
				} else {
					SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_FONT_STYLE:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "fixed") == 0) {
					if (m_CurrentFont != g_DefaultFixed1Font) {
						SetFont(g_DefaultFixed1Font, true);
						m_FontResizable = false; // for now fixed font is not resizable
					}
					return 1;
				} else if (_stricmp(_Value, "default") == 0) {
					if (m_CurrentFont != g_DefaultSmallFont && m_CurrentFont != g_DefaultNormalFont && m_CurrentFont != g_DefaultLargeFont) {
						if (m_CurrentFont == g_DefaultFixed1Font)
							m_FontResizable = true;
						SetFont(g_DefaultNormalFont, true);
					}
					return 1;
				} else {
					SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_ICON_POS:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "bl") == 0 || _stricmp(_Value, "lb") == 0 || _stricmp(_Value, "bottomleft") == 0 || _stricmp(_Value, "leftbottom") == 0) {
					m_IconPos = 0;
					return 1;
				} else if (_stricmp(_Value, "br") == 0 || _stricmp(_Value, "rb") == 0 || _stricmp(_Value, "bottomright") == 0 || _stricmp(_Value, "rightbottom") == 0) {
					m_IconPos = 1;
					return 1;
				} else if (_stricmp(_Value, "tl") == 0 || _stricmp(_Value, "lt") == 0 || _stricmp(_Value, "topleft") == 0 || _stricmp(_Value, "lefttop") == 0) {
					m_IconPos = 2;
					return 1;
				} else if (_stricmp(_Value, "tr") == 0 || _stricmp(_Value, "rt") == 0 || _stricmp(_Value, "topright") == 0 || _stricmp(_Value, "righttop") == 0) {
					m_IconPos = 3;
					return 1;
				} else {
					SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_ICON_ALIGN:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "vert") == 0 || _stricmp(_Value, "vertical") == 0) {
					m_IconAlign = 0;
					return 1;
				} else if (_stricmp(_Value, "horiz") == 0 || _stricmp(_Value, "horizontal") == 0) {
					m_IconAlign = 1;
					return 1;
				} else {
					SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_ICON_MARGIN:
			if (_Value && strlen(_Value) > 0) {
				int x, y;
				int n = sscanf(_Value, "%d%d", &x, &y);
				if (n == 2 && x >= 0 && y >= 0) {
					m_IconMarginX = x;
					m_IconMarginY = y;
					return 1;
				} else {
					SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_FONT_RESIZABLE:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					m_FontResizable = true;
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					m_FontResizable = false;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_COLOR_SCHEME:
			if (_Value && strlen(_Value) > 0) {
				int s;
				int n = sscanf(_Value, "%d", &s);
				if (n == 1 && s >= 0 && s <= 1) {
					if (s == 0)
						m_UseOldColorScheme = true;
					else
						m_UseOldColorScheme = false;
					return 1;
				} else {
					SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_CONTAINED:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0)
					m_Contained = true;
				else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0)
					m_Contained = false;
				else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
				std::vector<TwBar *>::iterator barIt;
				for (barIt = g_TwMgr->m_Bars.begin(); barIt != g_TwMgr->m_Bars.end(); ++barIt)
					if ((*barIt) != NULL)
						(*barIt)->m_Contained = m_Contained;
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_BUTTON_ALIGN:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "left") == 0)
					m_ButtonAlign = BUTTON_ALIGN_LEFT;
				else if (_stricmp(_Value, "center") == 0)
					m_ButtonAlign = BUTTON_ALIGN_CENTER;
				else if (_stricmp(_Value, "right") == 0)
					m_ButtonAlign = BUTTON_ALIGN_RIGHT;
				else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
				std::vector<TwBar *>::iterator barIt;
				for (barIt = g_TwMgr->m_Bars.begin(); barIt != g_TwMgr->m_Bars.end(); ++barIt)
					if ((*barIt) != NULL)
						(*barIt)->m_ButtonAlign = m_ButtonAlign;
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case MGR_OVERLAP:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					m_OverlapContent = true;
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					m_OverlapContent = false;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		default:
			g_TwMgr->SetLastError(g_ErrUnknownAttrib);
			return 0;
	}
}

ERetType CTwMgr::GetAttrib(int _AttribID, std::vector<double> &outDoubles, std::ostringstream &outString) const {
	outDoubles.clear();
	outString.clear();

	switch (_AttribID) {
		case MGR_HELP:
			outString << m_Help;
			return RET_STRING;
		case MGR_FONT_SIZE:
			if (m_CurrentFont == g_DefaultSmallFont)
				outDoubles.push_back(1);
			else if (m_CurrentFont == g_DefaultNormalFont)
				outDoubles.push_back(2);
			else if (m_CurrentFont == g_DefaultLargeFont)
				outDoubles.push_back(3);
			else
				outDoubles.push_back(0); // should not happened
			return RET_DOUBLE;
		case MGR_FONT_STYLE:
			if (m_CurrentFont == g_DefaultFixed1Font)
				outString << "fixed";
			else
				outString << "default";
			return RET_STRING;
		case MGR_ICON_POS:
			if (m_IconPos == 0)
				outString << "bottomleft";
			else if (m_IconPos == 1)
				outString << "bottomright";
			else if (m_IconPos == 2)
				outString << "topleft";
			else if (m_IconPos == 3)
				outString << "topright";
			else
				outString << "undefined"; // should not happened
			return RET_STRING;
		case MGR_ICON_ALIGN:
			if (m_IconAlign == 0)
				outString << "vertical";
			else if (m_IconAlign == 1)
				outString << "horizontal";
			else
				outString << "undefined"; // should not happened
			return RET_STRING;
		case MGR_ICON_MARGIN:
			outDoubles.push_back(m_IconMarginX);
			outDoubles.push_back(m_IconMarginY);
			return RET_DOUBLE;
		case MGR_FONT_RESIZABLE:
			outDoubles.push_back(m_FontResizable);
			return RET_DOUBLE;
		case MGR_COLOR_SCHEME:
			outDoubles.push_back(m_UseOldColorScheme ? 0 : 1);
			return RET_DOUBLE;
		case MGR_CONTAINED: {
			bool contained = m_Contained;
			outDoubles.push_back(contained);
			return RET_DOUBLE;
		}
		case MGR_BUTTON_ALIGN:
			if (m_ButtonAlign == BUTTON_ALIGN_LEFT)
				outString << "left";
			else if (m_ButtonAlign == BUTTON_ALIGN_CENTER)
				outString << "center";
			else
				outString << "right";
			return RET_STRING;
		case MGR_OVERLAP:
			outDoubles.push_back(m_OverlapContent);
			return RET_DOUBLE;
		default:
			g_TwMgr->SetLastError(g_ErrUnknownAttrib);
			return RET_ERROR;
	}
}

//  ---------------------------------------------------------------------------

void CTwMgr::Minimize(TwBar *_Bar) {
	DEV_ASSERT(m_Graph != NULL && _Bar != NULL);
	DEV_ASSERT(m_Bars.size() == m_MinOccupied.size());
	if (_Bar->m_IsMinimized)
		return;
	if (_Bar->m_Visible) {
		size_t i = m_NbMinimizedBars;
		m_NbMinimizedBars++;
		for (i = 0; i < m_MinOccupied.size(); ++i)
			if (!m_MinOccupied[i])
				break;
		if (i < m_MinOccupied.size())
			m_MinOccupied[i] = true;
		_Bar->m_MinNumber = (int)i;
	} else
		_Bar->m_MinNumber = -1;
	_Bar->m_IsMinimized = true;
	_Bar->NotUpToDate();
}

//  ---------------------------------------------------------------------------

void CTwMgr::Maximize(TwBar *_Bar) {
	DEV_ASSERT(m_Graph != nullptr && _Bar != nullptr);
	DEV_ASSERT(m_Bars.size() == m_MinOccupied.size());
	if (!_Bar->m_IsMinimized)
		return;
	if (_Bar->m_Visible) {
		--m_NbMinimizedBars;
		if (m_NbMinimizedBars < 0)
			m_NbMinimizedBars = 0;
		if (_Bar->m_MinNumber >= 0 && _Bar->m_MinNumber < (int)m_MinOccupied.size())
			m_MinOccupied[_Bar->m_MinNumber] = false;
	}
	_Bar->m_IsMinimized = false;
	_Bar->NotUpToDate();
	if (_Bar->m_IsHelpBar)
		m_HelpBarNotUpToDate = true;
}

//  ---------------------------------------------------------------------------

void CTwMgr::Hide(TwBar *_Bar) {
	DEV_ASSERT(m_Graph != nullptr && _Bar != nullptr);
	if (!_Bar->m_Visible)
		return;
	if (_Bar->IsMinimized()) {
		Maximize(_Bar);
		_Bar->m_Visible = false;
		Minimize(_Bar);
	} else
		_Bar->m_Visible = false;
	if (!_Bar->m_IsHelpBar)
		m_HelpBarNotUpToDate = true;
}

//  ---------------------------------------------------------------------------

void CTwMgr::Unhide(TwBar *_Bar) {
	DEV_ASSERT(m_Graph != nullptr && _Bar != nullptr);
	if (_Bar->m_Visible)
		return;
	if (_Bar->IsMinimized()) {
		Maximize(_Bar);
		_Bar->m_Visible = true;
		Minimize(_Bar);
	} else
		_Bar->m_Visible = true;
	_Bar->NotUpToDate();
	if (!_Bar->m_IsHelpBar)
		m_HelpBarNotUpToDate = true;
}

//  ---------------------------------------------------------------------------

void CTwMgr::SetFont(const CTexFont *_Font, bool _ResizeBars) {
	DEV_ASSERT(m_Graph != nullptr);
	DEV_ASSERT(_Font != nullptr);

	m_CurrentFont = _Font;

	for (int i = 0; i < (int)m_Bars.size(); ++i)
		if (m_Bars[i] != NULL) {
			int fh = m_Bars[i]->m_Font->m_CharHeight;
			m_Bars[i]->m_Font = _Font;
			if (_ResizeBars) {
				if (m_Bars[i]->m_Movable) {
					m_Bars[i]->m_PosX += (3 * (fh - _Font->m_CharHeight)) / 2;
					m_Bars[i]->m_PosY += (fh - _Font->m_CharHeight) / 2;
				}
				if (m_Bars[i]->m_Resizable) {
					m_Bars[i]->m_Width = (m_Bars[i]->m_Width * _Font->m_CharHeight) / fh;
					m_Bars[i]->m_Height = (m_Bars[i]->m_Height * _Font->m_CharHeight) / fh;
					m_Bars[i]->m_ValuesWidth = (m_Bars[i]->m_ValuesWidth * _Font->m_CharHeight) / fh;
				}
			}
			m_Bars[i]->NotUpToDate();
		}

	if (g_TwMgr->m_HelpBar != nullptr)
		g_TwMgr->m_HelpBar->Update();
	g_TwMgr->m_InfoBuildText = true;
	g_TwMgr->m_KeyPressedBuildText = true;
	m_HelpBarNotUpToDate = true;
}

//  ---------------------------------------------------------------------------

void ANT_CALL TwGlobalError(const char *_ErrorMessage) // to be called when g_TwMasterMgr is not created
{
	if (g_ErrorHandler == nullptr) {
		fprintf(stderr, "ERROR(AntTweakBar) >> %s\n", _ErrorMessage);
#ifdef _WINDOWS
		OutputDebugString("ERROR(AntTweakBar) >> ");
		OutputDebugString(_ErrorMessage);
		OutputDebugString("\n");
#endif // ANT_WINDOWS
	} else
		g_ErrorHandler(_ErrorMessage);

	if (g_BreakOnError)
		abort();
}

//  ---------------------------------------------------------------------------

void CTwMgr::SetLastError(const char *_ErrorMessage) // _ErrorMessage must be a static string
{
	if (this != g_TwMasterMgr) {
		// route to master
		g_TwMasterMgr->SetLastError(_ErrorMessage);
		return;
	}

	m_LastError = _ErrorMessage;

	if (g_ErrorHandler == NULL) {
		if (m_CurrentDbgFile != NULL && strlen(m_CurrentDbgFile) > 0 && m_CurrentDbgLine > 0)
			fprintf(stderr, "%s(%d): ", m_CurrentDbgFile, m_CurrentDbgLine);
		fprintf(stderr, "ERROR(AntTweakBar) >> %s\n", m_LastError);
#ifdef _WINDOWS
		if (m_CurrentDbgFile != NULL && strlen(m_CurrentDbgFile) > 0 && m_CurrentDbgLine > 0) {
			OutputDebugString(m_CurrentDbgFile);
			char sl[32];
			sprintf(sl, "(%d): ", m_CurrentDbgLine);
			OutputDebugString(sl);
		}
		OutputDebugString("ERROR(AntTweakBar) >> ");
		OutputDebugString(m_LastError);
		OutputDebugString("\n");
#endif // _WINDOWS
	} else
		g_ErrorHandler(_ErrorMessage);

	if (g_BreakOnError)
		abort();
}

//  ---------------------------------------------------------------------------

const char *CTwMgr::GetLastError() {
	if (this != g_TwMasterMgr) {
		// route to master
		return g_TwMasterMgr->GetLastError();
	}

	const char *Err = m_LastError;
	m_LastError = NULL;
	return Err;
}

//  ---------------------------------------------------------------------------

const char *CTwMgr::CheckLastError() const {
	return m_LastError;
}

//  ---------------------------------------------------------------------------

void CTwMgr::SetCurrentDbgParams(const char *dbgFile, int dbgLine) {
	m_CurrentDbgFile = dbgFile;
	m_CurrentDbgLine = dbgLine;
}

//  ---------------------------------------------------------------------------

int ANT_CALL __TwDbg(const char *dbgFile, int dbgLine) {
	if (g_TwMgr != nullptr)
		g_TwMgr->SetCurrentDbgParams(dbgFile, dbgLine);
	return 0; // always returns zero
}

//  ---------------------------------------------------------------------------

void ANT_CALL TwHandleErrors(TwErrorHandler _ErrorHandler, int _BreakOnError) {
	g_ErrorHandler = _ErrorHandler;
	g_BreakOnError = (_BreakOnError) ? true : false;
}

void ANT_CALL TwHandleErrors(TwErrorHandler _ErrorHandler) {
	TwHandleErrors(_ErrorHandler, false);
}

//  ---------------------------------------------------------------------------

const char *ANT_CALL TwGetLastError() {
	if (g_TwMasterMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return g_ErrNotInit;
	} else
		return g_TwMasterMgr->GetLastError();
}

//  ---------------------------------------------------------------------------

TwBar *ANT_CALL TwNewBar(const char *_Name) {
	if (g_TwMgr == nullptr || g_TwMgr->m_Graph == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return nullptr; // not initialized
	}

	TwFreeAsyncDrawing(); // For multi-thread savety

	if (_Name == nullptr || strlen(_Name) <= 0) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return nullptr;
	}
	if (g_TwMgr->FindBar(_Name) >= 0) {
		g_TwMgr->SetLastError(g_ErrExist);
		return nullptr;
	}

	if (strstr(_Name, "`") != nullptr) {
		g_TwMgr->SetLastError(g_ErrNoBackQuote);
		return nullptr;
	}

	if (g_TwMgr->m_PopupBar != nullptr) // delete popup bar if it exists
	{
		TwDeleteBar(g_TwMgr->m_PopupBar);
		g_TwMgr->m_PopupBar = nullptr;
	}

	TwBar *Bar = new CTwBar(_Name);
	g_TwMgr->m_Bars.push_back(Bar);
	g_TwMgr->m_Order.push_back((int)g_TwMgr->m_Bars.size() - 1);
	g_TwMgr->m_MinOccupied.push_back(false);
	g_TwMgr->m_HelpBarNotUpToDate = true;

	return Bar;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwDeleteBar(TwBar *_Bar) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (_Bar == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}
	if (_Bar == g_TwMgr->m_HelpBar) {
		g_TwMgr->SetLastError(g_ErrDelHelp);
		return 0;
	}

	TwFreeAsyncDrawing(); // For multi-thread savety

	std::vector<TwBar *>::iterator BarIt;
	int i = 0;
	for (BarIt = g_TwMgr->m_Bars.begin(); BarIt != g_TwMgr->m_Bars.end(); ++BarIt, ++i)
		if ((*BarIt) == _Bar)
			break;
	if (BarIt == g_TwMgr->m_Bars.end()) {
		g_TwMgr->SetLastError(g_ErrNotFound);
		return 0;
	}

	if (g_TwMgr->m_PopupBar != NULL && _Bar != g_TwMgr->m_PopupBar) { // delete popup bar first if it exists
		TwDeleteBar(g_TwMgr->m_PopupBar);
		g_TwMgr->m_PopupBar = NULL;
	}

	// force bar to un-minimize
	g_TwMgr->Maximize(_Bar);
	// find an empty MinOccupied
	std::vector<bool>::iterator itm;
	int j = 0;
	for (itm = g_TwMgr->m_MinOccupied.begin(); itm != g_TwMgr->m_MinOccupied.end(); ++itm, ++j)
		if ((*itm) == false)
			break;
	DEV_ASSERT(itm != g_TwMgr->m_MinOccupied.end());
	// shift MinNumbers and erase the empty MinOccupied
	for (size_t k = 0; k < g_TwMgr->m_Bars.size(); ++k)
		if (g_TwMgr->m_Bars[k] != NULL && g_TwMgr->m_Bars[k]->m_MinNumber > j)
			g_TwMgr->m_Bars[k]->m_MinNumber -= 1;
	g_TwMgr->m_MinOccupied.erase(itm);
	// erase _Bar order
	std::vector<int>::iterator BarOrderIt = g_TwMgr->m_Order.end();
	for (std::vector<int>::iterator it = g_TwMgr->m_Order.begin(); it != g_TwMgr->m_Order.end(); ++it)
		if ((*it) == i)
			BarOrderIt = it;
		else if ((*it) > i)
			(*it) -= 1;
	DEV_ASSERT(BarOrderIt != g_TwMgr->m_Order.end());
	g_TwMgr->m_Order.erase(BarOrderIt);

	// erase & delete _Bar
	g_TwMgr->m_Bars.erase(BarIt);
	delete _Bar;

	g_TwMgr->m_HelpBarNotUpToDate = true;
	return 1;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwDeleteAllBars() {
	if (g_TwMgr == NULL) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}

	TwFreeAsyncDrawing(); // For multi-thread savety

	int n = 0;
	if (g_TwMgr->m_Terminating || g_TwMgr->m_HelpBar == NULL) {
		for (size_t i = 0; i < g_TwMgr->m_Bars.size(); ++i)
			if (g_TwMgr->m_Bars[i] != NULL) {
				++n;
				delete g_TwMgr->m_Bars[i];
				g_TwMgr->m_Bars[i] = NULL;
			}
		g_TwMgr->m_Bars.clear();
		g_TwMgr->m_Order.clear();
		g_TwMgr->m_MinOccupied.clear();
		g_TwMgr->m_HelpBarNotUpToDate = true;
	} else {
		std::vector<CTwBar *> bars = g_TwMgr->m_Bars;
		for (size_t i = 0; i < bars.size(); ++i)
			if (bars[i] != 0 && bars[i] != g_TwMgr->m_HelpBar) {
				++n;
				TwDeleteBar(bars[i]);
			}
		g_TwMgr->m_HelpBarNotUpToDate = true;
	}

	if (n == 0) {
		return 0; // g_TwMgr->SetLastError(g_ErrNthToDo);
	} else
		return 1;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwSetTopBar(const TwBar *_Bar) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (_Bar == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}

	TwFreeAsyncDrawing(); // For multi-thread savety

	if (_Bar != g_TwMgr->m_PopupBar && g_TwMgr->m_BarAlwaysOnBottom.length() > 0) {
		if (strcmp(_Bar->m_Name.c_str(), g_TwMgr->m_BarAlwaysOnBottom.c_str()) == 0)
			return TwSetBottomBar(_Bar);
	}

	int i = -1, iOrder;
	for (iOrder = 0; iOrder < (int)g_TwMgr->m_Bars.size(); ++iOrder) {
		i = g_TwMgr->m_Order[iOrder];
		DEV_ASSERT(i >= 0 && i < (int)g_TwMgr->m_Bars.size());
		if (g_TwMgr->m_Bars[i] == _Bar)
			break;
	}
	if (i < 0 || iOrder >= (int)g_TwMgr->m_Bars.size()) { // bar not found
		g_TwMgr->SetLastError(g_ErrNotFound);
		return 0;
	}

	for (int j = iOrder; j < (int)g_TwMgr->m_Bars.size() - 1; ++j)
		g_TwMgr->m_Order[j] = g_TwMgr->m_Order[j + 1];
	g_TwMgr->m_Order[(int)g_TwMgr->m_Bars.size() - 1] = i;

	if (_Bar != g_TwMgr->m_PopupBar && g_TwMgr->m_BarAlwaysOnTop.length() > 0) {
		int topIdx = g_TwMgr->FindBar(g_TwMgr->m_BarAlwaysOnTop.c_str());
		TwBar *top = (topIdx >= 0 && topIdx < (int)g_TwMgr->m_Bars.size()) ? g_TwMgr->m_Bars[topIdx] : nullptr;
		if (top != nullptr && top != _Bar)
			TwSetTopBar(top);
	}

	if (g_TwMgr->m_PopupBar != nullptr && _Bar != g_TwMgr->m_PopupBar)
		TwSetTopBar(g_TwMgr->m_PopupBar);

	return 1;
}

//  ---------------------------------------------------------------------------

TwBar *ANT_CALL TwGetTopBar() {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return nullptr; // not initialized
	}

	if (g_TwMgr->m_Bars.size() > 0 && g_TwMgr->m_PopupBar == nullptr)
		return g_TwMgr->m_Bars[g_TwMgr->m_Order[g_TwMgr->m_Bars.size() - 1]];
	else if (g_TwMgr->m_Bars.size() > 1 && g_TwMgr->m_PopupBar != nullptr)
		return g_TwMgr->m_Bars[g_TwMgr->m_Order[g_TwMgr->m_Bars.size() - 2]];
	else
		return nullptr;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwSetBottomBar(const TwBar *_Bar) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (_Bar == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}

	TwFreeAsyncDrawing(); // For multi-thread savety

	if (_Bar != g_TwMgr->m_PopupBar && g_TwMgr->m_BarAlwaysOnTop.length() > 0) {
		if (strcmp(_Bar->m_Name.c_str(), g_TwMgr->m_BarAlwaysOnTop.c_str()) == 0)
			return TwSetTopBar(_Bar);
	}

	int i = -1, iOrder;
	for (iOrder = 0; iOrder < (int)g_TwMgr->m_Bars.size(); ++iOrder) {
		i = g_TwMgr->m_Order[iOrder];
		assert(i >= 0 && i < (int)g_TwMgr->m_Bars.size());
		if (g_TwMgr->m_Bars[i] == _Bar)
			break;
	}
	if (i < 0 || iOrder >= (int)g_TwMgr->m_Bars.size()) // bar not found
	{
		g_TwMgr->SetLastError(g_ErrNotFound);
		return 0;
	}

	if (iOrder > 0)
		for (int j = iOrder - 1; j >= 0; --j)
			g_TwMgr->m_Order[j + 1] = g_TwMgr->m_Order[j];
	g_TwMgr->m_Order[0] = i;

	if (_Bar != g_TwMgr->m_PopupBar && g_TwMgr->m_BarAlwaysOnBottom.length() > 0) {
		int btmIdx = g_TwMgr->FindBar(g_TwMgr->m_BarAlwaysOnBottom.c_str());
		TwBar *btm = (btmIdx >= 0 && btmIdx < (int)g_TwMgr->m_Bars.size()) ? g_TwMgr->m_Bars[btmIdx] : nullptr;
		if (btm != nullptr && btm != _Bar)
			TwSetBottomBar(btm);
	}

	return 1;
}

//  ---------------------------------------------------------------------------

TwBar *ANT_CALL TwGetBottomBar() {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return nullptr; // not initialized
	}

	if (g_TwMgr->m_Bars.size() > 0)
		return g_TwMgr->m_Bars[g_TwMgr->m_Order[0]];
	else
		return nullptr;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwSetBarState(TwBar *_Bar, TwState _State) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (_Bar == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}

	TwFreeAsyncDrawing(); // For multi-thread savety

	switch (_State) {
		case TW_STATE_SHOWN:
			g_TwMgr->Unhide(_Bar);
			return 1;
		case TW_STATE_ICONIFIED:
			//g_TwMgr->Unhide(_Bar);
			g_TwMgr->Minimize(_Bar);
			return 1;
		case TW_STATE_HIDDEN:
			//g_TwMgr->Maximize(_Bar);
			g_TwMgr->Hide(_Bar);
			return 1;
		case TW_STATE_UNICONIFIED:
			//g_TwMgr->Unhide(_Bar);
			g_TwMgr->Maximize(_Bar);
			return 1;
		default:
			g_TwMgr->SetLastError(g_ErrBadParam);
			return 0;
	}
}

//  ---------------------------------------------------------------------------

const char *ANT_CALL TwGetBarName(const TwBar *_Bar) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return nullptr; // not initialized
	}
	if (_Bar == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return nullptr;
	}
	std::vector<TwBar *>::iterator BarIt;
	int i = 0;
	for (BarIt = g_TwMgr->m_Bars.begin(); BarIt != g_TwMgr->m_Bars.end(); ++BarIt, ++i)
		if ((*BarIt) == _Bar)
			break;
	if (BarIt == g_TwMgr->m_Bars.end()) {
		g_TwMgr->SetLastError(g_ErrNotFound);
		return nullptr;
	}

	return _Bar->m_Name.c_str();
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwGetBarCount() {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}

	return (int)g_TwMgr->m_Bars.size();
}

//  ---------------------------------------------------------------------------

TwBar *ANT_CALL TwGetBarByIndex(int index) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return nullptr; // not initialized
	}

	if (index >= 0 && index < (int)g_TwMgr->m_Bars.size())
		return g_TwMgr->m_Bars[index];
	else {
		g_TwMgr->SetLastError(g_ErrOutOfRange);
		return nullptr;
	}
}

//  ---------------------------------------------------------------------------

TwBar *ANT_CALL TwGetBarByName(const char *name) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return nullptr; // not initialized
	}

	int idx = g_TwMgr->FindBar(name);
	if (idx >= 0 && idx < (int)g_TwMgr->m_Bars.size())
		return g_TwMgr->m_Bars[idx];
	else
		return nullptr;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwRefreshBar(TwBar *bar) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (bar == nullptr) {
		std::vector<TwBar *>::iterator BarIt;
		for (BarIt = g_TwMgr->m_Bars.begin(); BarIt != g_TwMgr->m_Bars.end(); ++BarIt)
			if (*BarIt != nullptr)
				(*BarIt)->NotUpToDate();
	} else {
		std::vector<TwBar *>::iterator BarIt;
		int i = 0;
		for (BarIt = g_TwMgr->m_Bars.begin(); BarIt != g_TwMgr->m_Bars.end(); ++BarIt, ++i)
			if ((*BarIt) == bar)
				break;
		if (BarIt == g_TwMgr->m_Bars.end()) {
			g_TwMgr->SetLastError(g_ErrNotFound);
			return 0;
		}

		bar->NotUpToDate();
	}
	return 1;
}

//  ---------------------------------------------------------------------------

int BarVarHasAttrib(CTwBar *_Bar, CTwVar *_Var, const char *_Attrib, bool *_HasValue);
int BarVarSetAttrib(CTwBar *_Bar, CTwVar *_Var, CTwVarGroup *_VarParent, int _VarIndex, int _AttribID, const char *_Value);
ERetType BarVarGetAttrib(CTwBar *_Bar, CTwVar *_Var, CTwVarGroup *_VarParent, int _VarIndex, int _AttribID, std::vector<double> &outDouble, std::ostringstream &outString);

int ANT_CALL TwGetParam(TwBar *bar, const char *varName, const char *paramName, TwParamValueType paramValueType, unsigned int outValueMaxCount, void *outValues) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (paramName == nullptr || strlen(paramName) <= 0) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}
	if (outValueMaxCount <= 0 || outValues == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}

	if (bar == nullptr)
		bar = TW_GLOBAL_BAR;
	else {
		std::vector<TwBar *>::iterator barIt;
		int i = 0;
		for (barIt = g_TwMgr->m_Bars.begin(); barIt != g_TwMgr->m_Bars.end(); ++barIt, ++i)
			if ((*barIt) == bar)
				break;
		if (barIt == g_TwMgr->m_Bars.end()) {
			g_TwMgr->SetLastError(g_ErrNotFound);
			return 0;
		}
	}
	CTwVarGroup *varParent = nullptr;
	int varIndex = -1;
	CTwVar *var = nullptr;
	if (varName != nullptr && strlen(varName) > 0) {
		var = bar->Find(varName, &varParent, &varIndex);
		if (var == nullptr) {
			_snprintf(g_ErrParse, sizeof(g_ErrParse), "Unknown var '%s/%s'", (bar == TW_GLOBAL_BAR) ? "GLOBAL" : bar->m_Name.c_str(), varName);
			g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
			g_TwMgr->SetLastError(g_ErrParse);
			return 0;
		}
	}

	bool hasValue = false;
	int paramID = BarVarHasAttrib(bar, var, paramName, &hasValue);
	if (paramID > 0) {
		std::ostringstream valStr;
		std::vector<double> valDbl;
		const char *PrevLastErrorPtr = g_TwMgr->CheckLastError();

		ERetType retType = BarVarGetAttrib(bar, var, varParent, varIndex, paramID, valDbl, valStr);
		unsigned int i, valDblCount = (unsigned int)valDbl.size();
		if (valDblCount > outValueMaxCount)
			valDblCount = outValueMaxCount;
		if (retType == RET_DOUBLE && valDblCount == 0) {
			g_TwMgr->SetLastError(g_ErrHasNoValue);
			retType = RET_ERROR;
		}

		if (retType == RET_DOUBLE) {
			switch (paramValueType) {
				case TW_PARAM_INT32:
					for (i = 0; i < valDblCount; i++)
						(static_cast<int *>(outValues))[i] = (int)valDbl[i];
					return valDblCount;
				case TW_PARAM_FLOAT:
					for (i = 0; i < valDblCount; i++)
						(static_cast<float *>(outValues))[i] = (float)valDbl[i];
					return valDblCount;
				case TW_PARAM_DOUBLE:
					for (i = 0; i < valDblCount; i++)
						(static_cast<double *>(outValues))[i] = valDbl[i];
					return valDblCount;
				case TW_PARAM_CSTRING:
					valStr.clear();
					for (i = 0; i < (unsigned int)valDbl.size(); i++) // not valDblCount here
						valStr << ((i > 0) ? " " : "") << valDbl[i];
					strncpy(static_cast<char *>(outValues), valStr.str().c_str(), outValueMaxCount);
					i = (unsigned int)valStr.str().size();
					if (i > outValueMaxCount - 1)
						i = outValueMaxCount - 1;
					(static_cast<char *>(outValues))[i] = '\0';
					return 1; // always returns 1 for CSTRING
				default:
					g_TwMgr->SetLastError(g_ErrBadParam); // Unknown param value type
					retType = RET_ERROR;
			}
		} else if (retType == RET_STRING) {
			if (paramValueType == TW_PARAM_CSTRING) {
				strncpy(static_cast<char *>(outValues), valStr.str().c_str(), outValueMaxCount);
				i = (unsigned int)valStr.str().size();
				if (i > outValueMaxCount - 1)
					i = outValueMaxCount - 1;
				(static_cast<char *>(outValues))[i] = '\0';
				return 1; // always returns 1 for CSTRING
			} else {
				g_TwMgr->SetLastError(g_ErrBadType); // string cannot be converted to int or double
				retType = RET_ERROR;
			}
		}

		if (retType == RET_ERROR) {
			bool errMsg = (g_TwMgr->CheckLastError() != nullptr && strlen(g_TwMgr->CheckLastError()) > 0 && PrevLastErrorPtr != g_TwMgr->CheckLastError());
			_snprintf(g_ErrParse, sizeof(g_ErrParse), "Unable to get param '%s%s%s %s' %s%s",
					(bar == TW_GLOBAL_BAR) ? "GLOBAL" : bar->m_Name.c_str(), (var != nullptr) ? "/" : "",
					(var != nullptr) ? varName : "", paramName, errMsg ? " : " : "",
					errMsg ? g_TwMgr->CheckLastError() : "");
			g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
			g_TwMgr->SetLastError(g_ErrParse);
		}
		return retType;
	} else {
		_snprintf(g_ErrParse, sizeof(g_ErrParse), "Unknown param '%s%s%s %s'",
				(bar == TW_GLOBAL_BAR) ? "GLOBAL" : bar->m_Name.c_str(),
				(var != nullptr) ? "/" : "", (var != nullptr) ? varName : "", paramName);
		g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
		g_TwMgr->SetLastError(g_ErrParse);
		return 0;
	}
}

int ANT_CALL TwSetParam(TwBar *bar, const char *varName, const char *paramName, TwParamValueType paramValueType, unsigned int inValueCount, const void *inValues) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (paramName == nullptr || strlen(paramName) <= 0) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}
	if (inValueCount > 0 && inValues == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}

	TwFreeAsyncDrawing(); // For multi-thread savety

	if (bar == nullptr)
		bar = TW_GLOBAL_BAR;
	else {
		std::vector<TwBar *>::iterator barIt;
		int i = 0;
		for (barIt = g_TwMgr->m_Bars.begin(); barIt != g_TwMgr->m_Bars.end(); ++barIt, ++i)
			if ((*barIt) == bar)
				break;
		if (barIt == g_TwMgr->m_Bars.end()) {
			g_TwMgr->SetLastError(g_ErrNotFound);
			return 0;
		}
	}
	CTwVarGroup *varParent = nullptr;
	int varIndex = -1;
	CTwVar *var = nullptr;
	if (varName != nullptr && strlen(varName) > 0) {
		var = bar->Find(varName, &varParent, &varIndex);
		if (var == nullptr) {
			_snprintf(g_ErrParse, sizeof(g_ErrParse), "Unknown var '%s/%s'", (bar == TW_GLOBAL_BAR) ? "GLOBAL" : bar->m_Name.c_str(), varName);
			g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
			g_TwMgr->SetLastError(g_ErrParse);
			return 0;
		}
	}

	bool hasValue = false;
	int paramID = BarVarHasAttrib(bar, var, paramName, &hasValue);
	if (paramID > 0) {
		int ret = 0;
		const char *PrevLastErrorPtr = g_TwMgr->CheckLastError();
		if (hasValue) {
			std::ostringstream valuesStr;
			unsigned int i;
			switch (paramValueType) {
				case TW_PARAM_INT32:
					for (i = 0; i < inValueCount; i++)
						valuesStr << (static_cast<const int *>(inValues))[i] << ((i < inValueCount - 1) ? " " : "");
					break;
				case TW_PARAM_FLOAT:
					for (i = 0; i < inValueCount; i++)
						valuesStr << (static_cast<const float *>(inValues))[i] << ((i < inValueCount - 1) ? " " : "");
					break;
				case TW_PARAM_DOUBLE:
					for (i = 0; i < inValueCount; i++)
						valuesStr << (static_cast<const double *>(inValues))[i] << ((i < inValueCount - 1) ? " " : "");
					break;
				case TW_PARAM_CSTRING:
					if (inValueCount != 1) {
						g_TwMgr->SetLastError(g_ErrCStrParam); // count for CString param must be 1
						return 0;
					} else
						valuesStr << static_cast<const char *>(inValues);
					break;
				default:
					g_TwMgr->SetLastError(g_ErrBadParam); // Unknown param value type
					return 0;
			}
			ret = BarVarSetAttrib(bar, var, varParent, varIndex, paramID, valuesStr.str().c_str());
		} else
			ret = BarVarSetAttrib(bar, var, varParent, varIndex, paramID, nullptr);
		if (ret == 0) {
			bool errMsg = (g_TwMgr->CheckLastError() != nullptr && strlen(g_TwMgr->CheckLastError()) > 0 && PrevLastErrorPtr != g_TwMgr->CheckLastError());
			_snprintf(g_ErrParse, sizeof(g_ErrParse), "Unable to set param '%s%s%s %s' %s%s",
					(bar == TW_GLOBAL_BAR) ? "GLOBAL" : bar->m_Name.c_str(), (var != nullptr) ? "/" : "",
					(var != nullptr) ? varName : "", paramName, errMsg ? " : " : "",
					errMsg ? g_TwMgr->CheckLastError() : "");
			g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
			g_TwMgr->SetLastError(g_ErrParse);
		}
		return ret;
	} else {
		_snprintf(g_ErrParse, sizeof(g_ErrParse), "Unknown param '%s%s%s %s'",
				(bar == TW_GLOBAL_BAR) ? "GLOBAL" : bar->m_Name.c_str(),
				(var != nullptr) ? "/" : "", (var != nullptr) ? varName : "", paramName);
		g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
		g_TwMgr->SetLastError(g_ErrParse);
		return 0;
	}
}

//  ---------------------------------------------------------------------------

static int s_PassProxy = 0;
void *CTwMgr::CStruct::s_PassProxyAsClientData = &s_PassProxy; // special tag

CTwMgr::CStructProxy::CStructProxy() {
	memset(this, 0, sizeof(*this));
}

CTwMgr::CStructProxy::~CStructProxy() {
	if (m_StructData != NULL && m_DeleteStructData) {
		// if( m_StructExtData==NULL && g_TwMgr!=NULL && m_Type>=TW_TYPE_STRUCT_BASE && m_Type<TW_TYPE_STRUCT_BASE+(int)g_TwMgr->m_Structs.size() )
		//   g_TwMgr->UninitVarData(m_Type, m_StructData, g_TwMgr->m_Structs[m_Type-TW_TYPE_STRUCT_BASE].m_Size);
		delete[] (char *)m_StructData;
	}
	if (m_StructExtData != NULL) {
		// if( g_TwMgr!=NULL && m_Type>=TW_TYPE_STRUCT_BASE && m_Type<TW_TYPE_STRUCT_BASE+(int)g_TwMgr->m_Structs.size() )
		//   g_TwMgr->UninitVarData(m_Type, m_StructExtData, g_TwMgr->m_Structs[m_Type-TW_TYPE_STRUCT_BASE].m_Size);
		delete[] (char *)m_StructExtData;
	}
	memset(this, 0, sizeof(*this));
}

void CTwMgr::UnrollCDStdString(std::vector<CCDStdStringRecord> &_Records, TwType _Type, void *_Data) {
	if (_Data != NULL) {
		if (_Type >= TW_TYPE_STRUCT_BASE && _Type < TW_TYPE_STRUCT_BASE + (int)m_Structs.size()) {
			CTwMgr::CStruct &s = m_Structs[_Type - TW_TYPE_STRUCT_BASE];
			if (!s.m_IsExt)
				for (size_t i = 0; i < s.m_Members.size(); ++i) {
					CTwMgr::CStructMember &sm = s.m_Members[i];
					UnrollCDStdString(_Records, sm.m_Type, (char *)_Data + sm.m_Offset);
				}
			else {
				// nothing:
				// Ext struct cannot have var of type TW_TYPE_CDSTDSTRING (converted from TW_TYPE_STDSTRING)
			}
		} else if (_Type == TW_TYPE_STDSTRING || _Type == TW_TYPE_CDSTDSTRING) {
			_Records.push_back(CCDStdStringRecord());
			CCDStdStringRecord &Rec = _Records.back();
			Rec.m_DataPtr = _Data;
			memcpy(Rec.m_PrevValue, _Data, m_ClientStdStringStructSize);
			const char *Str = *(const char **)_Data;
			if (Str != NULL)
				// Rec.m_StdString = Str;
				Rec.m_ClientStdString.FromLib(Str);
			memcpy(Rec.m_DataPtr, &(Rec.m_ClientStdString.ToClient()), sizeof(std::string));
		}
	}
}

void CTwMgr::RestoreCDStdString(const std::vector<CCDStdStringRecord> &_Records) {
	for (size_t i = 0; i < _Records.size(); ++i)
		memcpy(_Records[i].m_DataPtr, _Records[i].m_PrevValue, m_ClientStdStringStructSize);
}

CTwMgr::CMemberProxy::CMemberProxy() { memset(this, 0, sizeof(*this)); }

CTwMgr::CMemberProxy::~CMemberProxy() { memset(this, 0, sizeof(*this)); }

void ANT_CALL CTwMgr::CMemberProxy::SetCB(const void *_Value, void *_ClientData) {
	if (_ClientData && _Value) {
		const CMemberProxy *mProxy = static_cast<const CMemberProxy *>(_ClientData);
		if (g_TwMgr && mProxy) {
			const CStructProxy *sProxy = mProxy->m_StructProxy;
			if (sProxy && sProxy->m_StructData && sProxy->m_Type >= TW_TYPE_STRUCT_BASE && sProxy->m_Type < TW_TYPE_STRUCT_BASE + (int)g_TwMgr->m_Structs.size()) {
				CTwMgr::CStruct &s = g_TwMgr->m_Structs[sProxy->m_Type - TW_TYPE_STRUCT_BASE];
				if (mProxy->m_MemberIndex >= 0 && mProxy->m_MemberIndex < (int)s.m_Members.size()) {
					CTwMgr::CStructMember &m = s.m_Members[mProxy->m_MemberIndex];
					if (m.m_Size > 0 && m.m_Type != TW_TYPE_BUTTON) {
						if (s.m_IsExt) {
							memcpy((char *)sProxy->m_StructExtData + m.m_Offset, _Value, m.m_Size);
							if (s.m_CopyVarFromExtCallback && sProxy->m_StructExtData)
								s.m_CopyVarFromExtCallback(sProxy->m_StructData, sProxy->m_StructExtData, mProxy->m_MemberIndex, (s.m_ExtClientData == s.s_PassProxyAsClientData) ? _ClientData : s.m_ExtClientData);
						} else
							memcpy((char *)sProxy->m_StructData + m.m_Offset, _Value, m.m_Size);
						if (sProxy->m_StructSetCallback) {
							g_TwMgr->m_CDStdStringRecords.resize(0);
							g_TwMgr->UnrollCDStdString(g_TwMgr->m_CDStdStringRecords, sProxy->m_Type, sProxy->m_StructData);
							sProxy->m_StructSetCallback(sProxy->m_StructData, sProxy->m_StructClientData);
							g_TwMgr->RestoreCDStdString(g_TwMgr->m_CDStdStringRecords);
						}
					}
				}
			}
		}
	}
}

void ANT_CALL CTwMgr::CMemberProxy::GetCB(void *_Value, void *_ClientData) {
	if (_ClientData && _Value) {
		const CMemberProxy *mProxy = static_cast<const CMemberProxy *>(_ClientData);
		if (g_TwMgr && mProxy) {
			const CStructProxy *sProxy = mProxy->m_StructProxy;
			if (sProxy && sProxy->m_StructData && sProxy->m_Type >= TW_TYPE_STRUCT_BASE && sProxy->m_Type < TW_TYPE_STRUCT_BASE + (int)g_TwMgr->m_Structs.size()) {
				CTwMgr::CStruct &s = g_TwMgr->m_Structs[sProxy->m_Type - TW_TYPE_STRUCT_BASE];
				if (mProxy->m_MemberIndex >= 0 && mProxy->m_MemberIndex < (int)s.m_Members.size()) {
					CTwMgr::CStructMember &m = s.m_Members[mProxy->m_MemberIndex];
					if (m.m_Size > 0 && m.m_Type != TW_TYPE_BUTTON) {
						if (sProxy->m_StructGetCallback)
							sProxy->m_StructGetCallback(sProxy->m_StructData, sProxy->m_StructClientData);
						if (s.m_IsExt) {
							if (s.m_CopyVarToExtCallback && sProxy->m_StructExtData)
								s.m_CopyVarToExtCallback(sProxy->m_StructData, sProxy->m_StructExtData, mProxy->m_MemberIndex, (s.m_ExtClientData == s.s_PassProxyAsClientData) ? _ClientData : s.m_ExtClientData);
							memcpy(_Value, (char *)sProxy->m_StructExtData + m.m_Offset, m.m_Size);
						} else
							memcpy(_Value, (char *)sProxy->m_StructData + m.m_Offset, m.m_Size);
					}
				}
			}
		}
	}
}

//  ---------------------------------------------------------------------------

void ANT_CALL CTwMgr::CCDStdString::SetCB(const void *_Value, void *_ClientData) {
	if (_Value == nullptr || _ClientData == nullptr || g_TwMgr == nullptr)
		return;
	CTwMgr::CCDStdString *CDStdString = (CTwMgr::CCDStdString *)_ClientData;
	const char *SrcStr = *(const char **)_Value;
	if (SrcStr == nullptr) {
		static char s_EmptyString[] = "";
		SrcStr = s_EmptyString;
	}
	if (CDStdString->m_ClientSetCallback == nullptr) {
		if (g_TwMgr->m_CopyStdStringToClient && CDStdString->m_ClientStdStringPtr != nullptr) {
			CTwMgr::CClientStdString clientSrcStr; // convert VC++ Release/Debug std::string
			clientSrcStr.FromLib(SrcStr);
			g_TwMgr->m_CopyStdStringToClient(*(CDStdString->m_ClientStdStringPtr), clientSrcStr.ToClient());
		}
	} else {
		if (CDStdString->m_ClientSetCallback == CMemberProxy::SetCB)
			CDStdString->m_ClientSetCallback(&SrcStr, CDStdString->m_ClientData);
		else {
			CTwMgr::CClientStdString clientSrcStr; // convert VC++ Release/Debug std::string
			clientSrcStr.FromLib(SrcStr);
			std::string &ValStr = clientSrcStr.ToClient();
			CDStdString->m_ClientSetCallback(&ValStr, CDStdString->m_ClientData);
		}
	}
}

void ANT_CALL CTwMgr::CCDStdString::GetCB(void *_Value, void *_ClientData) {
	if (_Value == nullptr || _ClientData == nullptr || g_TwMgr == nullptr)
		return;
	CTwMgr::CCDStdString *CDStdString = (CTwMgr::CCDStdString *)_ClientData;
	char **DstStrPtr = (char **)_Value;
	if (CDStdString->m_ClientGetCallback == nullptr) {
		if (CDStdString->m_ClientStdStringPtr != nullptr) {
			// *DstStrPtr = const_cast<char *>(CDStdString->m_ClientStdStringPtr->c_str());
			static CTwMgr::CLibStdString s_LibStr; // static because it will be used as a returned value
			s_LibStr.FromClient(*CDStdString->m_ClientStdStringPtr);
			*DstStrPtr = const_cast<char *>(s_LibStr.ToLib().c_str());
		} else {
			static char s_EmptyString[] = "";
			*DstStrPtr = s_EmptyString;
		}
	} else {
		// m_ClientGetCallback uses TwCopyStdStringToLibrary to copy string
		// and TwCopyStdStringToLibrary does the VC++ Debug/Release std::string conversion.
		CDStdString->m_ClientGetCallback(&(CDStdString->m_LocalString[0]), CDStdString->m_ClientData);
		//*DstStrPtr = const_cast<char *>(CDStdString->m_LocalString.c_str());
		char **StrPtr = (char **)&(CDStdString->m_LocalString[0]);
		*DstStrPtr = *StrPtr;
	}
}

//  ---------------------------------------------------------------------------

static int s_SeparatorTag = 0;

//  ---------------------------------------------------------------------------

static int AddVar(TwBar *_Bar, const char *_Name, ETwType _Type, void *_VarPtr, bool _ReadOnly, TwSetVarCallback _SetCallback, TwGetVarCallback _GetCallback, TwButtonCallback _ButtonCallback, void *_ClientData, const char *_Def) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr == NULL) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}

	char unnamedVarName[64];
	if (_Name == NULL || strlen(_Name) == 0) // create a name automatically
	{
		static unsigned int s_UnnamedVarCount = 0;
		_snprintf(unnamedVarName, sizeof(unnamedVarName), "TW_UNNAMED_%04X", s_UnnamedVarCount);
		_Name = unnamedVarName;
		++s_UnnamedVarCount;
	}

	if (_Bar == NULL || _Name == NULL || strlen(_Name) == 0 || (_VarPtr == NULL && _GetCallback == NULL && _Type != TW_TYPE_BUTTON)) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}
	if (_Bar->Find(_Name) != NULL) {
		g_TwMgr->SetLastError(g_ErrExist);
		return 0;
	}

	if (strstr(_Name, "`") != NULL) {
		g_TwMgr->SetLastError(g_ErrNoBackQuote);
		return 0;
	}

	if (_VarPtr == NULL && _Type != TW_TYPE_BUTTON && _GetCallback != NULL && _SetCallback == NULL)
		_ReadOnly = true; // force readonly in this case

	// Convert color types
	if (_Type == TW_TYPE_COLOR32)
		_Type = g_TwMgr->m_TypeColor32;
	else if (_Type == TW_TYPE_COLOR3F)
		_Type = g_TwMgr->m_TypeColor3F;
	else if (_Type == TW_TYPE_COLOR4F)
		_Type = g_TwMgr->m_TypeColor4F;

	// Convert rotation types
	if (_Type == TW_TYPE_QUAT4F)
		_Type = g_TwMgr->m_TypeQuat4F;
	else if (_Type == TW_TYPE_QUAT4D)
		_Type = g_TwMgr->m_TypeQuat4D;
	else if (_Type == TW_TYPE_DIR3F)
		_Type = g_TwMgr->m_TypeDir3F;
	else if (_Type == TW_TYPE_DIR3D)
		_Type = g_TwMgr->m_TypeDir3D;

	// VC++ uses a different definition of std::string in Debug and Release modes.
	// sizeof(std::string) is encoded in TW_TYPE_STDSTRING to overcome this issue.
	// With VS2010 the binary representation of std::string has changed too. This is
	// also detected here.
	if ((_Type & 0xffff0000) == (TW_TYPE_STDSTRING & 0xffff0000) || (_Type & 0xffff0000) == TW_TYPE_STDSTRING_VS2010 || (_Type & 0xffff0000) == TW_TYPE_STDSTRING_VS2008) {
		if (g_TwMgr->m_ClientStdStringBaseType == 0)
			g_TwMgr->m_ClientStdStringBaseType = (TwType)(_Type & 0xffff0000);

		size_t clientStdStringStructSize = (_Type & 0xffff);
		if (g_TwMgr->m_ClientStdStringStructSize == 0)
			g_TwMgr->m_ClientStdStringStructSize = clientStdStringStructSize;
		int diff = abs((int)g_TwMgr->m_ClientStdStringStructSize - (int)sizeof(std::string));
		if (g_TwMgr->m_ClientStdStringStructSize != clientStdStringStructSize || g_TwMgr->m_ClientStdStringStructSize == 0 || (diff != 0 && diff != sizeof(void *))) {
			g_TwMgr->SetLastError(g_ErrStdString);
			return 0;
		}

		_Type = TW_TYPE_STDSTRING; // force type to be our TW_TYPE_STDSTRING
	}

	if (_Type == TW_TYPE_STDSTRING) {
		g_TwMgr->m_CDStdStrings.push_back(CTwMgr::CCDStdString());
		CTwMgr::CCDStdString &CDStdString = g_TwMgr->m_CDStdStrings.back();
		CDStdString.m_ClientStdStringPtr = (std::string *)_VarPtr;
		CDStdString.m_ClientSetCallback = _SetCallback;
		CDStdString.m_ClientGetCallback = _GetCallback;
		CDStdString.m_ClientData = _ClientData;
		// CDStdString.m_This = g_TwMgr->m_CDStdStrings.end();
		// --CDStdString.m_This;
		TwGetVarCallback GetCB = CTwMgr::CCDStdString::GetCB;
		TwSetVarCallback SetCB = CTwMgr::CCDStdString::SetCB;
		if (_VarPtr == nullptr && _SetCallback == nullptr)
			SetCB = nullptr;
		if (_VarPtr == nullptr && _GetCallback == nullptr)
			GetCB = nullptr;
		return AddVar(_Bar, _Name, TW_TYPE_CDSTDSTRING, nullptr, _ReadOnly, SetCB, GetCB, nullptr, &CDStdString, _Def);
	} else if ((_Type > TW_TYPE_UNDEF && _Type < TW_TYPE_STRUCT_BASE) || (_Type >= TW_TYPE_ENUM_BASE && _Type < TW_TYPE_ENUM_BASE + (int)g_TwMgr->m_Enums.size()) || (_Type > TW_TYPE_CSSTRING_BASE && _Type <= TW_TYPE_CSSTRING_MAX) || _Type == TW_TYPE_CDSTDSTRING || IsCustomType(_Type)) // (_Type>=TW_TYPE_CUSTOM_BASE && _Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size()) )
	{
		CTwVarAtom *Var = new CTwVarAtom;
		Var->m_Name = _Name;
		Var->m_Ptr = _VarPtr;
		Var->m_Type = _Type;
		Var->m_ColorPtr = &(_Bar->m_ColLabelText);
		if (_VarPtr != nullptr) {
			DEV_ASSERT(_GetCallback == nullptr && _SetCallback == nullptr && _ButtonCallback == nullptr);

			Var->m_ReadOnly = _ReadOnly;
			Var->m_GetCallback = nullptr;
			Var->m_SetCallback = nullptr;
			Var->m_ClientData = nullptr;
		} else {
			DEV_ASSERT(_GetCallback != nullptr || _Type == TW_TYPE_BUTTON);

			Var->m_GetCallback = _GetCallback;
			Var->m_SetCallback = _SetCallback;
			Var->m_ClientData = _ClientData;
			if (_Type == TW_TYPE_BUTTON) {
				Var->m_Val.m_Button.m_Callback = _ButtonCallback;
				if (_ButtonCallback == nullptr && _ClientData == &s_SeparatorTag) {
					Var->m_Val.m_Button.m_Separator = 1;
					Var->m_Label = " ";
				} else if (_ButtonCallback == nullptr)
					Var->m_ColorPtr = &(_Bar->m_ColStaticText);
			}
			if (_Type != TW_TYPE_BUTTON)
				Var->m_ReadOnly = (_SetCallback == nullptr || _ReadOnly);
			else
				Var->m_ReadOnly = (_ButtonCallback == nullptr);
		}
		Var->SetDefaults();

		if (IsCustomType(_Type)) // _Type>=TW_TYPE_CUSTOM_BASE && _Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size() )
		{
			if (Var->m_GetCallback == CTwMgr::CMemberProxy::GetCB && Var->m_SetCallback == CTwMgr::CMemberProxy::SetCB)
				Var->m_Val.m_Custom.m_MemberProxy = static_cast<CTwMgr::CMemberProxy *>(Var->m_ClientData);
			else
				Var->m_Val.m_Custom.m_MemberProxy = nullptr;
		}

		_Bar->m_VarRoot.m_Vars.push_back(Var);
		_Bar->NotUpToDate();
		g_TwMgr->m_HelpBarNotUpToDate = true;

		if (_Def != nullptr && strlen(_Def) > 0) {
			std::string d = '`' + _Bar->m_Name + "`/`" + _Name + "` " + _Def;
			return TwDefine(d.c_str());
		} else
			return 1;
	} else if (_Type >= TW_TYPE_STRUCT_BASE && _Type < TW_TYPE_STRUCT_BASE + (TwType)g_TwMgr->m_Structs.size()) {
		CTwMgr::CStruct &s = g_TwMgr->m_Structs[_Type - TW_TYPE_STRUCT_BASE];
		CTwMgr::CStructProxy *sProxy = nullptr;
		void *vPtr;
		if (!s.m_IsExt) {
			if (_VarPtr != nullptr)
				vPtr = _VarPtr;
			else {
				DEV_ASSERT(_GetCallback != nullptr || _SetCallback != nullptr);
				DEV_ASSERT(s.m_Size > 0);
				vPtr = new char[s.m_Size];
				memset(vPtr, 0, s.m_Size);
				// create a new StructProxy
				g_TwMgr->m_StructProxies.push_back(CTwMgr::CStructProxy());
				sProxy = &(g_TwMgr->m_StructProxies.back());
				sProxy->m_Type = _Type;
				sProxy->m_StructData = vPtr;
				sProxy->m_DeleteStructData = true;
				sProxy->m_StructSetCallback = _SetCallback;
				sProxy->m_StructGetCallback = _GetCallback;
				sProxy->m_StructClientData = _ClientData;
				sProxy->m_CustomDrawCallback = nullptr;
				sProxy->m_CustomMouseButtonCallback = nullptr;
				sProxy->m_CustomMouseMotionCallback = nullptr;
				sProxy->m_CustomMouseLeaveCallback = nullptr;
				sProxy->m_CustomCaptureFocus = false;
				sProxy->m_CustomIndexFirst = -1;
				sProxy->m_CustomIndexLast = -1;
				//g_TwMgr->InitVarData(sProxy->m_Type, sProxy->m_StructData, s.m_Size);
			}
		} else { // s.m_IsExt
			assert(s.m_Size > 0 && s.m_ClientStructSize > 0);
			vPtr = new char[s.m_Size]; // will be m_StructExtData
			memset(vPtr, 0, s.m_Size);
			// create a new StructProxy
			g_TwMgr->m_StructProxies.push_back(CTwMgr::CStructProxy());
			sProxy = &(g_TwMgr->m_StructProxies.back());
			sProxy->m_Type = _Type;
			sProxy->m_StructExtData = vPtr;
			sProxy->m_StructSetCallback = _SetCallback;
			sProxy->m_StructGetCallback = _GetCallback;
			sProxy->m_StructClientData = _ClientData;
			sProxy->m_CustomDrawCallback = nullptr;
			sProxy->m_CustomMouseButtonCallback = nullptr;
			sProxy->m_CustomMouseMotionCallback = nullptr;
			sProxy->m_CustomMouseLeaveCallback = nullptr;
			sProxy->m_CustomCaptureFocus = false;
			sProxy->m_CustomIndexFirst = -1;
			sProxy->m_CustomIndexLast = -1;
			// g_TwMgr->InitVarData(sProxy->m_Type, sProxy->m_StructExtData, s.m_Size);
			if (_VarPtr != nullptr) {
				sProxy->m_StructData = _VarPtr;
				sProxy->m_DeleteStructData = false;
			} else {
				sProxy->m_StructData = new char[s.m_ClientStructSize];
				memset(sProxy->m_StructData, 0, s.m_ClientStructSize);
				sProxy->m_DeleteStructData = true;
				// g_TwMgr->InitVarData(ClientStructType, sProxy->m_StructData, s.m_ClientStructSize); //ClientStructType is unknown
			}
			_VarPtr = nullptr; // force use of TwAddVarCB for members

			// init m_StructExtdata
			if (s.m_ExtClientData == CTwMgr::CStruct::s_PassProxyAsClientData)
				s.m_StructExtInitCallback(sProxy->m_StructExtData, sProxy);
			else
				s.m_StructExtInitCallback(sProxy->m_StructExtData, s.m_ExtClientData);
		}

		for (int i = 0; i < (int)s.m_Members.size(); ++i) {
			CTwMgr::CStructMember &m = s.m_Members[i];
			std::string name = std::string(_Name) + '.' + m.m_Name;
			const char *access = "";
			if (_ReadOnly)
				access = "readonly ";
			std::string def = "label=`" + m.m_Name + "` group=`" + _Name + "` " + access; // + m.m_DefString;  // member def must be done after group def
			if (_VarPtr != nullptr) {
				if (TwAddVarRW(_Bar, name.c_str(), m.m_Type, (char *)vPtr + m.m_Offset, def.c_str()) == 0)
					return 0;
			} else {
				DEV_ASSERT(sProxy != nullptr);
				// create a new MemberProxy
				g_TwMgr->m_MemberProxies.push_back(CTwMgr::CMemberProxy());
				CTwMgr::CMemberProxy &mProxy = g_TwMgr->m_MemberProxies.back();
				mProxy.m_StructProxy = sProxy;
				mProxy.m_MemberIndex = i;
				DEV_ASSERT(!(s.m_IsExt && (m.m_Type == TW_TYPE_STDSTRING || m.m_Type == TW_TYPE_CDSTDSTRING))); // forbidden because this case is not handled by UnrollCDStdString
				if (TwAddVarCB(_Bar, name.c_str(), m.m_Type, CTwMgr::CMemberProxy::SetCB, CTwMgr::CMemberProxy::GetCB, &mProxy, def.c_str()) == 0)
					return 0;
				mProxy.m_Var = _Bar->Find(name.c_str(), &mProxy.m_VarParent, nullptr);
				mProxy.m_Bar = _Bar;
			}

			if (sProxy != nullptr && IsCustomType(m.m_Type)) // m.m_Type>=TW_TYPE_CUSTOM_BASE && m.m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size() )
			{
				if (sProxy->m_CustomIndexFirst < 0)
					sProxy->m_CustomIndexFirst = sProxy->m_CustomIndexLast = i;
				else
					sProxy->m_CustomIndexLast = i;
			}
		}
		char structInfo[64];
		snprintf(structInfo, 64, "typeid=%d valptr=%p close ", _Type, vPtr);
		std::string grpDef = '`' + _Bar->m_Name + "`/`" + _Name + "` " + structInfo;
		if (_Def != nullptr && strlen(_Def) > 0)
			grpDef += _Def;
		int ret = TwDefine(grpDef.c_str());
		for (int i = 0; i < (int)s.m_Members.size(); ++i) // members must be defined even if grpDef has error
		{
			CTwMgr::CStructMember &m = s.m_Members[i];
			if (m.m_DefString.length() > 0) {
				std::string memberDef = '`' + _Bar->m_Name + "`/`" + _Name + '.' + m.m_Name + "` " + m.m_DefString;
				if (!TwDefine(memberDef.c_str())) // all members must be defined even if memberDef has error
					ret = 0;
			}
		}
		return ret;
	} else {
		if (_Type == TW_TYPE_CSSTRING_BASE)
			g_TwMgr->SetLastError(g_ErrBadSize); // static string of size null
		else
			g_TwMgr->SetLastError(g_ErrNotFound);
		return 0;
	}
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwAddVarRW(TwBar *_Bar, const char *_Name, ETwType _Type, void *_Var, const char *_Def) {
	return AddVar(_Bar, _Name, _Type, _Var, false, nullptr, nullptr, nullptr, nullptr, _Def);
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwAddVarRO(TwBar *_Bar, const char *_Name, ETwType _Type, const void *_Var, const char *_Def) {
	return AddVar(_Bar, _Name, _Type, const_cast<void *>(_Var), true, nullptr, nullptr, nullptr, nullptr, _Def);
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwAddVarCB(TwBar *_Bar, const char *_Name, ETwType _Type, TwSetVarCallback _SetCallback, TwGetVarCallback _GetCallback, void *_ClientData, const char *_Def) {
	return AddVar(_Bar, _Name, _Type, nullptr, false, _SetCallback, _GetCallback, nullptr, _ClientData, _Def);
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwAddButton(TwBar *_Bar, const char *_Name, TwButtonCallback _Callback, void *_ClientData, const char *_Def) {
	return AddVar(_Bar, _Name, TW_TYPE_BUTTON, nullptr, false, nullptr, nullptr, _Callback, _ClientData, _Def);
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwAddSeparator(TwBar *_Bar, const char *_Name, const char *_Def) {
	return AddVar(_Bar, _Name, TW_TYPE_BUTTON, nullptr, true, nullptr, nullptr, nullptr, &s_SeparatorTag, _Def);
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwRemoveVar(TwBar *_Bar, const char *_Name) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (_Bar == nullptr || _Name == nullptr || strlen(_Name) == 0) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}

	if (g_TwMgr->m_PopupBar != nullptr && _Bar != g_TwMgr->m_PopupBar) // delete popup bar first if it exists
	{
		TwDeleteBar(g_TwMgr->m_PopupBar);
		g_TwMgr->m_PopupBar = nullptr;
	}

	_Bar->StopEditInPlace(); // desactivate EditInPlace

	CTwVarGroup *Parent = nullptr;
	int Index = -1;
	CTwVar *Var = _Bar->Find(_Name, &Parent, &Index);
	if (Var != nullptr && Parent != nullptr && Index >= 0) {
		if (Parent->m_StructValuePtr != nullptr) {
			g_TwMgr->SetLastError(g_ErrDelStruct);
			return 0;
		}

		delete Var;
		Parent->m_Vars.erase(Parent->m_Vars.begin() + Index);
		if (Parent != &(_Bar->m_VarRoot) && Parent->m_Vars.size() <= 0)
			TwRemoveVar(_Bar, Parent->m_Name.c_str());
		_Bar->NotUpToDate();
		if (_Bar != g_TwMgr->m_HelpBar)
			g_TwMgr->m_HelpBarNotUpToDate = true;
		return 1;
	}

	g_TwMgr->SetLastError(g_ErrNotFound);
	return 0;
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwRemoveAllVars(TwBar *_Bar) {
	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (_Bar == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}

	if (g_TwMgr->m_PopupBar != nullptr && _Bar != g_TwMgr->m_PopupBar && _Bar != g_TwMgr->m_HelpBar) // delete popup bar first if it exists
	{
		TwDeleteBar(g_TwMgr->m_PopupBar);
		g_TwMgr->m_PopupBar = nullptr;
	}

	_Bar->StopEditInPlace(); // desactivate EditInPlace

	for (std::vector<CTwVar *>::iterator it = _Bar->m_VarRoot.m_Vars.begin(); it != _Bar->m_VarRoot.m_Vars.end(); ++it)
		if (*it != nullptr) {
			delete *it;
			*it = nullptr;
		}
	_Bar->m_VarRoot.m_Vars.resize(0);
	_Bar->NotUpToDate();
	g_TwMgr->m_HelpBarNotUpToDate = true;
	return 1;
}

//  ---------------------------------------------------------------------------

int ParseToken(std::string &_Token, const char *_Def, int &Line, int &Column, bool _KeepQuotes, bool _EndCR, char _Sep1 = '\0', char _Sep2 = '\0') {
	const char *Cur = _Def;
	_Token = "";
	// skip spaces
	while (*Cur == ' ' || *Cur == '\t' || *Cur == '\r' || *Cur == '\n') {
		if (*Cur == '\n' && _EndCR)
			return (int)(Cur - _Def); // a CR has been found
		++Cur;
		if (*Cur == '\n') {
			++Line;
			Column = 1;
		} else if (*Cur == '\t')
			Column += g_TabLength;
		else if (*Cur != '\r')
			++Column;
	}
	// read token
	int QuoteLine = 0, QuoteColumn = 0;
	char Quote = 0;
	bool AddChar;
	bool LineJustIncremented = false;
	while ((Quote == 0 && (*Cur != '\0' && *Cur != ' ' && *Cur != '\t' && *Cur != '\r' && *Cur != '\n' && *Cur != _Sep1 && *Cur != _Sep2)) || (Quote != 0 && (*Cur != '\0' /* && *Cur!='\r' && *Cur!='\n' */))) // allow multi-line strings
	{
		LineJustIncremented = false;
		AddChar = true;
		if (Quote == 0 && (*Cur == '\'' || *Cur == '\"' || *Cur == '`')) {
			Quote = *Cur;
			QuoteLine = Line;
			QuoteColumn = Column;
			AddChar = _KeepQuotes;
		} else if (Quote != 0 && *Cur == Quote) {
			Quote = 0;
			AddChar = _KeepQuotes;
		}

		if (AddChar)
			_Token += *Cur;
		++Cur;
		if (*Cur == '\t')
			Column += g_TabLength;
		else if (*Cur == '\n') {
			++Line;
			LineJustIncremented = true;
			Column = 1;
		} else
			++Column;
	}

	if (Quote != 0) {
		Line = QuoteLine;
		Column = QuoteColumn;
		return -(int)(Cur - _Def); // unclosed quote
	} else {
		if (*Cur == '\n') {
			if (!LineJustIncremented)
				++Line;
			Column = 1;
		} else if (*Cur == '\t')
			Column += g_TabLength;
		else if (*Cur != '\r' && *Cur != '\0')
			++Column;
		return (int)(Cur - _Def);
	}
}

//  ---------------------------------------------------------------------------

int GetBarVarFromString(CTwBar **_Bar, CTwVar **_Var, CTwVarGroup **_VarParent, int *_VarIndex, const char *_Str) {
	*_Bar = nullptr;
	*_Var = nullptr;
	*_VarParent = nullptr;
	*_VarIndex = -1;
	std::vector<std::string> Names;
	std::string Token;
	const char *Cur = _Str;
	int l = 1, c = 1, p = 1;
	while (*Cur != '\0' && p > 0 && Names.size() <= 3) {
		p = ParseToken(Token, Cur, l, c, false, true, '/', '\\');
		if (p > 0 && Token.size() > 0) {
			Names.push_back(Token);
			Cur += p + ((Cur[p] != '\0') ? 1 : 0);
		}
	}
	if (p <= 0 || (Names.size() != 1 && Names.size() != 2))
		return 0; // parse error
	int BarIdx = g_TwMgr->FindBar(Names[0].c_str());
	if (BarIdx < 0) {
		if (Names.size() == 1 && strcmp(Names[0].c_str(), "GLOBAL") == 0) {
			*_Bar = TW_GLOBAL_BAR;
			return +3; // 'GLOBAL' found
		} else
			return -1; // bar not found
	}
	*_Bar = g_TwMgr->m_Bars[BarIdx];
	if (Names.size() == 1)
		return 1; // bar found, no var name parsed
	*_Var = (*_Bar)->Find(Names[1].c_str(), _VarParent, _VarIndex);
	if (*_Var == nullptr)
		return -2; // var not found
	return 2; // bar and var found
}

int BarVarHasAttrib(CTwBar *_Bar, CTwVar *_Var, const char *_Attrib, bool *_HasValue) {
	DEV_ASSERT(_Bar != nullptr && _HasValue != nullptr && _Attrib != nullptr && strlen(_Attrib) > 0);
	*_HasValue = false;
	if (_Bar == TW_GLOBAL_BAR) {
		DEV_ASSERT(_Var == nullptr);
		return g_TwMgr->HasAttrib(_Attrib, _HasValue);
	} else if (_Var == nullptr)
		return _Bar->HasAttrib(_Attrib, _HasValue);
	else
		return _Var->HasAttrib(_Attrib, _HasValue);
}

int BarVarSetAttrib(CTwBar *_Bar, CTwVar *_Var, CTwVarGroup *_VarParent, int _VarIndex, int _AttribID, const char *_Value) {
	DEV_ASSERT(_Bar != nullptr && _AttribID > 0);

	if (_Bar == TW_GLOBAL_BAR) {
		DEV_ASSERT(_Var == nullptr);
		return g_TwMgr->SetAttrib(_AttribID, _Value);
	} else if (_Var == nullptr)
		return _Bar->SetAttrib(_AttribID, _Value);
	else
		return _Var->SetAttrib(_AttribID, _Value, _Bar, _VarParent, _VarIndex);
	// don't make _Bar not-up-to-date here, should be done in SetAttrib if needed to avoid too frequent refreshs
}

ERetType BarVarGetAttrib(CTwBar *_Bar, CTwVar *_Var, CTwVarGroup *_VarParent, int _VarIndex, int _AttribID, std::vector<double> &outDoubles, std::ostringstream &outString) {
	DEV_ASSERT(_Bar != nullptr && _AttribID > 0);

	if (_Bar == TW_GLOBAL_BAR) {
		DEV_ASSERT(_Var == nullptr);
		return g_TwMgr->GetAttrib(_AttribID, outDoubles, outString);
	} else if (_Var == nullptr)
		return _Bar->GetAttrib(_AttribID, outDoubles, outString);
	else
		return _Var->GetAttrib(_AttribID, _Bar, _VarParent, _VarIndex, outDoubles, outString);
}

//  ---------------------------------------------------------------------------

static _FORCE_INLINE_ std::string ErrorPosition(bool _MultiLine, int _Line, int _Column) {
	if (!_MultiLine)
		return "";
	else {
		char pos[32];
		//_snprintf(pos, sizeof(pos)-1, " line %d column %d", _Line, _Column);
		_snprintf(pos, sizeof(pos) - 1, " line %d", _Line);
		(void)_Column;
		pos[sizeof(pos) - 1] = '\0';
		return pos;
	}
}

//  ---------------------------------------------------------------------------

int ANT_CALL TwDefine(const char *_Def) {
	CTwFPU fpu; // force fpu precision

	// hack to scale fonts artificially (for retina display for instance)
	if (g_TwMgr == nullptr && _Def != nullptr) {
		size_t l = strlen(_Def);
		const char *eq = strchr(_Def, '=');
		if (eq != nullptr && eq != _Def && l > 0 && l < 512) {
			char *a = new char[l + 1];
			char *b = new char[l + 1];
			if (sscanf(_Def, "%s%s", a, b) == 2 && strcmp(a, "GLOBAL") == 0) {
				if (strchr(b, '=') != nullptr)
					*strchr(b, '=') = '\0';
				double scal = 1.0;
				if (_stricmp(b, "fontscaling") == 0 && sscanf(eq + 1, "%lf", &scal) == 1 && scal > 0) {
					g_FontScaling = (float)scal;
					delete[] a;
					delete[] b;
					return 1;
				}
			}
			delete[] a;
			delete[] b;
		}
	}

	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return 0; // not initialized
	}
	if (_Def == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return 0;
	}

	bool MultiLine = false;
	const char *Cur = _Def;
	while (*Cur != '\0') {
		if (*Cur == '\n') {
			MultiLine = true;
			break;
		}
		++Cur;
	}

	int Line = 1;
	int Column = 1;
	enum EState { PARSE_NAME,
		PARSE_ATTRIB };
	EState State = PARSE_NAME;
	std::string Token;
	std::string Value;
	CTwBar *Bar = nullptr;
	CTwVar *Var = nullptr;
	CTwVarGroup *VarParent = nullptr;
	int VarIndex = -1;
	int p;

	Cur = _Def;
	while (*Cur != '\0') {
		const char *PrevCur = Cur;
		p = ParseToken(Token, Cur, Line, Column, (State == PARSE_NAME), (State == PARSE_ATTRIB), (State == PARSE_ATTRIB) ? '=' : '\0');
		if (p <= 0 || Token.size() <= 0) {
			if (p > 0 && Cur[p] == '\0') {
				Cur += p;
				continue;
			}
			_snprintf(g_ErrParse, sizeof(g_ErrParse), "Parsing error in def string%s [%-16s...]", ErrorPosition(MultiLine, Line, Column).c_str(), (p < 0) ? (Cur - p) : PrevCur);
			g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
			g_TwMgr->SetLastError(g_ErrParse);
			return 0;
		}
		char CurSep = Cur[p];
		Cur += p + ((CurSep != '\0') ? 1 : 0);

		if (State == PARSE_NAME) {
			int Err = GetBarVarFromString(&Bar, &Var, &VarParent, &VarIndex, Token.c_str());
			if (Err <= 0) {
				if (Err == -1)
					_snprintf(g_ErrParse, sizeof(g_ErrParse), "Parsing error in def string: Bar not found%s [%-16s...]", ErrorPosition(MultiLine, Line, Column).c_str(), Token.c_str());
				else if (Err == -2)
					_snprintf(g_ErrParse, sizeof(g_ErrParse), "Parsing error in def string: Variable not found%s [%-16s...]", ErrorPosition(MultiLine, Line, Column).c_str(), Token.c_str());
				else
					_snprintf(g_ErrParse, sizeof(g_ErrParse), "Parsing error in def string%s [%-16s...]", ErrorPosition(MultiLine, Line, Column).c_str(), Token.c_str());
				g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
				g_TwMgr->SetLastError(g_ErrParse);
				return 0;
			}
			State = PARSE_ATTRIB;
		} else { // State==PARSE_ATTRIB
			DEV_ASSERT(State == PARSE_ATTRIB);
			DEV_ASSERT(Bar != nullptr);

			bool HasValue = false;
			Value = "";
			int AttribID = BarVarHasAttrib(Bar, Var, Token.c_str(), &HasValue);
			if (AttribID <= 0) {
				_snprintf(g_ErrParse, sizeof(g_ErrParse), "Parsing error in def string: Unknown attribute%s [%-16s...]", ErrorPosition(MultiLine, Line, Column).c_str(), Token.c_str());
				g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
				g_TwMgr->SetLastError(g_ErrParse);
				return 0;
			}

			// special case for backward compatibility
			if (HasValue && (_stricmp(Token.c_str(), "readonly") == 0 || _stricmp(Token.c_str(), "hexa") == 0)) {
				if (CurSep == ' ' || CurSep == '\t') {
					const char *ch = Cur;
					while (*ch == ' ' || *ch == '\t') // find next non-space character
						++ch;
					if (*ch != '=') // if this is not '=' the param has no value
						HasValue = false;
				}
			}

			if (HasValue) {
				if (CurSep != '=') {
					std::string EqualStr;
					p = ParseToken(EqualStr, Cur, Line, Column, true, true, '=');
					CurSep = Cur[p];
					if (p < 0 || EqualStr.size() > 0 || CurSep != '=') {
						_snprintf(g_ErrParse, sizeof(g_ErrParse), "Parsing error in def string: '=' not found while reading attribute value%s [%-16s...]", ErrorPosition(MultiLine, Line, Column).c_str(), Token.c_str());
						g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
						g_TwMgr->SetLastError(g_ErrParse);
						return 0;
					}
					Cur += p + 1;
				}
				p = ParseToken(Value, Cur, Line, Column, false, true);
				if (p <= 0) {
					_snprintf(g_ErrParse, sizeof(g_ErrParse), "Parsing error in def string: can't read attribute value%s [%-16s...]", ErrorPosition(MultiLine, Line, Column).c_str(), Token.c_str());
					g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
					g_TwMgr->SetLastError(g_ErrParse);
					return 0;
				}
				CurSep = Cur[p];
				Cur += p + ((CurSep != '\0') ? 1 : 0);
			}
			const char *PrevLastErrorPtr = g_TwMgr->CheckLastError();
			if (BarVarSetAttrib(Bar, Var, VarParent, VarIndex, AttribID, HasValue ? Value.c_str() : NULL) == 0) {
				if (g_TwMgr->CheckLastError() == NULL || strlen(g_TwMgr->CheckLastError()) <= 0 || g_TwMgr->CheckLastError() == PrevLastErrorPtr)
					_snprintf(g_ErrParse, sizeof(g_ErrParse), "Parsing error in def string: wrong attribute value%s [%-16s...]", ErrorPosition(MultiLine, Line, Column).c_str(), Token.c_str());
				else
					_snprintf(g_ErrParse, sizeof(g_ErrParse), "%s%s [%-16s...]", g_TwMgr->CheckLastError(), ErrorPosition(MultiLine, Line, Column).c_str(), Token.c_str());
				g_ErrParse[sizeof(g_ErrParse) - 1] = '\0';
				g_TwMgr->SetLastError(g_ErrParse);
				return 0;
			}
			// sweep spaces to detect next attrib
			while (*Cur == ' ' || *Cur == '\t' || *Cur == '\r') {
				++Cur;
				if (*Cur == '\t')
					Column += g_TabLength;
				else if (*Cur != '\r')
					++Column;
			}
			if (*Cur == '\n') // new line detected
			{
				++Line;
				Column = 1;
				State = PARSE_NAME;
			}
		}
	}

	g_TwMgr->m_HelpBarNotUpToDate = true;
	return 1;
}

//  ---------------------------------------------------------------------------

TwType ANT_CALL TwDefineEnum(const char *_Name, const TwEnumVal *_EnumValues, unsigned int _NbValues) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return TW_TYPE_UNDEF; // not initialized
	}
	if (_EnumValues == nullptr && _NbValues != 0) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return TW_TYPE_UNDEF;
	}

	if (g_TwMgr->m_PopupBar != nullptr) // delete popup bar first if it exists
	{
		TwDeleteBar(g_TwMgr->m_PopupBar);
		g_TwMgr->m_PopupBar = nullptr;
	}

	size_t enumIndex = g_TwMgr->m_Enums.size();
	if (_Name != nullptr && strlen(_Name) > 0)
		for (size_t j = 0; j < g_TwMgr->m_Enums.size(); ++j)
			if (strcmp(_Name, g_TwMgr->m_Enums[j].m_Name.c_str()) == 0) {
				enumIndex = j;
				break;
			}
	if (enumIndex == g_TwMgr->m_Enums.size())
		g_TwMgr->m_Enums.push_back(CTwMgr::CEnum());
	DEV_ASSERT(enumIndex >= 0 && enumIndex < g_TwMgr->m_Enums.size());
	CTwMgr::CEnum &e = g_TwMgr->m_Enums[enumIndex];
	if (_Name != nullptr && strlen(_Name) > 0)
		e.m_Name = _Name;
	else
		e.m_Name = "";
	e.m_Entries.clear();
	for (unsigned int i = 0; i < _NbValues; ++i) {
		CTwMgr::CEnum::CEntries::value_type Entry(_EnumValues[i].Value, (_EnumValues[i].Label != nullptr) ? _EnumValues[i].Label : "");
		std::pair<CTwMgr::CEnum::CEntries::iterator, bool> Result = e.m_Entries.insert(Entry);
		if (!Result.second)
			(Result.first)->second = Entry.second;
	}

	return TwType(TW_TYPE_ENUM_BASE + enumIndex);
}

//  ---------------------------------------------------------------------------

TwType TW_CALL TwDefineEnumFromString(const char *_Name, const char *_EnumString) {
	if (_EnumString == nullptr)
		return TwDefineEnum(_Name, nullptr, 0);

	// split enumString
	std::stringstream EnumStream(_EnumString);
	std::string Label;
	std::vector<std::string> Labels;
	while (getline(EnumStream, Label, ',')) {
		// trim Label
		size_t Start = Label.find_first_not_of(" \n\r\t");
		size_t End = Label.find_last_not_of(" \n\r\t");
		if (Start == std::string::npos || End == std::string::npos)
			Label = "";
		else
			Label = Label.substr(Start, (End - Start) + 1);
		// store Label
		Labels.push_back(Label);
	}
	// create TwEnumVal array
	std::vector<TwEnumVal> Vals(Labels.size());
	for (int i = 0; i < (int)Labels.size(); i++) {
		Vals[i].Value = i;
		Vals[i].Label = Labels[i].c_str();
	}

	return TwDefineEnum(_Name, Vals.empty() ? nullptr : &(Vals[0]), (unsigned int)Vals.size());
}

//  ---------------------------------------------------------------------------

void ANT_CALL CTwMgr::CStruct::DefaultSummary(char *_SummaryString, size_t _SummaryMaxLength, const void *_Value, void *_ClientData) {
	const CTwVarGroup *varGroup = static_cast<const CTwVarGroup *>(_Value); // special case
	if (_SummaryString && _SummaryMaxLength > 0)
		_SummaryString[0] = '\0';
	size_t structIndex = (size_t)(_ClientData);
	if (g_TwMgr && _SummaryString && _SummaryMaxLength > 2 && varGroup && static_cast<const CTwVar *>(varGroup)->IsGroup() && structIndex >= 0 && structIndex <= g_TwMgr->m_Structs.size()) {
		// return g_TwMgr->m_Structs[structIndex].m_Name.c_str();
		CTwMgr::CStruct &s = g_TwMgr->m_Structs[structIndex];
		_SummaryString[0] = '{';
		_SummaryString[1] = '\0';
		bool separator = false;
		for (size_t i = 0; i < s.m_Members.size(); ++i) {
			std::string varName = varGroup->m_Name + '.' + s.m_Members[i].m_Name;
			const CTwVar *var = varGroup->Find(varName.c_str(), nullptr, nullptr);
			if (var) {
				if (var->IsGroup()) {
					const CTwVarGroup *grp = static_cast<const CTwVarGroup *>(var);
					if (grp->m_SummaryCallback != nullptr) {
						size_t l = strlen(_SummaryString);
						if (separator) {
							_SummaryString[l++] = ',';
							_SummaryString[l++] = '\0';
						}
						if (grp->m_SummaryCallback == CTwMgr::CStruct::DefaultSummary)
							grp->m_SummaryCallback(_SummaryString + l, _SummaryMaxLength - l, grp, grp->m_SummaryClientData);
						else
							grp->m_SummaryCallback(_SummaryString + l, _SummaryMaxLength - l, grp->m_StructValuePtr, grp->m_SummaryClientData);
						separator = true;
					}
				} else {
					size_t l = strlen(_SummaryString);
					if (separator) {
						_SummaryString[l++] = ',';
						_SummaryString[l++] = '\0';
					}
					std::string valString;
					const CTwVarAtom *atom = static_cast<const CTwVarAtom *>(var);
					atom->ValueToString(&valString);
					if (atom->m_Type == TW_TYPE_BOOLCPP || atom->m_Type == TW_TYPE_BOOL8 || atom->m_Type == TW_TYPE_BOOL16 || atom->m_Type == TW_TYPE_BOOL32) {
						if (valString == "0")
							valString = "-";
						else if (valString == "1")
							valString = "\x7f"; // check sign
					}
					strncat(_SummaryString, valString.c_str(), _SummaryMaxLength - l);
					separator = true;
				}
				if (strlen(_SummaryString) > _SummaryMaxLength - 2)
					break;
			}
		}
		size_t l = strlen(_SummaryString);
		if (l > _SummaryMaxLength - 2) {
			_SummaryString[_SummaryMaxLength - 2] = '.';
			_SummaryString[_SummaryMaxLength - 1] = '.';
			_SummaryString[_SummaryMaxLength + 0] = '\0';
		} else {
			_SummaryString[l + 0] = '}';
			_SummaryString[l + 1] = '\0';
		}
	}
}

//  ---------------------------------------------------------------------------

TwType ANT_CALL TwDefineStruct(const char *_StructName, const TwStructMember *_StructMembers, unsigned int _NbMembers, size_t _StructSize, TwSummaryCallback _SummaryCallback, void *_SummaryClientData) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return TW_TYPE_UNDEF; // not initialized
	}
	if (_StructMembers == nullptr || _NbMembers == 0 || _StructSize == 0) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return TW_TYPE_UNDEF;
	}

	if (_StructName != nullptr && strlen(_StructName) > 0)
		for (size_t j = 0; j < g_TwMgr->m_Structs.size(); ++j)
			if (strcmp(_StructName, g_TwMgr->m_Structs[j].m_Name.c_str()) == 0) {
				g_TwMgr->SetLastError(g_ErrExist);
				return TW_TYPE_UNDEF;
			}

	size_t structIndex = g_TwMgr->m_Structs.size();
	CTwMgr::CStruct s;
	s.m_Size = _StructSize;
	if (_StructName != nullptr && strlen(_StructName) > 0)
		s.m_Name = _StructName;
	else
		s.m_Name = "";
	s.m_Members.resize(_NbMembers);
	if (_SummaryCallback != nullptr) {
		s.m_SummaryCallback = _SummaryCallback;
		s.m_SummaryClientData = _SummaryClientData;
	} else {
		s.m_SummaryCallback = CTwMgr::CStruct::DefaultSummary;
		s.m_SummaryClientData = (void *)(structIndex);
	}
	for (unsigned int i = 0; i < _NbMembers; ++i) {
		CTwMgr::CStructMember &m = s.m_Members[i];
		if (_StructMembers[i].Name != nullptr)
			m.m_Name = _StructMembers[i].Name;
		else {
			char name[16];
			snprintf(name, 16, "%u", i);
			m.m_Name = name;
		}
		m.m_Type = _StructMembers[i].Type;
		m.m_Size = 0; // to avoid endless recursivity in GetDataSize
		m.m_Size = CTwVar::GetDataSize(m.m_Type);
		if (_StructMembers[i].Offset < _StructSize)
			m.m_Offset = _StructMembers[i].Offset;
		else {
			g_TwMgr->SetLastError(g_ErrOffset);
			return TW_TYPE_UNDEF;
		}
		if (_StructMembers[i].DefString != nullptr && strlen(_StructMembers[i].DefString) > 0)
			m.m_DefString = _StructMembers[i].DefString;
		else
			m.m_DefString = "";
	}

	g_TwMgr->m_Structs.push_back(s);
	DEV_ASSERT(g_TwMgr->m_Structs.size() == structIndex + 1);
	return TwType(TW_TYPE_STRUCT_BASE + structIndex);
}

//  ---------------------------------------------------------------------------

TwType ANT_CALL TwDefineStructExt(const char *_StructName, const TwStructMember *_StructExtMembers, unsigned int _NbExtMembers, size_t _StructSize, size_t _StructExtSize, TwStructExtInitCallback _StructExtInitCallback, TwCopyVarFromExtCallback _CopyVarFromExtCallback, TwCopyVarToExtCallback _CopyVarToExtCallback, TwSummaryCallback _SummaryCallback, void *_ClientData, const char *_Help) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr == nullptr) {
		TwGlobalError(g_ErrNotInit);
		return TW_TYPE_UNDEF; // not initialized
	}
	if (_StructSize == 0 || _StructExtInitCallback == nullptr || _CopyVarFromExtCallback == nullptr || _CopyVarToExtCallback == nullptr) {
		g_TwMgr->SetLastError(g_ErrBadParam);
		return TW_TYPE_UNDEF;
	}
	TwType type = TwDefineStruct(_StructName, _StructExtMembers, _NbExtMembers, _StructExtSize, _SummaryCallback, _ClientData);
	if (type >= TW_TYPE_STRUCT_BASE && type < TW_TYPE_STRUCT_BASE + (int)g_TwMgr->m_Structs.size()) {
		CTwMgr::CStruct &s = g_TwMgr->m_Structs[type - TW_TYPE_STRUCT_BASE];
		s.m_IsExt = true;
		s.m_ClientStructSize = _StructSize;
		s.m_StructExtInitCallback = _StructExtInitCallback;
		s.m_CopyVarFromExtCallback = _CopyVarFromExtCallback;
		s.m_CopyVarToExtCallback = _CopyVarToExtCallback;
		s.m_ExtClientData = _ClientData;
		if (_Help != nullptr)
			s.m_Help = _Help;
	}
	return type;
}

//  ---------------------------------------------------------------------------

bool TwGetKeyCode(int *_Code, int *_Modif, const char *_String) {
	DEV_ASSERT(_Code != nullptr && _Modif != nullptr);
	bool Ok = true;
	*_Modif = TW_KMOD_NONE;
	*_Code = 0;
	size_t Start = strlen(_String) - 1;
	if (Start < 0)
		return false;
	while (Start > 0 && _String[Start - 1] != '+')
		--Start;
	while (_String[Start] == ' ' || _String[Start] == '\t')
		++Start;
	char *CodeStr = _strdup(_String + Start);
	for (size_t i = strlen(CodeStr) - 1; i >= 0; ++i)
		if (CodeStr[i] == ' ' || CodeStr[i] == '\t')
			CodeStr[i] = '\0';
		else
			break;

	char *up = _strdup(_String); // _strupr(up);
	for (char *upch = up; *upch != '\0'; ++upch)
		*upch = (char)toupper(*upch);
	if (strstr(up, "SHIFT") != NULL)
		*_Modif |= TW_KMOD_SHIFT;
	if (strstr(up, "CTRL") != NULL)
		*_Modif |= TW_KMOD_CTRL;
	if (strstr(up, "META") != NULL)
		*_Modif |= TW_KMOD_META;

	if (strstr(up, "ALTGR") != NULL)
		((void)(0)); // *_Modif |= TW_KMOD_ALTGR;
	else // ALT and ALTGR are exclusive
		if (strstr(up, "ALT") != NULL)
			*_Modif |= TW_KMOD_ALT;
	free(up);

	if (strlen(CodeStr) == 1)
		*_Code = (unsigned char)(CodeStr[0]);
	else if (_stricmp(CodeStr, "backspace") == 0 || _stricmp(CodeStr, "bs") == 0)
		*_Code = TW_KEY_BACKSPACE;
	else if (_stricmp(CodeStr, "tab") == 0)
		*_Code = TW_KEY_TAB;
	else if (_stricmp(CodeStr, "clear") == 0 || _stricmp(CodeStr, "clr") == 0)
		*_Code = TW_KEY_CLEAR;
	else if (_stricmp(CodeStr, "return") == 0 || _stricmp(CodeStr, "ret") == 0)
		*_Code = TW_KEY_RETURN;
	else if (_stricmp(CodeStr, "pause") == 0)
		*_Code = TW_KEY_PAUSE;
	else if (_stricmp(CodeStr, "escape") == 0 || _stricmp(CodeStr, "esc") == 0)
		*_Code = TW_KEY_ESCAPE;
	else if (_stricmp(CodeStr, "space") == 0)
		*_Code = TW_KEY_SPACE;
	else if (_stricmp(CodeStr, "delete") == 0 || _stricmp(CodeStr, "del") == 0)
		*_Code = TW_KEY_DELETE;
	else if (_stricmp(CodeStr, "up") == 0)
		*_Code = TW_KEY_UP;
	else if (_stricmp(CodeStr, "down") == 0)
		*_Code = TW_KEY_DOWN;
	else if (_stricmp(CodeStr, "right") == 0)
		*_Code = TW_KEY_RIGHT;
	else if (_stricmp(CodeStr, "left") == 0)
		*_Code = TW_KEY_LEFT;
	else if (_stricmp(CodeStr, "insert") == 0 || _stricmp(CodeStr, "ins") == 0)
		*_Code = TW_KEY_INSERT;
	else if (_stricmp(CodeStr, "home") == 0)
		*_Code = TW_KEY_HOME;
	else if (_stricmp(CodeStr, "end") == 0)
		*_Code = TW_KEY_END;
	else if (_stricmp(CodeStr, "pgup") == 0)
		*_Code = TW_KEY_PAGE_UP;
	else if (_stricmp(CodeStr, "pgdown") == 0)
		*_Code = TW_KEY_PAGE_DOWN;
	else if ((strlen(CodeStr) == 2 || strlen(CodeStr) == 3) && (CodeStr[0] == 'f' || CodeStr[0] == 'F')) {
		int n = 0;
		if (sscanf(CodeStr + 1, "%d", &n) == 1 && n > 0 && n < 16)
			*_Code = TW_KEY_F1 + n - 1;
		else
			Ok = false;
	}

	free(CodeStr);
	return Ok;
}

bool TwGetKeyString(std::string *_String, int _Code, int _Modif) {
	assert(_String != NULL);
	bool Ok = true;
	if (_Modif & TW_KMOD_SHIFT)
		*_String += "SHIFT+";
	if (_Modif & TW_KMOD_CTRL)
		*_String += "CTRL+";
	if (_Modif & TW_KMOD_ALT)
		*_String += "ALT+";
	if (_Modif & TW_KMOD_META)
		*_String += "META+";
	// if ( _Modif & TW_KMOD_ALTGR )
	//  *_String += "ALTGR+";
	switch (_Code) {
		case TW_KEY_BACKSPACE:
			*_String += "BackSpace";
			break;
		case TW_KEY_TAB:
			*_String += "Tab";
			break;
		case TW_KEY_CLEAR:
			*_String += "Clear";
			break;
		case TW_KEY_RETURN:
			*_String += "Return";
			break;
		case TW_KEY_PAUSE:
			*_String += "Pause";
			break;
		case TW_KEY_ESCAPE:
			*_String += "Esc";
			break;
		case TW_KEY_SPACE:
			*_String += "Space";
			break;
		case TW_KEY_DELETE:
			*_String += "Delete";
			break;
		case TW_KEY_UP:
			*_String += "Up";
			break;
		case TW_KEY_DOWN:
			*_String += "Down";
			break;
		case TW_KEY_RIGHT:
			*_String += "Right";
			break;
		case TW_KEY_LEFT:
			*_String += "Left";
			break;
		case TW_KEY_INSERT:
			*_String += "Insert";
			break;
		case TW_KEY_HOME:
			*_String += "Home";
			break;
		case TW_KEY_END:
			*_String += "End";
			break;
		case TW_KEY_PAGE_UP:
			*_String += "PgUp";
			break;
		case TW_KEY_PAGE_DOWN:
			*_String += "PgDown";
			break;
		case TW_KEY_F1:
			*_String += "F1";
			break;
		case TW_KEY_F2:
			*_String += "F2";
			break;
		case TW_KEY_F3:
			*_String += "F3";
			break;
		case TW_KEY_F4:
			*_String += "F4";
			break;
		case TW_KEY_F5:
			*_String += "F5";
			break;
		case TW_KEY_F6:
			*_String += "F6";
			break;
		case TW_KEY_F7:
			*_String += "F7";
			break;
		case TW_KEY_F8:
			*_String += "F8";
			break;
		case TW_KEY_F9:
			*_String += "F9";
			break;
		case TW_KEY_F10:
			*_String += "F10";
			break;
		case TW_KEY_F11:
			*_String += "F11";
			break;
		case TW_KEY_F12:
			*_String += "F12";
			break;
		case TW_KEY_F13:
			*_String += "F13";
			break;
		case TW_KEY_F14:
			*_String += "F14";
			break;
		case TW_KEY_F15:
			*_String += "F15";
			break;
		default:
			if (_Code > 0 && _Code < 256)
				*_String += char(_Code);
			else {
				*_String += "Unknown";
				Ok = false;
			}
	}
	return Ok;
}

//  ---------------------------------------------------------------------------

const int TW_MOUSE_NOMOTION = -1;
ETwMouseAction TW_MOUSE_MOTION = (ETwMouseAction)(-2);
ETwMouseAction TW_MOUSE_WHEEL = (ETwMouseAction)(-3);
ETwMouseButtonID TW_MOUSE_NA = (ETwMouseButtonID)(-1);

static int TwMouseEvent(ETwMouseAction _EventType, TwMouseButtonID _Button, int _MouseX, int _MouseY, int _WheelPos) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr == NULL || g_TwMgr->m_Graph == NULL) {
		// TwGlobalError(g_ErrNotInit); -> not an error here
		return 0; // not initialized
	}
	if (g_TwMgr->m_WndHeight <= 0 || g_TwMgr->m_WndWidth <= 0) {
		// g_TwMgr->SetLastError(g_ErrBadWndSize);   // not an error, windows not yet ready.
		return 0;
	}

	// For multi-thread safety
	if (!TwFreeAsyncDrawing())
		return 0;

	if (_MouseX == TW_MOUSE_NOMOTION)
		_MouseX = g_TwMgr->m_LastMouseX;
	else
		g_TwMgr->m_LastMouseX = _MouseX;
	if (_MouseY == TW_MOUSE_NOMOTION)
		_MouseY = g_TwMgr->m_LastMouseY;
	else
		g_TwMgr->m_LastMouseY = _MouseY;

	// for autorepeat
	if ((!g_TwMgr->m_IsRepeatingMousePressed || !g_TwMgr->m_CanRepeatMousePressed) && _EventType == TW_MOUSE_PRESSED) {
		g_TwMgr->m_LastMousePressedTime = g_TwMgr->m_Timer.GetTime();
		g_TwMgr->m_LastMousePressedButtonID = _Button;
		g_TwMgr->m_LastMousePressedPosition[0] = _MouseX;
		g_TwMgr->m_LastMousePressedPosition[1] = _MouseY;
		g_TwMgr->m_CanRepeatMousePressed = true;
		g_TwMgr->m_IsRepeatingMousePressed = false;
	} else if (_EventType == TW_MOUSE_RELEASED || _EventType == TW_MOUSE_WHEEL) {
		g_TwMgr->m_CanRepeatMousePressed = false;
		g_TwMgr->m_IsRepeatingMousePressed = false;
	}

	bool Handled = false;
	bool wasPopup = (g_TwMgr->m_PopupBar != nullptr);
	CTwBar *Bar = nullptr;
	int i;

	// search for a bar with mousedrag enabled
	CTwBar *BarDragging = nullptr;
	for (i = ((int)g_TwMgr->m_Bars.size()) - 1; i >= 0; --i) {
		Bar = g_TwMgr->m_Bars[g_TwMgr->m_Order[i]];
		if (Bar != nullptr && Bar->m_Visible && Bar->IsDragging()) {
			BarDragging = Bar;
			break;
		}
	}

	for (i = (int)g_TwMgr->m_Bars.size(); i >= 0; --i) {
		if (i == (int)g_TwMgr->m_Bars.size()) // first try the bar with mousedrag enabled (this bar has the focus)
			Bar = BarDragging;
		else {
			Bar = g_TwMgr->m_Bars[g_TwMgr->m_Order[i]];
			if (Bar == BarDragging)
				continue;
		}
		if (Bar != nullptr && Bar->m_Visible) {
			if (_EventType == TW_MOUSE_MOTION)
				Handled = Bar->MouseMotion(_MouseX, _MouseY);
			else if (_EventType == TW_MOUSE_PRESSED || _EventType == TW_MOUSE_RELEASED)
				Handled = Bar->MouseButton(_Button, (_EventType == TW_MOUSE_PRESSED), _MouseX, _MouseY);
			else if (_EventType == TW_MOUSE_WHEEL) {
				if (abs(_WheelPos - g_TwMgr->m_LastMouseWheelPos) < 4) // avoid crazy wheel positions
					Handled = Bar->MouseWheel(_WheelPos, g_TwMgr->m_LastMouseWheelPos, _MouseX, _MouseY);
			}
			if (Handled)
				break;
		}
	}

	if (g_TwMgr == nullptr) // Mgr might have been destroyed by the client inside a callback call
		return 1;

	// if( i>=0 && Bar!=nullptr && Handled && (_EventType==TW_MOUSE_PRESSED || Bar->IsMinimized()) && i!=((int)g_TwMgr->m_Bars.size())-1 )
	// {
	//   int iOrder = g_TwMgr->m_Order[i];
	//   for( int j=i; j<(int)g_TwMgr->m_Bars.size()-1; ++j )
	//     g_TwMgr->m_Order[j] = g_TwMgr->m_Order[j+1];
	//   g_TwMgr->m_Order[(int)g_TwMgr->m_Bars.size()-1] = iOrder;
	// }

	if (_EventType == TW_MOUSE_PRESSED || (Bar != nullptr && Bar->IsMinimized() && Handled)) {
		if (wasPopup && Bar != g_TwMgr->m_PopupBar && g_TwMgr->m_PopupBar != nullptr) { // delete popup
			TwDeleteBar(g_TwMgr->m_PopupBar);
			g_TwMgr->m_PopupBar = nullptr;
		}

		if (i >= 0 && Bar != nullptr && Handled && !wasPopup)
			TwSetTopBar(Bar);
	}

	if (_EventType == TW_MOUSE_WHEEL)
		g_TwMgr->m_LastMouseWheelPos = _WheelPos;

	return Handled ? 1 : 0;
}

int ANT_CALL TwMouseButton(ETwMouseAction _EventType, TwMouseButtonID _Button) { return TwMouseEvent(_EventType, _Button, TW_MOUSE_NOMOTION, TW_MOUSE_NOMOTION, 0); }

int ANT_CALL TwMouseMotion(int _MouseX, int _MouseY) { return TwMouseEvent(TW_MOUSE_MOTION, TW_MOUSE_NA, _MouseX, _MouseY, 0); }

int ANT_CALL TwMouseWheel(int _Pos) { return TwMouseEvent(TW_MOUSE_WHEEL, TW_MOUSE_NA, TW_MOUSE_NOMOTION, TW_MOUSE_NOMOTION, _Pos); }

//  ---------------------------------------------------------------------------

static int TranslateKey(int _Key, int _Modifiers) {
	// CTRL special cases
	// if( (_Modifiers&TW_KMOD_CTRL) && !(_Modifiers&TW_KMOD_ALT || _Modifiers&TW_KMOD_META) && _Key>0 && _Key<32 )
	//   _Key += 'a'-1;
	if ((_Modifiers & TW_KMOD_CTRL)) {
		if (_Key >= 'a' && _Key <= 'z' && (((_Modifiers & 0x2000) && !(_Modifiers & TW_KMOD_SHIFT)) || (!(_Modifiers & 0x2000) && (_Modifiers & TW_KMOD_SHIFT)))) // 0x2000 is SDL's KMOD_CAPS
			_Key += 'A' - 'a';
		else if (_Key >= 'A' && _Key <= 'Z' && (((_Modifiers & 0x2000) && (_Modifiers & TW_KMOD_SHIFT)) || (!(_Modifiers & 0x2000) && !(_Modifiers & TW_KMOD_SHIFT)))) // 0x2000 is SDL's KMOD_CAPS
			_Key += 'a' - 'A';
	}

	// PAD translation (for SDL keysym)
	if (_Key >= 256 && _Key <= 272) // 256=SDLK_KP0 ... 272=SDLK_KP_EQUALS
	{
		// bool Num = ((_Modifiers&TW_KMOD_SHIFT) && !(_Modifiers&0x1000)) || (!(_Modifiers&TW_KMOD_SHIFT) && (_Modifiers&0x1000)); // 0x1000 is SDL's KMOD_NUM
		// _Modifiers &= ~TW_KMOD_SHIFT; // remove shift modifier
		bool Num = (!(_Modifiers & TW_KMOD_SHIFT) && (_Modifiers & 0x1000)); // 0x1000 is SDL's KMOD_NUM
		if (_Key == 266) // SDLK_KP_PERIOD
			_Key = Num ? '.' : TW_KEY_DELETE;
		else if (_Key == 267) // SDLK_KP_DIVIDE
			_Key = '/';
		else if (_Key == 268) // SDLK_KP_MULTIPLY
			_Key = '*';
		else if (_Key == 269) // SDLK_KP_MINUS
			_Key = '-';
		else if (_Key == 270) // SDLK_KP_PLUS
			_Key = '+';
		else if (_Key == 271) // SDLK_KP_ENTER
			_Key = TW_KEY_RETURN;
		else if (_Key == 272) // SDLK_KP_EQUALS
			_Key = '=';
		else if (Num) // num SDLK_KP0..9
			_Key += '0' - 256;
		else if (_Key == 256) // non-num SDLK_KP01
			_Key = TW_KEY_INSERT;
		else if (_Key == 257) // non-num SDLK_KP1
			_Key = TW_KEY_END;
		else if (_Key == 258) // non-num SDLK_KP2
			_Key = TW_KEY_DOWN;
		else if (_Key == 259) // non-num SDLK_KP3
			_Key = TW_KEY_PAGE_DOWN;
		else if (_Key == 260) // non-num SDLK_KP4
			_Key = TW_KEY_LEFT;
		else if (_Key == 262) // non-num SDLK_KP6
			_Key = TW_KEY_RIGHT;
		else if (_Key == 263) // non-num SDLK_KP7
			_Key = TW_KEY_HOME;
		else if (_Key == 264) // non-num SDLK_KP8
			_Key = TW_KEY_UP;
		else if (_Key == 265) // non-num SDLK_KP9
			_Key = TW_KEY_PAGE_UP;
	}
	return _Key;
}

//  ---------------------------------------------------------------------------

static int KeyPressed(int _Key, int _Modifiers, bool _TestOnly) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr == NULL || g_TwMgr->m_Graph == nullptr) {
		// TwGlobalError(g_ErrNotInit); -> not an error here
		return 0; // not initialized
	}
	if (g_TwMgr->m_WndHeight <= 0 || g_TwMgr->m_WndWidth <= 0) {
		//g_TwMgr->SetLastError(g_ErrBadWndSize); // not an error, windows not yet ready.
		return 0;
	}

	// For multi-thread savety
	if (!TwFreeAsyncDrawing())
		return 0;

	// Test for TwDeleteBar
	// if( _Key>='0' && _Key<='9' )
	// {
	//   int n = _Key-'0';
	//   if( (int)g_TwMgr->m_Bars.size()>n && g_TwMgr->m_Bars[n]!=nullptr )
	//   {
	//     printf("Delete %s\n", g_TwMgr->m_Bars[n]->m_Name.c_str());
	//     TwDeleteBar(g_TwMgr->m_Bars[n]);
	//   }
	//   else
	//     printf("can't delete %d\n", n);
	//   return 1;
	// }

	// char s[256];
	// snprintf(s, 256, "twkeypressed k=%d m=%x\n", _Key, _Modifiers);
	// OutputDebugString(s);

	_Key = TranslateKey(_Key, _Modifiers);
	if (_Key > ' ' && _Key < 256) // don't test SHIFT if _Key is a common key
		_Modifiers &= ~TW_KMOD_SHIFT;
	// complete partial modifiers comming from SDL
	if (_Modifiers & TW_KMOD_SHIFT)
		_Modifiers |= TW_KMOD_SHIFT;
	if (_Modifiers & TW_KMOD_CTRL)
		_Modifiers |= TW_KMOD_CTRL;
	if (_Modifiers & TW_KMOD_ALT)
		_Modifiers |= TW_KMOD_ALT;
	if (_Modifiers & TW_KMOD_META)
		_Modifiers |= TW_KMOD_META;

	bool Handled = false;
	CTwBar *Bar = nullptr;
	CTwBar *PopupBar = g_TwMgr->m_PopupBar;
	// int Order = 0;
	int i;
	if (_Key > 0 && _Key < TW_KEY_LAST) {
		// First send it to bar which includes the mouse pointer
		int MouseX = g_TwMgr->m_LastMouseX;
		int MouseY = g_TwMgr->m_LastMouseY;
		for (i = ((int)g_TwMgr->m_Bars.size()) - 1; i >= 0 && !Handled; --i) {
			Bar = g_TwMgr->m_Bars[g_TwMgr->m_Order[i]];
			if (Bar != nullptr && Bar->m_Visible && !Bar->IsMinimized() && ((MouseX >= Bar->m_PosX && MouseX < Bar->m_PosX + Bar->m_Width && MouseY >= Bar->m_PosY && MouseY < Bar->m_PosY + Bar->m_Height) || Bar == PopupBar)) {
				if (_TestOnly)
					Handled = Bar->KeyTest(_Key, _Modifiers);
				else
					Handled = Bar->KeyPressed(_Key, _Modifiers);
			}
		}

		// If not handled, send it to non-iconified bars in the right order
		for (i = ((int)g_TwMgr->m_Bars.size()) - 1; i >= 0 && !Handled; --i) {
			Bar = g_TwMgr->m_Bars[g_TwMgr->m_Order[i]];
			if (Bar != nullptr && Bar->m_Visible && !Bar->IsMinimized()) {
				if (_TestOnly)
					Handled = Bar->KeyTest(_Key, _Modifiers);
				else
					Handled = Bar->KeyPressed(_Key, _Modifiers);
				if (g_TwMgr == nullptr) // Mgr might have been destroyed by the client inside a callback call
					return 1;
			}
		}

		// If not handled, send it to iconified bars in the right order
		for (i = ((int)g_TwMgr->m_Bars.size()) - 1; i >= 0 && !Handled; --i) {
			Bar = g_TwMgr->m_Bars[g_TwMgr->m_Order[i]];
			if (Bar != nullptr && Bar->m_Visible && Bar->IsMinimized()) {
				if (_TestOnly)
					Handled = Bar->KeyTest(_Key, _Modifiers);
				else
					Handled = Bar->KeyPressed(_Key, _Modifiers);
			}
		}

		if (g_TwMgr->m_HelpBar != nullptr && g_TwMgr->m_Graph && !_TestOnly) {
			std::string Str;
			TwGetKeyString(&Str, _Key, _Modifiers);
			char Msg[256];
			snprintf(Msg, 256, "Key pressed: %s", Str.c_str());
			g_TwMgr->m_KeyPressedStr = Msg;
			g_TwMgr->m_KeyPressedBuildText = true;
			// OutputDebugString(Msg);
		}
	}

	if (Handled && Bar != g_TwMgr->m_PopupBar && g_TwMgr->m_PopupBar != nullptr && g_TwMgr->m_PopupBar == PopupBar) // delete popup
	{
		TwDeleteBar(g_TwMgr->m_PopupBar);
		g_TwMgr->m_PopupBar = nullptr;
	}

	if (Handled && Bar != nullptr && Bar != g_TwMgr->m_PopupBar && Bar != PopupBar) // popup bar may have been destroyed
		TwSetTopBar(Bar);

	return Handled ? 1 : 0;
}

int ANT_CALL TwKeyPressed(int _Key, int _Modifiers) { return KeyPressed(_Key, _Modifiers, false); }

int ANT_CALL TwKeyTest(int _Key, int _Modifiers) { return KeyPressed(_Key, _Modifiers, true); }

//  ---------------------------------------------------------------------------

struct StructCompare : public std::function<bool(TwType, TwType)> {
	bool operator()(const TwType &_Left, const TwType &_Right) const {
		DEV_ASSERT(g_TwMgr != nullptr);
		int i0 = _Left - TW_TYPE_STRUCT_BASE;
		int i1 = _Right - TW_TYPE_STRUCT_BASE;
		if (i0 >= 0 && i0 < (int)g_TwMgr->m_Structs.size() && i1 >= 0 && i1 < (int)g_TwMgr->m_Structs.size())
			return g_TwMgr->m_Structs[i0].m_Name < g_TwMgr->m_Structs[i1].m_Name;
		else
			return false;
	}
};

typedef std::set<TwType, StructCompare> StructSet;

static void InsertUsedStructs(StructSet &_Set, const CTwVarGroup *_Grp) {
	DEV_ASSERT(g_TwMgr != nullptr && _Grp != nullptr);

	for (size_t i = 0; i < _Grp->m_Vars.size(); ++i)
		if (_Grp->m_Vars[i] != nullptr && _Grp->m_Vars[i]->m_Visible && _Grp->m_Vars[i]->IsGroup()) // && _Grp->m_Vars[i]->m_Help.length()>0 )
		{
			const CTwVarGroup *SubGrp = static_cast<const CTwVarGroup *>(_Grp->m_Vars[i]);
			if (SubGrp->m_StructValuePtr != nullptr && SubGrp->m_StructType >= TW_TYPE_STRUCT_BASE && SubGrp->m_StructType < TW_TYPE_STRUCT_BASE + (int)g_TwMgr->m_Structs.size() && g_TwMgr->m_Structs[SubGrp->m_StructType - TW_TYPE_STRUCT_BASE].m_Name.length() > 0) {
				if (SubGrp->m_Help.length() > 0)
					_Set.insert(SubGrp->m_StructType);
				else {
					int idx = SubGrp->m_StructType - TW_TYPE_STRUCT_BASE;
					if (idx >= 0 && idx < (int)g_TwMgr->m_Structs.size() && g_TwMgr->m_Structs[idx].m_Name.length() > 0) {
						for (size_t j = 0; j < g_TwMgr->m_Structs[idx].m_Members.size(); ++j)
							if (g_TwMgr->m_Structs[idx].m_Members[j].m_Help.length() > 0) {
								_Set.insert(SubGrp->m_StructType);
								break;
							}
					}
				}
			}
			InsertUsedStructs(_Set, SubGrp);
		}
}

static void SplitString(std::vector<std::string> &_OutSplits, const char *_String, int _Width, const CTexFont *_Font) {
	DEV_ASSERT(_Font != NULL && _String != NULL);
	_OutSplits.resize(0);
	int l = (int)strlen(_String);
	if (l == 0) {
		_String = " ";
		l = 1;
	}

	if (_String != NULL && l > 0 && _Width > 0) {
		int w = 0;
		int i = 0;
		int First = 0;
		int Last = 0;
		bool PrevNotBlank = true;
		unsigned char c;
		bool Tab = false, CR = false;
		std::string Split;
		const std::string TabString(g_TabLength, ' ');

		while (i < l) {
			c = _String[i];
			if (c == '\t') {
				w += g_TabLength * _Font->m_CharWidth[(int)' '];
				Tab = true;
			} else if (c == '\n') {
				w += _Width + 1; // force split
				Last = i;
				CR = true;
			} else
				w += _Font->m_CharWidth[(int)c];
			if (w > _Width || i == l - 1) {
				if (Last <= First || i == l - 1)
					Last = i;
				if (Tab) {
					Split.resize(0);
					for (int k = 0; k < Last - First + (CR ? 0 : 1); ++k)
						if (_String[First + k] == '\t')
							Split += TabString;
						else
							Split += _String[First + k];
					Tab = false;
				} else
					Split.assign(_String + First, Last - First + (CR ? 0 : 1));
				_OutSplits.push_back(Split);
				First = Last + 1;
				if (!CR)
					while (First < l && (_String[First] == ' ' || _String[First] == '\t')) // skip blanks
						++First;
				Last = First;
				w = 0;
				PrevNotBlank = true;
				i = First;
				CR = false;
			} else if (c == ' ' || c == '\t') {
				if (PrevNotBlank)
					Last = i - 1;
				PrevNotBlank = false;
				++i;
			} else {
				PrevNotBlank = true;
				++i;
			}
		}
	}
}

static int AppendHelpString(CTwVarGroup *_Grp, const char *_String, int _Level, int _Width, ETwType _Type) {
	DEV_ASSERT(_Grp != nullptr && g_TwMgr != nullptr && g_TwMgr->m_HelpBar != nullptr);
	DEV_ASSERT(_String != nullptr);
	int n = 0;
	const CTexFont *Font = g_TwMgr->m_HelpBar->m_Font;
	DEV_ASSERT(Font != nullptr);
	std::string Decal;
	for (int s = 0; s < _Level; ++s)
		Decal += ' ';
	int DecalWidth = (_Level + 2) * Font->m_CharWidth[(int)' '];

	if (_Width > DecalWidth) {
		std::vector<std::string> Split;
		SplitString(Split, _String, _Width - DecalWidth, Font);
		for (int i = 0; i < (int)Split.size(); ++i) {
			CTwVarAtom *Var = new CTwVarAtom;
			Var->m_Name = Decal + Split[i];
			Var->m_Ptr = nullptr;
			if (_Type == TW_TYPE_HELP_HEADER)
				Var->m_ReadOnly = false;
			else
				Var->m_ReadOnly = true;
			Var->m_NoSlider = true;
			Var->m_DontClip = true;
			Var->m_Type = _Type;
			Var->m_LeftMargin = (signed short)((_Level + 1) * Font->m_CharWidth[(int)' ']);
			Var->m_TopMargin = (signed short)(-g_TwMgr->m_HelpBar->m_Sep);
			// Var->m_TopMargin  = 1;
			Var->m_ColorPtr = &(g_TwMgr->m_HelpBar->m_ColHelpText);
			Var->SetDefaults();
			_Grp->m_Vars.push_back(Var);
			++n;
		}
	}
	return n;
}

static int AppendHelp(CTwVarGroup *_Grp, const CTwVarGroup *_ToAppend, int _Level, int _Width) {
	DEV_ASSERT(_Grp != NULL);
	DEV_ASSERT(_ToAppend != NULL);
	int n = 0;
	std::string Decal;
	for (int s = 0; s < _Level; ++s)
		Decal += ' ';

	if (_ToAppend->m_Help.size() > 0)
		n += AppendHelpString(_Grp, _ToAppend->m_Help.c_str(), _Level, _Width, TW_TYPE_HELP_GRP);

	for (size_t i = 0; i < _ToAppend->m_Vars.size(); ++i)
		if (_ToAppend->m_Vars[i] != NULL && _ToAppend->m_Vars[i]->m_Visible) {
			bool append = true;
			if (!_ToAppend->m_Vars[i]->IsGroup()) {
				const CTwVarAtom *a = static_cast<const CTwVarAtom *>(_ToAppend->m_Vars[i]);
				if (a->m_Type == TW_TYPE_BUTTON && a->m_Val.m_Button.m_Callback == NULL)
					append = false;
				else if (a->m_KeyIncr[0] == 0 && a->m_KeyIncr[1] == 0 && a->m_KeyDecr[0] == 0 && a->m_KeyDecr[1] == 0 && a->m_Help.length() <= 0)
					append = false;
			} else if (_ToAppend->m_Vars[i]->IsGroup() && static_cast<const CTwVarGroup *>(_ToAppend->m_Vars[i])->m_StructValuePtr != NULL // that's a struct var
					&& _ToAppend->m_Vars[i]->m_Help.length() <= 0)
				append = false;

			if (append) {
				CTwVarAtom *Var = new CTwVarAtom;
				Var->m_Name = Decal;
				if (_ToAppend->m_Vars[i]->m_Label.size() > 0)
					Var->m_Name += _ToAppend->m_Vars[i]->m_Label;
				else
					Var->m_Name += _ToAppend->m_Vars[i]->m_Name;
				Var->m_Ptr = NULL;
				if (_ToAppend->m_Vars[i]->IsGroup() && static_cast<const CTwVarGroup *>(_ToAppend->m_Vars[i])->m_StructValuePtr != NULL) { // That's a struct var
					Var->m_Type = TW_TYPE_HELP_STRUCT;
					Var->m_Val.m_HelpStruct.m_StructType = static_cast<const CTwVarGroup *>(_ToAppend->m_Vars[i])->m_StructType;
					Var->m_ReadOnly = true;
					Var->m_NoSlider = true;
				} else if (!_ToAppend->m_Vars[i]->IsGroup()) {
					Var->m_Type = TW_TYPE_SHORTCUT;
					Var->m_Val.m_Shortcut.m_Incr[0] = static_cast<const CTwVarAtom *>(_ToAppend->m_Vars[i])->m_KeyIncr[0];
					Var->m_Val.m_Shortcut.m_Incr[1] = static_cast<const CTwVarAtom *>(_ToAppend->m_Vars[i])->m_KeyIncr[1];
					Var->m_Val.m_Shortcut.m_Decr[0] = static_cast<const CTwVarAtom *>(_ToAppend->m_Vars[i])->m_KeyDecr[0];
					Var->m_Val.m_Shortcut.m_Decr[1] = static_cast<const CTwVarAtom *>(_ToAppend->m_Vars[i])->m_KeyDecr[1];
					Var->m_ReadOnly = static_cast<const CTwVarAtom *>(_ToAppend->m_Vars[i])->m_ReadOnly;
					Var->m_NoSlider = true;
				} else {
					Var->m_Type = TW_TYPE_HELP_GRP;
					Var->m_DontClip = true;
					Var->m_LeftMargin = (signed short)((_Level + 2) * g_TwMgr->m_HelpBar->m_Font->m_CharWidth[(int)' ']);
					// Var->m_TopMargin  = (signed short)(g_TwMgr->m_HelpBar->m_Font->m_CharHeight/2-2+2*(_Level-1));
					Var->m_TopMargin = 2;
					if (Var->m_TopMargin > g_TwMgr->m_HelpBar->m_Font->m_CharHeight - 3)
						Var->m_TopMargin = (signed short)(g_TwMgr->m_HelpBar->m_Font->m_CharHeight - 3);
					Var->m_ReadOnly = true;
				}
				Var->SetDefaults();
				_Grp->m_Vars.push_back(Var);
				size_t VarIndex = _Grp->m_Vars.size() - 1;
				++n;
				if (_ToAppend->m_Vars[i]->IsGroup() && static_cast<const CTwVarGroup *>(_ToAppend->m_Vars[i])->m_StructValuePtr == NULL) {
					int nAppended = AppendHelp(_Grp, static_cast<const CTwVarGroup *>(_ToAppend->m_Vars[i]), _Level + 1, _Width);
					if (_Grp->m_Vars.size() == VarIndex + 1) {
						delete _Grp->m_Vars[VarIndex];
						_Grp->m_Vars.resize(VarIndex);
					} else
						n += nAppended;
				} else if (_ToAppend->m_Vars[i]->m_Help.length() > 0)
					n += AppendHelpString(_Grp, _ToAppend->m_Vars[i]->m_Help.c_str(), _Level + 1, _Width, TW_TYPE_HELP_ATOM);
			}
		}
	return n;
}

static void CopyHierarchy(CTwVarGroup *dst, const CTwVarGroup *src) {
	if (dst == nullptr || src == nullptr)
		return;

	dst->m_Name = src->m_Name;
	dst->m_Open = src->m_Open;
	dst->m_Visible = src->m_Visible;
	dst->m_ColorPtr = src->m_ColorPtr;
	dst->m_DontClip = src->m_DontClip;
	dst->m_IsRoot = src->m_IsRoot;
	dst->m_LeftMargin = src->m_LeftMargin;
	dst->m_TopMargin = src->m_TopMargin;

	dst->m_Vars.resize(src->m_Vars.size());
	for (size_t i = 0; i < src->m_Vars.size(); ++i)
		if (src->m_Vars[i] != nullptr && src->m_Vars[i]->IsGroup()) {
			CTwVarGroup *grp = new CTwVarGroup;
			CopyHierarchy(grp, static_cast<const CTwVarGroup *>(src->m_Vars[i]));
			dst->m_Vars[i] = grp;
		} else
			dst->m_Vars[i] = nullptr;
}

// copy the 'open' flag from original hierarchy to current hierarchy
static void SynchroHierarchy(CTwVarGroup *cur, const CTwVarGroup *orig) {
	if (cur == nullptr || orig == nullptr)
		return;

	if (strcmp(cur->m_Name.c_str(), orig->m_Name.c_str()) == 0)
		cur->m_Open = orig->m_Open;

	size_t j = 0;
	while (j < orig->m_Vars.size() && (orig->m_Vars[j] == nullptr || !orig->m_Vars[j]->IsGroup()))
		++j;

	for (size_t i = 0; i < cur->m_Vars.size(); ++i)
		if (cur->m_Vars[i] != nullptr && cur->m_Vars[i]->IsGroup() && j < orig->m_Vars.size() && orig->m_Vars[j] != nullptr && orig->m_Vars[j]->IsGroup()) {
			CTwVarGroup *curGrp = static_cast<CTwVarGroup *>(cur->m_Vars[i]);
			const CTwVarGroup *origGrp = static_cast<const CTwVarGroup *>(orig->m_Vars[j]);
			if (strcmp(curGrp->m_Name.c_str(), origGrp->m_Name.c_str()) == 0) {
				curGrp->m_Open = origGrp->m_Open;

				SynchroHierarchy(curGrp, origGrp);

				++j;
				while (j < orig->m_Vars.size() && (orig->m_Vars[j] == nullptr || !orig->m_Vars[j]->IsGroup()))
					++j;
			}
		}
}

void CTwMgr::UpdateHelpBar() {
	if (m_HelpBar == nullptr || m_HelpBar->IsMinimized())
		return;
	if (!m_HelpBarUpdateNow && (float)m_Timer.GetTime() < m_LastHelpUpdateTime + 2) // update at most every 2 seconds
		return;
	m_HelpBarUpdateNow = false;
	m_LastHelpUpdateTime = (float)m_Timer.GetTime();

	CTwVarGroup prevHierarchy;
	CopyHierarchy(&prevHierarchy, &m_HelpBar->m_VarRoot);

	TwRemoveAllVars(m_HelpBar);

	if (m_HelpBar->m_UpToDate)
		m_HelpBar->Update();

	if (m_Help.size() > 0)
		AppendHelpString(&(m_HelpBar->m_VarRoot), m_Help.c_str(), 0, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0, TW_TYPE_HELP_ATOM);
	if (m_HelpBar->m_Help.size() > 0)
		AppendHelpString(&(m_HelpBar->m_VarRoot), m_HelpBar->m_Help.c_str(), 0, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0, TW_TYPE_HELP_ATOM);
	AppendHelpString(&(m_HelpBar->m_VarRoot), "", 0, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0, TW_TYPE_HELP_HEADER);

	for (size_t ib = 0; ib < m_Bars.size(); ++ib)
		if (m_Bars[ib] != NULL && !(m_Bars[ib]->m_IsHelpBar) && m_Bars[ib] != m_PopupBar && m_Bars[ib]->m_Visible) {
			// Create a group
			CTwVarGroup *Grp = new CTwVarGroup;
			Grp->m_SummaryCallback = NULL;
			Grp->m_SummaryClientData = NULL;
			Grp->m_StructValuePtr = NULL;
			if (m_Bars[ib]->m_Label.size() <= 0)
				Grp->m_Name = m_Bars[ib]->m_Name;
			else
				Grp->m_Name = m_Bars[ib]->m_Label;
			Grp->m_Open = true;
			Grp->m_ColorPtr = &(m_HelpBar->m_ColGrpText);
			m_HelpBar->m_VarRoot.m_Vars.push_back(Grp);
			if (m_Bars[ib]->m_Help.size() > 0)
				AppendHelpString(Grp, m_Bars[ib]->m_Help.c_str(), 0, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0, TW_TYPE_HELP_GRP);

			// Append variables (recursive)
			AppendHelp(Grp, &(m_Bars[ib]->m_VarRoot), 1, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0);

			// Append structures
			StructSet UsedStructs;
			InsertUsedStructs(UsedStructs, &(m_Bars[ib]->m_VarRoot));
			CTwVarGroup *StructGrp = NULL;
			int MemberCount = 0;
			for (StructSet::iterator it = UsedStructs.begin(); it != UsedStructs.end(); ++it) {
				int idx = (*it) - TW_TYPE_STRUCT_BASE;
				if (idx >= 0 && idx < (int)g_TwMgr->m_Structs.size() && g_TwMgr->m_Structs[idx].m_Name.length() > 0) {
					if (StructGrp == NULL) {
						StructGrp = new CTwVarGroup;
						StructGrp->m_StructType = TW_TYPE_HELP_STRUCT; // a special line background color will be used
						StructGrp->m_Name = "Structures";
						StructGrp->m_Open = false;
						StructGrp->m_ColorPtr = &(m_HelpBar->m_ColStructText);
						//Grp->m_Vars.push_back(StructGrp);
						MemberCount = 0;
					}
					CTwVarAtom *Var = new CTwVarAtom;
					Var->m_Ptr = NULL;
					Var->m_Type = TW_TYPE_HELP_GRP;
					Var->m_DontClip = true;
					Var->m_LeftMargin = (signed short)(3 * g_TwMgr->m_HelpBar->m_Font->m_CharWidth[(int)' ']);
					Var->m_TopMargin = 2;
					Var->m_ReadOnly = true;
					Var->m_NoSlider = true;
					Var->m_Name = '{' + g_TwMgr->m_Structs[idx].m_Name + '}';
					StructGrp->m_Vars.push_back(Var);
					size_t structIndex = StructGrp->m_Vars.size() - 1;
					if (g_TwMgr->m_Structs[idx].m_Help.size() > 0)
						AppendHelpString(StructGrp, g_TwMgr->m_Structs[idx].m_Help.c_str(), 2, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0 - 2 * Var->m_LeftMargin, TW_TYPE_HELP_ATOM);

					// Append struct members
					for (size_t im = 0; im < g_TwMgr->m_Structs[idx].m_Members.size(); ++im) {
						if (g_TwMgr->m_Structs[idx].m_Members[im].m_Help.size() > 0) {
							CTwVarAtom *Var = new CTwVarAtom;
							Var->m_Ptr = nullptr;
							Var->m_Type = TW_TYPE_SHORTCUT;
							Var->m_Val.m_Shortcut.m_Incr[0] = 0;
							Var->m_Val.m_Shortcut.m_Incr[1] = 0;
							Var->m_Val.m_Shortcut.m_Decr[0] = 0;
							Var->m_Val.m_Shortcut.m_Decr[1] = 0;
							Var->m_ReadOnly = false;
							Var->m_NoSlider = true;
							if (g_TwMgr->m_Structs[idx].m_Members[im].m_Label.length() > 0)
								Var->m_Name = "  " + g_TwMgr->m_Structs[idx].m_Members[im].m_Label;
							else
								Var->m_Name = "  " + g_TwMgr->m_Structs[idx].m_Members[im].m_Name;
							StructGrp->m_Vars.push_back(Var);
							// if( g_TwMgr->m_Structs[idx].m_Members[im].m_Help.size()>0 )
							AppendHelpString(StructGrp, g_TwMgr->m_Structs[idx].m_Members[im].m_Help.c_str(), 3, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0 - 4 * Var->m_LeftMargin, TW_TYPE_HELP_ATOM);
						}
					}

					if (StructGrp->m_Vars.size() == structIndex + 1) // remove struct from help
					{
						delete StructGrp->m_Vars[structIndex];
						StructGrp->m_Vars.resize(structIndex);
					} else
						++MemberCount;
				}
			}
			if (StructGrp != nullptr) {
				if (MemberCount == 1)
					StructGrp->m_Name = "Structure";
				if (StructGrp->m_Vars.size() > 0)
					Grp->m_Vars.push_back(StructGrp);
				else {
					delete StructGrp;
					StructGrp = nullptr;
				}
			}
		}

	// Append RotoSlider
	CTwVarGroup *RotoGrp = new CTwVarGroup;
	RotoGrp->m_SummaryCallback = nullptr;
	RotoGrp->m_SummaryClientData = nullptr;
	RotoGrp->m_StructValuePtr = nullptr;
	RotoGrp->m_Name = "RotoSlider";
	RotoGrp->m_Open = false;
	RotoGrp->m_ColorPtr = &(m_HelpBar->m_ColGrpText);
	m_HelpBar->m_VarRoot.m_Vars.push_back(RotoGrp);
	AppendHelpString(RotoGrp, "The RotoSlider allows rapid editing of numerical values.", 0, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0, TW_TYPE_HELP_ATOM);
	AppendHelpString(RotoGrp, "To modify a numerical value, click on its label or on its roto [.] button, then move the mouse outside of the grey circle while keeping the mouse button pressed, and turn around the circle to increase or decrease the numerical value.", 0, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0, TW_TYPE_HELP_ATOM);
	AppendHelpString(RotoGrp, "The two grey lines depict the min and max bounds.", 0, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0, TW_TYPE_HELP_ATOM);
	AppendHelpString(RotoGrp, "Moving the mouse far form the circle allows precise increase or decrease, while moving near the circle allows fast increase or decrease.", 0, m_HelpBar->m_VarX2 - m_HelpBar->m_VarX0, TW_TYPE_HELP_ATOM);

	SynchroHierarchy(&m_HelpBar->m_VarRoot, &prevHierarchy);

	m_HelpBarNotUpToDate = false;
}

//  ---------------------------------------------------------------------------

void ANT_CALL TwCopyCDStringToClientFunc(TwCopyCDStringToClient copyCDStringToClientFunc) {
	g_InitCopyCDStringToClient = copyCDStringToClientFunc;
	if (g_TwMgr != NULL)
		g_TwMgr->m_CopyCDStringToClient = copyCDStringToClientFunc;
}

void ANT_CALL TwCopyCDStringToLibrary(char **destinationLibraryStringPtr, const char *sourceClientString) {
	if (g_TwMgr == NULL) {
		if (destinationLibraryStringPtr != NULL)
			*destinationLibraryStringPtr = const_cast<char *>(sourceClientString);
		return;
	}

	// static buffer to store sourceClientString copy associated to sourceClientString pointer
	std::vector<char> &Buf = g_TwMgr->m_CDStdStringCopyBuffers[(void *)sourceClientString];

	size_t len = (sourceClientString != NULL) ? strlen(sourceClientString) : 0;
	if (Buf.size() < len + 1)
		Buf.resize(len + 128); // len + some margin
	char *SrcStrCopy = &(Buf[0]);
	SrcStrCopy[0] = '\0';
	if (sourceClientString != NULL)
		memcpy(SrcStrCopy, sourceClientString, len + 1);
	SrcStrCopy[len] = '\0';
	if (destinationLibraryStringPtr != NULL)
		*destinationLibraryStringPtr = SrcStrCopy;
}

void ANT_CALL TwCopyStdStringToClientFunc(TwCopyStdStringToClient copyStdStringToClientFunc) {
	g_InitCopyStdStringToClient = copyStdStringToClientFunc;
	if (g_TwMgr != NULL)
		g_TwMgr->m_CopyStdStringToClient = copyStdStringToClientFunc;
}

void ANT_CALL TwCopyStdStringToLibrary(std::string &destLibraryString, const std::string &srcClientString) {
	if (g_TwMgr == NULL)
		return;

	CTwMgr::CLibStdString srcLibString; // Convert VC++ Debug/Release std::string
	srcLibString.FromClient(srcClientString);
	const char *SrcStr = srcLibString.ToLib().c_str();
	const char **DstStrPtr = (const char **)&destLibraryString;

	// SrcStr can be defined locally by the caller, so we need to copy it
	// ( *DstStrPtr = copy of SrcStr )

	// static buffer to store srcClientString copy associated to srcClientString pointer
	std::vector<char> &Buf = g_TwMgr->m_CDStdStringCopyBuffers[(void *)&srcClientString];

	size_t len = strlen(SrcStr);
	if (Buf.size() < len + 1)
		Buf.resize(len + 128); // len + some margin
	char *SrcStrCopy = &(Buf[0]);

	memcpy(SrcStrCopy, SrcStr, len + 1);
	SrcStrCopy[len] = '\0';
	*DstStrPtr = SrcStrCopy;
	// *(const char **)&destLibraryString = srcClientString.c_str();
}

//  ---------------------------------------------------------------------------

bool CRect::Subtract(const CRect &_Rect, std::vector<CRect> &_OutRects) const {
	if (Empty())
		return false;
	if (_Rect.Empty() || _Rect.Y >= Y + H || _Rect.Y + _Rect.H <= Y || _Rect.X >= X + W || _Rect.X + _Rect.W <= X) {
		_OutRects.push_back(*this);
		return true;
	}

	bool Ret = false;
	int Y0 = Y;
	int Y1 = Y + H - 1;
	if (_Rect.Y > Y) {
		Y0 = _Rect.Y;
		_OutRects.push_back(CRect(X, Y, W, Y0 - Y + 1));
		Ret = true;
	}
	if (_Rect.Y + _Rect.H < Y + H) {
		Y1 = _Rect.Y + _Rect.H;
		_OutRects.push_back(CRect(X, Y1, W, Y + H - Y1));
		Ret = true;
	}
	int X0 = X;
	int X1 = X + W - 1;
	if (_Rect.X > X) {
		X0 = _Rect.X; //-2;
		_OutRects.push_back(CRect(X, Y0, X0 - X + 1, Y1 - Y0 + 1));
		Ret = true;
	}
	if (_Rect.X + _Rect.W < X + W) {
		X1 = _Rect.X + _Rect.W; //-1;
		_OutRects.push_back(CRect(X1, Y0, X + W - X1, Y1 - Y0 + 1));
		Ret = true;
	}
	return Ret;
}

bool CRect::Subtract(const std::vector<CRect> &_Rects, std::vector<CRect> &_OutRects) const {
	_OutRects.clear();
	size_t NbRects = _Rects.size();
	if (NbRects == 0) {
		_OutRects.push_back(*this);
		return true;
	} else {
		std::vector<CRect> TmpRects;
		Subtract(_Rects[0], _OutRects);

		for (size_t i = 1; i < NbRects; i++) {
			for (size_t j = 0; j < _OutRects.size(); j++)
				_OutRects[j].Subtract(_Rects[i], TmpRects);
			_OutRects.swap(TmpRects);
			TmpRects.clear();
		}
		return _OutRects.empty();
	}
}

//  ---------------------------------------------------------------------------
//  @file       TwBar.cpp
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

extern const char *g_ErrNotFound;
const char *g_ErrUnknownAttrib = "Unknown parameter";
const char *g_ErrInvalidAttrib = "Invalid parameter";
const char *g_ErrNotGroup = "Value is not a group";
const char *g_ErrNoValue = "Value required";
const char *g_ErrBadValue = "Bad value";
const char *g_ErrUnknownType = "Unknown type";
const char *g_ErrNotEnum = "Must be of type Enum";

#undef PERF // comment to print benchs
#define PERF(cmd)

PerfTimer g_BarTimer;

#define ANT_SET_CURSOR(_Name) g_TwMgr->SetCursor(CTwMgr::Cursor##_Name)
#define ANT_SET_ROTO_CURSOR(_Num) g_TwMgr->SetCursor(CTwMgr::s_RotoCursors[_Num])

#if !defined(ANT_WINDOWS)
#define _stricmp strcasecmp
#define _strdup strdup
#endif // defined(ANT_WINDOWS)

#if !defined(M_PI)
#define M_PI 3.1415926535897932384626433832795
#endif // !defined(M_PI)

#if !defined(FLOAT_MAX)
#define FLOAT_MAX 3.0e+38f
#endif // !defined(FLOAT_MAX)
#if !defined(DOUBLE_MAX)
#define DOUBLE_MAX 1.0e+308
#endif // !defined(DOUBLE_MAX)
#if !defined(DOUBLE_EPS)
#define DOUBLE_EPS 1.0e-307
#endif // !defined(DOUBLE_EPS)

bool IsCustomType(int _Type) {
	return (g_TwMgr && _Type >= TW_TYPE_CUSTOM_BASE && _Type < TW_TYPE_CUSTOM_BASE + (int)g_TwMgr->m_Customs.size());
}

bool IsCSStringType(int _Type) {
	return (_Type > TW_TYPE_CSSTRING_BASE && _Type <= TW_TYPE_CSSTRING_MAX);
}

bool IsEnumType(int _Type) {
	return (g_TwMgr && _Type >= TW_TYPE_ENUM_BASE && _Type < TW_TYPE_ENUM_BASE + (int)g_TwMgr->m_Enums.size());
}

//  ---------------------------------------------------------------------------

CTwVar::CTwVar() {
	m_IsRoot = false;
	m_DontClip = false;
	m_Visible = true;
	m_LeftMargin = 0;
	m_TopMargin = 0;
	m_ColorPtr = &COLOR32_WHITE;
	m_BgColorPtr = &COLOR32_ZERO; // default
}

CTwVarAtom::CTwVarAtom() {
	m_Type = TW_TYPE_UNDEF;
	m_Ptr = NULL;
	m_SetCallback = NULL;
	m_GetCallback = nullptr;
	m_ClientData = nullptr;
	m_ReadOnly = false;
	m_NoSlider = false;
	m_KeyIncr[0] = 0;
	m_KeyIncr[1] = 0;
	m_KeyDecr[0] = 0;
	m_KeyDecr[1] = 0;
	memset(&m_Val, 0, sizeof(UVal));
}

CTwVarAtom::~CTwVarAtom() {
	if (m_Type == TW_TYPE_BOOL8 || m_Type == TW_TYPE_BOOL16 || m_Type == TW_TYPE_BOOL32 || m_Type == TW_TYPE_BOOLCPP) {
		if (m_Val.m_Bool.m_FreeTrueString && m_Val.m_Bool.m_TrueString != nullptr) {
			free(m_Val.m_Bool.m_TrueString);
			m_Val.m_Bool.m_TrueString = nullptr;
		}
		if (m_Val.m_Bool.m_FreeFalseString && m_Val.m_Bool.m_FalseString != nullptr) {
			free(m_Val.m_Bool.m_FalseString);
			m_Val.m_Bool.m_FalseString = nullptr;
		}
	} else if (m_Type == TW_TYPE_CDSTDSTRING && m_GetCallback == CTwMgr::CCDStdString::GetCB && m_ClientData != nullptr && g_TwMgr != nullptr) {
		// delete corresponding g_TwMgr->m_CDStdStrings element
		const CTwMgr::CCDStdString *CDStdString = (const CTwMgr::CCDStdString *)m_ClientData;
		//if( &(*CDStdString->m_This)==CDStdString )
		//  g_TwMgr->m_CDStdStrings.erase(CDStdString->m_This);
		for (std::list<CTwMgr::CCDStdString>::iterator it = g_TwMgr->m_CDStdStrings.begin(); it != g_TwMgr->m_CDStdStrings.end(); ++it)
			if (&(*it) == CDStdString) {
				g_TwMgr->m_CDStdStrings.erase(it);
				break;
			}
	}
}

//  ---------------------------------------------------------------------------

void CTwVarAtom::ValueToString(std::string *_Str) const {
	DEV_ASSERT(_Str != nullptr);
	static const char *ErrStr = "unreachable";
	char Tmp[1024];
	if (m_Type == TW_TYPE_UNDEF || m_Type == TW_TYPE_HELP_ATOM || m_Type == TW_TYPE_HELP_GRP || m_Type == TW_TYPE_BUTTON) { // has no value
		*_Str = "";
		return;
	} else if (m_Type == TW_TYPE_HELP_HEADER) {
		*_Str = "SHORTCUTS";
		return;
	} else if (m_Type == TW_TYPE_SHORTCUT) { // special case for help bar: display shortcut
		*_Str = "";
		if (m_ReadOnly && m_Val.m_Shortcut.m_Incr[0] == 0 && m_Val.m_Shortcut.m_Decr[0] == 0)
			(*_Str) = "(read only)";
		else {
			if (m_Val.m_Shortcut.m_Incr[0] > 0)
				TwGetKeyString(_Str, m_Val.m_Shortcut.m_Incr[0], m_Val.m_Shortcut.m_Incr[1]);
			else
				(*_Str) += "(none)";
			if (m_Val.m_Shortcut.m_Decr[0] > 0) {
				(*_Str) += "  ";
				TwGetKeyString(_Str, m_Val.m_Shortcut.m_Decr[0], m_Val.m_Shortcut.m_Decr[1]);
			}
		}
		return;
	} else if (m_Type == TW_TYPE_HELP_STRUCT) {
		int idx = m_Val.m_HelpStruct.m_StructType - TW_TYPE_STRUCT_BASE;
		if (idx >= 0 && idx < (int)g_TwMgr->m_Structs.size()) {
			if (g_TwMgr->m_Structs[idx].m_Name.length() > 0)
				(*_Str) = '{' + g_TwMgr->m_Structs[idx].m_Name + '}';
			else
				(*_Str) = "{struct}";
		}
		return;
	}

	if (m_Ptr == nullptr && m_GetCallback == nullptr) {
		*_Str = ErrStr;
		return;
	}
	bool UseGet = (m_GetCallback != nullptr);
	switch (m_Type) {
		case TW_TYPE_BOOLCPP: {
			bool Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(bool *)m_Ptr;
			if (Val)
				*_Str = (m_Val.m_Bool.m_TrueString != nullptr) ? m_Val.m_Bool.m_TrueString : "1";
			else
				*_Str = (m_Val.m_Bool.m_FalseString != nullptr) ? m_Val.m_Bool.m_FalseString : "0";
		} break;
		case TW_TYPE_BOOL8: {
			char Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(char *)m_Ptr;
			if (Val)
				*_Str = (m_Val.m_Bool.m_TrueString != nullptr) ? m_Val.m_Bool.m_TrueString : "1";
			else
				*_Str = (m_Val.m_Bool.m_FalseString != nullptr) ? m_Val.m_Bool.m_FalseString : "0";
		} break;
		case TW_TYPE_BOOL16: {
			short Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(short *)m_Ptr;
			if (Val)
				*_Str = (m_Val.m_Bool.m_TrueString != nullptr) ? m_Val.m_Bool.m_TrueString : "1";
			else
				*_Str = (m_Val.m_Bool.m_FalseString != nullptr) ? m_Val.m_Bool.m_FalseString : "0";
		} break;
		case TW_TYPE_BOOL32: {
			int Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(int *)m_Ptr;
			if (Val)
				*_Str = (m_Val.m_Bool.m_TrueString != nullptr) ? m_Val.m_Bool.m_TrueString : "1";
			else
				*_Str = (m_Val.m_Bool.m_FalseString != nullptr) ? m_Val.m_Bool.m_FalseString : "0";
		} break;
		case TW_TYPE_CHAR: {
			unsigned char Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(unsigned char *)m_Ptr;
			if (Val != 0) {
				int d = Val;
				if (m_Val.m_Char.m_Hexa)
					snprintf(Tmp, 1024, "%c (0x%.2X)", Val, d);
				else
					snprintf(Tmp, 1024, "%c (%d)", Val, d);
				*_Str = Tmp;
			} else {
				*_Str = "  (0)";
				const_cast<char *>(_Str->c_str())[0] = '\0';
			}
		} break;
		case TW_TYPE_INT8: {
			signed char Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(signed char *)m_Ptr;
			int d = Val;
			if (m_Val.m_Int8.m_Hexa)
				snprintf(Tmp, 1024, "0x%.2X", d & 0xff);
			else
				snprintf(Tmp, 1024, "%d", d);
			*_Str = Tmp;
		} break;
		case TW_TYPE_UINT8: {
			unsigned char Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(unsigned char *)m_Ptr;
			unsigned int d = Val;
			if (m_Val.m_UInt8.m_Hexa)
				snprintf(Tmp, 1024, "0x%.2X", d);
			else
				snprintf(Tmp, 1024, "%u", d);
			*_Str = Tmp;
		} break;
		case TW_TYPE_INT16: {
			short Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(short *)m_Ptr;
			int d = Val;
			if (m_Val.m_Int16.m_Hexa)
				snprintf(Tmp, 1024, "0x%.4X", d & 0xffff);
			else
				snprintf(Tmp, 1024, "%d", d);
			*_Str = Tmp;
		} break;
		case TW_TYPE_UINT16: {
			unsigned short Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(unsigned short *)m_Ptr;
			unsigned int d = Val;
			if (m_Val.m_UInt16.m_Hexa)
				snprintf(Tmp, 1024, "0x%.4X", d);
			else
				snprintf(Tmp, 1024, "%u", d);
			*_Str = Tmp;
		} break;
		case TW_TYPE_INT32: {
			int Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(int *)m_Ptr;
			if (m_Val.m_Int32.m_Hexa)
				snprintf(Tmp, 1024, "0x%.8X", Val);
			else
				snprintf(Tmp, 1024, "%d", Val);
			*_Str = Tmp;
		} break;
		case TW_TYPE_UINT32: {
			unsigned int Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(unsigned int *)m_Ptr;
			if (m_Val.m_UInt32.m_Hexa)
				snprintf(Tmp, 1024, "0x%.8X", Val);
			else
				snprintf(Tmp, 1024, "%u", Val);
			*_Str = Tmp;
		} break;
		case TW_TYPE_FLOAT: {
			float Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(float *)m_Ptr;
			if (m_Val.m_Float32.m_Precision < 0)
				snprintf(Tmp, 1024, "%g", Val);
			else {
				char Fmt[64];
				snprintf(Fmt, 64, "%%.%df", (int)m_Val.m_Float32.m_Precision);
				snprintf(Tmp, 1024, Fmt, Val);
			}
			*_Str = Tmp;
		} break;
		case TW_TYPE_DOUBLE: {
			double Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(double *)m_Ptr;
			if (m_Val.m_Float64.m_Precision < 0)
				snprintf(Tmp, 1024, "%g", Val);
			else {
				char Fmt[128];
				snprintf(Fmt, 128, "%%.%dlf", (int)m_Val.m_Float64.m_Precision);
				snprintf(Tmp, 1024, Fmt, Val);
			}
			*_Str = Tmp;
		} break;
		case TW_TYPE_STDSTRING: {
			if (UseGet)
				m_GetCallback(_Str, m_ClientData);
			else
				*_Str = *(std::string *)m_Ptr;
		} break;
		default:
			if (IsEnumType(m_Type)) {
				unsigned int Val = 0;
				if (UseGet)
					m_GetCallback(&Val, m_ClientData);
				else
					Val = *(unsigned int *)m_Ptr;

				CTwMgr::CEnum &e = g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE];
				CTwMgr::CEnum::CEntries::iterator It = e.m_Entries.find(Val);
				if (It != e.m_Entries.end())
					*_Str = It->second;
				else {
					snprintf(Tmp, 1024, "%u", Val);
					*_Str = Tmp;
				}
			} else if (IsCSStringType(m_Type)) {
				char *Val = nullptr;
				if (UseGet) {
					int n = TW_CSSTRING_SIZE(m_Type);
					if (n + 32 > (int)g_TwMgr->m_CSStringBuffer.size())
						g_TwMgr->m_CSStringBuffer.resize(n + 32);
					Val = &(g_TwMgr->m_CSStringBuffer[0]);
					m_GetCallback(Val, m_ClientData);
					Val[n] = '\0';
				} else
					Val = (char *)m_Ptr;
				if (Val != nullptr)
					*_Str = Val;
				else
					*_Str = "";
			} else if (m_Type == TW_TYPE_CDSTRING || m_Type == TW_TYPE_CDSTDSTRING) {
				char *Val = nullptr;
				if (UseGet)
					m_GetCallback(&Val, m_ClientData);
				else
					Val = *(char **)m_Ptr;
				if (Val != nullptr)
					*_Str = Val;
				else
					*_Str = "";
			} else if (IsCustom()) // m_Type>=TW_TYPE_CUSTOM_BASE && m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size() )
			{
				*_Str = "";
			} else {
				*_Str = "unknown type";
				const_cast<CTwVarAtom *>(this)->m_ReadOnly = true;
			}
	}
}

//  ---------------------------------------------------------------------------

double CTwVarAtom::ValueToDouble() const {
	if (m_Ptr == nullptr && m_GetCallback == nullptr)
		return 0; // unreachable
	bool UseGet = (m_GetCallback != nullptr);
	switch (m_Type) {
		case TW_TYPE_BOOLCPP: {
			bool Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(bool *)m_Ptr;
			if (Val)
				return 1;
			else
				return 0;
		} break;
		case TW_TYPE_BOOL8: {
			char Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(char *)m_Ptr;
			if (Val)
				return 1;
			else
				return 0;
		} break;
		case TW_TYPE_BOOL16: {
			short Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(short *)m_Ptr;
			if (Val)
				return 1;
			else
				return 0;
		} break;
		case TW_TYPE_BOOL32: {
			int Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(int *)m_Ptr;
			if (Val)
				return 1;
			else
				return 0;
		} break;
		case TW_TYPE_CHAR: {
			unsigned char Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(unsigned char *)m_Ptr;
			return Val;
		} break;
		case TW_TYPE_INT8: {
			signed char Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(signed char *)m_Ptr;
			int d = Val;
			return d;
		} break;
		case TW_TYPE_UINT8: {
			unsigned char Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(unsigned char *)m_Ptr;
			unsigned int d = Val;
			return d;
		} break;
		case TW_TYPE_INT16: {
			short Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(short *)m_Ptr;
			int d = Val;
			return d;
		} break;
		case TW_TYPE_UINT16: {
			unsigned short Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(unsigned short *)m_Ptr;
			unsigned int d = Val;
			return d;
		} break;
		case TW_TYPE_INT32: {
			int Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(int *)m_Ptr;
			return Val;
		} break;
		case TW_TYPE_UINT32: {
			unsigned int Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(unsigned int *)m_Ptr;
			return Val;
		} break;
		case TW_TYPE_FLOAT: {
			float Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(float *)m_Ptr;
			return Val;
		} break;
		case TW_TYPE_DOUBLE: {
			double Val = 0;
			if (UseGet)
				m_GetCallback(&Val, m_ClientData);
			else
				Val = *(double *)m_Ptr;
			return Val;
		} break;
		default:
			if (IsEnumType(m_Type)) {
				unsigned int Val = 0;
				if (UseGet)
					m_GetCallback(&Val, m_ClientData);
				else
					Val = *(unsigned int *)m_Ptr;
				return Val;
			} else
				return 0; // unknown type
	}
}

//  ---------------------------------------------------------------------------

void CTwVarAtom::ValueFromDouble(double _Val) {
	if (m_Ptr == nullptr && m_SetCallback == nullptr)
		return; // unreachable
	bool UseSet = (m_SetCallback != nullptr);
	switch (m_Type) {
		case TW_TYPE_BOOLCPP: {
			bool Val = (_Val != 0);
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(bool *)m_Ptr = Val;
		} break;
		case TW_TYPE_BOOL8: {
			char Val = (_Val != 0) ? 1 : 0;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(char *)m_Ptr = Val;
		} break;
		case TW_TYPE_BOOL16: {
			short Val = (_Val != 0) ? 1 : 0;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(short *)m_Ptr = Val;
		} break;
		case TW_TYPE_BOOL32: {
			int Val = (_Val != 0) ? 1 : 0;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(int *)m_Ptr = Val;
		} break;
		case TW_TYPE_CHAR: {
			unsigned char Val = (unsigned char)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(unsigned char *)m_Ptr = Val;
		} break;
		case TW_TYPE_INT8: {
			signed char Val = (signed char)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(signed char *)m_Ptr = Val;
		} break;
		// case TW_TYPE_ENUM8:
		case TW_TYPE_UINT8: {
			unsigned char Val = (unsigned char)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(unsigned char *)m_Ptr = Val;
		} break;
		case TW_TYPE_INT16: {
			short Val = (short)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(short *)m_Ptr = Val;
		} break;
		// case TW_TYPE_ENUM16:
		case TW_TYPE_UINT16: {
			unsigned short Val = (unsigned short)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(unsigned short *)m_Ptr = Val;
		} break;
		case TW_TYPE_INT32: {
			int Val = (int)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(int *)m_Ptr = Val;
		} break;
		// case TW_TYPE_ENUM32:
		case TW_TYPE_UINT32: {
			unsigned int Val = (unsigned int)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(unsigned int *)m_Ptr = Val;
		} break;
		case TW_TYPE_FLOAT: {
			float Val = (float)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(float *)m_Ptr = Val;
		} break;
		case TW_TYPE_DOUBLE: {
			double Val = (double)_Val;
			if (UseSet)
				m_SetCallback(&Val, m_ClientData);
			else
				*(double *)m_Ptr = Val;
		} break;
		default:
			if (IsEnumType(m_Type)) {
				unsigned int Val = (unsigned int)_Val;
				if (UseSet)
					m_SetCallback(&Val, m_ClientData);
				else
					*(unsigned int *)m_Ptr = Val;
			}
	}
}

//  ---------------------------------------------------------------------------

void CTwVarAtom::MinMaxStepToDouble(double *_Min, double *_Max, double *_Step) const {
	double max = DOUBLE_MAX;
	double min = -DOUBLE_MAX;
	double step = 1;

	switch (m_Type) {
		case TW_TYPE_BOOLCPP:
		case TW_TYPE_BOOL8:
		case TW_TYPE_BOOL16:
		case TW_TYPE_BOOL32:
			min = 0;
			max = 1;
			step = 1;
			break;
		case TW_TYPE_CHAR:
			min = (double)m_Val.m_Char.m_Min;
			max = (double)m_Val.m_Char.m_Max;
			step = (double)m_Val.m_Char.m_Step;
			break;
		case TW_TYPE_INT8:
			min = (double)m_Val.m_Int8.m_Min;
			max = (double)m_Val.m_Int8.m_Max;
			step = (double)m_Val.m_Int8.m_Step;
			break;
		case TW_TYPE_UINT8:
			min = (double)m_Val.m_UInt8.m_Min;
			max = (double)m_Val.m_UInt8.m_Max;
			step = (double)m_Val.m_UInt8.m_Step;
			break;
		case TW_TYPE_INT16:
			min = (double)m_Val.m_Int16.m_Min;
			max = (double)m_Val.m_Int16.m_Max;
			step = (double)m_Val.m_Int16.m_Step;
			break;
		case TW_TYPE_UINT16:
			min = (double)m_Val.m_UInt16.m_Min;
			max = (double)m_Val.m_UInt16.m_Max;
			step = (double)m_Val.m_UInt16.m_Step;
			break;
		case TW_TYPE_INT32:
			min = (double)m_Val.m_Int32.m_Min;
			max = (double)m_Val.m_Int32.m_Max;
			step = (double)m_Val.m_Int32.m_Step;
			break;
		case TW_TYPE_UINT32:
			min = (double)m_Val.m_UInt32.m_Min;
			max = (double)m_Val.m_UInt32.m_Max;
			step = (double)m_Val.m_UInt32.m_Step;
			break;
		case TW_TYPE_FLOAT:
			min = (double)m_Val.m_Float32.m_Min;
			max = (double)m_Val.m_Float32.m_Max;
			step = (double)m_Val.m_Float32.m_Step;
			break;
		case TW_TYPE_DOUBLE:
			min = m_Val.m_Float64.m_Min;
			max = m_Val.m_Float64.m_Max;
			step = m_Val.m_Float64.m_Step;
			break;
		default: {
		} // nothing
	}

	if (_Min != nullptr)
		*_Min = min;
	if (_Max != nullptr)
		*_Max = max;
	if (_Step != nullptr)
		*_Step = step;
}

//  ---------------------------------------------------------------------------

const CTwVar *CTwVarAtom::Find(const char *_Name, CTwVarGroup **_Parent, int *_Index) const {
	if (strcmp(_Name, m_Name.c_str()) == 0) {
		if (_Parent != nullptr)
			*_Parent = nullptr;
		if (_Index != nullptr)
			*_Index = -1;
		return this;
	} else
		return nullptr;
}

//  ---------------------------------------------------------------------------

enum EVarAttribs {
	V_LABEL = 1,
	V_HELP,
	V_GROUP,
	V_SHOW,
	V_HIDE,
	V_READONLY,
	V_READWRITE,
	V_ORDER,
	V_VISIBLE,
	V_ENDTAG
};

int CTwVar::HasAttrib(const char *_Attrib, bool *_HasValue) const {
	*_HasValue = true;
	if (_stricmp(_Attrib, "label") == 0)
		return V_LABEL;
	else if (_stricmp(_Attrib, "help") == 0)
		return V_HELP;
	else if (_stricmp(_Attrib, "group") == 0)
		return V_GROUP;
	else if (_stricmp(_Attrib, "order") == 0)
		return V_ORDER;
	else if (_stricmp(_Attrib, "visible") == 0)
		return V_VISIBLE;
	else if (_stricmp(_Attrib, "readonly") == 0)
		return V_READONLY;

	// for backward compatibility
	*_HasValue = false;
	if (_stricmp(_Attrib, "show") == 0)
		return V_SHOW;
	else if (_stricmp(_Attrib, "hide") == 0)
		return V_HIDE;
	if (_stricmp(_Attrib, "readonly") == 0)
		return V_READONLY;
	else if (_stricmp(_Attrib, "readwrite") == 0)
		return V_READWRITE;

	return 0; // not found
}

int CTwVar::SetAttrib(int _AttribID, const char *_Value, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex) {
	switch (_AttribID) {
		case V_LABEL:
		case V_HELP:
			if (_Value && strlen(_Value) > 0) {
				{
					CTwVarGroup *Parent = nullptr;
					CTwVar *ThisVar = _Bar->Find(m_Name.c_str(), &Parent);
					if (this == ThisVar && Parent != nullptr && Parent->m_StructValuePtr != nullptr) {
						int Idx = Parent->m_StructType - TW_TYPE_STRUCT_BASE;
						if (Idx >= 0 && Idx < (int)g_TwMgr->m_Structs.size()) {
							size_t nl = m_Name.length();
							for (size_t im = 0; im < g_TwMgr->m_Structs[Idx].m_Members.size(); ++im) {
								size_t ml = g_TwMgr->m_Structs[Idx].m_Members[im].m_Name.length();
								if (nl >= ml && strcmp(g_TwMgr->m_Structs[Idx].m_Members[im].m_Name.c_str(), m_Name.c_str() + (nl - ml)) == 0) {
									// TODO: would have to be applied to other vars already created
									if (_AttribID == V_LABEL) {
										g_TwMgr->m_Structs[Idx].m_Members[im].m_Label = _Value;
										//                                    m_Label = _Value;
									} else // V_HELP
										g_TwMgr->m_Structs[Idx].m_Members[im].m_Help = _Value;
									break;
								}
							}
						}
					} else {
						if (_AttribID == V_LABEL)
							m_Label = _Value;
						else // V_HELP
							m_Help = _Value;
					}
				}
				_Bar->NotUpToDate();
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case V_GROUP: {
			CTwVarGroup *Grp = nullptr;
			if (_Value == nullptr || strlen(_Value) <= 0)
				Grp = &(_Bar->m_VarRoot);
			else {
				CTwVar *v = _Bar->Find(_Value, nullptr, nullptr);
				if (v && !v->IsGroup()) {
					g_TwMgr->SetLastError(g_ErrNotGroup);
					return 0;
				}
				Grp = static_cast<CTwVarGroup *>(v);
				if (Grp == NULL) {
					Grp = new CTwVarGroup;
					Grp->m_Name = _Value;
					Grp->m_Open = true;
					Grp->m_SummaryCallback = NULL;
					Grp->m_SummaryClientData = NULL;
					Grp->m_StructValuePtr = NULL;
					Grp->m_ColorPtr = &(_Bar->m_ColGrpText);
					_Bar->m_VarRoot.m_Vars.push_back(Grp);
				}
			}
			Grp->m_Vars.push_back(this);
			if (_VarParent != NULL && _VarIndex >= 0) {
				_VarParent->m_Vars.erase(_VarParent->m_Vars.begin() + _VarIndex);
				if (_VarParent != &(_Bar->m_VarRoot) && _VarParent->m_Vars.size() <= 0)
					TwRemoveVar(_Bar, _VarParent->m_Name.c_str());
			}
			_Bar->NotUpToDate();
			return 1;
		}
		case V_SHOW: // for backward compatibility
			if (!m_Visible) {
				m_Visible = true;
				_Bar->NotUpToDate();
			}
			return 1;
		case V_HIDE: // for backward compatibility
			if (m_Visible) {
				m_Visible = false;
				_Bar->NotUpToDate();
			}
			return 1;
		// case V_READONLY:
		//   SetReadOnly(true);
		//   _Bar->NotUpToDate();
		//   return 1;
		case V_READWRITE: // for backward compatibility
			SetReadOnly(false);
			_Bar->NotUpToDate();
			return 1;
		case V_ORDER:
			// a special case for compatibility with deprecated command 'option=ogl/dx'
			if (IsGroup() && _Value != NULL && static_cast<CTwVarGroup *>(this)->m_SummaryCallback == CColorExt::SummaryCB && static_cast<CTwVarGroup *>(this)->m_StructValuePtr != nullptr) { // is tw_type_color?
				if (_stricmp(_Value, "ogl") == 0) {
					static_cast<CColorExt *>(static_cast<CTwVarGroup *>(this)->m_StructValuePtr)->m_OGL = true;
					return 1;
				} else if (_stricmp(_Value, "dx") == 0) {
					static_cast<CColorExt *>(static_cast<CTwVarGroup *>(this)->m_StructValuePtr)->m_OGL = false;
					return 1;
				}
			}
			// todo: general 'order' command (no else)
			return 0;
		case V_VISIBLE:
			if (_Value != NULL && strlen(_Value) > 0) {
				if (_stricmp(_Value, "true") == 0 || _stricmp(_Value, "1") == 0) {
					if (!m_Visible) {
						m_Visible = true;
						_Bar->NotUpToDate();
					}
					return 1;
				} else if (_stricmp(_Value, "false") == 0 || _stricmp(_Value, "0") == 0) {
					if (m_Visible) {
						m_Visible = false;
						_Bar->NotUpToDate();
					}
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case V_READONLY:
			if (_Value == NULL || strlen(_Value) == 0 // no value is acceptable (for backward compatibility)
					|| _stricmp(_Value, "true") == 0 || _stricmp(_Value, "1") == 0) {
				if (!IsReadOnly()) {
					SetReadOnly(true);
					_Bar->NotUpToDate();
				}
				return 1;
			} else if (_stricmp(_Value, "false") == 0 || _stricmp(_Value, "0") == 0) {
				if (IsReadOnly()) {
					SetReadOnly(false);
					_Bar->NotUpToDate();
				}
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrBadValue);
				return 0;
			}
		default:
			g_TwMgr->SetLastError(g_ErrUnknownAttrib);
			return 0;
	}
}

ERetType CTwVar::GetAttrib(int _AttribID, TwBar * /*_Bar*/, CTwVarGroup *_VarParent, int /*_VarIndex*/, std::vector<double> &outDoubles, std::ostringstream &outString) const {
	outDoubles.clear();
	outString.clear();

	switch (_AttribID) {
		case V_LABEL:
			outString << m_Label;
			return RET_STRING;
		case V_HELP:
			outString << m_Help;
			return RET_STRING;
		case V_GROUP:
			if (_VarParent != NULL)
				outString << _VarParent->m_Name;
			return RET_STRING;
		case V_VISIBLE:
			outDoubles.push_back(m_Visible ? 1 : 0);
			return RET_DOUBLE;
		case V_READONLY:
			outDoubles.push_back(IsReadOnly() ? 1 : 0);
			return RET_DOUBLE;
		default:
			g_TwMgr->SetLastError(g_ErrUnknownAttrib);
			return RET_ERROR;
	}
}

//  ---------------------------------------------------------------------------

enum EVarAtomAttribs {
	VA_KEY_INCR = V_ENDTAG + 1,
	VA_KEY_DECR,
	VA_MIN,
	VA_MAX,
	VA_STEP,
	VA_PRECISION,
	VA_HEXA,
	VA_DECIMAL, // for backward compatibility
	VA_TRUE,
	VA_FALSE,
	VA_ENUM,
	VA_VALUE
};

int CTwVarAtom::HasAttrib(const char *_Attrib, bool *_HasValue) const {
	*_HasValue = true;
	if (_stricmp(_Attrib, "keyincr") == 0 || _stricmp(_Attrib, "key") == 0)
		return VA_KEY_INCR;
	else if (_stricmp(_Attrib, "keydecr") == 0)
		return VA_KEY_DECR;
	else if (_stricmp(_Attrib, "min") == 0)
		return VA_MIN;
	else if (_stricmp(_Attrib, "max") == 0)
		return VA_MAX;
	else if (_stricmp(_Attrib, "step") == 0)
		return VA_STEP;
	else if (_stricmp(_Attrib, "precision") == 0)
		return VA_PRECISION;
	else if (_stricmp(_Attrib, "hexa") == 0)
		return VA_HEXA;
	else if (_stricmp(_Attrib, "decimal") == 0) { // for backward compatibility
		*_HasValue = false;
		return VA_DECIMAL;
	} else if (_stricmp(_Attrib, "true") == 0)
		return VA_TRUE;
	else if (_stricmp(_Attrib, "false") == 0)
		return VA_FALSE;
	else if (_stricmp(_Attrib, "enum") == 0 || _stricmp(_Attrib, "val") == 0) // for backward compatibility
		return VA_ENUM;
	else if (_stricmp(_Attrib, "value") == 0)
		return VA_VALUE;

	return CTwVar::HasAttrib(_Attrib, _HasValue);
}

int CTwVarAtom::SetAttrib(int _AttribID, const char *_Value, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex) {
	switch (_AttribID) {
		case VA_KEY_INCR: {
			int Key = 0;
			int Mod = 0;
			if (TwGetKeyCode(&Key, &Mod, _Value)) {
				m_KeyIncr[0] = Key;
				m_KeyIncr[1] = Mod;
				return 1;
			} else
				return 0;
		}
		case VA_KEY_DECR: {
			int Key = 0;
			int Mod = 0;
			if (TwGetKeyCode(&Key, &Mod, _Value)) {
				m_KeyDecr[0] = Key;
				m_KeyDecr[1] = Mod;
				return 1;
			} else
				return 0;
		}
		case VA_TRUE:
			if ((m_Type == TW_TYPE_BOOL8 || m_Type == TW_TYPE_BOOL16 || m_Type == TW_TYPE_BOOL32 || m_Type == TW_TYPE_BOOLCPP) && _Value != nullptr) {
				if (m_Val.m_Bool.m_FreeTrueString && m_Val.m_Bool.m_TrueString != nullptr)
					free(m_Val.m_Bool.m_TrueString);
				m_Val.m_Bool.m_TrueString = _strdup(_Value);
				m_Val.m_Bool.m_FreeTrueString = true;
				return 1;
			} else
				return 0;
		case VA_FALSE:
			if ((m_Type == TW_TYPE_BOOL8 || m_Type == TW_TYPE_BOOL16 || m_Type == TW_TYPE_BOOL32 || m_Type == TW_TYPE_BOOLCPP) && _Value != nullptr) {
				if (m_Val.m_Bool.m_FreeFalseString && m_Val.m_Bool.m_FalseString != nullptr)
					free(m_Val.m_Bool.m_FalseString);
				m_Val.m_Bool.m_FalseString = _strdup(_Value);
				m_Val.m_Bool.m_FreeFalseString = true;
				return 1;
			} else
				return 0;
		case VA_MIN:
		case VA_MAX:
		case VA_STEP:
			if (_Value && strlen(_Value) > 0) {
				void *Ptr = nullptr;
				const char *Fmt = nullptr;
				int d = 0;
				unsigned int u = 0;
				int Num = (_AttribID == VA_STEP) ? 2 : ((_AttribID == VA_MAX) ? 1 : 0);
				switch (m_Type) {
					case TW_TYPE_CHAR:
						// Ptr = (&m_Val.m_Char.m_Min) + Num;
						// Fmt = "%c";
						Ptr = &u;
						Fmt = "%u";
						break;
					case TW_TYPE_INT16:
						Ptr = (&m_Val.m_Int16.m_Min) + Num;
						Fmt = "%hd";
						break;
					case TW_TYPE_INT32:
						Ptr = (&m_Val.m_Int32.m_Min) + Num;
						Fmt = "%d";
						break;
					case TW_TYPE_UINT16:
						Ptr = (&m_Val.m_UInt16.m_Min) + Num;
						Fmt = "%hu";
						break;
					case TW_TYPE_UINT32:
						Ptr = (&m_Val.m_UInt32.m_Min) + Num;
						Fmt = "%u";
						break;
					case TW_TYPE_FLOAT:
						Ptr = (&m_Val.m_Float32.m_Min) + Num;
						Fmt = "%f";
						break;
					case TW_TYPE_DOUBLE:
						Ptr = (&m_Val.m_Float64.m_Min) + Num;
						Fmt = "%lf";
						break;
					case TW_TYPE_INT8:
						Ptr = &d;
						Fmt = "%d";
						break;
					case TW_TYPE_UINT8:
						Ptr = &u;
						Fmt = "%u";
						break;
					default:
						g_TwMgr->SetLastError(g_ErrUnknownType);
						return 0;
				}

				if (Fmt != NULL && Ptr != NULL && sscanf(_Value, Fmt, Ptr) == 1) {
					if (m_Type == TW_TYPE_CHAR)
						*((&m_Val.m_Char.m_Min) + Num) = (unsigned char)(u);
					else if (m_Type == TW_TYPE_INT8)
						*((&m_Val.m_Int8.m_Min) + Num) = (signed char)(d);
					else if (m_Type == TW_TYPE_UINT8)
						*((&m_Val.m_UInt8.m_Min) + Num) = (unsigned char)(u);

					// set precision
					if (_AttribID == VA_STEP && ((m_Type == TW_TYPE_FLOAT && m_Val.m_Float32.m_Precision < 0) || (m_Type == TW_TYPE_DOUBLE && m_Val.m_Float64.m_Precision < 0))) {
						double Step = fabs((m_Type == TW_TYPE_FLOAT) ? m_Val.m_Float32.m_Step : m_Val.m_Float64.m_Step);
						signed char *Precision = (m_Type == TW_TYPE_FLOAT) ? &m_Val.m_Float32.m_Precision : &m_Val.m_Float64.m_Precision;
						const double K_EPS = 1.0 - 1.0e-6;
						if (Step >= 1)
							*Precision = 0;
						else if (Step >= 0.1 * K_EPS)
							*Precision = 1;
						else if (Step >= 0.01 * K_EPS)
							*Precision = 2;
						else if (Step >= 0.001 * K_EPS)
							*Precision = 3;
						else if (Step >= 0.0001 * K_EPS)
							*Precision = 4;
						else if (Step >= 0.00001 * K_EPS)
							*Precision = 5;
						else if (Step >= 0.000001 * K_EPS)
							*Precision = 6;
						else if (Step >= 0.0000001 * K_EPS)
							*Precision = 7;
						else if (Step >= 0.00000001 * K_EPS)
							*Precision = 8;
						else if (Step >= 0.000000001 * K_EPS)
							*Precision = 9;
						else if (Step >= 0.0000000001 * K_EPS)
							*Precision = 10;
						else if (Step >= 0.00000000001 * K_EPS)
							*Precision = 11;
						else if (Step >= 0.000000000001 * K_EPS)
							*Precision = 12;
						else
							*Precision = -1;
					}

					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case VA_PRECISION:
			if (_Value && strlen(_Value) > 0) {
				int Precision = 0;
				if (sscanf(_Value, "%d", &Precision) == 1 && Precision >= -1 && Precision <= 12) {
					if (m_Type == TW_TYPE_FLOAT)
						m_Val.m_Float32.m_Precision = (signed char)Precision;
					else if (m_Type == TW_TYPE_DOUBLE)
						m_Val.m_Float64.m_Precision = (signed char)Precision;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case VA_HEXA:
		case VA_DECIMAL: {
			bool hexa = false;
			if (_AttribID == VA_HEXA) {
				if (_Value == NULL || strlen(_Value) == 0 // no value is acceptable (for backward compatibility)
						|| _stricmp(_Value, "true") == 0 || _stricmp(_Value, "1") == 0)
					hexa = true;
			}

			switch (m_Type) {
				case TW_TYPE_CHAR:
					m_Val.m_Char.m_Hexa = hexa;
					return 1;
				case TW_TYPE_INT8:
					m_Val.m_Int8.m_Hexa = hexa;
					return 1;
				case TW_TYPE_INT16:
					m_Val.m_Int16.m_Hexa = hexa;
					return 1;
				case TW_TYPE_INT32:
					m_Val.m_Int32.m_Hexa = hexa;
					return 1;
				case TW_TYPE_UINT8:
					m_Val.m_UInt8.m_Hexa = hexa;
					return 1;
				case TW_TYPE_UINT16:
					m_Val.m_UInt16.m_Hexa = hexa;
					return 1;
				case TW_TYPE_UINT32:
					m_Val.m_UInt32.m_Hexa = hexa;
					return 1;
				default:
					return 0;
			}
		}
		case VA_ENUM:
			if (_Value && strlen(_Value) > 0 && IsEnumType(m_Type)) {
				const char *s = _Value;
				int n = 0, i = 0;
				unsigned int u;
				bool Cont;
				g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE].m_Entries.clear(); // anyway reset entries
				do {
					Cont = false;
					i = 0;
					char Sep;
					n = sscanf(s, "%u %c%n", &u, &Sep, &i);
					if (n == 2 && i > 0 && (Sep == '<' || Sep == '{' || Sep == '[' || Sep == '(')) {
						if (Sep == '<') // Change to closing separator
							Sep = '>';
						else if (Sep == '{')
							Sep = '}';
						else if (Sep == '[')
							Sep = ']';
						else if (Sep == '(')
							Sep = ')';
						s += i;
						i = 0;
						while (s[i] != Sep && s[i] != 0)
							++i;
						if (s[i] == Sep) {
							// if( m_Val.m_Enum.m_Entries==NULL )
							//   m_Val.m_Enum.m_Entries = new UVal::CEnumVal::CEntries;
							// UVal::CEnumVal::CEntries::value_type v(u, "");
							CTwMgr::CEnum::CEntries::value_type v(u, "");
							if (i > 0)
								v.second.assign(s, i);
							// m_Val.m_Enum.m_Entries->insert(v);
							std::pair<CTwMgr::CEnum::CEntries::iterator, bool> ret;
							ret = g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE].m_Entries.insert(v);
							if (!ret.second) { // force overwrite if element already exists
								g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE].m_Entries.erase(ret.first);
								g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE].m_Entries.insert(v);
							}

							s += i + 1;
							i = 0;
							n = sscanf(s, " ,%n", &i);
							if (n == 0 && i >= 1) {
								s += i;
								Cont = true;
							}
						} else {
							g_TwMgr->SetLastError(g_ErrBadValue);
							return 0;
						}
					} else {
						g_TwMgr->SetLastError(g_ErrBadValue);
						return 0;
					}
				} while (Cont);
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
			break;
		case VA_VALUE:
			if (_Value != NULL && strlen(_Value) > 0) { // do not check ReadOnly here.
				if (!(m_Type == TW_TYPE_BUTTON || IsCustom())) { // || (m_Type>=TW_TYPE_CUSTOM_BASE && m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size()) ) )
					if (m_Type == TW_TYPE_CDSTRING || m_Type == TW_TYPE_CDSTDSTRING) {
						if (m_SetCallback != NULL) {
							m_SetCallback(&_Value, m_ClientData);
							if (g_TwMgr != NULL) // Mgr might have been destroyed by the client inside a callback call
								_Bar->NotUpToDate();
							return 1;
						} else if (m_Type != TW_TYPE_CDSTDSTRING) {
							char **StringPtr = (char **)m_Ptr;
							if (StringPtr != NULL && g_TwMgr->m_CopyCDStringToClient != NULL) {
								g_TwMgr->m_CopyCDStringToClient(StringPtr, _Value);
								_Bar->NotUpToDate();
								return 1;
							}
						}
					} else if (IsCSStringType(m_Type)) {
						int n = TW_CSSTRING_SIZE(m_Type);
						if (n > 0) {
							std::string str = _Value;
							if ((int)str.length() > n - 1)
								str.resize(n - 1);
							if (m_SetCallback != NULL) {
								m_SetCallback(str.c_str(), m_ClientData);
								if (g_TwMgr != NULL) // Mgr might have been destroyed by the client inside a callback call
									_Bar->NotUpToDate();
								return 1;
							} else if (m_Ptr != NULL) {
								if (n > 1)
									strncpy((char *)m_Ptr, str.c_str(), n - 1);
								((char *)m_Ptr)[n - 1] = '\0';
								_Bar->NotUpToDate();
								return 1;
							}
						}
					} else {
						double dbl;
						if (sscanf(_Value, "%lf", &dbl) == 1) {
							ValueFromDouble(dbl);
							if (g_TwMgr != NULL) // Mgr might have been destroyed by the client inside a callback call
								_Bar->NotUpToDate();
							return 1;
						}
					}
				}
			}
			return 0;
		default:
			return CTwVar::SetAttrib(_AttribID, _Value, _Bar, _VarParent, _VarIndex);
	}
}

ERetType CTwVarAtom::GetAttrib(int _AttribID, TwBar *_Bar, CTwVarGroup *_VarParent, int _VarIndex, std::vector<double> &outDoubles, std::ostringstream &outString) const {
	outDoubles.clear();
	outString.clear();
	std::string str;
	int num = 0;

	switch (_AttribID) {
		case VA_KEY_INCR:
			if (TwGetKeyString(&str, m_KeyIncr[0], m_KeyIncr[1]))
				outString << str;
			return RET_STRING;
		case VA_KEY_DECR:
			if (TwGetKeyString(&str, m_KeyDecr[0], m_KeyDecr[1]))
				outString << str;
			return RET_STRING;
		case VA_TRUE:
			if (m_Type == TW_TYPE_BOOL8 || m_Type == TW_TYPE_BOOL16 || m_Type == TW_TYPE_BOOL32 || m_Type == TW_TYPE_BOOLCPP) {
				outString << m_Val.m_Bool.m_TrueString;
				return RET_STRING;
			} else {
				g_TwMgr->SetLastError(g_ErrInvalidAttrib);
				return RET_ERROR;
			}
		case VA_FALSE:
			if (m_Type == TW_TYPE_BOOL8 || m_Type == TW_TYPE_BOOL16 || m_Type == TW_TYPE_BOOL32 || m_Type == TW_TYPE_BOOLCPP) {
				outString << m_Val.m_Bool.m_FalseString;
				return RET_STRING;
			} else {
				g_TwMgr->SetLastError(g_ErrInvalidAttrib);
				return RET_ERROR;
			}
		case VA_MIN:
		case VA_MAX:
		case VA_STEP:
			num = (_AttribID == VA_STEP) ? 2 : ((_AttribID == VA_MAX) ? 1 : 0);
			switch (m_Type) {
				case TW_TYPE_CHAR:
					outDoubles.push_back(*((&m_Val.m_Char.m_Min) + num));
					return RET_DOUBLE;
				case TW_TYPE_INT8:
					outDoubles.push_back(*((&m_Val.m_Int8.m_Min) + num));
					return RET_DOUBLE;
				case TW_TYPE_UINT8:
					outDoubles.push_back(*((&m_Val.m_UInt8.m_Min) + num));
					return RET_DOUBLE;
				case TW_TYPE_INT16:
					outDoubles.push_back(*((&m_Val.m_Int16.m_Min) + num));
					return RET_DOUBLE;
				case TW_TYPE_INT32:
					outDoubles.push_back(*((&m_Val.m_Int32.m_Min) + num));
					return RET_DOUBLE;
				case TW_TYPE_UINT16:
					outDoubles.push_back(*((&m_Val.m_UInt16.m_Min) + num));
					return RET_DOUBLE;
				case TW_TYPE_UINT32:
					outDoubles.push_back(*((&m_Val.m_UInt32.m_Min) + num));
					return RET_DOUBLE;
				case TW_TYPE_FLOAT:
					outDoubles.push_back(*((&m_Val.m_Float32.m_Min) + num));
					return RET_DOUBLE;
				case TW_TYPE_DOUBLE:
					outDoubles.push_back(*((&m_Val.m_Float64.m_Min) + num));
					return RET_DOUBLE;
				default:
					g_TwMgr->SetLastError(g_ErrInvalidAttrib);
					return RET_ERROR;
			}
		case VA_PRECISION:
			if (m_Type == TW_TYPE_FLOAT) {
				outDoubles.push_back(m_Val.m_Float32.m_Precision);
				return RET_DOUBLE;
			} else if (m_Type == TW_TYPE_DOUBLE) {
				outDoubles.push_back(m_Val.m_Float64.m_Precision);
				return RET_DOUBLE;
			} else {
				g_TwMgr->SetLastError(g_ErrInvalidAttrib);
				return RET_ERROR;
			}
		case VA_HEXA:
			switch (m_Type) {
				case TW_TYPE_CHAR:
					outDoubles.push_back(m_Val.m_Char.m_Hexa);
					return RET_DOUBLE;
				case TW_TYPE_INT8:
					outDoubles.push_back(m_Val.m_Int8.m_Hexa);
					return RET_DOUBLE;
				case TW_TYPE_INT16:
					outDoubles.push_back(m_Val.m_Int16.m_Hexa);
					return RET_DOUBLE;
				case TW_TYPE_INT32:
					outDoubles.push_back(m_Val.m_Int32.m_Hexa);
					return RET_DOUBLE;
				case TW_TYPE_UINT8:
					outDoubles.push_back(m_Val.m_UInt8.m_Hexa);
					return RET_DOUBLE;
				case TW_TYPE_UINT16:
					outDoubles.push_back(m_Val.m_UInt16.m_Hexa);
					return RET_DOUBLE;
				case TW_TYPE_UINT32:
					outDoubles.push_back(m_Val.m_UInt32.m_Hexa);
					return RET_DOUBLE;
				default:
					g_TwMgr->SetLastError(g_ErrInvalidAttrib);
					return RET_ERROR;
			}
		case VA_ENUM:
			if (IsEnumType(m_Type)) {
				CTwMgr::CEnum::CEntries::iterator it = g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE].m_Entries.begin();
				for (; it != g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE].m_Entries.end(); ++it) {
					if (it != g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE].m_Entries.begin())
						outString << ',';
					outString << it->first << ' ';
					if (it->second.find_first_of("{}") == std::string::npos)
						outString << '{' << it->second << '}';
					else if (it->second.find_first_of("<>") == std::string::npos)
						outString << '<' << it->second << '>';
					else if (it->second.find_first_of("()") == std::string::npos)
						outString << '(' << it->second << ')';
					else if (it->second.find_first_of("[]") == std::string::npos)
						outString << '[' << it->second << ']';
					else
						outString << '{' << it->second << '}'; // should not occured (use braces)
				}
				return RET_STRING;
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		case VA_VALUE:
			if (!(m_Type == TW_TYPE_BUTTON || IsCustom())) { // || (m_Type>=TW_TYPE_CUSTOM_BASE && m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size()) ) )
				if (m_Type == TW_TYPE_CDSTRING || m_Type == TW_TYPE_CDSTDSTRING || IsCSStringType(m_Type)) {
					std::string str;
					ValueToString(&str);
					outString << str;
					return RET_STRING;
				} else {
					outDoubles.push_back(ValueToDouble());
					return RET_DOUBLE;
				}
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		default:
			return CTwVar::GetAttrib(_AttribID, _Bar, _VarParent, _VarIndex, outDoubles, outString);
	}
}

//  ---------------------------------------------------------------------------

void CTwVarAtom::Increment(int _Step) {
	if (_Step == 0)
		return;
	switch (m_Type) {
		case TW_TYPE_BOOL8: {
			char v = false;
			if (m_Ptr != nullptr)
				v = *((char *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			if (v)
				v = false;
			else
				v = true;
			if (m_Ptr != nullptr)
				*((char *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_BOOL16: {
			short v = false;
			if (m_Ptr != nullptr)
				v = *((short *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			if (v)
				v = false;
			else
				v = true;
			if (m_Ptr != nullptr)
				*((short *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_BOOL32: {
			int v = false;
			if (m_Ptr != nullptr)
				v = *((int *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			if (v)
				v = false;
			else
				v = true;
			if (m_Ptr != nullptr)
				*((int *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_BOOLCPP: {
			bool v = false;
			if (m_Ptr != nullptr)
				v = *((bool *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			if (v)
				v = false;
			else
				v = true;
			if (m_Ptr != nullptr)
				*((bool *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_CHAR: {
			unsigned char v = 0;
			if (m_Ptr != nullptr)
				v = *((unsigned char *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			int iv = _Step * (int)m_Val.m_Char.m_Step + (int)v;
			if (iv < m_Val.m_Char.m_Min)
				iv = m_Val.m_Char.m_Min;
			if (iv > m_Val.m_Char.m_Max)
				iv = m_Val.m_Char.m_Max;
			if (iv < 0)
				iv = 0;
			else if (iv > 0xff)
				iv = 0xff;
			v = (unsigned char)iv;
			if (m_Ptr != nullptr)
				*((unsigned char *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_INT8: {
			signed char v = 0;
			if (m_Ptr != nullptr)
				v = *((signed char *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			int iv = _Step * (int)m_Val.m_Int8.m_Step + (int)v;
			if (iv < m_Val.m_Int8.m_Min)
				iv = m_Val.m_Int8.m_Min;
			if (iv > m_Val.m_Int8.m_Max)
				iv = m_Val.m_Int8.m_Max;
			v = (signed char)iv;
			if (m_Ptr != nullptr)
				*((signed char *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_UINT8: {
			unsigned char v = 0;
			if (m_Ptr != nullptr)
				v = *((unsigned char *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			int iv = _Step * (int)m_Val.m_UInt8.m_Step + (int)v;
			if (iv < m_Val.m_UInt8.m_Min)
				iv = m_Val.m_UInt8.m_Min;
			if (iv > m_Val.m_UInt8.m_Max)
				iv = m_Val.m_UInt8.m_Max;
			if (iv < 0)
				iv = 0;
			else if (iv > 0xff)
				iv = 0xff;
			v = (unsigned char)iv;
			if (m_Ptr != nullptr)
				*((unsigned char *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_INT16: {
			short v = 0;
			if (m_Ptr != nullptr)
				v = *((short *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			int iv = _Step * (int)m_Val.m_Int16.m_Step + (int)v;
			if (iv < m_Val.m_Int16.m_Min)
				iv = m_Val.m_Int16.m_Min;
			if (iv > m_Val.m_Int16.m_Max)
				iv = m_Val.m_Int16.m_Max;
			v = (short)iv;
			if (m_Ptr != nullptr)
				*((short *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_UINT16: {
			unsigned short v = 0;
			if (m_Ptr != nullptr)
				v = *((unsigned short *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			int iv = _Step * (int)m_Val.m_UInt16.m_Step + (int)v;
			if (iv < m_Val.m_UInt16.m_Min)
				iv = m_Val.m_UInt16.m_Min;
			if (iv > m_Val.m_UInt16.m_Max)
				iv = m_Val.m_UInt16.m_Max;
			if (iv < 0)
				iv = 0;
			else if (iv > 0xffff)
				iv = 0xffff;
			v = (unsigned short)iv;
			if (m_Ptr != nullptr)
				*((unsigned short *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_INT32: {
			int v = 0;
			if (m_Ptr != nullptr)
				v = *((int *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			double dv = (double)_Step * (double)m_Val.m_Int32.m_Step + (double)v;
			if (dv > (double)0x7fffffff)
				v = 0x7fffffff;
			else if (dv < (double)(-0x7fffffff - 1))
				v = -0x7fffffff - 1;
			else
				v = _Step * m_Val.m_Int32.m_Step + v;
			if (v < m_Val.m_Int32.m_Min)
				v = m_Val.m_Int32.m_Min;
			if (v > m_Val.m_Int32.m_Max)
				v = m_Val.m_Int32.m_Max;
			if (m_Ptr != nullptr)
				*((int *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_UINT32: {
			unsigned int v = 0;
			if (m_Ptr != nullptr)
				v = *((unsigned int *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			double dv = (double)_Step * (double)m_Val.m_UInt32.m_Step + (double)v;
			if (dv > (double)0xffffffff)
				v = 0xffffffff;
			else if (dv < 0)
				v = 0;
			else
				v = _Step * m_Val.m_UInt32.m_Step + v;
			if (v < m_Val.m_UInt32.m_Min)
				v = m_Val.m_UInt32.m_Min;
			if (v > m_Val.m_UInt32.m_Max)
				v = m_Val.m_UInt32.m_Max;
			if (m_Ptr != nullptr)
				*((unsigned int *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_FLOAT: {
			float v = 0;
			if (m_Ptr != nullptr)
				v = *((float *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			v += _Step * m_Val.m_Float32.m_Step;
			if (v < m_Val.m_Float32.m_Min)
				v = m_Val.m_Float32.m_Min;
			if (v > m_Val.m_Float32.m_Max)
				v = m_Val.m_Float32.m_Max;
			if (m_Ptr != nullptr)
				*((float *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		case TW_TYPE_DOUBLE: {
			double v = 0;
			if (m_Ptr != nullptr)
				v = *((double *)m_Ptr);
			else if (m_GetCallback != nullptr)
				m_GetCallback(&v, m_ClientData);
			v += _Step * m_Val.m_Float64.m_Step;
			if (v < m_Val.m_Float64.m_Min)
				v = m_Val.m_Float64.m_Min;
			if (v > m_Val.m_Float64.m_Max)
				v = m_Val.m_Float64.m_Max;
			if (m_Ptr != nullptr)
				*((double *)m_Ptr) = v;
			else if (m_SetCallback != nullptr)
				m_SetCallback(&v, m_ClientData);
		} break;
		default:
			if (m_Type == TW_TYPE_BUTTON) {
				if (m_Val.m_Button.m_Callback != NULL) {
					m_Val.m_Button.m_Callback(m_ClientData);
					if (g_TwMgr == NULL) // Mgr might have been destroyed by the client inside a callback call
						return;
				}
			} else if (IsEnumType(m_Type)) {
				assert(_Step == 1 || _Step == -1);
				unsigned int v = 0;
				if (m_Ptr != NULL)
					v = *((unsigned int *)m_Ptr);
				else if (m_GetCallback != NULL)
					m_GetCallback(&v, m_ClientData);
				CTwMgr::CEnum &e = g_TwMgr->m_Enums[m_Type - TW_TYPE_ENUM_BASE];
				CTwMgr::CEnum::CEntries::iterator It = e.m_Entries.find(v);
				if (It == e.m_Entries.end())
					It = e.m_Entries.begin();
				else if (_Step == 1) {
					++It;
					if (It == e.m_Entries.end())
						It = e.m_Entries.begin();
				} else if (_Step == -1) {
					if (It == e.m_Entries.begin())
						It = e.m_Entries.end();
					if (It != e.m_Entries.begin())
						--It;
				}
				if (It != e.m_Entries.end()) {
					v = (unsigned int)(It->first);
					if (m_Ptr != NULL)
						*((unsigned int *)m_Ptr) = v;
					else if (m_SetCallback != NULL)
						m_SetCallback(&v, m_ClientData);
				}
			} else
				fprintf(stderr, "CTwVarAtom::Increment : unknown or unimplemented type\n");
	}
}

//  ---------------------------------------------------------------------------

void CTwVarAtom::SetDefaults() {
	switch (m_Type) {
		case TW_TYPE_BOOL8:
		case TW_TYPE_BOOL16:
		case TW_TYPE_BOOL32:
		case TW_TYPE_BOOLCPP:
			m_NoSlider = true;
			break;
		case TW_TYPE_CHAR:
			m_Val.m_Char.m_Max = 0xff;
			m_Val.m_Char.m_Min = 0;
			m_Val.m_Char.m_Step = 1;
			m_Val.m_Char.m_Precision = -1;
			m_Val.m_Char.m_Hexa = false;
			break;
		case TW_TYPE_INT8:
			m_Val.m_Int8.m_Max = 0x7f;
			m_Val.m_Int8.m_Min = -m_Val.m_Int8.m_Max - 1;
			m_Val.m_Int8.m_Step = 1;
			m_Val.m_Int8.m_Precision = -1;
			m_Val.m_Int8.m_Hexa = false;
			break;
		case TW_TYPE_UINT8:
			m_Val.m_UInt8.m_Max = 0xff;
			m_Val.m_UInt8.m_Min = 0;
			m_Val.m_UInt8.m_Step = 1;
			m_Val.m_UInt8.m_Precision = -1;
			m_Val.m_UInt8.m_Hexa = false;
			break;
		case TW_TYPE_INT16:
			m_Val.m_Int16.m_Max = 0x7fff;
			m_Val.m_Int16.m_Min = -m_Val.m_Int16.m_Max - 1;
			m_Val.m_Int16.m_Step = 1;
			m_Val.m_Int16.m_Precision = -1;
			m_Val.m_Int16.m_Hexa = false;
			break;
		case TW_TYPE_UINT16:
			m_Val.m_UInt16.m_Max = 0xffff;
			m_Val.m_UInt16.m_Min = 0;
			m_Val.m_UInt16.m_Step = 1;
			m_Val.m_UInt16.m_Precision = -1;
			m_Val.m_UInt16.m_Hexa = false;
			break;
		case TW_TYPE_INT32:
			m_Val.m_Int32.m_Max = 0x7fffffff;
			m_Val.m_Int32.m_Min = -m_Val.m_Int32.m_Max - 1;
			m_Val.m_Int32.m_Step = 1;
			m_Val.m_Int32.m_Precision = -1;
			m_Val.m_Int32.m_Hexa = false;
			break;
		case TW_TYPE_UINT32:
			m_Val.m_UInt32.m_Max = 0xffffffff;
			m_Val.m_UInt32.m_Min = 0;
			m_Val.m_UInt32.m_Step = 1;
			m_Val.m_UInt32.m_Precision = -1;
			m_Val.m_UInt32.m_Hexa = false;
			break;
		case TW_TYPE_FLOAT:
			m_Val.m_Float32.m_Max = FLOAT_MAX;
			m_Val.m_Float32.m_Min = -FLOAT_MAX;
			m_Val.m_Float32.m_Step = 1;
			m_Val.m_Float32.m_Precision = -1;
			m_Val.m_Float32.m_Hexa = false;
			break;
		case TW_TYPE_DOUBLE:
			m_Val.m_Float64.m_Max = DOUBLE_MAX;
			m_Val.m_Float64.m_Min = -DOUBLE_MAX;
			m_Val.m_Float64.m_Step = 1;
			m_Val.m_Float64.m_Precision = -1;
			m_Val.m_Float64.m_Hexa = false;
			break;
		case TW_TYPE_CDSTRING:
		case TW_TYPE_STDSTRING:
			m_NoSlider = true;
			break;
		default: {
		} // nothing
	}

	// special types
	if (m_Type == TW_TYPE_BUTTON || IsEnumType(m_Type) // (m_Type>=TW_TYPE_ENUM_BASE && m_Type<TW_TYPE_ENUM_BASE+(int)g_TwMgr->m_Enums.size())
			|| IsCSStringType(m_Type) // (m_Type>=TW_TYPE_CSSTRING_BASE && m_Type<=TW_TYPE_CSSTRING_MAX)
			|| m_Type == TW_TYPE_CDSTDSTRING || IsCustom()) // (m_Type>=TW_TYPE_CUSTOM_BASE && m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size()) )
		m_NoSlider = true;
}

//  ---------------------------------------------------------------------------

enum EVarGroupAttribs {
	VG_OPEN = V_ENDTAG + 1, // for backward compatibility
	VG_CLOSE, // for backward compatibility
	VG_OPENED,
	VG_TYPEID, // used internally for structs
	VG_VALPTR, // used internally for structs
	VG_ALPHA, // for backward compatibility
	VG_NOALPHA, // for backward compatibility
	VG_COLORALPHA, // tw_type_color* only
	VG_HLS, // for backward compatibility
	VG_RGB, // for backward compatibility
	VG_COLORMODE, // tw_type_color* only
	VG_COLORORDER, // tw_type_color* only
	VG_ARROW, // tw_type_quat* only
	VG_ARROWCOLOR, // tw_type_quat* only
	VG_AXISX, // tw_type_quat* only
	VG_AXISY, // tw_type_quat* only
	VG_AXISZ, // tw_type_quat* only
	VG_SHOWVAL, // tw_type_quat* only
};

int CTwVarGroup::HasAttrib(const char *_Attrib, bool *_HasValue) const {
	*_HasValue = false;
	if (_stricmp(_Attrib, "open") == 0) // for backward compatibility
		return VG_OPEN;
	else if (_stricmp(_Attrib, "close") == 0) // for backward compatibility
		return VG_CLOSE;
	else if (_stricmp(_Attrib, "opened") == 0) {
		*_HasValue = true;
		return VG_OPENED;
	} else if (_stricmp(_Attrib, "typeid") == 0) {
		*_HasValue = true;
		return VG_TYPEID;
	} else if (_stricmp(_Attrib, "valptr") == 0) {
		*_HasValue = true;
		return VG_VALPTR;
	} else if (_stricmp(_Attrib, "alpha") == 0) // for backward compatibility
		return VG_ALPHA;
	else if (_stricmp(_Attrib, "noalpha") == 0) // for backward compatibility
		return VG_NOALPHA;
	else if (_stricmp(_Attrib, "coloralpha") == 0) {
		*_HasValue = true;
		return VG_COLORALPHA;
	} else if (_stricmp(_Attrib, "hls") == 0) // for backward compatibility
		return VG_HLS;
	else if (_stricmp(_Attrib, "rgb") == 0) // for backward compatibility
		return VG_RGB;
	else if (_stricmp(_Attrib, "colormode") == 0) {
		*_HasValue = true;
		return VG_COLORMODE;
	} else if (_stricmp(_Attrib, "colororder") == 0) {
		*_HasValue = true;
		return VG_COLORORDER;
	} else if (_stricmp(_Attrib, "arrow") == 0) {
		*_HasValue = true;
		return VG_ARROW;
	} else if (_stricmp(_Attrib, "arrowcolor") == 0) {
		*_HasValue = true;
		return VG_ARROWCOLOR;
	} else if (_stricmp(_Attrib, "axisx") == 0) {
		*_HasValue = true;
		return VG_AXISX;
	} else if (_stricmp(_Attrib, "axisy") == 0) {
		*_HasValue = true;
		return VG_AXISY;
	} else if (_stricmp(_Attrib, "axisz") == 0) {
		*_HasValue = true;
		return VG_AXISZ;
	} else if (_stricmp(_Attrib, "showval") == 0) {
		*_HasValue = true;
		return VG_SHOWVAL;
	}

	return CTwVar::HasAttrib(_Attrib, _HasValue);
}

int CTwVarGroup::SetAttrib(int _AttribID, const char *_Value, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex) {
	switch (_AttribID) {
		case VG_OPEN: // for backward compatibility
			if (!m_Open) {
				m_Open = true;
				_Bar->NotUpToDate();
			}
			return 1;
		case VG_CLOSE: // for backward compatibility
			if (m_Open) {
				m_Open = false;
				_Bar->NotUpToDate();
			}
			return 1;
		case VG_OPENED:
			if (_Value != NULL && strlen(_Value) > 0) {
				if (_stricmp(_Value, "true") == 0 || _stricmp(_Value, "1") == 0) {
					if (!m_Open) {
						m_Open = true;
						_Bar->NotUpToDate();
					}
					return 1;
				} else if (_stricmp(_Value, "false") == 0 || _stricmp(_Value, "0") == 0) {
					if (m_Open) {
						m_Open = false;
						_Bar->NotUpToDate();
					}
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case VG_TYPEID: {
			int type = TW_TYPE_UNDEF;
			if (_Value != NULL && sscanf(_Value, "%d", &type) == 1) {
				int idx = type - TW_TYPE_STRUCT_BASE;
				if (idx >= 0 && idx < (int)g_TwMgr->m_Structs.size()) {
					m_SummaryCallback = g_TwMgr->m_Structs[idx].m_SummaryCallback;
					m_SummaryClientData = g_TwMgr->m_Structs[idx].m_SummaryClientData;
					m_StructType = (TwType)type;
					return 1;
				}
			}
			return 0;
		}
		case VG_VALPTR: {
			void *structValuePtr = NULL;
			if (_Value != NULL && sscanf(_Value, "%p", &structValuePtr) == 1) {
				m_StructValuePtr = structValuePtr;
				m_ColorPtr = &(_Bar->m_ColStructText);
				return 1;
			}
			return 0;
		}
		case VG_ALPHA: // for backward compatibility
			if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_color?
				if (static_cast<CColorExt *>(m_StructValuePtr)->m_CanHaveAlpha) {
					static_cast<CColorExt *>(m_StructValuePtr)->m_HasAlpha = true;
					_Bar->NotUpToDate();
					return 1;
				}
			return 0;
		case VG_NOALPHA: // for backward compatibility
			if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_color?
			{
				static_cast<CColorExt *>(m_StructValuePtr)->m_HasAlpha = false;
				_Bar->NotUpToDate();
				return 1;
			} else
				return 0;
		case VG_COLORALPHA:
			if (_Value != NULL && strlen(_Value) > 0) {
				if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_color?
				{
					if (_stricmp(_Value, "true") == 0 || _stricmp(_Value, "1") == 0) {
						if (static_cast<CColorExt *>(m_StructValuePtr)->m_CanHaveAlpha) {
							if (!static_cast<CColorExt *>(m_StructValuePtr)->m_HasAlpha) {
								static_cast<CColorExt *>(m_StructValuePtr)->m_HasAlpha = true;
								_Bar->NotUpToDate();
							}
							return 1;
						}
					} else if (_stricmp(_Value, "false") == 0 || _stricmp(_Value, "0") == 0) {
						if (static_cast<CColorExt *>(m_StructValuePtr)->m_HasAlpha) {
							static_cast<CColorExt *>(m_StructValuePtr)->m_HasAlpha = false;
							_Bar->NotUpToDate();
						}
						return 1;
					}
				}
			}
			return 0;
		case VG_HLS: // for backward compatibility
			if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_color?
			{
				static_cast<CColorExt *>(m_StructValuePtr)->m_HLS = true;
				_Bar->NotUpToDate();
				return 1;
			} else
				return 0;
		case VG_RGB: // for backward compatibility
			if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != nullptr) // is tw_type_color?
			{
				static_cast<CColorExt *>(m_StructValuePtr)->m_HLS = false;
				_Bar->NotUpToDate();
				return 1;
			} else
				return 0;
		case VG_COLORMODE:
			if (_Value != nullptr && strlen(_Value) > 0) {
				if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != nullptr) // is tw_type_color?
				{
					if (_stricmp(_Value, "hls") == 0) {
						if (!static_cast<CColorExt *>(m_StructValuePtr)->m_HLS) {
							static_cast<CColorExt *>(m_StructValuePtr)->m_HLS = true;
							_Bar->NotUpToDate();
						}
						return 1;
					} else if (_stricmp(_Value, "rgb") == 0) {
						if (static_cast<CColorExt *>(m_StructValuePtr)->m_HLS) {
							static_cast<CColorExt *>(m_StructValuePtr)->m_HLS = false;
							_Bar->NotUpToDate();
						}
						return 1;
					}
				}
			}
			return 0;
		case VG_COLORORDER:
			if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != nullptr) // is tw_type_color?
			{
				if (_Value != nullptr) {
					if (_stricmp(_Value, "rgba") == 0)
						static_cast<CColorExt *>(m_StructValuePtr)->m_OGL = true;
					else if (_stricmp(_Value, "argb") == 0)
						static_cast<CColorExt *>(m_StructValuePtr)->m_OGL = false;
					else
						return 0;
					return 1;
				}
				return 0;
			} else
				return 0;
		case VG_ARROW:
			if (m_SummaryCallback == CQuaternionExt::SummaryCB && m_StructValuePtr != nullptr) { // is tw_type_quat?
				if (_Value != nullptr) {
					double *dir = static_cast<CQuaternionExt *>(m_StructValuePtr)->m_Dir;
					double x, y, z;
					if (sscanf(_Value, "%lf %lf %lf", &x, &y, &z) == 3) {
						dir[0] = x;
						dir[1] = y;
						dir[2] = z;
					} else if (_stricmp(_Value, "off") == 0 || _stricmp(_Value, "0") == 0)
						dir[0] = dir[1] = dir[2] = 0;
					else
						return 0;
					return 1;
				}
				return 0;
			} else
				return 0;
		case VG_ARROWCOLOR:
			if (m_SummaryCallback == CQuaternionExt::SummaryCB && m_StructValuePtr != nullptr) // is tw_type_quat?
			{
				if (_Value != nullptr) {
					int r, g, b;
					if (sscanf(_Value, "%d %d %d", &r, &g, &b) == 3)
						static_cast<CQuaternionExt *>(m_StructValuePtr)->m_DirColor = Color32FromARGBi(255, r, g, b);
					else
						return 0;
					return 1;
				}
				return 0;
			} else
				return 0;
		case VG_AXISX:
		case VG_AXISY:
		case VG_AXISZ:
			if (m_SummaryCallback == CQuaternionExt::SummaryCB && m_StructValuePtr != nullptr) { // is tw_type_quat?
				if (_Value != nullptr) {
					float x = 0, y = 0, z = 0;
					if (_stricmp(_Value, "x") == 0 || _stricmp(_Value, "+x") == 0)
						x = 1;
					else if (_stricmp(_Value, "-x") == 0)
						x = -1;
					else if (_stricmp(_Value, "y") == 0 || _stricmp(_Value, "+y") == 0)
						y = 1;
					else if (_stricmp(_Value, "-y") == 0)
						y = -1;
					else if (_stricmp(_Value, "z") == 0 || _stricmp(_Value, "+z") == 0)
						z = 1;
					else if (_stricmp(_Value, "-z") == 0)
						z = -1;
					else
						return 0;
					int i = (_AttribID == VG_AXISX) ? 0 : ((_AttribID == VG_AXISY) ? 1 : 2);
					static_cast<CQuaternionExt *>(m_StructValuePtr)->m_Permute[i][0] = x;
					static_cast<CQuaternionExt *>(m_StructValuePtr)->m_Permute[i][1] = y;
					static_cast<CQuaternionExt *>(m_StructValuePtr)->m_Permute[i][2] = z;
					return 1;
				}
				return 0;
			} else
				return 0;
		case VG_SHOWVAL:
			if (m_SummaryCallback == CQuaternionExt::SummaryCB && m_StructValuePtr != NULL) { // is tw_type_quat?
				if (_Value != NULL) {
					if (_stricmp(_Value, "true") == 0 || _stricmp(_Value, "on") == 0 || _stricmp(_Value, "1") == 0) {
						static_cast<CQuaternionExt *>(m_StructValuePtr)->m_ShowVal = true;
						_Bar->NotUpToDate();
						return 1;
					} else if (_stricmp(_Value, "false") == 0 || _stricmp(_Value, "off") == 0 || _stricmp(_Value, "0") == 0) {
						static_cast<CQuaternionExt *>(m_StructValuePtr)->m_ShowVal = false;
						_Bar->NotUpToDate();
						return 1;
					}
				}
				return 0;
			} else
				return 0;
		default:
			return CTwVar::SetAttrib(_AttribID, _Value, _Bar, _VarParent, _VarIndex);
	}
}

ERetType CTwVarGroup::GetAttrib(int _AttribID, TwBar *_Bar, struct CTwVarGroup *_VarParent, int _VarIndex, std::vector<double> &outDoubles, std::ostringstream &outString) const {
	outDoubles.clear();
	outString.clear();

	switch (_AttribID) {
		case VG_OPENED:
			outDoubles.push_back(m_Open);
			return RET_DOUBLE;
		case VG_COLORALPHA:
			if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_color?
			{
				outDoubles.push_back(static_cast<CColorExt *>(m_StructValuePtr)->m_HasAlpha);
				return RET_DOUBLE;
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		case VG_COLORMODE:
			if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_color?
			{
				if (static_cast<CColorExt *>(m_StructValuePtr)->m_HLS)
					outString << "hls";
				else
					outString << "rgb";
				return RET_STRING;
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		case VG_COLORORDER:
			if (m_SummaryCallback == CColorExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_color?
			{
				if (static_cast<CColorExt *>(m_StructValuePtr)->m_OGL)
					outString << "rgba";
				else
					outString << "argb";
				return RET_STRING;
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		case VG_ARROW:
			if (m_SummaryCallback == CQuaternionExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_quat?
			{
				double *dir = static_cast<CQuaternionExt *>(m_StructValuePtr)->m_Dir;
				outDoubles.push_back(dir[0]);
				outDoubles.push_back(dir[1]);
				outDoubles.push_back(dir[2]);
				return RET_DOUBLE;
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		case VG_ARROWCOLOR:
			if (m_SummaryCallback == CQuaternionExt::SummaryCB && m_StructValuePtr != NULL) // is tw_type_quat?
			{
				int a, r, g, b;
				a = r = g = b = 0;
				Color32ToARGBi(static_cast<CQuaternionExt *>(m_StructValuePtr)->m_DirColor, &a, &r, &g, &b);
				outDoubles.push_back(r);
				outDoubles.push_back(g);
				outDoubles.push_back(b);
				return RET_DOUBLE;
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		case VG_AXISX:
		case VG_AXISY:
		case VG_AXISZ:
			if (m_SummaryCallback == CQuaternionExt::SummaryCB && m_StructValuePtr != nullptr) { // is tw_type_quat?
				int i = (_AttribID == VG_AXISX) ? 0 : ((_AttribID == VG_AXISY) ? 1 : 2);
				float x = static_cast<CQuaternionExt *>(m_StructValuePtr)->m_Permute[i][0];
				float y = static_cast<CQuaternionExt *>(m_StructValuePtr)->m_Permute[i][1];
				float z = static_cast<CQuaternionExt *>(m_StructValuePtr)->m_Permute[i][2];
				if (x > 0)
					outString << "+x";
				else if (x < 0)
					outString << "-x";
				else if (y > 0)
					outString << "+y";
				else if (y < 0)
					outString << "-y";
				else if (z > 0)
					outString << "+z";
				else if (z < 0)
					outString << "-z";
				else
					outString << "0"; // should not happened
				return RET_DOUBLE;
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		case VG_SHOWVAL:
			if (m_SummaryCallback == CQuaternionExt::SummaryCB && m_StructValuePtr != nullptr) // is tw_type_quat?
			{
				outDoubles.push_back(static_cast<CQuaternionExt *>(m_StructValuePtr)->m_ShowVal);
				return RET_DOUBLE;
			}
			g_TwMgr->SetLastError(g_ErrInvalidAttrib);
			return RET_ERROR;
		default:
			return CTwVar::GetAttrib(_AttribID, _Bar, _VarParent, _VarIndex, outDoubles, outString);
	}
}

//  ---------------------------------------------------------------------------

const CTwVar *CTwVarGroup::Find(const char *_Name, CTwVarGroup **_Parent, int *_Index) const {
	if (strcmp(_Name, m_Name.c_str()) == 0) {
		if (_Parent != nullptr)
			*_Parent = nullptr;
		if (_Index != nullptr)
			*_Index = -1;
		return this;
	} else {
		const CTwVar *v;
		for (size_t i = 0; i < m_Vars.size(); ++i)
			if (m_Vars[i] != nullptr) {
				v = m_Vars[i]->Find(_Name, _Parent, _Index);
				if (v != nullptr) {
					if (_Parent != nullptr && *_Parent == nullptr) {
						*_Parent = const_cast<CTwVarGroup *>(this);
						if (_Index != nullptr)
							*_Index = (int)i;
					}
					return v;
				}
			}
		return nullptr;
	}
}

//  ---------------------------------------------------------------------------

size_t CTwVar::GetDataSize(TwType _Type) {
	switch (_Type) {
		case TW_TYPE_BOOLCPP:
			return sizeof(bool);
		case TW_TYPE_BOOL8:
		case TW_TYPE_CHAR:
		case TW_TYPE_INT8:
		// case TW_TYPE_ENUM8:
		case TW_TYPE_UINT8:
			return 1;
		// case TW_TYPE_ENUM16:
		case TW_TYPE_BOOL16:
		case TW_TYPE_INT16:
		case TW_TYPE_UINT16:
			return 2;
		// case TW_TYPE_ENUM32:
		case TW_TYPE_BOOL32:
		case TW_TYPE_INT32:
		case TW_TYPE_UINT32:
		case TW_TYPE_FLOAT:
			return 4;
		case TW_TYPE_DOUBLE:
			return 8;
		case TW_TYPE_CDSTRING:
			return sizeof(char *);
		case TW_TYPE_STDSTRING:
			return (g_TwMgr != 0) ? g_TwMgr->m_ClientStdStringStructSize : sizeof(std::string);
		default:
			if (g_TwMgr && _Type >= TW_TYPE_STRUCT_BASE && _Type < TW_TYPE_STRUCT_BASE + (int)g_TwMgr->m_Structs.size()) {
				const CTwMgr::CStruct &s = g_TwMgr->m_Structs[_Type - TW_TYPE_STRUCT_BASE];
				return s.m_Size;
			} else if (g_TwMgr && IsEnumType(_Type))
				return 4;
			else if (IsCSStringType(_Type))
				return TW_CSSTRING_SIZE(_Type);
			else if (_Type == TW_TYPE_CDSTDSTRING)
				return (g_TwMgr != 0) ? g_TwMgr->m_ClientStdStringStructSize : sizeof(std::string);
			else // includes TW_TYPE_BUTTON
				return 0;
	}
}

//  ---------------------------------------------------------------------------

CTwBar::CTwBar(const char *_Name) {
	assert(g_TwMgr != nullptr && g_TwMgr->m_Graph != nullptr);

	m_Name = _Name;
	m_Visible = true;
	m_VarRoot.m_IsRoot = true;
	m_VarRoot.m_Open = true;
	m_VarRoot.m_SummaryCallback = nullptr;
	m_VarRoot.m_SummaryClientData = nullptr;
	m_VarRoot.m_StructValuePtr = nullptr;

	m_UpToDate = false;
	int n = (int)g_TwMgr->m_Bars.size();
	m_PosX = 24 * n - 8;
	m_PosY = 24 * n - 8;
	m_Width = 200;
	m_Height = 320;
	int cr, cg, cb;
	if (g_TwMgr->m_UseOldColorScheme) {
		ColorHLSToRGBi(g_TwMgr->m_BarInitColorHue % 256, 180, 200, &cr, &cg, &cb);
		m_Color = Color32FromARGBi(0xf0, cr, cg, cb);
		m_DarkText = true;
	} else {
		ColorHLSToRGBi(g_TwMgr->m_BarInitColorHue % 256, 80, 200, &cr, &cg, &cb);
		m_Color = Color32FromARGBi(64, cr, cg, cb);
		m_DarkText = false;
	}
	g_TwMgr->m_BarInitColorHue -= 16;
	if (g_TwMgr->m_BarInitColorHue < 0)
		g_TwMgr->m_BarInitColorHue += 256;
	m_Font = g_TwMgr->m_CurrentFont;
	// m_Font = g_DefaultNormalFont;
	// m_Font = g_DefaultSmallFont;
	// m_Font = g_DefaultLargeFont;
	m_TitleWidth = 0;
	m_Sep = 1;
	m_LineSep = 1;
	m_ValuesWidth = 10 * (m_Font->m_CharHeight / 2); // about 10 characters
	m_NbHierLines = 0;
	m_NbDisplayedLines = 0;
	m_FirstLine = 0;
	m_LastUpdateTime = 0;
	m_UpdatePeriod = 2;
	m_ScrollYW = 0;
	m_ScrollYH = 0;
	m_ScrollY0 = 0;
	m_ScrollY1 = 0;

	m_DrawHandles = false;
	m_DrawIncrDecrBtn = false;
	m_DrawRotoBtn = false;
	m_DrawClickBtn = false;
	m_DrawListBtn = false;
	m_DrawBoolBtn = false;
	m_MouseDrag = false;
	m_MouseDragVar = false;
	m_MouseDragTitle = false;
	m_MouseDragScroll = false;
	m_MouseDragResizeUR = false;
	m_MouseDragResizeUL = false;
	m_MouseDragResizeLR = false;
	m_MouseDragResizeLL = false;
	m_MouseDragValWidth = false;
	m_MouseOriginX = 0;
	m_MouseOriginY = 0;
	m_ValuesWidthRatio = 0;
	m_VarHasBeenIncr = true;
	m_FirstLine0 = 0;
	m_HighlightedLine = -1;
	m_HighlightedLinePrev = -1;
	m_HighlightedLineLastValid = -1;
	m_HighlightIncrBtn = false;
	m_HighlightDecrBtn = false;
	m_HighlightRotoBtn = false;
	m_HighlightClickBtn = false;
	m_HighlightClickBtnAuto = 0;
	m_HighlightListBtn = false;
	m_HighlightBoolBtn = false;
	m_HighlightTitle = false;
	m_HighlightScroll = false;
	m_HighlightUpScroll = false;
	m_HighlightDnScroll = false;
	m_HighlightMinimize = false;
	m_HighlightFont = false;
	m_HighlightValWidth = false;
	m_HighlightLabelsHeader = false;
	m_HighlightValuesHeader = false;
	m_ButtonAlign = g_TwMgr->m_ButtonAlign;

	m_IsMinimized = false;
	m_MinNumber = 0;
	m_MinPosX = 0;
	m_MinPosY = 0;
	m_HighlightMaximize = false;
	m_IsHelpBar = false;
	m_IsPopupList = false;
	m_VarEnumLinkedToPopupList = NULL;
	m_BarLinkedToPopupList = NULL;

	m_Resizable = true;
	m_Movable = true;
	m_Iconifiable = true;
	m_Contained = g_TwMgr->m_Contained;

	m_TitleTextObj = g_TwMgr->m_Graph->NewTextObj();
	m_LabelsTextObj = g_TwMgr->m_Graph->NewTextObj();
	m_ValuesTextObj = g_TwMgr->m_Graph->NewTextObj();
	m_ShortcutTextObj = g_TwMgr->m_Graph->NewTextObj();
	m_HeadersTextObj = g_TwMgr->m_Graph->NewTextObj();
	m_ShortcutLine = -1;

	m_RotoMinRadius = 24;
	m_RotoNbSubdiv = 256; // number of steps for one turn

	m_CustomActiveStructProxy = NULL;

	UpdateColors();
	NotUpToDate();
}

//  ---------------------------------------------------------------------------

CTwBar::~CTwBar() {
	if (m_IsMinimized)
		g_TwMgr->Maximize(this);
	if (m_TitleTextObj)
		g_TwMgr->m_Graph->DeleteTextObj(m_TitleTextObj);
	if (m_LabelsTextObj)
		g_TwMgr->m_Graph->DeleteTextObj(m_LabelsTextObj);
	if (m_ValuesTextObj)
		g_TwMgr->m_Graph->DeleteTextObj(m_ValuesTextObj);
	if (m_ShortcutTextObj)
		g_TwMgr->m_Graph->DeleteTextObj(m_ShortcutTextObj);
	if (m_HeadersTextObj)
		g_TwMgr->m_Graph->DeleteTextObj(m_HeadersTextObj);
}

//  ---------------------------------------------------------------------------

const CTwVar *CTwBar::Find(const char *_Name, CTwVarGroup **_Parent, int *_Index) const { return m_VarRoot.Find(_Name, _Parent, _Index); }

CTwVar *CTwBar::Find(const char *_Name, CTwVarGroup **_Parent, int *_Index) { return const_cast<CTwVar *>(const_cast<const CTwBar *>(this)->Find(_Name, _Parent, _Index)); }

//  ---------------------------------------------------------------------------

enum EBarAttribs {
	BAR_LABEL = 1,
	BAR_HELP,
	BAR_COLOR,
	BAR_ALPHA,
	BAR_TEXT,
	BAR_SHOW, // deprecated, used BAR_VISIBLE instead
	BAR_HIDE, // deprecated, used BAR_VISIBLE instead
	BAR_ICONIFY, // deprecated, used BAR_ICONIFIED instead
	BAR_VISIBLE,
	BAR_ICONIFIED,
	BAR_SIZE,
	BAR_POSITION,
	BAR_REFRESH,
	BAR_FONT_SIZE,
	BAR_FONT_STYLE,
	BAR_VALUES_WIDTH,
	BAR_ICON_POS,
	BAR_ICON_ALIGN,
	BAR_ICON_MARGIN,
	BAR_RESIZABLE,
	BAR_MOVABLE,
	BAR_ICONIFIABLE,
	BAR_FONT_RESIZABLE,
	BAR_ALWAYS_TOP,
	BAR_ALWAYS_BOTTOM,
	BAR_COLOR_SCHEME,
	BAR_CONTAINED,
	BAR_BUTTON_ALIGN,
};

int CTwBar::HasAttrib(const char *_Attrib, bool *_HasValue) const {
	*_HasValue = true;
	if (_stricmp(_Attrib, "label") == 0)
		return BAR_LABEL;
	else if (_stricmp(_Attrib, "help") == 0)
		return BAR_HELP;
	else if (_stricmp(_Attrib, "color") == 0)
		return BAR_COLOR;
	else if (_stricmp(_Attrib, "alpha") == 0)
		return BAR_ALPHA;
	else if (_stricmp(_Attrib, "text") == 0)
		return BAR_TEXT;
	else if (_stricmp(_Attrib, "size") == 0)
		return BAR_SIZE;
	else if (_stricmp(_Attrib, "position") == 0)
		return BAR_POSITION;
	else if (_stricmp(_Attrib, "refresh") == 0)
		return BAR_REFRESH;
	else if (_stricmp(_Attrib, "fontsize") == 0)
		return BAR_FONT_SIZE;
	else if (_stricmp(_Attrib, "fontstyle") == 0)
		return BAR_FONT_STYLE;
	else if (_stricmp(_Attrib, "valueswidth") == 0)
		return BAR_VALUES_WIDTH;
	else if (_stricmp(_Attrib, "iconpos") == 0)
		return BAR_ICON_POS;
	else if (_stricmp(_Attrib, "iconalign") == 0)
		return BAR_ICON_ALIGN;
	else if (_stricmp(_Attrib, "iconmargin") == 0)
		return BAR_ICON_MARGIN;
	else if (_stricmp(_Attrib, "resizable") == 0)
		return BAR_RESIZABLE;
	else if (_stricmp(_Attrib, "movable") == 0)
		return BAR_MOVABLE;
	else if (_stricmp(_Attrib, "iconifiable") == 0)
		return BAR_ICONIFIABLE;
	else if (_stricmp(_Attrib, "fontresizable") == 0)
		return BAR_FONT_RESIZABLE;
	else if (_stricmp(_Attrib, "alwaystop") == 0)
		return BAR_ALWAYS_TOP;
	else if (_stricmp(_Attrib, "alwaysbottom") == 0)
		return BAR_ALWAYS_BOTTOM;
	else if (_stricmp(_Attrib, "visible") == 0)
		return BAR_VISIBLE;
	else if (_stricmp(_Attrib, "iconified") == 0)
		return BAR_ICONIFIED;
	else if (_stricmp(_Attrib, "colorscheme") == 0)
		return BAR_COLOR_SCHEME;
	else if (_stricmp(_Attrib, "contained") == 0)
		return BAR_CONTAINED;
	else if (_stricmp(_Attrib, "buttonalign") == 0)
		return BAR_BUTTON_ALIGN;

	*_HasValue = false;
	if (_stricmp(_Attrib, "show") == 0) // for backward compatibility
		return BAR_SHOW;
	else if (_stricmp(_Attrib, "hide") == 0) // for backward compatibility
		return BAR_HIDE;
	else if (_stricmp(_Attrib, "iconify") == 0) // for backward compatibility
		return BAR_ICONIFY;

	return 0; // not found
}

int CTwBar::SetAttrib(int _AttribID, const char *_Value) {
	switch (_AttribID) {
		case BAR_LABEL:
			if (_Value && strlen(_Value) > 0) {
				m_Label = _Value;
				NotUpToDate();
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_HELP:
			if (_Value && strlen(_Value) > 0) {
				m_Help = _Value;
				NotUpToDate();
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_COLOR:
			if (_Value && strlen(_Value) > 0) {
				int v0, v1, v2, v3;
				int n = sscanf(_Value, "%d%d%d%d", &v0, &v1, &v2, &v3);
				color32 c;
				int alpha = (m_Color >> 24) & 0xff;
				if (n == 3 && v0 >= 0 && v0 <= 255 && v1 >= 0 && v1 <= 255 && v2 >= 0 && v2 <= 255)
					c = Color32FromARGBi(alpha, v0, v1, v2);
				else if (n == 4 && v0 >= 0 && v0 <= 255 && v1 >= 0 && v1 <= 255 && v2 >= 0 && v2 <= 255 && v3 >= 0 && v3 <= 255)
					c = Color32FromARGBi(v0, v1, v2, v3);
				else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
				m_Color = c;
				NotUpToDate();
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_ALPHA:
			if (_Value && strlen(_Value) > 0) {
				int alpha = 255;
				int n = sscanf(_Value, "%d", &alpha);
				if (n == 1 && alpha >= 0 && alpha <= 255)
					m_Color = (alpha << 24) | (m_Color & 0xffffff);
				else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
				NotUpToDate();
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_TEXT:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "dark") == 0)
					m_DarkText = true;
				else if (_stricmp(_Value, "light") == 0)
					m_DarkText = false;
				else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
				NotUpToDate();
				return 1;
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_SIZE:
			if (_Value && strlen(_Value) > 0) {
				int sx, sy;
				int n = sscanf(_Value, "%d%d", &sx, &sy);
				if (n == 2 && sx > 0 && sy > 0) {
					m_Width = sx;
					m_Height = sy;
					NotUpToDate();
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_POSITION:
			if (_Value && strlen(_Value) > 0) {
				int x, y;
				int n = sscanf(_Value, "%d%d", &x, &y);
				if (n == 2 && x >= 0 && y >= 0) {
					m_PosX = x;
					m_PosY = y;
					NotUpToDate();
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_REFRESH:
			if (_Value && strlen(_Value) > 0) {
				float r;
				int n = sscanf(_Value, "%f", &r);
				if (n == 1 && r >= 0) {
					m_UpdatePeriod = r;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_VALUES_WIDTH:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "fit") == 0) {
					m_ValuesWidth = VALUES_WIDTH_FIT;
					NotUpToDate();
					return 1;
				} else {
					int w;
					int n = sscanf(_Value, "%d", &w);
					if (n == 1 && w > 0) {
						m_ValuesWidth = w;
						NotUpToDate();
						return 1;
					} else {
						g_TwMgr->SetLastError(g_ErrBadValue);
						return 0;
					}
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_FONT_SIZE:
			return g_TwMgr->SetAttrib(MGR_FONT_SIZE, _Value);
		case BAR_FONT_STYLE:
			return g_TwMgr->SetAttrib(MGR_FONT_STYLE, _Value);
		case BAR_ICON_POS:
			return g_TwMgr->SetAttrib(MGR_ICON_POS, _Value);
		case BAR_ICON_ALIGN:
			return g_TwMgr->SetAttrib(MGR_ICON_ALIGN, _Value);
		case BAR_ICON_MARGIN:
			return g_TwMgr->SetAttrib(MGR_ICON_MARGIN, _Value);
		case BAR_SHOW: // deprecated
			TwSetBarState(this, TW_STATE_SHOWN);
			return 1;
		case BAR_HIDE: // deprecated
			TwSetBarState(this, TW_STATE_HIDDEN);
			return 1;
		case BAR_ICONIFY: // deprecated
			TwSetBarState(this, TW_STATE_ICONIFIED);
			return 1;
		case BAR_RESIZABLE:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					m_Resizable = true;
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					m_Resizable = false;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_MOVABLE:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					m_Movable = true;
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					m_Movable = false;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_ICONIFIABLE:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					m_Iconifiable = true;
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					m_Iconifiable = false;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_FONT_RESIZABLE:
			return g_TwMgr->SetAttrib(MGR_FONT_RESIZABLE, _Value);
		case BAR_ALWAYS_TOP:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					g_TwMgr->m_BarAlwaysOnTop = m_Name;
					if (g_TwMgr->m_BarAlwaysOnBottom.length() > 0 && strcmp(g_TwMgr->m_BarAlwaysOnBottom.c_str(), m_Name.c_str()) == 0)
						g_TwMgr->m_BarAlwaysOnBottom.clear();
					TwSetTopBar(this);
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					if (g_TwMgr->m_BarAlwaysOnTop.length() > 0 && strcmp(g_TwMgr->m_BarAlwaysOnTop.c_str(), m_Name.c_str()) == 0)
						g_TwMgr->m_BarAlwaysOnTop.clear();
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_ALWAYS_BOTTOM:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					g_TwMgr->m_BarAlwaysOnBottom = m_Name;
					if (g_TwMgr->m_BarAlwaysOnTop.length() > 0 && strcmp(g_TwMgr->m_BarAlwaysOnTop.c_str(), m_Name.c_str()) == 0)
						g_TwMgr->m_BarAlwaysOnTop.clear();
					TwSetBottomBar(this);
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					if (g_TwMgr->m_BarAlwaysOnBottom.length() > 0 && strcmp(g_TwMgr->m_BarAlwaysOnBottom.c_str(), m_Name.c_str()) == 0)
						g_TwMgr->m_BarAlwaysOnBottom.clear();
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_VISIBLE:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					TwSetBarState(this, TW_STATE_SHOWN);
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					TwSetBarState(this, TW_STATE_HIDDEN);
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_ICONIFIED:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					TwSetBarState(this, TW_STATE_ICONIFIED);
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					TwSetBarState(this, TW_STATE_UNICONIFIED);
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_COLOR_SCHEME:
			return g_TwMgr->SetAttrib(MGR_COLOR_SCHEME, _Value);
		case BAR_CONTAINED:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "1") == 0 || _stricmp(_Value, "true") == 0) {
					m_Contained = true;
					return 1;
				} else if (_stricmp(_Value, "0") == 0 || _stricmp(_Value, "false") == 0) {
					m_Contained = false;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		case BAR_BUTTON_ALIGN:
			if (_Value && strlen(_Value) > 0) {
				if (_stricmp(_Value, "left") == 0) {
					m_ButtonAlign = BUTTON_ALIGN_LEFT;
					return 1;
				} else if (_stricmp(_Value, "center") == 0) {
					m_ButtonAlign = BUTTON_ALIGN_CENTER;
					return 1;
				}
				if (_stricmp(_Value, "right") == 0) {
					m_ButtonAlign = BUTTON_ALIGN_RIGHT;
					return 1;
				} else {
					g_TwMgr->SetLastError(g_ErrBadValue);
					return 0;
				}
			} else {
				g_TwMgr->SetLastError(g_ErrNoValue);
				return 0;
			}
		default:
			g_TwMgr->SetLastError(g_ErrUnknownAttrib);
			return 0;
	}
}

ERetType CTwBar::GetAttrib(int _AttribID, std::vector<double> &outDoubles, std::ostringstream &outString) const {
	outDoubles.clear();
	outString.clear();

	switch (_AttribID) {
		case BAR_LABEL:
			outString << m_Label;
			return RET_STRING;
		case BAR_HELP:
			outString << m_Help;
			return RET_STRING;
		case BAR_COLOR: {
			int a, r, g, b;
			a = r = g = b = 0;
			Color32ToARGBi(m_Color, &a, &r, &g, &b);
			outDoubles.push_back(r);
			outDoubles.push_back(g);
			outDoubles.push_back(b);
			return RET_DOUBLE;
		}
		case BAR_ALPHA: {
			int a, r, g, b;
			a = r = g = b = 0;
			Color32ToARGBi(m_Color, &a, &r, &g, &b);
			outDoubles.push_back(a);
			return RET_DOUBLE;
		}
		case BAR_TEXT:
			if (m_DarkText)
				outString << "dark";
			else
				outString << "light";
			return RET_STRING;
		case BAR_SIZE:
			outDoubles.push_back(m_Width);
			outDoubles.push_back(m_Height);
			return RET_DOUBLE;
		case BAR_POSITION:
			outDoubles.push_back(m_PosX);
			outDoubles.push_back(m_PosY);
			return RET_DOUBLE;
		case BAR_REFRESH:
			outDoubles.push_back(m_UpdatePeriod);
			return RET_DOUBLE;
		case BAR_VALUES_WIDTH:
			outDoubles.push_back(m_ValuesWidth);
			return RET_DOUBLE;
		case BAR_FONT_SIZE:
			return g_TwMgr->GetAttrib(MGR_FONT_SIZE, outDoubles, outString);
		case BAR_FONT_STYLE:
			return g_TwMgr->GetAttrib(MGR_FONT_STYLE, outDoubles, outString);
		case BAR_ICON_POS:
			return g_TwMgr->GetAttrib(MGR_ICON_POS, outDoubles, outString);
		case BAR_ICON_ALIGN:
			return g_TwMgr->GetAttrib(MGR_ICON_ALIGN, outDoubles, outString);
		case BAR_ICON_MARGIN:
			return g_TwMgr->GetAttrib(MGR_ICON_MARGIN, outDoubles, outString);
		case BAR_RESIZABLE:
			outDoubles.push_back(m_Resizable);
			return RET_DOUBLE;
		case BAR_MOVABLE:
			outDoubles.push_back(m_Movable);
			return RET_DOUBLE;
		case BAR_ICONIFIABLE:
			outDoubles.push_back(m_Iconifiable);
			return RET_DOUBLE;
		case BAR_FONT_RESIZABLE:
			return g_TwMgr->GetAttrib(MGR_FONT_RESIZABLE, outDoubles, outString);
		case BAR_ALWAYS_TOP:
			outDoubles.push_back(g_TwMgr->m_BarAlwaysOnTop == m_Name);
			return RET_DOUBLE;
		case BAR_ALWAYS_BOTTOM:
			outDoubles.push_back(g_TwMgr->m_BarAlwaysOnBottom == m_Name);
			return RET_DOUBLE;
		case BAR_VISIBLE:
			outDoubles.push_back(m_Visible);
			return RET_DOUBLE;
		case BAR_ICONIFIED:
			outDoubles.push_back(m_IsMinimized);
			return RET_DOUBLE;
		case BAR_COLOR_SCHEME:
			return g_TwMgr->GetAttrib(MGR_COLOR_SCHEME, outDoubles, outString);
		case BAR_CONTAINED:
			outDoubles.push_back(m_Contained);
			return RET_DOUBLE;
		case BAR_BUTTON_ALIGN:
			if (m_ButtonAlign == BUTTON_ALIGN_LEFT)
				outString << "left";
			else if (m_ButtonAlign == BUTTON_ALIGN_CENTER)
				outString << "center";
			else
				outString << "right";
			return RET_STRING;
		default:
			g_TwMgr->SetLastError(g_ErrUnknownAttrib);
			return RET_ERROR;
	}
}

//  ---------------------------------------------------------------------------

void CTwBar::NotUpToDate() { m_UpToDate = false; }

//  ---------------------------------------------------------------------------

void CTwBar::UpdateColors() {
	float a, r, g, b, h, l, s;
	Color32ToARGBf(m_Color, &a, &r, &g, &b);
	ColorRGBToHLSf(r, g, b, &h, &l, &s);
	bool lightText = !m_DarkText;

	// Colors independant of m_Color

	// Highlighted line background ramp
	m_ColHighBg0 = lightText ? Color32FromARGBf(0.4, 0.9, 0.9, 0.9) : Color32FromARGBf(0.4, 1.0, 1.0, 1.0);
	m_ColHighBg1 = lightText ? Color32FromARGBf(0.4, 0.2, 0.2, 0.2) : Color32FromARGBf(0.1, 0.7, 0.7, 0.7);

	// Text colors & background
	m_ColLabelText = lightText ? COLOR32_WHITE : COLOR32_BLACK;
	m_ColStructText = lightText ? 0xffefef00 : 0xff303000;

	m_ColValText = lightText ? 0xffc7d7ff : 0xff000080;
	m_ColValTextRO = lightText ? 0xffb7b7b7 : 0xff505050;
	m_ColValMin = lightText ? 0xff9797ff : 0xff0000f0;
	m_ColValMax = m_ColValMin;
	m_ColValTextNE = lightText ? 0xff97f797 : 0xff004000;

	m_ColValBg = lightText ? Color32FromARGBf(0.2 + 0.3 * a, 0.1, 0.1, 0.1) : Color32FromARGBf(0.2 + 0.3 * a, 1, 1, 1);
	m_ColStructBg = lightText ? Color32FromARGBf(0.4 * a, 0, 0, 0) : Color32FromARGBf(0.4 * a, 1, 1, 1);

	m_ColLine = lightText ? Color32FromARGBf(0.6, 1, 1, 1) : Color32FromARGBf(0.6, 0.3, 0.3, 0.3);
	m_ColLineShadow = lightText ? Color32FromARGBf(0.6, 0, 0, 0) : Color32FromARGBf(0.6, 0, 0, 0);
	m_ColUnderline = lightText ? 0xffd0d0d0 : 0xff202000;

	m_ColGrpBg = lightText ? Color32FromARGBf(0.1 + 0.25 * a, 1, 1, 1) : Color32FromARGBf(0.1 + 0.05 * a, 0, 0, 0);
	m_ColGrpText = lightText ? 0xffffff80 : 0xff000000;

	m_ColShortcutText = lightText ? 0xffffb060 : 0xff802000;
	m_ColShortcutBg = lightText ? Color32FromARGBf(0.4 * a, 0.2, 0.2, 0.2) : Color32FromARGBf(0.4 * a, 0.8, 0.8, 0.8);
	m_ColInfoText = Color32FromARGBf(1.0, 0.5, 0.5, 0.5);

	m_ColRoto = lightText ? Color32FromARGBf(0.8, 0.85, 0.85f, 0.85) : Color32FromARGBf(0.8, 0.1, 0.1, 0.1);
	m_ColRotoVal = Color32FromARGBf(1, 1.0, 0.2, 0.2);
	m_ColRotoBound = lightText ? Color32FromARGBf(0.8, 0.6, 0.6, 0.6) : Color32FromARGBf(0.8, 0.3, 0.3, 0.3);

	m_ColEditText = lightText ? COLOR32_WHITE : COLOR32_BLACK;
	m_ColEditBg = lightText ? 0xff575757 : 0xffc7c7c7; // must be opaque
	m_ColEditSelText = lightText ? COLOR32_BLACK : COLOR32_WHITE;
	m_ColEditSelBg = lightText ? 0xffc7c7c7 : 0xff575757;

	// Colors dependant of m_Colors

	// Bar background
	ColorHLSToRGBf(h, l, s, &r, &g, &b);
	m_ColBg = Color32FromARGBf(a, r, g, b);
	ColorHLSToRGBf(h, l - 0.05f, s, &r, &g, &b);
	m_ColBg1 = Color32FromARGBf(a, r, g, b);
	ColorHLSToRGBf(h, l - 0.1f, s, &r, &g, &b);
	m_ColBg2 = Color32FromARGBf(a, r, g, b);

	ColorHLSToRGBf(h, l - 0.15f, s, &r, &g, &b);
	m_ColTitleBg = Color32FromARGBf(a + 0.9f, r, g, b);
	m_ColTitleText = lightText ? COLOR32_WHITE : COLOR32_BLACK;
	m_ColTitleShadow = lightText ? 0x40000000 : 0x00000000;
	ColorHLSToRGBf(h, l - 0.25f, s, &r, &g, &b);
	m_ColTitleHighBg = Color32FromARGBf(a + 0.8f, r, g, b);
	ColorHLSToRGBf(h, l - 0.3f, s, &r, &g, &b);
	m_ColTitleUnactiveBg = Color32FromARGBf(a + 0.2f, r, g, b);

	ColorHLSToRGBf(h, l - 0.2f, s, &r, &g, &b);
	m_ColHierBg = Color32FromARGBf(a, r, g, b);

	ColorHLSToRGBf(h, l + 0.1f, s, &r, &g, &b);
	m_ColBtn = Color32FromARGBf(0.2f + 0.4f * a, r, g, b);
	ColorHLSToRGBf(h, l - 0.35f, s, &r, &g, &b);
	m_ColHighBtn = Color32FromARGBf(0.4f + 0.4f * a, r, g, b);
	ColorHLSToRGBf(h, l - 0.25, s, &r, &g, &b);
	m_ColFold = Color32FromARGBf(0.1 + 0.4 * a, r, g, b);
	ColorHLSToRGBf(h, l - 0.35, s, &r, &g, &b);
	m_ColHighFold = Color32FromARGBf(0.3 + 0.4 * a, r, g, b);

	ColorHLSToRGBf(h, 0.75, s, &r, &g, &b);
	m_ColHelpBg = Color32FromARGBf(0.2f, 1, 1, 1);
	m_ColHelpText = lightText ? Color32FromARGBf(1, 0.2, 1.0, 0.2) : Color32FromARGBf(1, 0, 0.4, 0);
	m_ColSeparator = m_ColValTextRO;
	m_ColStaticText = m_ColHelpText;
}

//  ---------------------------------------------------------------------------

CTwVarGroup::~CTwVarGroup() {
	for (std::vector<CTwVar *>::iterator it = m_Vars.begin(); it != m_Vars.end(); ++it)
		if (*it != nullptr) {
			CTwVar *Var = *it;
			delete Var;
			*it = nullptr;
		}
}

//  ---------------------------------------------------------------------------

static inline int IncrBtnWidth(int _CharHeight) {
	return ((2 * _CharHeight) / 3 + 2) & 0xfffe; // force even value
}

//  ---------------------------------------------------------------------------

void CTwBar::BrowseHierarchy(int *_CurrLine, int _CurrLevel, const CTwVar *_Var, int _First, int _Last) {
	DEV_ASSERT(_Var != nullptr);
	if (!_Var->m_IsRoot) {
		if ((*_CurrLine) >= _First && (*_CurrLine) <= _Last) {
			CHierTag Tag;
			Tag.m_Level = _CurrLevel;
			Tag.m_Var = const_cast<CTwVar *>(_Var);
			Tag.m_Closing = false;
			m_HierTags.push_back(Tag);
		}
		*_CurrLine += 1;
	} else {
		*_CurrLine = 0;
		_CurrLevel = -1;
		m_HierTags.resize(0);
	}

	if (_Var->IsGroup()) {
		const CTwVarGroup *Grp = static_cast<const CTwVarGroup *>(_Var);
		if (Grp->m_Open)
			for (std::vector<CTwVar *>::const_iterator it = Grp->m_Vars.begin(); it != Grp->m_Vars.end(); ++it)
				if ((*it)->m_Visible)
					BrowseHierarchy(_CurrLine, _CurrLevel + 1, *it, _First, _Last);
		if (m_HierTags.size() > 0)
			m_HierTags[m_HierTags.size() - 1].m_Closing = true;
	}
}

//  ---------------------------------------------------------------------------

void CTwBar::ListLabels(std::vector<std::string> &_Labels, std::vector<color32> &_Colors, std::vector<color32> &_BgColors, bool *_HasBgColors, const CTexFont *_Font, int _AtomWidthMax, int _GroupWidthMax) {
	const int NbEtc = 2;
	std::string ValStr;
	int Len, i, x, Etc, s;
	const unsigned char *Text;
	unsigned char ch;
	int WidthMax;

	int Space = _Font->m_CharWidth[(int)' '];
	int LevelSpace = MAX(_Font->m_CharHeight - 6, 4); // space used by DrawHierHandles

	int nh = (int)m_HierTags.size();
	for (int h = 0; h < nh; ++h) {
		Len = (int)m_HierTags[h].m_Var->m_Label.length();
		if (Len > 0)
			Text = (const unsigned char *)(m_HierTags[h].m_Var->m_Label.c_str());
		else {
			Text = (const unsigned char *)(m_HierTags[h].m_Var->m_Name.c_str());
			Len = (int)m_HierTags[h].m_Var->m_Name.length();
		}
		x = 0;
		Etc = 0;
		_Labels.push_back(""); // add a new text line
		if (!m_HierTags[h].m_Var->IsGroup() && static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type == TW_TYPE_BUTTON && static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_ReadOnly && static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_Val.m_Button.m_Callback != nullptr)
			_Colors.push_back(m_ColValTextRO); // special case for read-only buttons
		else
			_Colors.push_back(m_HierTags[h].m_Var->m_ColorPtr != nullptr ? *(m_HierTags[h].m_Var->m_ColorPtr) : COLOR32_WHITE);
		color32 bg = m_HierTags[h].m_Var->m_BgColorPtr != nullptr ? *(m_HierTags[h].m_Var->m_BgColorPtr) : 0;
		_BgColors.push_back(bg);
		if (_HasBgColors != nullptr && bg != 0)
			*_HasBgColors = true;
		bool IsCustom = m_HierTags[h].m_Var->IsCustom(); // !m_HierTags[h].m_Var->IsGroup() && (static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type>=TW_TYPE_CUSTOM_BASE && static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size());
		if (!IsCustom) {
			std::string &CurrentLabel = _Labels[_Labels.size() - 1];
			if (m_HierTags[h].m_Var->IsGroup() && static_cast<const CTwVarGroup *>(m_HierTags[h].m_Var)->m_SummaryCallback == nullptr)
				WidthMax = _GroupWidthMax;
			else if (!m_HierTags[h].m_Var->IsGroup() && static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type == TW_TYPE_BUTTON) {
				if (static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_Val.m_Button.m_Callback == nullptr)
					WidthMax = _GroupWidthMax;
				else if (m_ButtonAlign == BUTTON_ALIGN_RIGHT)
					WidthMax = _GroupWidthMax - 2 * IncrBtnWidth(m_Font->m_CharHeight);
				else
					WidthMax = _AtomWidthMax;
			}
			// else if( m_HighlightedLine==h && m_DrawRotoBtn )
			//   WidthMax = _AtomWidthMax - IncrBtnWidth(m_Font->m_CharHeight);
			else
				WidthMax = _AtomWidthMax;
			if (Space > 0)
				for (s = 0; s < m_HierTags[h].m_Level * LevelSpace; s += Space) {
					CurrentLabel += ' ';
					x += Space;
				}
			if (x + (NbEtc + 2) * _Font->m_CharWidth[(int)'.'] < WidthMax || m_HierTags[h].m_Var->m_DontClip)
				for (i = 0; i < Len; ++i) {
					ch = (Etc == 0) ? Text[i] : '.';
					CurrentLabel += ch;
					x += _Font->m_CharWidth[(int)ch];
					if (Etc > 0) {
						++Etc;
						if (Etc > NbEtc)
							break;
					} else if (i < Len - 2 && x + (NbEtc + 2) * _Font->m_CharWidth[(int)'.'] >= WidthMax && !(m_HierTags[h].m_Var->m_DontClip))
						Etc = 1;
				}
		}
	}
}

//  ---------------------------------------------------------------------------

void CTwBar::ListValues(std::vector<std::string> &_Values, std::vector<color32> &_Colors, std::vector<color32> &_BgColors, const CTexFont *_Font, int _WidthMax) {
	CTwFPU fpu; // force fpu precision

	const int NbEtc = 2;
	const CTwVarAtom *Atom = nullptr;
	std::string ValStr;
	int Len, i, x, Etc;
	const unsigned char *Text;
	unsigned char ch;
	bool ReadOnly;
	bool IsMax;
	bool IsMin;
	bool IsROText;
	bool HasBgColor;
	bool AcceptEdit;
	size_t SummaryMaxLength = MAX(_WidthMax / _Font->m_CharWidth[(int)'I'], 4);
	static std::vector<char> Summary;
	Summary.resize(SummaryMaxLength + 32);

	int nh = (int)m_HierTags.size();
	for (int h = 0; h < nh; ++h)
		if (!m_HierTags[h].m_Var->IsGroup() || m_IsHelpBar || (m_HierTags[h].m_Var->IsGroup() && static_cast<const CTwVarGroup *>(m_HierTags[h].m_Var)->m_SummaryCallback != nullptr)) {
			ReadOnly = true;
			IsMax = false;
			IsMin = false;
			IsROText = false;
			HasBgColor = true;
			AcceptEdit = false;
			if (!m_HierTags[h].m_Var->IsGroup()) {
				Atom = static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var);
				Atom->ValueToString(&ValStr);
				if (!m_IsHelpBar || (Atom->m_Type == TW_TYPE_SHORTCUT && (Atom->m_Val.m_Shortcut.m_Incr[0] > 0 || Atom->m_Val.m_Shortcut.m_Decr[0] > 0)))
					ReadOnly = Atom->m_ReadOnly;
				if (!Atom->m_NoSlider) {
					double v, vmin, vmax;
					v = Atom->ValueToDouble();
					Atom->MinMaxStepToDouble(&vmin, &vmax, nullptr);
					IsMax = (v >= vmax);
					IsMin = (v <= vmin);
				}
				if (Atom->m_Type == TW_TYPE_BOOLCPP || Atom->m_Type == TW_TYPE_BOOL8 || Atom->m_Type == TW_TYPE_BOOL16 || Atom->m_Type == TW_TYPE_BOOL32) {
					if (ValStr == "1")
						ValStr = "\x7f"; // check sign
					else if (ValStr == "0")
						ValStr = " -"; //"\x97"; // uncheck sign
				}
				if ((Atom->m_Type == TW_TYPE_CDSTRING && Atom->m_SetCallback == nullptr && g_TwMgr->m_CopyCDStringToClient == nullptr) || (Atom->m_Type == TW_TYPE_CDSTDSTRING && Atom->m_SetCallback == nullptr) || (Atom->m_Type == TW_TYPE_STDSTRING && Atom->m_SetCallback == nullptr && g_TwMgr->m_CopyStdStringToClient == nullptr))
					IsROText = true;
				if (Atom->m_Type == TW_TYPE_HELP_ATOM || Atom->m_Type == TW_TYPE_HELP_GRP || Atom->m_Type == TW_TYPE_BUTTON || Atom->IsCustom()) // (Atom->m_Type>=TW_TYPE_CUSTOM_BASE && Atom->m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size()) )
					HasBgColor = false;
				AcceptEdit = EditInPlaceAcceptVar(Atom) || (Atom->m_Type == TW_TYPE_SHORTCUT);
			} else if (m_HierTags[h].m_Var->IsGroup() && static_cast<const CTwVarGroup *>(m_HierTags[h].m_Var)->m_SummaryCallback != nullptr) {
				const CTwVarGroup *Grp = static_cast<const CTwVarGroup *>(m_HierTags[h].m_Var);
				// force internal value update
				for (size_t v = 0; v < Grp->m_Vars.size(); v++)
					if (Grp->m_Vars[v] != nullptr && !Grp->m_Vars[v]->IsGroup() && Grp->m_Vars[v]->m_Visible)
						static_cast<CTwVarAtom *>(Grp->m_Vars[v])->ValueToDouble();

				Summary[0] = '\0';
				if (Grp->m_SummaryCallback == CTwMgr::CStruct::DefaultSummary)
					Grp->m_SummaryCallback(&Summary[0], SummaryMaxLength, Grp, Grp->m_SummaryClientData);
				else
					Grp->m_SummaryCallback(&Summary[0], SummaryMaxLength, Grp->m_StructValuePtr, Grp->m_SummaryClientData);
				ValStr = (const char *)(&Summary[0]);
			} else {
				ValStr = ""; // is a group in the help bar
				HasBgColor = false;
			}
			Len = (int)ValStr.length();
			Text = (const unsigned char *)(ValStr.c_str());
			x = 0;
			Etc = 0;
			_Values.push_back(""); // add a new text line
			if (ReadOnly || (IsMin && IsMax) || IsROText)
				_Colors.push_back(m_ColValTextRO);
			else if (IsMin)
				_Colors.push_back(m_ColValMin);
			else if (IsMax)
				_Colors.push_back(m_ColValMax);
			else if (!AcceptEdit)
				_Colors.push_back(m_ColValTextNE);
			else
				_Colors.push_back(m_ColValText);
			if (!HasBgColor)
				_BgColors.push_back(0x00000000);
			else if (m_HierTags[h].m_Var->IsGroup()) {
				const CTwVarGroup *Grp = static_cast<const CTwVarGroup *>(m_HierTags[h].m_Var);
				// if typecolor set bgcolor
				if (Grp->m_SummaryCallback == CColorExt::SummaryCB)
					_BgColors.push_back(0xff000000);
				else
					_BgColors.push_back(m_ColStructBg);
			} else
				_BgColors.push_back(m_ColValBg);

			std::string &CurrentValue = _Values[_Values.size() - 1];
			int wmax = _WidthMax;
			if (m_HighlightedLine == h && m_DrawRotoBtn)
				wmax -= 3 * IncrBtnWidth(m_Font->m_CharHeight);
			else if (m_HighlightedLine == h && m_DrawIncrDecrBtn)
				wmax -= 2 * IncrBtnWidth(m_Font->m_CharHeight);
			else if (m_HighlightedLine == h && m_DrawListBtn)
				wmax -= 1 * IncrBtnWidth(m_Font->m_CharHeight);
			else if (m_HighlightedLine == h && m_DrawBoolBtn)
				wmax -= 1 * IncrBtnWidth(m_Font->m_CharHeight);
			for (i = 0; i < Len; ++i) {
				ch = (Etc == 0) ? Text[i] : '.';
				CurrentValue += ch;
				x += _Font->m_CharWidth[(int)ch];
				if (Etc > 0) {
					++Etc;
					if (Etc > NbEtc)
						break;
				} else if (i < Len - 2 && x + (NbEtc + 2) * (_Font->m_CharWidth[(int)'.']) >= wmax)
					Etc = 1;
			}
		} else {
			_Values.push_back(""); // add a new empty line
			_Colors.push_back(COLOR32_BLACK);
			_BgColors.push_back(0x00000000);
		}
}

//  ---------------------------------------------------------------------------

int CTwBar::ComputeLabelsWidth(const CTexFont *_Font) {
	int Len, x;
	const unsigned char *Text;
	int LabelsWidth = 0;
	int Space = _Font->m_CharWidth[(int)' '];
	int LevelSpace = MAX(_Font->m_CharHeight - 6, 4); // space used by DrawHierHandles

	int nh = (int)m_HierTags.size();
	for (int h = 0; h < nh; ++h) {
		Len = (int)m_HierTags[h].m_Var->m_Label.length();
		if (Len > 0)
			Text = (const unsigned char *)(m_HierTags[h].m_Var->m_Label.c_str());
		else {
			Text = (const unsigned char *)(m_HierTags[h].m_Var->m_Name.c_str());
			Len = (int)m_HierTags[h].m_Var->m_Name.length();
		}
		x = 0;
		bool IsCustom = m_HierTags[h].m_Var->IsCustom(); // !m_HierTags[h].m_Var->IsGroup() && (static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type>=TW_TYPE_CUSTOM_BASE && static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size());
		if (!IsCustom) {
			if (Space > 0)
				for (int s = 0; s < m_HierTags[h].m_Level * LevelSpace; s += Space)
					x += Space;
			for (int i = 0; i < Len; ++i)
				x += _Font->m_CharWidth[(int)Text[i]];
			x += 3 * Space; // add little margin
		}
		if (x > LabelsWidth)
			LabelsWidth = x;
	}

	return LabelsWidth;
}

int CTwBar::ComputeValuesWidth(const CTexFont *_Font) {
	CTwFPU fpu; // force fpu precision

	const CTwVarAtom *Atom = NULL;
	std::string ValStr;
	int Len, i, x;
	int Space = _Font->m_CharWidth[(int)' '];
	const unsigned char *Text;
	int ValuesWidth = 0;

	int nh = (int)m_HierTags.size();
	for (int h = 0; h < nh; ++h)
		if (!m_HierTags[h].m_Var->IsGroup()) {
			Atom = static_cast<const CTwVarAtom *>(m_HierTags[h].m_Var);
			Atom->ValueToString(&ValStr);

			Len = (int)ValStr.length();
			Text = (const unsigned char *)(ValStr.c_str());
			x = 0;
			for (i = 0; i < Len; ++i)
				x += _Font->m_CharWidth[(int)Text[i]];
			x += 2 * Space; // add little margin
			if (x > ValuesWidth)
				ValuesWidth = x;
		}

	return ValuesWidth;
}

//  ---------------------------------------------------------------------------

static int ClampText(std::string &_Text, const CTexFont *_Font, int _WidthMax) {
	int i, Len = (int)_Text.length(), Width = 0;
	unsigned char ch;
	for (i = 0; i < Len; ++i) {
		ch = _Text.at(i);
		if (i < Len - 1 && Width + _Font->m_CharWidth[(int)'.'] >= _WidthMax)
			break;
		Width += _Font->m_CharWidth[ch];
	}
	if (i < Len) // clamp
	{
		_Text.resize(i + 2);
		_Text.at(i + 0) = '.';
		_Text.at(i + 1) = '.';
		Width += 2 * _Font->m_CharWidth[(int)'.'];
	}
	return Width;
}

//  ---------------------------------------------------------------------------

void CTwBar::Update() {
	DEV_ASSERT(m_UpToDate == false);
	DEV_ASSERT(m_Font);
	ITwGraph *Gr = g_TwMgr->m_Graph;

	if (g_TwMgr->m_WndWidth <= 0 || g_TwMgr->m_WndHeight <= 0)
		return; // graphic window is not ready

	bool DoEndDraw = false;
	if (!Gr->IsDrawing()) {
		Gr->BeginDraw(g_TwMgr->m_WndWidth, g_TwMgr->m_WndHeight);
		DoEndDraw = true;
	}

	bool ValuesWidthFit = false;
	if (m_ValuesWidth == VALUES_WIDTH_FIT) {
		ValuesWidthFit = true;
		m_ValuesWidth = 0;
	}
	int PrevPosY = m_PosY;
	int vpx, vpy, vpw, vph;
	vpx = 0;
	vpy = 0;
	vpw = g_TwMgr->m_WndWidth;
	vph = g_TwMgr->m_WndHeight;
	if (!m_IsMinimized && vpw > 0 && vph > 0) {
		bool Modif = false;
		if (m_Resizable) {
			if (m_Width > vpw && m_Contained) {
				m_Width = vpw;
				Modif = true;
			}
			if (m_Width < 8 * m_Font->m_CharHeight) {
				m_Width = 8 * m_Font->m_CharHeight;
				Modif = true;
			}
			if (m_Height > vph && m_Contained) {
				m_Height = vph;
				Modif = true;
			}
			if (m_Height < 5 * m_Font->m_CharHeight) {
				m_Height = 5 * m_Font->m_CharHeight;
				Modif = true;
			}
		}
		if (m_Movable && m_Contained) {
			if (m_PosX + m_Width > vpx + vpw)
				m_PosX = vpx + vpw - m_Width;
			if (m_PosX < vpx)
				m_PosX = vpx;
			if (m_PosY + m_Height > vpy + vph)
				m_PosY = vpy + vph - m_Height;
			if (m_PosY < vpy)
				m_PosY = vpy;
		}
		m_ScrollY0 += m_PosY - PrevPosY;
		m_ScrollY1 += m_PosY - PrevPosY;
		if (m_ValuesWidth < 2 * m_Font->m_CharHeight) {
			m_ValuesWidth = 2 * m_Font->m_CharHeight;
			Modif = true;
		}
		if (m_ValuesWidth > m_Width - 4 * m_Font->m_CharHeight) {
			m_ValuesWidth = m_Width - 4 * m_Font->m_CharHeight;
			Modif = true;
		}
		if (ValuesWidthFit)
			Modif = true;
		if (Modif && m_IsHelpBar) {
			g_TwMgr->m_HelpBarNotUpToDate = true;
			g_TwMgr->m_KeyPressedBuildText = true;
			g_TwMgr->m_InfoBuildText = true;
		}
	}

	UpdateColors();

	// update geometry relatively to (m_PosX, m_PosY)
	if (!m_IsPopupList) {
		// m_VarX0 = 2*m_Font->m_CharHeight+m_Sep;
		m_VarX0 = m_Font->m_CharHeight + m_Sep;
		// m_VarX2 = m_Width - 4;
		m_VarX2 = m_Width - m_Font->m_CharHeight - m_Sep - 2;
		m_VarX1 = m_VarX2 - m_ValuesWidth;
	} else {
		// m_VarX0 = m_Font->m_CharHeight+6+m_Sep;
		m_VarX0 = 2;
		// m_VarX2 = m_Width - 4;
		m_VarX2 = m_Width - m_Font->m_CharHeight - m_Sep - 2;
		m_VarX1 = m_VarX2;
	}
	if (m_VarX1 < m_VarX0 + 32)
		m_VarX1 = m_VarX0 + 32;
	if (m_VarX1 > m_VarX2)
		m_VarX1 = m_VarX2;
	if (!m_IsPopupList) {
		m_VarY0 = m_Font->m_CharHeight + 2 + m_Sep + 6;
		m_VarY1 = m_Height - m_Font->m_CharHeight - 2 - m_Sep;
		m_VarY2 = m_Height - 1;
	} else {
		m_VarY0 = 4;
		m_VarY1 = m_Height - 2 - m_Sep;
		m_VarY2 = m_Height - 1;
	}

	int NbLines = (m_VarY1 - m_VarY0 + 1) / (m_Font->m_CharHeight + m_LineSep);
	if (NbLines <= 0)
		NbLines = 1;
	if (!m_IsMinimized) {
		int LineNum = 0;
		BrowseHierarchy(&LineNum, 0, &m_VarRoot, m_FirstLine, m_FirstLine + NbLines); // add a dummy tag at the end to avoid wrong 'tag-closing' problems
		if ((int)m_HierTags.size() > NbLines)
			m_HierTags.resize(NbLines); // remove the last dummy tag
		m_NbHierLines = LineNum;
		m_NbDisplayedLines = (int)m_HierTags.size();

		if (ValuesWidthFit) {
			m_ValuesWidth = ComputeValuesWidth(m_Font);
			if (m_ValuesWidth < 2 * m_Font->m_CharHeight)
				m_ValuesWidth = 2 * m_Font->m_CharHeight; // enough to draw buttons
			if (m_ValuesWidth > m_VarX2 - m_VarX0)
				m_ValuesWidth = MAX(m_VarX2 - m_VarX0 - m_Font->m_CharHeight, 0);
			m_VarX1 = m_VarX2 - m_ValuesWidth;
			if (m_VarX1 < m_VarX0 + 32)
				m_VarX1 = m_VarX0 + 32;
			if (m_VarX1 > m_VarX2)
				m_VarX1 = m_VarX2;
			m_ValuesWidth = m_VarX2 - m_VarX1;
		}
	}

	// scroll bar
	int y0 = m_PosY + m_VarY0;
	int y1 = m_PosY + m_VarY1;
	int x0 = m_PosX + 2;
	int x1 = m_PosX + m_Font->m_CharHeight - 2;
	if (((x0 + x1) & 1) == 1)
		x1 += 1;
	int w = x1 - x0 + 1;
	int h = y1 - y0 - 2 * w;
	int hscr = (m_NbHierLines > 0) ? ((h * m_NbDisplayedLines) / m_NbHierLines) : h;
	if (hscr <= 4)
		hscr = 4;
	if (hscr > h)
		hscr = h;
	int yscr = (m_NbHierLines > 0) ? ((h * m_FirstLine) / m_NbHierLines) : 0;
	if (yscr <= 0)
		yscr = 0;
	if (yscr > h - 4)
		yscr = h - 4;
	if (yscr + hscr > h)
		hscr = h - yscr;
	if (hscr > h)
		hscr = h;
	if (hscr <= 4)
		hscr = 4;
	m_ScrollYW = w;
	m_ScrollYH = h;
	m_ScrollY0 = y0 + w + yscr;
	m_ScrollY1 = y0 + w + yscr + hscr;

	// Build title
	std::string Title;
	if (m_Label.size() > 0)
		Title = m_Label;
	else
		Title = m_Name;
	m_TitleWidth = ClampText(Title, m_Font, (!m_IsMinimized) ? (m_Width - 5 * m_Font->m_CharHeight) : (16 * m_Font->m_CharHeight));
	Gr->BuildText(m_TitleTextObj, &Title, nullptr, nullptr, 1, m_Font, 0, 0);

	if (!m_IsMinimized) {
		// Build labels
		std::vector<std::string> Labels;
		std::vector<color32> Colors;
		std::vector<color32> BgColors;
		bool HasBgColors = false;
		ListLabels(Labels, Colors, BgColors, &HasBgColors, m_Font, m_VarX1 - m_VarX0, m_VarX2 - m_VarX0);
		assert(Labels.size() == Colors.size() && Labels.size() == BgColors.size());
		if (Labels.size() > 0)
			Gr->BuildText(m_LabelsTextObj, &(Labels[0]), &(Colors[0]), &(BgColors[0]), (int)Labels.size(), m_Font, m_LineSep, HasBgColors ? m_VarX1 - m_VarX0 - m_Font->m_CharHeight + 2 : 0);
		else
			Gr->BuildText(m_LabelsTextObj, nullptr, nullptr, nullptr, 0, m_Font, m_LineSep, 0);

		// Should draw click button?
		m_DrawClickBtn = (m_VarX2 - m_VarX1 > 4 * IncrBtnWidth(m_Font->m_CharHeight) && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var != nullptr && !m_HierTags[m_HighlightedLine].m_Var->IsGroup() && !static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly && (static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type == TW_TYPE_BUTTON));
		// || static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type==TW_TYPE_BOOLCPP
		// || static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type==TW_TYPE_BOOL8
		// || static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type==TW_TYPE_BOOL16
		// || static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type==TW_TYPE_BOOL32 ));

		// Should draw [-/+] button?
		m_DrawIncrDecrBtn = (m_VarX2 - m_VarX1 > 5 * IncrBtnWidth(m_Font->m_CharHeight) && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var != nullptr && !m_HierTags[m_HighlightedLine].m_Var->IsGroup() && static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type != TW_TYPE_BUTTON && !static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly && !static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_NoSlider && !(m_EditInPlace.m_Active && m_EditInPlace.m_Var == m_HierTags[m_HighlightedLine].m_Var));

		// Should draw [v] button (list)?
		m_DrawListBtn = (m_VarX2 - m_VarX1 > 2 * IncrBtnWidth(m_Font->m_CharHeight) && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var != nullptr && !m_HierTags[m_HighlightedLine].m_Var->IsGroup() && IsEnumType(static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type) && !static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly);

		// Should draw [<>] button (bool)?
		m_DrawBoolBtn = (m_VarX2 - m_VarX1 > 4 * IncrBtnWidth(m_Font->m_CharHeight) && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var != nullptr && !m_HierTags[m_HighlightedLine].m_Var->IsGroup() && !static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly && (static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type == TW_TYPE_BOOLCPP || static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type == TW_TYPE_BOOL8 || static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type == TW_TYPE_BOOL16 || static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type == TW_TYPE_BOOL32));

		// Should draw [o] button?
		m_DrawRotoBtn = m_DrawIncrDecrBtn;
		// m_DrawRotoBtn = ( m_HighlightedLine>=0 && m_HighlightedLine<(int)m_HierTags.size()
		//   && m_HierTags[m_HighlightedLine].m_Var!=nullptr
		//   && !m_HierTags[m_HighlightedLine].m_Var->IsGroup()
		//   && static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type!=TW_TYPE_BUTTON
		//   && !static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly
		//  && !static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_NoSlider );

		// Build values
		std::vector<std::string> &Values = Labels; // reuse
		Values.resize(0);
		Colors.resize(0);
		BgColors.resize(0);
		ListValues(Values, Colors, BgColors, m_Font, m_VarX2 - m_VarX1);
		DEV_ASSERT(BgColors.size() == Values.size() && Colors.size() == Values.size());
		if (Values.size() > 0)
			Gr->BuildText(m_ValuesTextObj, &(Values[0]), &(Colors[0]), &(BgColors[0]), (int)Values.size(), m_Font, m_LineSep, m_VarX2 - m_VarX1);
		else
			Gr->BuildText(m_ValuesTextObj, nullptr, nullptr, nullptr, 0, m_Font, m_LineSep, m_VarX2 - m_VarX1);

		// Build key shortcut text
		std::string Shortcut;
		m_ShortcutLine = -1;
		if (m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var != nullptr && !m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
			const CTwVarAtom *Atom = static_cast<const CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
			if (Atom->m_KeyIncr[0] > 0 || Atom->m_KeyDecr[0] > 0) {
				if (Atom->m_KeyIncr[0] > 0 && Atom->m_KeyDecr[0] > 0)
					Shortcut = "Keys: ";
				else
					Shortcut = "Key: ";
				if (Atom->m_KeyIncr[0] > 0)
					TwGetKeyString(&Shortcut, Atom->m_KeyIncr[0], Atom->m_KeyIncr[1]);
				else
					Shortcut += "(none)";
				if (Atom->m_KeyDecr[0] > 0) {
					Shortcut += "  ";
					TwGetKeyString(&Shortcut, Atom->m_KeyDecr[0], Atom->m_KeyDecr[1]);
				}
				m_ShortcutLine = m_HighlightedLine;
			}
		}
		ClampText(Shortcut, m_Font, m_Width - 3 * m_Font->m_CharHeight);
		Gr->BuildText(m_ShortcutTextObj, &Shortcut, nullptr, nullptr, 1, m_Font, 0, 0);

		// build headers text
		if (m_HighlightLabelsHeader || m_HighlightValuesHeader) {
			std::string HeadersText = "Fit column content";
			ClampText(HeadersText, m_Font, m_Width - 3 * m_Font->m_CharHeight);
			Gr->BuildText(m_HeadersTextObj, &HeadersText, NULL, NULL, 1, m_Font, 0, 0);
		}
	}

	if (DoEndDraw)
		Gr->EndDraw();

	m_UpToDate = true;
	m_LastUpdateTime = float(g_BarTimer.GetTime());
}

//  ---------------------------------------------------------------------------

void CTwBar::DrawHierHandle() {
	DEV_ASSERT(m_Font);
	ITwGraph *Gr = g_TwMgr->m_Graph;

	// int x0 = m_PosX+m_Font->m_CharHeight+1;
	int x0 = m_PosX + 3;
	// int x2 = m_PosX+m_VarX0-5;
	// int x2 = m_PosX+3*m_Font->m_CharWidth[(int)' ']-2;
	int x2 = m_PosX + m_Font->m_CharHeight - 3;
	if (x2 - x0 < 4)
		x2 = x0 + 4;
	if ((x2 - x0) & 1)
		--x2;
	int x1 = (x0 + x2) / 2;
	int w = x2 - x0 + 1;
	int y0 = m_PosY + m_VarY0 + 1;
	int y1;
	int dh0 = (m_Font->m_CharHeight + m_Sep - 1 - w) / 2;
	if (dh0 < 0)
		dh0 = 0;
	int dh1 = dh0 + w - 1;
	int i, h = 0;

	if (!m_IsPopupList) {
		CTwVarGroup *Grp;
		int nh = (int)m_HierTags.size();
		for (h = 0; h < nh; ++h) {
			y1 = y0 + m_Font->m_CharHeight + m_Sep - 1;
			if (m_HierTags[h].m_Var->IsGroup())
				Grp = static_cast<CTwVarGroup *>(m_HierTags[h].m_Var);
			else
				Grp = NULL;

			int dx = m_HierTags[h].m_Level * (x2 - x0);

			if (Grp) {
				if (m_ColGrpBg != 0 && Grp->m_StructValuePtr == NULL) {
					color32 cb = (Grp->m_StructType == TW_TYPE_HELP_STRUCT) ? m_ColStructBg : m_ColGrpBg;
					//Gr->DrawRect(x0+dx-1, y0, m_PosX+m_VarX2, y0+m_Font->m_CharHeight-1, cb);
					Gr->DrawRect(x2 + dx + 3, y0, m_PosX + m_VarX2, y0 + m_Font->m_CharHeight - 1 + m_LineSep - 1, cb);
				}

				if (m_DrawHandles) {
					Gr->DrawLine(dx + x2 + 1, y0 + dh0 + 1, dx + x2 + 1, y0 + dh1 + 1, m_ColLineShadow);
					Gr->DrawLine(dx + x0 + 1, y0 + dh1 + 1, dx + x2 + 2, y0 + dh1 + 1, m_ColLineShadow);
				}

				// Gr->DrawRect(x0+1,y0+dh0+1,x2-1,y0+dh1-1, (h==m_HighlightedLine) ? m_ColHighBtn : m_ColBtn);
				Gr->DrawRect(dx + x0, y0 + dh0, dx + x2, y0 + dh1, (h == m_HighlightedLine) ? m_ColHighFold : m_ColFold);
				if (m_DrawHandles) {
					Gr->DrawLine(dx + x0, y0 + dh0, dx + x2, y0 + dh0, m_ColLine);
					Gr->DrawLine(dx + x2, y0 + dh0, dx + x2, y0 + dh1 + 1, m_ColLine);
					Gr->DrawLine(dx + x2, y0 + dh1, dx + x0, y0 + dh1, m_ColLine);
					Gr->DrawLine(dx + x0, y0 + dh1, dx + x0, y0 + dh0, m_ColLine);
				}

				Gr->DrawLine(dx + x0 + 2, y0 + dh0 + w / 2, dx + x2 - 1, y0 + dh0 + w / 2, m_ColTitleText);
				if (!Grp->m_Open)
					Gr->DrawLine(dx + x1, y0 + dh0 + 2, dx + x1, y0 + dh1 - 1, m_ColTitleText);
			} else if (static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type == TW_TYPE_HELP_GRP && m_ColHelpBg != 0)
				Gr->DrawRect(m_PosX + m_VarX0 + m_HierTags[h].m_Var->m_LeftMargin, y0 + m_HierTags[h].m_Var->m_TopMargin, m_PosX + m_VarX2, y0 + m_Font->m_CharHeight - 1, m_ColHelpBg);
			// else if( static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type==TW_TYPE_HELP_HEADER && m_ColHelpBg!=0 )
			//   Gr->DrawRect(m_PosX+m_VarX0+m_HierTags[h].m_Var->m_LeftMargin, y0+m_HierTags[h].m_Var->m_TopMargin, m_PosX+m_VarX2, y0+m_Font->m_CharHeight-1, m_ColHelpBg);

			y0 = y1 + m_LineSep;
		}
	}

	if (m_NbDisplayedLines < m_NbHierLines) {
		// Draw scroll bar
		y0 = m_PosY + m_VarY0;
		y1 = m_PosY + m_VarY1;
		// x0 = m_PosX+2;
		// x1 = m_PosX+m_Font->m_CharHeight-2;
		x0 = m_PosX + m_VarX2 + 4;
		x1 = x0 + m_Font->m_CharHeight - 4;
		if (((x0 + x1) & 1) == 1)
			x1 += 1;
		w = m_ScrollYW;
		h = m_ScrollYH;

		Gr->DrawRect(x0 + 2, y0 + w, x1 - 2, y1 - 1 - w, (m_ColBg & 0xffffff) | 0x11000000);
		if (m_DrawHandles || m_IsPopupList) {
			// scroll handle shadow lines
			Gr->DrawLine(x1 - 1, m_ScrollY0 + 1, x1 - 1, m_ScrollY1 + 1, m_ColLineShadow);
			Gr->DrawLine(x0 + 2, m_ScrollY1 + 1, x1, m_ScrollY1 + 1, m_ColLineShadow);

			// up & down arrow
			for (i = 0; i < (x1 - x0 - 2) / 2; ++i) {
				Gr->DrawLine(x0 + 2 + i, y0 + w - 2 * i, x1 - i, y0 + w - 2 * i, m_ColLineShadow);
				Gr->DrawLine(x0 + 1 + i, y0 + w - 1 - 2 * i, x1 - 1 - i, y0 + w - 1 - 2 * i, m_HighlightUpScroll ? ((m_ColLine & 0xffffff) | 0x4f000000) : m_ColLine);

				Gr->DrawLine(x0 + 2 + i, y1 - w + 2 + 2 * i, x1 - i, y1 - w + 2 + 2 * i, m_ColLineShadow);
				Gr->DrawLine(x0 + 1 + i, y1 - w + 1 + 2 * i, x1 - 1 - i, y1 - w + 1 + 2 * i, m_HighlightDnScroll ? ((m_ColLine & 0xffffff) | 0x4f000000) : m_ColLine);
			}

			// middle lines
			Gr->DrawLine((x0 + x1) / 2 - 1, y0 + w, (x0 + x1) / 2 - 1, m_ScrollY0, m_ColLine);
			Gr->DrawLine((x0 + x1) / 2, y0 + w, (x0 + x1) / 2, m_ScrollY0, m_ColLine);
			Gr->DrawLine((x0 + x1) / 2 + 1, y0 + w, (x0 + x1) / 2 + 1, m_ScrollY0, m_ColLineShadow);
			Gr->DrawLine((x0 + x1) / 2 - 1, m_ScrollY1, (x0 + x1) / 2 - 1, y1 - w + 1, m_ColLine);
			Gr->DrawLine((x0 + x1) / 2, m_ScrollY1, (x0 + x1) / 2, y1 - w + 1, m_ColLine);
			Gr->DrawLine((x0 + x1) / 2 + 1, m_ScrollY1, (x0 + x1) / 2 + 1, y1 - w + 1, m_ColLineShadow);
			// scroll handle lines
			Gr->DrawRect(x0 + 2, m_ScrollY0 + 1, x1 - 3, m_ScrollY1 - 1, m_HighlightScroll ? m_ColHighBtn : m_ColBtn);
			Gr->DrawLine(x1 - 2, m_ScrollY0, x1 - 2, m_ScrollY1, m_ColLine);
			Gr->DrawLine(x0 + 1, m_ScrollY0, x0 + 1, m_ScrollY1, m_ColLine);
			Gr->DrawLine(x0 + 1, m_ScrollY1, x1 - 1, m_ScrollY1, m_ColLine);
			Gr->DrawLine(x0 + 1, m_ScrollY0, x1 - 2, m_ScrollY0, m_ColLine);
		} else
			Gr->DrawRect(x0 + 3, m_ScrollY0 + 1, x1 - 3, m_ScrollY1 - 1, m_ColBtn);
	}

	if (m_DrawHandles && !m_IsPopupList) {
		if (m_Resizable) { // Draw resize handles
			//   lower-left
			Gr->DrawLine(m_PosX + 3, m_PosY + m_Height - m_Font->m_CharHeight + 3, m_PosX + 3, m_PosY + m_Height - 4, m_ColLine);
			Gr->DrawLine(m_PosX + 4, m_PosY + m_Height - m_Font->m_CharHeight + 4, m_PosX + 4, m_PosY + m_Height - 3, m_ColLineShadow);
			Gr->DrawLine(m_PosX + 3, m_PosY + m_Height - 4, m_PosX + m_Font->m_CharHeight - 4, m_PosY + m_Height - 4, m_ColLine);
			Gr->DrawLine(m_PosX + 4, m_PosY + m_Height - 3, m_PosX + m_Font->m_CharHeight - 3, m_PosY + m_Height - 3, m_ColLineShadow);
			//   lower-right
			Gr->DrawLine(m_PosX + m_Width - 4, m_PosY + m_Height - m_Font->m_CharHeight + 3, m_PosX + m_Width - 4, m_PosY + m_Height - 4, m_ColLine);
			Gr->DrawLine(m_PosX + m_Width - 3, m_PosY + m_Height - m_Font->m_CharHeight + 4, m_PosX + m_Width - 3, m_PosY + m_Height - 3, m_ColLineShadow);
			Gr->DrawLine(m_PosX + m_Width - 4, m_PosY + m_Height - 4, m_PosX + m_Width - m_Font->m_CharHeight + 3, m_PosY + m_Height - 4, m_ColLine);
			Gr->DrawLine(m_PosX + m_Width - 3, m_PosY + m_Height - 3, m_PosX + m_Width - m_Font->m_CharHeight + 4, m_PosY + m_Height - 3, m_ColLineShadow);
			//   upper-left
			Gr->DrawLine(m_PosX + 3, m_PosY + m_Font->m_CharHeight - 4, m_PosX + 3, m_PosY + 3, m_ColLine);
			Gr->DrawLine(m_PosX + 4, m_PosY + m_Font->m_CharHeight - 3, m_PosX + 4, m_PosY + 4, m_ColLineShadow);
			Gr->DrawLine(m_PosX + 3, m_PosY + 3, m_PosX + m_Font->m_CharHeight - 4, m_PosY + 3, m_ColLine);
			Gr->DrawLine(m_PosX + 4, m_PosY + 4, m_PosX + m_Font->m_CharHeight - 3, m_PosY + 4, m_ColLineShadow);
			//   upper-right
			Gr->DrawLine(m_PosX + m_Width - 4, m_PosY + 3, m_PosX + m_Width - m_Font->m_CharHeight + 3, m_PosY + 3, m_ColLine);
			Gr->DrawLine(m_PosX + m_Width - 3, m_PosY + 4, m_PosX + m_Width - m_Font->m_CharHeight + 4, m_PosY + 4, m_ColLineShadow);
			Gr->DrawLine(m_PosX + m_Width - 4, m_PosY + m_Font->m_CharHeight - 4, m_PosX + m_Width - 4, m_PosY + 3, m_ColLine);
			Gr->DrawLine(m_PosX + m_Width - 3, m_PosY + m_Font->m_CharHeight - 3, m_PosX + m_Width - 3, m_PosY + 4, m_ColLineShadow);
		}

		int xm = m_PosX + m_Width - 2 * m_Font->m_CharHeight, wm = m_Font->m_CharHeight - 6;
		wm = (wm < 6) ? 6 : wm;
		if (m_Iconifiable) { // Draw minimize button
			Gr->DrawRect(xm + 1, m_PosY + 4, xm + wm - 1, m_PosY + 3 + wm, m_HighlightMinimize ? m_ColHighBtn : ((m_ColBtn & 0xffffff) | 0x4f000000));
			Gr->DrawLine(xm, m_PosY + 3, xm + wm, m_PosY + 3, m_ColLine);
			Gr->DrawLine(xm + wm, m_PosY + 3, xm + wm, m_PosY + 3 + wm, m_ColLine);
			Gr->DrawLine(xm + wm, m_PosY + 3 + wm, xm, m_PosY + 3 + wm, m_ColLine);
			Gr->DrawLine(xm, m_PosY + 3 + wm, xm, m_PosY + 3, m_ColLine);
			Gr->DrawLine(xm + wm + 1, m_PosY + 4, xm + wm + 1, m_PosY + 4 + wm, m_ColLineShadow);
			Gr->DrawLine(xm + wm + 1, m_PosY + 4 + wm, xm, m_PosY + 4 + wm, m_ColLineShadow);
			Gr->DrawLine(xm + wm / 3 + ((wm < 9) ? 1 : 0) - 1, m_PosY + 4 + wm / 3 - ((wm < 9) ? 0 : 1), xm + wm / 2, m_PosY + 2 + wm - 1, m_ColTitleText, true);
			Gr->DrawLine(xm + wm - wm / 3 + ((wm < 9) ? 0 : 1), m_PosY + 4 + wm / 3 - ((wm < 9) ? 0 : 1), xm + wm / 2, m_PosY + 2 + wm - 1, m_ColTitleText, true);
		}

		if (g_TwMgr->m_FontResizable) { // Draw font button
			xm = m_PosX + m_Font->m_CharHeight + 2;
			Gr->DrawRect(xm + 1, m_PosY + 4, xm + wm - 1, m_PosY + 3 + wm, m_HighlightFont ? m_ColHighBtn : ((m_ColBtn & 0xffffff) | 0x4f000000));
			Gr->DrawLine(xm, m_PosY + 3, xm + wm, m_PosY + 3, m_ColLine);
			Gr->DrawLine(xm + wm, m_PosY + 3, xm + wm, m_PosY + 3 + wm, m_ColLine);
			Gr->DrawLine(xm + wm, m_PosY + 3 + wm, xm, m_PosY + 3 + wm, m_ColLine);
			Gr->DrawLine(xm, m_PosY + 3 + wm, xm, m_PosY + 3, m_ColLine);
			Gr->DrawLine(xm + wm + 1, m_PosY + 4, xm + wm + 1, m_PosY + 4 + wm, m_ColLineShadow);
			Gr->DrawLine(xm + wm + 1, m_PosY + 4 + wm, xm, m_PosY + 4 + wm, m_ColLineShadow);
			Gr->DrawLine(xm + wm / 2 - wm / 6, m_PosY + 3 + wm / 3, xm + wm / 2 + wm / 6 + 1, m_PosY + 3 + wm / 3, m_ColTitleText);
			Gr->DrawLine(xm + wm / 2 - wm / 6, m_PosY + 3 + wm / 3, xm + wm / 2 - wm / 6, m_PosY + 4 + wm - wm / 3 + (wm > 11 ? 1 : 0), m_ColTitleText);
			Gr->DrawLine(xm + wm / 2 - wm / 6, m_PosY + 3 + wm / 2 + (wm > 11 ? 1 : 0), xm + wm / 2 + wm / 6, m_PosY + 3 + wm / 2 + (wm > 11 ? 1 : 0), m_ColTitleText);
		}
	}
}

//  ---------------------------------------------------------------------------

void CTwBar::Draw(int _DrawPart) {
	PERF(PerfTimer Timer; double DT;)

	DEV_ASSERT(m_Font);
	ITwGraph *Gr = g_TwMgr->m_Graph;

	m_CustomRecords.clear();

	if (float(g_BarTimer.GetTime()) > m_LastUpdateTime + m_UpdatePeriod)
		NotUpToDate();

	if (m_HighlightedLine != m_HighlightedLinePrev) {
		m_HighlightedLinePrev = m_HighlightedLine;
		NotUpToDate();
	}

	if (m_IsHelpBar && g_TwMgr->m_HelpBarNotUpToDate)
		g_TwMgr->UpdateHelpBar();

	if (!m_UpToDate)
		Update();

	if (!m_IsMinimized) {
		int y = m_PosY + 1;
		int LevelSpace = MAX(m_Font->m_CharHeight - 6, 4); // space used by DrawHierHandles

		color32 colBg = m_ColBg, colBg1 = m_ColBg1, colBg2 = m_ColBg2;
		if (m_DrawHandles || m_IsPopupList) {
			unsigned int alphaMin = 0x70;
			if (m_IsPopupList)
				alphaMin = 0xa0;
			if ((colBg >> 24) < alphaMin)
				colBg = (colBg & 0xffffff) | (alphaMin << 24);
			if ((colBg1 >> 24) < alphaMin)
				colBg1 = (colBg1 & 0xffffff) | (alphaMin << 24);
			if ((colBg2 >> 24) < alphaMin)
				colBg2 = (colBg2 & 0xffffff) | (alphaMin << 24);
		}

		// Draw title
		if (!m_IsPopupList) {
			PERF(Timer.Reset();)
			if (_DrawPart & DRAW_BG) {
				// Gr->DrawRect(m_PosX, m_PosY, m_PosX+m_Width-1, m_PosY+m_Font->m_CharHeight+1, (m_HighlightTitle||m_MouseDragTitle) ? m_ColTitleHighBg : (m_DrawHandles ? m_ColTitleBg : m_ColTitleUnactiveBg));
				if (m_HighlightTitle || m_MouseDragTitle)
					Gr->DrawRect(m_PosX, m_PosY, m_PosX + m_Width - 1, m_PosY + m_Font->m_CharHeight + 1, m_ColTitleHighBg);
				else if (m_DrawHandles)
					Gr->DrawRect(m_PosX, m_PosY, m_PosX + m_Width - 1, m_PosY + m_Font->m_CharHeight + 1, m_ColTitleBg, m_ColTitleBg, colBg2, colBg1);
				else
					Gr->DrawRect(m_PosX, m_PosY, m_PosX + m_Width - 1, m_PosY + m_Font->m_CharHeight + 1, m_ColTitleBg, m_ColTitleBg, colBg2, colBg1);
			}
			if (_DrawPart & DRAW_CONTENT) {
				const color32 COL0 = 0x50ffffff;
				const color32 COL1 = 0x501f1f1f;
				Gr->DrawRect(m_PosX, m_PosY, m_PosX + m_Width - 1, y, COL0, COL0, COL1, COL1);
				if (m_ColTitleShadow != 0)
					Gr->DrawText(m_TitleTextObj, m_PosX + (m_Width - m_TitleWidth) / 2 + 1, m_PosY + 1, m_ColTitleShadow, 0);
				Gr->DrawText(m_TitleTextObj, m_PosX + (m_Width - m_TitleWidth) / 2, m_PosY, m_ColTitleText, 0);
			}
			y = m_PosY + m_Font->m_CharHeight + 1;
			if (_DrawPart & DRAW_CONTENT && m_DrawHandles)
				Gr->DrawLine(m_PosX, y, m_PosX + m_Width - 1, y, 0x30ffffff); // 0x80afafaf);
			y++;
			PERF(DT = Timer.GetTime(); printf("Title=%.4fms ", 1000.0 * DT);)
		}

		// Draw background
		PERF(Timer.Reset();)
		if (_DrawPart & DRAW_BG) {
			Gr->DrawRect(m_PosX, y, m_PosX + m_Width - 1, m_PosY + m_Height - 1, colBg2, colBg1, colBg1, colBg);
			// Gr->DrawRect(m_PosX, y, m_PosX+m_VarX0-5, m_PosY+m_Height-1, m_ColHierBg);
			Gr->DrawRect(m_PosX + m_VarX2 + 3, y, m_PosX + m_Width - 1, m_PosY + m_Height - 1, m_ColHierBg);
		}

		if (_DrawPart & DRAW_CONTENT) {
			// Draw highlighted line
			if (m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var != NULL && (m_HierTags[m_HighlightedLine].m_Var->IsGroup() || (!static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly && !m_IsHelpBar && !m_HierTags[m_HighlightedLine].m_Var->IsCustom()))) // !(static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type>=TW_TYPE_CUSTOM_BASE && static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size()))) )
			{
				int y0 = m_PosY + m_VarY0 + m_HighlightedLine * (m_Font->m_CharHeight + m_LineSep);
				Gr->DrawRect(m_PosX + LevelSpace + 6 + LevelSpace * m_HierTags[m_HighlightedLine].m_Level, y0 + 1, m_PosX + m_VarX2, y0 + m_Font->m_CharHeight - 1 + m_LineSep - 1, m_ColHighBg0, m_ColHighBg0, m_ColHighBg1, m_ColHighBg1);
				int eps = ANT_PIXEL_OFFSET;
				if (!m_EditInPlace.m_Active)
					Gr->DrawLine(m_PosX + LevelSpace + 6 + LevelSpace * m_HierTags[m_HighlightedLine].m_Level, y0 + m_Font->m_CharHeight + m_LineSep - 1 + eps, m_PosX + m_VarX2, y0 + m_Font->m_CharHeight + m_LineSep - 1 + eps, m_ColUnderline);
			} else if (m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && !m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
				int y0 = m_PosY + m_VarY0 + m_HighlightedLine * (m_Font->m_CharHeight + m_LineSep);
				color32 col = ColorBlend(m_ColHighBg0, m_ColHighBg1, 0.5f);
				CTwVarAtom *Atom = static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
				if (!Atom->IsCustom() // !(Atom->m_Type>=TW_TYPE_CUSTOM_BASE && Atom->m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size())
						&& !(Atom->m_Type == TW_TYPE_BUTTON && Atom->m_Val.m_Button.m_Callback == NULL))
					Gr->DrawRect(m_PosX + LevelSpace + 6 + LevelSpace * m_HierTags[m_HighlightedLine].m_Level, y0 + 1, m_PosX + m_VarX2, y0 + m_Font->m_CharHeight - 1 + m_LineSep - 1, col);
				else
					Gr->DrawRect(m_PosX + LevelSpace + 6 + LevelSpace * m_HierTags[m_HighlightedLine].m_Level, y0 + 1, m_PosX + LevelSpace + 6 + LevelSpace * m_HierTags[m_HighlightedLine].m_Level + 4, y0 + m_Font->m_CharHeight - 1 + m_LineSep - 1, col);
			}
			color32 clight = 0x5FFFFFFF; // bar contour
			Gr->DrawLine(m_PosX, m_PosY, m_PosX, m_PosY + m_Height, clight);
			Gr->DrawLine(m_PosX, m_PosY, m_PosX + m_Width, m_PosY, clight);
			Gr->DrawLine(m_PosX + m_Width, m_PosY, m_PosX + m_Width, m_PosY + m_Height, clight);
			Gr->DrawLine(m_PosX, m_PosY + m_Height, m_PosX + m_Width, m_PosY + m_Height, clight);
			int dshad = 3; // bar shadows
			color32 cshad = (((m_Color >> 24) / 2) << 24) & 0xFF000000;
			Gr->DrawRect(m_PosX, m_PosY + m_Height, m_PosX + dshad, m_PosY + m_Height + dshad, 0, cshad, 0, 0);
			Gr->DrawRect(m_PosX + dshad + 1, m_PosY + m_Height, m_PosX + m_Width - 1, m_PosY + m_Height + dshad, cshad, cshad, 0, 0);
			Gr->DrawRect(m_PosX + m_Width, m_PosY + m_Height, m_PosX + m_Width + dshad, m_PosY + m_Height + dshad, cshad, 0, 0, 0);
			Gr->DrawRect(m_PosX + m_Width, m_PosY, m_PosX + m_Width + dshad, m_PosY + dshad, 0, 0, cshad, 0);
			Gr->DrawRect(m_PosX + m_Width, m_PosY + dshad + 1, m_PosX + m_Width + dshad, m_PosY + m_Height - 1, cshad, 0, cshad, 0);
			PERF(DT = Timer.GetTime(); printf("Bg=%.4fms ", 1000.0 * DT);)

			// Draw hierarchy handle
			PERF(Timer.Reset();)
			DrawHierHandle();
			PERF(DT = Timer.GetTime(); printf("Handles=%.4fms ", 1000.0 * DT);)

			// Draw labels
			PERF(Timer.Reset();)
			Gr->DrawText(m_LabelsTextObj, m_PosX + LevelSpace + 6, m_PosY + m_VarY0, 0 /*m_ColLabelText*/, 0);
			PERF(DT = Timer.GetTime(); printf("Labels=%.4fms ", 1000.0 * DT);)

			// Draw values
			if (!m_IsPopupList) {
				PERF(Timer.Reset();)
				Gr->DrawText(m_ValuesTextObj, m_PosX + m_VarX1, m_PosY + m_VarY0, 0 /*m_ColValText*/, 0 /*m_ColValBg*/);
				PERF(DT = Timer.GetTime(); printf("Values=%.4fms ", 1000.0 * DT);)
			}

			// Draw preview for color values and draw buttons and custom types
			int nh = (int)m_HierTags.size();
			int yh = m_PosY + m_VarY0;
			int bw = IncrBtnWidth(m_Font->m_CharHeight);
			for (int h = 0; h < nh; ++h) {
				if (m_HierTags[h].m_Var->IsGroup()) {
					const CTwVarGroup *Grp = static_cast<const CTwVarGroup *>(m_HierTags[h].m_Var);
					if (Grp->m_SummaryCallback == CColorExt::SummaryCB && Grp->m_StructValuePtr != nullptr) {
						// draw color value
						if (Grp->m_Vars.size() > 0 && Grp->m_Vars[0] != nullptr && !Grp->m_Vars[0]->IsGroup())
							static_cast<CTwVarAtom *>(Grp->m_Vars[0])->ValueToDouble(); // force ext update
						int ydecal = ANT_PIXEL_OFFSET;
						const int checker = 8;
						for (int c = 0; c < checker; ++c)
							Gr->DrawRect(m_PosX + m_VarX1 + (c * (m_VarX2 - m_VarX1)) / checker, yh + 1 + ydecal + ((c % 2) * (m_Font->m_CharHeight - 2)) / 2, m_PosX + m_VarX1 - 1 + ((c + 1) * (m_VarX2 - m_VarX1)) / checker, yh + ydecal + (((c % 2) + 1) * (m_Font->m_CharHeight - 2)) / 2, 0xffffffff);
						Gr->DrawRect(m_PosX + m_VarX1, yh + 1 + ydecal, m_PosX + m_VarX2 - 1, yh + ydecal + m_Font->m_CharHeight - 2, 0xbfffffff);
						const CColorExt *colExt = static_cast<const CColorExt *>(Grp->m_StructValuePtr);
						color32 col = Color32FromARGBi((colExt->m_HasAlpha ? colExt->A : 255), colExt->R, colExt->G, colExt->B);
						if (col != 0)
							Gr->DrawRect(m_PosX + m_VarX1, yh + 1 + ydecal, m_PosX + m_VarX2 - 1, yh + ydecal + m_Font->m_CharHeight - 2, col);
						// Gr->DrawLine(m_PosX+m_VarX1-1, yh, m_PosX+m_VarX2+1, yh, 0xff000000);
						// Gr->DrawLine(m_PosX+m_VarX1-1, yh+m_Font->m_CharHeight, m_PosX+m_VarX2+1, yh+m_Font->m_CharHeight, 0xff000000);
						// Gr->DrawLine(m_PosX+m_VarX1-1, yh, m_PosX+m_VarX1-1, yh+m_Font->m_CharHeight, 0xff000000);
						// Gr->DrawLine(m_PosX+m_VarX2, yh, m_PosX+m_VarX2, yh+m_Font->m_CharHeight, 0xff000000);
					}
					// else if( Grp->m_SummaryCallback==CustomTypeSummaryCB && Grp->m_StructValuePtr!=nullptr) { }
				} else if (static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type == TW_TYPE_BUTTON && !m_IsPopupList) {
					// draw button
					int cbx0, cbx1;
					if (m_ButtonAlign == BUTTON_ALIGN_LEFT) {
						cbx0 = m_PosX + m_VarX1 + 2;
						cbx1 = m_PosX + m_VarX1 + bw;
					} else if (m_ButtonAlign == BUTTON_ALIGN_CENTER) {
						cbx0 = m_PosX + (m_VarX1 + m_VarX2) / 2 - bw / 2 + 1;
						cbx1 = m_PosX + (m_VarX1 + m_VarX2) / 2 + bw / 2 - 1;
					} else {
						cbx0 = m_PosX + m_VarX2 - 2 * bw + bw / 2;
						cbx1 = m_PosX + m_VarX2 - 2 - bw / 2;
					}
					int cby0 = yh + 3;
					int cby1 = yh + m_Font->m_CharHeight - 3;
					if (!static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_ReadOnly) {
						double BtnAutoDelta = g_TwMgr->m_Timer.GetTime() - m_HighlightClickBtnAuto;
						if ((m_HighlightClickBtn || (BtnAutoDelta >= 0 && BtnAutoDelta < 0.1)) && h == m_HighlightedLine) {
							cbx0--;
							cby0--;
							cbx1--;
							cby1--;
							Gr->DrawRect(cbx0 + 2, cby0 + 2, cbx1 + 2, cby1 + 2, m_ColHighBtn);
							Gr->DrawLine(cbx0 + 3, cby1 + 3, cbx1 + 4, cby1 + 3, 0xAF000000);
							Gr->DrawLine(cbx1 + 3, cby0 + 3, cbx1 + 3, cby1 + 3, 0xAF000000);
							Gr->DrawLine(cbx0 + 2, cby0 + 2, cbx0 + 2, cby1 + 2, m_ColLine);
							Gr->DrawLine(cbx0 + 2, cby1 + 2, cbx1 + 2, cby1 + 2, m_ColLine);
							Gr->DrawLine(cbx1 + 2, cby1 + 2, cbx1 + 2, cby0 + 2, m_ColLine);
							Gr->DrawLine(cbx1 + 2, cby0 + 2, cbx0 + 2, cby0 + 2, m_ColLine);
						} else {
							Gr->DrawRect(cbx0 + 2, cby1 + 1, cbx1 + 2, cby1 + 2, (h == m_HighlightedLine) ? 0xAF000000 : 0x7F000000);
							Gr->DrawRect(cbx1 + 1, cby0 + 2, cbx1 + 2, cby1, (h == m_HighlightedLine) ? 0xAF000000 : 0x7F000000);
							Gr->DrawRect(cbx0, cby0, cbx1, cby1, (h == m_HighlightedLine) ? m_ColHighBtn : m_ColBtn);
							Gr->DrawLine(cbx0, cby0, cbx0, cby1, m_ColLine);
							Gr->DrawLine(cbx0, cby1, cbx1, cby1, m_ColLine);
							Gr->DrawLine(cbx1, cby1, cbx1, cby0, m_ColLine);
							Gr->DrawLine(cbx1, cby0, cbx0, cby0, m_ColLine);
						}
					} else if (static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_Val.m_Button.m_Callback != NULL) {
						Gr->DrawRect(cbx0 + 1, cby0 + 1, cbx1 + 1, cby1 + 1, m_ColBtn);
					} else if (static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_Val.m_Button.m_Separator == 1) {
						int LevelSpace = MAX(m_Font->m_CharHeight - 6, 4); // space used by DrawHierHandles
						Gr->DrawLine(m_PosX + m_VarX0 + m_HierTags[h].m_Level * LevelSpace, yh + m_Font->m_CharHeight / 2, m_PosX + m_VarX2, yh + m_Font->m_CharHeight / 2, m_ColSeparator);
					}
				} else if (m_HierTags[h].m_Var->IsCustom()) //static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type>=TW_TYPE_CUSTOM_BASE && static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_Type<TW_TYPE_CUSTOM_BASE+(int)g_TwMgr->m_Customs.size() )
				{ // record custom types
					CTwMgr::CMemberProxy *mProxy = static_cast<CTwVarAtom *>(m_HierTags[h].m_Var)->m_Val.m_Custom.m_MemberProxy;
					if (mProxy != NULL && mProxy->m_StructProxy != NULL) {
						CustomMap::iterator it = m_CustomRecords.find(mProxy->m_StructProxy);
						int xMin = m_PosX + m_VarX0 + m_HierTags[h].m_Level * LevelSpace;
						int xMax = m_PosX + m_VarX2 - 2;
						int yMin = yh + 1;
						int yMax = yh + m_Font->m_CharHeight;
						if (it == m_CustomRecords.end()) {
							std::pair<CTwMgr::CStructProxy *, CCustomRecord> pr;
							pr.first = mProxy->m_StructProxy;
							pr.second.m_IndexMin = pr.second.m_IndexMax = mProxy->m_MemberIndex;
							pr.second.m_XMin = xMin;
							pr.second.m_XMax = xMax;
							pr.second.m_YMin = yMin;
							pr.second.m_YMax = yMax;
							pr.second.m_Y0 = 0; // will be filled by the draw loop below
							pr.second.m_Y1 = 0; // will be filled by the draw loop below
							pr.second.m_Var = mProxy->m_VarParent;
							m_CustomRecords.insert(pr);
						} else {
							it->second.m_IndexMin = MIN(it->second.m_IndexMin, mProxy->m_MemberIndex);
							it->second.m_IndexMax = MIN(it->second.m_IndexMax, mProxy->m_MemberIndex);
							it->second.m_XMin = MIN(it->second.m_XMin, xMin);
							it->second.m_XMax = MAX(it->second.m_XMax, xMax);
							it->second.m_YMin = MIN(it->second.m_YMin, yMin);
							it->second.m_YMax = MAX(it->second.m_YMax, yMax);
							it->second.m_Y0 = 0;
							it->second.m_Y1 = 0;
							assert(it->second.m_Var == mProxy->m_VarParent);
						}
					}
				}

				yh += m_Font->m_CharHeight + m_LineSep;
			}

			// Draw custom types
			for (CustomMap::iterator it = m_CustomRecords.begin(); it != m_CustomRecords.end(); ++it) {
				CTwMgr::CStructProxy *sProxy = it->first;
				assert(sProxy != NULL);
				CCustomRecord &r = it->second;
				if (sProxy->m_CustomDrawCallback != NULL) {
					int y0 = r.m_YMin - MAX(r.m_IndexMin - sProxy->m_CustomIndexFirst, 0) * (m_Font->m_CharHeight + m_LineSep);
					int y1 = y0 + MAX(sProxy->m_CustomIndexLast - sProxy->m_CustomIndexFirst + 1, 0) * (m_Font->m_CharHeight + m_LineSep) - 2;
					if (y0 < y1) {
						r.m_Y0 = y0;
						r.m_Y1 = y1;
						Gr->ChangeViewport(r.m_XMin, r.m_YMin, r.m_XMax - r.m_XMin + 1, r.m_YMax - r.m_YMin + 1, 0, y0 - r.m_YMin + 1);
						sProxy->m_CustomDrawCallback(r.m_XMax - r.m_XMin, y1 - y0, sProxy->m_StructExtData, sProxy->m_StructClientData, this, r.m_Var);
						Gr->RestoreViewport();
					}
				}
			}

			if (m_DrawHandles && !m_IsPopupList) {
				// Draw -/+/o/click/v buttons
				if ((m_DrawIncrDecrBtn || m_DrawClickBtn || m_DrawListBtn || m_DrawBoolBtn || m_DrawRotoBtn) && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size()) {
					int y0 = m_PosY + m_VarY0 + m_HighlightedLine * (m_Font->m_CharHeight + m_LineSep);
					if (m_DrawIncrDecrBtn) {
						bool IsMin = false;
						bool IsMax = false;
						if (!m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
							const CTwVarAtom *Atom = static_cast<const CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
							double v, vmin, vmax;
							v = Atom->ValueToDouble();
							Atom->MinMaxStepToDouble(&vmin, &vmax, NULL);
							IsMax = (v >= vmax);
							IsMin = (v <= vmin);
						}

						// Gr->DrawRect(m_PosX+m_VarX2-2*bw+1, y0+1, m_PosX+m_VarX2-bw-1, y0+m_Font->m_CharHeight-2, (m_HighlightDecrBtn && !IsMin)?m_ColHighBtn:m_ColBtn);
						// Gr->DrawRect(m_PosX+m_VarX2-bw+1, y0+1, m_PosX+m_VarX2-1, y0+m_Font->m_CharHeight-2, (m_HighlightIncrBtn && !IsMax)?m_ColHighBtn:m_ColBtn);
						// [-]
						// Gr->DrawLine(m_PosX+m_VarX2-2*bw+3+(bw>8?1:0), y0+m_Font->m_CharHeight/2, m_PosX+m_VarX2-bw-2-(bw>8?1:0), y0+m_Font->m_CharHeight/2, IsMin?m_ColValTextRO:m_ColTitleText);
						// [+]
						// Gr->DrawLine(m_PosX+m_VarX2-bw+3, y0+m_Font->m_CharHeight/2, m_PosX+m_VarX2-2, y0+m_Font->m_CharHeight/2, IsMax?m_ColValTextRO:m_ColTitleText);
						// Gr->DrawLine(m_PosX+m_VarX2-bw/2, y0+m_Font->m_CharHeight/2-bw/2+2, m_PosX+m_VarX2-bw/2, y0+m_Font->m_CharHeight/2+bw/2-1, IsMax?m_ColValTextRO:m_ColTitleText);
						Gr->DrawRect(m_PosX + m_VarX2 - 3 * bw + 1, y0 + 1, m_PosX + m_VarX2 - 2 * bw - 1, y0 + m_Font->m_CharHeight - 2, (m_HighlightDecrBtn && !IsMin) ? m_ColHighBtn : m_ColBtn);
						Gr->DrawRect(m_PosX + m_VarX2 - 2 * bw + 1, y0 + 1, m_PosX + m_VarX2 - bw - 1, y0 + m_Font->m_CharHeight - 2, (m_HighlightIncrBtn && !IsMax) ? m_ColHighBtn : m_ColBtn);
						// [-]
						Gr->DrawLine(m_PosX + m_VarX2 - 3 * bw + 3 + (bw > 8 ? 1 : 0), y0 + m_Font->m_CharHeight / 2, m_PosX + m_VarX2 - 2 * bw - 2 - (bw > 8 ? 1 : 0), y0 + m_Font->m_CharHeight / 2, IsMin ? m_ColValTextRO : m_ColTitleText);
						// [+]
						Gr->DrawLine(m_PosX + m_VarX2 - 2 * bw + 3, y0 + m_Font->m_CharHeight / 2, m_PosX + m_VarX2 - bw - 2, y0 + m_Font->m_CharHeight / 2, IsMax ? m_ColValTextRO : m_ColTitleText);
						Gr->DrawLine(m_PosX + m_VarX2 - bw - bw / 2, y0 + m_Font->m_CharHeight / 2 - bw / 2 + 2, m_PosX + m_VarX2 - bw - bw / 2, y0 + m_Font->m_CharHeight / 2 + bw / 2 - 1, IsMax ? m_ColValTextRO : m_ColTitleText);
					} else if (m_DrawListBtn) {
						// [v]
						int eps = 1;
						int dx = -1;
						Gr->DrawRect(m_PosX + m_VarX2 - bw + 1, y0 + 1, m_PosX + m_VarX2 - 1, y0 + m_Font->m_CharHeight - 2, m_HighlightListBtn ? m_ColHighBtn : m_ColBtn);
						Gr->DrawLine(m_PosX + m_VarX2 - bw + 4 + dx, y0 + m_Font->m_CharHeight / 2 - eps, m_PosX + m_VarX2 - bw / 2 + 1 + dx, y0 + m_Font->m_CharHeight - 4, m_ColTitleText, true);
						Gr->DrawLine(m_PosX + m_VarX2 - bw / 2 + 1 + dx, y0 + m_Font->m_CharHeight - 4, m_PosX + m_VarX2 - 2 + dx, y0 + m_Font->m_CharHeight / 2 - 1, m_ColTitleText, true);
					} else if (m_DrawBoolBtn) {
						Gr->DrawRect(m_PosX + m_VarX2 - bw + 1, y0 + 1, m_PosX + m_VarX2 - 1, y0 + m_Font->m_CharHeight - 2, m_HighlightBoolBtn ? m_ColHighBtn : m_ColBtn);
						// [x]
						// Gr->DrawLine(m_PosX+m_VarX2-bw/2-bw/6, y0+m_Font->m_CharHeight/2-bw/6, m_PosX+m_VarX2-bw/2+bw/6, y0+m_Font->m_CharHeight/2+bw/6, m_ColTitleText, true);
						// Gr->DrawLine(m_PosX+m_VarX2-bw/2-bw/6, y0+m_Font->m_CharHeight/2+bw/6, m_PosX+m_VarX2-bw/2+bw/6, y0+m_Font->m_CharHeight/2-bw/6, m_ColTitleText, true);
						// [<>]
						int s = bw / 4;
						int eps = 1;
						Gr->DrawLine(m_PosX + m_VarX2 - bw / 2 - 1, y0 + m_Font->m_CharHeight / 2 - s, m_PosX + m_VarX2 - bw / 2 - s - 1, y0 + m_Font->m_CharHeight / 2, m_ColTitleText, true);
						Gr->DrawLine(m_PosX + m_VarX2 - bw / 2 - s - 1, y0 + m_Font->m_CharHeight / 2, m_PosX + m_VarX2 - bw / 2 - eps, y0 + m_Font->m_CharHeight / 2 + s + 1 - eps, m_ColTitleText, true);
						// Gr->DrawLine(m_PosX+m_VarX2-bw/2+1, y0+m_Font->m_CharHeight/2+s, m_PosX+m_VarX2-bw/2+s+1, y0+m_Font->m_CharHeight/2, m_ColTitleText, true);
						// Gr->DrawLine(m_PosX+m_VarX2-bw/2+s+1, y0+m_Font->m_CharHeight/2, m_PosX+m_VarX2-bw/2+1, y0+m_Font->m_CharHeight/2-s, m_ColTitleText, true);
						Gr->DrawLine(m_PosX + m_VarX2 - bw / 2 + 2, y0 + m_Font->m_CharHeight / 2 - s, m_PosX + m_VarX2 - bw / 2 + s + 2, y0 + m_Font->m_CharHeight / 2, m_ColTitleText, true);
						Gr->DrawLine(m_PosX + m_VarX2 - bw / 2 + s + 2, y0 + m_Font->m_CharHeight / 2, m_PosX + m_VarX2 - bw / 2 + 1 + eps, y0 + m_Font->m_CharHeight / 2 + s + 1 - eps, m_ColTitleText, true);
					}

					if (m_DrawRotoBtn) {
						// [o] rotoslider button
						// Gr->DrawRect(m_PosX+m_VarX1-bw-1, y0+1, m_PosX+m_VarX1-3, y0+m_Font->m_CharHeight-2, m_HighlightRotoBtn?m_ColHighBtn:m_ColBtn);
						// Gr->DrawLine(m_PosX+m_VarX1-bw+bw/2-2, y0+m_Font->m_CharHeight/2-1, m_PosX+m_VarX1-bw+bw/2-1, y0+m_Font->m_CharHeight/2-1, m_ColTitleText);
						// Gr->DrawLine(m_PosX+m_VarX1-bw+bw/2-3, y0+m_Font->m_CharHeight/2+0, m_PosX+m_VarX1-bw+bw/2+0, y0+m_Font->m_CharHeight/2+0, m_ColTitleText);
						// Gr->DrawLine(m_PosX+m_VarX1-bw+bw/2-3, y0+m_Font->m_CharHeight/2+1, m_PosX+m_VarX1-bw+bw/2+0, y0+m_Font->m_CharHeight/2+1, m_ColTitleText);
						// Gr->DrawLine(m_PosX+m_VarX1-bw+bw/2-2, y0+m_Font->m_CharHeight/2+2, m_PosX+m_VarX1-bw+bw/2-1, y0+m_Font->m_CharHeight/2+2, m_ColTitleText);

						// Gr->DrawRect(m_PosX+m_VarX2-3*bw+1, y0+1, m_PosX+m_VarX2-2*bw-1, y0+m_Font->m_CharHeight-2, m_HighlightRotoBtn?m_ColHighBtn:m_ColBtn);
						// Gr->DrawLine(m_PosX+m_VarX2-3*bw+bw/2+0, y0+m_Font->m_CharHeight/2-1, m_PosX+m_VarX2-3*bw+bw/2+1, y0+m_Font->m_CharHeight/2-1, m_ColTitleText);
						// Gr->DrawLine(m_PosX+m_VarX2-3*bw+bw/2-1, y0+m_Font->m_CharHeight/2+0, m_PosX+m_VarX2-3*bw+bw/2+2, y0+m_Font->m_CharHeight/2+0, m_ColTitleText);
						// Gr->DrawLine(m_PosX+m_VarX2-3*bw+bw/2-1, y0+m_Font->m_CharHeight/2+1, m_PosX+m_VarX2-3*bw+bw/2+2, y0+m_Font->m_CharHeight/2+1, m_ColTitleText);
						// Gr->DrawLine(m_PosX+m_VarX2-3*bw+bw/2+0, y0+m_Font->m_CharHeight/2+2, m_PosX+m_VarX2-3*bw+bw/2+1, y0+m_Font->m_CharHeight/2+2, m_ColTitleText);

						int dy = 0;
						Gr->DrawRect(m_PosX + m_VarX2 - bw + 1, y0 + 1, m_PosX + m_VarX2 - 1, y0 + m_Font->m_CharHeight - 2, m_HighlightRotoBtn ? m_ColHighBtn : m_ColBtn);
						Gr->DrawLine(m_PosX + m_VarX2 - bw + bw / 2 + 0, y0 + m_Font->m_CharHeight / 2 - 1 + dy, m_PosX + m_VarX2 - bw + bw / 2 + 1, y0 + m_Font->m_CharHeight / 2 - 1 + dy, m_ColTitleText, true);
						Gr->DrawLine(m_PosX + m_VarX2 - bw + bw / 2 - 1, y0 + m_Font->m_CharHeight / 2 + 0 + dy, m_PosX + m_VarX2 - bw + bw / 2 + 2, y0 + m_Font->m_CharHeight / 2 + 0 + dy, m_ColTitleText, true);
						Gr->DrawLine(m_PosX + m_VarX2 - bw + bw / 2 - 1, y0 + m_Font->m_CharHeight / 2 + 1 + dy, m_PosX + m_VarX2 - bw + bw / 2 + 2, y0 + m_Font->m_CharHeight / 2 + 1 + dy, m_ColTitleText, true);
						Gr->DrawLine(m_PosX + m_VarX2 - bw + bw / 2 + 0, y0 + m_Font->m_CharHeight / 2 + 2 + dy, m_PosX + m_VarX2 - bw + bw / 2 + 1, y0 + m_Font->m_CharHeight / 2 + 2 + dy, m_ColTitleText, true);
					}
				}

				// Draw value width slider
				if (!m_HighlightValWidth) {
					color32 col = m_DarkText ? COLOR32_WHITE : m_ColTitleText;
					Gr->DrawRect(m_PosX + m_VarX1 - 2, m_PosY + m_VarY0 - 8, m_PosX + m_VarX1 - 1, m_PosY + m_VarY0 - 4, col);
					Gr->DrawLine(m_PosX + m_VarX1 - 1, m_PosY + m_VarY0 - 3, m_PosX + m_VarX1, m_PosY + m_VarY0 - 3, m_ColLineShadow);
					Gr->DrawLine(m_PosX + m_VarX1, m_PosY + m_VarY0 - 3, m_PosX + m_VarX1, m_PosY + m_VarY0 - 8, m_ColLineShadow);
				} else {
					color32 col = m_DarkText ? COLOR32_WHITE : m_ColTitleText;
					Gr->DrawRect(m_PosX + m_VarX1 - 2, m_PosY + m_VarY0 - 8, m_PosX + m_VarX1 - 1, m_PosY + m_VarY1, col);
					Gr->DrawLine(m_PosX + m_VarX1 - 1, m_PosY + m_VarY1 + 1, m_PosX + m_VarX1, m_PosY + m_VarY1 + 1, m_ColLineShadow);
					Gr->DrawLine(m_PosX + m_VarX1, m_PosY + m_VarY1 + 1, m_PosX + m_VarX1, m_PosY + m_VarY0 - 8, m_ColLineShadow);
				}

				// Draw labels & values headers
				if (m_HighlightLabelsHeader) {
					Gr->DrawRect(m_PosX + m_VarX0, m_PosY + m_Font->m_CharHeight + 2, m_PosX + m_VarX1 - 4, m_PosY + m_VarY0 - 1, m_ColHighBg0, m_ColHighBg0, m_ColHighBg1, m_ColHighBg1);
				}
				if (m_HighlightValuesHeader) {
					Gr->DrawRect(m_PosX + m_VarX1 + 2, m_PosY + m_Font->m_CharHeight + 2, m_PosX + m_VarX2, m_PosY + m_VarY0 - 1, m_ColHighBg0, m_ColHighBg0, m_ColHighBg1, m_ColHighBg1);
				}
			}

			// Draw key shortcut text
			if (m_HighlightedLine >= 0 && m_HighlightedLine == m_ShortcutLine && !m_IsPopupList && !m_EditInPlace.m_Active) {
				PERF(Timer.Reset();)
				Gr->DrawRect(m_PosX + m_Font->m_CharHeight - 2, m_PosY + m_VarY1 + 1, m_PosX + m_Width - m_Font->m_CharHeight - 2, m_PosY + m_VarY1 + 1 + m_Font->m_CharHeight, m_ColShortcutBg);
				Gr->DrawText(m_ShortcutTextObj, m_PosX + m_Font->m_CharHeight, m_PosY + m_VarY1 + 1, m_ColShortcutText, 0);
				PERF(DT = Timer.GetTime(); printf("Shortcut=%.4fms ", 1000.0 * DT);)
			} else if ((m_HighlightLabelsHeader || m_HighlightValuesHeader) && !m_IsPopupList && !m_EditInPlace.m_Active) {
				Gr->DrawRect(m_PosX + m_Font->m_CharHeight - 2, m_PosY + m_VarY1 + 1, m_PosX + m_Width - m_Font->m_CharHeight - 2, m_PosY + m_VarY1 + 1 + m_Font->m_CharHeight, m_ColShortcutBg);
				Gr->DrawText(m_HeadersTextObj, m_PosX + m_Font->m_CharHeight, m_PosY + m_VarY1 + 1, m_ColShortcutText, 0);
			} else if (m_IsHelpBar) {
				if (g_TwMgr->m_KeyPressedTextObj && g_TwMgr->m_KeyPressedStr.size() > 0) { // Draw key pressed
					if (g_TwMgr->m_KeyPressedBuildText) {
						std::string Str = g_TwMgr->m_KeyPressedStr;
						ClampText(Str, m_Font, m_Width - 2 * m_Font->m_CharHeight);
						g_TwMgr->m_Graph->BuildText(g_TwMgr->m_KeyPressedTextObj, &Str, nullptr, nullptr, 1, g_TwMgr->m_HelpBar->m_Font, 0, 0);
						g_TwMgr->m_KeyPressedBuildText = false;
						g_TwMgr->m_KeyPressedTime = (float)g_BarTimer.GetTime();
					}
					if ((float)g_BarTimer.GetTime() > g_TwMgr->m_KeyPressedTime + 1) // draw key pressed at least 1 second
						g_TwMgr->m_KeyPressedStr = "";
					PERF(Timer.Reset();)
					Gr->DrawRect(m_PosX + m_Font->m_CharHeight - 2, m_PosY + m_VarY1 + 1, m_PosX + m_Width - m_Font->m_CharHeight - 2, m_PosY + m_VarY1 + 1 + m_Font->m_CharHeight, m_ColShortcutBg);
					Gr->DrawText(g_TwMgr->m_KeyPressedTextObj, m_PosX + m_Font->m_CharHeight, m_PosY + m_VarY1 + 1, m_ColShortcutText, 0);
					PERF(DT = Timer.GetTime(); printf("KeyPressed=%.4fms ", 1000.0 * DT);)
				} else {
					if (g_TwMgr->m_InfoBuildText) {
						std::string Info = "atb ";
						char Ver[64];
						snprintf(Ver, 64, " %d.%02d", TW_VERSION / 100, TW_VERSION % 100);
						Info += Ver;
						ClampText(Info, m_Font, m_Width - 2 * m_Font->m_CharHeight);
						g_TwMgr->m_Graph->BuildText(g_TwMgr->m_InfoTextObj, &Info, nullptr, nullptr, 1, g_TwMgr->m_HelpBar->m_Font, 0, 0);
						g_TwMgr->m_InfoBuildText = false;
					}
					PERF(Timer.Reset();)
					Gr->DrawRect(m_PosX + m_Font->m_CharHeight - 2, m_PosY + m_VarY1 + 1, m_PosX + m_Width - m_Font->m_CharHeight - 2, m_PosY + m_VarY1 + 1 + m_Font->m_CharHeight, m_ColShortcutBg);
					Gr->DrawText(g_TwMgr->m_InfoTextObj, m_PosX + m_Font->m_CharHeight, m_PosY + m_VarY1 + 1, m_ColInfoText, 0);
					PERF(DT = Timer.GetTime(); printf("Info=%.4fms ", 1000.0 * DT);)
				}
			}

			if (!m_IsPopupList) {
				RotoDraw(); // Draw RotoSlider
				EditInPlaceDraw(); // Draw EditInPlace
			}

			if (g_TwMgr->m_PopupBar != nullptr && this != g_TwMgr->m_PopupBar) {
				// darken bar if a popup bar is displayed
				Gr->DrawRect(m_PosX, m_PosY, m_PosX + m_Width - 1, m_PosY + m_Height - 1, 0x1F000000);
			}
		}
	} else { // minimized
		int vpx, vpy, vpw, vph;
		vpx = 0;
		vpy = 0;
		vpw = g_TwMgr->m_WndWidth;
		vph = g_TwMgr->m_WndHeight;
		if (g_TwMgr->m_IconMarginX > 0) {
			vpx = MIN(g_TwMgr->m_IconMarginX, vpw / 3);
			vpw -= 2 * vpx;
		}
		if (g_TwMgr->m_IconMarginY > 0) {
			vpy = MIN(g_TwMgr->m_IconMarginY, vph / 3);
			vph -= 2 * vpy;
		}

		int MinXOffset = 0, MinYOffset = 0;
		if (g_TwMgr->m_IconPos == 3) { // top-right
			if (g_TwMgr->m_IconAlign == 1) { // horizontal
				int n = MAX(1, vpw / m_Font->m_CharHeight - 1);
				m_MinPosX = vpx + vpw - ((m_MinNumber % n) + 1) * m_Font->m_CharHeight;
				m_MinPosY = vpy + (m_MinNumber / n) * m_Font->m_CharHeight;
				MinYOffset = m_Font->m_CharHeight;
				MinXOffset = -m_TitleWidth;
			} else { // vertical
				int n = MAX(1, vph / m_Font->m_CharHeight - 1);
				m_MinPosY = vpy + (m_MinNumber % n) * m_Font->m_CharHeight;
				m_MinPosX = vpx + vpw - ((m_MinNumber / n) + 1) * m_Font->m_CharHeight;
				MinXOffset = -m_TitleWidth - m_Font->m_CharHeight;
			}
		} else if (g_TwMgr->m_IconPos == 2) { // top-left
			if (g_TwMgr->m_IconAlign == 1) { // horizontal
				int n = MAX(1, vpw / m_Font->m_CharHeight - 1);
				m_MinPosX = vpx + (m_MinNumber % n) * m_Font->m_CharHeight;
				m_MinPosY = vpy + (m_MinNumber / n) * m_Font->m_CharHeight;
				MinYOffset = m_Font->m_CharHeight;
			} else { // vertical
				int n = MAX(1, vph / m_Font->m_CharHeight - 1);
				m_MinPosY = vpy + (m_MinNumber % n) * m_Font->m_CharHeight;
				m_MinPosX = vpx + (m_MinNumber / n) * m_Font->m_CharHeight;
				MinXOffset = m_Font->m_CharHeight;
			}
		} else if (g_TwMgr->m_IconPos == 1) { // bottom-right
			if (g_TwMgr->m_IconAlign == 1) { // horizontal
				int n = MAX(1, vpw / m_Font->m_CharHeight - 1);
				m_MinPosX = vpx + vpw - ((m_MinNumber % n) + 1) * m_Font->m_CharHeight;
				m_MinPosY = vpy + vph - ((m_MinNumber / n) + 1) * m_Font->m_CharHeight;
				MinYOffset = -m_Font->m_CharHeight;
				MinXOffset = -m_TitleWidth;
			} else { // vertical
				int n = MAX(1, vph / m_Font->m_CharHeight - 1);
				m_MinPosY = vpy + vph - ((m_MinNumber % n) + 1) * m_Font->m_CharHeight;
				m_MinPosX = vpx + vpw - ((m_MinNumber / n) + 1) * m_Font->m_CharHeight;
				MinXOffset = -m_TitleWidth - m_Font->m_CharHeight;
			}
		} else { // bottom-left
			if (g_TwMgr->m_IconAlign == 1) { // horizontal
				int n = MAX(1, vpw / m_Font->m_CharHeight - 1);
				m_MinPosX = vpx + (m_MinNumber % n) * m_Font->m_CharHeight;
				m_MinPosY = vpy + vph - ((m_MinNumber / n) + 1) * m_Font->m_CharHeight;
				MinYOffset = -m_Font->m_CharHeight;
			} else { // vertical
				int n = MAX(1, vph / m_Font->m_CharHeight - 1);
				m_MinPosY = vpy + vph - ((m_MinNumber % n) + 1) * m_Font->m_CharHeight;
				m_MinPosX = vpx + (m_MinNumber / n) * m_Font->m_CharHeight;
				MinXOffset = m_Font->m_CharHeight;
			}
		}

		if (m_HighlightMaximize) {
			// Draw title
			if (_DrawPart & DRAW_BG) {
				Gr->DrawRect(m_MinPosX, m_MinPosY, m_MinPosX + m_Font->m_CharHeight, m_MinPosY + m_Font->m_CharHeight, m_ColTitleUnactiveBg);
				Gr->DrawRect(m_MinPosX + MinXOffset, m_MinPosY + MinYOffset, m_MinPosX + MinXOffset + m_TitleWidth + m_Font->m_CharHeight, m_MinPosY + MinYOffset + m_Font->m_CharHeight, m_ColTitleUnactiveBg);
			}
			if (_DrawPart & DRAW_CONTENT) {
				if (m_ColTitleShadow != 0)
					Gr->DrawText(m_TitleTextObj, m_MinPosX + MinXOffset + m_Font->m_CharHeight / 2, m_MinPosY + 1 + MinYOffset, m_ColTitleShadow, 0);
				Gr->DrawText(m_TitleTextObj, m_MinPosX + MinXOffset + m_Font->m_CharHeight / 2, m_MinPosY + MinYOffset, m_ColTitleText, 0);
			}
		}

		if (!m_IsHelpBar) {
			// Draw maximize button
			int xm = m_MinPosX + 2, wm = m_Font->m_CharHeight - 6;
			wm = (wm < 6) ? 6 : wm;
			if (_DrawPart & DRAW_BG)
				Gr->DrawRect(xm + 1, m_MinPosY + 4, xm + wm - 1, m_MinPosY + 3 + wm, m_HighlightMaximize ? m_ColHighBtn : m_ColBtn);
			if (_DrawPart & DRAW_CONTENT) {
				Gr->DrawLine(xm, m_MinPosY + 3, xm + wm, m_MinPosY + 3, m_ColLine);
				Gr->DrawLine(xm + wm, m_MinPosY + 3, xm + wm, m_MinPosY + 3 + wm, m_ColLine);
				Gr->DrawLine(xm + wm, m_MinPosY + 3 + wm, xm, m_MinPosY + 3 + wm, m_ColLine);
				Gr->DrawLine(xm, m_MinPosY + 3 + wm, xm, m_MinPosY + 3, m_ColLine);
				Gr->DrawLine(xm + wm + 1, m_MinPosY + 4, xm + wm + 1, m_MinPosY + 4 + wm, m_ColLineShadow);
				Gr->DrawLine(xm + wm + 1, m_MinPosY + 4 + wm, xm, m_MinPosY + 4 + wm, m_ColLineShadow);
				Gr->DrawLine(xm + wm / 3 - 1, m_MinPosY + 3 + wm - wm / 3, xm + wm / 2, m_MinPosY + 6, m_ColTitleText, true);
				Gr->DrawLine(xm + wm - wm / 3 + 1, m_MinPosY + 3 + wm - wm / 3, xm + wm / 2, m_MinPosY + 6, m_ColTitleText, true);
			}
		} else {
			// Draw help button
			int xm = m_MinPosX + 2, wm = m_Font->m_CharHeight - 6;
			wm = (wm < 6) ? 6 : wm;
			if (_DrawPart & DRAW_BG)
				Gr->DrawRect(xm + 1, m_MinPosY + 4, xm + wm - 1, m_MinPosY + 3 + wm, m_HighlightMaximize ? m_ColHighBtn : m_ColBtn);
			if (_DrawPart & DRAW_CONTENT) {
				Gr->DrawLine(xm, m_MinPosY + 3, xm + wm, m_MinPosY + 3, m_ColLine);
				Gr->DrawLine(xm + wm, m_MinPosY + 3, xm + wm, m_MinPosY + 3 + wm, m_ColLine);
				Gr->DrawLine(xm + wm, m_MinPosY + 3 + wm, xm, m_MinPosY + 3 + wm, m_ColLine);
				Gr->DrawLine(xm, m_MinPosY + 3 + wm, xm, m_MinPosY + 3, m_ColLine);
				Gr->DrawLine(xm + wm + 1, m_MinPosY + 4, xm + wm + 1, m_MinPosY + 4 + wm, m_ColLineShadow);
				Gr->DrawLine(xm + wm + 1, m_MinPosY + 4 + wm, xm, m_MinPosY + 4 + wm, m_ColLineShadow);
				Gr->DrawLine(xm + wm / 2 - wm / 6, m_MinPosY + 3 + wm / 4, xm + wm - wm / 3, m_MinPosY + 3 + wm / 4, m_ColTitleText);
				Gr->DrawLine(xm + wm - wm / 3, m_MinPosY + 3 + wm / 4, xm + wm - wm / 3, m_MinPosY + 3 + wm / 2, m_ColTitleText);
				Gr->DrawLine(xm + wm - wm / 3, m_MinPosY + 3 + wm / 2, xm + wm / 2, m_MinPosY + 3 + wm / 2, m_ColTitleText);
				Gr->DrawLine(xm + wm / 2, m_MinPosY + 3 + wm / 2, xm + wm / 2, m_MinPosY + 3 + wm - wm / 4, m_ColTitleText);
				Gr->DrawLine(xm + wm / 2, m_MinPosY + 3 + wm - wm / 4 + 1, xm + wm / 2, m_MinPosY + 3 + wm - wm / 4 + 2, m_ColTitleText);
			}
		}
	}
}

//  ---------------------------------------------------------------------------

bool CTwBar::MouseMotion(int _X, int _Y) {
	DEV_ASSERT(g_TwMgr->m_Graph && g_TwMgr->m_WndHeight > 0 && g_TwMgr->m_WndWidth > 0);
	if (!m_UpToDate)
		Update();

	bool Handled = false;
	bool CustomArea = false;
	if (!m_IsMinimized) {
		bool InBar = (_X >= m_PosX && _X < m_PosX + m_Width && _Y >= m_PosY && _Y < m_PosY + m_Height);
		for (size_t ib = 0; ib < g_TwMgr->m_Bars.size(); ++ib)
			if (g_TwMgr->m_Bars[ib] != NULL) {
				g_TwMgr->m_Bars[ib]->m_DrawHandles = false;
				g_TwMgr->m_Bars[ib]->m_HighlightTitle = false;
			}
		m_DrawHandles = InBar;
		const int ContainedMargin = 32;

		if (!m_MouseDrag) {
			Handled = InBar;
			m_HighlightedLine = -1;
			m_HighlightIncrBtn = false;
			m_HighlightDecrBtn = false;
			m_HighlightRotoBtn = false;
			if (Math::abs(m_MouseOriginX - _X) > 6 || abs(m_MouseOriginY - _Y) > 6)
				m_HighlightClickBtn = false;
			m_HighlightListBtn = false;
			m_HighlightTitle = false;
			m_HighlightScroll = false;
			m_HighlightUpScroll = false;
			m_HighlightDnScroll = false;
			m_HighlightMinimize = false;
			m_HighlightFont = false;
			m_HighlightValWidth = false;
			m_HighlightLabelsHeader = false;
			m_HighlightValuesHeader = false;
			// if (InBar && _X>m_PosX+m_Font->m_CharHeight+1 && _X<m_PosX+m_VarX2 && _Y>=m_PosY+m_VarY0 && _Y<m_PosY+m_VarY1)
			if (InBar && _X > m_PosX + 2 && _X < m_PosX + m_VarX2 && _Y >= m_PosY + m_VarY0 && _Y < m_PosY + m_VarY1) { // mouse over var line
				m_HighlightedLine = (_Y - m_PosY - m_VarY0) / (m_Font->m_CharHeight + m_LineSep);
				if (m_HighlightedLine >= (int)m_HierTags.size())
					m_HighlightedLine = -1;
				else if (m_HighlightedLine >= 0)
					m_HighlightedLineLastValid = m_HighlightedLine;
				if (m_HighlightedLine < 0 || m_HierTags[m_HighlightedLine].m_Var == NULL || m_HierTags[m_HighlightedLine].m_Var->IsGroup())
					ANT_SET_CURSOR(Arrow);
				else {
					if (!m_HierTags[m_HighlightedLine].m_Var->IsGroup() && static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_NoSlider) {
						if (static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly && !m_IsHelpBar && !(static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Type == TW_TYPE_BUTTON && static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_Val.m_Button.m_Callback == NULL))
							ANT_SET_CURSOR(No); // (Arrow);
						else {
							ANT_SET_CURSOR(Arrow);
							CustomArea = true;
						}

						if (m_DrawListBtn) {
							m_HighlightListBtn = true;
							CustomArea = false;
						}
						if (m_DrawBoolBtn) {
							m_HighlightBoolBtn = true;
							CustomArea = false;
						}
					} else if (m_DrawRotoBtn && (_X >= m_PosX + m_VarX2 - IncrBtnWidth(m_Font->m_CharHeight) || _X < m_PosX + m_VarX1)) { // [o] button // else if( m_DrawRotoBtn && _X<m_PosX+m_VarX1 )
						m_HighlightRotoBtn = true;
						ANT_SET_CURSOR(Point);
					} else if (m_DrawIncrDecrBtn && _X >= m_PosX + m_VarX2 - 2 * IncrBtnWidth(m_Font->m_CharHeight)) { // [+] button
						m_HighlightIncrBtn = true;
						ANT_SET_CURSOR(Arrow);
					} else if (m_DrawIncrDecrBtn && _X >= m_PosX + m_VarX2 - 3 * IncrBtnWidth(m_Font->m_CharHeight)) { // [-] button
						m_HighlightDecrBtn = true;
						ANT_SET_CURSOR(Arrow);
					} else if (!m_HierTags[m_HighlightedLine].m_Var->IsGroup() && static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly) {
						if (!m_IsHelpBar)
							ANT_SET_CURSOR(No);
						else
							ANT_SET_CURSOR(Arrow);
					} else
						ANT_SET_CURSOR(IBeam);
				}
			} else if (InBar && m_Movable && !m_IsPopupList && _X >= m_PosX + 2 * m_Font->m_CharHeight && _X < m_PosX + m_Width - 2 * m_Font->m_CharHeight && _Y < m_PosY + m_Font->m_CharHeight) { // mouse over title
				m_HighlightTitle = true;
				ANT_SET_CURSOR(Move);
			} else if (InBar && !m_IsPopupList && _X >= m_PosX + m_VarX1 - 5 && _X < m_PosX + m_VarX1 + 5 && _Y > m_PosY + m_Font->m_CharHeight && _Y < m_PosY + m_VarY0) { // mouse over ValuesWidth handle
				m_HighlightValWidth = true;
				ANT_SET_CURSOR(WE);
			} else if (InBar && !m_IsPopupList && !m_IsHelpBar && _X >= m_PosX + m_VarX0 && _X < m_PosX + m_VarX1 - 5 && _Y > m_PosY + m_Font->m_CharHeight && _Y < m_PosY + m_VarY0) { // mouse over left column header
				m_HighlightLabelsHeader = true;
				ANT_SET_CURSOR(Arrow);
			} else if (InBar && !m_IsPopupList && _X >= m_PosX + m_VarX1 + 5 && _X < m_PosX + m_VarX2 && _Y > m_PosY + m_Font->m_CharHeight && _Y < m_PosY + m_VarY0) { // mouse over right column header
				m_HighlightValuesHeader = true;
				ANT_SET_CURSOR(Arrow);
			}
			//else if( InBar && m_NbDisplayedLines<m_NbHierLines && _X>=m_PosX && _X<m_PosX+m_Font->m_CharHeight && _Y>=m_ScrollY0 && _Y<m_ScrollY1 )
			else if (InBar && m_NbDisplayedLines < m_NbHierLines && _X >= m_PosX + m_VarX2 + 2 && _X < m_PosX + m_Width - 2 && _Y >= m_ScrollY0 && _Y < m_ScrollY1) {
				m_HighlightScroll = true;
#ifdef _WINDOWS
				ANT_SET_CURSOR(NS);
#else
				ANT_SET_CURSOR(Arrow);
#endif
			} else if (InBar && _X >= m_PosX + m_VarX2 + 2 && _X < m_PosX + m_Width - 2 && _Y >= m_PosY + m_VarY0 && _Y < m_ScrollY0) {
				m_HighlightUpScroll = true;
				ANT_SET_CURSOR(Arrow);
			} else if (InBar && _X >= m_PosX + m_VarX2 + 2 && _X < m_PosX + m_Width - 2 && _Y >= m_ScrollY1 && _Y < m_PosY + m_VarY1) {
				m_HighlightDnScroll = true;
				ANT_SET_CURSOR(Arrow);
			} else if (InBar && m_Resizable && !m_IsPopupList && _X >= m_PosX && _X < m_PosX + m_Font->m_CharHeight && _Y >= m_PosY && _Y < m_PosY + m_Font->m_CharHeight)
				ANT_SET_CURSOR(TopLeft);
			else if (InBar && !m_IsPopupList && _X >= m_PosX && _X < m_PosX + m_Font->m_CharHeight && _Y >= m_PosY + m_Height - m_Font->m_CharHeight && _Y < m_PosY + m_Height)
				ANT_SET_CURSOR(BottomLeft);
			else if (InBar && m_Resizable && !m_IsPopupList && _X >= m_PosX + m_Width - m_Font->m_CharHeight && _X < m_PosX + m_Width && _Y >= m_PosY && _Y < m_PosY + m_Font->m_CharHeight)
				ANT_SET_CURSOR(TopRight);
			else if (InBar && m_Resizable && !m_IsPopupList && _X >= m_PosX + m_Width - m_Font->m_CharHeight && _X < m_PosX + m_Width && _Y >= m_PosY + m_Height - m_Font->m_CharHeight && _Y < m_PosY + m_Height)
				ANT_SET_CURSOR(BottomRight);
			else if (InBar && g_TwMgr->m_FontResizable && !m_IsPopupList && _X >= m_PosX + m_Font->m_CharHeight && _X < m_PosX + 2 * m_Font->m_CharHeight && _Y < m_PosY + m_Font->m_CharHeight) {
				m_HighlightFont = true;
				ANT_SET_CURSOR(Arrow);
			} else if (InBar && m_Iconifiable && !m_IsPopupList && _X >= m_PosX + m_Width - 2 * m_Font->m_CharHeight && _X < m_PosX + m_Width - m_Font->m_CharHeight && _Y < m_PosY + m_Font->m_CharHeight) {
				m_HighlightMinimize = true;
				ANT_SET_CURSOR(Arrow);
			} else if (m_IsHelpBar && InBar && _X >= m_PosX + m_VarX0 && _X < m_PosX + m_Width - m_Font->m_CharHeight && _Y > m_PosY + m_Height - m_Font->m_CharHeight && _Y < m_PosY + m_Height)
				ANT_SET_CURSOR(Arrow); //(Hand);   // web link
			else // if( InBar )
				ANT_SET_CURSOR(Arrow);
		} else {
			if (m_MouseDragVar && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var && !m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
				// move rotoslider
				if (!static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_NoSlider)
					RotoOnMouseMove(_X, _Y);

				if (static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly)
					ANT_SET_CURSOR(No);
				else if (static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_NoSlider) {
					ANT_SET_CURSOR(Arrow);
					CustomArea = true;
				}
				m_VarHasBeenIncr = true;
				Handled = true;
				m_DrawHandles = true;
			} else if (m_MouseDragTitle) {
				int y = m_PosY;
				m_PosX += _X - m_MouseOriginX;
				m_PosY += _Y - m_MouseOriginY;
				m_MouseOriginX = _X;
				m_MouseOriginY = _Y;
				int vpx, vpy, vpw, vph;
				vpx = 0;
				vpy = 0;
				vpw = g_TwMgr->m_WndWidth;
				vph = g_TwMgr->m_WndHeight;
				if (m_Contained) {
					if (m_PosX + m_Width > vpx + vpw)
						m_PosX = vpx + vpw - m_Width;
					if (m_PosX < vpx)
						m_PosX = vpx;
					if (m_PosY + m_Height > vpy + vph)
						m_PosY = vpy + vph - m_Height;
					if (m_PosY < vpy)
						m_PosY = vpy;
				} else {
					if (m_PosX + ContainedMargin > vpx + vpw)
						m_PosX = vpx + vpw - ContainedMargin;
					if (m_PosX + m_Width < vpx + ContainedMargin)
						m_PosX = vpx + ContainedMargin - m_Width;
					if (m_PosY + ContainedMargin > vpy + vph)
						m_PosY = vpy + vph - ContainedMargin;
					if (m_PosY + m_Height < vpy + ContainedMargin)
						m_PosY = vpy + ContainedMargin - m_Height;
				}
				m_ScrollY0 += m_PosY - y;
				m_ScrollY1 += m_PosY - y;
				ANT_SET_CURSOR(Move);
				Handled = true;
			} else if (m_MouseDragValWidth) {
				m_ValuesWidth += m_MouseOriginX - _X;
				m_MouseOriginX = _X;
				NotUpToDate();
				if (m_IsHelpBar)
					g_TwMgr->m_HelpBarNotUpToDate = true;
				ANT_SET_CURSOR(WE);
				Handled = true;
				m_DrawHandles = true;
			} else if (m_MouseDragScroll) {
				if (m_ScrollYH > 0) {
					int dl = ((_Y - m_MouseOriginY) * m_NbHierLines) / m_ScrollYH;
					if (m_FirstLine0 + dl < 0)
						m_FirstLine = 0;
					else if (m_FirstLine0 + dl + m_NbDisplayedLines > m_NbHierLines)
						m_FirstLine = m_NbHierLines - m_NbDisplayedLines;
					else
						m_FirstLine = m_FirstLine0 + dl;
					NotUpToDate();
				}
#ifdef _WINDOWS
				ANT_SET_CURSOR(NS);
#else
				ANT_SET_CURSOR(Arrow);
#endif
				Handled = true;
				m_DrawHandles = true;
			} else if (m_MouseDragResizeUL) {
				int w = m_Width;
				int h = m_Height;
				m_PosX += _X - m_MouseOriginX;
				m_PosY += _Y - m_MouseOriginY;
				m_Width -= _X - m_MouseOriginX;
				m_Height -= _Y - m_MouseOriginY;
				m_MouseOriginX = _X;
				m_MouseOriginY = _Y;
				int vpx = 0, vpy = 0, vpw = g_TwMgr->m_WndWidth, vph = g_TwMgr->m_WndHeight;
				if (!m_Contained) {
					if (m_PosX + ContainedMargin > vpx + vpw)
						m_PosX = vpx + vpw - ContainedMargin;
					if (m_PosX + m_Width < vpx + ContainedMargin)
						m_PosX = vpx + ContainedMargin - m_Width;
					if (m_PosY + ContainedMargin > vpy + vph)
						m_PosY = vpy + vph - ContainedMargin;
					if (m_PosY + m_Height < vpy + ContainedMargin)
						m_PosY = vpy + ContainedMargin - m_Height;
				} else {
					if (m_PosX < vpx) {
						m_PosX = vpx;
						m_Width = w;
					}
					if (m_PosY < vpy) {
						m_PosY = vpy;
						m_Height = h;
					}
				}
				if (m_ValuesWidthRatio > 0)
					m_ValuesWidth = int(m_ValuesWidthRatio * m_Width + 0.5);
				ANT_SET_CURSOR(TopLeft);
				NotUpToDate();
				if (m_IsHelpBar) {
					g_TwMgr->m_HelpBarNotUpToDate = true;
					g_TwMgr->m_HelpBarUpdateNow = true;
				}
				g_TwMgr->m_KeyPressedBuildText = true;
				g_TwMgr->m_InfoBuildText = true;
				Handled = true;
				m_DrawHandles = true;
			} else if (m_MouseDragResizeUR) {
				int h = m_Height;
				m_PosY += _Y - m_MouseOriginY;
				m_Width += _X - m_MouseOriginX;
				m_Height -= _Y - m_MouseOriginY;
				m_MouseOriginX = _X;
				m_MouseOriginY = _Y;
				int vpx = 0, vpy = 0, vpw = g_TwMgr->m_WndWidth, vph = g_TwMgr->m_WndHeight;
				if (!m_Contained) {
					if (m_PosX + ContainedMargin > vpx + vpw)
						m_PosX = vpx + vpw - ContainedMargin;
					if (m_PosX + m_Width < vpx + ContainedMargin)
						m_PosX = vpx + ContainedMargin - m_Width;
					if (m_PosY + ContainedMargin > vpy + vph)
						m_PosY = vpy + vph - ContainedMargin;
					if (m_PosY + m_Height < vpy + ContainedMargin)
						m_PosY = vpy + ContainedMargin - m_Height;
				} else {
					if (m_PosX + m_Width > vpx + vpw)
						m_Width = vpx + vpw - m_PosX;
					if (m_PosY < vpy) {
						m_PosY = vpy;
						m_Height = h;
					}
				}
				if (m_ValuesWidthRatio > 0)
					m_ValuesWidth = int(m_ValuesWidthRatio * m_Width + 0.5);
				ANT_SET_CURSOR(TopRight);
				NotUpToDate();
				if (m_IsHelpBar) {
					g_TwMgr->m_HelpBarNotUpToDate = true;
					g_TwMgr->m_HelpBarUpdateNow = true;
				}
				g_TwMgr->m_KeyPressedBuildText = true;
				g_TwMgr->m_InfoBuildText = true;
				Handled = true;
				m_DrawHandles = true;
			} else if (m_MouseDragResizeLL) {
				int w = m_Width;
				m_PosX += _X - m_MouseOriginX;
				m_Width -= _X - m_MouseOriginX;
				m_Height += _Y - m_MouseOriginY;
				m_MouseOriginX = _X;
				m_MouseOriginY = _Y;
				int vpx = 0, vpy = 0, vpw = g_TwMgr->m_WndWidth, vph = g_TwMgr->m_WndHeight;
				if (!m_Contained) {
					if (m_PosX + ContainedMargin > vpx + vpw)
						m_PosX = vpx + vpw - ContainedMargin;
					if (m_PosX + m_Width < vpx + ContainedMargin)
						m_PosX = vpx + ContainedMargin - m_Width;
					if (m_PosY + ContainedMargin > vpy + vph)
						m_PosY = vpy + vph - ContainedMargin;
					if (m_PosY + m_Height < vpy + ContainedMargin)
						m_PosY = vpy + ContainedMargin - m_Height;
				} else {
					if (m_PosY + m_Height > vpy + vph)
						m_Height = vpy + vph - m_PosY;
					if (m_PosX < vpx) {
						m_PosX = vpx;
						m_Width = w;
					}
				}
				if (m_ValuesWidthRatio > 0)
					m_ValuesWidth = int(m_ValuesWidthRatio * m_Width + 0.5);
				ANT_SET_CURSOR(BottomLeft);
				NotUpToDate();
				if (m_IsHelpBar) {
					g_TwMgr->m_HelpBarNotUpToDate = true;
					g_TwMgr->m_HelpBarUpdateNow = true;
				}
				g_TwMgr->m_KeyPressedBuildText = true;
				g_TwMgr->m_InfoBuildText = true;
				Handled = true;
				m_DrawHandles = true;
			} else if (m_MouseDragResizeLR) {
				m_Width += _X - m_MouseOriginX;
				m_Height += _Y - m_MouseOriginY;
				m_MouseOriginX = _X;
				m_MouseOriginY = _Y;
				int vpx = 0, vpy = 0, vpw = g_TwMgr->m_WndWidth, vph = g_TwMgr->m_WndHeight;
				if (!m_Contained) {
					if (m_PosX + ContainedMargin > vpx + vpw)
						m_PosX = vpx + vpw - ContainedMargin;
					if (m_PosX + m_Width < vpx + ContainedMargin)
						m_PosX = vpx + ContainedMargin - m_Width;
					if (m_PosY + ContainedMargin > vpy + vph)
						m_PosY = vpy + vph - ContainedMargin;
					if (m_PosY + m_Height < vpy + ContainedMargin)
						m_PosY = vpy + ContainedMargin - m_Height;
				} else {
					if (m_PosX + m_Width > vpx + vpw)
						m_Width = vpx + vpw - m_PosX;
					if (m_PosY + m_Height > vpy + vph)
						m_Height = vpy + vph - m_PosY;
				}
				if (m_ValuesWidthRatio > 0)
					m_ValuesWidth = int(m_ValuesWidthRatio * m_Width + 0.5);
				ANT_SET_CURSOR(BottomRight);
				NotUpToDate();
				if (m_IsHelpBar) {
					g_TwMgr->m_HelpBarNotUpToDate = true;
					g_TwMgr->m_HelpBarUpdateNow = true;
				}
				g_TwMgr->m_KeyPressedBuildText = true;
				g_TwMgr->m_InfoBuildText = true;
				Handled = true;
				m_DrawHandles = true;
			} else if (m_EditInPlace.m_Active) {
				EditInPlaceMouseMove(_X, _Y, true);
				ANT_SET_CURSOR(IBeam);
				Handled = true;
			}
			// else if (InBar)
			//   ANT_SET_CURSOR(Arrow);
		}
	} else { // minimized
		if (m_Iconifiable && _X >= m_MinPosX + 2 && _X < m_MinPosX + m_Font->m_CharHeight && _Y > m_MinPosY && _Y < m_MinPosY + m_Font->m_CharHeight - 2) {
			m_HighlightMaximize = true;
			if (!m_IsHelpBar)
				ANT_SET_CURSOR(Arrow);
			else
#ifdef _WINDOWS
				ANT_SET_CURSOR(Help);
#else
				ANT_SET_CURSOR(Arrow);
#endif
			Handled = true;
		} else
			m_HighlightMaximize = false;
	}

	// Handled by a custom widget?
	CTwMgr::CStructProxy *currentCustomActiveStructProxy = nullptr;
	if (g_TwMgr != nullptr && (!Handled || CustomArea) && !m_IsMinimized && m_CustomRecords.size() > 0) {
		bool CustomHandled = false;
		for (int s = 0; s < 2; ++s) // 2 iterations: first for custom widget having focus, second for others if no focused widget.
			for (CustomMap::iterator it = m_CustomRecords.begin(); it != m_CustomRecords.end(); ++it) {
				CTwMgr::CStructProxy *sProxy = it->first;
				const CCustomRecord &r = it->second;
				if ((s == 1 || sProxy->m_CustomCaptureFocus) && !CustomHandled && sProxy != nullptr && sProxy->m_CustomMouseMotionCallback != nullptr && r.m_XMin < r.m_XMax && r.m_Y0 < r.m_Y1 && r.m_YMin <= r.m_YMax && r.m_YMin >= r.m_Y0 && r.m_YMax <= r.m_Y1) {
					if (sProxy->m_CustomCaptureFocus || (_X >= r.m_XMin && _X < r.m_XMax && _Y >= r.m_YMin && _Y < r.m_YMax)) {
						CustomHandled = sProxy->m_CustomMouseMotionCallback(_X - r.m_XMin, _Y - r.m_Y0, r.m_XMax - r.m_XMin, r.m_Y1 - r.m_Y0, sProxy->m_StructExtData, sProxy->m_StructClientData, this, r.m_Var);
						currentCustomActiveStructProxy = sProxy;
						s = 2; // force s-loop exit
					}
				} else if (sProxy != nullptr) {
					sProxy->m_CustomCaptureFocus = false; // force free focus, just in case.
					ANT_SET_CURSOR(Arrow);
				}
			}
		if (CustomHandled)
			Handled = true;
	}
	// If needed, send a 'MouseLeave' message to previously active custom struct
	if (g_TwMgr != nullptr && m_CustomActiveStructProxy != nullptr && m_CustomActiveStructProxy != currentCustomActiveStructProxy) {
		bool found = false;
		for (std::list<CTwMgr::CStructProxy>::iterator it = g_TwMgr->m_StructProxies.begin(); it != g_TwMgr->m_StructProxies.end() && !found; ++it)
			found = (&(*it) == m_CustomActiveStructProxy);
		if (found && m_CustomActiveStructProxy->m_CustomMouseLeaveCallback != nullptr)
			m_CustomActiveStructProxy->m_CustomMouseLeaveCallback(m_CustomActiveStructProxy->m_StructExtData, m_CustomActiveStructProxy->m_StructClientData, this);
	}
	m_CustomActiveStructProxy = currentCustomActiveStructProxy;

	return Handled;
}

//  ---------------------------------------------------------------------------

#ifdef _WINDOWS
#pragma optimize("", off)
//  disable optimizations because the conversion of Enum from unsigned int to double is not always exact if optimized and GraphAPI=DirectX !
#endif
static void ANT_CALL PopupCallback(void *_ClientData) {
	CTwFPU fpu; // force fpu precision

	if (g_TwMgr != nullptr && g_TwMgr->m_PopupBar != nullptr) {
		unsigned int Enum = *(unsigned int *)&_ClientData;
		CTwVarAtom *Var = g_TwMgr->m_PopupBar->m_VarEnumLinkedToPopupList;
		CTwBar *Bar = g_TwMgr->m_PopupBar->m_BarLinkedToPopupList;
		if (Bar != nullptr && Var != nullptr && !Var->m_ReadOnly && IsEnumType(Var->m_Type)) {
			Var->ValueFromDouble(Enum);
			// Bar->UnHighlightLine();
			Bar->HaveFocus(true);
			Bar->NotUpToDate();
		}
		if (g_TwMgr->m_PopupBar != nullptr) // check again because it might have been destroyed by an enum callback
			TwDeleteBar(g_TwMgr->m_PopupBar);
		g_TwMgr->m_PopupBar = nullptr;
	}
}
#ifdef _WINDOWS
#pragma optimize("", on)
#endif

//  ---------------------------------------------------------------------------

bool CTwBar::MouseButton(ETwMouseButtonID _Button, bool _Pressed, int _X, int _Y) {
	assert(g_TwMgr->m_Graph && g_TwMgr->m_WndHeight > 0 && g_TwMgr->m_WndWidth > 0);
	bool Handled = false;
	if (!m_UpToDate)
		Update();
	bool EditInPlaceActive = false;
	bool CustomArea = false;

	if (!m_IsMinimized) {
		Handled = (_X >= m_PosX && _X < m_PosX + m_Width && _Y >= m_PosY && _Y < m_PosY + m_Height);
		if (_Button == TW_MOUSE_LEFT && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var) {
			bool OnFocus = (m_HighlightedLine == (_Y - m_PosY - m_VarY0) / (m_Font->m_CharHeight + m_LineSep) && Handled);
			if (m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
				if (_Pressed && !g_TwMgr->m_IsRepeatingMousePressed && OnFocus) {
					CTwVarGroup *Grp = static_cast<CTwVarGroup *>(m_HierTags[m_HighlightedLine].m_Var);
					Grp->m_Open = !Grp->m_Open;
					NotUpToDate();
					ANT_SET_CURSOR(Arrow);
				}
			} else if (_Pressed && m_HighlightIncrBtn) {
				static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->Increment(1);
				if (g_TwMgr == NULL) // Mgr might have been destroyed by the client inside a callback call
					return 1;
				NotUpToDate();
			} else if (_Pressed && m_HighlightDecrBtn) {
				static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->Increment(-1);
				if (g_TwMgr == NULL) // Mgr might have been destroyed by the client inside a callback call
					return 1;
				NotUpToDate();
			} else if (_Pressed && !m_MouseDrag) {
				m_MouseDrag = true;
				m_MouseDragVar = true;
				m_MouseOriginX = _X;
				m_MouseOriginY = _Y;
				m_VarHasBeenIncr = false;
				CTwVarAtom *Var = static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
				if (!Var->m_NoSlider && !Var->m_ReadOnly && m_HighlightRotoBtn) {
					// begin rotoslider
					if (_X > m_PosX + m_VarX1 && OnFocus)
						RotoOnLButtonDown(m_PosX + m_VarX2 - (1 * IncrBtnWidth(m_Font->m_CharHeight)) / 2, _Y);
					else
						RotoOnLButtonDown(_X, _Y);
					m_MouseDrag = true;
					m_MouseDragVar = true;
				} else if ((Var->m_Type == TW_TYPE_BOOL8 || Var->m_Type == TW_TYPE_BOOL16 || Var->m_Type == TW_TYPE_BOOL32 || Var->m_Type == TW_TYPE_BOOLCPP) && !Var->m_ReadOnly && OnFocus) {
					Var->Increment(1);
					//m_HighlightClickBtn = true;
					m_VarHasBeenIncr = true;
					m_MouseDragVar = false;
					m_MouseDrag = false;
					NotUpToDate();
				} else if (Var->m_Type == TW_TYPE_BUTTON && !Var->m_ReadOnly) {
					m_HighlightClickBtn = true;
					m_MouseDragVar = false;
					m_MouseDrag = false;
				} else if (IsEnumType(Var->m_Type) && !Var->m_ReadOnly && !g_TwMgr->m_IsRepeatingMousePressed && OnFocus) {
					m_MouseDragVar = false;
					m_MouseDrag = false;
					if (g_TwMgr->m_PopupBar != NULL) {
						TwDeleteBar(g_TwMgr->m_PopupBar);
						g_TwMgr->m_PopupBar = NULL;
					}
					// popup list
					CTwMgr::CEnum &e = g_TwMgr->m_Enums[Var->m_Type - TW_TYPE_ENUM_BASE];
					g_TwMgr->m_PopupBar = TwNewBar("~ Enum Popup ~");
					g_TwMgr->m_PopupBar->m_IsPopupList = true;
					g_TwMgr->m_PopupBar->m_Color = m_Color;
					g_TwMgr->m_PopupBar->m_DarkText = m_DarkText;
					g_TwMgr->m_PopupBar->m_PosX = m_PosX + m_VarX1 - 2;
					g_TwMgr->m_PopupBar->m_PosY = m_PosY + m_VarY0 + (m_HighlightedLine + 1) * (m_Font->m_CharHeight + m_LineSep);
					g_TwMgr->m_PopupBar->m_Width = m_Width - 2 * m_Font->m_CharHeight;
					g_TwMgr->m_PopupBar->m_LineSep = g_TwMgr->m_PopupBar->m_Sep;
					int popHeight0 = (int)e.m_Entries.size() * (m_Font->m_CharHeight + m_Sep) + m_Font->m_CharHeight / 2 + 2;
					int popHeight = popHeight0;
					if (g_TwMgr->m_PopupBar->m_PosY + popHeight + 2 > g_TwMgr->m_WndHeight)
						popHeight = g_TwMgr->m_WndHeight - g_TwMgr->m_PopupBar->m_PosY - 2;
					if (popHeight < popHeight0 / 2 && popHeight < g_TwMgr->m_WndHeight / 2)
						popHeight = MIN(popHeight0, g_TwMgr->m_WndHeight / 2);
					if (popHeight < 3 * (m_Font->m_CharHeight + m_Sep))
						popHeight = 3 * (m_Font->m_CharHeight + m_Sep);
					g_TwMgr->m_PopupBar->m_Height = popHeight;
					g_TwMgr->m_PopupBar->m_VarEnumLinkedToPopupList = Var;
					g_TwMgr->m_PopupBar->m_BarLinkedToPopupList = this;
					unsigned int CurrentEnumValue = (unsigned int)((int)Var->ValueToDouble());
					for (CTwMgr::CEnum::CEntries::iterator It = e.m_Entries.begin(); It != e.m_Entries.end(); ++It) {
						char ID[64];
						snprintf(ID, 64, "%u", It->first);
						// ultoa(It->first, ID, 10);
						TwAddButton(g_TwMgr->m_PopupBar, ID, PopupCallback, *(void **)&(It->first), NULL);
						CTwVar *Btn = g_TwMgr->m_PopupBar->Find(ID);
						if (Btn != NULL) {
							Btn->m_Label = It->second.c_str();
							if (It->first == CurrentEnumValue) {
								Btn->m_ColorPtr = &m_ColValTextNE;
								Btn->m_BgColorPtr = &m_ColGrpBg;
							}
						}
					}
					g_TwMgr->m_HelpBarNotUpToDate = false;
				} else if ((Var->m_ReadOnly && (Var->m_Type == TW_TYPE_CDSTRING || Var->m_Type == TW_TYPE_CDSTDSTRING || Var->m_Type == TW_TYPE_STDSTRING || IsCSStringType(Var->m_Type)) && EditInPlaceAcceptVar(Var)) || (!Var->m_ReadOnly && EditInPlaceAcceptVar(Var))) {
					int dw = 0;
					// if( m_DrawIncrDecrBtn )
					//   dw = 2*IncrBtnWidth(m_Font->m_CharHeight);
					if (!m_EditInPlace.m_Active || m_EditInPlace.m_Var != Var) {
						EditInPlaceStart(Var, m_VarX1, m_VarY0 + (m_HighlightedLine) * (m_Font->m_CharHeight + m_LineSep), m_VarX2 - m_VarX1 - dw - 1);
						if (EditInPlaceIsReadOnly())
							EditInPlaceMouseMove(_X, _Y, false);
						m_MouseDrag = false;
						m_MouseDragVar = false;
					} else {
						EditInPlaceMouseMove(_X, _Y, false);
						m_MouseDrag = true;
						m_MouseDragVar = false;
					}
					EditInPlaceActive = m_EditInPlace.m_Active;
					if (Var->m_ReadOnly)
						ANT_SET_CURSOR(No);
					else
						ANT_SET_CURSOR(IBeam);
				} else if (Var->m_ReadOnly)
					ANT_SET_CURSOR(No);
				else {
					ANT_SET_CURSOR(Arrow);
					CustomArea = true;
				}
			} else if (!_Pressed && m_MouseDragVar) {
				m_MouseDrag = false;
				m_MouseDragVar = false;
				if (!Handled)
					m_DrawHandles = false;
				Handled = true;
				// end rotoslider
				RotoOnLButtonUp(_X, _Y);

				// Incr/decr on right or left click
				// if (!m_VarHasBeenIncr && !static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly)
				// {
				//   if( _Button==TW_MOUSE_LEFT )
				//     static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->Increment(-1);
				//   else if( _Button==TW_MOUSE_RIGHT )
				//     static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->Increment(1);
				//   NotUpToDate();
				// }

				if (static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var)->m_ReadOnly)
					ANT_SET_CURSOR(No);
				else {
					ANT_SET_CURSOR(Arrow);
					CustomArea = true;
				}
			} else if (!_Pressed && m_HighlightClickBtn) { // a button variable is activated
				m_HighlightClickBtn = false;
				m_MouseDragVar = false;
				m_MouseDrag = false;
				Handled = true;
				NotUpToDate();
				if (!m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
					CTwVarAtom *Var = static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
					if (!Var->m_ReadOnly && Var->m_Type == TW_TYPE_BUTTON && Var->m_Val.m_Button.m_Callback != nullptr) {
						Var->m_Val.m_Button.m_Callback(Var->m_ClientData);
						if (g_TwMgr == nullptr) // Mgr might have been destroyed by the client inside a callback call
							return 1;
					}
				}
			} else if (!_Pressed) {
				m_MouseDragVar = false;
				m_MouseDrag = false;
				CustomArea = true;
			}
		} else if (_Pressed && !m_MouseDrag && m_Movable && !m_IsPopupList && ((_Button == TW_MOUSE_LEFT && _X >= m_PosX + 2 * m_Font->m_CharHeight && _X < m_PosX + m_Width - 2 * m_Font->m_CharHeight && _Y >= m_PosY && _Y < m_PosY + m_Font->m_CharHeight) || (_Button == TW_MOUSE_MIDDLE && _X >= m_PosX && _X < m_PosX + m_Width && _Y >= m_PosY && _Y < m_PosY + m_Height))) {
			m_MouseDrag = true;
			m_MouseDragTitle = true;
			m_MouseOriginX = _X;
			m_MouseOriginY = _Y;
			m_HighlightTitle = true;
			ANT_SET_CURSOR(Move);
		} else if (!_Pressed && m_MouseDragTitle) {
			m_MouseDrag = false;
			m_MouseDragTitle = false;
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && !m_MouseDrag && !m_IsPopupList && _Button == TW_MOUSE_LEFT && _X >= m_PosX + m_VarX1 - 3 && _X < m_PosX + m_VarX1 + 3 && _Y > m_PosY + m_Font->m_CharHeight && _Y < m_PosY + m_VarY0) {
			m_MouseDrag = true;
			m_MouseDragValWidth = true;
			m_MouseOriginX = _X;
			m_MouseOriginY = _Y;
			ANT_SET_CURSOR(WE);
		} else if (!_Pressed && m_MouseDragValWidth) {
			m_MouseDrag = false;
			m_MouseDragValWidth = false;
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && !m_MouseDrag && m_NbDisplayedLines < m_NbHierLines && _Button == TW_MOUSE_LEFT && _X >= m_PosX + m_VarX2 + 2 && _X < m_PosX + m_Width - 2 && _Y >= m_ScrollY0 && _Y < m_ScrollY1) {
			m_MouseDrag = true;
			m_MouseDragScroll = true;
			m_MouseOriginX = _X;
			m_MouseOriginY = _Y;
			m_FirstLine0 = m_FirstLine;
#ifdef _WINDOWS
			ANT_SET_CURSOR(NS);
#else
			ANT_SET_CURSOR(Arrow);
#endif
		} else if (!_Pressed && m_MouseDragScroll) {
			m_MouseDrag = false;
			m_MouseDragScroll = false;
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && _Button == TW_MOUSE_LEFT && _X >= m_PosX + m_VarX2 + 2 && _X < m_PosX + m_Width - 2 && _Y >= m_PosY + m_VarY0 && _Y < m_ScrollY0) {
			if (m_FirstLine > 0) {
				--m_FirstLine;
				NotUpToDate();
			}
		} else if (_Pressed && _Button == TW_MOUSE_LEFT && _X >= m_PosX + m_VarX2 + 2 && _X < m_PosX + m_Width - 2 && _Y >= m_ScrollY1 && _Y < m_PosY + m_VarY1) {
			if (m_FirstLine < m_NbHierLines - m_NbDisplayedLines) {
				++m_FirstLine;
				NotUpToDate();
			}
		} else if (_Pressed && !m_MouseDrag && m_Resizable && !m_IsPopupList && _Button == TW_MOUSE_LEFT && _X >= m_PosX && _X < m_PosX + m_Font->m_CharHeight && _Y >= m_PosY && _Y < m_PosY + m_Font->m_CharHeight) {
			m_MouseDrag = true;
			m_MouseDragResizeUL = true;
			m_MouseOriginX = _X;
			m_MouseOriginY = _Y;
			m_ValuesWidthRatio = (m_Width > 0) ? (double)m_ValuesWidth / m_Width : 0;
			ANT_SET_CURSOR(TopLeft);
		} else if (!_Pressed && m_MouseDragResizeUL) {
			m_MouseDrag = false;
			m_MouseDragResizeUL = false;
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && !m_MouseDrag && m_Resizable && !m_IsPopupList && _Button == TW_MOUSE_LEFT && _X >= m_PosX + m_Width - m_Font->m_CharHeight && _X < m_PosX + m_Width && _Y >= m_PosY && _Y < m_PosY + m_Font->m_CharHeight) {
			m_MouseDrag = true;
			m_MouseDragResizeUR = true;
			m_MouseOriginX = _X;
			m_MouseOriginY = _Y;
			m_ValuesWidthRatio = (m_Width > 0) ? (double)m_ValuesWidth / m_Width : 0;
			ANT_SET_CURSOR(TopRight);
		} else if (!_Pressed && m_MouseDragResizeUR) {
			m_MouseDrag = false;
			m_MouseDragResizeUR = false;
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && !m_MouseDrag && m_Resizable && !m_IsPopupList && _Button == TW_MOUSE_LEFT && _X >= m_PosX && _X < m_PosX + m_Font->m_CharHeight && _Y >= m_PosY + m_Height - m_Font->m_CharHeight && _Y < m_PosY + m_Height) {
			m_MouseDrag = true;
			m_MouseDragResizeLL = true;
			m_MouseOriginX = _X;
			m_MouseOriginY = _Y;
			m_ValuesWidthRatio = (m_Width > 0) ? (double)m_ValuesWidth / m_Width : 0;
			ANT_SET_CURSOR(BottomLeft);
		} else if (!_Pressed && m_MouseDragResizeLL) {
			m_MouseDrag = false;
			m_MouseDragResizeLL = false;
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && !m_MouseDrag && m_Resizable && !m_IsPopupList && _Button == TW_MOUSE_LEFT && _X >= m_PosX + m_Width - m_Font->m_CharHeight && _X < m_PosX + m_Width && _Y >= m_PosY + m_Height - m_Font->m_CharHeight && _Y < m_PosY + m_Height) {
			m_MouseDrag = true;
			m_MouseDragResizeLR = true;
			m_MouseOriginX = _X;
			m_MouseOriginY = _Y;
			m_ValuesWidthRatio = (m_Width > 0) ? (double)m_ValuesWidth / m_Width : 0;
			ANT_SET_CURSOR(BottomRight);
		} else if (!_Pressed && m_MouseDragResizeLR) {
			m_MouseDrag = false;
			m_MouseDragResizeLR = false;
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && !m_IsPopupList && _Button == TW_MOUSE_LEFT && m_HighlightLabelsHeader) {
			int w = ComputeLabelsWidth(m_Font);
			if (w < m_Font->m_CharHeight)
				w = m_Font->m_CharHeight;
			m_ValuesWidth = m_VarX2 - m_VarX0 - w;
			if (m_ValuesWidth < m_Font->m_CharHeight)
				m_ValuesWidth = m_Font->m_CharHeight;
			if (m_ValuesWidth > m_VarX2 - m_VarX0)
				m_ValuesWidth = MAX(m_VarX2 - m_VarX0 - m_Font->m_CharHeight, 0);
			NotUpToDate();
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && !m_IsPopupList && _Button == TW_MOUSE_LEFT && m_HighlightValuesHeader) {
			int w = ComputeValuesWidth(m_Font);
			if (w < 2 * m_Font->m_CharHeight)
				w = 2 * m_Font->m_CharHeight; // enough to draw a button
			m_ValuesWidth = w;
			if (m_ValuesWidth > m_VarX2 - m_VarX0)
				m_ValuesWidth = MAX(m_VarX2 - m_VarX0 - m_Font->m_CharHeight, 0);
			NotUpToDate();
			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && g_TwMgr->m_FontResizable && !m_IsPopupList && _X >= m_PosX + m_Font->m_CharHeight && _X < m_PosX + 2 * m_Font->m_CharHeight && _Y > m_PosY && _Y < m_PosY + m_Font->m_CharHeight) {
			// change font
			if (_Button == TW_MOUSE_LEFT) {
				if (m_Font == g_DefaultSmallFont)
					g_TwMgr->SetFont(g_DefaultNormalFont, true);
				else if (m_Font == g_DefaultNormalFont)
					g_TwMgr->SetFont(g_DefaultLargeFont, true);
				else if (m_Font == g_DefaultLargeFont)
					g_TwMgr->SetFont(g_DefaultSmallFont, true);
				else
					g_TwMgr->SetFont(g_DefaultNormalFont, true);
			} else if (_Button == TW_MOUSE_RIGHT) {
				if (m_Font == g_DefaultSmallFont)
					g_TwMgr->SetFont(g_DefaultLargeFont, true);
				else if (m_Font == g_DefaultNormalFont)
					g_TwMgr->SetFont(g_DefaultSmallFont, true);
				else if (m_Font == g_DefaultLargeFont)
					g_TwMgr->SetFont(g_DefaultNormalFont, true);
				else
					g_TwMgr->SetFont(g_DefaultNormalFont, true);
			}

			ANT_SET_CURSOR(Arrow);
		} else if (_Pressed && m_Iconifiable && !m_IsPopupList && _Button == TW_MOUSE_LEFT && _X >= m_PosX + m_Width - 2 * m_Font->m_CharHeight && _X < m_PosX + m_Width - m_Font->m_CharHeight && _Y > m_PosY && _Y < m_PosY + m_Font->m_CharHeight) {
			// minimize
			g_TwMgr->Minimize(this);
			ANT_SET_CURSOR(Arrow);
		} else if (m_IsHelpBar && _Pressed && !g_TwMgr->m_IsRepeatingMousePressed && _X >= m_PosX + m_VarX0 && _X < m_PosX + m_Width - m_Font->m_CharHeight && _Y > m_PosY + m_Height - m_Font->m_CharHeight && _Y < m_PosY + m_Height) {
			// TODO Open link?
		} else {
			CustomArea = true;
		}
	} else { // minimized
		if (_Pressed && m_HighlightMaximize) {
			m_HighlightMaximize = false;
			g_TwMgr->Maximize(this);
			ANT_SET_CURSOR(Arrow);
			Handled = true;
		}
	}

	if (g_TwMgr != nullptr) // Mgr might have been destroyed by the client inside a callback call
		if (_Pressed && !EditInPlaceActive && m_EditInPlace.m_Active)
			EditInPlaceEnd(true);

	// Handled by a custom widget?
	if (g_TwMgr != nullptr && (!Handled || CustomArea) && !m_IsMinimized && m_CustomRecords.size() > 0) {
		bool CustomHandled = false;
		for (int s = 0; s < 2; ++s) // 2 iterations: first for custom widget having focus, second for others if no focused widget.
			for (CustomMap::iterator it = m_CustomRecords.begin(); it != m_CustomRecords.end(); ++it) {
				CTwMgr::CStructProxy *sProxy = it->first;
				const CCustomRecord &r = it->second;
				if ((s == 1 || sProxy->m_CustomCaptureFocus) && !CustomHandled && sProxy != nullptr && sProxy->m_CustomMouseButtonCallback != nullptr && r.m_XMin < r.m_XMax && r.m_Y0 < r.m_Y1 && r.m_YMin <= r.m_YMax && r.m_YMin >= r.m_Y0 && r.m_YMax <= r.m_Y1) {
					if (sProxy->m_CustomCaptureFocus || (_X >= r.m_XMin && _X < r.m_XMax && _Y >= r.m_YMin && _Y < r.m_YMax)) {
						sProxy->m_CustomCaptureFocus = _Pressed;
						CustomHandled = sProxy->m_CustomMouseButtonCallback(_Button, _Pressed, _X - r.m_XMin, _Y - r.m_Y0, r.m_XMax - r.m_XMin, r.m_Y1 - r.m_Y0, sProxy->m_StructExtData, sProxy->m_StructClientData, this, r.m_Var);
						s = 2; // force s-loop exit
					}
				} else if (sProxy != nullptr) {
					sProxy->m_CustomCaptureFocus = false; // force free focus, just in case.
					ANT_SET_CURSOR(Arrow);
				}
			}
		if (CustomHandled)
			Handled = true;
	}

	return Handled;
}

//  ---------------------------------------------------------------------------

bool CTwBar::MouseWheel(int _Pos, int _PrevPos, int _MouseX, int _MouseY) {
	DEV_ASSERT(g_TwMgr->m_Graph && g_TwMgr->m_WndHeight > 0 && g_TwMgr->m_WndWidth > 0);
	if (!m_UpToDate)
		Update();

	bool Handled = false;
	if (!m_IsMinimized && _MouseX >= m_PosX && _MouseX < m_PosX + m_Width && _MouseY >= m_PosY && _MouseY < m_PosY + m_Height) {
		if (_Pos > _PrevPos && m_FirstLine > 0) {
			--m_FirstLine;
			NotUpToDate();
		} else if (_Pos < _PrevPos && m_FirstLine < m_NbHierLines - m_NbDisplayedLines) {
			++m_FirstLine;
			NotUpToDate();
		}

		if (_Pos != _PrevPos) {
			Handled = true;
			if (m_EditInPlace.m_Active)
				EditInPlaceEnd(true);
		}
	}

	return Handled;
}

//  ---------------------------------------------------------------------------

CTwVarAtom *CTwVarGroup::FindShortcut(int _Key, int _Modifiers, bool *_DoIncr) {
	CTwVarAtom *Atom;
	int Mask = 0xffffffff;
	if (_Key > ' ' && _Key < 256) // don't test SHIFT if _Key is a common key
		Mask &= ~TW_KMOD_SHIFT;

	// don't test KMOD_NUM and KMOD_CAPS modifiers coming from SDL
	Mask &= ~(0x1000); // 0x1000 is the KMOD_NUM value defined in SDL_keysym.h
	Mask &= ~(0x2000); // 0x2000 is the KMOD_CAPS value defined in SDL_keysym.h

	// complete partial modifiers comming from SDL
	if (_Modifiers & TW_KMOD_SHIFT)
		_Modifiers |= TW_KMOD_SHIFT;
	if (_Modifiers & TW_KMOD_CTRL)
		_Modifiers |= TW_KMOD_CTRL;
	if (_Modifiers & TW_KMOD_ALT)
		_Modifiers |= TW_KMOD_ALT;
	if (_Modifiers & TW_KMOD_META)
		_Modifiers |= TW_KMOD_META;

	for (size_t i = 0; i < m_Vars.size(); ++i)
		if (m_Vars[i] != NULL) {
			if (m_Vars[i]->IsGroup()) {
				Atom = static_cast<CTwVarGroup *>(m_Vars[i])->FindShortcut(_Key, _Modifiers, _DoIncr);
				if (Atom != NULL)
					return Atom;
			} else {
				Atom = static_cast<CTwVarAtom *>(m_Vars[i]);
				if (Atom->m_KeyIncr[0] == _Key && (Atom->m_KeyIncr[1] & Mask) == (_Modifiers & Mask)) {
					if (_DoIncr != NULL)
						*_DoIncr = true;
					return Atom;
				} else if (Atom->m_KeyDecr[0] == _Key && (Atom->m_KeyDecr[1] & Mask) == (_Modifiers & Mask)) {
					if (_DoIncr != NULL)
						*_DoIncr = false;
					return Atom;
				}
			}
		}
	return NULL;
}

bool CTwBar::KeyPressed(int _Key, int _Modifiers) {
	DEV_ASSERT(g_TwMgr->m_Graph && g_TwMgr->m_WndHeight > 0 && g_TwMgr->m_WndWidth > 0);
	bool Handled = false;
	if (!m_UpToDate)
		Update();

	if (_Key > 0 && _Key < TW_KEY_LAST) {
		// std::string Str;
		// TwGetKeyString(&Str, _Key, _Modifiers);
		// printf("key: %d 0x%04xd %s\n", _Key, _Modifiers, Str.c_str());

		if (m_EditInPlace.m_Active) {
			Handled = EditInPlaceKeyPressed(_Key, _Modifiers);
		} else {
			bool BarActive = (m_DrawHandles || m_IsPopupList) && !m_IsMinimized;
			bool DoIncr = true;
			CTwVarAtom *Atom = m_VarRoot.FindShortcut(_Key, _Modifiers, &DoIncr);
			if (Atom != NULL && Atom->m_Visible) {
				if (!Atom->m_ReadOnly) {
					Atom->Increment(DoIncr ? +1 : -1);
					if (g_TwMgr == NULL) // Mgr might have been destroyed by the client inside a callback call
						return 1;
					m_HighlightClickBtnAuto = g_TwMgr->m_Timer.GetTime();
				}
				NotUpToDate();
				Show(Atom);
				Handled = true;
			} else if (BarActive && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var) {
				if (_Key == TW_KEY_RIGHT) {
					if (!m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
						CTwVarAtom *Atom = static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
						bool Accept = !Atom->m_NoSlider || Atom->m_Type == TW_TYPE_BUTTON || Atom->m_Type == TW_TYPE_BOOL8 || Atom->m_Type == TW_TYPE_BOOL16 || Atom->m_Type == TW_TYPE_BOOL32 || Atom->m_Type == TW_TYPE_BOOLCPP || IsEnumType(Atom->m_Type);
						if (!Atom->IsReadOnly() && !m_IsPopupList && Accept) {
							Atom->Increment(+1);
							if (g_TwMgr == NULL) // Mgr might have been destroyed by the client inside a callback call
								return 1;
							m_HighlightClickBtnAuto = g_TwMgr->m_Timer.GetTime();
							NotUpToDate();
						}
					} else {
						CTwVarGroup *Grp = static_cast<CTwVarGroup *>(m_HierTags[m_HighlightedLine].m_Var);
						if (!Grp->m_Open) {
							Grp->m_Open = true;
							NotUpToDate();
						}
					}
					Handled = true;
				} else if (_Key == TW_KEY_LEFT) {
					if (!m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
						CTwVarAtom *Atom = static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
						bool Accept = !Atom->m_NoSlider || Atom->m_Type == TW_TYPE_BUTTON || Atom->m_Type == TW_TYPE_BOOL8 || Atom->m_Type == TW_TYPE_BOOL16 || Atom->m_Type == TW_TYPE_BOOL32 || Atom->m_Type == TW_TYPE_BOOLCPP || IsEnumType(Atom->m_Type);
						if (!Atom->IsReadOnly() && Accept && !m_IsPopupList) {
							Atom->Increment(-1);
							if (g_TwMgr == NULL) // Mgr might have been destroyed by the client inside a callback call
								return 1;
							m_HighlightClickBtnAuto = g_TwMgr->m_Timer.GetTime();
							NotUpToDate();
						}
					} else {
						CTwVarGroup *Grp = static_cast<CTwVarGroup *>(m_HierTags[m_HighlightedLine].m_Var);
						if (Grp->m_Open) {
							Grp->m_Open = false;
							NotUpToDate();
						}
					}
					Handled = true;
				} else if (_Key == TW_KEY_RETURN) {
					if (!m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
						CTwVarAtom *Atom = static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
						if (!Atom->IsReadOnly()) {
							if (Atom->m_Type == TW_TYPE_BUTTON || Atom->m_Type == TW_TYPE_BOOLCPP || Atom->m_Type == TW_TYPE_BOOL8 || Atom->m_Type == TW_TYPE_BOOL16 || Atom->m_Type == TW_TYPE_BOOL32) {
								bool isPopup = m_IsPopupList;
								Atom->Increment(+1);
								if (g_TwMgr == NULL // Mgr might have been destroyed by the client inside a callback call
										|| isPopup) // A popup destroys itself
									return 1;
								m_HighlightClickBtnAuto = g_TwMgr->m_Timer.GetTime();
								NotUpToDate();
							} else { // if (IsEnumType(Atom->m_Type))
								// simulate a mouse click
								int y = m_PosY + m_VarY0 + m_HighlightedLine * (m_Font->m_CharHeight + m_LineSep) + m_Font->m_CharHeight / 2;
								int x = m_PosX + m_VarX1 + 2;
								if (x > m_PosX + m_VarX2 - 2)
									x = m_PosX + m_VarX2 - 2;
								MouseMotion(x, y);
								MouseButton(TW_MOUSE_LEFT, true, x, y);
							}
						}
					} else {
						CTwVarGroup *Grp = static_cast<CTwVarGroup *>(m_HierTags[m_HighlightedLine].m_Var);
						Grp->m_Open = !Grp->m_Open;
						NotUpToDate();
					}
					Handled = true;
				} else if (_Key == TW_KEY_UP) {
					--m_HighlightedLine;
					if (m_HighlightedLine < 0) {
						m_HighlightedLine = 0;
						if (m_FirstLine > 0) {
							--m_FirstLine;
							NotUpToDate();
						}
					}
					m_HighlightedLineLastValid = m_HighlightedLine;
					Handled = true;
				} else if (_Key == TW_KEY_DOWN) {
					++m_HighlightedLine;
					if (m_HighlightedLine >= (int)m_HierTags.size()) {
						m_HighlightedLine = (int)m_HierTags.size() - 1;
						if (m_FirstLine < m_NbHierLines - m_NbDisplayedLines) {
							++m_FirstLine;
							NotUpToDate();
						}
					}
					m_HighlightedLineLastValid = m_HighlightedLine;
					Handled = true;
				} else if (_Key == TW_KEY_ESCAPE && m_IsPopupList) {
					Handled = true;
					CTwBar *LinkedBar = m_BarLinkedToPopupList;
					TwDeleteBar(this);
					g_TwMgr->m_PopupBar = NULL;
					if (LinkedBar != NULL)
						LinkedBar->m_DrawHandles = true;
					return true; // this bar has been destroyed
				}
			} else if (BarActive) {
				if (_Key == TW_KEY_UP || _Key == TW_KEY_DOWN || _Key == TW_KEY_LEFT || _Key == TW_KEY_RIGHT || _Key == TW_KEY_RETURN) {
					if (m_HighlightedLineLastValid >= 0 && m_HighlightedLineLastValid < (int)m_HierTags.size())
						m_HighlightedLine = m_HighlightedLineLastValid;
					else if (m_HierTags.size() > 0) {
						if (_Key == TW_KEY_UP)
							m_HighlightedLine = (int)m_HierTags.size() - 1;
						else
							m_HighlightedLine = 0;
					}
					Handled = true;
				} else if (_Key == TW_KEY_ESCAPE && m_IsPopupList) {
					Handled = true;
					CTwBar *LinkedBar = m_BarLinkedToPopupList;
					TwDeleteBar(this);
					g_TwMgr->m_PopupBar = NULL;
					if (LinkedBar != NULL)
						LinkedBar->m_DrawHandles = true;
					return true; // this bar has been destroyed
				}
			}
		}
	}
	return Handled;
}

//  ---------------------------------------------------------------------------

bool CTwBar::KeyTest(int _Key, int _Modifiers) {
	DEV_ASSERT(g_TwMgr->m_Graph && g_TwMgr->m_WndHeight > 0 && g_TwMgr->m_WndWidth > 0);
	bool Handled = false;
	if (!m_UpToDate)
		Update();

	if (_Key > 0 && _Key < TW_KEY_LAST) {
		if (m_EditInPlace.m_Active)
			Handled = true;
		else {
			bool BarActive = (m_DrawHandles || m_IsPopupList) && !m_IsMinimized;
			bool DoIncr;
			CTwVarAtom *Atom = m_VarRoot.FindShortcut(_Key, _Modifiers, &DoIncr);
			if (Atom != NULL && Atom->m_Visible)
				Handled = true;
			else if (BarActive && (_Key == TW_KEY_RIGHT || _Key == TW_KEY_LEFT || _Key == TW_KEY_UP || _Key == TW_KEY_DOWN || _Key == TW_KEY_RETURN || (_Key == TW_KEY_ESCAPE && m_IsPopupList)))
				Handled = true;
		}
	}
	return Handled;
}

//  ---------------------------------------------------------------------------

bool CTwBar::Show(CTwVar *_Var) {
	if (_Var == NULL || !_Var->m_Visible)
		return false;
	if (!m_UpToDate)
		Update();

	if (OpenHier(&m_VarRoot, _Var)) {
		if (!m_UpToDate)
			Update();
		int l = LineInHier(&m_VarRoot, _Var);
		if (l >= 0) {
			int NbLines = (m_VarY1 - m_VarY0 + 1) / (m_Font->m_CharHeight + m_LineSep);
			if (NbLines <= 0)
				NbLines = 1;
			if (l < m_FirstLine || l >= m_FirstLine + NbLines) {
				m_FirstLine = l - NbLines / 2;
				if (m_FirstLine < 0)
					m_FirstLine = 0;
				NotUpToDate();
				Update();
				if (m_NbDisplayedLines < NbLines) {
					m_FirstLine -= NbLines - m_NbDisplayedLines;
					if (m_FirstLine < 0)
						m_FirstLine = 0;
					NotUpToDate();
				}
			}
			m_HighlightedLine = l - m_FirstLine;
			return true;
		}
	}

	return false;
}

//  ---------------------------------------------------------------------------

bool CTwBar::OpenHier(CTwVarGroup *_Root, CTwVar *_Var) {
	DEV_ASSERT(_Root != nullptr);
	for (size_t i = 0; i < _Root->m_Vars.size(); ++i)
		if (_Root->m_Vars[i] != nullptr) {
			if (_Var == _Root->m_Vars[i] || (_Root->m_Vars[i]->IsGroup() && OpenHier(static_cast<CTwVarGroup *>(_Root->m_Vars[i]), _Var))) {
				_Root->m_Open = true;
				NotUpToDate();
				return true;
			}
		}
	return false;
}

//  ---------------------------------------------------------------------------

int CTwBar::LineInHier(CTwVarGroup *_Root, CTwVar *_Var) {
	DEV_ASSERT(_Root != nullptr);
	int l = 0;
	for (size_t i = 0; i < _Root->m_Vars.size(); ++i)
		if (_Root->m_Vars[i] != nullptr && _Root->m_Vars[i]->m_Visible) {
			if (_Var == _Root->m_Vars[i])
				return l;
			else if (_Root->m_Vars[i]->IsGroup() && static_cast<CTwVarGroup *>(_Root->m_Vars[i])->m_Open) {
				++l;
				int ll = LineInHier(static_cast<CTwVarGroup *>(_Root->m_Vars[i]), _Var);
				if (ll >= 0)
					return l + ll;
				else
					l += -ll - 2;
			}
			++l;
		}
	return -l - 1;
}

//  ---------------------------------------------------------------------------

void DrawArc(int _X, int _Y, int _Radius, float _StartAngleDeg, float _EndAngleDeg, color32 _Color) { // angles in degree
	ITwGraph *Gr = g_TwMgr->m_Graph;
	if (Gr == nullptr || !Gr->IsDrawing() || _Radius == 0 || _StartAngleDeg == _EndAngleDeg)
		return;

	float startAngle = (float)M_PI * _StartAngleDeg / 180;
	float endAngle = (float)M_PI * _EndAngleDeg / 180;
	// float stepAngle = 8/(float)_Radius;   // segment length = 8 pixels
	float stepAngle = 4 / (float)_Radius; // segment length = 4 pixels
	if (stepAngle > (float)M_PI / 4)
		stepAngle = (float)M_PI / 4;
	bool fullCircle = Math::abs(endAngle - startAngle) >= 2.0f * (float)M_PI + Math::abs(stepAngle);
	int numSteps;
	if (fullCircle) {
		numSteps = int((2.0f * (float)M_PI) / stepAngle);
		startAngle = 0;
		endAngle = 2.0f * (float)M_PI;
	} else
		numSteps = int(Math::abs(endAngle - startAngle) / stepAngle);
	if (startAngle > endAngle)
		stepAngle = -stepAngle;

	int x0 = int(_X + _Radius * Math::cos(startAngle) + 0.5f);
	int y0 = int(_Y - _Radius * Math::sin(startAngle) + 0.5f);
	int x1, y1;
	float angle = startAngle + stepAngle;

	for (int i = 0; i < numSteps; ++i, angle += stepAngle) {
		x1 = int(_X + _Radius * Math::cos(angle) + 0.5f);
		y1 = int(_Y - _Radius * Math::sin(angle) + 0.5f);
		Gr->DrawLine(x0, y0, x1, y1, _Color, true);
		x0 = x1;
		y0 = y1;
	}

	if (fullCircle) {
		x1 = int(_X + _Radius * Math::cos(startAngle) + 0.5f);
		y1 = int(_Y - _Radius * Math::sin(startAngle) + 0.5f);
	} else {
		x1 = int(_X + _Radius * Math::cos(endAngle) + 0.5f);
		y1 = int(_Y - _Radius * Math::sin(endAngle) + 0.5f);
	}
	Gr->DrawLine(x0, y0, x1, y1, _Color, true);
}

//  ---------------------------------------------------------------------------

CTwBar::CRotoSlider::CRotoSlider() {
	m_Var = NULL;
	m_Active = false;
	m_ActiveMiddle = false;
	m_Subdiv = 256; // will be recalculated in RotoOnLButtonDown
}

void CTwBar::RotoDraw() {
	ITwGraph *Gr = g_TwMgr->m_Graph;
	if (Gr == NULL || !Gr->IsDrawing())
		return;

	if (m_Roto.m_Active) {
		DrawArc(m_Roto.m_Origin.x, m_Roto.m_Origin.y, 32, 0, 360, m_ColRoto);
		DrawArc(m_Roto.m_Origin.x + 1, m_Roto.m_Origin.y, 32, 0, 360, m_ColRoto);
		DrawArc(m_Roto.m_Origin.x, m_Roto.m_Origin.y + 1, 32, 0, 360, m_ColRoto);

		if (m_Roto.m_HasPrevious) {
			double varMax = RotoGetMax();
			double varMin = RotoGetMin();
			double varStep = RotoGetStep();
			if (varMax < DOUBLE_MAX && varMin > -DOUBLE_MAX && fabs(varStep) > DOUBLE_EPS && m_Roto.m_Subdiv > 0) {
				double dtMax = 360.0 * (varMax - m_Roto.m_ValueAngle0) / ((double)m_Roto.m_Subdiv * varStep); //+2;
				double dtMin = 360.0 * (varMin - m_Roto.m_ValueAngle0) / ((double)m_Roto.m_Subdiv * varStep); //-2;

				if (dtMax >= 0 && dtMax < 360 && dtMin <= 0 && dtMin > -360 && fabs(dtMax - dtMin) <= 360) {
					int x1, y1, x2, y2;
					double da = 2.0 * M_PI / m_Roto.m_Subdiv;

					x1 = m_Roto.m_Origin.x + (int)(40 * cos(-M_PI * (m_Roto.m_Angle0 + dtMax) / 180 - da));
					y1 = m_Roto.m_Origin.y + (int)(40 * sin(-M_PI * (m_Roto.m_Angle0 + dtMax) / 180 - da) + 0.5);
					x2 = m_Roto.m_Origin.x + (int)(40 * cos(-M_PI * (m_Roto.m_Angle0 + dtMax - 10) / 180 - da));
					y2 = m_Roto.m_Origin.y + (int)(40 * sin(-M_PI * (m_Roto.m_Angle0 + dtMax - 10) / 180 - da) + 0.5);
					Gr->DrawLine(m_Roto.m_Origin.x, m_Roto.m_Origin.y, x1, y1, m_ColRotoBound, true);
					Gr->DrawLine(m_Roto.m_Origin.x + 1, m_Roto.m_Origin.y, x1 + 1, y1, m_ColRotoBound, true);
					Gr->DrawLine(m_Roto.m_Origin.x, m_Roto.m_Origin.y + 1, x1, y1 + 1, m_ColRotoBound, true);
					Gr->DrawLine(x1, y1, x2, y2, m_ColRotoBound, true);
					Gr->DrawLine(x1 + 1, y1, x2 + 1, y2, m_ColRotoBound, true);
					Gr->DrawLine(x1, y1 + 1, x2, y2 + 1, m_ColRotoBound, true);

					x1 = m_Roto.m_Origin.x + (int)(40 * cos(-M_PI * (m_Roto.m_Angle0 + dtMin) / 180 + da));
					y1 = m_Roto.m_Origin.y + (int)(40 * sin(-M_PI * (m_Roto.m_Angle0 + dtMin) / 180 + da) + 0.5);
					x2 = m_Roto.m_Origin.x + (int)(40 * cos(-M_PI * (m_Roto.m_Angle0 + dtMin + 10) / 180 + da));
					y2 = m_Roto.m_Origin.y + (int)(40 * sin(-M_PI * (m_Roto.m_Angle0 + dtMin + 10) / 180 + da) + 0.5);
					Gr->DrawLine(m_Roto.m_Origin.x, m_Roto.m_Origin.y, x1, y1, m_ColRotoBound, true);
					Gr->DrawLine(m_Roto.m_Origin.x + 1, m_Roto.m_Origin.y, x1 + 1, y1, m_ColRotoBound, true);
					Gr->DrawLine(m_Roto.m_Origin.x, m_Roto.m_Origin.y + 1, x1, y1 + 1, m_ColRotoBound, true);
					Gr->DrawLine(x1, y1, x2, y2, m_ColRotoBound, true);
					Gr->DrawLine(x1 + 1, y1, x2 + 1, y2, m_ColRotoBound, true);
					Gr->DrawLine(x1, y1 + 1, x2, y2 + 1, m_ColRotoBound, true);
				}
			}
		}

		Gr->DrawLine(m_Roto.m_Origin.x + 1, m_Roto.m_Origin.y, m_Roto.m_Current.x + 1, m_Roto.m_Current.y, m_ColRotoVal, true);
		Gr->DrawLine(m_Roto.m_Origin.x, m_Roto.m_Origin.y + 1, m_Roto.m_Current.x, m_Roto.m_Current.y + 1, m_ColRotoVal, true);
		Gr->DrawLine(m_Roto.m_Origin.x, m_Roto.m_Origin.y, m_Roto.m_Current.x, m_Roto.m_Current.y, m_ColRotoVal, true);

		if (fabs(m_Roto.m_AngleDT) >= 1) {
			DrawArc(m_Roto.m_Origin.x, m_Roto.m_Origin.y, 32, float(m_Roto.m_Angle0), float(m_Roto.m_Angle0 + m_Roto.m_AngleDT - 1), m_ColRotoVal);
			DrawArc(m_Roto.m_Origin.x + 1, m_Roto.m_Origin.y, 32, float(m_Roto.m_Angle0), float(m_Roto.m_Angle0 + m_Roto.m_AngleDT - 1), m_ColRotoVal);
			DrawArc(m_Roto.m_Origin.x, m_Roto.m_Origin.y + 1, 32, float(m_Roto.m_Angle0), float(m_Roto.m_Angle0 + m_Roto.m_AngleDT - 1), m_ColRotoVal);
		}
	}
}

double CTwBar::RotoGetValue() const {
	assert(m_Roto.m_Var != nullptr);
	return m_Roto.m_Var->ValueToDouble();
}

void CTwBar::RotoSetValue(double _Val) {
	assert(m_Roto.m_Var != nullptr);
	if (_Val != m_Roto.m_CurrentValue) {
		m_Roto.m_CurrentValue = _Val;
		m_Roto.m_Var->ValueFromDouble(_Val);
		NotUpToDate();
	}
}

double CTwBar::RotoGetMin() const {
	assert(m_Roto.m_Var != nullptr);
	double min = -DOUBLE_MAX;
	m_Roto.m_Var->MinMaxStepToDouble(&min, nullptr, nullptr);
	return min;
}

double CTwBar::RotoGetMax() const {
	assert(m_Roto.m_Var != nullptr);
	double max = DOUBLE_MAX;
	m_Roto.m_Var->MinMaxStepToDouble(nullptr, &max, nullptr);
	return max;
}

double CTwBar::RotoGetStep() const {
	assert(m_Roto.m_Var != nullptr);
	double step = 1;
	m_Roto.m_Var->MinMaxStepToDouble(nullptr, nullptr, &step);
	return step;
}

double CTwBar::RotoGetSteppedValue() const {
	double d = m_Roto.m_PreciseValue - m_Roto.m_Value0;
	double n = int(d / RotoGetStep());
	return m_Roto.m_Value0 + RotoGetStep() * n;
}

void CTwBar::RotoOnMouseMove(int _X, int _Y) {
	CPoint p(_X, _Y);
	if (m_Roto.m_Active) {
		m_Roto.m_Current = p;
		RotoSetValue(RotoGetSteppedValue());
		// DrawManip();
		int ti = -1;
		double t = 0;
		float r = Math::sqrt(float((m_Roto.m_Current.x - m_Roto.m_Origin.x) * (m_Roto.m_Current.x - m_Roto.m_Origin.x) + (m_Roto.m_Current.y - m_Roto.m_Origin.y) * (m_Roto.m_Current.y - m_Roto.m_Origin.y)));
		if (r > m_RotoMinRadius) {
			t = -Math::atan2(double(m_Roto.m_Current.y - m_Roto.m_Origin.y), double(m_Roto.m_Current.x - m_Roto.m_Origin.x));
			ti = (int((t / (2.0 * M_PI) + 1.0) * NB_ROTO_CURSORS + 0.5)) % NB_ROTO_CURSORS;
			if (m_Roto.m_HasPrevious) {
				CPoint v0 = m_Roto.m_Previous - m_Roto.m_Origin;
				CPoint v1 = m_Roto.m_Current - m_Roto.m_Origin;
				double l0 = Math::sqrt(double(v0.x * v0.x + v0.y * v0.y));
				double l1 = Math::sqrt(double(v1.x * v1.x + v1.y * v1.y));
				double dt = Math::acos(MAX(-1 + 1.0e-30, MIN(1 - 1.0e-30, double(v0.x * v1.x + v0.y * v1.y) / (l0 * l1))));
				if (v0.x * v1.y - v0.y * v1.x > 0)
					dt = -dt;
				double preciseInc = double(m_Roto.m_Subdiv) * dt / (2.0 * M_PI) * RotoGetStep();
				if (preciseInc > RotoGetStep() || preciseInc < -RotoGetStep()) {
					m_Roto.m_PreciseValue += preciseInc;
					if (m_Roto.m_PreciseValue > RotoGetMax()) {
						m_Roto.m_PreciseValue = RotoGetMax();
						m_Roto.m_Value0 = RotoGetMax();

						double da = 360 * (RotoGetMax() - m_Roto.m_ValueAngle0) / (double(m_Roto.m_Subdiv) * RotoGetStep());
						m_Roto.m_Angle0 = ((int((t / (2.0 * M_PI) + 1.0) * 360.0 + 0.5)) % 360) - da;
						m_Roto.m_AngleDT = da;
					} else if (m_Roto.m_PreciseValue < RotoGetMin()) {
						m_Roto.m_PreciseValue = RotoGetMin();
						m_Roto.m_Value0 = RotoGetMin();

						double da = 360 * (RotoGetMin() - m_Roto.m_ValueAngle0) / (double(m_Roto.m_Subdiv) * RotoGetStep());
						m_Roto.m_Angle0 = ((int((t / (2.0 * M_PI) + 1.0) * 360.0 + 0.5)) % 360) - da;
						m_Roto.m_AngleDT = da;
					}
					m_Roto.m_Previous = m_Roto.m_Current;
					m_Roto.m_AngleDT += 180.0 * dt / M_PI;
				}
			} else {
				m_Roto.m_Previous = m_Roto.m_Current;
				m_Roto.m_Value0 = RotoGetValue();
				m_Roto.m_PreciseValue = m_Roto.m_Value0;
				m_Roto.m_HasPrevious = true;
				m_Roto.m_Angle0 = (int((t / (2.0 * M_PI) + 1.0) * 360.0 + 0.5)) % 360;
				m_Roto.m_ValueAngle0 = m_Roto.m_Value0;
				m_Roto.m_AngleDT = 0;
			}
		} else {
			if (m_Roto.m_HasPrevious) {
				RotoSetValue(RotoGetSteppedValue());
				m_Roto.m_Value0 = RotoGetValue();
				m_Roto.m_ValueAngle0 = m_Roto.m_Value0;
				m_Roto.m_PreciseValue = m_Roto.m_Value0;
				m_Roto.m_Angle0 = 0;
			}
			m_Roto.m_HasPrevious = false;
			m_Roto.m_AngleDT = 0;
		}
		if (ti >= 0 && ti < NB_ROTO_CURSORS)
			ANT_SET_ROTO_CURSOR(ti);
		else
			ANT_SET_CURSOR(Center);
	} else {
		if (m_HighlightRotoBtn)
			ANT_SET_CURSOR(Point);
		else
			ANT_SET_CURSOR(Arrow);
	}
}

void CTwBar::RotoOnLButtonDown(int _X, int _Y) {
	CPoint p(_X, _Y);
	if (!m_Roto.m_Active && m_HighlightedLine >= 0 && m_HighlightedLine < (int)m_HierTags.size() && m_HierTags[m_HighlightedLine].m_Var && !m_HierTags[m_HighlightedLine].m_Var->IsGroup()) {
		m_Roto.m_Var = static_cast<CTwVarAtom *>(m_HierTags[m_HighlightedLine].m_Var);
		int y = m_PosY + m_VarY0 + m_HighlightedLine * (m_Font->m_CharHeight + m_LineSep) + m_Font->m_CharHeight / 2;
		m_Roto.m_Origin = CPoint(p.x, y); // r.CenterPoint().y);
		m_Roto.m_Current = p;
		m_Roto.m_Active = true;
		m_Roto.m_HasPrevious = false;
		m_Roto.m_Angle0 = 0;
		m_Roto.m_AngleDT = 0;

		//SetCapture();

		m_Roto.m_Value0 = RotoGetValue();
		m_Roto.m_CurrentValue = m_Roto.m_Value0;
		m_Roto.m_ValueAngle0 = m_Roto.m_Value0;
		m_Roto.m_PreciseValue = m_Roto.m_Value0;

		// RotoSetValue(RotoGetSteppedValue());  Not here
		// DrawManip();

		m_Roto.m_Subdiv = m_RotoNbSubdiv;
		// re-adjust m_Subdiv if needed:
		double min = -DOUBLE_MAX, max = DOUBLE_MAX, step = 1;
		m_Roto.m_Var->MinMaxStepToDouble(&min, &max, &step);
		if (fabs(step) > 0 && min > -DOUBLE_MAX && max < DOUBLE_MAX) {
			double dsubdiv = fabs(max - min) / fabs(step) + 0.5;
			if (dsubdiv < m_RotoNbSubdiv / 3)
				m_Roto.m_Subdiv = 3 * (int)dsubdiv;
		}

		ANT_SET_CURSOR(Center);
	}
}

void CTwBar::RotoOnLButtonUp(int /*_X*/, int /*_Y*/) {
	if (!m_Roto.m_ActiveMiddle) {
		// if( m_Roto.m_Var )
		//   RotoSetValue(RotoGetSteppedValue());
		m_Roto.m_Var = nullptr;
		m_Roto.m_Active = false;
	}
}

void CTwBar::RotoOnMButtonDown(int _X, int _Y) {
	if (!m_Roto.m_Active) {
		m_Roto.m_ActiveMiddle = true;
		RotoOnLButtonDown(_X, _Y);
	}
}

void CTwBar::RotoOnMButtonUp(int _X, int _Y) {
	if (m_Roto.m_ActiveMiddle) {
		m_Roto.m_ActiveMiddle = false;
		RotoOnLButtonUp(_X, _Y);
	}
}

//  ---------------------------------------------------------------------------

CTwBar::CEditInPlace::CEditInPlace() {
	DEV_ASSERT(g_TwMgr != nullptr && g_TwMgr->m_Graph != nullptr);

	m_Var = nullptr;
	m_Active = false;
	m_EditTextObj = g_TwMgr->m_Graph->NewTextObj();
	m_EditSelTextObj = g_TwMgr->m_Graph->NewTextObj();

	m_X = m_Y = m_Width = 0;
}

CTwBar::CEditInPlace::~CEditInPlace() {
	DEV_ASSERT(g_TwMgr != nullptr && g_TwMgr->m_Graph != nullptr);

	if (m_EditTextObj)
		g_TwMgr->m_Graph->DeleteTextObj(m_EditTextObj);
	if (m_EditSelTextObj)
		g_TwMgr->m_Graph->DeleteTextObj(m_EditSelTextObj);
}

bool CTwBar::EditInPlaceIsReadOnly() {
	if (m_EditInPlace.m_Var == nullptr)
		return true;
	else if (m_EditInPlace.m_Var->m_ReadOnly)
		return true;
	else if (m_EditInPlace.m_Var->m_Type == TW_TYPE_CDSTRING && ((m_EditInPlace.m_Var->m_Ptr == nullptr && m_EditInPlace.m_Var->m_SetCallback == nullptr) || (m_EditInPlace.m_Var->m_Ptr != nullptr && g_TwMgr->m_CopyCDStringToClient == nullptr)))
		return true;
	else if (m_EditInPlace.m_Var->m_Type == TW_TYPE_CDSTDSTRING && m_EditInPlace.m_Var->m_SetCallback == nullptr)
		return true;
	else if (m_EditInPlace.m_Var->m_Type == TW_TYPE_STDSTRING && ((m_EditInPlace.m_Var->m_Ptr == nullptr && m_EditInPlace.m_Var->m_SetCallback == nullptr) || (m_EditInPlace.m_Var->m_Ptr != nullptr && g_TwMgr->m_CopyStdStringToClient == nullptr)))
		return true;
	else
		return false;
}

void CTwBar::EditInPlaceDraw() {
	if (!m_EditInPlace.m_Active || m_EditInPlace.m_Var == NULL || m_EditInPlace.m_Width <= 0)
		return;

	// adjust m_FirstChar to see the caret, and extract the visible sub-string
	int StringLen = (int)m_EditInPlace.m_String.length();
	if (m_EditInPlace.m_FirstChar > m_EditInPlace.m_CaretPos)
		m_EditInPlace.m_FirstChar = m_EditInPlace.m_CaretPos;
	int i, SubstrWidth = 0;
	for (i = MIN(m_EditInPlace.m_CaretPos, StringLen - 1); i >= 0 && SubstrWidth < m_EditInPlace.m_Width; --i) {
		unsigned char u = m_EditInPlace.m_String.c_str()[i];
		SubstrWidth += m_Font->m_CharWidth[u];
	}
	int FirstChar = MAX(0, i);
	if (SubstrWidth >= m_EditInPlace.m_Width)
		FirstChar += 2;
	if (m_EditInPlace.m_FirstChar < FirstChar && FirstChar < StringLen)
		m_EditInPlace.m_FirstChar = FirstChar;
	if (m_EditInPlace.m_CaretPos == m_EditInPlace.m_FirstChar && m_EditInPlace.m_FirstChar > 0)
		--m_EditInPlace.m_FirstChar;
	SubstrWidth = 0;
	for (i = m_EditInPlace.m_FirstChar; i < StringLen && SubstrWidth < m_EditInPlace.m_Width; ++i) {
		unsigned char u = m_EditInPlace.m_String.c_str()[i];
		SubstrWidth += m_Font->m_CharWidth[u];
	}
	int LastChar = i;
	if (SubstrWidth >= m_EditInPlace.m_Width)
		--LastChar;
	std::string Substr = m_EditInPlace.m_String.substr(m_EditInPlace.m_FirstChar, LastChar - m_EditInPlace.m_FirstChar);

	// compute caret x pos
	int CaretX = m_PosX + m_EditInPlace.m_X;
	for (i = m_EditInPlace.m_FirstChar; i < m_EditInPlace.m_CaretPos && i < StringLen; ++i) {
		unsigned char u = m_EditInPlace.m_String.c_str()[i];
		CaretX += m_Font->m_CharWidth[u];
	}

	// draw edit text
	color32 ColText = EditInPlaceIsReadOnly() ? m_ColValTextRO : m_ColEditText;
	color32 ColBg = EditInPlaceIsReadOnly() ? m_ColValBg : m_ColEditBg;
	g_TwMgr->m_Graph->BuildText(m_EditInPlace.m_EditTextObj, &Substr, NULL, NULL, 1, m_Font, 0, m_EditInPlace.m_Width);
	g_TwMgr->m_Graph->DrawText(m_EditInPlace.m_EditTextObj, m_PosX + m_EditInPlace.m_X, m_PosY + m_EditInPlace.m_Y, ColText, ColBg);

	// draw selected text
	std::string StrSelected = "";
	if (m_EditInPlace.m_CaretPos > m_EditInPlace.m_SelectionStart) {
		int FirstSel = MAX(m_EditInPlace.m_SelectionStart, m_EditInPlace.m_FirstChar);
		int LastSel = MIN(m_EditInPlace.m_CaretPos, LastChar);
		StrSelected = m_EditInPlace.m_String.substr(FirstSel, LastSel - FirstSel);
	} else {
		int FirstSel = MAX(m_EditInPlace.m_CaretPos, m_EditInPlace.m_FirstChar);
		int LastSel = MIN(m_EditInPlace.m_SelectionStart, LastChar);
		StrSelected = m_EditInPlace.m_String.substr(FirstSel, LastSel - FirstSel);
	}
	int SelWidth = 0;
	for (i = 0; i < (int)StrSelected.length(); ++i) {
		unsigned char u = StrSelected.c_str()[i];
		SelWidth += m_Font->m_CharWidth[u];
	}
	if (SelWidth > 0 && StrSelected.length() > 0) {
		color32 ColSelBg = EditInPlaceIsReadOnly() ? m_ColValTextRO : m_ColEditSelBg;
		g_TwMgr->m_Graph->BuildText(m_EditInPlace.m_EditSelTextObj, &StrSelected, NULL, NULL, 1, m_Font, 0, SelWidth);
		if (m_EditInPlace.m_CaretPos > m_EditInPlace.m_SelectionStart)
			g_TwMgr->m_Graph->DrawText(m_EditInPlace.m_EditSelTextObj, CaretX - SelWidth, m_PosY + m_EditInPlace.m_Y, m_ColEditSelText, ColSelBg);
		else
			g_TwMgr->m_Graph->DrawText(m_EditInPlace.m_EditSelTextObj, CaretX, m_PosY + m_EditInPlace.m_Y, m_ColEditSelText, ColSelBg);
	}

	// draw caret
	if (CaretX <= m_PosX + m_EditInPlace.m_X + m_EditInPlace.m_Width)
		g_TwMgr->m_Graph->DrawLine(CaretX, m_PosY + m_EditInPlace.m_Y + 1, CaretX, m_PosY + m_EditInPlace.m_Y + m_Font->m_CharHeight, m_ColEditText);
}

bool CTwBar::EditInPlaceAcceptVar(const CTwVarAtom *_Var) {
	if (_Var == NULL)
		return false;
	if (_Var->m_Type >= TW_TYPE_CHAR && _Var->m_Type <= TW_TYPE_DOUBLE)
		return true;
	if (_Var->m_Type == TW_TYPE_CDSTRING || _Var->m_Type == TW_TYPE_CDSTDSTRING || _Var->m_Type == TW_TYPE_STDSTRING)
		return true;
	if (IsCSStringType(_Var->m_Type))
		return true;

	return false;
}

void CTwBar::EditInPlaceStart(CTwVarAtom *_Var, int _X, int _Y, int _Width) {
	if (m_EditInPlace.m_Active)
		EditInPlaceEnd(true);

	m_EditInPlace.m_Active = true;
	m_EditInPlace.m_Var = _Var;
	m_EditInPlace.m_X = _X;
	m_EditInPlace.m_Y = _Y;
	m_EditInPlace.m_Width = _Width;
	m_EditInPlace.m_Var->ValueToString(&m_EditInPlace.m_String);
	if (m_EditInPlace.m_Var->m_Type == TW_TYPE_CHAR)
		m_EditInPlace.m_String = m_EditInPlace.m_String.substr(0, 1);
	m_EditInPlace.m_CaretPos = (int)m_EditInPlace.m_String.length();
	if (EditInPlaceIsReadOnly())
		m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos;
	else
		m_EditInPlace.m_SelectionStart = 0;
	m_EditInPlace.m_FirstChar = 0;
}

void CTwBar::EditInPlaceEnd(bool _Commit) {
	if (_Commit && m_EditInPlace.m_Active && m_EditInPlace.m_Var != nullptr) {
		if (m_EditInPlace.m_Var->m_Type == TW_TYPE_CDSTRING || m_EditInPlace.m_Var->m_Type == TW_TYPE_CDSTDSTRING) {
			if (m_EditInPlace.m_Var->m_SetCallback != nullptr) {
				const char *String = m_EditInPlace.m_String.c_str();
				m_EditInPlace.m_Var->m_SetCallback(&String, m_EditInPlace.m_Var->m_ClientData);
			} else if (m_EditInPlace.m_Var->m_Type != TW_TYPE_CDSTDSTRING) {
				char **StringPtr = (char **)m_EditInPlace.m_Var->m_Ptr;
				if (StringPtr != nullptr && g_TwMgr->m_CopyCDStringToClient != nullptr)
					g_TwMgr->m_CopyCDStringToClient(StringPtr, m_EditInPlace.m_String.c_str());
			}
		} else if (m_EditInPlace.m_Var->m_Type == TW_TYPE_STDSTRING) {
			// this case should never happened: TW_TYPE_STDSTRING are converted to TW_TYPE_CDSTDSTRING by TwAddVar
			if (m_EditInPlace.m_Var->m_SetCallback != nullptr)
				m_EditInPlace.m_Var->m_SetCallback(&(m_EditInPlace.m_String), m_EditInPlace.m_Var->m_ClientData);
			else {
				std::string *StringPtr = (std::string *)m_EditInPlace.m_Var->m_Ptr;
				if (StringPtr != nullptr && g_TwMgr->m_CopyStdStringToClient != nullptr)
					g_TwMgr->m_CopyStdStringToClient(*StringPtr, m_EditInPlace.m_String);
			}
		} else if (IsCSStringType(m_EditInPlace.m_Var->m_Type)) {
			int n = TW_CSSTRING_SIZE(m_EditInPlace.m_Var->m_Type);
			if (n > 0) {
				if ((int)m_EditInPlace.m_String.length() > n - 1)
					m_EditInPlace.m_String.resize(n - 1);
				if (m_EditInPlace.m_Var->m_SetCallback != nullptr)
					m_EditInPlace.m_Var->m_SetCallback(m_EditInPlace.m_String.c_str(), m_EditInPlace.m_Var->m_ClientData);
				else if (m_EditInPlace.m_Var->m_Ptr != nullptr) {
					if (n > 1)
						strncpy((char *)m_EditInPlace.m_Var->m_Ptr, m_EditInPlace.m_String.c_str(), n - 1);
					((char *)m_EditInPlace.m_Var->m_Ptr)[n - 1] = '\0';
				}
			}
		} else {
			double Val = 0, Min = 0, Max = 0, Step = 0;
			int n = 0;
			if (m_EditInPlace.m_Var->m_Type == TW_TYPE_CHAR) {
				unsigned char Char = 0;
				n = sscanf(m_EditInPlace.m_String.c_str(), "%c", &Char);
				Val = Char;
			} else
				n = sscanf(m_EditInPlace.m_String.c_str(), "%lf", &Val);
			if (n == 1) {
				m_EditInPlace.m_Var->MinMaxStepToDouble(&Min, &Max, &Step);
				if (Val < Min)
					Val = Min;
				else if (Val > Max)
					Val = Max;
				m_EditInPlace.m_Var->ValueFromDouble(Val);
			}
		}
		if (g_TwMgr != nullptr) // Mgr might have been destroyed by the client inside a callback call
			NotUpToDate();
	}
	m_EditInPlace.m_Active = false;
	m_EditInPlace.m_Var = nullptr;
}

bool CTwBar::EditInPlaceKeyPressed(int _Key, int _Modifiers) {
	if (!m_EditInPlace.m_Active)
		return false;
	bool Handled = true; // if EditInPlace is active, it catches all key events
	bool DoCopy = false, DoPaste = false;

	switch (_Key) {
		case TW_KEY_ESCAPE:
			EditInPlaceEnd(false);
			break;
		case TW_KEY_RETURN:
			EditInPlaceEnd(true);
			break;
		case TW_KEY_LEFT:
			if (_Modifiers == TW_KMOD_SHIFT)
				m_EditInPlace.m_CaretPos = MAX(0, m_EditInPlace.m_CaretPos - 1);
			else {
				if (m_EditInPlace.m_SelectionStart != m_EditInPlace.m_CaretPos)
					m_EditInPlace.m_CaretPos = MIN(m_EditInPlace.m_SelectionStart, m_EditInPlace.m_CaretPos);
				else
					m_EditInPlace.m_CaretPos = MAX(0, m_EditInPlace.m_CaretPos - 1);
				m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos;
			}
			break;
		case TW_KEY_RIGHT:
			if (_Modifiers == TW_KMOD_SHIFT)
				m_EditInPlace.m_CaretPos = MIN((int)m_EditInPlace.m_String.length(), m_EditInPlace.m_CaretPos + 1);
			else {
				if (m_EditInPlace.m_SelectionStart != m_EditInPlace.m_CaretPos)
					m_EditInPlace.m_CaretPos = MAX(m_EditInPlace.m_SelectionStart, m_EditInPlace.m_CaretPos);
				else
					m_EditInPlace.m_CaretPos = MIN((int)m_EditInPlace.m_String.length(), m_EditInPlace.m_CaretPos + 1);
				m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos;
			}
			break;
		case TW_KEY_BACKSPACE:
			if (!EditInPlaceIsReadOnly()) {
				if (m_EditInPlace.m_SelectionStart == m_EditInPlace.m_CaretPos)
					m_EditInPlace.m_SelectionStart = MAX(0, m_EditInPlace.m_CaretPos - 1);
				EditInPlaceEraseSelect();
			}
			break;
		case TW_KEY_DELETE:
			if (!EditInPlaceIsReadOnly()) {
				if (m_EditInPlace.m_SelectionStart == m_EditInPlace.m_CaretPos)
					m_EditInPlace.m_SelectionStart = MIN(m_EditInPlace.m_CaretPos + 1, (int)m_EditInPlace.m_String.length());
				EditInPlaceEraseSelect();
			}
			break;
		case TW_KEY_HOME:
			m_EditInPlace.m_CaretPos = 0;
			if (_Modifiers != TW_KMOD_SHIFT)
				m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos;
			break;
		case TW_KEY_END:
			m_EditInPlace.m_CaretPos = (int)m_EditInPlace.m_String.length();
			if (_Modifiers != TW_KMOD_SHIFT)
				m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos;
			break;
		case TW_KEY_INSERT:
			if (_Modifiers == TW_KMOD_CTRL)
				DoCopy = true;
			else if (_Modifiers == TW_KMOD_SHIFT)
				DoPaste = true;
			break;
		default:
			if (_Modifiers == TW_KMOD_CTRL) {
				if (_Key == 'c' || _Key == 'C')
					DoCopy = true;
				else if (_Key == 'v' || _Key == 'V')
					DoPaste = true;
			} else if (_Key >= 32 && _Key <= 255) {
				if (!EditInPlaceIsReadOnly() && m_EditInPlace.m_CaretPos >= 0 && m_EditInPlace.m_CaretPos <= (int)m_EditInPlace.m_String.length()) {
					if (m_EditInPlace.m_SelectionStart != m_EditInPlace.m_CaretPos)
						EditInPlaceEraseSelect();
					std::string Str(1, (char)_Key);
					m_EditInPlace.m_String.insert(m_EditInPlace.m_CaretPos, Str);
					++m_EditInPlace.m_CaretPos;
					m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos;
				}
			}
	}

	if (DoPaste && !EditInPlaceIsReadOnly()) {
		if (m_EditInPlace.m_SelectionStart != m_EditInPlace.m_CaretPos)
			EditInPlaceEraseSelect();
		std::string Str = "";
		if (EditInPlaceGetClipboard(&Str) && Str.length() > 0) {
			m_EditInPlace.m_String.insert(m_EditInPlace.m_CaretPos, Str);
			m_EditInPlace.m_CaretPos += (int)Str.length();
			m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos;
		}
	}
	if (DoCopy) {
		std::string Str = "";
		if (m_EditInPlace.m_CaretPos > m_EditInPlace.m_SelectionStart)
			Str = m_EditInPlace.m_String.substr(m_EditInPlace.m_SelectionStart, m_EditInPlace.m_CaretPos - m_EditInPlace.m_SelectionStart);
		else if (m_EditInPlace.m_CaretPos < m_EditInPlace.m_SelectionStart)
			Str = m_EditInPlace.m_String.substr(m_EditInPlace.m_CaretPos, m_EditInPlace.m_SelectionStart - m_EditInPlace.m_CaretPos);
		EditInPlaceSetClipboard(Str);
	}

	return Handled;
}

bool CTwBar::EditInPlaceEraseSelect() {
	DEV_ASSERT(m_EditInPlace.m_Active);
	if (!EditInPlaceIsReadOnly() && m_EditInPlace.m_SelectionStart != m_EditInPlace.m_CaretPos) {
		int PosMin = MIN(m_EditInPlace.m_CaretPos, m_EditInPlace.m_SelectionStart);
		m_EditInPlace.m_String.erase(PosMin, abs(m_EditInPlace.m_CaretPos - m_EditInPlace.m_SelectionStart));
		m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos = PosMin;
		if (m_EditInPlace.m_FirstChar > PosMin)
			m_EditInPlace.m_FirstChar = PosMin;
		return true;
	} else
		return false;
}

bool CTwBar::EditInPlaceMouseMove(int _X, int _Y, bool _Select) {
	if (!m_EditInPlace.m_Active || _Y < m_PosY + m_EditInPlace.m_Y || _Y > m_PosY + m_EditInPlace.m_Y + m_Font->m_CharHeight)
		return false;

	int i, CaretX = m_PosX + m_EditInPlace.m_X;
	for (i = m_EditInPlace.m_FirstChar; i < (int)m_EditInPlace.m_String.length() && CaretX < m_PosX + m_EditInPlace.m_X + m_EditInPlace.m_Width; ++i) {
		unsigned char u = m_EditInPlace.m_String.c_str()[i];
		int CharWidth = m_Font->m_CharWidth[u];
		if (_X < CaretX + CharWidth / 2)
			break;
		CaretX += CharWidth;
	}
	if (CaretX >= m_PosX + m_EditInPlace.m_X + m_EditInPlace.m_Width)
		i = MAX(0, i - 1);

	m_EditInPlace.m_CaretPos = i;
	if (!_Select)
		m_EditInPlace.m_SelectionStart = m_EditInPlace.m_CaretPos;
	return true;
}

bool CTwBar::EditInPlaceGetClipboard(std::string *_OutString) {
	DEV_ASSERT(_OutString != nullptr);
	*_OutString = m_EditInPlace.m_Clipboard; // default implementation
	*_OutString = fromStr(OS::get_singleton()->get_clipboard());
	return true;
}

bool CTwBar::EditInPlaceSetClipboard(const std::string &_String) {
	if (_String.length() <= 0)
		return false; // keep last clipboard
	m_EditInPlace.m_Clipboard = _String; // default implementation
	OS::get_singleton()->set_clipboard(_String.c_str());
	return true;
}

//  ---------------------------------------------------------------------------
//  @file       TwColors.cpp
//  @author     Philippe Decaudin
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

void ColorRGBToHLSf(float _R, float _G, float _B, float *_Hue, float *_Light, float *_Saturation) {
	// Compute HLS from RGB. The r,g,b triplet is between [0,1],
	// hue is between [0,360], light and saturation are [0,1].

	float rnorm, gnorm, bnorm, minval, maxval, msum, mdiff, r, g, b;
	r = g = b = 0;
	if (_R > 0)
		r = _R;
	if (r > 1)
		r = 1;
	if (_G > 0)
		g = _G;
	if (g > 1)
		g = 1;
	if (_B > 0)
		b = _B;
	if (b > 1)
		b = 1;

	minval = r;
	if (g < minval)
		minval = g;
	if (b < minval)
		minval = b;
	maxval = r;
	if (g > maxval)
		maxval = g;
	if (b > maxval)
		maxval = b;

	rnorm = gnorm = bnorm = 0;
	mdiff = maxval - minval;
	msum = maxval + minval;
	float l = 0.5 * msum;
	if (_Light)
		*_Light = l;
	if (maxval != minval) {
		rnorm = (maxval - r) / mdiff;
		gnorm = (maxval - g) / mdiff;
		bnorm = (maxval - b) / mdiff;
	} else {
		if (_Saturation)
			*_Saturation = 0;
		if (_Hue)
			*_Hue = 0;
		return;
	}

	if (_Saturation) {
		if (l < 0.5)
			*_Saturation = mdiff / msum;
		else
			*_Saturation = mdiff / (2.0 - msum);
	}

	if (_Hue) {
		if (r == maxval)
			*_Hue = 60.0 * (6 + bnorm - gnorm);
		else if (g == maxval)
			*_Hue = 60.0 * (2 + rnorm - bnorm);
		else
			*_Hue = 60.0 * (4 + gnorm - rnorm);

		if (*_Hue > 360.0)
			*_Hue -= 360.0;
	}
}

void ColorRGBToHLSi(int _R, int _G, int _B, int *_Hue, int *_Light, int *_Saturation) {
	float h, l, s;
	ColorRGBToHLSf((1.0 / 255.0) * float(_R), (1.0 / 255.0) * float(_G), (1.0 / 255.0) * float(_B), &h, &l, &s);
	if (_Hue)
		*_Hue = (int)TClamp(h * (256.0 / 360.0), 0.0, 255.0);
	if (_Light)
		*_Light = (int)TClamp(l * 256.0, 0.0, 255.0);
	if (_Saturation)
		*_Saturation = (int)TClamp(s * 256.0, 0.0, 255.0);
}

void ColorHLSToRGBf(float _Hue, float _Light, float _Saturation, float *_R, float *_G, float *_B) {
	// Compute RGB from HLS. The light and saturation are between [0,1]
	// and hue is between [0,360]. The returned r,g,b triplet is between [0,1].

	// a local auxiliary function
	struct CLocal {
		static float HLSToRGB(float _Rn1, float _Rn2, float _Huei) {
			float hue = _Huei;
			if (hue > 360)
				hue = hue - 360;
			if (hue < 0)
				hue = hue + 360;
			if (hue < 60)
				return _Rn1 + (_Rn2 - _Rn1) * hue / 60;
			if (hue < 180)
				return _Rn2;
			if (hue < 240)
				return _Rn1 + (_Rn2 - _Rn1) * (240 - hue) / 60;
			return _Rn1;
		}
	};

	float rh, rl, rs, rm1, rm2;
	rh = rl = rs = 0;
	if (_Hue > 0)
		rh = _Hue;
	if (rh > 360)
		rh = 360;
	if (_Light > 0)
		rl = _Light;
	if (rl > 1)
		rl = 1;
	if (_Saturation > 0)
		rs = _Saturation;
	if (rs > 1)
		rs = 1;

	if (rl <= 0.5)
		rm2 = rl * (1.0 + rs);
	else
		rm2 = rl + rs - rl * rs;
	rm1 = 2.0 * rl - rm2;

	if (!rs) {
		if (_R)
			*_R = rl;
		if (_G)
			*_G = rl;
		if (_B)
			*_B = rl;
	} else {
		if (_R)
			*_R = CLocal::HLSToRGB(rm1, rm2, rh + 120);
		if (_G)
			*_G = CLocal::HLSToRGB(rm1, rm2, rh);
		if (_B)
			*_B = CLocal::HLSToRGB(rm1, rm2, rh - 120);
	}
}

void ColorHLSToRGBi(int _Hue, int _Light, int _Saturation, int *_R, int *_G, int *_B) {
	float r, g, b;
	ColorHLSToRGBf((360.0f / 255.0f) * float(_Hue), (1.0f / 255.0f) * float(_Light), (1.0f / 255.0f) * float(_Saturation), &r, &g, &b);
	if (_R)
		*_R = (int)TClamp(r * 256.0f, 0.0f, 255.0f);
	if (_G)
		*_G = (int)TClamp(g * 256.0f, 0.0f, 255.0f);
	if (_B)
		*_B = (int)TClamp(b * 256.0f, 0.0f, 255.0f);
}

color32 ColorBlend(color32 _Color1, color32 _Color2, float _S) {
	float a1, r1, g1, b1, a2, r2, g2, b2;
	Color32ToARGBf(_Color1, &a1, &r1, &g1, &b1);
	Color32ToARGBf(_Color2, &a2, &r2, &g2, &b2);
	const float t = 1 - _S;
	return Color32FromARGBf(t * a1 + _S * a2, t * r1 + _S * r2, t * g1 + _S * g2, t * b1 + _S * b2);
}

//  ---------------------------------------------------------------------------
//  @file       TwGodotEvents.h
//  @brief      Godot Engine events, mapping and integration functions.
//  @author     Pawel Piecuch
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

void TW_CALL CopyCDStringToClient(char **destPtr, const char *src);
void TW_CALL CopyStdStringToClient(std::string &destClientString, const std::string &srcLibraryString);

void TW_CALL CopyCDStringToClient(char **destPtr, const char *src) {
	size_t srcLen = (src != nullptr) ? strlen(src) : 0;
	size_t destLen = (*destPtr != nullptr) ? strlen(*destPtr) : 0;

	// Alloc or realloc dest memory block if needed
	if (*destPtr == nullptr)
		*destPtr = (char *)malloc(srcLen + 1);
	else if (srcLen > destLen)
		*destPtr = (char *)realloc(*destPtr, srcLen + 1);

	// Copy src
	if (srcLen > 0)
		strncpy(*destPtr, src, srcLen);
	(*destPtr)[srcLen] = '\0'; // null-terminated string
}

void TW_CALL CopyStdStringToClient(std::string &destClientString, const std::string &srcLibraryString) {
	destClientString = srcLibraryString;
}

int TwEventGodot(const Ref<InputEvent> &ev) {
	int handled = 0;

	if (Ref<InputEventMouseButton> mb = ev) {
		const Vector2 gpoint = mb->get_position();
		TwMouseMotion(gpoint.x, gpoint.y);

		switch (mb->get_button_index()) {
			case BUTTON_LEFT: {
				handled = TwMouseButton(mb->is_pressed() ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
			} break;
			case BUTTON_RIGHT: {
				handled = TwMouseButton(mb->is_pressed() ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED, TW_MOUSE_RIGHT);
			} break;
			case BUTTON_WHEEL_UP:
			case BUTTON_WHEEL_DOWN: {
			}
		}
	}

	if (Ref<InputEventMouseMotion> mm = ev) {
		const Vector2 gpoint = mm->get_position();
		handled = TwMouseMotion(gpoint.x, gpoint.y);
	}

	if (Ref<InputEventKey> kb = ev) {
		int kmod = 0;
		if (kb->get_shift())
			kmod |= TW_KMOD_SHIFT;
		if (kb->get_command())
			kmod |= TW_KMOD_CTRL;
		if (kb->get_alt())
			kmod |= TW_KMOD_ALT;
		bool down = kb->is_pressed() || kb->is_echo();
		switch (kb->get_scancode()) {
			case KEY_F1: {
				handled = TwKeyPressed(TW_KEY_F1, kmod);
			} break;
			case KEY_LEFT: {
				handled = TwKeyPressed(TW_KEY_LEFT, kmod);
			} break;
			case KEY_UP: {
				handled = TwKeyPressed(TW_KEY_UP, kmod);
			} break;
			case KEY_RIGHT: {
				handled = TwKeyPressed(TW_KEY_RIGHT, kmod);
			} break;
			case KEY_DOWN: {
				handled = TwKeyPressed(TW_KEY_DOWN, kmod);
			} break;
			case KEY_PAGEUP: {
				handled = TwKeyPressed(TW_KEY_PAGE_UP, kmod);
			} break;
			case KEY_PAGEDOWN: {
				handled = TwKeyPressed(TW_KEY_PAGE_DOWN, kmod);
			} break;
			case KEY_HOME: {
				handled = TwKeyPressed(TW_KEY_HOME, kmod);
			} break;
			case KEY_END: {
				handled = TwKeyPressed(TW_KEY_END, kmod);
			} break;
			case KEY_INSERT: {
				handled = TwKeyPressed(TW_KEY_INSERT, kmod);
			} break;
			case KEY_BACKSPACE: {
				handled = TwKeyPressed(TW_KEY_BACKSPACE, kmod);
			} break;
			case KEY_DELETE: {
				handled = TwKeyPressed(TW_KEY_DELETE, kmod);
			} break;
			case KEY_ENTER: {
				handled = TwKeyPressed(TW_KEY_RETURN, kmod);
			} break;
			case KEY_ESCAPE: {
				handled = TwKeyPressed(TW_KEY_ESCAPE, kmod);
			} break;
			case KEY_TAB: {
				handled = TwKeyPressed(TW_KEY_TAB, kmod);
			} break;
			case KEY_SPACE: {
				handled = TwKeyPressed(TW_KEY_SPACE, kmod);
			} break;
		}
	}

	return handled;
}

//  ---------------------------------------------------------------------------
//  @file       TwGodot.h
//  @brief      Godot Engine graph and integration functions.
//  @author     Pawel Piecuch
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//  ---------------------------------------------------------------------------

class CTwGraphGodot : public ITwGraph {
public:
	virtual int Init();
	virtual int Shut();
	virtual void BeginDraw(int _WndWidth, int _WndHeight);
	virtual void EndDraw();
	virtual bool IsDrawing();
	virtual void Restore();
	virtual void DrawLine(int _X0, int _Y0, int _X1, int _Y1, color32 _Color0, color32 _Color1, bool _AntiAliased = false);
	virtual void DrawLine(int _X0, int _Y0, int _X1, int _Y1, color32 _Color, bool _AntiAliased = false) { DrawLine(_X0, _Y0, _X1, _Y1, _Color, _Color, _AntiAliased); }
	virtual void DrawRect(int _X0, int _Y0, int _X1, int _Y1, color32 _Color00, color32 _Color10, color32 _Color01, color32 _Color11);
	virtual void DrawRect(int _X0, int _Y0, int _X1, int _Y1, color32 _Color) { DrawRect(_X0, _Y0, _X1, _Y1, _Color, _Color, _Color, _Color); }
	virtual void DrawTriangles(int _NumTriangles, int *_Vertices, color32 *_Colors, Cull _CullMode);

	virtual void *NewTextObj();
	virtual void DeleteTextObj(void *_TextObj);
	virtual void BuildText(void *_TextObj, const std::string *_TextLines, color32 *_LineColors, color32 *_LineBgColors, int _NbLines, const CTexFont *_Font, int _Sep, int _BgWidth);
	virtual void DrawText(void *_TextObj, int _X, int _Y, color32 _Color, color32 _BgColor);

	virtual void ChangeViewport(int _X0, int _Y0, int _Width, int _Height, int _OffsetX, int _OffsetY);
	virtual void RestoreViewport();
	virtual void SetScissor(int _X0, int _Y0, int _Width, int _Height);

protected:
	bool m_Drawing;
	unsigned m_FontTexID;
	const CTexFont *m_FontTex;
	float m_PrevLineWidth;
	int m_PrevTexEnv;
	int m_PrevPolygonMode[2];
	int m_MaxClipPlanes;
	int m_PrevTexture;
	int m_PrevArrayBufferARB;
	int m_PrevElementArrayBufferARB;
	bool m_PrevVertexProgramARB;
	bool m_PrevFragmentProgramARB;
	unsigned m_PrevProgramObjectARB;
	bool m_PrevTexture3D;
	enum EMaxTextures { MAX_TEXTURES = 128 };
	bool m_PrevActiveTexture1D[MAX_TEXTURES];
	bool m_PrevActiveTexture2D[MAX_TEXTURES];
	bool m_PrevActiveTexture3D[MAX_TEXTURES];
	bool m_PrevClientTexCoordArray[MAX_TEXTURES];
	int m_PrevActiveTextureARB;
	int m_PrevClientActiveTextureARB;
	bool m_SupportTexRect;
	bool m_PrevTexRectARB;
	int m_PrevBlendEquation;
	int m_PrevBlendEquationRGB;
	int m_PrevBlendEquationAlpha;
	int m_PrevBlendSrcRGB;
	int m_PrevBlendDstRGB;
	int m_PrevBlendSrcAlpha;
	int m_PrevBlendDstAlpha;
	unsigned m_PrevVertexArray;
	int m_ViewportInit[4];
	float m_ProjMatrixInit[16];
	enum EMaxVtxAttribs { MAX_VERTEX_ATTRIBS = 128 };
	int m_PrevEnabledVertexAttrib[MAX_VERTEX_ATTRIBS];
	int m_WndWidth;
	int m_WndHeight;

	struct vec2 {
		real_t x, y;
		vec2() {}
		vec2(float _X, float _Y) :
				x(_X), y(_Y) {}
		vec2(int _X, int _Y) :
				x(float(_X)), y(float(_Y)) {}
	};
	struct CTextObj {
		std::vector<vec2> m_TextVerts;
		std::vector<vec2> m_TextUVs;
		std::vector<vec2> m_BgVerts;
		std::vector<color32> m_Colors;
		std::vector<color32> m_BgColors;
	};
};

ITwGraph *TwCreateRenderer(void *_Device) {
	ITwGraph *rendr;
	CanvasItem *canvas = (CanvasItem *)_Device;
	return rendr;
}
