//+-------------------------------------------------------------------------
//
//  CabinetEx - Windows NT Explorer
//  Copyright (C) Microsoft
//
//  File:       shundoc.h
//
//  History:    Jan-27-25   kfh83  Created
//  
//--------------------------------------------------------------------------

#include <Windows.h>
#include <winnt.h>
#include <dpa_dsa.h>
#include <shtypes.h>



#pragma once


//
//	Macros
//

typedef HANDLE LPSHChangeNotificationLock;

#define FCIDM_DRIVELIST    (FCIDM_BROWSERFIRST + 2) //
#define FCIDM_TREE         (FCIDM_BROWSERFIRST + 3) //
#define FCIDM_TABS         (FCIDM_BROWSERFIRST + 4) //
#define FCIDM_REBAR        (FCIDM_BROWSERFIRST + 5) //
#define FCW_VIEW        0x0004
#define FCW_BROWSER     0x0005
#define FCW_TABS        0x0006
#define STRRET_OLESTR   0x0000
#define STRRET_OFFPTR(pidl,lpstrret) ((LPSTR)((LPBYTE)(pidl)+(lpstrret)->uOffset))


// Change the path of an existing folder.
// wParam:
//      0:              LPARAM is a pidl, handle the message immediately.
//      CSP_REPOST:     LPARAM is a pidl, copy the pidl and handle the
//                      message later.
//      CSP_NOEXECUTE:  if this path is not a folder, fail, don't shell exec
//
//
// lParam: LPITEMIDLIST of path.
//
//
#define CSP_REPOST      0x0001
#define CSP_NONAVIGATE  0x0002
#define CSP_NOEXECUTE   0x0004
#define CWM_SETPATH             (WM_USER + 2)

// Inform the File Cabinet that you want idle messages.
// This should ONLY be used by File Cabinet extensions.
// wParam: app define UINT (passed to FCIDLEPROC).
// lParam: pointer to an FCIDLEPROC.
// return: TRUE if successful; FALSE otherwise
//
#define CWM_WANTIDLE            (WM_USER + 3)

// get or set the FOLDERSETTINGS for a view
// wParam: BOOL TRUE -> set to view info buffer, FALSE -> get view info buffer
// lParam: LPFOLDERSETTINGS buffer to get or set view info
//
#define CWM_GETSETCURRENTINFO   (WM_USER + 4)
#define FileCabinet_GetSetCurrentInfo(_hwnd, _bSet, _lpfs) \
        SendMessage(_hwnd, CWM_GETSETCURRENTINFO, (WPARAM)(_bSet), \
        (LPARAM)(LPFOLDERSETTINGS)_lpfs)

// selects the specified item in the current view
// wParam: SVSI_* flags
// lParam: LPCITEMIDLIST of the item ID, NULL -> all items
//
#define CWM_SELECTITEM          (WM_USER + 5)
#define FileCabinet_SelectItem(_hwnd, _sel, _item) \
    SendMessage(_hwnd, CWM_SELECTITEM, _sel, (LPARAM)(LPCITEMIDLIST)(_item))

// selects the specified path in the current view
// wParam: SVSI_* flags
// lParam: LPCSTR of the display name
//
#define CWM_SELECTPATH          (WM_USER + 6)
#define FileCabinet_SelectPath(_hwnd, _sel, _path) \
        SendMessage(_hwnd, CWM_SELECTPATH, _sel, (LPARAM)(LPCSTR)(_path))

// Get the IShellBrowser object associated with an hwndMain
#define CWM_GETISHELLBROWSER    (WM_USER + 7)
#define FileCabinet_GetIShellBrowser(_hwnd) \
        (IShellBrowser  *)SendMessage(_hwnd, CWM_GETISHELLBROWSER, 0, 0L)

// Onetree notification.                        ;Internal
// since onetree is internal to cabinet, we can no longer use WM_NOTIFY
// codes.
// so we need to reserve a WM_ id nere.
#define CWM_ONETREEFSE          (WM_USER + 8)
//
//  two pidls can have the same path, so we need a compare pidl message
#define CWM_COMPAREPIDL         (WM_USER + 9)
//
//  sent when the global state changes
#define CWM_GLOBALSTATECHANGE   (WM_USER + 10)
//
//  sent to the desktop from a second instance
#define CWM_COMMANDLINE         (WM_USER + 11)
// global clone your current pidl
#define CWM_CLONEPIDL           (WM_USER + 12)
// See if the root of the instance is as specified
#define CWM_COMPAREROOT         (WM_USER + 13)
// Tell desktop our root
#define CWM_SPECIFYCOMPARE      (WM_USER + 14)
// See if the root of the instance matches a hwnd
#define CWM_PERFORMCOMPARE      (WM_USER + 15)
// Forward SHChangeNotify events
#define CWM_FSNOTIFY            (WM_USER + 16)
// Forward SHChangeRegistration events
#define CWM_CHANGEREGISTRATION  (WM_USER + 17)
// For AddToRecentDocs processing by desktop
#define CWM_ADDTORECENT         (WM_USER + 18)
// For SHWaitForFile processing by desktopop
#define CWM_WAITOP              (WM_USER + 19)

// Notify for changes to the fav's folder.
#define CWM_FAV_CHANGE          (WM_USER + 20)



// Help id's for Find File dialog box (shell.dll)

#define NO_HELP				((DWORD) -1) // Disables Help for a control

