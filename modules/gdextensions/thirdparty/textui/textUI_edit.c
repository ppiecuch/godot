// kate: replace-tabs on; tab-indents on; tab-width 4; indent-width 4; indent-mode cstyle;

/*  FreeDOS Editor */


/* D E F I N E S ///////////////////////////////////////////////////////// */

#define CHARSLINE 80
#define LINESPAGE 66
#define ENABLEGLOBALARGV

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern DBOX PrintSetup;
static char Untitled[] = "Untitled";
static int wndpos, /* LineStartsAt, StartLine, */ LineCtr, CharCtr;
BOOL ConfigLoaded = FALSE;
WINDOW EditApplication = NULL;


/* ---------------- config.h -------------- */

/* ----------- configuration parameters ----------- */
typedef struct config {
    char version[8];
    char mono;         /* 0=color, 1=mono, 2=reverse mono    */
    BOOL InsertMode;   /* Editor insert mode                 */
    int Tabs;          /* Editor tab stops                   */
    BOOL WordWrap;     /* True to word wrap editor           */
    //#ifdef INCLUDE_WINDOWOPTIONS
    //  BOOL Border;       /* True for application window border */
    //  BOOL Title;        /* True for application window title  */
    //  BOOL StatusBar;    /* True for appl'n window status bar  */
    //#endif
    BOOL Texture;      /* True for textured appl window      */
    int ScreenLines;   /* Number of screen lines (25/43/50)  */
    char PrinterPort[5];
    int LinesPage;     /* Lines per printer page             */
    int CharsLine;     /* Characters per printer line        */
    int LeftMargin;    /* Printer margins                    */
    int RightMargin;
    int TopMargin;
    int BottomMargin;
    BOOL ReadOnlyMode; /* added in EDIT 0.7b */
    ColorScheme clr;
    // unsigned char clr[CLASSCOUNT] [4] [2]; /* Colors         */
} CONFIG;

CONFIG cfg;


/* ------------- config.c ------------- */

/* ------ default configuration values ------- */
CONFIG cfg = {
    "0.9.1.0",
    0,          /* Color                        */
    TRUE,       /* Editor Insert Mode           */
    8,          /* Editor tab stop size         */ /* was 4 before 0.7b */
    FALSE,      /* Editor word wrap             */
    FALSE,      /* Textured application window  */
    25,         /* Number of screen lines       */
    "Lpt1",     /* Printer Port                 */
    66,         /* Lines per printer page       */
    80,         /* characters per printer line  */
    6,          /* Left printer margin          */
    70,         /* Right printer margin         */
    3,          /* Top printer margin           */
    55,         /* Bottom printer margin        */
    FALSE       /* Read only mode               */
};

extern BOOL ConfigLoaded;

void BuildFileName(char *path, const char *fn, const char *ext)
{
    char *cp = path;

/* if Argv[0] is available then open file in same dir as Application binary */
#ifdef ENABLEGLOBALARGV
    strcpy(path, Argv[0]);
    cp = strrchr(path, '\\');
    if (cp == NULL)
        cp = path;
    else
        cp++;
#endif
    strcpy(cp, fn);
    strcat(cp, ext);
}

FILE *OpenConfig(char *mode)
{
    char path[64];
    BuildFileName(path, DFlatApplication, ".cfg");
    return fopen(path, mode);
}

/* ------ load a configuration file from disk ------- */
BOOL LoadConfig(void)
{
    strcpy (cfg.version, ProgramVersion);

    if (ConfigLoaded == FALSE)	{
        FILE *fp = OpenConfig("rb");
        if (fp != NULL)    {
            fread(cfg.version, sizeof cfg.version, 1, fp);
            if (strcmp(cfg.version, ProgramVersion) == 0) {
                fseek(fp, 0L, SEEK_SET);
                fread(&cfg, sizeof(CONFIG), 1, fp);
                fclose(fp);
            } else {
                char path[64];
                BuildFileName(path, DFlatApplication, ".cfg");
                fclose(fp);
                unlink(path);
                strcpy(cfg.version, ProgramVersion);
            }
            ConfigLoaded = TRUE;
        }
    }
    return ConfigLoaded;
}

/* ------ save a configuration file to disk ------- */
void SaveConfig(void)
{
    FILE *fp = OpenConfig("wb");

    memcpy(&cfg.clr, &SysConfig.VideoCurrentColorScheme, sizeof (ColorScheme));
    cfg.snowy = GetSnowyFlag();

    cfg.ScreenLines = SysConfig.VideoCurrentResolution.VRes;

    cfg.ReadOnlyMode = FALSE;	/* always save as FALSE for now (0.7b).  */
    /* There is no toggle / menu item: Nobody should override /R option! */
    /* however, you can manually modify the flag in the edit.cfg file... */
    if (fp != NULL)    {
        fwrite(&cfg, sizeof(CONFIG), 1, fp);
        fclose(fp);
    }
}


/* ---------------- edit.h ----------------- */

/* To disable calendar item in utils define NOCALENDAR */
/* #define NOCALENDAR */

/* set this to 1 to enable the ASCII Table utility. New 0.7c */
#define WITH_ASCIITAB 1

void PrepFileMenu(void *, struct Menu *);
void PrepEditMenu(void *, struct Menu *);
void PrepSearchMenu(void *, struct Menu *);
void PrepWindowMenu(void *, struct Menu *);

/* -------------- calendar ------------------------ */
#ifndef NOCALENDAR
void Calendar(WINDOW pwnd);
#endif
#if WITH_ASCIITAB
void Asciitable(WINDOW pwnd);	/* new 0.7c */
#endif


BEGIN_USER_COMMANDS
    /* --------------- File menu ---------------- */
    ID_OPEN,
    ID_NEW,
    ID_SAVE,
    ID_SAVEAS,
    ID_CLOSE,
    ID_DELETEFILE,
    ID_PRINT,
    ID_PRINTSETUP,
    ID_DOS,
    ID_EXIT,
    /* --------------- Edit menu ---------------- */
    // ID_UNDO,
    // ID_CUT,
    // ID_COPY,
    // ID_PASTE,
    // ID_PARAGRAPH,
    // ID_CLEAR,
    // ID_DELETETEXT,
    /* 0.7d additions for edit menu: */
    // ID_UPCASE,
    // ID_DOWNCASE,
    // ID_WORDCOUNT,
    /* --------------- Search Menu -------------- */
    // ID_SEARCH,
    // ID_REPLACE,
    // ID_SEARCHNEXT,
    /* --------------- Utilities Menu ------------- */
#ifndef NOCALENDAR
    ID_CALENDAR,
#endif
#if WITH_ASCIITAB
    ID_ASCIITAB,	/* new 0.7c */
#endif
    /* -------------- Options menu -------------- */
    ID_INSERT,
    ID_WRAP,
    ID_LOG,
    ID_TABS,
    ID_DISPLAY,
    ID_SAVEOPTIONS,
    /* --------------- Window menu -------------- */
    ID_CLOSEALL,
    ID_WINDOW,
    ID_MOREWINDOWS,
    /* --------------- Help menu ---------------- */
    ID_HELPHELP,
    ID_EXTHELP,
    ID_KEYSHELP,
    ID_HELPINDEX,
    ID_ABOUT,
    ID_ABOUTDFP,
    /* -------------- TabStops menu ------------- */
    ID_TAB0, /* tab-as-char mode -ea */
    ID_TAB2,
    ID_TAB4,
    ID_TAB6,
    ID_TAB8
