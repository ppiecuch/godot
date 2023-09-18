// kate: replace-tabs on; tab-indents on; tab-width 4; indent-width 4; indent-mode cstyle;

#ifndef TEXTUI_H
#define TEXTUI_H

#include <assert.h>
#include <time.h>
#include <stdint.h>

/* ------------------------------------------------------ */
/* ----- DFlat+ Portability layer ----------------------- */
/* ------------------------------------------------------ */

typedef uint16_t con_char_t;
typedef struct char_info_t {
    union {
        struct {
            char character;
            char attribute;
        };
        con_char_t ch;
    };
} char_info_t;

int getScreenWidth();
int getScreenHeight();

/* ------------------------------------------------------ */
/* ----- DFlat+ Compilation include --------------------- */
/* ------------------------------------------------------ */

/* DFlat+ library compilation include
 * All the internal cross-references between modules.
 */

/* /////// GLOBAL DEFINES /////////////////////////////// */


/* NOTE: Future versions will include these by default.
         Do not alter/rely on them                        */

#define INCLUDE_MULTI_WINDOWS   /* MDI */
#define INCLUDE_SHELLDOS        /* Hability to run DOS apps */
#define INCLUDE_MINIMIZE
#define INCLUDE_MAXIMIZE
#define INCLUDE_RESTORE
#define INCLUDE_EXTENDEDSELECTIONS  /* Multiline text boxes */
#define CLASSIC_WINDOW_NUMBERING 0  /* 0= stack-independent ordering */

/* Components */
#define INCLUDE_PICTUREBOX


/* /////// INCLUDES  //////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ----- programming basics ---- */
#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif
#ifndef YES
# define YES TRUE
#endif
#ifndef NO
# define NO FALSE
#endif
#ifndef BOOL
# define BOOL bool
#endif
#define swapi(a,b) {int x=a;a=b;b=x;}
#define MK_WORD(s, o) (((uint16_t) (s)) | ((uint32_t) (o)) << 16)

typedef struct    {
    int lf,tp,rt,bt;
} RECT;

/* /////// CONSTANTS //////////////////////////////////// */

#define MAXDRIVE 16
#define MAXPATH 256
#define MAXDIR 128
#define MAXFILE 64
#define MAXEXT 8

enum {
    EXTENSION = 0x01,
    FILENAME = 0x02,
    DIRECTORY = 0x04,
    DRIVE = 0x08
};

#define MAXMESSAGES 100
#define DELAYTICKS 1
#define FIRSTDELAY 7
#define DOUBLETICKS 5

#define MAXTEXTLEN 65000U /* maximum text buffer            */
#define EDITLEN     1024  /* starting length for multiliner */
#define ENTRYLEN     256  /* starting length for one-liner  */
#define GROWLENGTH    64  /* buffers grow by this much      */


#define ICONHEIGHT 3      /* Minimized window size */
#define ICONWIDTH  10


/* --------- space between menubar labels --------- */
#define MSPACE 2
/* --------------- border characters ------------- */
#define FOCUS_NW      '\xda'     /* \xc9 */
#define FOCUS_NE      '\xbf'     /* \xbb */
#define FOCUS_SE      '\xd9'     /* \xbc */
#define FOCUS_SW      '\xc0'     /* \xc8 */
#define FOCUS_SIDE    '\xb3'     /* \xba */
#define FOCUS_LINE    '\xc4'     /* \xcd */
#define NW            '\xda'
#define NE            '\xbf'
#define SE            '\xd9'
#define SW            '\xc0'
#define SIDE          '\xb3'
#define LINE          '\xc4'
#define LEDGE         '\xc3'
#define REDGE         '\xb4'
#define SIZETOKEN     '\x04'
/* ------------- scroll bar characters ------------ */
#define UPSCROLLBOX    '\x1e'
#define DOWNSCROLLBOX  '\x1f'
#define LEFTSCROLLBOX  '\x11'
#define RIGHTSCROLLBOX '\x10'
#define SCROLLBARCHAR  '\xb0' /* 176 */
#define SCROLLBOXCHAR  '\xb2' /* 178 */
/* ------------------ menu characters --------------------- */
#define CHECKMARK      (SCREENHEIGHT==25?'\xfb':'\x04')
#define CASCADEPOINTER '\x10'
/* ----------------- title bar characters ----------------- */
#define CONTROLBOXCHAR '\xf0'
#define MAXPOINTER     24      /* maximize token            */
#define MINPOINTER     25      /* minimize token            */
#define RESTOREPOINTER 18      /* restore token             */
/* --------------- text control characters ---------------- */
#define APPLCHAR     176 /* fills application window */
#define SHORTCUTCHAR '~'    /* prefix: shortcut key display */
#define LISTSELECTOR   4    /* selected list box entry      */


/* ------- macros for commands and messages ------- */


#define BEGIN_USER_MESSAGES  enum user_messages { DEFAULT_USER_MESSAGE = 10000,
#define END_USER_MESSAGES    , LAST_USER_MESSAGE  };

#define BEGIN_DIALOG_MESSAGES  enum std_dialog_messages { DEFAULT_DIALOG_MESSAGE = 6000,
#define END_DIALOG_MESSAGES    , LAST_DIALOG_MESSAGE  };


#define BEGIN_USER_COMMANDS  enum user_commands { DEFAULT_USER_COMMAND = 10000,
#define END_USER_COMMANDS    , LAST_USER_COMMAND  };


/* --------- macros to define a menu bar with popdowns and selections ------------- */
#define SEPCHAR                     "\xc4"
#define DEFMENU(m)                  MBAR m = {-1,{
#define POPDOWN(ttl,func,stat)      {ttl,func,stat,-1,0,{
#define CASCADED_POPDOWN(id,func)   {NULL,func,NULL,id,0,{
#define SELECTION(stxt,acc,id,attr) {stxt,acc,id,attr,#acc},
#define SEPARATOR                   {SEPCHAR},
#define ENDPOPDOWN                  {NULL}}},
#define ENDMENU                     {(char *)-1} }};


/* ------------ Macros to describe a Module      ---------- */

#define DEFPROGRAM  MODULE ProgramModule = {
#define DEFDFLATP   MODULE DFlatpModule = {
#define MOD_DESCRIPTION(s)   s,
#define MOD_VERSION(m1,m2,r,p)  m1,m2,r,p,
#define MOD_COPYRIGHT(s)     s
#define MOD_LICENSE(s)      ,s
#define MOD_ABOUT(s)        ,s
#define END_DEFMODULE        };

/* ---------------- commands.h ----------------- */

/*
 * Command values sent as the first parameter
 * in the COMMAND message
 *
 * Add application-specific commands to this enum
 */