#define IDH_FINDFILENAME_NAME		2200
#define IDH_FINDFILENAME_LOOKIN 	2201
#define IDH_FINDFILENAME_BROWSE 	2202
#define IDH_FINDFILENAME_TOPLEVEL	2203
#define IDH_FINDFILENAME_FINDNOW	2204
#define IDH_FINDFILENAME_STOP		2205
#define IDH_FINDFILENAME_NEWSEARCH	2206
#define IDH_FINDFILECRIT_OFTYPE 	2207
#define IDH_FINDFILECRIT_CONTTEXT	2208
#define IDH_FINDFILECRIT_SIZEIS 	2209
#define IDH_FINDFILECRIT_K		2210
#define IDH_FINDFILEDATE_ALLFILES	2211
#define IDH_FINDFILEDATE_CREATEORMOD	2212
#define IDH_FINDFILEDATE_DAYS		2213
#define IDH_FINDFILEDATE_FROM		2214
#define IDH_FINDFILEDATE_MONTHS		2215
#define IDH_FINDFILEDATE_RANGE		2216
#define IDH_FINDFILEDATE_TO		2217
#define IDH_FINDCOMP_NAME		2218
#define IDH_FINDCOMP_LOOKIN		2220
#define IDH_FINDCOMP_BROWSE		2221
#define IDH_FINDFILENAME_STATUSSCREEN	2223
#define IDH_BROWSELIST                  2224

/* Get/SetWindowWord/Long offsets for use with WC_DIALOG windows */
#define DWL_MSGRESULT   0
#define DWL_DLGPROC     4
#define DWL_USER        8

#define NM_STARTWAIT            (NM_FIRST-9)
#define NM_ENDWAIT              (NM_FIRST-10)
#define NM_BTNCLK               (NM_FIRST-11)

// Needed for RunFileDlg
#define RFD_NOBROWSE            0x00000001
#define RFD_NODEFFILE           0x00000002
#define RFD_USEFULLPATHDIR      0x00000004
#define RFD_NOSHOWOPEN          0x00000008
#define RFD_WOW_APP             0x00000010
#define RFD_NOSEPMEMORY_BOX     0x00000020

#define STR_DESKTOPINIA         "desktop.ini"

#define WNDCLASS_TRAYNOTIFY     "Shell_TrayWnd"
#define STR_DESKTOPCLASS "Progman"



#define GPD_PPI                 0       // Going away
#define GPD_FLAGS               -4
#define GPD_PARENT              -8
#define GPD_STARTF_FLAGS        -12     // Can be changed
#define GPD_STARTF_POS          -16
#define GPD_STARTF_SIZE         -20
#define GPD_STARTF_SHOWCMD      -24
#define GPD_STARTF_HOTKEY       -28
#define GPD_STARTF_SHELLDATA    -32
#define	GPD_CURR_PROCESS_ID     -36
#define	GPD_CURR_THREAD_ID      -40
#define	GPD_EXP_WINVER          -44
#define GPD_HINST               -48

// 
// For GPD_FLAGS
//
#define GPF_DEBUG_PROCESS   0x00000001
#define GPF_WIN16_PROCESS   0x00000008
#define GPF_DOS_PROCESS     0x00000010
#define GPF_CONSOLE_PROCESS 0x00000020
#define GPF_SERVICE_PROCESS 0x00000100

//
// Some dde stuff
//

#define IT_HSZS                     0
#define IT_CONVS                    1
#define IT_LINKS                    2
#define IT_SVRS                     3

#define IF_HSZ                      0

#define IDS_ACTION_CLEANEDUP        17
#define IDS_ACTION_DESTROYED        18
#define IDS_ACTION_INCREMENTED      19
#define IDS_ACTION_CREATED          20

#define IDS_FMT_SH_MSG1             123
#define IDS_FMT_TRS_MSG1            120
#define IDS_FMT_MSG1                111
#define IDS_FMT_MSG2                112
#define IDS_FMT_TRS_MSG2            121
#define IDS_FMT_TRS_CB1             114
#define IDS_FMT_CB1                 105
#define IDS_FMT_TRS_CB2             115
#define IDS_FMT_CB2                 106
#define IDS_FMT_TRS_CTXT1           116
#define IDS_FMT_CTXT1               107
#define IDS_FMT_TRS_ER1             118
#define IDS_FMT_ER1                 109

#define IDS_HUH                     4
#define IDS_SENT                    21
#define IDS_POSTED                  22
#define IDS_INPUT_DATA              23
#define IDS_OUTPUT_DATA             25
#define IDS_TABDDD                  24
#define IDS_WARM                    26
#define IDS_HOT                     27
#define IDS_UNKNOWN_CALLBACK        28

#define MAX_DISPDATA                48      // max bytes of non-text data to dump

// RUN FILE RETURN values from notify message
#define RFR_NOTHANDLED 0
#define RFR_SUCCESS 1
#define RFR_FAILURE 2

// TTN_FIRST - TTN_LAST defined in commctrl.h (0U-520U) - (OU-549U)
#define RFN_FIRST       (0U-510U) // run file dialog notify
#define RFN_LAST        (0U-519U)

#define SEN_FIRST       (0U-550U)       // ;Internal
#define SEN_LAST        (0U-559U)       // ;Internal

#define SEN_DDEEXECUTE (SEN_FIRST-0)
#define RFN_EXECUTE    (RFN_FIRST - 0)

#define CAPS1         94    /* Extra Caps */

#define C1_REINIT_ABLE      0x0080