END_USER_COMMANDS



/* P R O T O T Y P E S /////////////////////////////////////////////////// */

int classify_args(int, char *[], char *[], char *[]);
static int MemoPadProc(WINDOW, MESSAGE, PARAM, PARAM);
static void NewFile(WINDOW,char *);
static void SelectFile(WINDOW);
static void PadWindow(WINDOW, char *);
static void OpenPadWindow(WINDOW, char *,char *);
static void LoadFile(WINDOW);
static void PrintPad(WINDOW);
static void SaveFile(WINDOW, int);
#ifdef DELFILE
static void DeleteFile(WINDOW);
#endif
static int EditorProc(WINDOW, MESSAGE, PARAM, PARAM);
static char *NameComponent(char *);
static int PrintSetupProc(WINDOW, MESSAGE, PARAM, PARAM);
static void FixTabMenu(void);


/* F U N C T I O N S ///////////////////////////////////////////////////// */


/* ------ open text files and put them into editboxes ----- */
static void PadWindow(WINDOW wnd, char *FileName)
{
    int ax, criterr = 1;
    FBLOCK ff;

    char path[66];
    char *cp;

    CreatePath(path, FileName, FALSE, FALSE);
    cp = path+strlen(path);
    CreatePath(path, FileName, TRUE, FALSE);
    while (criterr == 1)
        {
        ax = FindFirst (path, _A_NORMAL, ff);
        criterr = TestCriticalError();
        }

    while (ax == 0 && !criterr)
        {
        strcpy(cp, NameOf(ff));
        OpenPadWindow(wnd, path,NULL);
        ax = FindNext(ff);
        }
}

/* ------- window processing module for the Edit application window ----- */
static int MemoPadProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    int rtn;

    switch (msg)
        {
        case CREATE_WINDOW:
      
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            if (cfg.InsertMode)
                SetCommandToggle(&MainMenu, ID_INSERT);

            if (cfg.WordWrap)
                SetCommandToggle(&MainMenu, ID_WRAP);

            FixTabMenu();
            return rtn;
        case CLOSE_WINDOW:
            UnLoadHelpFile();
            break;
        case COMMAND:
            switch ((int)p1)
                {
                case ID_WINDOW:
                    /* C.M.S. set by menubar (global hotkey, new 0.7c) */
                    /* or by popdown (click, local hotkey)... */
                    ChooseWindow(wnd, CurrentMenuSelection-2);
                    break;
                case ID_CLOSEALL:
                    CloseAll(wnd, FALSE);
                    break;
                case ID_MOREWINDOWS:
                    MoreWindows(wnd);
                    break;
                case ID_HELP:
                    DisplayHelp(wnd, DFlatApplication);
                    break;
                case ID_HELPHELP:
                    DisplayHelp(wnd, "HelpHelp");
                    break;
                case ID_EXTHELP:
                    DisplayHelp(wnd, "ExtHelp");
                    break;
                case ID_KEYSHELP:
                    DisplayHelp(wnd, "KeysHelp");
                    break;
                case ID_HELPINDEX:
                    DisplayHelp(wnd, "HelpIndex");
                    break;
                case ID_DISPLAY:
                    DisplayProperties (wnd);
                    return TRUE;
#ifdef INCLUDE_LOGGING
                case ID_LOG:
                    MessageLog(wnd);
                    if (CheckBoxSetting(&dbLog, ID_LOGGING))
                        SetCommandToggle(&MainMenu, ID_LOG);
                    else
                        ClearCommandToggle(&MainMenu, ID_LOG);
                    break;
#endif
                case ID_NEW:
                    NewFile(wnd,NULL);
                    return TRUE;
                case ID_OPEN:
                    SelectFile(wnd);
                    return TRUE;
                case ID_SAVE:
                    SaveFile(inFocus, FALSE);
                    return TRUE;
                case ID_SAVEAS:
                    SaveFile(inFocus, TRUE);
                    return TRUE;
                case ID_CLOSE:
                    SendMessage(inFocus, CLOSE_WINDOW, 0, 0);
                    SkipApplicationControls();
                    return TRUE;
#ifdef DELFILE
                case ID_DELETEFILE:
                    DeleteFile(inFocus);
                    return TRUE;
#endif
                case ID_PRINTSETUP:
                    DialogBox(wnd, &PrintSetup, TRUE, PrintSetupProc);
                    return TRUE;
                case ID_PRINT:
                    PrintPad(inFocus);
                    return TRUE;
                case ID_DOS:
                    ExecuteNonDFP ( getenv("COMSPEC"));
                    break;
                case ID_EXIT: 
                    PostMessage(wnd, CLOSE_WINDOW, 0, 0);
                    return TRUE;
                case ID_WRAP:
                    cfg.WordWrap = GetCommandToggle(&MainMenu, ID_WRAP);
                    return TRUE;
                case ID_INSERT:
                    cfg.InsertMode = GetCommandToggle(&MainMenu, ID_INSERT);
                    return TRUE;
                case ID_TAB0:
                    SysConfig.EditorTabSize = cfg.Tabs = 1; /* type-through TAB char mode -ea */
                    FixTabMenu(); /* show current value in tab menu */
                    return TRUE;
                case ID_TAB2:
                    SysConfig.EditorTabSize = cfg.Tabs = 2;
                    FixTabMenu(); /* show current value in tab menu */
                    return TRUE;
                case ID_TAB4:
                    SysConfig.EditorTabSize = cfg.Tabs = 4;
                    FixTabMenu();
                    return TRUE;
                case ID_TAB6:
                    SysConfig.EditorTabSize = cfg.Tabs = 6; 
                    FixTabMenu();
                    return TRUE;
                case ID_TAB8:
                    SysConfig.EditorTabSize = cfg.Tabs = 8;
                    FixTabMenu();
                    return TRUE;
                case ID_SAVEOPTIONS:
                    SaveConfig();
                    return TRUE;
#ifndef NOCALENDAR
                case ID_CALENDAR:
                    Calendar(wnd); 
                    return TRUE;
#endif
#if WITH_ASCIITAB /* new 0.7c */
                case ID_ASCIITAB:
                    Asciitable(wnd);
                    return TRUE;
#endif
                case ID_ABOUT:
                    ProgramAboutBox ();
                    return TRUE;
                case ID_ABOUTDFP:
                    DFlatpAboutBox ();
                    return TRUE;
                default:
                    break;
                }
            break;
        default:
            break;
        }

    return DefaultWndProc(wnd, msg, p1, p2);
}

/* The New command. Open an empty editor window */
static void NewFile(WINDOW wnd, char *FileName)
{
    OpenPadWindow(wnd, Untitled,FileName);
}