enum commands {
    /* --------------- System menu -------------- */
#ifdef INCLUDE_RESTORE
    ID_SYSRESTORE,
#endif
    ID_SYSMOVE,
    ID_SYSSIZE,
#ifdef INCLUDE_MINIMIZE
    ID_SYSMINIMIZE,
#endif
#ifdef INCLUDE_MAXIMIZE
    ID_SYSMAXIMIZE,
#endif
    ID_SYSCLOSE,
    /* --------------- Edit box  ---------------- */
    ID_UNDO,
    ID_CUT,
    ID_COPY,
    ID_PASTE,
    ID_PARAGRAPH,
    ID_CLEAR,
    ID_DELETETEXT,
    ID_UPCASE,
    ID_DOWNCASE,
    ID_WORDCOUNT,
    ID_SEARCH,
    ID_REPLACE,
    ID_SEARCHNEXT,
    /* ---- FileOpen and SaveAs dialog boxes ---- */
    ID_FILENAME,
    ID_FILES,
    ID_DIRECTORY,
    ID_DRIVE,
    ID_PATH,
    /* ----- Search and Replace dialog boxes ---- */
    ID_SEARCHFOR,
    ID_REPLACEWITH,
    ID_MATCHCASE,
    ID_REPLACEALL,
    /* ----------- Windows dialog box ----------- */
    ID_WINDOWLIST,
    /* --------- generic command buttons -------- */
    ID_OK,
    ID_CANCEL,
    ID_HELP,
    /* ------------ Display dialog box ---------- */
    ID_BORDER,
    ID_TITLE,
    ID_STATUSBAR,
    ID_TEXTURE,
    ID_COLOR,
    ID_MONO,
    ID_REVERSE,
    /* ---------- Print Select dialog box --------- */
    ID_PRINTERPORT,
    ID_LEFTMARGIN,
    ID_RIGHTMARGIN,
    ID_TOPMARGIN,
    ID_BOTTOMMARGIN,
    /* ----------- InputBox dialog box ------------ */
    ID_INPUTTEXT
};

/* ----------- keys.h ------------ */

#define FKEY 0x1000     /* offset for non-ASCII keys */

#define BELL          7 /* no scancode  */
#define BS            8 /* scancode: 14 */ /* backspace / rubout */
#define TAB           9 /* scancode: 15 */
#define ESC          27 /* scancode:  1 */

#define F1          (FKEY+0x3b) /* scancode: 0x3b */
#define F2          (FKEY+0x3c)
#define F3          (FKEY+0x3d)
#define F4          (FKEY+0x3e)
#define F5          (FKEY+0x3f)
#define F6          (FKEY+0x40)
#define F7          (FKEY+0x41)
#define F8          (FKEY+0x42)
#define F9          (FKEY+0x43)
#define F10         (FKEY+0x44)

#define CTRL_F1     (FKEY+94)
#define CTRL_F2     (FKEY+95)
#define CTRL_F3     (FKEY+96)
#define CTRL_F4     (FKEY+97)
#define CTRL_F5     (FKEY+98)
#define CTRL_F6     (FKEY+99)
#define CTRL_F7     (FKEY+100)
#define CTRL_F8     (FKEY+101)
#define CTRL_F9     (FKEY+102)
#define CTRL_F10    (FKEY+103)

#define ALT_F1      (FKEY+104)
#define ALT_F2      (FKEY+105)
#define ALT_F3      (FKEY+106)
#define ALT_F4      (FKEY+107)
#define ALT_F5      (FKEY+108)
#define ALT_F6      (FKEY+109)
#define ALT_F7      (FKEY+110)
#define ALT_F8      (FKEY+111)
#define ALT_F9      (FKEY+112)
#define ALT_F10     (FKEY+113)

#define HOME        (FKEY+0x47) /* scancode: 0x47 */
#define UP          (FKEY+0x48)
#define PGUP        (FKEY+0x49)
/* 4a: grey- 4b: left 4c: keypad5 4d: right 4e: grey+ */
#define END         (FKEY+0x4f)
#define DN          (FKEY+0x50)
#define PGDN        (FKEY+0x51)
#define INS         (FKEY+0x52)
#define DEL         (FKEY+0x53)

#define LARROW      (FKEY+0x4b) /* -ea */
#define RARROW      (FKEY+0x4d) /* -ea */
/* old name of RARROW is "FWD" ... */

/* valid in ANSI, so assuming that those are universal: */
#define CTRL_END    (FKEY+117)
#define CTRL_PGDN   (FKEY+118)
#define CTRL_HOME   (FKEY+119)
#define CTRL_PGUP   (FKEY+132)

/* #define CTRL_FIVE   (143) */ /* ctrl-numeric-keypad-5 */
#ifdef HOOKKEYB
#define CTRL_FWD    (244)	/* ctrl-rightarrow */
#else
#define CTRL_LARROW (FKEY+0x73)	/* ctrl-leftarrow */
#define CTRL_RARROW (FKEY+0x74)	/* ctrl-rightarrow */
#endif

#define CTRL_BS     (127)	/* yet another deletion keystroke */
#define SHIFT_HT    (FKEY+0x0f) /* scancode: 0x0f */
#define ALT_HYPHEN  (130)

#ifdef HOOKKEYB	/* only available with own int 9 handler! */
#define ALT_BS      (197) /* HOOKKEYB only! */
#define ALT_DEL     (184) /* HOOKKEYB only! */
#define CTRL_INS    (186) /* HOOKKEYB only! */
/* the next few are even missing in our int 9 handler!? */
#define SHIFT_DEL   (198) /* HOOKKEYB only! */
#define SHIFT_INS   (185) /* HOOKKEYB only! */
#define SHIFT_F8    (219) /* HOOKKEYB only! */
#else /* in 0.7, this finally works again, for AT keyboards */
#define ALT_BS      CTRL_Z /* undo block removal */
#define CTRL_INS    CTRL_C /* clipboard copy  */
#define SHIFT_DEL   CTRL_X /* clipboard cut   */
#define SHIFT_INS   CTRL_V /* clipboard paste */
#endif


/* stupid... those depend on keyboard layout! */
#define ALT_A       (FKEY+0x1e) /* scancode 0x1e */
#define ALT_S       (FKEY+0x1f)
#define ALT_D       (FKEY+0x20)
#define ALT_F       (FKEY+0x21)
#define ALT_G       (FKEY+0x22)
#define ALT_H       (FKEY+0x23)
#define ALT_J       (FKEY+0x24)
#define ALT_K       (FKEY+0x25)
#define ALT_L       (FKEY+0x26)

#define ALT_Q       (FKEY+0x10)
#define ALT_W       (FKEY+0x11)
#define ALT_E       (FKEY+0x12)
#define ALT_R       (FKEY+0x13)
#define ALT_T       (FKEY+0x14)
#define ALT_Y       (FKEY+0x15)
#define ALT_U       (FKEY+0x16)
#define ALT_I       (FKEY+0x17)
#define ALT_O       (FKEY+0x18)
#define ALT_P       (FKEY+0x19)

#define ALT_Z       (FKEY+0x2c)
#define ALT_X       (FKEY+0x2d)
#define ALT_C       (FKEY+0x2e)
#define ALT_B       (FKEY+0x2f)
#define ALT_V       (FKEY+0x30)
#define ALT_N       (FKEY+0x31)
#define ALT_M       (FKEY+0x32)


#define ALT_1      (FKEY+0x78)	/* 120 */
#define ALT_2      (FKEY+0x79)
#define ALT_3      (FKEY+0x7a)
#define ALT_4      (FKEY+0x7b)
#define ALT_5      (FKEY+0x7c)
#define ALT_6      (FKEY+0x7d)
#define ALT_7      (FKEY+0x7e)
#define ALT_8      (FKEY+0x7f)
#define ALT_9      (FKEY+0x80)
#define ALT_0      (FKEY+0x81)


/* those are values that are at least typical for DOS: */
#define CTRL_A 1
#define CTRL_B 2
#define CTRL_C 3    /* must have "ignore ^C / ^Break handler to use this */
                    /* see messages.c */