#define WM_SHELLNOTIFY                  0x0034
#define SHELLNOTIFY_OLELOADED           0x0002
#define SHELLNOTIFY_OLEUNLOADED         0x0003
#define SHELLNOTIFY_WALLPAPERCHANGED    0x0004

#define RNC_NETWORKS              0x00000001
#define RNC_LOGON                 0x00000002

#define MRU_BINARY              0x0001
#define MRU_CACHEWRITE          0x0002
#define MRU_ANSI                0x0004

#define _IOffset(class, itf)         ((UINT)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class  *)(((LPSTR)pitf)-_IOffset(class, itf)))
#define IToClassN(class, itf, pitf)  IToClass(class, itf, pitf)

#define	PNPBIOS_SERVICE_GETDOCKCAPABILITIES	0x200
#define	PNPBIOS_ERR_SYSTEM_NOT_DOCKED		0x87
#define	PNPBIOS_ERR_CANT_DETERMINE_DOCKING	0x89
#define PNPBIOS_DOCK_CAPABILITY_TEMPERATURE	0x0006



//
// Path processing function
//

#define PPCF_ADDQUOTES               0x00000001        // return a quoted name if required
#define PPCF_ADDARGUMENTS            0x00000003        // appends arguments (and wraps in quotes if required)
#define PPCF_NODIRECTORIES           0x00000010        // don't match to directories
#define PPCF_NORELATIVEOBJECTQUALIFY 0x00000020        // don't return fully qualified relative objects
#define PPCF_FORCEQUALIFY            0x00000040        // qualify even non-relative names

//
// Functions to help the cabinets sync to each other
//  uOptions parameter to SHWaitForFileOpen
//
#define WFFO_WAITTIME 10000L

#define WFFO_ADD        0x0001
#define WFFO_REMOVE     0x0002
#define WFFO_WAIT       0x0004
#define WFFO_SIGNAL     0x0008

//===========================================================================
// Another block of private API
//===========================================================================

// indexes into the shell image lists (Shell_GetImageList) for default images
// If you add to this list, you also need to update II_LASTSYSICON!

#define II_DOCNOASSOC         0         // document (blank page) (not associated)
#define II_DOCUMENT           1         // document (with stuff on the page)
#define II_APPLICATION        2         // application (exe, com, bat)
#define II_FOLDER             3         // folder (plain)
#define II_FOLDEROPEN         4         // folder (open)
#define II_DRIVE525           5
#define II_DRIVE35            6
#define II_DRIVEREMOVE        7
#define II_DRIVEFIXED         8
#define II_DRIVENET           9
#define II_DRIVENETDISABLED  10
#define II_DRIVECD           11
#define II_DRIVERAM          12
#define II_WORLD             13
#define II_NETWORK           14
#define II_SERVER            15
#define II_PRINTER           16
#define II_MYNETWORK         17
#define II_GROUP             18

#define WM_SYSMENU                      0x0313  
#define WM_HOOKMSG                      0x0314  
#define WM_EXITPROCESS                  0x0315  

#define TM_WINDOWDESTROYED	(WM_USER+0x100)
#define TM_POSTEDRCLICK 	(WM_USER+0x101)
#define TM_CONTEXTMENU          (WM_USER+0x102)
#define TM_ACTASTASKSW          (WM_USER+0x104)
#define TM_SYSMENUCOUNT         (WM_USER+0x105)
#define TM_TASKTAB              (WM_USER+0x106)

/* flags for DrawCaption */
#define DC_NOVISIBLE        0x0800  
#define DC_BUTTONS          0x1000  
#define DC_NOSENDMSG        0x2000  
#define DC_CENTER           0x4000  
#define DC_FRAME            0x8000  
#define DC_CAPTION          (DC_ICON | DC_TEXT | DC_BUTTONS) 
#define DC_NC               (DC_CAPTION | DC_FRAME) 

#define HELP_HIDE_WINDOW        0x0010
#define HELP_SHOW_WINDOW        0x0011
#define HELP_DEMO_DEST          0x0012

/* The following are sent to the application */

#define HELP_MAIN_HWND          0x0300
#define HELP_SECONDARY_HWND     0x0301
#define HELP_REQUESTS_DEMO      0x0302
#define HELP_REQUESTS_MORE      0x0303

#define WM_WINHELP           0x0038     // ;Internal WinNT

/* Default IDs for original User images */
#define OIC_FIRST_DEFAULT           100 
#define OIC_APPLICATION_DEFAULT     100 
#define OIC_HAND_DEFAULT            101 
#define OIC_WARNING_DEFAULT         101 
#define OIC_QUESTION_DEFAULT        102 
#define OIC_EXCLAMATION_DEFAULT     103 
#define OIC_ERROR_DEFAULT           103 
#define OIC_ASTERISK_DEFAULT        104 
#define OIC_INFORMATION_DEFAULT     104 
#define OIC_WINLOGO_DEFAULT         105 
#define COIC_CONFIGURABLE           (OIC_WINLOGO_DEFAULT - OIC_FIRST_DEFAULT + 1) 

// Startmenu images.
#define II_STPROGS           19
#define II_STDOCS            20
#define II_STSETNGS          21
#define II_STFIND            22
#define II_STHELP            23
#define II_STRUN             24
#define II_STSUSPD           25
#define II_STEJECT           26
#define II_STSHUTD           27

#define II_SHARE             28
#define II_LINK              29
#define II_READONLY          30
#define II_RECYCLER          31
#define II_RECYCLERFULL      32
#define II_RNA               33
#define II_DESKTOP           34