/* --- The Open... command. Select a file  --- */
static void SelectFile(WINDOW wnd)
{
    char FileName[64];

    if (OpenFileDialogBox("*.*", FileName))
        {
        /* See if the document is already in a window */
        WINDOW wnd1 = FirstWindow(wnd);

        while (wnd1 != NULL)
            {
            if (stricmp(FileName, wnd1->extension) == 0)
                {
                SendMessage(wnd1, SETFOCUS, TRUE, 0);
                SendMessage(wnd1, RESTORE, 0, 0);
                return;
                }

            wnd1 = NextWindow(wnd1);
            }
        OpenPadWindow(wnd, FileName,NULL);
        }
}

/* --- open a document window and load a file --- */
static void OpenPadWindow(WINDOW wnd, char *FileName,char *NewFileName)
{
    static WINDOW wnd1 = NULL;
    struct stat sb;
    char *Fname = FileName, *ermsg;

    if (strcmp(FileName, Untitled))
        {
        if (stat(FileName, &sb))
            {
                ermsg = DFmalloc(strlen(FileName)+20);
                strcpy(ermsg, "No such file:\n");
                strcat(ermsg, FileName);
                ErrorMessage(ermsg);
                free(ermsg);
                return;
            }

        Fname = NameComponent(FileName);

        /* check file size */
        if (sb.st_size > 64000UL)
            {
            ermsg = DFmalloc(strlen(FileName)+100); /* alloc fixed 0.7a */
            strcpy(ermsg, "File too large for this version of Edit:\n");
            strcat(ermsg, FileName);
            ErrorMessage(ermsg);
            free(ermsg);
            return;
            }
        } /* actual filename given */

    wndpos += 2;
    if (NewFileName != NULL)
        Fname = NameComponent(NewFileName);

    if (wndpos == 20)
        wndpos = 2;

    wnd1 = CreateWindow(EDITBOX,
                        Fname,
                        (wndpos-1)*2, wndpos, 10, 40,
                        NULL, wnd, EditorProc,
                        SHADOW     |
                        MINMAXBOX  |
                        CONTROLBOX |
                        VSCROLLBAR |
                        HSCROLLBAR |
                        MOVEABLE   |
                        HASBORDER  |
                        SIZEABLE   |
                        MULTILINE);

    if (cfg.ReadOnlyMode) /* new feature in 0.7b */
        AddAttribute(wnd1, READONLY);
        /* needed because ReadOnlyMode must not make ALL text */
        /* entry fields read only, only EDITBOXes become r/o! */

        /* suggested code change to ix saveas bug - contrib: James Sandys-Lumsdaine

        OLD CODE SEGMENT!

        if (strcmp(FileName, Untitled))    {
            wnd1->extension = DFmalloc(strlen(FileName)+1);
            strcpy(wnd1->extension, FileName);
            LoadFile(wnd1);
        }

        NEW CODE SEGMENT!
    */

    if (NewFileName != NULL)
        {
        /* Either a command line new file or one that's on the
        disk to load - Either way, must set the extension
        to the given filename */

        wnd1->extension = DFmalloc(strlen(NewFileName) + 1);
        strcpy(wnd1->extension,NewFileName);
        }
    else
        {
        if (strcmp(FileName,Untitled))
            wnd1->extension = DFmalloc(strlen(FileName)+1);

        strcpy(wnd1->extension, FileName);
        LoadFile(wnd1); /* Only load if not a new file */
        }

    SendMessage(wnd1, SETFOCUS, TRUE, 0);
    SendMessage(wnd1, MAXIMIZE, 0, 0); 

}

/* --- Load the notepad file into the editor text buffer --- */
static void LoadFile(WINDOW wnd)
{
    char *Buf = NULL;
    unsigned int recptr = 0;
    FILE *fp;
    WINDOW wwnd;

    if (!strcmp(wnd->extension, Untitled))
    {
        SendMessage(wnd, SETTEXT, (PARAM) "", 0); /* fill with empty string */
        /* could show a messagebox of some kind here */
        return;
    } /* not a real file load */

    if ((fp = fopen(wnd->extension, "rt")) != NULL) /* why "t"ext mode? */
    {
        /* (could use ExpandTabs() here alternatively!?) */
        int expandTabs = -1;
        int theColumn = 0;
        unsigned int rmax;

        wwnd = WatchIcon();

        while (!feof(fp))
        {
            Cooperate();			/* let messages flow */
            rmax = 1024;
            Buf = DFrealloc(Buf, recptr+rmax);	/* inflate buffer */
            memset(Buf+recptr, 0, rmax);	/* clear new area */
            fgets(Buf+recptr, 512, fp);		/* read more data */
            if ( (expandTabs == -1) && (cfg.Tabs > 1) && (strchr(Buf+recptr,'\t') != NULL) )
            {
                char tMsg[200];

                SendMessage(wwnd, CLOSE_WINDOW, 0, 0);

                sprintf(tMsg,"Tabs detected in\n%s\nExpand them at tab width %d?",
                    (strlen(wnd->extension)>120) ? "file" : wnd->extension,
                    cfg.Tabs);
                expandTabs = (YesNoBox(tMsg)) ? 1 : 0;

                wwnd = WatchIcon();

            }
            for (int i=0; Buf[recptr+i]; i++)
            {
                switch (Buf[recptr+i])
                {
                    case '\r':
                    case '\n':
                        theColumn = 0;
                        break;
                    /* backspace intentionally not handled */
                    case '\t':
                        if (expandTabs == 1)
                        {
                            if ((strlen(Buf+recptr)+cfg.Tabs+8 /* 5 */) >= rmax)
                            /* changed extra offset from 5 to 8 for 0.7b */
                            {
                                rmax += 512;
                                Buf = DFrealloc(Buf, recptr+rmax);
                                memset(Buf+recptr+rmax-512, 0, 512);
                            };
                            {   /* limit scope of j */
                                int j = cfg.Tabs - (theColumn % cfg.Tabs);
                                /* move by dist. to next tab, pad with ' ' */
                                /* BROKEN 0.7a version:
                                    strcpy(Buf+recptr+i+j, Buf+recptr+i+1);
                                */ /* Fixed 0.7b version: */
                                memmove(Buf+recptr+i+j, Buf+recptr+i+1, rmax-i-j);
                                /* end of 0.7b fix */
                                /* ... +1 as we do not copy the \t itself */
                                memset(Buf+recptr+i, ' ', j);
                                theColumn += j;
                                i += j; /* do not read padding again */
                                i--;	/* because of i++ in the loop */
                            };
                        } else
                            theColumn++; /* do not expand */
                        break;
                    default:
                        theColumn++;
                } /* switch */
            } /* for */
            recptr += strlen(Buf+recptr);	/* add read-len */
        } /* while not eof */

        fclose(fp);
        if (Buf != NULL)
        {
            SendMessage(wnd, SETTEXT, (PARAM) Buf, 0); /* paste read text */
            free(Buf); /* buffer no longer needed */
        }

        SendMessage(wwnd, CLOSE_WINDOW, 0, 0);

        /* else ran out of memory? */
    } else {
        char fMsg[200];
        sprintf(fMsg,"Could not load %s", (strlen(wnd->extension)>120) ? "file" : wnd->extension);
        ErrorMessage(fMsg);
    }
}