#define CTRL_D 4
#define CTRL_E 5
#define CTRL_F 6	/* (special meaning for DOS-CON readline?) */
#define CTRL_G 7
#define CTRL_H 8
#define CTRL_I 9
#define CTRL_J 10
#define CTRL_K 11
#define CTRL_L 12
#define CTRL_M 13
#define CTRL_N 14
#define CTRL_O 15
#define CTRL_P 16	/* (causes print in DOS-CON) */
#define CTRL_Q 17
#define CTRL_R 18
#define CTRL_S 19	/* (causes scroll-halt in DOS-CON) */
#define CTRL_T 20
#define CTRL_U 21
#define CTRL_V 22
#define CTRL_W 23
#define CTRL_X 24
#define CTRL_Y 25
#define CTRL_Z 26	/* (marks EOF in DOS-CON) */


/* shift bit mask */
#define RIGHTSHIFT 0x01
#define LEFTSHIFT  0x02
#define CTRLKEY    0x04
#define ALTKEY     0x08
#define SCROLLLOCK 0x10
#define NUMLOCK    0x20
#define CAPSLOCK   0x40 /* caps lock BEING on */
#define INSERTKEY  0x80

/* Following is new by Eric 11/2002, but see CONSOLE.C */
#define SYSRQKEY   0x8000
#define CAPSLKEY   0x4000 /* PRESSING caps lock */
#define NUMLKEY    0x2000
#define SCROLLLKEY 0x1000
/* Especially the L/R distinction is important - Eric */
#define RALTKEY    0x800 /* treat this als AltGr, which is NOT Alt */
#define RCTRLKEY   0x400
#define LALTKEY    0x200
#define LCTRLKEY   0x100

struct keys {
    int keycode;
    char *keylabel;
};
extern struct keys keys[];

/* ------------------------------------------------------ */
/* ----- DFlat+ CORE include ---------------------------- */
/* ------------------------------------------------------ */

/* Contains core definitions necessary for the API
 * but that should NOT be used or modified by the
 * developer
 */

/* /////// INCLUDE //////////////////////////////////////// */

/* KEEP THESE ALWAYS ALIGNED WITH DFPCOMP!! */

#define INCLUDE_MULTI_WINDOWS   /* MDI */
#define INCLUDE_SHELLDOS        /* Hability to run DOS apps */
#define INCLUDE_MINIMIZE
#define INCLUDE_MAXIMIZE
#define INCLUDE_RESTORE
#define INCLUDE_EXTENDEDSELECTIONS  /* Multiline text boxes */
#define USECBRKHNLDR 1 /* removing this line disables ctrl-break processing */
#define CLASSIC_WINDOW_NUMBERING 0  /* 0= stack-independent ordering */

/* Components */
#define INCLUDE_PICTUREBOX


/* /////// MODULE ///////////////////////////////////////// */


typedef struct ModuleDesc {
    char *Description;
    int   Ver_maj;
    int   Ver_min;
    int   Ver_rel;
    int   Ver_patch;
    char *Copyright;
    char *License;
    char *AboutComment;
} MODULE;


extern  char  VerStr [64];

#define MK_VER(a,b,c,d)   ((sprintf(VerStr,"%i.%i.%i.%i",a,b,c,d),VerStr))
#define ModuleVersion(m)  (MK_VER(m.Ver_maj,m.Ver_min,m.Ver_rel,m.Ver_patch))

/* /////// CLASS ////////////////////////////////////////// */

typedef enum window_class    {
# define ClassDef(c,b,p,a) c,
# include "textUI_Classes.h"
CLASSCOUNT
} CLASS;


/* /////// DATA TYPES /////////////////////////////////// */

typedef unsigned int MESSAGE;
typedef long PARAM;
typedef PARAM UCOMMAND;


/* Above there goes the definition of:
   MODULE            A programming unit (Program or Library)
   HWND              Handle to a window
   CLASS             Class (number)
   ColorScheme       A color combination for the UI
   VideoResolution   A screen mode resolution
 */

/* ---- types of vectors that can be in a picture box ------- */
enum VectTypes {VECTOR, SOLIDBAR, HEAVYBAR, CROSSBAR, LIGHTBAR};

/* ------------ menu.h ------------- */

#define MAXPULLDOWNS 15
#define MAXSELECTIONS 20
#define MAXCASCADES 3  /* nesting level of cascaded menus */

/* ----------- popdown menu selection structure
       one for each selection on a popdown menu --------- */
struct PopDown {
    char *SelectionTitle;   /* title of the selection */
    int ActionId;           /* the command executed */
    int Accelerator;        /* the accelerator key */
    int Attrib;             /* INACTIVE | CHECKED | TOGGLE | CASCADED*/
    char *help;             /* Help mnemonic */
};

/* ----------- popdown menu structure one for each popdown menu on the menu bar -------- */
typedef struct Menu {
    char *Title;           /* title on the menu bar       */
    void (*PrepMenu)(void *, struct Menu *); /* function  */
    char *StatusText;      /* text for the status bar     */
    int CascadeId;   /* command id of cascading selection */
    int Selection;         /* most recent selection       */
    struct PopDown Selections[MAXSELECTIONS+1];
} MENU;

/* ----- one for each menu bar ----- */
typedef struct MenuBar {
    int ActiveSelection;
    MENU PullDown[MAXPULLDOWNS+1];
} MBAR;


/* -------- menu selection attributes -------- */
#define INACTIVE    1
#define CHECKED     2
#define TOGGLE      4
#define CASCADED    8

/* --------- the standard menus ---------- */
extern MBAR MainMenu;
extern MBAR SystemMenu;
extern MBAR *ActiveMenuBar;

int MenuHeight(struct PopDown *);
int MenuWidth(struct PopDown *);

/* ----------------- dialbox.h ---------------- */

#define MAXCONTROLS 30
#define MAXRADIOS 20

#define OFF FALSE
#define ON  TRUE

/* -------- dialog box and control window structure ------- */
typedef struct  {
    char *title;       /* window title         */
    int x, y;          /* relative coordinates */
    int h, w;          /* size                 */
} DIALOGWINDOW;

/* ------ one of these for each control window ------- */
typedef struct {
    DIALOGWINDOW dwnd;
    CLASS cls;         /* LISTBOX, BUTTON, etc */
    char *itext;       /* initialized text     */
    UCOMMAND command;  /* command code         */
    char *help;        /* help mnemonic        */
    BOOL isetting;     /* initially ON or OFF  */
    BOOL setting;      /* ON or OFF            */
    void *wnd;         /* window handle        */
} CTLWINDOW;

/* --------- one of these for each dialog box ------- */
typedef struct {
    char *HelpName;
    DIALOGWINDOW dwnd;
    CTLWINDOW ctl[MAXCONTROLS+1];
} DBOX;

/* -------- macros for dialog box resource compile -------- */
#define DIALOGBOX(db) DBOX db={ #db,
#define DB_TITLE(ttl,x,y,h,w) {ttl,x,y,h,w},{
#define CONTROL(ty,tx,x,y,h,w,c) {{NULL,x,y,h,w},ty,((ty==EDITBOX||ty==COMBOBOX)?NULL:tx), c,#c,(ty==BUTTON?ON:OFF),OFF,NULL},
#define ENDDB {{NULL}} }};