// More startmenu image.
#define II_STCPANEL          35
#define II_STSPROGS          36
#define II_STPRNTRS          37
#define II_STFONTS           38
#define II_STTASKBR          39

#define II_CDAUDIO           40
#define II_TREE              41
#define II_STCPROGS          42

//
//  Valid power management levels.
//

#define PMLEVEL_ADVANCED                0
#define PMLEVEL_STANDARD                1
#define PMLEVEL_OFF                     2
#define PMLEVEL_MAXIMUM                 PMLEVEL_OFF

#define NI_SIGNATURE    0x34753423                                     
#define TVIS_ALL 0xFF7F

#define VPOWERD_IOCTL_GET_PM_LEVEL              0x00000003

#define FMAI_SEPARATOR          0x00000001

#define SHELLFILENAME TEXT("\\\\.\\SHELL")

//
//  IOCTL codes
//
#define WSHIOCTL_GETVERSION       0
#define WSHIOCTL_BLUESCREEN       1
#define WSHIOCTL_GET1APPINFO      2
#define WSHIOCTL_SIGNALSEM        3
#define WSHIOCTL_MAX              4        /* Remember, _MAX = _LIMIT + 1 */

// Tray CopyData Messages
#define TCDM_APPBAR     0x00000000
#define TCDM_NOTIFY     0x00000001
#define TCDM_LOADINPROC 0x00000002

#define VDATEINPUTBUF(ptr, type, const)

#define IsBadInterfacePtr(pitf, ITF)  SHIsBadInterfacePtr(pitf, sizeof(ITF##Vtbl))

#define SHCNNU_SET        1   // Set the notify list to passed in list
#define SHCNNU_ADD        2   // Add the items to the current list
#define SHCNNU_REMOVE     3   // Remove the items from the current list
#define SHCNF_PRINTJOBA   0x0004        // dwItem1: printer name
                                        // dwItem2: SHCNF_PRINTJOB_DATA
#define SHCNF_PRINTJOBW   0x0007        // dwItem1: printer name
                                        // dwItem2: SHCNF_PRINTJOB_DATA
#define SHCNF_INSTRUMENT  0x0080        // dwItem1: LPSHCNF_INSTRUMENT
#define SHCNF_NONOTIFYINTERNALS 0x4000 // means don't do shell notify internals.  see comments in code

#define SetWaitCursor()   hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT))
#define ResetWaitCursor() SetCursor(hcursor_wait_cursor_save)

#define ASSERT(f)

#define PXM_DRAGDROP    WM_USER
#define PXM_FOUNDOLE    (WM_USER+1)

#define HINST_THISDLL   g_hinst

//-------- drive type identification --------------
// iDrive      drive index (0=A, 1=B, ...)
//
#define DRIVE_CDROM     5           // extended DriveType() types
#define DRIVE_RAMDRIVE  6
#define DRIVE_TYPE      0x000F      // type masek
#define DRIVE_SLOW      0x0010      // drive is on a slow link
#define DRIVE_LFN       0x0020      // drive supports LFNs
#define DRIVE_AUTORUN   0x0040      // drive has AutoRun.inf in root.
#define DRIVE_AUDIOCD   0x0080      // drive is a AudioCD
#define DRIVE_AUTOOPEN  0x0100      // should *always* auto open on insert
#define DRIVE_NETUNAVAIL 0x0200     // Network drive that is not available
#define DRIVE_SHELLOPEN  0x0400     // should auto open on insert, if shell has focus
#define DRIVE_SECURITY   0x0800     // Supports ACLs
#define DRIVE_COMPRESSED 0x1000     // Root of volume is compressed
#define DRIVE_ISCOMPRESSIBLE 0x2000 // Drive supports compression (not nescesarrily compressed)


#define DriveIsAutoRun(iDrive)      (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_AUTORUN)
#define DriveIsShellOpen(iDrive)    (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_SHELLOPEN)
#define DriveIsAutoOpen(iDrive)     (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_AUTOOPEN)
#define Drives_IsAutoRun(_pidd) (DriveIsAutoRun(CDrives_GetDriveIndex((LPIDDRIVE)(_pidd))))

#define SHID_JUNCTION           0x80

#define SHID_GROUPMASK          0x70
#define SHID_TYPEMASK           0x7f
#define SHID_INGROUPMASK        0x0f
#define SHID_COMPUTER           0x20

#define SHCR_CMD_REGISTER   1
#define SHCR_CMD_DEREGISTER 2

#define NTHDATA(p, n) (*((LPBYTE FAR *)((LPBYTE)p+sizeof(MRUDATA))+n))
#define pMRU ((PMRUDATA)hMRU)
#define DATAPDATA(p) 	(p+sizeof(DWORD))
#define BASE_CHAR TEXT('a')
#define MRU_ORDERDIRTY 0x1000

#define strchr StrChr

#define DATASIZE(p)	(*((LPDWORD)p))

#define DRIVEID(path)   ((path[0] - 'A') & 31)

#define GetClassCursor(hwnd)    ((HCURSOR)GetClassLong(hwnd, GCLP_HCURSOR))

#define TOKEN_NOEXPRESSION 0x8000

#define     GetWindowStyle(hwnd)    ((DWORD)GetWindowLong(hwnd, GWL_STYLE))

#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

#define V_HIMAGELIST(himl)

#define V_HIMAGELISTVOID(himl)

#define V_HIMAGELISTERR(himl, err)