/* ------- print a character -------- */
static void PrintChar(FILE *prn, int c)
{
    if (c == '\n' || CharCtr == cfg.RightMargin)
        {
        fputs("\r\n", prn);
        LineCtr++;
        if (LineCtr == cfg.BottomMargin)
            {
            fputc('\f', prn);
            for (int i = 0; i < cfg.TopMargin; i++)
                fputc('\n', prn);

            LineCtr = cfg.TopMargin;
            }

        CharCtr = 0;
        if (c == '\n')
            return;

        }

    if (CharCtr == 0)
        {
        for (int i = 0; i < cfg.LeftMargin; i++)
            {
            fputc(' ', prn);
            CharCtr++;
            }

        }

    CharCtr++;
    fputc(c, prn);
}

/* --- print the current notepad --- */
static void PrintPad(WINDOW wnd)
{
    if (*cfg.PrinterPort)   {
        FILE *prn;
        if ((prn = fopen(cfg.PrinterPort, "wt")) != NULL)       {
            long percent;
            BOOL KeepPrinting = TRUE;
            unsigned char *text = GetText(wnd);
            unsigned oldpct = 100, cct = 0, len = strlen(text);
            /* the ONLY place where slidebox is used right now: */
            WINDOW swnd = SliderBox(20, GetTitle(wnd), "Printing");
            /* ------- print the notepad text --------- */
            LineCtr = CharCtr = 0;
            while (KeepPrinting && *text)   {
                PrintChar(prn, *text++);
                percent = ((long) ++cct * 100) / len;
                if ((int) percent != oldpct)    {
                    oldpct = (int) percent;
                    KeepPrinting = SendMessage(swnd, PAINT, 0, oldpct);
                }
        }
            if (KeepPrinting)
                /* ---- user did not cancel ---- */
                if (oldpct < 100)
                    SendMessage(swnd, PAINT, 0, 100);
            /* ------- follow with a form feed? --------- */
            if (YesNoBox("Form Feed?"))
            fputc('\f', prn);
            fclose(prn);
        }
        else
            ErrorMessage("Cannot open printer file");
    }
    else
        ErrorMessage("No printer selected");
}

/* ---------- save a file to disk ------------ */
static void SaveFile(WINDOW wnd, int Saveas)
{
    FILE *fp;

    if (wnd->extension == NULL || Saveas) /* ask for new name? */
        {
        char FileName[64];

        FileName[0] = 0;
        trySaveAgain:	/* moved label up in 0.7c */

        if (SaveAsDialogBox("*.*", NULL, FileName))
            {
            if (wnd->extension != NULL)
                free(wnd->extension);

            wnd->extension = DFmalloc(strlen(FileName)+1);
            strcpy(wnd->extension, FileName);
            AddTitle(wnd, NameComponent(FileName));
            SendMessage(wnd, BORDER, 0, 0);
            }
        else
            {
            ErrorMessage("No name given - not saved.");
            return; /* abort if no name provided by user */
            }
        }

    if (wnd->extension != NULL)	/* if there is a filename for the window */
        {
        WINDOW mwnd;
        /* trySaveAgain: */
        mwnd = MomentaryMessage("Saving...");

        if ((fp = fopen(wnd->extension, "wt")) != NULL)
            {
            /* could use CollapseTabs() here if user wants us to do so!? */
            size_t howmuch = strlen(GetText(wnd));
            howmuch = fwrite(GetText(wnd), howmuch, 1, fp); /* ONE item, SIZE howmuch */
            fclose(fp);
            SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
            if (howmuch != 1) /* ONE item actually written? */
                {
                if (YesNoBox("Ran out of disk space while saving. Try again?"))
                    {
                    Saveas = 1;	/* 0.7c: ask user for a new place for next try */
                    goto trySaveAgain;
                    }
                }
            else
                wnd->TextChanged = FALSE;	/* give up */
            }
        else
            {
            char fMsg[200];
            SendMessage(mwnd, CLOSE_WINDOW, 0, 0);
            sprintf(fMsg,"Could not save %s, try again?",
                    (strlen(wnd->extension)>120) ? "file" : wnd->extension);
            if (YesNoBox(fMsg))
                {
                Saveas = 1;	/* 0.7c: ask user for a new place for next try */
                goto trySaveAgain;
                }
            }
        } /* if any file loaded */
}

/* -------- delete a file ------------ */
#ifdef DELFILE
static void DeleteFile(WINDOW wnd)
{
    if (wnd->extension != NULL)    {
        if (strcmp(wnd->extension, Untitled))    {
            char *fn = NameComponent(wnd->extension);
            if (fn != NULL)    {
            char msg[150];
            sprintf(msg, "Delete %s?", (strlen(fn)>100) ? "file" : fn);
            if (YesNoBox(msg)) {
                unlink(wnd->extension);
                SendMessage(wnd, CLOSE_WINDOW, 0, 0);
            }
            }
        }
    }
}
#endif

/* ------ display the row and column in the statusbar ------ */
static void ShowPosition(WINDOW wnd)
{
    /* This is where we place the "INS" display */
    char status[40], *InsModeText;
    if (wnd->InsertMode)
        {
        InsModeText = "INS ";           /* Not on */
        }
    else
        {
        InsModeText = "OVER";           /* "Insert" (Overtype!?) is on */
        }

    if (WindowWidth(wnd) < 50) /* auto-condense new in EDIT 0.7 */
        {
        sprintf(status, "%c %c Li:%d Co:%d", 
            (cfg.ReadOnlyMode) ? 'R' : (wnd->TextChanged ? '*' : ' '),
            InsModeText[0], (wnd->CurrLine)+1, (wnd->CurrCol)+1);
            /* 1-based column / row are nicer for humans (EDIT 0.7b) */
            /* new flag char R for readonly added 0.7b */
        }
    else
        sprintf(status, "%c %4s  Line: %4d  Col: %3d ",
            (cfg.ReadOnlyMode) ? 'R' : (wnd->TextChanged ? '*' : ' '),
            InsModeText, (wnd->CurrLine)+1, (wnd->CurrCol)+1);
            /* 1-based column / row are nicer for humans (EDIT 0.7b) */
            /* new flag char R for readonly added 0.7b */
    SendMessage(GetParent(wnd), ADDSTATUS, (PARAM) status, 0);

}