#define tCancel  " Cancel "
#define tOk      "   OK   "
#define tYes     "  Yes   "
#define tNo      "   No   "

/* /////// WINDOW ///////////////////////////////////////// */

enum Condition     {
    ISRESTORED, ISMINIMIZED, ISMAXIMIZED, ISCLOSING
};


typedef struct window {
    CLASS cls;             /* window class                  */
    char *title;           /* window title                  */
    int (*wndproc)
        (struct window *, MESSAGE, PARAM, PARAM);
    /* ---------------- window dimensions ----------------- */
    RECT rc;               /* window coordinates
                                            (0/0 to 79/24)  */
    int ht, wd;            /* window height and width       */
    RECT RestoredRC;       /* restored condition rect       */
    /* ----------------- window colors -------------------- */
    char WindowColors[4][2];
    /* -------------- linked list pointers ---------------- */
    struct window *parent;      /* parent window            */
    struct window *firstchild;  /* first child this parent  */
    struct window *lastchild;   /* last child this parent   */
    struct window *nextsibling; /* next sibling             */
    struct window *prevsibling; /* previous sibling         */
#if CLASSIC_WINDOW_NUMBERING    /* if enabled, select 0.7c style window list */
                                /* no extra work */
#else                           /* 0.7c: window list does not depend on stacking */
    struct window *numberonechild;      /* first child for list  */
    struct window *nextnumberedsibling; /* next sibling for list */
#endif

    struct window *childfocus;	/* child that ha(s/d) focus */
    int attrib;                 /* Window attributes        */
    char *videosave;            /* video save buffer        */
    enum Condition condition;   /* Restored, Maximized,
                                   Minimized, Closing       */
    enum Condition oldcondition;/* previous condition       */
    BOOL wasCleared;
    int restored_attrib;        /* attributes when restored */
    void *extension;      /* menus, dialogs, documents, etc */
    void *wrapper;             /* used by C++ wrapper class */
    struct window *PrevMouse;   /* previous mouse capture   */
    struct window *PrevKeyboard;/* previous keyboard capture*/
    struct window *PrevClock;   /* previous clock capture   */
    struct window *MenuBarWnd;  /* menu bar                 */
    struct window *StatusBar;   /* status bar               */
    int isHelping;      /* > 0 when help is being displayed */
    /* ----------------- text box fields ------------------ */
    int wlines;     /* number of lines of text              */
    int wtop;       /* text line that is on the top display */
    char *text;          /* window text                     */
    unsigned int textlen;  /* text length                   */
    int wleft;      /* left position in window viewport     */
    int textwidth;  /* width of longest line in textbox     */
    int BlkBegLine; /* beginning line of marked block       */
    int BlkBegCol;  /* beginning column of marked block     */
    int BlkEndLine; /* ending line of marked block          */
    int BlkEndCol;  /* ending column of marked block        */
    int HScrollBox; /* position of horizontal scroll box    */
    int VScrollBox; /* position of vertical scroll box      */
    unsigned int *TextPointers; /* -> list of line offsets  */
    /* ----------------- list box fields ------------------ */
    int selection;  /* current selection                    */
    BOOL AddMode;   /* adding extended selections mode      */
    int AnchorPoint;/* anchor point for extended selections */
    int SelectCount;/* count of selected items              */
    /* ----------------- edit box fields ------------------ */
    int CurrCol;      /* Current column                     */
    int CurrLine;     /* Current line                       */
    int WndRow;       /* Current window row                 */
    BOOL TextChanged; /* TRUE if text has changed           */
    BOOL protect;     /* TRUE to display '*'                */
    char *DeletedText;/* for undo                           */
    unsigned DeletedLength; /* Length of deleted field      */
    BOOL InsertMode;   /* TRUE or FALSE for text insert     */
    BOOL WordWrapMode; /* TRUE or FALSE for word wrap       */
    unsigned int MaxTextLength; /* maximum text length      */
    /* ---------------- dialog box fields ----------------- */
    int ReturnCode;        /* return code from a dialog box */
    BOOL Modal;            /* True if a modeless dialog box */
    CTLWINDOW *ct;         /* control structure             */
    struct window *dfocus; /* control window that has focus */
    /* -------------- popdownmenu fields ------------------ */
    MENU *mnu;          /* points to menu structure         */
    MBAR *holdmenu; /* previous active menu                 */
    struct window *oldFocus;
    /* -------------- status bar fields ------------------- */
    BOOL TimePosted; /* True if time has been posted        */
#ifdef INCLUDE_PICTUREBOX
    /* ------------- picture box fields ------------------- */
    int VectorCount;  /* number of vectors in vector list   */
    void *VectorList; /* list of picture box vectors        */
#endif
} * WINDOW;

typedef WINDOW HWND;

typedef struct classdefs {
    CLASS base; /* base window class */
    int (*wndproc)(struct window *,MESSAGE,PARAM,PARAM);
    int attrib;
} CLASSDEFS;

extern CLASSDEFS classdefs[];

/* /////// MESSAGE //////////////////////////////////////// */


enum messages {
# undef DFlatMsg
# define DFlatMsg(m) m,
# include "textUI_Msg.h"
MESSAGECOUNT
};


/* /////// VIDEO RESOLUTION /////////////////////////////// */

typedef struct videoresolution {
    unsigned int HRes;
    unsigned int VRes;
    char Description[31];
} VideoResolution;

/* /////// COLOR SCHEME /////////////////////////////////// */

typedef struct colorscheme {
    BOOL isMonoScheme;
    unsigned char index;
    unsigned char clrArray [CLASSCOUNT] [4] [2];
} ColorScheme;

enum colortypes {
    STD_COLOR,
    SELECT_COLOR,
    FRAME_COLOR,
    HILITE_COLOR
};

enum grounds { FG, BG };

#define ColorSchemeArraySize (CLASSCOUNT*4*2)


/* /////// MISCELLANEOUS ////////////////////////////////// */

int ErrorBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int YesNoBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int MessageBoxProc(WINDOW, MESSAGE, PARAM, PARAM);

enum alignments {
    ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER
};

/* ------------------------------------------------------ */
/* ----- DFlat+ API include ----------------------------- */
/* ------------------------------------------------------ */


/* /////// CONSTANTS /////////////////////////////////// */

/*
   Class list:       see classes.h
   System Messages:  see dflatmsg.h
*/

extern char *ClassNames[];

/* Modules */
extern MODULE ProgramModule;  /* Must be created by developer */
extern MODULE DFlatpModule;

/* Module version strings (fmt x.y.z.t) */
#define ProgramVersion    (ModuleVersion(ProgramModule))
#define DFlatpVersion     (ModuleVersion(DFlatpModule))

/* System color schemes */
extern ColorScheme color;
extern ColorScheme bw;
extern ColorScheme reverse;

/* Window attributes */
#define SHADOW       0x0001
#define MOVEABLE     0x0002
#define SIZEABLE     0x0004
#define HASMENUBAR   0x0008
#define VSCROLLBAR   0x0010
#define HSCROLLBAR   0x0020
#define VISIBLE      0x0040
#define SAVESELF     0x0080
#define HASTITLEBAR  0x0100
#define CONTROLBOX   0x0200
#define MINMAXBOX    0x0400
#define NOCLIP       0x0800
#define READONLY     0x1000
#define MULTILINE    0x2000
#define HASBORDER    0x4000
#define HASSTATUSBAR 0x8000