#define ISSEP(c)   ((c) == TEXT('=')  || (c) == TEXT(','))
#define ISWHITE(c) ((c) == TEXT(' ')  || (c) == TEXT('\t') || (c) == TEXT('\n') || (c) == TEXT('\r'))

#define QUOTE   TEXT('"')
#define COMMA   TEXT(',')

#define ResultFromShort(i)  ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(i)))

#define ShortFromResult(r)  (short)SCODE_CODE(GetScode(r))

//
//	Structs
//

typedef struct {
    NMHDR  hdr;
    WCHAR  szCmd[MAX_PATH * 2];
    DWORD  dwHotKey;
} NMVIEWFOLDER, FAR *LPNMVIEWFOLDER;

typedef struct {
    NMHDR hdr;
    LPCWSTR lpszCmd;
    LPCWSTR lpszWorkingDir;
    int nShowCmd;
} NMRUNFILE, FAR* LPNMRUNFILE;

struct {                           /* profile data */
    BOOL fOutput[3];
    BOOL fFilter[5];
    BOOL fTrack[4];
    BOOL fTerse;
} pro;

typedef struct _MRUINFOA {
    DWORD cbSize;
    UINT uMax;
    UINT fFlags;
    HKEY hKey;
    LPCSTR lpszSubKey;
    int lpfnCompare;
} MRUINFO, FAR* LPMRUINFO;

struct BIOSPARAMSTAG {
    DWORD bp_ret;
    WORD* bp_pTableSize;
    char* bp_pTable;
};

typedef struct _ARD {
    DWORD   dwOffsetPath;
    DWORD   dwOffsetPidl;
} XMITARD, * PXMITARD;

typedef struct BIOSPARAMSTAG BIOSPARAMS;
typedef struct BIOSPARAMSTAG* PBIOSPARAMS;


#ifndef	MAX_PROFILE_LEN
#define	MAX_PROFILE_LEN	80
#endif

struct	HWProfileInfo_s {
    ULONG	HWPI_ulHWProfile;			// the profile handle
    char	HWPI_szFriendlyName[MAX_PROFILE_LEN];	// the friendly name
    DWORD	HWPI_dwFlags;				// CM_HWPI_* flags
};

typedef	struct	HWProfileInfo_s	       HWPROFILEINFO;
typedef	struct	HWProfileInfo_s* PHWPROFILEINFO;
typedef	struct	HWProfileInfo_s*PFARHWPROFILEINFO;

typedef struct _TRAYNOTIFYDATA
{ 
        DWORD dwSignature; 
        DWORD dwMessage; 
        NOTIFYICONDATA nid; 
} TRAYNOTIFYDATA, * PTRAYNOTIFYDATA; 

typedef struct _BLUESCREENINFO {    /* bsi */

    TCHAR* pszText;         /* Message text (OEM character set) */
    TCHAR* pszTitle;        /* Message title (OEM character set) */
    /* NULL means "Windows" */
    DWORD  flStyle;         /* Message box flags (see windows.h) */
    /* Add'l flags defined in ddk\inc\shell.h */

} BLUESCREENINFO;

typedef struct tagSHCNF_PRINTJOB_DATA {
    DWORD JobId;
    DWORD Status;
    DWORD TotalPages;
    DWORD Size;
    DWORD PagesPrinted;
} SHCNF_PRINTJOB_DATA, FAR* LPSHCNF_PRINTJOB_DATA;

//
// Flags for SHGetSetSettings
//

typedef struct {
    BOOL fShowAllObjects : 1;
    BOOL fShowExtensions : 1;
    BOOL fNoConfirmRecycle : 1;
    BOOL fShowCompColor : 1;
    UINT fRestFlags : 13;

    LPSTR pszHiddenFileExts;
    UINT cbHiddenFileExts;
} DRATSNFARTS, * FARTSNDRATS;

typedef struct _TRAYAPPBARDATA
{
    APPBARDATA abd;
    DWORD dwMessage;
    HANDLE hSharedRect;
    DWORD dwProcId;
} TRAYAPPBARDATA, * PTRAYAPPBARDATA;


typedef struct _FSNotifyPerProc
{
    HANDLE  htStarting;
    DWORD   idtStarting;
    HANDLE  htRunning;
    DWORD   idtRunning;             // invalid if htRunning is NULL
    UINT    cclients;               // number of registered client
    int     iCallbackCount;
    HDSA    hdsaIntEvents;
    HDSA    hdsaIntClients;
    DWORD   dwLastFlush;
    BOOL    fFlushNow : 1;
    HANDLE  hCallbackEvent;         // iCallbackCount == 0 ? SetEvent : ResetClear
} FSNotifyPerProc;

typedef struct _DROPTARGETINFO  // dti
{
    HWND hwndTarget;
    HWND hwndProxyTarget;
    LPDROPTARGET pdtgt;
    UINT idProcess;
    UINT idThread;
    BOOL fRegisteredToOLE;
} DROPTARGETINFO, FAR* LPDROPTARGETINFO;

typedef struct _shmapheader {
    DWORD   dwSize;
} SHMAPHEADER, * LPSHMAPHEADER;

