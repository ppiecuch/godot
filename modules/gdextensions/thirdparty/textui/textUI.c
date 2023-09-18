// kate: replace-tabs on; tab-indents on; tab-width 4; indent-width 4; indent-mode cstyle;

#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <limits.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
# include <alloca.h>
#endif

#define FIXHELP
#include "textUI.h"
#include "textUI_support.h"

#ifndef max
# define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#if defined(__APPLE__) || defined(__ANDROID__)
# define stricmp strcasecmp
#endif

#ifdef DEBUG_ENABLED
# define DEV_ASSERT(C) _dev_assert(C)
#else
# define DEV_ASSERT(C)
#endif

SysConfigType SysConfig = {
    {FALSE,255},             /* Current Color Scheme */
    {80,25,"No set"},        /* Resolution */
    ':',                     /* Country: Time separator */
    2,                       /* Editor Tab Size */
    FALSE                    /* Editor Global Read-only */
};

DEFDFLATP
    MOD_DESCRIPTION("DFlat+")
    MOD_VERSION(1,0,0,0)
    MOD_COPYRIGHT("KomSoft/Santamaria/Auer/Cosentino/Dr.DobbJ")
    MOD_LICENSE("GNU GPL 2.0")
    MOD_ABOUT("TextUI DFlat+ application|framework for the text console")
END_DEFMODULE

DEFPROGRAM
    MOD_DESCRIPTION("TextUI Interface 1.0")
    MOD_VERSION(1,0,0,0)
    MOD_COPYRIGHT("   Pawel Piecuch")
    MOD_LICENSE("GNU GPL 2.0")
    MOD_ABOUT("Portable General Purpose TextUI Interface")
END_DEFMODULE

/* http://en.wikipedia.org/wiki/Attrib */
#define _A_NORMAL   0x00    /* Normal file.     */
#define _A_RDONLY   0x01    /* Read only file.  */
#define _A_HIDDEN   0x02    /* Hidden file.     */
#define _A_SYSTEM   0x04    /* System file.     */
#define _A_SUBDIR   0x10    /* Subdirectory.    */
#define _A_ARCH     0x20    /* Archive file.    */

#define FINDDATA_NAME_SZ 260
typedef struct _finddata_t {
    unsigned attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    off_t size;
    char name[FINDDATA_NAME_SZ];
} _finddata_t;

/* Returns a unique search handle identifying the file or group of files matching the filespec specification, which can be used in
 * a subsequent call to findnext or to findclose. Otherwise, findfirst returns NULL and sets errno to EINVAL if filespec or fileinfo
 * was NULL or if the operating system returned an unexpected error and ENOENT if the file specification could not be matched. */
intptr_t _findfirst(const char* filespec, _finddata_t* fileinfo);

/* Find the next entry, if any, that matches the filespec argument of a previous call to findfirst, and then alter the fileinfo
 * structure contents accordingly. If successful, returns 0. Otherwise, returns -1 and sets errno to EINVAL if handle or fileinfo was NULL
 * or if the operating system returned an unexpected error and ENOENT if no more matching files could be found. */
int _findnext(intptr_t handle, _finddata_t* fileinfo);

/* Closes the specified search handle and releases associated resources. If successful, findclose returns 0. Otherwise, it
 * returns -1 and sets errno to ENOENT, indicating that no more matching files could be found. */
int _findclose(intptr_t handle);

/* Matches the regular language accepted by findfirst/findnext. More precisely, * matches zero or more characters and ? matches any
 * characters, but only one. Every other characters match themself. To respect the Windows behavior, *.* matches everything. */
int match_spec(const char* spec, const char* text);

/* ------------- applicat.c ------------- */

int ScreenHeight;
WINDOW ApplicationWindow = NULL;
extern DBOX Display;

static void DoWindowColors(WINDOW);

#ifdef INCLUDE_WINDOWOPTIONS
static void SelectTexture(void);
static void SelectBorder(WINDOW);
static void SelectTitle(WINDOW);
static void SelectStatusBar(WINDOW);
#endif

WINDOW oldFocus;
#ifdef INCLUDE_MULTI_WINDOWS
int WindowSel;
#endif

static char Cwd[65];



/* --------------- CREATE_WINDOW Message -------------- */
static int AppCreateWindowMsg(WINDOW wnd)
{
    int rtn;
    ApplicationWindow = wnd;
    ScreenHeight = SCREENHEIGHT;
    getcwd(Cwd, 64);

    DoWindowColors (wnd);

    if (SCREENHEIGHT != SysConfig.VideoCurrentResolution.VRes) {
        SetScreenHeight(SysConfig.VideoCurrentResolution.VRes);
        if (WindowHeight(wnd) == ScreenHeight || SCREENHEIGHT-1 < GetBottom(wnd)) {
            WindowHeight(wnd) = SCREENHEIGHT;
            GetBottom(wnd) = GetTop(wnd)+WindowHeight(wnd)-1;
            wnd->RestoredRC = WindowRect(wnd);
        }
    }

#ifdef INCLUDE_WINDOWOPTIONS
    SelectBorder(wnd);
    SelectTitle(wnd);
    SelectStatusBar(wnd);
#endif

    rtn = BaseWndProc(APPLICATION, wnd, CREATE_WINDOW, 0, 0);
    if (wnd->extension != NULL)
        CreateMenu(wnd);
    CreateStatusBar(wnd);
    SendMessage(NULL, SHOW_MOUSE, 0, 0);
    return rtn;
}

/* --------- ADDSTATUS Message ---------- */
static void AddStatusMsg(WINDOW wnd, PARAM p1)
{
    if (wnd->StatusBar != NULL)    {
        if (p1 && *(char *)p1)
            SendMessage(wnd->StatusBar, SETTEXT, p1, 0);
        else 
            SendMessage(wnd->StatusBar, CLEARTEXT, 0, 0);
        SendMessage(wnd->StatusBar, PAINT, 0, 0);
    }
}

/* -------- SETFOCUS Message -------- */
static void AppSetFocusMsg(WINDOW wnd, BOOL p1)
{
    if (p1)
        SendMessage(inFocus, SETFOCUS, FALSE, 0);
    inFocus = p1 ? wnd : NULL;
    SendMessage(NULL, HIDE_CURSOR, 0, 0);
    if (isVisible(wnd))
        SendMessage(wnd, BORDER, 0, 0);
    else
        SendMessage(wnd, SHOW_WINDOW, 0, 0);
}

/* ------- SIZE Message -------- */
static void AppSizeMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    BOOL WasVisible;
    WasVisible = isVisible(wnd);
    if (WasVisible)
        SendMessage(wnd, HIDE_WINDOW, 0, 0);
    if (p1-GetLeft(wnd) < 30)
        p1 = GetLeft(wnd) + 30;
    BaseWndProc(APPLICATION, wnd, SIZE, p1, p2);
    CreateMenu(wnd);
    CreateStatusBar(wnd);
    if (WasVisible)
        SendMessage(wnd, SHOW_WINDOW, 0, 0);
}

/* ----------- KEYBOARD Message ------------ */
static int AppKeyboardMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    if (WindowMoving || WindowSizing || (int) p1 == F1)
        return BaseWndProc(APPLICATION, wnd, KEYBOARD, p1, p2);
    switch ((int) p1)    {
        case ALT_F4:
            if (TestAttribute(wnd, CONTROLBOX))
                PostMessage(wnd, CLOSE_WINDOW, 0, 0);
            return TRUE;
#ifdef INCLUDE_MULTI_WINDOWS
        case ALT_F6: /* same effect as Alt-Tab in Win ;-) */
            SetNextFocus();
            return TRUE;
#endif
        case ALT_HYPHEN:
            if (TestAttribute(wnd, CONTROLBOX))
                BuildSystemMenu(wnd);
            return TRUE;
        default:
            break;
    }
    PostMessage(wnd->MenuBarWnd, KEYBOARD, p1, p2);
    return TRUE;
}

/* --------- SHIFT_CHANGED Message -------- */
static void AppShiftChangedMsg(WINDOW wnd, PARAM p1)
{
    extern BOOL AltDown;
    if ((int)p1 & ALTKEY)
        AltDown = TRUE;
    else if (AltDown)    {
        AltDown = FALSE;
        if (wnd->MenuBarWnd != inFocus)
            SendMessage(NULL, HIDE_CURSOR, 0, 0);
        SendMessage(wnd->MenuBarWnd, KEYBOARD, F10, 0);
    }
}

/* -------- COMMAND Message ------- */
static void AppCommandMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    switch ((int)p1)    {
        case ID_SYSCLOSE:
            PostMessage(wnd, CLOSE_WINDOW, 0, 0);
            break;
#ifdef INCLUDE_RESTORE
        case ID_SYSRESTORE:
#endif
        case ID_SYSMOVE:
        case ID_SYSSIZE:
#ifdef INCLUDE_MINIMIZE
        case ID_SYSMINIMIZE:
#endif
#ifdef INCLUDE_MAXIMIZE
        case ID_SYSMAXIMIZE:
#endif
            BaseWndProc(APPLICATION, wnd, COMMAND, p1, p2);
            break;
        default:
            if (inFocus != wnd->MenuBarWnd && inFocus != wnd)
                PostMessage(inFocus, COMMAND, p1, p2);
            break;
    }
}

/* --------- CLOSE_WINDOW Message -------- */
static int AppCloseWindowMsg(WINDOW wnd)
{
    int rtn;
#ifdef INCLUDE_MULTI_WINDOWS
    CloseAll(wnd, TRUE);
    WindowSel = 0;
#endif
    PostMessage(NULL, STOP, 0, 0);
    rtn = BaseWndProc(APPLICATION, wnd, CLOSE_WINDOW, 0, 0);
    if (ScreenHeight != SCREENHEIGHT)
        SetScreenHeight(ScreenHeight);
    ApplicationWindow = NULL;
    setdisk(toupper(*Cwd) - 'A');
    chdir(Cwd+2);
    return rtn;
}

/* --- APPLICATION Window Class window processing module --- */
int ApplicationProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            return AppCreateWindowMsg(wnd);
        case HIDE_WINDOW:
            if (wnd == inFocus)
                inFocus = NULL;
            break;
        case ADDSTATUS:
            AddStatusMsg(wnd, p1);
            return TRUE;
        case SETFOCUS:
            if ((int)p1 == (inFocus != wnd))    {
                AppSetFocusMsg(wnd, (BOOL) p1);
                return TRUE;
            }
            break;
        case SIZE:
            AppSizeMsg(wnd, p1, p2);
            return TRUE;
#ifdef INCLUDE_MINIMIZE
        case MINIMIZE:
            return TRUE;
#endif
        case KEYBOARD:
            return AppKeyboardMsg(wnd, p1, p2);
        case SHIFT_CHANGED:
            AppShiftChangedMsg(wnd, p1);
            return TRUE;
        case PAINT:
            if (isVisible(wnd)) {
                int cl = APPLCHAR;
                ClearWindow(wnd, (RECT *)p1, cl);
            }
            return TRUE;
        case COMMAND:
            AppCommandMsg(wnd, p1, p2);
            return TRUE;
        case CLOSE_WINDOW:
            return AppCloseWindowMsg(wnd);
        default:
            break;
    }
    return BaseWndProc(APPLICATION, wnd, msg, p1, p2);
}

static void SwitchCursor(void)
{
    SendMessage(NULL, SAVE_CURSOR, 0, 0);
    SwapCursorStack();
    SendMessage(NULL, RESTORE_CURSOR, 0, 0);
}


/* -------- Create the menu bar -------- */
void CreateMenu(WINDOW wnd)
{
    AddAttribute(wnd, HASMENUBAR);
    if (wnd->MenuBarWnd != NULL)
        SendMessage(wnd->MenuBarWnd, CLOSE_WINDOW, 0, 0);
    wnd->MenuBarWnd = CreateWindow(MENUBAR,
                        NULL,
                        GetClientLeft(wnd),
                        GetClientTop(wnd)-1,
                        1,
                        ClientWidth(wnd),
                        NULL,
                        wnd,
                        NULL,
                        0);
    SendMessage(wnd->MenuBarWnd,BUILDMENU,(PARAM)wnd->extension,0);
    AddAttribute(wnd->MenuBarWnd, VISIBLE);
}

/* ----------- Create the status bar ------------- */
void CreateStatusBar(WINDOW wnd)
{
    if (wnd->StatusBar != NULL)    {
        SendMessage(wnd->StatusBar, CLOSE_WINDOW, 0, 0);
        wnd->StatusBar = NULL;
    }
    if (TestAttribute(wnd, HASSTATUSBAR))    {
        wnd->StatusBar = CreateWindow(STATUSBAR,
                                      NULL,
                                      GetClientLeft(wnd),
                                      GetBottom(wnd),
                                      1,
                                      ClientWidth(wnd),
                                      NULL,
                                      wnd,
                                      NULL,
                                      0);
        AddAttribute(wnd->StatusBar, VISIBLE);
    }
}

#ifdef INCLUDE_MULTI_WINDOWS
/* -------- return the name of a document window ------- */
const char *WindowName(WINDOW wnd)
{
    if (GetTitle(wnd) == NULL)    {
        if (GetClass(wnd) == DIALOG)
            return ((DBOX *)(wnd->extension))->HelpName;
        else 
            return "Untitled";
    }
    else
        return GetTitle(wnd);
}




/* ----- user chose a window from the Window menu OR the More Window dialog box ----- */
void ChooseWindow(WINDOW wnd, int WindowNo)
{
#if CLASSIC_WINDOW_NUMBERING
    WINDOW cwnd = FirstWindow(wnd);
#else
    WINDOW cwnd = NumberOneChildWindow(wnd);
#endif
    while (cwnd != NULL) {
        if (isVisible(cwnd) && GetClass(cwnd) != MENUBAR && GetClass(cwnd) != STATUSBAR)
            if (WindowNo-- == 0)
                break;
#if CLASSIC_WINDOW_NUMBERING
        cwnd = NextWindow(cwnd);
#else
        cwnd = NextNumberedWindow(cwnd);
#endif
    }
    if (cwnd != NULL)    {
        SendMessage(cwnd, SETFOCUS, TRUE, 0);
        if (cwnd->condition == ISMINIMIZED)
            SendMessage(cwnd, RESTORE, 0, 0);
    }
}

/* ----- Close all document windows ----- */
void CloseAll(WINDOW wnd, int closing)
{
    WINDOW wnd1, wnd2;
    SendMessage(wnd, SETFOCUS, TRUE, 0);
    wnd1 = LastWindow(wnd);
    while (wnd1 != NULL)	{
        wnd2 = PrevWindow(wnd1);
        if (isVisible(wnd1) && GetClass(wnd1) != MENUBAR && GetClass(wnd1) != STATUSBAR) {
            ClearVisible(wnd1);
            SendMessage(wnd1, CLOSE_WINDOW, 0, 0);
        }
        wnd1 = wnd2;
    }
    if (!closing)
        SendMessage(wnd, PAINT, 0, 0);
}

#endif /* #ifdef INCLUDE_MULTI_WINDOWS */

static void DoWindowColors(WINDOW wnd)
{
    WINDOW cwnd;
    InitWindowColors(wnd);
    cwnd = FirstWindow(wnd);
    while (cwnd != NULL) {
        DoWindowColors(cwnd);
        if (GetClass(cwnd) == TEXT && GetText(cwnd) != NULL)
            SendMessage(cwnd, CLEARTEXT, 0, 0);
        cwnd = NextWindow(cwnd);
    }
}

/* ---- select screen lines ---- */
void SelectLines(VideoResolution  reqVR)
{
    SysConfig.VideoCurrentResolution = reqVR;
    if (SCREENHEIGHT != reqVR.VRes)    {

        SetScreenHeight(reqVR.VRes);

        /* ---- re-maximize ---- */
        if (ApplicationWindow->condition == ISMAXIMIZED)	{
           SendMessage(ApplicationWindow, SIZE, (PARAM) GetRight(ApplicationWindow), SCREENHEIGHT-1);
                return;
            }
            /* --- adjust if current size does not fit --- */
            if (WindowHeight(ApplicationWindow) > SCREENHEIGHT)
                SendMessage(ApplicationWindow, SIZE, (PARAM) GetRight(ApplicationWindow),(PARAM) GetTop(ApplicationWindow)+SCREENHEIGHT-1);
            /* --- if window is off-screen, move it on-screen --- */
            if (GetTop(ApplicationWindow) >= SCREENHEIGHT-1)
                    SendMessage(ApplicationWindow, MOVE, (PARAM) GetLeft(ApplicationWindow),(PARAM) SCREENHEIGHT-WindowHeight(ApplicationWindow));
      }
}

BOOL SelectColorScheme (ColorScheme cs)
{
    memcpy(&SysConfig.VideoCurrentColorScheme, &cs, sizeof (ColorScheme));
    if (ApplicationWindow != NULL)
        DoWindowColors(ApplicationWindow);
    return TRUE;
}


#ifdef INCLUDE_WINDOWOPTIONS

/* ----- select the screen texture ----- */
static void SelectTexture(void)
{
    cfg.Texture = CheckBoxSetting(&Display, ID_TEXTURE);
}

/* -- select whether the application screen has a border -- */
static void SelectBorder(WINDOW wnd)
{
    cfg.Border = CheckBoxSetting(&Display, ID_BORDER);
    if (cfg.Border)
        AddAttribute(wnd, HASBORDER);
    else
        ClearAttribute(wnd, HASBORDER);
}

/* select whether the application screen has a status bar */
static void SelectStatusBar(WINDOW wnd)
{
    cfg.StatusBar = CheckBoxSetting(&Display, ID_STATUSBAR);
    if (cfg.StatusBar)
        AddAttribute(wnd, HASSTATUSBAR);
    else
        ClearAttribute(wnd, HASSTATUSBAR);
}

/* select whether the application screen has a title bar */
static void SelectTitle(WINDOW wnd)
{
    cfg.Title = CheckBoxSetting(&Display, ID_TITLE);
    if (cfg.Title)
        AddAttribute(wnd, HASTITLEBAR);
    else
        ClearAttribute(wnd, HASTITLEBAR);
}


#endif

/* ----------- box.c ------------ */

int BoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    CTLWINDOW *ct = GetControl(wnd);
    if (ct != NULL)    {
        switch (msg)    {
            case SETFOCUS:
            case PAINT:
                return FALSE;
            case LEFT_BUTTON:
            case BUTTON_RELEASED:
                return SendMessage(GetParent(wnd), msg, p1, p2);
            case BORDER:
                rtn = BaseWndProc(BOX, wnd, msg, p1, p2);
                if (ct != NULL && ct->itext != NULL)
                    writeline(wnd, ct->itext, 1, 0, FALSE);
                return rtn;
            default:
                break;
        }
    }
    return BaseWndProc(BOX, wnd, msg, p1, p2);
}

/* -------------- button.c -------------- */

void ButtonPaintMsg(WINDOW wnd, CTLWINDOW *ct, RECT *rc)
{
    if (isVisible(wnd))    {
        if (TestAttribute(wnd, SHADOW) && !SysConfig.VideoCurrentColorScheme.isMonoScheme) {
            /* -------- draw the button's shadow ------- */
            int x;
            background = WndBackground(GetParent(wnd));
            foreground = BLACK;
            for (x = 1; x <= WindowWidth(wnd); x++)
                wputch(wnd, 223, x, 1);
            wputch(wnd, 220, WindowWidth(wnd), 0);
        }
        if (ct->itext != NULL)    {
            char *txt = DFcalloc(1, strlen(ct->itext)+10);
            if (ct->setting == OFF)    {
                txt[0] = CHANGECOLOR;
                txt[1] = wnd->WindowColors[HILITE_COLOR] [FG] | 0x80;
                txt[2] = wnd->WindowColors[STD_COLOR] [BG] | 0x80;
            }
            CopyCommand(txt+strlen(txt),ct->itext,!ct->setting,WndBackground(wnd));
            SendMessage(wnd, CLEARTEXT, 0, 0);
            SendMessage(wnd, ADDTEXT, (PARAM) txt, 0);
            free(txt);
        }
        /* --------- write the button's text ------- */
        WriteTextLine(wnd, rc, 0, wnd == inFocus);
    }
}

void ButtonLeftButtonMsg(WINDOW wnd, MESSAGE msg, CTLWINDOW *ct)
{
    if (!SysConfig.VideoCurrentColorScheme.isMonoScheme)    {
        /* --------- draw a pushed button -------- */
        background = WndBackground(GetParent(wnd));
        foreground = WndBackground(wnd);
        wputch(wnd, ' ', 0, 0);
        for (int x = 0; x < WindowWidth(wnd); x++)    {
            wputch(wnd, 220, x+1, 0);
            wputch(wnd, 223, x+1, 1);
        }
    }
    if (msg == LEFT_BUTTON)
        SendMessage(NULL, WAITMOUSE, 0, 0);
    else
        SendMessage(NULL, WAITKEYBOARD, 0, 0);
    SendMessage(wnd, PAINT, 0, 0);
    if (ct->setting == ON)
        PostMessage(GetParent(wnd), COMMAND, ct->command, 0);
    else
        beep();
}

int ButtonProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    CTLWINDOW *ct = GetControl(wnd);

    if (ct != NULL)    {
        switch (msg)    {
            case SETFOCUS:
                BaseWndProc(BUTTON, wnd, msg, p1, p2);
                p1 = 0;
                /* ------- fall through ------- */
            case PAINT:
                ButtonPaintMsg(wnd, ct, (RECT*)p1);
                return TRUE;
            case KEYBOARD:
                if ((p1 == LARROW) || (p1 == RARROW)) { /* DFlat+ 1.0: allow buttons to pass arrows to parent*/
                    PostMessage ( GetParent(wnd), msg, p1, p2);
                    return TRUE;
                }
                if (p1 != '\r')
                    break;
                /* ---- fall through ---- */
            case LEFT_BUTTON:
                ButtonLeftButtonMsg(wnd, msg, ct);
                return TRUE;
            case HORIZSCROLL:
                return TRUE;
            default:
                break;
        }
    }
    return BaseWndProc(BUTTON, wnd, msg, p1, p2);
}
/* -------------- checkbox.c ------------ */

int CheckBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    CTLWINDOW *ct = GetControl(wnd);
    if (ct != NULL)    {
        switch (msg)    {
            case SETFOCUS:
                if (!(int)p1)
                    SendMessage(NULL, HIDE_CURSOR, 0, 0);
            case MOVE:
                rtn = BaseWndProc(CHECKBOX, wnd, msg, p1, p2);
                SetFocusCursor(wnd);
                return rtn;
            case PAINT:    {
                char cb[] = "[ ]";
                if (ct->setting)
                    cb[1] = 'X';
                SendMessage(wnd, CLEARTEXT, 0, 0);
                SendMessage(wnd, ADDTEXT, (PARAM) cb, 0);
                SetFocusCursor(wnd);
                break;
            }
            case KEYBOARD:
                if ((int)p1 != ' ')
                    break;
            case LEFT_BUTTON:
                ct->setting ^= ON;
                SendMessage(wnd, PAINT, 0, 0);
                return TRUE;
            default:
                break;
        }
    }
    return BaseWndProc(CHECKBOX, wnd, msg, p1, p2);
}

BOOL CheckBoxSetting(DBOX *db, UCOMMAND cmd)
{
    CTLWINDOW *ct = FindCommand(db, cmd, CHECKBOX);
    return ct ? (ct->wnd ? (ct->setting==ON) : (ct->isetting==ON)) : FALSE;
}

/* ----------- clipbord.c ------------ */

char *Clipboard;
unsigned ClipboardLength;

void CopyTextToClipboard(char *text)
{
    ClipboardLength = strlen(text);
    Clipboard = DFrealloc(Clipboard, ClipboardLength);
    memmove(Clipboard, text, ClipboardLength);
}

void CopyToClipboard(WINDOW wnd)
{
    if (TextBlockMarked(wnd))    {
        char *bb = TextBlockBegin(wnd);
        char *be = TextBlockEnd(wnd);
        if (bb >= be) { /* *** 0.6e extra check *** */
            bb = TextBlockEnd(wnd); /* sic! */
            be = TextBlockBegin(wnd); /* sic! */
        }
        ClipboardLength = (unsigned) (be - bb); /* *** unsigned *** */
        Clipboard = DFrealloc(Clipboard, ClipboardLength);
        memmove(Clipboard, bb, ClipboardLength);
    }
}

void ClearClipboard(void)
{
    if (Clipboard != NULL)  {
        free(Clipboard);
        Clipboard = NULL;
    }
}


BOOL PasteText(WINDOW wnd, char *SaveTo, unsigned len)
{
    if (SaveTo != NULL && len > 0)    {
        unsigned plen = strlen(wnd->text) + len;

        if (plen <= wnd->MaxTextLength) {
            if (plen+1 > wnd->textlen) {
                wnd->text = DFrealloc(wnd->text, plen+3);
                wnd->textlen = plen+1;
            }
            memmove(CurrChar+len, CurrChar, strlen(CurrChar)+1);
            memmove(CurrChar, SaveTo, len);
            BuildTextPointers(wnd);
            wnd->TextChanged = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

/* -------------- combobox.c -------------- */

int ListProc(WINDOW, MESSAGE, PARAM, PARAM);

int ComboProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            wnd->extension = CreateWindow(LISTBOX,
                                          NULL,
                                          wnd->rc.lf,wnd->rc.tp+1,
                                          wnd->ht-1, wnd->wd+1,
                                          NULL,
                                          wnd,
                                          ListProc,
                                          HASBORDER | NOCLIP | SAVESELF);
            ((WINDOW)(wnd->extension))->ct->command = wnd->ct->command;
            wnd->ht = 1;
            wnd->rc.bt = wnd->rc.tp;
            break;
        case PAINT:
            foreground = WndBackground(wnd);
            background = WndForeground(wnd);
            wputch(wnd, DOWNSCROLLBOX, WindowWidth(wnd), 0);
            break;
        case KEYBOARD:
            if ((int)p1 == DN)    {
                SendMessage(wnd->extension, SETFOCUS, TRUE, 0);
                return TRUE;
            }
            break;
        case LEFT_BUTTON:
            if ((int)p1 == GetRight(wnd) + 1)
                SendMessage(wnd->extension, SETFOCUS, TRUE, 0);
            break;
        case CLOSE_WINDOW:
            SendMessage(wnd->extension, CLOSE_WINDOW, 0, 0);
            break;
        default:
            break;
    }
    return BaseWndProc(COMBOBOX, wnd, msg, p1, p2);
}

int ListProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    WINDOW pwnd = GetParent(GetParent(wnd));
    DBOX *db = pwnd->extension;
    WINDOW cwnd = ControlWindow(db, wnd->ct->command);
    char text[130];
    int rtn;
    WINDOW currFocus;
    switch (msg)    {
        case CREATE_WINDOW:
            wnd->ct = DFmalloc(sizeof(CTLWINDOW));
            wnd->ct->setting = OFF;
            wnd->WindowColors[FRAME_COLOR][FG] = wnd->WindowColors[STD_COLOR][FG];
            wnd->WindowColors[FRAME_COLOR][BG] = wnd->WindowColors[STD_COLOR][BG];
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            return rtn;
        case SETFOCUS:
            if ((int)p1 == FALSE) {
                if (!wnd->isHelping) {
                    SendMessage(wnd, HIDE_WINDOW, 0, 0);
                    wnd->ct->setting = OFF;
                }
            } else
                wnd->ct->setting = ON;
            break;
        case SHOW_WINDOW:
            if (wnd->ct->setting == OFF)
                return TRUE;
            break;
        case BORDER:
            currFocus = inFocus;
            inFocus = NULL;
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            inFocus = currFocus;
            return rtn;
        case LB_SELECTION:
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            SendMessage(wnd, LB_GETTEXT, (PARAM) text, wnd->selection);
            PutItemText(pwnd, wnd->ct->command, text);
            SendMessage(cwnd, PAINT, 0, 0);
            cwnd->TextChanged = TRUE;
            return rtn;
        case KEYBOARD:
            switch ((int) p1) {
                case ESC:
#ifdef HOOKKEYB
                case FWD: /* right arrow */
#else
                case RARROW: /* formerly called FWD */
#endif
                case BS:
                    SendMessage(cwnd, SETFOCUS, TRUE, 0);
                    return TRUE;
                default:
                    break;
            }
            break;
        case LB_CHOOSE:
            SendMessage(cwnd, SETFOCUS, TRUE, 0);
            return TRUE;
        case CLOSE_WINDOW:
            if (wnd->ct != NULL)
                free(wnd->ct);
            wnd->ct = NULL;
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

void PutComboListText(WINDOW wnd, UCOMMAND cmd, char *text)
{
    CTLWINDOW *ct = FindCommand(wnd->extension, cmd, COMBOBOX);
    if (ct != NULL)        {
        WINDOW lwnd = ((WINDOW)(ct->wnd))->extension;
        SendMessage(lwnd, ADDTEXT, (PARAM) text, 0);
    }
}

/*  Common Dialogs

    Part of FreeDOS DFlat+

*/

/* G L O B A L S ///////////////////////////////////////////////////////// */

void *mypointer;
static char FileSpec[15], SrchSpec[15], FileName[15];
extern DBOX FileOpen, SaveAs;
extern DBOX Display;
#ifdef INCLUDE_MULTI_WINDOWS
extern DBOX Windows;
#endif
extern int WindowSel;

char  VerStr [64];



/* P R O T O T Y P E S /////////////////////////////////////////////////// */

static BOOL DlgFileOpen(char *, char *, char *, DBOX *);
static int DlgFnOpen(WINDOW, MESSAGE, PARAM, PARAM);
static void InitFnOpenDlgBox(WINDOW);
#ifdef STRIPPATH
static void StripPath(char *);
#endif
static BOOL IncompleteFilename(char *);
BOOL BuildFileList(WINDOW, char *);
void BuildDirectoryList(WINDOW);
void BuildDriveList(WINDOW);
void BuildPathDisplay(WINDOW);
void SelectColors( void );

/* F U N C T I O N S ///////////////////////////////////////////////////// */

BOOL OpenFileDialogBox(char *Fspec, char *Fname)
{
    return DlgFileOpen(Fspec, Fspec, Fname, &FileOpen);
}

/* Save as Dialog Box */
BOOL SaveAsDialogBox(char *Fspec, char *Sspec, char *Fname)
{
    return DlgFileOpen(Fspec, Sspec ? Sspec : Fspec, Fname, &SaveAs);
}

/* Generic File Open */
static BOOL DlgFileOpen(char *Fspec, char *Sspec, char *Fname, DBOX *db)
{
    BOOL rtn;

    strncpy(FileSpec, Fspec, 15);
    strncpy(SrchSpec, Sspec, 15);
    if ((rtn = DialogBox(NULL, db, TRUE, DlgFnOpen)) != FALSE)
        strcpy(Fname, FileName);
    return rtn;

}

/* Process dialog box messages */
static int DlgFnOpen(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    switch (msg)
        {
        case CREATE_WINDOW:
            {
            int rtn = DefaultWndProc(wnd, msg, p1, p2);
            DBOX *db = wnd->extension;
            WINDOW cwnd = ControlWindow(db, ID_FILENAME);
            SendMessage(cwnd, SETTEXTLENGTH, 64, 0);
            return rtn;
            }
        case INITIATE_DIALOG:
            InitFnOpenDlgBox(wnd);
            break;
        case COMMAND:
            switch ((int) p1)
                {
                case ID_OK:
                    {
                    if ((int)p2 == 0)
                        {
                        char fn[MAXPATH+1], nm[MAXFILE], ext[MAXEXT];

                        GetItemText(wnd, ID_FILENAME, fn, MAXPATH);
                        fnsplit(fn, NULL, NULL, nm, ext);
                        strcpy(FileName, nm);
                        strcat(FileName, ext);
                        CreatePath(NULL, fn, FALSE, TRUE);
                        if (IncompleteFilename(FileName))
                            {
                            /* --- no file name yet --- */
                            DBOX *db = wnd->extension;
                            WINDOW cwnd = ControlWindow(db, ID_FILENAME);

                            strcpy(FileSpec, FileName);
                            strcpy(SrchSpec, FileName);
                            InitFnOpenDlgBox(wnd);
                            SendMessage(cwnd, SETFOCUS, TRUE, 0);
                            return TRUE;
                            }
                        }
                    break;
                    }
                case ID_FILES:
                    switch ((int) p2)
                        {
                        case ENTERFOCUS:
                        case LB_CHILDSELECTION:
                            /* Selected a different filename */
                            GetDlgListText(wnd, FileName, ID_FILES);
                            PutItemText(wnd, ID_FILENAME, FileName);
                            break;
                        case LB_CHOOSE:
                            /* Choose a file name */
                            GetDlgListText(wnd, FileName, ID_FILES);
                            SendMessage(wnd, COMMAND, ID_OK, 0);
                            break;
                        default:
                            break;

                        }
                    return TRUE;
                case ID_DIRECTORY:
                    switch ((int) p2)
                        {
                        case ENTERFOCUS:
                            PutItemText(wnd, ID_FILENAME, FileSpec);
                            break;
                        case LB_CHOOSE:
                            {
                            char dd[15]; /* Choose dir */
                            GetDlgListText(wnd, dd, ID_DIRECTORY);
                            chdir(dd);
                            InitFnOpenDlgBox(wnd);
                            SendMessage(wnd, COMMAND, ID_OK, 0);
                            break;
                        }

                        default:
                            break;

                        }
                    return TRUE;

                case ID_DRIVE:
                    switch ((int) p2)
                        {
                        case ENTERFOCUS:
                            PutItemText(wnd, ID_FILENAME, FileSpec);
                            break;
                        case LB_CHOOSE:
                            {
                            char dr[15]; /* Choose dir */
                            GetDlgListText(wnd, dr, ID_DRIVE);
                            /* *** 0.6e: string has form "[-X-]" *** */
                            setdisk(dr[2] - 'A'); /* fixed 0.6e: must use [2] */
                            InitFnOpenDlgBox(wnd);
                            SendMessage(wnd, COMMAND, ID_OK, 0);
                        }
                        default:
                            break;

                        }
                    return TRUE;

                default:
                    break;
                }
        default:
            break;

        }

    return DefaultWndProc(wnd, msg, p1, p2);
}

/* Initialize the dialog box */
static void InitFnOpenDlgBox(WINDOW wnd)
{
    if (*FileSpec)
        PutItemText(wnd, ID_FILENAME, FileSpec);

    BuildPathDisplay(wnd);
    if (BuildFileList(wnd, SrchSpec))
        BuildDirectoryList(wnd);

    BuildDriveList(wnd);
}

/* Strip the drive and path information from a file spec */
#ifdef STRIPPATH /* normally not used... */
static void StripPath(char *filespec)
{
    char *cp = strchr(filespec, ':');
    if (cp != NULL)
        cp++;
    else
        cp = filespec;

    while (TRUE)
        {
        char *cp1 = strchr(cp, '\\');
        if (cp1 == NULL)
            break;

        cp = cp1+1;
        }

    strcpy(filespec, cp);

}
#endif

static BOOL IncompleteFilename(char *s)
{
    int lc = strlen(s)-1;

    if (strchr(s, '?') || strchr(s, '*') || !*s)
        return TRUE;

    if (*(s+lc) == ':' || *(s+lc) == '\\')
        return TRUE;

    return FALSE;
}


void AboutBox ( MODULE m, BOOL BasedOnDFlat )
{
    char maxAbout[]= "                                      ";
    char aboutMsg [] =
             "          @                        \n"
             "          Version @                \n"
             "          @                        \n"
             "@                                  \n"
             "                                   \n"
             "@                                  \n"
             "@                                  \n"
             "@                                   ";

    char AboutStr[255];
    char *About1, *About2;

    strncpy (AboutStr, m.AboutComment, 255);
    About1 = strtok (AboutStr, "|");
    About2 = strtok (NULL, "@");

    strncpy (maxAbout, m.Description, 22); maxAbout[23] = 0;
    strncpy(strchr(aboutMsg,'@'), maxAbout,  strlen(maxAbout));

    strncpy (maxAbout, ModuleVersion(m), 14); maxAbout[15] = 0;
    strncpy(strchr(aboutMsg,'@'), maxAbout,  strlen(maxAbout));

    strncpy (maxAbout, m.License, 22); maxAbout[23] = 0;
    strncpy(strchr(aboutMsg,'@'), maxAbout,  strlen(maxAbout));

    strncpy (maxAbout, m.Copyright, 34); maxAbout[35] = 0;
    strncpy(strchr(aboutMsg,'@'), maxAbout,  strlen(maxAbout));

    strncpy (maxAbout, About1, 34); maxAbout[35] = 0;
    strncpy(strchr(aboutMsg,'@'), maxAbout,  strlen(maxAbout));

    strncpy (maxAbout, About2, 34); maxAbout[35] = 0;
    strncpy(strchr(aboutMsg,'@'), maxAbout,  strlen(maxAbout));

    // strncpy (maxAbout, SysConfig.DFlatpVersion, 4); maxAbout[5] = 0;
    // strncpy(strchr(aboutMsg,'@'), maxAbout,  strlen(maxAbout));

    if (BasedOnDFlat)
        sprintf (AboutStr, "Based on FreeDOS-DFlat+ %i.%i",DFlatpModule.Ver_maj,DFlatpModule.Ver_min);
    else
        strcpy (AboutStr, " ");

    strncpy (maxAbout, AboutStr, 33); maxAbout[34] = 0;
    strncpy(strchr(aboutMsg,'@'), maxAbout,  strlen(maxAbout));

    sprintf (AboutStr, "About %s", m.Description);

    MessageBox(AboutStr, aboutMsg);
}

void ProgramAboutBox ( void )
{
    AboutBox ( ProgramModule, TRUE );
}

void DFlatpAboutBox ( void )
{
    AboutBox ( DFlatpModule, FALSE );
}

/* Display Properties Box */
void DisplayProperties ( WINDOW wnd )
{
#ifdef INCLUDE_WINDOWOPTIONS
    if (cfg.Border)
        SetCheckBox(&Display, ID_BORDER);
    if (cfg.Title)
        SetCheckBox(&Display, ID_TITLE);
    if (cfg.StatusBar)
        SetCheckBox(&Display, ID_STATUSBAR);
    if (cfg.Texture)
        SetCheckBox(&Display, ID_TEXTURE);
#endif

    /* Prepare the dialog box items */

    switch (SysConfig.VideoCurrentColorScheme.index)   {
        case 1: PushRadioButton(&Display, ID_MONO); break;
        case 2: PushRadioButton(&Display, ID_REVERSE); break;
        default: PushRadioButton(&Display, ID_COLOR);
    }
    
    /* Execute the dialog box */

    if (DialogBox(wnd, &Display, TRUE, NULL)) {
        if (inFocus == wnd->MenuBarWnd || inFocus == wnd->StatusBar)
            oldFocus = ApplicationWindow;
        else
            oldFocus = inFocus;

        SendMessage(wnd, HIDE_WINDOW, 0, 0);
        SelectColors();

#ifdef INCLUDE_WINDOWOPTIONS
        SelectBorder(wnd);
        SelectTitle(wnd);
        SelectStatusBar(wnd);
        SelectTexture();
#endif

        CreateMenu(wnd);
        CreateStatusBar(wnd);
        SendMessage(wnd, SHOW_WINDOW, 0, 0);
        SendMessage(oldFocus, SETFOCUS, TRUE, 0);
    }
}

/* ----- set up colors for the application window ------ */
void SelectColors( void )
{
    get_videomode();
    if (RadioButtonSetting(&Display, ID_MONO))
        SelectColorScheme (bw);
    else if (RadioButtonSetting(&Display, ID_REVERSE))
        SelectColorScheme (reverse);
    else if (ismono() || video_mode == 2)
        SelectColorScheme (bw);
    else
        SelectColorScheme (color);
}



/* window processing module for the More Windows dialog box */
/* used if you have more than 9 windows open, lists ALL windows */
static int WindowPrep(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    switch (msg)    {
        case INITIATE_DIALOG:    {
            WINDOW wnd1;
            WINDOW cwnd = ControlWindow(&Windows,ID_WINDOWLIST);
            int sel = 0;
            if ((cwnd == NULL) || (ApplicationWindow == NULL))
                return FALSE;
#if CLASSIC_WINDOW_NUMBERING
            wnd1 = FirstWindow(ApplicationWindow);
#else
            wnd1 = NumberOneChildWindow(ApplicationWindow);
#endif
            while (wnd1 != NULL) {
                if (isVisible(wnd1) && wnd1 != wnd &&
                    GetClass(wnd1) != MENUBAR &&
                    GetClass(wnd1) != STATUSBAR)    {
                    if (wnd1 == oldFocus)
                        WindowSel = sel;
                    SendMessage(cwnd, ADDTEXT, (PARAM) WindowName(wnd1), 0);
                    sel++;
                }
#if CLASSIC_WINDOW_NUMBERING
                wnd1 = NextWindow(wnd1);
#else
                wnd1 = NextNumberedWindow(wnd1);
#endif
            }
            SendMessage(cwnd, LB_SETSELECTION, WindowSel, 0);
            AddAttribute(cwnd, VSCROLLBAR);
            PostMessage(cwnd, SHOW_WINDOW, 0, 0);
            break;
        }
        case COMMAND:
            switch ((int) p1)    {
                case ID_OK:
                    if ((int)p2 == 0)
                        WindowSel = SendMessage(
                                    ControlWindow(&Windows,
                                    ID_WINDOWLIST),
                                    LB_CURRENTSELECTION, 0, 0);
                    break;
                case ID_WINDOWLIST:
                    if ((int) p2 == LB_CHOOSE)
                        SendMessage(wnd, COMMAND, ID_OK, 0);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

/* ---- the More Windows command on the Window menu ---- */
void MoreWindows(WINDOW wnd)
{
    if (DialogBox(wnd, &Windows, TRUE, WindowPrep))
        ChooseWindow(wnd, WindowSel);
}

/* ----------- console.c ---------- */

/* ----- table of alt keys for finding shortcut keys ----- */
static int altconvert[] = {
    ALT_A,ALT_B,ALT_C,ALT_D,ALT_E,ALT_F,ALT_G,ALT_H,
    ALT_I,ALT_J,ALT_K,ALT_L,ALT_M,ALT_N,ALT_O,ALT_P,
    ALT_Q,ALT_R,ALT_S,ALT_T,ALT_U,ALT_V,ALT_W,ALT_X,
    ALT_Y,ALT_Z,ALT_0,ALT_1,ALT_2,ALT_3,ALT_4,ALT_5,
    ALT_6,ALT_7,ALT_8,ALT_9
};

unsigned video_mode;

static int cursorpos[MAXSAVES];
static int cursorshape[MAXSAVES];
static int cs;

/* ------------- clear the screen -------------- */
void clearscreen(void)
{
    cursor(0, 0);
}

void SwapCursorStack(void)
{
    if (cs > 1) {
        swapi(cursorpos[cs-2], cursorpos[cs-1]);
        swapi(cursorshape[cs-2], cursorshape[cs-1]);
    }
}

/* ---- Test for keystroke ---- */
BOOL keyhit(void)
{
    return system_keyhit();
}

/* ---- Read a keystroke ---- */
int getkey(void)
{
    return system_getkey();
}

/* ---------- read the keyboard shift status --------- */
int getshift(void)
{
    return system_getshift();
}

/* ------- macro to wait one clock tick -------- */
#define wait() { volatile int now = *clk; while (now == *clk) ; }

/* -------- sound a buzz tone, using hardware directly ---------- */
void beep(void)
{
}

/* -------- get the video mode and page from BIOS -------- */
void videomode(void)
{
    video_mode = -1;
}

/* ------ position the cursor ------ */
void cursor(int x, int y)
{
    videomode();
    if (y >= SCREENHEIGHT) y = SCREENHEIGHT - 1; /* 0.7c */
}

/* ------ get cursor shape and position ------ */
static void getcursor(void)
{
    videomode();
}

/* ------- get the current cursor position ------- */
void curr_cursor(int *x, int *y)
{
    getcursor();
}

/* ------ save the current cursor configuration ------ */
void savecursor(void)
{
    if (cs < MAXSAVES)    {
        getcursor();
        cursorshape[cs] = 0;
        cursorpos[cs] = 0;
        cs++;
    }
}

/* ---- restore the saved cursor configuration ---- */
void restorecursor(void)
{
    if (cs) {
        --cs;
        videomode();
        set_cursor_type(cursorshape[cs]);
    }
}

/* ------ make a normal cursor ------ */
void normalcursor(void)
{
    set_cursor_type(0x0106);
}

/* ------ hide the cursor ------ */
void hidecursor(void)
{
    getcursor();
}

/* ------ unhide the cursor ------ */
void unhidecursor(void)
{
    getcursor();
}

/* ---- use BIOS to set the cursor type ---- */
void set_cursor_type(unsigned t)
{
    videomode();
}

/* ------ convert an Alt+ key to its letter equivalent ----- */
int AltConvert(int c)
{
    int i, a = 0;
    for (i = 0; i < 36; i++)
        if (c == altconvert[i])
            break;
    if (i < 26)
        a = 'a' + i;
    else if (i < 36)
        a = '0' + i - 26;
    return a;
}

/* ------------------- decomp.c -------------------- */

/*
 * Decompress the application.HLP file
 * or load the application.TXT file if the .HLP file
 * does not exist
 */

static int in8;
static int ct8 = 8;
static FILE *fi;
static BYTECOUNTER bytectr;
struct htr *HelpTree;
static int root;


void BuildFileName(char *fn, const char *fname, const char *ext)
{
    strcpy(fn, fname);
    strcat(fn, ext);
}

/* ------- open the help database file -------- */
FILE *OpenHelpFile(const char *fn, const char *md)
{
    /* char *cp; */
    int treect;
    char helpname[65];

    /* -------- get the name of the help file ---------- */
    BuildFileName(helpname, fn, ".hlp");
    if ((fi = fopen(helpname, md)) == NULL)
        return NULL;
    if (HelpTree == NULL) {
        /* ----- read the byte count ------ */
        fread(&bytectr, sizeof bytectr, 1, fi);
        /* ----- read the frequency count ------ */
        fread(&treect, sizeof treect, 1, fi);
        /* ----- read the root offset ------ */
        fread(&root, sizeof root, 1, fi);
        HelpTree = calloc(treect-256, sizeof(struct htr));
        if (HelpTree != NULL)	{
            /* ---- read in the tree --- */
            for (int i = 0; i < treect-256; i++)    {
                fread(&HelpTree[i].left,  sizeof(int), 1, fi);
                fread(&HelpTree[i].right, sizeof(int), 1, fi);
            }
        }
    }
    return fi;
}

/* ----- read a line of text from the help database ----- */
void *GetHelpLine(char *line)
{
    int h;
    *line = '\0';
    while (TRUE)    {
        /* ----- decompress a line from the file ------ */
        h = root;
        /* ----- walk the Huffman tree ----- */
        while (h > 255)    {
            /* --- h is a node pointer --- */
            if (ct8 == 8)   {
                /* --- read 8 bits of compressed data --- */
                if ((in8 = fgetc(fi)) == EOF)    {
                    *line = '\0';
                    return NULL;
                }
                ct8 = 0;
            }
            /* -- point to left or right node based on msb -- */
            if (in8 & 0x80)
                h = HelpTree[h-256].left;
            else
                h = HelpTree[h-256].right;
            /* --- shift the next bit in --- */
            in8 <<= 1;
            ct8++;
        }
        /* --- h < 255 = decompressed character --- */
        if (h == '\r')
            continue;    /* skip the '\r' character */
        /* --- put the character in the buffer --- */
        *line++ = h;
        /* --- if '\n', end of line --- */
        if (h == '\n')
            break;
    }
    *line = '\0';    /* null-terminate the line */
    return line;
}

/* --- compute the database file byte and bit position --- */
void HelpFilePosition(long *offset, int *bit)
{
    *offset = ftell(fi);
    if (ct8 < 8)
        --*offset;
    *bit = ct8;
}

/* -- position the database to the specified byte and bit -- */
void SeekHelpLine(long offset, int bit)
{
    int fs = fseek(fi, offset, 0);
    DEV_ASSERT(fs == 0);
    ct8 = bit;
    if (ct8 < 8)    {
        in8 = fgetc(fi);
        in8 <<= bit;
    }
}

/* ---------- dfalloc.c ---------- */

static void AllocationError(void)
{
    static BOOL OnceIn = FALSE;
    static char *ErrMsg[] = {
        "0xda, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xbf",
        "0xb3 Out of Memory! 0xb3",
        "0xc0, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xd9"
    };
    char savbuf[108];
    RECT rc = {30,11,47,13};

    if (!OnceIn)	{
        OnceIn = TRUE;
        /* ------ close all windows ------ */
        SendMessage(ApplicationWindow, CLOSE_WINDOW, 0, 0);
        getvideo(rc, savbuf);
        for (int x = 0; x < 18; x++)	{
            for (int y = 0; y < 3; y++)		{
                int c = (255 & (*(*(ErrMsg+y)+x))) | 0x7000;
                PutVideoChar(x+rc.lf, y+rc.tp, c);
            }
        }
        getkey();
        storevideo(rc, savbuf);
    }
}

void *DFcalloc(size_t nitems, size_t size)
{
    void *rtn = calloc(nitems, size);
    if (size && rtn == NULL)
        AllocationError();
    return rtn;
}

void *DFmalloc(size_t size)
{
    void *rtn = malloc(size);
    if (size && rtn == NULL)
        AllocationError();
    return rtn;
}

void *DFrealloc(void *block, size_t size)
{
    void *rtn = (block == NULL)
        ? malloc(size) /* *** new in 0.6e *** */
        : realloc(block, size);
    if (size && rtn == NULL)
        AllocationError();
    return rtn;
}
/* ----------------- dialbox.c -------------- */

static PARAM inFocusCommand(DBOX *);
static BOOL dbShortcutKeys(DBOX *, int);
static int ControlProc(WINDOW, MESSAGE, PARAM, PARAM);
static void FirstFocus(DBOX *db);
static void NextFocus(DBOX *db);
static void PrevFocus(DBOX *db);
static CTLWINDOW *AssociatedControl(DBOX *, UCOMMAND);

static BOOL SysMenuOpen;

static DBOX **dbs = NULL;
static int dbct = 0;

/* --- clear all heap allocations to control text fields --- */
void ClearDialogBoxes(void)
{
    for (int i = 0; i < dbct; i++)    {
        CTLWINDOW *ct = (*(dbs+i))->ctl;
        while (ct->cls)    {
            if ((ct->cls == EDITBOX || ct->cls == TEXTBOX || ct->cls == COMBOBOX) && ct->itext != NULL) {
                free(ct->itext);
                ct->itext = NULL;
            }
            ct++;
        }
    }
    if (dbs != NULL) {
        free(dbs);
        dbs = NULL;
    }
    dbct = 0;
}

/* -------- CREATE_WINDOW Message --------- */
static int DialogCreateWindowMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    DBOX *db = wnd->extension;
    CTLWINDOW *ct = db->ctl;
    WINDOW cwnd;
    int rtn, i;
    /* ---- build a table of processed dialog boxes ---- */
    for (i = 0; i < dbct; i++)
        if (db == dbs[i])
            break;
    if (i == dbct)    {
        dbs = DFrealloc(dbs, sizeof(DBOX *) * (dbct+1));
        *(dbs + dbct++) = db;
    }
    rtn = BaseWndProc(DIALOG, wnd, CREATE_WINDOW, p1, p2);
    ct = db->ctl;
    while (ct->cls)    {
        int attrib = 0;
        if (TestAttribute(wnd, NOCLIP))
            attrib |= NOCLIP;
        if (wnd->Modal)
            attrib |= SAVESELF;
        ct->setting = ct->isetting;
        if (ct->cls == EDITBOX && ct->dwnd.h > 1)
            attrib |= (MULTILINE | HASBORDER);
        else if ((ct->cls == LISTBOX || ct->cls == TEXTBOX) && ct->dwnd.h > 2)
            attrib |= HASBORDER;
        cwnd = CreateWindow(ct->cls,
                        ct->dwnd.title,
                        ct->dwnd.x+GetClientLeft(wnd),
                        ct->dwnd.y+GetClientTop(wnd),
                        ct->dwnd.h,
                        ct->dwnd.w,
                        ct,
                        wnd,
                        ControlProc,
                        attrib);
        if ((ct->cls == EDITBOX || ct->cls == TEXTBOX || ct->cls == COMBOBOX) && ct->itext != NULL)
            SendMessage(cwnd, SETTEXT, (PARAM) ct->itext, 0);
        ct++;
    }
    return rtn;
}

/* -------- LEFT_BUTTON Message --------- */
static BOOL DialogLeftButtonMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    DBOX *db = wnd->extension;
    CTLWINDOW *ct = db->ctl;
    if (WindowSizing || WindowMoving)
        return TRUE;
    if (HitControlBox(wnd, p1-GetLeft(wnd), p2-GetTop(wnd))) {
        PostMessage(wnd, KEYBOARD, ' ', ALTKEY);
        return TRUE;
    }
    while (ct->cls) {
        WINDOW cwnd = ct->wnd;
        if (ct->cls == COMBOBOX) {
            if (p2 == GetTop(cwnd)) {
                if (p1 == GetRight(cwnd)+1) {
                    SendMessage(cwnd, LEFT_BUTTON, p1, p2);
                    return TRUE;
                }
            }
            if (GetClass(inFocus) == LISTBOX)
                SendMessage(wnd, SETFOCUS, TRUE, 0);
        } else if (ct->cls == SPINBUTTON) {
            if (p2 == GetTop(cwnd))    {
                if (p1 == GetRight(cwnd)+1 ||
                        p1 == GetRight(cwnd)+2)    {
                    SendMessage(cwnd, LEFT_BUTTON, p1, p2);
                    return TRUE;
                }
            }
        }
        ct++;
    }
    return FALSE;
}

/* -------- KEYBOARD Message --------- */
static BOOL DialogKeyboardMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    DBOX *db = wnd->extension;
    CTLWINDOW *ct;

    if (WindowMoving || WindowSizing)
        return FALSE;
    switch ((int)p1)    {
        case F1:
            ct = GetControl(inFocus);
            if (ct != NULL)
                if (SystemHelp(wnd, ct->help))
                    return TRUE;
            break;
        case SHIFT_HT:
        case BS:
#ifndef HOOKKEYB
        case LARROW: /* hope this makes sense */
#endif
        case UP:
            PrevFocus(db);
            break;
        case ALT_F6:
        case '\t':
#ifdef HOOKKEYB
        case FWD: /* right arrow */
#else
        case RARROW: /* formerly called FWD */
#endif
        case DN:
            NextFocus(db);
            break;
        case ' ':
            if (((int)p2 & ALTKEY) &&
                    TestAttribute(wnd, CONTROLBOX))    {
                SysMenuOpen = TRUE;
                BuildSystemMenu(wnd);
                return TRUE;
            }
            break;
        case CTRL_F4:
        case ESC:
            SendMessage(wnd, COMMAND, ID_CANCEL, 0);
            break;
        default:
            /* ------ search all the shortcut keys ----- */
            if (dbShortcutKeys(db, (int) p1))
                return TRUE;
            break;
    }
    return wnd->Modal;
}

/* -------- COMMAND Message --------- */
static BOOL DialogCommandMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    DBOX *db = wnd->extension;
    switch ((int) p1)    {
        case ID_OK:
        case ID_CANCEL:
            if ((int)p2 != 0)
                return TRUE;
            wnd->ReturnCode = (int) p1;
            if (wnd->Modal)
                PostMessage(wnd, ENDDIALOG, 0, 0);
            else
                SendMessage(wnd, CLOSE_WINDOW, TRUE, 0);
            return TRUE;
        case ID_HELP:
            if ((int)p2 != 0)
                return TRUE;
            return SystemHelp(wnd, db->HelpName);
        default:
            break;
    }
    return FALSE;
}

/* ----- window-processing module, DIALOG window class ----- */
int DialogProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    DBOX *db = wnd->extension;

    switch (msg)    {
        case CREATE_WINDOW:
            return DialogCreateWindowMsg(wnd, p1, p2);
        case SHIFT_CHANGED:
            if (wnd->Modal)
                return TRUE;
            break;
        case LEFT_BUTTON:
            if (DialogLeftButtonMsg(wnd, p1, p2))
                return TRUE;
            break;
        case KEYBOARD:
            if (DialogKeyboardMsg(wnd, p1, p2))
                return TRUE;
            break;
        case CLOSE_POPDOWN:
            SysMenuOpen = FALSE;
            break;
        case LB_SELECTION:
        case LB_CHILDSELECTION:
        case LB_CHOOSE:
            if (SysMenuOpen)
                return TRUE;
            SendMessage(wnd, COMMAND, inFocusCommand(db), msg);
            break;
        case SETFOCUS:
            if ((int)p1 && wnd->dfocus != NULL && isVisible(wnd))
                return SendMessage(wnd->dfocus, SETFOCUS, TRUE, 0);
            break;
        case COMMAND:
            if (DialogCommandMsg(wnd, p1, p2))
                return TRUE;
            break;
        case PAINT:
            p2 = TRUE;
            break;
        case MOVE:
        case SIZE:
            rtn = BaseWndProc(DIALOG, wnd, msg, p1, p2);
            if (wnd->dfocus != NULL && isVisible(wnd))
                SendMessage(wnd->dfocus, SETFOCUS, TRUE, 0);
            return rtn;
        case CLOSE_WINDOW:
            if (!p1)    {
                SendMessage(wnd, COMMAND, ID_CANCEL, 0);
                return TRUE;
            }
            break;
        default:
            break;
    }
    return BaseWndProc(DIALOG, wnd, msg, p1, p2);
}

/* ------- create and execute a dialog box ---------- */
BOOL DialogBox(WINDOW wnd, DBOX *db, BOOL Modal,
  int (*wndproc)(struct window *, MESSAGE, PARAM, PARAM))
{
    BOOL rtn = FALSE;
    int x = db->dwnd.x, y = db->dwnd.y;
    WINDOW DialogWnd;

    DialogWnd = CreateWindow(DIALOG,
                        db->dwnd.title,
                        x, y,
                        db->dwnd.h,
                        db->dwnd.w,
                        db,
                        wnd,
                        wndproc,
                        (Modal ? SAVESELF : 0));
    SendMessage(DialogWnd, SETFOCUS, TRUE, 0);
    DialogWnd->Modal = Modal;
    FirstFocus(db);
    PostMessage(DialogWnd, INITIATE_DIALOG, 0, 0);
    if (Modal)    {
        SendMessage(DialogWnd, CAPTURE_MOUSE, 0, 0);
        SendMessage(DialogWnd, CAPTURE_KEYBOARD, 0, 0);
        while (dispatch_message())
            ;
        rtn = DialogWnd->ReturnCode == ID_OK;
        SendMessage(DialogWnd, RELEASE_MOUSE, 0, 0);
        SendMessage(DialogWnd, RELEASE_KEYBOARD, 0, 0);
        SendMessage(DialogWnd, CLOSE_WINDOW, TRUE, 0);
    }
    return rtn;
}

/* ----- return command code of in-focus control window ---- */
static PARAM inFocusCommand(DBOX *db)
{
    CTLWINDOW *ct = db->ctl;
    while (ct->cls)    {
        if (ct->wnd == inFocus)
            return ct->command;
        ct++;
    }
    return -1;
}

/* -------- find a specified control structure ------- */
CTLWINDOW *FindCommand(DBOX *db, UCOMMAND cmd, int cls)
{
    CTLWINDOW *ct = db->ctl;
    while (ct->cls)    {
        if (cls == -1 || ct->cls == cls)
            if (cmd == ct->command)
                return ct;
        ct++;
    }
    return NULL;
}

/* ---- return the window handle of a specified command ---- */
WINDOW ControlWindow(const DBOX *db, UCOMMAND cmd)
{
    const CTLWINDOW *ct = db->ctl;
    while (ct->cls)    {
        if (ct->cls != TEXT && cmd == ct->command)
            return ct->wnd;
        ct++;
    }
    return NULL;
}

/* --- return a pointer to the control structure that matches a window --- */
CTLWINDOW *WindowControl(DBOX *db, WINDOW wnd)
{
    CTLWINDOW *ct = db->ctl;
    while (ct->cls)    {
        if (ct->wnd == wnd)
            return ct;
        ct++;
    }
    return NULL;
}

/* ---- set a control ON or OFF ----- */
void ControlSetting(DBOX *db, UCOMMAND cmd,
                                int cls, int setting)
{
    CTLWINDOW *ct = FindCommand(db, cmd, cls);
    if (ct != NULL) {
        ct->isetting = setting;
        if (ct->wnd != NULL)
            ct->setting = setting;
    }
}

/* ----- test if a control is on or off ----- */
BOOL isControlOn(DBOX *db, UCOMMAND cmd, int cls)
{
    const CTLWINDOW *ct = FindCommand(db, cmd, cls);
    return ct ? (ct->wnd ? ct->setting : ct->isetting) : FALSE;
}

/* ---- return pointer to the text of a control window ---- */
char *GetDlgTextString(DBOX *db, UCOMMAND cmd, CLASS cls)
{
    CTLWINDOW *ct = FindCommand(db, cmd, cls);
    if (ct != NULL)
        return ct->itext;
    else
        return NULL;
}

/* ------- set the text of a control specification ------ */
void SetDlgTextString(DBOX *db, UCOMMAND cmd, char *text, CLASS cls)
{
    CTLWINDOW *ct = FindCommand(db, cmd, cls);
    if (ct != NULL) {
        if (text != NULL) {
            if (ct->cls == TEXT)
                ct->itext = text;  /* text may not go out of scope */
            else {
                ct->itext = DFrealloc(ct->itext, strlen(text)+1);
                strcpy(ct->itext, text);
            }
        } else {
            if (ct->cls == TEXT)
                ct->itext = "";
            else 	{
                free(ct->itext);
                ct->itext = NULL;
            }
        }
        if (ct->wnd != NULL)	{
            if (text != NULL)
                SendMessage(ct->wnd, SETTEXT, (PARAM) text, 0);
            else
                SendMessage(ct->wnd, CLEARTEXT, 0, 0);
            SendMessage(ct->wnd, PAINT, 0, 0);
        }
    }
}

/* ------- set the text of a control window ------ */
void PutItemText(WINDOW wnd, UCOMMAND cmd, char *text)
{
    CTLWINDOW *ct = FindCommand(wnd->extension, cmd, EDITBOX);

    if (ct == NULL)
        ct = FindCommand(wnd->extension, cmd, TEXTBOX);
    if (ct == NULL)
        ct = FindCommand(wnd->extension, cmd, COMBOBOX);
    if (ct == NULL)
        ct = FindCommand(wnd->extension, cmd, LISTBOX);
    if (ct == NULL)
        ct = FindCommand(wnd->extension, cmd, SPINBUTTON);
    if (ct == NULL)
        ct = FindCommand(wnd->extension, cmd, TEXT);
    if (ct != NULL)        {
        WINDOW cwnd = (WINDOW) (ct->wnd);
        switch (ct->cls)    {
            case COMBOBOX:
            case EDITBOX:
                SendMessage(cwnd, CLEARTEXT, 0, 0);
                SendMessage(cwnd, ADDTEXT, (PARAM) text, 0);
                if (!isMultiLine(cwnd))
                    SendMessage(cwnd, PAINT, 0, 0);
                break;
            case LISTBOX:
            case TEXTBOX:
            case SPINBUTTON:
                SendMessage(cwnd, ADDTEXT, (PARAM) text, 0);
                break;
            case TEXT:    {
                SendMessage(cwnd, CLEARTEXT, 0, 0);
                SendMessage(cwnd, ADDTEXT, (PARAM) text, 0);
                SendMessage(cwnd, PAINT, 0, 0);
                break;
            }
            default:
                break;
        }
    }
}

/* ------- get the text of a control window ------ */
void GetItemText(WINDOW wnd, UCOMMAND cmd,
                                char *text, int len)
{
    CTLWINDOW *ct = FindCommand(wnd->extension, cmd, EDITBOX);
    char *cp;

    if (ct == NULL)
        ct = FindCommand(wnd->extension, cmd, COMBOBOX);
    if (ct == NULL)
        ct = FindCommand(wnd->extension, cmd, TEXTBOX);
    if (ct == NULL)
        ct = FindCommand(wnd->extension, cmd, TEXT);
    if (ct != NULL)    {
        WINDOW cwnd = (WINDOW) (ct->wnd);
        if (cwnd != NULL)    {
            switch (ct->cls)    {
                case TEXT:
                    if (GetText(cwnd) != NULL)    {
                        cp = strchr(GetText(cwnd), '\n');
                        if (cp != NULL)
                            len = (int) (cp - GetText(cwnd));
                        strncpy(text, GetText(cwnd), len);
                        *(text+len) = '\0';
                    }
                    break;
                case TEXTBOX:
                    if (GetText(cwnd) != NULL)
                        strncpy(text, GetText(cwnd), len);
                    break;
                case COMBOBOX:
                case EDITBOX:
                    SendMessage(cwnd,GETTEXT,(PARAM)text,len);
                    break;
                default:
                    break;
            }
        }
    }
}

/* ------- set the text of a listbox control window ------ */
void GetDlgListText(WINDOW wnd, char *text, UCOMMAND cmd)
{
    CTLWINDOW *ct = FindCommand(wnd->extension, cmd, LISTBOX);
    int sel = SendMessage(ct->wnd, LB_CURRENTSELECTION, 0, 0);
    SendMessage(ct->wnd, LB_GETTEXT, (PARAM) text, sel);
}

/* -- find control structure associated with text control -- */
static CTLWINDOW *AssociatedControl(DBOX *db,UCOMMAND Tcmd)
{
    CTLWINDOW *ct = db->ctl;
    while (ct->cls)    {
        if (ct->cls != TEXT)
            if (ct->command == Tcmd)
                break;
        ct++;
    }
    return ct;
}

/* --- process dialog box shortcut keys --- */
static BOOL dbShortcutKeys(DBOX *db, int ky)
{
    CTLWINDOW *ct;
    int ch = AltConvert(ky);

    if (ch != 0)    {
        ct = db->ctl;
        while (ct->cls)    {
            char *cp = ct->itext;
            while (cp && *cp)    {
                if (*cp == SHORTCUTCHAR && tolower(*(cp+1)) == ch)    {
                    if (ct->cls == TEXT)
                        ct = AssociatedControl(db, ct->command);
                    if (ct->cls == RADIOBUTTON)
                        SetRadioButton(db, ct);
                    else if (ct->cls == CHECKBOX)    {
                        ct->setting ^= ON;
                        SendMessage(ct->wnd, PAINT, 0, 0);
                    }
                    else if (ct->cls)    {
                        SendMessage(ct->wnd, SETFOCUS, TRUE, 0);
                        if (ct->cls == BUTTON)
                           SendMessage(ct->wnd,KEYBOARD,'\r',0);
                    }
                    return TRUE;
                }
                cp++;
            }
            ct++;
        }
    }
    return FALSE;
}

/* --- dynamically add or remove scroll bars
                            from a control window ---- */
void SetScrollBars(WINDOW wnd)
{
    int oldattr = GetAttribute(wnd);
    if (wnd->wlines > ClientHeight(wnd))
        AddAttribute(wnd, VSCROLLBAR);
    else 
        ClearAttribute(wnd, VSCROLLBAR);
    if (wnd->textwidth > ClientWidth(wnd))
        AddAttribute(wnd, HSCROLLBAR);
    else 
        ClearAttribute(wnd, HSCROLLBAR);
    if (GetAttribute(wnd) != oldattr)
        SendMessage(wnd, BORDER, 0, 0);
}

/* ------- CREATE_WINDOW Message (Control) ----- */
static void CtlCreateWindowMsg(WINDOW wnd)
{
    CTLWINDOW *ct;
    ct = wnd->ct = wnd->extension;
    wnd->extension = NULL;
    if (ct != NULL)
        ct->wnd = wnd;
}

/* ------- KEYBOARD Message (Control) ----- */
static BOOL CtlKeyboardMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    CTLWINDOW *ct = GetControl(wnd);
    switch ((int) p1)    {
        case F1:
            if (WindowMoving || WindowSizing)
                break;
            if (!SystemHelp(wnd, ct->help))
                SendMessage(GetParent(wnd),COMMAND,ID_HELP,0);
            return TRUE;
        case ' ':
            if (!((int)p2 & ALTKEY))
                break;
        case ALT_F6:
        case CTRL_F4:
        case ALT_F4:
            PostMessage(GetParent(wnd), KEYBOARD, p1, p2);
            return TRUE;
        default:
            break;
    }
    if (GetClass(wnd) == EDITBOX)
        if (isMultiLine(wnd))
            return FALSE;
    if (GetClass(wnd) == TEXTBOX)
        if (WindowHeight(wnd) > 1)
            return FALSE;
    switch ((int) p1)    {
#if 0 /* ??? */
        case UP:
            if (!isDerivedFrom(wnd, LISTBOX)) {
                p1 = CTRL_FIVE; /* ??? */
                p2 = LEFTSHIFT;
            }
            break;
#endif
#if 0 /* ??? */
        case BS:
            if (!isDerivedFrom(wnd, EDITBOX)) {
                p1 = CTRL_FIVE; /* ??? */
                p2 = LEFTSHIFT;
            }
            break;
#endif
        case DN:
            if (!isDerivedFrom(wnd, LISTBOX) &&
                    !isDerivedFrom(wnd, COMBOBOX))
                p1 = '\t';
            break;
#ifdef HOOKKEYB
        case FWD: /* right arrow */
#else
        case RARROW: /* formerly called FWD */
#endif
            if (!isDerivedFrom(wnd, EDITBOX))
                p1 = '\t';
            break;
        case '\r':
            if (isDerivedFrom(wnd, EDITBOX))
                if (isMultiLine(wnd))
                    break;
            if (isDerivedFrom(wnd, BUTTON))
                break;
            if (isDerivedFrom(wnd, LISTBOX))
                break;
            SendMessage(GetParent(wnd), COMMAND, ID_OK, 0);
            return TRUE;
        default:
            break;
    }
    return FALSE;
}

/* ------- CLOSE_WINDOW Message (Control) ----- */
static void CtlCloseWindowMsg(WINDOW wnd)
{
    CTLWINDOW *ct = GetControl(wnd);
    if (ct != NULL)    {
        ct->wnd = NULL;
        if (GetParent(wnd)->ReturnCode == ID_OK) {
            if (ct->cls == EDITBOX || ct->cls == COMBOBOX) {
                ct->itext=DFrealloc(ct->itext,strlen(wnd->text)+1);
                strcpy(ct->itext, wnd->text);
                if (!isMultiLine(wnd)) {
                    char *cp = ct->itext+strlen(ct->itext)-1;
                    if (*cp == '\n')
                        *cp = '\0';
                }
            } else if (ct->cls == RADIOBUTTON || ct->cls == CHECKBOX)
                ct->isetting = ct->setting;
        }
    }
}

static void FixColors(WINDOW wnd)
{
    CTLWINDOW *ct = wnd->ct;
    if (ct->cls != BUTTON)	{
        if (ct->cls != SPINBUTTON && ct->cls != COMBOBOX) {
            if (ct->cls != EDITBOX && ct->cls != LISTBOX) {
                wnd->WindowColors[FRAME_COLOR][FG] = GetParent(wnd)->WindowColors[FRAME_COLOR][FG];
                wnd->WindowColors[FRAME_COLOR][BG] = GetParent(wnd)->WindowColors[FRAME_COLOR][BG];
                wnd->WindowColors[STD_COLOR][FG] = GetParent(wnd)->WindowColors[STD_COLOR][FG];
                wnd->WindowColors[STD_COLOR][BG] = GetParent(wnd)->WindowColors[STD_COLOR][BG];
            }
        }
    }
}

/* -- generic window processor used by dialog box controls -- */
static int ControlProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    DBOX *db;

    if (wnd == NULL)
        return FALSE;
    db = GetParent(wnd) ? GetParent(wnd)->extension : NULL;

    switch (msg)    {
        case CREATE_WINDOW:
            CtlCreateWindowMsg(wnd);
            break;
        case KEYBOARD:
            if (CtlKeyboardMsg(wnd, p1, p2))
                return TRUE;
            break;
        case PAINT:
            FixColors(wnd);
            if (GetClass(wnd) == EDITBOX || GetClass(wnd) == LISTBOX || GetClass(wnd) == TEXTBOX)
                SetScrollBars(wnd);
            break;
        case BORDER:
            FixColors(wnd);
            if (GetClass(wnd) == EDITBOX)    {
                WINDOW oldFocus = inFocus;
                inFocus = NULL;
                DefaultWndProc(wnd, msg, p1, p2);
                inFocus = oldFocus;
                return TRUE;
            }
            break;
        case SETFOCUS:	{
            WINDOW pwnd = GetParent(wnd);

            if (p1)    {
                WINDOW oldFocus = inFocus;
                if (pwnd && GetClass(oldFocus) != APPLICATION && !isAncestor(inFocus, pwnd))	{
                    inFocus = NULL;
                    SendMessage(oldFocus, BORDER, 0, 0);
                    SendMessage(pwnd, SHOW_WINDOW, 0, 0);
                    inFocus = oldFocus;
                    ClearVisible(oldFocus);
                }
                if (GetClass(oldFocus) == APPLICATION && NextWindow(pwnd) != NULL)
                    pwnd->wasCleared = FALSE;
                DefaultWndProc(wnd, msg, p1, p2);
                SetVisible(oldFocus);
                if (pwnd != NULL) {
                    pwnd->dfocus = wnd;
                    SendMessage(pwnd, COMMAND, inFocusCommand(db), ENTERFOCUS);
                }
                return TRUE;
            } else
                SendMessage(pwnd, COMMAND, inFocusCommand(db), LEAVEFOCUS);
            break;
        }
        case CLOSE_WINDOW:
            CtlCloseWindowMsg(wnd);
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

/* ---- change the focus to the first control --- */
static void FirstFocus(DBOX *db)
{
    CTLWINDOW *ct = db->ctl;
    if (ct != NULL)	{
        while (ct->cls == TEXT || ct->cls == BOX) {
            ct++;
            if (ct->cls == 0)
                return;
        }
        SendMessage(ct->wnd, SETFOCUS, TRUE, 0);
    }
}

/* ---- change the focus to the next control --- */
static void NextFocus(DBOX *db)
{
    CTLWINDOW *ct = WindowControl(db, inFocus);
    int looped = 0;

    if (ct != NULL) {
        do {
            ct++;
            if (ct->cls == 0) {
                if (looped)
                    return;
                looped++;
                ct = db->ctl;
            }
        } while (ct->cls == TEXT || ct->cls == BOX);
        SendMessage(ct->wnd, SETFOCUS, TRUE, 0);
    }
}

/* ---- change the focus to the previous control --- */
static void PrevFocus(DBOX *db)
{
    CTLWINDOW *ct = WindowControl(db, inFocus);
    int looped = 0;

    if (ct != NULL)	{
        do {
            if (ct == db->ctl) {
                if (looped)
                    return;
                looped++;
                while (ct->cls)
                    ct++;
            }
            --ct;
        } while (ct->cls == TEXT || ct->cls == BOX);
        SendMessage(ct->wnd, SETFOCUS, TRUE, 0);
    }
}

void SetFocusCursor(WINDOW wnd)
{
    if (wnd == inFocus)    {
        SendMessage(NULL, SHOW_CURSOR, 0, 0);
        SendMessage(wnd, KEYBOARD_CURSOR, 1, 0);
    }
}

/* ---------- direct.c --------- */

#ifndef FA_DIREC
#define FA_DIREC 0x10
#endif

static char path[MAXPATH];
static char drive[MAXDRIVE] = " :";
static char dir[MAXDIR];
static char name[MAXFILE];
static char ext[MAXEXT];

/* ----- Create unambiguous path from file spec, filling in the
     drive and directory if incomplete. Optionally change to
     the new drive and subdirectory ------ */
void CreatePath(char *spath,char *fspec,int InclName,int Change)
{
    int cm = 0;
    unsigned currdrive;
    char currdir[MAXPATH+1];
    char *cp;

    if (!Change)    { /* backup if not to be changed */
        /* ---- save the current drive and subdirectory ---- */
        currdrive = getdisk();
        getcwd(currdir, sizeof currdir);
        memmove(currdir, currdir+2, strlen(currdir+1));
        cp = currdir+strlen(currdir)-1;
        if ((*cp == '\\') && (strlen(dir) > 1)) /* save "\\" - Eric */
            *cp = '\0';
    }
    *drive = *dir = *name = *ext = '\0';

    fnsplit(fspec, drive, dir, name, ext);

    if (!InclName)
        *name = *ext = '\0';
    *drive = toupper(*drive);
    if (*ext)
        cm |= EXTENSION;
    if (InclName && *name)
        cm |= FILENAME;
    if (*dir)
        cm |= DIRECTORY;

    if (*drive)
        cm |= DRIVE;
    if (cm & DRIVE)
        setdisk(*drive - 'A');
    else     {
        *drive = getdisk();
        *drive += 'A';
    }

    if (cm & DIRECTORY)    {
        cp = dir+strlen(dir)-1;
        if ((*cp == '\\') && (strlen(dir) > 1))
            /* save "\\" if it is the only dirspec - Eric */
            *cp = '\0'; /* remove trailing backslash if not needed */
        chdir(dir);
    }
    getcwd(dir, sizeof dir);
    memmove(dir, dir+2, strlen(dir+1));

    if (InclName) { /* unless only drive / directory wanted */
        if (!(cm & FILENAME))
            strcpy(name, "*");
        if (!(cm & EXTENSION) && strchr(fspec, '.') != NULL)
            strcpy(ext, ".*");
    }
    else
        *name = *ext = '\0';

    if (dir[strlen(dir)-1] != '\\')
        strcat(dir, "\\"); /* supply a trailing backslash */

    if (spath != NULL)
        fnmerge(spath, drive, dir, name, ext);

    if (!Change) { /* restore if not to be changed */
        setdisk(currdrive);
        chdir(currdir);
    }
}

static int dircmp(const void *c1, const void *c2)
{
    return stricmp(*(char **)c1, *(char **)c2);
}

static BOOL BuildList(WINDOW wnd, char *fspec, BOOL dirs)
{
    int ax, i = 0, criterr = 1;
    ffblk ff;
    CTLWINDOW *ct = FindCommand(wnd->extension, dirs ? ID_DIRECTORY : ID_FILES,LISTBOX);
    WINDOW lwnd;
    char **dirlist = NULL;

    if (ct != NULL)    {
        lwnd = ct->wnd;
        SendMessage(lwnd, CLEARTEXT, 0, 0);

        while (criterr == 1) {
            ax = FindFirst(fspec, dirs ? FA_DIREC: 0, ff);
            criterr = TestCriticalError();
        }
        if (criterr)
            return FALSE;
        while (ax == 0) {
            if (!dirs || ((AttribOf(ff) & FA_DIREC) && strcmp(NameOf(ff), "."))) {
                dirlist = DFrealloc(dirlist, sizeof(char *)*(i+1));
                dirlist[i] = DFmalloc(strlen(NameOf(ff))+1);
                strcpy(dirlist[i++], NameOf(ff));
            }
            ax = FindNext(ff);
        }
        if (dirlist != NULL)    {
            int j;
            /* -- sort file or directory list box data -- */
            qsort(dirlist, i, sizeof(void *), dircmp);
            /* ---- send sorted list to list box ---- */
            for (j = 0; j < i; j++)    {
                SendMessage(lwnd,ADDTEXT,(PARAM)dirlist[j],0);
                free(dirlist[j]);
            }
            free(dirlist);
        }
        SendMessage(lwnd, SHOW_WINDOW, 0, 0);
    }
    return TRUE;
}

BOOL BuildFileList(WINDOW wnd, char *fspec)
{
    return BuildList(wnd, fspec, FALSE);
}

void BuildDirectoryList(WINDOW wnd)
{
    BuildList(wnd, "*.*", TRUE);
}

void BuildDriveList(WINDOW wnd)
{
    CTLWINDOW *ct = FindCommand(wnd->extension, ID_DRIVE,LISTBOX);
    if (ct != NULL)
        {
        char drname[15];
        WINDOW lwnd = ct->wnd;

        SendMessage(lwnd, CLEARTEXT, 0, 0);
        int cd = getdisk();
        for (int dr = 0; dr < 26; dr++) /* would be better to use LASTDRIVE */
            {
            unsigned ndr;

            setdisk(dr);
            ndr = getdisk(); /* drive really accessible? */
            if (ndr == dr)
                {
                sprintf(drname, "[-%c-]", dr+'A');
                }
                SendMessage(lwnd,ADDTEXT,(PARAM)drname,0);
            }

        SendMessage(lwnd, SHOW_WINDOW, 0, 0);
        setdisk(cd);
    }
}

void BuildPathDisplay(WINDOW wnd)
{
    CTLWINDOW *ct = FindCommand(wnd->extension, ID_PATH,TEXT);
    if (ct != NULL)
        {
        int len;
        WINDOW lwnd = ct->wnd;

        CreatePath(path, "*.*", FALSE, FALSE);
        len = strlen(path);
        if ( (path[len-1] == '\\') && (len > 3) )
            path[len-1] = '\0'; /* strip final backslash IF dirname given */

        SendMessage(lwnd,SETTEXT,(PARAM)path,0);
        SendMessage(lwnd, PAINT, 0, 0);
    }
}

/* ------------- editbox.c ------------ */

#define EditBufLen(wnd) (isMultiLine(wnd) ? EDITLEN : ENTRYLEN)
#define SetLinePointer(wnd, ln) (wnd->CurrLine = ln)

#define FANCY_CTRL_P /* kludgy eye-candy: ^P clock / statusbar handling */

#ifdef TAB_TOGGLING
#define Ch(c) ((c)&0x7f)    /* ignore high bit for whitespace check  */
                            /* (for tab-substitute and tabby-spaces) */
#define isWhite(c) ( (Ch(c)==' ') || (Ch(c)=='\n') || (Ch(c)=='\f') || (Ch(c)=='\t') )
#else
#define isWhite(c) ( ((c)==' ') || ((c)=='\n') || ((c)=='\t') )
#endif

/* ---------- local prototypes ----------- */
static void SaveDeletedText(WINDOW, char *, unsigned int); /* *** UNSIGNED *** */
static void Forward(WINDOW);
static void Backward(WINDOW);
static void End(WINDOW);
static void Home(WINDOW);
static void Downward(WINDOW);
static void Upward(WINDOW);
static void StickEnd(WINDOW);
static void NextWord(WINDOW);
static void PrevWord(WINDOW);
static void ModTextPointers(WINDOW, int, int);
static void SetAnchor(WINDOW, int, int);
/* -------- local variables -------- */
static BOOL KeyBoardMarking, ButtonDown;
static BOOL TextMarking;
static int ButtonX, ButtonY;
static int PrevY = -1;

/* ----------- CREATE_WINDOW Message ---------- */
static int EBCreateWindowMsg(WINDOW wnd)
{
    int rtn = BaseWndProc(EDITBOX, wnd, CREATE_WINDOW, 0, 0);
    /* *** added in 0.6e *** */
    wnd->BlkBegLine = 0;
    wnd->BlkBegCol = 0;
    wnd->BlkEndLine = 0;
    wnd->BlkEndCol = 0;
    wnd->TextChanged = FALSE;
    wnd->DeletedText = NULL;
    wnd->DeletedLength = 0;
    /* *** /added *** */
    wnd->MaxTextLength = MAXTEXTLEN+1;
    wnd->textlen = EditBufLen(wnd);
    wnd->InsertMode = TRUE;
    if (isMultiLine(wnd))
        wnd->WordWrapMode = TRUE;
    SendMessage(wnd, CLEARTEXT, 0, 0);
    return rtn;
}
/* ----------- SETTEXT Message ---------- */
static int EBSetTextMsg(WINDOW wnd, PARAM p1)
{
    int rtn = FALSE;
    if (strlen((char *)p1) <= wnd->MaxTextLength)	{
        rtn = BaseWndProc(EDITBOX, wnd, SETTEXT, p1, 0);
        wnd->TextChanged = FALSE;
    }
    return rtn;
}
/* ----------- CLEARTEXT Message ------------ */
static int EBClearTextMsg(WINDOW wnd)
{
    int rtn = BaseWndProc(EDITBOX, wnd, CLEARTEXT, 0, 0);
    unsigned blen = EditBufLen(wnd)+2;
    wnd->text = DFrealloc(wnd->text, blen);
    memset(wnd->text, 0, blen);
    wnd->wlines = 0;
    wnd->CurrLine = 0;
    wnd->CurrCol = 0;
    wnd->WndRow = 0;
    wnd->wleft = 0;
    wnd->wtop = 0;
    wnd->textwidth = 0;
    wnd->TextChanged = FALSE;
    return rtn;
}
/* ----------- ADDTEXT Message ---------- */
static int EBAddTextMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int rtn = FALSE;
    if (strlen((char *)p1)+wnd->textlen <= wnd->MaxTextLength) {
        rtn = BaseWndProc(EDITBOX, wnd, ADDTEXT, p1, p2);
        if (rtn != FALSE)    {
            if (!isMultiLine(wnd))    {
                wnd->CurrLine = 0;
                wnd->CurrCol = strlen((char *)p1);
                if (wnd->CurrCol >= ClientWidth(wnd))    {
                    wnd->wleft = wnd->CurrCol-ClientWidth(wnd);
                    wnd->CurrCol -= wnd->wleft;
                }
                wnd->BlkEndCol = wnd->CurrCol;
                SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
            }
        }
    }
    return rtn;
}
/* ----------- GETTEXT Message ---------- */
static int EBGetTextMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    char *cp1 = (char *)p1;
    char *cp2 = wnd->text;
    if (cp2 != NULL)    {
        while (p2-- && *cp2 && *cp2 != '\n')
            *cp1++ = *cp2++;
        *cp1 = '\0';
        return TRUE;
    }
    return FALSE;
}
/* ----------- SETTEXTLENGTH Message ---------- */
static int EBSetTextLengthMsg(WINDOW wnd, unsigned int len)
{
    if (++len < MAXTEXTLEN)    {
        wnd->MaxTextLength = len;
        if (len < wnd->textlen)    {
            wnd->text=DFrealloc(wnd->text, len+2);
            wnd->textlen = len;
            *((wnd->text)+len) = '\0';
            *((wnd->text)+len+1) = '\0';
            BuildTextPointers(wnd);
        }
        return TRUE;
    }
    return FALSE;
}
/* ----------- KEYBOARD_CURSOR Message ---------- */
static void EBKeyboardCursorMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    wnd->CurrCol = (int)p1 + wnd->wleft;
    wnd->WndRow = (int)p2;
    wnd->CurrLine = (int)p2 + wnd->wtop;
    if (wnd == inFocus)	{
        if (CharInView(wnd, (int)p1, (int)p2))
            SendMessage(NULL, SHOW_CURSOR, (wnd->InsertMode && !TextMarking), 0);
        else
            SendMessage(NULL, HIDE_CURSOR, 0, 0);
    }
}
/* ----------- SIZE Message ---------- */
int EBSizeMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int rtn = BaseWndProc(EDITBOX, wnd, SIZE, p1, p2);
    if (WndCol > ClientWidth(wnd)-1)
        wnd->CurrCol = ClientWidth(wnd)-1 + wnd->wleft;
    if (wnd->WndRow > ClientHeight(wnd)-1)    {
        wnd->WndRow = ClientHeight(wnd)-1;
        SetLinePointer(wnd, wnd->WndRow+wnd->wtop);
    }
    SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
    return rtn;
}
/* ----------- SCROLL Message ---------- */
static int EBScrollMsg(WINDOW wnd, PARAM p1)
{
    int rtn = FALSE;
    if (isMultiLine(wnd))    {
        rtn = BaseWndProc(EDITBOX,wnd,SCROLL,p1,0);
        if (rtn != FALSE)    {
            if (p1)    {
                /* -------- scrolling up --------- */
                if (wnd->WndRow == 0)    {
                    wnd->CurrLine++;
                    StickEnd(wnd);
                }
                else
                    --wnd->WndRow;
            }
            else    {
                /* -------- scrolling down --------- */
                if (wnd->WndRow == ClientHeight(wnd)-1)    {
                    if (wnd->CurrLine > 0)
                        --wnd->CurrLine;
                    StickEnd(wnd);
                }
                else
                    wnd->WndRow++;
            }
            SendMessage(wnd,KEYBOARD_CURSOR,WndCol,wnd->WndRow);
        }
    }
    return rtn;
}
/* ----------- HORIZSCROLL Message ---------- */
static int EBHorizScrollMsg(WINDOW wnd, PARAM p1)
{
    int rtn = FALSE;
    char *currchar = CurrChar;
    if (!(p1 &&
            wnd->CurrCol == wnd->wleft && *currchar == '\n'))  {
        rtn = BaseWndProc(EDITBOX, wnd, HORIZSCROLL, p1, 0);
        if (rtn != FALSE)    {
            if (wnd->CurrCol < wnd->wleft)
                wnd->CurrCol++;
            else if (WndCol == ClientWidth(wnd))
                --wnd->CurrCol;
            SendMessage(wnd,KEYBOARD_CURSOR,WndCol,wnd->WndRow);
        }
    }
    return rtn;
}
/* ----------- SCROLLPAGE Message ---------- */
static int EBScrollPageMsg(WINDOW wnd, PARAM p1)
{
    int rtn = FALSE;
    if (isMultiLine(wnd))    {
        rtn = BaseWndProc(EDITBOX, wnd, SCROLLPAGE, p1, 0);
        SetLinePointer(wnd, wnd->wtop+wnd->WndRow);
        StickEnd(wnd);
        SendMessage(wnd, KEYBOARD_CURSOR,WndCol, wnd->WndRow);
    }
    return rtn;
}
/* ----------- HORIZSCROLLPAGE Message ---------- */
static int EBHorizPageMsg(WINDOW wnd, PARAM p1)
{
    int rtn = BaseWndProc(EDITBOX, wnd, HORIZPAGE, p1, 0);
    if ((int) p1 == FALSE)    {
        if (wnd->CurrCol > wnd->wleft+ClientWidth(wnd)-1)
            wnd->CurrCol = wnd->wleft+ClientWidth(wnd)-1;
    }
    else if (wnd->CurrCol < wnd->wleft)
        wnd->CurrCol = wnd->wleft;
    SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
    return rtn;
}
/* ----- Extend the marked block to the new x,y position ---- */
static void ExtendBlock(WINDOW wnd, int x, int y)
{
    int bbl, bel;
    int ptop = min(wnd->BlkBegLine, wnd->BlkEndLine);
    int pbot = max(wnd->BlkBegLine, wnd->BlkEndLine);
    char *lp = TextLine(wnd, wnd->wtop+y);
    int len = (int) (strchr(lp, '\n') - lp);
    x = max(0, min(x, len));
    y = max(0, y);
    wnd->BlkEndCol = min(len, x+wnd->wleft);
    wnd->BlkEndLine = y+wnd->wtop;
    bbl = min(wnd->BlkBegLine, wnd->BlkEndLine);
    bel = max(wnd->BlkBegLine, wnd->BlkEndLine);
    /* *** end-before-beginning is allowed WHILE area size is edited *** */
    while (ptop < bbl)    {
        WriteTextLine(wnd, NULL, ptop, FALSE);
        ptop++;
    }
    for (y = bbl; y <= bel; y++)
        WriteTextLine(wnd, NULL, y, FALSE);
    while (pbot > bel)    {
        WriteTextLine(wnd, NULL, pbot, FALSE);
        --pbot;
    }
}
/* ----------- LEFT_BUTTON Message ---------- */
static int EBLeftButtonMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int MouseX = (int) p1 - GetClientLeft(wnd);
    int MouseY = (int) p2 - GetClientTop(wnd);
    RECT rc = ClientRect(wnd);
    char *lp;
    int len;
    if (KeyBoardMarking)
        return TRUE;
    if (WindowMoving || WindowSizing)
        return FALSE;
    if (TextMarking)    {
        if (!InsideRect(p1, p2, rc))    {
            int x = MouseX, y = MouseY;
            int dir;
            MESSAGE msg = 0;
            if ((int)p2 == GetTop(wnd))
                y++, dir = FALSE, msg = SCROLL;
            else if ((int)p2 == GetBottom(wnd))
                --y, dir = TRUE, msg = SCROLL;
            else if ((int)p1 == GetLeft(wnd))
                --x, dir = FALSE, msg = HORIZSCROLL;
            else if ((int)p1 == GetRight(wnd))
                x++, dir = TRUE, msg = HORIZSCROLL;
            if (msg != 0)	{
                if (SendMessage(wnd, msg, dir, 0))
                    ExtendBlock(wnd, x, y);
                SendMessage(wnd, PAINT, 0, 0);
            }
        }
        return TRUE;
    }
    if (!InsideRect(p1, p2, rc))
        return FALSE;
    if (TextBlockMarked(wnd))    {
        ClearTextBlock(wnd); /* un-mark block */
        SendMessage(wnd, PAINT, 0, 0);
    }
    if (wnd->wlines)    {
        if (MouseY > wnd->wlines-1)
            return TRUE;
        lp = TextLine(wnd, MouseY+wnd->wtop);
        len = (int) (strchr(lp, '\n') - lp);
        MouseX = min(MouseX, len);
        if (MouseX < wnd->wleft)    {
            MouseX = 0;
            SendMessage(wnd, KEYBOARD, HOME, 0);
        }
        ButtonDown = TRUE;
        ButtonX = MouseX;
        ButtonY = MouseY;
    } else
        MouseX = MouseY = 0;
    wnd->WndRow = MouseY;
    SetLinePointer(wnd, MouseY+wnd->wtop);

    if (isMultiLine(wnd) || (!TextBlockMarked(wnd) && MouseX+wnd->wleft < strlen(wnd->text)))
        wnd->CurrCol = MouseX+wnd->wleft;
    SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
    return TRUE;
}
/* ----------- MOUSE_MOVED Message ---------- */
static int EBMouseMovedMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int MouseX = (int) p1 - GetClientLeft(wnd);
    int MouseY = (int) p2 - GetClientTop(wnd);
    RECT rc = ClientRect(wnd);
    if (!InsideRect(p1, p2, rc))
        return FALSE;
    if (MouseY > wnd->wlines-1)
        return FALSE;
    if (ButtonDown)    {
        SetAnchor(wnd, ButtonX+wnd->wleft, ButtonY+wnd->wtop);
        TextMarking = TRUE;
        rc = WindowRect(wnd);
        SendMessage(NULL,MOUSE_TRAVEL,(PARAM) &rc, 0);
        ButtonDown = FALSE;
    }
    if (TextMarking && !(WindowMoving || WindowSizing))    {
        ExtendBlock(wnd, MouseX, MouseY);
        return TRUE;
    }
    return FALSE;
}

/* End an "editing the marked area size and position" session AND */
/* ensure that the Beg(inning) is before the End! Swap if needed. */
static void StopMarking(WINDOW wnd)
{
    TextMarking = FALSE;
    if ( wnd->BlkBegLine > wnd->BlkEndLine ) {
        swapi(wnd->BlkBegLine, wnd->BlkEndLine);
        swapi(wnd->BlkBegCol, wnd->BlkEndCol);
    }
    if ( (wnd->BlkBegLine == wnd->BlkEndLine) &&
         (wnd->BlkBegCol > wnd->BlkEndCol) )
        swapi(wnd->BlkBegCol, wnd->BlkEndCol);
}
/* ----------- BUTTON_RELEASED Message ---------- */
static int EBButtonReleasedMsg(WINDOW wnd)
{
    ButtonDown = FALSE;
    if (TextMarking && !(WindowMoving || WindowSizing))  {
        /* release the mouse ouside the edit box */
        SendMessage(NULL, MOUSE_TRAVEL, 0, 0);
        StopMarking(wnd);
        return TRUE;
    }
    PrevY = -1;
    return FALSE;
}
/* ---- Process text block keys for multiline text box ---- */
static void DoMultiLines(WINDOW wnd, int c, PARAM p2)
{
    if (!KeyBoardMarking)    {
        if ((int)p2 & (LEFTSHIFT | RIGHTSHIFT))    { /* shift-cursor */
            switch (c)    {
                case HOME:
                case CTRL_HOME:
                case CTRL_BS:
                case PGUP:
                case CTRL_PGUP:
                case UP:
                case BS:
                case END:
                case CTRL_END:
                case PGDN:
                case CTRL_PGDN:
                case DN:
#ifdef HOOKKEYB
                case FWD:	/* right arrow! */
                case CTRL_FWD:	/* old ctrl-rightarrow */
#else
                case RARROW:	/* formerly called FWD */
                case LARROW:	/* hope that makes sense */
                case CTRL_RARROW: /* new ctrl-rightarrow */
                case CTRL_LARROW: /* ctrl-leftarrow */
#endif
                    KeyBoardMarking = TextMarking = TRUE;
                    SetAnchor(wnd, wnd->CurrCol, wnd->CurrLine);
                    break;
                default:
                    break;
            }
        }
    }
}
/* ---------- page/scroll keys ----------- */
static int DoScrolling(WINDOW wnd, int c, PARAM p2)
{
    switch (c)    {
        case PGUP:
        case PGDN:
            if (isMultiLine(wnd))
                BaseWndProc(EDITBOX, wnd, KEYBOARD, c, p2);
            break;
        case CTRL_PGUP:
        case CTRL_PGDN:
            BaseWndProc(EDITBOX, wnd, KEYBOARD, c, p2);
            break;
        case HOME:
            Home(wnd);
            break;
        case END:
            End(wnd);
            break;
#ifdef HOOKKEYB
        case CTRL_FWD:		/* old ctrl-rightarrow */
#else
        case CTRL_RARROW:	/* new ctrl-rightarrow */
#endif
            NextWord(wnd);
            break;
#ifndef HOOKKEYB
	case CTRL_LARROW:	/* ctrl-leftarrow */
#endif
        case CTRL_BS:
            PrevWord(wnd);
            break;
        case CTRL_HOME:
            if (isMultiLine(wnd))    {
                SendMessage(wnd, SCROLLDOC, TRUE, 0);
                wnd->CurrLine = 0;
                wnd->WndRow = 0;
            }
            Home(wnd);
            break;
        case CTRL_END:
            if (isMultiLine(wnd) && wnd->WndRow+wnd->wtop+1 < wnd->wlines && wnd->wlines > 0) {
                SendMessage(wnd, SCROLLDOC, FALSE, 0);
                SetLinePointer(wnd, wnd->wlines-1);
                wnd->WndRow = min(ClientHeight(wnd)-1, wnd->wlines-1);
                Home(wnd);
            }
            End(wnd);
            break;
        case UP:
            if (isMultiLine(wnd))
                Upward(wnd);
            break;
        case DN:
            if (isMultiLine(wnd))
                Downward(wnd);
            break;
#ifdef HOOKKEYB
        case FWD: /* old name for rightarrow */
#else
        case RARROW: /* formerly called FWD */
#endif
            Forward(wnd);
            break;
#ifdef HOOKKEYB
        case BS: /* why should BackSpace do only cursor movement??? */
#else
        /* indeed: if we had BS here, BS would only move the cursor! */
        case LARROW: /* hope this makes sense */
#endif
            Backward(wnd);
            break;
        default:
            return FALSE;
    }
    if (!KeyBoardMarking && TextBlockMarked(wnd))    {
        ClearTextBlock(wnd); /* un-mark block */
        SendMessage(wnd, PAINT, 0, 0);
    }
    SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
    return TRUE;
}
/* -------------- Del key ---------------- */
static void DelKey(WINDOW wnd)
{
    char *currchar = CurrChar;
    int repaint = *currchar == '\n';
    if (TextBlockMarked(wnd)) {
        SendMessage(wnd, COMMAND, ID_DELETETEXT, 0);
        SendMessage(wnd, PAINT, 0, 0);
        return;
    }
    if (isMultiLine(wnd) && *currchar == '\n' && *(currchar+1) == '\0')
        return;
    strcpy(currchar, currchar+1);
    if (repaint) {
        BuildTextPointers(wnd);
        SendMessage(wnd, PAINT, 0, 0);
    } else {
        ModTextPointers(wnd, wnd->CurrLine+1, -1);
        WriteTextLine(wnd, NULL, wnd->WndRow+wnd->wtop, FALSE);
    }
    wnd->TextChanged = TRUE;
}
/* ------------ Tab key ------------ */
/* not called in tab-type-through mode -ea */
static void TabKey(WINDOW wnd, PARAM p2)
{
    int insmd = wnd->InsertMode;

    if (isMultiLine(wnd))
        {
        do
            {
            char *cc = CurrChar+1;

            if (!insmd && *cc == '\0')
                break;

            if (wnd->textlen == wnd->MaxTextLength)
                break;

#ifdef HOOKKEYB
            SendMessage(wnd,KEYBOARD,insmd ? ' ' : FWD,0); /* !?!? */
#else
            SendMessage(wnd,KEYBOARD,insmd ? ' ' : RARROW,0); /* !?!? */
#endif
            }
        while (wnd->CurrCol % SysConfig.EditorTabSize);

        }
    else
        PostMessage(GetParent(wnd), KEYBOARD, '\t', p2);
}
/* ------------ Shift+Tab key ------------ */
/* inverse tab. Not called in tab-type-through mode -ea */
static void ShiftTabKey(WINDOW wnd, PARAM p2)
{
    if (isMultiLine(wnd))    {
        do {
            if (CurrChar == GetText(wnd))
                break;
            SendMessage(wnd,KEYBOARD,BS,0);
            /* *** ^-- yet again, BS used as alias for LARROW here *** */
        } while (wnd->CurrCol % SysConfig.EditorTabSize);
    } else
        PostMessage(GetParent(wnd), KEYBOARD, SHIFT_HT, p2);
}
/* --------- All displayable typed keys ------------- */
static void KeyTyped(WINDOW wnd, int c)
{
    char *currchar = CurrChar;
#ifdef HOOKKEYB
    if (c == '\0' || (c & OFFSET))
#else
    if ( (c == '\0') || ((c & FKEY) != 0) ) /* skip this if function key */
#endif
        /* ---- not recognized by editor --- */
        /* cursor and stuff already done by our caller - Eric */
        /* so I change (c != '\n' && c < ' ') to c == '\0'    */
        /* -> now we may type ESC and other stuff - Eric      */
        return;

    if ( (!isMultiLine(wnd)) && TextBlockMarked(wnd) )    {
        SendMessage(wnd, CLEARTEXT, 0, 0);
        /* ^-- huh? Anything typed zaps current selection contents? */
        currchar = CurrChar;
    }
    /* ---- test typing at end of text ---- */
    if (currchar == wnd->text+wnd->MaxTextLength)    {
        /* ---- typing at the end of maximum buffer ---- */
        beep();
        return;
    }
    if (*currchar == '\0')    {
        /* --- insert a newline at end of text --- */
        *currchar = '\n';
        *(currchar+1) = '\0';
        BuildTextPointers(wnd);
    }
    /* --- displayable char or newline --- */
    if (c == '\n' || wnd->InsertMode || *currchar == '\n') {
        /* ------ inserting the keyed character ------ */
        if (wnd->text[wnd->textlen-1] != '\0')    {
            /* --- the current text buffer is full --- */
            if (wnd->textlen == wnd->MaxTextLength)    {
                /* --- text buffer is at maximum size --- */
                beep();
                return;
            }
            /* ---- increase the text buffer size ---- */
            wnd->textlen += GROWLENGTH;
            /* --- but not above maximum size --- */
            if (wnd->textlen > wnd->MaxTextLength)
                wnd->textlen = wnd->MaxTextLength;
            wnd->text = DFrealloc(wnd->text, wnd->textlen+2);
            wnd->text[wnd->textlen-1] = '\0';
            currchar = CurrChar;
        }
        memmove(currchar+1, currchar, strlen(currchar)+1);
        ModTextPointers(wnd, wnd->CurrLine+1, 1);
        if (isMultiLine(wnd) && wnd->wlines > 1)
            wnd->textwidth = max(wnd->textwidth,(int) (TextLine(wnd, wnd->CurrLine+1)-TextLine(wnd, wnd->CurrLine)));
        else
            wnd->textwidth = max(wnd->textwidth,strlen(wnd->text));
        WriteTextLine(wnd, NULL,wnd->wtop+wnd->WndRow, FALSE);
    }
    /* ----- put the char in the buffer ----- */
    *currchar = c;
    wnd->TextChanged = TRUE;
    if (c == '\n')    {
        wnd->wleft = 0;
        BuildTextPointers(wnd);
        End(wnd);
        Forward(wnd);
        SendMessage(wnd, PAINT, 0, 0);
        return;
    }
    /* ---------- test end of window --------- */
    if (WndCol == ClientWidth(wnd)-1)    {
        if (!isMultiLine(wnd))	{
            if (!(currchar == wnd->text+wnd->MaxTextLength-2))
            SendMessage(wnd, HORIZSCROLL, TRUE, 0);
        } else {
            char *cp = currchar;
            while (*cp != ' ' && cp != TextLine(wnd, wnd->CurrLine))
                --cp;
            if (cp == TextLine(wnd, wnd->CurrLine) || !wnd->WordWrapMode)
                SendMessage(wnd, HORIZSCROLL, TRUE, 0);
            else {
                int dif = 0;
                if (c != ' ')    {
                    dif = (int) (currchar - cp);
                    wnd->CurrCol -= dif;
                    SendMessage(wnd, KEYBOARD, DEL, 0);
                    --dif;
                }
                SendMessage(wnd, KEYBOARD, '\n', 0);
                currchar = CurrChar;
                wnd->CurrCol = dif;
                if (c == ' ')
                    return;
            }
        }
    }
    /* ------ display the character ------ */
    SetStandardColor(wnd);
    if (wnd->protect)
        c = '*';
    PutWindowChar(wnd, c, WndCol, wnd->WndRow);
    /* ----- advance the pointers ------ */
    wnd->CurrCol++;
}

/* ------------ screen changing key strokes ------------- */
static void DoKeyStroke(WINDOW wnd, int c, PARAM p2)
{
    if (SysConfig.EditorGlobalReadOnly && TestAttribute(wnd, READONLY)) {
        /* read only mode added 0.7b */
        beep();
        return;
    }

    if ( ((unsigned int)p2) == SYSRQKEY )
        goto doNormalKey;   /* verbatim type mode: ^P + any key  */
                            /* see KeyboardMsg for preparations! */
    switch (c) {
        case BS:	/* formerly called RUBOUT */
            if (wnd->CurrCol == 0 && wnd->CurrLine == 0)
                break;
#ifdef HOOKKEYB
            SendMessage(wnd, KEYBOARD, BS, 0);
#else
            SendMessage(wnd, KEYBOARD, LARROW, 0);
#endif
            /* *** ^-- to be seen as "left arrow" here! *** */
            SendMessage(wnd, KEYBOARD, DEL, 0);
            /* *** ^-- remove char at cursor *** */
            break;
        case DEL:
            DelKey(wnd);
            break;
        case SHIFT_HT:
            if (SysConfig.EditorTabSize <= 1)
                goto doNormalKey; /* tab-type-through mode -ea */
            ShiftTabKey(wnd, p2);
            break;
        case '\t':
            if (SysConfig.EditorTabSize <= 1)
                goto doNormalKey; /* tab-type-through mode -ea */
            TabKey(wnd, p2);
            break;
        case '\r':
            if (!isMultiLine(wnd))    {
                PostMessage(GetParent(wnd), KEYBOARD, c, p2);
                break;
            }
            c = '\n'; /* fall through to default case here... */
        default:
            doNormalKey:
            if ( TextBlockMarked(wnd) &&
#ifdef HOOKKEYB
                 ((c & OFFSET) == 0)
#else
                 ((c & FKEY) == 0)
#endif
                 /* F-keys should not zap current selection! (0.6c) */
               )
            {
                SendMessage(wnd, COMMAND, ID_DELETETEXT, 0);
                SendMessage(wnd, PAINT, 0, 0);
            } /* /typed normal key while text was selected... */
            KeyTyped(wnd, c);
            break;
    }
}

/* ----------- KEYBOARD Message ---------- */
static int EBKeyboardMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int c = (int) p1;
    if (WindowMoving || WindowSizing || ((int)p2 & ALTKEY))
        return FALSE;
    /* unless window is moving / resizing... */

    switch (c)    { /* all except Alt-... */
        /* we stop processing F-keys / Ctrl-... at this point... */
        /* --- these keys get processed by lower classes --- */

        /* *** Allow people to type ESC as normal text while editbox has focus -ea
        ASM: NO! Then the dialogs no longer process ESC as Cancel dialog
        */

        case ESC:

        case F1: /* help */
        case F2:
        case F3: /* repeat search */
        case F4:
        case F5:
        case F6:
        case F7:
        case F8: /* ... */
        case F9:
        case F10: /* open menu */
        case INS:
/*
        case SHIFT_INS:
        case SHIFT_DEL:
*/
            return FALSE;	/* for all keys which are NOT for us! */
            /* (all other keys will be HIDDEN from more generic classes!) */
        /* --- these keys get processed here --- */
        case CTRL_RARROW:
        case CTRL_LARROW:
        case CTRL_BS:
        case CTRL_HOME:
        case CTRL_END:
        case CTRL_PGUP:
        case CTRL_PGDN:
            break;

        case CTRL_N:	/* 0.7a NEW  file */
        case CTRL_O:	/* 0.7a OPEN file */
        case CTRL_S:	/* 0.7a SAVE file */
        case CTRL_F:	/* 0.7c FIND text */
        case ALT_1: case ALT_2: case ALT_3:
        case ALT_4: case ALT_5: case ALT_6:
        case ALT_7: case ALT_8: case ALT_9:	/* 0.7c goto window */
            return FALSE; /* bounce to lower classes (for menu items) ... */
                          /* ... so they are not avail for verbatim typing  */

        /* *** force processing those elsewhere even if CTRL not really *** */
        /* *** pressed, for shift-ins/del emulation of ctrl-v/x paste/  *** */
        /* *** cut and shift-shift-ins ctrl-c copy emulation...         *** */
        case CTRL_V:	/* for PASTE */
        case CTRL_X:	/* for CUT   */
        case CTRL_C:	/* for COPY  */
        case CTRL_Z:	/* for UNDO  */
        case CTRL_F4:	/* for close */
        {
            BOOL tmb = TextMarking;
            if ( ((int)p2 & (CTRLKEY|ALTKEY|LEFTSHIFT|RIGHTSHIFT)) == 0) {
                p2 = SYSRQKEY; /* CTRL-... but CTRL/SHIFT/ALT all not  */
                break; /* pressed -> must be Alt-digit typing! */
            }
            StopMarking(wnd); /* *** ensure un-swapped selection endpoints *** */
            TextMarking = tmb;
            return FALSE; /* bounce keyboard message back to lower classes */
        }

        case CTRL_P: /* CTRL-P -> fetch 1 more key for verbatim typing */
#ifdef FANCY_CTRL_P
            SendMessage(GetParent(wnd), ADDSTATUS, (PARAM) "^P: Press any key for verbatim insertion.", 0);
            while (!keyhit())
                dispatch_message();	/* let (nested!) messages flow */
#endif	        /* *** if this ever crashes, remove the dispatch_message() call! *** */
            c = getkey() & 0xff;	/* not elegant, of course! */
            p2 = SYSRQKEY;		/* magic shift status */
#ifdef FANCY_CTRL_P
            SendMessage(wnd,KEYBOARD_CURSOR,WndCol,wnd->WndRow); /* normal status bar again */
#endif
            break;

        default:
            /* other ctrl keys get processed by lower classes */
            if ((int)p2 & CTRLKEY)
#ifndef DISABLE_TYPING_CTRL_KEYS
                /* we enumerated all USED CTRL keys above.  */
                /* Others may be typed as text from now on. */
                if (p1 > CTRL_Z) /* CTRL_A ... CTRL_Z are ASCII 1..26 */
                    return FALSE;
#endif
            /* --- all other keys get processed here --- */
            break;
    }
    /* only reached if: no Alt-key, no F-key, no USED Ctrl-key */

    if (p2 == SYSRQKEY) { /* special "type verbatim" mode */
        DoKeyStroke(wnd, c, p2);
        SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
        return TRUE;	/* consume key event */
    }

    DoMultiLines(wnd, c, p2);
    if (DoScrolling(wnd, c, p2)) {
        if (KeyBoardMarking)
            ExtendBlock(wnd, WndCol, wnd->WndRow);
    } else if (!TestAttribute(wnd, READONLY)) {
        DoKeyStroke(wnd, c, p2);
        SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
    } else if (c == '\t')
        PostMessage(GetParent(wnd), KEYBOARD, '\t', p2);
    else
        beep(); /* readonly and tried to really type something */
    return TRUE;
}

/* ----------- SHIFT_CHANGED Message ---------- */
static void EBShiftChangedMsg(WINDOW wnd, PARAM p1)
{
    if ( !( (int)p1 & (LEFTSHIFT | RIGHTSHIFT) ) && KeyBoardMarking) {
        StopMarking(wnd);
        KeyBoardMarking = FALSE;
    }
}

/* ----------- ID_DELETETEXT Command ---------- */
static void DeleteTextCmd(WINDOW wnd)
{
    if (TextBlockMarked(wnd))    {
        char *bb;
        char *be;
        unsigned int len;
        BOOL tmb = TextMarking;
        StopMarking(wnd); /* swap marks if begin was after end */
        TextMarking = tmb;
        bb = TextBlockBegin(wnd);
        be = TextBlockEnd(wnd);
        if (bb == be)
            return; /* empty deletion */
        if (bb >= be) { /* new check 0.7c */
            bb = TextBlockEnd(wnd); /* sic! */
            be = TextBlockBegin(wnd); /* sic! */
        }
        len = (unsigned int) (be - bb); /* *** unsigned *** */
        SaveDeletedText(wnd, bb, len);
        wnd->TextChanged = TRUE;
        strcpy(bb, be);	/* copy text after deletion over deleted text */
        wnd->CurrLine = TextLineNumber(wnd, bb-wnd->BlkBegCol);	/* ?? */
        wnd->CurrCol = wnd->BlkBegCol;
        wnd->WndRow = wnd->BlkBegLine - wnd->wtop;
        if (wnd->WndRow < 0)    {
            wnd->wtop = wnd->BlkBegLine;
            wnd->WndRow = 0;
        }
        SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
        ClearTextBlock(wnd);
        BuildTextPointers(wnd);
    }
}

/* ----------- ID_CLEAR Command ---------- */
static void ClearCmd(WINDOW wnd)
{
    if (TextBlockMarked(wnd))    {
        char *bb;
        char *be;
        unsigned int len;
        BOOL tmb = TextMarking;
        StopMarking(wnd); /* swap marks if begin was after end */
        TextMarking = tmb;
        bb = TextBlockBegin(wnd);
        be = TextBlockEnd(wnd);
        if (bb == be)
            return;	/* empty deletion */
        if (bb >= be) {	/* new check 0.7c */
            bb = TextBlockEnd(wnd);	/* sic! */
            be = TextBlockBegin(wnd);	/* sic! */
        }
        len = (unsigned int) (be - bb); /* *** unsigned *** */
        SaveDeletedText(wnd, bb, len);
        wnd->CurrLine = TextLineNumber(wnd, bb);
        wnd->CurrCol = wnd->BlkBegCol;
        wnd->WndRow = wnd->BlkBegLine - wnd->wtop;
        if (wnd->WndRow < 0)    {
            wnd->WndRow = 0;
            wnd->wtop = wnd->BlkBegLine;
        }
        /* ------ change all text lines in block to \n ----- */
        while (bb < be)    {
            char *cp = strchr(bb, '\n');
            if (cp > be)
                cp = be;
            strcpy(bb, cp);
            be -= (int) (cp - bb);	/* is that safe to do? */
            bb++;
        }
        ClearTextBlock(wnd);
        BuildTextPointers(wnd);
        SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
        wnd->TextChanged = TRUE;
    }
}

/* ----------- ID_UNDO Command ---------- */
static void UndoCmd(WINDOW wnd)
{
    if (wnd->DeletedText != NULL)    {
        PasteText(wnd, wnd->DeletedText, wnd->DeletedLength);
        free(wnd->DeletedText);
        wnd->DeletedText = NULL;
        wnd->DeletedLength = 0;
        SendMessage(wnd, PAINT, 0, 0);
    }
}

/* ----------- ID_PARAGRAPH Command ---------- */
static void ParagraphCmd(WINDOW wnd)
{
    int bc, fl;
    char *bl, *bbl, *bel, *bb;

    ClearTextBlock(wnd);
    /* ---- forming paragraph from cursor position --- */
    fl = wnd->wtop + wnd->WndRow;
    bbl = bel = bl = TextLine(wnd, wnd->CurrLine);
    if ((bc = wnd->CurrCol) >= ClientWidth(wnd))
        bc = 0;
    Home(wnd);
    /* ---- locate the end of the paragraph ---- */
    while (*bel)    {
        int blank = TRUE;
        char *bll = bel;
        /* --- blank line marks end of paragraph --- */
        while (*bel && *bel != '\n')    {
            if (*bel != ' ')
                blank = FALSE;
            bel++;
        }
        if (blank)    {
            bel = bll;
            break;
        }
        if (*bel)
            bel++;
    }
    if (bel == bbl)    {
        SendMessage(wnd, KEYBOARD, DN, 0);
        return;
    }
    if (*bel == '\0')
        --bel;
    if (*bel == '\n')
        --bel;
    /* --- change all newlines in block to spaces --- */
    while (CurrChar < bel)    {
        if (*CurrChar == '\n')    {
            *CurrChar = ' ';
            wnd->CurrLine++;
            wnd->CurrCol = 0;
        }
        else
            wnd->CurrCol++;
    }
    /* ---- insert newlines at new margin boundaries ---- */
    bb = bbl;
    while (bbl < bel)    {
        bbl++;
        if ((int)(bbl - bb) == ClientWidth(wnd)-1)    {
            while (*bbl != ' ' && bbl > bb)
                --bbl;
            if (*bbl != ' ')    {
                bbl = strchr(bbl, ' ');
                if (bbl == NULL || bbl >= bel)
                    break;
            }
            *bbl = '\n';
            bb = bbl+1;
        }
    }
    BuildTextPointers(wnd);
    /* --- put cursor back at beginning --- */
    wnd->CurrLine = TextLineNumber(wnd, bl);
    wnd->CurrCol = bc;
    if (fl < wnd->wtop)
        wnd->wtop = fl;
    wnd->WndRow = fl - wnd->wtop;
    SendMessage(wnd, PAINT, 0, 0);
    SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
    wnd->TextChanged = TRUE;
    BuildTextPointers(wnd);
}

/* ----------- COMMAND Message ---------- */
static int EBCommandMsg(WINDOW wnd, PARAM p1)
{
    if (SysConfig.EditorGlobalReadOnly && TestAttribute(wnd, READONLY)) {
        /* read only mode added 0.7b */
        switch ((int)p1) {
            case ID_REPLACE:
            case ID_CUT:
            case ID_PASTE:
            case ID_DELETETEXT:
            case ID_CLEAR:
            case ID_PARAGRAPH:
            case ID_UPCASE:	/* new readonly mode handling for 0.7d */
            case ID_DOWNCASE:
                beep();
                return TRUE;	/* consume event */
        }
    }

    switch ((int)p1)    {
        case ID_SEARCH:
            SearchText(wnd);
            return TRUE;
        case ID_REPLACE:
            ReplaceText(wnd);
            return TRUE;
        case ID_SEARCHNEXT:
            SearchNext(wnd);
            return TRUE;
        case ID_CUT:
            CopyToClipboard(wnd);
            SendMessage(wnd, COMMAND, ID_DELETETEXT, 0);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case ID_COPY:
            CopyToClipboard(wnd);
            ClearTextBlock(wnd);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case ID_PASTE:
            PasteFromClipboard(wnd);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case ID_DELETETEXT:
            DeleteTextCmd(wnd);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case ID_CLEAR:
            ClearCmd(wnd);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case ID_UNDO:
            UndoCmd(wnd);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case ID_PARAGRAPH:
            ParagraphCmd(wnd);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;

        /* 0.7d added commands follow (7/2005) */
        case ID_UPCASE:
            UpCaseMarked(wnd);
            ClearTextBlock(wnd);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case ID_DOWNCASE:
            DownCaseMarked(wnd);
            ClearTextBlock(wnd);
            SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case ID_WORDCOUNT:
            {
                unsigned bytes, words, lines;
                char statsline[50];
                statsline[0] = 0;
                StatsForMarked(wnd, &bytes, &words, &lines);
                ClearTextBlock(wnd);
                SendMessage(wnd, PAINT, 0, 0);
                sprintf(statsline," %u lines, %u words, %u bytes ",lines, words, bytes);
                SendMessage(GetParent(wnd), ADDSTATUS, (PARAM) statsline, 0);
#if 0
                while (!keyhit())
                    dispatch_message(); /* let (nested!) messages flow */
#endif
            }
            return TRUE;
        default:
            break;
    }
    return FALSE;
}

/* ---------- CLOSE_WINDOW Message ----------- */
static int EBCloseWindowMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int rtn;
    SendMessage(NULL, HIDE_CURSOR, 0, 0);
    if (wnd->DeletedText != NULL) {
        free(wnd->DeletedText);
        wnd->DeletedText = NULL;
    }
    rtn = BaseWndProc(EDITBOX, wnd, CLOSE_WINDOW, p1, p2);
    if (wnd->text != NULL) {
        free(wnd->text);
        wnd->text = NULL;
    }
    return rtn;
}

/* ------- Window processing module for EDITBOX class ------ */
int EditBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    switch (msg)    {
        case CREATE_WINDOW:
            return EBCreateWindowMsg(wnd);
        case ADDTEXT:
            return EBAddTextMsg(wnd, p1, p2);
        case SETTEXT:
            return EBSetTextMsg(wnd, p1);
        case CLEARTEXT:
        return EBClearTextMsg(wnd);
        case GETTEXT:
            return EBGetTextMsg(wnd, p1, p2);
        case SETTEXTLENGTH:
            return EBSetTextLengthMsg(wnd, (unsigned) p1);
        case KEYBOARD_CURSOR:
            EBKeyboardCursorMsg(wnd, p1, p2);
            return TRUE;
        case SETFOCUS:
            if (!(int)p1)
                SendMessage(NULL, HIDE_CURSOR, 0, 0);
        case PAINT:
        case MOVE:
            rtn = BaseWndProc(EDITBOX, wnd, msg, p1, p2);
            SendMessage(wnd,KEYBOARD_CURSOR,WndCol,wnd->WndRow);
            return rtn;
        case SIZE:
            return EBSizeMsg(wnd, p1, p2);
        case SCROLL:
            return EBScrollMsg(wnd, p1);
        case HORIZSCROLL:
            return EBHorizScrollMsg(wnd, p1);
        case SCROLLPAGE:
            return EBScrollPageMsg(wnd, p1);
        case HORIZPAGE:
            return EBHorizPageMsg(wnd, p1);
        case LEFT_BUTTON:
            if (EBLeftButtonMsg(wnd, p1, p2))
                return TRUE;
            break;
        case MOUSE_MOVED:
            if (EBMouseMovedMsg(wnd, p1, p2))
                return TRUE;
            break;
        case BUTTON_RELEASED:
            if (EBButtonReleasedMsg(wnd))
                return TRUE;
            break;
        case KEYBOARD:
            if (EBKeyboardMsg(wnd, p1, p2))
                return TRUE;
            break;
        case SHIFT_CHANGED:
            EBShiftChangedMsg(wnd, p1);
            break;
        case COMMAND:
            if (EBCommandMsg(wnd, p1))
                return TRUE;
            break;
        case CLOSE_WINDOW:
            return EBCloseWindowMsg(wnd, p1, p2);
        default:
            break;
    }
    return BaseWndProc(EDITBOX, wnd, msg, p1, p2);
}

/* ------ save deleted text for the Undo command ------ */
static void SaveDeletedText(WINDOW wnd, char *bbl, unsigned int len)
{
    /* removed "if len > 5000 or even > 0x8000" check in 0.7c, */
    /* hopefully you CAN save/undelete > 32k chars properly... */

    wnd->DeletedLength = len; /* *** UNSIGNED! *** */
    wnd->DeletedText=DFrealloc(wnd->DeletedText,len);
    memmove(wnd->DeletedText, bbl, len);
}

/* ---- cursor right key: right one character position ---- */
static void Forward(WINDOW wnd)
{
    char *cc = CurrChar+1;
    if (*cc == '\0')
        return;
    if (*CurrChar == '\n')    {
        Home(wnd);
        Downward(wnd);
    } else {
        wnd->CurrCol++;
        if (WndCol == ClientWidth(wnd))
            SendMessage(wnd, HORIZSCROLL, TRUE, 0);
    }
}

/* ----- stick the moving cursor to the end of the line ---- */
static void StickEnd(WINDOW wnd)
{
    char *cp = TextLine(wnd, wnd->CurrLine);
    char *cp1 = strchr(cp, '\n');
    int len = cp1 ? (int) (cp1 - cp) : 0;
    wnd->CurrCol = min(len, wnd->CurrCol);
    if (wnd->wleft > wnd->CurrCol)    {
        wnd->wleft = max(0, wnd->CurrCol - 4);
        SendMessage(wnd, PAINT, 0, 0);
    } else if (wnd->CurrCol-wnd->wleft >= ClientWidth(wnd)) {
        wnd->wleft = wnd->CurrCol - (ClientWidth(wnd)-1);
        SendMessage(wnd, PAINT, 0, 0);
    }
}

/* --------- cursor down key: down one line --------- */
static void Downward(WINDOW wnd)
{
    if (isMultiLine(wnd) &&
            wnd->WndRow+wnd->wtop+1 < wnd->wlines)  {
        wnd->CurrLine++;
        if (wnd->WndRow == ClientHeight(wnd)-1)
            SendMessage(wnd, SCROLL, TRUE, 0);
        wnd->WndRow++;
        StickEnd(wnd);
    }
}

/* -------- cursor up key: up one line ------------ */
static void Upward(WINDOW wnd)
{
    if (isMultiLine(wnd) && wnd->CurrLine != 0)    {
        --wnd->CurrLine;
        if (wnd->WndRow == 0)
            SendMessage(wnd, SCROLL, FALSE, 0);
        --wnd->WndRow;
        StickEnd(wnd);
    }
}

/* ---- cursor left key: left one character position ---- */
static void Backward(WINDOW wnd)
{
    if (wnd->CurrCol)    {
        --wnd->CurrCol;
        if (wnd->CurrCol < wnd->wleft)
            SendMessage(wnd, HORIZSCROLL, FALSE, 0);
    } else if (isMultiLine(wnd) && wnd->CurrLine != 0) {
        Upward(wnd);
        End(wnd);
    }
}

/* -------- End key: to end of line ------- */
static void End(WINDOW wnd)
{
    while (*CurrChar && *CurrChar != '\n')
        ++wnd->CurrCol;
    if (WndCol >= ClientWidth(wnd))    {
        wnd->wleft = wnd->CurrCol - (ClientWidth(wnd)-1);
        SendMessage(wnd, PAINT, 0, 0);
    }
}

/* -------- Home key: to beginning of line ------- */
static void Home(WINDOW wnd)
{
    wnd->CurrCol = 0;
    if (wnd->wleft != 0)    {
        wnd->wleft = 0;
        SendMessage(wnd, PAINT, 0, 0);
    }
}

/* -- Ctrl+cursor right key: to beginning of next word -- */
static void NextWord(WINDOW wnd)
{
    int savetop = wnd->wtop;
    int saveleft = wnd->wleft;
    ClearVisible(wnd);
    while (!isWhite(*CurrChar))    {
        char *cc = CurrChar+1;
        if (*cc == '\0')
            break;
        Forward(wnd);
    }
    while (isWhite(*CurrChar))    {
        char *cc = CurrChar+1;
        if (*cc == '\0')
            break;
        Forward(wnd);
    }
    SetVisible(wnd);
    SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
    if (wnd->wtop != savetop || wnd->wleft != saveleft)
        SendMessage(wnd, PAINT, 0, 0);
}

/* -- Ctrl+cursor left key: to beginning of previous word -- */
static void PrevWord(WINDOW wnd)
{
    int savetop = wnd->wtop;
    int saveleft = wnd->wleft;
    ClearVisible(wnd);
    Backward(wnd);
    while (isWhite(*CurrChar))    {
        if (wnd->CurrLine == 0 && wnd->CurrCol == 0)
            break;
        Backward(wnd);
    }
    while (wnd->CurrCol != 0 && !isWhite(*CurrChar))
        Backward(wnd);
    if (isWhite(*CurrChar))
        Forward(wnd);
    SetVisible(wnd);
    if (wnd->wleft != saveleft)
        if (wnd->CurrCol >= saveleft)
            if (wnd->CurrCol - saveleft < ClientWidth(wnd))
                wnd->wleft = saveleft;
    SendMessage(wnd, KEYBOARD_CURSOR, WndCol, wnd->WndRow);
    if (wnd->wtop != savetop || wnd->wleft != saveleft)
        SendMessage(wnd, PAINT, 0, 0);
}

/* ----- modify text pointers from a specified position by a specified plus or minus amount ----- */
static void ModTextPointers(WINDOW wnd, int lineno, int var)
{
    while (lineno < wnd->wlines)
        *((wnd->TextPointers) + lineno++) += var;
}

/* ----- set anchor point for marking text block ----- */
static void SetAnchor(WINDOW wnd, int mx, int my)
{
    ClearTextBlock(wnd);
    /* ------ set the anchor ------ */
    wnd->BlkBegLine = wnd->BlkEndLine = my;
    wnd->BlkBegCol = wnd->BlkEndCol = mx;
    SendMessage(wnd, PAINT, 0, 0);
}


/* NEW 7/2005 - not actually clipboard related but editbox.c is */
/* already tooo long anyway ;-) In-place text section stuff...  */
/* toupper/tolower: see ctype.h  -  do they support COUNTRY...? */

void UpCaseMarked(WINDOW wnd)
{
    if (TextBlockMarked(wnd))    {
        char *bb = TextBlockBegin(wnd);
        char *be = TextBlockEnd(wnd);
        if (bb >= be) {
            bb = TextBlockEnd(wnd); /* sic! */
            be = TextBlockBegin(wnd); /* sic! */
        }
        while (bb < be) {
           bb[0] = toupper(bb[0]);
           bb++;
        }
    }
}

void DownCaseMarked(WINDOW wnd)
{
    if (TextBlockMarked(wnd))    {
        char *bb = TextBlockBegin(wnd);
        char *be = TextBlockEnd(wnd);
        if (bb >= be) {
            bb = TextBlockEnd(wnd); /* sic! */
            be = TextBlockBegin(wnd); /* sic! */
        }
        while (bb < be) {
           bb[0] = tolower(bb[0]);
           bb++;
        }
    }
}

void StatsForMarked(WINDOW wnd, unsigned *bytes, unsigned *words, unsigned *lines)
{
    bytes[0] = words[0] = lines[0] = 0;
    if (TextBlockMarked(wnd))    {
    	int inWord = 0;
        char *bb = TextBlockBegin(wnd);
        char *be = TextBlockEnd(wnd);
        if (bb >= be) {
            bb = TextBlockEnd(wnd); /* sic! */
            be = TextBlockBegin(wnd); /* sic! */
        }
        while (bb < be) {
           char c = bb[0];
           if (c == '\n') lines[0]++;
           if (isspace(c)) {
                if (inWord) words[0]++;
                inWord = 0;
           } else {
                inWord = 1;
           }
           bytes[0]++;
           bb++;
        }
        if (inWord) words[0]++;
    }
}


/* ------------ helpbox.c ----------- */

/* ------------ the Help window dialog box -------------- */
DIALOGBOX(HelpBox)
    DB_TITLE(NULL, -1, -1, 0, 45)
    CONTROL(TEXTBOX, NULL,         1,  1, 0, 40, ID_HELPTEXT)
    CONTROL(BUTTON,  "  ~Close ",  0,  0, 1,  8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Back  ", 10,  0, 1,  8, ID_BACK)
    CONTROL(BUTTON,  "<< ~Prev ", 20,  0, 1,  8, ID_PREV)
    CONTROL(BUTTON,  " ~Next >>", 30,  0, 1,  8, ID_NEXT)
ENDDB


//void UnLoadHelpFile(void);

/* -------- strings of D-Flat classes for calling default help text collections -------- */

#define MAXHEIGHT (SCREENHEIGHT-10)
#define MAXHELPKEYWORDS 50  /* --- maximum keywords in a window --- */
#define MAXHELPSTACK 100

static struct helps *FirstHelp;
static struct helps *ThisHelp;
static int HelpCount;
static char HelpFileName[9];

static unsigned long int HelpStack[MAXHELPSTACK]; /* *** was int array *** */
static int stacked;

/* --- keywords in the current help text -------- */
static struct keywords {
    struct helps *hkey;
    int lineno;
    int off1, off2, off3;
    char isDefinition;
} KeyWords[MAXHELPKEYWORDS];
static struct keywords *thisword;
static int keywordcount;

static FILE *helpfp;
static char hline [160];
static BOOL Helping;

static void SelectHelp(WINDOW, struct helps *, BOOL);
static void ReadHelp(WINDOW);
static struct helps *FindHelp(char *);
static void DisplayDefinition(WINDOW, char *);
static void BestFit(WINDOW, DIALOGWINDOW *);

/* ------------- CREATE_WINDOW message ------------ */
static void HelpCreateWindowMsg(WINDOW wnd)
{
    Helping = TRUE;
    GetClass(wnd) = HELPBOX;
    InitWindowColors(wnd);
    if (ThisHelp != NULL)
        ThisHelp->hwnd = wnd;
}

/* ------------- COMMAND message ------------ */
static BOOL HelpCommandMsg(WINDOW wnd, PARAM p1)
{
    switch ((int)p1)    {
        case ID_PREV:
            if (ThisHelp  != NULL)
                SelectHelp(wnd, FirstHelp+(ThisHelp->prevhlp), TRUE);
            return TRUE;
        case ID_NEXT:
            if (ThisHelp != NULL)
                SelectHelp(wnd, FirstHelp+(ThisHelp->nexthlp), TRUE);
            return TRUE;
        case ID_BACK:
            if (stacked)
                SelectHelp(wnd, (FirstHelp + (unsigned)HelpStack[--stacked]), FALSE); /* *** uint pointer delta? *** */
            return TRUE;
        default:
            break;
    }
    return FALSE;
}

/* ------------- KEYBOARD message ------------ */
static BOOL HelpKeyboardMsg(WINDOW wnd, PARAM p1)
{
    WINDOW cwnd;

    cwnd = ControlWindow(wnd->extension, ID_HELPTEXT);
    if (cwnd == NULL || inFocus != cwnd)
        return FALSE;
    switch ((int)p1)    {
        case '\r':
            if (keywordcount)
                if (thisword != NULL) {
                    char *hp = thisword->hkey->hname;
                    if (thisword->isDefinition)
                        DisplayDefinition(GetParent(wnd), hp);
                    else
                        SelectHelp(wnd, thisword->hkey, TRUE);
                }
            return TRUE;
        case '\t':
            if (!keywordcount)
                return TRUE;
            if (thisword == NULL || ++thisword == KeyWords+keywordcount)
                thisword = KeyWords;
            break;
        case SHIFT_HT:
            if (!keywordcount)
                return TRUE;
            if (thisword == NULL || thisword == KeyWords)
                thisword = KeyWords+keywordcount;
            --thisword;
            break;;
        default:
            return FALSE;
    }
    if (thisword->lineno < cwnd->wtop ||
            thisword->lineno >=
                cwnd->wtop + ClientHeight(cwnd))  {
        int distance = ClientHeight(cwnd)/2;
        do {
            cwnd->wtop = thisword->lineno-distance;
            distance /= 2;
        } while (cwnd->wtop < 0);
    }
    SendMessage(cwnd, PAINT, 0, 0);
    return TRUE;
}

/* ---- window processing module for the HELPBOX ------- */
int HelpBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            HelpCreateWindowMsg(wnd);
            break;
        case INITIATE_DIALOG:
            ReadHelp(wnd);
            break;
        case COMMAND:
            if (p2 != 0)
                break;
            if (HelpCommandMsg(wnd, p1))
                return TRUE;
            break;
        case KEYBOARD:
            if (WindowMoving)
                break;
            if (HelpKeyboardMsg(wnd, p1))
                return TRUE;
            break;
        case CLOSE_WINDOW:
            if (ThisHelp != NULL)
                ThisHelp->hwnd = NULL;
            Helping = FALSE;
            break;
        default:
            break;
    }
    return BaseWndProc(HELPBOX, wnd, msg, p1, p2);
}

/* ---- PAINT message for the helpbox text editbox ---- */
static int HelpPaintMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int rtn;
    if (thisword != NULL)    {
        WINDOW pwnd = GetParent(wnd);
        char *cp;
        cp = TextLine(wnd, thisword->lineno);
        cp += thisword->off1;
        *(cp+1) = (pwnd->WindowColors[SELECT_COLOR][FG] & 255) | 0x80;
        *(cp+2) = (pwnd->WindowColors[SELECT_COLOR][BG] & 255) | 0x80;
        rtn = DefaultWndProc(wnd, PAINT, p1, p2);
        *(cp+1) = (pwnd->WindowColors[HILITE_COLOR][FG] & 255) | 0x80;
        *(cp+2) = (pwnd->WindowColors[HILITE_COLOR][BG] & 255) | 0x80;
        return rtn;
    }
    return DefaultWndProc(wnd, PAINT, p1, p2);
}

/* ---- LEFT_BUTTON message for the helpbox text editbox ---- */
static int HelpLeftButtonMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int rtn, mx, my, i;

    rtn = DefaultWndProc(wnd, LEFT_BUTTON, p1, p2);
    mx = (int)p1 - GetClientLeft(wnd);
    my = (int)p2 - GetClientTop(wnd);
    my += wnd->wtop;
    thisword = KeyWords;
    for (i = 0; i < keywordcount; i++)    {
        if (my == thisword->lineno) {
            if (mx >= thisword->off2 && mx < thisword->off3) {
                SendMessage(wnd, PAINT, 0, 0);
                if (thisword->isDefinition) {
                    WINDOW pwnd = GetParent(wnd);
                    if (pwnd != NULL)
                        DisplayDefinition(GetParent(pwnd), thisword->hkey->hname);
                }
                break;
            }
        }
        thisword++;
    }
    if (i == keywordcount)
        thisword = NULL;
    return rtn;
}

/* --- window processing module for HELPBOX's text EDITBOX -- */
int HelpTextProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case KEYBOARD:
            break;
        case PAINT:
            return HelpPaintMsg(wnd, p1, p2);
        case LEFT_BUTTON:
            return HelpLeftButtonMsg(wnd, p1, p2);
        case DOUBLE_CLICK:
            PostMessage(wnd, KEYBOARD, '\r', 0);
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

/* -------- read the help text into the editbox ------- */
static void ReadHelp(WINDOW wnd)
{
    WINDOW cwnd = ControlWindow(wnd->extension, ID_HELPTEXT);
    int linectr = 0;
    if (cwnd == NULL)
        return;
    thisword = KeyWords;
    keywordcount = 0;
    cwnd->wndproc = HelpTextProc;
    SendMessage(cwnd, CLEARTEXT, 0, 0);
    /* ----- read the help text ------- */
    while (TRUE)    {
        char *cp = hline, *cp1;
        int colorct = 0;
        if (GetHelpLine(hline) == NULL)
            break;
        if (*hline == '<')
            break;
        hline[strlen(hline)-1] = '\0';
        /* --- add help text to the help window --- */
        while (cp != NULL)    {
            if ((cp = strchr(cp, '[')) != NULL)    {
                /* ----- hit a new key word ----- */
                if (*(cp+1) != '.' && *(cp+1) != '*') {
                    cp++;
                    continue;
                }
                thisword->lineno = cwnd->wlines;
                thisword->off1 = (int) (cp - hline);
                thisword->off2 = thisword->off1 - colorct * 4;
                thisword->isDefinition = *(cp+1) == '*';
                colorct++;
                *cp++ = CHANGECOLOR;
                *cp++ = (wnd->WindowColors [HILITE_COLOR] [FG] & 255) | 0x80;
                *cp++ = (wnd->WindowColors [HILITE_COLOR] [BG] & 255) | 0x80;
                cp1 = cp;
                if ((cp = strchr(cp, ']')) != NULL)    {
                    if (thisword != NULL)
                        thisword->off3 =
                            thisword->off2 + (int) (cp - cp1);
                    *cp++ = RESETCOLOR;
                }
                if ((cp = strchr(cp, '<')) != NULL)    {
                    char *cp1 = strchr(cp, '>');
                    if (cp1 != NULL) {
                        char hname[80];
                        int len = (int) (cp1 - cp);
                        memset(hname, 0, 80);
                        strncpy(hname, cp+1, len-1);
                        thisword->hkey = FindHelp(hname);
                        memmove(cp, cp1+1, strlen(cp1));
                    }
                }
                thisword++;
                keywordcount++;
            }
        }
        PutItemText(wnd, ID_HELPTEXT, hline);
        /* -- display help text as soon as window is full -- */
        if (++linectr == ClientHeight(cwnd)) {
            struct keywords *holdthis = thisword;
            thisword = NULL;
            SendMessage(cwnd, PAINT, 0, 0);
            thisword = holdthis;
        }
        if (linectr > ClientHeight(cwnd) && !TestAttribute(cwnd, VSCROLLBAR)) {
            AddAttribute(cwnd, VSCROLLBAR);
            SendMessage(cwnd, BORDER, 0, 0);
        }
    }
    thisword = NULL;
}

/* ---- compute the displayed length of a help text line --- */
#if HELPLENGTH /* is not used here at all!? */
static int HelpLength(char *s)
{
    int len = strlen(s);
    char *cp = strchr(s, '[');
    while (cp != NULL) {
        len -= 4;
        cp = strchr(cp+1, '[');
    }
    cp = strchr(s, '<');
    while (cp != NULL) {
        char *cp1 = strchr(cp, '>');
        if (cp1 != NULL)
            len -= (int) (cp1-cp)+1;
        cp = strchr(cp1, '<');
    }
    return len;
}
#endif

/* ----------- load the help text file ------------ */
void LoadHelpFile(char *fname)
{
    long where;
    if (Helping)
        return;
    UnLoadHelpFile();
    if ((helpfp = OpenHelpFile(fname, "rb")) == NULL)
        return;
    strcpy(HelpFileName, fname);
    fseek(helpfp, - (long) sizeof(long), SEEK_END);
    fread(&where, sizeof(long), 1, helpfp);
    fseek(helpfp, where, SEEK_SET);
    fread(&HelpCount, sizeof(int), 1, helpfp);
    FirstHelp = DFcalloc(sizeof(struct helps) * HelpCount, 1);
    for (int i = 0; i < HelpCount; i++)	{
        int len;
        fread(&len, sizeof(int), 1, helpfp);
        if (len) {
            (FirstHelp+i)->hname = DFcalloc(len+1, 1);
            fread((FirstHelp+i)->hname, len+1, 1, helpfp);
        }
        fread(&len, sizeof(int), 1, helpfp);
        if (len)	{
            (FirstHelp+i)->comment = DFcalloc(len+1, 1);
            fread((FirstHelp+i)->comment, len+1, 1, helpfp);
        }
        fread(&(FirstHelp+i)->hptr, sizeof(int)*5+sizeof(long), 1, helpfp);
    }
    fclose(helpfp);
    helpfp = NULL;
}

/* ------ free the memory used by the help file table ------ */
void UnLoadHelpFile(void)
{
    for (int i = 0; i < HelpCount; i++)	{
        free((FirstHelp+i)->comment);
        free((FirstHelp+i)->hname);
    }
    free(FirstHelp);
    FirstHelp = NULL;
    free(HelpTree);
    HelpTree = NULL;
}

static void BuildHelpBox(WINDOW wnd)
{
    int offset;

    /* -- seek to the first line of the help text -- */
    SeekHelpLine(ThisHelp->hptr, ThisHelp->bit);
    /* ----- read the title ----- */
    GetHelpLine(hline);
    hline[strlen(hline)-1] = '\0';
    free(HelpBox.dwnd.title);
    HelpBox.dwnd.title = DFmalloc(strlen(hline)+1);
    strcpy(HelpBox.dwnd.title, hline);
    /* ----- set the height and width ----- */
    HelpBox.dwnd.h = min(ThisHelp->hheight, MAXHEIGHT)+7;
    HelpBox.dwnd.w = max(45, ThisHelp->hwidth+6);
    /* ------ position the help window ----- */
    if (wnd != NULL)
        BestFit(wnd, &HelpBox.dwnd);
    /* ------- position the command buttons ------ */
    HelpBox.ctl[0].dwnd.w = max(40, ThisHelp->hwidth+2);
    HelpBox.ctl[0].dwnd.h =
                min(ThisHelp->hheight, MAXHEIGHT)+2;
    offset = (HelpBox.dwnd.w-40) / 2;
    for (int i = 1; i < 5; i++)    {
        HelpBox.ctl[i].dwnd.y = min(ThisHelp->hheight, MAXHEIGHT)+3;
        HelpBox.ctl[i].dwnd.x = (i-1) * 10 + offset;
    }
    /* ---- disable ineffective buttons ---- */
    if (ThisHelp->nexthlp == -1)
        DisableButton(&HelpBox, ID_NEXT);
    else
        EnableButton(&HelpBox, ID_NEXT);
    if (ThisHelp->prevhlp == -1)
        DisableButton(&HelpBox, ID_PREV);
    else
        EnableButton(&HelpBox, ID_PREV);
}

/* ----- select a new help window from its name ----- */
static void SelectHelp(WINDOW wnd, struct helps *newhelp, BOOL recall)
{
    if (newhelp != NULL) {
        SendMessage(wnd, HIDE_WINDOW, 0, 0);
        if (recall && stacked < MAXHELPSTACK)
            HelpStack[stacked++] = ThisHelp-FirstHelp;
        ThisHelp = newhelp;
        SendMessage(GetParent(wnd), DISPLAY_HELP, (PARAM) ThisHelp->hname, 0);
        if (stacked)
            EnableButton(&HelpBox, ID_BACK);
        else
            DisableButton(&HelpBox, ID_BACK);
        BuildHelpBox(NULL);
        AddTitle(wnd, HelpBox.dwnd.title);
        /* --- reposition and resize the help window --- */
        HelpBox.dwnd.x = (SCREENWIDTH-HelpBox.dwnd.w)/2;
        HelpBox.dwnd.y = (SCREENHEIGHT-HelpBox.dwnd.h)/2;
        SendMessage(wnd, MOVE, HelpBox.dwnd.x, HelpBox.dwnd.y);
        SendMessage(wnd, SIZE, HelpBox.dwnd.x + HelpBox.dwnd.w - 1, HelpBox.dwnd.y + HelpBox.dwnd.h - 1);
        /* --- reposition the controls --- */
        for (int i = 0; i < 5; i++)    {
            WINDOW cwnd = HelpBox.ctl[i].wnd;
            int x = HelpBox.ctl[i].dwnd.x+GetClientLeft(wnd);
            int y = HelpBox.ctl[i].dwnd.y+GetClientTop(wnd);
            SendMessage(cwnd, MOVE, x, y);
            if (i == 0)	{
                x += HelpBox.ctl[i].dwnd.w - 1;
                y += HelpBox.ctl[i].dwnd.h - 1;
                SendMessage(cwnd, SIZE, x, y);
            }
        }
        /* --- read the help text into the help window --- */
        ReadHelp(wnd);
        ReFocus(wnd);
        SendMessage(wnd, SHOW_WINDOW, 0, 0);
    }
}
/* ---- strip tildes from the help name ---- */
static void StripTildes(char *fh, char *hp)
{
    while (*hp)	{
        if (*hp != '~')
            *fh++ = *hp;
        hp++;
    }
    *fh = '\0';
}
/* --- return the comment associated with a help window --- */
char *HelpComment(char *Help)
{
    char FixedHelp[30];
    StripTildes(FixedHelp, Help);
    if ((ThisHelp = FindHelp(FixedHelp)) != NULL)
        return ThisHelp->comment;
    return NULL;
}
/* ---------- display help text ----------- */
BOOL DisplayHelp(WINDOW wnd, char *Help)
{
    char FixedHelp[30];
    BOOL rtn = FALSE;

    if (Helping)
        return TRUE;
    StripTildes(FixedHelp, Help);
    stacked = 0;
    wnd->isHelping++;
    if ((ThisHelp = FindHelp(FixedHelp)) != NULL)	{
        if ((helpfp = OpenHelpFile(HelpFileName, "rb")) != NULL) {
            BuildHelpBox(wnd);
            DisableButton(&HelpBox, ID_BACK);
            /* ------- display the help window ----- */
            DialogBox(NULL, &HelpBox, TRUE, HelpBoxProc);
            free(HelpBox.dwnd.title);
            HelpBox.dwnd.title = NULL;
            fclose(helpfp);
            helpfp = NULL;
            rtn = TRUE;
        }
    }
    --wnd->isHelping;
    return rtn;
}

/* ------- display a definition window --------- */
static void DisplayDefinition(WINDOW wnd, char *def)
{
    WINDOW dwnd;
    WINDOW hwnd = wnd;
    int y;
    struct helps *HoldThisHelp;

    HoldThisHelp = ThisHelp;
    if (GetClass(wnd) == POPDOWNMENU)
        hwnd = GetParent(wnd);
    y = GetClass(hwnd) == MENUBAR ? 2 : 1;
    if ((ThisHelp = FindHelp(def)) != NULL)    {
        dwnd = CreateWindow(
                    TEXTBOX,
                    NULL,
                    GetClientLeft(hwnd),
                    GetClientTop(hwnd)+y,
                    min(ThisHelp->hheight, MAXHEIGHT)+3,
                    ThisHelp->hwidth+2,
                    NULL,
                    wnd,
                    NULL,
                    HASBORDER | NOCLIP | SAVESELF);
        if (dwnd != NULL)    {
            /* ----- read the help text ------- */
            SeekHelpLine(ThisHelp->hptr, ThisHelp->bit);
            while (TRUE)    {
                if (GetHelpLine(hline) == NULL)
                    break;
                if (*hline == '<')
                    break;
                hline[strlen(hline)-1] = '\0';
                SendMessage(dwnd,ADDTEXT,(PARAM)hline,0);
            }
            SendMessage(dwnd, SHOW_WINDOW, 0, 0);
            SendMessage(NULL, WAITKEYBOARD, 0, 0);
            SendMessage(NULL, WAITMOUSE, 0, 0);
            SendMessage(dwnd, CLOSE_WINDOW, 0, 0);
        }
    }
    ThisHelp = HoldThisHelp;
}

/* ------ compare help names with wild cards ----- */
static BOOL wildcmp(char *s1, char *s2)
{
    while (*s1 || *s2)    {
        if (tolower(*s1) != tolower(*s2))
            if (*s1 != '?' && *s2 != '?')
                return TRUE;
        s1++, s2++;
    }
    return FALSE;
}

/* --- ThisHelp = the help window matching specified name --- */
static struct helps *FindHelp(char *Help)
{
    struct helps *thishelp = NULL;
    for (int i = 0; i < HelpCount; i++)	{
        if (wildcmp(Help, (FirstHelp+i)->hname) == FALSE) {
            thishelp = FirstHelp+i;
            break;
        }
    }
    return thishelp;
}

static int OverLap(int a, int b)
{
    int ov = a - b;
    if (ov < 0)
        ov = 0;
    return ov;
}

/* ----- compute the best location for a help dialogbox ----- */
static void BestFit(WINDOW wnd, DIALOGWINDOW *dwnd)
{
    int above, below, right, left;
    if (GetClass(wnd) == MENUBAR ||
                GetClass(wnd) == APPLICATION)    {
        dwnd->x = dwnd->y = -1;
        return;
    }
    /* --- compute above overlap ---- */
    above = OverLap(dwnd->h, GetTop(wnd));
    /* --- compute below overlap ---- */
    below = OverLap(GetBottom(wnd), SCREENHEIGHT-dwnd->h);
    /* --- compute right overlap ---- */
    right = OverLap(GetRight(wnd), SCREENWIDTH-dwnd->w);
    /* --- compute left  overlap ---- */
    left = OverLap(dwnd->w, GetLeft(wnd));

    if (above < below)
        dwnd->y = max(0, GetTop(wnd)-dwnd->h-2);
    else
        dwnd->y = min(SCREENHEIGHT-dwnd->h, GetBottom(wnd)+2);
    if (right < left)
        dwnd->x = min(GetRight(wnd)+2, SCREENWIDTH-dwnd->w);
    else
        dwnd->x = max(0, GetLeft(wnd)-dwnd->w-2);

    if (dwnd->x == GetRight(wnd)+2 ||
            dwnd->x == GetLeft(wnd)-dwnd->w-2)
        dwnd->y = -1;
    if (dwnd->y ==GetTop(wnd)-dwnd->h-2 ||
            dwnd->y == GetBottom(wnd)+2)
        dwnd->x = -1;
}

/*  Key combinations

    Part of the FreeDOS Editor

*/

struct keys keys[] = {
    {F1,         "F1"},
    {F2,         "F2"},
    {F3,         "F3"},
    {F4,         "F4"},
    {F5,         "F5"},
    {F6,         "F6"},
    {F7,         "F7"},
    {F8,         "F8"},
    {F9,         "F9"},
    {F10,        "F10"},
    {CTRL_F1,    "Ctrl+F1"},
    {CTRL_F2,    "Ctrl+F2"},
    {CTRL_F3,    "Ctrl+F3"},
    {CTRL_F4,    "Ctrl+F4"},
    {CTRL_F5,    "Ctrl+F5"},
    {CTRL_F6,    "Ctrl+F6"},
    {CTRL_F7,    "Ctrl+F7"},
    {CTRL_F8,    "Ctrl+F8"},
    {CTRL_F9,    "Ctrl+F9"},
    {CTRL_F10,   "Ctrl+F10"},
    {ALT_F1,     "Alt+F1"},
    {ALT_F2,     "Alt+F2"},
    {ALT_F3,     "Alt+F3"},
    {ALT_F4,     "Alt+F4"},
    {ALT_F5,     "Alt+F5"},
    {ALT_F6,     "Alt+F6"},
    {ALT_F7,     "Alt+F7"},
    {ALT_F8,     "Alt+F8"},
    {ALT_F9,     "Alt+F9"},
    {ALT_F10,    "Alt+F10"},
    {HOME,       "Home"},
    {UP,         "Up"},
    {PGUP,       "PgUp"},
    {BS,         "BS"},
    {END,        "End"},
    {DN,         "Dn"},
    {PGDN,       "PgDn"},
    {INS,        "Ins"},
    {DEL,        "Del"},
    {CTRL_HOME,  "Ctrl+Home"},
    {CTRL_PGUP,  "Ctrl+PgUp"},
    {CTRL_BS,    "Ctrl+BS"},
    {CTRL_END,   "Ctrl+End"},
    {CTRL_PGDN,  "Ctrl+PgDn"},
    {SHIFT_HT,   "Shift+Tab"},
#ifdef HOOKKEYB
    {ALT_BS,     "Alt+BS"},
    {ALT_DEL,    "Alt+Del"},
    {SHIFT_DEL,  "Shift+Del"},
    {SHIFT_INS,  "Shift+Ins"},
    {CTRL_INS,   "Ctrl+Ins"},
#endif
    {ALT_A,      "Alt+A"},
    {ALT_B,      "Alt+B"},
    {ALT_C,      "Alt+C"},
    {ALT_D,      "Alt+D"},
    {ALT_E,      "Alt+E"},
    {ALT_F,      "Alt+F"},
    {ALT_G,      "Alt+G"},
    {ALT_H,      "Alt+H"},
    {ALT_I,      "Alt+I"},
    {ALT_J,      "Alt+J"},
    {ALT_K,      "Alt+K"},
    {ALT_L,      "Alt+L"},
    {ALT_M,      "Alt+M"},
    {ALT_N,      "Alt+N"},
    {ALT_O,      "Alt+O"},
    {ALT_P,      "Alt+P"},
    {ALT_Q,      "Alt+Q"},
    {ALT_R,      "Alt+R"},
    {ALT_S,      "Alt+S"},
    {ALT_T,      "Alt+T"},
    {ALT_U,      "Alt+U"},
    {ALT_V,      "Alt+V"},
    {ALT_W,      "Alt+W"},
    {ALT_X,      "Alt+X"},
    {ALT_Y,      "Alt+Y"},
    {ALT_Z,      "Alt+Z"},
    {CTRL_C,     "Ctrl+C"},
    {CTRL_V,     "Ctrl+V"},
    {CTRL_X,     "Ctrl+X"},
/* new 0.7c - note that this has to be the non-numpad style alt-digit... */
/* otherwise you have problems to distinguish alt-digit from ctrl-letter */
    {ALT_1,      "Alt+1"},
    {ALT_2,      "Alt+2"},
    {ALT_3,      "Alt+3"},
    {ALT_4,      "Alt+4"},
    {ALT_5,      "Alt+5"},
    {ALT_6,      "Alt+6"},
    {ALT_7,      "Alt+7"},
    {ALT_8,      "Alt+8"},
    {ALT_9,      "Alt+9"},
    {-1,         NULL}
};

/* ------------- calendar.c ------------- */

#define CALHEIGHT 17
#define CALWIDTH 33

static int DyMo[] = {31,28,31,30,31,30,31,31,30,31,30,31};
static struct tm ttm, ctm;
static int dys[42] = { 0 };
static WINDOW CalWnd = NULL;

#ifndef strftime
static char * nameOfMonth[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
#endif

/* returns 1 if year (0-based, not 1900-based) is a leap year (longer) */
/* -ea */
int isLeapYear(int year)
{
    if (!(year % 400))
        return 1; /* e.g. 2000 is a leap year */
    if (!(year % 100))
        return 0; /* e.g. 1900 and 2100 are not */
    if (!(year % 4))
        return 1; /* multiple of 4? Then it is a leap year */
    return 0;   /* default: not a leap year */
}

/* Fixes ttm.tm_mday for a given month and year,
 * and recomputes the day of week and day of year
 * values, as ANSI C mktime() does. Turbo C 2.01
 * has no mktime(), so...
 * (tm_mday, tm_mon, tm_year -> tm_wday, tm_yday).
 * wday 0: Sunday. yday 0: Jan 1st. year 0: 1900.
 * mon 0: Jan. Contributed by Eric Auer.
 */
static void FixDate(void)
{
    int i,j;
    /* ---- adjust Feb for leap year ---- */
    DyMo[1] = isLeapYear(1900 + ttm.tm_year) ? 29 : 28;
    
    /* enforce ranges: 1..?? for mday, 0..11 for mon */
    /* (old version just clipped mday value...) -ea  */
    while (ttm.tm_mday > DyMo[ttm.tm_mon]) {
        ttm.tm_mday -= DyMo[ttm.tm_mon];
        ttm.tm_mon++;
        if (ttm.tm_mon > 11) {
            ttm.tm_mon = 0;
            ttm.tm_year++;
        }
    }
    
    /* re-calculate yday in 0..??? range */
    ttm.tm_yday = 0;
    i = 0;
    while (i < ttm.tm_mon) {
        ttm.tm_yday += DyMo[i];
        i++;
    }
    ttm.tm_yday += ttm.tm_mday - 1; /* mday is 1 based! */
    
    /* 1st of January of 1900 (tm_year base) was a Monday (wday = 1) */
    i = 0; /* year-1900 */
    j = 1; /* wday of Jan 1 1900: Monday */
    if (ttm.tm_year >= 1980) { /* leap closer: DOS epoch shortcut */
        i = 80;	/* start scanning from 1980 */
        j = 2;	/* Jan 1 1980 was a Tuesday */
    }
    while (i < ttm.tm_year) {
        j += isLeapYear(i+1900) ? 2 : 1; /* shift 1 or 2 days each year */
        if (j>6)
            j -= 7; /* wrap back in range */
        i++;
    }
    ttm.tm_wday = (j + ttm.tm_yday) % 7; /* "day of year" helps us! */
}

/* ---- build calendar dates array ---- */
static void BuildDateArray(void)
{
    int offset, dy = 0;
    memset(dys, 0, sizeof dys);
    FixDate();
    /* ----- compute the weekday for the 1st ----- */
    offset = (ttm.tm_mday - ttm.tm_wday) % 7; // offset = ((ttm.tm_mday-1) - ttm.tm_wday) % 7; for Sun first
    if (offset < 0)
        offset += 7;
    if (offset)
        offset = (offset - 7) * -1;
    /* ----- build the dates into the array ---- */
    for (dy = 1; dy <= DyMo[ttm.tm_mon]; dy++)
        dys[offset++] = dy;
}

static void CalCreateWindowMsg(WINDOW wnd)
{
    DrawBox(wnd, 1, 2, CALHEIGHT-4, CALWIDTH-4);
    for (int x = 5; x < CALWIDTH-4; x += 4)
        DrawVector(wnd, x, 2, CALHEIGHT-4, FALSE);
    for (int y = 4; y < CALHEIGHT-3; y+=2)
        DrawVector(wnd, 1, y, CALWIDTH-4, TRUE);
}

static void DisplayDates(WINDOW wnd)
{
    char dyln[10];
    int offset;
    char banner[CALWIDTH-1];
    char banner1[30];
    
    SetStandardColor(wnd);
    PutWindowLine(wnd, "Mon Tue Wed Thu Fri Sat Sun", 2, 1);
    memset(banner, ' ', CALWIDTH-2);
    
#ifndef strftime /* why was this disabled? */
    sprintf(banner1, "%s, %i", nameOfMonth[ttm.tm_mon], 1900+ttm.tm_year);
#else
    strftime(banner1, 16, "%B, %Y", &ttm);
#endif
    
    offset = (CALWIDTH-2 - strlen(banner1)) / 2;
    strcpy(banner+offset, banner1);
    strcat(banner, "    ");
    PutWindowLine(wnd, banner, 0, 0);
    BuildDateArray();
    for (int week = 0; week < 6; week++)    {
        for (int day = 0; day < 7; day++)    {
            int dy = dys[week*7+day];
            if (dy == 0)
                strcpy(dyln, "   ");
            else    {
                sprintf(dyln, "%2d ", dy);
            }
            if ( (dy == ctm.tm_mday) && (ctm.tm_mon == ttm.tm_mon) )
                SetReverseColor(wnd);
            else
                SetStandardColor(wnd);
            
            PutWindowLine(wnd, dyln, 2 + day * 4, 3 + week*2);
        }
    }
}

static int CalcKeyboardMsg(WINDOW wnd, PARAM p1)
{
    switch ((int)p1)    {
        case PGUP:
            if (ttm.tm_mon == 0)    {
                ttm.tm_mon = 12;
                ttm.tm_year--;
            }
            ttm.tm_mon--;
            FixDate();
            DisplayDates(wnd);
            return TRUE;
        case PGDN:
            ttm.tm_mon++;
            if (ttm.tm_mon == 12)    {
                ttm.tm_mon = 0;
                ttm.tm_year++;
            }
            FixDate();
            DisplayDates(wnd);
            return TRUE;
        default:
            break;
    }
    return FALSE;
}

static int CalendarProc(WINDOW wnd,MESSAGE msg,
                        PARAM p1,PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            DefaultWndProc(wnd, msg, p1, p2);
            CalCreateWindowMsg(wnd);
            return TRUE;
        case KEYBOARD:
            if (CalcKeyboardMsg(wnd, p1))
                return TRUE;
            break;
        case PAINT:
            DefaultWndProc(wnd, msg, p1, p2);
            DisplayDates(wnd);
            return TRUE;
        case COMMAND:
            if ((int)p1 == ID_HELP)    {
                DisplayHelp(wnd, "Calendar");
                return TRUE;
            }
            break;
        case CLOSE_WINDOW:
            CalWnd = NULL;
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

void Calendar(WINDOW pwnd)
{
    if (CalWnd == NULL)    {
        time_t tim = time(NULL);
        ttm = *localtime(&tim);
        ctm = ttm;  /* store current calendar day and month */
        CalWnd = CreateWindow(PICTUREBOX,
                            "Calendar",
                            -1, -1, CALHEIGHT, CALWIDTH,
                            NULL, pwnd, CalendarProc,
                            SHADOW     |
                            MINMAXBOX  |
                            CONTROLBOX |
                            MOVEABLE   |
                            HASBORDER
                            );
    }
    SendMessage(CalWnd, SETFOCUS, TRUE, 0);
}


/* ------------- listbox.c ------------ */

#ifdef INCLUDE_EXTENDEDSELECTIONS
static int ExtendSelections(WINDOW, int, int);
static void TestExtended(WINDOW, PARAM);
static void ClearAllSelections(WINDOW);
static void SetSelection(WINDOW, int);
static void FlipSelection(WINDOW, int);
static void ClearSelection(WINDOW, int);
#else
#define TestExtended(w,p) /**/
#endif
static void ChangeSelection(WINDOW, int, int);
static void WriteSelection(WINDOW, int, int, RECT *);
static BOOL SelectionInWindow(WINDOW, int);

static int lpy = -1;    /* the previous y mouse coordinate */

#ifdef INCLUDE_EXTENDEDSELECTIONS
/* --------- SHIFT_F8 / CTRL_F8 Key ------------ */
static void AddModeKey(WINDOW wnd)
{
    if (isMultiLine(wnd))    {
        wnd->AddMode ^= TRUE;
        SendMessage(GetParent(wnd), ADDSTATUS,
            wnd->AddMode ? ((PARAM) "Add Mode") : 0, 0);
    }
}
#endif

/* --------- UP (Up Arrow) Key ------------ */
static void UpKey(WINDOW wnd, PARAM p2)
{
    if (wnd->selection > 0)    {
        if (wnd->selection == wnd->wtop)    {
            BaseWndProc(LISTBOX, wnd, KEYBOARD, UP, p2);
            PostMessage(wnd, LB_SELECTION, wnd->selection-1,
                isMultiLine(wnd) ? p2 : FALSE);
        } else {
            int newsel = wnd->selection-1;
            if (wnd->wlines == ClientHeight(wnd))
                while (*TextLine(wnd, newsel) == LINE)
                    --newsel;
            PostMessage(wnd, LB_SELECTION, newsel,
#ifdef INCLUDE_EXTENDEDSELECTIONS
                isMultiLine(wnd) ? p2 :
#endif
                FALSE);
        }
    }
}

/* --------- DN (Down Arrow) Key ------------ */
static void DnKey(WINDOW wnd, PARAM p2)
{
    if (wnd->selection < wnd->wlines-1)    {
        if (wnd->selection == wnd->wtop+ClientHeight(wnd)-1)  {
            BaseWndProc(LISTBOX, wnd, KEYBOARD, DN, p2);
            PostMessage(wnd, LB_SELECTION, wnd->selection+1,
                isMultiLine(wnd) ? p2 : FALSE);
        } else {
            int newsel = wnd->selection+1;
            if (wnd->wlines == ClientHeight(wnd))
                while (*TextLine(wnd, newsel) == LINE)
                    newsel++;
            PostMessage(wnd, LB_SELECTION, newsel,
#ifdef INCLUDE_EXTENDEDSELECTIONS
                isMultiLine(wnd) ? p2 :
#endif
                FALSE);
        }
    }
}

/* --------- HOME and PGUP Keys ------------ */
static void HomePgUpKey(WINDOW wnd, PARAM p1, PARAM p2)
{
    BaseWndProc(LISTBOX, wnd, KEYBOARD, p1, p2);
    PostMessage(wnd, LB_SELECTION, wnd->wtop,
#ifdef INCLUDE_EXTENDEDSELECTIONS
        isMultiLine(wnd) ? p2 :
#endif
        FALSE);
}

/* --------- END and PGDN Keys ------------ */
static void EndPgDnKey(WINDOW wnd, PARAM p1, PARAM p2)
{
    int bot;
    BaseWndProc(LISTBOX, wnd, KEYBOARD, p1, p2);
    bot = wnd->wtop+ClientHeight(wnd)-1;
    if (bot > wnd->wlines-1)
        bot = wnd->wlines-1;
    PostMessage(wnd, LB_SELECTION, bot,
#ifdef INCLUDE_EXTENDEDSELECTIONS
        isMultiLine(wnd) ? p2 :
#endif
        FALSE);
}

#ifdef INCLUDE_EXTENDEDSELECTIONS
/* --------- Space Bar Key ------------ */
static void SpacebarKey(WINDOW wnd, PARAM p2)
{
    if (isMultiLine(wnd))    {
        int sel = SendMessage(wnd, LB_CURRENTSELECTION, 0, 0);
        if (sel != -1)    {
            if (wnd->AddMode)
                FlipSelection(wnd, sel);
            if (ItemSelected(wnd, sel))    {
                if (!((int) p2 & (LEFTSHIFT | RIGHTSHIFT)))
                    wnd->AnchorPoint = sel;
                ExtendSelections(wnd, sel, (int) p2);
            }
            else
                wnd->AnchorPoint = -1;
            SendMessage(wnd, PAINT, 0, 0);
        }
    }
}
#endif

/* --------- Enter ('\r') Key ------------ */
static void EnterKey(WINDOW wnd)
{
    if (wnd->selection != -1)    {
        SendMessage(wnd, LB_SELECTION, wnd->selection, TRUE);
        SendMessage(wnd, LB_CHOOSE, wnd->selection, 0);
    }
}

/* --------- All Other Key Presses ------------ */
static void KeyPress(WINDOW wnd, PARAM p1, PARAM p2)
{
    int sel = wnd->selection+1;
    while (sel < wnd->wlines)    {
        char *cp = TextLine(wnd, sel);
        if (cp == NULL)
            break;
#ifdef INCLUDE_EXTENDEDSELECTIONS
        if (isMultiLine(wnd))
            cp++;
#endif
        if (tolower(*cp) == (int)p1)    {
            SendMessage(wnd, LB_SELECTION, sel,
                isMultiLine(wnd) ? p2 : FALSE);
            if (!SelectionInWindow(wnd, sel))    {
                wnd->wtop = sel-ClientHeight(wnd)+1;
                SendMessage(wnd, PAINT, 0, 0);
            }
            break;
        }
        sel++;
    }
}

/* --------- KEYBOARD Message ------------ */
static int ListKeyboardMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    switch ((int) p1)    {
#ifdef INCLUDE_EXTENDEDSELECTIONS
#ifdef HOOKKEYB
        case SHIFT_F8: /* only if HOOKKEYB */
#endif
        case CTRL_F8:
            AddModeKey(wnd);
            return TRUE;
#endif
        case UP:
            TestExtended(wnd, p2);
            UpKey(wnd, p2);
            return TRUE;
        case DN:
            TestExtended(wnd, p2);
            DnKey(wnd, p2);
            return TRUE;
        case PGUP:
        case HOME:
            TestExtended(wnd, p2);
            HomePgUpKey(wnd, p1, p2);
            return TRUE;
        case PGDN:
        case END:
            TestExtended(wnd, p2);
            EndPgDnKey(wnd, p1, p2);
            return TRUE;
#ifdef INCLUDE_EXTENDEDSELECTIONS
        case ' ':
            SpacebarKey(wnd, p2);
            break;
#endif
        case '\r':
            EnterKey(wnd);
            return TRUE;
        default:
            KeyPress(wnd, p1, p2);
            break;
    }
    return FALSE;
}

/* ------- LEFT_BUTTON Message -------- */
static int ListLeftButtonMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int my = (int) p2 - GetTop(wnd);
    if (my >= wnd->wlines-wnd->wtop)
        my = wnd->wlines - wnd->wtop;

    if (!InsideRect(p1, p2, ClientRect(wnd)))
        return FALSE;
    if (wnd->wlines && my != lpy)    {
        int sel = wnd->wtop+my-1;
#ifdef INCLUDE_EXTENDEDSELECTIONS
        int sh = getshift();
        if (!(sh & (LEFTSHIFT | RIGHTSHIFT)))    {
            if (!(sh & CTRLKEY))
                ClearAllSelections(wnd);
            wnd->AnchorPoint = sel;
            SendMessage(wnd, PAINT, 0, 0);
        }
#endif
        SendMessage(wnd, LB_SELECTION, sel, TRUE);
        lpy = my;
    }
    return TRUE;
}

/* ------------- DOUBLE_CLICK Message ------------ */
static int ListDoubleClickMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    if (WindowMoving || WindowSizing)
        return FALSE;
    if (wnd->wlines)    {
        RECT rc = ClientRect(wnd);
        BaseWndProc(LISTBOX, wnd, DOUBLE_CLICK, p1, p2);
        if (InsideRect(p1, p2, rc))
            SendMessage(wnd, LB_CHOOSE, wnd->selection, 0);
    }
    return TRUE;
}

/* ------------ ADDTEXT Message -------------- */
static int ListAddTextMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int rtn = BaseWndProc(LISTBOX, wnd, ADDTEXT, p1, p2);
    if (wnd->selection == -1)
        SendMessage(wnd, LB_SETSELECTION, 0, 0);
#ifdef INCLUDE_EXTENDEDSELECTIONS
    if (*(char *)p1 == LISTSELECTOR)
        wnd->SelectCount++;
#endif
    return rtn;
}

/* --------- GETTEXT Message ------------ */
static void ListGetTextMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    if ((int)p2 != -1)    {
        char *cp1 = (char *)p1;
        char *cp2 = TextLine(wnd, (int)p2);
        while (cp2 && *cp2 && *cp2 != '\n')
            *cp1++ = *cp2++;
        *cp1 = '\0';
    }
}

/* --------- LISTBOX Window Processing Module ------------ */
int ListBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            BaseWndProc(LISTBOX, wnd, msg, p1, p2);
            wnd->selection = -1;
#ifdef INCLUDE_EXTENDEDSELECTIONS
            wnd->AnchorPoint = -1;
#endif
            return TRUE;
        case KEYBOARD:
            if (WindowMoving || WindowSizing)
                break;
            if (ListKeyboardMsg(wnd, p1, p2))
                return TRUE;
            break;
        case LEFT_BUTTON:
            if (ListLeftButtonMsg(wnd, p1, p2) == TRUE)
                return TRUE;
            break;
        case DOUBLE_CLICK:
            if (ListDoubleClickMsg(wnd, p1, p2))
                return TRUE;
            break;
        case BUTTON_RELEASED:
            if (WindowMoving || WindowSizing || VSliding)
                break;
            lpy = -1;
            return TRUE;
        case ADDTEXT:
            return ListAddTextMsg(wnd, p1, p2);
        case LB_GETTEXT:
            ListGetTextMsg(wnd, p1, p2);
            return TRUE;
        case CLEARTEXT:
            wnd->selection = -1;
#ifdef INCLUDE_EXTENDEDSELECTIONS
            wnd->AnchorPoint = -1;
#endif
            wnd->SelectCount = 0;
            break;
        case PAINT:
            BaseWndProc(LISTBOX, wnd, msg, p1, p2);
            WriteSelection(wnd, wnd->selection, TRUE, (RECT *)p1);
            return TRUE;
        case SETFOCUS:
            BaseWndProc(LISTBOX, wnd, msg, p1, p2);
            if ((int)p1)
                WriteSelection(wnd, wnd->selection, TRUE, NULL);
            return TRUE;
        case SCROLL:
        case HORIZSCROLL:
        case SCROLLPAGE:
        case HORIZPAGE:
        case SCROLLDOC:
            BaseWndProc(LISTBOX, wnd, msg, p1, p2);
            WriteSelection(wnd,wnd->selection,TRUE,NULL);
            return TRUE;
        case LB_CHOOSE:
            SendMessage(GetParent(wnd), LB_CHOOSE, p1, p2);
            return TRUE;
        case LB_SELECTION:
            ChangeSelection(wnd, (int) p1, (int) p2);
            /* DFlat+ 1.0: not logic way of notifying parent of a selection */
            SendMessage(GetParent(wnd), LB_CHILDSELECTION, wnd->selection, 0);
            return TRUE;
        case LB_CURRENTSELECTION:
            return wnd->selection;
        case LB_SETSELECTION:
            ChangeSelection(wnd, (int) p1, 0);
            return TRUE;
#ifdef INCLUDE_EXTENDEDSELECTIONS
        case CLOSE_WINDOW:
            if (isMultiLine(wnd) && wnd->AddMode)    {
                wnd->AddMode = FALSE;
                SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
            }
            break;
#endif
        default:
            break;
    }
    return BaseWndProc(LISTBOX, wnd, msg, p1, p2);
}

static BOOL SelectionInWindow(WINDOW wnd, int sel)
{
    return (wnd->wlines && sel >= wnd->wtop && sel < wnd->wtop+ClientHeight(wnd));
}

static void WriteSelection(WINDOW wnd, int sel,
                                    int reverse, RECT *rc)
{
    if (isVisible(wnd))
        if (SelectionInWindow(wnd, sel))
            WriteTextLine(wnd, rc, sel, reverse);
}

#ifdef INCLUDE_EXTENDEDSELECTIONS
/* ----- Test for extended selections in the listbox ----- */
static void TestExtended(WINDOW wnd, PARAM p2)
{
    if (isMultiLine(wnd) && !wnd->AddMode &&
            !((int) p2 & (LEFTSHIFT | RIGHTSHIFT)))    {
        if (wnd->SelectCount > 1)    {
            ClearAllSelections(wnd);
            SendMessage(wnd, PAINT, 0, 0);
        }
    }
}

/* ----- Clear selections in the listbox ----- */
static void ClearAllSelections(WINDOW wnd)
{
    if (isMultiLine(wnd) && wnd->SelectCount > 0)    {
        int sel;
        for (sel = 0; sel < wnd->wlines; sel++)
            ClearSelection(wnd, sel);
    }
}

/* ----- Invert a selection in the listbox ----- */
static void FlipSelection(WINDOW wnd, int sel)
{
    if (isMultiLine(wnd))    {
        if (ItemSelected(wnd, sel))
            ClearSelection(wnd, sel);
        else
            SetSelection(wnd, sel);
    }
}

static int ExtendSelections(WINDOW wnd, int sel, int shift)
{    
    if (shift & (LEFTSHIFT | RIGHTSHIFT) && wnd->AnchorPoint != -1)    {
        int i = sel;
        int j = wnd->AnchorPoint;
        int rtn;
        if (j > i)
            swapi(i,j);
        rtn = i - j;
        while (j <= i)
            SetSelection(wnd, j++);
        return rtn;
    }
    return 0;
}

static void SetSelection(WINDOW wnd, int sel)
{
    if (isMultiLine(wnd) && !ItemSelected(wnd, sel))    {
        char *lp = TextLine(wnd, sel);
        *lp = LISTSELECTOR;
        wnd->SelectCount++;
    }
}

static void ClearSelection(WINDOW wnd, int sel)
{
    if (isMultiLine(wnd) && ItemSelected(wnd, sel))    {
        char *lp = TextLine(wnd, sel);
        *lp = ' ';
        --wnd->SelectCount;
    }
}

BOOL ItemSelected(WINDOW wnd, int sel)
{
    if (sel != -1 && isMultiLine(wnd) && sel < wnd->wlines)    {
        char *cp = TextLine(wnd, sel);
        return (int)((*cp) & 255) == LISTSELECTOR;
    }
    return FALSE;
}
#endif

static void ChangeSelection(WINDOW wnd,int sel,int shift)
{
    if (sel != wnd->selection)    {
#ifdef INCLUDE_EXTENDEDSELECTIONS
        if (sel != -1 && isMultiLine(wnd))        {
            int sels;
            if (!wnd->AddMode)
                ClearAllSelections(wnd);
            sels = ExtendSelections(wnd, sel, shift);
            if (sels > 1)
                SendMessage(wnd, PAINT, 0, 0);
            if (sels == 0 && !wnd->AddMode)    {
                ClearSelection(wnd, wnd->selection);
                SetSelection(wnd, sel);
                wnd->AnchorPoint = sel;
            }
        }
#endif
        WriteSelection(wnd, wnd->selection, FALSE, NULL);
        wnd->selection = sel;
        if (sel != -1)
            WriteSelection(wnd, sel, TRUE, NULL);
     }
}


/* --------------- lists.c -------------- */

/* ----- set focus to the next sibling ----- */
void SetNextFocus(void)
{
    if (inFocus != NULL)    {
        WINDOW wnd1 = inFocus, pwnd;
        while (TRUE) {
            pwnd = GetParent(wnd1);
            if (NextWindow(wnd1) != NULL)
                wnd1 = NextWindow(wnd1);
            else if (pwnd != NULL)
                wnd1 = FirstWindow(pwnd);
            if (wnd1 == NULL || wnd1 == inFocus)	{
                wnd1 = pwnd;
                break;
            }
            if (GetClass(wnd1) == STATUSBAR || GetClass(wnd1) == MENUBAR)
                continue;
            if (isVisible(wnd1))
                break;
        }
        if (wnd1 != NULL)	{
            while (wnd1->childfocus != NULL)
                wnd1 = wnd1->childfocus;
            if (wnd1->condition != ISCLOSING)
                SendMessage(wnd1, SETFOCUS, TRUE, 0);
        }
    }
}

/* ----- set focus to the previous sibling ----- */
void SetPrevFocus(void)
{
    if (inFocus != NULL)    {
        WINDOW wnd1 = inFocus, pwnd;
        while (TRUE) {
            pwnd = GetParent(wnd1);
            if (PrevWindow(wnd1) != NULL)
                wnd1 = PrevWindow(wnd1);
            else if (pwnd != NULL)
                wnd1 = LastWindow(pwnd);
            if (wnd1 == NULL || wnd1 == inFocus)	{
                wnd1 = pwnd;
                break;
            }
            if (GetClass(wnd1) == STATUSBAR)
                continue;
            if (isVisible(wnd1))
                break;
        }
        if (wnd1 != NULL)	{
            while (wnd1->childfocus != NULL)
                wnd1 = wnd1->childfocus;
            if (wnd1->condition != ISCLOSING)
                SendMessage(wnd1, SETFOCUS, TRUE, 0);
        }
    }
}

/* ------- move a window to the end of its parents list ----- */
/* Bad: This shuffles around the window list items, but we need */
/* it as the "next" relation of siblings determines the window  */
/* stacking: Z coordinate, "last" window is the topmost one...  */
void ReFocus(WINDOW wnd)
{
    if (GetParent(wnd) != NULL)	{
        RemoveWindow(wnd); /* splice out window from list:    */
        /* if it was a first-child, make the next one first,  */
        /* if it was a last-child, make the previous one last */
        AppendWindow(wnd); /* add window as last one in list: */
        /* if it has a parent w/o first-child, make it first, */
        /* if it has a parent, make it last-child...          */
        ReFocus(GetParent(wnd)); /* recurse until at top */
    }
}

/* ---- remove a window from the linked list ---- */
void RemoveWindow(WINDOW wnd)
{
    if (wnd != NULL)    {
        WINDOW pwnd = GetParent(wnd);
        if (PrevWindow(wnd) != NULL)
            NextWindow(PrevWindow(wnd)) = NextWindow(wnd);
        if (NextWindow(wnd) != NULL)
            PrevWindow(NextWindow(wnd)) = PrevWindow(wnd);
        if (pwnd != NULL)	{
            if (wnd == FirstWindow(pwnd))
                FirstWindow(pwnd) = NextWindow(wnd);
            if (wnd == LastWindow(pwnd))
                LastWindow(pwnd) = PrevWindow(wnd);
        }
    }
}

/* ---- append a window to the linked list ---- */
void AppendWindow(WINDOW wnd)
{
    if (wnd != NULL)    {
        WINDOW pwnd = GetParent(wnd);
        if (pwnd != NULL)	{
            if (FirstWindow(pwnd) == NULL)
                FirstWindow(pwnd) = wnd;
            if (LastWindow(pwnd) != NULL)
                NextWindow(LastWindow(pwnd)) = wnd;
            PrevWindow(wnd) = LastWindow(pwnd);
            LastWindow(pwnd) = wnd;
        }
        NextWindow(wnd) = NULL;
    }
}

/* ----- if document windows and statusbar or menubar get the focus,
              pass it on ------- */
void SkipApplicationControls(void)
{
    BOOL EmptyAppl = FALSE;
    int ct = 0;
    while (!EmptyAppl && inFocus != NULL)	{
        CLASS cl = GetClass(inFocus);
        if (cl == MENUBAR || cl == STATUSBAR)	{
            SetPrevFocus();
            EmptyAppl = (cl == MENUBAR && ct++);
        }
        else
            break;
    }
}

/* ------------ log.c ------------ */

enum dfplog_messages {
    /* ------------- Legacy Log dialog box ------------- */
    ID_LOGLIST = 5010,
    ID_LOGGING
};

static char *message[] = {
    #undef DFlatMsg
    #define DFlatMsg(m) " " #m,
    #include "textUI_Msg.h"
    NULL
};

static FILE *log_file = NULL;

/* ------------ Message Log dialog box -------------- */
DIALOGBOX(dbLog)
    DB_TITLE("Edit Message Log", -1, -1, 18, 41)
    CONTROL(TEXT,  "~Messages",   10,   1,  1,  8, ID_LOGLIST)
    CONTROL(LISTBOX,    NULL,     1,    2, 14, 26, ID_LOGLIST)
    CONTROL(TEXT,    "~Logging:", 29,   4,  1, 10, ID_LOGGING)
    CONTROL(CHECKBOX,    NULL,    31,   5,  1,  3, ID_LOGGING)
    CONTROL(BUTTON,  "   ~OK   ", 29,   7,  1,  8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ", 29,  10,  1,  8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ", 29,  13, 1,   8, ID_HELP)
ENDDB

#if 0
BOOL LogMessageStart (WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    if (log != NULL && message[msg][0] != ' ')
        fprintf(log,
            "%-20.20s %-12.12s %-20.20s, %5.5ld, %5.5ld\n",
            wnd ? (GetTitle(wnd) ? GetTitle(wnd) : "") : "",
            wnd ? ClassNames[GetClass(wnd)] : "",
            message[msg]+1, p1, p2);
    return TRUE;
}


BOOL LogMessageEnd ( WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2 )
{
    return TRUE;
}
#endif

static int LogProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    WINDOW cwnd = ControlWindow(&dbLog, ID_LOGLIST);
    char **mn = message;
    switch (msg)    {
        case INITIATE_DIALOG:
            AddAttribute(cwnd, MULTILINE | VSCROLLBAR);
            while (*mn)    {
                SendMessage(cwnd, ADDTEXT, (PARAM) (*mn), 0);
                mn++;
            }
            SendMessage(cwnd, SHOW_WINDOW, 0, 0);
            break;
        case COMMAND:
            if ((int) p1 == ID_OK)    {
                int item;
                int tl = GetTextLines(cwnd);
                for (item = 0; item < tl; item++)
                    if (ItemSelected(cwnd, item))
                        mn[item][0] = LISTSELECTOR;
            }
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

void MessageLog(WINDOW wnd)
{
    if (DialogBox(wnd, &dbLog, TRUE, LogProc))    {
        if (CheckBoxSetting(&dbLog, ID_LOGGING))    {
            log_file = fopen("DFLAT.LOG", "wt");
        } else if (log_file != NULL)    {
            fclose(log_file);
            log_file = NULL;
        }
    }
}


/* ------------ logger .c ------------ */


BOOL LogMessageStart(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2);
BOOL LogMessageEnd(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2);


char logstr[512];
log_levels CurrentLogLevel = LL_CRITICAL;
BOOL Debug_LogMessages = FALSE;
BOOL Debug_LogClockMessages = FALSE;

FILE *LogFile;


void Log ( log_levels L, char *ModuleName, char *message )
{
    time_t rawtime;
    char times[27];
    time_t t;
    struct tm tm;

    if ( (LogFile != NULL) && (L<=CurrentLogLevel) ) {
        t = time (NULL);
        tm = *localtime(&t);
        strcpy ( times, ctime (&rawtime)); times [24]= 0;
        fprintf (LogFile, "%4d%02d%02d %02d:%02d:%02d.%02d [%s] %s\n",
                tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec, 00, ModuleName, message);
        fflush ( LogFile );
    }
}

int  StartLogger ( char *LogFileName, log_levels Initial)
{
    LogFile = fopen (LogFileName, "awt");
    CurrentLogLevel = Initial;
    Log (LL_CRITICAL, "Logger", "--- Logging started ---");

    return ( LogFile != NULL);
}

int  StopLogger  ( void )
{
    Log (LL_CRITICAL, "Logger", "--- Logging finished ---");
    return  fclose (LogFile);
}

static char __s1[50];
char  *MessageText ( MESSAGE msg )
{
    switch (msg) {
        case START                  : return "START                  ";
        case STOP                   : return "STOP                   ";
        case COMMAND                : return "COMMAND                ";
        case CREATE_WINDOW          : return "CREATE_WINDOW          ";
        case OPEN_WINDOW            : return "OPEN_WINDOW            ";
        case SHOW_WINDOW            : return "SHOW_WINDOW            ";
        case HIDE_WINDOW            : return "HIDE_WINDOW            ";
        case CLOSE_WINDOW           : return "CLOSE_WINDOW           ";
        case SETFOCUS               : return "SETFOCUS               ";
        case KILLFOCUS              : return "KILLFOCUS              ";
        case PAINT                  : return "PAINT                  ";
        case BORDER                 : return "BORDER                 ";
        case TITLE                  : return "TITLE                  ";
        case MOVE                   : return "MOVE                   ";
        case SIZE                   : return "SIZE                   ";
        case MAXIMIZE               : return "MAXIMIZE               ";
        case MINIMIZE               : return "MINIMIZE               ";
        case RESTORE                : return "RESTORE                ";
        case INSIDE_WINDOW          : return "INSIDE_WINDOW          ";
        case DEADCHILD              : return "DEADCHILD              ";
        case CLOCKTICK              : return "CLOCKTICK              ";
        case CAPTURE_CLOCK          : return "CAPTURE_CLOCK          ";
        case RELEASE_CLOCK          : return "RELEASE_CLOCK          ";
        case KEYBOARD               : return "KEYBOARD               ";
        case CAPTURE_KEYBOARD       : return "CAPTURE_KEYBOARD       ";
        case RELEASE_KEYBOARD       : return "RELEASE_KEYBOARD       ";
        case KEYBOARD_CURSOR        : return "KEYBOARD_CURSOR        ";
        case CURRENT_KEYBOARD_CURSOR: return "CURRENT_KEYBOARD_CURSOR";
        case HIDE_CURSOR            : return "HIDE_CURSOR            ";
        case SHOW_CURSOR            : return "SHOW_CURSOR            ";
        case SAVE_CURSOR            : return "SAVE_CURSOR            ";
        case RESTORE_CURSOR         : return "RESTORE_CURSOR         ";
        case SHIFT_CHANGED          : return "SHIFT_CHANGED          ";
        case WAITKEYBOARD           : return "WAITKEYBOARD           ";
        case RESET_MOUSE            : return "RESET_MOUSE            ";
        case MOUSE_TRAVEL           : return "MOUSE_TRAVEL           ";
        case MOUSE_INSTALLED        : return "MOUSE_INSTALLED        ";
        case RIGHT_BUTTON           : return "RIGHT_BUTTON           ";
        case LEFT_BUTTON            : return "LEFT_BUTTON            ";
        case DOUBLE_CLICK           : return "DOUBLE_CLICK           ";
        case MOUSE_MOVED            : return "MOUSE_MOVED            ";
        case BUTTON_RELEASED        : return "BUTTON_RELEASED        ";
        case CURRENT_MOUSE_CURSOR   : return "CURRENT_MOUSE_CURSOR   ";
        case MOUSE_CURSOR           : return "MOUSE_CURSOR           ";
        case SHOW_MOUSE             : return "SHOW_MOUSE             ";
        case HIDE_MOUSE             : return "HIDE_MOUSE             ";
        case WAITMOUSE              : return "WAITMOUSE              ";
        case TESTMOUSE              : return "TESTMOUSE              ";
        case CAPTURE_MOUSE          : return "CAPTURE_MOUSE          ";
        case RELEASE_MOUSE          : return "RELEASE_MOUSE          ";
        case ADDTEXT                : return "ADDTEXT                ";
        case INSERTTEXT             : return "INSERTTEXT             ";
        case DELETETEXT             : return "DELETETEXT             ";
        case CLEARTEXT              : return "CLEARTEXT              ";
        case SETTEXT                : return "SETTEXT                ";
        case SCROLL                 : return "SCROLL                 ";
        case HORIZSCROLL            : return "HORIZSCROLL            ";
        case SCROLLPAGE             : return "SCROLLPAGE             ";
        case HORIZPAGE              : return "HORIZPAGE              ";
        case SCROLLDOC              : return "SCROLLDOC              ";
        case GETTEXT                : return "GETTEXT                ";
        case SETTEXTLENGTH          : return "SETTEXTLENGTH          ";
        case BUILDMENU              : return "BUILDMENU              ";
        case MB_SELECTION           : return "MB_SELECTION           ";
        case BUILD_SELECTIONS       : return "BUILD_SELECTIONS       ";
        case CLOSE_POPDOWN          : return "CLOSE_POPDOWN          ";
        case LB_SELECTION           : return "LB_SELECTION           ";
        case LB_CHILDSELECTION      : return "LB_CHILDSELECTION      ";
        case LB_CHOOSE              : return "LB_CHOOSE              ";
        case LB_CURRENTSELECTION    : return "LB_CURRENTSELECTION    ";
        case LB_GETTEXT             : return "LB_GETTEXT             ";
        case LB_SETSELECTION        : return "LB_SETSELECTION        ";
        case INITIATE_DIALOG        : return "INITIATE_DIALOG        ";
        case ENTERFOCUS             : return "ENTERFOCUS             ";
        case LEAVEFOCUS             : return "LEAVEFOCUS             ";
        case ENDDIALOG              : return "ENDDIALOG              ";
        case DISPLAY_HELP           : return "DISPLAY_HELP           ";
        case ADDSTATUS              : return "ADDSTATUS              ";
        case DRAWVECTOR             : return "DRAWVECTOR             ";
        case DRAWBOX                : return "DRAWBOX                ";
        case DRAWBAR                : return "DRAWBAR                ";
        default:  sprintf (__s1, "%i", msg); return __s1;
        }
}


static char __s2[50];
char  *ClassText ( CLASS cls )
{
    switch (cls) {
        case NORMAL     : return "NORMAL     ";
        case APPLICATION: return "APPLICATION";
        case TEXTBOX    : return "TEXTBOX    ";
        case LISTBOX    : return "LISTBOX    ";
        case GRAPHBOX   : return "GRAPHBOX   ";
        case LCDBOX     : return "LCDBOX     ";
        case EDITBOX    : return "EDITBOX    ";
        case MENUBAR    : return "MENUBAR    ";
        case POPDOWNMENU: return "POPDOWNMENU";
        case PICTUREBOX : return "PICTUREBOX ";
        case DIALOG     : return "DIALOG     ";
        case BOX        : return "BOX        ";
        case BUTTON     : return "BUTTON     ";
        case COMBOBOX   : return "COMBOBOX   ";
        case TEXT       : return "TEXT       ";
        case RADIOBUTTON: return "RADIOBUTTON";
        case CHECKBOX   : return "CHECKBOX   ";
        case SPINBUTTON : return "SPINBUTTON ";
        case ERRORBOX   : return "ERRORBOX   ";
        case MESSAGEBOX : return "MESSAGEBOX ";
        case HELPBOX    : return "HELPBOX    ";
        case STATUSBAR  : return "STATUSBAR  ";
        case EDITOR     : return "EDITOR     ";
        case TITLEBAR   : return "TITLEBAR   ";
        case DUMMY      : return "DUMMY      ";
        default: sprintf (__s2, "%i", cls); return __s2;
    }
}


static void LogMessage ( char * SectionName, WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2 )
{
    if ( wnd != NULL )
        Log5 (LL_NOTIFY, SectionName, "Wnd[%ld] Class[%s] Msg[%s], P1[%ld], P2[%ld]", (long)wnd, ClassText(GetClass(wnd)), MessageText(msg), p1, p2)
    else
        Log3 (LL_NOTIFY, SectionName, "SysMsg Msg[%s], P1[%ld], P2[%ld]", MessageText(msg), p1, p2);
}


BOOL LogMessageStart ( WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2 )
{
    if (Debug_LogMessages)
        if  (Debug_LogClockMessages ||  (msg != CLOCKTICK))
                    LogMessage("MsgSysStart", wnd, msg, p1, p2);
        return TRUE;
}


BOOL LogMessageEnd ( WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2 )
{
    if (Debug_LogMessages)
        if (Debug_LogClockMessages ||  (msg != CLOCKTICK) )
            LogMessage ( "MsgSysEnd", wnd, msg, p1, p2);
        return TRUE;
}


/* ------------- menu.c ------------- */

/* DFLat+ 1.0: Find command at the level of menu, not menubar */
static struct PopDown *FindCmd2(MENU *mn, int cmd)
{
    MENU *mnu = mn;
    while (mnu->Title != (void *)-1)    {
        struct PopDown *pd = mnu->Selections;
        if (pd == NULL) continue;
        while (pd->SelectionTitle != NULL) {
            if (pd->ActionId == cmd)
                return pd;
            pd++;
        }
        mnu++;
    }
    return NULL;
}

static struct PopDown *FindCmd(MBAR *mn, int cmd)
{
    return FindCmd2 (mn->PullDown, cmd);
}


char *GetCommandText(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        return pd->SelectionTitle;
    return NULL;
}

BOOL isCascadedCommand(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        return pd->Attrib & CASCADED;
    return FALSE;
}

/* DFlat+ 1.0: isCascadedCommand2 based on MENU, not MENUBAR */
BOOL isCascadedCommand2(MENU *mn, int cmd)
{
    struct PopDown *pd = FindCmd2(mn, cmd);

    if (pd != NULL)
        return !!(pd->Attrib & CASCADED);
      
    return FALSE;
}

void ActivateCommand(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        pd->Attrib &= ~INACTIVE;
}

void DeactivateCommand(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        pd->Attrib |= INACTIVE;
}

BOOL isActive(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        return !(pd->Attrib & INACTIVE);
    return FALSE;
}

BOOL GetCommandToggle(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        return (pd->Attrib & CHECKED) != 0;
    return FALSE;
}

void SetCommandToggle(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        pd->Attrib |= CHECKED;
}

void ClearCommandToggle(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        pd->Attrib &= ~CHECKED;
}

void InvertCommandToggle(MBAR *mn, int cmd)
{
    struct PopDown *pd = FindCmd(mn, cmd);
    if (pd != NULL)
        pd->Attrib ^= CHECKED;
}
/* ---------------- menubar.c ------------------ */

static void reset_menubar(WINDOW);

static struct {
    int x1, x2;     /* position in menu bar */
    char sc;        /* shortcut key value   */
} menu[10];

int mctr;

MBAR *ActiveMenuBar;
static MENU *ActiveMenu;

static WINDOW mwnd;

/* ----------- SETFOCUS Message ----------- */
static int MenuSetFocusMsg(WINDOW wnd, PARAM p1)
{
    int rtn;
    rtn = BaseWndProc(MENUBAR, wnd, SETFOCUS, p1, 0);
    if (!(int)p1)
        SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
    else
        SendMessage(NULL, HIDE_CURSOR, 0, 0);

    return rtn;
}

/* --------- BUILDMENU Message --------- */
static void BuildMenuMsg(WINDOW wnd, PARAM p1)
{
    int offset = 3;
    reset_menubar(wnd);
    mctr = 0;
    ActiveMenuBar = (MBAR *) p1;
    ActiveMenu = ActiveMenuBar->PullDown;
    while (ActiveMenu->Title != NULL && ActiveMenu->Title != (void*)-1)    {
        char *cp;
        if (strlen(GetText(wnd)+offset) < strlen(ActiveMenu->Title)+3)
            break;
        GetText(wnd) = DFrealloc(GetText(wnd),strlen(GetText(wnd))+5);
        memmove(GetText(wnd) + offset+4, GetText(wnd) + offset,strlen(GetText(wnd))-offset+1);
        CopyCommand(GetText(wnd)+offset,ActiveMenu->Title,FALSE, wnd->WindowColors [STD_COLOR] [BG]);
        menu[mctr].x1 = offset;
        offset += strlen(ActiveMenu->Title) + (3+MSPACE);
        menu[mctr].x2 = offset-MSPACE;
        cp = strchr(ActiveMenu->Title, SHORTCUTCHAR);
        if (cp)
            menu[mctr].sc = tolower(*(cp+1));
        mctr++;
        ActiveMenu++;
    }
    ActiveMenu = ActiveMenuBar->PullDown;
}

/* ---------- PAINT Message ---------- */
static void MenuPaintMsg(WINDOW wnd)
{
    if (wnd == inFocus)
        SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
    SetStandardColor(wnd);
    wputs(wnd, GetText(wnd), 0, 0);
    if (ActiveMenuBar->ActiveSelection != -1 &&
            (wnd == inFocus || mwnd != NULL))    {
        char *sel, *cp;
        int offset, offset1;

        sel = DFmalloc(200);
        offset=menu[ActiveMenuBar->ActiveSelection].x1;
        offset1=menu[ActiveMenuBar->ActiveSelection].x2;
        GetText(wnd)[offset1] = '\0';
        SetReverseColor(wnd);
        memset(sel, '\0', 200);
        strcpy(sel, GetText(wnd)+offset);
        cp = strchr(sel, CHANGECOLOR);
        if (cp != NULL)
            *(cp + 2) = background | 0x80;
        wputs(wnd, sel,
            offset-ActiveMenuBar->ActiveSelection*4, 0);
        GetText(wnd)[offset1] = ' ';
        if (mwnd == NULL && wnd == inFocus) {
            char *st = ActiveMenu
                [ActiveMenuBar->ActiveSelection].StatusText;
            if (st != NULL)
                SendMessage(GetParent(wnd), ADDSTATUS,
                    (PARAM)st, 0);
        }
        free(sel);
    }
}

/* ------------ KEYBOARD Message ------------- */
/* is the key one of the globally valid accel keys? */
static void MenuKeyboardMsg(WINDOW wnd, PARAM p1)
{
    MENU *mnu;
    int sel;

    if (mwnd == NULL)    {
        /* ----- search for menu bar shortcut keys ---- */
        int c = tolower((int)p1);
        int a = AltConvert((int)p1);
        int j;
        for (j = 0; j < mctr; j++)    {
            if ((inFocus == wnd && menu[j].sc == c) || (a && menu[j].sc == a))    {
                SendMessage(wnd, SETFOCUS, TRUE, 0);
                SendMessage(wnd, MB_SELECTION, j, 0);
                return;
            }
        }
    }
    /* -------- search for accelerator keys -------- */
    mnu = ActiveMenu;
    while (mnu->Title != (void *)-1)    {
        struct PopDown *pd = mnu->Selections;
        if (mnu->PrepMenu)
            (*(mnu->PrepMenu))(GetDocFocus(), mnu);
        sel = 0;
        while (pd->SelectionTitle != NULL)    {
            if (pd->Accelerator == (int) p1)    {
                if (pd->Attrib & INACTIVE)
                    beep();
                else    {
                    if (pd->Attrib & TOGGLE)
                        pd->Attrib ^= CHECKED;
                    /* wnd->mnu->Selection is only for highlighting  */
                    /* inside visible popdowns. however, we set the  */
                    /* CurrentMenuSelection for applicat.c, as extra */
                    /* parameter for the ID_WINDOW message... - 0.7c */
                    /* alternative would be using several ID_WINDOWx */
                    /* as actually *only* ID_WINDOW uses C.M.S. yet! */
                    CurrentMenuSelection = sel;
                    SendMessage(GetDocFocus(), SETFOCUS, TRUE, 0);
                    PostMessage(GetParent(wnd),COMMAND, pd->ActionId, 0);
                }
                return;
            }
            pd++;	/* search through all items of the popdown */
            sel++;
        }
        mnu++;	/* search through all possible popdowns */
    }
    switch ((int)p1)    {
        case F1: /* help inside menubar - possible context sensitive */
            if (ActiveMenu == NULL || ActiveMenuBar == NULL)
                break;
            sel = ActiveMenuBar->ActiveSelection;
            if (sel == -1)	{
                BaseWndProc(MENUBAR, wnd, KEYBOARD, F1, 0);
                return;
            }
            mnu = ActiveMenu+sel;
            if (mwnd == NULL || mnu->Selections[0].SelectionTitle == NULL) {
                SystemHelp(wnd,mnu->Title);
                return;
            }
            break;
        case DN:	/* suggested by Fox: down arrow to open sub-menu (0.7b) */
        case '\r':
            if (mwnd == NULL &&
                    ActiveMenuBar->ActiveSelection != -1)
                SendMessage(wnd, MB_SELECTION,
                    ActiveMenuBar->ActiveSelection, 0);
            break;
        case F10: /* F10, as ALT, toggles menu bar activation */
            if (wnd != inFocus && mwnd == NULL)    {
                SendMessage(wnd, SETFOCUS, TRUE, 0);
                if ( ActiveMenuBar->ActiveSelection == -1)
                    ActiveMenuBar->ActiveSelection = 0;
                SendMessage(wnd, PAINT, 0, 0);
                break;
            }
            /* ------- fall through ------- */
        case ESC:
            PostMessage (wnd, CLOSE_POPDOWN, TRUE, 0);
            break;

#ifdef HOOKKEYB
        case FWD: /* right arrow */
#else
	case RARROW: /* formerly called FWD */
#endif
            ActiveMenuBar->ActiveSelection++;
            if (ActiveMenuBar->ActiveSelection == mctr)
                ActiveMenuBar->ActiveSelection = 0;
            if (mwnd != NULL)
                SendMessage(wnd, MB_SELECTION,
                    ActiveMenuBar->ActiveSelection, 0);
            else 
                SendMessage(wnd, PAINT, 0, 0);
            break;
#ifndef HOOKKEYB
	case LARROW: /* hope that makes sense */
#endif
            if (ActiveMenuBar->ActiveSelection == 0 || ActiveMenuBar->ActiveSelection == -1)
                ActiveMenuBar->ActiveSelection = mctr;
            --ActiveMenuBar->ActiveSelection;
            if (mwnd != NULL)
                SendMessage(wnd, MB_SELECTION, ActiveMenuBar->ActiveSelection, 0);
            else
                SendMessage(wnd, PAINT, 0, 0);
            break;
        default:
            break;
    }
}

/* --------------- LEFT_BUTTON Message ---------- */
static void MenuLeftButtonMsg(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int i;
    int mx = (int) p1 - GetLeft(wnd);
    
    if ( p2 == GetTop(wnd))
        {
            /* --- compute the selection that the left button hit --- */
            for (i = 0; i < mctr; i++)
                if (mx >= menu[i].x1-4*i && mx <= menu[i].x2-4*i-5)
                    break;
            if (i < mctr)  {
                if (i != ActiveMenuBar->ActiveSelection || mwnd == NULL)
                    SendMessage(wnd, MB_SELECTION, i, 0);
            return;
        }
      }
      PostMessage (GetParent (wnd), msg, p1, p2);
      SendMessage (wnd, CLOSE_POPDOWN, FALSE, 0);
}

/* -------------- MB_SELECTION Message -------------- */
static void SelectionMsg(WINDOW wnd, PARAM p1)
{
    int wd;
    int offset = menu[(int)p1].x1 - 4 * (int)p1;
    MENU *mnu= ActiveMenu+(int)p1;

    /* Disable current pop-down (if any) and selection */
    SendMessage (wnd, CLOSE_POPDOWN, FALSE, 0);

    /* Prepare creation of pop-down */
    if (mnu->PrepMenu != NULL)
        (*(mnu->PrepMenu))(GetDocFocus(), mnu);

    wd = MenuWidth(mnu->Selections);

    if (offset > WindowWidth(wnd)-wd)
        offset = WindowWidth(wnd)-wd;

    /* Create and activate pop-down */
    mwnd = CreateWindow(POPDOWNMENU, NULL,
                GetLeft(wnd)+offset, GetTop(wnd)+1,
                MenuHeight(mnu->Selections),
                wd,
                NULL,
                wnd,
                NULL,
                SHADOW);

    if (mnu->Selections[0].SelectionTitle != NULL)    {
        SendMessage(mwnd, BUILD_SELECTIONS, (PARAM) mnu, 0);
        SendMessage(mwnd, SETFOCUS, TRUE, 0);
        SendMessage(mwnd, SHOW_WINDOW, 0, 0);
    }

    /* Repaint the menubar with the new situation */
    ActiveMenuBar->ActiveSelection = (int) p1;
    SendMessage(wnd, PAINT, 0, 0);

}

/* --------- COMMAND Message ---------- */
static void MenuCommandMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    if (p1 == ID_HELP)	{
        BaseWndProc(MENUBAR, wnd, COMMAND, p1, p2);
            return;
    }
    SendMessage(wnd, CLOSE_POPDOWN, TRUE,0);
  
    PostMessage(GetParent(wnd), COMMAND, p1, p2);
}

/* --------------- CLOSE_POPDOWN Message --------------- */
static void ClosePopdownMsg(WINDOW wnd, PARAM p1)
{
    if (mwnd == NULL)
        p1 = TRUE;

    if (mwnd != NULL)
        SendMessage (mwnd, CLOSE_WINDOW,0,0);

    if ( p1 )
    {
        ActiveMenuBar->ActiveSelection = -1;
        SendMessage(GetDocFocus(), SETFOCUS, TRUE, 0);
        SendMessage(wnd, PAINT, 0, 0);
    }
}

/* ---------------- CLOSE_WINDOW Message --------------- */
static void MenuCloseWindowMsg(WINDOW wnd)
{
    if (GetText(wnd) != NULL)    {
        free(GetText(wnd));
        GetText(wnd) = NULL;
    }
    mctr = 0;
    ActiveMenuBar->ActiveSelection = -1;
    ActiveMenu = NULL;
    ActiveMenuBar = NULL;
}

/* --- Window processing module for MENUBAR window class --- */
int MenuBarProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;

    switch (msg)    {
        case CREATE_WINDOW:
            reset_menubar(wnd);
            break;
        case SETFOCUS:
            return MenuSetFocusMsg(wnd, p1);
        case BUILDMENU:
            BuildMenuMsg(wnd, p1);
            break;
        case PAINT:    
            if (!isVisible(wnd) || GetText(wnd) == NULL)
                break;
            MenuPaintMsg(wnd);
            return FALSE;
        case BORDER:
            if (mwnd == NULL)
                SendMessage(wnd, PAINT, 0, 0);
            return TRUE;
        case KEYBOARD:
            MenuKeyboardMsg(wnd, p1);
            return TRUE;
        case MOUSE_MOVED:
            if (ActiveMenuBar->ActiveSelection != -1)
            {
                int i;
                int mx = (int) p1 - GetLeft(wnd);
                for (i = 0; i < mctr; i++)
                if (mx >= menu[i].x1-4*i && mx <= menu[i].x2-4*i-5)
                    break;

                if ( (i < mctr) && (p2 == GetTop (wnd)) && (i != ActiveMenuBar->ActiveSelection))  {
                    ActiveMenuBar->ActiveSelection=i;
                    if (mwnd != NULL)
                        SendMessage(wnd, MB_SELECTION, ActiveMenuBar->ActiveSelection, 0);
                    else
                        SendMessage(wnd, PAINT, 0, 0);
                }
            }
            break;
            
        case BUTTON_RELEASED:
        case LEFT_BUTTON:
            MenuLeftButtonMsg(wnd, msg, p1, p2);
            return TRUE;
        case MB_SELECTION:
            SelectionMsg(wnd, p1);
            break;
        case COMMAND:
            MenuCommandMsg(wnd, p1, p2);
            return TRUE;
        case CLOSE_POPDOWN:
            ClosePopdownMsg(wnd, p1);
            break;
        case DEADCHILD:
            mwnd = NULL;
            return TRUE;
        case CLOSE_WINDOW:
            rtn = BaseWndProc(MENUBAR, wnd, msg, p1, p2);
            MenuCloseWindowMsg(wnd);
            return rtn;
        default:
            break;
    }

    return BaseWndProc(MENUBAR, wnd, msg, p1, p2);
}

/* ------------- reset the MENUBAR -------------- */
static void reset_menubar(WINDOW wnd)
{
    GetText(wnd) = DFrealloc(GetText(wnd), SCREENWIDTH+5);
    memset(GetText(wnd), ' ', SCREENWIDTH);
    *(GetText(wnd)+WindowWidth(wnd)) = '\0';
}

WINDOW GetDocFocus(void)
{
    WINDOW wnd = ApplicationWindow;
    if (wnd != NULL) {
        wnd = LastWindow(wnd);
        while (wnd != NULL && (GetClass(wnd) == MENUBAR || GetClass(wnd) == STATUSBAR))
            wnd = PrevWindow(wnd);
        if (wnd != NULL)
            while (wnd->childfocus != NULL)
                wnd = wnd->childfocus;
    }

    return wnd ? wnd : ApplicationWindow;
}

/* --------- message.c ---------- */

#define BLINKING_CLOCK 1	/* set to 1 to make the clock blink */
                            /* blinking is off by default since 0.7c */
                            /* other defines: HOOKTIMER (use obsolete int 8 handler) and of course */
                            /* HOOKKEYB (use int 9 handler to allow ctrl/alt/shift - ins/del/bs)   */

static int mpx = 0, mpy = 0;
static int pmx = 0, pmy = 0;
static int mx = 0, my = 0;
static int handshaking = 0;
static volatile BOOL CriticalError;
BOOL AltDown = FALSE;


/* ---------- event queue ---------- */
static struct events    {
    MESSAGE event;
    union {
        PARAM mx;
        PARAM p1;
    };
    union {
        PARAM my;
        PARAM p2;
    };
} EventQueue[MAXMESSAGES];

/* ---------- message queue --------- */
static struct msgs {
    WINDOW wnd;
    MESSAGE msg;
    PARAM p1;
    PARAM p2;
} MsgQueue[MAXMESSAGES];

static int EventQueueOnCtr;
static int EventQueueOffCtr;
static int EventQueueCtr;

static int MsgQueueOnCtr;
static int MsgQueueOffCtr;
static int MsgQueueCtr;

static int lagdelay = FIRSTDELAY;

WINDOW CaptureMouse;
WINDOW CaptureKeyboard;
static BOOL NoChildCaptureMouse;
static BOOL NoChildCaptureKeyboard;

/* variables only contain an array index! */
static int doubletimer = 0;
static int delaytimer  = 1;
static int clocktimer  = 2;
static char timerused[3] = {0, 0, 0}; /* 0 means off, 2 timed out */
/* 1 counting */
static long unsigned int timerend[3] = {0, 0, 0};
static long unsigned int timerstart[3] = {0, 0, 0};
static long biostimer[1] = {0};
char time_string[] = "           "; /* max. length "12:34:56pm "; */

static WINDOW Cwnd;

/* More complex countdown handling by Eric Auer */
/* Allows us to work without hooking intr. 0x08 */

int timed_out(int timer) /* was: countdown 0? */
{
    if ((timer > 2) || (timer < 0))
        return -1; /* invalid -> always elapsed */
    if (timerused[timer] == 0) /* not active at all? */
        return 0;
    if (timerused[timer] == 2) /* timeout already known? */
        return 1;
    if (  (biostimer[0] < timerstart[timer])  /* wrapped? */
       || (biostimer[0] >= timerend[timer]) ) /* elapsed? */
    {
        /* could me more exact here - the "elapsed if wrapped"  */
        /* logics gives early timeout at midnight. On the other */
        /* hand, we do not know the ticks-per-day BIOS limit... */
        timerused[timer] = 2; /* countdown elapsed */
        return 1;
    }
    return 0; /* still waiting */
}

int timer_running(int timer) /* was: countdown > 0? */
{
    if ((timer > 2) || (timer < 0))
        return 0; /* invalid -> never running */
    if (timerused[timer] == 1) /* running? */
    {
        return (1 - timed_out(timer)); /* if not elapsed, running */
    }
    else return 0; /* certainly not running */
}

int timer_disabled(int timer) /* was: countdown -1? */
{
    if ((timer > 2) || (timer < 0))
        return 1; /* invalid -> always disabled */
    return (timerused[timer] == 0);
}

void disable_timer(int timer) /* was: countdown = -1 */
{
    if ((timer > 2) || (timer < 0))
        return;
    timerused[timer] = 0;
}

void set_timer(int timer, int secs)
{
    if ((timer > 2) || (timer < 0))
        return;
    timerstart[timer] = biostimer[0];
    timerend[timer] = timerstart[timer] + (secs*182UL/10) + 1;
    timerused[timer] = 1; /* mark as running */
}

void set_timer_ticks(int timer, int ticks)
{
    if ((timer > 2) || (timer < 0))
        return;
    timerstart[timer] = biostimer[0];
    timerend[timer] = timerstart[timer] + ticks;
    timerused[timer] = 1; /* mark as running */
}

static char ermsg[] = "Error accessing drive";

/* -------- test for critical errors --------- */
int TestCriticalError(void)
{
    int rtn = 0;
    if (CriticalError)    {
        beep();
        rtn = 1;
        CriticalError = FALSE;
        if (TestErrorMessage(ermsg) == FALSE)
            rtn = 2;
    }
    return rtn;
}

int crit_error(void);

/* ----- critical error handler ----- */
/* HINT: Whoever tests CriticalError should display errors as */
/* (errno < sys_nerr) ? sys_errlist[errno] : "Unknown error"  */
int crit_error(void)
{
    CriticalError = TRUE;
    hardretn(-1);		/* return an error!    */
    return 2;			/* is this correct???  */
}

void at_exit(void);
static void StopMsg(void);

/* ----- extra safety handler - avoid dangling int.vect. ----- */
void at_exit(void)
{
}

static void StopMsg(void)
{
    ClearClipboard();
    ClearDialogBoxes();
    restorecursor();
    unhidecursor();
    hide_mousecursor();
}

/* ------------ initialize the message system --------- */
BOOL init_messages(void)
{
    resetmouse();
    set_mousetravel(0, SCREENWIDTH-1, 0, SCREENHEIGHT-1);
    savecursor();
    hidecursor();
    mpx = mpy = 0;
    pmx = pmy = 0;
    mx = my = 0;
    CaptureMouse = CaptureKeyboard = NULL;
    NoChildCaptureMouse = FALSE;
    NoChildCaptureKeyboard = FALSE;
    MsgQueueOnCtr = MsgQueueOffCtr = MsgQueueCtr = 0;
    EventQueueOnCtr = EventQueueOffCtr = EventQueueCtr = 0;

    harderr(crit_error); /* set critical error handler (dos.h)  */
                         /* handler uses hardretn / hardresume  */

    atexit(at_exit); /* do not allow exit without restoring vectors! */

    PostMessage(NULL,START,0,0);
    lagdelay = FIRSTDELAY;
    return TRUE;
}

/* ----- post an event and parameters to event queue ---- */
static void PostEvent(MESSAGE event, PARAM p1, PARAM p2)
{
    if (EventQueueCtr != MAXMESSAGES)    {
        EventQueue[EventQueueOnCtr].event = event;
        EventQueue[EventQueueOnCtr].mx = p1;
        EventQueue[EventQueueOnCtr].my = p2;
        if (++EventQueueOnCtr == MAXMESSAGES)
            EventQueueOnCtr = 0;
        EventQueueCtr++;
    }
}

/* ------ collect mouse, clock, and keyboard events ----- */
static void collect_events(void)
{
    static int ShiftKeys = 0;
    int sk;
    struct tm *now;
#if BLINKING_CLOCK
    static BOOL flipflop = FALSE;
#endif
    int hr;

    /* -------- test for a clock event (one/second) ------- */
    // if (timed_out(clocktimer))    {
        time_t t = time(NULL);	/* the current time */
        char timesep = SysConfig.CountryTimeSeparator;	/* default 12:12:12am separator */
        int ampmflag = 0;   /* 1 ampm 0 24h clock */
#if BLINKING_CLOCK
        /* ------- blink the ':'s at one-second intervals ----- */
        flipflop = (!flipflop);
        if (flipflop) timesep = (timesep == ':') ? '.' : ' ';
            /* new in EDIT 0.7b: toggle ':' / '.' (or [other] / ' ') */
            /* which looks less 'nervous' than ':' / space toggling. */
#endif
        /* ----- get the current time ----- */
        now = localtime(&t);
        hr = now->tm_hour;
        if (ampmflag && (hr > 12)) hr -= 12;
        if (ampmflag && (hr == 0)) hr  = 12;
        if (ampmflag) {
          sprintf(time_string, "%2d%c%02d%c%02d%s", hr, timesep, now->tm_min,
            timesep, now->tm_sec,
            ((now->tm_hour > 11) ? "pm " : "am ") );
        } else
            sprintf(time_string, "%2d%c%02d%c%02d  ", hr, timesep, now->tm_min, timesep, now->tm_sec);
            /* min / sec use 02 digits, only 0.6 had 2 digits ' ' padded hours */
        /* -------- reset the timer -------- */
        set_timer(clocktimer, 1);
        /* -------- post the clock event -------- */
        PostEvent(CLOCKTICK, (PARAM)time_string, 0);
    // }

    /* --------- keyboard events ---------- */
    if ((sk = getshift()) != ShiftKeys)    {
        ShiftKeys = sk;
        /* ---- the shift status changed ---- */
        PostEvent(SHIFT_CHANGED, sk, 0);
        if (sk & ALTKEY)
            AltDown = TRUE;
    }

    if (keyhit())    {
        int c = getkey(); /* ASCII, or (FKEY | scancode) */
        AltDown = FALSE;
        /* ------ post the keyboard event ------ */
        PostEvent(KEYBOARD, c, sk);
    }

    /* ------------ test for mouse events --------- */
    if (button_releases())    {
        /* ------- the button was released -------- */
        AltDown = FALSE;
        set_timer_ticks(doubletimer, DOUBLETICKS); /* *** */
        PostEvent(BUTTON_RELEASED, mx, my);
        disable_timer(delaytimer);
    }
    get_mouseposition(&mx, &my);
    if (mx != mpx || my != mpy)  {
        mpx = mx;
        mpy = my;
        PostEvent(MOUSE_MOVED, mx, my);
    }
    if (rightbutton())	{
        AltDown = FALSE;
        PostEvent(RIGHT_BUTTON, mx, my);
    }
    if (leftbutton())    {
        AltDown = FALSE;
        if (mx == pmx && my == pmy)    {
            /* ---- same position as last left button ---- */
            if (timer_running(doubletimer))    {
                /* -- second click before double timeout -- */
                disable_timer(doubletimer);
                PostEvent(DOUBLE_CLICK, mx, my);
            } else if (!timer_running(delaytimer))    {
                /* ---- button held down a while ---- */
                set_timer_ticks(delaytimer, lagdelay); /* *** */
                lagdelay = DELAYTICKS;
                /* ---- post a typematic-like button ---- */
                PostEvent(LEFT_BUTTON, mx, my);
            }
        } else {
            /* --------- new button press ------- */
            disable_timer(doubletimer);
            set_timer_ticks(delaytimer, FIRSTDELAY); /* *** */
            lagdelay = DELAYTICKS;
            PostEvent(LEFT_BUTTON, mx, my);
            pmx = mx;
            pmy = my;
        }
    } else
        lagdelay = FIRSTDELAY;
}

/* ----- post a message and parameters to msg queue ---- */
void PostMessage(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    if (MsgQueueCtr != MAXMESSAGES)    {
        MsgQueue[MsgQueueOnCtr].wnd = wnd;
        MsgQueue[MsgQueueOnCtr].msg = msg;
        MsgQueue[MsgQueueOnCtr].p1 = p1;
        MsgQueue[MsgQueueOnCtr].p2 = p2;
        if (++MsgQueueOnCtr == MAXMESSAGES)
            MsgQueueOnCtr = 0;
        MsgQueueCtr++;
    }
}

/* --------- send a message to a window ----------- */
int SendMessage(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn = TRUE, x, y;


    LogMessageStart ( wnd, msg, p1, p2 );

    if (wnd != NULL)
        switch (msg)    {
            case PAINT:
            case BORDER:
                /* ------- don't send these messages unless the window is visible -------- */
                if (isVisible(wnd))
                    rtn = (*wnd->wndproc)(wnd, msg, p1, p2);
                break;
            case RIGHT_BUTTON:
            case LEFT_BUTTON:
            case DOUBLE_CLICK:
            case BUTTON_RELEASED:
                /* --- don't send these messages unless the window is visible or has captured the mouse -- */
                if (isVisible(wnd) || wnd == CaptureMouse)
                    rtn = (*wnd->wndproc)(wnd, msg, p1, p2);
                break;
            case KEYBOARD:
            case SHIFT_CHANGED:
                /* ------- don't send these messages unless the window is visible or has captured the keyboard -- */
                if (!(isVisible(wnd) || wnd == CaptureKeyboard))
                    break;
            default:
                rtn = (*wnd->wndproc)(wnd, msg, p1, p2);
                break;
        }
    /* ----- window processor returned true or the message was sent to no window at all (NULL) ----- */
    if (rtn != FALSE)    {
        /* --------- process messages that a window sends to the system itself ---------- */
        switch (msg)    {
            case STOP:
                StopMsg();
                break;
            /* ------- clock messages --------- */
            case CAPTURE_CLOCK:
                if (Cwnd == NULL)
                    set_timer(clocktimer, 0);
                wnd->PrevClock = Cwnd;
                Cwnd = wnd;
                break;
            case RELEASE_CLOCK:
                Cwnd = wnd->PrevClock;
                if (Cwnd == NULL)
                    disable_timer(clocktimer);
                break;
            /* -------- keyboard messages ------- */
            case KEYBOARD_CURSOR:
                if (wnd == NULL)
                    cursor((int)p1, (int)p2);
                else if (wnd == inFocus)
                    cursor(GetClientLeft(wnd)+(int)p1,
                                GetClientTop(wnd)+(int)p2);
                break;
            case CAPTURE_KEYBOARD:
                if (p2)
                    ((WINDOW)p2)->PrevKeyboard=CaptureKeyboard;
                else
                    wnd->PrevKeyboard = CaptureKeyboard;
                CaptureKeyboard = wnd;
                NoChildCaptureKeyboard = (int)p1;
                break;
            case RELEASE_KEYBOARD:
                if (wnd != NULL) {
                    if (CaptureKeyboard == wnd || (int)p1)
                        CaptureKeyboard = wnd->PrevKeyboard;
                    else	{
                        WINDOW twnd = CaptureKeyboard;
                        while (twnd != NULL) {
                            if (twnd->PrevKeyboard == wnd)	{
                                twnd->PrevKeyboard = wnd->PrevKeyboard;
                                break;
                            }
                            twnd = twnd->PrevKeyboard;
                        }
                        if (twnd == NULL)
                            CaptureKeyboard = NULL;
                    }
                    wnd->PrevKeyboard = NULL;
                }
                else
                    CaptureKeyboard = NULL;
                NoChildCaptureKeyboard = FALSE;
                break;
            case CURRENT_KEYBOARD_CURSOR:
                curr_cursor(&x, &y);
                *(int*)p1 = x;
                *(int*)p2 = y;
                break;
            case SAVE_CURSOR:
                savecursor();
                break;
            case RESTORE_CURSOR:
                restorecursor();
                break;
            case HIDE_CURSOR:
                normalcursor();
                hidecursor();
                break;
            case SHOW_CURSOR:
                if (p1)
                    set_cursor_type(0x0607);
                else
                    set_cursor_type(0x0106);

                unhidecursor();
                break;
            case WAITKEYBOARD:
                waitforkeyboard();
                break;
            /* -------- mouse messages -------- */
            case RESET_MOUSE:
                resetmouse();
                set_mousetravel(0, SCREENWIDTH-1, 0, SCREENHEIGHT-1);
                break;
            case MOUSE_INSTALLED:
                rtn = mouse_installed();
                break;
            case MOUSE_TRAVEL:
            {
                RECT rc;
                if (!p1)	{
                    rc.lf = rc.tp = 0;
                    rc.rt = SCREENWIDTH-1;
                    rc.bt = SCREENHEIGHT-1;
                }
                else
                    rc = *(RECT *)p1;
                set_mousetravel(rc.lf, rc.rt, rc.tp, rc.bt);
                break;
            }
            case SHOW_MOUSE:
                show_mousecursor();
                break;
            case HIDE_MOUSE:
                hide_mousecursor();
                break;
            case MOUSE_CURSOR:
                set_mouseposition((int)p1, (int)p2);
                break;
            case CURRENT_MOUSE_CURSOR:
                get_mouseposition((int*)p1,(int*)p2);
                break;
            case WAITMOUSE:
                waitformouse();
                break;
            case TESTMOUSE:
                rtn = mousebuttons();
                break;
            case CAPTURE_MOUSE:
                if (p2)
                    ((WINDOW)p2)->PrevMouse = CaptureMouse;
                else
                    wnd->PrevMouse = CaptureMouse;
                CaptureMouse = wnd;
                NoChildCaptureMouse = (int)p1;
                break;
            case RELEASE_MOUSE:
                if (wnd != NULL) {
                    if (CaptureMouse == wnd || (int)p1)
                        CaptureMouse = wnd->PrevMouse;
                    else	{
                        WINDOW twnd = CaptureMouse;
                        while (twnd != NULL)	{
                            if (twnd->PrevMouse == wnd)	{
                                twnd->PrevMouse = wnd->PrevMouse;
                                break;
                            }
                            twnd = twnd->PrevMouse;
                        }
                        if (twnd == NULL)
                            CaptureMouse = NULL;
                    }
                    wnd->PrevMouse = NULL;
                }
                else
                    CaptureMouse = NULL;
                NoChildCaptureMouse = FALSE;
                break;
            default:
                break;
        }
    }

    LogMessageEnd ( wnd, msg, p1, p2 );
    return rtn;
}

static RECT VisibleRect(WINDOW wnd)
{
    RECT rc = WindowRect(wnd);
    if (!TestAttribute(wnd, NOCLIP))	{
        WINDOW pwnd = GetParent(wnd);
        RECT prc;
        prc = ClientRect(pwnd);
        while (pwnd != NULL)	{
            if (TestAttribute(pwnd, NOCLIP))
                break;
            rc = subRectangle(rc, prc);
            if (!ValidRect(rc))
                break;
            if ((pwnd = GetParent(pwnd)) != NULL)
                prc = ClientRect(pwnd);
        }
    }
    return rc;
}

/* ----- find window that mouse coordinates are in --- */
static WINDOW inWindow(WINDOW wnd, int x, int y)
{
    WINDOW Hit = NULL;
    while (wnd != NULL) {
        if (isVisible(wnd)) {
            WINDOW wnd1;
            RECT rc = VisibleRect(wnd);
            if (InsideRect(x, y, rc))
                Hit = wnd;
            if ((wnd1 = inWindow(LastWindow(wnd), x, y)) != NULL)
                Hit = wnd1;
            if (Hit != NULL)
                break;
        }
        wnd = PrevWindow(wnd);
    }
    return Hit;
}

static WINDOW MouseWindow(int x, int y)
{
    /* ------ get the window in which a mouse event occurred ------ */
    WINDOW Mwnd = inWindow(ApplicationWindow, x, y);
    /* ---- process mouse captures ----- */
    if (CaptureMouse != NULL)	{
        if (NoChildCaptureMouse || Mwnd == NULL || !isAncestor(Mwnd, CaptureMouse))
            Mwnd = CaptureMouse;
    }
    return Mwnd;
}

void Cooperate(void)
{
    handshaking++;
    dispatch_message();
    --handshaking;
}

/* ---- dispatch messages to the message proc function ---- */
BOOL dispatch_message(void)
{
    WINDOW Mwnd, Kwnd;
    /* -------- collect mouse and keyboard events ------- */
    collect_events();

    /* only message.c can fill the event queue, but all components */
    /* can fill the message queue. Events come from user or clock. */
    if ( (EventQueueCtr == 0) && (MsgQueueCtr == 0) &&
        (handshaking == 0) ) {	/* BORED - new 0.7c */
    }

    /* --------- dequeue and process events -------- */
    while (EventQueueCtr > 0)  {
        struct events ev;

        ev = EventQueue[EventQueueOffCtr];
        if (++EventQueueOffCtr == MAXMESSAGES)
            EventQueueOffCtr = 0;
        --EventQueueCtr;

        /* ------ get the window in which a keyboard event occurred ------ */
        Kwnd = inFocus;

        /* ---- process keyboard captures ----- */
        if (CaptureKeyboard != NULL)
            if (Kwnd == NULL || NoChildCaptureKeyboard || !isAncestor(Kwnd, CaptureKeyboard))
                Kwnd = CaptureKeyboard;

        /* -------- send mouse and keyboard messages to the
            window that should get them -------- */
        switch (ev.event)    {
            case SHIFT_CHANGED:
            case KEYBOARD:
                if (!handshaking)
                    SendMessage(Kwnd, ev.event, ev.mx, ev.my);
                break;
            case LEFT_BUTTON:
                if (!handshaking)	{
                    Mwnd = MouseWindow(ev.mx, ev.my);
                    if (!CaptureMouse || (!NoChildCaptureMouse && isAncestor(Mwnd, CaptureMouse)))
                        if (Mwnd != inFocus)
                            SendMessage(Mwnd, SETFOCUS, TRUE, 0);
                    SendMessage(Mwnd, LEFT_BUTTON, ev.mx, ev.my);
                }
                break;
            case BUTTON_RELEASED:
            case DOUBLE_CLICK:
            case RIGHT_BUTTON:
                if (handshaking)
                    break;
            case MOUSE_MOVED:
                Mwnd = MouseWindow(ev.mx, ev.my);
                SendMessage(Mwnd, ev.event, ev.mx, ev.my);
                break;
            case CLOCKTICK:
                SendMessage(Cwnd, ev.event, ev.p1, ev.p2);
                break;
            default:
                break;
        }
    }
    /* ------ dequeue and process messages ----- */
    while (MsgQueueCtr > 0)  {
        struct msgs mq;

        mq = MsgQueue[MsgQueueOffCtr];
        if (++MsgQueueOffCtr == MAXMESSAGES)
            MsgQueueOffCtr = 0;
        --MsgQueueCtr;
        SendMessage(mq.wnd, mq.msg, mq.p1, mq.p2);
        if (mq.msg == ENDDIALOG)
            return FALSE;
        if (mq.msg == STOP)	{
            PostMessage(NULL, STOP, 0, 0);
            return FALSE;
        }
    }
    return TRUE;
}

void ProcessMessages (void)
{
    while (dispatch_message());	
}
/* ------------- mouse.c ------------- */

/* ---------- reset the mouse ---------- */
void resetmouse(void)
{
}

/* ----- test to see if the mouse driver is installed ----- */
BOOL mouse_installed(void)
{
    return TRUE;
}

/* ------ return true if mouse buttons are pressed ------- */
int mousebuttons(void)
{
    if (mouse_installed())	{
        return 0;
    }
    return 0;
}

/* ---------- return mouse coordinates ---------- */
void get_mouseposition(int *x, int *y)
{
    *x = *y = 0;
}

/* --- return true if a mouse button has been released --- */
int button_releases(void)
{
    if (mouse_installed())	{
        return 0;
    }
    return 0;
}

/* ------------------ msgbox.c ------------------ */

extern DBOX MsgBox;
extern DBOX InputBoxDB;
WINDOW CancelWnd;

static int ReturnValue;

int MessageBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            GetClass(wnd) = MESSAGEBOX;
            InitWindowColors(wnd);
            ClearAttribute(wnd, CONTROLBOX);
            break;
        case KEYBOARD:
            if (p1 == '\r' || p1 == ESC)
                ReturnValue = (int)p1;
            break;
        default:
            break;
    }
    return BaseWndProc(MESSAGEBOX, wnd, msg, p1, p2);
}

int YesNoBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            GetClass(wnd) = MESSAGEBOX;
            InitWindowColors(wnd);
            ClearAttribute(wnd, CONTROLBOX);
            break;
        case KEYBOARD:    {
            int c = tolower((int)p1);

            if ( p1 == LARROW ) p1 = SHIFT_HT; /* DFlat+ 1.0: for these, arrows are turned */
            if ( p1 == RARROW ) p1 = TAB;      /* into TABs */
            
            if (c == 'y')
                SendMessage(wnd, COMMAND, ID_OK, 0);
            else if (c == 'n')
                SendMessage(wnd, COMMAND, ID_CANCEL, 0);
            break;

        }
        default:
            break;
    }
    return BaseWndProc(MESSAGEBOX, wnd, msg, p1, p2);
}

int ErrorBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            GetClass(wnd) = ERRORBOX;
            InitWindowColors(wnd);
            break;
        case KEYBOARD:
            if (p1 == '\r' || p1 == ESC)
                ReturnValue = (int)p1;
            break;
        default:
            break;
    }
    return BaseWndProc(ERRORBOX, wnd, msg, p1, p2);
}

int CancelBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            CancelWnd = wnd;
            SendMessage(wnd, CAPTURE_MOUSE, 0, 0);
            SendMessage(wnd, CAPTURE_KEYBOARD, 0, 0);
            break;
        case COMMAND:
            if ((int) p1 == ID_CANCEL && (int) p2 == 0)
                SendMessage(GetParent(wnd), msg, p1, p2);
            return TRUE;
        case CLOSE_WINDOW:
            CancelWnd = NULL;
            SendMessage(wnd, RELEASE_MOUSE, 0, 0);
            SendMessage(wnd, RELEASE_KEYBOARD, 0, 0);
            p1 = TRUE;
            break;
        default:
            break;
    }
    return BaseWndProc(MESSAGEBOX, wnd, msg, p1, p2);
}

void CloseCancelBox(void)
{
    if (CancelWnd != NULL)
        SendMessage(CancelWnd, CLOSE_WINDOW, 0, 0);
}

static char *InputText;
static int TextLength;

int InputBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    switch (msg)    {
        case CREATE_WINDOW:
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            SendMessage(ControlWindow(&InputBoxDB,ID_INPUTTEXT),
                        SETTEXTLENGTH, TextLength, 0);
            SendMessage(ControlWindow(&InputBoxDB,ID_INPUTTEXT),
                        ADDTEXT, (PARAM) InputText, 0);
            return rtn;
        case COMMAND:
            if ((int) p1 == ID_OK && (int) p2 == 0)
                GetItemText(wnd, ID_INPUTTEXT,
                            InputText, TextLength);
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

BOOL InputBox(WINDOW wnd,char *ttl,char *msg,char *text,int len,int wd)
{
    int ln = wd ? wd : len;
    ln = min(SCREENWIDTH-8, ln);
    InputText = text;
    TextLength = len;
    InputBoxDB.dwnd.title = ttl;
    InputBoxDB.dwnd.w = 4 + max(20, max(ln, max(strlen(ttl), strlen(msg))));
    InputBoxDB.ctl[1].dwnd.x = (InputBoxDB.dwnd.w-2-ln)/2;
    InputBoxDB.ctl[0].dwnd.w = strlen(msg);
    InputBoxDB.ctl[0].itext = msg;
    InputBoxDB.ctl[1].itext = NULL;
    InputBoxDB.ctl[1].dwnd.w = ln;
    InputBoxDB.ctl[2].dwnd.x = (InputBoxDB.dwnd.w - 20) / 2;
    InputBoxDB.ctl[3].dwnd.x = InputBoxDB.ctl[2].dwnd.x + 10;
    InputBoxDB.ctl[2].isetting = ON;
    InputBoxDB.ctl[3].isetting = ON;
    return DialogBox(wnd, &InputBoxDB, TRUE, InputBoxProc);
}

BOOL GenericMessage(WINDOW wnd,char *ttl,char *msg,int buttonct,
      int (*wndproc)(struct window *,MESSAGE,PARAM,PARAM),
      char *b1, char *b2, int c1, int c2, int isModal)
{
    BOOL rtn;
    MsgBox.dwnd.title = ttl;
    MsgBox.ctl[0].dwnd.h = MsgHeight(msg);
    MsgBox.ctl[0].dwnd.w = max(max(MsgWidth(msg), buttonct*8 + buttonct + 2), strlen(ttl)+2);
    MsgBox.dwnd.h = MsgBox.ctl[0].dwnd.h+6;
    MsgBox.dwnd.w = MsgBox.ctl[0].dwnd.w+4;
    if (buttonct == 1)
        MsgBox.ctl[1].dwnd.x = (MsgBox.dwnd.w - 10) / 2;
    else    {
        MsgBox.ctl[1].dwnd.x = (MsgBox.dwnd.w - 20) / 2;
        MsgBox.ctl[2].dwnd.x = MsgBox.ctl[1].dwnd.x + 10;
        MsgBox.ctl[2].cls = BUTTON;
    }
    MsgBox.ctl[1].dwnd.y = MsgBox.dwnd.h - 4;
    MsgBox.ctl[2].dwnd.y = MsgBox.dwnd.h - 4;
    MsgBox.ctl[0].itext = msg;
    MsgBox.ctl[1].itext = b1;
    MsgBox.ctl[2].itext = b2;
    MsgBox.ctl[1].command = c1;
    MsgBox.ctl[2].command = c2;
    MsgBox.ctl[1].isetting = ON;
    MsgBox.ctl[2].isetting = ON;
    rtn = DialogBox(wnd, &MsgBox, isModal, wndproc);
    MsgBox.ctl[2].cls = 0;
    return rtn;
}

WINDOW MomentaryMessage(char *msg)
{
    WINDOW wnd = CreateWindow(
                    TEXTBOX,
                    NULL,
                    -1,-1,MsgHeight(msg)+2,MsgWidth(msg)+2,
                    NULL,NULL,NULL,
                    HASBORDER | SHADOW | SAVESELF);
    SendMessage(wnd, SETTEXT, (PARAM) msg, 0);
    if (!SysConfig.VideoCurrentColorScheme.isMonoScheme)    {
        WindowClientColor(wnd, WHITE, GREEN);
        WindowFrameColor(wnd, WHITE, GREEN);
    }
    SendMessage(wnd, SHOW_WINDOW, 0, 0);
    return wnd;
}

int MsgHeight(char *msg)
{
    int h = 1;
    while ((msg = strchr(msg, '\n')) != NULL)    {
        h++;
        msg++;
    }
    return min(h, SCREENHEIGHT-10);
}

int MsgWidth(char *msg)
{
    int w = 0;
    char *cp = msg;
    while ((cp = strchr(msg, '\n')) != NULL)    {
        w = max(w, (int) (cp-msg));
        msg = cp+1;
    }
    return min(max(strlen(msg),w), SCREENWIDTH-10);
}

/* ------------- normal.c ------------ */

#ifdef INCLUDE_MULTI_WINDOWS
static void PaintOverLappers(WINDOW wnd);
static void PaintUnderLappers(WINDOW wnd);
#endif

static BOOL InsideWindow(WINDOW, int, int);
static void TerminateMoveSize(void);
static void SaveBorder(RECT);
static void RestoreBorder(RECT);
static void GetVideoBuffer(WINDOW);
static void PutVideoBuffer(WINDOW);
#ifdef INCLUDE_MINIMIZE
static RECT PositionIcon(WINDOW);
#endif
static void dragborder(WINDOW, int, int);
static void sizeborder(WINDOW, int, int);
static int px = 0, py = 0;
static int diff;
static struct window dwnd = {DUMMY, NULL, NormalProc,
                                {-1,-1,-1,-1}};
static con_char_t *Bsave;
static con_char_t Bht, Bwd;
BOOL WindowMoving;
BOOL WindowSizing;
/* -------- array of class definitions -------- */
CLASSDEFS classdefs[] = {
    #undef ClassDef
    #define ClassDef(c,b,p,a) {b,p,a},
    #include "textUI_Classes.h"
};
WINDOW HiddenWindow;



char *ClassNames[] = {
    #undef ClassDef
    #define ClassDef(c,b,p,a) #c,
	#include "textUI_Classes.h"
    NULL
};

/* --------- CREATE_WINDOW Message ---------- */
static void CreateWindowMsg(WINDOW wnd)
{
#if CLASSIC_WINDOW_NUMBERING
    AppendWindow(wnd);
#else /* new 0.7c: stacking-independent window numbering */
    WINDOW pwnd = GetParent(wnd);
    /* printf("CREATE %p\n", wnd); getkey(); */
    if ((pwnd != NULL) && 
        (GetClass(pwnd) == APPLICATION)) {	/* only SUCH windows */
        if (NumberOneChildWindow(pwnd) == NULL) {
            NumberOneChildWindow(pwnd) = wnd;	/* add as first window */
        } else {
            WINDOW scanwnd = NumberOneChildWindow(pwnd);
            while ((scanwnd != NULL) && (NextNumberedWindow(scanwnd) != NULL)) {
                /* printf("CREATE LOOP: %p->%p\n", scanwnd, */
                /* NextNumberedWindow(scanwnd)); getkey();  */
                scanwnd = NextNumberedWindow(scanwnd);
            }
            if (scanwnd != NULL)
                NextNumberedWindow(scanwnd) = wnd; /* append this window */
        }
        NextNumberedWindow(wnd) = NULL;		/* end of the list */
    }
    /* printf("CREATE %p CALLING APPEND\n", wnd); getkey(); */
    AppendWindow(wnd);
    /* printf("CREATE %p DONE\n", wnd); getkey(); */
#endif
    if (!SendMessage(NULL, MOUSE_INSTALLED, 0, 0))
        ClearAttribute(wnd, VSCROLLBAR | HSCROLLBAR);
    if (TestAttribute(wnd, SAVESELF) && isVisible(wnd))
        GetVideoBuffer(wnd);
}

/* --------- SHOW_WINDOW Message ---------- */
static void ShowWindowMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    if (GetParent(wnd) == NULL || isVisible(GetParent(wnd)))    {
        WINDOW cwnd;
        if (TestAttribute(wnd, SAVESELF) && wnd->videosave == NULL)
            GetVideoBuffer(wnd);
        SetVisible(wnd);
        SendMessage(wnd, PAINT, 0, TRUE);
        SendMessage(wnd, BORDER, 0, 0);
        /* --- show the children of this window --- */
        cwnd = FirstWindow(wnd);
        while (cwnd != NULL) {
            if (cwnd->condition != ISCLOSING)
                SendMessage(cwnd, SHOW_WINDOW, p1, p2);
            cwnd = NextWindow(cwnd);
        }
    }
}

/* --------- HIDE_WINDOW Message ---------- */
static void HideWindowMsg(WINDOW wnd)
{
    if (isVisible(wnd))    {
        ClearVisible(wnd);
        /* --- paint what this window covered --- */
        if (TestAttribute(wnd, SAVESELF))
            PutVideoBuffer(wnd);
#ifdef INCLUDE_MULTI_WINDOWS
        else
            PaintOverLappers(wnd);
#endif
        wnd->wasCleared = FALSE;
    }
}

/* --------- KEYBOARD Message ---------- */
static BOOL KeyboardMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    if (WindowMoving || WindowSizing)    {
        /* -- move or size a window with keyboard -- */
        int x=WindowMoving?GetLeft(&dwnd):GetRight(&dwnd);
        int y=WindowMoving?GetTop(&dwnd):GetBottom(&dwnd);
        switch ((int)p1)    {
            case ESC:
                TerminateMoveSize();
                return TRUE;
            case UP:
                if (y)
                    --y;
                break;
            case DN:
                if (y < SCREENHEIGHT-1)
                    y++;
                break;
#ifdef HOOKKEYB
            case FWD: /* right arrow */
#else
            case RARROW: /* formerly called FWD */
#endif
                if (x < SCREENWIDTH-1)
                    x++;
                break;
#ifndef HOOKKEYB
            case LARROW: /* hope this makes sense */
#endif
            case BS: /* backspace implies going left... */
                if (x)
                    --x;
                break;
            case '\r':
                SendMessage(wnd,BUTTON_RELEASED,x,y);
            default:
                return TRUE;
        }
        /* -- use the mouse functions to move/size - */
        SendMessage(wnd, MOUSE_CURSOR, x, y);
        SendMessage(wnd, MOUSE_MOVED, x, y);
        return TRUE;
    }
    switch ((int)p1)    {
        case F1:
            SendMessage(wnd, COMMAND, ID_HELP, 0);
            return TRUE;
        case ' ':
            if ((int)p2 & ALTKEY)
                if (TestAttribute(wnd, HASTITLEBAR))
                    if (TestAttribute(wnd, CONTROLBOX))
                        BuildSystemMenu(wnd);
            return TRUE;
        case CTRL_F4:
            if (TestAttribute(wnd, CONTROLBOX))	{
                SendMessage(wnd, CLOSE_WINDOW, 0, 0);
                SkipApplicationControls();
                return TRUE;
            }
            break;
        default:
            break;
    }
    return FALSE;
}

/* --------- COMMAND Message ---------- */
static void CommandMsg(WINDOW wnd, PARAM p1)
{
    switch ((int)p1)    {
        case ID_HELP:
            SystemHelp(wnd,ClassNames[GetClass(wnd)]);
            break;
#ifdef INCLUDE_RESTORE
        case ID_SYSRESTORE:
            SendMessage(wnd, RESTORE, 0, 0);
            break;
#endif
        case ID_SYSMOVE:
            SendMessage(wnd, CAPTURE_MOUSE, TRUE,
                (PARAM) &dwnd);
            SendMessage(wnd, CAPTURE_KEYBOARD, TRUE,
                (PARAM) &dwnd);
            SendMessage(wnd, MOUSE_CURSOR,
                GetLeft(wnd), GetTop(wnd));
            WindowMoving = TRUE;
            dragborder(wnd, GetLeft(wnd), GetTop(wnd));
            break;
        case ID_SYSSIZE:
            SendMessage(wnd, CAPTURE_MOUSE, TRUE,
                (PARAM) &dwnd);
            SendMessage(wnd, CAPTURE_KEYBOARD, TRUE,
                (PARAM) &dwnd);
            SendMessage(wnd, MOUSE_CURSOR,
                GetRight(wnd), GetBottom(wnd));
            WindowSizing = TRUE;
            dragborder(wnd, GetLeft(wnd), GetTop(wnd));
            break;
#ifdef INCLUDE_MINIMIZE
        case ID_SYSMINIMIZE:
            SendMessage(wnd, MINIMIZE, 0, 0);
            break;
#endif
#ifdef INCLUDE_MAXIMIZE
        case ID_SYSMAXIMIZE:
            SendMessage(wnd, MAXIMIZE, 0, 0);
            break;
#endif
        case ID_SYSCLOSE:
            SendMessage(wnd, CLOSE_WINDOW, 0, 0);
            SkipApplicationControls();
            break;
        default:
            break;
    }
}

/* --------- SETFOCUS Message ---------- */
static void SetFocusMsg(WINDOW wnd, PARAM p1)
{
    RECT rc = {0,0,0,0};
    if (p1 && wnd != NULL && inFocus != wnd)    {
        WINDOW This, thispar;
        WINDOW that = NULL, thatpar = NULL;

        WINDOW cwnd = wnd, fwnd = GetParent(wnd);
        /* ---- post focus in ancestors ---- */
        while (fwnd != NULL)	{
            fwnd->childfocus = cwnd;
            cwnd = fwnd;
            fwnd = GetParent(fwnd);
        }
        /* ---- de-post focus in self and children ---- */
        fwnd = wnd;
        while (fwnd != NULL)	{
            cwnd = fwnd->childfocus;
            fwnd->childfocus = NULL;
            fwnd = cwnd;
        }

        This = wnd;
        that = thatpar = inFocus;

        /* ---- find common ancestor of prev focus and this window --- */
        while (thatpar != NULL)	{
            thispar = wnd;
            while (thispar != NULL)	{
                if (This == CaptureMouse || This == CaptureKeyboard)	{
                    /* ---- don't repaint if this window has capture ---- */
                    that = thatpar = NULL;
                    break;
                }
                if (thispar == thatpar)	{
                    /* ---- don't repaint if SAVESELF window had focus ---- */
                    if (This != that && TestAttribute(that, SAVESELF))
                        that = thatpar = NULL;
                    break;
                }
                This = thispar;
                thispar = GetParent(thispar);
            }
            if (thispar != NULL)
                break;
            that = thatpar;
            thatpar = GetParent(thatpar);
        }
        if (inFocus != NULL)
            SendMessage(inFocus, SETFOCUS, FALSE, 0);
        inFocus = wnd;
        if (that != NULL && isVisible(wnd))	{
            rc = subRectangle(WindowRect(that), WindowRect(This));
            if (!ValidRect(rc))	{
                if (ApplicationWindow != NULL)	{
                    WINDOW fwnd = FirstWindow(ApplicationWindow);
                    while (fwnd != NULL)	{
                        if (!isAncestor(wnd, fwnd))	{
                            rc = subRectangle(WindowRect(wnd),WindowRect(fwnd));
                            if (ValidRect(rc))
                                break;
                        }
                        fwnd = NextWindow(fwnd);
                    }
                }
            }
        }
        if (that != NULL && !ValidRect(rc) && isVisible(wnd))
            This = NULL;
        ReFocus(wnd);
        if (This != NULL && (!isVisible(This) || !TestAttribute(This, SAVESELF))) {
            wnd->wasCleared = FALSE;
            SendMessage(This, SHOW_WINDOW, 0, 0);
        } else if (!isVisible(wnd))
            SendMessage(wnd, SHOW_WINDOW, 0, 0);
        else
            SendMessage(wnd, BORDER, 0, 0);
    } else if (!p1 && inFocus == wnd) {
        /* -------- clearing focus --------- */
        inFocus = NULL;
        SendMessage(wnd, BORDER, 0, 0);
    }
}

/* --------- DOUBLE_CLICK Message ---------- */
static void DoubleClickMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int mx = (int) p1 - GetLeft(wnd);
    int my = (int) p2 - GetTop(wnd);
    if (!WindowSizing && !WindowMoving)	{
        if (HitControlBox(wnd, mx, my))	{
            PostMessage(wnd, CLOSE_WINDOW, 0, 0);
            SkipApplicationControls();
        }
    }
}

/* --------- LEFT_BUTTON Message ---------- */
static void LeftButtonMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int mx = (int) p1 - GetLeft(wnd);
    int my = (int) p2 - GetTop(wnd);
    if (WindowSizing || WindowMoving)
        return;
    if (HitControlBox(wnd, mx, my))    {
        BuildSystemMenu(wnd);
        return;
    }
    if (my == 0 && mx > -1 && mx < WindowWidth(wnd))  {
        /* ---------- hit the top border -------- */
        if (TestAttribute(wnd, MINMAXBOX) &&
                TestAttribute(wnd, HASTITLEBAR))  {
            if (mx == WindowWidth(wnd)-2)    {
                if (wnd->condition != ISRESTORED)
                    /* --- hit the restore box --- */
                    SendMessage(wnd, RESTORE, 0, 0);
#ifdef INCLUDE_MAXIMIZE
                else
                    /* --- hit the maximize box --- */
                    SendMessage(wnd, MAXIMIZE, 0, 0);
#endif
                return;
            }
#ifdef INCLUDE_MINIMIZE
            if (mx == WindowWidth(wnd)-3)    {
                /* --- hit the minimize box --- */
                if (wnd->condition != ISMINIMIZED)
                    SendMessage(wnd, MINIMIZE, 0, 0);
                return;
            }
#endif
        }
#ifdef INCLUDE_MAXIMIZE
        if (wnd->condition == ISMAXIMIZED)
            return;
#endif
        if (TestAttribute(wnd, MOVEABLE))    {
            WindowMoving = TRUE;
            px = mx;
            py = my;
            diff = (int) mx;
            SendMessage(wnd, CAPTURE_MOUSE, TRUE,
                (PARAM) &dwnd);
            dragborder(wnd, GetLeft(wnd), GetTop(wnd));
        }
        return;
    }
    if (mx == WindowWidth(wnd)-1 && my == WindowHeight(wnd)-1)    {
        /* ------- hit the resize corner ------- */
#ifdef INCLUDE_MINIMIZE
        if (wnd->condition == ISMINIMIZED)
            return;
#endif
        if (!TestAttribute(wnd, SIZEABLE))
            return;
#ifdef INCLUDE_MAXIMIZE
        if (wnd->condition == ISMAXIMIZED)    {
            if (GetParent(wnd) == NULL)
                return;
            if (TestAttribute(GetParent(wnd),HASBORDER))
                return;
            /* ----- resizing a maximized window over a
                    borderless parent ----- */
            wnd = GetParent(wnd);
            if (!TestAttribute(wnd, SIZEABLE))
                return;
        }
#endif
        WindowSizing = TRUE;
        SendMessage(wnd, CAPTURE_MOUSE, TRUE, (PARAM) &dwnd);
        dragborder(wnd, GetLeft(wnd), GetTop(wnd));
    }
}

/* --------- MOUSE_MOVED Message ---------- */
static BOOL MouseMovedMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    if (WindowMoving)    {
        int leftmost = 0, topmost = 0,
            bottommost = SCREENHEIGHT-2,
            rightmost = SCREENWIDTH-2;
        int x = (int) p1 - diff;
        int y = (int) p2;
        if (GetParent(wnd) != NULL && !TestAttribute(wnd, NOCLIP))    {
            WINDOW wnd1 = GetParent(wnd);
            topmost    = GetClientTop(wnd1);
            leftmost   = GetClientLeft(wnd1);
            bottommost = GetClientBottom(wnd1);
            rightmost  = GetClientRight(wnd1);
        }
        if (x < leftmost || x > rightmost || y < topmost || y > bottommost)    {
            x = max(x, leftmost);
            x = min(x, rightmost);
            y = max(y, topmost);
            y = min(y, bottommost);
            SendMessage(NULL,MOUSE_CURSOR,x+diff,y);
        }
        if (x != px || y != py)    {
            px = x;
            py = y;
            dragborder(wnd, x, y);
        }
        return TRUE;
    }
    if (WindowSizing)    {
        sizeborder(wnd, (int) p1, (int) p2);
        return TRUE;
    }
    return FALSE;
}

#ifdef INCLUDE_MAXIMIZE
/* --------- MAXIMIZE Message ---------- */
static void MaximizeMsg(WINDOW wnd)
{
    RECT rc = {0, 0, 0, 0};
    RECT holdrc;
    holdrc = wnd->RestoredRC;
    rc.rt = SCREENWIDTH-1;
    rc.bt = SCREENHEIGHT-1;
    if (GetParent(wnd))
        rc = ClientRect(GetParent(wnd));
    wnd->oldcondition = wnd->condition;
    wnd->condition = ISMAXIMIZED;
    wnd->wasCleared = FALSE;
    SendMessage(wnd, HIDE_WINDOW, 0, 0);
    SendMessage(wnd, MOVE,
        RectLeft(rc), RectTop(rc));
    SendMessage(wnd, SIZE,
        RectRight(rc), RectBottom(rc));
    if (wnd->restored_attrib == 0)
        wnd->restored_attrib = wnd->attrib;
    ClearAttribute(wnd, SHADOW);
    SendMessage(wnd, SHOW_WINDOW, 0, 0);
    wnd->RestoredRC = holdrc;
}
#endif

#ifdef INCLUDE_MINIMIZE
/* --------- MINIMIZE Message ---------- */
static void MinimizeMsg(WINDOW wnd)
{
    RECT rc;
    RECT holdrc;

    holdrc = wnd->RestoredRC;
    rc = PositionIcon(wnd);
    wnd->oldcondition = wnd->condition;
    wnd->condition = ISMINIMIZED;
    wnd->wasCleared = FALSE;
    SendMessage(wnd, HIDE_WINDOW, 0, 0);
    SendMessage(wnd, MOVE,
        RectLeft(rc), RectTop(rc));
    SendMessage(wnd, SIZE,
        RectRight(rc), RectBottom(rc));
    if (wnd == inFocus)
        SetNextFocus();
    if (wnd->restored_attrib == 0)
        wnd->restored_attrib = wnd->attrib;
    ClearAttribute(wnd,
        SHADOW | SIZEABLE | HASMENUBAR |
        VSCROLLBAR | HSCROLLBAR);
    SendMessage(wnd, SHOW_WINDOW, 0, 0);
    wnd->RestoredRC = holdrc;
}
#endif

#ifdef INCLUDE_RESTORE
/* --------- RESTORE Message ---------- */
static void RestoreMsg(WINDOW wnd)
{
    RECT holdrc;
    holdrc = wnd->RestoredRC;
    wnd->oldcondition = wnd->condition;
    wnd->condition = ISRESTORED;
    wnd->wasCleared = FALSE;
    SendMessage(wnd, HIDE_WINDOW, 0, 0);
    wnd->attrib = wnd->restored_attrib;
    wnd->restored_attrib = 0;
    SendMessage(wnd, MOVE, wnd->RestoredRC.lf,
        wnd->RestoredRC.tp);
    wnd->RestoredRC = holdrc;
    SendMessage(wnd, SIZE, wnd->RestoredRC.rt,
        wnd->RestoredRC.bt);
    if (wnd != inFocus)
        SendMessage(wnd, SETFOCUS, TRUE, 0);
    else
        SendMessage(wnd, SHOW_WINDOW, 0, 0);
}
#endif

/* --------- MOVE Message ---------- */
static void MoveMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    WINDOW cwnd;
    BOOL wasVisible = isVisible(wnd);
    int xdif = (int) p1 - wnd->rc.lf;
    int ydif = (int) p2 - wnd->rc.tp;

    if (xdif == 0 && ydif == 0)
        return;
    wnd->wasCleared = FALSE;
    if (wasVisible)
        SendMessage(wnd, HIDE_WINDOW, 0, 0);
    wnd->rc.lf = (int) p1;
    wnd->rc.tp = (int) p2;
    wnd->rc.rt = GetLeft(wnd)+WindowWidth(wnd)-1;
    wnd->rc.bt = GetTop(wnd)+WindowHeight(wnd)-1;
    if (wnd->condition == ISRESTORED)
        wnd->RestoredRC = wnd->rc;

    cwnd = FirstWindow(wnd);
    while (cwnd != NULL)	{
        SendMessage(cwnd, MOVE, cwnd->rc.lf+xdif, cwnd->rc.tp+ydif);
        cwnd = NextWindow(cwnd);
    }
    if (wasVisible)
        SendMessage(wnd, SHOW_WINDOW, 0, 0);
}

/* --------- SIZE Message ---------- */
static void SizeMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    BOOL wasVisible = isVisible(wnd);
    WINDOW cwnd;
    RECT rc;
    int xdif = (int) p1 - wnd->rc.rt;
    int ydif = (int) p2 - wnd->rc.bt;

    if (xdif == 0 && ydif == 0)
        return;
    wnd->wasCleared = FALSE;
    if (wasVisible)
        SendMessage(wnd, HIDE_WINDOW, 0, 0);
    wnd->rc.rt = (int) p1;
    wnd->rc.bt = (int) p2;
    wnd->ht = GetBottom(wnd)-GetTop(wnd)+1;
    wnd->wd = GetRight(wnd)-GetLeft(wnd)+1;

    if (wnd->condition == ISRESTORED)
        wnd->RestoredRC = WindowRect(wnd);

#ifdef INCLUDE_MAXIMIZE
    rc = ClientRect(wnd);

    cwnd = FirstWindow(wnd);
    while (cwnd != NULL)	{
        if (cwnd->condition == ISMAXIMIZED)
            SendMessage(cwnd, SIZE, RectRight(rc), RectBottom(rc));
        cwnd = NextWindow(cwnd);
    }

#endif
    if (wasVisible)
        SendMessage(wnd, SHOW_WINDOW, 0, 0);
}

/* --------- CLOSE_WINDOW Message ---------- */
static void CloseWindowMsg(WINDOW wnd)
{
    WINDOW cwnd;
#if CLASSIC_WINDOW_NUMBERING
    /* nothing */
#else		/* new 0.7c: stacking-independent window numbering */
    WINDOW pwnd;
#endif
    wnd->condition = ISCLOSING;
    /* ----------- hide this window ------------ */
    SendMessage(wnd, HIDE_WINDOW, 0, 0);

    /* --- close the children of this window --- */
    cwnd = LastWindow(wnd);
    while (cwnd != NULL)	{
        if (inFocus == cwnd)
            inFocus = wnd;
        SendMessage(cwnd,CLOSE_WINDOW,0,0);
        cwnd = LastWindow(wnd);
    }

    /* ----- release captured resources ------ */
    if (wnd->PrevClock != NULL)
        SendMessage(wnd, RELEASE_CLOCK, 0, 0);
    if (wnd->PrevMouse != NULL)
        SendMessage(wnd, RELEASE_MOUSE, 0, 0);
    if (wnd->PrevKeyboard != NULL)
        SendMessage(wnd, RELEASE_KEYBOARD, 0, 0);

    /* --- change focus if this window had it -- */
    if (wnd == inFocus)
        SetPrevFocus();
    /* -- free memory allocated to this window - */
    if (wnd->title != NULL)
        free(wnd->title);
    if (wnd->videosave != NULL)
        free(wnd->videosave);
    /* -- remove window from parent's list of children -- */
#if CLASSIC_WINDOW_NUMBERING
    RemoveWindow(wnd);
#else /* new 0.7c: stacking-independent window numbering */
    /* printf("REMOVE / CLOSEWINDOWMSG %p\n", wnd); getkey(); */
    pwnd = GetParent(wnd);
    if ((pwnd != NULL) &&
        (GetClass(pwnd) == APPLICATION)) { /* only SUCH windows */
        WINDOW scanwnd;
        if (NumberOneChildWindow(pwnd) == wnd) {
            NumberOneChildWindow(pwnd) = NextNumberedWindow(pwnd); /* make another one the first */
        }
        scanwnd = NumberOneChildWindow(pwnd);
        while (scanwnd != NULL) {
            if (NextNumberedWindow(scanwnd) == wnd)	/* this next? */
                NextNumberedWindow(scanwnd) =
                    NextNumberedWindow(wnd);	/* skip this! */
            scanwnd = NextNumberedWindow(scanwnd);
        }
        NextNumberedWindow(wnd) = NULL;		/* clean up */
    }
    RemoveWindow(wnd);
    /* printf("REMOVE / CLOSEWINDOWMSG %p DONE\n", wnd); getkey(); */
#endif
    if (wnd == inFocus)
        inFocus = NULL;
    free(wnd);
}

/* ---- Window-processing module for NORMAL window class ---- */
int NormalProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            CreateWindowMsg(wnd);
            break;
        case SHOW_WINDOW:
            ShowWindowMsg(wnd, p1, p2);
            break;
        case HIDE_WINDOW:
            HideWindowMsg(wnd);
            break;
        case DISPLAY_HELP:
            return SystemHelp(wnd, (char *)p1);
        case INSIDE_WINDOW:
            return InsideWindow(wnd, (int) p1, (int) p2);
        case KEYBOARD:
            if (KeyboardMsg(wnd, p1, p2))
                return TRUE;
            /* ------- fall through ------- */
        case ADDSTATUS:
        case SHIFT_CHANGED:
            if (GetParent(wnd) != NULL)
                PostMessage(GetParent(wnd), msg, p1, p2);
            break;
        case PAINT:
            if (isVisible(wnd)) {
#ifdef INCLUDE_MULTI_WINDOWS
                if (wnd->wasCleared)
                    PaintUnderLappers(wnd);
                else
#endif
                {
                    wnd->wasCleared = TRUE;
                    ClearWindow(wnd, (RECT *)p1, ' ');
                }
            }
            break;
        case BORDER:
            if (isVisible(wnd))    {
                if (TestAttribute(wnd, HASBORDER)) {
                    RepaintBorder(wnd, (RECT *)p1);
                } else if (TestAttribute(wnd, HASTITLEBAR)) {
                    DisplayTitle(wnd, (RECT *)p1);
                }
            }
            break;
        case COMMAND:
            CommandMsg(wnd, p1);
            break;
        case SETFOCUS:
            SetFocusMsg(wnd, p1);
            break;
        case DOUBLE_CLICK:
            DoubleClickMsg(wnd, p1, p2);
            break;
        case LEFT_BUTTON:
            LeftButtonMsg(wnd, p1, p2);
            break;
        case MOUSE_MOVED:
            if (MouseMovedMsg(wnd, p1, p2))
                return TRUE;
            break;
        case BUTTON_RELEASED:
            if (WindowMoving || WindowSizing)    {
                if (WindowMoving)
                    PostMessage(wnd,MOVE,dwnd.rc.lf,dwnd.rc.tp);
                else
                    PostMessage(wnd,SIZE,dwnd.rc.rt,dwnd.rc.bt);
                TerminateMoveSize();
            }
            break;
#ifdef INCLUDE_MAXIMIZE
        case MAXIMIZE:
            if (wnd->condition != ISMAXIMIZED)
                MaximizeMsg(wnd);
            break;
#endif
#ifdef INCLUDE_MINIMIZE
        case MINIMIZE:
            if (wnd->condition != ISMINIMIZED)
                MinimizeMsg(wnd);
            break;
#endif
#ifdef INCLUDE_RESTORE
        case RESTORE:
            if (wnd->condition != ISRESTORED)    {
#ifdef INCLUDE_MAXIMIZE
                if (wnd->oldcondition == ISMAXIMIZED)
                    SendMessage(wnd, MAXIMIZE, 0, 0);
                else
#endif
                    RestoreMsg(wnd);
            }
            break;
#endif
        case MOVE:
            MoveMsg(wnd, p1, p2);
            break;
        case SIZE:    {
            SizeMsg(wnd, p1, p2);
            break;
        }
        case CLOSE_WINDOW:
            CloseWindowMsg(wnd);
            break;
        default:
            break;
    }
    return TRUE;
}
#ifdef INCLUDE_MINIMIZE
/* ---- compute lower right icon space in a rectangle ---- */
static RECT LowerRight(RECT prc)
{
    RECT rc;
    RectLeft(rc) = RectRight(prc) - ICONWIDTH;
    RectTop(rc) = RectBottom(prc) - ICONHEIGHT;
    RectRight(rc) = RectLeft(rc)+ICONWIDTH-1;
    RectBottom(rc) = RectTop(rc)+ICONHEIGHT-1;
    return rc;
}
/* ----- compute a position for a minimized window icon ---- */
static RECT PositionIcon(WINDOW wnd)
{
    WINDOW pwnd = GetParent(wnd);
    RECT rc;
    RectLeft(rc) = SCREENWIDTH-ICONWIDTH;
    RectTop(rc) = SCREENHEIGHT-ICONHEIGHT;
    RectRight(rc) = SCREENWIDTH-1;
    RectBottom(rc) = SCREENHEIGHT-1;
    if (pwnd != NULL)    {
        RECT prc = WindowRect(pwnd);
        WINDOW cwnd = FirstWindow(pwnd);
        rc = LowerRight(prc);
        /* - search for icon available location - */
        while (cwnd != NULL)	{
            if (cwnd->condition == ISMINIMIZED)    {
                RECT rc1;
                rc1 = WindowRect(cwnd);
                if (RectLeft(rc1) == RectLeft(rc) &&
                        RectTop(rc1) == RectTop(rc))    {
                    RectLeft(rc) -= ICONWIDTH;
                    RectRight(rc) -= ICONWIDTH;
                    if (RectLeft(rc) < RectLeft(prc)+1)   {
                        RectLeft(rc) =
                            RectRight(prc)-ICONWIDTH;
                        RectRight(rc) =
                            RectLeft(rc)+ICONWIDTH-1;
                        RectTop(rc) -= ICONHEIGHT;
                        RectBottom(rc) -= ICONHEIGHT;
                        if (RectTop(rc) < RectTop(prc)+1)
                            return LowerRight(prc);
                    }
                    break;
                }
            }
            cwnd = NextWindow(cwnd);
        }
    }
    return rc;
}
#endif
/* ----- terminate the move or size operation ----- */
static void TerminateMoveSize(void)
{
    px = py = -1;
    diff = 0;
    SendMessage(&dwnd, RELEASE_MOUSE, TRUE, 0);
    SendMessage(&dwnd, RELEASE_KEYBOARD, TRUE, 0);
    RestoreBorder(dwnd.rc);
    WindowMoving = WindowSizing = FALSE;
}
/* ---- build a dummy window border for moving or sizing --- */
static void dragborder(WINDOW wnd, int x, int y)
{
    RestoreBorder(dwnd.rc);
    /* ------- build the dummy window -------- */
    dwnd.rc.lf = x;
    dwnd.rc.tp = y;
    dwnd.rc.rt = dwnd.rc.lf+WindowWidth(wnd)-1;
    dwnd.rc.bt = dwnd.rc.tp+WindowHeight(wnd)-1;
    dwnd.ht = WindowHeight(wnd);
    dwnd.wd = WindowWidth(wnd);
    dwnd.parent = GetParent(wnd);
    dwnd.attrib = VISIBLE | HASBORDER | NOCLIP;
    InitWindowColors(&dwnd);
    SaveBorder(dwnd.rc);
    RepaintBorder(&dwnd, NULL);
}
/* ---- write the dummy window border for sizing ---- */
static void sizeborder(WINDOW wnd, int rt, int bt)
{
    int leftmost = GetLeft(wnd)+10;
    int topmost = GetTop(wnd)+3;
    int bottommost = SCREENHEIGHT-1;
    int rightmost  = SCREENWIDTH-1;
    if (GetParent(wnd))    {
        bottommost = min(bottommost, GetClientBottom(GetParent(wnd)));
        rightmost  = min(rightmost, GetClientRight(GetParent(wnd)));
    }
    rt = min(rt, rightmost);
    bt = min(bt, bottommost);
    rt = max(rt, leftmost);
    bt = max(bt, topmost);
    SendMessage(NULL, MOUSE_CURSOR, rt, bt);

    if (rt != px || bt != py)
        RestoreBorder(dwnd.rc);

    /* ------- change the dummy window -------- */
    dwnd.ht = bt-dwnd.rc.tp+1;
    dwnd.wd = rt-dwnd.rc.lf+1;
    dwnd.rc.rt = rt;
    dwnd.rc.bt = bt;
    if (rt != px || bt != py)    {
        px = rt;
        py = bt;
        SaveBorder(dwnd.rc);
        RepaintBorder(&dwnd, NULL);
    }
}
#ifdef INCLUDE_MULTI_WINDOWS
/* ----- adjust a rectangle to include the shadow ----- */
static RECT adjShadow(WINDOW wnd)
{
    RECT rc;
    rc = wnd->rc;
    if (TestAttribute(wnd, SHADOW))    {
        if (RectRight(rc) < SCREENWIDTH-1)
            RectRight(rc)++;           
        if (RectBottom(rc) < SCREENHEIGHT-1)
            RectBottom(rc)++;
    }
    return rc;
}
/* --- repaint a rectangular subsection of a window --- */
static void PaintOverLap(WINDOW wnd, RECT rc)
{
    if (isVisible(wnd))    {
        int isBorder, isTitle, isData;
        isBorder = isTitle = FALSE;
        isData = TRUE;
        if (TestAttribute(wnd, HASBORDER))    {
            isBorder =  RectLeft(rc) == 0 && RectTop(rc) < WindowHeight(wnd);
            isBorder |= RectLeft(rc) < WindowWidth(wnd) && RectRight(rc) >= WindowWidth(wnd)-1 && RectTop(rc) < WindowHeight(wnd);
            isBorder |= RectTop(rc) == 0 && RectLeft(rc) < WindowWidth(wnd);
            isBorder |= RectTop(rc) < WindowHeight(wnd) && RectBottom(rc) >= WindowHeight(wnd)-1 && RectLeft(rc) < WindowWidth(wnd);
        } else if (TestAttribute(wnd, HASTITLEBAR))
            isTitle = RectTop(rc) == 0 && RectRight(rc) > 0 && RectLeft(rc)<WindowWidth(wnd)-BorderAdj(wnd);

        if (RectLeft(rc) >= WindowWidth(wnd)-BorderAdj(wnd))
            isData = FALSE;
        if (RectTop(rc) >= WindowHeight(wnd)-BottomBorderAdj(wnd))
            isData = FALSE;
        if (TestAttribute(wnd, HASBORDER))    {
            if (RectRight(rc) == 0)
                isData = FALSE;
            if (RectBottom(rc) == 0)
                isData = FALSE;
        }
        if (TestAttribute(wnd, SHADOW))
            isBorder |= RectRight(rc) == WindowWidth(wnd) ||
                        RectBottom(rc) == WindowHeight(wnd);
        if (isData)	{
            wnd->wasCleared = FALSE;
            SendMessage(wnd, PAINT, (PARAM) &rc, TRUE);
        }
        if (isBorder)
            SendMessage(wnd, BORDER, (PARAM) &rc, 0);
        else if (isTitle)
            DisplayTitle(wnd, &rc);
    }
}
/* ------ paint the part of a window that is overlapped
            by another window that is being hidden ------- */
static void PaintOver(WINDOW wnd)
{
    RECT wrc, rc;
    wrc = adjShadow(HiddenWindow);
    rc = adjShadow(wnd);
    rc = subRectangle(rc, wrc);
    if (ValidRect(rc))
        PaintOverLap(wnd, RelativeWindowRect(wnd, rc));
}
/* --- paint the overlapped parts of all children --- */
static void PaintOverChildren(WINDOW pwnd)
{
    WINDOW cwnd = FirstWindow(pwnd);
    while (cwnd != NULL)    {
        if (cwnd != HiddenWindow)    {
            PaintOver(cwnd);
            PaintOverChildren(cwnd);
        }
        cwnd = NextWindow(cwnd);
    }
}
/* -- recursive overlapping paint of parents -- */
static void PaintOverParents(WINDOW wnd)
{
    WINDOW pwnd = GetParent(wnd);
    if (pwnd != NULL)    {
        PaintOverParents(pwnd);
        PaintOver(pwnd);
        PaintOverChildren(pwnd);
    }
}
/* - paint the parts of all windows that a window is over - */
static void PaintOverLappers(WINDOW wnd)
{
    HiddenWindow = wnd;
    PaintOverParents(wnd);
}
/* --- paint those parts of a window that are overlapped --- */
static void PaintUnderLappers(WINDOW wnd)
{
    WINDOW hwnd = NextWindow(wnd);
    while (hwnd != NULL) {
        /* ------- test only at document window level ------ */
        WINDOW pwnd = GetParent(hwnd);
        /* if (pwnd == NULL || GetClass(pwnd) == APPLICATION) */  {
            /* ---- don't bother testing self ----- */
            if (isVisible(hwnd) && hwnd != wnd)    {
                /* --- see if other window is descendent --- */
                while (pwnd != NULL)    {
                    if (pwnd == wnd)
                        break;
                    pwnd = GetParent(pwnd);
                }
                /* ----- don't test descendent overlaps ----- */
                if (pwnd == NULL)    {
                    /* -- see if other window is ancestor --- */
                    pwnd = GetParent(wnd);
                    while (pwnd != NULL)    {
                        if (pwnd == hwnd)
                            break;
                        pwnd = GetParent(pwnd);
                    }
                    /* --- don't test ancestor overlaps --- */
                    if (pwnd == NULL)    {
                        HiddenWindow = GetAncestor(hwnd);
                        ClearVisible(HiddenWindow);
                        PaintOver(wnd);
                        SetVisible(HiddenWindow);
                    }
                }
            }
        }
        hwnd = NextWindow(hwnd);
    }
    /* --------- repaint all children of this window the same way ----------- */
    hwnd = FirstWindow(wnd);
    while (hwnd != NULL)    {
        PaintUnderLappers(hwnd);
        hwnd = NextWindow(hwnd);
    }
}
#endif /* #ifdef INCLUDE_MULTI_WINDOWS */

/* --- save video area to be used by dummy window border --- */
static void SaveBorder(RECT rc)
{
    RECT lrc;
    con_char_t *cp;
    Bht = RectBottom(rc) - RectTop(rc) + 1;
    Bwd = RectRight(rc) - RectLeft(rc) + 1;
    Bsave = DFrealloc(Bsave, (Bht + Bwd) * 4);

    lrc = rc;
    RectBottom(lrc) = RectTop(lrc);
    getvideo(lrc, Bsave);
    RectTop(lrc) = RectBottom(lrc) = RectBottom(rc);
    getvideo(lrc, Bsave + Bwd);
    cp = Bsave + Bwd * 2;
    for (int i = 1; i < Bht-1; i++)    {
        *cp++ = GetVideoChar(RectLeft(rc),RectTop(rc)+i);
        *cp++ = GetVideoChar(RectRight(rc),RectTop(rc)+i);
    }
}
/* ---- restore video area used by dummy window border ---- */
static void RestoreBorder(RECT rc)
{
    if (Bsave != NULL)    {
        RECT lrc;
        int i;
        con_char_t *cp;
        lrc = rc;
        RectBottom(lrc) = RectTop(lrc);
        storevideo(lrc, Bsave);
        RectTop(lrc) = RectBottom(lrc) = RectBottom(rc);
        storevideo(lrc, Bsave + Bwd);
        cp = Bsave + Bwd * 2;
        for (i = 1; i < Bht-1; i++)    {
            PutVideoChar(RectLeft(rc),RectTop(rc)+i, *cp++);
            PutVideoChar(RectRight(rc),RectTop(rc)+i, *cp++);
        }
        free(Bsave);
        Bsave = NULL;
    }
}
/* ----- test if screen coordinates are in a window ---- */
static BOOL InsideWindow(WINDOW wnd, int x, int y)
{
    RECT rc;
    rc = WindowRect(wnd);
    if (!TestAttribute(wnd, NOCLIP))    {
        WINDOW pwnd = GetParent(wnd);
        while (pwnd != NULL)    {
            rc = subRectangle(rc, ClientRect(pwnd));
            pwnd = GetParent(pwnd);
        }
    }
    return InsideRect(x, y, rc);
}

BOOL isDerivedFrom(WINDOW wnd, CLASS cls)
{
    CLASS tclass = GetClass(wnd);
    while (tclass != -1)    {
        if (tclass == cls)
            return TRUE;
        tclass = (classdefs[tclass].base);
    }
    return FALSE;
}

/* -- find the oldest document window ancestor of a window -- */
WINDOW GetAncestor(WINDOW wnd)
{
    if (wnd != NULL)    {
        while (GetParent(wnd) != NULL)    {
            if (GetClass(GetParent(wnd)) == APPLICATION)
                break;
            wnd = GetParent(wnd);
        }
    }
    return wnd;
}

BOOL isVisible(WINDOW wnd)
{
    while (wnd != NULL)    {
        if (isHidden(wnd))
            return FALSE;
        wnd = GetParent(wnd);
    }
    return TRUE;
}

/* -- adjust a window's rectangle to clip it to its parent - */
static RECT ClipRect(WINDOW wnd)
{
    RECT rc;
    rc = WindowRect(wnd);
    if (TestAttribute(wnd, SHADOW))    {
        RectBottom(rc)++;
        RectRight(rc)++;
    }
    return ClipRectangle(wnd, rc);
}

/* -- get the video memory that is to be used by a window -- */
static void GetVideoBuffer(WINDOW wnd)
{
    RECT rc;
    int ht;
    int wd;

    rc = ClipRect(wnd);
    ht = RectBottom(rc) - RectTop(rc) + 1;
    wd = RectRight(rc) - RectLeft(rc) + 1;
    wnd->videosave = DFrealloc(wnd->videosave, (ht * wd * 2));
    get_videomode();
    getvideo(rc, wnd->videosave);
}

/* -- put the video memory that is used by a window -- */
static void PutVideoBuffer(WINDOW wnd)
{
    if (wnd->videosave != NULL)    {
        RECT rc;
        rc = ClipRect(wnd);
        get_videomode();
        storevideo(rc, wnd->videosave);
        free(wnd->videosave);
        wnd->videosave = NULL;
    }
}

/* ------- return TRUE if awnd is an ancestor of wnd ------- */
BOOL isAncestor(WINDOW wnd, WINDOW awnd)
{
    while (wnd != NULL)	{
        if (wnd == awnd)
            return TRUE;
        wnd = GetParent(wnd);
    }
    return FALSE;
}


/* -------------- pictbox.c -------------- */

typedef struct    {
    enum VectTypes vt;
    RECT rc;
} VECT;

static unsigned char CharInWnd[] = { 0xc4, 0xb3, 0xda, 0xbf, 0xd9, 0xc0, 0xc5, 0xc3, 0xb4, 0xc1, 0xc2 };

static unsigned char VectCvt[3][11][2][3] = {
    {   /* --- first character in collision vector --- */
        /* ( drawing - ) ( drawing | ) */
             {{0xc4, 0xc4, 0xc4},     {0xda, 0xc3, 0xc0}},
             {{0xda, 0xc2, 0xbf},     {0xb3, 0xb3, 0xb3}},
             {{0xda, 0xc2, 0xc2},     {0xda, 0xc3, 0xc3}},
             {{0xbf, 0xbf, 0xbf},     {0xbf, 0xbf, 0xbf}},
             {{0xd9, 0xd9, 0xd9},     {0xd9, 0xd9, 0xd9}},
             {{0xc0, 0xc1, 0xc1},     {0xc3, 0xc3, 0xc0}},
             {{0xc5, 0xc5, 0xc5},     {0xc5, 0xc5, 0xc5}},
             {{0xc3, 0xc5, 0xc5},     {0xc3, 0xc3, 0xc3}},
             {{0xb4, 0xb4, 0xb4},     {0xb4, 0xb4, 0xb4}},
             {{0xc1, 0xc1, 0xc1},     {0xc1, 0xc1, 0xc1}},
             {{0xc2, 0xc2, 0xc2},     {0xc2, 0xc5, 0xc5}}    },
    {   /* --- middle character in collision vector --- */
        /* ( drawing - ) ( drawing | ) */
             {{0xc4, 0xc4, 0xc4},     {0xc2, 0xc5, 0xc1}},
             {{0xc3, 0xc5, 0xb4},     {0xb3, 0xb3, 0xb3}},
             {{0xda, 0xda, 0xda},     {0xda, 0xda, 0xda}},
             {{0xbf, 0xbf, 0xbf},     {0xbf, 0xbf, 0xbf}},
             {{0xd9, 0xd9, 0xd9},     {0xd9, 0xd9, 0xd9}},
             {{0xc0, 0xc0, 0xc0},     {0xc0, 0xc0, 0xc0}},
             {{0xc5, 0xc5, 0xc5},     {0xc5, 0xc5, 0xc5}},
             {{0xc3, 0xc3, 0xc3},     {0xc3, 0xc3, 0xc3}},
             {{0xc5, 0xc5, 0xb4},     {0xb4, 0xb4, 0xb4}},
             {{0xc1, 0xc1, 0xc1},     {0xc5, 0xc5, 0xc1}},
             {{0xc2, 0xc2, 0xc2},     {0xc2, 0xc2, 0xc2}}    },
    {   /* --- last character in collision vector --- */
        /* ( drawing - ) ( drawing | ) */
             {{0xc4, 0xc4, 0xc4},     {0xbf, 0xb4, 0xd9}},
             {{0xc0, 0xc1, 0xd9},     {0xb3, 0xb3, 0xb3}},
             {{0xda, 0xda, 0xda},     {0xda, 0xda, 0xda}},
             {{0xc2, 0xc2, 0xbf},     {0xbf, 0xb4, 0xb4}},
             {{0xc1, 0xc1, 0xd9},     {0xb4, 0xb4, 0xd9}},
             {{0xc0, 0xc0, 0xc0},     {0xc0, 0xc0, 0xc0}},
             {{0xc5, 0xc5, 0xc5},     {0xc5, 0xc5, 0xc5}},
             {{0xc3, 0xc3, 0xc3},     {0xc3, 0xc3, 0xc3}},
             {{0xc5, 0xc5, 0xb4},     {0xb4, 0xb4, 0xb4}},
             {{0xc1, 0xc1, 0xc1},     {0xc5, 0xc5, 0xc1}},
             {{0xc2, 0xc2, 0xc2},     {0xc2, 0xc2, 0xc2}}    }
};

/* -- compute whether character is first, middle, or last -- */
static int FindVector(WINDOW wnd, RECT rc, int x, int y)
{
    RECT rcc;
    VECT *vc = wnd->VectorList;
    int coll = -1;
    for (int i = 0; i < wnd->VectorCount; i++)    {
        if ((vc+i)->vt == VECTOR)    {
            rcc = (vc+i)->rc;
            /* --- skip the colliding vector --- */
            if (rcc.lf == rc.lf && rcc.rt == rc.rt && rcc.tp == rc.tp && rc.bt == rcc.bt)
                continue;
            if (rcc.tp == rcc.bt)    {
                /* ---- horizontal vector,
                    see if character is in it --- */
                if (rc.lf+x >= rcc.lf && rc.lf+x <= rcc.rt && rc.tp+y == rcc.tp)    {
                    /* --- it is --- */
                    if (rc.lf+x == rcc.lf)
                        coll = 0;
                    else if (rc.lf+x == rcc.rt)
                        coll = 2;
                    else 
                        coll = 1;
                }
            } else {
                /* ---- vertical vector, see if character is in it --- */
                if (rc.tp+y >= rcc.tp && rc.tp+y <= rcc.bt && rc.lf+x == rcc.lf)    {
                    /* --- it is --- */
                    if (rc.tp+y == rcc.tp)
                        coll = 0;
                    else if (rc.tp+y == rcc.bt)
                        coll = 2;
                    else 
                        coll = 1;
                }
            }
        }
    }
    return coll;
}

static void PaintVector(WINDOW wnd, RECT rc)
{
    int xi, yi, len;
    unsigned char nc;
    unsigned char newch;
    static int fml, vertvect, coll;

    if (rc.rt == rc.lf)    {
        /* ------ vertical vector ------- */
        nc = 0xb3;
        vertvect = 1;
        len = rc.bt-rc.tp+1;
    } else {
        /* ------ horizontal vector ------- */
        nc = 0xc4;
        vertvect = 0;
        len = rc.rt-rc.lf+1;
    }

    for (int i = 0; i < len; i++)    {
        newch = nc;
        xi = yi = 0;
        if (vertvect)
            yi = i;
        else
            xi = i;
        unsigned char ch = videochar(GetClientLeft(wnd)+rc.lf+xi,GetClientTop(wnd)+rc.tp+yi);
        for (int cw = 0; cw < sizeof(CharInWnd); cw++)    {
            if (ch == CharInWnd[cw])    {
                /* ---- hit another vector character ---- */
                if ((coll=FindVector(wnd, rc, xi, yi)) != -1) {
                    /* compute first/middle/last subscript */
                    if (i == len-1)
                        fml = 2;
                    else if (i == 0)
                        fml = 0;
                    else
                        fml = 1;
                    newch = VectCvt[coll][cw][vertvect][fml];
                }
            }
        }
        PutWindowChar(wnd, newch, rc.lf+xi, rc.tp+yi);
    }
}

static void PaintBar(WINDOW wnd, RECT rc, enum VectTypes vt)
{
    int vertbar, len;
    unsigned int tys[] = {219, 178, 177, 176};
    unsigned int nc = tys[vt-1];

    if (rc.rt == rc.lf) {
        /* ------ vertical bar ------- */
        vertbar = 1;
        len = rc.bt-rc.tp+1;
    } else {
        /* ------ horizontal bar ------- */
        vertbar = 0;
        len = rc.rt-rc.lf+1;
    }

    for (int i = 0; i < len; i++) {
        int xi = 0, yi = 0;
        if (vertbar)
            yi = i;
        else
            xi = i;
        PutWindowChar(wnd, nc, rc.lf+xi, rc.tp+yi);
    }
}

static void PictPaintMsg(WINDOW wnd)
{
    VECT *vc = wnd->VectorList;
    for (int i = 0; i < wnd->VectorCount; i++) {
        if (vc->vt == VECTOR)
            PaintVector(wnd, vc->rc);
        else
            PaintBar(wnd, vc->rc, vc->vt);
        vc++;
    }
}

static void DrawVectorMsg(WINDOW wnd,PARAM p1,enum VectTypes vt)
{
    if (p1)    {
        VECT vc;
        wnd->VectorList = DFrealloc(wnd->VectorList,
                sizeof(VECT) * (wnd->VectorCount + 1));
        vc.vt = vt;
        vc.rc = *(RECT *)p1;
        *(((VECT *)(wnd->VectorList))+wnd->VectorCount)=vc;
        wnd->VectorCount++;
    }
}

static void DrawBoxMsg(WINDOW wnd, PARAM p1)
{
    if (p1)    {
        RECT rc = *(RECT *)p1;
        rc.bt = rc.tp;
        SendMessage(wnd, DRAWVECTOR, (PARAM) &rc, TRUE);
        rc = *(RECT *)p1;
        rc.lf = rc.rt;
        SendMessage(wnd, DRAWVECTOR, (PARAM) &rc, FALSE);
        rc = *(RECT *)p1;
        rc.tp = rc.bt;
        SendMessage(wnd, DRAWVECTOR, (PARAM) &rc, TRUE);
        rc = *(RECT *)p1;
        rc.rt = rc.lf;
        SendMessage(wnd, DRAWVECTOR, (PARAM) &rc, FALSE);
    }
}

int PictureProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case PAINT:
            BaseWndProc(PICTUREBOX, wnd, msg, p1, p2);
            PictPaintMsg(wnd);
            return TRUE;
        case DRAWVECTOR:
            DrawVectorMsg(wnd, p1, VECTOR);
            return TRUE;
        case DRAWBOX:
            DrawBoxMsg(wnd, p1);
            return TRUE;
        case DRAWBAR:
            DrawVectorMsg(wnd, p1, (enum VectTypes)p2);
            return TRUE;
        case CLOSE_WINDOW:
            if (wnd->VectorList != NULL)
                free(wnd->VectorList);
            break;
        default:
            break;
    }
    return BaseWndProc(PICTUREBOX, wnd, msg, p1, p2);
}

static RECT PictureRect(int x, int y, int len, int hv)
{
    RECT rc;
    rc.lf = rc.rt = x;
    rc.tp = rc.bt = y;
    if (hv)
        /* ---- horizontal vector ---- */
        rc.rt += len-1;
    else
        /* ---- vertical vector ---- */
        rc.bt += len-1;
    return rc;
}

void DrawVector(WINDOW wnd, int x, int y, int len, int hv)
{
    RECT rc = PictureRect(x,y,len,hv);
    SendMessage(wnd, DRAWVECTOR, (PARAM) &rc, 0);
}

void DrawBox(WINDOW wnd, int x, int y, int ht, int wd)
{
    RECT rc;
    rc.lf = x;
    rc.tp = y;
    rc.rt = x+wd-1;
    rc.bt = y+ht-1;
    SendMessage(wnd, DRAWBOX, (PARAM) &rc, 0);
}

void DrawBar(WINDOW wnd,enum VectTypes vt,
                        int x,int y,int len,int hv)
{
    RECT rc = PictureRect(x,y,len,hv);
    SendMessage(wnd, DRAWBAR, (PARAM) &rc, (PARAM) vt);
}
/* ------------- popdown.c ----------- */

static int SelectionWidth(struct PopDown *);
int CurrentMenuSelection;

static long int PopDownDelay = 0;
#define MaxPopDownDelay 8000 /* approx. half a second = 8000 */

extern int mctr;

/* ------------ CREATE_WINDOW Message ------------- */
static int PopDownCreateWindowMsg(WINDOW wnd)
{
    int rtn, adj;
    ClearAttribute(wnd, HASTITLEBAR | VSCROLLBAR | MOVEABLE | SIZEABLE | HSCROLLBAR);
	/* ------ adjust to keep popdown on screen ----- */
	adj = SCREENHEIGHT-1-wnd->rc.bt;
	if (adj < 0)	{
		wnd->rc.tp += adj;
		wnd->rc.bt += adj;
	}
	adj = SCREENWIDTH-1-wnd->rc.rt;
	if (adj < 0) {
		wnd->rc.lf += adj;
		wnd->rc.rt += adj;
	}
    rtn = BaseWndProc(POPDOWNMENU, wnd, CREATE_WINDOW, 0, 0);
    SendMessage(wnd, CAPTURE_MOUSE, 0, 0);
    SendMessage(wnd, CAPTURE_KEYBOARD, 0, 0);
    SendMessage(wnd, CAPTURE_CLOCK, 0, 0);            
    SendMessage(NULL, SAVE_CURSOR, 0, 0);
    SendMessage(NULL, HIDE_CURSOR, 0, 0);

    wnd->oldFocus = inFocus;
    inFocus = wnd;

    return rtn;
}


/* -------- BUTTON_RELEASED Message -------- */
static BOOL PopDownButtonReleasedMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    if (InsideRect((int)p1, (int)p2, ClientRect(wnd)))    {
        int sel = (int)p2 - GetClientTop(wnd);
        if (*TextLine(wnd, sel) != LINE)     {
            if (FocusedWindow(wnd) != NULL)
                SendMessage (FocusedWindow(wnd), CLOSE_WINDOW, 0, 0);
            SendMessage(wnd, LB_SELECTION, sel, 0);
            SendMessage(wnd, LB_CHOOSE, wnd->selection, 0);
        }
    } else {
        WINDOW pwnd = GetParent (wnd);
        PostMessage (pwnd, BUTTON_RELEASED, p1, p2);
        if ( (GetClass(pwnd) !=MENUBAR) && (GetClass(pwnd)!=POPDOWNMENU) )
            SendMessage(wnd, CLOSE_POPDOWN, TRUE, 0);
    }
    return FALSE;
}

/* --------- PAINT Message -------- */
static void PopDownPaintMsg(WINDOW wnd)
{
    int wd;
    char sep[80],*cp=sep,sel[80];
    struct PopDown *ActivePopDown,*pd1;

    ActivePopDown = pd1 = wnd->mnu->Selections;
    wd = MenuWidth(ActivePopDown)-2;
    while (wd--)
        *cp++ = LINE;

    *cp = '\0';
    SendMessage(wnd, CLEARTEXT, 0, 0);
    wnd->selection = wnd->mnu->Selection;
    while (pd1->SelectionTitle != NULL)
        {
        if (*pd1->SelectionTitle == LINE)
            SendMessage(wnd, ADDTEXT, (PARAM) sep, 0);
        else
            {
            int len;

            memset(sel, '\0', sizeof sel);
            if (pd1->Attrib & INACTIVE)
                /* ------ inactive menu selection ----- */
                sprintf(sel, "%c%c%c", CHANGECOLOR, wnd->WindowColors [HILITE_COLOR] [FG]|0x80, wnd->WindowColors [STD_COLOR] [BG]|0x80);

            strcat(sel, " ");
            if (pd1->Attrib & CHECKED)
                /* ---- paint the toggle checkmark ---- */
                sel[strlen(sel)-1] = CHECKMARK;

            len=CopyCommand(sel+strlen(sel), pd1->SelectionTitle, pd1->Attrib & INACTIVE, wnd->WindowColors [STD_COLOR] [BG]);
            if (pd1->Accelerator)
                {
                /* ---- paint accelerator key ---- */
                int i;
                int wd1=2+SelectionWidth(ActivePopDown)-strlen(pd1->SelectionTitle);
                int key=pd1->Accelerator;

                if (key > 0 && key < 27)
                    {
                    /* --- CTRL+ key --- */
                    while (wd1--)
                        strcat(sel, " ");

                    sprintf(sel+strlen(sel), "Ctrl+%c", key-1+'A');
                    }
                else /* accelerator keys with names defined in keys.c */
                    {
                    for (i = 0; keys[i].keylabel; i++)
                        {
                        if (keys[i].keycode == key)
                            {
                            while (wd1--)
                                strcat(sel, " ");

                            sprintf(sel+strlen(sel), "%s",
                            keys[i].keylabel);
                            break;
                            }
                        }
                    }
                }
            if (pd1->Attrib & CASCADED)
                {
                /* ---- paint cascaded menu token ---- */
                if (!pd1->Accelerator)
                    {
                    wd = MenuWidth(ActivePopDown)-len+1;
                    while (wd--)
                        strcat(sel, " ");
                    }

                sel[strlen(sel)-1] = CASCADEPOINTER;
                }
            else
                strcat(sel, " ");

            strcat(sel, " ");
            sel[strlen(sel)-1] = RESETCOLOR;
            SendMessage(wnd, ADDTEXT, (PARAM) sel, 0);
            }
        pd1++;
        }
}

/* ---------- BORDER Message ----------- */
static int BorderMsg(WINDOW wnd)
{
    int rtn = TRUE;
    WINDOW currFocus;
    if (wnd->mnu != NULL)    {
        currFocus = inFocus;
        inFocus = NULL;
        rtn = BaseWndProc(POPDOWNMENU, wnd, BORDER, 0, 0);
        inFocus = currFocus;
        for (int i = 0; i < ClientHeight(wnd); i++)    {
            if (*TextLine(wnd, i) == LINE)    {
                wputch(wnd, LEDGE, 0, i+1);
                wputch(wnd, REDGE, WindowWidth(wnd)-1, i+1);
            }
        }
    }
    return rtn;
}

/* -------------- LB_CHOOSE Message -------------- */
/* did the user left-click one of the items in the current popdown? */
static void LBChooseMsg(WINDOW wnd, PARAM p1)
{
    struct PopDown *ActivePopDown = wnd->mnu->Selections;

    if (ActivePopDown != NULL) {
        int *attr = &(ActivePopDown+(int)p1)->Attrib;

        wnd->mnu->Selection = (int)p1;
        if (!(*attr & INACTIVE)) {
            if (*attr & TOGGLE)
                *attr ^= CHECKED;
                    if (GetParent (wnd) != NULL) {
                        CurrentMenuSelection = (int)p1;

                        if (isCascadedCommand2(wnd->mnu, (ActivePopDown+(int)p1)->ActionId)) {
                            /* find the cascaded menu based on command id in p1 */
                            MENU *mnu = ActiveMenuBar->PullDown + mctr;

                            while (mnu->Title != (void *)-1)    {
                                if (mnu->CascadeId == (int) (ActivePopDown+(int)p1)->ActionId ) {

                                    WINDOW mwnd; /* Create a new pop-up child window */

                                    if (mnu->PrepMenu != NULL)
                                        (*(mnu->PrepMenu))(GetDocFocus(), mnu);
              
                                    mwnd = CreateWindow(POPDOWNMENU, NULL,
                                        GetLeft(wnd) + WindowWidth(wnd) - 1,
                                        GetTop(wnd) + wnd->selection,
                                        MenuHeight(mnu->Selections),
                                        MenuWidth(mnu->Selections),
                                        NULL,
                                        wnd,
                                        NULL,
                                        SHADOW);
              
                                    wnd->childfocus = mwnd;		/* Register it as current menu's child */
              
                                    SendMessage(mwnd, BUILD_SELECTIONS, (PARAM) mnu, 0);
                                    SendMessage(mwnd, SETFOCUS, TRUE, 0);
                                    SendMessage(mwnd, SHOW_WINDOW, 0, 0);
                                    break;
                                }
                                mnu++;
                            }
                        } else
                            PostMessage(wnd, COMMAND,(ActivePopDown+(int)p1)->ActionId, 0);
                    }
        } else
            PostMessage(wnd, CLOSE_POPDOWN, TRUE, 0);
     }
}

/* ---------- KEYBOARD Message --------- */
/* did the user hit the highlighted key for an item in the current popdown? */
static BOOL PopDownKeyboardMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    WINDOW pwnd = GetParent(wnd);
    struct PopDown *ActivePopDown = wnd->mnu->Selections;

    if (wnd->mnu != NULL)    {
        if (ActivePopDown != NULL)    {
            int c = (int)p1;
            int sel = 0;
            int a;
            struct PopDown *pd = ActivePopDown;

#ifdef HOOKKEYB
            if ((c & OFFSET) == 0)
#else
            if ((c & FKEY) == 0)
#endif
            c = tolower(c);
            a = AltConvert(c);

            while (pd->SelectionTitle != NULL)    {
                char *cp = strchr(pd->SelectionTitle,
                                SHORTCUTCHAR);
                int sc = tolower(*(cp+1));
                if ((cp && sc == c) ||
                    (a && sc == a) ||
                    (pd->Accelerator == c))
                {
                    PostMessage(wnd, LB_SELECTION, sel, 0);
                    PostMessage(wnd, LB_CHOOSE, sel, TRUE);
                    return TRUE;
                }
                pd++, sel++;
            }
        }
    }


    switch ((int)p1)    {
        case F1: /* (possibly context sensitive) help */
            if (ActivePopDown == NULL)
                SendMessage(pwnd, KEYBOARD, p1, p2);
            else 
                SystemHelp(wnd,
                    (ActivePopDown+wnd->selection)->help);
            return TRUE;
        case ESC:
            PostMessage(wnd, CLOSE_POPDOWN, FALSE, 0);
            return TRUE;
#ifdef HOOKKEYB
        case FWD: /* old name of right arrow */
#else
        case LARROW: /* hope this makes sense */
            if (GetClass(pwnd)==MENUBAR)
                PostMessage (pwnd, KEYBOARD, p1, p2);
            else  if (GetClass (pwnd) == POPDOWNMENU )
                PostMessage (wnd, CLOSE_WINDOW, 0, 0);
            break;
        case RARROW: /* formerly called FWD */
            if ( (FocusedWindow(wnd) == NULL) && (wnd->mnu != NULL) && (ActivePopDown != NULL) && isCascadedCommand2(wnd->mnu, (ActivePopDown+wnd->selection)->ActionId))
                PostMessage(wnd,LB_CHOOSE,wnd->selection, 0);
            else  if (GetClass(pwnd)==MENUBAR)
                PostMessage(pwnd, KEYBOARD, p1, p2);
      break;
#endif
        case UP:
            if (wnd->selection == 0)    {
                if (wnd->wlines == ClientHeight(wnd))    {
                    PostMessage(wnd, LB_SELECTION, wnd->wlines-1, FALSE);
                    return TRUE;
                }
            }
            break;
        case DN:
            if (wnd->selection == wnd->wlines-1)    {
                if (wnd->wlines == ClientHeight(wnd))    {
                    PostMessage(wnd, LB_SELECTION, 0, FALSE);
                    return TRUE;
                }
            }
            break;
        case HOME:
        case END:
        case '\r':
            break;
        default:
            return TRUE;
    }
    return FALSE;
}

/* ----------- CLOSE_WINDOW Message ---------- */
static int PopDownCloseWindowMsg(WINDOW wnd)
{
    if (FocusedWindow(wnd) != NULL) SendMessage (FocusedWindow(wnd), CLOSE_WINDOW, 0, 0);
        SendMessage (GetParent(wnd), DEADCHILD, 0, 0);

    SendMessage(wnd, RELEASE_CLOCK, 0, 0);
    SendMessage(wnd, RELEASE_MOUSE, 0, 0);
    SendMessage(wnd, RELEASE_KEYBOARD, 0, 0);
    SendMessage(NULL, RESTORE_CURSOR, 0, 0);

    inFocus = wnd->oldFocus;

    return  BaseWndProc(POPDOWNMENU, wnd, CLOSE_WINDOW, 0, 0);
}

/* - Window processing module for POPDOWNMENU window class - */
int PopDownProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int     rtn;
    WINDOW  pwnd = GetParent (wnd);


    if (msg != CLOCKTICK)
        PopDownDelay = 0;

    switch (msg)    {
        case CREATE_WINDOW:
            rtn = PopDownCreateWindowMsg(wnd);
            return rtn;

        case CLOCKTICK:
            PopDownDelay++;
            if (PopDownDelay >= MaxPopDownDelay ) {
                PopDownDelay = 0;
                if ((wnd->mnu != NULL) && (wnd->selection != -1))
                    if (isCascadedCommand2(wnd->mnu, wnd->mnu->Selections[wnd->selection].ActionId))
                        SendMessage(wnd, LB_CHOOSE, wnd->selection, 0);
            }
            break;
            /* With SmoothMenus, no longer need to do anything special with LEFT_BUTTON
            case LEFT_BUTTON:
                LeftButtonMsg(wnd, p1, p2);
                return FALSE;
            */
        case DOUBLE_CLICK:
            return TRUE;
        case LB_SELECTION:
            if (*TextLine(wnd, (int)p1) == LINE)
                return TRUE;
            wnd->mnu->Selection = (int)p1;
            break;
        case BUTTON_RELEASED:
            if (PopDownButtonReleasedMsg(wnd, p1, p2))
                return TRUE;
            break;
        case MOUSE_MOVED:
            if (InsideRect((int)p1, (int)p2, ClientRect(wnd)))  {
                if ((p2 - WindowRect(wnd).tp -1) != wnd->selection) {
                    if (FocusedWindow(wnd) != NULL)
                        SendMessage (FocusedWindow(wnd), CLOSE_WINDOW, 0, 0);
                    PostMessage(wnd, LB_SELECTION, p2 - WindowRect(wnd).tp -1, FALSE);
                }
            } else
                PostMessage(pwnd, MOUSE_MOVED, p1, p2);
            break;
        case BUILD_SELECTIONS:
            wnd->mnu = (void *) p1;
            wnd->selection = wnd->mnu->Selection;
            break;
        case PAINT:
            if (wnd->mnu == NULL)
                return TRUE;
            PopDownPaintMsg(wnd);
            break;
        case BORDER:
            return BorderMsg(wnd);
        case LB_CHOOSE:
            LBChooseMsg(wnd, p1);
            return TRUE;
        case DEADCHILD:
              (wnd->childfocus) = NULL;
              break;
        case SETFOCUS:
            if (FocusedWindow(wnd) != NULL)
                return SendMessage (FocusedWindow(wnd), SETFOCUS, 0, 0);
            wnd->oldFocus = inFocus;
            inFocus = wnd;
            break;
        case KEYBOARD:
            if (PopDownKeyboardMsg(wnd, p1, p2))
                return TRUE;
            break;
        case CLOSE_WINDOW:
            rtn = PopDownCloseWindowMsg(wnd);
            return rtn;
            
        case CLOSE_POPDOWN:
            if ( (GetClass(pwnd)==MENUBAR) || (GetClass(pwnd)==POPDOWNMENU) )
                PostMessage(pwnd, msg, p1, p2);
            else
                PostMessage (wnd, CLOSE_WINDOW, 0, 0);
            return TRUE;
        case COMMAND:
            PostMessage(pwnd, msg, p1, p2);
            return TRUE;
        default:
            break;
    }

    return BaseWndProc(POPDOWNMENU, wnd, msg, p1, p2);
}

/* --------- compute menu height -------- */
int MenuHeight(struct PopDown *pd)
{
    int ht = 0;
    while (pd[ht].SelectionTitle != NULL)
        ht++;
    return ht+2;
}

/* --------- compute menu width -------- */
int MenuWidth(struct PopDown *pd)
{
    int wd = 0, i;
    int len = 0;

    wd = SelectionWidth(pd);
    while (pd->SelectionTitle != NULL)    {
        if (pd->Accelerator)    {
            for (i = 0; keys[i].keylabel; i++)
                if (keys[i].keycode == pd->Accelerator)    {
                    len = max(len, 2+strlen(keys[i].keylabel));
                    break;
                }
        }
        if (pd->Attrib & CASCADED)
            len = max(len, 2);
        pd++;
    }
    return wd+5+len;
}

/* ---- compute the maximum selection width in a menu ---- */
static int SelectionWidth(struct PopDown *pd)
{
    int wd = 0;
    while (pd->SelectionTitle != NULL) {
        int len = strlen(pd->SelectionTitle)-1;
        wd = max(wd, len);
        pd++;
    }
    return wd;
}

/* ----- copy a menu command to a display buffer ---- */
int CopyCommand(char *dest, const char *src, int skipcolor, int bg)
{
    char *d = dest;
    while (*src && *src != '\n') {
        if (*src == SHORTCUTCHAR) {
            src++;
            if (!skipcolor) {
                *dest++ = CHANGECOLOR;
                *dest++ = SysConfig.VideoCurrentColorScheme.clrArray[POPDOWNMENU][HILITE_COLOR] [BG] | 0x80;
                *dest++ = bg | 0x80;
                *dest++ = *src++;
                *dest++ = RESETCOLOR;
            }
        } else
            *dest++ = *src++;
    }
    return (int) (dest - d);
}


/* -------- radio.c -------- */

static CTLWINDOW *rct[MAXRADIOS];

int RadioButtonProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    DBOX *db = GetParent(wnd)->extension;
    CTLWINDOW *ct = GetControl(wnd);
    if (ct != NULL)    {
        switch (msg)    {
            case SETFOCUS:
                if (!(int)p1)
                    SendMessage(NULL, HIDE_CURSOR, 0, 0);
            case MOVE:
                rtn = BaseWndProc(RADIOBUTTON,wnd,msg,p1,p2);
                SetFocusCursor(wnd);
                return rtn;
            case PAINT:    {
                char rb[] = "( )";
                if (ct->setting)
                    rb[1] = 7;
                SendMessage(wnd, CLEARTEXT, 0, 0);
                SendMessage(wnd, ADDTEXT, (PARAM) rb, 0);
                SetFocusCursor(wnd);
                break;
            }
            case KEYBOARD:
                if ((int)p1 != ' ')
                    break;
            case LEFT_BUTTON:
                SetRadioButton(db, ct);
                break;
            default:
                break;
        }
    }
    return BaseWndProc(RADIOBUTTON, wnd, msg, p1, p2);
}

static BOOL Setting = TRUE;

void SetRadioButton(DBOX *db, CTLWINDOW *ct)
{
    Setting = FALSE;
    PushRadioButton(db, ct->command);
    Setting = TRUE;
}

void PushRadioButton(DBOX *db, UCOMMAND cmd)
{
    CTLWINDOW *ctt = db->ctl;
    CTLWINDOW *ct = FindCommand(db, cmd, RADIOBUTTON);
    int i;

    if (ct == NULL)
        return;

    /* --- clear all the radio buttons
                in this group on the dialog box --- */

    /* -------- build a table of all radio buttons at the
            same x vector ---------- */
    for (i = 0; i < MAXRADIOS; i++)
        rct[i] = NULL;
    while (ctt->cls)    {
        if (ctt->cls == RADIOBUTTON)
            if (ct->dwnd.x == ctt->dwnd.x)
                rct[ctt->dwnd.y] = ctt;
        ctt++;
    }

    /* ----- find the start of the radiobutton group ---- */
    i = ct->dwnd.y;
    while (i >= 0 && rct[i] != NULL)
        --i;
    /* ---- ignore everthing before the group ------ */
    while (i >= 0)
        rct[i--] = NULL;

    /* ----- find the end of the radiobutton group ---- */
    i = ct->dwnd.y;
    while (i < MAXRADIOS && rct[i] != NULL)
        i++;
    /* ---- ignore everthing past the group ------ */
    while (i < MAXRADIOS)
        rct[i++] = NULL;

    for (i = 0; i < MAXRADIOS; i++)    {
        if (rct[i] != NULL)    {
            int wason = rct[i]->setting;
            rct[i]->setting = OFF;
            if (Setting)
                rct[i]->isetting = OFF;
            if (wason)
                SendMessage(rct[i]->wnd, PAINT, 0, 0);
        }
    }
    /* ----- set the specified radio button on ----- */
    ct->setting = ON;
    if (Setting)
        ct->isetting = ON;
  if (ct->wnd != NULL)	    
    SendMessage(ct->wnd, PAINT, 0, 0);
}

BOOL RadioButtonSetting(DBOX *db, UCOMMAND cmd)
{
    CTLWINDOW *ct = FindCommand(db, cmd, RADIOBUTTON);
    return ct ? (ct->wnd ? (ct->setting==ON) : (ct->isetting==ON)) : FALSE;
}

/* ------------- rect.c --------------- */

 /* --- Produce the vector end points produced by the overlap
        of two other vectors --- */
static void subVector(int *v1, int *v2,
                        int t1, int t2, int o1, int o2)
{
    *v1 = *v2 = -1;
    if (within(o1, t1, t2))    {
        *v1 = o1;
        if (within(o2, t1, t2))
            *v2 = o2;
        else
            *v2 = t2;
    }
    else if (within(o2, t1, t2))    {
        *v2 = o2;
        if (within(o1, t1, t2))
            *v1 = o1;
        else
            *v1 = t1;
    }
    else if (within(t1, o1, o2))    {
        *v1 = t1;
        if (within(t2, o1, o2))
            *v2 = t2;
        else
            *v2 = o2;
    }
    else if (within(t2, o1, o2))    {
        *v2 = t2;
        if (within(t1, o1, o2))
            *v1 = t1;
        else
            *v1 = o1;
    }
}

 /* --- Return the rectangle produced by the overlap
        of two other rectangles ---- */
RECT subRectangle(RECT r1, RECT r2)
{
    RECT r = {0,0,0,0};
    subVector((int *) &RectLeft(r), (int *) &RectRight(r),
        RectLeft(r1), RectRight(r1),
        RectLeft(r2), RectRight(r2));
    subVector((int *) &RectTop(r), (int *) &RectBottom(r),
        RectTop(r1), RectBottom(r1),
        RectTop(r2), RectBottom(r2));
    if (RectRight(r) == -1 || RectTop(r) == -1)
        RectRight(r) =
        RectLeft(r) =
        RectTop(r) =
        RectBottom(r) = 0;
    return r;
}

/* ------- return the client rectangle of a window ------ */
RECT ClientRect(WINDOW wnd)
{
    RECT rc;

    RectLeft(rc) = GetClientLeft((WINDOW)wnd);
    RectTop(rc) = GetClientTop((WINDOW)wnd);
    RectRight(rc) = GetClientRight((WINDOW)wnd);
    RectBottom(rc) = GetClientBottom((WINDOW)wnd);
    return rc;
}

/* ----- return the rectangle relative to
            its window's screen position -------- */
RECT RelativeWindowRect(WINDOW wnd, RECT rc)
{
    RectLeft(rc) -= GetLeft((WINDOW)wnd);
    RectRight(rc) -= GetLeft((WINDOW)wnd);
    RectTop(rc) -= GetTop((WINDOW)wnd);
    RectBottom(rc) -= GetTop((WINDOW)wnd);
    return rc;
}

/* ----- clip a rectangle to the parents of the window ----- */
RECT ClipRectangle(WINDOW wnd, RECT rc)
{
    RECT sr;
    RectLeft(sr) = RectTop(sr) = 0;
    RectRight(sr) = SCREENWIDTH-1;
    RectBottom(sr) = SCREENHEIGHT-1;
    if (!TestAttribute((WINDOW)wnd, NOCLIP))
        while ((wnd = GetParent((WINDOW)wnd)) != NULL)
            rc = subRectangle(rc, ClientRect(wnd));
    return subRectangle(rc, sr);
}
/* ---------------- search.c ------------- */

extern DBOX SearchTextDB;
extern DBOX ReplaceTextDB;
static int CheckCase = FALSE;
static int Replacing = FALSE;
static int lastsize;

/* - case-insensitive, white-space-normalized char compare - */
static BOOL SearchCmp(int a, int b)
{
    if (b == '\n')
        b = ' ';
    if (CheckCase)
        return a != b;
    return tolower(a) != tolower(b);
}

/* ----- replace a matching block of text ----- */
static void replacetext(WINDOW wnd, char *cp1, DBOX *db)
{
    char *cr = GetEditBoxText(db, ID_REPLACEWITH);
    char *cp = GetEditBoxText(db, ID_SEARCHFOR);
    int oldlen = strlen(cp); /* length of text being replaced */
    int newlen = strlen(cr); /* length of replacing text      */
    int dif;
    lastsize = newlen;
    if (oldlen < newlen)    {
        /* ---- new text expands text size ---- */
        dif = newlen-oldlen;
        if (wnd->textlen < strlen(wnd->text)+dif)    {
            /* ---- need to reallocate the text buffer ---- */
            int offset = (int)(cp1-wnd->text);
            wnd->textlen += dif;
            wnd->text = DFrealloc(wnd->text, wnd->textlen+2);
            cp1 = wnd->text + offset;
        }
        memmove(cp1+dif, cp1, strlen(cp1)+1);
    }
    else if (oldlen > newlen)    {
        /* ---- new text collapses text size ---- */
        dif = oldlen-newlen;
        memmove(cp1, cp1+dif, strlen(cp1)+1);
    }
    strncpy(cp1, cr, newlen);
}

/* ------- search for the occurrance of a string ------- */
static void SearchTextBox(WINDOW wnd, int incr)
{
    char *s1 = NULL, *s2, *cp1;
    DBOX *db = Replacing ? &ReplaceTextDB : &SearchTextDB;
    char *cp = GetEditBoxText(db, ID_SEARCHFOR);
    BOOL rpl = TRUE, FoundOne = FALSE;

    while (rpl == TRUE && cp != NULL && *cp)    {
        /* even for "find again" (not replacing) this runs at least once */
        rpl = Replacing ?
                CheckBoxSetting(&ReplaceTextDB, ID_REPLACEALL) :
                FALSE;
        if (TextBlockMarked(wnd))    {
            ClearTextBlock(wnd); /* un-mark block */
            SendMessage(wnd, PAINT, 0, 0);
        }
        /* search for a match starting at cursor position */
        cp1 = CurrChar;
        if (incr)
            cp1 += lastsize;    /* start past the last hit */
        /* --- compare at each character position --- */
        while (*cp1)    {
            s1 = cp;
            s2 = cp1;
            while (*s1 && *s1 != '\n')    {
                if (SearchCmp(*s1, *s2))
                    break;
                s1++, s2++;
            }
            if (*s1 == '\0' || *s1 == '\n')
                break;
            cp1++;
        }
        if (s1 != NULL && (*s1 == 0 || *s1 == '\n'))    {
            /* ----- match at *cp1 ------- */
            FoundOne = TRUE;

            /* mark a block at beginning of matching text */
            wnd->BlkEndLine = TextLineNumber(wnd, s2);
            wnd->BlkBegLine = TextLineNumber(wnd, cp1);
            if (wnd->BlkEndLine < wnd->BlkBegLine)
                wnd->BlkEndLine = wnd->BlkBegLine;
            wnd->BlkEndCol =
                (int)(s2 - TextLine(wnd, wnd->BlkEndLine));
            wnd->BlkBegCol =
                (int)(cp1 - TextLine(wnd, wnd->BlkBegLine));

            /* position the cursor at the matching text */
            wnd->CurrCol = wnd->BlkBegCol;
            wnd->CurrLine = wnd->BlkBegLine;
            wnd->WndRow = wnd->CurrLine - wnd->wtop;

            /* -- remember the size of the matching text -- */
            lastsize = strlen(cp);

            /* align the window scroll to matching text */
            if (WndCol > ClientWidth(wnd)-1)
                wnd->wleft = wnd->CurrCol;
            if (wnd->WndRow > ClientHeight(wnd)-1)    {
                wnd->wtop = wnd->CurrLine;
                wnd->WndRow = 0;
            }

            SendMessage(wnd, PAINT, 0, 0);
            SendMessage(wnd, KEYBOARD_CURSOR,
                WndCol, wnd->WndRow);

            if (Replacing)    {
                if (rpl || YesNoBox("Replace the text?"))  {
                    replacetext(wnd, cp1, db);
                    wnd->TextChanged = TRUE;
                    BuildTextPointers(wnd);
                    if (rpl)    {
                        incr = TRUE;
                        continue;
                    }
                }
                ClearTextBlock(wnd);
                SendMessage(wnd, PAINT, 0, 0);
            }
            return;
        }
        break;
    }
    if (!FoundOne)
        MessageBox("Search", "No matching text found");
}

/* ------- search for the occurrance of a string, replace it with a specified string ------- */
void ReplaceText(WINDOW wnd)
{
    Replacing = TRUE;
    lastsize = 0;
    if (CheckCase)
        SetCheckBox(&ReplaceTextDB, ID_MATCHCASE);
    if (DialogBox(NULL, &ReplaceTextDB, TRUE, NULL))    {
        CheckCase=CheckBoxSetting(&ReplaceTextDB,ID_MATCHCASE);
        SearchTextBox(wnd, FALSE);
    }
}

/* ------- search for the first occurrance of a string ------ */
void SearchText(WINDOW wnd)
{
    Replacing = FALSE;
    lastsize = 0;
    if (CheckCase)
        SetCheckBox(&SearchTextDB, ID_MATCHCASE);
    if (DialogBox(NULL, &SearchTextDB, TRUE, NULL))    {
        CheckCase=CheckBoxSetting(&SearchTextDB,ID_MATCHCASE);
        SearchTextBox(wnd, FALSE);
    }
}

/* ------- search for the next occurrance of a string ------- */
void SearchNext(WINDOW wnd)
{
    SearchTextBox(wnd, TRUE);
}

/* ------------- slidebox.c ------------ */

static int (*GenericProc)(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2);
static BOOL KeepRunning;
static int SliderLen;
static int Percent;
extern DBOX SliderBoxDB;

static void InsertPercent(char *s)
{
    int offset;
    char pcc[5];

    sprintf(s, "%c%c%c",
            CHANGECOLOR,
            color.clrArray[DIALOG][SELECT_COLOR][FG]+0x80,
            color.clrArray[DIALOG][SELECT_COLOR][BG]+0x80);
    s += 3;
    memset(s, ' ', SliderLen);
    *(s+SliderLen) = '\0';
    sprintf(pcc, "%d%%", Percent);
    strncpy(s+SliderLen/2-1, pcc, strlen(pcc));
    offset = (SliderLen * Percent) / 100;
    memmove(s+offset+4, s+offset, strlen(s+offset)+1);
    sprintf(pcc, "%c%c%c%c",
            RESETCOLOR,
            CHANGECOLOR,
            color.clrArray[DIALOG][SELECT_COLOR][BG]+0x80,
            color.clrArray[DIALOG][SELECT_COLOR][FG]+0x80);
    strncpy(s+offset, pcc, 4);
    *(s + strlen(s) - 1) = RESETCOLOR;
}

static int SliderTextProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    switch (msg)    {
        case PAINT:
            Percent = (int)p2;
            InsertPercent(GetText(wnd) ?
                GetText(wnd) : SliderBoxDB.ctl[1].itext);
            GenericProc(wnd, PAINT, 0, 0);
            if (Percent >= 100)
                SendMessage(GetParent(wnd),COMMAND,ID_CANCEL,0);
            if (!dispatch_message())
                PostMessage(GetParent(wnd), ENDDIALOG, 0, 0);
            return KeepRunning;
        default:
            break;
    }
    return GenericProc(wnd, msg, p1, p2);
}

static int SliderBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    WINDOW twnd;
    switch (msg)    {
        case CREATE_WINDOW:
            AddAttribute(wnd, SAVESELF);
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            twnd = SliderBoxDB.ctl[1].wnd;
            GenericProc = twnd->wndproc;
            twnd->wndproc = SliderTextProc;
            KeepRunning = TRUE;
            SendMessage(wnd, CAPTURE_MOUSE, 0, 0);
            SendMessage(wnd, CAPTURE_KEYBOARD, 0, 0);
            return rtn;
        case COMMAND:
            if ((int)p2 == 0 && (int)p1 == ID_CANCEL)    {
                if (Percent >= 100 ||
                        YesNoBox("Terminate process?"))
                    KeepRunning = FALSE;
                else
                    return TRUE;
            }
            break;
        case CLOSE_WINDOW:
            SendMessage(wnd, RELEASE_MOUSE, 0, 0);
            SendMessage(wnd, RELEASE_KEYBOARD, 0, 0);
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

WINDOW SliderBox(int len, char *ttl, char *msg)
{
    SliderLen = len;
    SliderBoxDB.dwnd.title = ttl;
    SliderBoxDB.dwnd.w = max(strlen(ttl),max(len, strlen(msg)))+4;
    SliderBoxDB.ctl[0].itext = msg;
    SliderBoxDB.ctl[0].dwnd.w = strlen(msg);
    SliderBoxDB.ctl[0].dwnd.x = (SliderBoxDB.dwnd.w - strlen(msg)-1) / 2;
    SliderBoxDB.ctl[1].itext = DFrealloc(SliderBoxDB.ctl[1].itext, len+10);
    Percent = 0;
    InsertPercent(SliderBoxDB.ctl[1].itext);
    SliderBoxDB.ctl[1].dwnd.w = len;
    SliderBoxDB.ctl[1].dwnd.x = (SliderBoxDB.dwnd.w-len-1)/2;
    SliderBoxDB.ctl[2].dwnd.x = (SliderBoxDB.dwnd.w-10)/2;
    DialogBox(NULL, &SliderBoxDB, FALSE, SliderBoxProc);
    return SliderBoxDB.ctl[1].wnd;
}
/* ------------ spinbutt.c ------------- */

int SpinButtonProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    CTLWINDOW *ct = GetControl(wnd);
    if (ct != NULL)    {
        switch (msg)    {
            case CREATE_WINDOW:
                wnd->wd -= 2;
                wnd->rc.rt -= 2;
                break;
            case SETFOCUS:
                rtn = BaseWndProc(SPINBUTTON, wnd, msg, p1, p2);
                if (!(int)p1)
                    SendMessage(NULL, HIDE_CURSOR, 0, 0);
                SetFocusCursor(wnd);
                return rtn;
            case PAINT:
                foreground = WndBackground(wnd);
                background = WndForeground(wnd);
                wputch(wnd,UPSCROLLBOX,WindowWidth(wnd), 0);
                wputch(wnd,DOWNSCROLLBOX,WindowWidth(wnd)+1,0);
                SetFocusCursor(wnd);
                break;
            case LEFT_BUTTON:
                if (p1 == GetRight(wnd) + 1)
                    SendMessage(wnd, KEYBOARD, UP, 0);
                else if (p1 == GetRight(wnd) + 2)
                    SendMessage(wnd, KEYBOARD, DN, 0);
                if (wnd != inFocus)
                    SendMessage(wnd, SETFOCUS, TRUE, 0);
                return TRUE;
            case LB_SETSELECTION:
                rtn = BaseWndProc(SPINBUTTON, wnd, msg, p1, p2);
                wnd->wtop = (int) p1;
                SendMessage(wnd, PAINT, 0, 0);
                return rtn;
            default:
                break;
        }
    }
    return BaseWndProc(SPINBUTTON, wnd, msg, p1, p2);
}

/*  Status Bar

    Part of the FreeDOS Editor

*/

extern char time_string[];

int StatusBarProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    char *statusbar;

    switch (msg)
        {
        case CREATE_WINDOW:
            SendMessage(wnd, CAPTURE_CLOCK, 0, 0);
            break;
        case KEYBOARD:
            if ((int)p1 == CTRL_F4)
                return TRUE;

            break;
        case PAINT: 
            if (!isVisible(wnd))
                break;

            statusbar = DFcalloc(1, WindowWidth(wnd)+1);
            memset(statusbar, ' ', WindowWidth(wnd));
            *(statusbar+WindowWidth(wnd)) = '\0';
            strncpy(statusbar+1, "F1=Help \335", 9); /* 9 */
            if (wnd->text)
                {
                int len = min(strlen(wnd->text),
                    WindowWidth(wnd)-(strlen(time_string)+9+1+1+1) ); /* 9 */
                    /* if len < strlen(wnd->text), we cannot display all the text */

                if (len > 3) /* space left for min. 3 char more than time */
                             /* and "F1=Help " and 2 block graphics chars */
                    {
                    int off=(WindowWidth(wnd)-
                      (strlen(time_string)+len+1+1) );
                      /* right-aligned, next to time display */
                    if (len < strlen(wnd->text))
                        {
                        strncpy(statusbar+off, wnd->text, len-3);
                        strncpy(statusbar+off+len-3, "...", 3);
                        }
                    else
                        strncpy(statusbar+off, wnd->text, len);
                    } /* could display anything */

                }

            strncpy(statusbar+WindowWidth(wnd)-(strlen(time_string)+2),"\263 ", 2);
            if (wnd->TimePosted)
                *(statusbar+WindowWidth(wnd)-strlen(time_string)) = '\0';
            else
                strncpy(statusbar+WindowWidth(wnd)-strlen(time_string),time_string, strlen(time_string));

            SetStandardColor(wnd);
            PutWindowLine(wnd, statusbar, 0, 0);
            free(statusbar);
            return TRUE;
        case BORDER:
            return TRUE;
        case CLOCKTICK:
            SetStandardColor(wnd);
            PutWindowLine(wnd, (char *)p1, WindowWidth(wnd)-strlen(time_string), 0);
            wnd->TimePosted = TRUE;
            SendMessage(wnd->PrevClock, msg, p1, p2);
            return TRUE;
        case CLOSE_WINDOW:
            SendMessage(wnd, RELEASE_CLOCK, 0, 0);
            break;
        default:
            break;
        }

    return BaseWndProc(STATUSBAR, wnd, msg, p1, p2);

}
/* ------------- syshelp.c ------------- */


static BOOL (* UserHelp)(WINDOW, char *);



BOOL InstallHelpProcedure ( BOOL (* f) () )
{
    if (f != NULL)
    {
        UserHelp = f;
        return TRUE;
    } else
        return FALSE;
}


BOOL SystemHelp(WINDOW w, char *topic)
{
    return  (*UserHelp) (w, topic);
}

/* ------------- sysmenu.c ------------ */

int SystemMenuProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int mx, my;
    WINDOW wnd1;

    switch (msg)    {
        case CREATE_WINDOW:
            wnd->holdmenu = ActiveMenuBar;
            ActiveMenuBar = &SystemMenu;
            SystemMenu.PullDown[0].Selection = 0;
            break;
        case BUTTON_RELEASED:			/* DFLat+ 1.0: messages tracked */
        case LEFT_BUTTON:
            wnd1 = GetParent(wnd);
            mx = (int) p1 - GetLeft(wnd1);
            my = (int) p2 - GetTop(wnd1);
            if (HitControlBox(wnd1, mx, my))
                return TRUE;
            break;
            /* DFlat 1.0+: with SmoothMenus, track COMMAND better than LB_CHOOSE */
        // case LB_CHOOSE:
        //    PostMessage(wnd, CLOSE_WINDOW, 0, 0);
        //    break;
        case COMMAND:
            SendMessage (wnd, CLOSE_POPDOWN, 0, 0);
            break;
        case DOUBLE_CLICK:
            if (p2 == GetTop(GetParent(wnd)))    {
                PostMessage(GetParent(wnd), msg, p1, p2);
                SendMessage(wnd, CLOSE_WINDOW, TRUE, 0);
            }
            return TRUE;
        case SHIFT_CHANGED:
            return TRUE;
        case CLOSE_WINDOW:
            ActiveMenuBar = wnd->holdmenu;
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

/* ------- Build a system menu -------- */
void BuildSystemMenu(WINDOW wnd)
{
    int lf, tp, ht, wd;
    WINDOW SystemMenuWnd;

    SystemMenu.PullDown[0].Selections[6].Accelerator = (GetClass(wnd) == APPLICATION) ? ALT_F4 : CTRL_F4;

    lf = GetLeft(wnd)+1;
    tp = GetTop(wnd)+1;
    ht = MenuHeight(SystemMenu.PullDown[0].Selections);
    wd = MenuWidth(SystemMenu.PullDown[0].Selections);

    if (lf+wd > SCREENWIDTH-1)
        lf = (SCREENWIDTH-1) - wd;
    if (tp+ht > SCREENHEIGHT-2)
        tp = (SCREENHEIGHT-2) - ht;

    SystemMenuWnd = CreateWindow(POPDOWNMENU, NULL,
                lf,tp,ht,wd,NULL,wnd,SystemMenuProc, 0);

#ifdef INCLUDE_RESTORE
    if (wnd->condition == ISRESTORED)
        DeactivateCommand(&SystemMenu, ID_SYSRESTORE);
    else
        ActivateCommand(&SystemMenu, ID_SYSRESTORE);
#endif

    if (TestAttribute(wnd, MOVEABLE)
#ifdef INCLUDE_MAXIMIZE
        && wnd->condition != ISMAXIMIZED
#endif
        )
        ActivateCommand(&SystemMenu, ID_SYSMOVE);
    else
        DeactivateCommand(&SystemMenu, ID_SYSMOVE);

    if (wnd->condition != ISRESTORED ||
            TestAttribute(wnd, SIZEABLE) == FALSE)
        DeactivateCommand(&SystemMenu, ID_SYSSIZE);
    else
        ActivateCommand(&SystemMenu, ID_SYSSIZE);

#ifdef INCLUDE_MINIMIZE
    if (wnd->condition == ISMINIMIZED || TestAttribute(wnd, MINMAXBOX) == FALSE)
        DeactivateCommand(&SystemMenu, ID_SYSMINIMIZE);
    else
        ActivateCommand(&SystemMenu, ID_SYSMINIMIZE);
#endif

#ifdef INCLUDE_MAXIMIZE
    if (wnd->condition != ISRESTORED || TestAttribute(wnd, MINMAXBOX) == FALSE)
        DeactivateCommand(&SystemMenu, ID_SYSMAXIMIZE);
    else
        ActivateCommand(&SystemMenu, ID_SYSMAXIMIZE);
#endif

    SendMessage(SystemMenuWnd, BUILD_SELECTIONS, (PARAM) &SystemMenu.PullDown[0], 0);
    SendMessage(SystemMenuWnd, SETFOCUS, TRUE, 0);
    SendMessage(SystemMenuWnd, SHOW_WINDOW, 0, 0);
}

/* ----------- dialogs.c --------------- */


/* -------------- the File Open dialog box --------------- */
DIALOGBOX(FileOpen)
    DB_TITLE("Open File", -1,-1,19,57)
    CONTROL(TEXT,    "File ~Name:",   3, 1, 1,10, ID_FILENAME)
    CONTROL(EDITBOX, NULL,           14, 1, 1,40, ID_FILENAME)
    CONTROL(TEXT,    NULL,            3, 3, 1,50, ID_PATH) 
    CONTROL(TEXT,    "~Files:",       3, 5, 1, 6, ID_FILES)
    CONTROL(LISTBOX, NULL,            3, 6,10,14, ID_FILES)
    CONTROL(TEXT,    "~Directories:",19, 5, 1,12, ID_DIRECTORY)
    CONTROL(LISTBOX, NULL,           19, 6,10,13, ID_DIRECTORY)
    CONTROL(TEXT,    "Dri~ves:",     34, 5, 1, 7, ID_DRIVE)
    CONTROL(LISTBOX, NULL,           34, 6,10,10, ID_DRIVE)
    CONTROL(BUTTON,  "   ~OK   ",    46, 7, 1, 8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ",    46,10, 1, 8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ",    46,13, 1, 8, ID_HELP)
ENDDB

/* -------------- the Save As dialog box --------------- */
DIALOGBOX(SaveAs)
    DB_TITLE("Save As", -1,-1,19,57)
    CONTROL(TEXT,    "File ~Name:",   3, 1, 1, 9, ID_FILENAME)
    CONTROL(EDITBOX, NULL,           13, 1, 1,40, ID_FILENAME)
    CONTROL(TEXT,    NULL,            3, 3, 1,50, ID_PATH) 
    CONTROL(TEXT,    "~Files:",       3, 5, 1, 6, ID_FILES)
    CONTROL(LISTBOX, NULL,            3, 6,10,14, ID_FILES)
    CONTROL(TEXT,    "~Directories:",19, 5, 1,12, ID_DIRECTORY)
    CONTROL(LISTBOX, NULL,           19, 6,10,13, ID_DIRECTORY)
    CONTROL(TEXT,    "Dri~ves:",     34, 5, 1, 7, ID_DRIVE)
    CONTROL(LISTBOX, NULL,           34, 6,10,10, ID_DRIVE)
    CONTROL(BUTTON,  "   ~OK   ",    46, 7, 1, 8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ",    46,10, 1, 8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ",    46,13, 1, 8, ID_HELP)
ENDDB

/* -------------- The Printer Setup dialog box ------------------ */
DIALOGBOX(PrintSetup)
    DB_TITLE("Print Setup", -1, -1, 17, 32)
    CONTROL(BOX,      "Margins",  2,  3,  9, 26, 0)
    CONTROL(TEXT,     "~Port:",   4,  1,  1,  5, ID_PRINTERPORT)
    CONTROL(COMBOBOX, NULL,      12,  1,  8,  9, ID_PRINTERPORT)
    CONTROL(TEXT,     "~Left:",   6,  4,  1,  5, ID_LEFTMARGIN)
    CONTROL(SPINBUTTON, NULL,    17,  4,  1,  6, ID_LEFTMARGIN)
    CONTROL(TEXT,     "~Right:",  6,  6,  1,  6, ID_RIGHTMARGIN)
    CONTROL(SPINBUTTON, NULL,    17,  6,  1,  6, ID_RIGHTMARGIN)
    CONTROL(TEXT,     "~Top:",    6,  8,  1,  4, ID_TOPMARGIN)
    CONTROL(SPINBUTTON, NULL,    17,  8,  1,  6, ID_TOPMARGIN)
    CONTROL(TEXT,     "~Bottom:", 6, 10,  1,  7, ID_BOTTOMMARGIN)
    CONTROL(SPINBUTTON, NULL,    17, 10,  1,  6, ID_BOTTOMMARGIN)
    CONTROL(BUTTON, "   ~OK   ",  1, 13,  1,  8, ID_OK)
    CONTROL(BUTTON, " ~Cancel ", 11, 13,  1,  8, ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ", 21, 13,  1,  8, ID_HELP)
ENDDB

/* -------------- the Search Text dialog box --------------- */
DIALOGBOX(SearchTextDB)
    DB_TITLE("Find",-1,-1,9,48)
    CONTROL(TEXT,    "~Find What:",           2, 1, 1, 11, ID_SEARCHFOR)
    CONTROL(EDITBOX, NULL,                   14, 1, 1, 29, ID_SEARCHFOR)
    CONTROL(TEXT, "Match ~Case:"  ,           2, 3, 1, 23, ID_MATCHCASE)
    CONTROL(CHECKBOX,  NULL,                 26, 3, 1,  3, ID_MATCHCASE)
    CONTROL(BUTTON, "   ~OK   ",              7, 5, 1,  8, ID_OK)
    CONTROL(BUTTON, " ~Cancel ",             19, 5, 1,  8, ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",             31, 5, 1,  8, ID_HELP)
ENDDB

/* -------------- the Replace Text dialog box --------------- */
DIALOGBOX(ReplaceTextDB)
    DB_TITLE("Replace",-1,-1,12,50)
    CONTROL(TEXT,    "~Search for:",          2, 1, 1, 11, ID_SEARCHFOR)
    CONTROL(EDITBOX, NULL,                   16, 1, 1, 29, ID_SEARCHFOR)
    CONTROL(TEXT,    "~Replace with:",        2, 3, 1, 13, ID_REPLACEWITH)
    CONTROL(EDITBOX, NULL,                   16, 3, 1, 29, ID_REPLACEWITH)
    CONTROL(TEXT, "Match ~Case:",             2, 5, 1, 23, ID_MATCHCASE)
    CONTROL(CHECKBOX,  NULL,                 26, 5, 1,  3, ID_MATCHCASE)
    CONTROL(TEXT, "Replace ~All:",            2, 6, 1, 23, ID_REPLACEALL)
    CONTROL(CHECKBOX,  NULL,                 26, 6, 1,  3, ID_REPLACEALL)
    CONTROL(BUTTON, "   ~OK   ",              7, 8, 1,  8, ID_OK)
    CONTROL(BUTTON, " ~Cancel ",             20, 8, 1,  8, ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",             33, 8, 1,  8, ID_HELP)
ENDDB

/* -------------- generic message dialog box --------------- */
DIALOGBOX(MsgBox)
    DB_TITLE(NULL,-1,-1, 0, 0)
    CONTROL(TEXT,   NULL,   1, 1, 0, 0, 0)
    CONTROL(BUTTON, NULL,   0, 0, 1, 8, ID_OK)
    CONTROL(0,      NULL,   0, 0, 1, 8, ID_CANCEL)
ENDDB

/* ----------- InputBox Dialog Box ------------ */
DIALOGBOX(InputBoxDB)
    DB_TITLE(NULL,-1,-1, 9, 0)
    CONTROL(TEXT,    NULL,       1, 1, 1, 0, 0)
    CONTROL(EDITBOX, NULL,       1, 3, 1, 0, ID_INPUTTEXT)
    CONTROL(BUTTON, "   ~OK   ", 0, 5, 1, 8, ID_OK)
    CONTROL(BUTTON, " ~Cancel ", 0, 5, 1, 8, ID_CANCEL)
ENDDB

/* ----------- SliderBox Dialog Box ------------- */
DIALOGBOX(SliderBoxDB)
    DB_TITLE(NULL,-1,-1, 9, 0)
    CONTROL(TEXT,   NULL,       0, 1, 1, 0, 0)
    CONTROL(TEXT,   NULL,       0, 3, 1, 0, 0)
    CONTROL(BUTTON, " Cancel ", 0, 5, 1, 8, ID_CANCEL)
ENDDB


/* ------------ Windows dialog box -------------- */
DIALOGBOX(Windows)
    DB_TITLE("Windows", -1, -1, 19, 24)
    CONTROL(LISTBOX, NULL,         1,  1,11,20, ID_WINDOWLIST)
    CONTROL(BUTTON,  "   ~OK   ",  2, 13, 1, 8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ", 12, 13, 1, 8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ",  7, 15, 1, 8, ID_HELP)
ENDDB

#ifdef INCLUDE_LOGGING
/* ------------ Message Log dialog box -------------- */
DIALOGBOX(dbLog)
    DB_TITLE("Edit Message Log", -1, -1, 18, 41)
    CONTROL(TEXT,  "~Messages",   10,   1,  1,  8, ID_LOGLIST)
    CONTROL(LISTBOX,    NULL,     1,    2, 14, 26, ID_LOGLIST)
    CONTROL(TEXT,    "~Logging:", 29,   4,  1, 10, ID_LOGGING)
    CONTROL(CHECKBOX,    NULL,    31,   5,  1,  3, ID_LOGGING)
    CONTROL(BUTTON,  "   ~OK   ", 29,   7,  1,  8, ID_OK)
    CONTROL(BUTTON,  " ~Cancel ", 29,  10,  1,  8, ID_CANCEL)
    CONTROL(BUTTON,  "  ~Help  ", 29,  13, 1,   8, ID_HELP)
ENDDB
#endif


/* ------------- the System Menu --------------------- */
DEFMENU(SystemMenu)
    POPDOWN("System Menu", NULL, NULL)
#ifdef INCLUDE_RESTORE
        SELECTION("~Restore",  ID_SYSRESTORE,  0,         0 )
#endif
        SELECTION("~Move",     ID_SYSMOVE,     0,         0 )
        SELECTION("~Size",     ID_SYSSIZE,     0,         0 )
#ifdef INCLUDE_MINIMIZE
        SELECTION("Mi~nimize", ID_SYSMINIMIZE, 0,         0 )
#endif
#ifdef INCLUDE_MAXIMIZE
        SELECTION("Ma~ximize", ID_SYSMAXIMIZE, 0,         0 )
#endif
        SEPARATOR
        SELECTION("~Close",    ID_SYSCLOSE,    CTRL_F4,   0 )
    ENDPOPDOWN
ENDMENU



#ifdef INCLUDE_WINDOWOPTIONS
#define offset 7
#else
#define offset 0
#endif

/* ------------ Display dialog box -------------- */
DIALOGBOX(Display)
    DB_TITLE("Display", -1, -1, 12+offset, 35)
#ifdef INCLUDE_WINDOWOPTIONS
    CONTROL(BOX,      "Window",    7, 1, 6,20, 0)
    CONTROL(CHECKBOX,    NULL,     9, 2, 1, 3, ID_TITLE)
    CONTROL(TEXT,     "~Title",   15, 2, 1, 5, ID_TITLE)
    CONTROL(CHECKBOX,    NULL,     9, 3, 1, 3, ID_BORDER)
    CONTROL(TEXT,     "~Border",  15, 3, 1, 6, ID_BORDER)
    CONTROL(CHECKBOX,    NULL,     9, 4, 1, 3, ID_STATUSBAR)
    CONTROL(TEXT,   "~Status bar",15, 4, 1,10, ID_STATUSBAR)
    CONTROL(CHECKBOX,    NULL,     9, 5, 1, 3, ID_TEXTURE)
    CONTROL(TEXT,     "Te~xture", 15, 5, 1, 7, ID_TEXTURE)
#endif
    CONTROL(BOX,      "Colors",    1, 1+offset,5,15, 0)
    CONTROL(RADIOBUTTON, NULL,     3, 2+offset,1,3,ID_COLOR)
    CONTROL(TEXT,     "Co~lor",    7, 2+offset,1,5,ID_COLOR)
    CONTROL(RADIOBUTTON, NULL,     3, 3+offset,1,3,ID_MONO)
    CONTROL(TEXT,     "~Mono",     7, 3+offset,1,4,ID_MONO)
    CONTROL(RADIOBUTTON, NULL,     3, 4+offset,1,3,ID_REVERSE)
    CONTROL(TEXT,     "~Reverse",  7, 4+offset,1,7,ID_REVERSE)

    CONTROL(BUTTON, "   ~OK   ",   2, 8+offset,1,8,ID_OK)
    CONTROL(BUTTON, " ~Cancel ",  12, 8+offset,1,8,ID_CANCEL)
    CONTROL(BUTTON, "  ~Help  ",  22, 8+offset,1,8,ID_HELP)
ENDDB


/* ----- default colors for color video system ----- */
ColorScheme color = {
    FALSE, 0,{
    /* ------------ NORMAL ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {WHITE, BLACK},     /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ---------- APPLICATION --------- */
   {{LIGHTGRAY, BLUE},  /* STD_COLOR    */
    {LIGHTGRAY, BLUE},  /* SELECT_COLOR */
    {LIGHTGRAY, BLUE},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLUE}}, /* HILITE_COLOR */

    /* ------------ TEXTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ TEXTVIEW ---------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ LISTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- EDITBOX ------------ */
   {{LIGHTGRAY, BLUE}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- MENUBAR ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, CYAN},      /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {DARKGRAY, RED}},   /* HILITE_COLOR
                          Inactive, Shortcut (both FG) */

    /* ---------- POPDOWNMENU --------- */
   {{BLACK, CYAN},      /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, CYAN},      /* FRAME_COLOR  */
    {DARKGRAY, RED}},   /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

#ifdef INCLUDE_PICTUREBOX
    /* ------------ PICTUREBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */
#endif

    /* ------------- DIALOG ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLUE}}, /* HILITE_COLOR */

    /* ------------ BOX --------------- */
   {{LIGHTGRAY, BLUE},  /* STD_COLOR    */
    {LIGHTGRAY, BLUE},  /* SELECT_COLOR */
    {LIGHTGRAY, BLUE},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLUE}}, /* HILITE_COLOR */

    /* ------------ BUTTON ------------ */
   {{BLACK, CYAN},      /* STD_COLOR    */
    {WHITE, CYAN},      /* SELECT_COLOR */
    {BLACK, CYAN},      /* FRAME_COLOR  */
    {DARKGRAY, RED}},   /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */
    /* ------------ COMBOBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------- TEXT ----------- */
   {{0xff, 0xff},  /* STD_COLOR    */
    {0xff, 0xff},  /* SELECT_COLOR */
    {0xff, 0xff},  /* FRAME_COLOR  */
    {0xff, 0xff}}, /* HILITE_COLOR */

    /* ------------- RADIOBUTTON ----------- */
   {{LIGHTGRAY, BLUE},  /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLUE},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLUE}}, /* HILITE_COLOR */

    /* ------------- CHECKBOX ----------- */
   {{LIGHTGRAY, BLUE},  /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLUE},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLUE}}, /* HILITE_COLOR */

    /* ------------ SPINBUTTON ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- ERRORBOX ----------- */
   {{YELLOW, RED},      /* STD_COLOR    */
    {YELLOW, RED},      /* SELECT_COLOR */
    {YELLOW, RED},      /* FRAME_COLOR  */
    {YELLOW, RED}},     /* HILITE_COLOR */

    /* ----------- MESSAGEBOX --------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- HELPBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLUE},  /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {WHITE, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- STATUSBAR ------------- */
   {{BLACK, CYAN},      /* STD_COLOR    */
    {BLACK, CYAN},      /* SELECT_COLOR */
    {BLACK, CYAN},      /* FRAME_COLOR  */
    {BLACK, CYAN}},     /* HILITE_COLOR */

    /* ----------- EDITOR ------------ */
   {{LIGHTGRAY, BLUE},  /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- GRAPHBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- LCDBOX ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- TITLEBAR ------------ */
   {{BLACK, CYAN},      /* STD_COLOR    */
    {BLACK, CYAN},      /* SELECT_COLOR */
    {CYAN, BLACK},      /* FRAME_COLOR  */
    {WHITE, CYAN}},     /* HILITE_COLOR */

    /* ------------ DUMMY ------------- */
   {{GREEN, LIGHTGRAY}, /* STD_COLOR    */
    {GREEN, LIGHTGRAY}, /* SELECT_COLOR */
    {GREEN, LIGHTGRAY}, /* FRAME_COLOR  */
    {GREEN, LIGHTGRAY}} /* HILITE_COLOR */
}};

/* ----- default colors for mono video system ----- */
ColorScheme bw = {
    TRUE, 1,{
    /* ------------ NORMAL ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ---------- APPLICATION --------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ------------ TEXTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ LISTBOX ----------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- EDITBOX ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- MENUBAR ------------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive, Shortcut (both FG) */

    /* ---------- POPDOWNMENU --------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

#ifdef INCLUDE_PICTUREBOX
    /* ------------ PICTUREBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */
#endif

    /* ------------- DIALOG ----------- */
   {{LIGHTGRAY, BLACK},  /* STD_COLOR    */
    {BLACK, LIGHTGRAY},  /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}}, /* HILITE_COLOR */

	/* ------------ BOX --------------- */
   {{LIGHTGRAY, BLACK},  /* STD_COLOR    */
    {LIGHTGRAY, BLACK},  /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}}, /* HILITE_COLOR */

    /* ------------ BUTTON ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {WHITE, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */
    /* ------------ COMBOBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------- TEXT ----------- */
   {{0xff, 0xff},  /* STD_COLOR    */
    {0xff, 0xff},  /* SELECT_COLOR */
    {0xff, 0xff},  /* FRAME_COLOR  */
    {0xff, 0xff}}, /* HILITE_COLOR */

    /* ------------- RADIOBUTTON ----------- */
   {{LIGHTGRAY, BLACK},  /* STD_COLOR    */
    {BLACK, LIGHTGRAY},  /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}}, /* HILITE_COLOR */

    /* ------------- CHECKBOX ----------- */
   {{LIGHTGRAY, BLACK},  /* STD_COLOR    */
    {BLACK, LIGHTGRAY},  /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},  /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}}, /* HILITE_COLOR */

    /* ------------ SPINBUTTON ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- ERRORBOX ----------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ----------- MESSAGEBOX --------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ----------- HELPBOX ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {WHITE, BLACK},     /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {WHITE, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- STATUSBAR ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- EDITOR ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},/* HILITE_COLOR */

    /* ---------- GRAPHBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- LCDBOX ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- TITLEBAR ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ DUMMY ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}} /* HILITE_COLOR */
}};

/* ----- default colors for reverse mono video ----- */
ColorScheme reverse = {
    TRUE, 2,{
    /* ------------ NORMAL ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- APPLICATION --------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ TEXTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ LISTBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- EDITBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- MENUBAR ------------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive, Shortcut (both FG) */

    /* ---------- POPDOWNMENU --------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */

#ifdef INCLUDE_PICTUREBOX
    /* ------------ PICTUREBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */
#endif

    /* ------------- DIALOG ----------- */
   {{BLACK, LIGHTGRAY},  /* STD_COLOR    */
    {LIGHTGRAY, BLACK},  /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}}, /* HILITE_COLOR */

    /* ------------ BOX --------------- */
   {{BLACK, LIGHTGRAY},  /* STD_COLOR    */
    {BLACK, LIGHTGRAY},  /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}}, /* HILITE_COLOR */

    /* ------------ BUTTON ------------ */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {WHITE, BLACK},     /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {DARKGRAY, WHITE}}, /* HILITE_COLOR
                           Inactive ,Shortcut (both FG) */
    /* ------------ COMBOBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------- TEXT ----------- */
   {{0xff, 0xff},  /* STD_COLOR    */
    {0xff, 0xff},  /* SELECT_COLOR */
    {0xff, 0xff},  /* FRAME_COLOR  */
    {0xff, 0xff}}, /* HILITE_COLOR */

    /* ------------- RADIOBUTTON ----------- */
   {{BLACK, LIGHTGRAY},  /* STD_COLOR    */
    {LIGHTGRAY, BLACK},  /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}}, /* HILITE_COLOR */

    /* ------------- CHECKBOX ----------- */
   {{BLACK, LIGHTGRAY},  /* STD_COLOR    */
    {LIGHTGRAY, BLACK},  /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},  /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}}, /* HILITE_COLOR */

    /* ------------ SPINBUTTON ----------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- ERRORBOX ----------- */
   {{BLACK, LIGHTGRAY},      /* STD_COLOR    */
    {BLACK, LIGHTGRAY},      /* SELECT_COLOR */
    {BLACK, LIGHTGRAY},      /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},     /* HILITE_COLOR */

    /* ----------- MESSAGEBOX --------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ----------- HELPBOX ------------ */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {WHITE, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- STATUSBAR ------------- */
   {{LIGHTGRAY, BLACK},      /* STD_COLOR    */
    {LIGHTGRAY, BLACK},      /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},      /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},     /* HILITE_COLOR */

   /* ----------- EDITOR ------------ */
  {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
   {BLACK, LIGHTGRAY}, /* SELECT_COLOR */
   {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
   {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ GRAPHBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ------------ LCDBOX ----------- */
   {{BLACK, LIGHTGRAY}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {BLACK, LIGHTGRAY}, /* FRAME_COLOR  */
    {BLACK, LIGHTGRAY}},/* HILITE_COLOR */

    /* ---------- TITLEBAR ------------ */
   {{LIGHTGRAY, BLACK},      /* STD_COLOR    */
    {LIGHTGRAY, BLACK},      /* SELECT_COLOR */
    {LIGHTGRAY, BLACK},      /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}},     /* HILITE_COLOR */

    /* ------------ DUMMY ------------- */
   {{LIGHTGRAY, BLACK}, /* STD_COLOR    */
    {LIGHTGRAY, BLACK}, /* SELECT_COLOR */
    {LIGHTGRAY, BLACK}, /* FRAME_COLOR  */
    {LIGHTGRAY, BLACK}} /* HILITE_COLOR */
}};
/* -------------- text.c -------------- */

int TextProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int len;
    CTLWINDOW *ct = GetControl(wnd);
    char *cp, *cp2;
    switch (msg) {
        case SETFOCUS:
            return TRUE;
        case LEFT_BUTTON:
            return TRUE;
        case PAINT:
            if (ct == NULL || ct->itext == NULL || GetText(wnd) != NULL)
                break;
            cp2 = ct->itext;
            len = min(ct->dwnd.h, MsgHeight(cp2));
            cp = cp2;
            for (int i = 0; i < len; i++)    {
                int mlen;
                char *txt = cp;
                char *cp1 = cp;
                char *np = strchr(cp, '\n');
                if (np != NULL)
                    *np = '\0';
                mlen = strlen(cp);
                while ((cp1=strchr(cp1,SHORTCUTCHAR)) != NULL) {
                    mlen += 3;
                    cp1++;
                }
                if (np != NULL)
                    *np = '\n';
                txt = DFmalloc(mlen+1);
                 CopyCommand(txt, cp, FALSE, WndBackground(wnd));
                txt[mlen] = '\0';
                SendMessage(wnd, ADDTEXT, (PARAM)txt, 0);
                if ((cp = strchr(cp, '\n')) != NULL)
                    cp++;
                free(txt);
            }
            break;
        default:
            break;
    }
    return BaseWndProc(TEXT, wnd, msg, p1, p2);
}
/* ------------- textbox.c ------------ */

static void ComputeWindowTop(WINDOW);
static void ComputeWindowLeft(WINDOW);
static int ComputeVScrollBox(WINDOW);
static int ComputeHScrollBox(WINDOW);
static void MoveScrollBox(WINDOW, int);
static char *GetTextLine(WINDOW, int);

BOOL VSliding;
BOOL HSliding;

/* ------------ ADDTEXT Message -------------- */
static BOOL TBAddTextMsg(WINDOW wnd, char *txt)
{
    /* --- append text to the textbox's buffer --- */
    unsigned adln = strlen(txt);
    if (adln > (unsigned)0xfff0)
        return FALSE;
    if (wnd->text != NULL)    {
        /* ---- appending to existing text ---- */
        unsigned txln = strlen(wnd->text);
        if ((long)txln+adln > (unsigned) 0xfff0)
            return FALSE;
        if (txln+adln > wnd->textlen)    {
            wnd->text = DFrealloc(wnd->text, txln+adln+3);
            wnd->textlen = txln+adln+1;
        }
    } else    {
        /* ------ 1st text appended ------ */
        wnd->text = DFcalloc(1, adln+3);
        wnd->textlen = adln+1;
    }
    wnd->TextChanged = TRUE;
    if (wnd->text != NULL)    {
        /* ---- append the text ---- */
        strcat(wnd->text, txt);
        strcat(wnd->text, "\n");
        BuildTextPointers(wnd);
        return TRUE;
    }
    return FALSE;
}

/* ------------ DELETETEXT Message -------------- */
static void DeleteTextMsg(WINDOW wnd, int lno)
{
    char *cp1 = TextLine(wnd, lno);
    --wnd->wlines;
    if (lno == wnd->wlines)
        *cp1 = '\0';
    else 	{
        char *cp2 = TextLine(wnd, lno+1);
        memmove(cp1, cp2, strlen(cp2)+1);
    }
    BuildTextPointers(wnd);
}

/* ------------ INSERTTEXT Message -------------- */
static void InsertTextMsg(WINDOW wnd, char *txt, int lno)
{
    if (TBAddTextMsg(wnd, txt))	{
        int len = strlen(txt)+1;
        char *cp2 = TextLine(wnd, lno);
        char *cp1 = cp2+len;
        memmove(cp1, cp2, strlen(cp2)-len);
        strcpy(cp2, txt);
        *(cp2+len-1) = '\n';
        BuildTextPointers(wnd);
        wnd->TextChanged = TRUE;
    }
}

/* ------------ SETTEXT Message -------------- */
static void TBSetTextMsg(WINDOW wnd, char *txt)
{
    /* -- assign new text value to textbox buffer -- */
    unsigned int len = strlen(txt)+1;
    SendMessage(wnd, CLEARTEXT, 0, 0);
    wnd->textlen = len;
    wnd->text=DFrealloc(wnd->text, len+1);
    wnd->text[len] = '\0';
    strcpy(wnd->text, txt);
    BuildTextPointers(wnd);
}

/* ------------ CLEARTEXT Message -------------- */
static void TBClearTextMsg(WINDOW wnd)
{
    /* ----- clear text from textbox ----- */
    if (wnd->text != NULL)
        free(wnd->text);
    wnd->text = NULL;
    wnd->textlen = 0;
    wnd->wlines = 0;
    wnd->textwidth = 0;
    wnd->wtop = wnd->wleft = 0;
    ClearTextBlock(wnd);
    ClearTextPointers(wnd);
}

/* ------------ KEYBOARD Message -------------- */
static int TBKeyboardMsg(WINDOW wnd, PARAM p1)
{
    switch ((int) p1)    {
        case UP:
            return SendMessage(wnd,SCROLL,FALSE,0);
        case DN:
            return SendMessage(wnd,SCROLL,TRUE,0);
#ifdef HOOKKEYB
        case FWD: /* right arrow */
#else
        case RARROW: /* formerly called FWD */
#endif
            return SendMessage(wnd,HORIZSCROLL,TRUE,0);
#ifndef HOOKKEYB
        case LARROW: /* hope this makes sense */
#endif
        case BS:
            return SendMessage(wnd,HORIZSCROLL,FALSE,0);
        case PGUP:
            return SendMessage(wnd,SCROLLPAGE,FALSE,0);
        case PGDN:
            return SendMessage(wnd,SCROLLPAGE,TRUE,0);
        case CTRL_PGUP:
            return SendMessage(wnd,HORIZPAGE,FALSE,0);
        case CTRL_PGDN:
            return SendMessage(wnd,HORIZPAGE,TRUE,0);
        case HOME:
            return SendMessage(wnd,SCROLLDOC,TRUE,0);
        case END:
            return SendMessage(wnd,SCROLLDOC,FALSE,0);
        default:
            break;
    }
    return FALSE;
}

/* ------------ LEFT_BUTTON Message -------------- */
static int TBLeftButtonMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int mx = (int) p1 - GetLeft(wnd);
    int my = (int) p2 - GetTop(wnd);
    if (TestAttribute(wnd, VSCROLLBAR) && mx == WindowWidth(wnd)-1)    {
        /* -------- in the right border ------- */
        if (my == 0 || my == ClientHeight(wnd)+1)
            /* --- above or below the scroll bar --- */
            return FALSE;
        if (my == 1)
            /* -------- top scroll button --------- */
            return SendMessage(wnd, SCROLL, FALSE, 0);
        if (my == ClientHeight(wnd))
            /* -------- bottom scroll button --------- */
            return SendMessage(wnd, SCROLL, TRUE, 0);
        /* ---------- in the scroll bar ----------- */
        if (!VSliding && my-1 == wnd->VScrollBox) {
            RECT rc;
            VSliding = TRUE;
            rc.lf = rc.rt = GetRight(wnd);
            rc.tp = GetTop(wnd)+2;
            rc.bt = GetBottom(wnd)-2;
            return SendMessage(NULL, MOUSE_TRAVEL,
                (PARAM) &rc, 0);
        }
        if (my-1 < wnd->VScrollBox)
            return SendMessage(wnd,SCROLLPAGE,FALSE,0);
        if (my-1 > wnd->VScrollBox)
            return SendMessage(wnd,SCROLLPAGE,TRUE,0);
    }
    if (TestAttribute(wnd, HSCROLLBAR) && my == WindowHeight(wnd)-1) {
        /* -------- in the bottom border ------- */
        if (mx == 0 || my == ClientWidth(wnd)+1)
            /* ------  outside the scroll bar ---- */
            return FALSE;
        if (mx == 1)
            return SendMessage(wnd, HORIZSCROLL,FALSE,0);
        if (mx == WindowWidth(wnd)-2)
            return SendMessage(wnd, HORIZSCROLL,TRUE,0);
        if (!HSliding && mx-1 == wnd->HScrollBox)    {
            /* --- hit the scroll box --- */
            RECT rc;
            rc.lf = GetLeft(wnd)+2;
            rc.rt = GetRight(wnd)-2;
            rc.tp = rc.bt = GetBottom(wnd);
            /* - keep the mouse in the scroll bar - */
            SendMessage(NULL,MOUSE_TRAVEL,(PARAM)&rc,0);
            HSliding = TRUE;
            return TRUE;
        }
        if (mx-1 < wnd->HScrollBox)
            return SendMessage(wnd,HORIZPAGE,FALSE,0);
        if (mx-1 > wnd->HScrollBox)
            return SendMessage(wnd,HORIZPAGE,TRUE,0);
    }
    return FALSE;
}

/* ------------ MOUSE_MOVED Message -------------- */
static BOOL TBMouseMovedMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    int mx = (int) p1 - GetLeft(wnd);
    int my = (int) p2 - GetTop(wnd);
    if (VSliding)    {
        /* ---- dragging the vertical scroll box --- */
        if (my-1 != wnd->VScrollBox)    {
            foreground = FrameForeground(wnd);
            background = FrameBackground(wnd);
            wputch(wnd, SCROLLBARCHAR, WindowWidth(wnd)-1, wnd->VScrollBox+1);
            wnd->VScrollBox = my-1;
            wputch(wnd, SCROLLBOXCHAR, WindowWidth(wnd)-1, my);
        }
        return TRUE;
    }
    if (HSliding)    {
        /* --- dragging the horizontal scroll box --- */
        if (mx-1 != wnd->HScrollBox)    {
            foreground = FrameForeground(wnd);
            background = FrameBackground(wnd);
            wputch(wnd, SCROLLBARCHAR, wnd->HScrollBox+1, WindowHeight(wnd)-1);
            wnd->HScrollBox = mx-1;
            wputch(wnd, SCROLLBOXCHAR, mx, WindowHeight(wnd)-1);
        }
        return TRUE;
    }
    return FALSE;
}

/* ------------ BUTTON_RELEASED Message -------------- */
static void TBButtonReleasedMsg(WINDOW wnd)
{
    if (HSliding || VSliding)    {
        /* release the mouse ouside the scroll bar */
        SendMessage(NULL, MOUSE_TRAVEL, 0, 0);
        VSliding ? ComputeWindowTop(wnd):ComputeWindowLeft(wnd);
        SendMessage(wnd, PAINT, 0, 0);
        SendMessage(wnd, KEYBOARD_CURSOR, 0, 0);
        VSliding = HSliding = FALSE;
    }
}

/* ------------ SCROLL Message -------------- */
static BOOL TBScrollMsg(WINDOW wnd, PARAM p1)
{
    /* ---- vertical scroll one line ---- */
    if (p1)    {
        /* ----- scroll one line up ----- */
        if (wnd->wtop+ClientHeight(wnd) >= wnd->wlines)
            return FALSE;
        wnd->wtop++;
    }
    else    {
        /* ----- scroll one line down ----- */
        if (wnd->wtop == 0)
            return FALSE;
        --wnd->wtop;
    }
    if (isVisible(wnd))    {
        RECT rc;
        rc = ClipRectangle(wnd, ClientRect(wnd));
        if (ValidRect(rc))    {
            /* ---- scroll the window ----- */
            if (wnd != inFocus)
                SendMessage(wnd, PAINT, 0, 0);
            else    {
                scroll_window(wnd, rc, (int)p1);
                if (!(int)p1)
                    /* -- write top line (down) -- */
                    WriteTextLine(wnd,NULL,wnd->wtop,FALSE);
                else    {
                    /* -- write bottom line (up) -- */
                    int y=RectBottom(rc)-GetClientTop(wnd);
                    WriteTextLine(wnd, NULL,
                        wnd->wtop+y, FALSE);
                }
            }
        }
        /* ---- reset the scroll box ---- */
        if (TestAttribute(wnd, VSCROLLBAR))    {
            int vscrollbox = ComputeVScrollBox(wnd);
            if (vscrollbox != wnd->VScrollBox)
                MoveScrollBox(wnd, vscrollbox);
        }
    }
    return TRUE;
}

/* ------------ HORIZSCROLL Message -------------- */
static BOOL TBHorizScrollMsg(WINDOW wnd, PARAM p1)
{
    /* --- horizontal scroll one column --- */
    if (p1)    {
        /* --- scroll left --- */
        if (wnd->wleft + ClientWidth(wnd)-1 >= wnd->textwidth)
            return FALSE;
        wnd->wleft++;
    }
    else	{
        /* --- scroll right --- */
        if (wnd->wleft == 0)
            return FALSE;
        --wnd->wleft;
    }
    SendMessage(wnd, PAINT, 0, 0);
    return TRUE;
}

/* ------------  SCROLLPAGE Message -------------- */
static void TBScrollPageMsg(WINDOW wnd, PARAM p1)
{
    /* --- vertical scroll one page --- */
    if ((int) p1 == FALSE)    {
        /* ---- page up ---- */
        if (wnd->wtop)
            wnd->wtop -= ClientHeight(wnd);
    }
    else     {
        /* ---- page down ---- */
        if (wnd->wtop+ClientHeight(wnd) < wnd->wlines) {
            wnd->wtop += ClientHeight(wnd);
            if (wnd->wtop>wnd->wlines-ClientHeight(wnd))
                wnd->wtop=wnd->wlines-ClientHeight(wnd);
        }
    }
    if (wnd->wtop < 0)
        wnd->wtop = 0;
    SendMessage(wnd, PAINT, 0, 0);
}

/* ------------ HORIZSCROLLPAGE Message -------------- */
static void TBHorizScrollPageMsg(WINDOW wnd, PARAM p1)
{
    /* --- horizontal scroll one page --- */
    if ((int) p1 == FALSE)
        /* ---- page left ----- */
        wnd->wleft -= ClientWidth(wnd);
    else    {
        /* ---- page right ----- */
        wnd->wleft += ClientWidth(wnd);
        if (wnd->wleft > wnd->textwidth-ClientWidth(wnd))
            wnd->wleft = wnd->textwidth-ClientWidth(wnd);
    }
    if (wnd->wleft < 0)
        wnd->wleft = 0;
    SendMessage(wnd, PAINT, 0, 0);
}

/* ------------ SCROLLDOC Message -------------- */
static void ScrollDocMsg(WINDOW wnd, PARAM p1)
{
    /* --- scroll to beginning or end of document --- */
    if ((int) p1)
        wnd->wtop = wnd->wleft = 0;
    else if (wnd->wtop+ClientHeight(wnd) < wnd->wlines){
        wnd->wtop = wnd->wlines-ClientHeight(wnd);
        wnd->wleft = 0;
    }
    SendMessage(wnd, PAINT, 0, 0);
}

/* ------------ PAINT Message -------------- */
static void TBPaintMsg(WINDOW wnd, PARAM p1, PARAM p2)
{
    /* ------ paint the client area ----- */
    RECT rc, rcc;
    int y;
    char blankline[201];

    /* ----- build the rectangle to paint ----- */
    if ((RECT *)p1 == NULL)
        rc=RelativeWindowRect(wnd, WindowRect(wnd));
    else
        rc= *(RECT *)p1;
    if (TestAttribute(wnd, HASBORDER) &&
            RectRight(rc) >= WindowWidth(wnd)-1) {
        if (RectLeft(rc) >= WindowWidth(wnd)-1)
            return;
        RectRight(rc) = WindowWidth(wnd)-2;
    }
    rcc = AdjustRectangle(wnd, rc);

    if (!p2 && wnd != inFocus)
        ClipString++;

    /* ----- blank line for padding ----- */
    memset(blankline, ' ', SCREENWIDTH);
    blankline[RectRight(rcc)+1] = '\0';

    /* ------- each line within rectangle ------ */
    for (y = RectTop(rc); y <= RectBottom(rc); y++){
        int yy;
        /* ---- test outside of Client area ---- */
        if (TestAttribute(wnd,
                    HASBORDER | HASTITLEBAR))    {
            if (y < TopBorderAdj(wnd))
                continue;
            if (y > WindowHeight(wnd)-2)
                continue;
        }
        yy = y-TopBorderAdj(wnd);
        if (yy < wnd->wlines-wnd->wtop)
            /* ---- paint a text line ---- */
            WriteTextLine(wnd, &rc,
                        yy+wnd->wtop, FALSE);
        else    {
            /* ---- paint a blank line ---- */
            SetStandardColor(wnd);
            writeline(wnd, blankline+RectLeft(rcc),
                    RectLeft(rcc)+BorderAdj(wnd), y, FALSE);
        }
    }
    /* ------- position the scroll box ------- */
    if (TestAttribute(wnd, VSCROLLBAR|HSCROLLBAR)) {
        int hscrollbox = ComputeHScrollBox(wnd);
        int vscrollbox = ComputeVScrollBox(wnd);
        if (hscrollbox != wnd->HScrollBox ||
                vscrollbox != wnd->VScrollBox)    {
            wnd->HScrollBox = hscrollbox;
            wnd->VScrollBox = vscrollbox;
            SendMessage(wnd, BORDER, p1, 0);
        }
    }
    if (!p2 && wnd != inFocus)
        --ClipString;
}

/* ------------ CLOSE_WINDOW Message -------------- */
static void TBCloseWindowMsg(WINDOW wnd)
{
    SendMessage(wnd, CLEARTEXT, 0, 0);
    if (wnd->TextPointers != NULL)    {
        free(wnd->TextPointers);
        wnd->TextPointers = NULL;
    }
}

/* ----------- TEXTBOX Message-processing Module ----------- */
int TextBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            wnd->HScrollBox = wnd->VScrollBox = 1;
            ClearTextPointers(wnd);
            break;
        case ADDTEXT:
            return TBAddTextMsg(wnd, (char *) p1);
        case DELETETEXT:
            DeleteTextMsg(wnd, (int) p1);
            return TRUE;
        case INSERTTEXT:
            InsertTextMsg(wnd, (char *) p1, (int) p2);
            return TRUE;
        case SETTEXT:
            TBSetTextMsg(wnd, (char *) p1);
            return TRUE;
        case CLEARTEXT:
            TBClearTextMsg(wnd);
            break;
        case KEYBOARD:
            if (WindowMoving || WindowSizing)
                break;
            if (TBKeyboardMsg(wnd, p1))
                return TRUE;
            break;
        case LEFT_BUTTON:
            if (WindowSizing || WindowMoving)
                return FALSE;
            if (TBLeftButtonMsg(wnd, p1, p2))
                return TRUE;
            break;
        case MOUSE_MOVED:
            if (MouseMovedMsg(wnd, p1, p2))
                return TRUE;
            break;
        case BUTTON_RELEASED:
            TBButtonReleasedMsg(wnd);
            break;
        case SCROLL:
            return TBScrollMsg(wnd, p1);
        case HORIZSCROLL:
            return TBHorizScrollMsg(wnd, p1);
        case SCROLLPAGE:
            TBScrollPageMsg(wnd, p1);
            return TRUE;
        case HORIZPAGE:
            TBHorizScrollPageMsg(wnd, p1);
            return TRUE;
        case SCROLLDOC:
            ScrollDocMsg(wnd, p1);
            return TRUE;
        case PAINT:
            if (isVisible(wnd))    {
                TBPaintMsg(wnd, p1, p2);
                return FALSE;
            }
            break;
        case CLOSE_WINDOW:
            TBCloseWindowMsg(wnd);
            break;
        default:
            break;
    }
    return BaseWndProc(TEXTBOX, wnd, msg, p1, p2);
}

/* ------ compute the vertical scroll box position from
                   the text pointers --------- */
static int ComputeVScrollBox(WINDOW wnd)
{
    int pagelen = wnd->wlines - ClientHeight(wnd);
    int barlen = ClientHeight(wnd)-2;
    int lines_tick;
    int vscrollbox;

    if (pagelen < 1 || barlen < 1)
        vscrollbox = 1;
    else    {
        if (pagelen > barlen)
            lines_tick = pagelen / barlen;
        else
            lines_tick = barlen / pagelen;
        vscrollbox = 1 + (wnd->wtop / lines_tick);
        if (vscrollbox > ClientHeight(wnd)-2 ||
                wnd->wtop + ClientHeight(wnd) >= wnd->wlines)
            vscrollbox = ClientHeight(wnd)-2;
    }
    return vscrollbox;
}

/* ---- compute top text line from scroll box position ---- */
static void ComputeWindowTop(WINDOW wnd)
{
    int pagelen = wnd->wlines - ClientHeight(wnd);
    if (wnd->VScrollBox == 0)
        wnd->wtop = 0;
    else if (wnd->VScrollBox == ClientHeight(wnd)-2)
        wnd->wtop = pagelen;
    else    {
        int barlen = ClientHeight(wnd)-2;
        int lines_tick;

        if (pagelen > barlen)
            lines_tick = barlen ? (pagelen / barlen) : 0;
        else
            lines_tick = pagelen ? (barlen / pagelen) : 0;
        wnd->wtop = (wnd->VScrollBox-1) * lines_tick;
        if (wnd->wtop + ClientHeight(wnd) > wnd->wlines)
            wnd->wtop = pagelen;
    }
    if (wnd->wtop < 0)
        wnd->wtop = 0;
}

/* ------ compute the horizontal scroll box position from
                   the text pointers --------- */
static int ComputeHScrollBox(WINDOW wnd)
{
    int pagewidth = wnd->textwidth - ClientWidth(wnd);
    int barlen = ClientWidth(wnd)-2;
    int chars_tick;
    int hscrollbox;

    if (pagewidth < 1 || barlen < 1)
        hscrollbox = 1;
    else     {
        if (pagewidth > barlen)
            chars_tick = barlen ? (pagewidth / barlen) : 0;
        else
            chars_tick = pagewidth ? (barlen / pagewidth) : 0;
        hscrollbox = 1 + (chars_tick ? (wnd->wleft / chars_tick) : 0);
        if (hscrollbox > ClientWidth(wnd)-2 ||
                wnd->wleft + ClientWidth(wnd) >= wnd->textwidth)
            hscrollbox = ClientWidth(wnd)-2;
    }
    return hscrollbox;
}

/* ---- compute left column from scroll box position ---- */
static void ComputeWindowLeft(WINDOW wnd)
{
    int pagewidth = wnd->textwidth - ClientWidth(wnd);

    if (wnd->HScrollBox == 0)
        wnd->wleft = 0;
    else if (wnd->HScrollBox == ClientWidth(wnd)-2)
        wnd->wleft = pagewidth;
    else    {
        int barlen = ClientWidth(wnd)-2;
        int chars_tick;

        if (pagewidth > barlen)
            chars_tick = pagewidth / barlen;
        else
            chars_tick = barlen / pagewidth;
        wnd->wleft = (wnd->HScrollBox-1) * chars_tick;
        if (wnd->wleft + ClientWidth(wnd) > wnd->textwidth)
            wnd->wleft = pagewidth;
    }
    if (wnd->wleft < 0)
        wnd->wleft = 0;
}

/* ----- get the text to a specified line ----- */
static char *GetTextLine(WINDOW wnd, int selection)
{
    char *line;
    int len = 0;
    char *cp, *cp1;
    cp = cp1 = TextLine(wnd, selection);
    while (*cp && *cp != '\n')    {
        len++;
        cp++;
    }
    line = DFmalloc(len+7);
    memmove(line, cp1, len);
    line[len] = '\0';
    return line;
}

/* ------- write a line of text to a textbox window ------- */
void WriteTextLine(WINDOW wnd, RECT *rcc, int y, BOOL reverse)
{
    int i, len = 0, dif = 0;
    char line[1000];
    RECT rc;
    char *lp, *svlp;
    int lnlen;
    BOOL trunc = FALSE;

    /* ------ make sure y is inside the window ----- */
    if (y < wnd->wtop || y >= wnd->wtop+ClientHeight(wnd))
        return;

    /* ---- build the rectangle within which can write ---- */
    if (rcc == NULL)    {
        rc = RelativeWindowRect(wnd, WindowRect(wnd));
        if (TestAttribute(wnd, HASBORDER) &&
                RectRight(rc) >= WindowWidth(wnd)-1)
            RectRight(rc) = WindowWidth(wnd)-2;
    } else
        rc = *rcc;

    /* ----- make sure rectangle is within window ------ */
    if (RectLeft(rc) >= WindowWidth(wnd)-1)
        return;
    if (RectRight(rc) == 0)
        return;
    rc = AdjustRectangle(wnd, rc);
    if (y-wnd->wtop<RectTop(rc) || y-wnd->wtop>RectBottom(rc))
        return;

    /* --- get the text and length of the text line --- */
    lp = svlp = GetTextLine(wnd, y);
    if (svlp == NULL)
        return;
    lnlen = LineLength(lp);

    if (wnd->protect)	{
        char *pp = lp;
        while (*pp) {
            if (isprint(*pp))
                *pp = '*'; /* why that??? */
            pp++;
        }
    }

    /* -------- insert block color change controls ------- */
    if (TextBlockMarked(wnd))    {
        int bbl = wnd->BlkBegLine;
        int bel = wnd->BlkEndLine;
        int bbc = wnd->BlkBegCol;
        int bec = wnd->BlkEndCol;
        int by = y;

        /* ----- put lowest marker first ----- */
        if (bbl > bel)    {
            swapi(bbl, bel);
            swapi(bbc, bec);
        }
        if (bbl == bel && bbc > bec)
            swapi(bbc, bec);

        if (by >= bbl && by <= bel)    {
            /* ------ the block includes this line ----- */
            int blkbeg = 0;
            int blkend = lnlen;
            if (!(by > bbl && by < bel))    {
                /* --- the entire line is not in the block -- */
                if (by == bbl)
                    /* ---- the block begins on this line --- */
                    blkbeg = bbc;
                if (by == bel)
                    /* ---- the block ends on this line ---- */
                    blkend = bec;
            }
            if (blkend == 0 && lnlen == 0)	{
                strcpy(lp, " ");
                blkend++;
            }
            /* ----- insert the reset color token ----- */
            memmove(lp+blkend+1,lp+blkend,strlen(lp+blkend)+1);
            lp[blkend] = RESETCOLOR;
            /* ----- insert the change color token ----- */
            memmove(lp+blkbeg+3,lp+blkbeg,strlen(lp+blkbeg)+1);
            lp[blkbeg] = CHANGECOLOR;
            /* ----- insert the color tokens ----- */
            SetReverseColor(wnd);
            lp[blkbeg+1] = foreground | 0x80;
            lp[blkbeg+2] = background | 0x80;
            lnlen += 4;
        }
    }
    /* - make sure left margin doesn't overlap color change - */
    for (i = 0; i < wnd->wleft+3; i++)    {
        if (*(lp+i) == '\0')
            break;
        if (*(char *)(lp + i) == RESETCOLOR)
            break;
    }
    if (*(lp+i) && i < wnd->wleft+3)    {
        if (wnd->wleft+4 > lnlen) {
            trunc = TRUE;
        } else
            lp += 4;
    } else {
        /* --- it does, shift the color change over --- */
        for (int i = 0; i < wnd->wleft; i++) {
            if (*(lp+i) == '\0')
                break;
            if (*(char *)(lp + i) == CHANGECOLOR)    {
                *(lp+wnd->wleft+2) = *(lp+i+2);
                *(lp+wnd->wleft+1) = *(lp+i+1);
                *(lp+wnd->wleft) = *(lp+i);
                break;
            }
        }
    }
    /* ------ build the line to display -------- */
    if (!trunc)    {
        if (lnlen < wnd->wleft)
            lnlen = 0;
        else
            lp += wnd->wleft;
        if (lnlen > RectLeft(rc))    {
            /* ---- the line exceeds the rectangle ---- */
            int ct = RectLeft(rc);
            char *initlp = lp;
            /* --- point to end of clipped line --- */
            while (ct)    {
                if (*(char *)lp == CHANGECOLOR)
                    lp += 3;
                else if (*(char *)lp == RESETCOLOR)
                    lp++;
                else
                    lp++, --ct;
            }
            if (RectLeft(rc))    {
                char *lpp = lp;
                while (*lpp)    {
                    if (*(char*)lpp==CHANGECOLOR)
                        break;
                    if (*(char*)lpp==RESETCOLOR) {
                        lpp = lp;
                        while (lpp >= initlp)    {
                            if (*(char *)lpp == CHANGECOLOR) {
                                lp -= 3;
                                memmove(lp,lpp,3);
                                break;
                            }
                            --lpp;
                        }
                        break;
                    }
                    lpp++;
                }
            }
            lnlen = LineLength(lp);
            len = min(lnlen, RectWidth(rc));
            dif = strlen(lp) - lnlen;
            len += dif;
            if (len > 0)
                strncpy(line, lp, len);
        }
    }
    /* -------- pad the line --------- */
    while (len < RectWidth(rc)+dif)
        line[len++] = ' ';
    line[len] = '\0';
    dif = 0;
    /* ------ establish the line's main color ----- */
    if (reverse)    {
        char *cp = line;
        SetReverseColor(wnd);
        while ((cp = strchr(cp, CHANGECOLOR)) != NULL)    {
            cp += 2;
            *cp++ = background | 0x80;
        }
        if (*(char *)line == CHANGECOLOR)
            dif = 3;
    } else
        SetStandardColor(wnd);
    /* ------- display the line -------- */
    writeline(wnd, line+dif, RectLeft(rc)+BorderAdj(wnd), y-wnd->wtop+TopBorderAdj(wnd), FALSE);
    free(svlp);
}

void MarkTextBlock(WINDOW wnd, int BegLine, int BegCol, int EndLine, int EndCol)
{
    wnd->BlkBegLine = BegLine;
    wnd->BlkEndLine = EndLine;
    wnd->BlkBegCol = BegCol;
    wnd->BlkEndCol = EndCol;
}

/* ----- clear and initialize text line pointer array ----- */
void ClearTextPointers(WINDOW wnd)
{
    wnd->TextPointers = DFrealloc(wnd->TextPointers, sizeof(int));
    *(wnd->TextPointers) = 0;
}

#define INITLINES 100

/* ---- build array of pointers to text lines ---- */
void BuildTextPointers(WINDOW wnd)
{
    char *cp = wnd->text, *cp1;
    int incrs = INITLINES;
    unsigned int off;
    wnd->textwidth = wnd->wlines = 0;
    while (*cp)    {
        if (incrs == INITLINES)    {
            incrs = 0;
            wnd->TextPointers = DFrealloc(wnd->TextPointers,
                    (wnd->wlines + INITLINES) * sizeof(int));
        }
        off = (unsigned int) (cp - wnd->text);
        *((wnd->TextPointers) + wnd->wlines) = off;
        wnd->wlines++;
        incrs++;
        cp1 = cp;
        while (*cp && *cp != '\n')
            cp++;
        wnd->textwidth = max(wnd->textwidth, (unsigned int) (cp - cp1));
        if (*cp)
            cp++;
    }
}

static void MoveScrollBox(WINDOW wnd, int vscrollbox)
{
    foreground = FrameForeground(wnd);
    background = FrameBackground(wnd);
    wputch(wnd, SCROLLBARCHAR, WindowWidth(wnd)-1, wnd->VScrollBox+1);
    wputch(wnd, SCROLLBOXCHAR, WindowWidth(wnd)-1, vscrollbox+1);
    wnd->VScrollBox = vscrollbox;
}

int TextLineNumber(WINDOW wnd, char *lp)
{
    int lineno;
    for (lineno = 0; lineno < wnd->wlines; lineno++)    {
        char *cp = wnd->text + *((wnd->TextPointers) + lineno);
        if (cp == lp)
            return lineno;
        if (cp > lp)
            break;
    }
    return lineno-1;
}


/* --------------------- video.c -------------------- */

BOOL ClipString;

static char *video_address = NULL; //! video memory in bytes
void movefromscreen(void *bf, int off, int len);
void movetoscreen(void *bf, int off, int len);

/* -- read a rectangle of video memory into a save buffer -- */
void getvideo(RECT rc, void *bf)
{
    int ht = RectBottom(rc)-RectTop(rc)+1;
    int bytes_row = (RectRight(rc)-RectLeft(rc)+1) * 2;
    unsigned vadr = vad(RectLeft(rc), RectTop(rc));
    hide_mousecursor();
    while (ht--) {
        movefromscreen(bf, vadr, bytes_row);
        vadr += SCREENWIDTH*2;
        bf = (char *)bf + bytes_row;
    }
    show_mousecursor();
}

/* -- write a rectangle of video memory from a save buffer -- */
void storevideo(RECT rc, void *bf)
{
    int ht = RectBottom(rc)-RectTop(rc)+1;
    int bytes_row = (RectRight(rc)-RectLeft(rc)+1) * 2;
    unsigned vadr = vad(RectLeft(rc), RectTop(rc));
    hide_mousecursor();
    while (ht--)    {
        movetoscreen(bf, vadr, bytes_row);
        vadr += SCREENWIDTH*2;
        bf = (char *)bf + bytes_row;
    }
    show_mousecursor();
}

/* -------- read a character of video memory ------- */
con_char_t GetVideoChar(int x, int y)
{
    con_char_t c;
    hide_mousecursor();
    c = vpeek(video_address, vad(x,y));
    show_mousecursor();
    return c;
}

/* -------- write a character of video memory ------- */
void PutVideoChar(int x, int y, con_char_t c)
{
    if (x < SCREENWIDTH && y < SCREENHEIGHT) {
        hide_mousecursor();
        vpoke(video_address, vad(x,y), c);
        show_mousecursor();
    }
}

BOOL CharInView(WINDOW wnd, int x, int y)
{
    WINDOW nwnd = NextWindow(wnd);
    WINDOW pwnd;
    RECT rc;
    int x1 = GetLeft(wnd)+x;
    int y1 = GetTop(wnd)+y;

    if (!TestAttribute(wnd, VISIBLE))
        return FALSE;
    if (!TestAttribute(wnd, NOCLIP))    {
        WINDOW wnd1 = GetParent(wnd);
        while (wnd1 != NULL)    {
            /* --- clip character to parent's borders -- */
            if (!TestAttribute(wnd1, VISIBLE))
                return FALSE;
            if (!InsideRect(x1, y1, ClientRect(wnd1)))
                return FALSE;
            wnd1 = GetParent(wnd1);
        }
    }
    while (nwnd != NULL)	{
        if (!isHidden(nwnd) /* && !isAncestor(wnd, nwnd) */ )	{
            rc = WindowRect(nwnd);
            if (TestAttribute(nwnd, SHADOW))    {
                RectBottom(rc)++;
                RectRight(rc)++;
            }
            if (!TestAttribute(nwnd, NOCLIP))	{
                pwnd = nwnd;
                while (GetParent(pwnd))	{
                    pwnd = GetParent(pwnd);
                    rc = subRectangle(rc, ClientRect(pwnd));
                }
            }
            if (InsideRect(x1,y1,rc))
                return FALSE;
        }
        nwnd = NextWindow(nwnd);
    }
    return (x1 < SCREENWIDTH && y1 < SCREENHEIGHT);
}

/* -------- write a character to a window ------- */
void wputch(WINDOW wnd, int c, int x, int y)
{
    if (CharInView(wnd, x, y))	{
        int ch = (c & 255) | (clr(foreground, background) << 8);
        int xc = GetLeft(wnd)+x;
        int yc = GetTop(wnd)+y;
        hide_mousecursor();
        vpoke(video_address, vad(xc, yc), ch);
        show_mousecursor();
    }
}

/* ------- write a string to a window ---------- */
void wputs(WINDOW wnd, void *s, int x, int y)
{
    int x1=GetLeft(wnd)+x;
    int x2=x1;
    int y1=GetTop(wnd)+y;

    if (x1 < SCREENWIDTH && y1 < SCREENHEIGHT && isVisible(wnd))
        {
        con_char_t ln[200];
        con_char_t *cp1=ln;
        int fg=foreground;
        int bg=background;
        int len;
        int off=0;
        char *str=s;

        while (*str)
            {
            if (*str == CHANGECOLOR)
                {
                int fgcode, bgcode;	/* new 0.7c: sanity checks */
                str++;
                fgcode = (*str++);
                bgcode = (*str++);
                if ((fgcode & 0x80) && (bgcode & 0x80) &&
                    !(fgcode & 0x70) && !(bgcode & 0x70)) {
                    foreground = fgcode & 0x7f;
                    background = bgcode & 0x7f;
                    continue;
                } else {    /* this also makes CHANGECOLOR almost normal */
                    str--;  /* and useable as character in your texts... */
                    str--;  /* treat as non-escape sequence */
                    str--;
                }
                }

            if (*str == RESETCOLOR)
                {
                foreground = fg & 0x7f;
                background = bg & 0x7f;
                str++;
                continue;
                }

#ifdef TAB_TOGGLING	/* made consistent with editor.c - 0.7c */
            if (*str == ('\t' | 0x80) || *str == ('\f' | 0x80))
                *cp1 = ' ' | (clr(foreground, background) << 8);
            else 
#endif
                *cp1 = (*str & 255) | (clr(foreground, background) << 8);

            if (ClipString)
                if (!CharInView(wnd, x, y))
                    *cp1 = vpeek(video_address, vad(x2,y1));

            cp1++;
            str++;
            x++;
            x2++;
            }

        foreground = fg;
        background = bg;
        len = (int)(cp1-ln);
        if (x1+len > SCREENWIDTH)
            len = SCREENWIDTH-x1;

        if (!ClipString && !TestAttribute(wnd, NOCLIP))
            {
            /* -- clip the line to within ancestor windows -- */
            RECT rc = WindowRect(wnd);
            WINDOW nwnd = GetParent(wnd);

            while (len > 0 && nwnd != NULL)
                {
                if (!isVisible(nwnd))
                    {
                    len = 0;
                    break;
                    }

                rc = subRectangle(rc, ClientRect(nwnd));
                nwnd = GetParent(nwnd);
                }

            while (len > 0 && !InsideRect(x1+off,y1,rc))
                {
                off++;
                --len;
                }

            if (len > 0)
                {
                x2 = x1+len-1;
                while (len && !InsideRect(x2,y1,rc))
                    {
                    --x2;
                    --len;
                    }

                }

            }

        if (len > 0)
            {
            hide_mousecursor();
            movetoscreen(ln+off, vad(x1+off,y1), len*2);
            show_mousecursor();
            }
        }
}

/* --------- get the current video mode -------- */
char *get_videomode(void)
{
    videomode();

    DEV_ASSERT(sizeof(char_info_t) == 2);

    if(!video_address)
        video_address = DFmalloc(SCREENWIDTH*SCREENHEIGHT*2);
    
    return video_address;
}

/* --------- scroll parts of a screen buffer --------- */
static void scroll_console_output( int xsrc, int ysrc, int xdst, int ydst, int w, int h )
{
    struct _screen_buffer {
      char_info_t *data;
      int width, height;
    } screen_buffer_info = { (char_info_t *)video_address, SCREENWIDTH, SCREENHEIGHT }, *screen_buffer = &screen_buffer_info;

    char_info_t *psrc, *pdst;

    if (xsrc < 0 || ysrc < 0 || xdst < 0 || ydst < 0 ||
        xsrc + w > screen_buffer->width  ||
        xdst + w > screen_buffer->width  ||
        ysrc + h > screen_buffer->height ||
        ydst + h > screen_buffer->height ||
        w == 0 || h == 0)
    {
        return;
    }

    if (ysrc < ydst)
    {
        psrc = &screen_buffer->data[(ysrc + h - 1) * screen_buffer->width + xsrc];
        pdst = &screen_buffer->data[(ydst + h - 1) * screen_buffer->width + xdst];

        for (int j = h; j > 0; j--)
        {
            memcpy(pdst, psrc, w * sizeof(*pdst) );
            pdst -= screen_buffer->width;
            psrc -= screen_buffer->width;
        }
    } else {
        psrc = &screen_buffer->data[ysrc * screen_buffer->width + xsrc];
        pdst = &screen_buffer->data[ydst * screen_buffer->width + xdst];

        for (int j = 0; j < h; j++)
        {
            /* we use memmove here because when psrc and pdst are the same,
            * copies are done on the same row, so the dst and src blocks
            * can overlap */
            memmove( pdst, psrc, w * sizeof(*pdst) );
            pdst += screen_buffer->width;
            psrc += screen_buffer->width;
        }
    }
}

/* --------- scroll the window. d: 1 = up, 0 = dn ---------- */
void scroll_window(WINDOW wnd, RECT rc, int d)
{
    if (RectTop(rc) != RectBottom(rc)) {
        if (d == 0) d = -1; else d = 1;
        scroll_console_output( rc.lf, rc.bt, rc.lf, rc.bt+d, rc.rt-rc.lf, rc.tp-rc.bt );
    }
}

void movetoscreen(void *bf, int off, int len)
{
    memcpy(video_address+off, bf, len);
}

void movefromscreen(void *bf, int off, int len)
{
    memcpy(bf, video_address+off, len);
}


/*  Little watch icon

*/

int WatchIconProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn;
    static int tick = 0;
    static char *hands[] = { " \300 ", " \332 ", " \252 ", " \331 " };
    switch (msg)
        {
        case CREATE_WINDOW:
            tick = 0;
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            SendMessage(wnd, CAPTURE_MOUSE, 0, 0);
            SendMessage(wnd, HIDE_MOUSE, 0, 0);
            SendMessage(wnd, CAPTURE_KEYBOARD, 0, 0);
            SendMessage(wnd, CAPTURE_CLOCK, 0, 0);
            return rtn;
        case CLOCKTICK:
            ++tick;
            tick &= 3;
            SendMessage(wnd->PrevClock, msg, p1, p2);
            /* (fall through and paint) */
        case PAINT:
            SetStandardColor(wnd);
            writeline(wnd, hands[tick], 1, 1, FALSE);
            return TRUE;
        case BORDER:
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            writeline(wnd, "I", 2, 0, FALSE);
            return rtn;
        case MOUSE_MOVED:
            SendMessage(wnd, HIDE_WINDOW, 0, 0);
            SendMessage(wnd, MOVE, p1, p2);
            SendMessage(wnd, SHOW_WINDOW, 0, 0);
            return TRUE;
        case CLOSE_WINDOW:
            SendMessage(wnd, RELEASE_CLOCK, 0, 0);
            SendMessage(wnd, RELEASE_MOUSE, 0, 0);
            SendMessage(wnd, RELEASE_KEYBOARD, 0, 0);
            SendMessage(wnd, SHOW_MOUSE, 0, 0);
            break;
        default:
            break;
        }

    return DefaultWndProc(wnd, msg, p1, p2);

}

WINDOW WatchIcon(void)
{
    int mx, my;
    WINDOW wnd;

    SendMessage(NULL, CURRENT_MOUSE_CURSOR, (PARAM) &mx, (PARAM) &my);
    wnd = CreateWindow(BOX, NULL, mx, my, 3, 5, NULL,NULL, WatchIconProc, VISIBLE | HASBORDER | SHADOW | SAVESELF);
    return wnd;

}
/* ---------- window.c ------------- */

WINDOW inFocus = NULL;

int foreground, background;   /* current video colors */

static void TopLine(WINDOW, int, RECT);

/* --------- create a window ------------ */
WINDOW CreateWindow(
    CLASS cls,                /* class of this window       */
    const char *ttl,          /* title or NULL              */
    int left, int top,        /* upper left coordinates     */
    int height, int width,    /* dimensions                 */
    void *extension,          /* pointer to additional data */
    WINDOW parent,            /* parent of this window      */
    int (*wndproc)(struct window *,MESSAGE,PARAM,PARAM),
    int attrib)               /* window attribute           */
{
    WINDOW wnd = DFcalloc(1, sizeof(struct window));

    get_videomode();

    if (wnd != NULL)    {
        int base;
        memset(wnd, 0, sizeof(struct window));	/* new 0.7c */
        /* ----- height, width = -1: fill the screen ------- */
        if (height == -1)
            height = SCREENHEIGHT;
        if (width == -1)
            width = SCREENWIDTH;
        /* ----- coordinates -1, -1 = center the window ---- */
        if (left == -1)
            wnd->rc.lf = (SCREENWIDTH-width)/2;
        else
            wnd->rc.lf = left;
        if (top == -1)
            wnd->rc.tp = (SCREENHEIGHT-height)/2;
        else
            wnd->rc.tp = top;
        wnd->attrib = attrib;
        if (ttl != NULL)
            if (*ttl != '\0')
                AddAttribute(wnd, HASTITLEBAR);
        if (wndproc == NULL)
            wnd->wndproc = classdefs[cls].wndproc;
        else
            wnd->wndproc = wndproc;
        /* ---- derive attributes of base classes ---- */
        base = cls;
        while (base != -1)    {
            AddAttribute(wnd, classdefs[base].attrib);
            base = classdefs[base].base;
        }
        if (parent)	{
            if (!TestAttribute(wnd, NOCLIP))    {
                /* -- keep upper left within borders of parent - */
                wnd->rc.lf = max(wnd->rc.lf,GetClientLeft(parent));
                wnd->rc.tp = max(wnd->rc.tp,GetClientTop(parent));
            }
        }
        else
            parent = ApplicationWindow;
        wnd->cls = cls;
        wnd->extension = extension;
        wnd->rc.rt = GetLeft(wnd)+width-1;
        wnd->rc.bt = GetTop(wnd)+height-1;
        wnd->ht = height;
        wnd->wd = width;
        if (ttl != NULL)
            InsertTitle(wnd, ttl);
        wnd->parent = parent;
        wnd->oldcondition = wnd->condition = ISRESTORED;
        wnd->RestoredRC = wnd->rc;
        InitWindowColors(wnd);
        SendMessage(wnd, CREATE_WINDOW, 0, 0);
        if (isVisible(wnd))
            SendMessage(wnd, SHOW_WINDOW, 0, 0);
    }
    return wnd;
}

/* -------- add a title to a window --------- */
void AddTitle(WINDOW wnd, const char *ttl)
{
    InsertTitle(wnd, ttl);
    SendMessage(wnd, BORDER, 0, 0);
}

/* ----- insert a title into a window ---------- */
void InsertTitle(WINDOW wnd, const char *ttl)
{
    wnd->title=DFrealloc(wnd->title,strlen(ttl)+1);
    strcpy(wnd->title, ttl);
}

static char line[300];

/* ------ write a line to video window client area ------ */
void writeline(WINDOW wnd, const char *str, int x, int y, BOOL pad)
{
    char *cp;
    int len;
    int dif;
    char wline[200];

    memset(wline, 0, 200);
    len = LineLength(str);
    dif = strlen(str) - len;
    strncpy(wline, str, ClientWidth(wnd) + dif);
    if (pad)    {
        cp = wline+strlen(wline);
        while (len++ < ClientWidth(wnd)-x)
            *cp++ = ' ';
    }
    wputs(wnd, wline, x, y);
}

RECT AdjustRectangle(WINDOW wnd, RECT rc)
{
    /* -------- adjust the rectangle ------- */
    if (TestAttribute(wnd, HASBORDER))    {
        if (RectLeft(rc) == 0)
            --rc.rt;
        else if (RectLeft(rc) < RectRight(rc) &&
                RectLeft(rc) < WindowWidth(wnd)+1)
            --rc.lf;
    }
    if (TestAttribute(wnd, HASBORDER | HASTITLEBAR))    {
        if (RectTop(rc) == 0)
            --rc.bt;
        else if (RectTop(rc) < RectBottom(rc) &&
                RectTop(rc) < WindowHeight(wnd)+1)
            --rc.tp;
    }
    RectRight(rc) = max(RectLeft(rc),
                        min(RectRight(rc),WindowWidth(wnd)));
    RectBottom(rc) = max(RectTop(rc),
                        min(RectBottom(rc),WindowHeight(wnd)));
    return rc;
}

/* -------- display a window's title --------- */
void DisplayTitle(WINDOW wnd, RECT *rcc)
{
    if (GetTitle(wnd) != NULL)	{
        int tlen = min(strlen(GetTitle(wnd)), WindowWidth(wnd)-2);
        int tend = WindowWidth(wnd)-3-BorderAdj(wnd);
        RECT rc;

        if (rcc == NULL)
            rc = RelativeWindowRect(wnd, WindowRect(wnd));
        else
            rc = *rcc;
        rc = AdjustRectangle(wnd, rc);

        if (SendMessage(wnd, TITLE, (PARAM) rcc, 0))    {
            if (wnd == inFocus)    {
                foreground = SysConfig.VideoCurrentColorScheme.clrArray[TITLEBAR] [HILITE_COLOR] [FG];
                background = SysConfig.VideoCurrentColorScheme.clrArray[TITLEBAR] [HILITE_COLOR] [BG];
            } else {
                foreground = SysConfig.VideoCurrentColorScheme.clrArray[TITLEBAR] [STD_COLOR] [FG];
                background = SysConfig.VideoCurrentColorScheme.clrArray[TITLEBAR] [STD_COLOR] [BG];
            }
            memset(line,' ',WindowWidth(wnd));
#ifdef INCLUDE_MINIMIZE
            if (wnd->condition != ISMINIMIZED)
#endif
                strncpy(line + ((WindowWidth(wnd)-2 - tlen) / 2), wnd->title, tlen);
            if (TestAttribute(wnd, CONTROLBOX))
                line[2-BorderAdj(wnd)] = CONTROLBOXCHAR;
            if (TestAttribute(wnd, MINMAXBOX))    {
                switch (wnd->condition)    {
                    case ISRESTORED:
#ifdef INCLUDE_MAXIMIZE
                        line[tend+1] = MAXPOINTER;
#endif
#ifdef INCLUDE_MINIMIZE
                        line[tend]   = MINPOINTER;
#endif
                    break;
#ifdef INCLUDE_MINIMIZE
                    case ISMINIMIZED:
                        line[tend+1] = MAXPOINTER;
                        break;
#endif
#ifdef INCLUDE_MAXIMIZE
                    case ISMAXIMIZED:
#ifdef INCLUDE_MINIMIZE
                        line[tend]   = MINPOINTER;
#endif
#ifdef INCLUDE_RESTORE
                        line[tend+1] = RESTOREPOINTER;
#endif
                        break;
#endif
                    default:
                        break;
                }
            }
            line[RectRight(rc)+1] = line[tend+3] = '\0';
            if (wnd != inFocus)
                ClipString++;
            writeline(wnd, line+RectLeft(rc), RectLeft(rc)+BorderAdj(wnd), 0, FALSE);
            ClipString = 0;
        }
    }
}

#ifdef INCLUDE_MINIMIZE
#define MinTest() (wnd->condition == ISMINIMIZED) ||
#else
#define MinTest() /**/
#endif

#ifdef INCLUDE_MAXIMIZE
#define MaxTest() (wnd->condition == ISMAXIMIZED) ||
#else
#define MaxTest() /**/
#endif

#define NoShadow(wnd) (TestAttribute(wnd, SHADOW) == 0 || MinTest() MaxTest() SysConfig.VideoCurrentColorScheme.isMonoScheme)

/* --- display right border shadow character of a window --- */
static void shadow_char(WINDOW wnd, int y)
{
    int fg = foreground;
    int bg = background;
    int x = WindowWidth(wnd);
    int c = videochar(GetLeft(wnd)+x, GetTop(wnd)+y);

    if (NoShadow(wnd))
        return;
    foreground = DARKGRAY;
    background = BLACK;
    wputch(wnd, c, x, y);
    foreground = fg;
    background = bg;
}

/* --- display the bottom border shadow line for a window -- */
static void shadowline(WINDOW wnd, RECT rc)
{
    int i;
    int y = GetBottom(wnd)+1;
    int fg = foreground;
    int bg = background;

    if (NoShadow(wnd))
        return;
    for (i = 0; i < WindowWidth(wnd)+1; i++)
        line[i] = videochar(GetLeft(wnd)+i, y);
    line[i] = '\0';
    foreground = DARKGRAY;
    background = BLACK;
    line[RectRight(rc)+1] = '\0';
    if (RectLeft(rc) == 0)
        rc.lf++;
    ClipString++;
    wputs(wnd, line+RectLeft(rc), RectLeft(rc),
        WindowHeight(wnd));
    --ClipString;
    foreground = fg;
    background = bg;
}

static RECT ParamRect(WINDOW wnd, RECT *rcc)
{
    RECT rc;
    if (rcc == NULL)    {
        rc = RelativeWindowRect(wnd, WindowRect(wnd));
        if (TestAttribute(wnd, SHADOW))    {
            rc.rt++;
            rc.bt++;
        }
    } else
        rc = *rcc;
    return rc;
}

void PaintShadow(WINDOW wnd)
{
    RECT rc = ParamRect(wnd, NULL);
    for (int y = 1; y < WindowHeight(wnd); y++)
        shadow_char(wnd, y);
    shadowline(wnd, rc);
}

static unsigned int SeCorner(WINDOW wnd, unsigned int stdse)
{
    if (TestAttribute(wnd, SIZEABLE) && wnd->condition == ISRESTORED)
        return SIZETOKEN;
    return stdse;
}

/* ------- display a window's border ----- */
void RepaintBorder(WINDOW wnd, RECT *rcc)
{
    int y;
    unsigned int lin, side, ne, nw, se, sw;
    RECT rc, clrc;

    if (!TestAttribute(wnd, HASBORDER))
        return;
    rc = ParamRect(wnd, rcc);
    clrc = AdjustRectangle(wnd, rc);

    if (wnd == inFocus)    {
        lin  = FOCUS_LINE;
        side = FOCUS_SIDE;
        ne   = FOCUS_NE;
        nw   = FOCUS_NW;
        se   = SeCorner(wnd, FOCUS_SE);
        sw   = FOCUS_SW;
    }
    else    {
        lin  = LINE;
        side = SIDE;
        ne   = NE;
        nw   = NW;
        se   = SeCorner(wnd, SE);
        sw   = SW;
    }
    line[WindowWidth(wnd)] = '\0';


    /* ---------- window title ------------ */
    if (TestAttribute(wnd, HASTITLEBAR))
        if (RectTop(rc) == 0)
            if (RectLeft(rc) < WindowWidth(wnd)-BorderAdj(wnd))
                DisplayTitle(wnd, &rc);
    foreground = FrameForeground(wnd);
    background = FrameBackground(wnd);
    /* -------- top frame corners --------- */
    if (RectTop(rc) == 0)    {
        if (RectLeft(rc) == 0)
            wputch(wnd, nw, 0, 0);
        if (RectLeft(rc) < WindowWidth(wnd))    {
            if (RectRight(rc) >= WindowWidth(wnd)-1)
                wputch(wnd, ne, WindowWidth(wnd)-1, 0);
            TopLine(wnd, lin, clrc);
        }
    }

    /* ----------- window body ------------ */
    for (y = RectTop(rc); y <= RectBottom(rc); y++)    {
        int ch;
        if (y == 0 || y >= WindowHeight(wnd)-1)
            continue;
        if (RectLeft(rc) == 0)
            wputch(wnd, side, 0, y);
        if (RectLeft(rc) < WindowWidth(wnd) &&
                RectRight(rc) >= WindowWidth(wnd)-1)    {
            if (TestAttribute(wnd, VSCROLLBAR))
                ch = (    y == 1 ? UPSCROLLBOX      :
                          y == WindowHeight(wnd)-2  ?
                                DOWNSCROLLBOX       :
                          y-1 == wnd->VScrollBox    ?
                                SCROLLBOXCHAR       :
                          SCROLLBARCHAR );
            else
                ch = side;
            wputch(wnd, ch, WindowWidth(wnd)-1, y);
        }
        if (RectRight(rc) == WindowWidth(wnd))
            shadow_char(wnd, y);
    }

    if (RectTop(rc) <= WindowHeight(wnd)-1 &&
            RectBottom(rc) >= WindowHeight(wnd)-1)    {
        /* -------- bottom frame corners ---------- */
        if (RectLeft(rc) == 0)
            wputch(wnd, sw, 0, WindowHeight(wnd)-1);
        if (RectLeft(rc) < WindowWidth(wnd) &&
                RectRight(rc) >= WindowWidth(wnd)-1)
            wputch(wnd, se, WindowWidth(wnd)-1,
                WindowHeight(wnd)-1);

        if (wnd->StatusBar == NULL)	{
            /* ----------- bottom line ------------- */
            memset(line,lin,WindowWidth(wnd)-1);
            if (TestAttribute(wnd, HSCROLLBAR))    {
                line[0] = LEFTSCROLLBOX;
                line[WindowWidth(wnd)-3] = RIGHTSCROLLBOX;
                memset(line+1, SCROLLBARCHAR, WindowWidth(wnd)-4);
                line[wnd->HScrollBox] = SCROLLBOXCHAR;
            }
            line[WindowWidth(wnd)-2] = line[RectRight(rc)] = '\0';
            if (RectLeft(rc) != RectRight(rc) ||
                (RectLeft(rc) && RectLeft(rc) < WindowWidth(wnd)-1))	{
                if (wnd != inFocus)
                    ClipString++;
                writeline(wnd, line+(RectLeft(clrc)), RectLeft(clrc)+1, WindowHeight(wnd)-1, FALSE);
                ClipString = 0;
            }
        }
        if (RectRight(rc) == WindowWidth(wnd))
            shadow_char(wnd, WindowHeight(wnd)-1);
    }
    if (RectBottom(rc) == WindowHeight(wnd))
        /* ---------- bottom shadow ------------- */
        shadowline(wnd, rc);
}

static void TopLine(WINDOW wnd, int lin, RECT rc)
{
    if (TestAttribute(wnd, HASMENUBAR))
        return;
    if (TestAttribute(wnd, HASTITLEBAR) && GetTitle(wnd))
        return;
    if (RectLeft(rc) == 0)	{
        RectLeft(rc) += BorderAdj(wnd);
        RectRight(rc) += BorderAdj(wnd);
    }
    if (RectRight(rc) < WindowWidth(wnd)-1)
        RectRight(rc)++;

    if (RectLeft(rc) < RectRight(rc))    {
        /* ----------- top line ------------- */
        memset(line,lin,WindowWidth(wnd)-1);
        if (TestAttribute(wnd, CONTROLBOX))	{
            strncpy(line+1, "   ", 3);
            *(line+2) = CONTROLBOXCHAR;
        }
        line[RectRight(rc)] = '\0';
        writeline(wnd, line+RectLeft(rc),
            RectLeft(rc), 0, FALSE);
    }
}

/* ------ clear the data space of a window -------- */
void ClearWindow(WINDOW wnd, RECT *rcc, int clrchar)
{
    if (isVisible(wnd))    {
        int y;
        RECT rc = rcc ? *rcc : RelativeWindowRect(wnd, WindowRect(wnd));

        int top = TopBorderAdj(wnd);
        int bot = WindowHeight(wnd)-1-BottomBorderAdj(wnd);

        if (RectLeft(rc) == 0)
            RectLeft(rc) = BorderAdj(wnd);
        if (RectRight(rc) > WindowWidth(wnd)-1)
            RectRight(rc) = WindowWidth(wnd)-1;
        SetStandardColor(wnd);
        memset(line, clrchar, sizeof line);
        line[RectRight(rc)+1] = '\0';
        for (y = RectTop(rc); y <= RectBottom(rc); y++)    {
            if (y < top || y > bot)
                continue;
            writeline(wnd, line+(RectLeft(rc)), RectLeft(rc), y, FALSE);
        }
    }
}

/* ------ compute the logical line length of a window ------ */
int LineLength(const char *ln)
{
    int len = strlen(ln);
    const char *cp = ln;
    while ((cp = strchr(cp, CHANGECOLOR)) != NULL)    {
        cp++;
        len -= 3;
    }
    cp = ln;
    while ((cp = strchr(cp, RESETCOLOR)) != NULL)    {
        cp++;
        --len;
    }
    return len;
}

void InitWindowColors(WINDOW wnd)
{
    int cls = GetClass(wnd);
    /* window classes without assigned colors inherit parent's colors */
    if (SysConfig.VideoCurrentColorScheme.clrArray[cls][0][0] == 0xff && GetParent(wnd) != NULL)
        cls = GetClass(GetParent(wnd));
    /* ---------- set the colors ---------- */
    for (int fbg = 0; fbg < 2; fbg++)
        for (int col = 0; col < 4; col++)
            wnd->WindowColors[col][fbg] = SysConfig.VideoCurrentColorScheme.clrArray[cls][col][fbg];
}

void PutWindowChar(WINDOW wnd, int c, int x, int y)
{
    if (x < ClientWidth(wnd) && y < ClientHeight(wnd))
        wputch(wnd, c, x+BorderAdj(wnd), y+TopBorderAdj(wnd));
}

void PutWindowLine(WINDOW wnd, char *s, int x, int y)
{
    int saved = FALSE, sv;
    if (x < ClientWidth(wnd) && y < ClientHeight(wnd))	{
        char *en = s+ClientWidth(wnd)-x;
        if (strlen(s)+x > ClientWidth(wnd))	{
            sv = *en;
            *en = '\0';
            saved = TRUE;
        }
        ClipString++;
        wputs(wnd, s, x+BorderAdj(wnd), y+TopBorderAdj(wnd));
        --ClipString;
        if (saved)
            *en = sv;
    }
}


/* --------- set window colors --------- */
void SetStandardColor(WINDOW wnd)
{
    foreground = WndForeground(wnd);
    background = WndBackground(wnd);
}

void SetReverseColor(WINDOW wnd)
{
    foreground = SelectForeground(wnd);
    background = SelectBackground(wnd);
}

/* ---------- port.c ------------- */

static int _width = -1, _height = -1;

BOOL init_console(int console_width, int console_height) {
    _width = console_width;
    _height = console_height;
    return TRUE;
}

int getScreenWidth()
{
  DEV_ASSERT(_width>0);
  DEV_ASSERT(_height>0);

  return _width;
}

int getScreenHeight()
{
  DEV_ASSERT(_width>0);
  DEV_ASSERT(_height>0);

  return _height;
}

#ifdef __linux__
#define _XOPEN_SOURCE 700   /* SUSv4 */
#endif

#define DOTDOT_HANDLE    0L
#define INVALID_HANDLE  -1L

typedef struct fhandle_t {
    DIR* dstream;
    short dironly;
    char* spec;
} fhandle_t;

static void fill_finddata(struct stat* st, const char* name, _finddata_t* fileinfo);
static intptr_t findfirst_dotdot(const char* filespec, _finddata_t* fileinfo);
static intptr_t findfirst_in_directory(const char* dirpath, const char* spec, _finddata_t* fileinfo);
static void findfirst_set_errno();

intptr_t _findfirst(const char* filespec, _finddata_t* fileinfo) {
    char* rmslash;      /* Rightmost forward slash in filespec. */
    const char* spec;   /* Specification string. */

    if (!fileinfo || !filespec) {
        errno = EINVAL;
        return INVALID_HANDLE;
    }

    if (filespec[0] == '\0') {
        errno = ENOENT;
        return INVALID_HANDLE;
    }

    rmslash = strrchr(filespec, '/');

    if (rmslash != NULL) {
        /*
         * At least one forward slash was found in the filespec
         * string, and rmslash points to the rightmost one. The
         * specification part, if any, begins right after it.
         */
        spec = rmslash + 1;
    } else {
        /*
         * Since no slash was found in the filespec string, its
         * entire content can be used as our spec string.
         */
        spec = filespec;
    }

    if (strcmp(spec, ".") == 0 || strcmp(spec, "..") == 0) {
        /* On Windows, . and .. must return canonicalized names. */
        return findfirst_dotdot(filespec, fileinfo);
    } else if (rmslash == filespec) {
        /*
         * Since the rightmost slash is the first character, we're
         * looking for something located at the file system's root.
         */
        return findfirst_in_directory("/", spec, fileinfo);
    } else if (rmslash != NULL) {
        /*
         * Since the rightmost slash isn't the first one, we're
         * looking for something located in a specific folder. In
         * order to open this folder, we split the folder path from
         * the specification part by overwriting the rightmost
         * forward slash.
         */
        size_t pathlen = strlen(filespec) +1;
        char* dirpath = alloca(pathlen);
        memcpy(dirpath, filespec, pathlen);
        dirpath[rmslash - filespec] = '\0';
        return findfirst_in_directory(dirpath, spec, fileinfo);
    } else {
        /*
         * Since the filespec doesn't contain any forward slash,
         * we're looking for something located in the current
         * directory.
         */
        return findfirst_in_directory(".", spec, fileinfo);
    }
}

/* Perfom a scan in the directory identified by dirpath. */
static intptr_t findfirst_in_directory(const char* dirpath, const char* spec, _finddata_t* fileinfo) {
    DIR* dstream;
    fhandle_t* ffhandle;

    if (spec[0] == '\0') {
        errno = ENOENT;
        return INVALID_HANDLE;
    }

    if ((dstream = opendir(dirpath)) == NULL) {
        findfirst_set_errno();
        return INVALID_HANDLE;
    }

    if ((ffhandle = malloc(sizeof(fhandle_t))) == NULL) {
        closedir(dstream);
        errno = ENOMEM;
        return INVALID_HANDLE;
    }

    /* On Windows, *. returns only directories. */
    ffhandle->dironly = strcmp(spec, "*.") == 0 ? 1 : 0;
    ffhandle->dstream = dstream;
    ffhandle->spec = strdup(spec);

    if (_findnext((intptr_t) ffhandle, fileinfo) != 0) {
        _findclose((intptr_t) ffhandle);
        errno = ENOENT;
        return INVALID_HANDLE;
    }

    return (intptr_t) ffhandle;
}

/* On Windows, . and .. return canonicalized directory names. */
static intptr_t findfirst_dotdot(const char* filespec, struct _finddata_t* fileinfo) {
    char* dirname;
    char* canonicalized;
    struct stat st;

    if (stat(filespec, &st) != 0) {
        findfirst_set_errno();
        return INVALID_HANDLE;
    }

    /* Resolve filespec to an absolute path. */
    if ((canonicalized = realpath(filespec, NULL)) == NULL) {
        findfirst_set_errno();
        return INVALID_HANDLE;
    }

    /* Retrieve the basename from it. */
    dirname = basename(canonicalized);

    /* Make sure that we actually have a basename. */
    if (dirname[0] == '\0') {
        free(canonicalized);
        errno = ENOENT;
        return INVALID_HANDLE;
    }

    /* Make sure that we won't overflow finddata_t::name. */
    if (strlen(dirname) > 259) {
        free(canonicalized);
        errno = ENOMEM;
        return INVALID_HANDLE;
    }

    fill_finddata(&st, dirname, fileinfo);

    free(canonicalized);

    /*
     * Return a special handle since we can't return
     * NULL. The findnext and findclose functions know
     * about this custom handle.
     */
    return DOTDOT_HANDLE;
}

/*
 * Windows implementation of _findfirst either returns EINVAL,
 * ENOENT or ENOMEM. This function makes sure that the above
 * implementation doesn't return anything else when an error
 * condition is encountered.
 */
static void findfirst_set_errno() {
    if (errno != ENOENT &&
        errno != ENOMEM &&
        errno != EINVAL) {
        errno = EINVAL;
    }
}

static void fill_finddata(struct stat* st, const char* name, _finddata_t* fileinfo) {
    fileinfo->attrib = S_ISDIR(st->st_mode) ? _A_SUBDIR : _A_NORMAL;
    fileinfo->size = st->st_size;
    fileinfo->time_create = st->st_ctime;
    fileinfo->time_access = st->st_atime;
    fileinfo->time_write = st->st_mtime;
    strcpy(fileinfo->name, name);
}

int _findnext(intptr_t fhandle, _finddata_t* fileinfo) {
    struct dirent entry, *result;
    struct fhandle_t* handle;
    struct stat st;

    if (fhandle == DOTDOT_HANDLE) {
        errno = ENOENT;
        return -1;
    }

    if (fhandle == INVALID_HANDLE || !fileinfo) {
        errno = EINVAL;
        return -1;
    }

    handle = (struct fhandle_t*)fhandle;

    while (readdir_r(handle->dstream, &entry, &result) == 0 && result != NULL) {
        if (!handle->dironly && !match_spec(handle->spec, entry.d_name)) {
            continue;
        }

        if (fnstatat(entry.d_name, &st) == -1) {
            return -1;
        }

        if (handle->dironly && !S_ISDIR(st.st_mode)) {
            continue;
        }

        fill_finddata(&st, entry.d_name, fileinfo);

        return 0;
    }

    errno = ENOENT;
    return -1;
}

int _findclose(intptr_t fhandle) {
    struct fhandle_t* handle;

    if (fhandle == DOTDOT_HANDLE) {
        return 0;
    }

    if (fhandle == INVALID_HANDLE) {
        errno = ENOENT;
        return -1;
    }

    handle = (struct fhandle_t*) fhandle;

    closedir(handle->dstream);
    free(handle->spec);
    free(handle);

    return 0;
}

int _match_spec(const char* spec, const char* text) {
    /* If the whole specification string was consumed and
     * the input text is also exhausted: it's a match.
     */
    if (spec[0] == '\0' && text[0] == '\0') {
        return 1;
    }

    /* A star matches 0 or more characters. */
    if (spec[0] == '*') {
        /* Skip the star and try to find a match after it
         * by successively incrementing the text pointer.
         */
        do {
            if (_match_spec(spec + 1, text)) {
                return 1;
            }
        } while (*text++ != '\0');
    }

    /* An interrogation mark matches any character. Other
     * characters match themself. Also, if the input text
     * is exhausted but the specification isn't, there is
     * no match.
     */
    if (text[0] != '\0' && (spec[0] == '?' || spec[0] == text[0])) {
        return _match_spec(spec + 1, text + 1);
    }

    return 0;
}

int match_spec(const char* spec, const char* text) {
    /* On Windows, *.* matches everything. */
    if (strcmp(spec, "*.*") == 0) {
        return 1;
    }

    return _match_spec(spec, text);
}

/* ---------- textview.c ------------- */

static char *_forward(char *s, int l) {
    while ( *s && l > 0 ) {
        --l; ++s;
    }
    return s;
}
                             
static int _window_print_internal(WINDOW wnd, int x, int y, int rw, int rh, enum alignments align, char *msg, BOOL can_split, BOOL count_only) {
    int cx = 0,cy = y;
    int minx, maxx, miny, maxy;
    const int maxw = ClientWidth(wnd), maxh = ClientHeight(wnd);
    if ( msg == NULL || x >= maxw || y >= maxh )
        return 0;
    if ( rh == 0 ) rh = maxh-y;
    if ( rw == 0 ) switch(align) {
        case ALIGN_LEFT : rw = maxw-x; break;
        case ALIGN_RIGHT : rw = x+1; break;
        case ALIGN_CENTER : default : rw = maxw; break;
    }
    miny = y;
    maxy = maxh-1;
    if (rh > 0) maxy = min(maxy,y+rh-1);
    switch (align) {
        case ALIGN_LEFT : minx = max(0,x); maxx = min(maxw-1,x+rw-1); break;
        case ALIGN_RIGHT : minx = max(0,x-rw+1); maxx = min(maxw-1,x); break;
        case ALIGN_CENTER : default : minx = max(0,x-rw/2); maxx = min(maxw-1,x+rw/2); break;
    }
 
    char *msg_dup = strdup(msg), *c = msg_dup;
    do {
        /* get \n delimited sub-message */
        char *end = strchr(c,'\n');
        char bak = 0;
        int cl;
        char *split = NULL;
        if ( end ) *end=0;
        cl = strlen(c);
        /* find starting x */
        switch (align) {
            case ALIGN_LEFT : cx=x; break;
            case ALIGN_RIGHT : cx=x-cl+1; break;
            case ALIGN_CENTER : cx= x-cl/2;break;
        }
        /* check if the string is completely out of the minx, miny, maxx, maxy frame */
        if ( cy >= miny && cy <= maxy && cx <= maxx && cx+cl -1 >= minx ) {
            if ( can_split && cy < maxy ) {
                /* if partially out of screen, try to split the sub-message */
                if ( cx < minx ) split = _forward(c, align == ALIGN_CENTER ? cl-2*(minx-cx) : cl-(minx-cx));
                else if ( align == ALIGN_CENTER ) {
                    if ( cx + cl/2 > maxx+1 ) split = _forward(c, maxx+1 - cx);
                } else {
                    if ( cx + cl > maxx+1 ) split = _forward(c, maxx+1 - cx);
                }
            }
            if ( split ) {
                char *oldsplit = split;
                while ( !isspace(*split) && split > c ) split --; // get back to the first space
                if (end) *end = '\n';
                if (!isspace(*split) ) {
                    split = oldsplit;
                }
                end=split;
                bak = *split;
                *split = 0;
                cl = strlen(c);
                switch (align) {
                    case ALIGN_LEFT : cx = x; break;
                    case ALIGN_RIGHT : cx = x-cl+1; break;
                    case ALIGN_CENTER : cx = x-cl/2;break;
                }
            }
            if ( cx < minx ) {
                /* truncate left part */
                c += minx-cx;
                cl -= minx-cx;
                cx = minx;
            }
            if ( cx + cl > maxx+1 ) {
                /* truncate right part */
                split = _forward(c, maxx+1 - cx);
                *split = 0;
            }
            /* render the sub-message */
            if ( cy >= 0 && cy < maxh )
                while (*c) {
                    if (! count_only) PutWindowChar(wnd, (int)(*c), cx, cy);
                    ++cx; ++c;
                }
        }
        if ( end ) {
            /* next line */
            if ( split && !isspace(bak) ) {
                *end = bak;
                c = end;
            } else {
                c = end+1;
            }
            cy++;
        } else c = NULL;
    } while ( c && cy < maxh && (rh == 0 || cy < y+rh) );

    free(msg_dup); // release duplicate
    
    return cy-y+1;
}

/* TextViewer
 * ----------
 * Only vertical scroller is allowed and supported. Text is word
 * wrapped automatically.
 */
int TextViewProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    switch (msg)    {
        case PAINT:
            if (GetText(wnd) == NULL)
                break;
            wnd->wlines = _window_print_internal(wnd, 0, 0, ClientWidth(wnd), ClientHeight(wnd), ALIGN_LEFT, GetText(wnd), TRUE, FALSE);
            /* ------- position the scroll box ------- */
            if (TestAttribute(wnd, VSCROLLBAR)) {
                int vscrollbox = ComputeVScrollBox(wnd);
                if (vscrollbox != wnd->VScrollBox)    {
                    wnd->VScrollBox = vscrollbox;
                    SendMessage(wnd, BORDER, p1, 0);
                }
            }
            return TRUE;
        default:
            break;
    }
    return BaseWndProc(TEXTVIEW, wnd, msg, p1, p2);
}

/* Graphing
 * --------
 * Controls that draws x-y plotting graph - with scrolling.
 */
int GraphBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    static const char div_top = '\xc2';
    static const char div_mid = '\xc4';
    static const char div_bot = '\xc1';

    /* CTLWINDOW *ct = GetControl(wnd); */
    RECT rc = ClientRect(wnd);
    switch (msg) {
        case PAINT:
            if (isVisible(wnd))
            {
                NormalProc(wnd, msg, p1, p2); // clear bg etc
                char *line = alloca(RectWidth(rc)+1);
                memset(line,div_top,RectWidth(rc)); line[RectWidth(rc)] = 0; PutWindowLine(wnd, line, 0, 0);
                memset(line,div_mid,RectWidth(rc)); line[RectWidth(rc)] = 0; PutWindowLine(wnd, line, 0, RectHeight(rc)/2);
                memset(line,div_bot,RectWidth(rc)); line[RectWidth(rc)] = 0; PutWindowLine(wnd, line, 0, RectHeight(rc)-1);
                return TRUE;
            }
            break;
        default:
            break;
    }
    return BaseWndProc(GRAPHBOX, wnd, msg, p1, p2);
};

/* LCD
 * ---
 * Control that draws text with big letters.
 */
const int kBoxFSize = 3;
static char boxf[][kBoxFSize+1] = {
"   ",
"   ",
"   ",
"\xb3",
"\xb3",
"\x07",
"\xb3\xb3",
"  ",
"  ",
"\xda\xc2\xbf",
"\xc0\xc5\xbf",
"\xc0\xc1\xd9",
"\xda\xbf\xb3",
"\xda\xc4\xd9",
"\xb3\xc0\xd9",
"\xda\xbf  ",
"\xc3\xc5\xc4",
"\xc0\xc4\xd9 ",
"\xb3",
" ",
" ",
"\xda\xc4",
"\xb3 ",
"\xc0\xc4",
"\xc4\xbf",
" \xb3",
"\xc4\xd9",
"\xb3 \xb3",
"\xc4\xc5\xc4",
"\xb3 \xb3",
" \xb3 ",
"\xc4\xc5\xc4",
" \xb3 ",
"  ",
"  ",
" \xd9",
"   ",
"\xc4\xc4\xc4",
"   ",
" ",
" ",
"\xb3",
" \xb3",
"\xda\xd9",
"\xb3 ",
"\xda\xc4\xbf",
"\xb3\xb3\xb3",
"\xc0\xc4\xd9",
"\xc4\xbf ",
" \xb3 ",
"\xc4\xc1\xc4",
"\xda\xc4\xbf",
"\xda\xc4\xd9",
"\xc0\xc4\xc4",
"\xda\xc4\xbf",
"\xc4\xc4\xb4",
"\xc0\xc4\xd9",
"\xb3 \xb3",
"\xc0\xc4\xb",
"  \xb3",
"\xda\xc4\xc4",
"\xc0\xc4\xbf",
"\xc0\xc4\xd9",
"\xda\xc4\xbf",
"\xc3\xc4\xbf",
"\xc0\xc4\xd9",
"\xda\xc4\xbf",
"  \xb3",
"  \xb3",
"\xda\xc4\xbf",
"\xc3\xc4\xb4",
"\xc0\xc4\xd9",
"\xda\xc4\xbf",
"\xc0\xc4\xb4",
"\xc0\xc4\xd9",
" ",
"\xb3",
"\xb3",
"  ",
" \xb3",
" \xd9",
" ",
" ",
"<",
"   ",
"\xc4\xc4\xc4",
"\xc4\xc4\xc4",
" ",
" ",
">",
"\xda\xc4\xbf",
" \xc4\xd9",
" \xb3 ",
"\xda\xc4\xbf",
"\xb3\xc3\xd9",
"\xc0\xc4\xc4",
"\xda\xc4\xbf",
"\xc3\xc4\xb4",
"\xb3 \xb3",
"\xda\xbf ",
"\xc3\xc1\xbf",
"\xc0\xc4\xd9",
"\xda\xc4\xc4",
"\xb3  ",
"\xc0\xc4\xc4",
"\xc4\xc2\xbf",
" \xb3\xb3",
"\xc4\xc1\xd9",
"\xda\xc4\xc4",
"\xc3\xc4 ",
"\xc0\xc4\xc4",
"\xda\xc4\xc4",
"\xc3\xc4 ",
"\xb3  ",
"\xda\xc4\xc4",
"\xb3\xc4\xbf",
"\xc0\xc4\xd9",
"\xb3 \xb3",
"\xc3\xc4\xb4",
"\xb3 \xb3",
"\xb3",
"\xb3",
"\xb3",
" \xda\xbf",
"  \xb3",
"\xc0\xc4\xd9",
"\xb3\xda ",
"\xc3\xc1\xbf",
"\xb3 \xb3",
"\xb3  ",
"\xb3  ",
"\xc0\xc4\xc4",
"\xda\xc2\xbf",
"\xb3\xb3\xb3",
"\xb3 \xb3",
"\xda\xbf\xb3",
"\xb3\xc0\xb4",
"\xb3 \xb3",
"\xda\xc4\xbf",
"\xb3 \xb3",
"\xc0\xc4\xd9",
"\xda\xc4\xbf",
"\xc3\xc4\xd9",
"\xb3  ",
"\xda\xc4\xbf",
"\xb3\xbf\xb3",
"\xc0\xc1\xd9",
"\xda\xc4\xbf",
"\xc3\xc2\xd9",
"\xb3\xc0\xc4",
"\xda\xc4\xbf",
"\xc0\xc4\xbf",
"\xc0\xc4\xd9",
"\xc4\xc2\xc4",
" \xb3 ",
" \xb3 ",
"\xb3 \xb3",
"\xb3 \xb3",
"\xc0\xc4\xd9",
"\xb3 \xb3",
"\xb3\xda\xd9",
"\xc0\xd9 ",
"\xb3 \xb3",
"\xb3\xb3\xb3",
"\xc0\xc1\xd9",
"\xb3 \xb3",
"\xda\xc5\xd9",
"\xb3 \xb3",
"\xb3 \xb3",
"\xc0\xc2\xd9",
" \xb3 ",
"\xc4\xc4\xbf",
"\xda\xc4\xd9",
"\xc0\xc4\xc4",
"\xda\xc4 ",
"\xb3  ",
"\xc0\xc4 ",
"\xb3 ",
"\xc0\xbf",
" \xb3",
" \xc4\xbf",
"  \xb3",
" \xc4\xd9",
" ",
" ",
"^",
"   ",
"   ",
"\xc4\xc4\xc4",
" \xbf",
"  ",
"  ",
"\xda\xc4\xbf",
"\xc3\xc4\xb4",
"\xb3 \xb3",
"\xda\xbf ",
"\xc3\xc1\xbf",
"\xc0\xc4\xd9",
"\xda\xc4\xc4",
"\xb3  ",
"\xc0\xc4\xc4",
"\xc4\xc2\xbf",
" \xb3\xb3",
"\xc4\xc1\xd9",
"\xda\xc4\xc4",
"\xc3\xc4 ",
"\xc0\xc4\xc4",
"\xda\xc4\xc4",
"\xc3\xc4 ",
"\xb3  ",
"\xda\xc4\xc4",
"\xb3\xc4\xbf",
"\xc0\xc4\xd9",
"\xb3 \xb3",
"\xc3\xc4\xb4",
"\xb3 \xb3",
"\xb3",
"\xb3",
"\xb3",
" \xda\xbf",
"  \xb3",
"\xc0\xc4\xd9",
"\xb3\xda ",
"\xc3\xc1\xbf",
"\xb3 \xb3",
"\xb3  ",
"\xb3  ",
"\xc0\xc4\xc4",
"\xda\xc2\xbf",
"\xb3\xb3\xb3",
"\xb3 \xb3",
"\xda\xbf\xb3",
"\xb3\xc0\xb4",
"\xb3 \xb3",
"\xda\xc4\xbf",
"\xb3 \xb3",
"\xc0\xc4\xd9",
"\xda\xc4\xbf",
"\xc3\xc4\xd9",
"\xb3  ",
"\xda\xc4\xbf",
"\xb3\xbf\xb3",
"\xc0\xc1\xd9",
"\xda\xc4\xbf",
"\xc3\xc2\xd9",
"\xb3\xc0\xc4",
"\xda\xc4\xbf",
"\xc0\xc4\xbf",
"\xc0\xc4\xd9",
"\xc4\xc2\xc4",
" \xb3 ",
" \xb3 ",
"\xb3 \xb3",
"\xb3 \xb3",
"\xc0\xc4\xd9",
"\xb3 \xb3",
"\xb3\xda\xd9",
"\xc0\xd9 ",
"\xb3 \xb3",
"\xb3\xb3\xb3",
"\xc0\xc1\xd9",
"\xb3 \xb3",
"\xda\xc5\xd9",
"\xb3 \xb3",
"\xb3 \xb3",
"\xc0\xc2\xd9",
" \xb3 ",
"\xc4\xc4\xbf",
"\xda\xc4\xd9",
"\xc0\xc4\xc4",
" \xda\xc4",
"\xc4\xb4 ",
" \xc0\xc4",
"\xb3",
"\xb3",
"\xb3",
"\xc4\xbf ",
" \xc3\xc4",
"\xc4\xd9 ",
"   ",
"\xda\xc4\xd9",
"   ",
{0} };
static const int boxf_offsets[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,6,0,9,12,15,18,21,24,27,30,33,36,39,42,45,48,51,54,57,60,63,66,69,72,75,78,0,84,0,90,81,96,99,102,105,108,111,114,117,120,123,126,129,132,135,138,141,144,147,150,153,156,159,162,165,168,171,174,177,180,0,186,189,192,195,198,201,204,207,210,213,216,219,222,225,228,231,234,237,240,243,246,249,252,255,258,261,264,267,270,273,276,279,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int LcdBoxProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    /* CTLWINDOW *ct = GetControl(wnd); */
    unsigned char *s = NULL;

    switch (msg) {
        case PAINT:
            if (isVisible(wnd))
            {
                NormalProc(wnd, msg, p1, p2); // clear bg etc
                s = (unsigned char*)GetText(wnd);
                SetStandardColor(wnd);
                if (s != NULL)
                {
                    int x = 0, y = 0;
                    while(*s) {
                        if (*s == ' ') {
                            PutWindowLine(wnd, "  ", x, y);
                            PutWindowLine(wnd, "  ", x, y+1);
                            PutWindowLine(wnd, "  ", x, y+2);
                            x+=2; ++s;
                        } else {
                            int off = boxf_offsets[*s];
                            PutWindowLine(wnd, boxf[off++], x, y);
                            PutWindowLine(wnd, boxf[off++], x, y+1);
                            PutWindowLine(wnd, boxf[off++], x, y+2);
                            x+=kBoxFSize; ++s;
                        }
                    }
                }
                return TRUE;
            }
            break;
    }
    return BaseWndProc(LCDBOX, wnd, msg, p1, p2);
};