/* /////// UI System Enhancements ////////////////////// */


/* Memory allocation */
void *DFcalloc(size_t, size_t);
void *DFmalloc(size_t);
void *DFrealloc(void *, size_t);

int TestCriticalError(void); /* Critical error detection */

BOOL SelectColorScheme (ColorScheme); /* Select color schemes */


/* /////// SYSTEM CONFIGURATION //////////////////////// */

typedef struct sysconfigtype {
    ColorScheme      VideoCurrentColorScheme;   /* Read-only */
    VideoResolution  VideoCurrentResolution;    /* Read-only */
    unsigned char    CountryTimeSeparator;      /* Read-write */
    unsigned char    EditorTabSize;             /* Read-write */
    BOOL             EditorGlobalReadOnly;      /* Read-write */
} SysConfigType;


/* Please be careful and respect the read-only/write note above.
   If you wish to modify the read-only attributes, use the given
   functions to do so.  */
extern SysConfigType SysConfig;


/* /////// MESSAGING SYSTEM //////////////////////////// */

/* Message sending */
void PostMessage(HWND, MESSAGE, PARAM, PARAM);
int SendMessage(HWND, MESSAGE, PARAM, PARAM);

/* Multitasking */
void ProcessMessages (void); /* once finished init, let messages flow */
void Cooperate(void);        /* momentarily process pending messages  */


/* /////// CLASSES ///////////////////////////////////// */


#define BaseWndProc(cls,wnd,msg,p1,p2) \
    (*classdefs[(classdefs[cls].base)].wndproc)(wnd,msg,p1,p2)

#define DefaultWndProc(wnd,msg,p1,p2) \
    (classdefs[wnd->cls].wndproc == NULL) ? BaseWndProc(wnd->cls,wnd,msg,p1,p2) : (*classdefs[wnd->cls].wndproc)(wnd,msg,p1,p2)


/* /////// WINDOW METHODS ////////////////////////////// */

/* ------- window manipulation ------ */

HWND CreateWindow(CLASS,const char *,int,int,int,int,void*,HWND,int (*)(struct window *,MESSAGE,PARAM,PARAM),int);

BOOL isDerivedFrom(HWND, CLASS);
#define GetClass(w) ((w)->cls)


/* ------- color manipulation ------- */

/* Get colors defined for a window */
#define WndForeground(wnd) \
    (wnd->WindowColors [STD_COLOR] [FG])
#define WndBackground(wnd) \
    (wnd->WindowColors [STD_COLOR] [BG])
#define FrameForeground(wnd) \
    (wnd->WindowColors [FRAME_COLOR] [FG])
#define FrameBackground(wnd) \
    (wnd->WindowColors [FRAME_COLOR] [BG])
#define SelectForeground(wnd) \
    (wnd->WindowColors [SELECT_COLOR] [FG])
#define SelectBackground(wnd) \
    (wnd->WindowColors [SELECT_COLOR] [BG])
#define HighlightForeground(wnd) \
    (wnd->WindowColors [HILITE_COLOR] [FG])
#define HighlightBackground(wnd) \
    (wnd->WindowColors [HILITE_COLOR] [BG])

/* Set colors defined for a window */
#define WindowClientColor(wnd, fg, bg) \
    WndForeground(wnd) = fg, WndBackground(wnd) = bg
#define WindowReverseColor(wnd, fg, bg) \
    SelectForeground(wnd) = fg, SelectBackground(wnd) = bg
#define WindowFrameColor(wnd, fg, bg) \
    FrameForeground(wnd) = fg, FrameBackground(wnd) = bg
#define WindowHighlightColor(wnd, fg, bg) \
    HighlightForeground(wnd) = fg, HighlightBackground(wnd) = bg


/* ------- window and client coords - */
#define WindowHeight(w)      ((w)->ht)
#define WindowWidth(w)       ((w)->wd)
#define BorderAdj(w)         (TestAttribute(w,HASBORDER)?1:0)
#define BottomBorderAdj(w)   (TestAttribute(w,HASSTATUSBAR)?1:BorderAdj(w))
#define TopBorderAdj(w)      ((TestAttribute(w,HASTITLEBAR) && TestAttribute(w,HASMENUBAR)) ? 2 : (TestAttribute(w,HASTITLEBAR | HASMENUBAR | HASBORDER) ? 1 : 0))
#define ClientWidth(w)       (WindowWidth(w)-BorderAdj(w)*2)
#define ClientHeight(w)      (WindowHeight(w)-TopBorderAdj(w)-BottomBorderAdj(w))
#define WindowRect(w)        ((w)->rc)
#define GetTop(w)            (RectTop(WindowRect(w)))
#define GetBottom(w)         (RectBottom(WindowRect(w)))
#define GetLeft(w)           (RectLeft(WindowRect(w)))
#define GetRight(w)          (RectRight(WindowRect(w)))
#define GetClientTop(w)      (GetTop(w)+TopBorderAdj(w))
#define GetClientBottom(w)   (GetBottom(w)-BottomBorderAdj(w))
#define GetClientLeft(w)     (GetLeft(w)+BorderAdj(w))
#define GetClientRight(w)    (GetRight(w)-BorderAdj(w))

RECT ClientRect(HWND);
RECT RelativeWindowRect(HWND, RECT);

RECT ClipRectangle(HWND, RECT);
RECT AdjustRectangle(HWND, RECT);


/* ------- window attributes and title - */

void AddTitle(HWND, const char *);
void DisplayTitle(HWND, RECT *);
#define GetTitle(w)          ((w)->title)

#define GetAttribute(w)      ((w)->attrib)
#define AddAttribute(w,a)    (GetAttribute(w) |= a)
#define ClearAttribute(w,a)  (GetAttribute(w) &= ~(a))
#define TestAttribute(w,a)   (GetAttribute(w) & (a))
#define isHidden(w)          (!(GetAttribute(w) & VISIBLE))
#define SetVisible(w)        (GetAttribute(w) |= VISIBLE)
#define ClearVisible(w)      (GetAttribute(w) &= ~VISIBLE)

BOOL isVisible(HWND);

extern BOOL WindowMoving;
extern BOOL WindowSizing;
extern BOOL VSliding;
extern BOOL HSliding;


/* ------- painting and content ----- */

extern int foreground, background;

void SetStandardColor(HWND);
void SetReverseColor(HWND);

void RepaintBorder(HWND, RECT *);
void PaintShadow(HWND);
void ClearWindow(HWND, RECT *, int);
void writeline(HWND, const char *, int, int, BOOL);
#define gotoxy(w,x,y) cursor(w->rc.lf+(x)+1,w->rc.tp+(y)+1)

extern BOOL ClipString;

BOOL CharInView(HWND, int, int);
void PutWindowChar(HWND,int,int,int);
void PutWindowLine(HWND, char *,int,int);


/* ------- object hierarchy/MDI ----- */

HWND GetAncestor(HWND);
BOOL isAncestor(HWND, HWND);

#define GetParent(w)         ((w)->parent)
#define FirstWindow(w)       ((w)->firstchild)

#define NumberOneChildWindow(w) ((w)->numberonechild)
#define NextNumberedWindow(w)   ((w)->nextnumberedsibling)
#define LastWindow(w)        ((w)->lastchild)
#define NextWindow(w)        ((w)->nextsibling)
#define PrevWindow(w)        ((w)->prevsibling)