typedef struct _IMAGELIST
{
    DWORD       wMagic;     //
    int         cImage;     // count of images in image list
    int         cAlloc;     // # of images we have space for
    int         cGrow;      // # of images to grow bitmaps by
    int         cx;         // width of each image
    int         cy;         // height
    int         cStrip;     // # images in horizontal strip
    UINT        flags;      // ILC_* flags
    COLORREF    clrBlend;   // last blend color
    COLORREF    clrBk;      // bk color or CLR_NONE for transparent.
    HBRUSH      hbrBk;      // bk brush or black
    BOOL        fSolidBk;   // is the bkcolor a solid color (in hbmImage)
    HBITMAP     hbmImage;   // all images are in here
    HBITMAP     hbmMask;    // all image masks are in here.
    HDC         hdcImage;
    HDC         hdcMask;
    int         aOverlayIndexes[4];    // array of special images
    int         aOverlayX[4];          // x offset of image
    int         aOverlayY[4];          // y offset of image
    int         aOverlayDX[4];         // cx offset of image
    int         aOverlayDY[4];         // cy offset of image
    int         aOverlayF[4];          // ILD_ flags for image

#ifdef _WIN32
    //
    // virtual imagelist support
    //
    LPARAM          lParamFilter;

    //
    // used for "blending" effects on a HiColor display.
    // assumes layout of a DIBSECTION.
    //
    // BUGBUG on a non-hicolor system we dont need to allocate this!!
    //
    struct {
        BITMAP              bm;
        BITMAPINFOHEADER    bi;
        DWORD               ct[256];
    }   dib;
#endif

} IMAGELIST, NEAR* HIMAGELIST;

typedef BOOL(CALLBACK* PRLCALLBACK)(HDPA hdpa, HKEY hkey, LPCTSTR pszKey,
    LPCTSTR pszValueName, LPTSTR pszValue, LPCTSTR pszSrc,
    LPCTSTR pszDest);

typedef struct _RLPI    // Registry List Process Info
{
    HDPA    hdpaRLList;             // The dpa of items
    BOOL    fCSInitialized;         // Have we initialized the CS in this process
    BOOL    fListValid;             // Is the list up to date and valid
    CRITICAL_SECTION csRLList;      // The critical section for the process
} RLPI;

typedef struct
{
    // these two must be together and at the front
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlExtra;
    LONG  lEvent;
    UINT cRef;
} FSNotifyEvent;

typedef struct _DRAGCONTEXT {   // drgc
    HWND        hwndProxyTarget;
    HWND        hwndHit;
    HWND        hwndProxySource;
    HWND        hwndOwner;
    DWORD       grfKeyState;
    POINTL      ptl;
    DWORD       dwEffect;
    DWORD       dwSrcEffect;
    LPDATAOBJECT pdtobjOrg;     // original data object (source process)
    LPDATAOBJECT pdtobj;        // marshalled data object (target process)
    LPVOID      pvDataObject;
    DWORD       dwProcessID;    // of calling process
    BOOL        fDropped;       // indicates whether it is dropped or not.
#ifdef WIN31_DROPTARGET
    BOOL        fHDrop;         // the data object contains an HDROP
    HWND        hwnd31Target;   // might be different from hwndTarget
#endif
} DRAGCONTEXT, FAR* LPDRAGCONTEXT;

typedef struct _IDDRIVE
{
    WORD    cb;
    BYTE    bFlags;
    CHAR    cName[4];
    __int64 qwSize;  // this is a "guess" at the disk size and free space
    __int64 qwFree;
    WORD    wChecksum;
} IDDRIVE;
typedef const UNALIGNED IDDRIVE* LPCIDDRIVE;

typedef struct _BitBucketDataEntryW {
    CHAR szShortName[MAX_PATH]; // original filename shortened
    int iIndex;                 // index (key to name)
    int idDrive;                // which drive bucket it's currently in
    FILETIME ft;
    DWORD dwSize;
    WCHAR szOriginal[MAX_PATH]; // original filename
} BBDATAENTRY, * LPBBDATAENTRY;

typedef struct _BitBucketDriveInfo {
    LPITEMIDLIST pidl;  // points to the BitBucket dir for this drive

    int         cFiles;  // counts the current "delete operation"

    DWORD       dwSize;   // how much stuff is in this bit bucket
    int         iPercent;
    ULONGLONG   cbMaxSize; // maximum size of bitbucket (in bytes)
    DWORD       dwClusterSize;
    BOOL        fNukeOnDelete;

    // for the write cache
    HDPA        hdpaDeleteCache; // deleted files.  not yet written to info file

    // for the read cache
    LPBBDATAENTRY lpbbdeRead; // cached read of bitbucket
    int         cReadCount;
    BOOL        fReadDirty;
    UINT        cbDataEntrySize;    // size of each data entry
} BBDRIVEINFO, * LPBBDRIVEINFO;

typedef struct _SHChangeRegistration {
    UINT    uCmd;
    ULONG   ulID;
    HWND    hwnd;
    UINT    uMsg;
    DWORD   fSources;
    LONG    lEvents;
    BOOL    fRecursive;
    UINT    uidlRegister;
} SHChangeRegistration, * LPSHChangeRegistration;

typedef struct _FSNCI FSNotifyClientInfo, * PFSNotifyClientInfo;

typedef struct _FSNCI
{
    PFSNotifyClientInfo pfsnciNext;
    HWND                hwnd;
    ULONG               ulID;
    DWORD               dwProcID;
    int                 fSources;
    int                 iSerializationFlags;
    LONG                fEvents;
    WORD                wMsg;
    HDSA                hdsaNE;
    HDPA                hdpaPendingEvents;
} FSNotifyClientInfo, * PFSNotifyClientInfo;