/* ----- window processing module for the editboxes ----- */
static int EditorProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    int rtn;
    switch (msg)    {
    case SETFOCUS:
        if ((int)p1)    {
            wnd->InsertMode = GetCommandToggle(&MainMenu, ID_INSERT);
            wnd->WordWrapMode = GetCommandToggle(&MainMenu, ID_WRAP);
        }
        rtn = DefaultWndProc(wnd, msg, p1, p2);
        if ((int)p1 == FALSE)
        SendMessage(GetParent(wnd), ADDSTATUS, 0, 0);
        else
        ShowPosition(wnd);
        return rtn;
    case KEYBOARD_CURSOR:
        rtn = DefaultWndProc(wnd, msg, p1, p2);
        ShowPosition(wnd);
        return rtn;
    case COMMAND:
        if (cfg.ReadOnlyMode && TestAttribute(wnd, READONLY)) {
            /* read only mode added 0.7b */
            switch ((int)p1) {
                case ID_REPLACE:
                case ID_CUT:
                case ID_PASTE:
                case ID_DELETETEXT:
                case ID_CLEAR:
                case ID_PARAGRAPH:
                    beep();
                    return TRUE;	/* consume event */
            }
        }
        switch ((int) p1)       {
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
            case ID_CLEAR:
                rtn = DefaultWndProc(wnd, msg, p1, p2);
                SendMessage(wnd, PAINT, 0, 0);
                return rtn;
            case ID_HELP:
                DisplayHelp(wnd, "MEMOPADDOC");
                return TRUE;
            case ID_WRAP:
                SendMessage(GetParent(wnd), COMMAND, ID_WRAP, 0);
                wnd->WordWrapMode = cfg.WordWrap;
                return TRUE;
            case ID_INSERT:
                SendMessage(GetParent(wnd), COMMAND, ID_INSERT, 0);
                wnd->InsertMode = cfg.InsertMode;
                SendMessage(NULL, SHOW_CURSOR, wnd->InsertMode, 0);
                return TRUE;
        case ID_DISPLAY:
            DisplayProperties (wnd);
        break;
            default:
                break;
            } /* end of switch int p1 */
        break; /* end of case COMMAND  */
    case CLOSE_WINDOW:
        if (wnd->TextChanged)
            {
            char *cp;
            BOOL saveAsFlag = 0;
            cp = DFmalloc(75+strlen(GetTitle(wnd)));
            strcpy(cp, "             The file\n            '");
            strcat(cp, GetTitle(wnd));
            strcat(cp, "'\nhas not been saved yet.  Save it now?");

            saveAsOnClose:
            SendMessage(wnd, SETFOCUS, TRUE, 0);
            if (YesNoBox(cp)) {            
                SendMessage(GetParent(wnd), COMMAND,
                    (saveAsFlag ? ID_SAVEAS : ID_SAVE), 0);
                if (wnd->TextChanged) { /* still unsaved changes? */
                  ErrorMessage("File could not be saved! Try to save elsewhere.");
                  saveAsFlag = 1;
                  goto saveAsOnClose; /* do not let user leave yet */
                } /* still unsaved */
            } /* user selected "yes", save before closing window */
            free(cp);
            } /* modified file - suggested to save */

        wndpos = 0;
        if (wnd->extension != NULL)
            {
            free(wnd->extension);
            wnd->extension = NULL;
            }
        break;
    default:
        break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

/* -- point to the name component of a file specification -- */
static char *NameComponent(char *FileName)
{
    char *Fname;
    if ((Fname = strrchr(FileName, '\\')) == NULL)
    if ((Fname = strrchr(FileName, ':')) == NULL)
        Fname = FileName-1;
    return Fname + 1;
}

static char *ports[] = {
    "Lpt1", "Lpt2", "Lpt3",
    "Com1", "Com2", "Com3", "Com4",
    NULL
};

static int PrintSetupProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    int rtn, i = 0, mar;
    char marg[10];
    WINDOW cwnd;
    switch (msg)    {
        case CREATE_WINDOW:
            rtn = DefaultWndProc(wnd, msg, p1, p2);
            PutItemText(wnd, ID_PRINTERPORT, cfg.PrinterPort);
            while (ports[i] != NULL)
                PutComboListText(wnd, ID_PRINTERPORT, ports[i++]);
            for (mar = CHARSLINE; mar >= 0; --mar)  {
                sprintf(marg, "%3d", mar);
                PutItemText(wnd, ID_LEFTMARGIN, marg);
                PutItemText(wnd, ID_RIGHTMARGIN, marg);
            }
            for (mar = LINESPAGE; mar >= 0; --mar)  {
                sprintf(marg, "%3d", mar);
                PutItemText(wnd, ID_TOPMARGIN, marg);
                PutItemText(wnd, ID_BOTTOMMARGIN, marg);
            }
            cwnd = ControlWindow(&PrintSetup, ID_LEFTMARGIN);
            SendMessage(cwnd, LB_SETSELECTION,
                CHARSLINE-cfg.LeftMargin, 0);
            cwnd = ControlWindow(&PrintSetup, ID_RIGHTMARGIN);
            SendMessage(cwnd, LB_SETSELECTION,
                CHARSLINE-cfg.RightMargin, 0);
            cwnd = ControlWindow(&PrintSetup, ID_TOPMARGIN);
            SendMessage(cwnd, LB_SETSELECTION,
                LINESPAGE-cfg.TopMargin, 0);
            cwnd = ControlWindow(&PrintSetup, ID_BOTTOMMARGIN);
            SendMessage(cwnd, LB_SETSELECTION,
                LINESPAGE-cfg.BottomMargin, 0);
            return rtn;
        case COMMAND:
            if ((int) p1 == ID_OK && (int) p2 == 0) {
                GetItemText(wnd, ID_PRINTERPORT, cfg.PrinterPort, 4);
                cwnd = ControlWindow(&PrintSetup, ID_LEFTMARGIN);
                cfg.LeftMargin = CHARSLINE - SendMessage(cwnd, LB_CURRENTSELECTION, 0, 0);
                cwnd = ControlWindow(&PrintSetup, ID_RIGHTMARGIN);
                cfg.RightMargin = CHARSLINE - SendMessage(cwnd, LB_CURRENTSELECTION, 0, 0);
                cwnd = ControlWindow(&PrintSetup, ID_TOPMARGIN);
                cfg.TopMargin = LINESPAGE - SendMessage(cwnd, LB_CURRENTSELECTION, 0, 0);
                cwnd = ControlWindow(&PrintSetup, ID_BOTTOMMARGIN);
                cfg.BottomMargin = LINESPAGE - SendMessage(cwnd, LB_CURRENTSELECTION, 0, 0);
            }
            break;
    default:
        break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

static void FixTabMenu(void)
{
    char *cp = GetCommandText(&MainMenu, ID_TABS);
    if (cp != NULL) {
        cp = strchr(cp, '(');
        if (cp != NULL) {
            *(cp+1) = (cfg.Tabs>1) ? (cfg.Tabs + '0') : '-';
            if (GetClass(inFocus) == POPDOWNMENU)
                SendMessage(inFocus, PAINT, 0, 0);
        }
    }
}

/* Prep....Menu are called to activate drop-downs in the main menu bar */
void PrepFileMenu(void *w, struct Menu *mnu)
{
    WINDOW wnd = w;

    if (mnu != NULL) {}; /* unused parameter! */

    DeactivateCommand(&MainMenu, ID_SAVE);
    DeactivateCommand(&MainMenu, ID_SAVEAS);
    DeactivateCommand(&MainMenu, ID_CLOSE);
/*  DeactivateCommand(&MainMenu, ID_DELETEFILE); */
    DeactivateCommand(&MainMenu, ID_PRINT);

    if (cfg.ReadOnlyMode) {		/* new in 0.7b */
        DeactivateCommand(&MainMenu, ID_NEW);
        DeactivateCommand(&MainMenu, ID_DOS); /* make viewer mode "safe" */
    }

    if (wnd != NULL && GetClass(wnd) == EDITBOX)
        {
        if (isMultiLine(wnd))
            {
            if (!cfg.ReadOnlyMode) {	/* new in 0.7b */
                ActivateCommand(&MainMenu, ID_SAVE);
                ActivateCommand(&MainMenu, ID_SAVEAS);
            }
            ActivateCommand(&MainMenu, ID_CLOSE);
/*          ActivateCommand(&MainMenu, ID_DELETEFILE);  */
            ActivateCommand(&MainMenu, ID_PRINT);
            }

	}

}

void PrepSearchMenu(void *w, struct Menu *mnu)
{
    WINDOW wnd = w;

    if (mnu != NULL) {}; /* unused parameter! */

    DeactivateCommand(&MainMenu, ID_SEARCH);
    DeactivateCommand(&MainMenu, ID_REPLACE);
    DeactivateCommand(&MainMenu, ID_SEARCHNEXT);

    if (wnd != NULL && GetClass(wnd) == EDITBOX) {
        if (isMultiLine(wnd))   {
            ActivateCommand(&MainMenu, ID_SEARCH);
                if (!cfg.ReadOnlyMode) { /* new in 0.7b */
                ActivateCommand(&MainMenu, ID_REPLACE);
            }
            ActivateCommand(&MainMenu, ID_SEARCHNEXT);
        }
    }
}

void PrepEditMenu(void *w, struct Menu *mnu)
{
    WINDOW wnd = w;

    if (mnu != NULL) {}; /* unused parameter! */

    DeactivateCommand(&MainMenu, ID_CUT);
    DeactivateCommand(&MainMenu, ID_COPY);
    DeactivateCommand(&MainMenu, ID_CLEAR);
    DeactivateCommand(&MainMenu, ID_DELETETEXT);
    DeactivateCommand(&MainMenu, ID_PARAGRAPH);
    DeactivateCommand(&MainMenu, ID_PASTE);
    DeactivateCommand(&MainMenu, ID_UNDO);
    DeactivateCommand(&MainMenu, ID_UPCASE);	/* new in 0.7d */
    DeactivateCommand(&MainMenu, ID_DOWNCASE);	/* new in 0.7d */
    ActivateCommand(&MainMenu, ID_WORDCOUNT);	/* new in 0.7d */
    if (wnd != NULL && GetClass(wnd) == EDITBOX) {
        if (isMultiLine(wnd) &&
            (!cfg.ReadOnlyMode)) {	/* new mode in 0.7b */

            if (TextBlockMarked(wnd))       {
                ActivateCommand(&MainMenu, ID_CUT);
                ActivateCommand(&MainMenu, ID_COPY);
                ActivateCommand(&MainMenu, ID_CLEAR);
                ActivateCommand(&MainMenu, ID_DELETETEXT);
                ActivateCommand(&MainMenu, ID_UPCASE); /* new in 0.7d */
                ActivateCommand(&MainMenu, ID_DOWNCASE); /* new in 0.7d */
            }
            ActivateCommand(&MainMenu, ID_PARAGRAPH);
            if (!TestAttribute(wnd, READONLY) &&
            Clipboard != NULL)
            ActivateCommand(&MainMenu, ID_PASTE);
            if (wnd->DeletedText != NULL)
            ActivateCommand(&MainMenu, ID_UNDO);
        } /* editable non-empty EDITBOX */
    }
}


static char *Menus[9] = {
    "~1.                      ",
    "~2.                      ",
    "~3.                      ",
    "~4.                      ",
    "~5.                      ",
    "~6.                      ",
    "~7.                      ",
    "~8.                      ",
    "~9.                      "
};

/* ----------- Prepare the Window menu (window list) ------------ */
void PrepWindowMenu(void *w, struct Menu *mnu)
{
    WINDOW wnd = w;
    struct PopDown *p0 = mnu->Selections;
    struct PopDown *pd = mnu->Selections + 2;	/* first 9 items */
    struct PopDown *ca = mnu->Selections + 13;
    int MenuNo = 0;
    WINDOW cwnd;
    mnu->Selection = 0;
    oldFocus = NULL;

    if (GetClass(wnd) != APPLICATION)    {
        oldFocus = wnd;
        /* ----- point to the APPLICATION window ----- */
        if (EditApplication == NULL)
            return;
        /* Application window is the "outer" / top level window. */
        /* It has FirstWindow and LastWindow child windows, and  */
        /* is Parent for them. The child windows are in a linked */
        /* circular list of NextWindow and PrevWindow items...   */
        /* Painting is done first to last, last being topmost... */
        /* normal.c and lists.c take care of that. As we want a  */
        /* STABLE / stacking-independent list, 0.7c has EXTRA    */
        /* links in window number order, maintained by normal.c! */

        //#if CLASSIC_WINDOW_NUMBERING
        // cwnd = FirstWindow(ApplicationWindow);
        //#else /* new 0.7c: stacking-independent window numbering */
        cwnd = NumberOneChildWindow(EditApplication);
        //#endif
        /* ----- get the first 9 document windows ----- */
        while (cwnd != NULL && MenuNo < 9)    {
            if (isVisible(cwnd) && GetClass(cwnd) != MENUBAR && GetClass(cwnd) != STATUSBAR) {
                /* --- add the document window to the menu --- */
                strncpy(Menus[MenuNo]+4, WindowName(cwnd), 20);
                /* fields are listed in menu.h... */
                pd->SelectionTitle = Menus[MenuNo];
                pd->Accelerator = ALT_1 + MenuNo;	/* new 0.7c */
                /* pd->ActionId left as is, pd->help not set at all... */
                if (cwnd == oldFocus)    {
                    /* -- mark the current document -- */
                    pd->Attrib |= CHECKED;
                    mnu->Selection = MenuNo+2;
                } else
                    pd->Attrib &= ~CHECKED;
                pd++;
                MenuNo++;
            } /* if listable window */
        cwnd = NextNumberedWindow(cwnd);
        } /* while in linked list of enumerateable windows */
    } /* build window list popdown menu */

    if (MenuNo)
        p0->SelectionTitle = "~Close all";
    else
        p0->SelectionTitle = NULL;
    if (MenuNo >= 9)    {
        *pd++ = *ca;
        if (mnu->Selection == 0)
            mnu->Selection = 11;
    }
    pd->SelectionTitle = NULL;
}


/* ------------- asciitab.c ------------- */

#if WITH_ASCIITAB

#define ASCIIHEIGHT 20
#define ASCIIWIDTH 41   /* highlights cannot be at very right edge, */
                        /* so ASCIIWIDTH has to be a bit too big, at least 3 chars  */
                        /* after the last highlight, e.g. 41. */

static int ascii_highlight;
static WINDOW ATwnd;


static void DisplayAsciitab(WINDOW wnd)
{
    int x, y, i, ch;
    char content[80];

    SetStandardColor(wnd);
    memset(content, ' ', 80);
    PutWindowLine(wnd, "  + 0 1 2 3 4 5 6 7 8 9 A B C D E F ", 0, 0);
    ch = 0;
    for (y = 0; y < 16; y++) {
        i = 0;
        content[i++] = ' ';
        content[i++] = (y < 10) ? ('0' + y) : ('A' + (y-10));
        content[i++] = '0';
        content[i++] = ' ';
        for (x = 0; x < 16; x++) {
            if (ascii_highlight == ch) {
                content[i++] = CHANGECOLOR;	/* next chars must be 8x 8x */
                content[i++] = SelectForeground(wnd)+0x80;
                content[i++] = SelectBackground(wnd)+0x80;
            }
            /* SelectBackground(wnd)+0x80 thechar ' ' RESETCOLOR now...   */
            content[i++] = ch;	/* see video.h wputs limitations! */
            if ((!content[i-1])
#if 0 /* video.c treats this as non-escape char in THIS context */
                || (content[i-1] == CHANGECOLOR)
#endif
                || (content[i-1] == RESETCOLOR)
#ifdef TAB_TOGGLING /* also used in editor.c and video.c */
                || (content[i-1] == ('\t' | 0x80))
                || (content[i-1] == ('\f' | 0x80))
#endif
                )
                content[i-1] = '*';
            if (ascii_highlight == ch)
                content[i++] = RESETCOLOR;
            content[i++] = ' ';
            ch++;
        } /* x loop */
#if ASCIIWIDTH > 40
        content[i++] = (y < 10) ? ('0' + y) : ('A' + (y-10));
        content[i++] = '0';
        content[i++] = ' ';
#endif
        content[i++] = 0;
        PutWindowLine(wnd, content, 0, y+1);
    } /* y loop */
    content[0] = 0;
    i = ascii_highlight;
    if ((!i)
#if 0 /* video.c treats this as non-escape char in THIS context */
        || (i == CHANGECOLOR)
#endif
        || (i == RESETCOLOR)
#ifdef TAB_TOGGLING
        || (i == ('\t' | 0x80)) || (i == ('\f' | 0x80))
#endif
        ) i = '*';
    sprintf(content,"  Character: Alt-%d (0x%02x) (\\%03o) '%c'    ", ascii_highlight, ascii_highlight, ascii_highlight, (char)i);
    PutWindowLine(wnd, content, 0, 17);
}


static void CreateWindowMsg(WINDOW wnd)
{
    ascii_highlight = 0;
    DisplayAsciitab(wnd);
}


static int KeyboardMsg(WINDOW wnd, PARAM p1)
{
    switch ((int)p1)    {
        case UP:
        case PGUP:
            ascii_highlight -= 16;
            break;
        case DN:
        case PGDN:
            ascii_highlight += 16;
            break;
        case LARROW:
            ascii_highlight--;
            break;
        case RARROW:
            ascii_highlight++;
            break;
        default:
            return FALSE;
    }
#if ASCIIWIDTH < 41
    if ( ((ascii_highlight & 15) == 14) && (p1 == LARROW)) {
        ascii_highlight += 16;
        DisplayAsciitab(wnd);
        ascii_highlight -= 16;
    } /* kludge to properly reset the highlight */
#endif
    if (ascii_highlight < 0) ascii_highlight += 256;
    if (ascii_highlight > 255) ascii_highlight -= 256;
    DisplayAsciitab(wnd);
    return TRUE;
}


static int AsciitabProc(WINDOW wnd,MESSAGE msg, PARAM p1,PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            DefaultWndProc(wnd, msg, p1, p2);
            CreateWindowMsg(wnd);
            return TRUE;
        case KEYBOARD:
            if (KeyboardMsg(wnd, p1))
                return TRUE;
            break;
        case PAINT:
            DefaultWndProc(wnd, msg, p1, p2);
            DisplayAsciitab(wnd);
            return TRUE;
        case COMMAND:
            if ((int)p1 == ID_HELP)    {
                DisplayHelp(wnd, "ASCII Table");
                return TRUE;
            }
            break;
        case CLOSE_WINDOW:
            ATwnd = NULL;
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}


void Asciitable(WINDOW pwnd)
{
    if (ATwnd == NULL)    {
        ATwnd = CreateWindow(PICTUREBOX,
            "ASCII Table (close: ctrl-F4)",
            -1, -1, ASCIIHEIGHT, ASCIIWIDTH,
            NULL, pwnd, AsciitabProc,
            SHADOW | MINMAXBOX | CONTROLBOX | MOVEABLE | HASBORDER
        );
    }
    SendMessage(ATwnd, SETFOCUS, TRUE, 0);
}


/* ------------- calendar.c ------------- */

#ifndef NOCALENDAR

#define CALHEIGHT 17
#define CALWIDTH 33

static int DyMo[] = {31,28,31,30,31,30,31,31,30,31,30,31};
static struct tm ttm, ctm;
static int dys[42];
static WINDOW Cwnd;

#ifndef strftime
static char * nameOfMonth[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
#endif

/* returns 1 if year (0-based, not 1900-based) is a leap year (longer) */
/* -ea */
int isLeapYear(int year) {
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
    offset = ((ttm.tm_mday-1) - ttm.tm_wday) % 7;
    if (offset < 0)
        offset += 7;
    if (offset)
        offset = (offset - 7) * -1;
    /* ----- build the dates into the array ---- */
    for (dy = 1; dy <= DyMo[ttm.tm_mon]; dy++)
        dys[offset++] = dy;
}

static void CreateWindowMsg(WINDOW wnd)
{
    DrawBox(wnd, 1, 2, CALHEIGHT-4, CALWIDTH-4);
    for (int x = 5; x < CALWIDTH-4; x += 4)
        DrawVector(wnd, x, 2, CALHEIGHT-4, FALSE);
    for (int y = 4; y < CALHEIGHT-3; y+=2)
        DrawVector(wnd, 1, y, CALWIDTH-4, TRUE);
}

static void DisplayDates(WINDOW wnd)
{
    int week, day;
    char dyln[10];
    int offset;
    char banner[CALWIDTH-1];
    char banner1[30];

    SetStandardColor(wnd);
    PutWindowLine(wnd, "Sun Mon Tue Wed Thu Fri Sat", 2, 1);
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
    for (week = 0; week < 6; week++)    {
        for (day = 0; day < 7; day++)    {
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

static int KeyboardMsg(WINDOW wnd, PARAM p1)
{
    switch ((int)p1)    {
        case PGUP:
            if (ttm.tm_mon == 0) {
                ttm.tm_mon = 12;
                ttm.tm_year--;
            }
            ttm.tm_mon--;
            FixDate();
            DisplayDates(wnd);
            return TRUE;
        case PGDN:
            ttm.tm_mon++;
            if (ttm.tm_mon == 12) {
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

static int CalendarProc(WINDOW wnd,MESSAGE msg, PARAM p1,PARAM p2)
{
    switch (msg)    {
        case CREATE_WINDOW:
            DefaultWndProc(wnd, msg, p1, p2);
            CreateWindowMsg(wnd);
            return TRUE;
        case KEYBOARD:
            if (KeyboardMsg(wnd, p1))
                return TRUE;
            break;
        case PAINT:
            DefaultWndProc(wnd, msg, p1, p2);
            DisplayDates(wnd);
            return TRUE;
        case COMMAND:
            if ((int)p1 == ID_HELP) {
                DisplayHelp(wnd, "Calendar");
                return TRUE;
            }
            break;
        case CLOSE_WINDOW:
            Cwnd = NULL;
            break;
        default:
            break;
    }
    return DefaultWndProc(wnd, msg, p1, p2);
}

void Calendar(WINDOW pwnd)
{
    if (Cwnd == NULL) {
        time_t tim = time(NULL);
        ttm = *localtime(&tim);
        ctm = ttm;  /* store current calendar day and month */
        Cwnd = CreateWindow(PICTUREBOX,
                    "Calendar (close: ctrl-F4)",
                    -1, -1, CALHEIGHT, CALWIDTH,
                    NULL, pwnd, CalendarProc,
                    SHADOW     |
                    MINMAXBOX  |
                    CONTROLBOX |
                    MOVEABLE   |
                    HASBORDER
        );
    }
    SendMessage(Cwnd, SETFOCUS, TRUE, 0);
}


/* -------------- menus.c ------------- */

/* --------------------- the main menu --------------------- */
DEFMENU(MainMenu)
    /* --------------- the File popdown menu ----------------*/
    POPDOWN( "~File",  PrepFileMenu, "Commands for manipulating files" )
        SELECTION( "~New",        ID_NEW,     CTRL_N, 0) /* 0.7a */
        SELECTION( "~Open...",    ID_OPEN,    CTRL_O, 0) /* 0.7a */
        SEPARATOR
        SELECTION( "~Save",       ID_SAVE,    CTRL_S, INACTIVE) /* 0.7a */
        SELECTION( "Save ~as...", ID_SAVEAS,       0, INACTIVE)
        SELECTION( "~Close",      ID_CLOSE,        0, INACTIVE)
#if DELFILE
        SELECTION( "D~elete",     ID_DELETEFILE,   0, INACTIVE)
#endif
        SEPARATOR
        SELECTION( "~Print",      ID_PRINT,        0, INACTIVE)
        SELECTION( "P~rinter setup...", ID_PRINTSETUP, 0, 0)
        SEPARATOR
        SELECTION( "~DOS Shell",  ID_DOS,          0, 0)
        SELECTION( "E~xit",       ID_EXIT,     ALT_X, 0)
    ENDPOPDOWN

    /* --------------- the Edit popdown menu ----------------*/
    POPDOWN( "~Edit", PrepEditMenu, "Commands for editing files" )
#ifdef HOOKKEYB
        SELECTION( "~Undo",      ID_UNDO,  ALT_BS,    INACTIVE)
#else
        SELECTION( "~Undo",      ID_UNDO,  CTRL_Z,    INACTIVE)
#endif
        SEPARATOR
        SELECTION( "Cu~t",       ID_CUT,   CTRL_X, INACTIVE)
        SELECTION( "~Copy",      ID_COPY,  CTRL_C,  INACTIVE)
        /* ^-- must handle ^C / ^Break as "ignore" to use this */
        SELECTION( "~Paste",     ID_PASTE, CTRL_V, INACTIVE)
        SEPARATOR
        SELECTION( "Cl~ear",     ID_CLEAR, 0,         INACTIVE)
        SELECTION( "~Delete",    ID_DELETETEXT, DEL,  INACTIVE)
        SEPARATOR
        SELECTION( "Pa~ragraph", ID_PARAGRAPH,  ALT_P,INACTIVE)
        /* new 0.7d stuff follows: */
        SELECTION( "Upc~ase Block", ID_UPCASE, 0,     INACTIVE)
        SELECTION( "Do~wncase Block", ID_DOWNCASE, 0, INACTIVE)
        SELECTION( "Stats of ~Block", ID_WORDCOUNT, 0, 0)
    ENDPOPDOWN

    /* --------------- the Search popdown menu ----------------*/
    POPDOWN( "~Search", PrepSearchMenu, "Search and replace text" )
        SELECTION( "~Find", ID_SEARCH,      CTRL_F,    INACTIVE)
       			/* *** CTRL_F added 0.7c, see also editbox.c *** */
        SELECTION( "~Next",      ID_SEARCHNEXT,  F3,   INACTIVE)
        SELECTION( "~Replace",ID_REPLACE,     0,    INACTIVE)
    ENDPOPDOWN

    /* ------------ the Utilities popdown menu --------------- */
    POPDOWN( "~Utilities", NULL, "Utility programs" )
#ifndef NOCALENDAR
        SELECTION( "~Calendar",   ID_CALENDAR,     0,   0)
#endif
#if WITH_ASCIITAB
        SELECTION( "~ASCII Table",   ID_ASCIITAB,     0,   0)	/* new 0.7c */
#endif
    ENDPOPDOWN

    /* ------------- the Options popdown menu ---------------*/
    POPDOWN( "~Options", NULL, "Commands for setting editor and display options" )
        SELECTION( "~Display...",   ID_DISPLAY,     0,      0 )
        SEPARATOR
#ifdef INCLUDE_LOGGING
        SELECTION( "~Log messages", ID_LOG,     ALT_L,      0 )
        SEPARATOR
#endif
        SELECTION( "~Insert",       ID_INSERT,     INS, TOGGLE)
        SELECTION( "~Word wrap",    ID_WRAP,        0,  TOGGLE)
        SELECTION( "~Tabs ( )",     ID_TABS,        0,  CASCADED)
        SEPARATOR
        SELECTION( "~Save options", ID_SAVEOPTIONS, 0,      0 )
    ENDPOPDOWN

    /* --------------- the Window popdown menu --------------*/
    POPDOWN( "~Window", PrepWindowMenu, "Select/close document windows" )
        SELECTION(  NULL,  ID_CLOSEALL, 0, 0)
        SEPARATOR
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
        SELECTION(  "~More Windows...", ID_MOREWINDOWS, 0, 0)
        SELECTION(  NULL,  ID_WINDOW, 0, 0 )
    ENDPOPDOWN

    /* --------------- the Help popdown menu ----------------*/
    POPDOWN( "~Help", NULL, "Get help...really." )
        SELECTION(  "~Help for help...",  ID_HELPHELP,  0, 0 )
        SELECTION(  "~Extended help...",  ID_EXTHELP,   0, 0 )
        SELECTION(  "~Keys help...",      ID_KEYSHELP,  0, 0 )
        SELECTION(  "Help ~index...",     ID_HELPINDEX, 0, 0 )
        SEPARATOR
        SELECTION(  "~About Edit...",          ID_ABOUT,     0, 0 )
        SELECTION(  "~About DFlat+...",          ID_ABOUTDFP,     0, 0 )
    ENDPOPDOWN

    /* ----- cascaded pulldown from Tabs... above ----- */
    CASCADED_POPDOWN( ID_TABS, NULL )
        SELECTION( "raw tabs ~0", ID_TAB0, 0, 0) /* -ea */
        SELECTION( "tab size ~2", ID_TAB2, 0, 0)
        SELECTION( "tab size ~4", ID_TAB4, 0, 0)
        SELECTION( "tab size ~6", ID_TAB6, 0, 0)
        SELECTION( "tab size ~8", ID_TAB8, 0, 0)
    ENDPOPDOWN
ENDMENU