void RemoveWindow(HWND);
void AppendWindow(HWND);

void ChooseWindow(HWND, int);
void CloseAll(HWND, int);


/* ------- Window focus ------------- */

#define FocusedWindow(w)     ((w)->childfocus)
HWND GetDocFocus(void);

void SetNextFocus(void);
void SetPrevFocus(void);

void ReFocus(HWND);


/* /////// OTHER CLASSES' METHODS ////////////////////// */


/* --------- menu prototypes ---------- */
BOOL isActive(MBAR *, int);
char *GetCommandText(MBAR *, int);
void ActivateCommand(MBAR *,int);
void DeactivateCommand(MBAR *,int);
BOOL GetCommandToggle(MBAR *,int);
void SetCommandToggle(MBAR *,int);
void ClearCommandToggle(MBAR *,int);
void InvertCommandToggle(MBAR *,int);

extern int CurrentMenuSelection;


/* ------------- list box prototypes -------------- */
BOOL ItemSelected(HWND, int);

/* -------- text box prototypes ---------- */
#define TextLine(wnd, sel) (wnd->text + *((wnd->TextPointers) + sel))
void WriteTextLine(HWND, RECT *, int, BOOL);
#define TextBlockMarked(wnd) (wnd->BlkBegLine || wnd->BlkEndLine || wnd->BlkBegCol  || wnd->BlkEndCol)
void MarkTextBlock(HWND, int, int, int, int);
#define ClearTextBlock(wnd) wnd->BlkBegLine = wnd->BlkEndLine = wnd->BlkBegCol  = wnd->BlkEndCol = 0;
#define TextBlockBegin(wnd) (TextLine(wnd,wnd->BlkBegLine)+wnd->BlkBegCol)
#define TextBlockEnd(wnd)   (TextLine(wnd,wnd->BlkEndLine)+wnd->BlkEndCol)
#define GetText(w)          ((w)->text)
#define GetTextLines(w)     ((w)->wlines)
void ClearTextPointers(HWND);
void BuildTextPointers(HWND);
int TextLineNumber(HWND, char *);
void UpCaseMarked(HWND);    /* new 0.7d */
void DownCaseMarked(HWND);  /* new 0.7d */
void StatsForMarked(HWND, unsigned *, unsigned *, unsigned *); /* new 0.7d */

/* ------------- edit box prototypes ----------- */
#define CurrChar (TextLine(wnd, wnd->CurrLine)+wnd->CurrCol)
#define WndCol   (wnd->CurrCol-wnd->wleft)
#define isMultiLine(wnd) TestAttribute(wnd, MULTILINE)
#define SetProtected(wnd) (wnd)->protect=TRUE

void SearchText(HWND);
void ReplaceText(HWND);
void SearchNext(HWND);

/* ------------- picture box prototypes ------------- */
void DrawVector(HWND, int, int, int, int);
void DrawBox(HWND, int, int, int, int);
void DrawBar(HWND, enum VectTypes, int, int, int, int);
HWND WatchIcon(void);


/* /////// DIALOG BOXES  /////////////////////////////// */


BOOL DialogBox(HWND, DBOX *, BOOL,
       int (*)(struct window *, MESSAGE, PARAM, PARAM));
void ClearDialogBoxes(void);
void GetDlgListText(HWND, char *, UCOMMAND);
BOOL RadioButtonSetting(DBOX *, UCOMMAND);
void PushRadioButton(DBOX *, UCOMMAND);
void PutItemText(HWND, UCOMMAND, char *);
void PutComboListText(HWND, UCOMMAND, char *);
void GetItemText(HWND, UCOMMAND, char *, int);
char *GetDlgTextString(DBOX *, UCOMMAND, CLASS);
void SetDlgTextString(DBOX *, UCOMMAND, char *, CLASS);
BOOL CheckBoxSetting(DBOX *, UCOMMAND);
CTLWINDOW *FindCommand(DBOX *, UCOMMAND, int);
HWND ControlWindow(const DBOX *, UCOMMAND);
void SetRadioButton(DBOX *, CTLWINDOW *);
void ControlSetting(DBOX *, UCOMMAND, int, int);
BOOL isControlOn(DBOX *, UCOMMAND, int);
void SetFocusCursor(HWND);

#define GetControl(wnd)             (wnd->ct)
#define GetDlgText(db, cmd)         GetDlgTextString(db, cmd, TEXT)
#define GetDlgTextBox(db, cmd)      GetDlgTextString(db, cmd, TEXTBOX)
#define GetEditBoxText(db, cmd)     GetDlgTextString(db, cmd, EDITBOX)
#define GetComboBoxText(db, cmd)    GetDlgTextString(db, cmd, COMBOBOX)
#define SetDlgText(db, cmd, s)      SetDlgTextString(db, cmd, s, TEXT)
#define SetDlgTextBox(db, cmd, s)   SetDlgTextString(db, cmd, s, TEXTBOX)
#define SetEditBoxText(db, cmd, s)  SetDlgTextString(db, cmd, s, EDITBOX)
#define SetComboBoxText(db, cmd, s) SetDlgTextString(db, cmd, s, COMBOBOX)
#define SetDlgTitle(db, ttl)        ((db)->dwnd.title = ttl)
#define SetCheckBox(db, cmd)        ControlSetting(db, cmd, CHECKBOX, ON)
#define ClearCheckBox(db, cmd)      ControlSetting(db, cmd, CHECKBOX, OFF)
#define EnableButton(db, cmd)       ControlSetting(db, cmd, BUTTON, ON)
#define DisableButton(db, cmd)      ControlSetting(db, cmd, BUTTON, OFF)
#define ButtonEnabled(db, cmd)      isControlOn(db, cmd, BUTTON)
#define CheckBoxEnabled(db, cmd)    isControlOn(db, cmd, CHECKBOX)


/* /////// MESSAGE BOXES  ////////////////////////////// */

HWND SliderBox(int, char *, char *);
BOOL InputBox(HWND, char *, char *, char *, int, int);
BOOL GenericMessage(HWND, char *, char *, int,
	int (*)(struct window *, MESSAGE, PARAM, PARAM),
	char *, char *, int, int, int);
#define TestErrorMessage(msg)	\
	GenericMessage(NULL, "Error", msg, 2, ErrorBoxProc,	  \
		tOk, tCancel, ID_OK, ID_CANCEL, TRUE)
#define ErrorMessage(msg) \
	GenericMessage(NULL, "Error", msg, 1, ErrorBoxProc,   \
		tOk, NULL, ID_OK, 0, TRUE)
#define MessageBox(ttl, msg) \
	GenericMessage(NULL, ttl, msg, 1, MessageBoxProc, \
		tOk, NULL, ID_OK, 0, TRUE)
#define YesNoBox(msg)	\
	GenericMessage(NULL, NULL, msg, 2, YesNoBoxProc,   \
		tYes, tNo, ID_OK, ID_CANCEL, TRUE)
#define CancelBox(wnd, msg) \
	GenericMessage(wnd, "Wait...", msg, 1, CancelBoxProc, \
		tCancel, NULL, ID_CANCEL, 0, FALSE)
void CloseCancelBox(void);
HWND MomentaryMessage(char *);


/* /////// COMMON DIALOGS ////////////////////////////// */