//
//  We don't start the event-dispatch timer until an
//  event occurs.
//
typedef struct _FSNotify
{
    UINT    cRefClientList;
    DWORD   dwLastEvent;
    HDSA    hdsaThreadAwake;
    FSNotifyClientInfo* pfsnciFirst;
    HDPA    hdpaIntEvents;
    ULONG   ulNextID;
} FSNotify;

typedef struct tagMRUDATA
{
    UINT fFlags;
    UINT uMax;
    LPVOID lpfnCompare;
    HKEY hKey;
#ifdef DEBUG
    TCHAR szSubKey[32];
#endif
    LPTSTR cOrder;
} MRUDATA, * PMRUDATA;

typedef struct {
    LPCITEMIDLIST pidl;
    int iCount;                 // how many of these events have we had?
} FSIntEvent, * LPFSIntEvent;

typedef struct _IDREGITEM
{
    WORD    cb;
    BYTE    bFlags;
    BYTE    bReserved;      // This is to get DWORD alignment
    CLSID   clsid;
} IDREGITEM, * LPIDREGITEM;

typedef const IDREGITEM* LPCIDREGITEM;
typedef struct _IDLREGITEM
{
    IDREGITEM       idri;
    USHORT          cbNext;
} IDLREGITEM;





//
//	Enums
//

typedef enum
{
    FMF_NONE = 0x0000,
    FMF_NOEMPTYITEM = 0x0001,
    FMF_INCLUDEFOLDERS = 0x0002,
    FMF_NOPROGRAMS = 0x0004,
    FMF_FILESMASK = 0x0007,
    FMF_LARGEICONS = 0x0008,
    FMF_NOBREAK = 0x0010,
    FMF_NOABORT = 0x0020,
} FMFLAGS;


//
//	Function definitions
//