void ProgramAboutBox ( void );
void DFlatpAboutBox ( void );

BOOL OpenFileDialogBox(char *, char *);
BOOL SaveAsDialogBox(char *, char *, char *);

void DisplayProperties ( HWND );

void MoreWindows(HWND);


/* /////// CLIPBOARD  ////////////////////////////////// */

void CopyTextToClipboard(char *);
void CopyToClipboard(HWND);
#define PasteFromClipboard(wnd) PasteText(wnd,Clipboard,ClipboardLength)
BOOL PasteText(HWND, char *, unsigned);
void ClearClipboard(void);


/* /////// DFLAT+ HOOKS  /////////////////////////////// */


/* ------------- help system --------------------- */
BOOL SystemHelp(HWND w, char *topic);
BOOL InstallHelpProcedure ( BOOL (* f) () );

/* -------------- message logging system -------- */
BOOL LogMessageStart ( WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2 );
BOOL LogMessageEnd ( WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2 );



/* /////// WILL BE DEPRECATED (DO NOT USE!!) /////////// */


const char *WindowName(HWND wnd);
void SkipApplicationControls(void);
extern HWND inFocus;
extern HWND oldFocus;

#define CHANGECOLOR  19 /* prefix to change colors  (old: 174) */
#define RESETCOLOR   23 /* reset colors to default  (old: 175) */
#define LISTSELECTOR 4  /* selected list box entry */

extern char *Clipboard;
extern unsigned ClipboardLength;


/* /////// END  //////////////////////////////////////// */

/* ------------- DFlat+ Tools include ----------- */

enum dftools_messages { 
    /* ------------ Legacy HelpBox dialog box ---------- */
    ID_HELPTEXT = 5000,
    ID_BACK,
    ID_PREV,
    ID_NEXT };


/* ---- Tool: INTEGRATED HELP ------------------ */



void BuildFileName(char *path, const char *fn, const char *ext);

BOOL DisplayHelp(WINDOW, char *);
void UnLoadHelpFile(void);
void LoadHelpFile(char *);

/* To use it: include in your code:
    InstallHelpProcedure (DisplayHelp);
    LoadHelpFile( "file name containing the HELP");

    The needed HUFFC.EXE and FIXHELP.EXE are also built.
    
    See the FreeDOS EDIT sources to see how to create a basic help
    file and include it within your application.
*/


/* ---- Tool: GENERIC LOGGING ------------------ */

/* Within DFlat+ you can have logging routines within your code.
   You choose the routines you want linking to different libraries.
   
   Link to:
   
   DDNLOG*.LIB     - To create a non-logging, minimum size version.
   DDLOG*.LIB      - To use the legacy logging system
   DDLOGR*.LIB     - To create a DFlat+ Logger version
*/

/* Definitions specific to the legacy logging: */

void MessageLog(HWND);
/* Open a dialog on window referred by HWND, which lets you start
   logging and choose the messages you want to log */
   
/* Definitions specific to the DFlat+ logger */

/* Log levels for the DFLat+ new Logger tool */
typedef enum  {
    LL_CRITICAL, LL_ERROR, LL_WARN, LL_NOTIFY
}  log_levels;

extern log_levels CurrentLogLevel;  /* Current Logging level */
extern char logstr[];

int StartLogger(char *LogFileName, log_levels); /* start logging to this file (append)*/
int StopLogger(void); /* Stop logging */

/* Different logging routines, depending on the number of parameters you need */
void Log(log_levels, char *ModuleName, char *message);

#define Log1(level, module, message, p1) {  if ( sprintf(logstr,message,p1)>=0 ) Log (level, module, logstr );}
#define Log2(level, module, message, p1, p2)  { if ( sprintf(logstr,message,p1, p2)>=0 ) Log (level, module, logstr );}
#define Log3(level, module, message, p1, p2, p3)  { if ( sprintf(logstr,message,p1, p2, p3)>=0 ) Log (level, module, logstr );}
#define Log4(level, module, message, p1, p2, p3, p4) {  if ( sprintf(logstr,message,p1, p2, p3, p4)>=0 ) Log (level, module, logstr );}
#define Log5(level, module, message, p1, p2, p3, p4, p5) {  if ( sprintf(logstr,message,p1, p2, p3, p4, p5)>=0 ) Log (level, module, logstr );}

/* Text strings for classes and messages */
char *MessageText ( MESSAGE msg );
char *ClassText ( CLASS cls );


extern BOOL Debug_LogMessages;      /* Log messages automatically? */
extern BOOL Debug_LogClockMessages; /* Log clock messages too? */


/* --------- calendar.h ----------- */
    
void Calendar(WINDOW pwnd);

/* --------- helpbox.h ----------- */

/* --------- linked list of help text collections -------- */
struct helps {
    char *hname;
    char *comment;
    long hptr;
    int bit;
    int hheight;
    int hwidth;
    int nexthlp;
    int prevhlp;
    void *hwnd;
    char *PrevName;
    char *NextName;
#ifdef FIXHELP
    struct helps *NextHelp;
#endif
};

/* ------------------- htree.h -------------------- */

typedef unsigned int BYTECOUNTER;

/* ---- Huffman tree structure for building ---- */
struct htree    {
    BYTECOUNTER cnt;        /* character frequency         */
    int parent;             /* offset to parent node       */
    int right;              /* offset to right child node  */
    int left;               /* offset to left child node   */
};

/* ---- Huffman tree structure in compressed file ---- */
struct htr    {
    int right;              /* offset to right child node  */
    int left;               /* offset to left child node   */
};

extern struct htr *HelpTree;

void buildtree(void);
FILE *OpenHelpFile(const char *fn, const char *md);
void HelpFilePosition(long *, int *);
void *GetHelpLine(char *);
void SeekHelpLine(long, int);


/* --------------- system.h -------------- */

#define within(p,v1,v2)   ((p)>=(v1)&&(p)<=(v2))
#define RectTop(r)        (r.tp)
#define RectBottom(r)     (r.bt)
#define RectLeft(r)       (r.lf)
#define RectRight(r)      (r.rt)
#define InsideRect(x,y,r) (within((x),RectLeft(r),RectRight(r)) && within((y),RectTop(r),RectBottom(r)))
#define ValidRect(r)      (RectRight(r) || RectLeft(r) || RectTop(r) || RectBottom(r))
#define RectWidth(r)      (RectRight(r)-RectLeft(r)+1)
#define RectHeight(r)     (RectBottom(r)-RectTop(r)+1)
    
RECT subRectangle(RECT, RECT);

/* ----- interrupt vectors ----- */
#define TIMER  8
#define VIDEO  0x10
#define KEYBRD 0x16
#define DOS    0x21
#define CTRLBREAK 0x23
#define CRIT   0x24
#define MOUSE  0x33
#define KEYBOARDVECT 9
/* ------- platform-dependent values ------ */
#define KEYBOARDPORT 0x60
#define FREQUENCY 100
#define COUNT (1193280L / FREQUENCY)
#define ZEROFLAG 0x40
#define MAXSAVES 50
#define SCREENWIDTH  (getScreenWidth())
#define SCREENHEIGHT (getScreenHeight())

#define waitforkeyboard() {}
#define disable()
#define enable()
#define harderr(vect) {}
#define hardretn(err) {}

/* ----- keyboard BIOS (0x16) functions -------- */
#define READKB 0
#define KBSTAT 1
/* ------- video BIOS (0x10) functions --------- */
#define SETCURSORTYPE 1
#define SETCURSOR     2
#define READCURSOR    3
#define READATTRCHAR  8
#define WRITEATTRCHAR 9
#define HIDECURSOR 0x20
/* ---------- keyboard prototypes -------- */
int AltConvert(int);
int Xbioskey(int); /* enhanced for 102 key keyboard support */
int getkey(void);
int getshift(void);
BOOL keyhit(void);
void beep(void);
/* ---------- cursor prototypes -------- */
void curr_cursor(int *x, int *y);
void cursor(int x, int y);
void hidecursor(void);
void unhidecursor(void);
void savecursor(void);
void restorecursor(void);
void normalcursor(void);
void set_cursor_type(unsigned t);
void videomode(void);
void SwapCursorStack(void);
/* --------- screen prototpyes -------- */
BOOL init_console(int console_width, int console_height);
void clearscreen(void);
/* ---------- mouse prototypes ---------- */
BOOL mouse_installed(void);
int mousebuttons(void);
void get_mouseposition(int *x, int *y);
int button_releases(void);
void resetmouse(void);
#define leftbutton() (mousebuttons()&1)
#define rightbutton() (mousebuttons()&2)
#define waitformouse() while(mousebuttons());
#define set_mousetravel(p1, p2, s1, s2) {}
#define set_mouseposition(x, y) {}
#define show_mousecursor() {}
#define hide_mousecursor() {}

/* ------------ timer macros -------------- */

/* functions only use constants in timer variables  */
/* to identify which timer is used! So take care:   */
/* Never manipulate timer variables directly!   -ea */
int timed_out(int timer);
void set_timer(int timer, int secs);
void set_timer_ticks(int timer, int ticks);
void disable_timer(int timer);
int timer_running(int timer);
int timer_disabled(int timer);

#ifndef BLACK
/* ============= Color Macros ============ */
#define BLACK         0
#define BLUE          1
#define GREEN         2
#define CYAN          3
#define RED           4
#define MAGENTA       5
#define BROWN         6
#define LIGHTGRAY     7
#define DARKGRAY      8
#define LIGHTBLUE     9
#define LIGHTGREEN   10
#define LIGHTCYAN    11
#define LIGHTRED     12
#define LIGHTMAGENTA 13
#define YELLOW       14
#define WHITE        15
#endif

/* ------- the interrupt function registers -------- */
typedef struct REGS {
    int bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,fl;
} IREGS;

/* --------- screen prototpyes -------- */

extern unsigned video_mode; /* Call get_videomode before using that */

void clearscreen(void);
char *get_videomode(void); /* returns video address */
con_char_t GetVideoChar(int, int);
void PutVideoChar(int, int, con_char_t);

#define videochar(x,y) (GetVideoChar(x,y) & 255)
#define vad(x,y) (((y)*SCREENWIDTH+(x))*2)
#define vpeek(base, offs) (*(con_char_t*)(base+offs))
#define vpoke(base, offs, v) (*(con_char_t*)(base+offs) = v)
#define ismono() (video_mode == 7)
#define istext() (video_mode < 4)
#define clr(fg,bg) ((fg)|((bg)<<4))


/* ---------- file system ---------- */

/* ----- Create unambiguous path from file spec, filling in the
     drive and directory if incomplete. Optionally change to
     the new drive and subdirectory ------ */
void CreatePath(char *spath,char *fspec,int InclName,int Change);


/* ---------------- video.h ----------------- */

void getvideo(RECT, void *);
void storevideo(RECT, void *);


void wputch(WINDOW, int, int, int);
void wputs(WINDOW, void *, int, int);
void scroll_window(WINDOW, RECT, int);

/* /////// VARIABLES //////////////////////////////////// */

extern WINDOW ApplicationWindow;

extern WINDOW inFocus;
extern WINDOW oldFocus;

extern WINDOW CaptureMouse;
extern WINDOW CaptureKeyboard;

extern MODULE ProgramModule;
extern MODULE DFlatpModule;

extern char *Clipboard;
extern unsigned ClipboardLength;

extern BOOL ClipString;


/* /////// METHODS  ///////////////////////////////////// */


/* --------- system enhancements ---------- */
void SetScreenHeight(int);

/* --------- modules  --------------------- */
extern  char  VerStr [64];
#define MK_VER(a,b,c,d)   ((sprintf(VerStr,"%i.%i.%i.%i",a,b,c,d),VerStr))
#define ModuleVersion(m)  (MK_VER(m.Ver_maj,m.Ver_min,m.Ver_rel,m.Ver_patch))


/* --------- windows  --------------------- */
void InsertTitle(WINDOW, const char *);
void InitWindowColors(WINDOW);
const char *WindowName(WINDOW wnd);
void SkipApplicationControls(void);
void CreateStatusBar(WINDOW);


/* --------- window printing -------------- */
#define SwapVideoBuffer(wnd, ish, fh) swapvideo(wnd, wnd->videosave, ish, fh)
int LineLength(const char *);
int MsgHeight(char *);
int MsgWidth(char *);


/* --------- messages  -------------------- */
BOOL init_messages(void);
BOOL dispatch_message(void);


/* --------- menus  ------------------- */
int CopyCommand(char *, const char *, int, int);
void BuildSystemMenu(WINDOW);
BOOL isCascadedCommand(MBAR *,int);
BOOL isCascadedCommand2(MENU *, int);
void CreateMenu(WINDOW);


/* --------- other components ------------- */
void SetScrollBars(WINDOW);
#define HitControlBox(wnd, p1, p2) (TestAttribute(wnd, CONTROLBOX) && p1 == 2 && p2 == 0)


/* ---- standard window message processing prototypes ----- */
int ApplicationProc(WINDOW, MESSAGE, PARAM, PARAM);
int NormalProc(WINDOW, MESSAGE, PARAM, PARAM);
int TextBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int TextViewProc(WINDOW, MESSAGE, PARAM, PARAM);
int ListBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int GraphBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int LcdBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int EditBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int EditorProc(WINDOW, MESSAGE, PARAM, PARAM);
int PictureProc(WINDOW, MESSAGE, PARAM, PARAM);
int MenuBarProc(WINDOW, MESSAGE, PARAM, PARAM);
int PopDownProc(WINDOW, MESSAGE, PARAM, PARAM);
int ButtonProc(WINDOW, MESSAGE, PARAM, PARAM);
int ComboProc(WINDOW, MESSAGE, PARAM, PARAM);
int TextProc(WINDOW, MESSAGE, PARAM, PARAM);
int RadioButtonProc(WINDOW, MESSAGE, PARAM, PARAM);
int CheckBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int SpinButtonProc(WINDOW, MESSAGE, PARAM, PARAM);
int BoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int DialogProc(WINDOW, MESSAGE, PARAM, PARAM);
int SystemMenuProc(WINDOW, MESSAGE, PARAM, PARAM);
int HelpBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int MessageBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int CancelBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int ErrorBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int YesNoBoxProc(WINDOW, MESSAGE, PARAM, PARAM);
int StatusBarProc(WINDOW, MESSAGE, PARAM, PARAM);
int WatchIconProc(WINDOW, MESSAGE, PARAM, PARAM);

#ifdef __cplusplus
}
#endif

#endif /* TEXTUI_H */