BOOL WINAPI SHIsBadInterfacePtr(LPCVOID pv, UINT cbVtbl);
void SHChangeNotifyReceive(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
HRESULT WINAPI SHRevokeDragDrop(HWND hwnd);
HRESULT WINAPI SHRegisterDragDrop(HWND hwnd, LPDROPTARGET pdtgt);
int WINAPI SHOutOfMemoryMessageBox(HWND hwndOwner, LPTSTR pszTitle, UINT fuStyle);
LPVOID SHLockSharedUnimpl(HANDLE  hData, DWORD   dwSourceProcessId);
BOOL SHUnlockSharedUnimpl(LPVOID  lpvData);
HANDLE SHAllocSharedUnimpl(LPCVOID lpvData, DWORD   dwSize, DWORD   dwDestinationProcessId);
BOOL SHFreeSharedUnimpl(HANDLE hData, DWORD dwSourceProcessId);
BOOL WINAPI SHWinHelpUnimpl(HWND hwndMain, LPCTSTR lpszHelp, UINT usCommand, DWORD ulData);
LPTSTR WINAPI PathFindFileNameUnimpl(LPCTSTR pPath);
BOOL  WINAPI PathIsRootUnimpl(LPCTSTR pPath);
BOOL WINAPI PathIsRelativeUnimpl(LPCTSTR lpszPath);
int WINAPI PathGetDriveNumberUnimpl(LPCTSTR lpsz);
LPTSTR WINAPI PathFindExtensionUnimpl(LPCTSTR pszPath);
void WINAPI PathUnquoteSpacesUnimpl(LPTSTR lpsz);
BOOL WINAPI PathRemoveFileSpecUnimpl(LPTSTR pFile);
void PathRemoveExtensionUnimpl(LPTSTR pszPath);
void WINAPI PathRemoveBlanksUnimpl(LPTSTR lpszString);
void PathRemoveArgsUnimpl(LPTSTR pszPath);
int WINAPI PathParseIconLocationUnimpl(LPTSTR pszIconFile);
__inline BOOL PathIsUNCUnimpl(LPWSTR psz);
BOOL WINAPI PathIsDirectoryUnimpl(LPCTSTR pszPath);
LPTSTR WINAPI PathGetArgsUnimpl(LPCTSTR pszPath);
BOOL WINAPI PathFileExistsUnimpl(LPCTSTR lpszPath);
LPTSTR WINAPI PathCombineUnimpl(LPTSTR lpszDest, LPCTSTR lpszDir, LPCTSTR lpszFile);
LPSTR WINAPI PathBuildRootUnimpl(LPSTR szRoot, int iDrive);
BOOL WINAPI PathAppendUnimpl(LPTSTR pPath, LPCTSTR pMore);
LPTSTR WINAPI PathAddBackslashUnimpl(LPTSTR lpszPath);
DWORD WINAPI PathIsUNCServerShareUnimpl(LPCTSTR pszPath);
BOOL PathStripToRootUnimpl(LPTSTR szRoot);
BOOL PathCanonicalizeUnimpl(LPTSTR lpszDst, LPCTSTR lpszSrc);
LONG WINAPI PathProcessCommandUnimpl(LPCTSTR lpSrc, LPTSTR lpDest, int iDestMax, DWORD dwFlags);
void NearRootFixupsUnimpl(LPTSTR lpszPath, BOOL fUNC);
void WINAPI GetEffectiveClientRectUnimpl(HWND hWnd, LPRECT lprc, LPINT lpInfo);
BOOL WINAPI ImageList_GetIconSizeUnimpl(IMAGELIST* piml, int FAR* cx, int FAR* cy);
int CALLBACK _CompareStringsUnimpl(LPVOID sz1, LPVOID sz2, LPARAM lparam);
BOOL CALLBACK _RLBuildListCallBackUnimpl(HDPA hdpa, HKEY hkey, LPCTSTR pszKey, LPCTSTR pszValueName, LPTSTR pszValue, LPCTSTR pszSource, LPCTSTR pszDest);
BOOL _SHChangeNotifyAddEventToClientQueuesUnimpl(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
BOOL WINAPI RLBuildListOfPathsUnimpl();
size_t ualstrlenW(UNALIGNED const WCHAR* wcs);
UNALIGNED WCHAR* ualstrcpynUnimpl(UNALIGNED WCHAR* lpString1, UNALIGNED const WCHAR* lpString2, int iMaxLength);
UNALIGNED WCHAR* ualstrcpyUnimpl(UNALIGNED WCHAR* dst, UNALIGNED const WCHAR* src);
LRESULT CALLBACK TargetProxyWndProcUnimpl(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND _CreateProxyWindowUnimpl();
LRESULT TargetProxy_OnDragDropUnimpl(LPDRAGCONTEXT pdrgc, WPARAM wParam);
int WINAPI StrToInt(LPCTSTR lpSrc);
void WINAPI InvalidateDriveType(int iDrive);
void WINAPI ILGlobalFree(LPITEMIDLIST pidl);
LPITEMIDLIST WINAPI ILGlobalClone(LPCITEMIDLIST pidl);
PVOID Alloc(ULONG   Bytes);
BOOL Free(PVOID   Buffer);
void WINAPI SHAbortInvokeCommand();
LONG WINAPI StrToLong(LPCTSTR lpSrc);
DWORD WINAPI GetProcessDword(DWORD idProcess, LONG iIndex);
int CDrives_GetDriveIndex(LPCIDDRIVE pidd);
int RealDriveTypeFlags(int iDrive, BOOL fOKToHitNet);
int WINAPI SHHandleDiskFull(HWND hwnd, int idDrive);
STDAPI SHLoadOLE(LPARAM lParam);
BOOL WINAPI SHChangeRegistrationReceive(HANDLE hChangeRegistration, DWORD dwProcId);
BOOL WINAPI SHChangeNotifyDeregisterInternal(ULONG ulID);
ULONG WINAPI SHChangeNotifyRegisterInternal(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, int cEntries, void* pfsne);
int WINAPI AddMRUData(HANDLE hMRU, const void FAR* lpData, UINT cbData);
HANDLE WINAPI CreateMRUList(LPMRUINFO lpmi);
int WINAPI FindMRUData(HANDLE hMRU, const void FAR* lpData, UINT cbData, LPINT lpiSlot);
__inline BOOL DBL_BSLASH(LPCTSTR psz);
void FSNRemoveInterruptEvent(LPCITEMIDLIST pidl);
LPCTSTR GetPCEnd(LPCTSTR lpszStart);
BOOL IsILShared(LPCITEMIDLIST pidl, BOOL fUpdateCache);
LPCTSTR PCStart(LPCTSTR lpszStart, LPCTSTR lpszEnd);
HRESULT WINAPI ILLoadFromStream(LPSTREAM pstm, LPITEMIDLIST* ppidl);
HANDLE MapHandle(HANDLE hData, DWORD dwSource, DWORD dwDest, DWORD dwDesiredAccess, DWORD dwFlags);
LPDROPTARGETINFO SHDrag_FindDropTarget(HWND hwndTarget, int* pi, BOOL fInContext);
STDAPI SHFlushClipboard(void);
int WINAPI StrToOleStr(LPOLESTR pwsz, LPCTSTR psz);
int WINAPI StrToOleStrN(LPOLESTR pwsz, int cchWideChar, LPCTSTR psz, int cchMultiByte);
BOOL WINAPI StrRetToStrN(LPTSTR szOut, UINT uszOut, LPSTRRET pStrRet, LPCITEMIDLIST pidl);
int WINAPI OleStrToStrN(LPTSTR psz, int cchMultiByte, LPCOLESTR pwsz, int cchWideChar);
BOOL WINAPI ParseField(LPCTSTR szData, int n, LPTSTR szBuf, int iBufLen);
DWORD WINAPI SHWaitForFileToOpen(LPCITEMIDLIST pidl, UINT uOptions, DWORD dwTimeout);
void WINAPI SetAppStartingCursor(HWND hwnd, BOOL bSet);
void WINAPI ReceiveAddToRecentDocs(HANDLE hARD, DWORD dwProcId);


// LOAD_MODULE shiz
BOOL(STDMETHODCALLTYPE* SHFindComputer)(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile);
BOOL(STDMETHODCALLTYPE* RegisterShellHook)(HWND hwnd, BOOL fInstall);
BOOL(STDMETHODCALLTYPE* FileIconInit)(BOOL fRestoreCache);
INT(STDMETHODCALLTYPE* RunFileDlg)(HWND hwndParent, HICON hIcon, LPCTSTR pszWorkingDir, LPCTSTR pszTitle, LPCTSTR pszPrompt, DWORD dwFlags);
HRESULT(STDMETHODCALLTYPE* ExitWindowsDialog)(HWND hwndParent);
BOOL(WINAPI* DrawCaptionTempW)(HWND, HDC, LPRECT, HFONT, HICON, LPWSTR, UINT);
BOOL(WINAPI* ILGetDisplayName)(LPCITEMIDLIST pidl, LPTSTR pszPath);





//
// Function loader
//
BOOL SHUndocInit(void);